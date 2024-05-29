/*
 * RAK3172 (STM32WLE5CC) ADC Example
 *
 * The STM32WL ADC consists of 12 external and 4 internal channels.
 *
 * The RAK3172 has the following ADC inputs broken out:
 * Channel 2  - P0.19 ADC0 'adc_in2_pb3'
 * Channel 3  - P0.20 ADC1 'adc_in3_pb4'
 * Channel 4  - P0.18 ADC2 'adc_in4_pb2'
 * Channel 6  - P0.10 ADC3 'adc_in6_pa10'
 * Channel 11 - P0.15 ADC4 'adc_in11_pa15'
 *
 * Internal Inputs are:
 * Channel 12 - Temperature
 * Channel 13 - Volt Ref
 * Channel 14 - VBAT/3
 * Channel 15 - DAC Output
 *
 * To connect the ADC to the I/O pin, they must be defined in the pinctrl property of the ADC node:
 * pinctrl-0 = <&adc_in2_pb3>, <&adc_in3_pb4>, <&adc_in4_pb2>, <&adc_in6_pa10>, <&adc_in11_pa15>;
 *
 */

#include "adc.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(adc_sample, LOG_LEVEL_INF);

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

#define INPUT_VOLT_RANGE 4.2f    // Volts
#define VALUE_RANGE_12_BIT 4096  // 2^12

static bool is_initialized = false;

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = { DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
                                                                        DT_SPEC_AND_COMMA) };

static int get_adc_channel_count(void) {
  return ARRAY_SIZE(adc_channels);
}

static const struct adc_dt_spec* get_adc_channel(int index) {
  if ((index + 1) > ARRAY_SIZE(adc_channels)) {
    return (struct adc_dt_spec*)NULL;
  }
  return &adc_channels[index];
}

// initialize and configure the adc channel
static const struct adc_dt_spec* init_adc(void) {
  int ret;

  int ch_count = get_adc_channel_count();
  if (ch_count > 1) {
    printk("Only support channel count 1\n");
    ch_count = 1;
  }

  const struct adc_dt_spec* adc_channel = get_adc_channel(ch_count - 1);
  if (adc_channel == NULL) {
    LOG_INF("Fail to intialize the adc channel!!!");
    return NULL;
  }

  if (!device_is_ready(adc_channel->dev)) {
    printk("ADC controller device %s not ready\n", adc_channel->dev->name);
    return NULL;
  }

  if (adc_channel->dev != NULL && !is_initialized) {
    ret = adc_channel_setup_dt(adc_channel);
    if (ret != 0) {
      LOG_INF("Could not setup channel with error = (%d)", ret);
    } else {
      is_initialized = true;  // we don't have any other analog users
    }
  }

  return adc_channel;
}

// ------------------------------------------------
// read one channel of adc
// ------------------------------------------------
static int16_t read_adc_value(void) {
  int32_t val_mv;
  int16_t value = 0;

  struct adc_sequence sequence = { .buffer = &value,  // where to put samples read
                                   .buffer_size = sizeof(value) };

  int ret;
  int16_t sample_value = BAD_ANALOG_READ;

  const struct adc_dt_spec* adc_channel = init_adc();

  if (adc_channel) {
    printk("- %s, channel %d: ", adc_channel->dev->name, adc_channel->channel_id);
    (void)adc_sequence_init_dt(adc_channel, &sequence);
    ret = adc_read(adc_channel->dev, &sequence);
    if (ret == 0) {
      sample_value = value;

      printk("sample_value = %" PRId32, sample_value);
    }

    /*
     * If using differential mode, the 16 bit value
     * in the ADC sample buffer should be a signed 2's
     * complement value.
     */
    if (adc_channel->channel_cfg.differential) {
      val_mv = (int32_t)((int16_t)value);
    } else {
      val_mv = (int32_t)value;
    }
    int err = adc_raw_to_millivolts_dt(adc_channel, &val_mv);
    /* conversion to mV may not be supported, skip if not */
    if (err < 0) {
      printk(" (value in mV not available)\n");
    } else {
      printk(", value = %" PRId32 " mV\n", val_mv);
    }
  }

  return sample_value;
}

// ------------------------------------------------
// high level read adc channel and convert to float voltage
// ------------------------------------------------
float adc_read_vlotage(void) {
  int16_t sv = read_adc_value();

  if (sv == BAD_ANALOG_READ) {
    return sv;
  }

  // Convert the result to voltage
  // Result = [V(p) - V(n)] * GAIN/REFERENCE / (2^(RESOLUTION) / 1000)

  // the 3.3 relates to the voltage divider being used in my circuit
  float fout = (sv * (INPUT_VOLT_RANGE / VALUE_RANGE_12_BIT));
  return (fout * 2);
}
