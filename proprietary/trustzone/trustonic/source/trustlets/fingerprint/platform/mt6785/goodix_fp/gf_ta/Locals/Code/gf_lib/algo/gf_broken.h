/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

/**
 * @file   gf_broken.h
 *
 * <Copyright goes here>
 */

#ifndef __TA_GF_BROKEN_H__
#define __TA_GF_BROKEN_H__

gf_error_t gf_broken_check_init(gf_spi_function_t *spi_function, gf_operation_type_t *operation);
gf_error_t gf_broken_check_on_image_irq(uint16_t *raw_data, uint32_t raw_len, gf_irq_t *cmd);
gf_error_t gf_broken_check_on_up_irq(uint16_t *raw_data, uint32_t raw_len, gf_irq_t *cmd);
gf_error_t gf_broken_check_dump(uint16_t *data, uint32_t data_len, uint32_t *frame_count);
gf_error_t gf_broken_check_destroy(void);

#endif  /* __TA_GF_BROKEN_H__ */

