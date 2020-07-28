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

#ifndef MTK_OMX_MP3_DEC
#define MTK_OMX_MP3_DEC

#include "MtkOmxAudioDecBase.h"

#include "mp3dec_exp.h"

#define MTK_CODEC_NEW

typedef struct
{
    Mp3dec_handle *handle;
    int workingbuf_size1;
    int workingbuf_size2;
    int min_bs_size;
    int pcm_size;
    void *working_buf1;
    void *working_buf2;
} mp3DecEngine;

typedef enum
{
    MP3DEC_HANDLE_NULL                 =  -1,
    MP3DEC_PCMBUF_NULL                 =  -2,
    MP3DEC_PCMBUF_NOTALIGN             =  -3,
    MP3DEC_BSBUF_NULL                  =  -4,
    MP3DEC_BSREAD_NULL                 =  -5,
    MP3DEC_BSBUF_SIZE_INVALID          =  -6
} mp3DecErrorCode;

/*8192  one mp3 frame max length 1441 byte*/
#define MTK_OMX_INPUT_BUFFER_SIZE_MP3 (1 << 16)
#define MTK_OMX_OUTPUT_BUFFER_SIZE_MP3 4608

#define MTK_OMX_NUMBER_INPUT_BUFFER_MP3  4
#define MTK_OMX_NUMBER_OUTPUT_BUFFER_MP3  4

class MtkOmxMp3Dec : public MtkOmxAudioDecBase
{
public:
    MtkOmxMp3Dec();
    ~MtkOmxMp3Dec();

    // override base class functions:
    virtual OMX_BOOL InitAudioParams();
    virtual void DecodeAudio(OMX_BUFFERHEADERTYPE *pInputBuf,
            OMX_BUFFERHEADERTYPE *pOutputBuf);
    virtual void FlushAudioDecoder();
    virtual void DeinitAudioDecoder();

    virtual OMX_ERRORTYPE SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_INDEXTYPE nParamIndex,OMX_IN OMX_PTR ComponentParameterStructure);
    virtual OMX_ERRORTYPE GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_INDEXTYPE nParamIndex,OMX_INOUT OMX_PTR ComponentParameterStructure);

    //copy mtkomxvdec queue the bufffer in front of the queue
    void  QueueInputBuffer(int index);
    // copy the bufffer in front of the queue
    void  QueueOutputBuffer(int index);

private:

    // mp3 related
    bool InitMp3Decoder(OMX_U8 *pInputBuffer);
    mp3DecEngine *mMp3Dec;

    OMX_BOOL mMp3InitFlag;
    void RampUp(OMX_S16 *RampBuff, OMX_U32 HalfRampSample);
    void ErrorDeclare(OMX_S32 ulBytesConsumed);
    int  Get_sample_rate(const int sf_index);
    void clearComponetAndThread();
    OMX_U32 U32_AT(const OMX_U8 *ptr);
    bool GetMPEGAudioFrameSize(
        OMX_U32 header, int *frame_size,
        int *out_sampling_rate = NULL, int *out_channels = NULL,
        int *out_bitrate = NULL, int *out_num_samples = NULL);

    bool isMp3FormatChanged(OMX_U8* pBitStreamRead /*input mp3 frame*/,
                                    int* sample_rate, int* num_channels);
    void handleFormatChanged(OMX_U8* pBitStreamRead /*input mp3 frame*/,
                                    int sample_rate, int num_channels);

    OMX_BOOL    mNewOutBufferRequired; //required
    OMX_BOOL    mNewInBufferRequired;
    OMX_U32      mOutputFrameLength;//output buffer length
    OMX_U32      mSeekFlagFrameCount;//Seek Process
#ifndef MTK_CODEC_NEW
    //MTK_OMX_MP3_LOW_POWER
    OMX_U32 mPreBufferPrepareFlag;
    int mPreBSBuffSize;//allocat temp buffer size for ringbuffer decoding
    OMX_U8   *pRead, *pWrite; //prebuffer read buffer's pointer
    OMX_U8   *pPreBS;//prebuffer's pointer
    int mBSBuffSize, size_PreBUF;
    OMX_U32 mSampleRate;
    //
#endif
    enum
    {
        kPVMP3DecoderDelay = 529 // frames
    };

    OMX_U32 mOutputNum;
    OMX_U32 mBufferLength;
    OMX_U8 *pLastBitStreamRead;
    OMX_BOOL mEndFlag;
    OMX_BOOL mIsEndOfStream;
    OMX_TICKS mLastTimeStamp;
    OMX_U32 mInSampleCount;

    OMX_BOOL mEOSAfterSeek;

#ifndef FOR_CTS_TEST
    bool isFirst;
    bool isOffsetFirst;
#endif
    OMX_BOOL mFirstFrameFlag;
    OMX_U32 mLastSampleCount;

    OMX_TICKS lastTimeStamp;
    OMX_TICKS lastInputTimeStamp;

};


#endif  //MTK_OMX_MP3_DEC
