/*
 * Copyright (c) 2019 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "sensor_cfg_sec.h"

extern SENINF_CSI_INFO seninfCSITypeInfo[CUSTOM_CFG_CSI_PORT_MAX_NUM];

CUSTOM_CFG_SECURE sensor_cfg_sec_from_seninf(SENINF_ENUM seninf)
{
    int i;
    SENINF_CSI_INFO *pcsi_info;
    CUSTOM_CFG      *pcust_cfg = getCustomConfig(IMGSENSOR_SENSOR_IDX_MIN_NUM);

    for (i = 0; i < CUSTOM_CFG_CSI_PORT_MAX_NUM; i++) {
        if(seninfCSITypeInfo[i].seninf == seninf) {
            pcsi_info = &seninfCSITypeInfo[i];
            break;
        }
    }

    if (pcsi_info->port >= CUSTOM_CFG_CSI_PORT_MAX_NUM || pcsi_info->port < CUSTOM_CFG_CSI_PORT_0)
        return CUSTOM_CFG_SECURE_NONE;

    while(pcust_cfg->sensorIdx != IMGSENSOR_SENSOR_IDX_NONE && pcust_cfg->port != pcsi_info->port)
        pcust_cfg++;

    if (pcust_cfg->sensorIdx == IMGSENSOR_SENSOR_IDX_NONE)
        return CUSTOM_CFG_SECURE_NONE;

    return pcust_cfg->secure;
}

CUSTOM_CFG_SECURE sensor_cfg_sec_from_sensor_idx(IMGSENSOR_SENSOR_IDX sensor_idx)
{
    return getCustomConfig((IMGSENSOR_SENSOR_IDX)sensor_idx)->secure;
}

