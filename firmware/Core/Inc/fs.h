/*
 * fs.h
 *
 *  Created on: Nov 30, 2023
 *      Author: andrew
 */

#ifndef CORE_INC_FS_H_
#define CORE_INC_FS_H_

#include "lfs.h"
extern struct lfs_config lfs_cfg;
extern struct lfs lfs;

void fs_init(void);
void append_to_current_file(char *buffer, int length);
#endif /* CORE_INC_FS_H_ */
