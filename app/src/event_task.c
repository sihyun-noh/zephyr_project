#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(event_task, LOG_LEVEL_DBG);

#define THIS_FILE "Event"

#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include <framework/msg_ids.h>
#include <framework/buffer_pool.h>
#include <framework/sys_msg_macros.h>
#include <framework/sys_msg_types.h>

/**************************************************************************************************/
/* Local Constant, Macro and Type Definitions                                                     */
/**************************************************************************************************/
#ifndef EVENT_TASK_PRIORITY
#define EVENT_TASK_PRIORITY K_PRIO_PREEMPT(1)
#endif

#ifndef EVENT_TASK_STACK_DEPTH
#define EVENT_TASK_STACK_DEPTH 4096
#endif

#ifndef EVENT_TASK_QUEUE_DEPTH
#define EVENT_TASK_QUEUE_DEPTH 32
#endif

typedef struct {
  msg_task_t msg_task;
} event_ctx_t;

/**************************************************************************************************/
/* Local Data Definitions                                                                         */
/**************************************************************************************************/
static event_ctx_t event_ctx;

K_THREAD_STACK_DEFINE(event_task_stack, EVENT_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(event_task_queue, MSG_QUEUE_ENTRY_SIZE, EVENT_TASK_QUEUE_DEPTH, MSG_QUEUE_ALIGNMENT);

static uint32_t event_task_event_id = 0;

/**************************************************************************************************/
/* Local Function Prototypes                                                                      */
/**************************************************************************************************/
static void event_task_thread(void *, void *, void *);
static dispatch_result_t event_msg_handler(msg_recv_t *p_msg_rxer, msg_t *p_msg);

static void send_event_msg_lorawan(sensor_event_t *event);

// System message dispatcher
static msg_handler_t *event_task_msg_dispatcher(msg_code_t msg_code) {
  /* clang-format off */
  switch (msg_code) {
    case SMC_INVALID:         return sys_unknown_msg_handler;
    case SMC_EVENT_TRIGGER:   return event_msg_handler;
    default:                  return NULL;
  }
  /* clang-format on */
}

/**************************************************************************************************/
/* Global Function Definitions                                                                    */
/**************************************************************************************************/
void event_task_init(void) {
  event_ctx.msg_task.rxer.id = MSG_ID_EVENT_TASK;
  event_ctx.msg_task.rxer.rx_block_ticks = K_FOREVER;
  event_ctx.msg_task.rxer.p_msg_dispatcher = event_task_msg_dispatcher;
  event_ctx.msg_task.timer_duration_ticks = K_MSEC(1000);
  event_ctx.msg_task.timer_period_ticks = K_MSEC(0);
  event_ctx.msg_task.rxer.p_queue = &event_task_queue;

  // register the event message task to the message handler task list.
  msg_register_task(&event_ctx.msg_task);

  event_ctx.msg_task.p_tid =
      k_thread_create(&event_ctx.msg_task.data, event_task_stack, K_THREAD_STACK_SIZEOF(event_task_stack),
                      event_task_thread, &event_ctx, NULL, NULL, EVENT_TASK_PRIORITY, 0, K_NO_WAIT);
  k_thread_name_set(event_ctx.msg_task.p_tid, THIS_FILE);
}

/**************************************************************************************************/
/* Local Function Definitions                                                                     */
/**************************************************************************************************/
static void event_task_thread(void *p_arg1, void *p_arg2, void *p_arg3) {
  event_ctx_t *p_event = (event_ctx_t *)p_arg1;

  while (true) {
    msg_receiver(&p_event->msg_task.rxer);
  }
}

static dispatch_result_t event_msg_handler(msg_recv_t *p_msg_rxer, msg_t *p_msg) {
  ARG_UNUSED(p_msg_rxer);
  sensor_event_t event;

  event_msg_t *p_event_msg = (event_msg_t *)p_msg;

  event.type = p_event_msg->event_type;
  event.data = p_event_msg->event_data;
  // event.timestamp = get_epoch_time();

  send_event_msg_lorawan(&event);

  return DISPATCH_OK;
}

static void send_event_msg_lorawan(sensor_event_t *event) {
  /* Now post the event to the control task */
  event_msg_t *p_event_msg = (event_msg_t *)buffer_pool_take(sizeof(event_msg_t));
  if (p_event_msg != NULL) {
    p_event_msg->header.msg_code = SMC_SENSOR_EVENT;
    p_event_msg->header.tx_id = MSG_ID_SENSOR_TASK;
    p_event_msg->header.rx_id = MSG_ID_CONTROL_TASK;
    p_event_msg->event_type = event->type;
    p_event_msg->event_data = event->data;
    p_event_msg->id = event_task_event_id++;
    p_event_msg->timestamp = event->timestamp;
    SYSMSG_SEND(p_event_msg);
  }
}
