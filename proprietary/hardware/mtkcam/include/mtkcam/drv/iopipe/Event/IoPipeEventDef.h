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

#ifndef __IO_PIPE_EVENT_DEF_H__
#define __IO_PIPE_EVENT_DEF_H__

#include <utils/LightRefBase.h>
#include <utils/RefBase.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>


namespace NSCam {
namespace NSIoPipe {

/**
 * Event ID.
 */
enum IoPipeEventId
{
    EVT_IOPIPE_NONE,
    /** NormalStream requires IP-RAW and ask P1 to stop
     * @see IpRawP1AcquiringEvent
     */
    EVT_IPRAW_P1_ACQUIRING,
    /** NormalStream released P1 hardware
     * @see IpRawP1ReleasedEvent
     */
    EVT_IPRAW_P1_RELEASED,
    /** Dequed from NormalPipe
     * @see NormalPipeDequedEvent
     */
    EVT_NORMAL_PIPE_DEQUED,
};

/**
 * The base class of events.
 * All event must be derived from this class.
 * @see IoPipeEventSystem::subscribe()
 */
class IoPipeEvent : public android::LightRefBase<IoPipeEvent>
{
public:
    /** Type of cookie */
    typedef MINTPTR CookieType;
    /** Type of setResult(), getResult() */
    typedef unsigned int ResultType;

    /** Predefined result enum.
     * Event can use this enum for the result simply or define theirs in the derived event structure.
     * Note derivations do not have to use the predefined values. Please always contact the
     * owner to confirm
     * @see setResult, getResult
     */
    enum PredefinedResult {
        RESULT_OK = 0,
        /// Unexpected error
        RESULT_ERROR,
        /// Current state does not accept the event
        RESULT_REJECT,
        /// Not executed due to busy
        RESULT_BUSY,
        /// Already executing the command asynchronously
        RESULT_EXECUTING,
        /// If derived event wants to not only use predefinied values but extend, begin from here
        RESULT_CUSTOM
    };

    virtual ~IoPipeEvent() { }

    IoPipeEventId getEventId() const {
        return mEventId;
    }

    /** This function will be called by the event system */
    void setCookie(CookieType cookie) {
        mCookie = cookie;
    }

    /** Get the cookie passed into the IoPipeEventSystem::subscribe() */
    CookieType getCookie() const {
        return mCookie;
    }

    /** Use for simple communication between to the event sender and a subscriber.
     * If subscriber sets result, it should also return STOP_BROADCASTING
     * For more complicated communication, there shall be a derived event struct
     * @see PredefinedResult
     */
    void setResult(ResultType result) {
        mResult = result;
    }

    /** For event sender to get the result
     * @see PredefinedResult
     */
    ResultType getResult() const {
        return mResult;
    }

protected:

    /** Every event should have its own type and derive from IoPipeEvent */
    explicit IoPipeEvent(IoPipeEventId eventId) :
        mEventId(eventId), mCookie(0), mResult(RESULT_OK)
    {
    }

    /** Copy constructor */
    IoPipeEvent(const IoPipeEvent &evt) :
        mEventId(evt.mEventId), mCookie(evt.mCookie), mResult(RESULT_OK)
    {
    }

    /** Assignment.
     * If drived extends the structure, must be overrided.
     */
    virtual IoPipeEvent& operator=(const IoPipeEvent &evt);

private:
    IoPipeEventId mEventId;
    CookieType mCookie;
    ResultType mResult;
};


/** An event structure which exactly bind an event ID */
template <IoPipeEventId _EvtId>
class StrictIoPipeEvent : public IoPipeEvent
{
public:
    StrictIoPipeEvent() : IoPipeEvent(_EvtId)
    {
    }

    StrictIoPipeEvent(const StrictIoPipeEvent& event) : IoPipeEvent(event)
    {
    }

    virtual StrictIoPipeEvent& operator=(const StrictIoPipeEvent &evt) {
        IoPipeEvent::operator=(evt);
        return *this;
    }

    /** Event ID is exactly bound to type
     * This value will be verifed by simplified subscribe() APIs.
     * @see IoPipeEventSystem::subscribe()
     */
    static constexpr IoPipeEventId __EVENT_ID__ = _EvtId;
};


/** Event structure of EVT_IPRAW_P1_ACQUIRING. It is strongly ID-associated */
typedef StrictIoPipeEvent<EVT_IPRAW_P1_ACQUIRING> IpRawP1AcquiringEvent;

/** Event structure of EVT_IPRAW_P1_RELEASED. It is strongly ID-associated */
typedef StrictIoPipeEvent<EVT_IPRAW_P1_RELEASED> IpRawP1ReleasedEvent;


/** Deque event from NormalPipe */
class NormalPipeDequedEvent : public StrictIoPipeEvent<EVT_NORMAL_PIPE_DEQUED>
{
public:
    NormalPipeDequedEvent(MUINT32 sensorIndex, MUINT32 magicNum, IImageBuffer *imgoBuffer, IImageBuffer* rrzoBuffer) :
        mSensorIndex(sensorIndex), mMagicNum(magicNum), mImgoBuffer(imgoBuffer), mRrzoBuffer(rrzoBuffer)
    {
    }

    /** Sensor index of the deque. */
    MUINT32 getSensorIndex() const {
        return mSensorIndex;
    }

    /** Magic number dequed. */
    MUINT32 getMagicNum() const {
        return mMagicNum;
    }

    /** Get IMGO buffer. If not available (e.g. preview), this field will be NULL.
     * @warning Do NOT hold the pointer after the event handler returned. The life time of the
     * IImageBuffer instance is only guaranteed in the event handler.
     */
    IImageBuffer *getImgoBuffer() const {
        return mImgoBuffer;
    }

    /** Get RRZO buffer. If not available (e.g. capture), this field will be NULL.
     * @warning Do NOT hold the pointer after the event handler returned. The life time of the
     * IImageBuffer instance is only guaranteed in the event handler.
     */
    IImageBuffer *getRrzoBuffer() const {
        return mRrzoBuffer;
    }

private:
    MUINT32 mSensorIndex;
    MUINT32 mMagicNum;
    IImageBuffer *mImgoBuffer;
    IImageBuffer *mRrzoBuffer;
};


}
}

#endif

