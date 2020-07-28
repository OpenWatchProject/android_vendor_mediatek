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
#ifndef __IMGSENSOR_SEC_SENSORLIST_H__
#define __IMGSENSOR_SEC_SENSORLIST_H__

#include "kd_imgsensor_define.h"
#include "imgsensor_sec_typedef.h"

/*Add sensor ID here*/
#define S5K4E6_SENSOR_ID                        0x4e60

/*Add sensor Init function here*/
UINT32 S5K4E6_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);

/*------------------Common structure----------------------*/
struct IMGSENSOR_SEC_SENSOR_LIST {
	MUINT32 id;
	MUINT32 (*init)(struct SENSOR_FUNCTION_STRUCT **pfFunc);
};

extern struct IMGSENSOR_SEC_SENSOR_LIST imgsensor_sec_sensor_list[];

#endif
