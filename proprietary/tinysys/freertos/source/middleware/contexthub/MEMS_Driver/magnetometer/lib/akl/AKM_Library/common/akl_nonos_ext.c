/******************************************************************************
 *
 * Copyright (c) 2004 Asahi Kasei Microdevices Corporation, Japan
 * All Rights Reserved.
 *
 * This software program is the proprietary program of Asahi Kasei Microdevices
 * Corporation("AKM") licensed to authorized Licensee under the respective
 * agreement between the Licensee and AKM only for use with AKM's electronic
 * compass IC.
 *
 * THIS SOFTWARE IS PROVIDED TO YOU "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABLITY, FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT OF
 * THIRD PARTY RIGHTS, AND WE SHALL NOT BE LIABLE FOR ANY LOSSES AND DAMAGES
 * WHICH MAY OCCUR THROUGH USE OF THIS SOFTWARE.
 *
 ******************************************************************************/
#include "AKL_APIs.h"
#include "akl_smart_compass.h"

static int16_t axisInfo2pat(
    const uint8_t axis_order[3],
    const uint8_t axis_sign[3],
    int16_t *pat
)
{
    /* check invalid input */
    if((axis_order[0] < 0) || (2 < axis_order[0]) ||
       (axis_order[1] < 0) || (2 < axis_order[1]) ||
       (axis_order[2] < 0) || (2 < axis_order[2]) ||
       (axis_sign[0] < 0) || (1 < axis_sign[0]) ||
       (axis_sign[1] < 0) || (1 < axis_sign[1]) ||
       (axis_sign[2] < 0) || (1 < axis_sign[2]) ||
      ((axis_order[0] * axis_order[1] * axis_order[2]) != 0) ||
      ((axis_order[0] + axis_order[1] + axis_order[2]) != 3)) {
        *pat = 0;

        return AKM_ERR_INVALID_ARG;
    } else {
        /* calculate pat
         * BIT MAP
         * [8] = sign_x
         * [7] = sign_y
         * [6] = sign_z
         * [5:4] = order_x
         * [3:2] = order_y
         * [1:0] = order_z
         */
        *pat = ((int16_t)axis_sign[0] << 8);
        *pat += ((int16_t)axis_sign[1] << 7);
        *pat += ((int16_t)axis_sign[2] << 6);
        *pat += ((int16_t)axis_order[0] << 4);
        *pat += ((int16_t)axis_order[1] << 2);
        *pat += ((int16_t)axis_order[2] << 0);

        return AKM_SUCCESS;
    }
}

uint16_t set_cert(
    struct AKL_SCL_PRMS                 *mem,
    const struct AKL_CERTIFICATION_INFO *info)
{
    uint8_t                       axis_order[3];
    uint8_t                       axis_sign[3];
    int16_t                       ret;

    if (mem == NULL) {
        return (uint16_t)AKM_ERR_INVALID_ARG;
    }

    axis_order[0] = (uint8_t)info->a_key[5];
    axis_order[1] = (uint8_t)info->a_key[6];
    axis_order[2] = (uint8_t)info->a_key[7];
    axis_sign[0] = (uint8_t)info->a_key[8];
    axis_sign[1] = (uint8_t)info->a_key[9];
    axis_sign[2] = (uint8_t)info->a_key[10];
    ret = axisInfo2pat(axis_order, axis_sign, &mem->m_pat);

    if (ret) {
            return AKM_ERROR;
    }

    /* Copy certification info. */
    mem->s_cert = *info;

    return (uint16_t)AKM_SUCCESS;
}

