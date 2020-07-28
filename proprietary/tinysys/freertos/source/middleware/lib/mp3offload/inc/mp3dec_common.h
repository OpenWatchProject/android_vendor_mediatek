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
 *   MP3_common.h
 *
 * Project:
 * --------
 *
 *
 * Description:
 * ------------
 *   The constant definition, structure definition, and subfunciton declaration.
 *
 * Author:
 * -------
 *   Sammy Shih
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


#ifndef MP3_MAIN_H
#define MP3_MAIN_H

//#ifdef ANDROID
//#include <cutils/log.h>
//#endif

#include "mp3dec_exp.h"

/*
BY ISO/IEC 13818-3 section 2.4.3.2mp3dec_armdsp
If the intensity stereo is selected, the maximum value for intensity position will indicate an illegal intensity position
=> save illegal intensity position in IIPBuf
*/
#define LSF_STEREO_PROC
#define MP2_DECODE_SUPPORT

/*   Type Definition */
typedef signed char Word8;
typedef unsigned char UWord8;
typedef short Word16;
typedef unsigned short UWord16;
typedef int Word32;
#if __CC_ARM
typedef unsigned int UWord32;
#else
typedef unsigned long int UWord32;
#endif

#if __CC_ARM
typedef long long Word64;
#elif defined(__ADSPBLACKFIN__)
typedef long long Word64;
#else
typedef long long Word64;
#endif


#ifndef FALSE
#define FALSE           0
#endif
#ifndef TRUE
#define TRUE            1
#endif
#define UNDERFLOW       -1

#define MPEG_LAYER_2          2
#define MPEG_LAYER_3          3

#define PRE_MAIN_DATA         512
#define BUF_SIZE_MP3          1536
#define BUF_SIZE_MP2       3072
#define MAX_MP3_FrameSize     1444
#define MAX_MP2_FrameSize     2884
#define MIN_BUF_SIZE       4*1024
#define PCM_BUF_SIZE       4608   /* in bytes */
#define MAX_NUM_CHANNELS         2
#define SBLIMIT                 32
#define SSLIMIT                 18

#define MPG_MD_JOINT_STEREO     1
#define MPG_MD_MONO             3

#define SCALEFACS_SIZE_32      2*39
#define SCALE                  11
#define ROUNING                0x400


#define FRAC4_Q13   2
#define SW_MIN  (Word32)0x80000000
#define SW_MIN_2 (Word32)0x40000000
#define ZERO_BAND_SCALE  -63

#define SCALE_BLOCK             12
#define CLZ_1                   1
#define MUL                     1
#define C_DIV_2_SCALE           1
#define S_DVI_4_SCALE           1+MUL
#define DEWINDOW_CALSE          1+MUL

#if defined(__ADSPBLACKFIN__)
#define MP3_FCORE_OPT

#ifdef MP3_FCORE_OPT
#define MP3_FCORE_OPT_BASICOP
//#define MP3_FCORE_OPT_DEQ
//#define MP3_FCORE_OPT_DEQ_READBS
//#define MP3_FCORE_OPT_SBS_DCT1
#define MP3_FCORE_OPT_SBS_DCT2
#define MP3_FCORE_OPT_SBS_WIN1
#define MP3_FCORE_OPT_SBS_WIN2
#define MP3_FCORE_OPT_IMDCT
#define MP3_FCORE_OPT_IMDCT_PRESHIFT
#define MP3_FCORE_OPT_INTEGRATE
#define MP3_FCPRE_OPT_AALIAS
#endif

#define MP3_FCORE_DEBUG

#ifdef MP3_FCORE_DEBUG
//#define MP3_FCORE_DEBUG_PROFILE
//#define MP3_FCORE_DEBUG_ONEFRAME 23
//#define MP3_FCORE_DEBUG_PROCESS_FRAME 5
//#define MP3_FCORE_DEBUG_PRINT_DEQ1
//#define MP3_FCORE_DEBUG_PRINT_DEQ2
//#define MP3_FCORE_DEBUG_PRINT_SBS_DCT1
//#define MP3_FCORE_DEBUG_PRINT_SBS_DCT2
#endif


#if defined(MP3_FCORE_DEBUG_PRINT_DEQ1) && defined(MP3_FCORE_DEBUG_ONEFRAME)
#define DEQ1_PRINT printf
#else
#define DEQ1_PRINT
#endif

#if defined(MP3_FCORE_DEBUG_PRINT_DEQ2) && defined(MP3_FCORE_DEBUG_ONEFRAME)
#define DEQ2_PRINT printf
#else
#define DEQ2_PRINT
#endif

#if defined(MP3_FCORE_DEBUG_PRINT_SBS_DCT1) && defined(MP3_FCORE_DEBUG_ONEFRAME)
#define DCT1_PRINT printf
#else
#define DCT1_PRINT
#endif

#if defined(MP3_FCORE_DEBUG_PRINT_SBS_DCT2) && defined(MP3_FCORE_DEBUG_ONEFRAME)
#define DCT2_PRINT printf
#else
#define DCT2_PRINT
#endif

#endif //__ADSPBLACKFIN__
/* COMMON STRUCTURE */

typedef struct {
    UWord8  *pMP3_BS_BUF;
    UWord8  *pMP3_BS_end;
    Word32  MP3_BS_BUF_size;
    UWord8  *pBS_read;
    Word32  bitIndex;
    UWord32 outputTemp;
} Mp3_in_out_BUF;

typedef struct {
    Word32 stereo;
    Word32 lsf;
    Word32 mpeg25;
    Word32 lay;
    Word32 errorProtection;
    Word32 bitrateIndex;
    Word32 samplingFreq;
    Word32 padding;
    Word32 extension;
    Word32 mode;
    Word32 modeExt;
    Word32 copyright;
    Word32 original;
    Word32 emphasis;

    UWord32 oldHeader;
    UWord32 firstHeader;

    Word32 errorFlag;
    Word32 FirstFrameValid;
    Word32 frameSize;
    Word32 frameSizeOld;
    Word32 frameCount;
    Word32 granules;


    /* MP3 */
    Word32 mainDataBeginBefore;
    Word32 mainDataBeginOld;
    Word32 MS_stereo;
    Word32 I_stereo;

    /* MP2 */
    Word32 JS_bound;
    Word32 II_sblimit;
    const Word32 *pNbalTable;
    const UWord8 *pAlloc;
} Frame;


/* MP3 STURCTURE */
typedef struct {
    Word32 scfsi;
    UWord32 part2_3_length;
    UWord32 scalefacScale;
    UWord32 bigValues;
    UWord32 region1Start;
    UWord32 region2Start;
    UWord32 globalGain;
    UWord32 blockType;
    UWord32 tableSelect[3];
    UWord32 preflag;
    UWord32 count1TableSelect;
    Word32 maxbands;
    Word32 maxbandl;
    Word32 maxb;
    Word32 mixedBlockFlag;
    Word32 subblockGain[3];
    Word32 maxband[3];
    UWord32 scalefacCompress;
} GrInfo;

typedef struct {
    Word32 mainDataBegin;
    UWord32 privateBits;
    struct {
        GrInfo gr[2];
    } ch[2];
} SideInfo;

typedef struct {
    Word16 longIdx[23];//Word32 longIdx[23];
    Word8  longDiff[22];//Word32 longDiff[22];
    Word16 shortIdx[14];//Word32 shortIdx[14];
    Word8  shortDiff[13];//Word32 shortDiff[13];
} BandInfo;

typedef struct {
    Word32 maxLength;
    const Word16 *pTable;
    UWord32 linbits;
    Word32 limit;
} HuffInfo;


typedef struct {
    Word32 maxLength;
    const Word32 *pTable;
} HuffInfo_2;


typedef struct {
    Word32 Shift;
    Word32 *buf_ptr_0;
    Word32 *buf_ptr_1;
    Word32 bo_channel;
    Word32 input_offset;
} SSynthesisInfo;


typedef struct {
    /***** Sync. with MP2_WorkingMem_Seg1 *****/
    Mp3dec_handle handle;
    UWord8 *pWorking_BUF1;
    UWord8 *pWorking_BUF2;
    Word32 BUF1_offset;
    Word32 BUF2_offset;
    Word32 layer;
#ifdef FAME_LENGTH_TABLE_USED
    Word32 FrameLenCheckEnable;
#endif
#ifdef ANDROID
    Word32 CheckHWResult;
#endif
    Mp3_in_out_BUF *pBUF; //Common

#ifdef FAME_LENGTH_TABLE_USED
    Word16 *frameLengthTable; //Common
#endif

    UWord8 *pPCM_BUF;
    UWord8 *pre_main_data;
    Frame *frameInfo;
    Word32 *buffs;
    UWord8 *BS_BUF;

    /************* Pointer ********************/

    Word32 *bo;
    SideInfo *sideinfo;
    Word32 *fix_hybridIn;
    Word32 *prevblck;
    Word32 *scalefacs;
    Word32 *long_scale;
    Word32 *short_scale;
    Word32 *preScale;
    Word32 *preMax;
#ifdef LSF_STEREO_PROC
    Word32 *IIPBufPtr;  //illegal intensity position buffer(see IEC 13818-3 2.4.3.2),the maximum value for intensity position will indicate an illegal intensity position
#endif
} MP3_WorkingMem_Seg1;

typedef struct {
    /***** Sync. with MP2_WorkingMem_Seg1 *****/
    Mp3dec_handle handle;
    UWord8 *pWorking_BUF1;
    UWord8 *pWorking_BUF2;
    Word32 BUF1_offset;
    Word32 BUF2_offset;
    Word32 layer;
#ifdef FAME_LENGTH_TABLE_USED
    Word32 FrameLenCheckEnable;
#endif
#ifdef ANDROID
    Word32 CheckHWResult;
#endif
    Mp3_in_out_BUF *pBUF; //Common

#ifdef FAME_LENGTH_TABLE_USED
    Word16 *frameLengthTable; //Common
#endif

    UWord8 *pPCM_BUF;
    UWord8 *pre_main_data;
    Frame *frameInfo;
    Word32 *buffs;
    UWord8 *BS_BUF;

    /************* Pointer ********************/

    Word32 *bitAlloc;
    Word32 *sample;
    Word32 *scfsi;
    Word32 *scaleIndex;
    Word32 bo;
} MP2_WorkingMem_Seg1;



/* FUNTION */


Word32 MP3_DecodeLayer3(Mp3dec_handle *handle, UWord8 *MP3_BS_BUF_R);

extern Word32 MP3_DecodeLayer2(Mp3dec_handle *handle, UWord8 *MP3_BS_BUF_R);

extern void   MP3_CleanBUF(void *, Word32);
extern Word32 MP3_CheckLayer(Mp3_in_out_BUF *, UWord8 *);
extern Word32 MP3_ReadFrame(void *,  UWord8 **, UWord8 *);
extern void   MP3_CopyBSData(UWord8 *, UWord8 **, Word32 , Mp3_in_out_BUF *);
extern Word32 MP3_GetSideInfo(MP3_WorkingMem_Seg1 *, UWord8 *);
extern Word32 MP3_GetScaleFactors_1(Word32 *, GrInfo *, Mp3_in_out_BUF *);
#ifdef LSF_STEREO_PROC
extern Word32 MP3_GetScaleFactors_2(Word32 *, GrInfo *, Word32 ,
                                    Mp3_in_out_BUF *, Word32 *);
#else
extern Word32 MP3_GetScaleFactors_2(Word32 *, GrInfo *, Word32 ,
                                    Mp3_in_out_BUF *);
#endif
extern Word32 MP3_ReadBits(Mp3_in_out_BUF *, Word32);
extern Word32 MP3_Dequantize(Word32 *, GrInfo *, Word32 *,
                             Word32 short_scale[13][3], Word32, Frame *, Mp3_in_out_BUF *, Word32 *);
extern void   MP3_MS_Stereo(GrInfo *, GrInfo *, Word32 *, Word32 , Word32 *,
                            Word32 *);
#ifdef LSF_STEREO_PROC
extern void   MP3_I_Stereo(Word32 *, Word32 *, GrInfo *, GrInfo *, Frame *,
                           Word32 *, Word32 short_scale[13][3], Word32 *);
#else
extern void   MP3_I_Stereo(Word32 *, Word32 *, GrInfo *, GrInfo *, Frame *,
                           Word32 *, Word32 short_scale[13][3]);
#endif
extern void   MP3_Integrate(Word32 *, Word32 *, Word32 short_scale[13][3],
                            Word32 *, GrInfo *, Word32 , Word32, Word32);
extern void   MP3_Antialias(Word32 *, GrInfo *);
extern void   MP3_InvMDCT(Word32 *, GrInfo *, MP3_WorkingMem_Seg1 *, Word32);
extern void   MP3_SubbandSynthesis(Word32 *, Word16 *, SSynthesisInfo *);


#endif
