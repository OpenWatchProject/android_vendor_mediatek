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
 *   MtkOmxRawDec.h
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX Raw Pcm  Decoder component
 *
 * Author:
 * -------
 *
 *
 ****************************************************************************/


#ifndef _MTK_OMX_RAW_DEC_
#define _MTK_OMX_RAW_DEC_

#include "MtkOmxAudioDecBase.h"
#include <utils/Vector.h>
#include <utils/KeyedVector.h>



#define MTK_OMX_INPUT_BUFFER_SIZE_RAW     65536     //64k,     default: 49152  == 48k
#define MTK_OMX_OUTPUT_BUFFER_SIZE_RAW    32768 << 1

#define MTK_OMX_NUMBER_INPUT_BUFFER_RAW   10
#define MTK_OMX_NUMBER_OUTPUT_BUFFER_RAW  9

#define MTK_OMX_AUDIO_DEFAULT_CHANNEL_NUMBERS 2
#define MTK_OMX_AUDIO_DEFAULT_SAMPLINGRATE 44100
#define MTK_OMX_AUDIO_DEFAULT_BITS_PER_SAMPLE 16

#define SAMPLE_NUM_EVERY_DECODE 1024 * 4

#define ENHANCE_CHANNEL_ASSIGNMENT
#define MY_TEST                           //Factory and XML file parse.

// typedef uint32_t char32_t;
// typedef uint16_t char16_t;


//channel assignment
typedef enum AUDIO_CHANNEL_ASSIGNMENT{
	NONE					= 0x0,

	//1channel
	C						= 0x1,

	//2channel
	L_R						= 0x10,

	//3channel
	L_R_C                   = 0x100,
	L_R_Bc                  = 0x101,
	C_L_R                   = 0x102,
	L_C_R                   = 0x103,
	L_R_Lfe                 = 0x104,

	//4channel
	L_R_C_Bc                = 0x1000,
	L_R_Bl_Br               = 0x1001,
	L_R_Sl_Sr               = 0x1002,
	C_L_R_Ls                = 0x1003,
	L_R_Ls_Rs               = 0x1004,
	L_R_Lfe_Bc              = 0x1005,
	L_R_C_Lfe               = 0x1006,
	L_R_C_Z                 = 0x1007,
	L_R_Bc_Z                = 0x1008,
	L_C_R_Bc                = 0x1009,

	//5channel
	L_R_C_Bl_Br             = 0x10000,
	L_R_C_Sl_Sr             = 0x10001,
	C_L_R_Ls_Rs             = 0x10002,
	L_C_R_Ls_Rs             = 0x10003,
	L_R_Lfe_Ls_Rs           = 0x10004,
	L_R_C_Ls_Rs             = 0x10005,
	L_R_C_Lfe_Bc            = 0x10006,
	L_R_Ls_Rs_Lfe           = 0x10007,
	L_R_Ls_Rs_C             = 0x10008,

	//6channel
	L_R_C_Lfe_Bl_Br         = 0x100000,
	L_R_C_Lfe_Sl_Sr         = 0x100001,
	L_R_C_Bl_Br_Bc          = 0x100002,
	L_R_C_Bc_Sl_Sr          = 0x100003,
	C_L_R_Ls_Rs_Lfe         = 0x100004,
	L_C_R_Ls_Rs_Lfe         = 0x100005,
	L_R_C_Lfe_Ls_Rs         = 0x100006,
	L_R_Ls_Rs_C_Lfe         = 0x100007,
	L_R_C_Ls_Rs_Z           = 0x100008,
	L_R_C_Ls_Rs_Lfe         = 0x100009,

	//7channel
	L_R_C_Bl_Br_Sl_Sr       = 0x1000000,

	//8channel
	L_R_C_Lfe_Bl_Br_Sl_Sr   = 0x10000000,
	C_L_R_Ls_Rs_Rls_Rrs_Lfe = 0x10000001,
	L_C_R_Ls_Rs_Rls_Rrs_Lfe = 0x10000002,
	L_R_C_Ls_Rls_Rrs_Rs_Z   = 0x10000003,
	L_R_C_Ls_Rls_Rrs_Rs_Lfe = 0x10000004
}AUDIO_CHANNEL_ASSIGNMENT;

#ifdef ENHANCE_CHANNEL_ASSIGNMENT
typedef struct ChannelAssign
{
	OMX_AUDIO_PCMTYPE rawType;
	OMX_U32 value;
	AUDIO_CHANNEL_ASSIGNMENT chnAssign;
}ChannelAssign;

ChannelAssign FileChnAssign[] =
		{
			{PCM_DVD_AOB, 0x00000b, C},
			{PCM_DVD_AOB, 0x00001b, L_R},
			{PCM_DVD_AOB, 0x00010b, L_R_Bc},
			{PCM_DVD_AOB, 0x00011b, L_R_Ls_Rs},
			{PCM_DVD_AOB, 0x00100b, L_R_Lfe},
			{PCM_DVD_AOB, 0x00101b, L_R_Lfe_Bc},
			{PCM_DVD_AOB, 0x00110b, L_R_Lfe_Ls_Rs},
			{PCM_DVD_AOB, 0x00111b, L_R_C},
			{PCM_DVD_AOB, 0x01000b, L_R_C_Bc},
			{PCM_DVD_AOB, 0x01001b, L_R_C_Ls_Rs},
			{PCM_DVD_AOB, 0x01010b, L_R_C_Lfe},
			{PCM_DVD_AOB, 0x01011b, L_R_C_Lfe_Bc},
			{PCM_DVD_AOB, 0x01100b, L_R_C_Lfe_Ls_Rs},
			{PCM_DVD_AOB, 0x01101b, L_R_C_Bc},
			{PCM_DVD_AOB, 0x01110b, L_R_C_Ls_Rs},
			{PCM_DVD_AOB, 0x01111b, L_R_C_Lfe},
			{PCM_DVD_AOB, 0x10000b, L_R_C_Lfe_Bc},
			{PCM_DVD_AOB, 0x10001b, L_R_C_Lfe_Ls_Rs},
			{PCM_DVD_AOB, 0x10010b, L_R_Ls_Rs_Lfe},
			{PCM_DVD_AOB, 0x10011b, L_R_Ls_Rs_C},
			{PCM_DVD_AOB, 0x10100b, L_R_Ls_Rs_C_Lfe},
			{PCM_DVD_VOB, 0x00000b, C},
			{PCM_DVD_VOB, 0x00001b, L_R},
			{PCM_DVD_VOB, 0x00010b, L_R_Bc},
			{PCM_DVD_VOB, 0x00011b, L_R_Ls_Rs},
			{PCM_DVD_VOB, 0x00100b, L_R_Lfe},
			{PCM_DVD_VOB, 0x00101b, L_R_Lfe_Bc},
			{PCM_DVD_VOB, 0x00110b, L_R_Lfe_Ls_Rs},
			{PCM_DVD_VOB, 0x00111b, L_R_C},
			{PCM_DVD_VOB, 0x01000b, L_R_C_Bc},
			{PCM_DVD_VOB, 0x01001b, L_R_C_Ls_Rs},
			{PCM_DVD_VOB, 0x01010b, L_R_C_Lfe},
			{PCM_DVD_VOB, 0x01011b, L_R_C_Lfe_Bc},
			{PCM_DVD_VOB, 0x01100b, L_R_C_Lfe_Ls_Rs},
			{PCM_DVD_VOB, 0x01101b, L_R_Bc},
			{PCM_DVD_VOB, 0x01110b, L_R_Ls_Rs},
			{PCM_DVD_VOB, 0x01111b, L_R_Lfe},
			{PCM_DVD_VOB, 0x10000b, L_R_Lfe_Bc},
			{PCM_DVD_VOB, 0x10001b, L_R_Lfe_Ls_Rs},
			{PCM_DVD_VOB, 0x10010b, L_R_C},
			{PCM_DVD_VOB, 0x10011b, L_R_C_Bc},
			{PCM_DVD_VOB, 0x10100b, L_R_C_Lfe_Ls_Rs},
			{PCM_DVD_VOB, 0x10101b, L_R_C_Lfe},
			{PCM_DVD_VOB, 0x10110b, L_R_C_Lfe_Bc},
			{PCM_DVD_VOB, 0x10111b, L_R_C_Lfe_Ls_Rs},
			{PCM_BD 	,		 1, C},
			{PCM_BD 	,		 3, L_R},
			{PCM_BD     ,        4, L_R_C_Z},
			{PCM_BD     ,        5, L_R_Bc_Z},
			{PCM_BD     ,        6, L_R_C_Bc},
			{PCM_BD     ,        7, L_R_Ls_Rs},
			{PCM_BD     ,        8, L_R_C_Ls_Rs_Z},
			{PCM_BD     ,        9, L_R_C_Ls_Rs_Lfe},
			{PCM_BD     ,        10,L_R_C_Ls_Rls_Rrs_Rs_Z},
			{PCM_BD     ,        11,L_R_C_Ls_Rls_Rrs_Rs_Lfe}
		};
#endif

typedef struct TEMPBUFFER
{
	int16_t* tempBufPointer;
	OMX_U32 bufferSize;

}TempBuffer;

class MtkOmxRawDec : public MtkOmxAudioDecBase
{
	public:
		MtkOmxRawDec();
		virtual ~MtkOmxRawDec();
		virtual OMX_BOOL InitAudioParams();
		virtual void DecodeAudio(OMX_BUFFERHEADERTYPE* pInputBuffer,OMX_BUFFERHEADERTYPE* pOutputBuffer);
		virtual void FlushAudioDecoder();
		virtual void DeinitAudioDecoder();

		//for update Raw Pcm paramters
		virtual OMX_ERRORTYPE GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
												OMX_IN OMX_INDEXTYPE nParamIndex,
												OMX_INOUT OMX_PTR ComponentParameterStructure);
		virtual OMX_ERRORTYPE SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
												OMX_IN OMX_INDEXTYPE nParamIndex,
												OMX_IN OMX_PTR ComponentParameterStructure);

	private:
		OMX_BOOL initRawDecoder(OMX_BUFFERHEADERTYPE* pInputBuffer);

		OMX_U32 BitWidthToPCM16(uint8_t* src, int length);
		OMX_U32 PCM8ToPCM16(uint8_t* src, int16_t* dst, int length);
		OMX_U32 PCM24ToPCM16(uint8_t* src, int length);
		OMX_U32 PCM32ToPCM16(uint8_t* src, int length);
		OMX_U32 PCM24ToPCM32(uint8_t* src, int length);
		void BigEndianToLittleEndian(uint8_t* src, int length);
		void UnsignedToSignedConverter(uint8_t* src, int length, int bitWidth);
		void ChannelAssignment(uint8_t* src, int length);
		OMX_BOOL getChannelAssignment();
		OMX_BOOL checkInputParameter(OMX_U32 channelCount, OMX_ENDIANTYPE eEndian, OMX_U32 bitWidth, OMX_AUDIO_PCMTYPE fileType, OMX_NUMERICALDATATYPE);
		void handleBuffers(OMX_BUFFERHEADERTYPE* pInputBuf,
								OMX_BUFFERHEADERTYPE* pOutputBuf,
								 OMX_BOOL emptyInput,
								  OMX_BOOL fillOutput);

#ifdef ENHANCE_CHANNEL_ASSIGNMENT
		void addChannelAssign(OMX_AUDIO_PCMTYPE rawType, OMX_U32 value, AUDIO_CHANNEL_ASSIGNMENT chnAssign);
		size_t findChnAssign(OMX_AUDIO_PCMTYPE rawType, OMX_U32 value);
		AUDIO_CHANNEL_ASSIGNMENT getChnAssign(size_t index);
		size_t countAssignNum();
		void initmChannelAssign();

		void registerChnAssignHandler();
		OMX_BOOL HandleChnAssignFunc1(uint8_t* src, int length);
		OMX_BOOL HandleChnAssignFunc2(uint8_t* src, int length);
		OMX_BOOL HandleChnAssignFunc3(uint8_t* src, int length);
		OMX_BOOL HandleChnAssignFunc4(uint8_t* src, int length);

#endif
		OMX_BOOL mInitFlag;
		OMX_BOOL mFirstFrameFlag;       // for computing time stamp

		OMX_U32 mLastSampleCount;
		OMX_TICKS lastTimeStamp;

		TempBuffer tempBuffer;
		TempBuffer tempBufferFor24Bit;

		AUDIO_CHANNEL_ASSIGNMENT mChannelAssign;
		int mValue;

#ifdef ENHANCE_CHANNEL_ASSIGNMENT
		typedef OMX_BOOL (MtkOmxRawDec::*HandleChnAssignFunc)(uint8_t* src, int length);
		void addChnAssignFunc(AUDIO_CHANNEL_ASSIGNMENT chnAssign, HandleChnAssignFunc func);

		typedef struct ChannelAssignFunc
		{
			AUDIO_CHANNEL_ASSIGNMENT chnAssign;
			HandleChnAssignFunc func;
		}ChannelAssignFunc;
		ChannelAssignFunc ChnAssignFunc[10];

		Vector<ChannelAssign> mChnAssignment;
		KeyedVector<AUDIO_CHANNEL_ASSIGNMENT, HandleChnAssignFunc> mHandleChnAssign;
#endif

		OMX_BOOL isNewOutputBuffer;
		OMX_U16 maxNumFramesInOutputBuf;
		OMX_U16 numFramesInOutputBuf;

		OMX_U32 totalConsumedBytes;  //just for one input buffer
		OMX_BOOL b24OptionFlag;
                int mlog_enable; // for log reduce
};
#endif



