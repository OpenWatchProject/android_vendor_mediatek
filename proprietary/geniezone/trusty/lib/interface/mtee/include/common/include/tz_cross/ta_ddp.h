/*
* Copyright (c) 2015 MediaTek Inc.
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

#ifndef __TRUSTZONE_TA_DDP__
#define __TRUSTZONE_TA_DDP__

#define TZ_TA_DDPU_NAME "DDPU TA"
#define TZ_TA_DDPU_UUID "11d28272-5c14-47a9-9f2b-180dc48ec29f"

/* Data Structure for Test TA */
/* You should define data structure used both in REE/TEE here
   N/A for Test TA */

/* Command for DDP TA */
#define TZCMD_DDP_OVL_START          0
#define TZCMD_DDP_OVL_STOP           1
#define TZCMD_DDP_OVL_RESET          2
#define TZCMD_DDP_OVL_ROI            3
#define TZCMD_DDP_OVL_LAYER_SWITCH   4
#define TZCMD_DDP_OVL_LAYER_CONFIG   5
#define TZCMD_DDP_OVL_3D_CONFIG      6
#define TZCMD_DDP_OVL_LAYER_TDSHP_EN 7
#define TZCMD_DDP_OVL_TEST           8
#define TZCMD_DDP_OVL_CONFIG_LAYER_ADDR      9
#define TZCMD_DDP_OVL_IS_EN      10

/* Data Structure for Test TA */
/* You should define data structure used both in REE/TEE here
   N/A for Test TA */

/* Command for DDP TA */
/* rotator control */
#define TZCMD_DDPU_ROT_ENABLE         0
#define TZCMD_DDPU_ROT_DISABLE        1
#define TZCMD_DDPU_ROT_RESET          2
#define TZCMD_DDPU_ROT_CONFIG         3
#define TZCMD_DDPU_ROT_CON            4
#define TZCMD_DDPU_ROT_EXTEND_FUNC    5

/* write dma control */
#define TZCMD_DDPU_WDMA_START         30
#define TZCMD_DDPU_WDMA_STOP          31
#define TZCMD_DDPU_WDMA_RESET         32
#define TZCMD_DDPU_WDMA_CONFIG        33
#define TZCMD_DDPU_WDMA_CONFIG_UV     34
#define TZCMD_DDPU_WDMA_WAIT          35
#define TZCMD_DDPU_WDMA_EXTEND_FUNC   36

#define TZCMD_DDPU_INTR_CALLBACK      39
#define TZCMD_DDPU_REGISTER_INTR      40

#define TZCMD_DDPU_SET_DAPC_MODE      50

#endif /* __TRUSTZONE_TA_DDP__ */
