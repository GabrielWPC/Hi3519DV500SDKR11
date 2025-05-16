/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __BACKUP_H_
#define __BACKUP_H_

void clean_boot_img_addr_size(void);
void add_boot_img_size(u32 size);
u32 get_verify_backup_img_reg(void);
void set_verify_backup_img_reg(unsigned int type);

#endif /* __BACKUP_H_ */
