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
#define LOG_TAG "MtkCam/STShotCtrl"
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
//
#include <mtkcam/featureio/stereo_setting_provider.h>
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")
//
//
#include <mtkcam/common.h>
using namespace NSCam;
//
#include <mtkcam/camnode/IspSyncControl.h>
#include "../inc/StereoShotCtrlNode.h"
using namespace NSCamNode;
//
#include <semaphore.h>
using namespace std;
//
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/featureio/IHal3A.h>
using namespace NS3A;
//
#include <mtkcam/hal/IHalSensor.h>
//
#include <mtkcam/exif/IDbgInfoContainer.h>
//
#include <utils/Mutex.h>
using namespace android;

#define USE_3A

#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}

#define DNG_META_CB 1

/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
//namespace NSCamStereo {
////////////////////////////////////////////////////////////////////////////////

#define MODULE_NAME        "SShotCtrl"

class SCtrlNodeImpl : public StereoShotCtrlNode
{
    public:     /*createInstance()*/
               SCtrlNodeImpl(ShotParam const & rShotParam);
               ~SCtrlNodeImpl();

        virtual MBOOL   setIspProfile(NS3A::EIspProfile_T const profile)
        {
            return MFALSE;
        }

//////////////////////////////

        virtual MVOID   setCallbacks(
                         NodeNotifyCallback_t notify_cb,
                         NodeDataCallback_t data_cb,
                         MVOID* user
                         );

        MVOID   enableNotifyMsg(MUINT32 const msg);
        MVOID   enableDataMsg(MUINT32 const msg);
        MVOID   disableNofityMsg(MUINT32 const msg);
        MVOID   disableDataMsg(MUINT32 const msg);

        MVOID   handleNotifyCallback(
                 MUINT32 const msg,
                 MUINT32 const ext1,
                 MUINT32 const ext2) const;

        MVOID   handleDataCallback(
                 MUINT32 const msg,
                 MUINTPTR const ext1,
                 MUINTPTR const ext2,
                 IImageBuffer* const pImgBuf) const;

    protected:

        MBOOL   isNotifyMsgEnabled(MUINT32 const msg) const;
        MBOOL   isDataMsgEnabled(MUINT32 const msg) const;

///////////////////////////////
    private:

        mutable Mutex                          mLock;
        MVOID*                                 mpUser;
        MUINT32                                mNotifyMsgEnabled;
        MUINT32                                mDataMsgEnabled;
        NodeNotifyCallback_t                   mNotifyCb;
        NodeDataCallback_t                     mDataCb;

//////////////////////////////

    protected:

        virtual MBOOL onInit();
        virtual MBOOL onUninit();
        virtual MBOOL onStart();
        virtual MBOOL onStop();
        virtual MBOOL onNotify(
                        MUINT32 const msg,
                        MUINT32 const ext1,
                        MUINT32 const ext2
                        ) = 0;
        virtual MBOOL onPostBuffer(
                        MUINT32 const data,
                        MUINTPTR const buf,
                        MUINT32 const ext
                        ) = 0;
        virtual MBOOL onReturnBuffer(
                        MUINT32 const data,
                        MUINTPTR const buf,
                        MUINT32 const ext
                        );
        virtual MVOID onDumpBuffer(
                        const char*   usr,
                        MUINT32 const data,
                        MUINTPTR const buf,
                        MUINT32 const ext
                        );

    protected:

        virtual MBOOL _init() { return MTRUE; }
        virtual MBOOL _uninit() { return MTRUE; }

        MUINT32 getSensorDevIdx() const { return mSensorDevIdx; }

    protected:

        IHal3A*                 mpHal3A;
        IspSyncControl*         mpIspSyncCtrl;

        EIspProfile_T           mIspProfile;

        MUINT32                 mSensorDevIdx;

        DNGInfo_T               mbDngInfo;
        IDbgInfoContainer*      mbDbgInfo_cap;
        IDbgInfoContainer*      mbDbgInfo_prv;

        // for Sensor 0 and 1 image buffer sync
        static MBOOL            mbSyncOk;
        static MBOOL            mbMain1Done;
        static MBOOL            mbMain2Done;

};

MBOOL SCtrlNodeImpl::mbSyncOk = MFALSE;
MBOOL SCtrlNodeImpl::mbMain1Done = MFALSE;
MBOOL SCtrlNodeImpl::mbMain2Done = MFALSE;

class NormalCtrl : public SCtrlNodeImpl, public I3ACallBack
{
    public:

        NormalCtrl(ShotParam const & rShotParam)
            : SCtrlNodeImpl(rShotParam)
            , mCurMagicNum(-1)
            , mbDoCap(MFALSE)
        {};
        ~NormalCtrl() {};

        //I3ACallBack
        virtual void doNotifyCb ( int32_t _msgType, int32_t _ext1, int32_t _ext2, int32_t _ext3, MINTPTR _ext4 = 0);

        virtual void doDataCb ( int32_t _msgType, void*   _data, uint32_t _size);

        virtual MBOOL setIspProfile(NS3A::EIspProfile_T const profile);

    protected:

        virtual MBOOL _init();
        virtual MBOOL _uninit();
        virtual MBOOL onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2);
        virtual MBOOL onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext);

    protected:
        // 3A related
        MINT32                mCurMagicNum;     //lastest updated magic num
        MBOOL                 mbDoCap;
};


class ZsdVssCtrl : public SCtrlNodeImpl
{
    public:

        ZsdVssCtrl(ShotParam const & rShotParam, MUINT32 const style)
            : SCtrlNodeImpl(rShotParam)
            , muStyle(style)
        {};
        ~ZsdVssCtrl() {};

    protected:

        virtual MBOOL onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2);
        virtual MBOOL onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext);

    private:

        const MUINT32       muStyle;
};


/*******************************************************************************
*
********************************************************************************/
StereoShotCtrlNode*
StereoShotCtrlNode::
createInstance(ShotParam const & rShotParam, MUINT32 const style, MBOOL const withP1)
{
    if( !withP1 )
        return new ZsdVssCtrl(rShotParam, style);

    return new NormalCtrl(rShotParam);
}


/*******************************************************************************
*
********************************************************************************/
void
StereoShotCtrlNode::
destroyInstance()
{
    //delete this;
}


/*******************************************************************************
*
********************************************************************************/
StereoShotCtrlNode::
StereoShotCtrlNode(ShotParam const & rShotParam)
    : ICamNode(MODULE_NAME)
    , mShotParam(rShotParam)
    , mSensorId_Main(0)
    , mSensorId_Main2(0)
{
}


/*******************************************************************************
*
********************************************************************************/
StereoShotCtrlNode::
~StereoShotCtrlNode()
{
}


/*******************************************************************************
*
********************************************************************************/
SCtrlNodeImpl::
SCtrlNodeImpl(ShotParam const & rShotParam)
    : StereoShotCtrlNode(rShotParam)
    , mpHal3A(NULL)
    , mpIspSyncCtrl(NULL)
    , mIspProfile(EIspProfile_Capture)
    , mSensorDevIdx(0)
    , mNotifyMsgEnabled(0x0)
    , mDataMsgEnabled(0x0)
    , mNotifyCb(NULL)
    , mDataCb(NULL)
    , mpUser(NULL)
    , mbDbgInfo_cap(NULL)
    , mbDbgInfo_prv(NULL)
{
    addDataSupport( ENDPOINT_SRC, CONTROL_FULLRAW );
    addDataSupport( ENDPOINT_SRC, CONTROL_RESIZEDRAW );

    addDataSupport( ENDPOINT_DST, CONTROL_PRV_SRC );
    addDataSupport( ENDPOINT_DST, CONTROL_CAP_SRC );
    addDataSupport( ENDPOINT_DST, CONTROL_DBGINFO );

    addNotifySupport( CONTROL_STOP_PASS1 | CONTROL_SHUTTER );
}


/*******************************************************************************
*
********************************************************************************/
SCtrlNodeImpl::
~SCtrlNodeImpl()
{
}

/*******************************************************************************
*
********************************************************************************/
MVOID
SCtrlNodeImpl::
setCallbacks(NodeNotifyCallback_t notify_cb, NodeDataCallback_t data_cb, MVOID* user)
{
    Mutex::Autolock _l(mLock);
    mpUser    = user;
    mNotifyCb = notify_cb;
    mDataCb   = data_cb;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
SCtrlNodeImpl::
enableNotifyMsg(MUINT32 const msg)
{
    Mutex::Autolock _l(mLock);
    mNotifyMsgEnabled |= msg;
    MY_LOGD("enabled notify(0x%x)", msg);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
SCtrlNodeImpl::
enableDataMsg(MUINT32 const msg)
{
    Mutex::Autolock _l(mLock);
    mDataMsgEnabled |= msg;
    MY_LOGD("enabled data(0x%x)", msg);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
SCtrlNodeImpl::
disableNofityMsg(MUINT32 const msg)
{
    Mutex::Autolock _l(mLock);
    mNotifyMsgEnabled &= ~msg;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
SCtrlNodeImpl::
disableDataMsg(MUINT32 const msg)
{
    Mutex::Autolock _l(mLock);
    mDataMsgEnabled &= ~msg;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SCtrlNodeImpl::
isNotifyMsgEnabled(MUINT32 const msg) const
{
    MBOOL ret;
    Mutex::Autolock _l(mLock);
    ret = mNotifyMsgEnabled & msg;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SCtrlNodeImpl::
isDataMsgEnabled(MUINT32 const msg) const
{
    MBOOL ret;
    Mutex::Autolock _l(mLock);
    ret = mDataMsgEnabled & msg;
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
SCtrlNodeImpl::
handleDataCallback(MUINT32 const msg, MUINTPTR const ext1, MUINTPTR const ext2, IImageBuffer* const pImgBuf) const
{
    MY_LOGD("handleDataCallback:%d ",msg );
    if( mDataCb == NULL )
    {
        MY_LOGE("dataCallback is not set");
        return;
    }

    if( isDataMsgEnabled(msg) )//ENode_DATA_MSG_RAW
    {
        MY_LOGD("isDataMsgEnabled");
        NodeDataInfo datainfo(msg, ext1, ext2, pImgBuf);//ENode_DATA_MSG_META , METADATA STRUCT, 0, 0
        mDataCb(mpUser, datainfo);
    }

}

/*******************************************************************************
*
********************************************************************************/
MBOOL
SCtrlNodeImpl::
onInit()
{
    MBOOL ret = MFALSE;
    // Get sensor id
        String8 const s8MainIdKey("MTK_SENSOR_DEV_MAIN");
        String8 const s8Main2IdKey("MTK_SENSOR_DEV_MAIN_2");
        Utils::Property::tryGet(s8MainIdKey, mSensorId_Main);
        Utils::Property::tryGet(s8Main2IdKey, mSensorId_Main2);
    //
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    mSensorDevIdx   = pHalSensorList->querySensorDevIdx(getSensorIdx());
    mbSyncOk = MFALSE;
    mbMain1Done = MFALSE;
    mbMain2Done = MFALSE;
    //
    mpIspSyncCtrl = IspSyncControl::createInstance(getSensorIdx());
    if( mpIspSyncCtrl == NULL )
    {
        MY_LOGE("IspSyncCtrl:createInstance fail");
        goto lbExit;
    }
    //
    mpIspSyncCtrl->setPreviewSize(mShotParam.u4PostViewWidth, mShotParam.u4PostViewHeight);
    mpIspSyncCtrl->setCurZoomRatio(mShotParam.u4ZoomRatio);
    //
#ifdef USE_3A
    mpHal3A = IHal3A::createInstance( IHal3A::E_Camera_1, getSensorIdx(), getName() );
    if(mpHal3A == NULL)
    {
        MY_LOGE("IHal3A:createInstance fail");
        goto lbExit;
    }
#endif
    //
    ret = _init();
lbExit:
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SCtrlNodeImpl::
onUninit()
{
    if( !_uninit() )
    {
        MY_LOGE("_uninit failed");
    }
    //
    if(mpHal3A)
    {
        mpHal3A->destroyInstance(getName());
        mpHal3A = NULL;
    }
    //
    if(mpIspSyncCtrl)
    {
        mpIspSyncCtrl->destroyInstance();
        mpIspSyncCtrl = NULL;
    }
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SCtrlNodeImpl::
onStart()
{
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SCtrlNodeImpl::
onStop()
{
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalCtrl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    MY_LOGD("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    MUINT32       dstdata = 0;
    // for Sensor 0 and 1 image buffer sync
    // if ( getSensorIdx()== 0 && !mbSyncOk){
    //     handleReturnBuffer(data, buf);
    //     goto lbExit;
    // }

    if( !mbDoCap ) {
        handleReturnBuffer(data, buf);
        goto lbExit;
    }
    //
    switch(data)
    {
        case CONTROL_FULLRAW:
            dstdata     = CONTROL_CAP_SRC;
            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: [CONTROL_FULLRAW]buf(%p), ext(0x%x)", buf, ext);
            break;
        case CONTROL_RESIZEDRAW:
            dstdata     = CONTROL_PRV_SRC;
            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: [CONTROL_RESIZEDRAW]buf(%p), ext(0x%x)", buf, ext);
            break;
        default:
            MY_LOGE("not support yet: %d", data);
            break;
    }
    //
    if( dstdata != 0 ) {

        if(getSensorIdx()== 0){
            mbMain1Done = MTRUE;
        }else{
            mbMain2Done = MTRUE;
        }

        if(mbMain1Done && mbMain2Done){
            handleNotify(CONTROL_SHUTTER);
        }

        handlePostBuffer(dstdata, buf);
    }
    //
lbExit:
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SCtrlNodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    MY_LOGV("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    switch(data)
    {
        case CONTROL_PRV_SRC:
            handleReturnBuffer(CONTROL_RESIZEDRAW, buf);
            break;
// #if DNG_META_CB
#if 0
        // case CONTROL_CAP_SRC:
        //     if(mSensorId_Main == getSensorIdx())  // main 1 need to pass to cb to process raw.
        //     {
        //         // for DNG raw
        //         MY_LOGD("CKH: DNG Raw post buffer");
        //         handlePostBuffer(CONTROL_STEREO_RAW_DST, buf);
        //     }
        //     else // if this buff's owner is main2, pass it directlly.
        //     {
        //         handleReturnBuffer(CONTROL_FULLRAW, buf);
        //     }
        //     break;
        // case CONTROL_STEREO_RAW_DST:
        //         handleReturnBuffer(CONTROL_FULLRAW, buf);
        //     break;
#else
        case CONTROL_CAP_SRC:
            handleReturnBuffer(CONTROL_FULLRAW, buf);
            break;
#endif
        case CONTROL_DBGINFO:
            if( buf != 0 )
            {
                IDbgInfoContainer* pDbgInfo = reinterpret_cast<IDbgInfoContainer*>(buf);
                pDbgInfo->destroyInstance();
            }
            return MTRUE;
            break;
        default:
            MY_LOGE("not support yet: %d", data);
            break;
    }

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
SCtrlNodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    FUNC_END;
}


/*******************************************************************************
*
********************************************************************************/
void
NormalCtrl::
doNotifyCb( int32_t _msgType, int32_t _ext1, int32_t _ext2, int32_t _ext3, MINTPTR _ext4)
{
    //
    if(_msgType == I3ACallBack::eID_NOTIFY_3APROC_FINISH)
    {
        MY_LOGD("3APROC_FINISH:0x%08X,0x%08X",
                _ext1,
                _ext2);

        if(mCurMagicNum == _ext1)
        {
            if(_ext2 & (1 << I3ACallBack::e3AProcOK))
            {
                mpIspSyncCtrl->send3AUpdateCmd(IspSyncControl::UPDATE_CMD_OK);
            }
            else
            {
                mpIspSyncCtrl->send3AUpdateCmd(IspSyncControl::UPDATE_CMD_FAIL);
            }
        }
        else
        {
            MY_LOGE("MagicNum:Cur(0x%08X) != Notify(0x%08X)",
                    mCurMagicNum,
                    _ext1);
        }
    }
}


/*******************************************************************************
*
********************************************************************************/
void
NormalCtrl::
doDataCb( int32_t _msgType, void*   _data, uint32_t _size)
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalCtrl::
setIspProfile(NS3A::EIspProfile_T const profile)
{
    mIspProfile = profile;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalCtrl::
_init()
{
    if( mpHal3A )
        mpHal3A->addCallbacks(this);
    //
    if( mIspProfile == EIspProfile_IHDR_Preview || mIspProfile == EIspProfile_IHDR_Video )
        mpIspSyncCtrl->setHdrState(MTRUE);

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalCtrl::
_uninit()
{
    //
    if( mpHal3A )
        mpHal3A->removeCallbacks(this);
    //
    if( mIspProfile == EIspProfile_IHDR_Preview || mIspProfile == EIspProfile_IHDR_Video )
        mpIspSyncCtrl->setHdrState(MFALSE);
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalCtrl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    //
    switch(msg)
    {
        case PASS1_START_ISP:
            {
                MUINT32     magicNum = ext1;
                MUINT32     sensorScenario;
                MUINT32     sensorWidth;
                MUINT32     sensorHeight;
                MUINT32     sensorType;
                //
                mpIspSyncCtrl->getSensorInfo(
                        sensorScenario,
                        sensorWidth,
                        sensorHeight,
                        sensorType);
                //
                mpHal3A->enterCaptureProcess();
                mpHal3A->setSensorMode(sensorScenario);
                mpHal3A->sendCommand(ECmd_CaptureStart);

                MY_LOGD("3A setIspProfile %d, # 0x%x",
                        EIspProfile_Preview,
                        magicNum );
                ParamIspProfile_T _3A_profile(
                        EIspProfile_Preview,
                        magicNum,
                        MTRUE,
                        ParamIspProfile_T::EParamValidate_All);
                mpHal3A->setIspProfile(_3A_profile);
                mpIspSyncCtrl->send3AUpdateCmd(IspSyncControl::UPDATE_CMD_OK);
            }
            break;

        case PASS1_EOF:


            // Check if 3A hal is ready
            if(mpHal3A && ext1 != MAGIC_NUM_INVALID ) //check if 3A udpate ready
            {
                // wait for sensor delay
                // Once it's done, start to post buffer
                if( ext2 != MAGIC_NUM_INVALID)
                {
                    mbDoCap = MTRUE;
                }
                else
                {
                    //do 3A update
                    mCurMagicNum = ext1;
                    ParamIspProfile_T _3A_profile(
                            EIspProfile_Preview,
                            mCurMagicNum,
                            MTRUE,
                            ParamIspProfile_T::EParamValidate_All);
                    mpHal3A->sendCommand(ECmd_Update, reinterpret_cast<MINTPTR>(&_3A_profile));
                }
                break;
            }

            // If it has started to post buffer,
            // do 3A capture command to update tuning parameters(which will be used by Pass2)
            if(mbDoCap){
                if(mpHal3A)
                        {
                            const MUINT32 magicDeque = ext2;

                            // mpHal3A->sendCommand(ECmd_CaptureEnd);

                            if( isDataConnected(CONTROL_CAP_SRC) ) {
                                // IDbgInfoContainer* pDbgInfo = NULL;
                                if( isDataConnected(CONTROL_DBGINFO) )
                                {
                                    mbDbgInfo_cap = IDbgInfoContainer::createInstance();
                                }
                                MY_LOGD("3A setIspProfile %d, # 0x%x",
                                        mIspProfile,
                                        SetCap(magicDeque) );
                                        //dng_metadata_cb
                                        // DNGInfo_T rDngInfo;//
                                        //
                                ParamIspProfile_T _3A_profile(
                                        mIspProfile,
                                        SetCap(magicDeque),
                                        MFALSE, //always pgn
                                        ParamIspProfile_T::EParamValidate_P2Only
                                        ,&mbDngInfo);//
                                mpHal3A->setIspProfile(_3A_profile, mbDbgInfo_cap);
                                //dng_metadata_cb
                                //
                                // if( isDataMsgEnabled(ENode_DATA_MSG_META) )
                                // {
                                //     MY_LOGD("handleDataCallback-start");
                                //     handleDataCallback(ENode_DATA_MSG_META , (MUINTPTR)(&rDngInfo),0,0);//
                                //     MY_LOGD("handleDataCallback-end");
                                // }
                                //
                                // if( pDbgInfo != NULL )
                                //     handlePostBuffer(CONTROL_DBGINFO, (MUINTPTR)pDbgInfo, 0);

                            }

                            if( isDataConnected(CONTROL_PRV_SRC) ) {
                                // IDbgInfoContainer* pDbgInfo = NULL;
                                if( isDataConnected(CONTROL_DBGINFO) )
                                {
                                    mbDbgInfo_prv = IDbgInfoContainer::createInstance();
                                }
                                MY_LOGD("3A setIspProfile %d, # 0x%x, dev(0x%x)",
                                        EIspProfile_Preview,
                                        magicDeque,
                                        getSensorDevIdx());
                                ParamIspProfile_T _3A_profile(
                                        EIspProfile_Preview,
                                        magicDeque,
                                        MTRUE, //always rpg
                                        ParamIspProfile_T::EParamValidate_P2Only);
                                if(SENSOR_DEV_MAIN == getSensorDevIdx())
                                {
                                    mpHal3A->setIspProfile(_3A_profile);
                                }
                                else
                                {
                                    mpHal3A->setIspProfile(_3A_profile, mbDbgInfo_prv);
                                    // if( pDbgInfo != NULL )
                                    //     handlePostBuffer(CONTROL_DBGINFO, (MUINTPTR)pDbgInfo, 0);
                                    // mbDbgInfo_prv = pDbgInfo;
                                }
                            }
                        }
            }

            // Stop pass1 if sync is OK
            if(mbSyncOk){
                // if syncOk == true, stop PASS1 and set 3A command such as captureEnd, etc.
                mbDoCap = MFALSE;
                handleNotify( CONTROL_STOP_PASS1, 0, 0 );
                if(mpHal3A){
                    mpHal3A->sendCommand(ECmd_CaptureEnd);
                }

                // handle cb data
                if( isDataConnected(CONTROL_CAP_SRC) ){
                    //dng_metadata_cb
                    if( isDataMsgEnabled(ENode_DATA_MSG_META))
                    {
                        MY_LOGD("handleDataCallback-start");
                        handleDataCallback(ENode_DATA_MSG_META , (MUINTPTR)(&mbDngInfo),0,0);//
                        MY_LOGD("handleDataCallback-end");
                    }
                    if( mbDbgInfo_cap != NULL ){
                        handlePostBuffer(CONTROL_DBGINFO, (MUINTPTR)mbDbgInfo_cap, 0);
                    }
                }
                if( isDataConnected(CONTROL_PRV_SRC) ) {
                    if(SENSOR_DEV_MAIN != getSensorDevIdx()){
                        if( mbDbgInfo_prv != NULL ){
                            handlePostBuffer(CONTROL_DBGINFO, (MUINTPTR)mbDbgInfo_prv, 0);
                        }
                    }
                }

                // notify shutter
                //handleNotify(CONTROL_SHUTTER);
            }

            break;
        case PASS1_STOP_ISP:
            if( mpHal3A){
                mpHal3A->exitCaptureProcess();
            }
            break;
        case SYNC_OK_SRC_0:
        case SYNC_OK_SRC_1:
            mbSyncOk = MTRUE;
            break;
        default:
            break;
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ZsdVssCtrl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    //
    switch(msg)
    {
        case PASS1_START_ISP:
            // do nothing
            break;

        case PASS1_EOF:
            if( ext2 != MAGIC_NUM_INVALID ) // wait for sensor dealy
            {
                // stop pass1
                handleNotify( CONTROL_STOP_PASS1, 0, 0 );
                if(mpHal3A)
                {
                    const MUINT32 magicDeque = ext2;

                    if( isDataConnected(CONTROL_CAP_SRC) ) {
                        IDbgInfoContainer* pDbgInfo = NULL;
                        if( isDataConnected(CONTROL_DBGINFO) )
                        {
                            pDbgInfo = IDbgInfoContainer::createInstance();
                        }
                        MY_LOGD("3A setIspProfile %d, # 0x%x",
                                mIspProfile,
                                SetCap(magicDeque) );
                        //dng_metadata_cb
                            DNGInfo_T rDngInfo;//
                        //
                        ParamIspProfile_T _3A_profile(
                                mIspProfile,
                                SetCap(magicDeque),
                                MFALSE,
                                ParamIspProfile_T::EParamValidate_P2Only,
                                &rDngInfo);
                        mpHal3A->setIspProfile(_3A_profile, pDbgInfo);
                        //
                        if( isDataMsgEnabled(ENode_DATA_MSG_META) )
                        {
                            MY_LOGD("cap-handleDataCallback-start");
                            handleDataCallback(ENode_DATA_MSG_META , (MUINTPTR)(&rDngInfo),0,0);//
                            MY_LOGD("cap-handleDataCallback-end");
                        }
                        //

                        if( pDbgInfo != NULL ){
                            handlePostBuffer(CONTROL_DBGINFO, (MUINTPTR)pDbgInfo, 0);
                        }

                    }

                    if( isDataConnected(CONTROL_PRV_SRC) ) {
                        IDbgInfoContainer* pDbgInfo = NULL;
                        if( isDataConnected(CONTROL_DBGINFO) )
                        {
                            pDbgInfo = IDbgInfoContainer::createInstance();
                        }
                        MY_LOGD("3A setIspProfile %d, # 0x%x, dev(0x%x)",
                                EIspProfile_Preview,
                                magicDeque,
                                getSensorDevIdx());
                        ParamIspProfile_T _3A_profile(
                                EIspProfile_Preview,
                                magicDeque,
                                MTRUE, //always rpg
                                ParamIspProfile_T::EParamValidate_P2Only);
                        if(SENSOR_DEV_MAIN == getSensorDevIdx())
                        {
                            mpHal3A->setIspProfile(_3A_profile);
                        }
                        else
                        {
                            mpHal3A->setIspProfile(_3A_profile, pDbgInfo);
                            if( pDbgInfo != NULL ){
                                handlePostBuffer(CONTROL_DBGINFO, (MUINTPTR)pDbgInfo, 0);
                            }

                        }
                    }

                }
                handleNotify(CONTROL_SHUTTER);
                break;
            }
            break;

        case PASS1_STOP_ISP:
            // do nothing
            break;

        default:
            break;
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ZsdVssCtrl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{

    MY_LOGD("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    MUINT32       dstdata = 0;
    //
    switch(data)
    {
        case CONTROL_FULLRAW:
            dstdata     = CONTROL_CAP_SRC;
            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: [CONTROL_FULLRAW]buf(%p), ext(0x%x)", buf, ext);
            break;
        case CONTROL_RESIZEDRAW:
            dstdata     = CONTROL_PRV_SRC;
            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: [CONTROL_RESIZEDRAW]buf(%p), ext(0x%x)", buf, ext);
            break;
        default:
            MY_LOGE("not support yet: %d", data);
            break;
    }
    //
    if( dstdata != 0 ) {
        handleNotify(CONTROL_SHUTTER);
        handlePostBuffer(dstdata, buf);
    }
    //
    return MTRUE;
}


////////////////////////////////////////////////////////////////////////////////
//};	//namespace NSCamStereo
};  //namespace NSCamShot

