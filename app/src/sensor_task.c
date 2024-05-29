#include <stdio.h>
// #include <stddef.h>

#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/toolchain.h>

#include <framework/buffer_pool.h>
#include <framework/events.h>
#include <framework/sys_cfg.h>
#include <framework/msg_ids.h>
#include <framework/sys_msg_macros.h>
#include <framework/sys_msg_types.h>

#include "adc.h"
#include "bsp.h"
#include "sensor_task.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sensor_task, LOG_LEVEL_DBG);

#define THIS_FILE "Sensor"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#ifndef SENSOR_TASK_PRIORITY
#define SENSOR_TASK_PRIORITY K_PRIO_PREEMPT(1)
#endif

#ifndef SENSOR_TASK_STACK_DEPTH
#define SENSOR_TASK_STACK_DEPTH 8192
#endif

#ifndef SENSOR_TASK_QUEUE_DEPTH
#define SENSOR_TASK_QUEUE_DEPTH 32
#endif

typedef struct {
  msg_task_t msg_task;
} sensor_ctx_t;

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static sensor_ctx_t sensor_ctx;
static struct k_timer power_timer;
static struct k_timer sensor_read_timer;

// static int32_t water_flow;

K_THREAD_STACK_DEFINE(sensor_task_stack, SENSOR_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(sensor_task_queue, MSG_QUEUE_ENTRY_SIZE, SENSOR_TASK_QUEUE_DEPTH,
              MSG_QUEUE_ALIGNMENT);

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void sensor_task_thread(void *, void *, void *);
static dispatch_result_t value_changed_msg_handler(msg_recv_t *p_msg_rxer,
                                                   msg_t *p_msg);
static dispatch_result_t sensor_measure_msg_handler(msg_recv_t *p_msg_rxer,
                                                    msg_t *p_msg);
static dispatch_result_t sensor_check_msg_handler(msg_recv_t *p_msg_rxer,
                                                  msg_t *p_msg);

static void send_sensor_event(event_type_t type, event_data_t data);

static void init_interval_timers(void);
static void start_power_interval(void);
static void start_sensor_interval(void);

static void power_timer_callback(struct k_timer *timer_id);
static void sensor_read_timer_callback(struct k_timer *timer_id);

/******************************************************************************/
/* System Message Dispatcher                                                  */
/******************************************************************************/
static msg_handler_t *sensor_task_msg_dispatcher(msg_code_t msg_code) {
  /* clang-format off */
  switch (msg_code) {
    case SMC_INVALID: return sys_unknown_msg_handler;
    case SMC_SENSOR_CHECK: return sensor_check_msg_handler;
    case SMC_VALUE_CHANGED: return value_changed_msg_handler;
    case SMC_SENSOR_MEASURE: return sensor_measure_msg_handler;
    default: return NULL;
  }
  /* clang-format on */
}

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void sensor_task_init(void) {
  sensor_ctx.msg_task.rxer.id = MSG_ID_SENSOR_TASK;
  sensor_ctx.msg_task.rxer.rx_block_ticks = K_FOREVER;
  sensor_ctx.msg_task.rxer.p_msg_dispatcher = sensor_task_msg_dispatcher;
  sensor_ctx.msg_task.timer_duration_ticks = K_MSEC(1000);
  sensor_ctx.msg_task.timer_period_ticks = K_MSEC(0); /* One shot */
  sensor_ctx.msg_task.rxer.p_queue = &sensor_task_queue;

  /* Register the sensor task for system messaging services */
  msg_register_task(&sensor_ctx.msg_task);

  sensor_ctx.msg_task.p_tid = k_thread_create(
      &sensor_ctx.msg_task.data, sensor_task_stack,
      K_THREAD_STACK_SIZEOF(sensor_task_stack), sensor_task_thread, &sensor_ctx,
      NULL, NULL, SENSOR_TASK_PRIORITY, 0, K_NO_WAIT);
  k_thread_name_set(sensor_ctx.msg_task.p_tid, THIS_FILE);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static float battery_calculate_percentage(float voltage) {
  float percent = 0.0;

  if (voltage >= 4.2) {
    percent = 100;
  } else if (voltage < 2.4) {
    percent = 0;
  } else {
    percent = ((float)(voltage - 2.4) / 1.8) * 100;
  }

  return percent;
}

static void sensor_task_thread(void *p_arg1, void *p_arg2, void *p_arg3) {
  sensor_ctx_t *p_sensor = (sensor_ctx_t *)p_arg1;

  /* Initiaiize interval timers to check the power and sensor data */
  init_interval_timers();

  while (true) {
    msg_receiver(&p_sensor->msg_task.rxer);
  }
}

static dispatch_result_t sensor_check_msg_handler(msg_recv_t *p_msg_rxer,
                                                  msg_t *p_msg) {
  LOG_INF("Checking the sensor status!!!");

  // TODO: If there is no way to recover the i2c error using i2c api, device
  // will be reboot so that the i2c error is recovery.

  // sys_reboot(SYS_REBOOT_WARM);

  // TODO: Need to figure out how to recover the i2c communication when the i2c
  // sensor is attached again.
  /*
   * Get a device structure from a devicetree node with compatible
   * "sensirion,shtcx"
   * const struct device *dev = DEVICE_DT_GET_ANY(sensirion_shtcx);
   * "sensirion,sht3xd"
   * const struct device *dev = DEVICE_DT_GET_ANY(sensirion_sht3xd);
   *
   * #define SHT3XD_NAME "SHT3XD"
   * const struct device *sht3x_dev = device_get_binding(SHT3XD_NAME);
   */
  // #define I2C_DEV_NAME DEVICE_DT_NAME(DT_NODELABEL(i2c2))
  // #define I2C_DEV_NAME "SHT3XD"
  // const struct device *i2c_dev = device_get_binding(I2C_DEV_NAME);
  // i2c_recover_bus(i2c_dev);
  // k_sleep(K_MSEC(500));

  /*
  if (!device_is_ready(sht3xd_dev)) {
    LOG_ERR("Device %s is not ready", sht3xd_dev->name);
  } else {
    LOG_INF("Device %s is ready to use", sht3xd_dev->name);
  }
  */

  return DISPATCH_OK;
}

static dispatch_result_t value_changed_msg_handler(msg_recv_t *p_msg_rxer,
                                                   msg_t *p_msg) {
  return DISPATCH_OK;
}

static dispatch_result_t sensor_measure_msg_handler(msg_recv_t *p_msg_rxer,
                                                    msg_t *p_msg) {
  ARG_UNUSED(p_msg);
  ARG_UNUSED(p_msg_rxer);

  char water_flow[12] = {0};
  uint32_t water_flow_cnt = 0;

  syscfg_get(WATER_FLOW, water_flow, sizeof(water_flow));

  LOG_INF("Send sensor flow data = [%s]", water_flow);

  water_flow_cnt = atoi(water_flow);
  send_sensor_event(SENSOR_EVENT_WATER_FLOW, (event_data_t)water_flow_cnt);

  /* Start sensor interval timer */
  start_sensor_interval();

  return DISPATCH_OK;
}

static void init_interval_timers(void) {
  /* Power interval timer */
  k_timer_init(&power_timer, power_timer_callback, NULL);
  start_power_interval();

  /* Read sensor data interval timer */
  k_timer_init(&sensor_read_timer, sensor_read_timer_callback, NULL);
  start_sensor_interval();
}

static void start_power_interval(void) {
  uint32_t interval_seconds = 60;
  if (interval_seconds != 0) {
    k_timer_start(&power_timer, K_SECONDS(interval_seconds), K_NO_WAIT);
  }
}

static void start_sensor_interval(void) {
  /* Check if the timer is already running */
  uint32_t temp_timer = 0;
  temp_timer = k_timer_remaining_get(&sensor_read_timer);
  if (temp_timer == 0) {
    uint32_t interval_seconds = 60;
    if (interval_seconds != 0) {
      k_timer_start(&sensor_read_timer, K_SECONDS(interval_seconds), K_NO_WAIT);
    }
  }
}

static void send_sensor_event(event_type_t type, event_data_t data) {
  event_msg_t *p_event_msg =
      (event_msg_t *)buffer_pool_take(sizeof(event_msg_t));

  if (p_event_msg != NULL) {
    p_event_msg->header.msg_code = SMC_EVENT_TRIGGER;
    p_event_msg->header.tx_id = MSG_ID_SENSOR_TASK;
    p_event_msg->header.rx_id = MSG_ID_EVENT_TASK;
    p_event_msg->event_type = type;
    p_event_msg->event_data = data;
    SYSMSG_SEND(p_event_msg);
  }
}

/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
static void power_timer_callback(struct k_timer *timer_id) {
  UNUSED_PARAMETER(timer_id);
  SYSMSG_CREATE_AND_SEND(MSG_ID_SENSOR_TASK, MSG_ID_SENSOR_TASK,
                         SMC_READ_POWER);
}

static void sensor_read_timer_callback(struct k_timer *timer_id) {
  UNUSED_PARAMETER(timer_id);
  SYSMSG_CREATE_AND_SEND(MSG_ID_SENSOR_TASK, MSG_ID_SENSOR_TASK,
                         SMC_SENSOR_MEASURE);
}
