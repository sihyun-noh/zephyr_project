#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/modbus/modbus.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(mbc_sample, LOG_LEVEL_INF);

static int iface;

const static struct modbus_iface_param client_param = {
	.mode = MODBUS_MODE_RTU,
	.rx_timeout = 80000,
	.serial = {
		.baud = 4800,
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits_client = UART_CFG_STOP_BITS_1,
	},
};

// #define MODBUS_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(zephyr_modbus_serial)

int init_modbus_client(void) {
  const char iface_name[] = { DT_PROP(DT_INST(0, zephyr_modbus_serial), label) };
  // const char iface_name[] = { DEVICE_DT_NAME(MODBUS_NODE) };

  iface = modbus_iface_get_by_name(iface_name);
  if (iface < 0) {
    printk("Failed to get Modbus interface\n");
    return -1;
  }

  LOG_INF("iface = %d, name = %s", iface, iface_name);

  if (modbus_init_client(iface, client_param) != 0) {
    printk("Fail to initialize interface\n");
    return -1;
  }

  return 0;
}

int read_holding_regs(const uint8_t unit_id, const uint16_t start_addr, uint16_t *const reg_buf,
                      const uint16_t num_regs) {
  int err = modbus_read_holding_regs(iface, unit_id, start_addr, reg_buf, num_regs);
  if (err != 0) {
    LOG_ERR("Fail to read holding registers with %d", err);
    return err;
  }
  return 0;
}

int write_holding_regs(const uint8_t unit_id, const uint16_t start_addr, uint16_t *const reg_buf,
                       const uint16_t num_regs) {
  int err = modbus_write_holding_regs(iface, unit_id, start_addr, reg_buf, num_regs);
  if (err != 0) {
    LOG_ERR("Fail to write holding registers with %d", err);
    return err;
  }
  return 0;
}
