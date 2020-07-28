/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef MTK_OMX_VDECEX
#define MTK_OMX_VDECEX

#include "stdlib.h"
#include <dlfcn.h>
#include "osal_utils.h"
#include <sched.h>
#include <cutils/properties.h>
#include "Mutex.h"

#define ATRACE_TAG ATRACE_TAG_VIDEO
#include <utils/Trace.h>

#include "MtkOmxBase.h"
#include "MJCFramework.h"
#include "MtkOmxMVAMgr.h"
#include "../../../omx/core/src/MtkOmxCore.h"

////////for fence in M0/////////////
#include <ui/Fence.h>
#define MTK_OMX_FENCE_TIMEOUT_MS 1000
///////////////////end///////////

#define VIDEO_M4U_MAX_BUFFER 100
#include "val_api_public.h"


#include "vdec_drv_if_public.h"
#define DEC_TRYALLOCMEMCOUNT 10
#define VC1_SEQUENCE_LAYER_LEN 36
#define PROFILING 1

typedef enum MTK_OMX_BUFFER_TYPE_ID_EXT
{
    MTK_OMX_FILL_CONVERTED_BUFFER_DONE_TYPE = MTK_OMX_FILL_THIS_BUFFER_TYPE+1,
} MTK_OMX_BUFFER_TYPE_ID_EXT;
#define MAX_COLORCONVERT_OUTPUTBUFFER_COUNT 1
#define CROSSMOUNT_MAX_COLORCONVERT_OUTPUTBUFFER_COUNT 2


#include <linux/ion_drv.h>
#include <ion.h>
#include <ion/ion.h>

#include <HardwareAPI.h>

#define VIDEO_ION_MAX_BUFFER 50
#define DEC_BS_PROTECT_MEM_SZ 512

#undef LOG_TAG
#define LOG_TAG "MtkOmxVdecEx"

#define MTK_OMX_LOGD(fmt, arg...)       \
if (this->mOmxVdecLogEnable) \
{  \
    ALOGD("[0x%08x] " fmt, this, ##arg) ;  \
}

#define MTK_OMX_LOGUD(fmt, arg...)       \
if (this->mOmxVdecPerfLogEnable) \
{  \
    ALOGD("[0x%08x] " fmt, this, ##arg) ;  \
}

#define MTK_OMX_LOGU(fmt, arg...)    ALOGD("[0x%08x] " fmt, this, ##arg)
#define MTK_OMX_LOGE(fmt, arg...)    ALOGE("[0x%08x] " fmt, this, ##arg)

#define VDEC_ROUND_16(X)     ((X + 0xF) & (~0xF))
#define VDEC_ROUND_32(X)     ((X + 0x1F) & (~0x1F))
#define VDEC_ROUND_N(X, N)   ((X + (N-1)) & (~(N-1)))    //only for N is exponential of 2

#define MTK_OMXVDEC_EXTENSION_INDEX_PARAM_SET_VDEC_SCENARIO "OMX.MTK.index.param.video.SetVdecScenario"

#define RT_THREAD_PRI_OMX_VIDEO 98

#define LOCK_TIMEOUT_MS 5000
#define LOCK_TIMEOUT_S 5

#define SECURE_OUTPUT_USE_ION 1

#define LOCK_T(X) \
    if (0 != LOCK_TIMEOUT(X,LOCK_TIMEOUT_S)){ \
        ALOGE("## [ERROR, %d] %s() line: %d LOCK timeout...", errno, __FUNCTION__,__LINE__); \
        LOCK(X); \
    }

#define WAIT_T(X) \
    if (0 != WAIT_TIMEOUT(X,LOCK_TIMEOUT_S)){ \
        ALOGE("## [ERROR, %d] %s() line: %d WAIT timeout...", errno, __FUNCTION__,__LINE__); \
        WAIT(X);\
    }

#define CLEAR_SEMAPHORE(X) \
    do { \
        int count = get_sem_value(&(X)); \
        for (; count > 0; count --) { \
            WAIT_T((X)); \
        } \
    } while (0)

typedef enum
{
    MTK_SECURE_AL_FAIL      = 0,
    MTK_SECURE_AL_SUCCESS   = 1,
} MTK_SECURE_AL_RESULT;


//
//  (a) MTK_VDEC_XXX_DEFAULT_OUTPUT_BUFFER_COUNT                                     // Codec DPB number
//  (b) MTK_MJC_DEFAULT_OUTPUT_BUFFER_COUNT + MTK_MJC_REF_VDEC_OUTPUT_BUFFER_COUNT   // MJC requirement (Actual MJC buffer + 2*EXTRA_CANCEL_BUFFER_CNT)
//  (c) MAX_MIN_UNDEQUEUED_BUFS                                                      // MIN_UNDEQUEUED_BUFS + EXTRA_CANCEL_BUFFER_CNT (Actual cancel buffer count)
//  (d) FRAMEWORK_OVERHEAD                                                           // Extra buffers besides DPB to make buffer flow
//  (e) MIN_UNDEQUEUED_BUFS                                                          // Should be identical to ACodec's one
//                                                                                              mNativeWindow->query(
//                                                                                                 mNativeWindow.get(), NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS,
//                                                                                                 (int *)minUndequeuedBuffers)
//  (f) EXTRA_CANCEL_BUFFER_CNT                                                      // Extra buffer besides MIN_UNDEQUEUED_BUFS
//  (g) mMinUndequeuedBufs                                                           // Set thru setParameter(OMX_IndexVendorMtkOmxVdecGetMinUndequeuedBufs) from ACodec.
//                                                                                             This value will be decided at run-time.
//
//
// Total buffer count: a + b + d + g
//


#if (ANDROID_VER >= ANDROID_KK)
#define MIN_UNDEQUEUED_BUFS 4 // Should be identical to ACodec's one
// mNativeWindow->query(
// mNativeWindow.get(), NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS,
// (int *)minUndequeuedBuffers)
#endif


#define FRAMEWORK_OVERHEAD 3
#define EXTRA_CANCEL_BUFFER_CNT 0
//#define TOTAL_MJC_BUFFER_CNT    0


#define MAX_MIN_UNDEQUEUED_BUFS     MIN_UNDEQUEUED_BUFS + EXTRA_CANCEL_BUFFER_CNT


#if (ANDROID_VER >= ANDROID_KK)
#define MAX_TOTAL_BUFFER_CNT     64
#else
#define MAX_TOTAL_BUFFER_CNT     32
#endif

typedef struct _MJC_BUF_INFO
{
    VAL_UINT32_T u4Hdr;     // header address
    VAL_UINT32_T u4VA;
    VAL_UINT32_T u4PA;
    VAL_UINT32_T u4Size;
    void *pNativeHandle;
} MJC_BUF_INFO;


typedef struct _VdecIonBufInfo
{
    VAL_UINT32_T u4OriVA;
    VAL_UINT32_T u4BuffHdr;
    VAL_UINT32_T u4VA;
    VAL_UINT32_T u4PA;
    VAL_UINT32_T u4BuffSize;
    int         ori_fd;
    int          fd;
    int          pIonBufhandle;
    void *pNativeHandle;

    unsigned long secure_handle;
    unsigned long va_share_handle;   // for testing (decoder output to frame buffer va)

} VdecIonBufInfo;


struct UseIonBufferParams
{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_PTR pAppPrivate;
    unsigned char *virAddr;
    OMX_S32 Ionfd;
    size_t size;
    OMX_BUFFERHEADERTYPE **bufferHeader;
};


typedef struct _VdecSecFrmBufInfo
{
    VAL_UINT32_T u4BuffId;
    VAL_UINT32_T u4BuffHdr;
    VAL_UINT32_T u4BuffSize;
    VAL_UINT32_T u4SecHandle;
    void *pNativeHandle;
} VdecSecFrmBufInfo;

typedef struct _VdecSecInputBufInfo
{
    VAL_UINT32_T u4IonShareFd;
    VAL_UINT32_T pIonHandle;
    VAL_UINT32_T u4SecHandle;
    void * pNativeHandle;
} VdecSecInputBufInfo;


/*
//for ACodec color convert
typedef struct MTK_VDEC_ACODEC_MVA_ADDR_MAPPING {
#define VIDEO_M4U_MAX_BUFFER 100
    OMX_U32 mM4UBufferPaA[VIDEO_M4U_MAX_BUFFER];
    OMX_U32 mM4UBufferPaB[VIDEO_M4U_MAX_BUFFER];
    OMX_U32 mM4UBufferPaC[VIDEO_M4U_MAX_BUFFER];
    OMX_U32 mM4UBufferCount;
    OMX_U32 mInputBufferPopulatedCnt;
    OMX_U32 mOutputBufferPopulatedCnt;
} MTK_VDEC_ACODEC_MVA_ADDR_MAPPING;
*/

#define ANDROID_VER 447 // M-EAC
#define ANDROID_KK  443
#define ANDROID_ICS 440
#define ANDROID_M  446

/* HEVC SEC Macro */
#define MTK_VDEC_HEVCSEC_DEFAULT_INPUT_BUFFER_COUNT 5
#define MTK_VDEC_HEVCSEC_DEFAULT_INPUT_BUFFER_SIZE  2800*1024

#define MTK_OMXVDEC_EXTENSION_INDEX_PARAM_3D_STEREO_PLAYBACK  "OMX.MTK.index.param.video.3DVideoPlayback"
#define MTK_OMXVDEC_EXTENSION_INDEX_PARAM_PRIORITY_ADJUSTMENT "OMX.MTK.index.param.video.EnablePriorityAdjustment"
#define MTK_OMX_EXTENSION_INDEX_PARTIAL_FRAME_QUERY_SUPPORTED "OMX.MTK.index.param.PartialFrameSupport"
#define MTK_OMXVDEC_EXTENSION_INDEX_PARAM_SWITCH_BW_TVOUT "OMX.MTK.index.param.video.SwitchBwTVout"
#define MTK_OMXVDEC_EXTENSION_INDEX_PARAM_STREAMING_MODE "OMX.MTK.index.param.video.StreamingMode"
#define MTK_OMXVDEC_EXTENSION_INDEX_PARAM_NO_REORDER_MODE "OMX.MTK.index.param.video.DecNoReorderMode"
#define MTK_OMXVDEC_EXTENSION_INDEX_PARAM_FIXED_MAX_BUFFER "OMX.MTK.index.param.video.FixedMaxBuffer"
#define MTK_OMXVDEC_EXTENSION_INDEX_PARAM_SKIP_REFERENCE_CHECK_MODE "OMX.MTK.index.param.video.SkipReferenceCheckMode"
#define MTK_OMXVDEC_EXTENSION_INDEX_PARAM_LOW_LATENCY_DECODE "OMX.MTK.index.param.video.LowLatencyDecode"
#define MTK_OMXVDEC_EXTENSION_INDEX_PARAM_SLICE_LOSS_INDICATION "OMX.MTK.index.param.video.SlicelossIndication"
#define MTK_OMXVDEC_EXTENSION_INDEX_PARAM_TASK_GROUP "OMX.MTK.index.param.video.TaskGroup"

//secure in-house decode lib&aip name
#define MTK_COMMON_SEC_VDEC_IN_HOUSE_LIB_NAME "lib_uree_mtk_video_secure_al.so"
#define MTK_COMMON_SEC_VDEC_IN_HOUSE_INIT_NAME "MtkVideoSecureMemAllocatorInit"
#define MTK_COMMON_SEC_VDEC_IN_HOUSE_DEINIT_NAME "MtkVideoSecureMemAllocatorDeInit"
#define MTK_COMMON_SEC_VDEC_IN_HOUSE_ALLOCATE_SECURE_FRAME_BUFFER "MtkVideoAllocateSecureFrameBuffer"
#define MTK_COMMON_SEC_VDEC_IN_HOUSE_REGISTER_SHARED_MEMORY "MtkVideoRegisterSharedMemory"
#define MTK_COMMON_SEC_VDEC_IN_HOUSE_ALLOCATE_SECURE_BUFFER "MtkVideoAllocateSecureBuffer"
#define MTK_COMMON_SEC_VDEC_IN_HOUSE_FREE_SECURE_BUFFER "MtkVideoFreeSecureBuffer"
#define MTK_COMMON_SEC_VDEC_IN_HOUSE_FREE_SECURE_FRAME_BUFFER "MtkVideoFreeSecureFrameBuffer"

//VDEC MTK In-house function pointer types
typedef MTK_SECURE_AL_RESULT MtkVideoSecureMemAllocatorInit_Ptr(int);
typedef unsigned long MtkVideoAllocateSecureFrameBuffer_Ptr(int , int, const char*,int);
typedef MTK_SECURE_AL_RESULT MtkVideoFreeSecureFrameBuffer_Ptr(unsigned long);
typedef unsigned long MtkVideoRegisterSharedMemory_Ptr(void *, int, const char*,int);
typedef unsigned long MtkVideoAllocateSecureBuffer_Ptr(int , int, const char*,int);
typedef MTK_SECURE_AL_RESULT MtkVideoFreeSecureBuffer_Ptr(unsigned long,int);

#if 0
MTK_SECURE_AL_RESULT MtkVideoSecureMemAllocatorInit();
MTK_SECURE_AL_RESULT MtkVideoSecureMemAllocatorDeinit();
unsigned long MtkVideoAllocateSecureBuffer(int size, int align, const char* tag);
MTK_SECURE_AL_RESULT MtkVideoFreeSecureBuffer(unsigned long memHandle);
unsigned long MtkVideoAllocateSecureFrameBuffer(int size, int align, const char* tag);
MTK_SECURE_AL_RESULT MtkVideoFreeSecureFrameBuffer(unsigned long memHandle);
unsigned long MtkVideoRegisterSharedMemory(void *buffer, int size);
MTK_SECURE_AL_RESULT MtkVideoUnregisterSharedMemory(unsigned long sharedHandle);
#endif

// VDEC TLC lib & api names
//#define MTK_H264_SEC_VDEC_TLC_LIB_NAME "libMtkH264SecVdecTLCLib.so"
#define MTK_H264_SEC_VDEC_TLC_HANDLE_CREATE_NAME "MtkH264SecVdec_tlcHandleCreate"
#define MTK_H264_SEC_VDEC_TLC_OPEN_NAME "MtkH264SecVdec_tlcOpen"
#define MTK_H264_SEC_VDEC_TLC_INIT_NAME "MtkH264SecVdec_tlcInit"
#define MTK_H264_SEC_VDEC_TLC_DECODE_NAME "MtkH264SecVdec_tlcDecode"
#define MTK_H264_SEC_VDEC_TLC_DEINIT_NAME "MtkH264SecVdec_tlcDeinit"
#define MTK_H264_SEC_VDEC_TLC_CLOSE_NAME "MtkH264SecVdec_tlcClose"
#define MTK_H264_SEC_VDEC_TLC_DUMP_SECMEM_NAME "MtkH264SecVdec_tlcDumpSecMem"
#define MTK_H264_SEC_VDEC_TLC_FILL_SECMEM_NAME "MtkH264SecVdec_tlcFillSecMem"
#define MTK_H264_SEC_VDEC_SEC_MEM_INIT_NAME "MtkH264SecVdec_secMemInit"
#define MTK_H264_SEC_VDEC_SEC_MEM_DEINIT_NAME "MtkH264SecVdec_secMemDeInit"
#define MTK_H264_SEC_VDEC_SEC_MEM_ALLOCATE_NAME "MtkH264SecVdec_secMemAllocate"
#define MTK_H264_SEC_VDEC_SEC_MEM_ALLOCATE_TBL_NAME "MtkH264SecVdec_secMemAllocateTBL"
#define MTK_H264_SEC_VDEC_SEC_MEM_FREE_NAME "MtkH264SecVdec_secMemFree"
#define MTK_H264_SEC_VDEC_SEC_MEM_FREE_TBL_NAME "MtkH264SecVdec_secMemFreeTBL"
// VDEC TLC function pointer types
typedef void *MtkH264SecVdec_tlcHandleCreate_Ptr();
typedef int MtkH264SecVdec_tlcOpen_Ptr(void *);
typedef int MtkH264SecVdec_tlcInit_Ptr(void *, void *, uint32_t, void *, uint32_t, void *, uint32_t, void *, uint32_t, uint32_t);
typedef int MtkH264SecVdec_tlcDecode_Ptr(void *, void *, uint32_t, uint32_t, void *, void *, uint32_t, uint32_t, void *);
typedef int MtkH264SecVdec_tlcDeinit_Ptr(void *);
typedef void MtkH264SecVdec_tlcClose_Ptr(void *);
typedef int MtkH264SecVdec_tlcDumpSecMem_Ptr(void *, uint32_t, void *, uint32_t);
typedef int MtkH264SecVdec_tlcFillSecMem_Ptr(void *, void *, uint32_t, uint32_t);
typedef int MtkH264SecVdec_secMemInit_Ptr();
typedef int MtkH264SecVdec_secMemDeInit_Ptr();
typedef unsigned long MtkH264SecVdec_secMemAllocate_Ptr(uint32_t, uint32_t);
typedef unsigned long MtkH264SecVdec_secMemAllocateTBL_Ptr(uint32_t, uint32_t);
typedef int MtkH264SecVdec_secMemFree_Ptr(unsigned long);
typedef int MtkH264SecVdec_secMemFreeTBL_Ptr(unsigned long);

// VDEC TLC lib & api names
//#define MTK_H265_SEC_VDEC_TLC_LIB_NAME "HEVCSecureVdecCA.so"
#define MTK_H265_SEC_VDEC_TLC_HANDLE_CREATE_NAME "MtkH265SecVdec_tlcHandleCreate"
#define MTK_H265_SEC_VDEC_TLC_OPEN_NAME "MtkH265SecVdec_tlcOpen"
#define MTK_H265_SEC_VDEC_TLC_INIT_NAME "MtkH265SecVdec_tlcInit"
#define MTK_H265_SEC_VDEC_TLC_DECODE_NAME "MtkH265SecVdec_tlcDecode"
#define MTK_H265_SEC_VDEC_TLC_DEINIT_NAME "MtkH265SecVdec_tlcDeinit"
#define MTK_H265_SEC_VDEC_TLC_CLOSE_NAME "MtkH265SecVdec_tlcClose"
#define MTK_H265_SEC_VDEC_TLC_DUMP_SECMEM_NAME "MtkH265SecVdec_tlcDumpSecMem"
#define MTK_H265_SEC_VDEC_TLC_FILL_SECMEM_NAME "MtkH265SecVdec_tlcFillSecMem"
#define MTK_H265_SEC_VDEC_SEC_MEM_INIT_NAME "MtkH265SecVdec_secMemInit"
#define MTK_H265_SEC_VDEC_SEC_MEM_DEINIT_NAME "MtkH265SecVdec_secMemDeInit"
#define MTK_H265_SEC_VDEC_SEC_MEM_ALLOCATE_NAME "MtkH265SecVdec_secMemAllocate"
#define MTK_H265_SEC_VDEC_SEC_MEM_ALLOCATE_TBL_NAME "MtkH265SecVdec_secMemAllocateTBL"
#define MTK_H265_SEC_VDEC_SEC_MEM_FREE_NAME "MtkH265SecVdec_secMemFree"
#define MTK_H265_SEC_VDEC_SEC_MEM_FREE_TBL_NAME "MtkH265SecVdec_secMemFreeTBL"
// VDEC TLC function pointer types
typedef void *MtkH265SecVdec_tlcHandleCreate_Ptr();
typedef int MtkH265SecVdec_tlcOpen_Ptr(void *);
typedef int MtkH265SecVdec_tlcInit_Ptr(void *, void *, uint32_t, void *, uint32_t, void *, uint32_t, void *, uint32_t, uint32_t);
typedef int MtkH265SecVdec_tlcSystraceEnable_Ptr(void *, uint32_t);
typedef int MtkH265SecVdec_tlcDecode_Ptr(void *, void *, uint32_t, uint32_t, void *, void *, uint32_t, uint32_t, void *);
typedef int MtkH265SecVdec_tlcDeinit_Ptr(void *);
typedef void MtkH265SecVdec_tlcClose_Ptr(void *);
typedef int MtkH265SecVdec_tlcDumpSecMem_Ptr(void *, uint32_t, void *, uint32_t);
typedef int MtkH265SecVdec_tlcFillSecMem_Ptr(void *, void *, uint32_t, uint32_t);
typedef int MtkH265SecVdec_secMemInit_Ptr();
typedef int MtkH265SecVdec_secMemDeInit_Ptr();
typedef unsigned long MtkH265SecVdec_secMemAllocate_Ptr(uint32_t, uint32_t);
typedef unsigned long MtkH265SecVdec_secMemAllocateTBL_Ptr(uint32_t, uint32_t);
typedef int MtkH265SecVdec_secMemFree_Ptr(unsigned long);
typedef int MtkH265SecVdec_secMemFreeTBL_Ptr(unsigned long);

// VDEC TLC lib & api names
#define MTK_VP9_SEC_VDEC_TLC_LIB_NAME "libMtkVP9SecVdecTLCLib.so"
#define MTK_VP9_SEC_VDEC_TLC_HANDLE_CREATE_NAME "MtkVP9SecVdec_tlcHandleCreate"
#define MTK_VP9_SEC_VDEC_TLC_OPEN_NAME "MtkVP9SecVdec_tlcOpen"
#define MTK_VP9_SEC_VDEC_TLC_INIT_NAME "MtkVP9SecVdec_tlcInit"
#define MTK_VP9_SEC_VDEC_TLC_DECODE_NAME "MtkVP9SecVdec_tlcDecode"
#define MTK_VP9_SEC_VDEC_TLC_DEINIT_NAME "MtkVP9SecVdec_tlcDeinit"
#define MTK_VP9_SEC_VDEC_TLC_CLOSE_NAME "MtkVP9SecVdec_tlcClose"
#define MTK_VP9_SEC_VDEC_TLC_DUMP_SECMEM_NAME "MtkVP9SecVdec_tlcDumpSecMem"
#define MTK_VP9_SEC_VDEC_TLC_FILL_SECMEM_NAME "MtkVP9SecVdec_tlcFillSecMem"
#define MTK_VP9_SEC_VDEC_SEC_MEM_INIT_NAME "MtkVP9SecVdec_secMemInit"
#define MTK_VP9_SEC_VDEC_SEC_MEM_DEINIT_NAME "MtkVP9SecVdec_secMemDeInit"
#define MTK_VP9_SEC_VDEC_SEC_MEM_ALLOCATE_NAME "MtkVP9SecVdec_secMemAllocate"
#define MTK_VP9_SEC_VDEC_SEC_MEM_ALLOCATE_TBL_NAME "MtkVP9SecVdec_secMemAllocateTBL"
#define MTK_VP9_SEC_VDEC_SEC_MEM_FREE_NAME "MtkVP9SecVdec_secMemFree"
#define MTK_VP9_SEC_VDEC_SEC_MEM_FREE_TBL_NAME "MtkVP9SecVdec_secMemFreeTBL"
// VDEC TLC function pointer types
typedef void *MtkVP9SecVdec_tlcHandleCreate_Ptr();
typedef int MtkVP9SecVdec_tlcOpen_Ptr(void *);
typedef int MtkVP9SecVdec_tlcInit_Ptr(void *, void *, uint32_t, uint32_t);
typedef int MtkVP9SecVdec_tlcDecode_Ptr(void *, void *, uint32_t, uint32_t, void *, uint32_t);
typedef int MtkVP9SecVdec_tlcDeinit_Ptr(void *, void *);
typedef void MtkVP9SecVdec_tlcClose_Ptr(void *);
typedef int MtkVP9SecVdec_tlcDumpSecMem_Ptr(void *, uint32_t, void *, uint32_t);
typedef int MtkVP9SecVdec_tlcFillSecMem_Ptr(void *, void *, uint32_t, uint32_t);
typedef int MtkVP9SecVdec_secMemInit_Ptr();
typedef int MtkVP9SecVdec_secMemDeinit_Ptr();
typedef unsigned long MtkVP9SecVdec_secMemAllocate_Ptr(uint32_t, uint32_t);
typedef unsigned long MtkVP9SecVdec_secMemAllocateTBL_Ptr(uint32_t align, uint32_t size);
typedef int MtkVP9SecVdec_secMemFree_Ptr(unsigned long);
typedef int MtkVP9SecVdec_secMemFreeTBL_Ptr(unsigned long sec_mem_handle);

typedef enum MTK_VDEC_CODEC_ID
{
    MTK_VDEC_CODEC_ID_H263,
    MTK_VDEC_CODEC_ID_MPEG4,
    MTK_VDEC_CODEC_ID_AVC,
    MTK_VDEC_CODEC_ID_RV,
    MTK_VDEC_CODEC_ID_VC1,
    MTK_VDEC_CODEC_ID_VPX,
    MTK_VDEC_CODEC_ID_VP9,
    MTK_VDEC_CODEC_ID_MPEG2,
    MTK_VDEC_CODEC_ID_DIVX,
    MTK_VDEC_CODEC_ID_DIVX3,
    MTK_VDEC_CODEC_ID_XVID,
    MTK_VDEC_CODEC_ID_MJPEG,
    MTK_VDEC_CODEC_ID_S263,
    MTK_VDEC_CODEC_ID_HEVC,
    MTK_VDEC_CODEC_ID_INVALID = 0xFFFFFFFF,
} MTK_VDEC_CODEC_ID;

typedef enum
{
    AVC_NALTYPE_SLICE    = 1,    /* non-IDR non-data partition */
    AVC_NALTYPE_DPA      = 2,    /* data partition A */
    AVC_NALTYPE_DPB      = 3,    /* data partition B */
    AVC_NALTYPE_DPC      = 4,    /* data partition C */
    AVC_NALTYPE_IDR      = 5,    /* IDR NAL */
    AVC_NALTYPE_SEI      = 6,    /* supplemental enhancement info */
    AVC_NALTYPE_SPS      = 7,    /* sequence parameter set */
    AVC_NALTYPE_PPS      = 8,    /* picture parameter set */
    AVC_NALTYPE_AUD      = 9,    /* access unit delimiter */
    AVC_NALTYPE_EOSEQ    = 10,   /* end of sequence */
    AVC_NALTYPE_EOSTREAM = 11,  /* end of stream */
    AVC_NALTYPE_FILL     = 12    /* filler data */
} AVCNalType;

typedef enum
{
    HEVC_ColorPrimaries_Reserved_1  = 0,
    HEVC_ColorPrimaries_BT709_5     = 1,
    HEVC_ColorPrimaries_Unspecified = 2,
    HEVC_ColorPrimaries_Reserved_2  = 3,
    HEVC_ColorPrimaries_BT470_6M    = 4,
    HEVC_ColorPrimaries_BT601_6_625 = 5,
    HEVC_ColorPrimaries_BT601_6_525 = 6,
    HEVC_ColorPrimaries_SMPTE_240M  = 7,
    HEVC_ColorPrimaries_GenericFilm = 8,
    HEVC_ColorPrimaries_BT2020      = 9,
    HEVC_ColorPrimaries_SMPTE_ST428 = 10
} HEVCColorPrimaries;

typedef enum
{
    HEVC_Transfer_Reserved_1    = 0,
    HEVC_Transfer_BT709_5       = 1,
    HEVC_Transfer_Unspecified   = 2,
    HEVC_Transfer_Reserved_2    = 3,
    HEVC_Transfer_Gamma22       = 4,
    HEVC_Transfer_Gamma28       = 5,
    HEVC_Transfer_BT601_6_625   = 6,
    HEVC_Transfer_SMPTE_240M    = 7,
    HEVC_Transfer_Linear        = 8,
    HEVC_Transfer_Log_1         = 9,
    HEVC_Transfer_Log_2         = 10,
    HEVC_Transfer_IEC_61966_2_4 = 11,
    HEVC_Transfer_BT1361        = 12,
    HEVC_Transfer_IEC_61966_2_1 = 13,
    HEVC_Transfer_BT2020_1      = 14,
    HEVC_Transfer_BT2020_2      = 15,
    HEVC_Transfer_SMPTE_ST_2048 = 16,
    HEVC_Transfer_SMPTE_ST_428  = 17,
    HEVC_Transfer_HLG           = 18
} HEVCTransfer;

typedef enum
{
    HEVC_MatrixCoeff_Identity    = 0,
    HEVC_MatrixCoeff_BT709_5     = 1,
    HEVC_MatrixCoeff_Unspecified = 2,
    HEVC_MatrixCoeff_Reserved    = 3,
    HEVC_MatrixCoeff_BT470_6M    = 4,
    HEVC_MatrixCoeff_BT601_6     = 5,
    HEVC_MatrixCoeff_SMPTE_170M  = 6,
    HEVC_MatrixCoeff_SMPTE_240M  = 7,
    HEVC_MatrixCoeff_YCgCo       = 8,
    HEVC_MatrixCoeff_BT2020      = 9,
    HEVC_MatrixCoeff_BT2020Cons  = 10
} HEVCMatrixCoeff;

typedef enum
{
    BUFFER_OWNER_COMPONENT      = 0,
    BUFFER_OWNER_DEC            = 1,
    BUFFER_OWNER_MJC            = 2
} BUFFER_OWNER;

#define FHD_AREA 1920*1080
#define HD_AREA 1280*736
#define SIZE_512K  512*1024

// adb shell property flags
#define MTK_OMX_VDEC_ENABLE_PRIORITY_ADJUSTMENT  (1 << 0)
#define MTK_OMX_VDEC_DUMP_BITSTREAM              (1 << 1)
#define MTK_OMX_VDEC_DUMP_OUTPUT                 (1 << 2)


#define MTK_MJC_VDO_BUFFER_QUEUE_MAX_NUM            32                                          // VDO queue buffer count
#define MTK_MJC_DISP_BUFFER_QUEUE_MAX_NUM           MTK_MJC_VDO_BUFFER_QUEUE_MAX_NUM            // DISP queue buffer count
#define MTK_MJC_REF_BUFFER_QUEUE_MAX_NUM            MTK_MJC_VDO_BUFFER_QUEUE_MAX_NUM            // MJC ref queue buffer count
#define MTK_MJC_PP_BUFFER_QUEUE_MAX_NUM             MTK_MJC_VDO_BUFFER_QUEUE_MAX_NUM            // MJC output queue buffer count


// HEVC
#define MTK_VDEC_HEVC_DEFAULT_INPUT_BUFFER_COUNT    8    // for Adaptive Playback = 21
#define MTK_VDEC_HEVC_DEFAULT_OUTPUT_BUFFER_COUNT   21
#define MTK_VDEC_HEVC_DEFAULT_INPUT_BUFFER_SIZE     1024*1024   // 400*1024
#define MTK_VDEC_HEVC_DEFAULT_INPUT_BUFFER_SIZE_4K  3.5*1024*1024
#define MTK_VDEC_HEVC_DEFAULT_OUTPUT_BUFFER_SIZE    38016
#define MTK_VDEC_HEVC_DEC_TIMESTAMP_ARRAY_SIZE      21

// H.263
#define MTK_VDEC_H263_DEFAULT_INPUT_BUFFER_COUNT    8    // for Adaptive Playback = 21
#define MTK_VDEC_H263_DEFAULT_OUTPUT_BUFFER_COUNT 4
#define MTK_VDEC_H263_DEFAULT_INPUT_BUFFER_SIZE     1.5*1024*1024   // 16000
#define MTK_VDEC_H263_DEFAULT_OUTPUT_BUFFER_SIZE  38016

// MPEG4
#define MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_COUNT   8    // for Adaptive Playback = 21
#define MTK_VDEC_MPEG4_DEFAULT_OUTPUT_BUFFER_COUNT 8 //4
#define MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_SIZE    2*1024*1024   // 16000
#define MTK_VDEC_MPEG4_DEFAULT_OUTPUT_BUFFER_SIZE  38016

// AVC
#define MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_COUNT     8    // for Adaptive Playback = 21
#define MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_COUNT_VILTE 5
#define MTK_VDEC_AVC_DEFAULT_OUTPUT_BUFFER_COUNT 21
#define MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE      1.6*1024*1024   // 400*1000
#define MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE_HD    1024*1024
#define MTK_VDEC_AVCSEC_DEFAULT_INPUT_BUFFER_SIZE    2*1024*1024
#define MTK_VDEC_AVCSEC_DEFAULT_INPUT_BUFFER_SIZE_4K    (2868*1024)
#define MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE_4K   (3.5*1024*1024)  // ALPS02863262: support file size up to 3.5MB
#define MTK_VDEC_AVC_DEFAULT_OUTPUT_BUFFER_SIZE  38016
#define MTK_VDEC_AVC_DEC_TIMESTAMP_ARRAY_SIZE 32

// RV
#define MTK_VDEC_RV_DEFAULT_INPUT_BUFFER_COUNT      4    // for Adaptive Playback = 21
#define MTK_VDEC_RV_DEFAULT_OUTPUT_BUFFER_COUNT 10
#define MTK_VDEC_RV_DEFAULT_INPUT_BUFFER_SIZE       1024*1024   // 16000
#define MTK_VDEC_RV_DEFAULT_OUTPUT_BUFFER_SIZE  38016

// VC1
#define MTK_VDEC_VC1_DEFAULT_INPUT_BUFFER_COUNT     8    // for Adaptive Playback = 21
#define MTK_VDEC_VC1_DEFAULT_OUTPUT_BUFFER_COUNT 8
#define MTK_VDEC_VC1_DEFAULT_INPUT_BUFFER_SIZE      1024*1024   // 16000
#define MTK_VDEC_VC1_DEFAULT_OUTPUT_BUFFER_SIZE  38016

// VPX
#define MTK_VDEC_VPX_DEFAULT_INPUT_BUFFER_COUNT     8    // for Adaptive Playback = 21
#define MTK_VDEC_VPX_DEFAULT_OUTPUT_BUFFER_COUNT 8
#define MTK_VDEC_VPX_DEFAULT_INPUT_BUFFER_SIZE   1024*1024
#define MTK_VDEC_VPX_DEFAULT_OUTPUT_BUFFER_SIZE  38016

// VP9
#define MTK_VDEC_VP9_DEFAULT_OUTPUT_BUFFER_COUNT 13 ////11+2 new codec need more two buffers
#define MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE_SW   2*1024*1024 //sw vp9 do not support 10 bit
#define MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE      4*1024*1024
#define MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE_4K    12*1024*1024 //12*1024*1024

// MPEG2
#define MTK_VDEC_MPEG2_DEFAULT_INPUT_BUFFER_COUNT   8    // for Adaptive Playback = 21
#define MTK_VDEC_MPEG2_DEFAULT_OUTPUT_BUFFER_COUNT 8
#define MTK_VDEC_MPEG2_DEFAULT_INPUT_BUFFER_SIZE    1024*1024
#define MTK_VDEC_MPEG2_DEFAULT_OUTPUT_BUFFER_SIZE  38016

// MJPEG
#define MTK_VDEC_MJPEG_DEFAULT_INPUT_BUFFER_COUNT   4    // for Adaptive Playback = 21
#define MTK_VDEC_MJPEG_DEFAULT_OUTPUT_BUFFER_COUNT  4
#define MTK_VDEC_MJPEG_DEFAULT_INPUT_BUFFER_SIZE    1024*1024
#define MTK_VDEC_MJPEG_DEFAULT_OUTPUT_BUFFER_SIZE   38016


#define MTK_VDEC_THUMBNAIL_OUTPUT_BUFFER_COUNT      3

#define REF_COUNT_ERROR_NOT_META_MODE               (-65535)
#define REF_COUNT_ERROR_NOT_EXIST                   (-65536)

#define VIDEO_HDR_COLOR_PRIMARIES_INVALID 0xFFFFFFFF

typedef struct  MTK_VDEC_PROFILE_MAP_ENTRY
{
    OMX_U32         profile;    // OMX_VIDEO_XXXPROFILETYPE
    VAL_UINT32_T    u4Profile;  // VDEC_DRV_XXX_VIDEO_PROFILE_T
} MTK_VDEC_PROFILE_MAP_ENTRY;

typedef struct  MTK_VDEC_LEVEL_MAP_ENTRY
{
    OMX_U32         level;      // OMX_VIDEO_XXXLEVELTYPE
    VAL_UINT32_T    u4Level;    // VDEC_DRV_VIDEO_LEVEL_T
} MTK_VDEC_LEVEL_MAP_ENTRY;

typedef struct BITBUF_STM
{
    OMX_U8 *StartAddr;
    OMX_U32 nSize;
    OMX_U32 nBitCnt;
    OMX_U32 nZeroCnt;
    OMX_U32 Cur32Bits;
    OMX_U32 CurBitCnt;
} BITBUF_STM;


#define CLEARMOTION_LIB_FULL_NAME           "/vendor/lib/libClearMotionFW.so"

#define MTK_MJC_FW_CREATEINSTANCE_NAME      "MJCCreateInstance"
#define MTK_MJC_FW_DESTROYINSTANCE_NAME     "MJCDestroyInstance"
#define MTK_MJC_FW_CREATE_NAME              "MJCCreate"
#define MTK_MJC_FW_INIT_NAME                "MJCInit"
#define MTK_MJC_FW_DEINIT_NAME              "MJCDeInit"
#define MTK_MJC_FW_SETPARAM_NAME            "MJCSetParameter"
#define MTK_MJC_FW_GETPARAM_NAME            "MJCGetParameter"


typedef MJC_ERRORTYPE (*MJCCreateInstance_ptr)(MJC**);
typedef MJC_ERRORTYPE (*MJCDestroyInstance_ptr)(MJC*);
typedef MJC_ERRORTYPE (*MJCFWCreate_ptr)(MJC*, MJC_USERHANDLETYPE);
typedef MJC_ERRORTYPE (*MJCFWInit_ptr)(MJC*, bool);
typedef MJC_ERRORTYPE (*MJCFWDeInit_ptr)(MJC*);
typedef MJC_ERRORTYPE (*MJCFWSetParameter_ptr)(MJC*, MJC_PARAMTYPE, MJC_PTR);
typedef MJC_ERRORTYPE (*MJCFWGetParameter_ptr)(MJC*, MJC_PARAMTYPE , MJC_PTR);


class MtkOmxVdec : public MtkOmxBase
{
    public:
        MtkOmxVdec();
        ~MtkOmxVdec();

        virtual OMX_ERRORTYPE ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN OMX_STRING componentName);

        virtual OMX_ERRORTYPE  ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent);

        virtual OMX_ERRORTYPE  GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
                                                   OMX_IN OMX_STRING componentName,
                                                   OMX_OUT OMX_VERSIONTYPE *componentVersion,
                                                   OMX_OUT OMX_VERSIONTYPE *specVersion,
                                                   OMX_OUT OMX_UUIDTYPE *componentUUID);

        virtual OMX_ERRORTYPE  SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
                                           OMX_IN OMX_COMMANDTYPE Cmd,
                                           OMX_IN OMX_U32 nParam1,
                                           OMX_IN OMX_PTR pCmdData);

        virtual OMX_ERRORTYPE  GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN  OMX_INDEXTYPE nParamIndex,
                                            OMX_INOUT OMX_PTR ComponentParameterStructure);

        virtual OMX_ERRORTYPE  SetParameter(OMX_IN OMX_HANDLETYPE hComp,
                                            OMX_IN OMX_INDEXTYPE nParamIndex,
                                            OMX_IN OMX_PTR pCompParam);

        virtual OMX_ERRORTYPE  GetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_IN OMX_INDEXTYPE nConfigIndex,
                                         OMX_INOUT OMX_PTR ComponentConfigStructure);

        virtual OMX_ERRORTYPE  SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_IN OMX_INDEXTYPE nConfigIndex,
                                         OMX_IN OMX_PTR ComponentConfigStructure);

        virtual OMX_ERRORTYPE GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
                                                OMX_IN OMX_STRING parameterName,
                                                OMX_OUT OMX_INDEXTYPE *pIndexType);

        virtual OMX_ERRORTYPE  GetState(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_INOUT OMX_STATETYPE *pState);

        virtual OMX_ERRORTYPE  UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                         OMX_IN OMX_U32 nPortIndex,
                                         OMX_IN OMX_PTR pAppPrivate,
                                         OMX_IN OMX_U32 nSizeBytes,
                                         OMX_IN OMX_U8 *pBuffer);


        virtual OMX_ERRORTYPE  AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                              OMX_INOUT OMX_BUFFERHEADERTYPE **pBuffHead,
                                              OMX_IN OMX_U32 nPortIndex,
                                              OMX_IN OMX_PTR pAppPrivate,
                                              OMX_IN OMX_U32 nSizeBytes);


        virtual OMX_ERRORTYPE  FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_IN OMX_U32 nPortIndex,
                                          OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead);


        virtual OMX_ERRORTYPE  EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                               OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead);


        virtual OMX_ERRORTYPE  FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                              OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead);

        virtual OMX_ERRORTYPE  SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN OMX_CALLBACKTYPE *pCallBacks,
                                            OMX_IN OMX_PTR pAppDat);

        virtual OMX_ERRORTYPE  ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
                                                 OMX_OUT OMX_U8 *cRole,
                                                 OMX_IN OMX_U32 nIndex);

        friend void *MtkOmxVdecThread(void *pData);
        friend void *MtkOmxVdecDecodeThread(void *pData);
        friend void *MtkOmxVdecConvertThread(void *pData);
        friend VAL_UINT32_T OMXGetOutputBufferCB(VAL_HANDLE_T hHandle, P_VDEC_DRV_FRAMEBUF_T *pFrame, VAL_UINT32_T u4Flag, VAL_BOOL_T bNewInterface, VAL_VOID_T *pExtra);

        friend void *MtkMJCThread(void *pData);
        friend OMX_BUFFERHEADERTYPE *MtkMJC_GetOutputFrame(OMX_IN OMX_HANDLETYPE hComponent);
        friend OMX_BUFFERHEADERTYPE *MtkMJC_GetInputFrame(OMX_IN OMX_HANDLETYPE hComponent);
        friend void MtkMJC_PutDispFrame(OMX_IN OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffHdr);
        friend void MtkMJC_SetRefFrame(OMX_IN OMX_HANDLETYPE hComponent, MJC_INT_BUF_INFO *pMJCBufInfo);
        friend void MtkMJC_ClearRefFrame(OMX_IN OMX_HANDLETYPE hComponent, MJC_INT_BUF_INFO *pBuffHdr);
        friend MJC_INT_BUF_INFO MtkMJC_GetBufInfo(OMX_IN OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffHdr);
        friend void *MtkMJC_GetBufGraphicHndl(OMX_IN OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffHdr);
        friend void *MtkMJC_GetBufVA(OMX_IN OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffHdr);
        friend void *MtkMJC_GetBufPA(OMX_IN OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffHdr);
        friend void MtkMJC_EventHandler(OMX_HANDLETYPE hComponent, const MJC_EVENT_TYPE eEventtype, void *param);


#ifdef DYNAMIC_PRIORITY_ADJUSTMENT
        void PriorityAdjustment();
#endif
        OMX_BOOL InitVideoDecodeHW(OMX_S32 *aWidth, OMX_S32 *aHeight, OMX_S32 *aAspectRatioWidth, OMX_S32 *aAspectRatioHeight, OMX_U8 *aBuffer, OMX_U32 *aSize);
        OMX_BOOL DeInitVideoDecodeHW();
        OMX_BOOL AllocateBitstreamBuffer(OMX_U8 *aInputBuf, OMX_U32 aInputSize);
        VDEC_DRV_FRAMEBUF_T *GetFrmBuf(OMX_TICKS iTimestamp, OMX_BUFFERHEADERTYPE *ipOutputBuffer);
        void InsertFrmBuf(OMX_BUFFERHEADERTYPE *ipOutputBuffer);
        void RemoveFrmBuf(OMX_BUFFERHEADERTYPE *ipOutputBuffer);
        OMX_BOOL PutFrmBuf(OMX_BUFFERHEADERTYPE *ipOutputBuffer);
        OMX_BUFFERHEADERTYPE *GetDisplayBuffer(OMX_BOOL bGetResolution = OMX_FALSE);
        OMX_BUFFERHEADERTYPE *GetFreeBuffer(OMX_BOOL *pbDisplay = NULL);
        OMX_U32 CheckFreeBuffer(OMX_BOOL bLOG);
        OMX_BOOL IsFreeBuffer(OMX_BUFFERHEADERTYPE *ipOutputBuffer);
        VDEC_DRV_RINGBUF_T *GetInputBuf(OMX_BUFFERHEADERTYPE *ipInputBuf);
        void InsertInputBuf(OMX_BUFFERHEADERTYPE *ipInputBuf);
        void RemoveInputBuf(OMX_BUFFERHEADERTYPE *ipInputBuf);
        void MarkInputBufferUnused(OMX_BUFFERHEADERTYPE *ipInputBuf);
        OMX_BUFFERHEADERTYPE *GetFreeInputBuffer(OMX_BOOL bFlushAll = OMX_FALSE);
        OMX_BOOL NotInETBQ(OMX_BUFFERHEADERTYPE *ipInputBuf);

        VDEC_DRV_VIDEO_FORMAT_T GetVdecFormat(MTK_VDEC_CODEC_ID codecId);
        OMX_BOOL DecodeVideo(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf);
        OMX_BOOL DecodeVideoEx(OMX_BUFFERHEADERTYPE *pInputBuf);

        // MTK_CLEARMOTION_SUPPORT, mBufFlag_EOS is for MJC Used to determine input/output buffer is OMX_BUFFERFLAG_EOS or not
        OMX_BOOL FlushDecoder(OMX_BOOL mBufFlag_EOS);

        // for H.264
        OMX_BOOL InsertionSortForInputPTS(OMX_TICKS timeTS);
        OMX_TICKS RemoveForInputPTS();
        OMX_BOOL RemoveForInputAtPTS(OMX_TICKS timeTS);

        OMX_BOOL QueryDriverFormat(VDEC_DRV_QUERY_VIDEO_FORMAT_T *pQinfoIn, VDEC_DRV_QUERY_VIDEO_FORMAT_T *pQinfoOut);


        OMX_BOOL SetupM4UPartialBsAndFrameBuffer(OMX_U8 *aInputBuf, OMX_U32 aInputSize);
        OMX_BOOL GetM4UFrameandBitstreamBuffer(VDEC_DRV_FRAMEBUF_T *aFrame, OMX_U8 *aInputBuf, OMX_U32 aInputSize, OMX_U8 *aOutputBuf);
        OMX_BOOL GetBitstreamBuffer(OMX_U8 *aInputBuf, OMX_U32 aInputSize);
        OMX_BOOL GetM4UFrame(VDEC_DRV_FRAMEBUF_T *aFrame, OMX_U8 *aOutputBuf);

        OMX_BOOL     ConfigIonBuffer(int ion_fd, int handle);
        VAL_UINT32_T GetIonPhysicalAddress(int ion_fd, int handle);

        friend class MJC;
        friend class MJCScaler;


#if (ANDROID_VER >= ANDROID_KK)
        OMX_BOOL GetMetaHandleFromOmxHeader(OMX_BUFFERHEADERTYPE *pBufHdr, OMX_U32 *pBufferHandle);
        OMX_BOOL GetMetaHandleFromBufferPtr(OMX_U8 *pBuffer, OMX_U32 *pBufferHandle);
        OMX_BOOL SetupMetaIonHandleAndGetFrame(VDEC_DRV_FRAMEBUF_T *aFrame, OMX_U8 *pBuffer);
        OMX_BOOL SetupMetaIonHandle(OMX_BUFFERHEADERTYPE *pBufHdr);
        OMX_BOOL SetupMetaSecureHandleAndGetFrame(VDEC_DRV_FRAMEBUF_T *aFrame, OMX_U8 *pBuffer); //HEVC.SEC.M0
        OMX_BOOL GetIonHandleFromGraphicHandle(OMX_U32 *pBufferHandle, int *pIonHandle);
        OMX_BOOL FreeIonHandle(int ionHandle);
#endif

    //secure meta mode
    OMX_BOOL SetupMetaSecureHandle(OMX_BUFFERHEADERTYPE *pBufHdr);

    private:

#if 1

        MJC* mpMJC;
        OMX_U32 TOTAL_MJC_BUFFER_CNT;
        OMX_BOOL mMJCEnable;

        void* mpMJCLib;

        MJCCreateInstance_ptr   m_fnMJCCreateInstance;
        MJCDestroyInstance_ptr  m_fnMJCDestroyInstance;
        MJCFWCreate_ptr         m_fnMJCCreate;
        MJCFWInit_ptr           m_fnMJCInit;
        MJCFWDeInit_ptr         m_fnMJCDeInit;
        MJCFWSetParameter_ptr   m_fnMJCSetParam;
        MJCFWGetParameter_ptr   m_fnMJCGetParam;

#endif

        void initInputPortCommon(OMX_PARAM_PORTDEFINITIONTYPE* pPortDef);
        void initOutputPortCommon(OMX_PARAM_PORTDEFINITIONTYPE* pPortDef);
        void initOutputPortFormatCommon(OMX_VIDEO_PARAM_PORTFORMATTYPE* pPortFormat);
        OMX_BOOL CheckTeeType();

        OMX_BOOL InitHEVCParams();
        OMX_BOOL InitHEVCSecParams(); //HEVC.SEC.M0
        OMX_BOOL InitH263Params();
        OMX_BOOL InitMpeg4Params();
        OMX_BOOL InitDivxParams();
        OMX_BOOL InitDivx3Params();
        OMX_BOOL InitXvidParams();
        OMX_BOOL InitS263Params();
        OMX_BOOL InitAvcParams();
        OMX_BOOL InitAvcSecParams();
        OMX_BOOL InitRvParams();
        OMX_BOOL InitVc1Params();
        OMX_BOOL InitVpxParams();
        OMX_BOOL InitVp9Params();
        OMX_BOOL InitVp9SecParams();
        OMX_BOOL InitMpeg2Params();
        OMX_BOOL InitMJpegParams();

        OMX_BOOL PortBuffersPopulated();
        OMX_BOOL FlushInputPort();
        OMX_BOOL FlushOutputPort();
        OMX_BOOL CheckBufferAvailability(); // check if we have at least one input buffer and one output buffer
        int DequeueInputBuffer();
        int DequeueOutputBuffer();
        int FindQueueOutputBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr);
        void QueueOutputBuffer(int index);
        void QueueInputBuffer(int index);    // queue input buffer to the head of the empty buffer list
        void CheckOutputBuffer();

        int DeQueueOutputColorConvertDstBuffer();
        int DeQueueOutputColorConvertSrcBuffer();
        void QueueOutputColorConvertDstBuffer(int index);
        void QueueOutputColorConvertSrcBuffer(int index);
        OMX_U32 CheckColorConvertBufferSize();
        int PrepareAvaliableColorConvertBuffer(int output_idx, OMX_BOOL direct_dq);
        OMX_ERRORTYPE HandleColorConvertForFillBufferDone(OMX_U32 index, OMX_BOOL fromDecodet);
        OMX_ERRORTYPE HandleColorConvertForFillBufferDone_1(OMX_U32 index, OMX_BOOL fromDecodet);

        OMX_ERRORTYPE HandleStateSet(OMX_U32 nNewState);
        OMX_ERRORTYPE HandlePortEnable(OMX_U32 nPortIndex);
        OMX_ERRORTYPE HandlePortDisable(OMX_U32 nPortIndex);
        OMX_ERRORTYPE HandlePortFlush(OMX_U32 nPortIndex);
        OMX_ERRORTYPE HandleMarkBuffer(OMX_U32 nParam1, OMX_PTR pCmdData);
        OMX_U64       GetInputBufferCheckSum(OMX_BUFFERHEADERTYPE *pBuffHdr);
        OMX_ERRORTYPE HandleEmptyThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr);
        OMX_ERRORTYPE HandleFillThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr);
        OMX_ERRORTYPE HandleEmptyBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr);
        // MTK_CLEARMOTION_SUPPORT, mRealCallBackFillBufferDone is for MJC Used, default is "OMX_TRUE"
        OMX_ERRORTYPE HandleFillBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr, OMX_BOOL mRealCallBackFillBufferDone);

        OMX_BOOL GrallocExtraSetBufParameter(buffer_handle_t _handle,
                                             VAL_UINT32_T gralloc_masks, VAL_UINT32_T gralloc_bits, OMX_TICKS nTimeStamp,
                                             VAL_BOOL_T bIsMJCOutputBuffer,VAL_BOOL_T bIsScalerOutputBuffer);
        OMX_BOOL HandleGrallocExtra(OMX_BUFFERHEADERTYPE *pBuffHdr);


        void MJCVdoBufQInsert(OMX_BUFFERHEADERTYPE *pBuffHdr);
        OMX_BUFFERHEADERTYPE *MJCVdoBufQRemove();

        void MJCDispBufQInsert(OMX_BUFFERHEADERTYPE *pBuffHdr);
        OMX_BUFFERHEADERTYPE *MJCDispBufQRemove();

        void MJCRefBufQInsert(MJC_INT_BUF_INFO *pMJCBufInfo);
        void MJCRefBufQRemove(MJC_INT_BUF_INFO *pMJCBufInfo);

        void MJCPPBufQInsert(OMX_BUFFERHEADERTYPE *pBuffHdr);
        OMX_BUFFERHEADERTYPE *MJCPPBufQRemove();

        void MJCScalerCheckandSetBufInfo();
        void MJCScalerByPass(OMX_BOOL mByPassFlag);
        void MJCEventHandler(const MJC_EVENT_TYPE eEventtype, void *param);
        void MJCSetBufFlagScalerFrame(OMX_BUFFERHEADERTYPE *pBuffHdr);
        OMX_BOOL MJCIsScalerBufandScalerEbable(OMX_BUFFERHEADERTYPE *pBuffHdr);


        OMX_BOOL MJCIsMJCOutBuf(OMX_BUFFERHEADERTYPE *pBuffHdr);

        OMX_BOOL MJCIsMJCRefBuf(OMX_BUFFERHEADERTYPE *pBuffHdr);

        void MJCHandleFillBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr);

        void MJCSetBufRes();
        void MJCSetBufFormat();

        void MJCSetBufFlagInterpolateFrame(OMX_BUFFERHEADERTYPE *pBuffHdr);

        MJC_INT_BUF_INFO MJCGetBufferInfo(OMX_BUFFERHEADERTYPE *pBuffHdr);
        void *MJCGetBufGraphicHndl(OMX_BUFFERHEADERTYPE *pBuffHdr);
        void *MJCGetBufVA(OMX_BUFFERHEADERTYPE *pBuffHdr);
        void *MJCGetBufPA(OMX_BUFFERHEADERTYPE *pBuffHdr);

        void MJCSetGrallocExtra(OMX_BUFFERHEADERTYPE *pBuffHdr, OMX_BOOL mIsMJCOutBuf);

        void MJCFlushAllBufQ();
        void MJCDrainVdoBufQ();

        void MJCCreateInstance();
        void MJCDestroyInstance();
        OMX_BOOL MJCLoadFWFunctions();


        void DISetGrallocExtra(OMX_BUFFERHEADERTYPE *pBuffHdr);

        void ReturnPendingInputBuffers();
        void ReturnPendingOutputBuffers();

        int findBufferHeaderIndex(OMX_U32 nPortIndex, OMX_BUFFERHEADERTYPE *pBuffHdr);

        OMX_BOOL SearchForVOLHeader(OMX_U8 *aInputBuf, OMX_U32 aInputSize);
        void printOmxColorFormat(OMX_COLOR_FORMATTYPE format);


        OMX_BOOL WaitFence(OMX_BUFFERHEADERTYPE *mBufHdrType, OMX_BOOL mWaitFence);

        OMX_BOOL IsH263()  { return (MTK_VDEC_CODEC_ID_H263 == mCodecId ? OMX_TRUE : OMX_FALSE);  }
        OMX_BOOL IsMPEG4()  { return ((MTK_VDEC_CODEC_ID_MPEG4 == mCodecId || MTK_VDEC_CODEC_ID_XVID == mCodecId) ? OMX_TRUE : OMX_FALSE);  }
        OMX_BOOL IsMPEG2()  { return (MTK_VDEC_CODEC_ID_MPEG2 == mCodecId ? OMX_TRUE : OMX_FALSE);  }
        OMX_BOOL IsVP8()  { return (MTK_VDEC_CODEC_ID_VPX == mCodecId ? OMX_TRUE : OMX_FALSE);  }
        OMX_BOOL IsVP9()  { return (MTK_VDEC_CODEC_ID_VP9 == mCodecId ? OMX_TRUE : OMX_FALSE);  }
        OMX_ERRORTYPE QureyVideoProfileLevel
        (
            VAL_UINT32_T                        u4VideoFormat,
            OMX_VIDEO_PARAM_PROFILELEVELTYPE    *pProfileLevel,
            MTK_VDEC_PROFILE_MAP_ENTRY          *pProfileMapTable,
            VAL_UINT32_T                        nProfileMapTableSize,
            MTK_VDEC_LEVEL_MAP_ENTRY            *pLevelMapTable,
            VAL_UINT32_T                        nLevelMapTableSize
        );

        OMX_ERRORTYPE QueryDivxLicense(OMX_VIDEO_PARAM_PROFILELEVELTYPE   *pProfileLevel);
        //OMX_BOOL HandleAssemblePartialFrame(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf);
        OMX_BOOL HandleAssemblePartialFrame(OMX_BUFFERHEADERTYPE *pInputBuf);

        OMX_VIDEO_PARAM_PORTFORMATTYPE mInputPortFormat;
        OMX_VIDEO_PARAM_PORTFORMATTYPE mOutputPortFormat;

        OMX_VIDEO_PARAM_RVTYPE mRvType;

        int mCmdPipe[2];      // commands from IL client to component

        pthread_mutex_t mCmdQLock;

        pthread_t mVdecThread;
        pthread_t mVdecDecodeThread;

        bool mVdecThreadCreated;
        bool mVdecDecodeThreadCreated;


        // MTK_CLEARMOTION_SUPPORT, mRealCallBackFillBufferDone is for MJC Used, default is "OMX_TRUE"
        OMX_BOOL mRealCallBackFillBufferDone;


        VAL_UINT32_T mVdecThreadTid;
        VAL_UINT32_T mVdecDecThreadTid;
        Mutex mFlushOrConvertLock;

        VAL_UINT32_T mFramesDecoded;
        VAL_UINT32_T mTotalDecodeTime;

        unsigned int mPendingStatus;
        OMX_BOOL mDecodeStarted;
        OMX_BOOL mIsComponentAlive;

        pthread_mutex_t mEmptyThisBufQLock;
        pthread_mutex_t mFillThisBufQLock;

        pthread_mutex_t mDecodeLock;
        pthread_mutex_t mWaitDecSemLock;
        pthread_mutex_t mFillUsedLock;

        pthread_mutex_t mMJCVdoBufQLock;
        pthread_mutex_t mMJCDispBufQLock;
        pthread_mutex_t mMJCRefBufQLock;
        pthread_mutex_t mMJCPPBufQLock;
        pthread_mutex_t mMJCFlushBufQLock;


        // for UseBuffer/AllocateBuffer
        sem_t mInPortAllocDoneSem;
        sem_t mOutPortAllocDoneSem;

        // for FreeBuffer
        sem_t mInPortFreeDoneSem;
        sem_t mOutPortFreeDoneSem;

        sem_t mDecodeSem;
        sem_t mOutputBufferSem;


        sem_t mPPBufQManagementSem;
        sem_t mMJCFlushBufQDoneSem;
        sem_t mMJCVdoBufQDrainedSem;


        sem_t mConvertSem;

        void *mProducer;

        OMX_U32  mNumPendingInput;
        OMX_U32  mNumPendingOutput;

        OMX_U32  mErrorInDecoding;

        MTK_VDEC_CODEC_ID mCodecId;

        int mCurrentSchedPolicy;

        OMX_U32 mPropFlags;
        OMX_U32 mForceOutputBufferCount;
        OMX_VIDEO_H264FPATYPE m3DStereoMode;


        OMX_BOOL mbH263InMPEG4;
#ifdef DYNAMIC_PRIORITY_ADJUSTMENT
        OMX_U32 mErrorCount;
        OMX_U32 mPendingOutputThreshold;
        OMX_TICKS mTimeThreshold;
#endif
#if defined(DYNAMIC_PRIORITY_ADJUSTMENT)
        OMX_TICKS mllLastDispTime; // include NOT_DISPLAY
#endif

        OMX_TICKS mllLastUpdateTime;
        typedef struct _FrmBufStruct
        {
            OMX_BOOL                bUsed;
            OMX_BOOL                bDisplay;
            OMX_BOOL                bNonRealDisplay;
            OMX_BOOL                bFillThis;
            VDEC_DRV_FRAMEBUF_T     frame_buffer;
            OMX_TICKS               iTimestamp;
            OMX_BUFFERHEADERTYPE    *ipOutputBuffer;
#if (ANDROID_VER >= ANDROID_KK)
            OMX_U32 bGraphicBufHandle;
            int ionBufHandle;
            int refCount;
#endif
        } FrmBufStruct;
        typedef struct _InputBufStruct
        {
            OMX_BOOL                bUsed;
            VDEC_DRV_RINGBUF_T      InputBuf;
            OMX_BUFFERHEADERTYPE    *ipInputBuffer;
        } InputBufStruct;


        OMX_BOOL mDecoderInitCompleteFlag;
        VAL_HANDLE_T mDrvHandle;
        VDEC_DRV_RINGBUF_T  mRingbuf;
        VDEC_DRV_RINGBUF_T  mPrevRingbuf;
        VDEC_DRV_SEQINFO_T  mSeqInfo;
        VAL_UINT32_T mDPBSize;
        OMX_BOOL mSeqInfoCompleteFlag;
        OMX_BOOL mMJCReconfigFlag;
        FrmBufStruct *mFrameBuf;
        FrmBufStruct *mFrameBufInt;
        InputBufStruct *mInputBuf;
        OMX_U32 mBitstreamBufferSize;
        OMX_BOOL mBitStreamBufferAllocated;
        VAL_UINT32_T mBitStreamBufferVa;
        VAL_UINT32_T mBitStreamBufferPa;

        // for speedy mode
        OMX_U32  mNumFreeAvailOutput;
        OMX_U32  mNumAllDispAvailOutput;
        OMX_U32  mNumNotDispAvailOutput;

        // for video subsample
        OMX_S32 mAspectRatioWidth;
        OMX_S32 mAspectRatioHeight;

        // for input semaphore
        OMX_S32  mNumSemaphoreCountForInput;
        OMX_S32  mNumSemaphoreCountForOutput;

        VAL_UINT32_T mScalerBufferCount;
        MJC_BUF_INFO mScalerBufferInfo[MTK_MJC_REF_VDEC_OUTPUT_BUFFER_COUNT];
        OMX_BOOL mMJCScalerByPassFlag;
        VAL_UINT32_T mOriFrameWidth;
        VAL_UINT32_T mOriFrameHeight;


        VAL_UINT32_T mMJCBufferCount;
        MJC_BUF_INFO mMJCBufferInfo[MTK_MJC_DEFAULT_OUTPUT_BUFFER_COUNT];
        OMX_BUFFERHEADERTYPE *mMJCVdoBufQ[MTK_MJC_VDO_BUFFER_QUEUE_MAX_NUM];
        VAL_UINT32_T mMJCVdoBufCount;
        OMX_BUFFERHEADERTYPE *mMJCDispBufQ[MTK_MJC_DISP_BUFFER_QUEUE_MAX_NUM];
        VAL_UINT32_T mMJCRefBufQ[MTK_MJC_REF_BUFFER_QUEUE_MAX_NUM];
        VAL_UINT32_T mMJCRefBufCount;
        OMX_BUFFERHEADERTYPE *mMJCPPBufQ[MTK_MJC_PP_BUFFER_QUEUE_MAX_NUM];
        VAL_UINT32_T mMJCPPBufCount;
        VAL_BOOL_T mMJCLog;
        VAL_BOOL_T mMJCFlushBufQ;
        VAL_BOOL_T mMJCCreated;


        VAL_BOOL_T mInterlaceChkComplete;
        VAL_BOOL_T mIsInterlacing;

        OMX_BOOL mInputAllocateBuffer;
        OMX_BOOL mOutputAllocateBuffer;

#if (ANDROID_VER >= ANDROID_KK)
        VAL_BOOL_T mAdaptivePlayback;
#endif

        VAL_BOOL_T mDeInterlaceEnable;

        VAL_UINT32_T mOutputBufferHdrsCnt;

        OmxMVAManager   *mInputMVAMgr;
        OmxMVAManager   *mOutputMVAMgr;

        VAL_VOID_T  *mM4UBufferHandle;
        VAL_UINT32_T mM4UBufferCount;

        VAL_UINT32_T   mIonInputBufferCount;
        VAL_UINT32_T   mIonOutputBufferCount;

        VBufInfo mBufInfo;

        OMX_BOOL mInputUseION;
        OMX_BOOL mOutputUseION;

        OMX_BOOL mIsClientLocally;
        OMX_BOOL mIsFromGralloc;
        int mIonDevFd;

        VAL_UINT32_T   mSecFrmBufCount;
        VdecSecFrmBufInfo mSecFrmBufInfo[VIDEO_ION_MAX_BUFFER];
        VAL_UINT32_T   mSecInputBufCount;
        VdecSecInputBufInfo mSecInputBufInfo[VIDEO_ION_MAX_BUFFER];

        OMX_S32 mOutputStrideBeforeReconfig;
        OMX_U32 mOutputSliceHeightBeforeReconfig;

        // for VC1
        OMX_TICKS mFrameTsInterval;
        OMX_TICKS mCurrentFrameTs;
        OMX_BOOL  mFirstFrameRetrieved;
        OMX_BOOL  mResetFirstFrameTs;
        OMX_BOOL  mCorrectTsFromOMX;

        // for H.264
        OMX_TICKS DisplayTSArray[MTK_VDEC_AVC_DEC_TIMESTAMP_ARRAY_SIZE];
        OMX_S32 iTSIn;
        OMX_BOOL mIs4kSwAvc;
        // for HEVC
        VAL_UINT32_T mInitRetryNum;
        VAL_UINT32_T mReconfigOutputPortBufferCount;
        VAL_UINT32_T mReconfigOutputPortBufferSize;
        //for VP9
        OMX_BOOL mIsVp9Sw4k;
        // for output dump
        OMX_BOOL  mDumpOutputFrame;

        // for output dump
        OMX_BOOL  mDumpOutputProfling;
        // for cts debug
        OMX_BOOL  mDumpCts;

        // for HDR video
        VDEC_DRV_COLORDESC_T mColorDesc;

        //for output checksum
        OMX_BOOL  mOutputChecksum;
#if (ANDROID_VER >= ANDROID_ICS)
        OMX_BOOL mIsUsingNativeBuffers;
#endif

#if (ANDROID_VER >= ANDROID_KK)
        OMX_BOOL mStoreMetaDataInBuffers;
        OMX_BOOL mEnableAdaptivePlayback;
        OMX_U32    mMaxWidth;
        OMX_U32    mMaxHeight;
        OMX_U32    mCropLeft;
        OMX_U32    mCropTop;
        OMX_U32    mCropWidth;
        OMX_U32    mCropHeight;
        OMX_BOOL mEarlyEOS;
#endif
        OMX_BOOL mIsResChg;


        // for UI response improvement
        OMX_U32   mRRSlidingWindowLength;
        OMX_U32   mRRSlidingWindowCnt;
        OMX_U32   mRRSlidingWindowLimit;
        OMX_U32   mRRCntCurWindow;
        unsigned long long mLastCpuIdleTime;
        unsigned long long mLastSchedClock;

        OMX_BOOL mNeedMoreOutputBuf;

        void DumpETBQ();
        void DumpFTBQ();

#if CPP_STL_SUPPORT
        vector<int> mEmptyThisBufQ;
        vector<int> mFillThisBufQ;
#endif

#if ANDROID
        Vector<int> mEmptyThisBufQ;
        Vector<int> mFillThisBufQ;
#endif

        Vector<int> mBufColorConvertSrcQ;
        Vector<int> mBufColorConvertDstQ;
        pthread_mutex_t mFillThisConvertBufQLock;

        OMX_U32 FNum; // for CRC check

        OMX_BOOL mUseClearMotion;
        VAL_UINT32_T mClearMotionDemoMode;
        VAL_UINT32_T mMinUndequeuedBufs;
        OMX_BOOL mMinUndequeuedBufsFlag;
        OMX_S32 mMinUndequeuedBufsDiff;
        OMX_U32 mStarvationSize;

        // for seek and thumbnail
        OMX_BOOL mThumbnailMode;
        OMX_TICKS mSeekTargetTime;
        OMX_BOOL mSeekMode;
        OMX_BOOL mPrepareSeek;

        OMX_S32 mConcealLevel;
        OMX_BOOL mEOSFound;
        OMX_TICKS mEOSTS;
        OMX_BOOL mFATALError;
        OMX_BOOL mKernelLogCntChg;
        VAL_UINT32_T mFailInitCounter;
        OMX_BOOL mStreamingMode;
        OMX_BOOL mInputZero;
        //MTK_PLATFORM_PRIVATE mMTKOmxVdecPlatformPrivate;

        OMX_BOOL mCodecTidInitialized;
        OMX_U32 mNumCodecThreads;
        pid_t mCodecTids[32];


        OMX_TICKS mFrameInterval;

        OMX_BOOL mIsSecureInst;
        OMX_BOOL mbShareableMemoryEnabled;
        OMX_BOOL mIsSecUsingNativeHandle;
        OMX_BOOL mIsSecTlcAllocOutput;
        enum teeType
        {
            NONE_TEE = 0,
            TRUSTONIC_TEE = 1,
            INHOUSE_TEE = 2,
            BLOWFISH_TEE = 3,
            MICROTRUST_TEE = 4,
        };
        int mTeeType;

#if 0
        void *mGlobalInstData;
#endif
        OMX_U32 mChipName;

        OMX_BOOL mNoReorderMode;

        VDEC_DRV_DECODER_TYPE_T meDecodeType;

        OMX_S32 mConvertYV12;
        OMX_BOOL mLegacyMode;
        OMX_U32 mACodecColorConvertMode;
        OMX_BOOL mSkipReferenceCheckMode;
        OMX_BOOL mLowLatencyDecodeMode;

        OMX_BOOL mInputBuffInuse;
        OMX_BOOL mFlushDecoderDoneInPortSettingChange;

        //for supporting SVP NAL size prefix content
        OMX_BOOL mAssignNALSizeLength;
        OMX_VIDEO_CONFIG_NALSIZE mNALSizeLengthInfo;

        void *mH264SecVdecTlcLib;
        void *mH265SecVdecTlcLib;
        void *mVP9SecVdecTlcLib;
        void *mTlcHandle;

        void *mCommonVdecInHouseLib;
        void FdDebugDump();

        OMX_U32 GetBufferCheckSum(char* u4VA, OMX_U32 u4Length);
        OMX_BOOL ConvertFrameToYUV420(FrmBufStruct *pFrameBuf, FrmBufStruct *pFrameBufOut, OMX_BOOL bGetResolution);

        OMX_BOOL DescribeFlexibleColorFormat(DescribeColorFormatParams *params);

        OMX_U32 AllocateIonBuffer(int IonFd, OMX_U32 Size, VdecIonBufInfo *IonBufInfo);
        char *GetVDECSrting(MTK_VDEC_CODEC_ID mCodecId, void *pBuffer);
        char mCodecSTR[30];


        OMX_U64 mAVSyncTime;

        // for Log Reduction
        VAL_UINT32_T mGET_DISP_i;
        VAL_UINT32_T mGET_DISP_tmp_frame_addr;

        OMX_BOOL mb16xSlowMotionMode;

        OMX_BOOL mbYUV420FlexibleMode;

        VAL_BOOL_T mbIs10Bit;  // 10 bit video
        VAL_BOOL_T mIsHorizontalScaninLSB;  //10 bit LSB scan

        bool mResetCurrTime;
        int mFrameBufSize;

        pthread_t mVdecConvertThread;
        VAL_UINT32_T mVdecConvertThreadTid;
        bool mVdecConvertThreadCreated;

        VDEC_DRV_QUERY_VIDEO_FORMAT_T  mQInfoIn;
        VDEC_DRV_QUERY_VIDEO_FORMAT_T  mQInfoOut;
        OMX_U32 mCodecProperty;
        OMX_BOOL mInputFlushALL;
        OMX_BOOL mEverCallback;
        OMX_BUFFERHEADERTYPE *mpCurrInput;
        int64_t mCBTime;

        bool mFullSpeedOn;

        bool mCrossMountSupportOn;
        OMX_U32 mMaxColorConvertOutputBufferCnt;
        OMX_BOOL mANW_HWComposer;
        OMX_BOOL IsGPUSupportBlock();

        int IncRef(int owner, VAL_ULONG_T va, bool isSecure);
        int DecRef(int owner, VAL_ULONG_T va, bool isSecure);
        int ProtectedIncRef(int owner, VAL_ULONG_T va, bool isSecure);
        int ProtectedDecRef(int owner, VAL_ULONG_T va, bool isSecure);

        OMX_BOOL HandleViLTEInitHWErr(void);
        void HandleViLTEBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr);
        OMX_BOOL IsAVPFEnabled(void);

        OMX_BOOL IsHDRSetByFramework();
        OMX_BOOL CopyHDRColorDesc();

        VDEC_DRV_ERROR_MB_INFO_T mrErrorMBInfo;
        OMX_CONFIG_SLICE_LOSS_INDICATION mSLI;
        OMX_BOOL mViLTESupportOn;
        OMX_BOOL mHDRVideoSupportOn;
        OMX_BOOL mIsHDRVideo;
        OMX_BOOL mNativeWindowHDRInternalConvert;

        OMX_U32  mContinuousSliceLoss;
        OMX_BOOL mFIRSent;
        OMX_BOOL mAVPFEnable; //ViLTE Audio Video Profile for rtcp Feedback
        OMX_U32 mPLIThres; // picture loss threshold
        OMX_U32 mResendPLIThres; // PLI resend threshold

        OMX_BOOL mOmxVdecLogEnable = OMX_FALSE;
        OMX_BOOL mOmxVdecPerfLogEnable = OMX_FALSE;

        void UpdateVideoInfo(OMX_TICKS Timestamp);
        OMX_TICKS mFirstTimestamp;
        OMX_U32   mFPSProfilingCountDown;

        OMX_BOOL needLegacyMode(void);
        OMX_BOOL needColorConvertWithNativeWindow(void);
        OMX_BOOL needColorConvertWithMetaMode(void);
        OMX_BOOL needColorConvertWithoutMetaMode(void);
        bool supportAutoEnlarge(void);

        OMX_BOOL mMJCFakeEngine;
        OMX_BOOL mSucessfullyRegisterMJC;
        void HandleVendorMtkOmxVdecUseClearMotion(OMX_BOOL mForceAllocateBuffer);

        DescribeColorAspectsParams mDescribeColorAspectsParams;
        DescribeHDRStaticInfoParams mDescribeHDRStaticInfoParams;
        OMX_BOOL EnableCodecDriverUFO(void);

        bool mNeedDecodeWhenFlush;
        bool mRestorePortReconfigFlag;
        OMX_BOOL mEnableAVTaskGroup;

        OMX_BOOL CheckLogEnable();
        void SetInputPortViLTE();

        OMX_U32 mPicWidth;
        OMX_U32 mPicHeight;

        OMX_BOOL dumpBuffer(char *name, unsigned char *data, int size);
        OMX_BOOL dumpInputBuffer(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_U8 *aInputBuf, OMX_U32 aInputSize);
        void UpdateColorAspectsParams();
        void HandleColorAspectChange(ColorAspects defaultAspects);
        OMX_BOOL ColorAspectsDiffer(ColorAspects a, ColorAspects b);
};


#endif

