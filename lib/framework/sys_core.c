#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sys_core, LOG_LEVEL_DBG);

#include <framework/buffer_pool.h>
#include <framework/sys_core.h>

#define MAX_MSG_RECVS 32

typedef struct msg_task_entries {
  msg_recv_t *p_msg_recv;
  bool in_use;
} msg_task_entries_t;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static int sys_initialize(const struct device *p_device);

static void periodic_timer_callback_isr(struct k_timer *p_arg);

static msg_task_entries_t msg_task_registry[MAX_MSG_RECVS];

/*****************************************************/
/* Global Function Definitions                       */
/*****************************************************/
SYS_INIT(sys_initialize, POST_KERNEL, 0);

void msg_register_receiver(msg_recv_t *p_rxer) {
  if (p_rxer == NULL) {
    SYSCORE_ASSERT(FORCED);
    return;
  }
  if (p_rxer->id >= MAX_MSG_RECVS) {
    SYSCORE_ASSERT(FORCED);
    return;
  }

  int key = irq_lock();
  {
    /* Waste some memory (ids are constant)
     * so that a for loop isn't required to look up msg task in table. */
    if (!msg_task_registry[p_rxer->id].in_use) {
      msg_task_registry[p_rxer->id].in_use = true;
      msg_task_registry[p_rxer->id].p_msg_recv = p_rxer;
    } else {
      SYSCORE_ASSERT(FORCED);
    }
  }
  irq_unlock(key);
}

void msg_register_task(msg_task_t *p_msg_task) {
  if (p_msg_task == NULL) {
    SYSCORE_ASSERT(FORCED);
    return;
  }
  msg_register_receiver(&p_msg_task->rxer);
  k_timer_init(&p_msg_task->timer, periodic_timer_callback_isr, NULL);
}

BaseType_t msg_send(mid_t rx_id, msg_t *p_msg) {
  BaseType_t ret = SYS_ERROR;
  if (p_msg == NULL) {
    SYSCORE_ASSERT(FORCED);
    return ret;
  }
  if (rx_id >= MAX_MSG_RECVS) {
    SYSCORE_ASSERT(FORCED);
    return ret;
  }
  if (!msg_task_registry[rx_id].in_use) {
    SYSCORE_ASSERT(FORCED);
    return ret;
  }

  msg_recv_t *p_msg_rxer = msg_task_registry[rx_id].p_msg_recv;
  if (p_msg_rxer != NULL) {
    p_msg->header.rx_id = rx_id;
    ret = msg_queue(p_msg_rxer->p_queue, &p_msg, K_NO_WAIT);
  }
  return ret;
}

BaseType_t msg_unicast(msg_t *p_msg) {
  BaseType_t ret = SYS_ERROR;

  if (p_msg == NULL) {
    SYSCORE_ASSERT(FORCED);
    return ret;
  }

  uint32_t i;
  for (i = MSG_ID_APP_START; i < MAX_MSG_RECVS; i++) {
    msg_recv_t *p_msg_rxer = msg_task_registry[i].p_msg_recv;

    if (p_msg_rxer != NULL && p_msg_rxer->p_msg_dispatcher != NULL) {
      /* The handler isn't called here.
       * It is only used to find the task the message belongs to.
       */
      msg_handler_t *msg_handler = p_msg_rxer->p_msg_dispatcher(p_msg->header.msg_code);

      /* If there is a dispatcher, then send the message to that task. */
      if (msg_handler != NULL) {
        p_msg->header.rx_id = p_msg_rxer->id;
        ret = msg_queue(p_msg_rxer->p_queue, &p_msg, K_NO_WAIT);
        break;
      }
    }
  }
  return ret;
}

BaseType_t msg_broadcast(msg_t *p_msg, size_t msg_size) {
  BaseType_t ret = SYS_ERROR;
  msg_recv_t *p_msg_rxer;
  msg_handler_t *p_msg_handler;
  msg_t *p_new_msg;
  bool accept;
  uint32_t i;

  if (p_msg == NULL) {
    SYSCORE_ASSERT(FORCED);
    return ret;
  }

#if CONFIG_SYS_ASSERT_ON_BROADCAST_FROM_ISR
  if (sys_interrupt_context()) {
    SYSCORE_ASSERT(FORCED);
    return ret;
  }
#endif

  for (i = MSG_ID_APP_START; i < MAX_MSG_RECVS; i++) {
    p_msg_rxer = msg_task_registry[i].p_msg_recv;

    if (p_msg_rxer != NULL && p_msg_rxer->p_msg_dispatcher != NULL) {
      /* The handler isn't called here. It is only used to determine
       * if a task should receive a broadcast message.
       */
      p_msg_handler = p_msg_rxer->p_msg_dispatcher(p_msg->header.msg_code);

      /* In general a task shouldn't have a handler for a message that it
       * doesn't want. However, a task that blocks for long periods may want to
       * filter the broadcasts that it accepts to keep the size of its message
       * queue small.
       */
      if (p_msg_rxer->accept_broadcast != NULL) {
        accept = p_msg_rxer->accept_broadcast(p_msg);
      } else {
        accept = true;
      }

      /* If there is a dispatcher and the task wants the message,
       * then create a copy of the message and place it on the queue.
       */
      if (p_msg_handler != NULL && accept) {
        p_new_msg = buffer_pool_try_to_take(msg_size, __func__);
        if (p_new_msg != NULL) {
          memcpy(p_new_msg, p_msg, msg_size);
          p_new_msg->header.rx_id = p_msg_rxer->id;
          ret = msg_queue(p_msg_rxer->p_queue, &p_new_msg, K_NO_WAIT);
          if (ret != SYS_SUCCESS) {
            buffer_pool_free(p_new_msg);
          }
        }
      }
    }
  }

  /* Free original message memory only when all messages were routed.
   * This contional is here because the message free should occur in
   * application code when the result returned is SYS_ERROR.
   */
  if (ret == SYS_SUCCESS) {
    buffer_pool_free(p_msg);
  }

  return ret;
}

BaseType_t msg_queue(msgq_t *p_queue, void *pp_data, TickType_t block_ticks) {
  BaseType_t status;
  msg_t *p_msg;
  struct k_msgq_attrs attrs;

  if (p_queue == NULL) {
    SYSCORE_ASSERT(FORCED);
    return SYS_ERROR;
  }

  if (pp_data == NULL) {
    SYSCORE_ASSERT(FORCED);
    return SYS_ERROR;
  }

  p_msg = *((msg_t **)pp_data);
  if (p_msg == NULL) {
    SYSCORE_ASSERT(FORCED);
    return SYS_ERROR;
  }

  if (p_msg->header.msg_code == SMC_INVALID) {
    SYSCORE_ASSERT(FORCED);
    return SYS_ERROR;
  }

  if (sys_interrupt_context()) {
    status = k_msgq_put(p_queue, pp_data, K_NO_WAIT);
  } else {
    status = k_msgq_put(p_queue, pp_data, block_ticks);
  }

  if (status != 0) {
    k_msgq_get_attrs(p_queue, &attrs);
    LOG_ERR("Unable to queue message code %u to task %u (%u/%u): %d", p_msg->header.msg_code, p_msg->header.rx_id,
            attrs.used_msgs, attrs.max_msgs, status);
  }

  return status;
}

BaseType_t msg_recv(msgq_t *p_queue, void *pp_data, TickType_t block_ticks) {
  if (p_queue == NULL) {
    SYSCORE_ASSERT(FORCED);
    return SYS_ERROR;
  }

  if (pp_data == NULL) {
    SYSCORE_ASSERT(FORCED);
    return SYS_ERROR;
  }

  if (sys_interrupt_context()) {
    return k_msgq_get(p_queue, pp_data, K_NO_WAIT);
  } else {
    return k_msgq_get(p_queue, pp_data, block_ticks);
  }
}

void msg_receiver(msg_recv_t *p_rxer) {
  if (p_rxer == NULL) {
    SYSCORE_ASSERT(FORCED);
    return;
  }

  dispatch_result_t res = DISPATCH_ERROR;
  msg_t *p_msg = NULL;

  BaseType_t status = msg_recv(p_rxer->p_queue, &p_msg, p_rxer->rx_block_ticks);

  if ((status == SYS_SUCCESS) && (p_msg != NULL)) {
    msg_handler_t *msg_handler = p_rxer->p_msg_dispatcher(p_msg->header.msg_code);
    if (msg_handler != NULL) {
      res = msg_handler(p_rxer, p_msg);
      if (p_msg->header.options & MSG_OPTION_CALLBACK) {
        cb_msg_t *p_cb_msg = (cb_msg_t *)p_msg;
        if (p_cb_msg->callback != NULL) {
          p_cb_msg->callback(p_cb_msg->data);
        }
      }
    } else {
      res = sys_unknown_msg_handler(p_rxer, p_msg);
    }

    if (res != DISPATCH_DO_NOT_FREE) {
      LOG_INF("Free message buffer!!!");
      buffer_pool_free(p_msg);
    }
  }
}

BaseType_t msg_queue_is_empty(mid_t rx_id) {
  if (rx_id >= MAX_MSG_RECVS) {
    return 1;
  }
  if (!msg_task_registry[rx_id].in_use) {
    return 1;
  }

  msgq_t *p_queue = msg_task_registry[rx_id].p_msg_recv->p_queue;
  return ((k_msgq_num_used_get(p_queue) == 0) ? 1 : 0);
}

size_t msg_flush(mid_t rx_id) {
  if (rx_id >= MAX_MSG_RECVS) {
    return 0;
  }
  if (!msg_task_registry[rx_id].in_use) {
    return 0;
  }

  msg_t *p_msg;
  size_t purged = 0;

  while (true) {
    p_msg = NULL;
    k_msgq_get(msg_task_registry[rx_id].p_msg_recv->p_queue, &p_msg, K_NO_WAIT);
    if (p_msg != NULL) {
      buffer_pool_free(p_msg);
      purged += 1;
    } else {
      break;
    }
  }
  return purged;
}

void msg_start_timer(msg_task_t *p_msg_task) {
  if (p_msg_task == NULL) {
    SYSCORE_ASSERT(FORCED);
    return;
  }

  LOG_INF("timer duration = %lld, periodic = %lld", p_msg_task->timer_duration_ticks.ticks,
          p_msg_task->timer_period_ticks.ticks);
  LOG_INF("call msg_start_timer!!!");

  k_timer_start(&p_msg_task->timer, p_msg_task->timer_duration_ticks, p_msg_task->timer_period_ticks);
}

void msg_stop_timer(msg_task_t *p_msg_task) {
  if (p_msg_task == NULL) {
    SYSCORE_ASSERT(FORCED);
    return;
  }

  k_timer_stop(&p_msg_task->timer);
}

void msg_change_timer_period(msg_task_t *p_msg_task, TickType_t duration, TickType_t period) {
  if (p_msg_task == NULL) {
    SYSCORE_ASSERT(FORCED);
    return;
  }

  p_msg_task->timer_duration_ticks = duration;
  p_msg_task->timer_period_ticks = period;
  k_timer_start(&p_msg_task->timer, p_msg_task->timer_duration_ticks, p_msg_task->timer_period_ticks);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/**
 * @brief Initialize buffer pool (statistics).
 */
static int sys_initialize(const struct device *p_device) {
  ARG_UNUSED(p_device);

  buffer_pool_init();

  return 0;
}

/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
static void periodic_timer_callback_isr(struct k_timer *p_arg) {
  msg_task_t *p_msg_task = (msg_task_t *)CONTAINER_OF(p_arg, msg_task_t, timer);

  msg_t *p_msg = (msg_t *)buffer_pool_try_to_take(sizeof(msg_t), __func__);

  if (p_msg != NULL) {
    LOG_INF("Take a buffer, tx,rx_id = %d", p_msg_task->rxer.id);
    p_msg->header.msg_code = SMC_PERIODIC;
    p_msg->header.tx_id = p_msg_task->rxer.id;
    p_msg->header.rx_id = p_msg_task->rxer.id;
    if (msg_send(p_msg_task->rxer.id, p_msg) != SYS_SUCCESS) {
      LOG_ERR("Failed to send message!!!");
      buffer_pool_free(p_msg);
      SYSCORE_ASSERT(FORCED);
    } else {
      LOG_INF("Success to send message!!!");
    }
  } else {
    LOG_WRN("Failed to get Message buffer!!!");
  }
}

__weak void sys_assertion_handler(char *file, int line) {
  UNUSED_PARAMETER(file);
  UNUSED_PARAMETER(line);

  LOG_ERR("assertion: line: %d %s", line, file);

  sys_reboot(SYS_REBOOT_WARM);
}

__weak bool sys_interrupt_context(void) {
  return k_is_in_isr();
}

__weak void sys_reset(void) {
  return;
}

__weak dispatch_result_t sys_unknown_msg_handler(msg_recv_t *p_msg_rxer, msg_t *p_msg) {
  LOG_WRN("Unknown message %u sent to task: %u", p_msg->header.msg_code, p_msg_rxer->id);
  return DISPATCH_ERROR;
}
