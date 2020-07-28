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

#ifndef __TRUSTZONE_TZ_DDP__
#define __TRUSTZONE_TZ_DDP__

#define TZ_TA_DDP_NAME "DDP TA"
#define TZ_TA_DDP_UUID "dd7b73bc-0244-4072-b541-c9a53d8fbf5b"

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
#define TZCMD_DDP_OVL_IS_EN          10

#define TZCMD_DDP_OVL_ALLOC_MVA      11
#define TZCMD_DDP_OVL_DEALLOC_MVA    12

#define TZCMD_DDP_SECURE_MVA_MAP     13
#define TZCMD_DDP_SECURE_MVA_UNMAP   14

#define TZCMD_DDP_INTR_CALLBACK      15
#define TZCMD_DDP_REGISTER_INTR      16

#define TZCMD_DDP_OVL_BACKUP_REG     17
#define TZCMD_DDP_OVL_RESTORE_REG    18

#define TZCMD_DDP_WDMA_BACKUP_REG    19
#define TZCMD_DDP_WDMA_RESTORE_REG   20

#define TZCMD_DDP_DUMP_REG          30
#define TZCMD_DDP_SET_SECURE_MODE   31

#define TZCMD_DDP_SET_DEBUG_LOG      40

#define TZCMD_DDP_SET_DAPC_MODE     50

#define TZCMD_DDP_WDMA_CONFIG       60
#define TZCMD_DDP_WDMA_INTEN        61
#define TZCMD_DDP_WDMA_INTSTA       62
#define TZCMD_DDP_WDMA_START        63
#define TZCMD_DDP_WDMA_STOP         64
#define TZCMD_DDP_WDMA_RST          65
#define TZCMD_DDP_WDMA1_CONFIG_UV   66



#define TZCMD_DDP_RDMA_ADDR_CONFIG  70

#define TZCMD_DDP_RDMA1_ADDR_CONFIG  71

#endif /* __TRUSTZONE_TZ_DDP__ */
