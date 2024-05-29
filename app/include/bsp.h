#ifndef __BSP_H__
#define __BSP_H__

#include <stddef.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WATER_FLOW  "water_flow"

void bsp_init(void);
int bsp_pin_get(const struct device *port, uint8_t pin);
int bsp_pin_set(const struct device *port, uint8_t pin, int value);
int bsp_pin_toggle(const struct device *port, uint8_t pin);

int power_control(int value);
int battery_control(int value);
int relay_control(int value);
int relay_toggle(void);
int relay_2_toggle(void);
int led_state1_toggle(void);
int led_state2_toggle(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_H__ */
