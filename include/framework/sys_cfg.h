/**
 * @file sys_cfg.h
 * @author Jeff Yoon
 * @brief
 *  This file includes platform abstraction for non-volatile storage of settings.
 * @version 0.1
 * @date 2023-10-24
 *
 * @copyright Copyright (c) 2023 SEEDFIC
 *
 */

#include <stdio.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>

#ifndef __SYSCFG_H__
#define __SYSCFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NVS_PARTITION storage_partition
#define NVS_PARTITION_DEVICE FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET FIXED_PARTITION_OFFSET(NVS_PARTITION)

/**
 * @brief Performs any intialization for the nvs setting, if necessary.
 *
 *
 */
int syscfg_init(void);

/**
 * @brief Performs any de-intialization for the nvs setting, if necessary.
 *
 *
 */
void syscfg_deinit(void);

/**
 * @brief Fetches the value of a setting.
 *
 * This function fetches the value of the setting indetified by @p key
 * and write it to the memory pointed to by value.
 * It then writes the length to the integer pointed to by @p value_len.
 * The initial value of @p value_len is the maximum number of bytes to be
 * written to @p value.
 *
 * This function can be used to check for the existance of a key without fetching
 * the value by setting @p value and @p value_len to NULL.
 * You cam also check the length of the setting without fetching it by setting
 * only value to NULL.
 *
 * @param[in]     id        The index of the specific item to get.
 * @param[in]     key       The key associated with the requested setting.
 * @param[out]    value     A poiner to where the value of the setting should be written.
 *                          May be set to NULL if just testing for the presence or length of a setting.
 * @param[in,out] value_len A pointer to the length of the value. When called, this pointer should point an
 *                          integer containing the maximum value size that can be written to @p value.
 *                          At return, the actual length of the setting is written. This may be set to NULL
 *                          if performing a presence check.
 */
int syscfg_get(const char *key, char *value, size_t value_len);

int syscfg_set(const char *key, const char *value);

int syscfg_unset(const char *key);

int syscfg_get_blob(const char *key, void *value, size_t value_len);

int syscfg_set_blob(const char *key, const void *value, size_t value_len);

#ifdef __cplusplus
}
#endif

#endif /* __SYSCFG_H__ */
