/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef __GF_MILAN_A_SERIES_H__
#define __GF_MILAN_A_SERIES_H__

// milan-a chip_id consists with 16bit, in which last 4bit is changeable
// so that we use first 12bit to present milan-a, chip_id >> 4 makes this chip_id
#define MILAN_A_SERIES_SIZE_CONFIG 256

extern gf_error_t gf_milan_a_series_function_init(gf_spi_function_t *spi_function);

gf_error_t gf_milan_a_series_notify_fw_adc_reading_end(void);

gf_error_t gf_milan_a_series_notify_fw_adc_reading_begin(void);

#endif /*__GF_MILAN_A_SERIES_H__ */

