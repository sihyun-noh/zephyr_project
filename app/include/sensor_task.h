/**
 * @file sensor_task.h
 * @brief
 *
 * Copyright (c) 2023 SEED FIC
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __SENSOR_TASK_H__
#define __SENSOR_TASK_H__

// #include <zephyr/types.h>
// #include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief The setup of the thread parameters
 */
void sensor_task_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_TASK_H__ */
