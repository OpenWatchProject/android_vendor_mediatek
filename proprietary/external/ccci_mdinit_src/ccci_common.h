/*
* Copyright (C) 2011-2017 MediaTek Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef __CCCI_SRV_COMMON_H__
#define __CCCI_SRV_COMMON_H__

#include "ccci_log.h"
#include "mdinit_relate.h"

extern int curr_md_id;
extern int  system_ch_handle;
extern const char md_img_folder[32];
extern const char md_img_cip_folder[32];

int is_factory_mode(void);
int is_meta_mode(void);

int is_current_md_en(int md_id);
int get_ccci_drv_ver(void);
int kernel_setting_prepare(void);
void depends_so_prepare(void);
void depends_so_free(void);
int show_aee_system_exception(const char *module, const char *path, unsigned int flag, const char* msg, ...);
int check_lk_load_md_status(int md_id);
//int check_decrypt_ready(void);
int md_image_exist_check(int fd,int md_id);
//int wait_for_property(const char *name, const char *desired_value, int waitmsec);
unsigned int str2uint(char *str);

int main_v1(int md_id, int sub_ver);
int main_v2(int md_id, int sub_ver);



#endif
