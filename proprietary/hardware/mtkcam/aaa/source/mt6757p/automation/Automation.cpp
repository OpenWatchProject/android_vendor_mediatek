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

#define LOG_TAG "AutoTest"

#include "aaa_log.h"
#include "aaa_types.h"
//#include "cct_feature.h"
#include "IRawToJpeg.h"
#include <cutils/properties.h>
#include <isp_reg.h>
#include "IAutomation.h"


#define AUTOMATION_FOLDER   "/data/cct"
//#define RAW_IMG_FILENAME    "unpack5312x2976.raw"
//#define JPEG_IMG_FILENAME   "image5312x2976.jpg"
#define RAW_IMG_FILENAME    "unpack5344x4016.raw"
#define JPEG_IMG_FILENAME   "image5344x4016.jpg"


class AutomationMain :public IAutomationMain
{
public:
    //    Ctor/Dtor.
                            AutomationMain();
    virtual                 ~AutomationMain();

                            AutomationMain(const AutomationMain&);
                            AutomationMain& operator=(const AutomationMain&);
    virtual void            destroyInstance();
    virtual MINT32          sendcommand(
                                MUINT32 const a_u4Ioctl,
                                MUINT8 *puParaIn,
                                MUINT32 const u4ParaInLen,
                                MUINT8 *puParaOut,
                                MUINT32 const u4ParaOutLen,
                                MUINT32 *pu4RealParaOutLen
                            );

    MUINT32                 mRgbEn;
    MUINT32                 mYuvEn;
    MUINT32                 mYuv2En;
private:
    /************************
     * function
     ************************/
    virtual MBOOL updateInfo(AUTOMATION_IMAGE_INFO* pImageInfo);
//    virtual MBOOL applyISPParams(AUTOMATION_NVRAM_ISP_PARAM* pIspParams);
    virtual MBOOL applyISPParams(AUTOMATION_ISP_PARAM_SET* pIspParam);
    virtual MBOOL run();

private:
    /************************
     * member
     ************************/
#if (ISP_AUTOMATION_FUNC_EN)
    IRawToJpeg::InputInfo mImageInfo;
    IRawToJpeg* pRawToJpeg;
#endif
    char mpInputFile[128];
    char mpOutputFile[128];
    MUINT32 mu4PixelID;
};

AutomationMain::
AutomationMain()
    : mu4PixelID(0)
{
#if (ISP_AUTOMATION_FUNC_EN)
    pRawToJpeg = IRawToJpeg::createInstance();
#endif
}

AutomationMain::
~AutomationMain()
{
#if (ISP_AUTOMATION_FUNC_EN)
    pRawToJpeg->destroyInstance();
#endif
}

MBOOL
AutomationMain::
sendcommand(
    MUINT32 const a_u4Ioctl,
    MUINT8 *puParaIn,
    MUINT32 const u4ParaInLen,
    MUINT8 *puParaOut,
    MUINT32 const u4ParaOutLen,
    MUINT32 *pu4RealParaOutLen
)
{
    MBOOL result = MTRUE;
    MY_LOG("[%s] Ioctl(%d) +",__FUNCTION__, a_u4Ioctl);
    AUTOMATION_IMAGE_INFO* pImageInfo = (AUTOMATION_IMAGE_INFO*)puParaIn;
    ACDK_CCT_TABLE_SET_STRUCT* pShadingTbl = (ACDK_CCT_TABLE_SET_STRUCT*)puParaIn;
//    AUTOMATION_NVRAM_ISP_PARAM* pIspParams = (AUTOMATION_NVRAM_ISP_PARAM*)puParaIn;
    AUTOMATION_ISP_PARAM_SET* pIspParamSet = (AUTOMATION_ISP_PARAM_SET*)puParaIn;
    switch(a_u4Ioctl)
    {
        case ACDK_AUTOMATION_SET_INFO:
            MY_LOG("[%s]Ioctl ACDK_AUTOMATION_SET_INFO",__FUNCTION__);
            result = updateInfo(pImageInfo);
            break;
        case ACDK_AUTOMATION_APPLY_ISP_PARAMS:
            MY_LOG("[%s]Ioctl ACDK_AUTOMATION_APPLY_ISP_PARAMS",__FUNCTION__);
//            result = applyISPParams(pIspParams);
            result = applyISPParams(pIspParamSet);
            break;
        case ACDK_AUTOMATION_RUN:
            MY_LOG("[%s]Ioctl ACDK_AUTOMATION_RUN",__FUNCTION__);
            result = run();
            break;
        default:
            MY_LOG("[%s] unsupport Ioctl(%d)",__FUNCTION__, a_u4Ioctl);
            break;
    }
    MY_LOG("[%s] -",__FUNCTION__);
    return result;

}

MBOOL
AutomationMain::
updateInfo(AUTOMATION_IMAGE_INFO* pImageInfo)
{
    MY_LOG("[%s] +",__FUNCTION__);
#if (ISP_AUTOMATION_FUNC_EN)
    // set info
    mImageInfo.i4Width = pImageInfo->u4Width;
    mImageInfo.i4Height= pImageInfo->u4Height;
    mImageInfo.i4JpegWidth = pImageInfo->u4JpegWidth;
    mImageInfo.i4JpegHeight= pImageInfo->u4JpegHeight;
    //mImageInfo.bUnpack2PackEn = pImageInfo->bUnpack2PackEn;
	mImageInfo.bUnpack2PackEn = 1;
    mImageInfo.i4Format = IRawToJpeg::ImgFmt_BAYER10;
	mImageInfo.u4Pixel = pImageInfo->u4Pixel;
    ::memcpy(mpInputFile, pImageInfo->pInput, 128);
    ::memcpy(mpOutputFile, pImageInfo->pOutput, 128);

    MY_LOG("[%s] i4Width(%d), i4Height(%d), mpInputFile(%s), mpOutputFile(%s), mu4PixelID(%d), bUnpack2PackEn(%d)",
        __FUNCTION__,
        mImageInfo.i4Width,
        mImageInfo.i4Height,
        mpInputFile,
        mpOutputFile,
        mu4PixelID,
        mImageInfo.bUnpack2PackEn);

    // read input
    char filename[256];
    sprintf(filename, "%s/%s", AUTOMATION_FOLDER,mpInputFile);
    MY_LOG("[%s] filename : %s",__FUNCTION__, filename);
    pRawToJpeg->readFile(filename, mImageInfo);
#endif
    MY_LOG("[%s] -",__FUNCTION__);
    return MTRUE;

}

MBOOL
AutomationMain::
//applyISPParams(AUTOMATION_NVRAM_ISP_PARAM* pIspParams)
applyISPParams(AUTOMATION_ISP_PARAM_SET* pIspParam)
{
    MY_LOG("[%s] +", __FUNCTION__);
#if (ISP_AUTOMATION_FUNC_EN)
#if 1
    pRawToJpeg->setIspCtl(pIspParam->IspCtl.IspReg_RgbEn.Raw, pIspParam->IspCtl.IspReg_YuvEn.Raw, pIspParam->IspCtl.IspReg_Yuv2En.Raw);
#endif

    pRawToJpeg->setPGN(pIspParam->IspNvram.sPGN);
    pRawToJpeg->setSL2(pIspParam->IspNvram.sSL2);
    pRawToJpeg->setUDM(pIspParam->IspNvram.sUDM);
    pRawToJpeg->setCCM(pIspParam->IspNvram.sCCM);
    pRawToJpeg->setGGM(pIspParam->IspNvram.sGGM);
    pRawToJpeg->setDBS(pIspParam->IspNvram.sDBS);
    pRawToJpeg->setOBC(pIspParam->IspNvram.sOBC2);
    pRawToJpeg->setBNR_BPC(pIspParam->IspNvram.sBNRBpc);
    pRawToJpeg->setBNR_NR1(pIspParam->IspNvram.sBNRNr1);
    pRawToJpeg->setBNR_PDC(pIspParam->IspNvram.sBNRPdc);
    pRawToJpeg->setLSC(pIspParam->IspNvram.sLSC2, pIspParam->IspLsc.LscTable, sizeof(pIspParam->IspLsc.LscTable));
    pRawToJpeg->setRNR(pIspParam->IspNvram.sRNR);
    pRawToJpeg->setSL2G(pIspParam->IspNvram.sSL2G);
    pRawToJpeg->setG2C(pIspParam->IspNvram.sG2C);
    pRawToJpeg->setANR(pIspParam->IspNvram.sANR, pIspParam->IspNvram.sANR_Tbl);
    pRawToJpeg->setANR2(pIspParam->IspNvram.sANR2);
    pRawToJpeg->setCCR(pIspParam->IspNvram.sCCR);
    pRawToJpeg->setPCA(pIspParam->IspNvram.sPCA, pIspParam->IspNvram.u4PCA_Lut);
    pRawToJpeg->setSEEE(pIspParam->IspNvram.sEE, pIspParam->IspNvram.sSE);
#endif

    MY_LOG("[%s] -", __FUNCTION__);
    return MTRUE;
}


#if 1
MBOOL
AutomationMain::
run()
{
    MINT32 bDump = property_get_int32("vendor.automation.dump.enable", 0);
    MY_LOG("[%s] +",__FUNCTION__);
#if (ISP_AUTOMATION_FUNC_EN)
    // 1. run
    MBOOL ret = pRawToJpeg->run();
    if(ret != MTRUE)
    {
        MY_LOG("[%s] fail!\n", __FUNCTION__);
        return MFALSE;
    }

    // 2. file path
    char filename[256];

    // 4. save output
    if(bDump)
    {
        MY_LOG("[%s] save pack raw file",__FUNCTION__);
        sprintf(filename, "%s/%s", AUTOMATION_FOLDER,"pack5312x2976.raw");
        pRawToJpeg->saveFile(filename,IRawToJpeg::E_PACK_RAW);

//        MY_LOG("[%s] save Yuv File",__FUNCTION__);
//        sprintf(filename, "%s/%s", AUTOMATION_FOLDER,"yuv5312x2976.yuv");
//        pRawToJpeg->saveFile(filename,IRawToJpeg::E_YUV);
    }
    MY_LOG("[%s] save Yuv File",__FUNCTION__);
    sprintf(filename, "%s/%s", AUTOMATION_FOLDER,"yuv5344x4016.yuv");
    pRawToJpeg->saveFile(filename,IRawToJpeg::E_YUV);

    sprintf(filename, "%s/%s", AUTOMATION_FOLDER, mpOutputFile);
    MY_LOG("[%s] filename : %s",__FUNCTION__, filename);
    MY_LOG("[%s] main save jpg file",LOG_TAG);
    pRawToJpeg->saveFile(filename,IRawToJpeg::E_JPEG);
    pRawToJpeg->releaseFile();
#endif

    MY_LOG("[%s] -",__FUNCTION__);
    return MTRUE;

}
#endif

IAutomationMain*
IAutomationMain::
createInstance()
{
    return new AutomationMain();
}

void
AutomationMain::
destroyInstance()
{
    delete this;
}


#if 0		// disable the test code by jokery
int main(int argc, char *argv[])
{
    MBOOL ret = MFALSE;
    char    Filename[256];
    MUINT32 Val = 0;

    AutomationMain AutoMain;
    AUTOMATION_IMAGE_INFO AtImageInfo;
//    AUTOMATION_NVRAM_ISP_PARAM AtIspParams;

    if (argc != 4) {
        ::printf("Command Error!\n");
        ::printf("Command Format > raw2yuv RgbEn YuvEn Yuv2En\n");
        return 1;
    }

    ::sscanf(argv[1], "%x", &Val);
    AutoMain.mRgbEn = Val;
    ::sscanf(argv[2], "%x", &Val);
    AutoMain.mYuvEn = Val;
    ::sscanf(argv[3], "%x", &Val);
    AutoMain.mYuv2En = Val;

    ::printf("Param - RgbEn:0x%x  YuvEn:0x%x  Yuv2En:0x%x\n",AutoMain.mRgbEn,AutoMain.mYuvEn,AutoMain.mYuv2En);

    AtImageInfo.u4Width = 5344;     //5312;
    AtImageInfo.u4Height = 4016;   //2976;
    AtImageInfo.bUnpack2PackEn = 1;
    AtImageInfo.u4Pixel = 2;    //0;
    AtImageInfo.u4JpegWidth = AtImageInfo.u4Width;
    AtImageInfo.u4JpegHeight = AtImageInfo.u4Height;
    strcpy( AtImageInfo.pInput, RAW_IMG_FILENAME );
    strcpy( AtImageInfo.pOutput, JPEG_IMG_FILENAME );

    AutoMain.sendcommand(ACDK_AUTOMATION_SET_INFO, (MUINT8 *) &AtImageInfo, sizeof(AtImageInfo), NULL, 0, NULL);

    AutoMain.sendcommand(ACDK_AUTOMATION_APPLY_ISP_PARAMS, (MUINT8 *) &AtIspParams, sizeof(AtIspParams), NULL, 0, NULL);

    AutoMain.sendcommand(ACDK_AUTOMATION_RUN, NULL, 0, NULL, 0, NULL);

    return 0;
}



#endif
