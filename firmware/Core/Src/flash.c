/*
 * flash.c
 *
 *  Created on: Nov 28, 2023
 *      Author: andrew
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "flash.h"

//#define FSH_DEBUG

/** */
void FSH_ResetEnable(void)
{
    uint8_t txbuf[] = {0x66};

    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi2, txbuf, txbuf, sizeof(txbuf), 10000L);
    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_SET);
}

/** */
void FSH_Reset(void)
{
    uint8_t txbuf[] = {0x99};

    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi2, txbuf, txbuf, sizeof(txbuf), 10000L);
    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_SET);
}

/** */
void FSH_GlobalUnlock(void)
{
    uint8_t txbuf[] = {0x98};

    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi2, txbuf, txbuf, sizeof(txbuf), 10000L);
    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_SET);
}

/** */
uint32_t FSH_ReadJedecID(void)
{
    uint8_t txbuf[] = {0x9f};
    uint8_t rxbuf[3];

    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_RESET);
     __disable_irq();
    HAL_SPI_Transmit(&hspi2, txbuf, sizeof(txbuf), 10000L);
    HAL_SPI_Receive(&hspi2, rxbuf, sizeof(rxbuf), 10000L);
     __enable_irq();
    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_SET);

    return rxbuf[0] << 16 | rxbuf[1] << 8 | rxbuf[2];
}

/** */
void FSH_Read(uint32_t addr, uint8_t *data, uint16_t len)
{
    uint8_t txbuf[] = {0x03, addr >> 16, addr >> 8, addr};
    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_RESET);
     __disable_irq();
    HAL_SPI_Transmit(&hspi2, txbuf, sizeof(txbuf), 10000L);
    HAL_SPI_Receive(&hspi2, data, len, 10000L);
     __enable_irq();
    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_SET);

#ifdef FSH_DEBUG
    fprintf(stderr,"FSH_Read addr=%lx len=%d\n", addr, len);
#endif
}

/** */
void FSH_EraseSector(uint32_t addr)
{
    uint8_t txbuf[] = {0x20, addr >> 16, addr >> 8, addr};

    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi2, txbuf, txbuf, sizeof(txbuf), 10000L);
    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_SET);
}

/** */
void FSH_EraseChip(void)
{
    uint8_t txbuf[] = {0xc7};

    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi2, txbuf, txbuf, sizeof(txbuf), 10000L);
    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_SET);
}

/** */
void FSH_ProgramPage(uint32_t addr, uint8_t *data, uint16_t len)
{
    uint8_t txbuf[] = {0x02, addr >> 16, addr >> 8, addr};

    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_RESET);
     __disable_irq();
    HAL_SPI_Transmit(&hspi2, txbuf, sizeof(txbuf), 10000L);
    HAL_SPI_Transmit(&hspi2, data, len, 10000L);
     __enable_irq();
    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_SET);

#ifdef FSH_DEBUG
    fprintf(stderr,"FSH_ProgramPage addr=%lx len=%d\n", addr, len);
#endif
}

/** */
void FSH_WriteEnable(void)
{
    uint8_t txbuf[] = {0x06};

    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi2, txbuf, txbuf, sizeof(txbuf), 10000L);
    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_SET);
}

/** */
void FSH_WriteDisable(void)
{
    uint8_t txbuf[] = {0x04};

    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi2, txbuf, txbuf, sizeof(txbuf), 10000L);
    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_SET);
}

/** */
void FSH_ClearBlockProtectionRegister(void)
{
    uint8_t txbuf[] = {0x42, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi2, txbuf, txbuf, sizeof(txbuf), 10000L);
    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_SET);
}

/** */
uint8_t FSH_ReadStatus(void)
{
    uint8_t txbuf[2] = {0x05, 0};

    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi2, txbuf, txbuf, sizeof(txbuf), 10000L);
    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_SET);

    return txbuf[1];
}

/** */
uint8_t FSH_ReadConfig(void)
{
    uint8_t txbuf[2] = {0x35, 0};

    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi2, txbuf, txbuf, sizeof(txbuf), 10000L);
    HAL_GPIO_WritePin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin, GPIO_PIN_SET);

    return txbuf[1];
}

/** */
void FSH_WaitWhileBusy(void)
{
#ifdef FSH_DEBUG
	printf("waiting for flash\n");

	uint32_t t0 = HAL_GetTick();

    while (true)
    {
        uint8_t status = FSH_ReadStatus();
        if(!(status & FSH_STATUS_BUSY))
        {
            break;
        }

        HAL_Delay(1);
    }

    printf("op took %ldms\n", HAL_GetTick() - t0);
#else
    while (FSH_ReadStatus()& FSH_STATUS_BUSY)
    {
        HAL_Delay(1);
    }
#endif  
}
