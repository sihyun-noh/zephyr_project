/**
 * @file sys_shell.c
 * @brief Command shell to show the syscfg usage and to get/set configuration
 * value to the nvs.
 *
 * Copyright (c) 2022-2023 SEED FIC
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/printk.h>

#include <framework/sys_cfg.h>

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static int cmd_syscfg_get(const struct shell *shell, size_t argc, char **argv);
static int cmd_syscfg_set(const struct shell *shell, size_t argc, char **argv);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
SHELL_STATIC_SUBCMD_SET_CREATE(syscfg_cmds,
                               SHELL_CMD_ARG(set, NULL,
                                             "set the config value to setting nvs\n"
                                             "usage:\n"
                                             "$ syscfg set <key> <value>\n"
                                             "example:\n"
                                             "$ syscfg set dev_nonce 1000\n",
                                             cmd_syscfg_set, 3, 0),
                               SHELL_CMD_ARG(get, NULL,
                                             "get the config value from setting nvs\n"
                                             "usage:\n"
                                             "$ syscfg get <key>\n"
                                             "example:\n"
                                             "$ syscfg get dev_nonce\n",
                                             cmd_syscfg_get, 2, 0),
                               SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(syscfg, &syscfg_cmds, "syscfg command", NULL);

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static int cmd_syscfg_get(const struct shell *shell, size_t argc, char **argv) {
  /* Received value is a string so do some test to convert and validate it */
  // key : string, You need to check if the key arg exists

  if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) {
    shell_fprintf(shell, SHELL_ERROR, "Missing key argument\n");
    return -1;
  }

  char *key = argv[1];
  char value[30] = { 0 };

  syscfg_get(key, value, sizeof(value));
  shell_fprintf(shell, SHELL_NORMAL, "Get value = {%s} using {%s}\n", value, key);

  return 0;
}

static int cmd_syscfg_set(const struct shell *shell, size_t argc, char **argv) {
  /* Received value is a string so do some test to convert and validate it */
  // key : string, value : string, You need to check if the key arg exists

  if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) {
    shell_fprintf(shell, SHELL_ERROR, "Missing key argument\n");
    return -1;
  }
  if ((argv[2] == NULL) || (strlen(argv[2]) == 0)) {
    shell_fprintf(shell, SHELL_ERROR, "Missing value argument\n");
    return -1;
  }

  char *key = argv[1];
  char *value = argv[2];

  syscfg_set(key, value);
  shell_fprintf(shell, SHELL_NORMAL, "Set value = {%s} using {%s}\n", value, key);

  return 0;
}
