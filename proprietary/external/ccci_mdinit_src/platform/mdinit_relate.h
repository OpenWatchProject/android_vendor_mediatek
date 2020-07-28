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

#ifndef __CCCI_MDINIT_RELATE_H__
#define __CCCI_MDINIT_RELATE_H__

int get_rsc_protol_value(int md_id, char *buf, int len);

int start_service_verified(const char *service_name, const char*service_status_name, int waitmsec);
int stop_service_verified(const char *service_name, const char*service_status_name, int waitmsec);

void start_all_ccci_up_layer_services(void);
void stop_all_ccci_up_layer_services(void);
void check_to_restart_md_v2(unsigned int monitor_fd, int first_boot);

void notify_md_status(int status, int flight_mode, char *buf);
void md_init_related_init(int md_id);


int check_decrypt_ready(void);

int wait_for_property(const char *name, const char *desired_value, int waitmsec);

#endif



