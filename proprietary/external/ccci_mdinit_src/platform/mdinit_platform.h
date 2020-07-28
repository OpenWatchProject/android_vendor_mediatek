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

#ifndef __CCCI_MDINIT_PLATFORM_H__
#define __CCCI_MDINIT_PLATFORM_H__

void store_md_statue(int md_id, char *buf, int len);
void delay_to_reset_md(int version);
/*boot env: sbp */
int get_md_sbp_code(int md_id, int version);
/*boot env*/
int get_mdlog_boot_mode(int md_id);
int get_md_dbg_dump_flag(int md_id);
int get_sbp_subid_setting(void);


#endif
