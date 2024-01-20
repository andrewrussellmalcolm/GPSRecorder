/*
 * fs.c
 *
 *  Created on: Nov 28, 2023
 *      Author: andrew
 */

#include <stdint.h>
#include "flash.h"
#include "lfs.h"
#include "main.h"

/** */
static int block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
	FSH_Read((block * c->block_size + off), (uint8_t*)buffer, size);
	return 0;
}

/** */
static int block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
	FSH_WriteEnable();
	FSH_ProgramPage((block * c->block_size + off), (uint8_t *)buffer, size);
	FSH_WaitWhileBusy();
	FSH_WriteDisable();
	return 0;
}

/** */
static int block_device_erase(const struct lfs_config *c, lfs_block_t block)
{
	FSH_WriteEnable();
	FSH_EraseSector(block * c->block_size);
	FSH_WaitWhileBusy();
	FSH_WriteDisable();
	return 0;
}

/** */
int block_device_sync(const struct lfs_config *c)
{
	return 0;
}

static uint8_t lfs_prog_buf[FSH_PAGE_SIZE];
static uint8_t lfs_read_buf[FSH_PAGE_SIZE];
static uint8_t lfs_lookahead_buf[FSH_PAGE_SIZE];

struct lfs_config lfs_cfg =
{
	.read = block_device_read,
	.prog  = block_device_prog,
	.erase = block_device_erase,
	.sync  = block_device_sync,

	// block device configuration
	.read_size = FSH_PAGE_SIZE,
	.prog_size = FSH_PAGE_SIZE,
	.block_size = FSH_SECTOR_SIZE,
	.cache_size = FSH_PAGE_SIZE,
	.block_count = FSH_MEMORY_SIZE/FSH_SECTOR_SIZE,
	.lookahead_size = FSH_PAGE_SIZE,
	.block_cycles = 500,
	.read_buffer = lfs_read_buf,
	.prog_buffer = lfs_prog_buf,
	.lookahead_buffer = lfs_lookahead_buf,
};

lfs_t lfs;

/** */
void fs_init(void)
{
	FSH_GlobalUnlock();
	FSH_WriteEnable();
	FSH_ClearBlockProtectionRegister();
	FSH_WaitWhileBusy();
	FSH_WriteDisable();

	uint32_t jedec_id = FSH_ReadJedecID();
	if(jedec_id != FSH_JEDEC_ID)
	{
		printf("Flash memory found with Jedec ID %8.8lx, expected %8.8lx\n", jedec_id, FSH_JEDEC_ID);
	}

	int err = lfs_mount(&lfs, &lfs_cfg);

	if (err < 0)
	{
		printf("could not mount filesystem\n");
		return;
	}
}

/** */
static void append_to_file(char *filename, char *buffer, int length)
{
	lfs_file_t file;

	lfs_file_open(&lfs, &file, filename, LFS_O_WRONLY | LFS_O_APPEND | LFS_O_CREAT);
	lfs_file_write(&lfs, &file, buffer, length);
	lfs_file_close(&lfs, &file);
}

/** */
void append_to_current_file(char *buffer, int length)
{
	char filename[16];
	RTC_DateTypeDef sDate;
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	snprintf(filename, sizeof(filename), "%2.2d_%2.2d_%2.2d", sDate.Date, sDate.Month, sDate.Year);

	append_to_file(filename, buffer, length);
}
