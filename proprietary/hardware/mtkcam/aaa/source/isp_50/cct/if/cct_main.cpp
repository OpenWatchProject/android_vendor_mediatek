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
#define LOG_TAG "CCTIF"
#define MTK_LOG_ENABLE 1

//
#include <utils/Errors.h>
#include <cutils/log.h>
#include "cct_feature.h"

#include <mtkcam/drv/IHalSensor.h>
#include <nvram_drv.h>
#include <drv/isp_reg.h>
#include "cct_main.h"
#include "cct_imp.h"
#include <nvbuf_util.h>
//
using namespace NSCam;

/*******************************************************************************
*
********************************************************************************/
#define MY_LOG(fmt, arg...)    ALOGD(fmt, ##arg)
#define MY_ERR(fmt, arg...)    ALOGE("Err: %5d: " fmt, __LINE__, ##arg)

/*******************************************************************************
*
********************************************************************************/
CCTIF* CCTIF::createInstance()
{
    return  new CctImp;
}

void CctImp::destroyInstance()
{
    delete this;
}

/*******************************************************************************
*
********************************************************************************/
CctImp::CctImp()
    : CCTIF()
    , m_bGetSensorStaticInfo(MFALSE)
    , m_pCctCtrl(NULL)
{
    MY_LOG("[CCTIF] E\n");

    NvramDrvBase** pnvram_drv = &m_cctctrl_prop.nvram_prop.m_pnvramdrv;

    /*FIXME: No sensorHal*///m_pSensorHalObj = SensorHal::createInstance();

    *pnvram_drv = NvramDrvBase::createInstance();

}

/*******************************************************************************
*
********************************************************************************/
CctImp::~CctImp()
{
    MY_LOG("[~CCTIF] E\n");
/*FIXME: No sensorHal*//*
    if (m_pSensorHalObj) {
        m_pSensorHalObj->destroyInstance();
        m_pSensorHalObj = NULL;
    }
*/
    if  (m_pCctCtrl )
    {
        m_pCctCtrl->destroyInstance();
        m_pCctCtrl = NULL;
    }
}
/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::init(MINT32 sensorType)
{
    MUINT32 sen_id = 0;
    MINT32 err = CCTIF_NO_ERROR;
    NvramDrvBase* pnvram_drv = m_cctctrl_prop.nvram_prop.m_pnvramdrv;

    /*
    *   SENSOR INIT
    */
    mSensorDev = sensorType;
    m_bGetSensorStaticInfo = MFALSE;

#if 0 /*FIXME: No sensorHal*/
    if(!m_pSensorHalObj) {
        MY_ERR("[CctImp::init] m_pSensorHalObj != NULL before init()\n");
        return -1;
    }

    m_pSensorHalObj->init();
    err = m_pSensorHalObj->sendCommand((halSensorDev_e)sensorType,
                                       SENSOR_CMD_SET_SENSOR_DEV,
                                       0,
                                       0,
                                       0);
    if (err != 0) {
        MY_ERR("[CctImp::init] set sensor dev error\n");
        return -1;
    }

    err = m_pSensorHalObj->sendCommand((halSensorDev_e)sensorType, SENSOR_CMD_GET_SENSOR_ID, (MINT32)&sen_id);
    if (err != 0) {
        MY_ERR("[CctImp::init] get sensor id error\n");
        return -1;
    }
#endif
    m_cctctrl_prop.sen_prop.m_sen_type = (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorType;
    MY_LOG("[CctImp::init] sen_id = %d\n", sen_id);
    MY_LOG("[CctImp::init] sensorType = %d\n", sensorType);


    /*
    *   NVRAM INIT
    */
    if(!pnvram_drv) {
        MY_ERR("[CctImp::init] pnvram_drv == NULL before init()\n");
        return -1;
    }

    aaaCCTFeatureControl(ACDK_CCT_MCU_NAME_INIT, NULL, 0, NULL, 0, NULL); //init AF before NVRAM get buf of lens
    aaaCCTFeatureControl(ACDK_CCT_MCU_NAME_UNINIT, NULL, 0, NULL, 0, NULL);

    /*
    *   CCT CTRL INIT
    */
    m_pCctCtrl = CctCtrl::createInstance(&m_cctctrl_prop);
    if  (!m_pCctCtrl )
    {
        MY_ERR("[CctImp::init] m_pCctCtrl == NULL\n");
        return  -1;
    }

    return  CCTIF_NO_ERROR;

}

MINT32 CctImp::setCctImpObj(AcdkCctBase *a_pAcdkCctBaseObj)
{
    m_pAcdkCctBaseObj = a_pAcdkCctBaseObj;
    return  CCTIF_NO_ERROR;
}

MINT32 CctImp::uninit()
{
    /*FIXME: No sensorHal*///m_pSensorHalObj->uninit();
    return  CCTIF_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
CctCtrl*
CctCtrl::
createInstance(const cctctrl_prop_t *prop)
{

    CctCtrl* pCctCtrl = NULL;
    CAMERA_DUAL_CAMERA_SENSOR_ENUM sen_type = prop->sen_prop.m_sen_type;
    NvramDrvBase* pnvram_drv = prop->nvram_prop.m_pnvramdrv;

    NVRAM_CAMERA_ISP_PARAM_STRUCT*  pbuf_isp;
    NVRAM_CAMERA_SHADING_STRUCT*    pbuf_shd;
    NVRAM_CAMERA_3A_STRUCT*    pbuf_3a;
    NVRAM_LENS_PARA_STRUCT*    pbuf_ln;
    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_ISP, sen_type, (void*&)pbuf_isp);
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_SHADING, sen_type, (void*&)pbuf_shd);
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, sen_type, (void*&)pbuf_3a);
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_LENS, sen_type, (void*&)pbuf_ln);

    pCctCtrl = new CctCtrl(prop, pbuf_isp, pbuf_shd, pbuf_3a, pbuf_ln);

    return  pCctCtrl;

}

void
CctCtrl::
destroyInstance()
{
}

CctCtrl::
CctCtrl(
    const cctctrl_prop_t *prop,
    NVRAM_CAMERA_ISP_PARAM_STRUCT*const pBuf_ISP,
    NVRAM_CAMERA_SHADING_STRUCT*const   pBuf_SD,
    NVRAM_CAMERA_3A_STRUCT*const   pBuf_3A,
    NVRAM_LENS_PARA_STRUCT*const   pBuf_LN
)
    : m_eSensorEnum(prop->sen_prop.m_sen_type)
    , m_pNvramDrv(prop->nvram_prop.m_pnvramdrv)
    , m_rBuf_ISP(*pBuf_ISP)
    , m_rISPComm(m_rBuf_ISP.ISPComm)
    , m_rISPRegs(m_rBuf_ISP.ISPRegs)
    , m_rISPRegsIdx(m_rBuf_ISP.ISPRegs.Idx)
    , m_rISPToneMap(m_rBuf_ISP.ISPToneMap)
    , m_rISPColorTbl (m_rBuf_ISP.ISPColorTbl)
    , m_fgEnabled_OB(MTRUE)
    , m_u4Backup_OB(0)
    , m_rBuf_SD (*pBuf_SD)
    , m_rBuf_3A (*pBuf_3A)
    , m_rBuf_LN (*pBuf_LN)
{
}

CctCtrl::
~CctCtrl()
{
    if (m_pNvramDrv) {
        m_pNvramDrv->destroyInstance();
        m_pNvramDrv = NULL;
    }
}

MINT32 CCTIF::setCCTSensorDev(MINT32 sensor_dev)
{
    if((sensor_dev < SENSOR_DEV_NONE) || (sensor_dev > SENSOR_DEV_SUB_2))
        return CCTIF_UNSUPPORT_SENSOR_TYPE;

    mSensorDev = sensor_dev;

    return CCTIF_NO_ERROR;
}

