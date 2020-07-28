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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _PD_BUF_COMMON_H_
#define _PD_BUF_COMMON_H_

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1
#include <log/log.h>
#include "MediaTypes.h"
#include <vector>

using namespace std;

#ifndef MUINTPTR
typedef uintptr_t MUINTPTR;
#endif

#ifndef MBOOL
typedef int MBOOL;
#endif


#define MTRUE  1
#define MFALSE 0

#define MAX_SIZE_OF_PD_SENSOR_LIST (32)
#define MAX_SIZE_OF_PD_ROI  (16) // this value should be set larger than AF_PSUBWIN_NUM which is defined in af_param.h

/**
    for PDO
    pdo data size is 2 byte per pix
*/
#define _PDO_STRIDE_ALIGN_(W) ({\
    MUINT32 _align = 0;\
    if(((W) % 16) != 0){\
        _align = ((W + 15)/16 * 16);\
    }\
    else{\
        _align = W;\
    }\
    _align;\
})

typedef struct SPDOBUFINFO_t
{
    MUINT8       *u1buf;
    MUINT32       u4sz;
    MUINT32       u4stride;

    SPDOBUFINFO_t()
    {
        u1buf    = NULL;
        u4sz     = 0;
        u4stride = 0;
    }
    SPDOBUFINFO_t operator =(const SPDOBUFINFO_t &in)
    {
        u1buf    = in.u1buf;
        u4sz     = in.u4sz;
        u4stride = in.u4stride;
        return (*this);
    }

} SPDOBUFINFO_T;



typedef struct SPDOHWINFO_t
{
    MUINT32       u4Bpci_xsz;
    MUINT32       u4Bpci_ysz;
    const MUINT8 *pu1Bpci_tbl;
    MUINTPTR      phyAddrBpci_tbl;
    MUINTPTR      virAddrBpci_tbl;
    MINT32        i4memID;
    MUINT32       u4Pdo_xsz;
    MUINT32       u4Pdo_ysz;
    MUINT32       i4TGSzW; //TODO : optimizate it
    MUINT32       u4BitDepth;

    MUINT8        u1IsDualPD;

    //for dualpd
    MUINT8        u1PBinType; //0:1x4, 1:4x4
    MUINT8        u1PBinStartLine; //0, 1, 2, 3
    MUINT8        u1PdSeparateMode;

    SPDOHWINFO_t()
    {
        u4Bpci_xsz      = 0;
        u4Bpci_ysz      = 0;
        pu1Bpci_tbl     = NULL;
        phyAddrBpci_tbl = 0;
        virAddrBpci_tbl = 0;
        i4memID         = 0;
        u4Pdo_xsz       = 0;
        u4Pdo_ysz       = 0;
        i4TGSzW         = 0;
        u1IsDualPD      = 0;
        u1PBinType      = 0;
        u1PBinStartLine = 0;
    }
    SPDOHWINFO_t operator =(const SPDOHWINFO_t &in)
    {
        u4Bpci_xsz      = in.u4Bpci_xsz;
        u4Bpci_ysz      = in.u4Bpci_ysz;
        pu1Bpci_tbl     = in.pu1Bpci_tbl;
        phyAddrBpci_tbl = in.phyAddrBpci_tbl;
        virAddrBpci_tbl = in.virAddrBpci_tbl;
        i4memID         = in.i4memID;
        u4Pdo_xsz       = in.u4Pdo_xsz;
        u4Pdo_ysz       = in.u4Pdo_ysz;
        i4TGSzW         = in.i4TGSzW;
        u1IsDualPD      = in.u1IsDualPD;
        u1PBinType      = in.u1PBinType;
        u1PBinStartLine = in.u1PBinStartLine;
        return (*this);
    }

} SPDOHWINFO_T;

typedef struct SDUALPDVCINFO_t
{
    MUINT32       u4VCBinningX;
    MUINT32       u4VCBinningY;
    MUINT32       u4VCBufFmt;

    SDUALPDVCINFO_t()
    {
        u4VCBinningX    = 0;
        u4VCBinningY    = 0;
        u4VCBufFmt      = 0;
    }
    SDUALPDVCINFO_t operator =(const SDUALPDVCINFO_t &in)
    {
        u4VCBinningX    = in.u4VCBinningX;
        u4VCBinningY    = in.u4VCBinningY;
        u4VCBufFmt      = in.u4VCBufFmt;
        return (*this);
    }
} SDUALPDVCINFO_T;



//bit 0:VC, 1:Raw, 2:PDO, 4: open, 5:dualPD
#define MSK_CATEGORY_OPEN    0x10
#define MSK_CATEGORY_DUALPD  0x20
#define MSK_CATEGORY         0xf0

#define MSK_PDBUF_TYPE_VC          0x01
#define MSK_PDBUF_TYPE_RAW_LEGACY  0x02
#define MSK_PDBUF_TYPE_PDO         0x04
#define MSK_PDBUF_TYPE             0x0f

typedef enum
{
    EPDBUF_NOTDEF           = 0x00,
    EPDBUF_VC               = 0x01,
    EPDBUF_VC_OPEN          = 0x11,
    EPDBUF_RAW_LEGACY       = 0x02,
    EPDBUF_RAW_LEGACY_OPEN  = 0x12,
    EPDBUF_PDO              = 0x04,
    EPDBUF_PDO_OPEN         = 0x14,
    EPDBUF_DUALPD_VC        = 0x21,
    EPDBUF_DUALPD_RAW       = 0x24

} EPDBUF_TYPE_t;

typedef struct
{
    EPDBUF_TYPE_t BufType;
    MINT32        i4CurrSensorId;
    MUINT32       u4IsZSD;
    MUINT32       uImgXsz;     /* current sensor output image size. */
    MUINT32       uImgYsz;
    MUINT32       uFullXsz;    /* Full size. */
    MUINT32       uFullYsz;
    MUINT32       u4PDAF_support_type;
    MUINT32       u4IsFrontalBinning;
    MBOOL         bEnablePBIN;
    MUINT8        uPdSeparateMode;
    MINT32        i4SensorMode;
    MINT32        AETargetMode;
    MUINT32       u4RawBit;
    MUINT32       u4PDPipeCtrl;
    MBOOL         bSensorModeSupportPD;
} SPDProfile_t;

typedef struct
{
    unsigned long MajorVersion;
    unsigned long MinorVersion;

} SPDLibVersion_t;

typedef struct
{
    MINT32 i4XStart;
    MINT32 i4YStart;
    MINT32 i4XEnd;
    MINT32 i4YEnd;
    MINT32 i4Info;
} SPDROI_T;

typedef struct
{
    MUINT32  curLensPos;
    UINT16   XSizeOfImage;
    UINT16   YSizeOfImage;
    SPDROI_T ROI;
    MINT32   afegain;
    MINT32   mode;

} SPDROIInput_T;


typedef struct
{
    signed long    Defocus;
    signed char    DefocusConfidence;
    unsigned long  DefocusConfidenceLevel;
    signed long    PhaseDifference;

} SPDROIResult_T;

typedef struct
{
    SPDROIResult_T ROIRes[MAX_SIZE_OF_PD_ROI];

} SPDResult_T;

typedef struct
{
    unsigned int u4frmNum;
    unsigned int u4cfgNum;
    vector<SPDROI_T> vecROIs;
} PDBUF_CFG_T;

// for dual PD
typedef enum
{
    DualPD_VCBuf_Normal = 0,
    DualPD_VCBuf_zHDR,
    DualPD_VCBuf_Type_Num
} DUALPD_VCBUF_TYPE_T;

typedef struct
{
    unsigned int i4BinningX;
    unsigned int i4BinningY;
    unsigned int i4BufFmt;
} DUALPD_VC_SETTING_T;


class SPDInputData_t
{
private :
    //default constructor is not allowed.
    SPDInputData_t() {}

public:
    SPDInputData_t( MUINT8 iNumROI, MUINT32 iBufSz, MUINT32 iBufStride, MUINT8 *iBufAddr)
    {
        numROI = iNumROI;
        ROI    = new SPDROI_T [numROI];

        databuf_size     = iBufSz;
        databuf_stride   = iBufStride;
        databuf_virtAddr = iBufAddr;
    }

    ~SPDInputData_t()
    {
        delete[] ROI;
        databuf_virtAddr = NULL;
    }

    MUINT32   magicNumber; //mMagicNumber from ptrPDdata for log recognition
    MUINT32   frmNum;
    MUINT32   curLensPos;
    //ROI coordinate
    UINT16    XSizeOfImage;
    UINT16    YSizeOfImage;
    //PD analyze ROI
    MUINT8    numROI;
    SPDROI_T *ROI;
    //PD data buffer
    MUINT32   databuf_size;
    MUINT32   databuf_stride;
    MUINT8   *databuf_virtAddr;
    MINT32    afeGain;
    MBOOL     hlrEn;

};


class SPDOutputData_t
{
private :
    //default constructor is not allowed.
    SPDOutputData_t() {}

public:
    SPDOutputData_t( MUINT8 iNumROI)
    {
        numRes = iNumROI;
        Res = new SPDROIResult_T [numRes];
    }

    ~SPDOutputData_t()
    {
        delete Res;
    }

    MUINT8          numRes;
    SPDROIResult_T *Res;

};

typedef struct
{
    UINT32 pdo_xsize;
    UINT32 pdo_ysize;
} PDBuf_Bpci_Info_t;


UINT32 GetPDBuf_Type( unsigned int a_u4CurrSensorDev, unsigned int a_u4CurrSensorId);


#endif // _PD_BUF_COMMON_H_
