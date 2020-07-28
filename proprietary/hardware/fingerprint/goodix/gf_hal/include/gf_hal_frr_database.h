/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF_HAL_FRR_DATABASE_H__
#define __GF_HAL_FRR_DATABASE_H__

#include <stdio.h>
#include "gf_error.h"
#include "gf_common.h"
#include "gf_type_define.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PROTOCOL_VERSION 0001

#define FRR_DATABASE_FILE "/data/gf_data/frr_database.db"
#define FRR_DATABASE_DIR "/data/gf_data/"
/*ensure the database file must be shorter than 10KB*/
#define FRR_DATABASE_METADATA_MAX 800

typedef enum {
    TAG_PACKAGE_VERSION = 0,
    TAG_PROTOCOL_VERSION,
    TAG_CHIP_TYPE,
    TAG_SCREEN_ON_AUTHENTICATE_FAIL_RETRY_COUNT,
    TAG_SCREEN_OFF_AUTHENTICATE_FAIL_RETRY_COUNT,
    TAG_CHIP_SUPPORT_BIO,
    TAG_IS_BIO_ENABLE,
    TAG_AUTHENTICATED_WITH_BIO_SUCCESS_COUNT,
    TAG_AUTHENTICATED_WITH_BIO_FAILED_COUNT,
    TAG_AUTHENTICATED_SUCCESS_COUNT,
    TAG_AUTHENTICATED_FAILED_COUNT,
    TAG_BUF_FULL,
    TAG_UPDATE_POS
} gf_frr_tag_e;

extern gf_error_t gf_hal_handle_frr_database(gf_error_t err, int32_t image_quality,
        int32_t image_area);
extern uint32_t gf_hal_get_tag_data_pos(gf_frr_tag_e tag_idx);
extern int gf_hal_get_tag_data(FILE *fp, uint32_t pos);
extern gf_error_t gf_hal_judge_delete_frr_database(gf_chip_type_t chip_type,
        gf_chip_series_t chip_series);

#ifdef __cplusplus
}
#endif

#endif  // __GF_HAL_FRR_DATABASE_H__

