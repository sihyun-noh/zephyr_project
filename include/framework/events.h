#ifndef __EVENTS_H__
#define __EVENTS_H__

// #include <zephyr/kernel.h>
// #include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum event_type {
  /* Sensor events */
  SENSOR_EVENT_RESERVED = 0,
  SENSOR_EVENT_TEMPERATURE = 1,
  SENSOR_EVENT_HUMIDITY = 2,
  SENSOR_EVENT_BATTERY_LEVEL = 3,
  SENSOR_EVENT_BATTERY_GOOD = 4,
  SENSOR_EVENT_BATTERY_BAD = 5,
  SENSOR_EVENT_SOIL_EC = 6,
  SENSOR_EVENT_SOIL_TEMPERATURE = 7,
  SENSOR_EVENT_SOIL_HUMIDITY = 8,
  SENSOR_EVENT_SOIL_PH = 9,
  SENSOR_EVENT_WATER_FLOW = 10,

  NUMBER_OF_EVENTS
} event_type_t;

typedef union __attribute__((packed)) event_data {
  uint32_t u32;
  int32_t s32;
  float f;
  struct {
    uint16_t u16;
    uint16_t reserved;
  };
} event_data_t;

typedef struct __attribute__((packed)) sensor_event {
  uint32_t timestamp;
  event_data_t data;
  event_type_t type;
  uint16_t index;
} sensor_event_t;

#ifdef __cplusplus
}
#endif

#endif /* __EVENTS_H__ */
