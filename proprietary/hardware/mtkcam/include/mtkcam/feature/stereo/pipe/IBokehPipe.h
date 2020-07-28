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
#ifndef _MTK_CAMERA_FEATURE_PIPE_BOKEH_PIPE_H_
#define _MTK_CAMERA_FEATURE_PIPE_BOKEH_PIPE_H_
//
using namespace android;

// Android system/core header file
#include <utils/RefBase.h>

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
class EffectRequest;
namespace NSCamFeature {
namespace NSFeaturePipe {

#define BOKEH_META_KEY_STRING "Metadata"
#define BOKEH_FLUSH_KEY "onFlush"
#define BOKEH_ERROR_KEY "onError"
#define BOKEH_YUV_DONE_KEY "onYUVDone"

/*******************************************************************************
* Enum Definition
********************************************************************************/

typedef enum BokehEffectRequestBufferType
{
    BOKEH_ER_BUF_START = 0,
    BOKEH_ER_BUF_INVALID,
    // The tag of Bokeh input buffer.
    BOKEH_ER_BUF_MYS,
    BOKEH_ER_BUF_DMW,
    BOKEH_ER_BUF_MAIN1,
    BOKEH_ER_BUF_DMG,        //5
    BOKEH_ER_BUF_DMBG,
    BOKEH_ER_BUF_3DNR_INPUT,
    BOKEH_ER_BUG_TUNING_BUFFER,
    // The tag of Bokeh output buffer.
    BOKEH_ER_BUF_3DNR_OUTPUT,
    BOKEH_ER_BUF_VSDOF_IMG,  //10
    BOKEH_ER_BUF_CLEAN_IMG,
    BOKEH_ER_BUF_RECORD,
    BOKEH_ER_BUF_DISPLAY,
    BOKEH_ER_BUF_THUMBNAIL,
    BOKEH_ER_BUF_WDMAIMG,    //15
    BOKEH_ER_BUF_PRVCB,
    // sw bokeh
    BOKEH_ER_BUF_HAL_DEPTHWRAPPER,
    BOKEH_ER_BUF_EXTRADATA,
    BOKEH_ER_BUF_DEPTHMAPWRAPPER,
    BOKEH_ER_BUF_APP_EXTRA_DATA, //20
    // P2 tuning
    BOKEH_ER_BUF_P2_TUNING,
    // input meta
    BOKEH_ER_IN_APP_META,
    BOKEH_ER_IN_HAL_META_MAIN1,
    BOKEH_ER_IN_HAL_META_MAIN2,
    // output meta
    BOKEH_ER_OUT_APP_META,   //25
    BOKEH_ER_OUT_HAL_META,
    // internel use meta
    BOKEH_ER_IN_P1_RETURN_META,
    // third party
    BOKEH_ER_OUTPUT_DEPTHMAP,
    // end
    BOKEH_ER_BUF_END = 100

}BokehEffectRequestBufferType;

/**
 * @brief run path of BokehPipe
 */
typedef enum eBokehPipeMode {
    GENERAL,
    ADVANCE,
    HW_BOKEH,
    SW_BOKEH,
    VENDOR_BOKEH
} BokehPipeMode;

/**
 * @brief featurePipe handle request strategy
 * SYNC:  Every request should be handle
 * FLUSH: Just handle the current request, others will be drop
 */
typedef enum VSDOF_STOP_MODE
{
    SYNC,
    FLUSH,
}VSDOF_STOP_MODE;

/*******************************************************************************
* Class Definition
********************************************************************************/
class IBokehPipe : public virtual android::RefBase
{
    public:
        /*
        * Return sp<IBokehPipe> instance.
        * @param openSensorIndex open sensor index
        * @param runPath decide feature pipe data flow
        */
        static sp<IBokehPipe> createInstance(MINT32 openSensorIndex, MINT32 runPath = 0);
        /*
        * destroy instance
        */
        MBOOL destroyInstance(const char *name=NULL);
        /*
        * BokekPipe initialization
        */
        virtual MBOOL init(const char *name=NULL) = 0;
        /*
        * uninit BokehPipe
        */
        virtual MBOOL uninit(const char *name=NULL) = 0;
        /*
        * Pass EffectRequest to BokehPipe.
        * EffectRequest contains MY_S, DMW and Main image buffers, related
        * output buffer by scenario and AF info.
        * After processing finished, BokehPipe will call callback function
        * (defined in EffectRequest) to return result.
        */
        virtual MBOOL enque(sp<EffectRequest> &param) = 0;
        /*
        * Discard all request in wait queue.
        * If request is been enque in HW, it will wait HW processing finish.
        */
        virtual MVOID flush() = 0;
        /*
        * Sync function will wait until all EffectRequest is processed done.
        */
        virtual MVOID sync() = 0;
        /*
        * Set stop mode to BokehPipe.
        * If flushOnStop is true, all EffectRequest will be discard when user
        * call uninit.
        * If flushOnStop is false, BokehPipe will uninit until all EffectRequest
        * is processed done.
        */
        virtual MVOID setStopMode(MBOOL flushOnStop) = 0;
        /*
        * Get pipe mode (run path)
        * return: eBokehPipeMode
        */
        virtual MINT32 getPipeMode() = 0;
};
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

#endif // _MTK_CAMERA_FEATURE_PIPE_BOKEH_PIPE_H_
