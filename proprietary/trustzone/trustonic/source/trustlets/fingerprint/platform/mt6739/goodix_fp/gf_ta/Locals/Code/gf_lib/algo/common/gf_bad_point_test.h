/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_BAD_POINT_TEST_H__
#define __GF_BAD_POINT_TEST_H__

#include "gf_error.h"
#include "gf_type_define.h"
#include "gf_common.h"

#ifdef __cplusplus
extern "C" {
#endif

gf_error_t gf_bad_point_test_init(void);

/*
 * * buf  --                  base frame data buffer
 * * buf_len --            base frame data buffer length(Bytes)
 * */
gf_error_t gf_bad_point_test_save_base_frame_data(uint16_t* buf, uint32_t buf_len);

gf_error_t gf_bad_point_test_post_frame_data(uint16_t* buf, uint32_t buf_len, gf_irq_t* irq);

void gf_bad_point_test_destroy(void);

gf_error_t gf_bad_point_test_on_image_irq(uint16_t* raw_data, uint32_t raw_len, gf_irq_t* irq);

#ifdef __cplusplus
}
#endif
#endif  // __GF_BAD_POINT_TEST_H__
