#ifndef __BUFFER_POOL_H__
#define __BUFFER_POOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/kernel.h>
#include <stddef.h>

/*******************************************************************/
/* Global Function Prototypes                                      */
/*******************************************************************/
struct bp_stats {
  bool initialized;
  int space_available;
  int min_space_available;
  int max_size;
  int min_size;
  int allocs;
  int cur_allocs;
  int max_allocs;
  int take_failures;
  int last_fail_size;
#if CONFIG_BUFFER_POOL_WINDOW_SIZE > 0
  size_t windex;
  uint16_t window[CONFIG_BUFFER_POOL_WINDOW_SIZE];
#endif
};

#define BP_CONTEXT_UNUSED "NA"
#define BP_TRY_TO_TAKE(s) buffer_pool_try_to_take(s, __func__)

/**
 * @brief Prepares sys buffer pool for use.
 */
void buffer_pool_init(void);

/**
 * @brief Waits up to timeout to allocate a buffer of at least
 * size bytes and return a pointer.
 * The buffer is set to zero.
 * This function won't assert if a buffer can't be taken.
 *
 * @param size in bytes
 * @param timeout zephyr timeout
 * @param context for printing warning when buffer can't be allocated
 * @return void *
 */
void *buffer_pool_try_to_take_timeout(size_t size, k_timeout_t timeout, const char *const context);

/**
 * @brief Allocates a buffer of at least size bytes and returns a pointer.
 * The buffer is set to zero.
 * This function won't assert if a buffer can't be taken.
 *
 * @param size in bytes
 * @param context for printing warning when buffer can't be allocated
 * @return void *
 */
void *buffer_pool_try_to_take(size_t size, const char *const context);

/**
 * @brief Allocates a buffer of at least size bytes and returns a pointer.
 * The buffer is set to zero.
 * This function will assert if a buffer can't be taken.
 */
void *buffer_pool_take(size_t size);

/**
 * @brief Put a buffer back into the free pool.
 */
void buffer_pool_free(void *p_buffer);

/**
 * @brief Get pointer to buffer pool statistics
 *
 * @param index of buffer pool.  Only 0 is valid at this time.
 * @param stats pointer to stats that will be copied into by this function.
 *
 * @return 0 on success, otherwise negative
 */
int buffer_pool_get_stats(uint8_t index, struct bp_stats *stats);

#ifdef __cplusplus
}
#endif

#endif /* __BUFFER_POOL_H__ */
