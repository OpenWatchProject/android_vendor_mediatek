/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*******************************************************************************
 *
 * Filename:
 * ---------
 *   heaacdec_exp.h
 *
 * Project:
 * --------
 *   
 *
 * Description:
 * ------------
 *   The HEAAC Decoder Interface
 *
 * Author:
 * -------
 *   Dior Lin
 *
 *==============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *==============================================================================
 *******************************************************************************/
#ifndef _HEAACDEC_EXP_H
#define _HEAACDEC_EXP_H


/*   ASSERT Definition */
#if defined(__MTK_TARGET__)
#include "kal_release.h"
#else

#ifndef ASSERT
#include <assert.h>
#define ASSERT(x)    assert(x)
#endif

#ifndef NULL
#define NULL    0
#endif
#endif


//======================================================================//
//  ERROR MESSAGE OF HEAAC Decoder
//======================================================================//
#define HEAAC_ERR_INFO_SAMPLERATE_NOTSYNC   0x00010001
#define HEAAC_ERR_INFO_CHANNEL_NOTSYNC      0x00010002


//======================================================================//
//  SBR operating option of HEAAC decoder
//  SBR_DISABLED:  Only plain AAC is decoded and no up-sampling is performed.
//  SBR_ENABLED :  SBR decoding enabled, up-sampling is always performed.
//  SBR_AUTO    :  SBR decoding enabled, up-sampling is only done
//                 when SBR is actually present(suggested).
//======================================================================//
typedef enum {
  SBR_DISABLED,
  SBR_ENABLED,
  SBR_AUTO
} SBR_SIGNALING;


//======================================================================//
//  SBR modes of HEAAC decoder
//  SBR_MODE_LP  : low power mode, real value transforms, fast.
//  SBR_MODE_HQ  : high quality mode, complex value  transforms, slow, good quality.
//  SBR_MODE_AUTO: automatic mode, high quality for mono and parametric stereo,
//                 low power mode otherwise (suggested).
//======================================================================//
typedef enum {
  SBR_MODE_LP,
  SBR_MODE_HQ,
  SBR_MODE_AUTO
} SBR_MODE;

//======================================================================//
// STRUCTURE
//  HEAACDEC_HANDLE
//
// DESCRIPTION
//  HEAAC decoder handler
//
// MEMBERS
//  SampleRate   : Sample rate of outpcm data.
//  ChannelNum   : Cannel number of output pcm data.
//  FrameSize    : PCM frame size per channel.
//  sbrFlag      : sbr bitstream detected or not (0: no sbr  , 1: with sbr).
//======================================================================//
typedef struct {
    int SampleRate;
    int ChannelNum;
    int FrameSize;
    int sbrFlag;
    int psFlag;
}HEAACDEC_HANDLE;


//======================================================================//
// FUNCTION
//  HEAACDec_GetMemSize
//
// DESCRIPTION
//  This function gets the buffers size for HEAAC decoder.
//
// PARAMETERS
//  InterBufSize    [o] required size of internal buffer.
//  TmpBufSize      [o] required size of temporary buffer.
//  PcmBufSize      [o] required size of PCM buffer.
//  BsBufSize       [o] minimum required size of bitstream buffer.
//
// RETURNS
//  None
//
//======================================================================//

void HEAACDec_GetMemSize( int *InterBufSize,
                          int *TmpBufSize ,
                          int *PcmBufSize,
                          int *BsBufSize);

//======================================================================//
// FUNCTION
//  HEAACDec_Init
//
// DESCRIPTION
//  This function initialize the buffer and decoding option for HEAAC decoder.
//
// PARAMETERS
//  pInterBuf       [i] pointer to internal buffer.
//  sbrSignaling    [i] SBR operating option of HEAAC decoder.
//  sbrMode         [i] SBR modes of HEAAC decoder.
//  fForce2Stereo   [i] For the output PCM data as stereo channels. 0 : dependent AAC
//                                                           non-zero : stereo
//
// RETURNS
//  the pointer of HEAAC decoder handler
//
//======================================================================//
HEAACDEC_HANDLE* HEAACDec_Init( void *pInterBuf,
                                SBR_SIGNALING sbrSignaling,
                                SBR_MODE sbrMode,
                                int fForce2Stereo);

//======================================================================//
// FUNCTION
//  HEAACDec_DecodeFrame
//
// DESCRIPTION
//  This function decodes HEAAC bitstreams and output
//  1024/ch pcm data for pure AAC ,and 2048/ch pcm data for HEAAC.
//
// PARAMETERS
//  pAACPlusDecHandle       [i] pointer to HEAAC decoder handler.
//  pPcmBuf                 [i] pointer to PCM buffer.
//  pTmpBuf                 [i] pointer to Temporary buffer.
//  pBsBuf                  [i] pointer to bitstream buffer.
//  BsBufLen                [i] bitstream length.
//  pBsRead                 [i] bitstream buffer read pointer.
//  consumBsLen             [o] used length (bytes) of HEAAC decoder.
//                              in this decoding frame.
// RETURNS
//  0: decoding successful  , no-zero : decoding unsuccessful.
//
//======================================================================//
int HEAACDec_DecodeFrame(HEAACDEC_HANDLE *pHEAACDecHandle,
                           void *pPcmBuf,
                           void *pTmpBuf,
                           const void *pBsBuf,
                           int BsBufLen,
                           const void *pBsRead,
                           int *consumBsLen);

//======================================================================//
// FUNCTION
//  HEAACDec_GetCurrentVersion
//
// DESCRIPTION
//  This function was used to get current version of library
//
// PARAMETERS
//  datalength              [o] show array data length of the character version
//  dataptr                 [o] pointer to pointer to array of character version
//
// RETURNS
//  None
//
//======================================================================//
void HEAACDec_GetCurrentVersion(int  *datalength, char **dataptr);


// DualSCE add 2011.01.18

int HEAACDec_Set_DSCE_Config (HEAACDEC_HANDLE *pHEAACDecHandle, int dsce_config);
int HEAACDec_Bypass_CRC_Check(HEAACDEC_HANDLE *pHEAACDecHandle, int bypass);
//


//======================================================================//
// FUNCTION
//  HEAACDec_IsSupportPS
//
// DESCRIPTION
//  This function was used to check if the library supports HEAACv2 (PS)
//
// PARAMETERS
//  None
//
// RETURNS
//  0: not support, 1: support
//
//======================================================================//
int HEAACDec_IsSupportPS(void);

//======================================================================//
// FUNCTION
//  HEAACDEC_ExtInterFace
//
// DESCRIPTION
//  This function extends HEAAC interface
//
// PARAMETERS
//  pAACPlusDecHandle       [i] pointer to HEAAC decoder handler.
//  interface_index         [i] interface_index
//  IO                      [i] input(0) or output(1).
//  datalength              [i/o] data length
//  dataptr                 [i/o] data pointer.
//
// RETURNS
//  0: successful  , no-zero : command not supported.
//
//======================================================================//
int HEAACDEC_ExtInterFace(HEAACDEC_HANDLE *pHEAACDecHandle,
                          int interface_index,
                          int *IO,
                          int *datalength,
                          char **dataptr);

//======================================================================//
// FUNCTION
//  HEAAC_SBRDec_GetMemSize
//
// DESCRIPTION
//  This function gets the buffers size for HEAAC-SBR decoder.
//
// PARAMETERS
//  InterBufSize    [o] required size of internal buffer.
//  TmpBufSize      [o] required size of temporary buffer.
//  PcmBufSize      [o] required size of PCM buffer.
//  SbrBSBufSize    [o] minimum required size of SBR bitstream buffer.
//
// RETURNS
//  None
//
//======================================================================//

void HEAAC_SBRDec_GetMemSize( int *InterBufSize,
                          int *TmpBufSize ,
                          int *PcmBufSize,
                          int *SbrBSBufSize);


//======================================================================//
// FUNCTION
//  HEAAC_SBRDec_Init
//
// DESCRIPTION
//  This function initialize the buffer and decoding option for HEAAC decoder.
//
// PARAMETERS
//  pInterBuf       [i] pointer to internal buffer.
//  pSBR_BS_buffer  [i] pointer to SBR Bit-stream buffer.
//  sbrSignaling    [i] SBR operating option of HEAAC decoder.
//  sbrMode         [i] SBR modes of HEAAC decoder.
//  fForce2Stereo   [i] For the output PCM data as stereo channels. 0 : dependent AAC
//                                                           non-zero : stereo
//
// RETURNS
//  the pointer of HEAAC decoder handler
//
//======================================================================//
HEAACDEC_HANDLE* HEAAC_SBRDec_Init( void *pInterBuf,
                                    void *pSBR_BS_buffer,
                                    SBR_SIGNALING sbrSignaling,
                                    SBR_MODE sbrMode,
                                    int fForce2Stereo);

//======================================================================//
// FUNCTION
//  HEAAC_SBRDec_DecodeFrame
//
// DESCRIPTION
//  This function decodes HEAAC bitstreams and output
//  1024/ch pcm data for pure AAC ,and 2048/ch pcm data for HEAAC.
//
// PARAMETERS
//  pAACPlusDecHandle       [i] pointer to HEAAC decoder handler.
//  pPcmBuf                 [i] pointer to PCM buffer.
//  pTmpBuf                 [i] pointer to Temporary buffer.
//  pSbrBSBuf               [i] pointer to SBR bitstream buffer.
//  AAC_sampleRate          [i] AAC sample rate of this decoded frame
//  AAC_channelNum          [i] AAC channel Number of this decoded frame
// RETURNS
//  0: decoding successful  , no-zero : decoding unsuccessful.
//
//======================================================================//
int HEAAC_SBRDec_DecodeFrame(HEAACDEC_HANDLE *pHEAACDecHandle,
                           void *pPcmBuf,
                           void *pTmpBuf,
                           const void *pSbrBSBuf,
                           int AAC_sampleRate,
                           int AAC_channelNum);


#endif //_HEAACDEC_EXP_H
