/*************************************************
 Copyright (C), 1997-2015, Shenzhen Huiding Technology Co.,Ltd.
 Version: V1.1
 Description:
 History:
 2015.11.27 - Yuyi - created
 2015.12.18 - Yuyi - revised
 *************************************************/
#ifndef __GF_HBD_TEST_H__
#define __GF_HBD_TEST_H__

#include <stdint.h>
#include "gf_error.h"

gf_error_t gf_calutate_hbd_avg(uint16_t *hbd_avg, uint8_t *data, uint32_t len);

#endif  // __GF_HBD_TEST_H__
