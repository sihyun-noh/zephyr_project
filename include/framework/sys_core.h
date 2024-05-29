#ifndef __SYS_CORE_H__
#define __SYS_CORE_H__

#include <stdbool.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UNUSED_VARIABLE(x) ((void)(x))
#define UNUSED_PARAMETER(x) UNUSED_VARIABLE(x)
#define UNUSED_RETURN_VALUE(x) UNUSED_VARIABLE(x)

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef NUL
#define NUL ('\0')
#endif

enum sys_status {
  SYS_SUCCESS = 0,
  SYS_ERROR,
};

/*******************************************************/
/* Framework Assertions                                */
/*******************************************************/
/* clang-format off */
#if CONFIG_SYS_ASSERT_ENABLED_USE_ZEPHYR
  #define SYSCORE_ASSERT(expr) __ASSERT(expr, "Syscore Assertion")
#elif CONFIG_SYS_ASSERT_ENABLED
  /* Shortened file names make it easier to support assertion that
   * print the file name on processors with limited flash space. */
  #ifndef FNAME
    #ifndef SYS_FNAME
      #define FNAME __FILE__
    #else
      #define FNAME SYS_FNAME
    #endif
  #endif

  #define SYSCORE_ASSERT(expr) do { \
    if( !(expr) ) { sys_assertion_handler(FNAME, __LINE__); } \
  } while(0)
#else
  #define SYSCORE_ASSERT(expr)
#endif

/* Force an assertion to fire */
#define FORCED 0

/* clang-format on */

/* System Message Codes (SMC) */
/* clang-format off */
#define SMC_INVALID             0
#define SMC_PERIODIC            1
#define SMC_SW_RESET            2
#define SMC_WTD_CHALLENGE       3
#define SMC_WTD_RESPOSE         4
#define SMC_VALUE_CHANGED       5
#define SMC_FACTORY_RESET       6
#define SMC_APP_SPECIFIC_START  8
#define SMC_JOIN_LORAWAN        9
#define SMC_SEND_DATA_LORAWAN   10
#define SMC_READ_POWER          11
#define SMC_SENSOR_MEASURE      12
#define SMC_SENSOR_EVENT        13
#define SMC_EVENT_TRIGGER       14
#define SMC_SENSOR_CHECK        15
/* clang-format on */

typedef uint8_t msg_code_t;
typedef uint8_t mid_t;

#define BaseType_t int
#define TickType_t k_timeout_t

enum msg_option {
  MSG_OPTION_NONE = 0,
  MSG_OPTION_CALLBACK = BIT(0),
};

typedef enum dispatch_result_enum {
  DISPATCH_OK = 0,
  DISPATCH_ERROR,
  DISPATCH_DO_NOT_FREE,
} dispatch_result_t;

typedef struct msg_header {
  msg_code_t msg_code;
  mid_t rx_id;
  mid_t tx_id;
  uint8_t options;
} msg_header_t;

typedef struct msg {
  msg_header_t header;
  /* Optional payload */
} msg_t;

/* A generic system message with a buffer */
typedef struct msg_buf {
  msg_header_t header;
  size_t size;      /** number of bytes allocated for buffer */
  size_t length;    /** number of used bytes in buffer */
  uint8_t buffer[]; /** size is determined by allocator */
} msg_buf_t;

/* system callback message
 * Callback occurs in msg receiver context.
 * Receiver may not know about callback.
 *
 * Can be used to give a semaphore or set an event.
 */
typedef struct cb_msg {
  msg_header_t header;
  void (*callback)(uint32_t data);
  uint32_t data;
} cb_msg_t;

/*
 * Each message task ahs a message handler or dispatcher.
 * The dispatcher should be implemented using a case statement
 * so that the number of messages doesn't affect the time to determine
 * what handler to call.
 */
typedef struct msg_recv msg_recv_t;
typedef dispatch_result_t msg_handler_t(msg_recv_t *p_msg_rxer, msg_t *p_msg);

/*
 * System Message Receiver Object
 * The dispatcher determines what function should handle each message.
 */
typedef struct k_msgq msgq_t;

struct msg_recv {
  mid_t id;
  msgq_t *p_queue;
  TickType_t rx_block_ticks;
  msg_handler_t *(*p_msg_dispatcher)(msg_code_t msg_code);
  bool (*accept_broadcast)(const msg_t *p_msg);
};

/**
 * @brief System Message Task Object
 *
 * Contains a receiver object, task/thread information, and a timer
 */
typedef struct msg_task {
  msg_recv_t rxer;
  struct k_thread data;
  struct k_thread *p_tid;
  struct k_timer timer;
  TickType_t timer_duration_ticks;
  TickType_t timer_period_ticks;
} msg_task_t;

/**
 * @brief Get pointer to object containing task (in dispatcher context).
 *
 * Example:
 * dispatch_result_t msg_handler(msg_recv_t *pMsgRxer, msg_t *pMsg)
 * {
 *   task_obj_t *pObj = MSG_TASK_CONTAINER(task_obj_t);
 * }
 */
#define MSG_TASK_CONTAINER(_obj_) CONTAINER_OF(CONTAINER_OF(p_msg_rxer, msg_task_t, rxer), _obj_, msg_task)

#define MSG_BUF_SIZE(t, s) (sizeof(t) + (s))

#define MSG_QUEUE_ENTRY_SIZE (sizeof(msg_t *))
#define MSG_TASK_QUEUE_DEPTH 32
#define MSG_QUEUE_ALIGNMENT 4 /* bytes */

/* Routing a message to task id 0 indicates a problem */
#define MSG_ID_RESERVED 0
#define MSG_ID_APP_START 1

/*********************************************************/
/* Global Function Prototypes                            */
/*********************************************************/
/**
 * @brief This function registers a message task or receiver with the
 * message framework. This is necessary to support routing of messages.
 * A critical section is entered when a task is registered.
 * However, the task should be registered before the task is created so that
 * the periodic timer can be created before the task starts.
 *
 * @param p_rxer A message receiver.
 * @param p_msgtask Pointer to a message task (which contains a rxer).
 */
void msg_register_receiver(msg_recv_t *p_rxer);
void msg_register_task(msg_task_t *p_msg_task);

/**
 * @brief Wraps the queue receive function of the OS and waits for
 * rx_block_ticks for a message to arrive in a task's queue.
 * When a message is received the appropriate message handler
 * function is called by the dispatcher.
 */
void msg_receiver(msg_recv_t *p_msg_rxer);

/**
 * @brief Sends a message to a single task based on task ID.
 *
 * @param rx_id
 * @param p_msg
 * @return BaseType_t An assert isn't generated if rx_id is invalid.
 * @note Caller is responsile for freeing memory, if status isn't success.
 */
BaseType_t msg_send(mid_t rx_id, msg_t *p_msg);

/**
 * @brief Blocks on queue waiting for a message.
 *
 * @note If the caller checks that *pp_data is not NULL
 * (without checking return), then it must initialize *pp_data.
 * Example:
 * msg_t *p_msg = NULL;
 * msg_recv(q, &p_msg, 0);
 * if (p_msg != NULL) { ... }
 *
 * @note Only needed in special cases.
 * This function is called by msg_receiver.
 */
BaseType_t msg_recv(msgq_t *p_queue, void *pp_data, TickType_t block_ticks);

/**
 * @brief Sends a single message to a single task by searching the
 * dispatcher of each message receiver.
 *
 * @note Prevents indirect coupling of tasks by message IDs.
 * @note This should not be used for messages that can go to more than
 * one destination.
 * @note As the number of tasks increases, the amount of time to find the
 * rx_id increases.
 *
 * @param p_msg
 * @return BaseType_t Caller is responsible for freeing memory, if status isn't
 * success.
 */
BaseType_t msg_unicast(msg_t *p_msg);

/**
 * @brief Copies a message and sends it to all tasks that have the message
 * code in their dispatcher.
 *
 * @param msg_size required to copy message.
 *
 * @note Currently an assertion fires if this is called in interrupt context.
 *
 * @retval Caller is responsible for freeing memory, if status isn't success.
 */
BaseType_t msg_broadcast(msg_t *p_msg, size_t msg_size);

/**
 * @brief Bypasses message router and puts a message directly on a queue.
 *
 * @note Most commonly used by a task to send a message to itself.
 */
BaseType_t msg_queue(msgq_t *p_queue, void *pp_data, TickType_t block_ticks);

/**
 * @retval 0 if not empty
 */
BaseType_t msg_queue_is_empty(mid_t rx_id);

/**
 * @brief Free all messages in a receiver's queue.
 *
 * @retval Number of messages that were purged.
 */
size_t msg_flush(mid_t rx_id);

/**
 * @brief Starts a task's periodic timer
 */
void msg_start_timer(msg_task_t *p_msg_task);

/**
 * @brief Stops a task's periodic timer.
 */
void msg_stop_timer(msg_task_t *p_mgs_task);

/**
 *@brief Changes the period of a task's periodic timer.
 */
void msg_change_timer_period(msg_task_t *p_msg_task, TickType_t duration, TickType_t period);

/* Define in application or weak inplementation will be used */
extern void sys_assertion_handler(char *file, int line);
extern bool sys_interrupt_context(void);
extern void sys_reset(void);
extern dispatch_result_t sys_unknown_msg_handler(msg_recv_t *p_msg_rxer, msg_t *p_msg);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_CORE_H__ */
