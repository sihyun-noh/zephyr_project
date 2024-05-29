#ifndef __SYS_MSG_H__
#define __SYS_MSG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include <zephyr/kernel.h>

#include "sys_core.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define SYS_MSG_HEADER_INIT(p, c, i)                                           \
  do {                                                                         \
    SYSCORE_ASSERT((p) != NULL);                                               \
    p->header.msg_code = (c);                                                  \
    p->header.rx_id = MSG_ID_RESERVED;                                         \
    p->header.tx_id = (i);                                                     \
    p->header.options = MSG_OPTION_NONE;                                       \
  } while (0)

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

/**
 * @brief Wrapper for sysmsg_send
 *
 * @param p_msg pointer to a sys message
 *
 * @retval SYS_SUCCESS or SYS_ERROR
 */
BaseType_t sysmsg_send(msg_t *p_msg);

/**
 * @brief Wrapper for sysmsg_send that doesn't assert if dest queue is full.
 *
 * @param p_msg pointer to a sys message
 *
 * @retval SYS_SUCCESS or SYS_ERROR
 */
BaseType_t sysmsg_try_to_send(msg_t *p_msg);

/**
 * @brief Wrapper for sysmsg_send when used with SYS_MSG_HEADER_INIT.
 *
 * @param p_msg pointer to a framework message
 * @param dest_id is used to set p_msg->header.rx_id
 *
 * @retval SYS_SUCCESS or SYS_ERROR
 */
BaseType_t sysmsg_sendto(msg_t *p_msg, mid_t dest_id);

/**
 * @brief Wrapper for sysmsg_unicast.
 *
 * @param p_msg pointer to a sys message
 *
 * @retval SYS_SUCCESS or SYS_ERROR
 */
BaseType_t sysmsg_unicast(msg_t *p_msg);

/**
 * @brief Allocates message from buffer pool and sends it using sysmsg_send.
 *
 * @param tx_id source of message
 * @param rx_id destination of message
 * @param code message type
 *
 * @retval SYS_SUCCESS or SYS_ERROR
 */
BaseType_t sysmsg_create_and_send(mid_t tx_id, mid_t rx_id, msg_code_t code);

/**
 * @brief Shorter form of sysmsg_create_and_send that can be used by a task to
 * send a message to itself.
 *
 * @param id source and destination of message
 * @param code message type
 *
 * @retval SYS_SUCCESS or SYS_ERROR
 */
BaseType_t sysmsg_create_and_sendto_self(mid_t id, msg_code_t code);

/**
 * @brief Allocates message from buffer pool and sends it using
 * sysmsg_unicast.
 *
 * @param tx_dd source of message
 * @param code message type
 *
 * @retval SYS_SUCCESS or SYS_ERROR
 */
BaseType_t sysmsg_unicast_create_and_send(mid_t tx_id, msg_code_t code);

/**
 * @brief Allocates message from buffer pool and broadcasts it using
 * sysmsg_broadcast.
 *
 * @param tx_id source of message
 * @param code message type
 *
 * @retval SYS_SUCCESS or SYS_ERROR
 */
BaseType_t sysmsg_create_and_broadcast(mid_t tx_id, msg_code_t code);

/**
 * @brief Swaps rxId and txId, changes message code, and then sends using
 * sysmsg_send.
 *
 * @note Often used with DISPATCH_DO_NOT_FREE.
 * @note The original sender must populate the tx_id.
 *
 * @param p_msg pointer to a sys message
 * @param code message type
 *
 * @retval SYS_SUCCESS or SYS_ERROR
 */
BaseType_t sysmsg_reply(msg_t *p_msg, msg_code_t code);

/**
 * @brief Allocates callback message from buffer pool and sends it
 *
 * @param tx_id source of message
 * @param rx_id destination of message
 * @param code message type
 * If message type is FWK_ID_RESERVED, then unicast is used.
 * @param callback function called in receiver context
 * @param callback_data passed into callback function
 *
 * @retval SYS_SUCCESS or SYS_ERROR
 */
BaseType_t sysmsg_cb_create_and_send(mid_t tx_id, mid_t rx_id, msg_code_t code,
                                     void (*cb)(uint32_t), uint32_t cb_data);

/**
 * @brief Sends a targeted sys message to a specific target or if no
 * target is provided, the event filter (or broadcast if event filter is not
 * enabled)
 *
 * @param p_msg pointer to a sys message
 * @param target_id pointer to target if sending a targeted message, NULL
 * otherwise (for broadcast/filter target)
 * @param msg_size size (in bytes) of message
 *
 * @retval SYS_SUCCESS or SYS_ERROR
 */
BaseType_t sysmsg_filtered_targeted_send(msg_t *p_msg, mid_t *target_id,
                                         size_t msg_size);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_MSG_H__ */
