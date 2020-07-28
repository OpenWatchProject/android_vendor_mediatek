/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _AF_PARAM_H
#define _AF_PARAM_H

#define AF_WIN_NUM_SPOT  1
#define MAX_AF_HW_WIN   37

#define FD_WIN_NUM     15

#define PATH_LENGTH    100

#define AFEXTENDCOEF

typedef struct
{
    MINT32 i4X;
    MINT32 i4Y;
    MINT32 i4W;
    MINT32 i4H;
    MINT32 i4Info;

} AREA_T;

typedef struct
{
    MINT32  i4Count;
    MINT32  i4Score;
    AREA_T  sRect[AF_WIN_NUM_SPOT];
} AF_AREA_T;

typedef struct
{
    MINT32 i4SGG_GAIN;
    MINT32 i4SGG_GMR1;
    MINT32 i4SGG_GMR2;
    MINT32 i4SGG_GMR3;
    MINT32 i4SGG_GMR4;
    MINT32 i4SGG_GMR5;
    MINT32 i4SGG_GMR6;
    MINT32 i4SGG_GMR7;
    MINT32 AF_DECI_1;
    MINT32 AF_ZIGZAG;
    MINT32 AF_ODD;
    MINT32 AF_FILT1[12];
    MINT32 AF_FILT2[4];
    MINT32 AF_TH[2];
    MINT32 AF_THEX;

} AF_CONFIG_T;


#define ISO_MAX_NUM     (8)
typedef struct
{
    MINT32 i4ISONum;
    MINT32 i4ISO[ISO_MAX_NUM];
    MINT32 i4GMR[7][ISO_MAX_NUM];
    MINT32 i4FV_DC[ISO_MAX_NUM];
    MINT32 i4MIN_TH[ISO_MAX_NUM];
    MINT32 i4HW_TH[ISO_MAX_NUM];
    MINT32 i4FV_DC2[ISO_MAX_NUM];
    MINT32 i4MIN_TH2[ISO_MAX_NUM];
    MINT32 i4HW_TH2[ISO_MAX_NUM];
} CustAF_THRES_T;

typedef struct
{
    MINT32 i4ImageWidth;
    MINT32 i4ImageHeight;
    MINT32 i4SensorID;
    CustAF_THRES_T Coef;
}AF_Extend_Coef_T;

typedef struct
{
    MINT32 AF_WINX[6];
    MINT32 AF_WINY[6];
    MINT32 AF_XSIZE;
    MINT32 AF_YSIZE;
    MINT32 AF_WINXE;
    MINT32 AF_WINYE;
    MINT32 AF_SIZE_XE;
    MINT32 AF_SIZE_YE;

} AF_WIN_CONFIG_T;


typedef struct
{
    MUINT32 u4Stat24;
    MUINT32 u4StatV;

} AF_HW_STAT_SINGLE_T;

typedef struct
{
    AF_HW_STAT_SINGLE_T sStat[MAX_AF_HW_WIN];

} AF_HW_STAT_T;

typedef struct
{
    MINT64 i8Stat24;
    MINT64 i8StatFL;
    MINT64 i8StatV;

} AF_STAT_T;

typedef struct
{
    MINT64 i8StatH[MAX_AF_HW_WIN-1];
    MINT64 i8StatV[MAX_AF_HW_WIN-1];
    MBOOL  bValid;
} AF_FULL_STAT_T;

typedef struct
{
    MINT32 i4CurrentPos;        //current position
    MINT32 i4MacroPos;          //macro position
    MINT32 i4InfPos;            //Infiniti position
    MINT32 bIsMotorMoving;      //Motor Status
    MINT32 bIsMotorOpen;        //Motor Open?
    MINT32 bIsSupportSR;

} LENS_INFO_T;

// AF v2.0
typedef struct
{
    MINT32      i4IsZSD;
    MINT32      i4IsVDO;
    MINT32      i4IsIHDR;
    MINT32      i4IsRevMode1;
    MINT32      i4IsRevMode2;    
    MINT32      i4IsRevMode3;
    MINT32      i4IsRevMode4;    
    MINT32      i4IsRevMode5;  
    MINT32      i4IsVDO1;
    MINT32      i4IsVDO2;
    MINT32      i4IsAEStable;
    MINT64      i8GSum;
    MINT32      i4ISO;
    MINT32      i4SceneLV;
    MINT32      i4ShutterValue;
    MINT32      i4FullScanStep;
    AF_AREA_T   sAFArea;
    AF_STAT_T   sAFStat;
    MINT32      i4IsFlashFrm;           // PL detect
    MINT32      i4AEBlockAreaYCnt;      // PL detect
    MUINT8      *pAEBlockAreaYvalue;    // PL detect
    LENS_INFO_T sLensInfo;
    AREA_T      sEZoom;
    // AF v2.1 HybridAF    
    MUINT32     i4HybridAFMode;
    MUINT32     i4CurrP1FrmNum;
    MUINT16     i4DafDacIndex;
    MUINT16     i4DafConfidence;
    MINT32      i4DafConverge;
    MUINT16     i4PDafDacIndex;
    MUINT16     i4PDafConfidence;
    MINT32      i4PDafConverge;
    MINT32      i4PDPureRawfrm;  
} AF_INPUT_T;

typedef struct
{
    MINT32      i4IsAFDone;
    MINT32      i4IsFocused;
    MINT32      i4IsMonitorFV;
    MINT32      i4AFBestPos;
    MINT32      i4AFPos;
    MINT32      i4FDDetect;
    MINT64      i8AFValue;
    AF_CONFIG_T sAFStatConfig;
    AF_AREA_T   sAFArea;
    // AF v2.1 HybridAF    
    MUINT8      i4IsLearning;      
    MUINT8      i4IsQuerying;
    MUINT8      i4AfValid;
    MUINT16     i4AfDacIndex;
    MUINT16     i4AfConfidence;    
    MUINT32     i4QueryFrmNum;

} AF_OUTPUT_T;
#define DistTBLLength 15
typedef struct
{
    MINT32  i4AFS_MODE;    //0 : singleAF, 1:smoothAF
    MINT32  i4AFC_MODE;    //0 : singleAF, 1:smoothAF
    MINT32  i4VAFC_MODE;   //0 : singleAF, 1:smoothAF

    MINT32  i4ReadOTP;     //0 : disable, 1:enable

    MINT32  i4FD_DETECT_CNT;
    MINT32  i4FD_NONE_CNT;

    MINT32  i4FV_SHOCK_THRES;    MINT32  i4FV_SHOCK_OFFSET;
    MINT32  i4FV_VALID_CNT;
    MINT32  i4FV_SHOCK_FRM_CNT;
    MINT32  i4FV_SHOCK_CNT;

// AF v2.0 : param move to NVRAM
//    MINT32  i4AFS_STEP_MIN_ENABLE;
//    MINT32  i4AFS_STEP_MIN_NORMAL;
//    MINT32  i4AFS_STEP_MIN_MACRO;
//    MINT32  i4ReadOTP;     //0 : disable, 1:enable     
//    MINT32  i4FD_DETECT_CNT;    
//    MINT32  i4FD_NONE_CNT;
//    MINT32  i4FV_SHOCK_THRES;
//    MINT32  i4FV_SHOCK_OFFSET;
//    MINT32  i4FV_VALID_CNT;
//    MINT32  i4FV_SHOCK_FRM_CNT;
//    MINT32  i4FV_SHOCK_CNT;

    // PL detect    
   /* MINT32  isPLenable;
    MINT32  plBoundL;
    MINT32  plBoundM;
    MINT32  plBoundH;    
    MINT32  plThresNsLv;	// night scene
    MINT32  plThresIdLv;	// indoor
    MINT32  plRatioBase;	
    MINT32  plRatioLoT;     // low/tot
    MINT32  plRatioMoT;     // mid/tot
    MINT32  plRatioHoT;     // high/tot
    MINT32  plRatioEoT;     // ext/tot
    MINT32  plRatioLEoT;	// (low+ext)/tot
    MINT32  plRatioExtInc;  // extMax/extMin    */
    MINT32 i4TBLL;
    MINT32 i4Dist[DistTBLLength];
    MINT32 i4Dacv[DistTBLLength];
    MINT32 i4FocusRange[DistTBLLength];
} AF_PARAM_T;

typedef struct
{
    MINT32  i4Num;
    MINT32  i4Pos[PATH_LENGTH];

} AF_STEP_T;

typedef enum
{
    AF_MARK_NORMAL = 0,
    AF_MARK_OK,
    AF_MARK_FAIL,
    AF_MARK_NONE

} AF_MARK_T;

// AF info for ISP tuning
typedef struct
{
    MINT32 i4AFPos; // AF position

} AF_INFO_T;

#endif

