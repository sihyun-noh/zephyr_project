#ifndef __SYS_MSG_TYPES_H__
#define __SYS_MSG_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "sys_core.h"
#include "events.h"

typedef struct {
  msg_header_t header;
  event_type_t event_type;
  event_data_t event_data;
  uint32_t id;
  uint32_t timestamp;
} event_msg_t;

#ifdef __cplusplus
}
#endif

#endif /* __SYS_MSG_TYPES_H__ */
