#ifndef __ANALOG_IN_H__
#define __ANALOG_IN_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

float adc_read_vlotage(void);
#define BAD_ANALOG_READ -123

#ifdef __cplusplus
}
#endif

#endif /* __ANALOG_IN_H__ */
