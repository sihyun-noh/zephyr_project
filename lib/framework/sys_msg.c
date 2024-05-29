#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sys_msg, LOG_LEVEL_DBG);

#include <framework/buffer_pool.h>
#include <framework/sys_msg.h>

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void deallocate_on_error(msg_t *p_msg, BaseType_t status);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
BaseType_t sysmsg_send(msg_t *p_msg) {
  BaseType_t result = msg_send(p_msg->header.rx_id, p_msg);
  deallocate_on_error(p_msg, result);
  SYSCORE_ASSERT(result == SYS_SUCCESS);

  return result;
}

BaseType_t sysmsg_try_to_send(msg_t *p_msg) {
  BaseType_t result = msg_send(p_msg->header.rx_id, p_msg);
  deallocate_on_error(p_msg, result);

  return result;
}

BaseType_t sysmsg_sendto(msg_t *p_msg, mid_t dest_id) {
  p_msg->header.rx_id = dest_id;
  BaseType_t result = msg_send(p_msg->header.rx_id, p_msg);
  deallocate_on_error(p_msg, result);
  SYSCORE_ASSERT(result == SYS_SUCCESS);

  return result;
}

BaseType_t sysmsg_unicast(msg_t *p_msg) {
  BaseType_t result = msg_unicast(p_msg);
  deallocate_on_error(p_msg, result);
  SYSCORE_ASSERT(result == SYS_SUCCESS);

  return result;
}

BaseType_t sysmsg_create_and_send(mid_t tx_id, mid_t rx_id, msg_code_t code) {
  BaseType_t result = SYS_ERROR;

  msg_t *p_msg = (msg_t *)buffer_pool_take(sizeof(msg_t));
  SYSCORE_ASSERT(p_msg != NULL);

  if (p_msg != NULL) {
    SYS_MSG_HEADER_INIT(p_msg, code, tx_id);
    result = msg_send(rx_id, p_msg);
    deallocate_on_error(p_msg, result);
    SYSCORE_ASSERT(result == SYS_SUCCESS);
  }

  return result;
}

BaseType_t sysmsg_create_and_sendto_self(mid_t id, msg_code_t code) {
  BaseType_t result = SYS_ERROR;

  msg_t *p_msg = (msg_t *)buffer_pool_take(sizeof(msg_t));
  SYSCORE_ASSERT(p_msg != NULL);

  if (p_msg != NULL) {
    SYS_MSG_HEADER_INIT(p_msg, code, id);
    p_msg->header.rx_id = id;
    result = msg_send(id, p_msg);
    deallocate_on_error(p_msg, result);
    SYSCORE_ASSERT(result == SYS_SUCCESS);
  }

  return result;
}

BaseType_t sysmsg_unicast_create_and_send(mid_t tx_id, msg_code_t code) {
  BaseType_t result = SYS_ERROR;

  msg_t *p_msg = (msg_t *)buffer_pool_take(sizeof(msg_t));
  SYSCORE_ASSERT(p_msg != NULL);

  if (p_msg != NULL) {
    SYS_MSG_HEADER_INIT(p_msg, code, tx_id);
    result = msg_unicast(p_msg);
    deallocate_on_error(p_msg, result);
    SYSCORE_ASSERT(result == SYS_SUCCESS);
  }

  return result;
}

BaseType_t sysmsg_create_and_broadcast(mid_t tx_id, msg_code_t code) {
  BaseType_t result = SYS_ERROR;

  size_t size = sizeof(msg_t);
  msg_t *p_msg = buffer_pool_take(size);

  if (p_msg != NULL) {
    SYS_MSG_HEADER_INIT(p_msg, code, tx_id);
    p_msg->header.rx_id = MSG_ID_RESERVED;
    result = msg_broadcast(p_msg, size);
    deallocate_on_error(p_msg, result);
  }

  return result;
}

BaseType_t sysmsg_reply(msg_t *p_msg, msg_code_t code) {
  BaseType_t result = SYS_ERROR;

  mid_t swap = p_msg->header.rx_id;
  p_msg->header.rx_id = p_msg->header.tx_id;
  p_msg->header.tx_id = swap;
  p_msg->header.msg_code = code;

  result = msg_send(p_msg->header.rx_id, p_msg);
  deallocate_on_error(p_msg, result);
  SYSCORE_ASSERT(result == SYS_SUCCESS);

  return result;
}

BaseType_t sysmsg_cb_create_and_send(mid_t tx_id, mid_t rx_id, msg_code_t code, void (*cb)(uint32_t), uint32_t cb_data) {
  BaseType_t result = SYS_ERROR;

  size_t size = sizeof(cb_msg_t);
  cb_msg_t *p_msg = buffer_pool_take(size);

  if (p_msg != NULL) {
    p_msg->header.msg_code = code;
    p_msg->header.rx_id = rx_id;
    p_msg->header.tx_id = tx_id;
    p_msg->header.options = MSG_OPTION_CALLBACK;
    p_msg->callback = cb;
    p_msg->data = cb_data;
    if (rx_id == MSG_ID_RESERVED) {
      result = msg_unicast((msg_t *)p_msg);
    } else {
      result = msg_send(rx_id, (msg_t *)p_msg);
    }
    deallocate_on_error((msg_t *)p_msg, result);
  }

  return result;
}

BaseType_t sysmsg_filtered_targeted_send(msg_t *p_msg, mid_t *target_id, size_t msg_size) {
  BaseType_t result;

  if (target_id == NULL) {
#ifdef CONFIG_FILTER
    /* With filtering, send targeted message to filter */
    p_msg->header.rx_id = MSG_ID_EVENT_FILTER;
    result = msg_send(MSG_ID_EVENT_FILTER, p_msg);
#else
    /* Without filtering, send broadcast */
    p_msg->header.rx_id = MSG_ID_RESERVED;
    result = msg_broadcast(p_msg, msg_size);
#endif
  } else {
    /* Targeted message, send only to target */
    p_msg->header.rx_id = *target_id;
    result = msg_send(*target_id, p_msg);
  }

  deallocate_on_error(p_msg, result);
  SYSCORE_ASSERT(result == SYS_SUCCESS);

  return result;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void deallocate_on_error(msg_t *p_msg, BaseType_t status) {
  if (status != SYS_SUCCESS) {
    buffer_pool_free(p_msg);
  }
}
