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
 *   MtkOmxGsmDec.h
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX GSM Decoder component
 *
 * Author:
 * -------
 *   
 *
 ****************************************************************************/


#ifndef _MTK_OMX_GSM_DEC_
#define _MTK_OMX_GSM_DEC_

#include "MtkOmxAudioDecBase.h"

extern "C" {
#include "gsm.h"
}



#define MTK_OMX_INPUT_BUFFER_SIZE_GSM     2048        // 2048/65 * 320 / 8000 = 1.24s
#define MTK_OMX_OUTPUT_BUFFER_SIZE_GSM    19840       //31 * 320 * 2                                     

#define MTK_OMX_NUMBER_INPUT_BUFFER_GSM   4
#define MTK_OMX_NUMBER_OUTPUT_BUFFER_GSM  4

#define MTK_OMX_GSM_DEFAULT_CHANNEL_NUMBERS 1
#define MTK_OMX_GSM_DEFAULT_SAMPLINGRATE    8000
#define MTK_OMX_GSM_DEFAULT_BITS_PER_SAMPLE 16

#define PCM_SIZE_FOR_TWO_FRAME 320*2


#define MTK_OMX_GSM_DUMP_DEC_OUTPUT_DATA

class MtkOmxGsmDec : public MtkOmxAudioDecBase
{
	public:
		MtkOmxGsmDec();
		virtual ~MtkOmxGsmDec();
		virtual OMX_BOOL InitAudioParams();
		virtual void DecodeAudio(OMX_BUFFERHEADERTYPE* pInputBuffer,OMX_BUFFERHEADERTYPE* pOutputBuffer);
		virtual void FlushAudioDecoder();
		virtual void DeinitAudioDecoder();

		//for update GSM paramters
		virtual OMX_ERRORTYPE GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
												OMX_IN OMX_INDEXTYPE nParamIndex,
												OMX_INOUT OMX_PTR ComponentParameterStructure);
		virtual OMX_ERRORTYPE SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
												OMX_IN OMX_INDEXTYPE nParamIndex,
												OMX_IN OMX_PTR ComponentParameterStructure);

	private:
		OMX_BOOL initGSMDecoder(OMX_BUFFERHEADERTYPE* pInputBuffer);
		void handleBuffers(OMX_BUFFERHEADERTYPE* pInputBuf,
								OMX_BUFFERHEADERTYPE* pOutputBuf, 
								 OMX_BOOL emptyInput, 
								  OMX_BOOL fillOutput);
		static int DecodeGSM(gsm handle, int16_t *out, uint8_t *in, size_t inSize);
		
		OMX_BOOL mInitFlag;

		OMX_BOOL mFirstFrameFlag;       // for computing time stamp 
		OMX_U32 mLastSampleCount;

		OMX_TICKS lastTimeStamp;
		OMX_BOOL isNewOutputBuffer;

		OMX_U16 maxNumFramesInOutputBuf;
		OMX_U16 numFramesInOutputBuf;
		OMX_U32 totalConsumedBytes;  //just for one input buffer
		OMX_U32 mSamplingRate;
		OMX_U32 mNumChannels;

		gsm mGsm;
};
#endif 



