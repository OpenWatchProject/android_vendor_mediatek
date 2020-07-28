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
#define LOG_TAG "MtkCam/HwPipeline/Adapter"
//
#include "IHal3AAdapter.h"
#include "MyUtils.h"
//
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_MODEL);
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3::pipeline::model;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("%d[Hal3AAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("%d[Hal3AAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("%d[Hal3AAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("%d[Hal3AAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("%d[Hal3AAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("%d[Hal3AAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("%d[Hal3AAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
class Hal3AAdapter : public IHal3AAdapter
{
protected:  ////            Data Members.
    int32_t                 mId;
    std::string const       mName;

    NS3Av3::IHal3A*         mHal3a = nullptr;

public:     ////    Operations.

                    Hal3AAdapter(int32_t id, std::string const& name)
                        : mId(id)
                        , mName(name)
                    {
                        MY_LOGD("%p", this);
                    }

    virtual auto    init() -> bool
                    {
                        CAM_TRACE_NAME("init(3A)");
                        mHal3a = MAKE_Hal3A(mId, mName.c_str());
                        MY_LOGE_IF(!mHal3a, "Bad mHal3a");
                        return (nullptr != mHal3a);
                    }

    virtual auto    onLastStrongRef(const void* /*id*/) -> void override
                    {
                        CAM_TRACE_NAME("uninit(3A)");
                        if  ( mHal3a ) {
                            mHal3a->destroyInstance(mName.c_str());
                            mHal3a = nullptr;
                        }
                    }

    virtual auto    notifyPowerOn() -> bool
                    {
                        if  ( mHal3a ) {
                            CAM_TRACE_NAME("3A notifyPowerOn");
                            return mHal3a->notifyPwrOn();
                        }
                        return true;
                    }

    virtual auto    notifyPowerOff() -> bool
                    {
                        CAM_TRACE_NAME("3A notifyPowerOff");
                        if  ( mHal3a ) {
                            bool ret = mHal3a->notifyPwrOff();
                            if  ( ! ret ) {
                                CAM_TRACE_NAME("3A notifyPowerOff fail");
                            }
                            return ret;
                        }
                        return true;
                    }

};


/******************************************************************************
 *
 ******************************************************************************/
auto
IHal3AAdapter::
create(
    int32_t id,
    std::string const& name
    ) -> android::sp<IHal3AAdapter>
{
    android::sp<Hal3AAdapter> p = new Hal3AAdapter(id, name);
    if  ( p == nullptr ) {
        CAM_ULOGME("Fail to new Hal3AAdapter");
        return nullptr;
    }

    if  ( ! p->init() ) {
        p = nullptr;
        return nullptr;
    }
    return p;
}

