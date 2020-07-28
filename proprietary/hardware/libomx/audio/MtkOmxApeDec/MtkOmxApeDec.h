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

#ifndef MTK_OMX_APE_DEC
#define MTK_OMX_APE_DEC

#include "MtkOmxAudioDecBase.h"
#include "BpMtkCodecService.h"

#include <android/hidl/allocator/1.0/IAllocator.h>
#include <hidlmemory/mapping.h>

//#include <binder/Parcel.h>
//#include <binder/IMemory.h>

///#define MTK_OMX_INPUT_BUFFER_SIZE_APE 1536
///#define MTK_OMX_OUTPUT_BUFFER_SIZE_APE 8192
#define MTK_OMX_NUMBER_INPUT_BUFFER_APE  2
#define MTK_OMX_NUMBER_OUTPUT_BUFFER_APE  9

using ::android::hidl::allocator::V1_0::IAllocator;
using ::android::hardware::hidl_memory;

typedef struct tMTKAPEDecoderExternal
{

    unsigned char       *pInputBuffer;
    int                 inputBufferUsedLength;
    int                 outputFrameSize;
    unsigned char       *pOutputBuffer;

} mtkAPEDecoderExternal;


class MtkOmxApeDec : public MtkOmxAudioDecBase
{
public:
    MtkOmxApeDec();
    ~MtkOmxApeDec();

    OMX_BOOL  OmxApeDecInit(OMX_BUFFERHEADERTYPE * pInputBuf); //decinit
    void  QueueInputBuffer(int index);//copy mtkomxvdec queue the bufffer in front of the queue
    void  QueueOutputBuffer(int index);// copy the bufffer in front of the queue

    // override base class functions:
    virtual OMX_BOOL InitAudioParams();
    virtual void DecodeAudio(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf);
    virtual void FlushAudioDecoder();
    virtual void DeinitAudioDecoder();
    bool IsMtkCodecSrvReady();

   //for update APE paramters
    OMX_ERRORTYPE SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_INDEXTYPE nParamIndex,
        OMX_IN OMX_PTR ComponentParameterStructure);

private:
    void APEDump(OMX_BUFFERHEADERTYPE *pBuf);

    void RampUp(int16_t *aRampBuff, uint32_t aHalfRampSample);//
    void RampUp(int8_t *aRampBuff, uint32_t aHalfRampSample);//
    // APE related

    OMX_BOOL mApeInitFlag; //Init flag
    OMX_S32 mFrameCount;//frame num

    OMX_U32 mInputCurrLength;// input current length
    OMX_U32 mSamplesPerFrame __unused; // sample per frame
    OMX_BOOL    mNewOutBufRequired; //required
    OMX_BOOL    mNewInBufferRequired;
    OMX_U32 mOutputFrameLength;//output buffer length

    bool mStarted;
    bool mSeekEnable;

    mtkAPEDecoderExternal mApeConfig __unused;
    int64_t mNumFramesOutput __unused;

    OMX_BOOL mCRCError;
    size_t mCRCZeroData __unused;
    int32_t mCurrentFrame __unused;
    int64_t mCurrentTime;

    int64_t mSeekTimeUs;
    size_t mSeekRampup;
    size_t mSeekMute;

    OMX_TICKS  mLastTimeStamp;//last buffer 's timestamp
    int mResetFlag __unused;//ramp up
    unsigned int in_size;
    unsigned int out_size;
    sp<BpMtkCodecService> pMtkCodec;

    hidl_memory m_hidl_memory_in;
    sp<IMemory> mapped_memory_input;
	//void *mStagefrightHandle;
	//sp<IMemory> (*getIMemoryFromBufferMeta)(OMX_PTR);
    void fn_ErrHandle(const char *pErrMsg, int pErrType, int pCodingType,
                      OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf, bool isBufQue);
};
#endif  //MTK_OMX_APE_DEC

