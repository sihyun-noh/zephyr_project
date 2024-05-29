/*
 * LoRaWAN Sensor Node
 *
 * Copyright (c) 2023 SEED FIC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>

#include "bsp.h"
#include "control_task.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void) {

  int ret;
  LOG_INF("Zephyr LoRaWAN Node irrigration\nBoard: %s\n", CONFIG_BOARD);

  bsp_init();

  syscfg_init();

  control_task_init();
  control_task_thread();

  return 0;
}
