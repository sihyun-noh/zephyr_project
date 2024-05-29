#ifndef __SYS_MSG_MACROS_H__
#define __SYS_MSG_MACROS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "sys_core.h"
#include "sys_msg.h"

/******************************************************************************/
/* Macro Definitions                                                          */
/******************************************************************************/
/**
 * @param rxId - The destination (receiver) of the message.
 * @param txId - The source of the message
 * @param code - The message code
 * @param pMacroMsg - A previously created message that is ready to send.
 * @param cb - Callback function
 * @param data - Callback data
 */

#define SYSMSG_SEND_TO_SELF(rxId, code)                                        \
  sysmsg_create_and_sendto_self(rxId, code)

#define SYSMSG_SEND(pMacroMsg) sysmsg_send((msg_t *)pMacroMsg)

#define SYSMSG_TRY_TO_SEND(pMacroMsg) sysmsg_try_to_send((msg_t *)pMacroMsg)

#define SYSMSG_SEND_TO(destId, pMacroMsg)                                      \
  sysmsg_sendto((msg_t *)pMacroMsg, destId)

#define SYSMSG_UNICAST(pMacroMsg) sysmsg_unicast((msg_t *)pMacroMsg)

#define SYSMSG_UNICAST_CREATE_AND_SEND(txId, code)                             \
  sysmsg_unicast_create_and_send(txId, code)

#define SYSMSG_CREATE_AND_SEND(txId, rxId, code)                               \
  sysmsg_create_and_send(txId, rxId, code)

#define SYSMSG_CREATE_AND_BROADCAST(id, code)                                  \
  sysmsg_create_and_broadcast(id, code)

#define SYSMSG_REPLY(pMacroMsg, code) sysmsg_reply((msg_t *)pMacroMsg, code)

#define SYSMSG_CALLBACK_CREATE_AND_SEND(txId, rxId, code, cb, data)            \
  sysmsg_cb_create_and_send(txId, rxId, code, cb, data)

#define SYSMSG_CALLBACK_CREATE_AND_UNICAST(txId, code, cb, data)               \
  sysmsg_cb_create_and_send(txId, MSG_ID_RESERVED, code, cb, data)

#ifdef __cplusplus
}
#endif

#endif /* __SYS_MSG_MACROS_H__ */
