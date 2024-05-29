#include <framework/sys_cfg.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sys_cfg, LOG_LEVEL_INF);

#define SEED 0x1234

static struct nvs_fs fs;

// The code below is used to generate the hash value using the string
uint16_t MurmurOATT_16(const char *str, uint16_t h) {
  for (; *str; ++str) {
    h ^= *str;
    h *= 0x5bd1;
    h ^= h >> 7;
  }
  return h;
}

uint16_t get_key_id(const char *key, int seed) {
  return MurmurOATT_16(key, seed);
}

int syscfg_init(void) {
  struct flash_pages_info info;
  int ret = 0;

  fs.flash_device = NVS_PARTITION_DEVICE;

  if (!device_is_ready(fs.flash_device)) {
    LOG_ERR("Flash device %s is not ready", fs.flash_device->name);
    return -1;
  }

  fs.offset = NVS_PARTITION_OFFSET;
  ret = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
  if (ret) {
    LOG_ERR("Unable to get page info");
    return -1;
  }
  fs.sector_size = info.size;
  fs.sector_count = 3U;

  ret = nvs_mount(&fs);
  if (ret) {
    LOG_ERR("Flash Init failed");
    return -1;
  }

  LOG_INF("nvs mount : sector_size = %d, sector_count = %d", fs.sector_size, fs.sector_count);

  return 0;
}

void syscfg_deinit(void) {}

int syscfg_get(const char *key, char *value, size_t value_len) {
  int ret = 0;

  uint16_t key_id = get_key_id(key, SEED);

  int read_bytes = nvs_read(&fs, key_id, (void *)value, value_len);
  if (read_bytes <= 0) {
    ret = -1;
  }
  return ret;
}

int syscfg_set(const char *key, const char *value) {
  int ret = 0;

  uint16_t key_id = get_key_id(key, SEED);

  int write_bytes = nvs_write(&fs, key_id, (void *)value, strlen(value));
  if (write_bytes < 0) {
    ret = -1;
  }
  return ret;
}

int syscfg_unset(const char *key) {
  uint16_t key_id = get_key_id(key, SEED);

  int rc = nvs_delete(&fs, key_id);
  return rc;
}

int syscfg_get_blob(const char *key, void *value, size_t value_len) {
  int ret = 0;

  uint16_t key_id = get_key_id(key, SEED);

  int read_bytes = nvs_read(&fs, key_id, value, value_len);
  if (read_bytes <= 0) {
    ret = -1;
  }
  return ret;
}

int syscfg_set_blob(const char *key, const void *value, size_t value_len) {
  int ret = 0;

  uint16_t key_id = get_key_id(key, SEED);

  int write_bytes = nvs_write(&fs, key_id, value, value_len);
  if (write_bytes < 0) {
    ret = -1;
  }
  return ret;
}
