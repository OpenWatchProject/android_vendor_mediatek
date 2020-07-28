#define LOG_TAG "CCTIF"

extern "C" {
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/mtkfb.h>
}

#include <mtkcam/aaa/cct/types.h>
#include "AcdkLog.h"
#include "AcdkCctBase.h"
#include "cct_main.h"
#include "cct_calibration.h"
#include <mtkcam/drv/IHalSensor.h>
#include "AcdkErrCode.h"

#include "cct_if.h"
#include "cct_imp.h"
#include <mtkcam/aaa/cct/errcode.h>
/*
*    @CCT_IF.CPP
*    CCT_IF provides user to do camera calibration or tuning by means of
*    mixing following commands. Which involoved with
*    3A, ISP, sensor, NVRAM, Calibration, ACDK releated.
*
*    CCT FW working model is based on ACDK framework, so it is required make
*    sure AcdkCctBase object is available before CCTIF Open.
*
*    CCT FW presents the following APIs:
*
*    CCTIF_Open
*    CCTIF_Close
*    CCTIF_Init
*    CCTIF_DeInit
*    CCTIF_IOControl
*/



/*static*/
static AcdkCctBase *g_pAcdkCctBaseObj = NULL;
static CCTIF *g_pCCTIFObj = NULL;
static AcdkCalibration *g_pCCTCalibrationObj = NULL;
static MBOOL g_bAcdkOpend = MFALSE;

using namespace NSACDK;


#ifdef __cplusplus
extern "C" {
#endif

MBOOL CCTIF_Open()
{
    ACDK_LOGD("[%s] Start\n", __FUNCTION__);
    MINT32 err = MTRUE;

    if(!g_pAcdkCctBaseObj) {
        ACDK_LOGE("[%s] no AcdjBaseObj\n", __FUNCTION__);
        return MFALSE;
    }

    g_pCCTIFObj = CCTIF::createInstance();

    if(!g_pCCTIFObj) {
        ACDK_LOGE("[%s] CCTIFObj create fail\n", __FUNCTION__);
        err &= MFALSE;

    }

    g_pCCTCalibrationObj = new AcdkCalibration();

    if(!g_pCCTCalibrationObj) {
        ACDK_LOGE("[%s] CCTCalibrationObj create fail\n", __FUNCTION__);
        err &= MFALSE;

        g_pCCTIFObj->destroyInstance();

    }

    ACDK_LOGD("[%s] End\n", __FUNCTION__);

    return err;

}


MBOOL CCTIF_Close()
{
    ACDK_LOGD("[%s] Start\n", __FUNCTION__);

    if(g_pCCTIFObj)
        g_pCCTIFObj->destroyInstance();


    if(g_pCCTCalibrationObj) {
        delete g_pCCTCalibrationObj;
        g_pCCTCalibrationObj = NULL;
    }

    ACDK_LOGD("[%s] End\n", __FUNCTION__);

    return MTRUE;
}


MBOOL CCTIF_Init(MINT32 dev)
{
    ACDK_LOGD("[%s] Start\n", __FUNCTION__);

    if(!g_pAcdkCctBaseObj) {
        ACDK_LOGE("[%s] no AcdjBaseObj\n", __FUNCTION__);
        return MFALSE;
    }

    if(!g_pCCTIFObj) {
        ACDK_LOGE("[%s] CCTIFObj create fail\n", __FUNCTION__);
        return MFALSE;
    }

    if(!g_pCCTCalibrationObj) {
        ACDK_LOGE("[%s] CCTCalibrationObj create fail\n", __FUNCTION__);
        return MFALSE;
    }

    MINT32 err;//, dev = SENSOR_DEV_MAIN;

    /*
        g_pAcdkCctBaseObj->get_sensor_info(sensor_info);
        dev = sensor_info.dev;
    */

    err = g_pCCTIFObj->setCCTSensorDev(dev);

    if(err != CCTIF_NO_ERROR) {
        ACDK_LOGE("[%s] Unsupported sensor type \n", __FUNCTION__);
        return /*err*/MFALSE;
    }

    err = g_pCCTIFObj->init(dev);
    err = ((CctImp*)g_pCCTIFObj)->setCctImpObj(g_pAcdkCctBaseObj);

    if(err != CCTIF_NO_ERROR) {
        ACDK_LOGE("[%s] CCTIF init fail\n", __FUNCTION__);
        return /*err*/MFALSE;
    }

    g_pCCTCalibrationObj->init(dev, g_pAcdkCctBaseObj);


    ACDK_LOGD("[%s] End\n", __FUNCTION__);

    return /*err*/MTRUE;

}


MBOOL CCTIF_DeInit()
{
    ACDK_LOGD("[%s] Start\n", __FUNCTION__);

    if(!g_pCCTIFObj) {
        ACDK_LOGE("[%s] CCTIFObj create fail\n", __FUNCTION__);
        return MFALSE;
    }

    MINT32 err;

    err = g_pCCTIFObj->uninit();

    if(err != CCTIF_NO_ERROR) {
        ACDK_LOGE("[%s] CCTIFObj uninit fail\n", __FUNCTION__);
        return MFALSE;
    }

    err = g_pCCTCalibrationObj->uninit();

    /*
    if(err == MFALSE) {
        ACDK_LOGE("[%s] CCTCalibrationObj uninit fail\n", __FUNCTION__);
        return err;
    }
    */

    return /*err*/MTRUE;

}

MBOOL CCTIF_FeatureCtrl(MUINT32 a_u4Ioctl, MUINT8 *puParaIn, MUINT32 u4ParaInLen, MUINT8 *puParaOut, MUINT32 u4ParaOutLen, MUINT32 *pu4RealParaOutLen)
{
    MBOOL err = MTRUE;
    MINT32 errID = 0;

    if(!g_pCCTIFObj) {
        ACDK_LOGE("[%s] CCTIFObj create fail\n", __FUNCTION__);
        return MFALSE;
    }

    if (a_u4Ioctl >= CCT_ISP_FEATURE_START && a_u4Ioctl < CCT_ISP_FEATURE_START + MAX_SUPPORT_CMD)
    {
        ACDK_LOGD("[%s] ISP Feature(0x%08x)\n", __FUNCTION__, a_u4Ioctl);

        errID = g_pCCTIFObj->ispCCTFeatureControl(a_u4Ioctl,
                                                puParaIn,
                                                u4ParaInLen,
                                                puParaOut,
                                                u4ParaOutLen,
                                                pu4RealParaOutLen);
        if (errID == CCTIF_NO_ERROR) err = MTRUE;
        else err = MFALSE;

    }
    else if (a_u4Ioctl >= CCT_SENSOR_FEATURE_START && a_u4Ioctl < CCT_SENSOR_FEATURE_START + MAX_SUPPORT_CMD)
    {
        ACDK_LOGD("[%s] Sensor Feature(0x%08x)\n", __FUNCTION__, a_u4Ioctl);

        errID = g_pCCTIFObj->sensorCCTFeatureControl(a_u4Ioctl,
                                                   puParaIn,
                                                   u4ParaInLen,
                                                   puParaOut,
                                                   u4ParaOutLen,
                                                   pu4RealParaOutLen);
        if (errID == CCTIF_NO_ERROR) err = MTRUE;
        else err = MFALSE;
    }
    else if (a_u4Ioctl >= CCT_NVRAM_FEATURE_START && a_u4Ioctl < CCT_NVRAM_FEATURE_START + MAX_SUPPORT_CMD)
    {
        ACDK_LOGD("[%s] NVRAM Feature(0x%08x)\n", __FUNCTION__, a_u4Ioctl);

        errID = g_pCCTIFObj->nvramCCTFeatureControl(a_u4Ioctl,
                                                  puParaIn,
                                                  u4ParaInLen,
                                                  puParaOut,
                                                  u4ParaOutLen,
                                                  pu4RealParaOutLen);
        if (errID == CCTIF_NO_ERROR) err = MTRUE;
        else err = MFALSE;
    }
    else if (a_u4Ioctl >= CCT_3A_FEATURE_START && a_u4Ioctl < CCT_3A_FEATURE_START + MAX_SUPPORT_CMD)
    {
        ACDK_LOGD("[%s] 3A Feature(0x%08x)\n", __FUNCTION__, a_u4Ioctl);

        errID = g_pCCTIFObj->aaaCCTFeatureControl(a_u4Ioctl,
                                                puParaIn,
                                                u4ParaInLen,
                                                puParaOut,
                                                u4ParaOutLen,
                                                pu4RealParaOutLen);
        if (errID == CCTIF_NO_ERROR) err = MTRUE;
        else err = MFALSE;
    }

    else if (a_u4Ioctl >= CCT_CAPTURE_FEATURE_START && a_u4Ioctl < CCT_CAPTURE_FEATURE_START + MAX_SUPPORT_CMD)
    {
        ACDK_LOGD("[%s] Capture Feature(0x%08x)\n", __FUNCTION__, a_u4Ioctl);

        errID = g_pCCTIFObj->captureFeatureControl(a_u4Ioctl,
                                                puParaIn,
                                                u4ParaInLen,
                                                puParaOut,
                                                u4ParaOutLen,
                                                pu4RealParaOutLen);
        if (errID == CCTIF_NO_ERROR) err = MTRUE;
        else err = MFALSE;
    }
    else
    {
        ACDK_LOGD("[%s] Can't interpret CCT cmd(0x%08x)\n", __FUNCTION__, a_u4Ioctl);
        err = MFALSE;
    }

    return err;
}

MBOOL CCTIF_IOControl(MUINT32 a_u4Ioctl, ACDK_FEATURE_INFO_STRUCT *a_prAcdkFeatureInfo)
{
    ACDK_LOGD("[%s] CCTIF cmd = 0x%x\n", __FUNCTION__, a_u4Ioctl);

    if(!g_pAcdkCctBaseObj) {
        ACDK_LOGE("[%s] no AcdjBaseObj\n", __FUNCTION__);
        return MFALSE;
    }

    if(!g_pCCTIFObj) {
        ACDK_LOGE("[%s] CCTIFObj create fail\n", __FUNCTION__);
        return MFALSE;
    }

    if(!g_pCCTCalibrationObj) {
        ACDK_LOGE("[%s] CCTCalibrationObj create fail\n", __FUNCTION__);
        return MFALSE;
    }

    MBOOL err = MTRUE;
    MINT32 errID = 0;

    if(a_u4Ioctl >= ACDK_CCT_CDVT_START && a_u4Ioctl < ACDK_CCT_CDVT_END)
    {
        ACDK_LOGD("[%s] CCT CDVT\n", __FUNCTION__);

        errID= g_pCCTCalibrationObj->sendcommand(a_u4Ioctl,
                                                  a_prAcdkFeatureInfo->puParaIn,
                                                  a_prAcdkFeatureInfo->u4ParaInLen,
                                                  a_prAcdkFeatureInfo->puParaOut,
                                                  a_prAcdkFeatureInfo->u4ParaOutLen,
                                                  a_prAcdkFeatureInfo->pu4RealParaOutLen);
    if (errID == S_CCT_CALIBRATION_OK) err = MTRUE;
    else err = MFALSE;

    }
    else if(a_u4Ioctl == ACDK_CCT_V2_OP_SHADING_CAL)
    {
        ACDK_LOGD("[%s] CCT LSC cal\n", __FUNCTION__);

        errID = g_pCCTCalibrationObj->sendcommand(a_u4Ioctl,
                                                  a_prAcdkFeatureInfo->puParaIn,
                                                  a_prAcdkFeatureInfo->u4ParaInLen,
                                                  a_prAcdkFeatureInfo->puParaOut,
                                                  a_prAcdkFeatureInfo->u4ParaOutLen,
                                                  a_prAcdkFeatureInfo->pu4RealParaOutLen);
        if (errID == S_CCT_CALIBRATION_OK) err = MTRUE;
        else err = MFALSE;

    }
    else {

        if (a_u4Ioctl >= CCT_ISP_FEATURE_START && a_u4Ioctl < CCT_ISP_FEATURE_START + MAX_SUPPORT_CMD)
        {
            ACDK_LOGD("[%s] ISP Feature\n", __FUNCTION__);

            errID = g_pCCTIFObj->ispCCTFeatureControl(a_u4Ioctl,
                                                    a_prAcdkFeatureInfo->puParaIn,
                                                    a_prAcdkFeatureInfo->u4ParaInLen,
                                                    a_prAcdkFeatureInfo->puParaOut,
                                                    a_prAcdkFeatureInfo->u4ParaOutLen,
                                                    a_prAcdkFeatureInfo->pu4RealParaOutLen);
            if (errID == CCTIF_NO_ERROR) err = MTRUE;
            else err = MFALSE;

        }
        else if (a_u4Ioctl >= CCT_SENSOR_FEATURE_START && a_u4Ioctl < CCT_SENSOR_FEATURE_START + MAX_SUPPORT_CMD)
        {
            ACDK_LOGD("[%s] Sensor Feature\n", __FUNCTION__);

            errID = g_pCCTIFObj->sensorCCTFeatureControl(a_u4Ioctl,
                                                       a_prAcdkFeatureInfo->puParaIn,
                                                       a_prAcdkFeatureInfo->u4ParaInLen,
                                                       a_prAcdkFeatureInfo->puParaOut,
                                                       a_prAcdkFeatureInfo->u4ParaOutLen,
                                                       a_prAcdkFeatureInfo->pu4RealParaOutLen);
            if (errID == CCTIF_NO_ERROR) err = MTRUE;
            else err = MFALSE;
        }
        else if (a_u4Ioctl >= CCT_NVRAM_FEATURE_START && a_u4Ioctl < CCT_NVRAM_FEATURE_START + MAX_SUPPORT_CMD)
        {
            ACDK_LOGD("[%s] NVRAM Feature\n", __FUNCTION__);

            errID = g_pCCTIFObj->nvramCCTFeatureControl(a_u4Ioctl,
                                                      a_prAcdkFeatureInfo->puParaIn,
                                                      a_prAcdkFeatureInfo->u4ParaInLen,
                                                      a_prAcdkFeatureInfo->puParaOut,
                                                      a_prAcdkFeatureInfo->u4ParaOutLen,
                                                      a_prAcdkFeatureInfo->pu4RealParaOutLen);
            if (errID == CCTIF_NO_ERROR) err = MTRUE;
            else err = MFALSE;
        }
        else if (a_u4Ioctl >= CCT_3A_FEATURE_START && a_u4Ioctl < CCT_3A_FEATURE_START + MAX_SUPPORT_CMD)
        {
            ACDK_LOGD("[%s] 3A Feature\n", __FUNCTION__);

            errID = g_pCCTIFObj->aaaCCTFeatureControl(a_u4Ioctl,
                                                    a_prAcdkFeatureInfo->puParaIn,
                                                    a_prAcdkFeatureInfo->u4ParaInLen,
                                                    a_prAcdkFeatureInfo->puParaOut,
                                                    a_prAcdkFeatureInfo->u4ParaOutLen,
                                                    a_prAcdkFeatureInfo->pu4RealParaOutLen);
            if (errID == CCTIF_NO_ERROR) err = MTRUE;
            else err = MFALSE;
        }

        else if (a_u4Ioctl >= CCT_CAPTURE_FEATURE_START && a_u4Ioctl < CCT_CAPTURE_FEATURE_START + MAX_SUPPORT_CMD)
        {
            ACDK_LOGD("[%s] Capture Feature\n", __FUNCTION__);

            errID = g_pCCTIFObj->captureFeatureControl(a_u4Ioctl,
                                                    a_prAcdkFeatureInfo->puParaIn,
                                                    a_prAcdkFeatureInfo->u4ParaInLen,
                                                    a_prAcdkFeatureInfo->puParaOut,
                                                    a_prAcdkFeatureInfo->u4ParaOutLen,
                                                    a_prAcdkFeatureInfo->pu4RealParaOutLen);
            if (errID == CCTIF_NO_ERROR) err = MTRUE;
            else err = MFALSE;

        }
        else
        {
            ACDK_LOGD("[%s] Can't interpret CCT cmd\n", __FUNCTION__);
            err = MFALSE;
        }

    }


    ACDK_LOGD("[%s] End err=%d\n", __FUNCTION__,err);

    return err;

}

/////////////////////////////////////////////////////////////////////////
//
//   CctIF_ACDK_Open () -
//!  brief ACDK I/F CctIF_ACDK_Open()
//!
/////////////////////////////////////////////////////////////////////////
MBOOL CctIF_ACDK_Open()
{
    ACDK_LOGD("+");

    if(g_bAcdkOpend == MTRUE)
    {
        ACDK_LOGE("ACDK device already opened");
        return MFALSE;
    }

    g_pAcdkCctBaseObj = AcdkCctBase::createInstance();
    g_bAcdkOpend = MTRUE;

    ACDK_LOGD("-");
    return MTRUE;
}

/////////////////////////////////////////////////////////////////////////
//
//   CctIF_ACDK_OpenBypass () -
//!  brief ACDK I/F CctIF_ACDK_OpenBypass()
//!
/////////////////////////////////////////////////////////////////////////
MBOOL CctIF_ACDK_OpenBypass(ACDK_CCT_BYPASS_FLAG bypassFlag)
{
    ACDK_LOGD("+");

    if(g_bAcdkOpend == MTRUE)
    {
        ACDK_LOGE("ACDK device already opened");
        return MFALSE;
    }

    g_pAcdkCctBaseObj = AcdkCctBase::createInstance(bypassFlag);
    g_bAcdkOpend = MTRUE;

    ACDK_LOGD("-");
    return MTRUE;
}

/////////////////////////////////////////////////////////////////////////
//
//   CctIF_ACDK_Close () -
//!  brief ACDK I/F CctIF_ACDK_Close()
//!
/////////////////////////////////////////////////////////////////////////
MBOOL CctIF_ACDK_Close()
{
    ACDK_LOGD("+");

    if(g_bAcdkOpend == MFALSE)
    {
        ACDK_LOGE("Acdk device is not opened");
        return MFALSE;
    }

    g_pAcdkCctBaseObj->destroyInstance();
    g_bAcdkOpend = MFALSE;

    ACDK_LOGD("-");
    return MTRUE;
}

/////////////////////////////////////////////////////////////////////////
//
//   CctIF_ACDK_Init () -
//!  brief ACDK I/F CctIF_ACDK_Init()
//!
/////////////////////////////////////////////////////////////////////////
MBOOL CctIF_ACDK_Init()
{
    ACDK_LOGD("+");

    if(g_bAcdkOpend == MFALSE)
    {
        ACDK_LOGE("ACDK device is not opened");
        return MFALSE;
    }

    MINT32 mrRet;

    mrRet = g_pAcdkCctBaseObj->init();
    if(mrRet != 0)
    {
        ACDK_LOGE("Fail to init AcdkCCTCtrl");
        return MFALSE;
    }

    ACDK_LOGD("-");
    return MTRUE;
}

/////////////////////////////////////////////////////////////////////////
//
//   CctIF_ACDK_DeInit () -
//!  brief ACDK I/F CctIF_ACDK_DeInit()
//!
/////////////////////////////////////////////////////////////////////////
MBOOL CctIF_ACDK_DeInit()
{
    ACDK_LOGD("+");

    MINT32 err;

    if (g_bAcdkOpend == MFALSE)
    {
        ACDK_LOGE("ACDK device is not opened");
        return MFALSE;
    }

    err = g_pAcdkCctBaseObj->uninit();
    if (err != 0)
    {
        ACDK_LOGE("Fail to disable ACDK CamCtrl  err(0x%x)", err);
        return MFALSE;
    }

    ACDK_LOGD("-");
    return MTRUE;
}

/////////////////////////////////////////////////////////////////////////
//
//   CctIF_ACDK_IOControl () -
//!  brief ACDK I/F CctIF_ACDK_IOControl()
//!
/////////////////////////////////////////////////////////////////////////
MBOOL CctIF_ACDK_IOControl(MUINT32 a_u4Ioctl, ACDK_FEATURE_INFO_STRUCT *a_prAcdkFeatureInfo)
{
    ACDK_LOGD("+");

    if (g_bAcdkOpend == MFALSE)
    {
        ACDK_LOGE("ACDK device is not opened");
        return MFALSE;
    }

    MBOOL bRet = MTRUE;

    ACDK_LOGD("Reveive IOControl Code:0x%x", a_u4Ioctl);

    if(a_u4Ioctl > ACDK_COMMAND_START && a_u4Ioctl < ACDK_COMMAND_END)
    {
        ACDK_LOGD("ACDK_COMMAND");
        bRet = g_pAcdkCctBaseObj->sendcommand (a_u4Ioctl,
                                            a_prAcdkFeatureInfo->puParaIn,
                                            a_prAcdkFeatureInfo->u4ParaInLen,
                                            a_prAcdkFeatureInfo->puParaOut,
                                            a_prAcdkFeatureInfo->u4ParaOutLen,
                                            a_prAcdkFeatureInfo->pu4RealParaOutLen);
    }
    else
    {
        ACDK_LOGD("[%s] Can't interpret ADCK cmd\n", __FUNCTION__);
        return MFALSE;
    }


    ACDK_LOGD("-");
    return MTRUE;
}


#ifdef __cplusplus
} // extern "C"
#endif
