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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkCLITest.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCLITest.cpp
//! \brief

#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
//
#include <errno.h>
#include <fcntl.h>

#include <stdlib.h>

#include <mtkcam/main/acdk/AcdkCommon.h>

#include <mtkcam/drv/IHalSensor.h>

#define MY_LOGV(fmt, arg...)    printf("TS' [%s] " fmt "\n", __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    printf("TS' [%s] " fmt "\n", __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("TS' [%s] " fmt "\n", __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("TS' [%s] " fmt "\n", __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("TS' [%s] " fmt "\n", __FUNCTION__, ##arg)

#ifdef USE_DYN_ENTRY
#include <AcdkBase.h>
#include <mtkcam/main/acdk/AcdkIF.h>
#define _mdk_open()         MDK_Open()
#define _mdk_openbypass(a)  MDK_OpenBypass(a)
#define _mdk_close()        MDK_Close()
#define _mdk_init()         MDK_Init()
#define _mdk_deinit()       MDK_DeInit()
#define _mdk_iocontrol(a,b) MDK_IOControl(a,b)

extern MBOOL MDK_OpenBypass(NSACDK::ACDK_BYPASS_FLAG bypassFlag);
#else
#include "MdkIF.h"

#define _mdk_open()         Mdk_Open()
#define _mdk_openbypass(a)  Mdk_Open()
#define _mdk_close()        Mdk_Close()
#define _mdk_init()         Mdk_Init()
#define _mdk_deinit()       Mdk_DeInit()
#define _mdk_iocontrol(a,b) Mdk_IOControl(a,b)
#endif
#define VERIFY_COUNT		(3)

#define OP_AAA_ON           (0x01)
#define OP_SNR_TP_ON        (0x02)
#define OPERATE_MASK        (OP_AAA_ON | OP_SNR_TP_ON)

using namespace NSCam;


MBOOL       gCapCBDone = MFALSE;
MUINT32     gOperation = OP_SNR_TP_ON; //[0]: 3a on, [1]: test pattern on
MUINT32		gCrcResult;

void vAutoCapCb_patg(void *a_pParam);

/*******************************************************************************
*  Main Function
********************************************************************************/
int main_testMdk(int argc, char** argv, MUINT32 *crc_result)
{
    MBOOL bRet = MTRUE, bypassSurface = MTRUE;
    MUINT32 srcDev = 0, u4RetLen = 0, u32checksum = 0, previewTimeMs = 0;
    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;
    ACDK_PREVIEW_STRUCT rACDKPrvConfig;
    ACDK_CAPTURE_STRUCT rACDKCapConfig;

    /*
        ACDK_CMD_SET_SRC_DEV
        ACDK_CMD_PREVIEW_START
        ACDK_CMD_GET_CHECKSUM
        ACDK_CMD_CAPTURE
        ACDK_CMD_PREVIEW_STOP
        ACDK_CMD_RESET_LAYER_BUFFER
    */

    MY_LOGD("argc : %d", argc);
    if (argc >= 2) {
        switch (atoi(argv[1])) {
        case 1:
            srcDev = SENSOR_DEV_SUB;
            break;
        case 2:
            srcDev = SENSOR_DEV_MAIN_2;
            break;
        case 0:
        default:
            srcDev = SENSOR_DEV_MAIN;
            break;
        }
    }
    else {
        srcDev = SENSOR_DEV_MAIN;
    }
    if (argc >= 3) {
        if (atoi(argv[2]) == 1) {
            bypassSurface = MFALSE;
        }
    }
    if (argc >= 4) {
        gOperation = (atoi(argv[3]) & OPERATE_MASK);
    }

    gCrcResult = 0;

    if (bypassSurface) {
        bRet = _mdk_openbypass(NSACDK::ACDK_BYPASS_SURFACEVIEW);
    }
    else {
        bRet = _mdk_open();
    }

    if (!bRet)
    {
        MY_LOGE("open fail");
        goto EXIT_MAIN;
    }

    rAcdkFeatureInfo.puParaIn = (MUINT8 *)&srcDev;
    rAcdkFeatureInfo.u4ParaInLen = sizeof(srcDev);
    rAcdkFeatureInfo.puParaOut = NULL;
    rAcdkFeatureInfo.u4ParaOutLen = 0;
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;
    bRet = _mdk_iocontrol(ACDK_CMD_SET_SRC_DEV, &rAcdkFeatureInfo);
    if (!bRet)
    {
        MY_LOGD("ACDK_FEATURE_SET_SRC_DEV Fail: %d\n", srcDev);
        goto EXIT_MAIN;
    }


    bRet = _mdk_init();
    if (!bRet)
    {
        MY_LOGE("init fail");
        goto EXIT_MAIN;
    }


    rACDKPrvConfig.fpPrvCB = NULL;
    rACDKPrvConfig.u4PrvW = 320;
    rACDKPrvConfig.u4PrvH = 240;
    rACDKPrvConfig.u16HDRModeEn = 0;
    rACDKPrvConfig.u16PreviewTestPatEn = !!(gOperation&OP_SNR_TP_ON);
    rACDKPrvConfig.u4OperaType = !!(gOperation&OP_AAA_ON);
    rACDKPrvConfig.eOperaMode = ACDK_OPT_FACTORY_MODE;

    rAcdkFeatureInfo.puParaIn     = (MUINT8*)&rACDKPrvConfig;
    rAcdkFeatureInfo.u4ParaInLen  = sizeof(rACDKPrvConfig);
    rAcdkFeatureInfo.puParaOut    = NULL;
    rAcdkFeatureInfo.u4ParaOutLen = 0;
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;
    bRet = _mdk_iocontrol(ACDK_CMD_PREVIEW_START, &rAcdkFeatureInfo);
    if (!bRet)
    {
        MY_LOGE("ioctl preview start fail");
        goto EXIT_MAIN;
    }

    previewTimeMs = 8000;
    for (; previewTimeMs; previewTimeMs--)
        usleep(1000);

    rAcdkFeatureInfo.puParaIn     = NULL;
    rAcdkFeatureInfo.u4ParaInLen  = 0;
    rAcdkFeatureInfo.puParaOut    = (MUINT8 *)&u32checksum;
    rAcdkFeatureInfo.u4ParaOutLen = sizeof(u32checksum);
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;
    bRet = _mdk_iocontrol(ACDK_CMD_GET_CHECKSUM, &rAcdkFeatureInfo);
    if (!bRet)
    {
        MY_LOGE("ioctl get checksum fail");
        goto EXIT_MAIN;
    }

    if (gOperation&OP_AAA_ON) {
        rACDKCapConfig.eOutputFormat = JPEG_TYPE;
        rACDKCapConfig.eCameraMode   = CAPTURE_MODE;
        rACDKCapConfig.eOperaMode    = ACDK_OPT_FACTORY_MODE;

        rACDKCapConfig.u2JPEGEncWidth =  0;
        rACDKCapConfig.u2JPEGEncHeight =  0;
        rACDKCapConfig.fpCapCB = vAutoCapCb_patg;//vCapCb;
        rACDKCapConfig.i4IsSave = 1;    // 0-no save, 1-save
    }
    else {
        rACDKCapConfig.eOutputFormat = PURE_RAW10_TYPE;
        rACDKCapConfig.eCameraMode   = CAPTURE_MODE;
        rACDKCapConfig.eOperaMode    = ACDK_OPT_FACTORY_MODE;

        rACDKCapConfig.u2JPEGEncWidth  = 0;
        rACDKCapConfig.u2JPEGEncHeight = 0;
        rACDKCapConfig.fpCapCB = vAutoCapCb_patg;
        rACDKCapConfig.i4IsSave = 0;    // 0-no save, 1-save

        rACDKCapConfig.bUnPack = MFALSE;
        rACDKCapConfig.MFLL_En = MFALSE;
        rACDKCapConfig.HDRModeEn = 0;
        rACDKCapConfig.eMultiNR = EMultiNR_Off;
        rACDKCapConfig.u4OperaType = 0;
    }

    rAcdkFeatureInfo.puParaIn     = (MUINT8*)&rACDKCapConfig;
    rAcdkFeatureInfo.u4ParaInLen  = sizeof(rACDKCapConfig);
    rAcdkFeatureInfo.puParaOut    = NULL;
    rAcdkFeatureInfo.u4ParaOutLen = 0;
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;

    gCapCBDone = MFALSE;
    bRet = _mdk_iocontrol(ACDK_CMD_CAPTURE, &rAcdkFeatureInfo);
    if (!bRet)
    {
        MY_LOGE("ioctl capture fail");
        goto EXIT_MAIN;
    }

    MY_LOGD("wait capture done");
    while(!gCapCBDone)
    {
        usleep(1000);
    }

    MY_LOGD("stop preview");
    rAcdkFeatureInfo.puParaIn     = NULL;
    rAcdkFeatureInfo.u4ParaInLen  = 0;
    rAcdkFeatureInfo.puParaOut    = NULL;
    rAcdkFeatureInfo.u4ParaOutLen = 0;
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;
    bRet = _mdk_iocontrol(ACDK_CMD_PREVIEW_STOP, &rAcdkFeatureInfo);
    if (!bRet)
    {
        MY_LOGE("ioctl preview stop fail");
        goto EXIT_MAIN;
    }


    rAcdkFeatureInfo.puParaIn = NULL;
    rAcdkFeatureInfo.u4ParaInLen = 0;
    rAcdkFeatureInfo.puParaOut = NULL;
    rAcdkFeatureInfo.u4ParaOutLen = 0;
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;
    bRet = _mdk_iocontrol(ACDK_CMD_RESET_LAYER_BUFFER, &rAcdkFeatureInfo);
    if (!bRet)
    {
        MY_LOGE("ioctl reset layer fail");
        goto EXIT_MAIN;
    }


    _mdk_deinit();
    _mdk_close();

    *crc_result = gCrcResult;

EXIT_MAIN:
    MY_LOGD("closed...");

    return bRet;
}


/*******************************************************************************
*  Calculates the CRC-8 of the first len bits in data
********************************************************************************/
static const MUINT32 ACDK_CRC_Table[256]=
{
    0x0,        0x4C11DB7,  0x9823B6E,  0xD4326D9,  0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039, 0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x18AEB13,  0x54BF6A4,  0x808D07D,  0xCC9CDCA,
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
    0x315D626,  0x7D4CB91,  0xA97ED48,  0xE56F0FF,  0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30, 0x29F3D35,  0x65E2082,  0xB1D065B,  0xFDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};  // Table of 8-bit remainders


void vAutoCapCb_patg(void *a_pParam)
{
    ImageBufInfo*   pImgBufInfo = (ImageBufInfo *)a_pParam;
    MUINT8*         out_buffer;
    MUINT32         size;
    MUINT32 crc_accum = 0;

    MY_LOGD("Capture CB %p !", a_pParam);

    if (!(gOperation&OP_AAA_ON) && (gOperation&OP_SNR_TP_ON)) {
        out_buffer = (MUINT8*)pImgBufInfo->RAWImgBufInfo.bufAddr;
        size = pImgBufInfo->RAWImgBufInfo.imgSize;

        MY_LOGD("Buf size:%d, VA:%p !", size, out_buffer);

        while (size-- > 0) {
            crc_accum = (crc_accum << 8) ^ ACDK_CRC_Table[(MUINT8)(crc_accum >> 24) ^ (*out_buffer++)];
        }

        MY_LOGD("CRC: x%08x", (~crc_accum));
    }

    gCapCBDone = MTRUE;
    gCrcResult = (~crc_accum);
}

