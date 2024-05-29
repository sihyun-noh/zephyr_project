#ifndef __MODBUS_RTU_H__
#define __MODBUS_RTU_H__

#ifdef __cplusplus
extern "C" {
#endif

int init_modbus_client(void);

int read_holding_regs(const uint8_t unit_id, const uint16_t start_addr, uint16_t *const reg_buf,
                         const uint16_t num_regs);
int write_holding_regs(const uint8_t unit_id, const uint16_t start_addr, uint16_t *const reg_buf,
                          const uint16_t num_regs);

#ifdef __cplusplus
}
#endif

#endif /* __MODBUS_RTU_H__ */
