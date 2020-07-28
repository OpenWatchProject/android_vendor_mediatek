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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_SDK_HAL_IEFFECTHAL_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_SDK_HAL_IEFFECTHAL_H_

#include <utils/String8.h>
#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <utils/KeyedVector.h>
#include <utils/Errors.h>
#include <utils/Singleton.h>
#include <utils/Flattenable.h>

#include <mtkcam/utils/imgbuf/IImageBuffer.h>


#include <mtkcam3/feature/effectHalBase/IEffectListener.h>
#include <mtkcam3/feature/effectHalBase/EffectRequest.h>

namespace NSCam
{
using namespace android;

//@todo remove this
class IEffectHal;
class IEffectHalClient;
class IImageBuffer;
class IEffectListener;

typedef enum
{
    HDR_MODE,
    FB_MODE,
    MFB_MODE,
} EFFECT_SDK_HAL_MODE;


class EffectHalVersion : public LightFlattenable<EffectHalVersion>
{
public:    //LightFlattenable
    inline  bool        isFixedSize() const
    {
        return false;
    }
    size_t              getFlattenedSize() const;
    status_t   flatten(void* buffer, size_t size) const;
    status_t   unflatten(void const* buffer, size_t size);

private:
    static void flattenString8(void*& buffer, size_t& size, const String8& string8);
    static bool unflattenString8(void const*& buffer, size_t& size, String8& outputString8);

public:    //@todo private
    String8     effectName;
    uint32_t    mCallbackID;
    uint32_t    major;
    uint32_t    minor;

};







/**
 *  @brief                      The prototype of MediaTek camera features.
 *  @details                    A common case of call sequence will be
 *  <pre>
 *  getNameVersion() (optional)
 *  init()
 *    setEffectListener()
 *    setParameter() * N
 *    prepare()
 *      setParameter() * N
 *      getCaptureRequirement()
 *      start()
 *        addInputFrame() * N
 *        addOutputFrame() * N
 *        abort() (optional)
 *    release()
 *  uninit()
 *  </pre>
 */
class IEffectHal
{
public:
    virtual     ~IEffectHal(){};

public: // may change state
    /**
     *  @brief                  The first function to initialize IEffectHal object.
     *
     *  @par When to call:
     *                          At the start of IEffectHal instance has been created.
     *
     *  @return                 status_t
     */
    virtual status_t   init() = 0;

    /**
     *  @brief                  The latest function to de-initialize IEffectHal object.
     *
     *  @par When to call:
     *  After calling init()
     */
    virtual status_t   uninit() = 0;

    /**
     *  @brief                  A start call to inform IEffectHal, the client is ready to initial a request.
     *
     *  @par When to call:
     *                          After calling prepare(), but before calling release().
     *
     *  @return                 status_t
     */
    virtual status_t   configure() = 0;

    /**
     *  @brief                  Abort current process.
     *  @details                client call this function to abort IEffectHal current activity.
     *
     *  @par When to call:
     *                          After calling start(), but before EffectListener::onAborted() or EffectListener::onCompleted() has been triggered.
     *
     *  @return                 status_t
     */
    virtual status_t   unconfigure() = 0;

    /**
     *  @brief                  A start call to inform IEffectHal, the client is ready to add input/output buffer.
     *
     *  @par When to call:
     *                          After calling prepare(), but before calling release().
     *
     *  @return                 session id - a unique id for all IEffectHal::start()
     */
    virtual uint64_t            start() = 0;

    /**
     *  @brief                  Abort current process.
     *  @details                client call this function to abort IEffectHal current activity.
     *
     *  @par When to call:
     *                          After calling start(), but before EffectListener::onAborted() or EffectListener::onCompleted() has been triggered.
     *
     *  @param parameter        for client to config abort behavior.
     *                          EX: For MAV and Panorama
     *                          - parameter["save"] = true
     *
     */
    virtual status_t   abort(EffectParameter const *parameter=NULL) = 0;


public: // would not change state
    /**
     *  @brief                  Get version of IEffectHal object
     *
     *  @par When to call:
     *                          At the start of IEffectHal instance has been created.
     *
     *  @param[out] nameVersion A reference of returned name, major, minor version number
     */
    virtual status_t   getNameVersion(EffectHalVersion &nameVersion) const = 0;

    /**
     *  @brief                  Client register listener object by this function.
     *
     *  @par When to call:
     *                          At the start of IEffectHal instance has been created.
     *
     *  @param[in] listener     Point to client's listener object
     */
    virtual status_t   setEffectListener(const wp<IEffectListener>& listener) = 0;


    /**
     *  @brief                  The usage is similar to Android CameraParameters. The client use this api to set IEffectHal's parameter.
     *  @details                EX:setParameter(ZoomRatio, "320"); setParameter(Transform, "90");
     *
     *  @par When to call:
     *                          After calling init(), but before calling start().
     *
     *  @param[in] parameterKey the key name for the parameter
     *  @param[in] object       The address that point to the string value of the parameter
     *  @return                 status_t
     */
    virtual status_t   setParameter(String8 &key, String8 &object) = 0;    //@todo find better solution
    virtual status_t   setParameters(const sp<EffectParameter> parameter) =0;

    /**
     *  @brief                  Get the requirement for following capture request.
     *
     *  @par When to call:
     *                          After calling init(), but before calling unint().
     *
     *  @param[out] requirement filled Capture requirement. EX: (HDR)Target exp_time, gain for bright/dark frame
     *  @return                 status_t
     */
    virtual status_t   getCaptureRequirement(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const = 0;

    /**
     *  @brief                  [non-blocking] The function to allocate necessary resource, initialize default setting of IEffectHal object.
     *
     *  @par When to call:
     *                          After calling init(), but before calling uninit()
     *
     *  @return                 status_t
     */
    //non-blocking
    virtual status_t   prepare() = 0;

    /**
     *  @brief                  Release the resource allocated by IEffectHal::prepare().
     *
     *  @par When to call:
     *                          After calling prepare(), but before calling uninit().
     */
    virtual status_t   release() = 0;

    /**
     *  @brief                  [non-blocking] Client call this function to feed input frame into IEffectHal for further process.
     *  @details                EX: (HDR) Client feed bright and dark frames by call this function twice.
     *
     *  @par When to call:
     *                          After calling start(), but before EffectListener::onAborted() or EffectListener::onCompleted() has been triggered.
     *
     *  @param[in] frame        Point to input frame buffer. Set this as NULL if frame is unnecessary.
     *  @param[in] parameter    For client to attach information with buffer.
     *                          EX:
     *                          - parameter["key"] = "long-exposure"
     *                          - parameter["id"] = 1
     *  @return                 status_t
     */
    //non-blocking
    //virtual status_t   addInputFrame(const sp<IImageBuffer> frame, const sp<EffectParameter> parameter=NULL) = 0;

    /**
     *  @brief                  [non-blocking] Client call this function to feed output frame into IEffectHal for further process.
     *  @details                EX: (HDR) HDR IEffectHal save processed frame in frame.
     *
     *  @par When to call:
     *                          After calling start(), but before EffectListener::onAborted() or EffectListener::onCompleted() has been triggered.
     *
     *  @param[in] frame        Point to output frame buffer. Set this as NULL if frame is unnecessary.
     *  @param[in] parameter    For client to attach information with buffer.
     *                          EX:
     *                          - parameter["key"] = "long-exposure"
     *                          - parameter["id"] = 1
     */
    //non-blocking
    //virtual status_t   addOutputFrame(const sp<IImageBuffer> frame, const sp<EffectParameter> parameter=NULL) = 0;

    virtual status_t   updateEffectRequest(const sp<EffectRequest> request) = 0;
public: //debug
public: //autotest
};


};  //namespace NSCam {


#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_SDK_HAL_IEFFECTHAL_H_

