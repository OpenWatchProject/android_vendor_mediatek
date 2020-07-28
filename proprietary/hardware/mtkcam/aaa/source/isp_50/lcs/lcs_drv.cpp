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
*      TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
* @file lcs_drv.cpp
*
* LCS Driver Source File
*
*/
#define LOG_TAG "LCSDrv"

#include <utils/Errors.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <queue>

using namespace std;
using namespace android;





//using namespace NSImageio;
//using namespace NSIspio;

#include <drv/isp_reg.h> // for register name

#include <mtkcam/def/PriorityDefs.h>
#include <mtkcam/drv/IHalSensor.h>

#include <imageio/Cam_Notify_datatype.h> // for LCS_REG_CFG definition

//#include "mtkcam/drv/imem_drv.h"
//#include "camera_custom_vhdr.h"    // TODO-not suer and file is not exist

#include "isp_mgr/isp_mgr.h"

//using namespace NSIspTuning;
using namespace NSIspTuningv3;

#include <mtkcam/utils/std/common.h>
//#include <mtkcam/utils/hw/HwInfoHelper.h>
using namespace NSCam::Utils;

#include <mtkcam/aaa/IHal3A.h>
using namespace NS3Av3;

#include "lcs_drv_imp.h"

/*******************************************************************************
*
********************************************************************************/

#undef __func__
#define __func__ __FUNCTION__


#define LCS_LOG(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define LCS_INF(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define LCS_WRN(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define LCS_ERR(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGD_IF(cond, ...)       do { if ( (cond) >= (2) ) { LCS_LOG(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) >= (1) ) { LCS_INF(__VA_ARGS__); } }while(0)
//#define MY_LOGW_IF(cond, ...)       do { if ( (cond) >= (1) ) { LCS_WRN(__VA_ARGS__); } }while(0)
//#define MY_LOGE_IF(cond, ...)       do { if ( (cond) >= (0) ) { LCS_ERR(__VA_ARGS__); } }while(0)

#define LCS_IN_INFO_QUEUE_SIZE (15)

#define LCS_DRV_NAME "LCSDrv"
#define NAME "LCSDrv"
/*******************************************************************************
*
********************************************************************************/
static MINT32 g_debugDump = 0;

/*******************************************************************************
*
********************************************************************************/
LcsDrv *LcsDrv::CreateInstance(const MUINT32 &aSensorIdx)
{
    return LcsDrvImp::GetInstance(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
LcsDrv *LcsDrvImp::GetInstance(const MUINT32 &aSensorIdx)
{
    LCS_LOG("aSensorIdx(%u)",aSensorIdx);
    return new LcsDrvImp(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
MVOID LcsDrvImp::DestroyInstance()
{
    LCS_LOG("+");
	delete this;
}

/*******************************************************************************
*
********************************************************************************/
LcsDrvImp::LcsDrvImp(const MUINT32 &aSensorIdx)
   :LcsDrv()
   ,m_u4Capacity(LCS_IN_INFO_QUEUE_SIZE)
   ,m_Lock_In()
   ,m_Lock_Out()
{
    mUsers = 0;

    mpNormalPipeModule = NULL;
    m_pNormalPipe = NULL;
    mSensorIdx = aSensorIdx;

    mSensorDev = 0;
    m_tuningSensorDevType = ESensorDev_Main;
    mSensorTg  = CAM_TG_NONE;

    mpLcsP1Cb = NULL;

    m_p3aHal = NULL;
}

#if 1

/*******************************************************************************
*
********************************************************************************/
MINT32 LcsDrvImp::Init()
{
    Mutex::Autolock lock(mLock);

    //====== Reference Count ======

    if(mUsers > 0)
    {
        MBOOL ret = android_atomic_inc(&mUsers);
        LCS_INF("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
        return LCS_RETURN_NO_ERROR;
    }

    MINT32 err = LCS_RETURN_NO_ERROR;

    //====== Dynamic Debug ======

    g_debugDump = ::property_get_int32("vendor.debug.lcs.dump", 0);

    //====== Create INormalPipe Object ======

    mpNormalPipeModule = INormalPipeModule::get();
    if  ( ! mpNormalPipeModule ) {
        LCS_ERR("INormalPipeModule::get() fail");
        err = LCS_RETURN_NULL_OBJ;
        return err;
    }

    {
        int status = 0;

        //  Select CamIO version
        size_t count = 0;
        MUINT32 const* version = NULL;
        status = mpNormalPipeModule->get_sub_module_api_version(&version, &count, mSensorIdx);
        if  ( status < 0 || ! count || ! version ) {
            LCS_ERR(
                "[%d] INormalPipeModule::get_sub_module_api_version - err:%#x count:%zu version:%p",
                mSensorIdx, status, count, version
            );
            return LCS_RETURN_NULL_OBJ;
        }

        MUINT32 selectedVersion = *(version + count - 1); //Select max. version

        //  Create CamIO
        status = mpNormalPipeModule->createSubModule(
            mSensorIdx, LCS_DRV_NAME, selectedVersion, (MVOID**)&m_pNormalPipe);
        if  ( ! m_pNormalPipe ) {
            LCS_ERR("create INormalPipe fail");
            return LCS_RETURN_NULL_OBJ;
        }
    }

    //====== Create Pass1 Callback Class ======

    mpLcsP1Cb = new LcsP1Cb(this);

    MINT32 inc_temp = android_atomic_inc(&mUsers);    // increase reference count

    MY_LOGD_IF(g_debugDump, "-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 LcsDrvImp::Uninit()
{
    Mutex::Autolock lock(mLock);

    //====== Reference Count ======

    if(mUsers <= 0) // No more users
    {
        MY_LOGD_IF(g_debugDump, "mSensorIdx(%u) has 0 user",mSensorIdx);
        return LCS_RETURN_NO_ERROR;
    }

    // >= one user
    MINT32 dec_temp = android_atomic_dec(&mUsers);

    MINT32 err = LCS_RETURN_NO_ERROR;

    if(mUsers == 0)
    {
        LCS_INF("mSensorIdx(%u) uninit",mSensorIdx);

        //====== Destory INormalPipe ======

        if(m_pNormalPipe != NULL)
        {
            m_pNormalPipe->destroyInstance(LCS_DRV_NAME);
            m_pNormalPipe = NULL;
        }

        if(m_p3aHal != NULL)
        {
            m_p3aHal->destroyInstance(LCS_DRV_NAME);
            m_p3aHal = NULL;
        }

        //====== Rest Member Variable ======

        mUsers = 0;
        mSensorIdx = 0;

        mSensorDev = 0;
        m_tuningSensorDevType = ESensorDev_Main;
        mSensorTg  = CAM_TG_NONE;

        // delete pass1 callback class
        if(mpLcsP1Cb != NULL)
        {
            delete mpLcsP1Cb;
            mpLcsP1Cb = NULL;
        }
    }
    else
    {
         LCS_INF("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
    }

    MY_LOGD_IF(g_debugDump, "X");

    clearList_In();

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MVOID LcsDrvImp::SetSensorInfo(const MUINT32 &aSensorDev,const MUINT32 &aSensorTg)
{
    mSensorDev = aSensorDev;
    m_tuningSensorDevType = ConvertSensorDevType(mSensorDev);
    mSensorTg  = aSensorTg;
    LCS_LOG( "(dev,tg)=(%u,%u)",mSensorDev,mSensorTg);
}

/*******************************************************************************
*
********************************************************************************/
MVOID LcsDrvImp::GetLCSInputDimen(MUINT32& w, MUINT32& h)
{

    MUINT32 tgW = 0;
    MUINT32 tgH = 0;
    MUINT32 binW = 0;
    MUINT32 binH = 0;

    m_pNormalPipe->sendCommand(ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)(&tgW),(MINTPTR)(&tgH), 0);
    m_pNormalPipe->sendCommand(ENPipeCmd_GET_HBIN_INFO, (MINTPTR)(&binW), (MINTPTR)(&binH), 0);

    if( binW < tgW)
        w = binW;
    else
        w = tgW;

    if( binH < tgH)
        h = binH;
    else
        h = tgH;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 LcsDrvImp::ConfigLcs()
{
    MY_LOGD_IF(g_debugDump, "(dev,tg)=(%u,%u)",mSensorDev,mSensorTg);

    ESensorDev_T tuningSensorDevType = ConvertSensorDevType(mSensorDev);
    ESensorTG_T tuningSensorTgType   = ConvertSensorTgType(mSensorTg);

    MUINT32 lcsInWidth = 0;
    MUINT32 lcsInHeight = 0;
    GetLCSInputDimen(lcsInWidth, lcsInHeight);

    // ======== register P1 Tuning Notify callback ==========
    // because this command need to be after MW configPipe, and don't want to add on more API
    // so just put it here

    m_pNormalPipe->sendCommand(ENPipeCmd_SET_LCS_CBFP,(MINTPTR)mpLcsP1Cb,-1,-1);

    m_p3aHal = MAKE_Hal3A(static_cast<MINT32>(mSensorIdx),LCS_DRV_NAME);
    if(m_p3aHal == NULL)
    {
        LCS_ERR("m_p3aHal create fail");
        return LCS_RETURN_NULL_OBJ;
    }

    MY_LOGD_IF(g_debugDump, "-");
    return LCS_RETURN_NO_ERROR;
}

/******************************************************************************
*
*******************************************************************************/
ESensorDev_T LcsDrvImp::ConvertSensorDevType(const MUINT32 &aSensorDev)
{
    return (ESensorDev_T)aSensorDev;
}

/******************************************************************************
*
*******************************************************************************/
ESensorTG_T LcsDrvImp::ConvertSensorTgType(const MUINT32 &aSensorTg)
{
    switch(aSensorTg)
    {
        case CAM_TG_1: return ESensorTG_1;
        case CAM_TG_2: return ESensorTG_2;
        default : MY_LOGE_IF(g_debugDump, "wrong sensorTg(%d), return ESensorTG_1",aSensorTg);
                  return ESensorTG_1;
    }
}

MVOID
LcsDrvImp::
clearList_In()
{
    Mutex::Autolock lock(m_Lock_In);
    CAM_LOGD("[%s] Size(%zu)", __FUNCTION__, m_rLCS_IN_info_Queue.size());
    m_rLCS_IN_info_Queue.clear();
}

MINT32
LcsDrvImp::
updateLCSList_In(MINT32 i4FrmId, const ISP_LCS_IN_INFO_T& rLCSinfo)
{
    Mutex::Autolock lock(m_Lock_In);

    MINT32 i4Ret = -1;
    MINT32 i4Pos = 0;
    MINT32 i4Size = m_rLCS_IN_info_Queue.size();
    LCS_IN_info_List_T::iterator it = m_rLCS_IN_info_Queue.begin();
#if 0
    for (; it != m_rResultQueue.end(); it++)
    {
        CAM_LOGD("[%s] #(%d)", __FUNCTION__, it->i4FrmId);
    }
#endif
    for (it = m_rLCS_IN_info_Queue.begin(); it != m_rLCS_IN_info_Queue.end(); it++, i4Pos++)
    {
        if ((MINT32)it->i4FrmId == i4FrmId)
        {
            CAM_LOGW("overwirte LCSList_In FrmID: %d", i4FrmId);
            *it = rLCSinfo;
            i4Ret = 1;
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        m_rLCS_IN_info_Queue.push_back(rLCSinfo);
        i4Ret = 0;
    }

    // remove item
    if (m_rLCS_IN_info_Queue.size() > m_u4Capacity)
    {
        m_rLCS_IN_info_Queue.erase(m_rLCS_IN_info_Queue.begin());
    }

    return i4Ret;
}

MINT32
LcsDrvImp::
getLCSList_info_In(MINT32 i4FrmId, ISP_LCS_IN_INFO_T& rLCSinfo)
{
    Mutex::Autolock lock(m_Lock_In);

    MINT32 i4Ret = 0;
    MINT32 i4Pos = 0;
    MINT32 i4Size = m_rLCS_IN_info_Queue.size();
    LCS_IN_info_List_T::iterator it = m_rLCS_IN_info_Queue.begin();
    for (; it != m_rLCS_IN_info_Queue.end(); it++, i4Pos++)
    {
        if ((MINT32)it->i4FrmId == i4FrmId)
        {
            rLCSinfo = *it;
            //CAM_LOGD("[%s] OK i4Pos(%d)", __FUNCTION__, i4Pos);
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        // does not exist
        CAM_LOGD("[%s] NG i4Pos(%d)", __FUNCTION__, i4Pos);
        i4Ret = -1;
    }

    return i4Ret;
}

MINT32
LcsDrvImp::
getLCSListLast_In(ISP_LCS_IN_INFO_T& rLCSinfo)
{
    Mutex::Autolock lock(m_Lock_In);

    if (!m_rLCS_IN_info_Queue.empty())
    {
        LCS_IN_info_List_T::iterator it = m_rLCS_IN_info_Queue.end();
        it--;
        rLCSinfo = *it;
        return 0;
    }

    return -1;
}

ISP_LCS_IN_INFO_T*
LcsDrvImp::getLCSList_info_In(MINT32 i4FrmId)
{
    Mutex::Autolock lock(m_Lock_In);

    ISP_LCS_IN_INFO_T* pBuf = NULL;
    MINT32 i4Pos = 0;
    MINT32 i4Size = m_rLCS_IN_info_Queue.size();
    LCS_IN_info_List_T::iterator it = m_rLCS_IN_info_Queue.begin();
    for (; it != m_rLCS_IN_info_Queue.end(); it++, i4Pos++)
    {
        if ((MINT32)it->i4FrmId == i4FrmId)
        {
            pBuf = &(*it);
            //CAM_LOGD("[%s] OK i4Pos(%d)", __FUNCTION__, i4Pos);
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        // does not exist
        CAM_LOGD("[%s] Ref NG i4Pos(%d)", __FUNCTION__, i4Pos);
    }

    return pBuf;
}

ESensorDev_T
LcsDrvImp::
getTuningSensorDevType()
{
    return m_tuningSensorDevType;
}


/*******************************************************************************
*
********************************************************************************/
LcsP1Cb::LcsP1Cb(MVOID *arg)
{
    m_pClassObj = arg;
}

/*******************************************************************************
*
********************************************************************************/
LcsP1Cb::~LcsP1Cb()
{
    m_pClassObj = NULL;
}

/*******************************************************************************
*
********************************************************************************/
void LcsP1Cb::p1TuningNotify(MVOID *pInput,MVOID *pOutput)
{

    MY_LOGD_IF(g_debugDump, "[ + ]");
    LcsDrvImp *_this = reinterpret_cast<LcsDrvImp *>(m_pClassObj);

    LCS_REG_CFG *apLcsCfgData   = (LCS_REG_CFG *)pOutput;
    LCS_INPUT_INFO *apLcsInInfo = (LCS_INPUT_INFO *)pInput;  // Crop Use
    LCS_REG_CFG     lcs_cfg; // output

    lcs_cfg.u4LCSO_Stride = (ISP_LCS_OUT_WD * 2); // 2 byte depth

    // ==== For no Crop situation, Find LCS input info ==========

    MUINT32 lcsInWidth = apLcsInInfo->sHBINOut.w;
    MUINT32 lcsInHeight = apLcsInInfo->sHBINOut.h;

    //MUINT32 lcsInWidth  = _this->mLcsAws & 0x1FFF;
    //MUINT32 lcsInHeight = (_this->mLcsAws & 0x1FFF0000) >> 16;

    MUINT32 lcsOutWidth = 32;
    MUINT32 lcsOutHeight = 32;

    MUINT32 TGOutW = apLcsInInfo->sTGOut.w;
    MUINT32 TGOutH = apLcsInInfo->sTGOut.h;

    //PD sensor
    if(apLcsInInfo->bIsDbin){
        TGOutW = TGOutW/2 ;
    }

    lcsOutWidth = (TGOutW / 10);
    lcsOutHeight = (TGOutH / 10);


    //HW constraint
    if((lcsInWidth/lcsOutWidth) < 4){

        lcsOutWidth = lcsInWidth >> 2;

        float ratio = (float)lcsOutWidth / (float) TGOutW;
        lcsOutHeight = (float)TGOutH * ratio;
        }

    //HW constraint 2
    if(lcsOutWidth > ISP_LCS_OUT_WD){

        lcsOutWidth = ISP_LCS_OUT_WD;
        float ratio2 = (float)lcsOutWidth / (float) TGOutW;
        lcsOutHeight = (float)TGOutH * ratio2;
    }

    if (lcsOutHeight > ISP_LCS_OUT_HT){

        lcsOutHeight = ISP_LCS_OUT_HT;
        float ratio3 = (float)lcsOutHeight / (float) TGOutH;
        lcsOutWidth = (float)TGOutW * ratio3;
    }

    //align 2
    if(lcsOutWidth%2) lcsOutWidth--;
    if(lcsOutHeight%2) lcsOutHeight--;


    if((lcsOutWidth<32) || (lcsOutHeight<32)){

            lcs_cfg.bLCS_EN = MFALSE;
            lcs_cfg.bLCS_Bypass = MTRUE;
        }
        else{
        lcs_cfg.bLCS_EN = MTRUE;
        lcs_cfg.bLCS_Bypass = MFALSE;
    }

    //lcsOutWidth = _this->mLcs_outW;
    //lcsOutHeight = _this->mLcs_outH;

    MUINT32 lcsCropRect_x = 0;
    MUINT32 lcsCropRect_y = 0;
    MUINT32 lcsCropRect_w = lcsInWidth;
    MUINT32 lcsCropRect_h = lcsInHeight;

    //NSCamHW::Rect lcsCropRect(0, 0, lcsInWidth, lcsInHeight);

    // ============================


    MUINT32         lrzr_x, lrzr_y;

    lrzr_x = ((lcsOutWidth-1)*1048576)/(lcsCropRect_w>>1);
    lrzr_y = ((lcsOutHeight-1)*1048576)/(lcsCropRect_h>>1);

    //MY_LOGD_IF(g_debugDump, "LCS info: in_%d_%d out_%d_%d lrzr_%d_%d", lcs_in.w, lcs_in.h, lcs_out.w, lcs_out.h,
    //        lrzr_x, lrzr_y);

    //MY_LOGD_IF(g_debugDump, "TG(%d),bIsHbin(%d), binInSize(%u,%u)",_this->mSensorTg,apLcsInInfo->bIsHbin, apLcsInInfo->sHBINOut.w, apLcsInInfo->sHBINOut.h);
    MY_LOGI_IF(g_debugDump, "bIsHbin(%d), binInSize(%u,%u), RRZ_crop(%u,%u,%u,%u) RRZ_In(%u,%u) RRZ_Out(%u,%u)",
                apLcsInInfo->bIsHbin, apLcsInInfo->sHBINOut.w, apLcsInInfo->sHBINOut.h,
                apLcsInInfo->RRZ_IN_CROP.start_x, apLcsInInfo->RRZ_IN_CROP.start_y,
                apLcsInInfo->RRZ_IN_CROP.crop_size_w, apLcsInInfo->RRZ_IN_CROP.crop_size_h,
                apLcsInInfo->RRZ_IN_CROP.in_size_w, apLcsInInfo->RRZ_IN_CROP.in_size_h,
                apLcsInInfo->sRRZOut.w, apLcsInInfo->sRRZOut.h);

    lcs_cfg._LCS_REG.LCS_CON = (((lcsOutHeight & 0x1FF)<<20) | ((lcsOutWidth & 0x1FF)<<8)) | 0x01;   // 1 size
    lcs_cfg._LCS_REG.LCS_ST  = 0 | lcsCropRect_x | (lcsCropRect_y << 16);
    lcs_cfg._LCS_REG.LCS_AWS = (((lcsInHeight&0x1FFF)<<16) | (lcsInWidth&0x1FFF));
    lcs_cfg._LCS_REG.LCS_LRZR_1 = (lrzr_x & 0xFFFFF);
    lcs_cfg._LCS_REG.LCS_LRZR_2 = (lrzr_y & 0xFFFFF);

    ISP_LCS_IN_INFO_T tempLCS_IN_info;

    if( _this->getLCSList_info_In(apLcsInInfo->magic, tempLCS_IN_info) == (-1) ){
        //CAM_LOGE("[-Fail to get LCS tuning ] FrmId(%d)", apLcsInInfo->magic);

        if( _this->getLCSListLast_In(tempLCS_IN_info) == (-1)){
             CAM_LOGE("[-Fail to get LCS Queue ]");
        }
    }


    ISP_NVRAM_LCS25_CON_T con;
    con.val = lcs_cfg._LCS_REG.LCS_CON;

    ISP_NVRAM_LCS25_ST_T st;
    st.val = lcs_cfg._LCS_REG.LCS_ST;

    ISP_NVRAM_LCS25_AWS_T aws;
    aws.val = lcs_cfg._LCS_REG.LCS_AWS;

    ISP_NVRAM_LCS25_LRZR_1_T lrzr_1;
    lrzr_1.val = lcs_cfg._LCS_REG.LCS_LRZR_1;

    ISP_NVRAM_LCS25_LRZR_2_T lrzr_2;
    lrzr_2.val = lcs_cfg._LCS_REG.LCS_LRZR_2;

#if 0
    MY_LOGE("LcsIn(%d, %d), LcsOut(%d,%d)\n",
                lcsInWidth, lcsInHeight, lcsOutWidth, lcsOutHeight);

    MY_LOGE("CON: LOG(%d), OUT_WD(%d, OUT_HT(%d)\n",
                con.bits.LCS25_LOG,
                con.bits.LCS25_OUT_WD,
                con.bits.LCS25_OUT_HT);

    MY_LOGE("ST:  START_J(%d), START_I(%d)\n",
                st.bits.LCS25_START_J,
                st.bits.LCS25_START_I);

    MY_LOGE("AWS: LCS25_IN_WD(%d), LCS25_IN_HT(%d)\n",
                aws.bits.LCS25_IN_WD,
                aws.bits.LCS25_IN_HT);

    MY_LOGE("LRZR_1: LCS25_LRZR_X(%d)\n",
                lrzr_1.bits.LCS25_LRZR_X);

    MY_LOGE("LRZR_2: LCS25_LRZR_Y(%d),\n",
                lrzr_2.bits.LCS25_LRZR_Y);
#endif

    lcs_cfg._LCS_REG.LCS_FLR       = tempLCS_IN_info.lcs.flr.val;
    lcs_cfg._LCS_REG.LCS_SATU1     = tempLCS_IN_info.lcs.satu_1.val;
    lcs_cfg._LCS_REG.LCS_SATU2     = tempLCS_IN_info.lcs.satu_2.val;
    lcs_cfg._LCS_REG.LCS_GAIN_1    = tempLCS_IN_info.lcs.gain_1.val;
    lcs_cfg._LCS_REG.LCS_GAIN_2    = tempLCS_IN_info.lcs.gain_2.val;
    lcs_cfg._LCS_REG.LCS_OFST_1    = tempLCS_IN_info.lcs.ofst_1.val;
    lcs_cfg._LCS_REG.LCS_OFST_2    = tempLCS_IN_info.lcs.ofst_2.val;
    lcs_cfg._LCS_REG.LCS_G2G_CNV_1 = tempLCS_IN_info.lcs.g2g_cnv_1.val;
    lcs_cfg._LCS_REG.LCS_G2G_CNV_2 = tempLCS_IN_info.lcs.g2g_cnv_2.val;
    lcs_cfg._LCS_REG.LCS_G2G_CNV_3 = tempLCS_IN_info.lcs.g2g_cnv_3.val;
    lcs_cfg._LCS_REG.LCS_G2G_CNV_4 = tempLCS_IN_info.lcs.g2g_cnv_4.val;
    lcs_cfg._LCS_REG.LCS_G2G_CNV_5 = tempLCS_IN_info.lcs.g2g_cnv_5.val;

    lcs_cfg._LCS_REG.LCS_LPF       = 0x012C0001;

    MY_LOGD_IF(g_debugDump, "LcsOut(%u,%u), CON(0x%08x), ST(0x%08x), AWS(0x%08x), FLR(0x%08x), LRZR1(0x%08x), LRZR2(0x%08x)", lcsOutWidth, lcsOutHeight,
                lcs_cfg._LCS_REG.LCS_CON, lcs_cfg._LCS_REG.LCS_ST, lcs_cfg._LCS_REG.LCS_AWS,
                lcs_cfg._LCS_REG.LCS_FLR, lcs_cfg._LCS_REG.LCS_LRZR_1, lcs_cfg._LCS_REG.LCS_LRZR_2);

    *(LCS_REG_CFG*)pOutput = lcs_cfg;

     ISP_LCS_OUT_INFO_T tempLCS_OUT_info;

     tempLCS_OUT_info.i4FrmId       = apLcsInInfo->magic;
     tempLCS_OUT_info.u4InWidth     = lcsInWidth;
     tempLCS_OUT_info.u4InHeight    = lcsInHeight;
     tempLCS_OUT_info.u4OutWidth    = lcsOutWidth;
     tempLCS_OUT_info.u4OutHeight   = lcsOutHeight;
     tempLCS_OUT_info.u4CropX       = lcsCropRect_x;
     tempLCS_OUT_info.u4CropY       = lcsCropRect_y;
     tempLCS_OUT_info.u4Lrzr1       = lrzr_x;
     tempLCS_OUT_info.u4Lrzr2       = lrzr_y;
     tempLCS_OUT_info.bFrontBin     = apLcsInInfo->bIsbin;
     tempLCS_OUT_info.u4Qbin_Rto    = apLcsInInfo->bQbinRatio;



     _this->m_p3aHal->send3ACtrl(E3ACtrl_SetLcsoParam,(MINTPTR)&tempLCS_OUT_info,NULL);

    MY_LOGD_IF(g_debugDump, "[ - ]");
}



#endif

