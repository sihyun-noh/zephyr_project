/**
 * @file bsp.c
 * @brief This is the board support file for defining and configuring the GPIO
 * pins and other peripheral
 *
 * Copyright (c) 2023 SEED FIC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>

LOG_MODULE_REGISTER(bsp, LOG_LEVEL_DBG);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>
#include <zephyr/sys/util.h>

#include "bsp.h"
#include <framework/sys_cfg.h>

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/

/* The devicetree node identifier for the "relay0" alias. */
#define REALY1_NODE DT_ALIAS(relay1)
#define REALY2_NODE DT_ALIAS(relay2)
#define POWER_NODE DT_ALIAS(power)
#define BATTERY_NODE DT_ALIAS(battery)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define WATER_FLOWER_NODE DT_ALIAS(waterflower)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */

static struct gpio_callback water_flower_cb_data;
static const struct gpio_dt_spec water_flower =
    GPIO_DT_SPEC_GET(WATER_FLOWER_NODE, gpios);

static const struct gpio_dt_spec power_ctrl =
    GPIO_DT_SPEC_GET(POWER_NODE, gpios);
static const struct gpio_dt_spec battery_ctrl =
    GPIO_DT_SPEC_GET(BATTERY_NODE, gpios);
static const struct gpio_dt_spec relay = GPIO_DT_SPEC_GET(REALY1_NODE, gpios);
static const struct gpio_dt_spec relay_2 = GPIO_DT_SPEC_GET(REALY2_NODE, gpios);
static const struct device *lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));

static const struct gpio_dt_spec led_state1 =
    GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led_state2 =
    GPIO_DT_SPEC_GET(LED2_NODE, gpios);

static uint32_t water_flower_count = 0;
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void configure_outputs(void);

void gpio_pin_interrupt_set(void);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void bsp_init(void) {

  if (!gpio_is_ready_dt(&relay)) {
    LOG_ERR("%s device not ready", relay.port->name);
  }

  if (!gpio_is_ready_dt(&relay_2)) {
    LOG_ERR("%s device not ready", relay_2.port->name);
  }

  if (!gpio_is_ready_dt(&power_ctrl)) {
    LOG_ERR("%s device not ready", power_ctrl.port->name);
  }

  if (!gpio_is_ready_dt(&battery_ctrl)) {
    LOG_ERR("%s device not ready", battery_ctrl.port->name);
  }

  if (!device_is_ready(lora_dev)) {
    LOG_ERR("%s: device not ready.", lora_dev->name);
  }

  if (!gpio_is_ready_dt(&water_flower)) {
    LOG_ERR("%s: device not ready.", water_flower.port->name);
  }

  if (!gpio_is_ready_dt(&led_state1)) {
    LOG_ERR("%s: device not ready.", led_state1.port->name);
  }

  if (!gpio_is_ready_dt(&led_state2)) {
    LOG_ERR("%s: device not ready.", led_state1.port->name);
  }

  configure_outputs();
  gpio_pin_interrupt_set();
}

int bsp_pin_get(const struct device *port, uint8_t pin) {
  int ret = gpio_pin_get_raw(port, pin);
  if (ret < 0) {
    LOG_ERR("Failed to set pin");
  }
  return ret;
}

int bsp_pin_set(const struct device *port, uint8_t pin, int value) {
  int ret = gpio_pin_set_raw(port, pin, value);
  if (ret < 0) {
    LOG_ERR("Failed to set pin");
  }
  return ret;
}

int bsp_pin_toggle(const struct device *port, uint8_t pin) {
  return gpio_pin_toggle(port, pin);
}

int power_control(int value) {
  return bsp_pin_set(power_ctrl.port, power_ctrl.pin, value);
}

int battery_control(int value) {
  return bsp_pin_set(battery_ctrl.port, battery_ctrl.pin, value);
}

int relay_control(int value) {
  return bsp_pin_set(relay.port, relay.pin, value);
}

int relay_toggle(void) { return bsp_pin_toggle(relay.port, relay.pin); }

int relay_2_toggle(void) { return bsp_pin_toggle(relay_2.port, relay_2.pin); }

int led_state1_toggle(void) {
  return bsp_pin_toggle(led_state1.port, led_state1.pin);
}

int led_state2_toggle(void) {
  return bsp_pin_toggle(led_state2.port, led_state2.pin);
}

static void configure_outputs(void) {
  gpio_pin_configure_dt(&water_flower, GPIO_INPUT);
  gpio_pin_configure_dt(&relay, GPIO_OUTPUT_ACTIVE);
  gpio_pin_configure_dt(&relay_2, GPIO_OUTPUT_ACTIVE);
  gpio_pin_configure_dt(&led_state1, GPIO_OUTPUT_ACTIVE);
  gpio_pin_configure_dt(&led_state2, GPIO_OUTPUT_ACTIVE);
  gpio_pin_configure_dt(&power_ctrl, GPIO_OUTPUT_ACTIVE);
  gpio_pin_configure_dt(&battery_ctrl, GPIO_OUTPUT_ACTIVE);
}

void water_flower_check(const struct device *dev, struct gpio_callback *cb,
                        uint32_t pins) {
  // TODO
  char water_flow[12] = {0};

  water_flower_count++;
  printk("water_flower count : %u\n", water_flower_count);
  snprintf(water_flow, sizeof(water_flow), "%u", water_flower_count);
  syscfg_set(WATER_FLOW, water_flow);
}

void gpio_pin_interrupt_set(void) {
  int ret;
  ret = gpio_pin_interrupt_configure_dt(&water_flower, GPIO_INT_EDGE_RISING);
  if (ret != 0) {
    printk("gpio_pin_interrupt_configure_dt error\n");
  }
  gpio_init_callback(&water_flower_cb_data, water_flower_check,
                     BIT(water_flower.pin));
  gpio_add_callback(water_flower.port, &water_flower_cb_data);
}
