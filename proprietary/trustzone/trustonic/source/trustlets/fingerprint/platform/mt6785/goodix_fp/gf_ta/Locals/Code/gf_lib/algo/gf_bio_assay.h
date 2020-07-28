/*************************************************
 Copyright (C), 1997-2015, Shenzhen Huiding Technology Co.,Ltd.
 Version: V1.1
 Description:
 History:
 2015.11.27 - Yuyi - created
 2015.12.18 - Yuyi - revised
 *************************************************/
#ifndef __GF_BIO_ASSAY_H__
#define __GF_BIO_ASSAY_H__

#include <stdint.h>
#include "gf_error.h"
#include "gf_sensor.h"

void gf_bio_assay_reset(void);

gf_error_t gf_get_bio_buf(int32_t* buf, uint32_t buf_len, gf_milan_a_series_fw_cfg_t* fw_cfg);

void gf_logout_bio_buf(int32_t* buf, int32_t buf_len);

#endif /** __GF_BIO_ASSAY_H__ **/
