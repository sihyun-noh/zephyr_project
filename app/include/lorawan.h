/*
 * LoRaWAN sample application
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __LORAWAN_H__
#define __LORAWAN_H__

// #define _WISGATE_OS_
// #define _CHIRPSTACK_OS_
#define _CHIRPSTACK_OS_V4_
// #define _HELTEC_TTS_
// #define SENSOR_NUMBER 1
#define IRRIGRATION_NUMBER 1

/* Customize based on network configuration */
#if defined(_WISGATE_OS_)

#if SENSOR_NUMBER == 1

#define LORAWAN_DEV_EUI                                                        \
  { 0xd4, 0x13, 0xcd, 0x92, 0x0f, 0x88, 0x56, 0x34 } // MSB Format!

#define FPORT 3

#pragma message("Sensor1")

#elif SENSOR_NUMBER == 2

#define LORAWAN_DEV_EUI                                                        \
  { 0xd4, 0x13, 0xcd, 0x92, 0x0f, 0x88, 0x56, 0x35 } // MSB Format!

#define FPORT 2

#pragma message("Sensor2")

#elif SENSOR_NUMBER == 3

#define LORAWAN_DEV_EUI                                                        \
  { 0xd4, 0x13, 0xcd, 0x92, 0x0f, 0x88, 0x56, 0x36 } // MSB Format!

#define FPORT 4

#pragma message("Sensor3")

#else

#error "Unexpected sensor number"

#endif

#define LORAWAN_JOIN_EUI                                                       \
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } // MSB Format!
#define LORAWAN_APP_KEY                                                        \
  {                                                                            \
    0x88, 0x51, 0xaf, 0xe5, 0xc0, 0x06, 0x01, 0x9c, 0xe2, 0x5e, 0x69, 0x9b,    \
        0x0c, 0x7e, 0x1a, 0xb1                                                 \
  }

#elif defined(_CHIRPSTACK_OS_)

#if SENSOR_NUMBER == 1

#define LORAWAN_DEV_EUI                                                        \
  { 0xd4, 0x13, 0xcd, 0x92, 0x0f, 0x88, 0x56, 0x34 } // MSB Format!

#define FPORT 2

#pragma message("Sensor1")

#elif SENSOR_NUMBER == 2

#define LORAWAN_DEV_EUI                                                        \
  { 0xd4, 0x13, 0xcd, 0x92, 0x0f, 0x88, 0x56, 0x35 } // MSB Format!

#define FPORT 3

#pragma message("Sensor2")

#elif SENSOR_NUMBER == 3

#define LORAWAN_DEV_EUI                                                        \
  { 0xd4, 0x13, 0xcd, 0x92, 0x0f, 0x88, 0x56, 0x36 } // MSB Format!

#define FPORT 4

#pragma message("Sensor3")

#elif SENSOR_NUMBER == 5

#define LORAWAN_DEV_EUI                                                        \
  { 0xd4, 0x13, 0xcd, 0x92, 0x0f, 0x88, 0x56, 0x38 } // MSB Format!

#pragma message("Sensor5")

#define FPORT 6

#elif SENSOR_NUMBER == 6

#define LORAWAN_DEV_EUI                                                        \
  { 0xd4, 0x13, 0xcd, 0x92, 0x0f, 0x88, 0x56, 0x39 } // MSB Format!

#pragma message("Sensor6")

#define FPORT 7

#elif SENSOR_NUMBER == 7

#define LORAWAN_DEV_EUI                                                        \
  { 0xd4, 0x13, 0xcd, 0x92, 0x0f, 0x88, 0x56, 0x3A } // MSB Format!

#pragma message("Sensor7")

#define FPORT 8

#elif SENSOR_NUMBER == 8

#define LORAWAN_DEV_EUI                                                        \
  { 0xd4, 0x13, 0xcd, 0x92, 0x0f, 0x88, 0x56, 0x3B } // MSB Format!

#pragma message("Sensor8")

#define FPORT 9

#elif SENSOR_NUMBER == 9

#define LORAWAN_DEV_EUI                                                        \
  { 0xd4, 0x13, 0xcd, 0x92, 0x0f, 0x88, 0x56, 0x3C } // MSB Format!

#pragma message("Sensor9")

#define FPORT 10

#endif

#define LORAWAN_JOIN_EUI                                                       \
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } // MSB Format!
#define LORAWAN_APP_KEY                                                        \
  {                                                                            \
    0xa7, 0x6f, 0xd9, 0xad, 0x23, 0xe4, 0x23, 0xea, 0x7e, 0xd4, 0x6a, 0xf4,    \
        0x6f, 0x1f, 0x5f, 0xd3                                                 \
  }

#elif defined(_CHIRPSTACK_OS_V4_)

#if SENSOR_NUMBER == 4

#define LORAWAN_DEV_EUI                                                        \
  { 0xD4, 0x13, 0xCD, 0x92, 0x0F, 0x81, 0x51, 0x57 } // MSB Format!

#pragma message("Sensor4")

#define FPORT 5

#elif SENSOR_NUMBER == 10

#define LORAWAN_DEV_EUI                                                        \
  { 0xd4, 0x13, 0xcd, 0x92, 0x0f, 0x88, 0x56, 0x3D } // MSB Format!

#pragma message("Sensor10")

#define FPORT 11

#elif SENSOR_NUMBER == 11

#define LORAWAN_DEV_EUI                                                        \
  { 0xd4, 0x13, 0xcd, 0x92, 0x0f, 0x88, 0x56, 0x3E } // MSB Format!

#pragma message("Sensor11")

#define FPORT 12

#elif IRRIGRATION_NUMBER == 1

#define LORAWAN_DEV_EUI                                                        \
  { 0x0e, 0xff, 0x9b, 0x43, 0x95, 0x24, 0x8f, 0xc8 } // MSB Format!
#pragma message("irrigration board #1")

#define FPORT 13

#endif /* SENSOR_NUMBER */

#define LORAWAN_JOIN_EUI                                                       \
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } // MSB Format!
#define LORAWAN_APP_KEY                                                        \
  {                                                                            \
    0xa8, 0x60, 0xd9, 0xac, 0x25, 0xed, 0x23, 0xe0, 0x7d, 0xd3, 0x6b, 0xf1,    \
        0x6e, 0x1d, 0x50, 0x53                                                 \
  }

#elif defined(_HELTEC_TTS_)

#if SENSOR_NUMBER == 1

#define LORAWAN_DEV_EUI                                                        \
  { 0xD4, 0x13, 0xCD, 0x92, 0x0F, 0x81, 0x51, 0x58 } // MSB Format!

#pragma message("Sensor5")

#define FPORT 6

#endif /* SENSOR_NUMBER */

#define LORAWAN_JOIN_EUI                                                       \
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } // MSB Format!

#define LORAWAN_APP_KEY                                                        \
  {                                                                            \
    0xC1, 0x14, 0xE3, 0xE6, 0x2A, 0x8F, 0xBC, 0x1E, 0xD4, 0x02, 0xE3, 0xAA,    \
        0x5C, 0x8B, 0x1E, 0xB6                                                 \
  }

#endif

#endif /* __LORAWAN_H__ */
