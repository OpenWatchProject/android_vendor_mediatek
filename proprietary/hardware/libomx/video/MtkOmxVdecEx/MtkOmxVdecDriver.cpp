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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   MtkOmxVdecDriver.cpp
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX Video Decoder component
 *
 * Author:
 * -------
 *   Morris Yang (mtk03147)
 *
 ****************************************************************************/

#include "MtkOmxVdecEx.h"

#if 1
#include <linux/svp_region.h>
#endif


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
extern int64_t getTickCountMs();

#if (ANDROID_VER >= ANDROID_ICS)
#include <android/native_window.h>
#include <HardwareAPI.h>
//#include <gralloc_priv.h>
#include <ui/GraphicBufferMapper.h>
#include <ui/gralloc_extra.h>
#include <ion.h>
#include "graphics_mtk_defs.h"
#include <poll.h>
#endif

const VAL_UINT32_T crc32_table[256] =
{
    0x0,        0x4C11DB7,  0x9823B6E,  0xD4326D9,  0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039, 0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x18AEB13,  0x54BF6A4,  0x808D07D,  0xCC9CDCA,
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
    0x315D626,  0x7D4CB91,  0xA97ED48,  0xE56F0FF,  0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30, 0x29F3D35,  0x65E2082,  0xB1D065B,  0xFDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};  // Table of 8-bit remainders

unsigned int _calculate_crc32(unsigned char *input_buffer, unsigned int size)
{
    unsigned int crc_accum = 0;

    while (size-- > 0)
    {
        crc_accum = (crc_accum << 8) ^
                    crc32_table[(unsigned char)(crc_accum >> 24) ^ (*input_buffer++)];
    }

    return crc_accum;
}

// CALLBACK FUNCTION
VAL_UINT32_T OMXGetOutputBufferCB(VAL_HANDLE_T hHandle, P_VDEC_DRV_FRAMEBUF_T *pFrame, VAL_UINT32_T u4Flag, VAL_BOOL_T bNewInterface, VAL_VOID_T *pExtra)
{
    int64_t _in_time_cb_1 = 0;
    int64_t _in_time_cb_2 = 0;
    unsigned int YStride;
    unsigned int CStride;
    unsigned int SliceHeight;

    _in_time_cb_1 = getTickCountMs();

    OMX_BOOL bFound;
    MtkOmxVdec *pVdec = (MtkOmxVdec *)hHandle;
#if 1

    if (hHandle == NULL || pVdec->mInputFlushALL == OMX_TRUE || pVdec->mRestorePortReconfigFlag == OMX_TRUE)
    {
        if (pVdec->mOmxVdecPerfLogEnable) ALOGD("mInputFlushALL is TRUE. 1");
        *pFrame = NULL;
        _in_time_cb_2 = getTickCountMs();
        pVdec->mCBTime = _in_time_cb_2 - _in_time_cb_1;
        return -1;
    }

    //
    //ALOGD("## 0x%08x Wait for output buffer (%d)", pVdec, get_sem_value(&pVdec->mOutputBufferSem));
    WAIT(pVdec->mOutputBufferSem);

    if (hHandle == NULL || pVdec->mInputFlushALL == OMX_TRUE)
    {
        if (pVdec->mOmxVdecPerfLogEnable) ALOGD("mInputFlushALL is TRUE. 2");
        *pFrame = NULL;
        _in_time_cb_2 = getTickCountMs();
        pVdec->mCBTime = _in_time_cb_2 - _in_time_cb_1;
        return -1;
    }

    int output_idx = pVdec->DequeueOutputBuffer();

    if (output_idx >= 0)
    {
        pVdec->mEverCallback = OMX_TRUE;

        pVdec->mOutputBufferHdrs[output_idx]->nTimeStamp = pVdec->mpCurrInput->nTimeStamp;
        pVdec->mOutputBufferHdrs[output_idx]->nFilledLen = pVdec->mOutputPortDef.format.video.nStride * pVdec->mOutputPortDef.format.video.nSliceHeight * 3 >> 1;

        if (pVdec->mOutputPortDef.format.video.eColorFormat == (OMX_COLOR_FORMATTYPE)OMX_MTK_COLOR_FormatYV12)
        {
            YStride = pVdec->mOutputPortDef.format.video.nStride;
            CStride = VDEC_ROUND_N(YStride/2,16);
            SliceHeight = pVdec->mOutputPortDef.format.video.nSliceHeight;
            pVdec->mOutputBufferHdrs[output_idx]->nFilledLen =  YStride*SliceHeight + CStride*SliceHeight;
        }

        if (pVdec->mbIs10Bit == OMX_TRUE) {
            pVdec->mOutputBufferHdrs[output_idx]->nFilledLen = pVdec->mOutputBufferHdrs[output_idx]->nFilledLen * 5 / 4;
        }

        if (pVdec->mDeInterlaceEnable)
        {
            pVdec->DISetGrallocExtra(pVdec->mOutputBufferHdrs[output_idx]);
        }

        VDEC_DRV_FRAMEBUF_T *pFrameBuf = (VDEC_DRV_FRAMEBUF_T*)malloc(sizeof(VDEC_DRV_FRAMEBUF_T));
        if (NULL == pFrameBuf)
        {
            ALOGE("[ERROR] Allocate frame buffer structure failed, line %d", __LINE__);
            *pFrame = NULL;
            _in_time_cb_2 = getTickCountMs();
            pVdec->mCBTime = _in_time_cb_2 - _in_time_cb_1;
            return -1;
        }
        P_VDEC_DRV_FRAMEBUF_T pTempBuf = NULL;
        memset(pFrameBuf, 0, sizeof(*pFrameBuf));
        pTempBuf = pVdec->GetFrmBuf(0, pVdec->mOutputBufferHdrs[output_idx]);
        if(pTempBuf == NULL)
        {
            free(pFrameBuf);
            *pFrame = NULL;
            _in_time_cb_2 = getTickCountMs();
            pVdec->mCBTime = _in_time_cb_2 - _in_time_cb_1;
            return -1;
        }
        //ALOGD("pFrame : 0x%08x (VA: 0x%08x, PA: 0x%08x, 0x%08x)", *pFrame, (*pFrame)->rBaseAddr.u4VA, (*pFrame)->rBaseAddr.u4PA, pVdec->mOutputBufferHdrs[output_idx]->pBuffer);
        pVdec->GetM4UFrame(pTempBuf, pVdec->mOutputBufferHdrs[output_idx]->pBuffer);
        memcpy(pFrameBuf, pTempBuf, sizeof(*pFrameBuf));
        //ALOGE("[TEST] Idx = %d, OutputBuf = 0x%x, VA = 0x%x, MVA = 0x%x", output_idx, pFrameBuf, pFrameBuf->rBaseAddr.u4VA, pFrameBuf->rBaseAddr.u4PA);
        *pFrame = pFrameBuf;

        if ((*pFrame)->rBaseAddr.u4VA == NULL || (*pFrame)->rBaseAddr.u4PA == NULL) {
            ALOGE("[Warning] buffer addr is NULL VA: 0x%08x, PA: 0x%08x", (*pFrame)->rBaseAddr.u4VA, (*pFrame)->rBaseAddr.u4PA);
        }

        if (pVdec->mOmxVdecPerfLogEnable)
            ALOGD("[0x%08x] pFrame : 0x%08x (0x%08X) (0x%08X) (VA: 0x%08x, PA: 0x%08x)", pVdec, *pFrame, pVdec->mOutputBufferHdrs[output_idx], pVdec->mOutputBufferHdrs[output_idx]->pBuffer, (*pFrame)->rBaseAddr.u4VA, (*pFrame)->rBaseAddr.u4PA);

        _in_time_cb_2 = getTickCountMs();
        pVdec->mCBTime = _in_time_cb_2 - _in_time_cb_1;
        return 0;
    }
    ALOGE("should not be here, output buffer should be available");
#endif
    *pFrame = NULL;
    _in_time_cb_2 = getTickCountMs();
    pVdec->mCBTime = _in_time_cb_2 - _in_time_cb_1;
    return -1;
}

OMX_BOOL MtkOmxVdec::CheckTeeType()
{
    // check secure environment
    int svp = 0;
    char value1[PROPERTY_VALUE_MAX];
    char value2[PROPERTY_VALUE_MAX];

    property_get("ro.vendor.mtk_sec_video_path_support", value1, "0");
    svp = atoi(value1);

    if (svp)
    {
        mIsSecureInst = OMX_TRUE;
        property_get("ro.vendor.mtk_trustonic_tee_support", value1, "0");

        if (atoi(value1))
        {
            mTeeType = TRUSTONIC_TEE;
        }
        else
        {
            property_get("ro.vendor.mtk_blowfish_tee_support", value1, "0");
            property_get("ro.vendor.mtk_microtrust_tee_support", value2, "0");
            if (atoi(value1))
            {
                mTeeType = BLOWFISH_TEE;
            }
            else if(atoi(value2))
            {
                mTeeType = MICROTRUST_TEE;
            }
            else //in house tee
            {
                mTeeType = INHOUSE_TEE;
            }
        }
    }

    if (mTeeType == NONE_TEE)
    {
        MTK_OMX_LOGE("[ERROR] Check Tee Type Failed !!");
        return OMX_FALSE;
    }

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::InitHEVCSecParams() //HEVC.SEC.M0
{
    // check secure environment
    if(OMX_FALSE == CheckTeeType())
    {
        return OMX_FALSE;
    }

    MTK_OMX_LOGD("InitHEVCSecParams, Tee Type = %d", mTeeType);

	// init input port format
	strncpy((char*)mCompRole, "video_decoder.hevc", sizeof(mCompRole));
	mInputPortFormat.nPortIndex 		= MTK_OMX_INPUT_PORT;
	mInputPortFormat.nIndex 			= 0;
	mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingHEVC;

	// init output port format
	mOutputPortFormat.nPortIndex			= MTK_OMX_OUTPUT_PORT;
	mOutputPortFormat.nIndex				= 0;
	mOutputPortFormat.eCompressionFormat	= OMX_VIDEO_CodingUnused;
	mOutputPortFormat.eColorFormat			= OMX_COLOR_FormatYUV420Planar;

	// init input port definition
	mInputPortDef.nPortIndex							= MTK_OMX_INPUT_PORT;
	mInputPortDef.eDir									= OMX_DirInput;
	mInputPortDef.eDomain								= OMX_PortDomainVideo;
	mInputPortDef.format.video.pNativeRender			= NULL;
	mInputPortDef.format.video.nFrameWidth				= 176;
	mInputPortDef.format.video.nFrameHeight 			= 144;
	mInputPortDef.format.video.nStride					= 176;
	mInputPortDef.format.video.nSliceHeight 			= 144;
	mInputPortDef.format.video.nBitrate 				= 64000;
	mInputPortDef.format.video.xFramerate				= (15 << 16);
	mInputPortDef.format.video.cMIMEType				= (OMX_STRING)"video/hevc";
	mInputPortDef.format.video.bFlagErrorConcealment	= OMX_FALSE;
	mInputPortDef.format.video.eCompressionFormat		= OMX_VIDEO_CodingHEVC;
	mInputPortDef.format.video.eColorFormat 			= OMX_COLOR_FormatUnused;

	mInputPortDef.nBufferCountActual	= MTK_VDEC_HEVCSEC_DEFAULT_INPUT_BUFFER_COUNT;
	mInputPortDef.nBufferCountMin		= 1;
	mInputPortDef.nBufferSize			= MTK_VDEC_HEVCSEC_DEFAULT_INPUT_BUFFER_SIZE;
	mInputPortDef.bEnabled				= OMX_TRUE;
	mInputPortDef.bPopulated			= OMX_FALSE;

	// init output port definition
	mOutputPortDef.nPortIndex							= MTK_OMX_OUTPUT_PORT;
	mOutputPortDef.eDomain								= OMX_PortDomainVideo;
	mOutputPortDef.format.video.cMIMEType				= (OMX_STRING)"raw";
	mOutputPortDef.format.video.pNativeRender			= 0;
	mOutputPortDef.format.video.bFlagErrorConcealment	= OMX_FALSE;
	mOutputPortDef.format.video.eCompressionFormat		= OMX_VIDEO_CodingUnused;
	mOutputPortDef.format.video.eColorFormat			= OMX_COLOR_FormatYUV420Planar;
	mOutputPortDef.format.video.nFrameWidth 			= 176; //320; //176;
	mOutputPortDef.format.video.nStride 				= 176;
	mOutputPortDef.format.video.nFrameHeight			= 144; //240; //144;
	mOutputPortDef.format.video.nSliceHeight			= 144;
	mOutputPortDef.format.video.nBitrate				= 64000;
	mOutputPortDef.format.video.xFramerate				= (15 << 16);
	mOutputPortDef.eDir 								= OMX_DirOutput;

	mOutputPortDef.nBufferCountActual = (mForceOutputBufferCount == 0 ? MTK_VDEC_HEVC_DEFAULT_OUTPUT_BUFFER_COUNT : mForceOutputBufferCount);
	mOutputPortDef.nBufferCountMin = 1;
	mOutputPortDef.nBufferSize = MTK_VDEC_HEVC_DEFAULT_OUTPUT_BUFFER_SIZE + 16;
	mOutputPortDef.bEnabled = OMX_TRUE;
	mOutputPortDef.bPopulated = OMX_FALSE;

    if (INHOUSE_TEE == mTeeType)
    {
       mCommonVdecInHouseLib  = dlopen(MTK_COMMON_SEC_VDEC_IN_HOUSE_LIB_NAME, RTLD_NOW);
       if (NULL == mCommonVdecInHouseLib)
       {
           MTK_OMX_LOGE("lib_uree_mtk_video_secure_al open failed: [%s]", dlerror());
           return OMX_FALSE;
       }
       //MtkVideoSecureMemAllocatorInit_Ptr *pfnMtkVideoSecureMemAllocatorInit_Ptr = (MtkVideoSecureMemAllocatorInit_Ptr *) dlsym(mH264SecVdecInHouseLib, MTK_H264_SEC_VDEC_IN_HOUSE_INIT_NAME);
       MtkVideoSecureMemAllocatorInit_Ptr *pfnMtkVideoSecureMemAllocatorInit_Ptr = (MtkVideoSecureMemAllocatorInit_Ptr *) dlsym(mCommonVdecInHouseLib, MTK_COMMON_SEC_VDEC_IN_HOUSE_INIT_NAME);
       if (NULL == pfnMtkVideoSecureMemAllocatorInit_Ptr)
       {
           MTK_OMX_LOGE("cannot find pfnMtkVideoSecureMemAllocatorInit_Ptr, LINE: %d", __LINE__);
           return OMX_FALSE;
       }
       if (MTK_SECURE_AL_SUCCESS != pfnMtkVideoSecureMemAllocatorInit_Ptr(1))
       {
           MTK_OMX_LOGE("[ERROR] pfnMtkVideoSecureMemAllocatorInit_Ptr error\n");
           return OMX_FALSE;
       }
   }
   else // TRUSTONIC_TEE
   {
       mInputPortDef.nBufferSize = MTK_VDEC_HEVCSEC_DEFAULT_INPUT_BUFFER_SIZE;   // for Trustonic t-base (D9)

       // init TLC lib
       VAL_CHAR_T value[PROPERTY_VALUE_MAX];
       property_get("ro.vendor.mtk_tee_gp_support", value, "0");
       int gpSupport = atoi(value);
       if ((1 == gpSupport) || (BLOWFISH_TEE == mTeeType)){
           char MTK_H265_SEC_VDEC_TLC_LIB_NAME[] = "HEVCSecureVdecCA.so";
           mH265SecVdecTlcLib = dlopen(MTK_H265_SEC_VDEC_TLC_LIB_NAME, RTLD_NOW);
       }
       else{
           char MTK_H265_SEC_VDEC_TLC_LIB_NAME[] = "libMtkH265SecVdecTLCLib.so";
           mH265SecVdecTlcLib = dlopen(MTK_H265_SEC_VDEC_TLC_LIB_NAME, RTLD_NOW);
       }
       if (NULL == mH265SecVdecTlcLib)
       {
           MTK_OMX_LOGE("mH265SecVdecTlcLib open failed: [%s]", dlerror());
           return OMX_FALSE;
       }

       MtkH265SecVdec_secMemInit_Ptr *pfnMtkH265SecVdec_secMemInit = (MtkH265SecVdec_secMemInit_Ptr *) dlsym(mH265SecVdecTlcLib, MTK_H265_SEC_VDEC_SEC_MEM_INIT_NAME);
       if (NULL == pfnMtkH265SecVdec_secMemInit)
       {
           MTK_OMX_LOGE("cannot find MtkH265SecVdec_secMemInit, LINE: %d", __LINE__);
           return OMX_FALSE;
       }

       // init secure memory sevice
       if (pfnMtkH265SecVdec_secMemInit() < 0)
       {
           MTK_OMX_LOGE("MtkH265SecVdec_secMemInit error, LINE: %d", __LINE__);
           return OMX_FALSE;
       }

       MtkH265SecVdec_tlcHandleCreate_Ptr *pfnMtkH265SecVdec_tlcHandleCreate = (MtkH265SecVdec_tlcHandleCreate_Ptr *) dlsym(mH265SecVdecTlcLib, MTK_H265_SEC_VDEC_TLC_HANDLE_CREATE_NAME);
       if (NULL == pfnMtkH265SecVdec_tlcHandleCreate)
       {
           MTK_OMX_LOGE("cannot find MtkH265SecVdec_tlcHandleCreate, LINE: %d", __LINE__);
           dlclose(mH265SecVdecTlcLib);
           return OMX_FALSE;
       }

       mTlcHandle = pfnMtkH265SecVdec_tlcHandleCreate();

   }
   /* Same with AVC */
   mInputUseION = OMX_FALSE;
   mOutputUseION = OMX_FALSE;
   //mStoreMetaDataInBuffers = OMX_FALSE;
#if SECURE_OUTPUT_USE_ION
    if (mOutputMVAMgr != NULL)
    {
        mOutputMVAMgr->setBoolProperty("SEC", mIsSecureInst);
    }
#endif
   return OMX_TRUE;
}

void MtkOmxVdec::initInputPortCommon(OMX_PARAM_PORTDEFINITIONTYPE* pPortDef)
{
    pPortDef->nPortIndex                            = MTK_OMX_INPUT_PORT;
    pPortDef->eDir                                  = OMX_DirInput;
    pPortDef->eDomain                               = OMX_PortDomainVideo;
    pPortDef->format.video.pNativeRender            = NULL;
    pPortDef->format.video.nFrameWidth              = 176;
    pPortDef->format.video.nFrameHeight             = 144;
    pPortDef->format.video.nStride                  = 176;
    pPortDef->format.video.nSliceHeight             = 144;
    pPortDef->format.video.nBitrate                 = 64000;
    pPortDef->format.video.xFramerate               = (15 << 16);
    pPortDef->format.video.bFlagErrorConcealment    = OMX_FALSE;
    pPortDef->format.video.eColorFormat             = OMX_COLOR_FormatUnused;
    pPortDef->nBufferCountMin                       = 1;
    pPortDef->bEnabled                              = OMX_TRUE;
    pPortDef->bPopulated                            = OMX_FALSE;

    //TODO: Fields must be set by codec did not have default values here
}

void MtkOmxVdec::initOutputPortCommon(OMX_PARAM_PORTDEFINITIONTYPE* pPortDef)
{
    pPortDef->nPortIndex                           = MTK_OMX_OUTPUT_PORT;
    pPortDef->eDomain                              = OMX_PortDomainVideo;
    pPortDef->format.video.cMIMEType               = (OMX_STRING)"raw";
    pPortDef->format.video.pNativeRender           = 0;
    pPortDef->format.video.bFlagErrorConcealment   = OMX_FALSE;
    pPortDef->format.video.eCompressionFormat      = OMX_VIDEO_CodingUnused;
    pPortDef->format.video.eColorFormat            = OMX_COLOR_FormatYUV420Planar;
    pPortDef->format.video.nFrameWidth             = 176;
    pPortDef->format.video.nStride                 = 176;
    pPortDef->format.video.nFrameHeight            = 144;
    pPortDef->format.video.nSliceHeight            = 144;
    pPortDef->format.video.nBitrate                = 64000;
    pPortDef->format.video.xFramerate              = (15 << 16);
    pPortDef->eDir                                 = OMX_DirOutput;
    pPortDef->bEnabled = OMX_TRUE;
    pPortDef->bPopulated = OMX_FALSE;

    //TODO: Fields must be set by codec did not have default values here
}

void MtkOmxVdec::initOutputPortFormatCommon(OMX_VIDEO_PARAM_PORTFORMATTYPE* pPortFormat)
{
    pPortFormat->nPortIndex            = MTK_OMX_OUTPUT_PORT;
    pPortFormat->nIndex                = 0;
    pPortFormat->eCompressionFormat    = OMX_VIDEO_CodingUnused;
    pPortFormat->eColorFormat          = OMX_COLOR_FormatYUV420Planar;
}

OMX_BOOL MtkOmxVdec::InitHEVCParams()
{
    // init input port format
    strncpy((char *)mCompRole, "video_decoder.hevc", sizeof(mCompRole));
    mInputPortFormat.nPortIndex         = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex             = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingHEVC;
    mInitRetryNum                       = 0;

    initOutputPortFormatCommon(&mOutputPortFormat);

    initInputPortCommon(&mInputPortDef);
    mInputPortDef.format.video.cMIMEType                = (OMX_STRING)"video/hevc";
    mInputPortDef.format.video.eCompressionFormat       = OMX_VIDEO_CodingHEVC;
    mInputPortDef.nBufferCountActual = MTK_VDEC_HEVC_DEFAULT_INPUT_BUFFER_COUNT;
    if (mAdaptivePlayback)
    {
        mInputPortDef.nBufferCountMin = MTK_VDEC_HEVC_DEFAULT_INPUT_BUFFER_COUNT;
    }
    mInputPortDef.nBufferSize = MTK_VDEC_HEVC_DEFAULT_INPUT_BUFFER_SIZE;

    initOutputPortCommon(&mOutputPortDef);
    mOutputPortDef.nBufferCountActual = (mForceOutputBufferCount == 0 ? MTK_VDEC_HEVC_DEFAULT_OUTPUT_BUFFER_COUNT : mForceOutputBufferCount);
    mOutputPortDef.nBufferCountMin = mOutputPortDef.nBufferCountActual - mMinUndequeuedBufs;
    mOutputPortDef.nBufferSize = MTK_VDEC_HEVC_DEFAULT_OUTPUT_BUFFER_SIZE + 16;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::InitH263Params()
{
    // init input port format
    strncpy((char *)mCompRole, "video_decoder.h263", sizeof(mCompRole));
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingH263;

    initOutputPortFormatCommon(&mOutputPortFormat);

    initInputPortCommon(&mInputPortDef);
    mInputPortDef.format.video.cMIMEType                = (OMX_STRING)"video/h263";
    mInputPortDef.format.video.eCompressionFormat       = OMX_VIDEO_CodingH263;
    mInputPortDef.nBufferCountActual = MTK_VDEC_H263_DEFAULT_INPUT_BUFFER_COUNT;
    if (mAdaptivePlayback)
    {
        mInputPortDef.nBufferCountMin = MTK_VDEC_H263_DEFAULT_INPUT_BUFFER_COUNT;
    }
    mInputPortDef.nBufferSize = MTK_VDEC_H263_DEFAULT_INPUT_BUFFER_SIZE;

    initOutputPortCommon(&mOutputPortDef);
    mOutputPortDef.nBufferCountActual = (mForceOutputBufferCount == 0 ? (MTK_VDEC_H263_DEFAULT_OUTPUT_BUFFER_COUNT + mMinUndequeuedBufs) : mForceOutputBufferCount);
    mOutputPortDef.nBufferCountMin = mOutputPortDef.nBufferCountActual - mMinUndequeuedBufs;
    mOutputPortDef.nBufferSize = MTK_VDEC_H263_DEFAULT_OUTPUT_BUFFER_SIZE + 16;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::InitMpeg4Params()
{
    // init input port format
    strncpy((char *)mCompRole, "video_decoder.mpeg4", sizeof(mCompRole));
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingMPEG4;

    initOutputPortFormatCommon(&mOutputPortFormat);

    initInputPortCommon(&mInputPortDef);
    mInputPortDef.format.video.cMIMEType                = (OMX_STRING)"video/mpeg4";
    mInputPortDef.format.video.eCompressionFormat       = OMX_VIDEO_CodingMPEG4;
    mInputPortDef.nBufferCountActual = MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_COUNT;
    if (mAdaptivePlayback)
    {
        mInputPortDef.nBufferCountMin = MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_COUNT;
    }
    mInputPortDef.nBufferSize = MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_SIZE;

    initOutputPortCommon(&mOutputPortDef);
    mOutputPortDef.nBufferCountActual = (mForceOutputBufferCount  == 0 ? (MTK_VDEC_MPEG4_DEFAULT_OUTPUT_BUFFER_COUNT + mMinUndequeuedBufs): mForceOutputBufferCount );
    mOutputPortDef.nBufferCountMin    = mOutputPortDef.nBufferCountActual - mMinUndequeuedBufs;
    mOutputPortDef.nBufferSize        = MTK_VDEC_MPEG4_DEFAULT_OUTPUT_BUFFER_SIZE + 16;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::InitDivxParams()
{
    // init input port format
    strncpy((char *)mCompRole, "video_decoder.divx", sizeof(mCompRole));
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingDIVX;

    initOutputPortFormatCommon(&mOutputPortFormat);

    initInputPortCommon(&mInputPortDef);
    mInputPortDef.format.video.cMIMEType                = (OMX_STRING)"video/divx";
    mInputPortDef.nBufferCountActual = MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_COUNT;
    if (mAdaptivePlayback)
    {
        mInputPortDef.nBufferCountMin = MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_COUNT;
    }
    mInputPortDef.nBufferSize = MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_SIZE;

    initOutputPortCommon(&mOutputPortDef);
    mOutputPortDef.nBufferCountActual = (mForceOutputBufferCount  == 0 ? (MTK_VDEC_MPEG4_DEFAULT_OUTPUT_BUFFER_COUNT + mMinUndequeuedBufs): mForceOutputBufferCount );
    mOutputPortDef.nBufferCountMin    = mOutputPortDef.nBufferCountActual - mMinUndequeuedBufs;
    mOutputPortDef.nBufferSize        = MTK_VDEC_MPEG4_DEFAULT_OUTPUT_BUFFER_SIZE + 16;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::InitDivx3Params()
{
    // init input port format
    strncpy((char *)mCompRole, "video_decoder.divx3", sizeof(mCompRole));
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingDIVX3;

    initOutputPortFormatCommon(&mOutputPortFormat);

    initInputPortCommon(&mInputPortDef);
    mInputPortDef.format.video.cMIMEType                = (OMX_STRING)"video/divx3";
    mInputPortDef.format.video.eCompressionFormat       = OMX_VIDEO_CodingMPEG4;
    mInputPortDef.nBufferCountActual = MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_COUNT;
    if (mAdaptivePlayback)
    {
        mInputPortDef.nBufferCountMin = MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_COUNT;
    }
    mInputPortDef.nBufferSize = MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_SIZE;

    initOutputPortCommon(&mOutputPortDef);
    mOutputPortDef.nBufferCountActual = (mForceOutputBufferCount  == 0 ? (MTK_VDEC_MPEG4_DEFAULT_OUTPUT_BUFFER_COUNT + mMinUndequeuedBufs): mForceOutputBufferCount );
    mOutputPortDef.nBufferCountMin    = mOutputPortDef.nBufferCountActual - mMinUndequeuedBufs;
    mOutputPortDef.nBufferSize        = MTK_VDEC_MPEG4_DEFAULT_OUTPUT_BUFFER_SIZE + 16;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::InitXvidParams()
{
    // init input port format
    strncpy((char *)mCompRole, "video_decoder.xvid", sizeof(mCompRole));
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingXVID;

    initOutputPortFormatCommon(&mOutputPortFormat);

    initInputPortCommon(&mInputPortDef);
    mInputPortDef.format.video.cMIMEType                = (OMX_STRING)"video/xvid";
    mInputPortDef.format.video.eCompressionFormat       = OMX_VIDEO_CodingMPEG4;
    mInputPortDef.nBufferCountActual = MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_COUNT;
    if (mAdaptivePlayback)
    {
        mInputPortDef.nBufferCountMin = MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_COUNT;
    }
    mInputPortDef.nBufferSize = MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_SIZE;

    initOutputPortCommon(&mOutputPortDef);
    mOutputPortDef.nBufferCountActual = (mForceOutputBufferCount  == 0 ? (MTK_VDEC_MPEG4_DEFAULT_OUTPUT_BUFFER_COUNT + mMinUndequeuedBufs): mForceOutputBufferCount );
    mOutputPortDef.nBufferCountMin    = mOutputPortDef.nBufferCountActual - mMinUndequeuedBufs;
    mOutputPortDef.nBufferSize        = MTK_VDEC_MPEG4_DEFAULT_OUTPUT_BUFFER_SIZE + 16;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::InitS263Params()
{
    // init input port format
    strncpy((char *)mCompRole, "video_decoder.s263", sizeof(mCompRole));
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingS263;

    initOutputPortFormatCommon(&mOutputPortFormat);

    initInputPortCommon(&mInputPortDef);
    mInputPortDef.format.video.cMIMEType                = (OMX_STRING)"video/flv1";
    mInputPortDef.format.video.eCompressionFormat       = OMX_VIDEO_CodingMPEG4;
    mInputPortDef.nBufferCountActual = MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_COUNT;
    if (mAdaptivePlayback)
    {
        mInputPortDef.nBufferCountMin       = MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_COUNT;
    }
    mInputPortDef.nBufferSize = MTK_VDEC_MPEG4_DEFAULT_INPUT_BUFFER_SIZE;

    initOutputPortCommon(&mOutputPortDef);
    mOutputPortDef.nBufferCountActual = (mForceOutputBufferCount  == 0 ? (MTK_VDEC_MPEG4_DEFAULT_OUTPUT_BUFFER_COUNT + mMinUndequeuedBufs): mForceOutputBufferCount );
    mOutputPortDef.nBufferCountMin    = mOutputPortDef.nBufferCountActual - mMinUndequeuedBufs;
    mOutputPortDef.nBufferSize        = MTK_VDEC_MPEG4_DEFAULT_OUTPUT_BUFFER_SIZE + 16;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::InitAvcParams()
{
    // init input port format
    strncpy((char *)mCompRole, "video_decoder.avc", sizeof(mCompRole));
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingAVC;

    initOutputPortFormatCommon(&mOutputPortFormat);

    initInputPortCommon(&mInputPortDef);
    mInputPortDef.format.video.cMIMEType                = (OMX_STRING)"video/avc";
    mInputPortDef.format.video.eCompressionFormat       = OMX_VIDEO_CodingAVC;
    mInputPortDef.nBufferCountActual                    = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_COUNT;
    if (mAdaptivePlayback)
    {
        mInputPortDef.nBufferCountMin = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_COUNT;
    }
    mInputPortDef.nBufferSize = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE;

    initOutputPortCommon(&mOutputPortDef);
    mOutputPortDef.nBufferCountActual = (mForceOutputBufferCount == 0 ? MTK_VDEC_AVC_DEFAULT_OUTPUT_BUFFER_COUNT : mForceOutputBufferCount);
    mOutputPortDef.nBufferCountMin = mOutputPortDef.nBufferCountActual - mMinUndequeuedBufs;
    mOutputPortDef.nBufferSize = MTK_VDEC_AVC_DEFAULT_OUTPUT_BUFFER_SIZE + 16;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::InitAvcSecParams()
{
    // check secure environment
    if(OMX_FALSE == CheckTeeType())
    {
        return OMX_FALSE;
    }

    MTK_OMX_LOGE("InitAvcSecParams, Tee Type = %d", mTeeType);

    // init input port format
    strncpy((char *)mCompRole, "video_decoder.avc", sizeof(mCompRole));
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingAVC;

    initOutputPortFormatCommon(&mOutputPortFormat);

    initInputPortCommon(&mInputPortDef);
    mInputPortDef.format.video.cMIMEType                   = (OMX_STRING)"video/avc";
    mInputPortDef.format.video.eCompressionFormat    = OMX_VIDEO_CodingAVC;
    mInputPortDef.nBufferCountActual                         = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_COUNT;
    if (mAdaptivePlayback)
    {
        mInputPortDef.nBufferCountMin       = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_COUNT;
    }
    mInputPortDef.nBufferSize                                    = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE;

    initOutputPortCommon(&mOutputPortDef);
    mOutputPortDef.nBufferCountActual = (mForceOutputBufferCount == 0 ? MTK_VDEC_AVC_DEFAULT_OUTPUT_BUFFER_COUNT : mForceOutputBufferCount);
    mOutputPortDef.nBufferCountMin = mOutputPortDef.nBufferCountActual - mMinUndequeuedBufs;
    mOutputPortDef.nBufferSize = MTK_VDEC_AVC_DEFAULT_OUTPUT_BUFFER_SIZE + 16;

    if (INHOUSE_TEE == mTeeType)
    {
        //#if defined(MTK_SEC_VIDEO_PATH_SUPPORT) && defined(MTK_IN_HOUSE_TEE_SUPPORT)
        mCommonVdecInHouseLib = dlopen(MTK_COMMON_SEC_VDEC_IN_HOUSE_LIB_NAME, RTLD_NOW);
        if (NULL == mCommonVdecInHouseLib)
        {
            MTK_OMX_LOGE("lib_uree_mtk_video_secure_al open failed: [%s]", dlerror());
            return OMX_FALSE;
        }
        MtkVideoSecureMemAllocatorInit_Ptr *pfnMtkVideoSecureMemAllocatorInit_Ptr = (MtkVideoSecureMemAllocatorInit_Ptr *) dlsym(mCommonVdecInHouseLib, MTK_COMMON_SEC_VDEC_IN_HOUSE_INIT_NAME);
        if (NULL == pfnMtkVideoSecureMemAllocatorInit_Ptr)
        {
            MTK_OMX_LOGE("cannot find pfnMtkVideoSecureMemAllocatorInit_Ptr, LINE: %d", __LINE__);
            return OMX_FALSE;
        }
        if (MTK_SECURE_AL_SUCCESS != pfnMtkVideoSecureMemAllocatorInit_Ptr(1))
        {
            MTK_OMX_LOGE("[ERROR] pfnMtkVideoSecureMemAllocatorInit_Ptr error\n");
            return OMX_FALSE;
        }
        //#endif
    }
    else // TRUSTONIC_TEE
    {
        mInputPortDef.nBufferSize = MTK_VDEC_AVCSEC_DEFAULT_INPUT_BUFFER_SIZE;   // for Trustonic t-base (D9)
        // init TLC lib
        VAL_CHAR_T value[PROPERTY_VALUE_MAX];
        property_get("ro.vendor.mtk_tee_gp_support", value, "0");
        int gpSupport = atoi(value);
        if ((1 == gpSupport) || (BLOWFISH_TEE == mTeeType)){
            char MTK_H264_SEC_VDEC_TLC_LIB_NAME[] = "AVCSecureVdecCA.so";
            mH264SecVdecTlcLib = dlopen(MTK_H264_SEC_VDEC_TLC_LIB_NAME, RTLD_NOW);
        }
        else{
            char MTK_H264_SEC_VDEC_TLC_LIB_NAME[] = "libMtkH264SecVdecTLCLib.so";
            mH264SecVdecTlcLib = dlopen(MTK_H264_SEC_VDEC_TLC_LIB_NAME, RTLD_NOW);
        }
        if (NULL == mH264SecVdecTlcLib)
        {
            MTK_OMX_LOGE("mH264SecVdecTlcLib open failed: [%s]", dlerror());
            return OMX_FALSE;
        }

        MtkH264SecVdec_secMemInit_Ptr *pfnMtkH264SecVdec_secMemInit = (MtkH264SecVdec_secMemInit_Ptr *) dlsym(mH264SecVdecTlcLib, MTK_H264_SEC_VDEC_SEC_MEM_INIT_NAME);
        if (NULL == pfnMtkH264SecVdec_secMemInit)
        {
            MTK_OMX_LOGE("cannot find MtkH264SecVdec_secMemInit, LINE: %d", __LINE__);
            return OMX_FALSE;
        }

        MtkH264SecVdec_tlcHandleCreate_Ptr *pfnMtkH264SecVdec_tlcHandleCreate = (MtkH264SecVdec_tlcHandleCreate_Ptr *) dlsym(mH264SecVdecTlcLib, MTK_H264_SEC_VDEC_TLC_HANDLE_CREATE_NAME);
        if (NULL == pfnMtkH264SecVdec_tlcHandleCreate)
        {
            MTK_OMX_LOGE("cannot find MtkH264SecVdec_tlcHandleCreate, LINE: %d", __LINE__);
            return OMX_FALSE;
        }

        MtkH264SecVdec_tlcOpen_Ptr *pfnMtkH264SecVdec_tlcOpen = (MtkH264SecVdec_tlcOpen_Ptr *) dlsym(mH264SecVdecTlcLib, MTK_H264_SEC_VDEC_TLC_OPEN_NAME);
        if (NULL == pfnMtkH264SecVdec_tlcOpen)
        {
            MTK_OMX_LOGE("cannot find MtkH264SecVdec_tlcOpen, LINE: %d", __LINE__);
            return OMX_FALSE;
        }

        // init secure memory sevice
        if (pfnMtkH264SecVdec_secMemInit() < 0)
        {
            MTK_OMX_LOGE("MtkH264SecVdec_secMemInit error, LINE: %d", __LINE__);
            return OMX_FALSE;
        }

        mTlcHandle = pfnMtkH264SecVdec_tlcHandleCreate();

#if 0
        if (pfnMtkH264SecVdec_tlcOpen(mTlcHandle) < 0)
        {
            MTK_OMX_LOGE("MtkH264SecVdec_tlcOpen failed, LINE: %d", __LINE__);
            return OMX_FALSE;
        }
#endif
    }
    //mIsSecureInst = OMX_TRUE;
    mInputUseION = OMX_FALSE;
    mOutputUseION = OMX_FALSE;
    //mAdaptivePlayback = VAL_FALSE;
    //mStoreMetaDataInBuffers = OMX_FALSE;
#if SECURE_OUTPUT_USE_ION
    if (mOutputMVAMgr != NULL)
    {
        mOutputMVAMgr->setBoolProperty("SEC", mIsSecureInst);
    }

#endif
    return OMX_TRUE;
}


OMX_BOOL MtkOmxVdec::InitRvParams()
{
    // init input port format
    strncpy((char *)mCompRole, "video_decoder.rv", sizeof(mCompRole));
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingRV;

    initOutputPortFormatCommon(&mOutputPortFormat);

    initInputPortCommon(&mInputPortDef);
    mInputPortDef.format.video.cMIMEType                   = (OMX_STRING)"video/vnd.rn-realvideo";
    mInputPortDef.format.video.eCompressionFormat    = OMX_VIDEO_CodingRV;
    mInputPortDef.nBufferCountActual                         = MTK_VDEC_RV_DEFAULT_INPUT_BUFFER_COUNT;
    if (mAdaptivePlayback)
    {
        mInputPortDef.nBufferCountMin       = MTK_VDEC_RV_DEFAULT_INPUT_BUFFER_COUNT;
    }
    mInputPortDef.nBufferSize                                    = MTK_VDEC_RV_DEFAULT_INPUT_BUFFER_SIZE;

    initOutputPortCommon(&mOutputPortDef);
    mOutputPortDef.nBufferCountActual = (mForceOutputBufferCount == 0 ? MTK_VDEC_RV_DEFAULT_OUTPUT_BUFFER_COUNT : mForceOutputBufferCount);
    mOutputPortDef.nBufferCountMin = mOutputPortDef.nBufferCountActual - mMinUndequeuedBufs;
    mOutputPortDef.nBufferSize = MTK_VDEC_RV_DEFAULT_OUTPUT_BUFFER_SIZE + 16;

    // init rv type
    MTK_OMX_MEMSET(&mRvType, 0x00, sizeof(OMX_VIDEO_PARAM_RVTYPE));
    mRvType.bEnableLatencyMode = OMX_FALSE;
    mRvType.bEnablePostFilter = OMX_FALSE;
    mRvType.bEnableTemporalInterpolation = OMX_FALSE;
    mRvType.eFormat = OMX_VIDEO_RVFormat9;
    mRvType.nBitsPerPixel = 16;
    mRvType.nBitstreamFlags = 0;
    mRvType.nBitstreamVersion = 0;
    mRvType.nFrameRate = 30;
    mRvType.nMaxEncodeFrameSize = 0;
    mRvType.nPaddedHeight = 0;
    mRvType.nPaddedWidth = 0;
    mRvType.nPortIndex = MTK_OMX_INPUT_PORT;

    return OMX_TRUE;
}


OMX_BOOL MtkOmxVdec::InitVc1Params()
{
    // init input port format
    strncpy((char *)mCompRole, "video_decoder.vc1", sizeof(mCompRole));
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingWMV;

    initOutputPortFormatCommon(&mOutputPortFormat);

    initInputPortCommon(&mInputPortDef);
    mInputPortDef.format.video.cMIMEType                   = (OMX_STRING)"video/x-ms-wmv";
    mInputPortDef.format.video.eCompressionFormat    = OMX_VIDEO_CodingWMV;
    mInputPortDef.nBufferCountActual                         = MTK_VDEC_VC1_DEFAULT_INPUT_BUFFER_COUNT;
    if (mAdaptivePlayback)
    {
        mInputPortDef.nBufferCountMin       = MTK_VDEC_VC1_DEFAULT_INPUT_BUFFER_COUNT;
    }
    mInputPortDef.nBufferSize                                    = MTK_VDEC_VC1_DEFAULT_INPUT_BUFFER_SIZE;

    initOutputPortCommon(&mOutputPortDef);
    mOutputPortDef.nBufferCountActual = (mForceOutputBufferCount == 0 ? MTK_VDEC_VC1_DEFAULT_OUTPUT_BUFFER_COUNT : mForceOutputBufferCount);
    mOutputPortDef.nBufferCountMin = mOutputPortDef.nBufferCountActual - mMinUndequeuedBufs;
    mOutputPortDef.nBufferSize = MTK_VDEC_VC1_DEFAULT_OUTPUT_BUFFER_SIZE + 16;

    VAL_UINT32_T is_support;
    VDEC_DRV_QUERY_VIDEO_FORMAT_T   qinfo;
    VDEC_DRV_QUERY_VIDEO_FORMAT_T   *pQinfoIn = &qinfo;

    pQinfoIn->u4VideoFormat = GetVdecFormat(MTK_VDEC_CODEC_ID_VC1);

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::InitVpxParams()
{
    // init input port format
    strncpy((char *)mCompRole, "video_decoder.vp8", sizeof(mCompRole));
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingVP8;

    initOutputPortFormatCommon(&mOutputPortFormat);

    initInputPortCommon(&mInputPortDef);
    mInputPortDef.format.video.cMIMEType                = (OMX_STRING)"video/x-vnd.on2.vp8";
    mInputPortDef.format.video.eCompressionFormat       = OMX_VIDEO_CodingVP8;
    mInputPortDef.nBufferCountActual                    = MTK_VDEC_VPX_DEFAULT_INPUT_BUFFER_COUNT;
    if (mAdaptivePlayback)
    {
        mInputPortDef.nBufferCountMin       = MTK_VDEC_VPX_DEFAULT_INPUT_BUFFER_COUNT;
    }
    mInputPortDef.nBufferSize                           = MTK_VDEC_VPX_DEFAULT_INPUT_BUFFER_SIZE;

    initOutputPortCommon(&mOutputPortDef);
#if (ANDROID_VER >= ANDROID_ICS)
    mOutputPortDef.nBufferCountActual                   = (mForceOutputBufferCount == 0 ? MTK_VDEC_VPX_DEFAULT_OUTPUT_BUFFER_COUNT + mMinUndequeuedBufs : mForceOutputBufferCount);
#else
    mOutputPortDef.nBufferCountActual                   = (mForceOutputBufferCount == 0 ? MTK_VDEC_VPX_DEFAULT_OUTPUT_BUFFER_COUNT : mForceOutputBufferCount);
#endif
    mOutputPortDef.nBufferCountMin                      = mOutputPortDef.nBufferCountActual - mMinUndequeuedBufs;
    mOutputPortDef.nBufferSize                          = MTK_VDEC_VPX_DEFAULT_OUTPUT_BUFFER_SIZE + 16;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::InitVp9Params()
{
    // init input port format
    strncpy((char *)mCompRole, "video_decoder.vp9", sizeof(mCompRole));
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingVP9;

    initOutputPortFormatCommon(&mOutputPortFormat);

    initInputPortCommon(&mInputPortDef);
    mInputPortDef.format.video.cMIMEType                = (OMX_STRING)"video/x-vnd.on2.vp9";
    mInputPortDef.format.video.eCompressionFormat       = OMX_VIDEO_CodingVP9;
    mInputPortDef.nBufferCountActual                    = MTK_VDEC_VPX_DEFAULT_INPUT_BUFFER_COUNT;
    if (mAdaptivePlayback)
    {
        mInputPortDef.nBufferCountMin       = MTK_VDEC_VPX_DEFAULT_INPUT_BUFFER_COUNT;
    }
    mInputPortDef.nBufferSize                           = MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE;

    initOutputPortCommon(&mOutputPortDef);
#if (ANDROID_VER >= ANDROID_ICS)
    mOutputPortDef.nBufferCountActual                   = (mForceOutputBufferCount == 0 ? MTK_VDEC_VP9_DEFAULT_OUTPUT_BUFFER_COUNT + mMinUndequeuedBufs : mForceOutputBufferCount);
#else
    mOutputPortDef.nBufferCountActual                   = (mForceOutputBufferCount == 0 ? MTK_VDEC_VP9_DEFAULT_OUTPUT_BUFFER_COUNT : mForceOutputBufferCount);
#endif
    mOutputPortDef.nBufferCountMin                      = mOutputPortDef.nBufferCountActual - mMinUndequeuedBufs;
    mOutputPortDef.nBufferSize                          = MTK_VDEC_VPX_DEFAULT_OUTPUT_BUFFER_SIZE + 16;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::InitVp9SecParams()
{
    // check secure environment
    if(OMX_FALSE == CheckTeeType())
    {
        return OMX_FALSE;
    }

    MTK_OMX_LOGE("InitVP9SecParams, Tee Type = %d", mTeeType);

    // init input port format
    strncpy((char *)mCompRole, "video_decoder.vp9", sizeof(mCompRole));
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingVP9;

    initOutputPortFormatCommon(&mOutputPortFormat);

    initInputPortCommon(&mInputPortDef);
    mInputPortDef.format.video.cMIMEType                = (OMX_STRING)"video/x-vnd.on2.vp9";
    mInputPortDef.format.video.eCompressionFormat       = OMX_VIDEO_CodingVP9;
    mInputPortDef.nBufferCountActual                    = MTK_VDEC_VPX_DEFAULT_INPUT_BUFFER_COUNT;
    if (mAdaptivePlayback)
    {
        mInputPortDef.nBufferCountMin       = MTK_VDEC_VPX_DEFAULT_INPUT_BUFFER_COUNT;
    }
    mInputPortDef.nBufferSize                           = MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE;

    initOutputPortCommon(&mOutputPortDef);
#if (ANDROID_VER >= ANDROID_ICS)
    mOutputPortDef.nBufferCountActual                   = (mForceOutputBufferCount == 0 ? (MTK_VDEC_VPX_DEFAULT_OUTPUT_BUFFER_COUNT )+ mMinUndequeuedBufs : mForceOutputBufferCount);
#else
    mOutputPortDef.nBufferCountActual                   = (mForceOutputBufferCount == 0 ? MTK_VDEC_VPX_DEFAULT_OUTPUT_BUFFER_COUNT : mForceOutputBufferCount);
#endif
    //for supporting frame partition mode, need more two buffers
    mOutputPortDef.nBufferCountActual += 2;

    mOutputPortDef.nBufferCountMin                      = mOutputPortDef.nBufferCountActual - mMinUndequeuedBufs;
    mOutputPortDef.nBufferSize                          = MTK_VDEC_VPX_DEFAULT_OUTPUT_BUFFER_SIZE + 16;

    // init tee secure env
    if (INHOUSE_TEE == mTeeType)
    {
        //#if defined(MTK_SEC_VIDEO_PATH_SUPPORT) && defined(MTK_IN_HOUSE_TEE_SUPPORT)
        mCommonVdecInHouseLib = dlopen(MTK_COMMON_SEC_VDEC_IN_HOUSE_LIB_NAME, RTLD_NOW);
        if (NULL == mCommonVdecInHouseLib)
        {
            MTK_OMX_LOGE("lib_uree_mtk_video_secure_al open failed: [%s]", dlerror());
            return OMX_FALSE;
        }
        MtkVideoSecureMemAllocatorInit_Ptr *pfnMtkVideoSecureMemAllocatorInit_Ptr = (MtkVideoSecureMemAllocatorInit_Ptr *) dlsym(mCommonVdecInHouseLib, MTK_COMMON_SEC_VDEC_IN_HOUSE_INIT_NAME);
        if (NULL == pfnMtkVideoSecureMemAllocatorInit_Ptr)
        {
            MTK_OMX_LOGE("cannot find pfnMtkVideoSecureMemAllocatorInit_Ptr, LINE: %d", __LINE__);
            return OMX_FALSE;
        }
        if (MTK_SECURE_AL_SUCCESS != pfnMtkVideoSecureMemAllocatorInit_Ptr(1))
        {
            MTK_OMX_LOGE("[ERROR] pfnMtkVideoSecureMemAllocatorInit_Ptr error\n");
            return OMX_FALSE;
        }
        //#endif
    }
    else // TRUSTONIC_TEE
    {
        mInputPortDef.nBufferSize = MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE;	 // for Trustonic t-base (D9)

        // init TLC lib
        mVP9SecVdecTlcLib = dlopen(MTK_VP9_SEC_VDEC_TLC_LIB_NAME, RTLD_NOW);
        if (NULL == mVP9SecVdecTlcLib)
        {
            MTK_OMX_LOGE("mVP9SecVdecTlcLib open failed: [%s]", dlerror());
            return OMX_FALSE;
        }

        MtkVP9SecVdec_secMemInit_Ptr *pfnMtkVP9SecVdec_secMemInit = (MtkVP9SecVdec_secMemInit_Ptr *) dlsym(mVP9SecVdecTlcLib, MTK_VP9_SEC_VDEC_SEC_MEM_INIT_NAME);
        if (NULL == pfnMtkVP9SecVdec_secMemInit)
        {
            MTK_OMX_LOGE("cannot find MtkVP9SecVdec_secMemInit, LINE: %d", __LINE__);
            return OMX_FALSE;
        }

        MtkVP9SecVdec_tlcHandleCreate_Ptr *pfnMtkVP9SecVdec_tlcHandleCreate = (MtkVP9SecVdec_tlcHandleCreate_Ptr *) dlsym(mVP9SecVdecTlcLib, MTK_VP9_SEC_VDEC_TLC_HANDLE_CREATE_NAME);
        if (NULL == pfnMtkVP9SecVdec_tlcHandleCreate)
        {
            MTK_OMX_LOGE("cannot find MtkVP9SecVdec_tlcHandleCreate, LINE: %d", __LINE__);
            return OMX_FALSE;
        }

        MtkVP9SecVdec_tlcOpen_Ptr *pfnMtkVP9SecVdec_tlcOpen = (MtkVP9SecVdec_tlcOpen_Ptr *) dlsym(mVP9SecVdecTlcLib, MTK_VP9_SEC_VDEC_TLC_OPEN_NAME);
        if (NULL == pfnMtkVP9SecVdec_tlcOpen)
        {
            MTK_OMX_LOGE("cannot find MtkVP9SecVdec_tlcOpen, LINE: %d", __LINE__);
            return OMX_FALSE;
        }

        // init secure memory sevice
        if (pfnMtkVP9SecVdec_secMemInit() < 0)
        {
            MTK_OMX_LOGE("MtkVP9SecVdec_secMemInit error, LINE: %d", __LINE__);
            return OMX_FALSE;
        }

        mTlcHandle = pfnMtkVP9SecVdec_tlcHandleCreate();
    }

    mInputUseION = OMX_FALSE;
    mOutputUseION = OMX_FALSE;
    //mAdaptivePlayback = VAL_FALSE;
    //mStoreMetaDataInBuffers = OMX_FALSE;

#if SECURE_OUTPUT_USE_ION
    if (mOutputMVAMgr != NULL)
    {
        mOutputMVAMgr->setBoolProperty("SEC", mIsSecureInst);
    }
#endif

    return OMX_TRUE;
}
OMX_BOOL MtkOmxVdec::InitMpeg2Params()
{
    // init input port format
    strncpy((char *)mCompRole, "video_decoder.mpeg2", sizeof(mCompRole));
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingMPEG2;

    initOutputPortFormatCommon(&mOutputPortFormat);

    initInputPortCommon(&mInputPortDef);
    mInputPortDef.format.video.cMIMEType                   = (OMX_STRING)"video/mpeg2";
    mInputPortDef.format.video.eCompressionFormat    = OMX_VIDEO_CodingMPEG2;
    mInputPortDef.nBufferCountActual                         = MTK_VDEC_MPEG2_DEFAULT_INPUT_BUFFER_COUNT;
    if (mAdaptivePlayback)
    {
        mInputPortDef.nBufferCountMin       = MTK_VDEC_MPEG2_DEFAULT_INPUT_BUFFER_COUNT;
    }
    mInputPortDef.nBufferSize                                    = MTK_VDEC_MPEG2_DEFAULT_INPUT_BUFFER_SIZE;

    initOutputPortCommon(&mOutputPortDef);
    mOutputPortDef.nBufferCountActual = (mForceOutputBufferCount == 0 ? MTK_VDEC_MPEG2_DEFAULT_OUTPUT_BUFFER_COUNT : mForceOutputBufferCount);
    mOutputPortDef.nBufferCountMin = mOutputPortDef.nBufferCountActual - mMinUndequeuedBufs;
    mOutputPortDef.nBufferSize = MTK_VDEC_MPEG2_DEFAULT_OUTPUT_BUFFER_SIZE + 16;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::InitMJpegParams()
{
    strncpy((char *)mCompRole, "video_decoder.mjpeg", sizeof(mCompRole));
    // init input port format
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingMJPEG;
    mInputPortFormat.eColorFormat = OMX_COLOR_FormatUnused;

    initOutputPortFormatCommon(&mOutputPortFormat);
    mOutputPortFormat.eColorFormat = OMX_COLOR_Format32bitARGB8888;

    initInputPortCommon(&mInputPortDef);
    mInputPortDef.format.video.cMIMEType                = (OMX_STRING)"video/x-motion-jpeg";
    mInputPortDef.format.video.eCompressionFormat       = OMX_VIDEO_CodingMJPEG;
    mInputPortDef.nBufferCountActual                    = MTK_VDEC_MJPEG_DEFAULT_INPUT_BUFFER_COUNT;
    if (mAdaptivePlayback)
    {
        mInputPortDef.nBufferCountMin       = MTK_VDEC_MJPEG_DEFAULT_INPUT_BUFFER_COUNT;
    }
    mInputPortDef.nBufferSize                           = MTK_VDEC_MJPEG_DEFAULT_INPUT_BUFFER_SIZE;

    initOutputPortCommon(&mOutputPortDef);
    mOutputPortDef.format.video.eColorFormat            = OMX_COLOR_Format32bitARGB8888;
    mOutputPortDef.nBufferCountActual                   = (mForceOutputBufferCount == 0 ? MTK_VDEC_MJPEG_DEFAULT_OUTPUT_BUFFER_COUNT : mForceOutputBufferCount);
    mOutputPortDef.nBufferCountMin                      = mOutputPortDef.nBufferCountActual - mMinUndequeuedBufs;
    mOutputPortDef.nBufferSize                          = MTK_VDEC_MJPEG_DEFAULT_OUTPUT_BUFFER_SIZE;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::DecodeVideo(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::DecodeVideoEx(OMX_BUFFERHEADERTYPE *pInputBuf)//, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    ATRACE_CALL();
    OMX_BOOL bRet = OMX_FALSE;

    OMX_U8 *aInputBuf   = pInputBuf->pBuffer + pInputBuf->nOffset;
    //OMX_U8 *aOutputBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset;
    OMX_U32 aInputSize  = pInputBuf->nFilledLen;
    //VDEC_DRV_FRAMEBUF_T *frame;
    OMX_BOOL bEverOutput = OMX_TRUE;

    VDEC_DRV_RINGBUF_T *pRingBuf = NULL;
    OMX_BOOL bCodecProfileOrLevelChanged = OMX_FALSE;

    if (mDrvHandle != NULL)
    {
        // set buffer status for speedy mode mNumFreeAvailOutput, mNumDispAvailOutput, mOutputPortDef.nBufferCountActual
        OMX_U32 nBufferStatus = ((mNumNotDispAvailOutput & 0xFF) << 24) | ((mNumFreeAvailOutput & 0xFF) << 16) | (((mNumAllDispAvailOutput - mNumNotDispAvailOutput) & 0xFF) << 8) | (mOutputPortDef.nBufferCountActual & 0xFF);
        //MTK_OMX_LOGD("nBufferStatus : 0x%08x, mOutputPortDef.nBufferCountActual: %d, mNumFreeAvailOutput: %d, mNumDispAvailOutput: %d (all %d), mNumPendingOutput: %d, mNumNotDispAvailOutput: %d", (unsigned int)nBufferStatus, (unsigned int)mOutputPortDef.nBufferCountActual, (unsigned int)mNumFreeAvailOutput, (unsigned int)(mNumAllDispAvailOutput - mNumNotDispAvailOutput),(unsigned int)mNumAllDispAvailOutput, (unsigned int)mNumPendingOutput, (unsigned int)mNumNotDispAvailOutput);
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_BUF_STATUS_FOR_SPEEDY, (VAL_VOID_T *)&nBufferStatus, NULL);
        int64_t currTimeStamp;
        if (mSeekMode == OMX_TRUE)
        {
            MTK_OMX_LOGD("[%s][seek mode] replace mAVSyncTime with mSeekTargetTime (%lld)!",
                         __FUNCTION__, mSeekTargetTime);
            mAVSyncTime = mSeekTargetTime;
        }
        currTimeStamp = mAVSyncTime;
        //MTK_OMX_LOGD("Current TS : %lld",currTimeStamp);

        char setVideotsThreshold[PROPERTY_VALUE_MAX];
        property_get("vendor.mtk.omxvdec.setVideotsThreshold", setVideotsThreshold, "1000000");
        int64_t VideotsThreshold = (int64_t) atoi(setVideotsThreshold);
        if (currTimeStamp > VideotsThreshold || mResetCurrTime)
        {
            eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_CURRENT_PLAY_TIME, (VAL_VOID_T *)&currTimeStamp, NULL);
            if (mResetCurrTime) { mResetCurrTime = false; }
        }
    }

    static int64_t _in_time_1 = 0;
    static int64_t _in_time_2 = 0;
    static int64_t _out_time = 0;
    _in_time_1 = getTickCountMs();

    if (mDecoderInitCompleteFlag == OMX_FALSE)     // decode config buffer (opaque data)
    {
        mReconfigOutputPortBufferCount = mOutputPortDef.nBufferCountActual;
        mReconfigOutputPortBufferSize = mOutputPortDef.nBufferSize;

        if ( (pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) && (pInputBuf->nFilledLen == 0))
        {
            MTK_OMX_LOGD(" %s EOS before init %lld", GetVDECSrting(mCodecId, NULL), pInputBuf->nTimeStamp);

            // return the EOS output buffer
            //pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
            //pOutputBuf->nFilledLen = 0;
            //pOutputBuf->nTimeStamp = 0;
            //HandleFillBufferDone(pOutputBuf, mRealCallBackFillBufferDone);
            if (mInputFlushALL != OMX_TRUE)
            {
                MTK_OMX_LOGD("wait one output buffer for EOS %d", get_sem_value(&mOutputBufferSem));
                WAIT_T(mOutputBufferSem);
            }

            int output_idx = DequeueOutputBuffer();

            if (output_idx >= 0)
            {
                OMX_BUFFERHEADERTYPE *pOutputBuf = mOutputBufferHdrs[output_idx];
                pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
                pOutputBuf->nFilledLen = 0;
                pOutputBuf->nTimeStamp = 0;
                HandleFillBufferDone(pOutputBuf, OMX_FALSE);
            }

            // return the EOS input buffer
            HandleEmptyBufferDone(pInputBuf);

            MTK_OMX_LOGD("mNumPendingInput(%d), mNumPendingOutput(%d)", (int)mNumPendingInput, (int)mNumPendingOutput);
            DumpFTBQ();
            return OMX_TRUE;
        }

        OMX_S32 iDisplayWidth;
        OMX_S32 iDisplayHeight;
        OMX_S32 iAspectRatioWidth;
        OMX_S32 iAspectRatioHeight;
        if (OMX_FALSE == InitVideoDecodeHW(&iDisplayWidth, &iDisplayHeight, &iAspectRatioWidth, &iAspectRatioHeight, aInputBuf, &aInputSize))
        {
            mFailInitCounter++;
            if (OMX_TRUE == mViLTESupportOn)
            {
                if (OMX_TRUE == IsAVPFEnabled())
                {
                    mFATALError = HandleViLTEInitHWErr();
                }
                else
                {
                    MTK_OMX_LOGE("InitVideoDecodeHW failed and retry");
                }
            }
            else
            {
                if(mFailInitCounter >= 30)
                {
                    mFATALError = OMX_TRUE;
                    MTK_OMX_LOGE("InitVideoDecodeHW failed over %d times",mFailInitCounter);
                }
                else
                {
                    MTK_OMX_LOGE("InitVideoDecodeHW failed and retry");
                }
            }
            HandleEmptyBufferDone(pInputBuf);
            //pOutputBuf->nFilledLen = 0;
            //pOutputBuf->nTimeStamp = 0;
            //HandleFillBufferDone(pOutputBuf, mRealCallBackFillBufferDone);
            if (mFATALError == OMX_TRUE)
            {
                if (mKernelLogCntChg == OMX_TRUE)
                {
                    MTK_OMX_LOGE("fatal error detect, set kernel log detect count to original");
                    VAL_BOOL_T fgIncCount = VAL_FALSE;
                    eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_KERNEL_LOG_COUNT, (VAL_VOID_T *)&fgIncCount, NULL);
                    mKernelLogCntChg = OMX_FALSE;
                }

                // report error bad parameter
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorBadParameter,
                                       NULL,
                                       NULL);
                MTK_OMX_LOGE("InitVideoDecodeHW failed (not compatible");
                return OMX_FALSE;
            }
            else
            {
                if(mKernelLogCntChg == OMX_FALSE)
                {
                    MTK_OMX_LOGE("first time init HW fail, set kernel log detect count to larger");
                    VAL_BOOL_T fgIncCount = VAL_TRUE;
                    eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_KERNEL_LOG_COUNT, (VAL_VOID_T *)&fgIncCount, NULL);
                    mKernelLogCntChg = OMX_TRUE;
                }

                return OMX_TRUE;
            }
        }
        UpdateColorAspectsParams();
        MTK_OMX_LOGU("[DECODE INFO] InitHW OK iDisplayWidth(%u), iDisplayHeight(%u), nFrameWidth(%u), nFrameHeight(%u), nStride(%u), nSliceHeight(%u), u4Width(%u), u4Height(%u), u4StrideAlign(%u), u4SliceHeightAlign(%u), luma&chroma(%u, %u)",
                     (unsigned int)iDisplayWidth, (unsigned int)iDisplayHeight, (unsigned int)mOutputPortDef.format.video.nFrameWidth, (unsigned int)mOutputPortDef.format.video.nFrameHeight,
                     (unsigned int)mOutputPortDef.format.video.nStride, (unsigned int) mOutputPortDef.format.video.nSliceHeight, mSeqInfo.u4Width, mSeqInfo.u4Height, mQInfoOut.u4StrideAlign,
                     mQInfoOut.u4SliceHeightAlign, mSeqInfo.u4BitDepthLuma, mSeqInfo.u4BitDepthChroma);
        mNeedDecodeWhenFlush = true;
        mDecoderInitCompleteFlag = OMX_TRUE;

        if (OMX_TRUE == IsAVPFEnabled() &&
            OMX_TRUE == mFIRSent)
        {
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                mAppData,
                                OMX_EventError,
                                OMX_ErrorFullIntraRequestEnd,
                                NULL,
                                NULL);
        }

        if (mKernelLogCntChg == OMX_TRUE)
        {
            MTK_OMX_LOGE("init HW complete, set kernel detect count to original");
            VAL_BOOL_T fgIncCount = VAL_FALSE;
            eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_KERNEL_LOG_COUNT, (VAL_VOID_T *)&fgIncCount, NULL);
            mKernelLogCntChg = OMX_FALSE;
        }

        if(mSeqInfo.u4BitDepthLuma == 10 || mSeqInfo.u4BitDepthChroma == 10)
        {
            mbIs10Bit = OMX_TRUE;
            mIsHorizontalScaninLSB = mSeqInfo.bIsHorizontalScaninLSB;

            if (mMJCCreated == VAL_TRUE)
            {
                MJCSetBufFormat();
            }
        }
        // We have to do port re-config,
        // for ACodec crop setting fail due to width > frame-width or height > frame-height.
        if (mLegacyMode && (iDisplayWidth*iDisplayHeight > mMaxWidth*mMaxHeight ||
                            iDisplayWidth > mMaxWidth || iDisplayHeight > mMaxHeight))
        {
            //resolution is larger than legacy mode max resolution, cancel legacy mode
            mLegacyMode = OMX_FALSE;
            mCropWidth = iDisplayWidth;
            mCropHeight = iDisplayHeight;
            MTK_OMX_LOGD("max w/h=%dx%d, new resolution=%dx%d, cancel legacy mode(%d)",
                    mMaxWidth, mMaxHeight, iDisplayWidth, iDisplayHeight, mLegacyMode);
        }

        if (mHDRVideoSupportOn && (mSeqInfo.bHDRColorDesc || IsHDRSetByFramework()) )
        {
            mIsHDRVideo = OMX_TRUE;
            mColorDesc.u4ColorPrimaries = VIDEO_HDR_COLOR_PRIMARIES_INVALID;

            if (mMJCEnable == OMX_TRUE)
            {
                MJC_HDRVideoInfo HDRinfoParams;
                HDRinfoParams.isHDRVideo = mIsHDRVideo;
                HDRinfoParams.u4Width = mOutputPortDef.format.video.nFrameWidth;
                HDRinfoParams.u4Height = mOutputPortDef.format.video.nFrameHeight;
                m_fnMJCSetParam(mpMJC, MJC_PARAM_IS_HDRVIDEO, &HDRinfoParams);
            }
        }

        // check if output port setting changes
        OMX_S32 isPortSettingsChanged = 0;
        if ((((mCodecProperty & VDEC_CODEC_SUPPORT_DPB_SIZE) != 0 && mThumbnailMode == OMX_FALSE)
            || (OMX_TRUE == mbIs10Bit)
            || ((OMX_U32)iDisplayWidth != mOutputPortDef.format.video.nFrameWidth)
            || ((OMX_U32)iDisplayHeight != mOutputPortDef.format.video.nFrameHeight)
            || mOutputPortDef.format.video.nStride != VDEC_ROUND_N(mSeqInfo.u4Width, mQInfoOut.u4StrideAlign)
            || mOutputPortDef.format.video.nSliceHeight != VDEC_ROUND_N(mSeqInfo.u4Height, mQInfoOut.u4SliceHeightAlign)
//#ifdef MTK_CLEARMOTION_SUPPORT
            || (mMJCEnable == OMX_TRUE && mMJCScalerByPassFlag == OMX_FALSE))
//#endif
            && !mLegacyMode
           )
        {
            mOutputPortDef.format.video.nFrameWidth = iDisplayWidth;
            mOutputPortDef.format.video.nFrameHeight = iDisplayHeight;
            mOutputPortDef.format.video.nStride = VDEC_ROUND_N(mSeqInfo.u4Width, mQInfoOut.u4StrideAlign);
            mOutputPortDef.format.video.nSliceHeight = VDEC_ROUND_N(mSeqInfo.u4Height, mQInfoOut.u4SliceHeightAlign);
            mInputPortDef.format.video.nFrameWidth = mOutputPortDef.format.video.nFrameWidth;
            mInputPortDef.format.video.nFrameHeight = mOutputPortDef.format.video.nFrameHeight;

            if (meDecodeType != VDEC_DRV_DECODER_MTK_SOFTWARE)
            {
                mReconfigOutputPortBufferSize = (mOutputPortDef.format.video.nStride * mOutputPortDef.format.video.nSliceHeight * 3 >> 1) + 16;
            }
            else
            {
                mReconfigOutputPortBufferSize = (mOutputPortDef.format.video.nStride * (mOutputPortDef.format.video.nSliceHeight + 1) * 3) >> 1;
            }

            if (OMX_TRUE == mbIs10Bit)
            {
                mReconfigOutputPortBufferSize *= 1.25;
            }
            if ((mCodecProperty & VDEC_CODEC_SUPPORT_DPB_SIZE) != 0)
            {
                //always use DPBsize for secure meta mode.
                if (VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_QUERY_VIDEO_DPB_SIZE, NULL, &mDPBSize))
                {
                    MTK_OMX_LOGE("[ERROR] Cannot get param: VDEC_DRV_GET_TYPE_QUERY_VIDEO_DPB_SIZE");
                    HandleEmptyBufferDone(pInputBuf);
                    //pOutputBuf->nFilledLen = 0;
                    //pOutputBuf->nTimeStamp = 0;
                    //HandleFillBufferDone(pOutputBuf, mRealCallBackFillBufferDone);
                    mNeedDecodeWhenFlush = false;
                    return OMX_FALSE;
                }

                MTK_OMX_LOGD("@@ mDPBSize:%d", mDPBSize);

                if (mThumbnailMode == OMX_FALSE)
                {
                    int32_t MaxDPBNumber, ActualOutBuffNums, ActualOutBuffSize;

                    MaxDPBNumber = mDPBSize;

                    if (mLegacyMode == OMX_TRUE)
                    {
                        MaxDPBNumber = 16;
                    }
                    ActualOutBuffNums = MaxDPBNumber + mMinUndequeuedBufs + FRAMEWORK_OVERHEAD; // for some HEVC baseline with other nal headers

                    if (mMinUndequeuedBufsFlag == OMX_TRUE)
                    {
                        mMinUndequeuedBufsFlag = OMX_FALSE;
                        MTK_OMX_LOGD("[Debug] avoid to re-count mMinUndequeuedBufsDiff\n");
                    }

//#ifdef MTK_CLEARMOTION_SUPPORT
                    if (mMJCEnable == OMX_TRUE)
                    {
                    MJC_MODE mMode;
                    if ((mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight) > (1920 * 1088))
                    {
                        if (meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE)
                        {
                            ActualOutBuffNums += (MTK_MJC_REF_VDEC_OUTPUT_BUFFER_COUNT);
                        }
                        mUseClearMotion = OMX_FALSE;
                        mMode = MJC_MODE_BYPASS;
                    }
                    else
                    {
                        if ((mMJCReconfigFlag == OMX_TRUE) && ((ActualOutBuffNums + TOTAL_MJC_BUFFER_CNT) <= MAX_TOTAL_BUFFER_CNT))
                        {
                            ActualOutBuffNums += TOTAL_MJC_BUFFER_CNT;
                            mUseClearMotion = OMX_TRUE;
                            mMode = MJC_MODE_NORMAL;
                        }
                        else
                        {
                            mUseClearMotion = OMX_FALSE;
                            mMode = MJC_MODE_BYPASS;
                        }
                    }

                    if (mMJCReconfigFlag == OMX_TRUE)
                    {
                        m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
                        m_fnMJCInit(mpMJC, mUseClearMotion);
                    }
                    MTK_OMX_LOGD("mMJCReconfigFlag = %d  mUseClearMotion = %d\n", mMJCReconfigFlag, mUseClearMotion);
                    }
//#endif
                    mReconfigOutputPortBufferCount = ActualOutBuffNums;
                    mOutputPortDef.nBufferCountMin = ActualOutBuffNums - mMinUndequeuedBufs;

                }
                mOutputPortDef.nBufferCountActual = mReconfigOutputPortBufferCount; //Buffer slim: Increased buffer count can actually update portDef, prevent bIsHEVC_ETBQcontainsPPS port reconfig twice.
                MTK_OMX_LOGD("--- nBufferCountActual: %d;  mDPBSize: %d---", mReconfigOutputPortBufferCount , mDPBSize);
            }

            mPortReconfigInProgress = OMX_TRUE;
            isPortSettingsChanged = 1;
        }
        else if (mLegacyMode)
        {
            mCropWidth = iDisplayWidth;
            mCropHeight = iDisplayHeight;
            mPicWidth = iDisplayWidth;
            mPicHeight = iDisplayHeight;
            MTK_OMX_LOGD("update crop info after driver init %d %d", mCropWidth, mCropHeight);
        }
        else
        {
//#ifdef MTK_CLEARMOTION_SUPPORT
            if (mMJCEnable == OMX_TRUE)
            {
            MJCSetBufRes();
            }
//#endif
        }
        //pOutputBuf->nFilledLen = 0;
        //pOutputBuf->nTimeStamp = 0;
        //TODO : MPEG2, H.263 in MPEG4 needs to queue first input buffer ?
        QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
        mNeedDecodeWhenFlush = false;
        //QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));
        SIGNAL(mDecodeSem);
        //HandleFillBufferDone(pOutputBuf, mRealCallBackFillBufferDone);

        VAL_UINT32_T  FrameInterval;
        if (VDEC_DRV_MRESULT_OK == eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_GET_FRAME_INTERVAL, NULL, &FrameInterval))
        {
            mFrameInterval = FrameInterval;
            MTK_OMX_LOGD("Frame Interval = 0x%llX", mFrameInterval);
        }
        else
        {
            mFrameInterval = 0;
            MTK_OMX_LOGUD("[WARNING] query VDEC_DRV_GET_TYPE_GET_FRAME_INTERVAL failed");
        }

//#ifdef MTK_CLEARMOTION_SUPPORT
        if (1)
        {
            // For HEVC buffer align and crop info +
            VDEC_DRV_CROPINFO_T temp_ccop_info;
            VAL_UINT32_T ret;

            ret = eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_GET_FRAME_CROP_INFO, NULL, &temp_ccop_info);
            if (VDEC_DRV_MRESULT_OK == ret)
            {
                mCropLeft = temp_ccop_info.u4CropLeft;
                mCropTop = temp_ccop_info.u4CropTop;
                mCropWidth = temp_ccop_info.u4CropRight - temp_ccop_info.u4CropLeft + 1;
                mCropHeight = temp_ccop_info.u4CropBottom - temp_ccop_info.u4CropTop + 1;
            }
            else
            {
                mCropLeft = 0;
                mCropTop = 0;
                mCropWidth = mOutputPortDef.format.video.nFrameWidth;
                mCropHeight = mOutputPortDef.format.video.nFrameHeight;
                if (mLegacyMode)
                {
                    // When SPS without crop info, crop is PicWidth/Height.
                    mCropWidth = iDisplayWidth;
                    mCropHeight = iDisplayHeight;
                }
            }

            MTK_OMX_LOGD("GetCropInfo() mCropLeft %d, mCropTop %d, mCropWidth %d, mCropHeight %d\n", mCropLeft, mCropTop, mCropWidth, mCropHeight);

            if (mMJCEnable == OMX_TRUE)
            {
                MJC_VIDEORESOLUTION mMJC_FrmRes;
                mMJC_FrmRes.u4Width = mOutputPortDef.format.video.nFrameWidth;
                mMJC_FrmRes.u4Height = mOutputPortDef.format.video.nFrameHeight;
                m_fnMJCSetParam(mpMJC, MJC_PARAM_FRAME_RESOLUTION, &mMJC_FrmRes);

                MJC_VIDEORESOLUTION rAlignment;
                MJC_VIDEOCROP rVideoCrop;

                rVideoCrop.mCropLeft = mCropLeft;
                rVideoCrop.mCropTop = mCropTop;
                rVideoCrop.mCropWidth = mCropWidth;
                rVideoCrop.mCropHeight = mCropHeight;

                rAlignment.u4Width = mQInfoOut.u4StrideAlign;
                rAlignment.u4Height = mQInfoOut.u4SliceHeightAlign;
                m_fnMJCSetParam(mpMJC, MJC_PARAM_ALIGH_SIZE, &rAlignment);
                m_fnMJCSetParam(mpMJC, MJC_PARAM_CROP_INFO, &rVideoCrop);

#if (ANDROID_VER >= ANDROID_KK)
                //mMJCScalerByPassFlag = OMX_TRUE;  //[ToDo]
                if (mMJCScalerByPassFlag == OMX_FALSE)
                {
                    MJC_VIDEORESOLUTION mMJC_BufRes;
                    m_fnMJCGetParam(mpMJC, MJC_PARAM_ALIGN_RESOLTUION, &mMJC_BufRes);
                    mCropLeft = 0;
                    mCropTop = 0;
                    mCropWidth = mMJC_BufRes.u4Width;
                    mCropHeight = mMJC_BufRes.u4Height;
                    MTK_OMX_LOGD("MJC Scaler mCropWidth : %d , mCropHeight : %d, LINE:%d", mCropWidth, mCropHeight, __LINE__);
                }
#endif

                // For Scaler ClearMotion +
                mOriFrameWidth = mOutputPortDef.format.video.nFrameWidth;
                mOriFrameHeight = mOutputPortDef.format.video.nFrameHeight;
                MJCScalerCheckandSetBufInfo();
                // For Scaler ClearMotion -
            }

            if (isPortSettingsChanged == 1)
            {
                MTK_OMX_LOGU("--- OMX_EventPortSettingsChanged ---");
                if ((iAspectRatioWidth != 1 || iAspectRatioHeight != 1) && iAspectRatioWidth != 0 && iAspectRatioHeight != 0)
                {
                    MTK_OMX_LOGD("--- OMX_EventPortSettingsChanged (with Aspect ratio)--- %i %i", iAspectRatioWidth, iAspectRatioHeight);
                    mAspectRatioWidth = iAspectRatioWidth;
                    mAspectRatioHeight = iAspectRatioHeight;
                    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                           mAppData,
                                           OMX_EventPortSettingsChanged,
                                           MTK_OMX_OUTPUT_PORT,
                                           OMX_IndexVendorMtkOmxVdecGetAspectRatio,
                                           NULL);
                }
                else
                {
                    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                           mAppData,
                                           OMX_EventPortSettingsChanged,
                                           MTK_OMX_OUTPUT_PORT,
                                           NULL,
                                           NULL);
                }
            }

            if ((ret == VDEC_DRV_MRESULT_OK || mMJCScalerByPassFlag == OMX_FALSE) && isPortSettingsChanged == 0)
            {
                //set crop info
                MTK_OMX_LOGD("--- OMX_EventPortSettingsChanged  (with Crop info)---");
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventPortSettingsChanged,
                                       MTK_OMX_OUTPUT_PORT,
                                       OMX_IndexVendorMtkOmxVdecGetCropInfo,
                                       NULL);
            }
        }
//#endif

        bRet = OMX_TRUE;
    }
    else if (mFATALError == OMX_TRUE)
    {
        HandleEmptyBufferDone(pInputBuf);
        //pOutputBuf->nFilledLen = 0;
        //pOutputBuf->nTimeStamp = 0;
        //HandleFillBufferDone(pOutputBuf, mRealCallBackFillBufferDone);
        bRet = OMX_TRUE;
    }
    else
    {
        if (pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)    // last frame
        {
            //MTK_OMX_LOGD("(%s) EOS Len (%d) (0x%X, 0x%X)", GetVDECSrting(mCodecId, NULL), (unsigned int)pInputBuf->nFilledLen, (unsigned int)pInputBuf, (unsigned int)pOutputBuf);
            MTK_OMX_LOGD("(%s) %lld, EOS Len (%d) (0x%X)", GetVDECSrting(mCodecId, NULL), pInputBuf->nTimeStamp, (unsigned int)pInputBuf->nFilledLen, (unsigned int)pInputBuf);
            if (pInputBuf->nFilledLen == 0)
            {
                OMX_U32 i;
                //for N ExtractDecodeEditEncodeMuxTest#testExtractDecodeEditEncodeMuxXXXX
                //input EOS with length = 0 and ts = 0, FBD with EOS after FlushDecoder()
                MTK_OMX_LOGD("EOS !!!%lld %lld", mEOSTS, pInputBuf->nTimeStamp);

                // flush decoder
                FlushDecoder(OMX_TRUE);

                // return the EOS output buffer
                //pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
                //pOutputBuf->nFilledLen = 0;
                //pOutputBuf->nTimeStamp = 0;
                //HandleFillBufferDone(pOutputBuf, mRealCallBackFillBufferDone);

                // return the EOS input buffer
                HandleEmptyBufferDone(pInputBuf);

                {
                    if (mInputFlushALL != OMX_TRUE)
                    {
                        MTK_OMX_LOGD("wait one output buffer for EOS %d", get_sem_value(&mOutputBufferSem));
                        WAIT_T(mOutputBufferSem);
                    }

                    int output_idx = DequeueOutputBuffer();

                    if (output_idx >= 0)
                    {
                        OMX_BUFFERHEADERTYPE *pOutputBuf = mOutputBufferHdrs[output_idx];
                        pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
                        pOutputBuf->nFilledLen = 0;
                        pOutputBuf->nTimeStamp = 0;
                        HandleFillBufferDone(pOutputBuf, OMX_FALSE);
                    }
                }

                MTK_OMX_LOGD("mNumPendingInput(%d), mNumPendingOutput(%d)", (int)mNumPendingInput, (int)mNumPendingOutput);
                DumpFTBQ();
                return OMX_TRUE;
            }
            else        // Morris for CTS test (early EOS)
            {
                mEarlyEOS = OMX_TRUE;
                mEOSTS = -1;
                if (1/*TODO : sort PTS*/)
                {
                    OMX_U32 i;
                    for (i = 0; i < iTSIn; i++)
                    {
                        if (DisplayTSArray[i] > mEOSTS)
                        {
                            mEOSTS = DisplayTSArray[i];
                        }
                    }
                    mEOSTS = (pInputBuf->nTimeStamp > mEOSTS) ? pInputBuf->nTimeStamp : mEOSTS;
                    if (mEOSTS == 0)  //for CTS VideoEncoderDecoderTest all timestamp = 0 case
                    {
                        //pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
                    }
                }
                else
                {
                    //pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
                }
                MTK_OMX_LOGD("early EOS !!!%lld %lld", mEOSTS, pInputBuf->nTimeStamp);
                // normal decode flow
            }
        }

        if ((meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE) &&
            (mPropFlags & MTK_OMX_VDEC_ENABLE_PRIORITY_ADJUSTMENT))
        {
            if (OMX_FALSE == mCodecTidInitialized)
            {
                VAL_VCODEC_THREAD_ID_T _thread_info;
                memset((void *)&_thread_info, 0x0, sizeof(VAL_VCODEC_THREAD_ID_T));
                if (VDEC_DRV_MRESULT_OK == eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_GET_CODEC_TIDS, NULL, &_thread_info))
                {
                    mNumCodecThreads = _thread_info.u4VCodecThreadNum;
                    for (int i = 0 ; i < _thread_info.u4VCodecThreadNum ; i++)
                    {
                        if (i >= 32)
                        {
                            //mCodecTids array max size is 32
                            MTK_OMX_LOGE("[%s] Warrning!! The mCodecTids array is too small", __func__);
                            break;
                        }
                        mCodecTids[i] = pthread_gettid_np(_thread_info.u4VCodecThreadID[i]);
                        struct sched_param sched_p;
                        if (0 != sched_getparam(mCodecTids[i], &sched_p))
                        {
                            MTK_OMX_LOGE("[%s] failed, errno: %d", __func__, errno);
                        }

                        if (mCurrentSchedPolicy == SCHED_RR)
                        {
                            sched_p.sched_priority = RT_THREAD_PRI_OMX_VIDEO;//RTPM_PRIO_OMX_VIDEO; //RTPM_PRIO_OMX_VIDEO_DECODE; // RT Priority
                        }
                        else    //SCHED_NORMAL
                        {
                            sched_p.sched_priority = 0;
                        }

                        if (0 != sched_setscheduler(mCodecTids[i], mCurrentSchedPolicy, &sched_p))
                        {
                            MTK_OMX_LOGE("[%s] failed, errno: %d", __func__, errno);
                        }
                    }
                }

                mCodecTidInitialized = OMX_TRUE;
            }
        }

        if (aInputSize == 0)
        {
            MTK_OMX_LOGD("Input size is 0");
            mNumSemaphoreCountForInput++;
            HandleEmptyBufferDone(pInputBuf);
            return OMX_TRUE;
        }

        if (OMX_FALSE == mNeedMoreOutputBuf)
        {
            if ((pInputBuf->nFilledLen + DEC_BS_PROTECT_MEM_SZ) > mInputPortDef.nBufferSize)
            {
                //MTK_OMX_LOGD("allocate larger input buffer ...");
                MTK_OMX_LOGE("[Warning] bitstream buffer size not enough: mInputPortDef.nBufferSize %d < pInputBuf->nFilledLen + bs_pt %d", mInputPortDef.nBufferSize, pInputBuf->nFilledLen + DEC_BS_PROTECT_MEM_SZ);
            }
        }

        //setup output buffer
        //frame = GetFrmBuf(pInputBuf->nTimeStamp, pOutputBuf);
        //if (NULL == frame)
        //{
        //    MTK_OMX_LOGE("GetFrmBuf Error !!!");
        //    return OMX_FALSE;
        //}

        GetBitstreamBuffer(aInputBuf, aInputSize);
        pRingBuf = GetInputBuf(pInputBuf);

        if (pRingBuf != NULL)
        {
            pRingBuf->rBase.u4VA = mRingbuf.rBase.u4VA;
            pRingBuf->rBase.u4PA = mRingbuf.rBase.u4PA;
            pRingBuf->rBase.u4Size = mRingbuf.rBase.u4Size;
            pRingBuf->rBase.hMemHandle = mRingbuf.rBase.hMemHandle;
            pRingBuf->u4Read = mRingbuf.u4Read;
            pRingBuf->u4Write = mRingbuf.u4Write;
            if (OMX_TRUE == mIsSecureInst){
                pRingBuf->rSecMemHandle = mRingbuf.rSecMemHandle;
            }
        }
        else
        {
            MTK_OMX_LOGE("[ERROR] no free input buffer");
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   OMX_ErrorBadParameter,
                                   NULL,
                                   NULL);
        }

#if PROFILING
        _in_time_2 = getTickCountMs();
#endif
        //MTK_OMX_LOGD("mPrepareSeek %d, mSeekMode %d, mSeekTargetTime %lld", mPrepareSeek, mSeekMode, mSeekTargetTime);
        if (OMX_TRUE == mPrepareSeek)
        {
            VDEC_DRV_SET_DECODE_MODE_T rtSetDecodeMode;
            rtSetDecodeMode.u4DisplayFrameNum = 0;
            rtSetDecodeMode.u4DropFrameNum = 0;
            rtSetDecodeMode.eDecodeMode = VDEC_DRV_DECODE_MODE_B_SKIP; // seek mode
            eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_DECODE_MODE, &rtSetDecodeMode, NULL);
            mSeekMode = OMX_TRUE;
            mPrepareSeek = OMX_FALSE;
            mOmxVdecLogEnable = OMX_FALSE;
            mOmxVdecPerfLogEnable = OMX_FALSE;
        }
        pRingBuf->u4InputFlag = 0;
        if (pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)
        {
            pRingBuf->u4InputFlag |= VDEC_DRV_INPUT_BUF_EOS;
        }
        if (pInputBuf->nFlags & OMX_BUFFERFLAG_CODECCONFIG)
        {
            if (mCodecId == MTK_VDEC_CODEC_ID_VP9)
            {
                MTK_OMX_LOGD("VP9 get CSD");
            }
            else
            {
                pRingBuf->u4InputFlag |= VDEC_DRV_INPUT_BUF_INIT_CONFIG_DATA;
            }
        }
        if (pInputBuf->nFlags & OMX_BUFFERFLAG_INVALID_TIMESTAMP)
        {
            pRingBuf->u4InputFlag |= VDEC_DRV_INPUT_BUF_INVALID_TIMESTAMP;
        }

        if (OMX_TRUE == IsAVPFEnabled())
        {
            if (pInputBuf->nFlags & OMX_BUFFERFLAG_DATACORRUPT) {
                pRingBuf->u4InputFlag |= VDEC_DRV_INPUT_BUF_DATACORRUPT;
                MTK_OMX_LOGD("[Error] intput buffer error from upstream\n");
            }
        }

        pRingBuf->u4Timestamp = pInputBuf->nTimeStamp;
        mEverCallback = OMX_FALSE;
        mpCurrInput = pInputBuf;
        mCBTime = 0;

        //ATRACE_BEGIN("DecodeOneUnit");
        //VDEC_DRV_MRESULT_T rResult = eVDecDrvDecode(mDrvHandle, pRingBuf, frame);
        if(meDecodeType != VDEC_DRV_DECODER_MTK_HARDWARE)
        {
            OMX_U32 outputBufIdx = mFillThisBufQ.size();
            //MTK_OMX_LOGD("outputBufIdx %d",outputBufIdx);
            eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_OUTPUT_BUF_INDEX, (VAL_VOID_T *)&outputBufIdx, NULL);
        }
        VDEC_DRV_MRESULT_T rResult = eVDecDrvDecodeEx(mDrvHandle, pRingBuf, 0, NULL);
        //ATRACE_END();

        dumpInputBuffer(pInputBuf, aInputBuf, aInputSize);

        //MTK_OMX_LOGD("Seek mode %d, target time %lld, input time %lld", mSeekMode, mSeekTargetTime, pInputBuf->nTimeStamp);
        if ((mSeekMode == OMX_TRUE) && (mSeekTargetTime > 0))
        {
            if (pInputBuf->nTimeStamp >= mSeekTargetTime)
            {
                CheckLogEnable();
                MTK_OMX_LOGUD("SDone, %lld", pInputBuf->nTimeStamp);
                mSeekMode = OMX_FALSE;
//#ifdef MTK_CLEARMOTION_SUPPORT
                if (mMJCEnable == OMX_TRUE)
                {
                    bool mSeek;
                    mSeek = false;
                    m_fnMJCSetParam(mpMJC, MJC_PARAM_SEEK, &mSeek);
                }
//#endif
                VDEC_DRV_SET_DECODE_MODE_T rtSetDecodeMode;
                rtSetDecodeMode.u4DisplayFrameNum = 0;
                rtSetDecodeMode.u4DropFrameNum = 0;
                rtSetDecodeMode.eDecodeMode = VDEC_DRV_DECODE_MODE_NORMAL; // normal mode
                eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_DECODE_MODE, &rtSetDecodeMode, NULL);
            }
        }

        mNeedMoreOutputBuf = OMX_FALSE;
        if (VDEC_DRV_MRESULT_FATAL == rResult)
        {
            MTK_OMX_LOGE("## DECODE ERROR FATAL!!!");
            // report bitstream corrupt error
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   OMX_ErrorBadParameter,
                                   NULL,
                                   NULL);
            mFATALError = OMX_TRUE;
        }
        else if (VDEC_DRV_MRESULT_NEED_MORE_OUTPUT_BUFFER == rResult)
        {

#if PROFILING
            _out_time = getTickCountMs();
            MTK_OMX_LOGD("%s, %lld, 0x%X %d (0x%X), (%lld ms, %lld ms, %lld ms), %d (NEED_MORE_OUTPUT_BUFFER)",
                         GetVDECSrting(mCodecId, (void *)pRingBuf->u4Read), pInputBuf->nTimeStamp, (unsigned int)pInputBuf->nFlags, pRingBuf->u4Write - pRingBuf->u4Read, (unsigned int)pInputBuf, _out_time - _in_time_2 - mCBTime, _out_time - _in_time_1 - mCBTime, mCBTime, mCurrentSchedPolicy);

            mFramesDecoded++;
            mTotalDecodeTime += (_out_time - _in_time_2);
#else
            MTK_OMX_LOGD("DECODE OK (NEED_MORE_OUTPUT_BUFFER)");
#endif
            QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
            MarkInputBufferUnused(pInputBuf);
            SIGNAL(mDecodeSem);
            mNeedMoreOutputBuf = OMX_TRUE;

            if(mCodecId == MTK_VDEC_CODEC_ID_AVC)
            {
                if(VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_PROFILEORLEVELCHANGED, NULL, &bCodecProfileOrLevelChanged))
                {
                    MTK_OMX_LOGD("[INFO] Cannot get param: VDEC_DRV_GET_TYPE_PROFILEORLEVELCHANGED");
                }
            }
        }
        else if (!(VDEC_DRV_MRESULT_OK == rResult || VDEC_DRV_MRESULT_RESOLUTION_CHANGED == rResult))
        {
            MTK_OMX_LOGE("## DECODE ERROR !!!");
        }
        else
        {
#if PROFILING
            _out_time = getTickCountMs();
            if (!(mSeekMode == OMX_TRUE && mSeekTargetTime > 0)) {
            MTK_OMX_LOGUD("%s (%d), %lld, 0x%X %d (0x%X), (%lld ms, %lld ms, %lld ms), %d",
                         GetVDECSrting(mCodecId, (void *)pRingBuf->u4Read), rResult, pInputBuf->nTimeStamp, (unsigned int)pInputBuf->nFlags, pRingBuf->u4Write - pRingBuf->u4Read, (unsigned int)pInputBuf,  _out_time - _in_time_2 - mCBTime, _out_time - _in_time_1 - mCBTime, mCBTime, mCurrentSchedPolicy);
            }
#else
            MTK_OMX_LOGD("DECODE OK");
#endif

            mFramesDecoded++;
            mTotalDecodeTime += (_out_time - _in_time_2);

            if (mDeInterlaceEnable)
            {
                if ((mInterlaceChkComplete == OMX_FALSE) && (mIsSecureInst != OMX_TRUE) && (mThumbnailMode != OMX_TRUE))
                {
                    VAL_UINT32_T u32VideoInteraceing  = 0;
                    if (VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_QUERY_VIDEO_INTERLACING, NULL, &u32VideoInteraceing))
                    {
                        //MTK_OMX_LOGD("VDEC_DRV_GET_TYPE_QUERY_VIDEO_INTERLACING not support");
                        u32VideoInteraceing = 0;
                    }
                    mIsInterlacing = (VAL_BOOL_T)u32VideoInteraceing;
                    //MTK_OMX_LOGD("mIsInterlacing %d", mIsInterlacing);
                    if (meDecodeType != VDEC_DRV_DECODER_MTK_SOFTWARE && mIsInterlacing == OMX_TRUE)
                    {
                        if (OMX_TRUE == mMJCEnable)
                        {
                            unsigned int mMJC_RunTimeDis = OMX_TRUE;
                            m_fnMJCSetParam(mpMJC, MJC_PARAM_RUNTIME_DISABLE, &mMJC_RunTimeDis);
                        }

                        mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_FCM;
                        mOutputPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_FCM;
                        mInterlaceChkComplete = OMX_TRUE;
                        mUseClearMotion = OMX_FALSE;
                        rResult = VDEC_DRV_MRESULT_RESOLUTION_CHANGED; // to go port reconfig flow.
                    }
                }
            }

            if(mCodecId == MTK_VDEC_CODEC_ID_AVC)
            {
                if(VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_PROFILEORLEVELCHANGED, NULL, &bCodecProfileOrLevelChanged))
                {
                    //MTK_OMX_LOGE("[INFO] Cannot get param: VDEC_DRV_GET_TYPE_PROFILEORLEVELCHANGED");
                }
            }
        }

        if (VDEC_DRV_MRESULT_RESOLUTION_CHANGED == rResult)
        {
            // handle settings changed during decode time
            VDEC_DRV_PICINFO_T rPicInfo;
            VAL_UINT32_T u4ChagnedDPBSize = 0;
            VAL_UINT32_T u4CheckBufferCount = 0;
            if (VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_GET_PICTURE_INFO, NULL, &rPicInfo))
            {
                MTK_OMX_LOGE("[ERROR] Cannot get param: VDEC_DRV_GET_TYPE_GET_PICTURE_INFO");
                return OMX_FALSE;
            }
            if ((mCodecProperty & VDEC_CODEC_SUPPORT_DPB_SIZE) != 0)
            {
                //always use DPBsize for secure meta mode
                if (VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_QUERY_VIDEO_DPB_SIZE, NULL, &u4ChagnedDPBSize))
                {
                    MTK_OMX_LOGE("[ERROR] Cannot get param: VDEC_DRV_GET_TYPE_QUERY_VIDEO_DPB_SIZE");
                    return OMX_FALSE;
                }

//#ifdef MTK_CLEARMOTION_SUPPORT
                if (mMJCEnable == OMX_TRUE)
                {
                u4CheckBufferCount = (mUseClearMotion == OMX_TRUE) ?
                                     (u4ChagnedDPBSize + TOTAL_MJC_BUFFER_CNT + mMinUndequeuedBufs + FRAMEWORK_OVERHEAD) :
                                     (u4ChagnedDPBSize + mMinUndequeuedBufs + FRAMEWORK_OVERHEAD);
                }
                else
                {
//#else
                u4CheckBufferCount = u4ChagnedDPBSize + mMinUndequeuedBufs + FRAMEWORK_OVERHEAD;
                }
//#endif
            }
            else
            {
                u4CheckBufferCount = mOutputPortDef.nBufferCountActual;
            }

            if (mHDRVideoSupportOn && (rPicInfo.bHDRColorDesc || IsHDRSetByFramework()) )
            {
                mIsHDRVideo = OMX_TRUE;
                mColorDesc.u4ColorPrimaries = VIDEO_HDR_COLOR_PRIMARIES_INVALID;
            }

            //MTK_OMX_LOGD ("--- !! Debug !! --- frame (%d %d %d) -> (%d %d %d) mpMJC->mMode: %d", mOriFrameWidth, mOriFrameHeight, mDPBSize, rPicInfo.u4Width, rPicInfo.u4Height, u4ChagnedDPBSize, mpMJC->mMode);
            //MTK_OMX_LOGD ("--- !! Debug !! --- mem (%d %d) -> (%d %d) mMJCScalerByPassFlag: %d", mOutputPortDef.format.video.nStride, mOutputPortDef.format.video.nSliceHeight, rPicInfo.u4RealWidth, rPicInfo.u4RealHeight, mMJCScalerByPassFlag);
            //MTK_OMX_LOGD ("nBufferCountActual: %d %d", mOutputPortDef.nBufferCountActual , mDPBSize);

            bool bResolutionChangeCheck;

//#ifdef MTK_CLEARMOTION_SUPPORT
            if (mMJCEnable == OMX_TRUE)
            {
            if (mMJCScalerByPassFlag == OMX_FALSE)
            {
                bResolutionChangeCheck = ((rPicInfo.u4Width != mOriFrameWidth) || (rPicInfo.u4Height != mOriFrameHeight));
                MTK_OMX_LOGD("bResolutionChangeCheck 1 (%d %d) -> (%d %d)", rPicInfo.u4Width, rPicInfo.u4Height, mOriFrameWidth, mOriFrameHeight);
            }
            else
            {
                bResolutionChangeCheck = ((rPicInfo.u4RealWidth != mOutputPortDef.format.video.nStride) || (rPicInfo.u4RealHeight != mOutputPortDef.format.video.nSliceHeight));
                MTK_OMX_LOGD("bResolutionChangeCheck 2 (%d %d) -> (%d %d)", rPicInfo.u4RealWidth, rPicInfo.u4RealHeight, mOutputPortDef.format.video.nStride, mOutputPortDef.format.video.nSliceHeight);
            }
            }
            else
            {
//#else
            bResolutionChangeCheck = ((rPicInfo.u4RealWidth != mOutputPortDef.format.video.nStride) || (rPicInfo.u4RealHeight != mOutputPortDef.format.video.nSliceHeight));
            MTK_OMX_LOGD("bResolutionChangeCheck 3 (%d %d) -> (%d %d)", rPicInfo.u4RealWidth, rPicInfo.u4RealHeight, mOutputPortDef.format.video.nStride, mOutputPortDef.format.video.nSliceHeight);
            }
//#endif

            if (VDEC_DRV_MRESULT_RESOLUTION_CHANGED == rResult
                || bResolutionChangeCheck
                || ((mOutputPortDef.nBufferCountActual < u4CheckBufferCount) && (mThumbnailMode == OMX_FALSE))
               )
            {
                if (mLegacyMode)
                {
                    //driver report resolution change, turn legacy mode off.
                    mLegacyMode = OMX_FALSE;
                }

                MTK_OMX_LOGD("--- OMX_EventPortSettingsChanged (2) --- (%d %d %d) -> (%d %d %d) [%d, %d] (%d, %d)",
                             mOutputPortDef.format.video.nFrameWidth, mOutputPortDef.format.video.nFrameHeight, mOutputPortDef.nBufferCountActual, rPicInfo.u4Width, rPicInfo.u4Height, u4CheckBufferCount,
                             bResolutionChangeCheck, mThumbnailMode, rPicInfo.u4BitDepthLuma, rPicInfo.u4BitDepthChroma);

                // for pass GTS ExoPlayer performance issue
                if (false == mFullSpeedOn)
                {
                    int on = 1;
                    mFullSpeedOn = true;
                    eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_FULL_SPEED, &on, NULL);
                }

                //for N migration, framework change the flow about poerSettingChange,
                //component need update latest portDefinition and crop info to framework during portSettingChange process
                //Merge "ACodec: move getPortFormat closer to PortSettingsChanged event" into nyc-dev
                mOutputPortDef.format.video.nFrameWidth = rPicInfo.u4Width;
                mOutputPortDef.format.video.nFrameHeight = rPicInfo.u4Height;
                mOutputPortDef.format.video.nStride = rPicInfo.u4RealWidth;
                mOutputPortDef.format.video.nSliceHeight = rPicInfo.u4RealHeight;
                mInputPortDef.format.video.nFrameWidth = mOutputPortDef.format.video.nFrameWidth;
                mInputPortDef.format.video.nFrameHeight = mOutputPortDef.format.video.nFrameHeight;

                if ((rPicInfo.u4BitDepthLuma == 10 || rPicInfo.u4BitDepthChroma == 10) && OMX_FALSE == mbIs10Bit)
                {
                    mbIs10Bit = OMX_TRUE;
                    mIsHorizontalScaninLSB = rPicInfo.bIsHorizontalScaninLSB;
                }

                if (mThumbnailMode == OMX_FALSE)
                {
                    mReconfigOutputPortBufferCount = u4CheckBufferCount;
                    mOutputPortDef.nBufferCountMin = u4CheckBufferCount - mMinUndequeuedBufs;
                }
                else
                {
                    mReconfigOutputPortBufferCount = mOutputPortDef.nBufferCountActual;
                }
                mDPBSize = u4ChagnedDPBSize;

                if (meDecodeType != VDEC_DRV_DECODER_MTK_SOFTWARE)
                {
                    mOutputPortDef.nBufferSize = (mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight * 3 >> 1) + 16;
                }
                else
                {
                    mOutputPortDef.nBufferSize = (mOutputPortDef.format.video.nFrameWidth * (mOutputPortDef.format.video.nFrameHeight + 1) * 3) >> 1;
                }

                if (OMX_TRUE == mbIs10Bit)
                {
                    mOutputPortDef.nBufferSize *= 1.25;
                }

                MTK_OMX_LOGD("--- OMX_EventPortSettingsChanged (2) --- (meDecodeType = %s, BufferSize = %d)", GetVDECSrting(mCodecId, NULL), mOutputPortDef.nBufferSize);

                mPortReconfigInProgress = OMX_TRUE;
                if (VDEC_DRV_MRESULT_OK != eVDecDrvDecodeEx(mDrvHandle, NULL, 0, NULL))
                {
                    MTK_OMX_LOGE("[ERROR] DecodeFinal failed");
                    //#ifdef MTK_CLEARMOTION_SUPPORT
                    if (mMJCEnable == OMX_TRUE)
                    {
                    MJCFlushAllBufQ();
                    }
                    //#endif
                    return OMX_FALSE;
                }

//#ifdef MTK_CLEARMOTION_SUPPORT
                // For HEVC buffer align and crop info +
                VDEC_DRV_CROPINFO_T temp_ccop_info;

                if (VDEC_DRV_MRESULT_OK == eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_GET_FRAME_CROP_INFO, NULL, &temp_ccop_info))
                {
                    mCropLeft = temp_ccop_info.u4CropLeft;
                    mCropTop = temp_ccop_info.u4CropTop;
                    mCropWidth = temp_ccop_info.u4CropRight - temp_ccop_info.u4CropLeft + 1;
                    mCropHeight = temp_ccop_info.u4CropBottom - temp_ccop_info.u4CropTop + 1;
                }
                else
                {
                    mCropLeft = 0;
                    mCropTop = 0;
                    mCropWidth = mOutputPortDef.format.video.nFrameWidth;
                    mCropHeight = mOutputPortDef.format.video.nFrameHeight;
                }

                MTK_OMX_LOGD("GetCropInfo() mCropLeft %d, mCropTop %d, mCropWidth %d, mCropHeight %d\n", mCropLeft, mCropTop, mCropWidth, mCropHeight);

//#endif
            }
        }
        else if(bCodecProfileOrLevelChanged)
        {
            MTK_OMX_LOGUD("bCodecProfileOrLevelChanged %d", bCodecProfileOrLevelChanged);
            //If we do flush when legacy mode && need_more_out_buffer, we will get warning for GetFreeInputBuffer
            //due to driver release the input we still keep.
            if (OMX_FALSE == mLegacyMode || VDEC_DRV_MRESULT_NEED_MORE_OUTPUT_BUFFER != rResult)
            {
                if (VDEC_DRV_MRESULT_OK != eVDecDrvDecodeEx(mDrvHandle, NULL, 0, NULL))
                {
                    MTK_OMX_LOGE("[ERROR] DecodeFinal failed");
                    //#ifdef MTK_CLEARMOTION_SUPPORT
                    if (mMJCEnable == OMX_TRUE)
                    {
                        MJCFlushAllBufQ();
                    }
                    //#endif
                    return OMX_FALSE;
                }
            }
        }

        if (OMX_FALSE == mPortReconfigInProgress && mFullSpeedOn)
        {
            int on = 0;
            mFullSpeedOn = false;
            eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_FULL_SPEED, &on, NULL);
        }

        //pOutputBuf->nFilledLen = mOutputPortDef.format.video.nStride * mOutputPortDef.format.video.nSliceHeight * 3 >> 1;
        //pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;
        //MTK_OMX_LOGD("output len %d, ts : %lld", pOutputBuf->nFilledLen, pOutputBuf->nTimeStamp);

        if (mEverCallback == OMX_TRUE)// && mNeedMoreOutputBuf != OMX_TRUE)
        {
            if (!(mPortReconfigInProgress || bCodecProfileOrLevelChanged))
            {
                MTK_OMX_LOGUD("insert PTS %lld", pInputBuf->nTimeStamp);
                if (InsertionSortForInputPTS(pInputBuf->nTimeStamp) == OMX_FALSE)
                {
                    MTK_OMX_LOGE("Insert PTS error");
                    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                           mAppData,
                                           OMX_EventError,
                                           OMX_ErrorOverflow,
                                           NULL,
                                           NULL);
                    HandleEmptyBufferDone(pInputBuf);
                    //pOutputBuf->nFilledLen = 0;
                    //pOutputBuf->nTimeStamp = 0;
                    //HandleFillBufferDone(pOutputBuf, mRealCallBackFillBufferDone);
                    return OMX_FALSE;
                }
            }
        }

        bEverOutput = OMX_TRUE;
        OMX_BUFFERHEADERTYPE *ipOutBuf = GetDisplayBuffer((mLegacyMode == OMX_TRUE) ? OMX_TRUE : OMX_FALSE);

        if (OMX_TRUE == IsAVPFEnabled())
        {
            HandleViLTEBufferDone(ipOutBuf);
        }

        while (ipOutBuf != NULL)
        {
            ipOutBuf->nOffset = 0;
            if (ipOutBuf->nTimeStamp != -1)
            {
                OMX_TICKS tempTS = RemoveForInputPTS();
                //for N migration, CTS EOSBehavior in 1st frame with ts =0
                //log: early EOS !!!0 0
                if( mEOSTS == tempTS )
                {
                    if( (mEOSTS != 0) || (OMX_TRUE == mEarlyEOS && mEOSTS == 0) )
                {
                    ipOutBuf->nFlags |= OMX_BUFFERFLAG_EOS;
                    mEOSFound = OMX_TRUE;
                }
                }
                ipOutBuf->nTimeStamp = tempTS;

                if(mFrameInterval!=0 && (ipOutBuf->nFlags & OMX_BUFFERFLAG_INVALID_TIMESTAMP))
                {
                    tempTS = mllLastUpdateTime + mFrameInterval;
                    ipOutBuf->nTimeStamp = tempTS;
                }
                mllLastUpdateTime = tempTS;
#ifdef DYNAMIC_PRIORITY_ADJUSTMENT
                mllLastDispTime = tempTS;
#endif
            }
            else
            {
                ipOutBuf->nTimeStamp = 0;
            }

            //in the case of resolution change from large to small without mjc scaling
            if (mCodecId == MTK_VDEC_CODEC_ID_VP9)
            {
                VDEC_DRV_PICINFO_T PicInfo;
                memset((void *)&PicInfo, 0x0, sizeof(PicInfo));
                if ((mUseClearMotion == OMX_FALSE) || (mMJCScalerByPassFlag == OMX_TRUE))
                {
                    if (VDEC_DRV_MRESULT_OK == eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_GET_PICTURE_INFO, NULL, &PicInfo))
                    {
                        //if ((mCropWidth > PicInfo.u4Width) || (mCropHeight > PicInfo.u4Height))
                        if ((mCropWidth != PicInfo.u4Width) && (mCropHeight != PicInfo.u4Height))
                        {
                            MTK_OMX_LOGD("crop w/h (%d %d)(%d %d):", mCropWidth, mCropHeight,PicInfo.u4Width,PicInfo.u4Height);

                            mCropWidth = PicInfo.u4Width;
                            mCropHeight = PicInfo.u4Height;

                            //set crop info
                            MTK_OMX_LOGD("--- OMX_EventPortSettingsChanged  (with Crop info)---");
                            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                                   mAppData,
                                                   OMX_EventPortSettingsChanged,
                                                   MTK_OMX_OUTPUT_PORT,
                                                   OMX_IndexVendorMtkOmxVdecGetCropInfo,
                                                   NULL);
                        }
                    }
                }
            }

            HandleFillBufferDone(ipOutBuf, mRealCallBackFillBufferDone);
            ipOutBuf = GetDisplayBuffer((mLegacyMode == OMX_TRUE) ? OMX_TRUE : OMX_FALSE);
        }
        ipOutBuf = NULL;

        while (NULL != GetFreeBuffer(&bEverOutput))
        {
            if (bEverOutput == OMX_FALSE)
            {
                RemoveForInputAtPTS(pInputBuf->nTimeStamp);
            }
        }

        if (VDEC_DRV_MRESULT_RESOLUTION_CHANGED == rResult)// || mPortReconfigInProgress == OMX_TRUE)
        {
            if (mMJCEnable == OMX_TRUE)
            {
                MJCDrainVdoBufQ();
                MJC_VIDEOCROP rVideoCrop;
                MJC_VIDEORESOLUTION rAlignment;

                rVideoCrop.mCropLeft = mCropLeft;
                rVideoCrop.mCropTop = mCropTop;
                rVideoCrop.mCropWidth = mCropWidth;
                rVideoCrop.mCropHeight = mCropHeight;

                rAlignment.u4Width = mQInfoOut.u4StrideAlign;
                rAlignment.u4Height = mQInfoOut.u4SliceHeightAlign;
                m_fnMJCSetParam(mpMJC, MJC_PARAM_ALIGH_SIZE, &rAlignment);
                m_fnMJCSetParam(mpMJC, MJC_PARAM_CROP_INFO, &rVideoCrop);
#if (ANDROID_VER >= ANDROID_KK)
                //mMJCScalerByPassFlag = OMX_TRUE;  //[ToDo]
                if (mMJCScalerByPassFlag == OMX_FALSE)
                {
                    MJC_VIDEORESOLUTION mMJC_BufRes;
                    m_fnMJCGetParam(mpMJC, MJC_PARAM_ALIGN_RESOLTUION, &mMJC_BufRes);
                    mCropLeft = 0;
                    mCropTop = 0;
                    mCropWidth = mMJC_BufRes.u4Width;
                    mCropHeight = mMJC_BufRes.u4Height;
                    MTK_OMX_LOGD("mCropWidth : %d , mCropHeight : %d, LINE:%d", mCropWidth, mCropHeight, __LINE__);
                }
#endif
                // For Scaler ClearMotion +
                mOriFrameWidth = mOutputPortDef.format.video.nFrameWidth;
                mOriFrameHeight = mOutputPortDef.format.video.nFrameHeight;
                MJCScalerCheckandSetBufInfo();
                // For Scaler ClearMotion -
            }
            QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
            SIGNAL(mDecodeSem);
            MarkInputBufferUnused(pInputBuf);
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventPortSettingsChanged,
                                   MTK_OMX_OUTPUT_PORT,
                                   NULL,
                                   NULL);
        }
        else if(OMX_TRUE == bCodecProfileOrLevelChanged)
        {
            //check this condition under ProfileOrLevelChanged to avoid GetFreeInputBuffer
            if (VDEC_DRV_MRESULT_NEED_MORE_OUTPUT_BUFFER != rResult)
            {
                //MTK_OMX_LOGUD("QueueInputBuffer LINE:%d", __LINE__);
                QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
                SIGNAL(mDecodeSem);
                MarkInputBufferUnused(pInputBuf);
            }
        }
        else
        {
            while (NULL != GetFreeInputBuffer())
                ;
        }

        if (mLegacyMode)
        {
            VDEC_DRV_PICINFO_T temp_pic_info;

            memset(&temp_pic_info, 0x0, sizeof(temp_pic_info));
            if (VDEC_DRV_MRESULT_OK !=
                    eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_GET_PICTURE_INFO, NULL, &temp_pic_info))
            {
                MTK_OMX_LOGD("[Info] Cannot get param: VDEC_DRV_GET_TYPE_GET_PICTURE_INFO");
                temp_pic_info.u4Width = mPicWidth;
                temp_pic_info.u4Height = mPicHeight;
                temp_pic_info.u4RealWidth = mOutputPortDef.format.video.nStride;
                temp_pic_info.u4RealHeight = mOutputPortDef.format.video.nSliceHeight;
            }
            // When output buffer width or height changed, try to update new crop info.
            if (mPicWidth != temp_pic_info.u4Width || mPicHeight != temp_pic_info.u4Height)
            {
                VDEC_DRV_CROPINFO_T temp_ccop_info;

                memset(&temp_ccop_info, 0x0, sizeof(temp_ccop_info));
                mPicWidth = temp_pic_info.u4Width;
                mPicHeight = temp_pic_info.u4Height;
                // Check new crop. If there is no crop info in SPS, crop info is picWidth/Height.
                if (VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_GET_FRAME_CROP_INFO, NULL, &temp_ccop_info))
                {
                    temp_ccop_info.u4CropLeft = 0;
                    temp_ccop_info.u4CropTop = 0;
                    temp_ccop_info.u4CropRight = mPicWidth - 1;
                    temp_ccop_info.u4CropBottom = mPicHeight - 1;
                }
                // if crop changed, notify the client.
                if (mCropLeft != temp_ccop_info.u4CropLeft || mCropTop != temp_ccop_info.u4CropTop ||
                        mCropWidth != (temp_ccop_info.u4CropRight - temp_ccop_info.u4CropLeft + 1) ||
                        mCropHeight != (temp_ccop_info.u4CropBottom - temp_ccop_info.u4CropTop + 1))
                {
                    mCropLeft = temp_ccop_info.u4CropLeft;
                    mCropTop = temp_ccop_info.u4CropTop;
                    mCropWidth = temp_ccop_info.u4CropRight - temp_ccop_info.u4CropLeft + 1;
                    mCropHeight = temp_ccop_info.u4CropBottom - temp_ccop_info.u4CropTop + 1;
                    MTK_OMX_LOGD("GetInfo() mCropLeft %d, mCropTop %d, mCropWidth %d, mCropHeight %d\n",
                            mCropLeft, mCropTop, mCropWidth, mCropHeight);
                    MTK_OMX_LOGD("--- OMX_EventPortSettingsChanged  (L) (with Crop info)---");
                    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                            mAppData,
                            OMX_EventPortSettingsChanged,
                            MTK_OMX_OUTPUT_PORT,
                            OMX_IndexConfigCommonOutputCrop,
                            NULL);
                }
            }
        }

#if 1 // Morris CTS test (early EOS)
        if (OMX_TRUE == mEarlyEOS)
        {
            // flush decoder
            FlushDecoder(OMX_TRUE);

            MTK_OMX_LOGD("%s EOS flush", GetVDECSrting(mCodecId, NULL));
            DumpFTBQ();

            mEarlyEOS = OMX_FALSE;   // for eosFlushSeek of AdaptivePlaybacKTest
            return OMX_TRUE;
        }
#endif
        bRet = OMX_TRUE;
    }
    //MTK_OMX_LOGD("DecodeVideo() --");

    return bRet;
}

OMX_BOOL MtkOmxVdec::FlushDecoder(OMX_BOOL mBufFlag_EOS)
{
    MTK_OMX_LOGD("+MtkOmxVdec::FlushDecoder");
    int ret;

    if (mDecoderInitCompleteFlag == OMX_FALSE)
    {
        MTK_OMX_LOGD("-MtkOmxVdec::FlushDecoder - decoder not init !!!");
//#ifdef MTK_CLEARMOTION_SUPPORT
        if (mMJCEnable == OMX_TRUE)
        {
        MJCFlushAllBufQ();
        }
//#endif
        return OMX_TRUE;
    }

    //for MTK SUPPORT MJPEG [
    if (mCodecId == MTK_VDEC_CODEC_ID_MJPEG)
    {
        //Bruce 20120829 clean hang due to race condition of WAIT_T(mDecodeSem)
        if (get_sem_value(&mDecodeSem) > 0)
        {
            LOCK_T(mWaitDecSemLock);
            while (get_sem_value(&mDecodeSem) > 0)
            {
                WAIT_T(mDecodeSem);
            }
            UNLOCK(mWaitDecSemLock);
        }
        MTK_OMX_LOGD("No Hang");
        MTK_OMX_LOGD("-MtkOmxVdec::FlushDecoder");
//#ifdef MTK_CLEARMOTION_SUPPORT
        if (OMX_TRUE == mMJCEnable && mBufFlag_EOS == OMX_FALSE)
        {
            MJCFlushAllBufQ();
        }
//#endif
        return OMX_TRUE;
    }
    //] MJPEG

    mNeedMoreOutputBuf = OMX_FALSE;

    if (mDrvHandle == VAL_NULL)
    {
        MTK_OMX_LOGE("[ERROR] mDrvHandle = NULL, return false");
        return OMX_FALSE;
    }

    if (VDEC_DRV_MRESULT_OK != eVDecDrvDecodeEx(mDrvHandle, NULL, 0, NULL))
    {
        MTK_OMX_LOGE("[ERROR]! DecodeFinal failed");
//#ifdef MTK_CLEARMOTION_SUPPORT
        if (mMJCEnable == OMX_TRUE)
        {
        MJCFlushAllBufQ();
        }
//#endif

        return OMX_FALSE;
    }

    OMX_BUFFERHEADERTYPE *ipOutBuf = GetDisplayBuffer((mLegacyMode == OMX_TRUE) ? OMX_TRUE : OMX_FALSE);

    while (ipOutBuf)
    {
        if (ipOutBuf->nTimeStamp != -1)
        {
            OMX_TICKS tempTS = RemoveForInputPTS();
            //for N migration, CTS EOSBehavior in 1st frame with ts =0
            //log: early EOS !!!0 0
            if( mEOSTS == tempTS )
            {
                if( (mEOSTS != 0) || (OMX_TRUE == mEarlyEOS && mEOSTS == 0) )
            {
                ipOutBuf->nFlags |= OMX_BUFFERFLAG_EOS;
                mEOSFound = OMX_TRUE;
            }
            }
            ipOutBuf->nTimeStamp = tempTS;
        }
        else
        {
            ipOutBuf->nTimeStamp = 0; // TS is for 0xFFFFFFFFFFFFFFFF case
        }

        MTK_OMX_LOGD("%06x Flushed frame (0x%08X) (0x%08X) %lld (%u) GET_DISP i(%d) frm_buf(0x%08X), flags(0x%08x)",
                     this, ipOutBuf, ipOutBuf->pBuffer, ipOutBuf->nTimeStamp, ipOutBuf->nFilledLen,
                     mGET_DISP_i, mGET_DISP_tmp_frame_addr, ipOutBuf->nFlags);

        MTK_OMX_LOGD("cnt %d, mFillThisBufQ size %d, mBufColorConvertDstQ %d, mBufColorConvertSrcQ %d",
                     CheckFreeBuffer(OMX_FALSE), mFillThisBufQ.size(), mBufColorConvertDstQ.size(),
                     mBufColorConvertSrcQ.size());
        HandleFillBufferDone(ipOutBuf, mRealCallBackFillBufferDone);
        ipOutBuf = GetDisplayBuffer((mLegacyMode == OMX_TRUE) ? OMX_TRUE : OMX_FALSE);
    }

    while (NULL != GetFreeBuffer())
        ;
    while (NULL != GetFreeInputBuffer(OMX_TRUE))
        ;

    CheckFreeBuffer(OMX_TRUE);

    iTSIn = 0;
    DisplayTSArray[0] = 0;

//#ifdef MTK_CLEARMOTION_SUPPORT
    if (OMX_TRUE == mMJCEnable && mBufFlag_EOS == OMX_FALSE)
    {
        MJCFlushAllBufQ();
    }
//#endif

    MTK_OMX_LOGD("-MtkOmxVdec::FlushDecoder");
    return OMX_TRUE;
}


OMX_BOOL MtkOmxVdec::InsertionSortForInputPTS(OMX_TICKS timeTS)
{
    //MTK_OMX_LOGD("InsertionSortForInputPTS ++, TSIn -> %d, PTS -> %lld", (int) iTSIn, timeTS);
    OMX_S32 i, j;
    if (iTSIn >= MTK_VDEC_AVC_DEC_TIMESTAMP_ARRAY_SIZE)
    {
        MTK_OMX_LOGE("Input picture is over MAX.");
        return OMX_FALSE;
    }

    for (i = 0; i < iTSIn; i++)
    {
        if (DisplayTSArray[i] == timeTS)
        {
            break;    // still insert;
        }
        else if (DisplayTSArray[i] > timeTS)
        {
            break;
        }
    }

    for (j = iTSIn; j > i; j--)
    {
        DisplayTSArray[j] = DisplayTSArray[j - 1];
    }
    DisplayTSArray[i] = timeTS;

#if 0
    for (j = 0; j <= iTSIn; j++)
    {
        MTK_OMX_LOGD("return input PTS[%2d] -> %10lld", (int)j, DisplayTSArray[j]);
    }
#endif

    iTSIn++;
    //MTK_OMX_LOGD("InsertionSortForInputPTS --");
    return OMX_TRUE;
}

OMX_TICKS MtkOmxVdec::RemoveForInputPTS()
{
    //MTK_OMX_LOGD("RemoveForInputPTS ++, index -> %2d, DisplayTSArray[0] -> %lld", (int)iTSIn, DisplayTSArray[0]);
    OMX_S32 i;
    OMX_TICKS PTS = DisplayTSArray[0];

    if (iTSIn > 0)
    {
        for (i = 1; i < iTSIn; i++)
        {
            DisplayTSArray[i - 1] = DisplayTSArray[i];
        }
        iTSIn--;
    }

#if 0
    for (i = 0; i < iTSIn; i++)
    {
        MTK_OMX_LOGD("return input PTS[%2d] -> %10lld", (int)i, DisplayTSArray[i]);
    }
#endif

    //MTK_OMX_LOGD("RemoveForInputPTS --");
    return PTS;
}

OMX_BOOL MtkOmxVdec::RemoveForInputAtPTS(OMX_TICKS timeTS)
{
    //MTK_OMX_LOGD("RemoveForInputAtPTS ++, index -> %2d, timeTS -> %lld", (int)iTSIn, timeTS);
    OMX_S32 i, j;
    OMX_TICKS PTS = DisplayTSArray[0];

    for (i = 0; i < iTSIn; i++)
    {
        if (DisplayTSArray[i] == timeTS)
        {
            break;    // found;
        }
    }

    if (i < iTSIn)
    {
        for (j = i; j < iTSIn - 1; j++)
        {
            DisplayTSArray[j] = DisplayTSArray[j + 1];
        }
        iTSIn--;
    }
    else
    {
        if (timeTS != 0)
        {
            MTK_OMX_LOGE("RFIAtPTS not found %lld", timeTS);
        }
    }

#if 0
    for (i = 0; i < iTSIn; i++)
    {
        MTK_OMX_LOGD("return input PTS[%2d] -> %10lld", (int)i, DisplayTSArray[i]);
    }
#endif

    //MTK_OMX_LOGD("RemoveForInputAtPTS --");
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::QueryDriverFormat(VDEC_DRV_QUERY_VIDEO_FORMAT_T *pQinfoIn, VDEC_DRV_QUERY_VIDEO_FORMAT_T *pQinfoOut)
{
    VAL_UINT32_T is_support;

    pQinfoIn->u4VideoFormat = GetVdecFormat(mCodecId);
    pQinfoIn->u4Resolution = VDEC_DRV_RESOLUTION_SUPPORT_720P;

    if (mSeqInfo.u4PicWidth != 0 && mSeqInfo.u4PicHeight != 0){
        pQinfoIn->u4Width = mSeqInfo.u4PicWidth;
        pQinfoIn->u4Height = mSeqInfo.u4PicHeight;
    } else {
        pQinfoIn->u4Width = mOutputPortDef.format.video.nFrameWidth;
        pQinfoIn->u4Height = mOutputPortDef.format.video.nFrameHeight;
    }

    switch (mCodecId)
    {
        case MTK_VDEC_CODEC_ID_HEVC:
            pQinfoIn->u4VideoFormat = VDEC_DRV_VIDEO_FORMAT_H265;
            pQinfoIn->u4Profile = VDEC_DRV_H265_VIDEO_PROFILE_H265_MAIN;
            pQinfoIn->u4Level = VDEC_DRV_VIDEO_LEVEL_UNKNOWN;
            break;
        case MTK_VDEC_CODEC_ID_MPEG4:
            pQinfoIn->u4VideoFormat = VDEC_DRV_VIDEO_FORMAT_MPEG4;
            pQinfoIn->u4Profile = VDEC_DRV_MPEG_VIDEO_PROFILE_MPEG4_ADVANCED_SIMPLE;
            pQinfoIn->u4Level = VDEC_DRV_VIDEO_LEVEL_5;
            break;
        case MTK_VDEC_CODEC_ID_H263:
            pQinfoIn->u4VideoFormat = VDEC_DRV_VIDEO_FORMAT_H263;
            pQinfoIn->u4Profile = VDEC_DRV_MPEG_VIDEO_PROFILE_H263_0;
            pQinfoIn->u4Level = VDEC_DRV_VIDEO_LEVEL_4;
            break;
        case MTK_VDEC_CODEC_ID_RV:
            pQinfoIn->u4VideoFormat = VDEC_DRV_VIDEO_FORMAT_REALVIDEO9;
            pQinfoIn->u4Profile = VDEC_DRV_MPEG_VIDEO_PROFILE_MPEG4_SIMPLE;
            pQinfoIn->u4Level = VDEC_DRV_VIDEO_LEVEL_3;
            break;
        case MTK_VDEC_CODEC_ID_AVC:
            pQinfoIn->u4VideoFormat = VDEC_DRV_VIDEO_FORMAT_H264;
            pQinfoIn->u4Profile = VDEC_DRV_H264_VIDEO_PROFILE_H264_HIGH;
            pQinfoIn->u4Level = VDEC_DRV_VIDEO_LEVEL_3_1;
            break;
        case MTK_VDEC_CODEC_ID_VC1:
            pQinfoIn->u4VideoFormat = VDEC_DRV_VIDEO_FORMAT_VC1;
            pQinfoIn->u4Profile = VDEC_DRV_MS_VIDEO_PROFILE_VC1_ADVANCED;
            pQinfoIn->u4Level = VDEC_DRV_VIDEO_LEVEL_3;
            break;
        case MTK_VDEC_CODEC_ID_VPX:
            pQinfoIn->u4VideoFormat = VDEC_DRV_VIDEO_FORMAT_VP8;
            pQinfoIn->u4Profile = VDEC_DRV_MPEG_VIDEO_PROFILE_MPEG4_SIMPLE;
            pQinfoIn->u4Level = VDEC_DRV_VIDEO_LEVEL_MEDIUM;
            break;
        case MTK_VDEC_CODEC_ID_VP9:
            pQinfoIn->u4VideoFormat = VDEC_DRV_VIDEO_FORMAT_VP9;
            pQinfoIn->u4Profile = VDEC_DRV_VP9_VIDEO_PROFILE_0;
            pQinfoIn->u4Level = VDEC_DRV_VIDEO_LEVEL_3;
            break;
        case MTK_VDEC_CODEC_ID_MPEG2:
            pQinfoIn->u4VideoFormat = VDEC_DRV_VIDEO_FORMAT_MPEG2;
            pQinfoIn->u4Profile = VDEC_DRV_MPEG_VIDEO_PROFILE_MPEG2_MAIN;
            pQinfoIn->u4Level = VDEC_DRV_VIDEO_LEVEL_HIGH;
            break;
        case MTK_VDEC_CODEC_ID_DIVX:
            pQinfoIn->u4VideoFormat = VDEC_DRV_VIDEO_FORMAT_DIVX4;
            pQinfoIn->u4Profile = VDEC_DRV_MPEG_VIDEO_PROFILE_MPEG4_ADVANCED_SIMPLE;
            pQinfoIn->u4Level = VDEC_DRV_VIDEO_LEVEL_5;
            break;
        case MTK_VDEC_CODEC_ID_DIVX3:
            pQinfoIn->u4VideoFormat = VDEC_DRV_VIDEO_FORMAT_DIVX311;
            pQinfoIn->u4Profile = VDEC_DRV_MPEG_VIDEO_PROFILE_MPEG4_ADVANCED_SIMPLE;
            pQinfoIn->u4Level = VDEC_DRV_VIDEO_LEVEL_5;
            break;
        case MTK_VDEC_CODEC_ID_XVID:
            pQinfoIn->u4VideoFormat = VDEC_DRV_VIDEO_FORMAT_XVID;
            pQinfoIn->u4Profile = VDEC_DRV_MPEG_VIDEO_PROFILE_MPEG4_ADVANCED_SIMPLE;
            pQinfoIn->u4Level = VDEC_DRV_VIDEO_LEVEL_5;
            break;
        case MTK_VDEC_CODEC_ID_S263:
            pQinfoIn->u4VideoFormat = VDEC_DRV_VIDEO_FORMAT_S263;
            pQinfoIn->u4Profile = VDEC_DRV_MPEG_VIDEO_PROFILE_MPEG4_ADVANCED_SIMPLE;
            pQinfoIn->u4Level = VDEC_DRV_VIDEO_LEVEL_5;
            break;
        case MTK_VDEC_CODEC_ID_MJPEG:
#define MJ_MAX_W    1280
#define MJ_MAX_H    720
            pQinfoOut->u4VideoFormat = VDEC_DRV_VIDEO_FORMAT_MJPEG;
            if (pQinfoIn->u4Width * pQinfoIn->u4Height > MJ_MAX_W * MJ_MAX_H)
            {
                pQinfoOut->u4Width = MJ_MAX_W;
                pQinfoOut->u4Height = MJ_MAX_H;
                return OMX_FALSE;
            }
            pQinfoOut->u4Width = pQinfoIn->u4Width;
            pQinfoOut->u4Height = pQinfoIn->u4Height;
            pQinfoOut->u4StrideAlign = 32;
            pQinfoOut->u4SliceHeightAlign = 16;
            return OMX_TRUE;
            break;
        default:
            break;
    }

    // query driver property
    is_support = eVDecDrvQueryCapability(VDEC_DRV_QUERY_TYPE_VIDEO_FORMAT, pQinfoIn, pQinfoOut);
    if (VDEC_DRV_MRESULT_FAIL == is_support)
    {
        return OMX_FALSE;
    }
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::InitVideoDecodeHW(OMX_S32 *aWidth, OMX_S32 *aHeight,
                                       OMX_S32 *aAspectRatioWidth, OMX_S32 *aAspectRatioHeight,
                                       OMX_U8 *aBuffer, OMX_U32 *aSize)
{
    VDEC_DRV_MRESULT_T eResult;

    VAL_BOOL_T bUFOsupport = VAL_FALSE;

    VAL_BOOL_T bMJCOff = VAL_FALSE;
    char value[PROPERTY_VALUE_MAX], value2[PROPERTY_VALUE_MAX];

    MTK_OMX_MEMSET((void *)&mSeqInfo, 0x00, sizeof(VDEC_DRV_SEQINFO_T));
    if (*aSize == 0 && mCodecId != MTK_VDEC_CODEC_ID_MPEG4 && mCodecId != MTK_VDEC_CODEC_ID_DIVX
        && mCodecId != MTK_VDEC_CODEC_ID_DIVX3 && mCodecId != MTK_VDEC_CODEC_ID_XVID
        && mCodecId != MTK_VDEC_CODEC_ID_S263)
    {
        MTK_OMX_LOGE("[ERROR] Input buffer size %d", *aSize);
        return OMX_FALSE;
    }

    if (mDrvHandle == NULL)
    {
        MTK_OMX_LOGE("%s driver handle released line(%d)", __func__, __LINE__);
        return OMX_FALSE;
    }

//#ifdef MTK_CLEARMOTION_SUPPORT
    if (OMX_TRUE == mMJCEnable && !mMJCReconfigFlag)
    {
        m_fnMJCInit(mpMJC, mUseClearMotion);
    }
//#endif

    VAL_UINT32_T omx_tids = 0;
    uint32_t waitForKeyframeValue = 0;
    //eResult = eVDecDrvCreate(&mDrvHandle, VDEC_DRV_VIDEO_FORMAT_REALVIDEO9);

    omx_tids = ((mVdecThreadTid << 16) | (mVdecDecThreadTid & 0xFFFF));
    eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_OMX_TIDS, (VAL_VOID_T *)&omx_tids, NULL);
    if ((mCodecId == MTK_VDEC_CODEC_ID_MPEG4 || mCodecId == MTK_VDEC_CODEC_ID_DIVX ||
                        mCodecId == MTK_VDEC_CODEC_ID_DIVX3 || mCodecId == MTK_VDEC_CODEC_ID_XVID ||
                        mCodecId == MTK_VDEC_CODEC_ID_S263))
    {
        VAL_UINT32_T nFrameSize = (mOutputPortDef.format.video.nFrameWidth & 0x0000FFFF) | ((mOutputPortDef.format.video.nFrameHeight & 0x0000FFFF) << 16);
        MTK_OMX_LOGUD("InitVideoDecodeHW() %d", nFrameSize);
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_FRAMESIZE, (VAL_VOID_T *)&nFrameSize, NULL);
    }

    if (mCodecId == MTK_VDEC_CODEC_ID_VC1)
    {
        VAL_UINT32_T nFrameSize = (mOutputPortDef.format.video.nFrameWidth & 0x0000FFFF) | ((mOutputPortDef.format.video.nFrameHeight & 0x0000FFFF) << 16);
        MTK_OMX_LOGU("InitVideoDecodeHW vc1() input size %d", *aSize);
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_FRAMESIZE, (VAL_VOID_T *)&nFrameSize, NULL);
    }

    if (mThumbnailMode == OMX_TRUE)   // for SW and hybrid decoder
    {
        VDEC_DRV_SET_DECODE_MODE_T rtSetDecodeMode;
        rtSetDecodeMode.eDecodeMode = VDEC_DRV_DECODE_MODE_THUMBNAIL; // thumbnail mode
        rtSetDecodeMode.u4DisplayFrameNum = 0;
        rtSetDecodeMode.u4DropFrameNum = 0;
        mSeqInfo.bIsThumbnail = OMX_TRUE; //for 4k sw H264
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_DECODE_MODE, &rtSetDecodeMode, NULL);
    }
    else
    {
        mSeqInfo.bIsThumbnail = OMX_FALSE;
    }

    if (mEnableAVTaskGroup == OMX_TRUE)
    {
        VAL_BOOL_T bEnableTaskGroup = VAL_TRUE;
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_AV_TASK_GROUP, &bEnableTaskGroup, NULL);
    }

#ifdef MTK_SW_RESOLUTION_CHANGE_SUPPORT
    mIsResChg = OMX_FALSE;
#endif


    // init driver using sequance layer
    if (OMX_TRUE == mIsSecureInst)
    {
        mRingbuf.rBase.u4VA  = 0;
        mRingbuf.rBase.u4PA  = 0;
        mRingbuf.rSecMemHandle = (OMX_U32)aBuffer;
        mRingbuf.rBase.u4Size = *aSize;
        mRingbuf.u4Read  = 0;
        mRingbuf.u4Write = (VAL_UINT32_T) * aSize;
        if(mIsSecUsingNativeHandle)
        {
            int i;
            for(i = 0; i < mSecInputBufCount; i++)
            {
                if(aBuffer == mSecInputBufInfo[i].pNativeHandle)
                {
                    mRingbuf.rSecMemHandle = mSecInputBufInfo[i].u4SecHandle;
                    break;
                }
            }

            if(mSecInputBufCount == i )
            {
                MTK_OMX_LOGE("[ERROR] aBuffer(0x%08X)Cannot find secure handle, LINE: %d\n", aBuffer, __LINE__);
                return OMX_FALSE;
            }
        }
    }
    else
    {
        if (OMX_TRUE == mInputUseION)
        {

            VBufInfo  info;
            int ret = mInputMVAMgr->getOmxInfoFromVA((void *) aBuffer, &info);
            if (ret < 0)
            {
                MTK_OMX_LOGE("[ERROR][ION][Input][InitVideoDecodeHW], LINE: %d\n", __LINE__);
                return OMX_FALSE;
            }
            else
            {
                mRingbuf.rBase.u4VA = info.u4VA;
                mRingbuf.rBase.u4PA = info.u4PA;
                mRingbuf.rBase.u4Size = *aSize;
                mRingbuf.u4Read = info.u4VA;
                mRingbuf.u4Write = info.u4VA + (VAL_UINT32_T) * aSize;
            }
        }
        else
        {
            mRingbuf.rBase.u4VA  = (VAL_UINT32_T)(aBuffer);
            mRingbuf.rBase.u4PA  = (VAL_UINT32_T)(aBuffer);
            mRingbuf.rBase.u4Size = *aSize;
            mRingbuf.u4Read  = (VAL_UINT32_T)(aBuffer);
            mRingbuf.u4Write = mRingbuf.u4Read + (VAL_UINT32_T) * aSize;
        }
    }

    //for Parsing SPS
    if (mAssignNALSizeLength == OMX_TRUE) {
        MTK_OMX_LOGE("assign NAL size before init drv");
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_NALSIZELENGTH, (VAL_VOID_T *)mNALSizeLengthInfo.nNaluBytes, NULL);
    }

    OMX_BOOL bNoReorderMode = OMX_FALSE;
    mRingbuf.u4OmxInputBufSz = mInputPortDef.nBufferSize;
    eResult = VDEC_DRV_MRESULT_OK;
    if ((eResult = eVDecDrvInit(mDrvHandle, &mRingbuf, &mSeqInfo)) != VDEC_DRV_MRESULT_OK )
    {
        MTK_OMX_LOGE("[ERROR] Cannot init driver");
        VDEC_DRV_QUERY_VIDEO_FORMAT_T qinfoIn;
        VDEC_DRV_QUERY_VIDEO_FORMAT_T qinfoOut;
        MTK_OMX_MEMSET(&qinfoIn, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));
        MTK_OMX_MEMSET(&qinfoOut, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));
        if (OMX_FALSE == QueryDriverFormat(&qinfoIn, &qinfoOut) || eResult == VDEC_DRV_MRESULT_FATAL )
        {
            MTK_OMX_LOGE("Video Resolution (%d, %d), MAX(%d, %d); Current Video Profile/Level (%x/%d), MAX(%x/%d)", qinfoIn.u4Width, qinfoIn.u4Height,\
                qinfoOut.u4Width, qinfoOut.u4Height, qinfoIn.u4Profile, qinfoIn.u4Level, qinfoOut.u4Profile, qinfoOut.u4Level);
            mFATALError = OMX_TRUE;
        }
        goto ERROR;
    }
    mSeqInfoCompleteFlag = OMX_TRUE;

    // Because ViLTE only supports H.264 baseline profile which dones't have B frames,
    // we can enable no reorder mode directly.
    bNoReorderMode = (mViLTESupportOn == OMX_TRUE) ? OMX_TRUE : mNoReorderMode;

    if( meDecodeType != VDEC_DRV_DECODER_MTK_HARDWARE )
    {  //for SW decoder
        OMX_U32 totalOutputBufCnt = mOutputPortDef.nBufferCountMin;
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_TOTAL_OUTPUT_BUF_SIZE, &totalOutputBufCnt, NULL);
    }

    if (mThumbnailMode == OMX_TRUE)   // for HW decoder
    {
        VDEC_DRV_SET_DECODE_MODE_T rtSetDecodeMode;
        rtSetDecodeMode.eDecodeMode = VDEC_DRV_DECODE_MODE_THUMBNAIL; // thumbnail mode
        rtSetDecodeMode.u4DisplayFrameNum = 0;
        rtSetDecodeMode.u4DropFrameNum = 0;
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_DECODE_MODE, &rtSetDecodeMode, NULL);
    }
    else if (bNoReorderMode == OMX_TRUE)
    {
        VDEC_DRV_SET_DECODE_MODE_T rtSetDecodeMode;
        rtSetDecodeMode.eDecodeMode = VDEC_DRV_DECODE_MODE_NO_REORDER; // no reorder mode
        rtSetDecodeMode.u4DisplayFrameNum = 0;
        rtSetDecodeMode.u4DropFrameNum = 0;
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_DECODE_MODE, &rtSetDecodeMode, NULL);
    }

    if (mSkipReferenceCheckMode == OMX_TRUE)
    {
        VDEC_DRV_SET_DECODE_MODE_T rtSetDecodeMode;
        rtSetDecodeMode.eDecodeMode = VDEC_DRV_DECODE_MODE_SKIP_REFERENCE_CHECK; // no reorder mode
        rtSetDecodeMode.u4DisplayFrameNum = 0;
        rtSetDecodeMode.u4DropFrameNum = 0;
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_DECODE_MODE, &rtSetDecodeMode, NULL);
    }

    if (mLowLatencyDecodeMode == OMX_TRUE)
    {
        VDEC_DRV_SET_DECODE_MODE_T rtSetDecodeMode;
        rtSetDecodeMode.eDecodeMode = VDEC_DRV_DECODE_MODE_LOW_LATENCY_DECODE; // no reorder mode
        rtSetDecodeMode.u4DisplayFrameNum = 0;
        rtSetDecodeMode.u4DropFrameNum = 0;
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_DECODE_MODE, &rtSetDecodeMode, NULL);
    }

    if (mViLTESupportOn == OMX_TRUE)
    {
        char dropErrframeProp[PROPERTY_VALUE_MAX] = {0};
        property_get("vendor.mtk.vdec.vtdroperrframe", dropErrframeProp, "0");
        if (atoi(dropErrframeProp) > 0)
        {
            VDEC_DRV_SET_DECODE_MODE_T rtSetDecodeMode;
            rtSetDecodeMode.u4DisplayFrameNum = 0;
            rtSetDecodeMode.u4DropFrameNum = 0;
            rtSetDecodeMode.eDecodeMode = VDEC_DRV_DECODE_MODE_DROP_ERROR_FRAME;
            eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_DECODE_MODE, &rtSetDecodeMode, NULL);
        }
    }

    if (mLegacyMode == OMX_TRUE)
    {
        VDEC_DRV_FIXED_MAX_OUTPUT_BUFFER_T  tFixedBufSettings={0};
        tFixedBufSettings.u4MaxWidth = mMaxWidth;
        tFixedBufSettings.u4MaxHeight = mMaxHeight;
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_FIXEDMAXOUTPUTBUFFER, &tFixedBufSettings, NULL);
    }

    {
        VDEC_DRV_CALLBACK_T rCallback;
        rCallback.u4hHandle = (VAL_HANDLE_T) this;
        rCallback.pfnGetOutputBuffer = OMXGetOutputBufferCB;
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_CALLBACK, &rCallback, NULL);
    }

    char waitForKeyframeProp[PROPERTY_VALUE_MAX];
    property_get(MTK_VDEC_PROP_WAITKEYFRAME, waitForKeyframeProp, "0");
    waitForKeyframeValue = (uint32_t)atoi(waitForKeyframeProp);
    if (bNoReorderMode == OMX_TRUE && mViLTESupportOn == OMX_TRUE &&
        mCodecId == MTK_VDEC_CODEC_ID_AVC)
    {
        waitForKeyframeValue |= MTK_VDEC_VALUE_WAITKEYFRAME_FOR_NO_REORDER;
    }
    eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_WAIT_KEYFRAME, &waitForKeyframeValue, NULL);

    *aWidth  = (OMX_S32)mSeqInfo.u4PicWidth;
    *aHeight = (OMX_S32)mSeqInfo.u4PicHeight;

    //MTK_OMX_LOGD("InitVideoDecodeHW *aWidth = %u, *aHeight = %u, mCodecId(0x%X)", (unsigned int)*aWidth, (unsigned int)*aHeight, mCodecId);

    if ((aAspectRatioWidth != NULL) && (aAspectRatioHeight != NULL))
    {
        *aAspectRatioWidth = (OMX_S32)mSeqInfo.i4AspectRatioWidth;
        *aAspectRatioHeight = (OMX_S32)mSeqInfo.i4AspectRatioHeight;
        //MTK_OMX_LOGD ("InitVideoDecodeHW *aAspectRatioWidth = %d, *aAspectRatioHeight = %d", (int)*aAspectRatioWidth, (int)*aAspectRatioHeight);
    }

    if (mConcealLevel != 0)
    {
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_CONCEAL_LEVEL, &mConcealLevel, NULL);
    }
    if (mSeekTargetTime > 0 && mSeekMode == OMX_FALSE)
    {
        mSeekMode = OMX_TRUE;
//#ifdef MTK_CLEARMOTION_SUPPORT
        if (mMJCEnable == OMX_TRUE)
        {
        bool mSeek;
        mSeek = true;
        m_fnMJCSetParam(mpMJC, MJC_PARAM_SEEK, &mSeek);
        }
//#endif

        VDEC_DRV_SET_DECODE_MODE_T rtSetDecodeMode;
        rtSetDecodeMode.u4DisplayFrameNum = 0;
        rtSetDecodeMode.u4DropFrameNum = 0;
        rtSetDecodeMode.eDecodeMode = VDEC_DRV_DECODE_MODE_B_SKIP; // seek mode
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_DECODE_MODE, &rtSetDecodeMode, NULL);
    }


    VAL_UINT32_T is_support;
    MTK_OMX_MEMSET(&mQInfoIn, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));
    MTK_OMX_MEMSET(&mQInfoOut, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));
    // query driver capability
    if (OMX_FALSE == QueryDriverFormat(&mQInfoIn, &mQInfoOut))
    {
        MTK_OMX_LOGE("[ERROR] Cannot get decoder property: Video Resolution (%d, %d), MAX(%d, %d); Current Video Profile/Level (%x/%d), MAX(%x/%d)", mQInfoIn.u4Width, mQInfoIn.u4Height,\
            mQInfoOut.u4Width, mQInfoOut.u4Height, mQInfoIn.u4Profile, mQInfoIn.u4Level, mQInfoOut.u4Profile, mQInfoOut.u4Level);
        mFATALError = OMX_TRUE;
        goto ERROR;
    }

    switch (mQInfoOut.ePixelFormat)
    {
        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER:
            mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
            mOutputPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK:
            mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV;
            mOutputPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_YV12:
            mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_MTK_COLOR_FormatYV12;
            mOutputPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_MTK_COLOR_FormatYV12;
            //MTK_OMX_LOGE("InitVideoDecodeHW:VDEC_DRV_PIXEL_FORMAT_YUV_YV12");
            break;
        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO:
            mOutputPortFormat.eColorFormat = OMX_COLOR_FormatVendorMTKYUV_UFO;
            mOutputPortDef.format.video.eColorFormat = OMX_COLOR_FormatVendorMTKYUV_UFO;
            break;

        default:
            break;
    }
    if (mLegacyMode == OMX_TRUE)
    {
        mOutputPortDef.format.video.nFrameWidth = mMaxWidth;
        mOutputPortDef.format.video.nFrameHeight = mMaxHeight;
    }
    mOutputPortDef.format.video.nStride = VDEC_ROUND_N(mOutputPortDef.format.video.nFrameWidth, mQInfoOut.u4StrideAlign);
    mOutputPortDef.format.video.nSliceHeight = VDEC_ROUND_N(mOutputPortDef.format.video.nFrameHeight, mQInfoOut.u4SliceHeightAlign);
    meDecodeType = mQInfoOut.eDecodeType;
    if (meDecodeType == VDEC_DRV_DECODER_MTK_HARDWARE)
    {
        mPropFlags &= ~MTK_OMX_VDEC_ENABLE_PRIORITY_ADJUSTMENT;
        //MTK_OMX_LOGD("MtkOmxVdec::SetConfig -> disable priority adjustment");
    }
    else
    {
        mPropFlags |= MTK_OMX_VDEC_ENABLE_PRIORITY_ADJUSTMENT;
        //MTK_OMX_LOGD("MtkOmxVdec::SetConfig -> enable priority adjustment");
    }

    MTK_OMX_LOGUD("InitVideoDecodeHW(): meDecodeType=%d, mThumbnailMode=%d, color format=%x, nStride=%d, nSliceHeight=%d ",
                 meDecodeType, mThumbnailMode, mOutputPortFormat.eColorFormat, mOutputPortDef.format.video.nStride, mOutputPortDef.format.video.nSliceHeight);

    // query chip name
    VAL_UINT32_T u4ChipName;
    if (VDEC_DRV_MRESULT_FAIL == eVDecDrvQueryCapability(VDEC_DRV_QUERY_TYPE_CHIP_NAME, VAL_NULL, &u4ChipName))
    {
        MTK_OMX_LOGE("[ERROR] Cannot get decoder property, VDEC_DRV_QUERY_TYPE_CHIP_NAME");
        goto ERROR;
    }

    // check device\mediatek\common\kernel-headers\Val_types_public.h
    MTK_OMX_LOGUD("VAL_CHIP_ID: %d", u4ChipName);

//#ifdef MTK_CLEARMOTION_SUPPORT
    if (mMJCEnable == OMX_TRUE)
    {
    m_fnMJCSetParam(mpMJC, MJC_PARAM_CHIP_NAME, &u4ChipName);
    }
//#endif

    mBitstreamBufferSize = 1024 * 1000;

    {
        VDEC_DRV_PIXEL_FORMAT_T eVCodecColorFormat;
        VDEC_DRV_YUV_STRIDE_T   rVCodecYUVStride;

        switch (mOutputPortFormat.eColorFormat)
        {
            case OMX_COLOR_FormatYUV420Planar:
                eVCodecColorFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER;
                rVCodecYUVStride.u4YStride = 16;
                rVCodecYUVStride.u4UVStride = 8;
                break;

            case OMX_COLOR_FormatVendorMTKYUV:
            case OMX_COLOR_FormatVendorMTKYUV_FCM:
                eVCodecColorFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK;
                rVCodecYUVStride.u4YStride = 16;
                rVCodecYUVStride.u4UVStride = 8;
                break;

            case OMX_MTK_COLOR_FormatYV12:
                eVCodecColorFormat = VDEC_DRV_PIXEL_FORMAT_YUV_YV12;
                rVCodecYUVStride.u4YStride = 16;
                rVCodecYUVStride.u4UVStride = 16;
                break;

            case OMX_COLOR_FormatVendorMTKYUV_UFO:
                eVCodecColorFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO;
                rVCodecYUVStride.u4YStride = 16;
                rVCodecYUVStride.u4UVStride = 8;
                break;

            case OMX_COLOR_FormatYUV420SemiPlanar:
                eVCodecColorFormat = VDEC_DRV_PIXEL_FORMAT_YUV_NV12;
                rVCodecYUVStride.u4YStride = 16;
                rVCodecYUVStride.u4UVStride = 16;
                break;

                //for MJPEG [
            case OMX_COLOR_Format32bitARGB8888:
                return OMX_TRUE;
                break;
                //]

            default:
                MTK_OMX_LOGE("[ERROR] [InitVideoDecodeHW] eColorFormat is not supported!! %d\n", mOutputPortFormat.eColorFormat);
                goto ERROR;
                break;
        }

        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_CODEC_COLOR_FORAMT, &eVCodecColorFormat, NULL);
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_CODEC_YUV_STRIDE, &rVCodecYUVStride, NULL);
    }
    mCodecProperty = 0;
    eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_CODEC_PROPERTY, NULL, &mCodecProperty);
    MTK_OMX_LOGUD("[Info][InitVideoDecodeHW] codec property 0x%08x\n", mCodecProperty);

    EnableCodecDriverUFO();

#if 0//def MTK S3D SUPPORT
    return AsvdInit();
#else
    return OMX_TRUE;
#endif

ERROR:
    eVDecDrvDeInit(mDrvHandle);
    //eVDecDrvRelease(mDrvHandle);
    //mDrvHandle = NULL;
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::DeInitVideoDecodeHW()
{
    MTK_OMX_LOGUD("+DeInitVideoDecodeHW");

    if (mDecoderInitCompleteFlag == OMX_TRUE)
    {
        LOCK(mDecodeLock);
        if (mDrvHandle != VAL_NULL && VDEC_DRV_MRESULT_OK != eVDecDrvDeInit(mDrvHandle))
        {
            MTK_OMX_LOGE("[ERROR] DeInitVideoDecodeHW failed: eVDecDrvDeInit");
            eVDecDrvRelease(mDrvHandle);
            mDrvHandle = VAL_NULL;
            mDecoderInitCompleteFlag = OMX_FALSE;// avoid decoder continue invaild action
            UNLOCK(mDecodeLock);
            return OMX_FALSE;
        }
        mDecoderInitCompleteFlag = OMX_FALSE;
        mSeqInfoCompleteFlag = OMX_FALSE;
        UNLOCK(mDecodeLock);
    }

    MTK_OMX_LOGUD("-DeInitVideoDecodeHW");
    return OMX_TRUE;
}

char *MtkOmxVdec::GetVDECSrting(MTK_VDEC_CODEC_ID mCodecId, void *pBuffer)
{
    OMX_U32 NalType;
    switch (mCodecId)
    {
        case MTK_VDEC_CODEC_ID_HEVC:
            if (OMX_FALSE == mIsSecureInst && pBuffer != NULL)
            {
                NalType = *((unsigned char *) pBuffer + 4) / 2;
                sprintf(mCodecSTR, "HEVC DEC, NAL %d", NalType);
            }
            else
            {
                sprintf(mCodecSTR, "HEVC DEC");
            }
            break;
        case MTK_VDEC_CODEC_ID_DIVX:
            sprintf(mCodecSTR, "DIVX DEC");
            break;
        case MTK_VDEC_CODEC_ID_DIVX3:
            sprintf(mCodecSTR, "DIVX3 DEC");
            break;
        case MTK_VDEC_CODEC_ID_XVID:
            sprintf(mCodecSTR, "XVID DEC");
            break;
        case MTK_VDEC_CODEC_ID_S263:
            sprintf(mCodecSTR, "S263 DEC");
            break;
        case MTK_VDEC_CODEC_ID_MPEG4:
            sprintf(mCodecSTR, "MPEG4 DEC");
            break;
        case MTK_VDEC_CODEC_ID_H263:
            sprintf(mCodecSTR, "H263 DEC");
            break;
        case MTK_VDEC_CODEC_ID_MPEG2:
            sprintf(mCodecSTR, "MPEG2 DEC");
            break;
        case MTK_VDEC_CODEC_ID_RV:
            sprintf(mCodecSTR, "RV DEC");
            break;
        case MTK_VDEC_CODEC_ID_AVC:
            if (OMX_FALSE == mIsSecureInst && pBuffer != NULL)
            {
                if (*((unsigned char *) pBuffer) == 0x00)
                {
                    OMX_U32 i = 0;
                    while (i < 10)
                    {
                        if (*(((unsigned char *) pBuffer) + i) != 0)
                        {
                            if (*(((unsigned char *) pBuffer) + i) == 0x01)
                            {
                                NalType = *(((unsigned char *) pBuffer) + i + 1) & 0x1F;
                            }
                            break;
                        }
                        i++;
                    }
                }
                else
                {
                    NalType = *((unsigned char *) pBuffer) & 0x1F;
                }
                sprintf(mCodecSTR, "AVC DEC, NAL %d", NalType);
            }
            else
            {
                sprintf(mCodecSTR, "AVC DEC");
            }
            break;
        case MTK_VDEC_CODEC_ID_VC1:
            sprintf(mCodecSTR, "VC1 DEC");
            break;
        case MTK_VDEC_CODEC_ID_VPX:
            sprintf(mCodecSTR, "VPX DEC");
            break;
        case MTK_VDEC_CODEC_ID_VP9:
            sprintf(mCodecSTR, "VP9 DEC");
            break;
        case MTK_VDEC_CODEC_ID_MJPEG:
            sprintf(mCodecSTR, "MJPEG DEC");
            break;
        default:
            sprintf(mCodecSTR, "Unknown Codec");
            break;
    }
    return mCodecSTR;
}

OMX_BUFFERHEADERTYPE *MtkOmxVdec::GetDisplayBuffer(OMX_BOOL bGetResolution)
{
    VAL_ULONG_T tmp_frame_addr = 0;
    VAL_UINT32_T i, cnt;
    VDEC_DRV_FRAMEBUF_T *pFrameBuf = NULL;

    if (VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_DISP_FRAME_BUFFER, NULL, &tmp_frame_addr))
    {
        MTK_OMX_LOGE("[ERROR] Cannot get param: VDEC_DRV_GET_TYPE_DISP_FRAME_BUFFER");
        return NULL;
    }
    /*
    for(i = 0, cnt=0; i < mOutputPortDef.nBufferCountActual; i++)
    {
        if(OMX_TRUE == mFrameBuf[i].bUsed)
        {
        cnt++;
        }
    }
    MTK_OMX_LOGD("DISP CNT:%d", cnt);
    */
    if (VAL_NULL != tmp_frame_addr)
    {
        pFrameBuf = (VDEC_DRV_FRAMEBUF_T*)tmp_frame_addr;
        //MTK_OMX_LOGE("[TEST] GetDisplay pFrameBuf = 0x%x, VA = 0x%x, MVA = 0x%x", pFrameBuf, pFrameBuf->rBaseAddr.u4VA, pFrameBuf->rBaseAddr.u4PA);
        LOCK_T(mFillThisBufQLock);
        for (i = 0; i < mOutputPortDef.nBufferCountActual; i++)
        {
            if (OMX_TRUE == mFrameBuf[i].bUsed)
            {
                //MTK_OMX_LOGE("[TEST] GetDisplay[%d] VA = 0x%x", i, mFrameBuf[i].frame_buffer.rBaseAddr.u4VA);
                if ((OMX_FALSE == mIsSecureInst && pFrameBuf->rBaseAddr.u4VA == mFrameBuf[i].frame_buffer.rBaseAddr.u4VA) ||
                    (OMX_TRUE == mIsSecureInst && pFrameBuf->rSecMemHandle == mFrameBuf[i].frame_buffer.rSecMemHandle))
                {
                    if (OMX_FALSE == mIsSecureInst && (&mFrameBuf[i].frame_buffer != pFrameBuf))
                    {
                        mFrameBuf[i].frame_buffer.u4FrameBufferStatus = pFrameBuf->u4FrameBufferStatus;
                    }

                    mFrameBuf[i].bDisplay = OMX_TRUE;
                    //MTK_OMX_LOGD("GET_DISP, i:%d, frm_buf:%08x, omx_buf:%08x, width=%d, height=%d", i, tmp_frame_addr, mFrameBuf[i].ipOutputBuffer, mFrameBuf[i].frame_buffer.u4DispWidth, mFrameBuf[i].frame_buffer.u4DispHeight);
                    mGET_DISP_i = i;
                    mGET_DISP_tmp_frame_addr = tmp_frame_addr;
                    //MTK_OMX_LOGD("GET_DISP, i:%d, frm_buf:%08x, omx_buf:%08x", i, tmp_frame_addr, mFrameBuf[i].ipOutputBuffer);
                    if (OMX_TRUE == bGetResolution || mLegacyMode == OMX_TRUE)
                    {
                        VDEC_DRV_PICINFO_T temp_pic_info;

                        memset(&temp_pic_info, 0x0, sizeof(temp_pic_info));
                        if (VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_GET_PICTURE_INFO, NULL, &temp_pic_info))
                        {
                            MTK_OMX_LOGE("[ERROR] Cannot get param: VDEC_DRV_GET_TYPE_GET_PICTURE_INFO");
                            temp_pic_info.u4Width = mOutputPortDef.format.video.nFrameWidth;
                            temp_pic_info.u4Height = mOutputPortDef.format.video.nFrameHeight;
                            temp_pic_info.u4RealWidth = mOutputPortDef.format.video.nStride;
                            temp_pic_info.u4RealHeight = mOutputPortDef.format.video.nSliceHeight;
                        }
                        MTK_OMX_LOGD("GetDisplayBuffer() u4Width %d, u4Height %d, u4RealWidth %d, u4RealHeight %d\n",
                                     temp_pic_info.u4Width, temp_pic_info.u4Height, temp_pic_info.u4RealWidth, temp_pic_info.u4RealHeight);
                        mFrameBuf[i].ipOutputBuffer->nWidth = temp_pic_info.u4Width;
                        mFrameBuf[i].ipOutputBuffer->nHeight = temp_pic_info.u4Height;

                        //remove 71 and 72
                        {
                            mFrameBuf[i].ipOutputBuffer->nStride = VDEC_ROUND_16(temp_pic_info.u4RealWidth);
                            mFrameBuf[i].ipOutputBuffer->nSliceHeight = VDEC_ROUND_32(temp_pic_info.u4RealHeight);
                        }
                    }

                    if (mCorrectTsFromOMX)   // for VC1
                    {
                        if (OMX_FALSE == mFirstFrameRetrieved)
                        {
                            mFirstFrameRetrieved = OMX_TRUE;
                        }
                        else
                        {
                            mCurrentFrameTs += mFrameTsInterval;
                        }
                        mFrameBuf[i].ipOutputBuffer->nTimeStamp = mCurrentFrameTs;
                        //MTK_OMX_LOGE ("@@ Correct TS to %lld", mCurrentFrameTs);
                    }

                    if ((mFrameBuf[i].frame_buffer.u4FrameBufferStatus & VDEC_DRV_FBSTSTUS_INVALID_TIMESTAMP) != 0)
                    {
                        mFrameBuf[i].ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_INVALID_TIMESTAMP;
                    }

#if defined(DYNAMIC_PRIORITY_ADJUSTMENT)
                    mllLastDispTime = mFrameBuf[i].ipOutputBuffer->nTimeStamp;
#endif
                    mFrameBuf[i].bNonRealDisplay = OMX_FALSE;
                    if ((mFrameBuf[i].frame_buffer.u4FrameBufferStatus & VDEC_DRV_FBSTSTUS_NOT_DISPLAY) != 0 ||
                        (mFrameBuf[i].frame_buffer.u4FrameBufferStatus & VDEC_DRV_FBSTSTUS_REPEAT_LAST) != 0 ||
                        (mFrameBuf[i].frame_buffer.u4FrameBufferStatus & VDEC_DRV_FBSTSTUS_NOT_USED) != 0)   // not to display
                    {
                        mFrameBuf[i].ipOutputBuffer->nFilledLen = 0;
                        mFrameBuf[i].bNonRealDisplay = OMX_TRUE;
                        mNumNotDispAvailOutput++;
                        if ((mFrameBuf[i].frame_buffer.u4FrameBufferStatus & VDEC_DRV_FBSTSTUS_REPEAT_LAST) != 0)
                        {
                            MTK_OMX_LOGD("RL return");
#ifdef DYNAMIC_PRIORITY_ADJUSTMENT
                            mErrorCount = 0;
#endif
                        }
                        else if ((mFrameBuf[i].frame_buffer.u4FrameBufferStatus & VDEC_DRV_FBSTSTUS_NOT_USED) != 0)
                        {
                            RemoveForInputAtPTS(mFrameBuf[i].ipOutputBuffer->nTimeStamp);
                            //for VP9, if early EOS bitstream is just alter-ref-frame which is never shown, EOS timestamp should
                            //use last previous one instead.
                            if ((mFrameBuf[i].ipOutputBuffer->nTimeStamp == mEOSTS) && (OMX_TRUE == mEarlyEOS))
                            {
                                MTK_OMX_LOGD("NU return #1 %lld %d",mEOSTS, mEarlyEOS);
                                mEOSTS = -1;
                                OMX_U32 i;
                                for (i = 0; i < iTSIn; i++)
                                {
                                    if (DisplayTSArray[i] > mEOSTS)
                                    {
                                        mEOSTS = DisplayTSArray[i];
                                    }
                                }
                                MTK_OMX_LOGD("NU return reset mEOSTS %lld",mEOSTS);
                            }
                            mFrameBuf[i].ipOutputBuffer->nTimeStamp = -1;
                            MTK_OMX_LOGD("NU return");
                        }
                        else
                        {
                            MTK_OMX_LOGD("ND return");
#ifdef DYNAMIC_PRIORITY_ADJUSTMENT
                            mErrorCount++;
#endif
                        }
                    }
                    else
                    {
                        if (pFrameBuf->u4FrameBufferType == VDEC_DRV_FBTYPE_3D_SBS) {
                            m3DStereoMode = OMX_VIDEO_H264FPA_SIDEBYSIDE;
                        }
                        else if (pFrameBuf->u4FrameBufferType == VDEC_DRV_FBTYPE_3D_TAB) {
                            m3DStereoMode = OMX_VIDEO_H264FPA_TOPANDBOTTOM;
                        }
                        //MTK_OMX_LOGD("CURRENT_DISPLAY return code, length %d, ts %lld, 3D %d", mFrameBuf[i].ipOutputBuffer->nFilledLen, mFrameBuf[i].ipOutputBuffer->nTimeStamp, m3DStereoMode);
                        mFrameBuf[i].ipOutputBuffer->nTimeStamp = 0;
                        //MTK_OMX_LOGD("CURRENT_DISPLAY return code, length %d, ts %lld", mFrameBuf[i].ipOutputBuffer->nFilledLen, mFrameBuf[i].ipOutputBuffer->nTimeStamp);
#ifdef DYNAMIC_PRIORITY_ADJUSTMENT
                        mErrorCount = 0;
#endif

#if 0  // dump secure video frame
                        static int dumpframe = 1;
                        if (dumpframe == 60)
                        {
                            MTK_OMX_LOGD("@@Hello there (0x%08X)", mFrameBuf[i].frame_buffer.rSecMemHandle);
                            FILE *fp_outputY = fopen("/data/yuv/testY.raw", "ab");
                            FILE *fp_outputC = fopen("/data/yuv/testC.raw", "ab");
                            VdecFrameDumpStruct dumpStruct;
                            OMX_U32 sizeY = 720 * 320;
                            OMX_U32 sizeC = 720 * 320 / 2;
                            unsigned char *pFrameBuffer = (unsigned char *)malloc(sizeY + sizeC);
                            dumpStruct.frame_secure_handle = mFrameBuf[i].frame_buffer.rSecMemHandle;
                            dumpStruct.frame_va_share_handle = MtkVdecRegisterSharedMemory(pFrameBuffer, sizeY + sizeC);
                            MtkVdecH264SecInitTest(&dumpStruct);
                            fwrite(pFrameBuffer, 1, sizeY, fp_outputY);
                            fwrite(pFrameBuffer + sizeY, 1, sizeC, fp_outputC);
                            fclose(fp_outputY);
                            fclose(fp_outputC);
                            free(pFrameBuffer);
                        }
                        dumpframe++;
#endif
#if 0
                        if (OMX_TRUE == mOutputAllocateBuffer)
                        {
                            OMX_BOOL converted = ConvertFrameToYUV420((FrmBufStruct *)&mFrameBuf[i], bGetResolution);
                            if (OMX_FALSE == converted)
                            {
                                MTK_OMX_LOGE("Internal color conversion not complete");
                            }
#if 0
                            // dump converted frames
                            char filename[256];
                            sprintf(filename, "/sdcard/VdecOutFrm_w%d_h%d_t%d.dat",
                                    mOutputPortDef.format.video.nFrameWidth,
                                    mOutputPortDef.format.video.nFrameHeight,
                                    gettid());
                            mFrameBuf[i].ipOutputBuffer->nFilledLen = (mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight) * 3 >> 1;
                            writeBufferToFile(filename, mFrameBuf[i].ipOutputBuffer->pBuffer, mFrameBuf[i].ipOutputBuffer->nFilledLen);
#endif
                        }
#endif
                        if (mDumpOutputFrame == OMX_TRUE)
                        {
                            FILE *fp_output;
                            OMX_U32 size_fp_output;
                            char ucStringyuv[100];
                            size_t temp_size, temp_size_Y, temp_size_C, w_size, h_size;
                            //char *ptemp_buff = (char *)mFrameBuf[i].ipOutputBuffer->pBuffer;
                            char *ptemp_buff = (char *)mFrameBuf[i].frame_buffer.rBaseAddr.u4VA;

                            w_size = mOutputPortDef.format.video.nStride;
                            h_size = mOutputPortDef.format.video.nSliceHeight;

                            if (mSeqInfo.u4BitDepthLuma == 10 || mSeqInfo.u4BitDepthChroma == 10)
                            {
                                temp_size_Y = w_size*h_size*1.25;
                                temp_size_C = temp_size_Y/2;
                                temp_size_Y = (((temp_size_Y+511)>>9)<<9);
                                temp_size = (temp_size_Y+temp_size_C);
                            }
                            else
                            {
                                temp_size_Y = w_size*h_size;
                                temp_size_C = temp_size_Y/2;
                                temp_size = (temp_size_Y+temp_size_C);
                            }

#if 1
                            if (mDumpCts == OMX_FALSE) {
                                sprintf(ucStringyuv, "//sdcard/Vdec%d_dump_w%d_h%d_t%4d.yuv", (int)mFrameBuf[i].ipOutputBuffer->nFilledLen,
                                    mOutputPortDef.format.video.nStride,
                                    mOutputPortDef.format.video.nSliceHeight, (int)gettid());
                                //MTK_OMX_LOGD("[DEBUG]nBufferSize %d, nFilledLen %d, w %d, h %d",mOutputPortDef.nBufferSize,mFrameBuf[i].ipOutputBuffer->nFilledLen,mOutputPortDef.format.video.nFrameWidth, mOutputPortDef.format.video.nFrameHeight);
                                //MTK_OMX_LOGD("[DEBUG] nStride %d, nSliceHeight %d",mOutputPortDef.format.video.nStride,mOutputPortDef.format.video.nSliceHeight);
                            } else { // dump to specific folder
                                sprintf(ucStringyuv, "/sdcard/vdump/Vdec_dump_%d_%d_t%05d.yuv",
                                    mOutputPortDef.format.video.nStride,
                                    mOutputPortDef.format.video.nSliceHeight, (int)gettid());
                            }
                            fp_output = fopen(ucStringyuv, "ab");
                            if (fp_output != NULL)
                            {
                                size_fp_output = temp_size;
                                MTK_OMX_LOGD("sdcard/mfv_264.out write size = %d\n", size_fp_output);
                                size_fp_output = fwrite(ptemp_buff, 1, size_fp_output, fp_output);
                                MTK_OMX_LOGD("sdcard/mfv_264.out real write size = %d\n", size_fp_output);
                                fclose(fp_output);
                            }
                            else
                            {
                                MTK_OMX_LOGE("sdcard/mfv_264.out file create error\n");
                                //dump frame buffer in data/vdec/
                                sprintf(ucStringyuv, "/data/vdec/Vdec_w%d_h%d_%4d.yuv",
                                    mOutputPortDef.format.video.nStride,
                                    mOutputPortDef.format.video.nSliceHeight, (int)gettid());
                                fp_output = fopen(ucStringyuv, "ab");
                                if (fp_output != NULL)
                                {
                                    size_fp_output = mFrameBuf[i].ipOutputBuffer->nFilledLen;
                                    MTK_OMX_LOGD("/data/vdec/ write size = %d\n", size_fp_output);
                                    size_fp_output = fwrite(ptemp_buff, 1, size_fp_output, fp_output);
                                    MTK_OMX_LOGD("/data/vdec/ real write size = %d\n", size_fp_output);
                                    fclose(fp_output);
                                }
                                else
                                {
                                    MTK_OMX_LOGE("/data/vdec/out file create error\n");
                                }
                                //end
                            }
#else                       // CRC check for output frame
                            OMX_U32  crc_accum;
                            size_fp_output = mFrameBuf[i].ipOutputBuffer->nFilledLen;
                            crc_accum = _calculate_crc32((unsigned char *)ptemp_buff, size_fp_output);
                            LOGE("%d %d crc_accum : %08x", FNum, this, crc_accum);
#if 0                       // to create CRC golden
                            ptemp_buff = (char *) &crc_accum;
                            sprintf(ucStringyuv, "//sdcard/Vdec%04d.crc", FNum);
                            fp_output = fopen(ucStringyuv, "ab");
                            if (fp_output != NULL)
                            {
                                fwrite(ptemp_buff, 1, 4, fp_output);
                                fclose(fp_output);
                            }
                            FNum++;
#else                       // to compare CRC golden
                            OMX_U32 crc_accum_rec;
                            char crcRead[4];
                            sprintf(ucStringyuv, "//sdcard/Vdec%04d.crc", FNum);
                            fp_output = fopen(ucStringyuv, "r+b");
                            if (fp_output != NULL)
                            {
                                fread(crcRead, 1, 4, fp_output);
                                fclose(fp_output);
                                crc_accum_rec = *((OMX_U32 *)crcRead);
                                if (crc_accum_rec != crc_accum)
                                {
                                    LOGE("%d %d crc_accum_rec error: %x", FNum, this, crc_accum_rec);
                                }
                                else
                                {
                                    LOGE("%d %d crc_accum_rec ok: %x", FNum, this, crc_accum_rec);
                                }
                            }
                            FNum++;
#endif
#endif
                        }
                    }

                    if (mIsHDRVideo == OMX_TRUE) {
                        if (mColorDesc.u4ColorPrimaries == VIDEO_HDR_COLOR_PRIMARIES_INVALID)
                        {
                            if (VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_GET_COLOR_DESC, NULL, &mColorDesc))
                            {
                                MTK_OMX_LOGD("[ERROR] Cannot get param: VDEC_DRV_GET_TYPE_GET_COLOR_DESC");
                                CopyHDRColorDesc();
                            }

                            if (mMJCEnable == OMX_TRUE && mUseClearMotion == OMX_TRUE)
                            {
                                m_fnMJCSetParam(mpMJC, MJC_PARAM_SET_COLOR_DESC, &mColorDesc);
                            }
                        }
                        MTK_OMX_LOGD("[info] HDR video !! Get COLOR_DESC param: mColorDesc u4ColorPrimaries %d u4TransformCharacter %d u4MatrixCoeffs %d u4DisplayPrimariesX %d %d %d u4DisplayPrimariesY %d %d %d",
                            mColorDesc.u4ColorPrimaries, mColorDesc.u4TransformCharacter, mColorDesc.u4MatrixCoeffs,
                            mColorDesc.u4DisplayPrimariesX[0], mColorDesc.u4DisplayPrimariesX[1], mColorDesc.u4DisplayPrimariesX[2],
                            mColorDesc.u4DisplayPrimariesY[0], mColorDesc.u4DisplayPrimariesY[1], mColorDesc.u4DisplayPrimariesY[2]);
                        MTK_OMX_LOGD("[info] HDR video !! Get COLOR_DESC param: u4WhitePointX %d u4WhitePointY %d u4MaxDisplayMasteringLuminance %d u4MinDisplayMasteringLuminance %d u4MaxContentLightLevel %d u4MaxPicAverageLightLevel %d",
                            mColorDesc.u4WhitePointX, mColorDesc.u4WhitePointY,
                            mColorDesc.u4MaxDisplayMasteringLuminance, mColorDesc.u4MinDisplayMasteringLuminance,
                            mColorDesc.u4MaxContentLightLevel, mColorDesc.u4MaxPicAverageLightLevel);
                    }

                    if (mHDRVideoSupportOn && OMX_FALSE == mOutputAllocateBuffer) {
                        int buffer_size = mFrameBuf[i].frame_buffer.rBaseAddr.u4Size;
                        int err = 0;
                        OMX_BUFFERHEADERTYPE *ipOutputBuffer = mFrameBuf[i].ipOutputBuffer;

                        if (OMX_TRUE == mStoreMetaDataInBuffers) {
                            buffer_size = mFrameBuf[i].frame_buffer.rBaseAddr.u4Size;
                        } else {
                            buffer_size = ipOutputBuffer->nAllocLen;
                        }

                        if (mIsHDRVideo == OMX_TRUE && mANW_HWComposer == OMX_TRUE && mNativeWindowHDRInternalConvert == OMX_FALSE) {
                            err = gralloc_extra_perform((buffer_handle_t)mFrameBuf[i].bGraphicBufHandle, GRALLOC_EXTRA_SET_HDR_INFO, (void *)&mColorDesc);
                            if (err != 0) {
                                MTK_OMX_LOGE("[ERROR] GRALLOC_EXTRA_SET_HDR_INFO fail err = %d", err);
                            }

                            if (sizeof(VDEC_DRV_COLORDESC_T) != sizeof(ge_hdr_info_t)) {
                                MTK_OMX_LOGE("[ERROR] VIDEO_HDR_HEADER_SIZE not sync!! %d != %d", sizeof(VDEC_DRV_COLORDESC_T), sizeof(ge_hdr_info_t));
                            }

                            MTK_OMX_LOGD("[Debug] HDR buffer_size %d sizeof(VDEC_DRV_COLORDESC_T) %d sizeof(ge_hdr_info_t) %d buffer Va 0x%x alloc size: %d %d",
                                buffer_size, sizeof(VDEC_DRV_COLORDESC_T), sizeof(ge_hdr_info_t), mFrameBuf[i].frame_buffer.rBaseAddr.u4VA, ipOutputBuffer->nAllocLen, mFrameBuf[i].frame_buffer.rBaseAddr.u4Size);
                        }
                    }

                    mNumAllDispAvailOutput++;
                    mFrameBuf[i].frame_buffer = *pFrameBuf;
                    if (VAL_TRUE == pFrameBuf->rColorPriInfo.bVideoRangeExist) {
                        mFrameBuf[i].ipOutputBuffer->bVideoRangeExist = OMX_TRUE;
                    } else {
                        mFrameBuf[i].ipOutputBuffer->bVideoRangeExist = OMX_FALSE;
                    }
                    mFrameBuf[i].ipOutputBuffer->u4VideoRange = pFrameBuf->rColorPriInfo.u4VideoRange;
                    if (VAL_TRUE == pFrameBuf->rColorPriInfo.bColourPrimariesExist) {
                        mFrameBuf[i].ipOutputBuffer->bColourPrimariesExist = OMX_TRUE;
                    } else {
                        mFrameBuf[i].ipOutputBuffer->bColourPrimariesExist = OMX_FALSE;
                    }
                    switch (pFrameBuf->rColorPriInfo.eColourPrimaries)
                    {
                        case COLOR_PRIMARIES_BT601:
                            mFrameBuf[i].ipOutputBuffer->eColourPrimaries = OMX_COLOR_PRIMARIES_BT601;
                            break;
                        case COLOR_PRIMARIES_BT709:
                            mFrameBuf[i].ipOutputBuffer->eColourPrimaries = OMX_COLOR_PRIMARIES_BT709;
                            break;
                        case COLOR_PRIMARIES_BT2020:
                            mFrameBuf[i].ipOutputBuffer->eColourPrimaries = OMX_COLOR_PRIMARIES_BT2020;
                            break;
                        default:
                            mFrameBuf[i].ipOutputBuffer->eColourPrimaries = OMX_COLOR_PRIMARIES_BT601;
                            break;
                    }
                    UpdateColorAspectsParams();
                    UNLOCK(mFillThisBufQLock);
                    return mFrameBuf[i].ipOutputBuffer;
                }
            }
        }
        UNLOCK(mFillThisBufQLock);
    }

    return NULL;
}

void MtkOmxVdec::MarkInputBufferUnused(OMX_BUFFERHEADERTYPE *ipInputBuf)
{
    VAL_UINT32_T i;
    for (i = 0; i < mInputPortDef.nBufferCountActual; i++)
    {
        if (mInputBuf[i].ipInputBuffer == ipInputBuf)
        {
            //MTK_OMX_LOGD("MarkInputBufferUnused mInputBuf[%d] 0x%08x (%d)", i, (VAL_UINT32_T)&mInputBuf[i].InputBuf, mInputBuf[i].bUsed);
            mInputBuf[i].bUsed = OMX_FALSE;
        }
    }
}

OMX_BUFFERHEADERTYPE *MtkOmxVdec::GetFreeInputBuffer(OMX_BOOL bFlushAll)
{
    VAL_ULONG_T tmp_frame_addr = 0;
    VAL_UINT32_T i;

RECHECK:
    //get un-used free frame.
    if (VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_FREE_INPUT_BUFFER, NULL, &tmp_frame_addr))
    {
        MTK_OMX_LOGE("[ERROR] Cannot get param:VDEC_DRV_GET_TYPE_FREE_INPUT_BUFFER");
        for (i = 0; i < mInputPortDef.nBufferCountActual; i++)
        {
            if (OMX_TRUE == mInputBuf[i].bUsed)
            {
                HandleEmptyBufferDone(mInputBuf[i].ipInputBuffer);
                mInputBuf[i].bUsed = OMX_FALSE;
                return mInputBuf[i].ipInputBuffer;
            }
        }
        if(i == mInputPortDef.nBufferCountActual)
        {
            MTK_OMX_LOGE("cannot find InputBuffer, tmp_frame_addr: 0x%08x",tmp_frame_addr);
            for (i = 0; i < mInputPortDef.nBufferCountActual; i++)
            {
                MTK_OMX_LOGE("mInputBuf[%d]:0x%08x",i,mInputBuf[i].ipInputBuffer);
            }
        }
        return NULL;
    }
    if (VAL_NULL != tmp_frame_addr)
    {
        //MTK_OMX_LOGD("tmp_frame_addr 0x%08x", tmp_frame_addr);
        for (i = 0; i < mInputPortDef.nBufferCountActual; i++)
        {
            //MTK_OMX_LOGD("mInputBuf[%d] 0x%08x (%d)", i, (VAL_UINT32_T)&mInputBuf[i].InputBuf, mInputBuf[i].bUsed);
            if (tmp_frame_addr == (VAL_ULONG_T)&mInputBuf[i].InputBuf || bFlushAll == OMX_TRUE)
            {
                if (OMX_TRUE == mInputBuf[i].bUsed  && !NotInETBQ(mInputBuf[i].ipInputBuffer))
                {
                    HandleEmptyBufferDone(mInputBuf[i].ipInputBuffer);
                    mInputBuf[i].bUsed = OMX_FALSE;
                    return mInputBuf[i].ipInputBuffer;
                }
                else
                {
                    MTK_OMX_LOGD("strange!! Input buffer is not free(0x%08x, %d, %d) %d", mInputBuf[i].ipInputBuffer, mInputBuf[i].bUsed, !NotInETBQ(mInputBuf[i].ipInputBuffer), mInputPortDef.nBufferCountActual);
                }
            }
        }

        if(i == mInputPortDef.nBufferCountActual)
        {
            MTK_OMX_LOGD("unknown input 0x%08x ??", tmp_frame_addr);
            for (i = 0; i < mInputPortDef.nBufferCountActual; i++)
            {
                //MTK_OMX_LOGD("GetFreeInputBuffer() not found input buffer to free (0x%08x, %d, %d) %d", mInputBuf[i].ipInputBuffer, mInputBuf[i].bUsed, !NotInETBQ(mInputBuf[i].ipInputBuffer), mInputPortDef.nBufferCountActual);
            }
        }
    }
    if (VAL_NULL != tmp_frame_addr)// && bFlushAll == OMX_TRUE)
    {
        goto RECHECK;
    }
    return NULL;
}


OMX_BUFFERHEADERTYPE *MtkOmxVdec::GetFreeBuffer(OMX_BOOL *pbDisplay)
{
    VAL_ULONG_T tmp_frame_addr = 0;
    VAL_UINT32_T i;
    VDEC_DRV_FRAMEBUF_T *pFrameBuf = NULL;

RECHECK:
    //get un-used free frame.
    if (VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_FREE_FRAME_BUFFER, NULL, &tmp_frame_addr))
    {
        MTK_OMX_LOGE("[ERROR] Cannot get param");
        return NULL;
    }

    if (VAL_NULL != tmp_frame_addr)
    {
        if (OMX_TRUE == mStoreMetaDataInBuffers)
        {
            pFrameBuf = (VDEC_DRV_FRAMEBUF_T*)tmp_frame_addr;
            LOCK_T(mFillThisBufQLock);
            for (i = 0; i < mOutputPortDef.nBufferCountActual; i++)
            {
                // TODO: Add secure handle validity check in mFrameBufInt array
                if (((OMX_FALSE == mIsSecureInst && 0 != mFrameBufInt[i].ionBufHandle &&
                    pFrameBuf->rBaseAddr.u4VA == mFrameBufInt[i].frame_buffer.rBaseAddr.u4VA) ||
                    (OMX_TRUE == mIsSecureInst && pFrameBuf->rSecMemHandle == mFrameBufInt[i].frame_buffer.rSecMemHandle)) &&
                    OMX_TRUE == mFrameBufInt[i].bUsed)
                {
                    if (mFrameBufInt[i].bNonRealDisplay == OMX_TRUE)
                    {
                        if (mNumNotDispAvailOutput > 0)
                        {
                            mNumNotDispAvailOutput--;
                        }
                        mFrameBufInt[i].bNonRealDisplay = OMX_FALSE;
                    }
                    mFrameBufInt[i].bUsed = OMX_FALSE;
                    int refCount = DecRef(BUFFER_OWNER_DEC, pFrameBuf->rBaseAddr.u4VA, (OMX_TRUE == mIsSecureInst));
                    if (0 != refCount)
                    {
                        //MTK_OMX_LOGE("[ERROR] GetFreeBuffer buffer refCount is not 0 (%d)", refCount);
                    }
                    //MTK_OMX_LOGD("GET_FREE, i:%d, frm_buf:%08x, omx_buf:%08x", i, tmp_frame_addr, mFrameBuf[i].ipOutputBuffer);
                    //MTK_OMX_LOGD ("GetFreeBuffer = 0x%x, bUsed:%d,  bFillThis:%d", mFrameBufInt[i].ipOutputBuffer, mFrameBufInt[i].bUsed, mFrameBufInt[i].bFillThis);
                    free(pFrameBuf);
                    UNLOCK(mFillThisBufQLock);
                    return mFrameBufInt[i].ipOutputBuffer;
                }
            }

            for (i = 0; i < mOutputPortDef.nBufferCountActual; i++)
            {
                if (((OMX_FALSE == mIsSecureInst && pFrameBuf->rBaseAddr.u4VA == mFrameBuf[i].frame_buffer.rBaseAddr.u4VA) ||
                    (OMX_TRUE == mIsSecureInst && pFrameBuf->rSecMemHandle == mFrameBuf[i].frame_buffer.rSecMemHandle)) &&
                    OMX_TRUE == mFrameBuf[i].bUsed)
                {
                    if (OMX_TRUE == mFrameBuf[i].bFillThis)
                    {
                        mFrameBuf[i].bUsed = OMX_FALSE;
                        mNumFreeAvailOutput++;
                        MTK_OMX_LOGD("0x%08x SIGNAL mDecodeSem from GetFreeBuffer()", mFrameBuf[i].ipOutputBuffer);
                        int refCount = DecRef(BUFFER_OWNER_DEC, pFrameBuf->rBaseAddr.u4VA, (OMX_TRUE == mIsSecureInst));
                        if (0 != refCount)
                        {
                            //MTK_OMX_LOGE("[ERROR] GetFreeBuffer buffer refCount is not 0 (%d)", refCount);
                        }
                        free(pFrameBuf);
                        SIGNAL(mOutputBufferSem);
                        UNLOCK(mFillThisBufQLock);
                        return mFrameBuf[i].ipOutputBuffer;
                    }
                    if (mFrameBuf[i].bDisplay == OMX_FALSE)
                    {
                        MTK_OMX_LOGD("GetFreeBuffer() not yet display 0x%08x", mFrameBuf[i].ipOutputBuffer);
                        if (pbDisplay != NULL)
                        {
                            if (*pbDisplay == OMX_FALSE)
                            {
                                RemoveForInputPTS();
                            }
                            *pbDisplay = OMX_FALSE;
                        }
                        mFrameBuf[i].bUsed = OMX_FALSE;
                        mNumFreeAvailOutput++;
                        int index = findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, mFrameBuf[i].ipOutputBuffer);
                        if (index < 0)
                        {
                            MTK_OMX_LOGE("[ERROR] GetFree buffer cannot find buffer header 0x%x", mFrameBuf[i].ipOutputBuffer);
                        }
                        int refCount = DecRef(BUFFER_OWNER_DEC, pFrameBuf->rBaseAddr.u4VA, (OMX_TRUE == mIsSecureInst));
                        if (0 != refCount)
                        {
                            //MTK_OMX_LOGE("[ERROR] GetFreeBuffer buffer refCount is not 0 (%d)", refCount);
                        }
                        free(pFrameBuf);
                        mFillThisBufQ.push_back(index);

                        SIGNAL(mOutputBufferSem);
                        UNLOCK(mFillThisBufQLock);
                        return mFrameBuf[i].ipOutputBuffer;
                    }
                    else // MJC is using it, not FBD yet
                    {
                        mFrameBuf[i].bUsed = OMX_FALSE;
                        int refCount = DecRef(BUFFER_OWNER_DEC, pFrameBuf->rBaseAddr.u4VA, (OMX_TRUE == mIsSecureInst));
                        if (0 != refCount)
                        {
                            //MTK_OMX_LOGE("[ERROR] GetFreeBuffer buffer refCount is not 0 (%d)", refCount);
                        }
                        free(pFrameBuf);
                        UNLOCK(mFillThisBufQLock);
                        return mFrameBuf[i].ipOutputBuffer;
                    }
                }
            }
            UNLOCK(mFillThisBufQLock);
        }
        else
        {
            pFrameBuf = (VDEC_DRV_FRAMEBUF_T*)tmp_frame_addr;
            for (i = 0; i < mOutputPortDef.nBufferCountActual; i++)
            {
                if (((OMX_FALSE == mIsSecureInst && pFrameBuf->rBaseAddr.u4VA == mFrameBuf[i].frame_buffer.rBaseAddr.u4VA) ||
                     (OMX_TRUE == mIsSecureInst && pFrameBuf->rSecMemHandle == mFrameBuf[i].frame_buffer.rSecMemHandle)) &&
                      OMX_TRUE == mFrameBuf[i].bUsed)
            {
                LOCK_T(mFillThisBufQLock);
                if (OMX_TRUE == mFrameBuf[i].bFillThis)
                {
                    mFrameBuf[i].bUsed = OMX_FALSE;
                    mNumFreeAvailOutput++;
                    MTK_OMX_LOGD("0x%08x SIGNAL mDecodeSem from GetFreeBuffer()", mFrameBuf[i].ipOutputBuffer);
                    SIGNAL(mOutputBufferSem);
                }
                if (mFrameBuf[i].bDisplay == OMX_FALSE)
                {
                    MTK_OMX_LOGD("GetFreeBuffer() not yet display 0x%08x", mFrameBuf[i].ipOutputBuffer);
                    if (pbDisplay != NULL)
                    {
                        if (*pbDisplay == OMX_FALSE)
                        {
                            RemoveForInputPTS();
                        }
                        *pbDisplay = OMX_FALSE;
                    }
                    mFrameBuf[i].bUsed = OMX_FALSE;
                    free(pFrameBuf);
                    mNumFreeAvailOutput++;
                    int index = findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, mFrameBuf[i].ipOutputBuffer);
                    if (index < 0)
                    {
                        MTK_OMX_LOGE("[ERROR] GetFree buffer cannot find buffer header 0x%x", mFrameBuf[i].ipOutputBuffer);
                    }
                    mFillThisBufQ.push_back(index);
                    SIGNAL(mOutputBufferSem);
                    UNLOCK(mFillThisBufQLock);
                    return mFrameBuf[i].ipOutputBuffer;
                }
                if (mFrameBuf[i].bNonRealDisplay == OMX_TRUE)
                {
                    if (mNumNotDispAvailOutput > 0)
                    {
                        mNumNotDispAvailOutput--;
                    }
                    mFrameBuf[i].bNonRealDisplay = OMX_FALSE;
                }
                mFrameBuf[i].bUsed = OMX_FALSE;
                UNLOCK(mFillThisBufQLock);
                //MTK_OMX_LOGD("GET_FREE, i:%d, frm_buf:%08x, omx_buf:%08x", i, tmp_frame_addr, mFrameBuf[i].ipOutputBuffer);
                MTK_OMX_LOGD ("GetFreeBuffer = 0x%x, bUsed:%d,  bFillThis:%d", mFrameBuf[i].ipOutputBuffer, mFrameBuf[i].bUsed, mFrameBuf[i].bFillThis);
                    free(pFrameBuf);
                return mFrameBuf[i].ipOutputBuffer;
            }
        }
    }
    }
    if (VAL_NULL != tmp_frame_addr)
    {
        MTK_OMX_LOGD("Warning!!GetFreeBuffer() frm_buf: 0x%08x not found !!", tmp_frame_addr);
        goto RECHECK;
    }
    return NULL;
}

VDEC_DRV_FRAMEBUF_T *MtkOmxVdec::GetFrmBuf(OMX_TICKS iTimestamp, OMX_BUFFERHEADERTYPE *ipOutputBuffer)
{
#if (ANDROID_VER >= ANDROID_KK)
    if (OMX_TRUE == mStoreMetaDataInBuffers)
    {
        OMX_U32 graphicBufHandle = 0;
        GetMetaHandleFromOmxHeader(ipOutputBuffer, &graphicBufHandle);

#if (ANDROID_VER >= ANDROID_M)
        WaitFence(ipOutputBuffer, OMX_TRUE);
#endif
            // we are in initial stage and cannot find any buffer handle, find from OMX header
        LOCK_T(mFillThisBufQLock);
        OMX_U32 j = 0;
        for (j = 0; j < mOutputPortDef.nBufferCountActual; j++)
        {
            if (mFrameBuf[j].ipOutputBuffer == ipOutputBuffer)
            {
                if (OMX_FALSE == mFrameBuf[j].bUsed)
                {
                    mFrameBuf[j].bUsed = OMX_TRUE;
                    mFrameBuf[j].bDisplay = OMX_FALSE;
                    mFrameBuf[j].bFillThis = OMX_FALSE;
                    mFrameBuf[j].iTimestamp = iTimestamp;
                    mFrameBuf[j].frame_buffer.u4Timestamp = iTimestamp;
                    mFrameBuf[j].frame_buffer.u4FrameBufferStatus = 0;
                    if (mFrameBuf[j].ionBufHandle == 0)
                    {
                        VBufInfo info;
                        int ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &info);
                        if (ret >= 0)
                        {
                            // found handle
                            MTK_OMX_LOGD("GetFrmBuf found handle, u4BuffHdr(0x%08x)", ipOutputBuffer);
                        }
                        else
                        {
                            // cannot found handle, create a new entry
                            MTK_OMX_LOGE("GetFrmBuf cannot find handle, ERROR %d", __LINE__);
                        }
                        mFrameBuf[j].bGraphicBufHandle = graphicBufHandle;
                        mFrameBuf[j].ionBufHandle = info.ionBufHndl;
                        //MTK_OMX_LOGD("assign mFrameBuf[%d].ionBufHandle = %d", j, info.ionBufHndl);
                    }
                    // TODO: secure meta mode
                    int refCount = 0;
                    VBufInfo info;
                    int ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &info);
                    if (ret >=  0)
                    {
                        refCount = IncRef(BUFFER_OWNER_DEC, info.u4VA, (OMX_TRUE == mIsSecureInst));
                        if ((2 != refCount) && (OMX_TRUE != mIsSecureInst))
                        {
                            MTK_OMX_LOGE("[ERROR] GetFrmBuf buffer refCount is not 2 (%d)", refCount);
                        }
                    }
                    else
                    {
                        MTK_OMX_LOGE("GetFrmBuf cannot find handle, ERROR %d", __LINE__);
                    }
                    UNLOCK(mFillThisBufQLock);
                    return (&mFrameBuf[j].frame_buffer);
                }
                else
                {
                    MTK_OMX_LOGE("[ERROR] GetFrmBuf is not free, LINE:%d", __LINE__);
                }
            }
        }
        UNLOCK(mFillThisBufQLock);

        MTK_OMX_LOGE("[ERROR] GetFrmBuf not found 0x%08x (%d), line: %d", ipOutputBuffer, mOutputPortDef.nBufferCountActual, __LINE__);
        return NULL;
    }
#endif

    LOCK_T(mFillThisBufQLock);
    for (OMX_U32 i = 0; i < mOutputPortDef.nBufferCountActual; i++)
    {
        if (mFrameBuf[i].ipOutputBuffer == ipOutputBuffer)
        {
            if (OMX_FALSE == mFrameBuf[i].bUsed)
            {
                mFrameBuf[i].bUsed = OMX_TRUE;
                mFrameBuf[i].bDisplay = OMX_FALSE;
                mFrameBuf[i].bFillThis = OMX_FALSE;
                mFrameBuf[i].iTimestamp = iTimestamp;
                mFrameBuf[i].frame_buffer.u4Timestamp = iTimestamp;
                mFrameBuf[i].frame_buffer.u4FrameBufferStatus = 0;
                //mFrameBuf[i].ipOutputBuffer = ipOutputBuffer;
                //MTK_OMX_LOGE ("@@ GetFrmBuf frm=0x%x, omx=0x%x, i=%d", &mFrameBuf[i].frame_buffer, ipOutputBuffer, i);
                UNLOCK(mFillThisBufQLock);
                return (&mFrameBuf[i].frame_buffer);
            }
            else
            {
                MTK_OMX_LOGE("[ERROR] GetFrmBuf is not free");
            }
        }
    }
    UNLOCK(mFillThisBufQLock);
    MTK_OMX_LOGE("[ERROR] GetFrmBuf not found 0x%08x, line: %d", ipOutputBuffer, __LINE__);
    return NULL;
}

OMX_U32 MtkOmxVdec::CheckFreeBuffer(OMX_BOOL bLOG)
{
    VAL_UINT32_T i, cnt = 0;
    //MTK_OMX_LOGD ("CheckFreeBuffer+");
    for (i = 0; i < mOutputPortDef.nBufferCountActual; i++)
    {
        if (mFrameBuf == NULL)
        {
            MTK_OMX_LOGE("[CheckFreeBuffer] mFrameBuf = NULL");
        }
        else
        {
            if (i >= mFrameBufSize)
            {
                MTK_OMX_LOGD("[CheckFreeBuffer] index out of range (%d, %d, %d)", i, mFrameBufSize, mOutputPortDef.nBufferCountActual);
                break;
            }
            if (OMX_TRUE == mFrameBuf[i].bUsed)
            {
                cnt++;
                if (bLOG == OMX_TRUE)
                {
                    MTK_OMX_LOGD("NOT_FREE, i:%d, frm_buf:%08x, omx_buf:%08x", i, (VAL_UINT32_T) & (mFrameBuf[i].frame_buffer), (VAL_UINT32_T)mFrameBuf[i].ipOutputBuffer);
                }
            }
        }
    }

    // handle seek (for VC1)
    if (mCorrectTsFromOMX)
    {
        mFirstFrameRetrieved = OMX_FALSE;
        mResetFirstFrameTs = OMX_TRUE;
        mCurrentFrameTs = 0;
    }

    //MTK_OMX_LOGD ("CheckFreeBuffer-, not free cnt:%u", cnt);
    return cnt;
}

VDEC_DRV_RINGBUF_T *MtkOmxVdec::GetInputBuf(OMX_BUFFERHEADERTYPE *ipInputBuf)
{
    OMX_U32 i = 0;
    for (i = 0; i < mInputPortDef.nBufferCountActual; i++)
    {
        if (mInputBuf[i].ipInputBuffer == ipInputBuf)
        {
            if (OMX_FALSE == mInputBuf[i].bUsed)
            {
                mInputBuf[i].bUsed = OMX_TRUE;
                mInputBuf[i].ipInputBuffer = ipInputBuf;
                return (&mInputBuf[i].InputBuf);
            }
            else
            {
                MTK_OMX_LOGE("[ERROR] GetInputBuf is not free");
            }
        }
    }
    MTK_OMX_LOGE("[ERROR] GetInputBuf not found 0x%08x", ipInputBuf);
    return NULL;
}

OMX_BOOL MtkOmxVdec::HandleViLTEInitHWErr(void)
{
    const OMX_U32 reTryThreshold = 30;
    const OMX_U32 reTryTimeout = reTryThreshold * 10;
    if(mFailInitCounter >= reTryTimeout)
    {
        MTK_OMX_LOGE("[VILTE] InitVideoDecodeHW failed over %d times",mFailInitCounter);
        return OMX_TRUE;
    }
    else
    {
        if (mFIRSent == OMX_FALSE)
        {
            if (mFailInitCounter % reTryThreshold == reTryThreshold - 1)
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                    mAppData,
                                    OMX_EventError,
                                    OMX_ErrorFullIntraRequestStart,
                                    NULL,
                                    NULL);
                mFIRSent = OMX_TRUE;
            }
        }
    }
    return OMX_FALSE;
}

void MtkOmxVdec::HandleViLTEBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    if (VDEC_DRV_MRESULT_OK == eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_ERRORMB_MAP, NULL, &mrErrorMBInfo) /* && pBuffHdr->nFilledLen != 0 */ )
    {
        mContinuousSliceLoss++;

        if (mContinuousSliceLoss >= mPLIThres &&
            mContinuousSliceLoss % mPLIThres == 0) // serious slice loss
        {
            if (mContinuousSliceLoss % mResendPLIThres == 1)
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                    mAppData,
                                    OMX_EventError,
                                    OMX_ErrorPictureLossIndication,
                                    NULL,
                                    NULL);
            }
            else
            {
                MTK_OMX_LOGD("[VILTE][HandleViLTEBufferDone] mContinuousSliceLoss = %d, mResendPLIThres = %d", mContinuousSliceLoss, mResendPLIThres);
            }
        }
        else if (pBuffHdr != NULL)
        {
            mSLI.nTimeStamp = pBuffHdr->nTimeStamp;
            mSLI.nSliceCount = 1;
            mSLI.SliceLoss[0] = (mrErrorMBInfo.u4ErrorStartMB << 19) |
                                (mrErrorMBInfo.u4ErrorMBCount << 6) |
                                (mrErrorMBInfo.u4ErrorFrameNum);
            MTK_OMX_LOGD("[VILTE][SLI][%d][%d] start:%d, count:%d, 0x%x",
                            mContinuousSliceLoss,
                            mrErrorMBInfo.u4ErrorFrameNum,
                            mrErrorMBInfo.u4ErrorStartMB,
                            mrErrorMBInfo.u4ErrorMBCount,
                            mSLI.SliceLoss[0]);
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                mAppData,
                                OMX_EventError,
                                OMX_ErrorSliceLossIndication,
                                NULL,
                                NULL);
        }
        else
        {
            MTK_OMX_LOGD("[VILTE][SLI] HandleViLTEBufferDone pBuffHdr = NULL");
        }
    }
    else
    {
        mContinuousSliceLoss = 0;
    }
}

OMX_BOOL MtkOmxVdec::IsAVPFEnabled(void)
{
    if (OMX_TRUE == mViLTESupportOn && OMX_TRUE == mAVPFEnable)
    {
        return OMX_TRUE;
    }
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::dumpBuffer(char *name, unsigned char *data, int size)
{
    FILE *fp = fopen(name, "ab");
    char newFile[256]={0};
    if(!fp)
    {
         char *ptsrc =NULL;
         if(0==memcmp("/data/",name, 6))
         {
            ptsrc =(char *)(name +6);
            memcpy(newFile,"/sdcard/",8);
            if (strlen(ptsrc) < 256)
            {
                memcpy(newFile+8,ptsrc, strlen(ptsrc));
            }
            else
            {
                MTK_OMX_LOGE("open file %s fail: %d %s, file path too long", name, errno, strerror(errno));
                return OMX_TRUE;
            }
         }
         else  if(0==memcmp("/sdcard/",name, 8))
         {
            ptsrc =(char *)(name +8);
            memcpy(newFile,"/data/",6);
            if (strlen(ptsrc) < 256)
            {
                memcpy(newFile+6,ptsrc, strlen(ptsrc));
            }
            else
            {
                MTK_OMX_LOGE("open file %s fail: %d %s, file path too long", name, errno, strerror(errno));
                return OMX_TRUE;
            }
         }
         else
         {
             MTK_OMX_LOGE("open file %s fail: %d %s,and new file not support", name, errno, strerror(errno));
             return OMX_TRUE;
         }
         MTK_OMX_LOGE("open file %s fail,and switch to %s\n", name, newFile);
         fp = fopen(newFile, "ab");
    }

    if (fp)
    {
        fwrite((void *)data, 1, size, fp);
        fclose(fp);
    }
    else
    {
        MTK_OMX_LOGE("open file %s fail: %d %s", name, errno, strerror(errno));
    }
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::dumpInputBuffer(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_U8 *aInputBuf, OMX_U32 aInputSize)
{
    char name[128];

    {
        char value[PROPERTY_VALUE_MAX];

        if (OMX_TRUE == mViLTESupportOn)
        {
            VAL_BOOL_T bDump = 0;

            property_get("vendor.mtk.omx.vdec.vilte.dump.bs", value, "0");
            bDump = atoi(value);
            if (bDump)
            {
                sprintf(name, "/sdcard/vilte_vdec_bs_%d.bin", gettid());
                dumpBuffer(name, (unsigned char *)aInputBuf, aInputSize);
            }
        }
    }
    return OMX_TRUE;
}

void MtkOmxVdec::UpdateColorAspectsParams()
{
    VDEC_DRV_COLORDESC_T u4ColorDesc;

    memset(&u4ColorDesc, 0x0, sizeof(u4ColorDesc));
    if (VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_GET_COLOR_DESC, NULL, &u4ColorDesc))
    {
        MTK_OMX_LOGD("[ERROR] UpdateColorAspectsParams: VDEC_DRV_GET_TYPE_GET_COLOR_DESC failed");
    }

    switch (u4ColorDesc.u4FullRange)
    {
        case 0:
            mDescribeColorAspectsParams.sAspects.mRange = ColorAspects::RangeLimited;
            break;
        case 1:
            mDescribeColorAspectsParams.sAspects.mRange = ColorAspects::RangeFull;
            break;
        default:
            mDescribeColorAspectsParams.sAspects.mRange = ColorAspects::RangeOther;
            break;
    }
    switch (u4ColorDesc.u4ColorPrimaries)
    {
        case 1:
            mDescribeColorAspectsParams.sAspects.mPrimaries = ColorAspects::PrimariesBT709_5;
            break;
        case 2:
            mDescribeColorAspectsParams.sAspects.mPrimaries = ColorAspects::PrimariesUnspecified;
            break;
        case 4:
            mDescribeColorAspectsParams.sAspects.mPrimaries = ColorAspects::PrimariesBT470_6M;
            break;
        case 5:
            mDescribeColorAspectsParams.sAspects.mPrimaries = ColorAspects::PrimariesBT601_6_625;
            break;
        case 6:
        case 7:
            mDescribeColorAspectsParams.sAspects.mPrimaries = ColorAspects::PrimariesBT601_6_525;
            break;
        case 8:
            mDescribeColorAspectsParams.sAspects.mPrimaries = ColorAspects::PrimariesGenericFilm;
            break;
        case 9:
            mDescribeColorAspectsParams.sAspects.mPrimaries = ColorAspects::PrimariesBT2020;
            break;
        default:
            mDescribeColorAspectsParams.sAspects.mPrimaries = ColorAspects::PrimariesOther;
            break;
    }
    switch (u4ColorDesc.u4TransformCharacter)
    {
        case 1:
        case 6:
        case 14:
        case 15:
            mDescribeColorAspectsParams.sAspects.mTransfer = ColorAspects::TransferSMPTE170M;
            break;
        case 2:
            mDescribeColorAspectsParams.sAspects.mTransfer = ColorAspects::TransferUnspecified;
            break;
        case 4:
            mDescribeColorAspectsParams.sAspects.mTransfer = ColorAspects::TransferGamma22;
            break;
        case 5:
            mDescribeColorAspectsParams.sAspects.mTransfer = ColorAspects::TransferGamma28;
            break;
        case 7:
            mDescribeColorAspectsParams.sAspects.mTransfer = ColorAspects::TransferSMPTE240M;
            break;
        case 8:
            mDescribeColorAspectsParams.sAspects.mTransfer = ColorAspects::TransferLinear;
            break;
        case 11:
            mDescribeColorAspectsParams.sAspects.mTransfer = ColorAspects::TransferXvYCC;
            break;
        case 12:
            mDescribeColorAspectsParams.sAspects.mTransfer = ColorAspects::TransferBT1361;
            break;
        case 13:
            mDescribeColorAspectsParams.sAspects.mTransfer = ColorAspects::TransferSRGB;
            break;
        case 16:
            mDescribeColorAspectsParams.sAspects.mTransfer = ColorAspects::TransferST2084;
            break;
        case 17:
            mDescribeColorAspectsParams.sAspects.mTransfer = ColorAspects::TransferST428;
            break;
        case 18:
            mDescribeColorAspectsParams.sAspects.mTransfer = ColorAspects::TransferHLG;
            break;
        default:
            mDescribeColorAspectsParams.sAspects.mTransfer = ColorAspects::TransferOther;
            break;
    }
    switch (u4ColorDesc.u4MatrixCoeffs)
    {
        case 1:
            mDescribeColorAspectsParams.sAspects.mMatrixCoeffs = ColorAspects::MatrixBT709_5;
            break;
        case 2:
            mDescribeColorAspectsParams.sAspects.mMatrixCoeffs = ColorAspects::MatrixUnspecified;
            break;
        case 4:
            mDescribeColorAspectsParams.sAspects.mMatrixCoeffs = ColorAspects::MatrixBT470_6M;
            break;
        case 5:
        case 6:
            mDescribeColorAspectsParams.sAspects.mMatrixCoeffs = ColorAspects::MatrixBT601_6;
            break;
        case 7:
            mDescribeColorAspectsParams.sAspects.mMatrixCoeffs = ColorAspects::MatrixSMPTE240M;
            break;
        case 9:
            mDescribeColorAspectsParams.sAspects.mMatrixCoeffs = ColorAspects::MatrixBT2020;
            break;
        case 10:
            mDescribeColorAspectsParams.sAspects.mMatrixCoeffs = ColorAspects::MatrixBT2020Constant;
            break;
        default:
            mDescribeColorAspectsParams.sAspects.mMatrixCoeffs = ColorAspects::MatrixOther;
            break;
    }
    MTK_OMX_LOGD("[Debug][UpdateColorAspectsParams] R: %d, P: %d, T: %d, M: %d\n",
        mDescribeColorAspectsParams.sAspects.mRange,
        mDescribeColorAspectsParams.sAspects.mPrimaries,
        mDescribeColorAspectsParams.sAspects.mTransfer,
        mDescribeColorAspectsParams.sAspects.mMatrixCoeffs);
}
