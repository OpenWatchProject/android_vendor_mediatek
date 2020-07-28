/*
 * Copyright (c) 2015-2017 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef __UT_KM_WECHAT_TAC_H__
#define __UT_KM_WECHAT_TAC_H__

#include "ut_kmwechat.h"

__BEGIN_DECLS

kmwechat_error_t wechat_generate_attk(const unsigned char copy_num,
                  unsigned char *device, unsigned int dlen);

kmwechat_error_t wechat_verify_attk(void);

kmwechat_error_t wechat_export_attk(unsigned char** key_data, size_t* key_datalen);

kmwechat_error_t wechat_get_device_id(unsigned char** key_data, size_t* key_datalen);

kmwechat_error_t wechat_get_wechat_info(wechat_info_t *wechat_info);

__END_DECLS
#endif //__UT_KM_WECHAT_TAC_H__
