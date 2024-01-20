/**

 */

#ifndef __FLASH_H
#define __FLASH_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif
#define FSH_PAGE_SIZE 0x100
#define FSH_SECTOR_SIZE 0x1000
#define FSH_MEMORY_SIZE 0x400000
#define FSH_STATUS_BUSY 0x01
#define FSH_STATUS_WRITE_ENABLED 0x02
#define FSH_STATUS_ERASE_SUSPENDED 0x04
#define FSH_STATUS_PROGRAM_SUSPENDED 0x08
#define FSH_STATUS_WRITE_LOCKDOWN_ENABLED 0x10
#define FSH_STATUS_SECURITY_ID_LOCKED 0x20
#define FSH_STATUS_RESERVED 0x40
#define FSH_STATUS_BUSY_REPEATED 0x80

#define FSH_CONFIG_RESERVED0 0x01
#define FSH_CONFIG_IOC 0x02
#define FSH_CONFIG_RESERVED1 0x04
#define FSH_CONFIG_BLOCK_PROTECTION_VOLATILITY 0x08
#define FSH_CONFIG_RESERVED2 0x10
#define FSH_CONFIG_RESERVED3 0x20
#define FSH_CONFIG_RESERVED4 0x40
#define FSH_CONFIG_WRITE_PROT_PIN_ENABLED 0x80

#define FSH_JEDEC_ID 0x00bf2642L

    uint32_t FSH_ReadJedecID(void);
    void FSH_Read(uint32_t addr, uint8_t *buf, uint16_t len);
    void FSH_ProgramPage(uint32_t addr, uint8_t *data, uint16_t len);
    void FSH_EraseSector(uint32_t addr);
    void FSH_EraseChip(void);
    void FSH_WriteEnable(void);
    void FSH_WriteDisable(void);
    void FSH_GlobalUnlock(void);
    void FSH_ResetEnable(void);
    void FSH_Reset(void);
    void FSH_ClearBlockProtectionRegister(void);
    uint8_t FSH_ReadStatus(void);
    uint8_t FSH_ReadConfig(void);
    void FSH_WaitWhileBusy(void);

#ifdef __cplusplus
}
#endif

#endif
