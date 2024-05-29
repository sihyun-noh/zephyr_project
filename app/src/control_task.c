#include <math.h>
#include <stdio.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/lorawan/lorawan.h>
#include <zephyr/sys/reboot.h>

// #include <zephyr/drivers/flash.h>
// #include <zephyr/fs/nvs.h>
// #include <zephyr/storage/flash_map.h>

#include <framework/msg_ids.h>
#include <framework/sys_msg_macros.h>
#include <framework/sys_msg_types.h>

#include "bsp.h"
#include "control_task.h"
#include "event_task.h"
#include "lorawan.h"
#include "sensor_task.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(control_task, LOG_LEVEL_DBG);

#define THIS_FILE "Control_Task"

// #define CONFIG_HEARTBEAT_SECONDS 300
#define CONFIG_HEARTBEAT_SECONDS 120

#if !CONTROL_TASK_USES_MAIN_THREAD
#ifndef CONTROL_TASK_PRIORITY
#define CONTROL_TASK_PRIORITY K_PRIO_PREEMPT(1)
#endif

#ifndef CONTROL_TASK_STACK_DEPTH
#define CONTROL_TASK_STACK_DEPTH 4096
#endif
#endif /* CONTROL_TASK_USES_MAIN_THREAD */

#ifndef CONTROL_TASK_QUEUE_DEPTH
#define CONTROL_TASK_QUEUE_DEPTH 32
#endif

// #define NVS_PARTITION storage_partition
// #define NVS_PARTITION_DEVICE FIXED_PARTITION_DEVICE(NVS_PARTITION)
// #define NVS_PARTITION_OFFSET FIXED_PARTITION_OFFSET(NVS_PARTITION)

// #define NVS_DEVNONCE_ID 1

typedef struct {
  msg_task_t msg_task;
  uint32_t boradcast_cnt;
  bool b_factory_reset;
  bool b_task_started;
} control_ctx_t;

/**********************************************************/
/* Local Data Definitions                                 */
/**********************************************************/
static control_ctx_t ctrl_ctx;

// static struct nvs_fs fs;
static uint16_t dev_nonce;

static bool b_send_msg_lorawan;

uint8_t tx_data[51] = {0};

#if !CONTROL_TASK_USES_MAIN_THREAD
K_THREAD_STACK_DEFINE(ctrl_task_stack, CONTROL_TASK_STACK_DEPTH);
#endif

K_MSGQ_DEFINE(ctrl_task_queue, MSG_QUEUE_ENTRY_SIZE, CONTROL_TASK_QUEUE_DEPTH,
              MSG_QUEUE_ALIGNMENT);

/**********************************************************/
/* Local Function Prototypes                              */
/**********************************************************/
static void ctrl_task_thread(void *, void *, void *);

static uint16_t get_dev_nonce(void);
static uint16_t inc_dev_nonce(uint16_t nonce);

// LoRaWAN callback functions
static void dl_callback(uint8_t port, bool data_pending, int16_t rssi,
                        int8_t snr, uint8_t len, const uint8_t *data);
static void lorwan_datarate_changed(enum lorawan_datarate dr);

static dispatch_result_t sw_reset_msg_handler(msg_recv_t *p_msg_rxer,
                                              msg_t *p_msg);
static dispatch_result_t heart_beat_msg_handler(msg_recv_t *p_msg_rxer,
                                                msg_t *p_msg);
static dispatch_result_t factory_reset_msg_handler(msg_recv_t *p_msg_rxer,
                                                   msg_t *p_msg);
static dispatch_result_t join_lorawan_msg_handler(msg_recv_t *p_msg_rxer,
                                                  msg_t *p_msg);
static dispatch_result_t send_data_lorawan_msg_handler(msg_recv_t *p_msg_rxer,
                                                       msg_t *p_msg);
static dispatch_result_t sensor_event_msg_handler(msg_recv_t *p_msg_rxer,
                                                  msg_t *p_msg);

static void reboot_handler(void);

/**********************************************************/
/* System Message Dispatcher                              */
/**********************************************************/
static msg_handler_t *control_task_msg_dispatcher(msg_code_t msg_code) {
  /* clang-format off */
  switch (msg_code) {
    case SMC_INVALID:           return sys_unknown_msg_handler;
    case SMC_PERIODIC:          return heart_beat_msg_handler;
    case SMC_SW_RESET:          return sw_reset_msg_handler;
    case SMC_FACTORY_RESET:     return factory_reset_msg_handler;
    case SMC_JOIN_LORAWAN:      return join_lorawan_msg_handler;
    case SMC_SENSOR_EVENT:      return sensor_event_msg_handler;
    case SMC_SEND_DATA_LORAWAN: return send_data_lorawan_msg_handler;
    default:                    return NULL;
  }
  /* clang-format on */
}

/**********************************************************/
/* Global Function Definitions                            */
/**********************************************************/
void control_task_init(void) {
  ctrl_ctx.msg_task.rxer.id = MSG_ID_CONTROL_TASK;
  ctrl_ctx.msg_task.rxer.rx_block_ticks = K_FOREVER;
  ctrl_ctx.msg_task.rxer.p_msg_dispatcher = control_task_msg_dispatcher;
  ctrl_ctx.msg_task.timer_duration_ticks = K_SECONDS(CONFIG_HEARTBEAT_SECONDS);
  ctrl_ctx.msg_task.timer_period_ticks = K_MSEC(0);
  ctrl_ctx.msg_task.rxer.p_queue = &ctrl_task_queue;

  msg_register_task(&ctrl_ctx.msg_task);

  ctrl_ctx.b_factory_reset = false;

#if CONTROL_TASK_USES_MAIN_THREAD
  ctrl_ctx.msg_task.p_tid = k_current_get();
#else
  ctrl_ctx.msg_task.p_tid = k_thread_create(
      &ctrl_ctx.msg_task.data, ctrl_task_stack,
      K_THREAD_STACK_SIZEOF(ctrl_task_stack), ctrl_task_thread, &ctrl_ctx, NULL,
      NULL, CONTROL_TASK_PRIORITY, 0, K_NO_WAIT);
#endif
  k_thread_name_set(ctrl_ctx.msg_task.p_tid, THIS_FILE);
}

void control_task_thread(void) {
#if CONTROL_TASK_USES_MAIN_THREAD
  ctrl_task_thread(&ctrl_ctx.msg_task.p_tid, NULL, NULL);
#endif
}

/**********************************************************/
/* Local Function Definitions                             */
/**********************************************************/
#if 0
static uint16_t get_dev_nonce(void) {
  struct flash_pages_info info;
  ssize_t bytes_written;
  uint16_t nonce = 0;
  int ret = 0;

  fs.flash_device = NVS_PARTITION_DEVICE;
  if (!device_is_ready(fs.flash_device)) {
    printk("Flash device %s is not ready\n", fs.flash_device->name);
    return 0;
  }
  fs.offset = NVS_PARTITION_OFFSET;
  ret = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
  if (ret) {
    printk("Unable to get page info\n");
    return 0;
  }
  fs.sector_size = info.size;
  fs.sector_count = 3U;

  ret = nvs_mount(&fs);
  if (ret) {
    printk("Flash Init failed\n");
    return 0;
  }

  ret = nvs_read(&fs, NVS_DEVNONCE_ID, &nonce, sizeof(nonce));
  if (ret > 0) { /* item was found, show it */
    printk("NVS: ID %d, DevNonce: %d\n", NVS_DEVNONCE_ID, nonce);
  } else { /* item was not found, add it */
    printk("NVS: No DevNonce found, resetting to %d\n", nonce);
    bytes_written = nvs_write(&fs, NVS_DEVNONCE_ID, &nonce, sizeof(nonce));
    if (bytes_written < 0) {
      printf("NVS: Failed to write id %d (%d)\n", NVS_DEVNONCE_ID,
             bytes_written);
    } else {
      printf("NVS: Wrote %d bytes to id %d\n", bytes_written, NVS_DEVNONCE_ID);
    }
  }

  return nonce;
}
#endif

static uint16_t get_dev_nonce(void) {
  char s_dev_nonce[10] = {0};
  uint16_t dev_nonce = 0;

  int ret = syscfg_get("dev_nonce", s_dev_nonce, sizeof(s_dev_nonce));
  if (ret == -1) {
    return dev_nonce;
  }

  dev_nonce = atoi(s_dev_nonce);
  return dev_nonce;
}

static uint16_t inc_dev_nonce(uint16_t nonce) {
  char s_dev_nonce[10] = {0};

  // Increment DevNonce as per LoRaWAN 1.0.4 Spec.
  nonce++;

  snprintf(s_dev_nonce, sizeof(s_dev_nonce), "%hu", nonce);
  syscfg_set("dev_nonce", s_dev_nonce);

  return nonce;
}

static bool lorawan_init(void) {
  printk("Starting LoRaWAN stack.\n");

  int ret = lorawan_start();
  if (ret < 0) {
    printk("lorawan_start failed: %d\n\n", ret);
    return false;
  }

  // Enable callbacks
  struct lorawan_downlink_cb downlink_cb = {.port = LW_RECV_PORT_ANY,
                                            .cb = dl_callback};

  lorawan_register_downlink_callback(&downlink_cb);
  lorawan_register_dr_changed_callback(lorwan_datarate_changed);

  dev_nonce = get_dev_nonce();

  // trigger join lorawan network message
  SYSMSG_CREATE_AND_SEND(MSG_ID_CONTROL_TASK, MSG_ID_CONTROL_TASK,
                         SMC_JOIN_LORAWAN);

  return true;
}

static void ctrl_task_thread(void *p_arg1, void *p_arg2, void *p_arg3) {
  control_ctx_t *p_ctrl = (control_ctx_t *)p_arg1;

  reboot_handler();

  // Initialize of other tasks and configuration.
  sensor_task_init();
  event_task_init();

  // Initialize LoRaWAN
  lorawan_init();

  msg_start_timer(&p_ctrl->msg_task);

  ctrl_ctx.b_task_started = true;

  LOG_INF("Control Task is started!!!");

  while (true) {
    msg_receiver(&p_ctrl->msg_task.rxer);
  }
}

static void dl_callback(uint8_t port, bool data_pending, int16_t rssi,
                        int8_t snr, uint8_t len, const uint8_t *data) {
  char rx_data[10] = {0};

  LOG_INF("Port %d, Pending %d, RSSI %ddB, SNR %ddBm", port, data_pending, rssi,
          snr);
  if (data) {
    LOG_HEXDUMP_INF(data, len, "Payload: ");
  }
  // Control the relay node when receiving the control command from the LoRa
  // network server.
  memcpy(rx_data, data, len);
  if (strncmp(rx_data, "on", len) == 0) {
    relay_control(1);
  } else if (strncmp(rx_data, "off", len) == 0) {
    relay_control(0);
  }
}

static void lorwan_datarate_changed(enum lorawan_datarate dr) {
  uint8_t unused, max_size;

  lorawan_get_payload_sizes(&unused, &max_size);
  LOG_INF("New Datarate: DR_%d, Max Payload %d", dr, max_size);
}

static void reboot_handler(void) {}

static dispatch_result_t sw_reset_msg_handler(msg_recv_t *p_msg_rxer,
                                              msg_t *p_msg) {
  LOG_WRN("Software Reset!!!");
  return DISPATCH_OK;
}

static dispatch_result_t heart_beat_msg_handler(msg_recv_t *p_msg_rxer,
                                                msg_t *p_msg) {
  ARG_UNUSED(p_msg);
  control_ctx_t *p_ctrl = MSG_TASK_CONTAINER(control_ctx_t);

  /* do something (send senor data to LoRa Gateway periodically) */
  SYSMSG_CREATE_AND_SEND(MSG_ID_CONTROL_TASK, MSG_ID_SENSOR_TASK,
                          SMC_SENSOR_MEASURE);

  LOG_INF("Received HeartBeat message!!!");

  // Restart timer to run periodic callback
  msg_start_timer(&p_ctrl->msg_task);

  return DISPATCH_OK;
}

static dispatch_result_t factory_reset_msg_handler(msg_recv_t *p_msg_rxer,
                                                   msg_t *p_msg) {
  LOG_WRN("Factory Rest!!!");
  ctrl_ctx.b_factory_reset = true;

  // Need to do factory reset
  // Need reset to init all the configuration values.

  SYSMSG_CREATE_AND_SEND(MSG_ID_CONTROL_TASK, MSG_ID_CONTROL_TASK,
                         SMC_SW_RESET);

  return DISPATCH_OK;
}

static dispatch_result_t join_lorawan_msg_handler(msg_recv_t *p_msg_rxer,
                                                  msg_t *p_msg) {
  LOG_INF("Joining LoRaWAN network");

  int i = 0, ret = 0;
  struct lorawan_join_config join_cfg;

  uint8_t dev_eui[] = LORAWAN_DEV_EUI;
  uint8_t join_eui[] = LORAWAN_JOIN_EUI;
  uint8_t app_key[] = LORAWAN_APP_KEY;

  join_cfg.mode = LORAWAN_ACT_OTAA;
  join_cfg.dev_eui = dev_eui;
  join_cfg.otaa.join_eui = join_eui;
  join_cfg.otaa.app_key = app_key;
  join_cfg.otaa.nwk_key = app_key;
  join_cfg.otaa.dev_nonce = dev_nonce;

  do {
    printk("Joining network using OTAA, dev nonce %d, attempt %d: ", dev_nonce,
           i++);
    ret = lorawan_join(&join_cfg);
    if (ret < 0) {
      if ((ret = -ETIMEDOUT)) {
        printf("Timed-out waiting for response.\n");
      } else {
        printk("Join failed (%d)\n", ret);
      }
    } else {
      printk("Join successful.\n");
    }

    dev_nonce = inc_dev_nonce(dev_nonce);
    join_cfg.otaa.dev_nonce = dev_nonce;

    if (ret < 0) {
      // If failed, wait before re-trying.
      k_sleep(K_MSEC(5000));
    }
  } while (ret != 0);

  ret = lorawan_set_class(LORAWAN_CLASS_C);
  if (ret != 0) {
    printk("lorawan_set_class with class C failed: %d\n\n", ret);
  }

  SYSMSG_CREATE_AND_SEND(MSG_ID_CONTROL_TASK, MSG_ID_SENSOR_TASK,
                          SMC_SENSOR_MEASURE);

  return DISPATCH_OK;
}

static dispatch_result_t sensor_event_msg_handler(msg_recv_t *p_msg_rxer,
                                                  msg_t *p_msg) {
  LOG_INF("Sensor Event Message Handler");

  event_msg_t *p_event_msg = (event_msg_t *)p_msg;

  if (p_event_msg->event_type == SENSOR_EVENT_WATER_FLOW) {
    uint32_t flow = (int)p_event_msg->event_data.u32;
    snprintf(tx_data, sizeof(tx_data), "flow: %d", flow);
    b_send_msg_lorawan = !b_send_msg_lorawan;
  }

  if (b_send_msg_lorawan) {
    SYSMSG_CREATE_AND_SEND(MSG_ID_CONTROL_TASK, MSG_ID_CONTROL_TASK, SMC_SEND_DATA_LORAWAN);
  }

  return DISPATCH_OK;
}

static dispatch_result_t send_data_lorawan_msg_handler(msg_recv_t *p_msg_rxer,
                                                       msg_t *p_msg) {
  int retries = 0;
  LOG_INF("Send data to LoRaWAN server");

  relay_toggle();

  LOG_HEXDUMP_INF(tx_data, sizeof(tx_data), "packet: ");

  while (true) {
    int ret = lorawan_send(FPORT, (uint8_t *)&tx_data, strlen(tx_data),
                           LORAWAN_MSG_UNCONFIRMED);
    if (ret == -EAGAIN) {
      LOG_ERR("lorawan_send failed: %d. Continuing...", ret);
      k_sleep(K_MSEC(500));
      continue;
    } else if (ret < 0) {
      LOG_ERR("lorawan_send failed: %d", ret);
      if (retries > 5) {
        break;
      }
      k_sleep(K_MSEC(500));
      retries++;
      continue;
    } else {
      LOG_INF("Data sent!");
      break;
    }
  }

  memset(tx_data, 0x00, sizeof(tx_data));

  return DISPATCH_OK;
}
