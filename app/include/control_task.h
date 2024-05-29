#ifndef __CONTROL_TASK_H__
#define __CONTROL_TASK_H__

// #include <zephyr/types.h>
#include <stdbool.h>

#include <framework/sys_cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/

/** The main thread can either have a stack large enough to do initialization,
 * or the control thread can be the "main" thread, or
 */
#ifndef CONTROL_TASK_USES_MAIN_THREAD
#define CONTROL_TASK_USES_MAIN_THREAD 0
#endif

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief Initialize control task and all other application tasks.
 */
void control_task_init(void);

/**
 * @brief Run main/control thread.
 */
void control_task_thread(void);

#ifdef __cplusplus
}
#endif

#endif /* __CONTROL_TASK_H__ */
