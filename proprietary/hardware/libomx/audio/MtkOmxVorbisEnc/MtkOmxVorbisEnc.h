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

#ifndef MTK_OMX_VORBIS_ENC
#define MTK_OMX_VORBIS_ENC


#include "MtkOmxAudioEncBase.h"
#include "minvorbis_encoder_exp.h"

// frame length is 20 milliseconds i.e. 20000 omx ticks/microseconds
//#define MTK_OMX_AMR_FRAME_LENGTH_IN_TIMESTAMP 20000
#define MTK_OMX_MAX_VORBIS_FRAME_SIZE 4096

//pcm buffer 8192
#define MTK_OMX_INPUT_BUFFER_SIZE_VORBIS_ENC         (2 * MTK_OMX_MAX_VORBIS_FRAME_SIZE)
//bit stream buffer 8192
#define MTK_OMX_OUTPUT_BUFFER_SIZE_VORBIS_ENC        (2 * MTK_OMX_MAX_VORBIS_FRAME_SIZE)

#define MTK_OMX_NUMBER_INPUT_BUFFER_VORBIS_ENC  6
#define MTK_OMX_NUMBER_OUTPUT_BUFFER_VORBIS_ENC  8

typedef struct
{
    OMX_U8      *buffer_base;
    OMX_U8      *write_ptr;
    OMX_U32      buffer_size;
    OMX_BOOL     buffer_partial;
    OMX_BOOL     buffer_full;
} vorbisEncTempBuffer;

class MtkOmxVorbisEnc : public MtkOmxAudioEncBase
{
public:
    MtkOmxVorbisEnc();
    ~MtkOmxVorbisEnc();

    // override base class functions:
    virtual OMX_BOOL InitAudioParams();
    virtual void EncodeAudio(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf);
    virtual int EncodeAudio();	
    virtual void FlushAudioEncoder();
    virtual void DeinitAudioEncoder();

private:

    enum
    {
        kNumSamplesPerFrame = 1024,
    };

    int32_t     m_bitrate;
    int32_t         m_samplerate;
    int32_t         m_ChannelCount;
    int mHeadSize;
    vorbisEncTempBuffer *mVorbisTempBuf;
    OMX_BOOL mVorbisEncInit;
    int32_t mFrameCount;
    int32_t mEosTimes;
    int sz_shared, sz_encoder, sz_parser, sz_pcm_in, sz_bs_out, sz_rt_tab;
    void *p_pcm_in, *p_bs_out, *p_shared, *p_encoder, *p_parser, *p_rt_tab;
    minvorbis_enc_handle h_minvorbis;
    //next frame timestamp
    OMX_TICKS m_NextTimeStamp;
    //first frame timestamp
    OMX_TICKS m_firstTimeStamp;
    OMX_TICKS m_FrameTime;
    //Dump File
    const char *mDumpProp;
    const char *mDumpFile;
    //
    int mdealsize;	

//refer to minvorbis_encoder_exp.h
    //default quality = 1, 2 high quality
    int fn_GetBitRate(int pSampleRate, int pQuality = 1);
    bool InitVorbisEncoder();
    //This function is not called by MtkOmxAudioEncBase::SetParameter,OMX_IndexParamAudioVorbis, just for virtual function implementation
    OMX_ERRORTYPE CheckParams(OMX_PTR params);
    void fn_ErrHandle(const char *pErrMsg, OMX_ERRORTYPE pErrType,
                      OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf);

};


#endif  //MTK_OMX_VORBIS_ENC

