/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF_OSWEGO_M_DOWNLOAD_H__
#define __GF_OSWEGO_M_DOWNLOAD_H__

#include "gf_common.h"
#include "gf_error.h"

extern gf_error_t gf_oswego_m_download_fw_from_file(uint8_t  *fw_data);
extern gf_error_t gf_oswego_m_download_cfg_from_file(uint8_t  *cfg_data,
        gf_config_type_t config_type);
extern uint16_t gf_oswego_m_get_checksum(uint8_t *data, uint32_t len);

#endif  // __GF_OSWEGO_M_DOWNLOAD_H__

