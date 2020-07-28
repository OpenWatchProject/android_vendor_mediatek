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
#define LOG_TAG "test_aaoneon"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <vector>
#include <iostream>
#include <met_tag.h>
#include <mtkcam/aaa/aaa_hal_common.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
//aao info header
#include<AAOSeparation.h>

#define fgOpt (0x7)
#define AAWidth (2816)
#define AAHight (2112)

using namespace std;
using namespace NS3Av3;

class aaoSeparationBase
{
    typedef struct
    {
        MUINT32                 m_u4AAOSepAWBsize;
        MUINT32                 m_u4AAOSepAEsize;
        MUINT32                 m_u4AAOSepHistsize;
        MUINT32                 m_u4AAOSepOverCntsize;
        MUINT32                 m_u4AAOSepLSCsize;
        MUINT32                 m_u4AAOSepsize;
        MVOID*                  m_pAAOSepBuf;
        AAO_BUF_T               m_rBufs;
    } AAO_STT_BUF_INFO_T;

public:
    //Ctor/Dtor
                            aaoSeparationBase();
    virtual                 ~aaoSeparationBase(){};
    MVOID                   run(const MVOID* AAOSrc, const EBitMode_T& i4BitMode);
    virtual void            aaoSeparation(const MVOID* AAOSrc, AAO_BUF_T& rAAOBuf, const MINT32 AAOSepType) = 0;
    MVOID                   config(const EBitMode_T& i4BitMode);
    MBOOL                   allocateBuf(AAO_STT_BUF_INFO_T& m_rBufInfo);
    MVOID                   freeBuf(AAO_STT_BUF_INFO_T& m_rBufInfo);
    MVOID                   dump(AAO_STT_BUF_INFO_T& m_rBufInfo, const EBitMode_T& i4BitMode, const MINT32& AAOSepType);
    MVOID                   bitCompare();

public:

    AAO_BUF_CONFIG_T        m_rCfg;
    AAO_STT_BUF_INFO_T      m_rBufInfoC;
    AAO_STT_BUF_INFO_T      m_rBufInfoNeon;
    MUINT8                  m_ufgOpt;
    MUINT32                 m_u4AAWidth;
    MUINT32                 m_u4AAHight;
    MUINT32                 m_u4BlocksizeX;
    MUINT32                 m_u4BlocksizeY;
    MBOOL                   m_bEnablePixelBaseHist;
};

class aao14 : public aaoSeparationBase
{
public:
    //Ctor/Dtor
                            aao14();
    virtual                 ~aao14(){};
    virtual MVOID           aaoSeparation(const MVOID* AAOSrc, AAO_BUF_T& rAAOBuf, const MINT32 AAOSepType);
};

class aao12 : public aaoSeparationBase
{
public:
    //Ctor/Dtor
                            aao12();
    virtual                 ~aao12(){};
    virtual MVOID           aaoSeparation(const MVOID* AAOSrc, AAO_BUF_T& rAAOBuf, const MINT32 AAOSepType);
};


aaoSeparationBase::
aaoSeparationBase()
    : m_bEnablePixelBaseHist(MTRUE)
    , m_ufgOpt(fgOpt)
    , m_u4AAWidth(AAWidth)
    , m_u4AAHight(AAHight)
{
    CAM_LOGD("[%s] + aaoSeparationBase construction", __FUNCTION__);
    memset(&m_rCfg, 0, sizeof(AAO_BUF_CONFIG_T));
    memset(&m_rBufInfoC, 0, sizeof(AAO_STT_BUF_INFO_T));
    memset(&m_rBufInfoNeon, 0, sizeof(AAO_STT_BUF_INFO_T));
    CAM_LOGD("[%s] -", __FUNCTION__);
}

MVOID
aaoSeparationBase::
config(const EBitMode_T& i4BitMode)
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    CAM_LOGD("[%s] i4BitMode = %d", __FUNCTION__, i4BitMode);

    m_rCfg.m_i4BlkNumX = AAO_STT_BLOCK_NUM;//m_rTsfEnvInfo.ImgWidth;
    m_rCfg.m_i4BlkNumY = AAO_STT_PLINE_NUM;//m_rTsfEnvInfo.ImgHeight;
    m_rCfg.m_bEnableHDRYConfig = (m_ufgOpt & 0x4); //1
    m_rCfg.m_bEnableOverCntConfig = (m_ufgOpt & 0x2); //1
    m_rCfg.m_bEnableTSFConfig = (m_ufgOpt & 0x1); //1
    m_rCfg.m_bDisablePixelHistConfig = ((m_bEnablePixelBaseHist==1)?MFALSE:MTRUE); //0
    m_u4BlocksizeX = ((m_u4AAWidth / m_rCfg.m_i4BlkNumX)/2)*2;
    m_u4BlocksizeY = ((m_u4AAHight / m_rCfg.m_i4BlkNumY)/2)*2;
    m_rCfg.m_u4AEOverExpCntShift = (((m_u4BlocksizeX/4)*(m_u4BlocksizeY/2)>255)?MTRUE:MFALSE);

    m_rCfg.m_i4AwbLineSize = m_rCfg.m_i4BlkNumX * 4;
    m_rCfg.m_i4AeSize      = ((m_rCfg.m_i4BlkNumX + 3)/4) * 4;                           // in byte

    if(i4BitMode == EBitMode_12Bit)
        m_rCfg.m_i4HdrSize     = (m_ufgOpt & 0x4) ? ((m_rCfg.m_i4BlkNumX*4 + 31)/32)*4 : 0;  // in byte
    else if(i4BitMode == EBitMode_14Bit)
        m_rCfg.m_i4HdrSize     = (m_ufgOpt & 0x4) ? ((m_rCfg.m_i4BlkNumX*8 + 31)/32)*4 : 0;  // in byte

    m_rCfg.m_i4AeOverSize  = (m_ufgOpt & 0x2) ? ((m_rCfg.m_i4BlkNumX*8 + 31)/32)*4 : 0;  // in byte
    m_rCfg.m_i4TsfSize     = (m_ufgOpt & 0x1) ? ((m_rCfg.m_i4BlkNumX*16 + 31)/32)*4 : 0; // in byte
    m_rCfg.m_i4LineSize    = m_rCfg.m_i4AwbLineSize + m_rCfg.m_i4AeSize + m_rCfg.m_i4HdrSize + m_rCfg.m_i4AeOverSize + m_rCfg.m_i4TsfSize;//lineByte
    m_rCfg.m_i4SkipSize    = m_rCfg.m_i4AwbLineSize + m_rCfg.m_i4AeSize + m_rCfg.m_i4HdrSize + m_rCfg.m_i4AeOverSize;

    CAM_LOGD("[%s] m_i4BlkNumX(%d) m_i4BlkNumY(%d) m_bEnableHDRYConfig(%d) m_bEnableOverCntConfig(%d) m_bEnableTSFConfig(%d) m_bDisablePixelHistConfig(%d) m_u4BlocksizeX(%d) m_u4BlocksizeY(%d) m_u4AEOverExpCntShift(%d)"
        , __FUNCTION__, m_rCfg.m_i4BlkNumX, m_rCfg.m_i4BlkNumY, m_rCfg.m_bEnableHDRYConfig, m_rCfg.m_bEnableOverCntConfig, m_rCfg.m_bEnableTSFConfig, m_rCfg.m_bDisablePixelHistConfig, m_u4BlocksizeX, m_u4BlocksizeY, m_rCfg.m_u4AEOverExpCntShift);


    CAM_LOGD("[%s] m_i4AwbLineSize(%d) m_i4AeSize(%d) m_i4HdrSize(%d) m_i4AeOverSize(%d) m_i4TsfSize(%d) m_i4LineSize(%d) m_i4SkipSize(%d)"
            , __FUNCTION__, m_rCfg.m_i4AwbLineSize, m_rCfg.m_i4AeSize, m_rCfg.m_i4HdrSize, m_rCfg.m_i4AeOverSize, m_rCfg.m_i4TsfSize, m_rCfg.m_i4LineSize, m_rCfg.m_i4SkipSize);

    CAM_LOGD("[%s] -", __FUNCTION__);
}


MBOOL
aaoSeparationBase::
allocateBuf(AAO_STT_BUF_INFO_T& m_rBufInfo)
{
    CAM_LOGD("[%s] +", __FUNCTION__);

    m_rBufInfo.m_u4AAOSepAWBsize = 4 * m_rCfg.m_i4BlkNumX * m_rCfg.m_i4BlkNumY;
    m_rBufInfo.m_u4AAOSepAEsize = 2 * m_rCfg.m_i4BlkNumX * m_rCfg.m_i4BlkNumY;
    m_rBufInfo.m_u4AAOSepHistsize = 4 * 4*AAO_STT_HIST_BIN_NUM;
    m_rBufInfo.m_u4AAOSepOverCntsize = 2 * m_rCfg.m_i4BlkNumX * m_rCfg.m_i4BlkNumY;
    m_rBufInfo.m_u4AAOSepLSCsize = 4 * 2 * m_rCfg.m_i4BlkNumX * m_rCfg.m_i4BlkNumY;
    m_rBufInfo.m_u4AAOSepsize = m_rBufInfo.m_u4AAOSepAWBsize+m_rBufInfo.m_u4AAOSepAEsize+m_rBufInfo.m_u4AAOSepHistsize
                                +m_rBufInfo.m_u4AAOSepOverCntsize+m_rBufInfo.m_u4AAOSepLSCsize;

    m_rBufInfo.m_pAAOSepBuf = (MVOID*)malloc(m_rBufInfo.m_u4AAOSepsize);
    m_rBufInfo.m_rBufs.m_pAAOSepAWBBuf = m_rBufInfo.m_pAAOSepBuf;
    m_rBufInfo.m_rBufs.m_pAAOSepAEBuf = (MUINT8 *)m_rBufInfo.m_pAAOSepBuf + m_rBufInfo.m_u4AAOSepAWBsize;
    m_rBufInfo.m_rBufs.m_pAAOSepHistBuf = (MUINT8 *)m_rBufInfo.m_pAAOSepBuf + m_rBufInfo.m_u4AAOSepAWBsize + m_rBufInfo.m_u4AAOSepAEsize;
    m_rBufInfo.m_rBufs.m_pAAOSepOverCntBuf = (MUINT8 *)m_rBufInfo.m_pAAOSepBuf + m_rBufInfo.m_u4AAOSepAWBsize+ m_rBufInfo.m_u4AAOSepAEsize+ m_rBufInfo.m_u4AAOSepHistsize;
    m_rBufInfo.m_rBufs.m_pAAOSepLSCBuf = (MUINT8 *)m_rBufInfo.m_pAAOSepBuf + m_rBufInfo.m_u4AAOSepAWBsize+ m_rBufInfo.m_u4AAOSepAEsize+ m_rBufInfo.m_u4AAOSepHistsize+ m_rBufInfo.m_u4AAOSepOverCntsize;

    if(m_rBufInfo.m_pAAOSepBuf != NULL && m_rBufInfo.m_u4AAOSepsize != 0){
        CAM_LOGD("[%s] m_rBufInfo->m_pAAOSepBuf = 0x%x, \n", __FUNCTION__, m_rBufInfo.m_pAAOSepBuf);
        CAM_LOGD("[%s] m_rBufInfo->m_u4AAOSepsize = 0x%x, \n", __FUNCTION__, m_rBufInfo.m_u4AAOSepsize);
    }

    CAM_LOGD("[%s] m_u4AAOSepAWBsize(%d) m_u4AAOSepAEsize(%d) m_u4AAOSepHistsize(%d) m_u4AAOSepOverCntsize(%d) m_u4AAOSepLSCsize(%d) m_u4AAOSepsize(%d)"
            , __FUNCTION__, m_rBufInfo.m_u4AAOSepAWBsize, m_rBufInfo.m_u4AAOSepAEsize, m_rBufInfo.m_u4AAOSepHistsize, m_rBufInfo.m_u4AAOSepOverCntsize, m_rBufInfo.m_u4AAOSepLSCsize, m_rBufInfo.m_u4AAOSepsize);

    if(m_rBufInfo.m_pAAOSepBuf == NULL)
    {
        CAM_LOGE("m_rBufInfo.m_pAAOSepBuf == NULL");
        return MFALSE;
    }
    CAM_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MVOID
aaoSeparationBase::
freeBuf(AAO_STT_BUF_INFO_T& m_rBufInfo)
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    if(m_rBufInfo.m_pAAOSepBuf != NULL && m_rBufInfo.m_u4AAOSepsize != 0){
        CAM_LOGD("[%s] m_rBufInfo->m_pAAOSepBuf = 0x%x, \n", __FUNCTION__, m_rBufInfo.m_pAAOSepBuf);
        free(m_rBufInfo.m_pAAOSepBuf);
    }

    memset(&m_rCfg, 0, sizeof(AAO_BUF_CONFIG_T));

    memset(&m_rBufInfo.m_rBufs, 0, sizeof(AAO_BUF_T));

    m_rBufInfo.m_pAAOSepBuf = NULL;
    m_rBufInfo.m_u4AAOSepAEsize = 0;
    m_rBufInfo.m_u4AAOSepAWBsize = 0;
    m_rBufInfo.m_u4AAOSepHistsize = 0;
    m_rBufInfo.m_u4AAOSepLSCsize = 0;
    m_rBufInfo.m_u4AAOSepOverCntsize = 0;
    m_rBufInfo.m_u4AAOSepsize = 0;
    CAM_LOGD("[%s] -", __FUNCTION__);
}
MVOID
aaoSeparationBase::
dump(AAO_STT_BUF_INFO_T& m_rBufInfo, const EBitMode_T& i4BitMode, const MINT32& AAOSepType)
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    CAM_LOGD("[%s] i4BitMode(%d) AAOSepType(%d)", __FUNCTION__, i4BitMode, AAOSepType);

    char* StrBitMode;
    char* StrType;
    char AWBStr[100] = {'\0'};
    char AEStr[100] = {'\0'};
    char HistStr[100] = {'\0'};
    char OverStr[100] = {'\0'};
    char LscStr[100] = {'\0'};
    char SepBufStr[100] = {'\0'};

    switch (i4BitMode)
    {
        case EBitMode_12Bit:
            StrBitMode = "12Bit";
            break;
        case EBitMode_14Bit:
            StrBitMode = "14Bit";
            break;
    }

    switch (AAOSepType)
    {
        case AAOCType:
            StrType = "C";
            break;
        case AAONeonType:
            StrType = "Neon";
            break;
    }

    sprintf(AWBStr, "/sdcard/aaoSep/aaoAWB_%s_%s.bin", StrBitMode, StrType);
    sprintf(AEStr, "/sdcard/aaoSep/aaoAE_%s_%s.bin", StrBitMode, StrType);
    sprintf(HistStr, "/sdcard/aaoSep/aaoHsit_%s_%s.bin", StrBitMode, StrType);
    sprintf(OverStr, "/sdcard/aaoSep/aaoOver_%s_%s.bin", StrBitMode, StrType);
    sprintf(LscStr, "/sdcard/aaoSep/aaoLsc_%s_%s.bin", StrBitMode, StrType);
    sprintf(SepBufStr, "/sdcard/aaoSep/aaoSepBuf_%s_%s.bin", StrBitMode, StrType);

    FILE* pFileAWB = fopen(AWBStr,"wb");
    FILE* pFileAE = fopen(AEStr,"wb");
    FILE* pFileHist = fopen(HistStr,"wb");
    FILE* pFileOver = fopen(OverStr,"wb");
    FILE* pFileLsc = fopen(LscStr,"wb");
    FILE* pFileSepBuf = fopen(SepBufStr,"wb");

    if (NULL == pFileAWB)
    {
        char folderName[64];
        strcpy(folderName, "/sdcard/aaoSep");

        CAM_LOGE("fail to open file to save bin: %s", AWBStr);
        MINT32 err = mkdir(folderName, S_IRWXU | S_IRWXG | S_IRWXO);
        CAM_LOGD("err = %d", err);
    }

    CAM_LOGD("[%s] m_u4AAOSepAWBsize(%d) m_u4AAOSepAEsize(%d) m_u4AAOSepHistsize(%d) m_u4AAOSepOverCntsize(%d) m_u4AAOSepLSCsize(%d) m_u4AAOSepsize(%d)"
            , __FUNCTION__, m_rBufInfo.m_u4AAOSepAWBsize, m_rBufInfo.m_u4AAOSepAEsize, m_rBufInfo.m_u4AAOSepHistsize, m_rBufInfo.m_u4AAOSepOverCntsize, m_rBufInfo.m_u4AAOSepLSCsize, m_rBufInfo.m_u4AAOSepsize);

    fwrite (m_rBufInfo.m_rBufs.m_pAAOSepAWBBuf ,1 , m_rBufInfo.m_u4AAOSepAWBsize, pFileAWB);//sizeof(MUINT8)
    fwrite (m_rBufInfo.m_rBufs.m_pAAOSepAEBuf ,sizeof(MUINT16) , m_rBufInfo.m_u4AAOSepAEsize, pFileAE);
    fwrite (m_rBufInfo.m_rBufs.m_pAAOSepHistBuf ,1 , m_rBufInfo.m_u4AAOSepHistsize, pFileHist);
    fwrite (m_rBufInfo.m_rBufs.m_pAAOSepOverCntBuf ,1 , m_rBufInfo.m_u4AAOSepOverCntsize, pFileOver);
    fwrite (m_rBufInfo.m_rBufs.m_pAAOSepLSCBuf ,1 , m_rBufInfo.m_u4AAOSepLSCsize, pFileLsc);
    fwrite (m_rBufInfo.m_pAAOSepBuf ,1 , m_rBufInfo.m_u4AAOSepsize, pFileSepBuf);

    fclose(pFileAWB);
    fclose(pFileAE);
    fclose(pFileHist);
    fclose(pFileOver);
    fclose(pFileLsc);
    fclose(pFileSepBuf);

    CAM_LOGD("[%s] -", __FUNCTION__);
}

MVOID
aaoSeparationBase::
run(const MVOID* AAOSrc, const EBitMode_T& i4BitMode)
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    CAM_LOGD("[%s] AAOSrc = 0x%02x, i4BitMode = %d", __FUNCTION__, AAOSrc, i4BitMode);
    MINT32 AAOSepType = 0;
    config(i4BitMode); //once

    CAM_LOGD("[%s] C-version", __FUNCTION__);
    // C-version
    AAOSepType = AAOCType;
    allocateBuf(m_rBufInfoC);
    aaoSeparation(AAOSrc, m_rBufInfoC.m_rBufs, AAOSepType);
    dump(m_rBufInfoC, i4BitMode, AAOSepType);


    CAM_LOGD("[%s] Neon-version", __FUNCTION__);
    // Neon-version
    AAOSepType = AAONeonType;
    allocateBuf(m_rBufInfoNeon);
    aaoSeparation(AAOSrc, m_rBufInfoNeon.m_rBufs, AAOSepType);
    dump(m_rBufInfoNeon, i4BitMode, AAOSepType);

    CAM_LOGD("[%s] Compare", __FUNCTION__);
    // compare
    bitCompare();

    // free
    freeBuf(m_rBufInfoC);
    freeBuf(m_rBufInfoNeon);

    CAM_LOGD("[%s] -", __FUNCTION__);
}

MVOID
aaoSeparationBase::
bitCompare()
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    MINT32 i4BitTrueCount = 0;
    // awb
    CAM_LOGD("[%s] awb bit true", __FUNCTION__);
    if( (m_rBufInfoC.m_u4AAOSepAWBsize != 0) && (m_rBufInfoC.m_u4AAOSepAWBsize == m_rBufInfoNeon.m_u4AAOSepAWBsize) )
    {
        MUINT8* pawbC         = reinterpret_cast<MUINT8*>(m_rBufInfoC.m_rBufs.m_pAAOSepAWBBuf);
        MUINT8* pawbNeon      = reinterpret_cast<MUINT8*>(m_rBufInfoNeon.m_rBufs.m_pAAOSepAWBBuf);

        for(int i = 0; i < m_rBufInfoC.m_u4AAOSepAWBsize; ++i)
        {
            if(pawbC[i] != pawbNeon[i])
                i4BitTrueCount++;
        }

        if(i4BitTrueCount != 0)
        {
            printf("awb bit true fail \n");
            CAM_LOGD("[%s] awb bit true fail", __FUNCTION__);
        }
        else
        {
            printf("awb bit true success \n");
            CAM_LOGD("[%s] awb bit true success", __FUNCTION__);
        }
    }
    // ae
    CAM_LOGD("[%s] ae bit true", __FUNCTION__);
    i4BitTrueCount = 0;
    if( (m_rBufInfoC.m_u4AAOSepAEsize != 0) && (m_rBufInfoC.m_u4AAOSepAEsize == m_rBufInfoNeon.m_u4AAOSepAEsize) )
    {
        MUINT8* paeC         = reinterpret_cast<MUINT8*>(m_rBufInfoC.m_rBufs.m_pAAOSepAEBuf);
        MUINT8* paeNeon      = reinterpret_cast<MUINT8*>(m_rBufInfoNeon.m_rBufs.m_pAAOSepAEBuf);

        for(int i = 0; i < m_rBufInfoC.m_u4AAOSepAEsize; ++i)
        {
            if(paeC[i] != paeNeon[i])
                i4BitTrueCount++;
        }

        if(i4BitTrueCount != 0)
        {
            printf("ae bit true fail \n");
            CAM_LOGD("[%s] ae bit true fail", __FUNCTION__);
        }
        else
        {
            printf("ae bit true success \n");
            CAM_LOGD("[%s] ae bit true success", __FUNCTION__);
        }
    }
    // hist
    CAM_LOGD("[%s] hist bit true", __FUNCTION__);
    i4BitTrueCount = 0;
    if( (m_rBufInfoC.m_u4AAOSepHistsize != 0) && (m_rBufInfoC.m_u4AAOSepHistsize == m_rBufInfoNeon.m_u4AAOSepHistsize) )
    {
        MINT8* pHistC        = reinterpret_cast<MINT8*>(m_rBufInfoC.m_rBufs.m_pAAOSepHistBuf);
        MINT8* pHistNeon     = reinterpret_cast<MINT8*>(m_rBufInfoNeon.m_rBufs.m_pAAOSepHistBuf);

        for(int i = 0; i < m_rBufInfoC.m_u4AAOSepHistsize; ++i)
        {
            if(pHistC[i] != pHistNeon[i])
                i4BitTrueCount++;
        }

        if(i4BitTrueCount != 0)
        {
            printf("hist bit true fail \n");
            CAM_LOGD("[%s] hist bit true fail", __FUNCTION__);
        }
        else
        {
            printf("hist bit true success \n");
            CAM_LOGD("[%s] hist bit true success", __FUNCTION__);
        }
    }
    // OverCnt
    CAM_LOGD("[%s] overCnt bit true", __FUNCTION__);
    i4BitTrueCount = 0;
    if( (m_rBufInfoC.m_u4AAOSepOverCntsize != 0) && (m_rBufInfoC.m_u4AAOSepOverCntsize == m_rBufInfoNeon.m_u4AAOSepOverCntsize) )
    {
        MUINT8* pOexpcntC    = reinterpret_cast<MUINT8*>(m_rBufInfoC.m_rBufs.m_pAAOSepOverCntBuf);
        MUINT8* pOxpcntNeon  = reinterpret_cast<MUINT8*>(m_rBufInfoNeon.m_rBufs.m_pAAOSepOverCntBuf);

        for(int i = 0; i < m_rBufInfoC.m_u4AAOSepOverCntsize; ++i)
        {
            if(pOexpcntC[i] != pOxpcntNeon[i])
                i4BitTrueCount++;
        }

        if(i4BitTrueCount != 0)
        {
            printf("overCnt bit true fail \n");
            CAM_LOGD("[%s] overCnt bit true fail", __FUNCTION__);
        }
        else
        {
            printf("overCnt bit true success \n");
            CAM_LOGD("[%s] overCnt bit true success", __FUNCTION__);
        }
    }
    // lsc
    CAM_LOGD("[%s] lsc bit true", __FUNCTION__);
    i4BitTrueCount = 0;
    if( (m_rBufInfoC.m_u4AAOSepLSCsize != 0) && (m_rBufInfoC.m_u4AAOSepLSCsize == m_rBufInfoNeon.m_u4AAOSepLSCsize) )
    {
        MUINT8* plscC        = reinterpret_cast<MUINT8*>(m_rBufInfoC.m_rBufs.m_pAAOSepLSCBuf);
        MUINT8* plscNeon     = reinterpret_cast<MUINT8*>(m_rBufInfoNeon.m_rBufs.m_pAAOSepLSCBuf);

        for(int i = 0; i < m_rBufInfoC.m_u4AAOSepLSCsize; ++i)
        {
            if(plscC[i] != plscNeon[i])
                i4BitTrueCount++;
        }

        if(i4BitTrueCount != 0)
        {
            printf("lsc bit true fail \n");
            CAM_LOGD("[%s] lsc bit true fail", __FUNCTION__);
        }
        else
        {
            printf("lsc bit true success \n");
            CAM_LOGD("[%s] lsc bit true success", __FUNCTION__);
        }
    }
    // AAOSep
    CAM_LOGD("[%s] AAOSep bit true", __FUNCTION__);
    i4BitTrueCount = 0;
    if( (m_rBufInfoC.m_u4AAOSepsize != 0) && (m_rBufInfoC.m_u4AAOSepsize == m_rBufInfoNeon.m_u4AAOSepsize) )
    {
        MUINT8* pSepBufC      = reinterpret_cast<MUINT8*>(m_rBufInfoC.m_pAAOSepBuf);
        MUINT8* pSepBufNeon   = reinterpret_cast<MUINT8*>(m_rBufInfoNeon.m_pAAOSepBuf);

        for(int i = 0; i < m_rBufInfoC.m_u4AAOSepsize; ++i)
        {
            if(pSepBufC[i] != pSepBufNeon[i])
                i4BitTrueCount++;
        }

        if(i4BitTrueCount != 0)
        {
            printf("AAOSep bit true fail \n");
            CAM_LOGD("[%s] AAOSep bit true fail", __FUNCTION__);
        }
        else
        {
            printf("AAOSep bit true success \n");
            CAM_LOGD("[%s] AAOSep bit true success", __FUNCTION__);
        }
    }
    CAM_LOGD("[%s] -", __FUNCTION__);
}

MVOID
aao14::
aaoSeparation(const MVOID* AAOSrc, AAO_BUF_T& rAAOBuf, const MINT32 AAOSepType)
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    CAM_LOGD("[%s] AAOSrc = 0x%02x, AAOSepType = %d", __FUNCTION__, AAOSrc, AAOSepType);

    CAM_LOGD("[%s] m_i4BlkNumX(%d) m_i4BlkNumY(%d) m_bEnableHDRYConfig(%d) m_bEnableOverCntConfig(%d) m_bEnableTSFConfig(%d) m_bDisablePixelHistConfig(%d) m_u4BlocksizeX(%d) m_u4BlocksizeY(%d) m_u4AEOverExpCntShift(%d)"
            , __FUNCTION__, m_rCfg.m_i4BlkNumX, m_rCfg.m_i4BlkNumY, m_rCfg.m_bEnableHDRYConfig, m_rCfg.m_bEnableOverCntConfig, m_rCfg.m_bEnableTSFConfig, m_rCfg.m_bDisablePixelHistConfig, m_u4BlocksizeX, m_u4BlocksizeY, m_rCfg.m_u4AEOverExpCntShift);

    CAM_LOGD("[%s] m_i4AwbLineSize(%d) m_i4AeSize(%d) m_i4HdrSize(%d) m_i4AeOverSize(%d) m_i4TsfSize(%d) m_i4LineSize(%d) m_i4SkipSize(%d)"
            , __FUNCTION__, m_rCfg.m_i4AwbLineSize, m_rCfg.m_i4AeSize, m_rCfg.m_i4HdrSize, m_rCfg.m_i4AeOverSize, m_rCfg.m_i4TsfSize, m_rCfg.m_i4LineSize, m_rCfg.m_i4SkipSize);

    CAM_LOGD("[%s] Call aaoSeparation14 function", __FUNCTION__);
    aaoSeparation14(AAOSrc ,m_rCfg, rAAOBuf, AAOSepType);

    CAM_LOGD("[%s] -", __FUNCTION__);
}

aao14::
aao14()
{
    CAM_LOGD("[%s] + aao14 Construction", __FUNCTION__);
    CAM_LOGD("[%s] -", __FUNCTION__);
}

MVOID
aao12::
aaoSeparation(const MVOID* AAOSrc, AAO_BUF_T& rAAOBuf, const MINT32 AAOSepType)
{
    CAM_LOGD("[%s] +", __FUNCTION__);

    CAM_LOGD("[%s] AAOSrc = 0x%02x, AAOSepType = %d", __FUNCTION__, AAOSrc, AAOSepType);

    CAM_LOGD("[%s] m_i4BlkNumX(%d) m_i4BlkNumY(%d) m_bEnableHDRYConfig(%d) m_bEnableOverCntConfig(%d) m_bEnableTSFConfig(%d) m_bDisablePixelHistConfig(%d) m_u4BlocksizeX(%d) m_u4BlocksizeY(%d) m_u4AEOverExpCntShift(%d)"
            , __FUNCTION__, m_rCfg.m_i4BlkNumX, m_rCfg.m_i4BlkNumY, m_rCfg.m_bEnableHDRYConfig, m_rCfg.m_bEnableOverCntConfig, m_rCfg.m_bEnableTSFConfig, m_rCfg.m_bDisablePixelHistConfig, m_u4BlocksizeX, m_u4BlocksizeY, m_rCfg.m_u4AEOverExpCntShift);

    CAM_LOGD("[%s] m_i4AwbLineSize(%d) m_i4AeSize(%d) m_i4HdrSize(%d) m_i4AeOverSize(%d) m_i4TsfSize(%d) m_i4LineSize(%d) m_i4SkipSize(%d)"
            , __FUNCTION__, m_rCfg.m_i4AwbLineSize, m_rCfg.m_i4AeSize, m_rCfg.m_i4HdrSize, m_rCfg.m_i4AeOverSize, m_rCfg.m_i4TsfSize, m_rCfg.m_i4LineSize, m_rCfg.m_i4SkipSize);

    CAM_LOGD("[%s] Call aaoSeparation12 function", __FUNCTION__);
    aaoSeparation12(AAOSrc , m_rCfg, rAAOBuf, AAOSepType);
    CAM_LOGD("[%s] -", __FUNCTION__);
}

aao12::
aao12()
{
    CAM_LOGD("[%s] + aao12 Construction", __FUNCTION__);
    CAM_LOGD("[%s] -", __FUNCTION__);
}
// Cmd : test_aaoneon aao_16.raw 12
int main(int argc, char** argv)
{
    CAM_LOGD("[%s] +", __FUNCTION__);

    char* inFileName = argv[1];

    MUINT32 bitmode = atoi(argv[2]);

    FILE* pInput = fopen(inFileName, "rb");

    EBitMode_T i4BitMode;

    //Get input data size
    fseek(pInput, 0, SEEK_END);
    MUINT32 size = ftell(pInput);
    fseek(pInput, 0, SEEK_SET);

    cout << "the file length = " << size;cout << " bytes" << endl;
    MUINT32 *inputarray = (MUINT32*)malloc(size);
    fread( inputarray, 1, size, pInput );

    printf("inputarray = 0x%02x\n", inputarray);
    CAM_LOGD("[%s] inputarray = 0x%02x", __FUNCTION__,inputarray);

    if(bitmode == 14)
    {
        i4BitMode = EBitMode_14Bit;
        aaoSeparationBase* pAAO14 = new aao14;
        pAAO14->run( inputarray, i4BitMode );
        delete pAAO14;
    }
    else
    {
        i4BitMode = EBitMode_12Bit;
        aaoSeparationBase* pAAO12 = new aao12;
        pAAO12->run( inputarray, i4BitMode );
        delete pAAO12;
    }

    free(inputarray);
    fclose(pInput);

    printf("done \n", inputarray);

    CAM_LOGD("[%s] -", __FUNCTION__);
}
