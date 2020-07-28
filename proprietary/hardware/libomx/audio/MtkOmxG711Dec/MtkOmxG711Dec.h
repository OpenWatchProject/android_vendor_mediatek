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

#ifndef MTK_OMX_G711_DEC
#define MTK_OMX_G711_DEC


#include "MtkOmxAudioDecBase.h"

#define MTK_OMX_INPUT_BUFFER_SIZE_G711 32768
#define MTK_OMX_OUTPUT_BUFFER_SIZE_G711 32768
#define MTK_OMX_NUMBER_INPUT_BUFFER_G711  10
#define MTK_OMX_NUMBER_OUTPUT_BUFFER_G711  9
#define KMaxNumSamplesPerFrame  16384


class MtkOmxG711Dec : public MtkOmxAudioDecBase
{

public:
    MtkOmxG711Dec();
    ~MtkOmxG711Dec();

    OMX_BOOL  OmxG711DecInit();
    void  QueueInputBuffer(int index);//copy mtkomxvdec
    void  QueueOutputBuffer(int index);//Queue the buffer as first Output Buffer

    // override base class functions:
    virtual OMX_BOOL InitAudioParams();
    virtual void DecodeAudio(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf);
    virtual void FlushAudioDecoder();
    virtual void DeinitAudioDecoder();

private:
    void G711Dump(OMX_BUFFERHEADERTYPE *pBuf);//for dump pcm
    //G711
    void DecodeMLaw(OMX_S16 *out, OMX_U8 *in, OMX_U32 inSize);//MLaw Codec
    void DecodeALaw(OMX_S16 *out, OMX_U8 *in, OMX_U32 inSize);//ALaw Codec

    OMX_BOOL mG711InitFlag;//Init flag
    bool mIsMLaw;//ALaw or MLaw Flag

};



#endif  //MTK_OMX_G711_DEC
