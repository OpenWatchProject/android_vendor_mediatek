/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
/*
 * File name:  RfxController.h
 * Author: Jun Liu (MTK80064)
 * Description:
 *  Define the prototypes of base controller class.
 */

#ifndef __RFX_CONTROLLER_H__
#define __RFX_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <sys/types.h>
#include "utils/String8.h"
#include "RfxDefs.h"
#include "RfxMessage.h"
#include "RfxObject.h"
#include "RfxStatusManager.h"
#include "RfxTimer.h"
#include "RfxSignal.h"

using ::android::Vector;
using ::android::String8;

typedef RfxCallback1<const sp<RfxMessage>&> RfxWaitResponseTimedOutCallback;

typedef enum {
    RESPONSE_STATUS_NO_MATCH_AND_SAVE,
    RESPONSE_STATUS_HAVE_MATCHED,
    RESPONSE_STATUS_HAVE_MATCHED_WITH_C_CHANGED,
    RESPONSE_STATUS_ALREADY_SAVED,
    RESPONSE_STATUS_HAVE_BEEN_DELETED,
    RESPONSE_STATUS_INVALID
} ResponseStatus;

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
class RfxController;

class RfxWaitReponseTimerHelper : public RfxObject {
    RFX_DECLARE_CLASS(RfxWaitReponseTimerHelper);

public:
    RfxWaitReponseTimerHelper() {
    }

    RfxWaitReponseTimerHelper(const sp<RfxMessage>& message,
        const RfxCallback1<const sp<RfxMessage>&> &callback, RfxController *controller) :
        m_reponse_msg(message),
        m_callback1(callback),
        m_controller(controller) {
    }

public:
    void onTimer();

public:
    sp<RfxMessage> m_reponse_msg;
    RfxCallback1<const sp<RfxMessage>&> m_callback1;
    wp<RfxController> m_controller;
};

class RfxWaitReponseEntry {
public:
    RfxWaitReponseEntry() :
        m_reponse_msg(NULL),
        m_timer_handle(NULL),
        m_entry_helper(NULL) {
    }

    RfxWaitReponseEntry(const sp<RfxMessage>& message) :
        m_reponse_msg(message),
        m_timer_handle(NULL),
        m_entry_helper(NULL) {
    }

    RfxWaitReponseEntry(const sp<RfxMessage>& message,
        const TimerHandle& timerHandle, const wp<RfxWaitReponseTimerHelper> &entryHelper) :
        m_reponse_msg(message),
        m_timer_handle(timerHandle),
        m_entry_helper(entryHelper) {
    }

    ~RfxWaitReponseEntry() {
    }

public:
    void deleteEntryHelper() {
        if (m_entry_helper != NULL) {
            RfxWaitReponseTimerHelper* helper = m_entry_helper.promote().get();
            if (helper != NULL) {
                RFX_OBJ_CLOSE(helper);
            }
        }
    }

public:
    sp<RfxMessage> m_reponse_msg;
    TimerHandle m_timer_handle;
    wp<RfxWaitReponseTimerHelper> m_entry_helper;
};


/*
 * Class RfxController,
 *  prototype of base controller class, define the virtual
 *  functions which sub-class need to override, implement
 *  the default behaviors of controllers
 */
class RfxController : public RfxObject
{
    RFX_DECLARE_CLASS(RfxController);
// Constructor / Destructor
public:
    // Constructor
    RfxController();

    virtual ~RfxController();

public:

    enum HANDLER_PRIORITY{
        NORMAL,
        DEFAULT = NORMAL,
        MEDIUM,
        HIGH,
        HIGHEST
    };
    enum ARGU_TYPE {
        PTOKEN,
        TOKEN
    };

// Overridable
protected:

    virtual bool onPreviewMessage(const sp<RfxMessage>& message);

    virtual bool onHandleRequest(const sp<RfxMessage>& message);

    virtual bool onHandleUrc(const sp<RfxMessage>& message);

    virtual bool onHandleResponse(const sp<RfxMessage>& message);

    virtual bool onCheckIfResumeMessage(const sp<RfxMessage>& message);

    virtual bool onCheckIfRemoveSuspendedMessage(const sp<RfxMessage>& message);

    virtual bool onHandleAtciRequest(const sp<RfxMessage>& message);

    virtual bool onHandleAtciResponse(const sp<RfxMessage>& message);

    virtual bool onCheckIfTransferParcelToObj(const sp<RfxMessage>& message);

    virtual RfxDataBase* onProcessParcelToObj(const sp<RfxMessage>& message);
// Methods
public:

    RfxController *getSlotRoot(int slot_id) const;

    RfxController *getSlotRoot() const;

    RfxController *findController(int slot_id, const RfxClassInfo *class_info);

    RfxController *findController(const RfxClassInfo *class_info);

    void requestToRild(const sp<RfxMessage>& message);

    void requestAckToRilj(const sp<RfxMessage>& message);

    bool responseToRilj(const sp<RfxMessage>& message);

    void registerToHandleRequest(const int *request_id_list,
            size_t length,
            HANDLER_PRIORITY priority = DEFAULT);

    void unregisterToHandleRequest(const int *request_id_list,
            size_t length,
            HANDLER_PRIORITY priority = DEFAULT);

    void registerToHandleUrc(const int *urc_id_list, size_t length);

    void unregisterToHandleUrc(const int *urc_id_list, size_t length);

    void registerRequestToCsRild(const int *request_id_list, size_t length);

    void unregisterRequestToCsRild(const int *request_id_list, size_t length);

    void registerRequestToPsRild(const int *request_id_list, size_t length);

    void unregisterRequestToPsRild(const int *request_id_list, size_t length);

    void registerToHandleRequest(int slot_id, const int *request_id_list,
            size_t length,
            HANDLER_PRIORITY priority = DEFAULT);

    void unregisterToHandleRequest(int slot_id, const int *request_id_list,
            size_t length,
            HANDLER_PRIORITY priority = DEFAULT);

    void registerToHandleUrc(int slot_id, const int *urc_id_list, size_t length);

    void unregisterToHandleUrc(int slot_id, const int *urc_id_list, size_t length);

    void registerRequestToCsRild(int slot_id, const int *request_id_list, size_t length);

    void unregisterRequestToCsRild(int slot_id, const int *request_id_list, size_t length);

    void registerRequestToPsRild(int slot_id, const int *request_id_list, size_t length);

    void unregisterRequestToPsRild(int slot_id, const int *request_id_list, size_t length);

    int getSlotId() const;

    RfxStatusManager *getStatusManager(int slot_id) const;

    RfxStatusManager *getStatusManager() const;

    RfxStatusManager *getNonSlotScopeStatusManager() const;

    ResponseStatus preprocessResponse(const sp<RfxMessage>& message, sp<RfxMessage>& outResponse);

    ResponseStatus preprocessResponse(const sp<RfxMessage>& message, sp<RfxMessage>& outResponse,
        const RfxWaitResponseTimedOutCallback &callback, const nsecs_t time = 0);

    void printLog(int level, String8 tag, String8 log, int slot) const;
    void logD(const char *tag, const char *fmt, ...) const;
    void logV(const char *tag, const char *fmt, ...) const;
    void logE(const char *tag, const char *fmt, ...) const;
    void logW(const char *tag, const char *fmt, ...) const;
    void logI(const char *tag, const char *fmt, ...) const;

    void transferParcelToObj(const sp <RfxMessage>& message);
    void addToBlackListForSwitchCDMASlot(
            const int *request_id_list, size_t length);
    sp<RfxMessage> processBlackListResponse(
            const sp<RfxMessage>& message);

    static void writeStringToParcel(Parcel *p, const char *s);
    void resetPreprocessedResponse(const sp<RfxMessage>& message);
    void resetPreprocessedResponse(int pId, nsecs_t timeStamp, int pToken);

// Override
public:

    virtual void onInit();

    virtual void onDeinit();

public:

    // framework internal use
    virtual bool processMessage(const sp<RfxMessage>& message);

    bool checkIfResumeMessage(const sp<RfxMessage>& message);

    bool checkIfRemoveSuspendedMessage(const sp<RfxMessage>& message);

    virtual bool processAtciMessage(const sp<RfxMessage>& message);

    virtual void clearMessages();

private:

    void setProcessedMsg(int id, nsecs_t timeStamp);
    int getProceesedMsgIndex(int id, nsecs_t timeStamp);
    String8 toString();
    void clearWaitResponseList();
    void removeCachedResponse(ARGU_TYPE arguType, int value);
    void removeProcessedMsg(int id, nsecs_t timeStamp);

protected:

    int m_slot_id;

    int m_last_token;

private:

    class ProcessedMsgInfo {
    public:
        ProcessedMsgInfo() : id(0), pTimeStamp(0) {}
        ProcessedMsgInfo(int _id, nsecs_t timeStamp) :
            id(_id), pTimeStamp(timeStamp) {}
    public:
        int id;
        nsecs_t pTimeStamp;
    } ;

private:

    Vector<RfxWaitReponseEntry> m_wait_reponse_msg_list;
    Vector<ProcessedMsgInfo> m_processed_msg_list;
    Vector<int> mBlackListForSwitchCdmaSlot;

    friend RfxWaitReponseTimerHelper;
};

inline
RfxController *RfxController::getSlotRoot() const {
    return getSlotRoot(m_slot_id);
}

inline
RfxController *RfxController::findController(const RfxClassInfo *class_info) {
    return findController(m_slot_id, class_info);
}

inline
RfxStatusManager *RfxController::getStatusManager() const {
    return getStatusManager(m_slot_id);
}

inline
RfxStatusManager *RfxController::getNonSlotScopeStatusManager() const {
    return getStatusManager(RFX_SLOT_ID_UNKNOWN);
}

inline
void RfxController::registerToHandleRequest(const int *request_id_list,
            size_t length, HANDLER_PRIORITY priority) {
    registerToHandleRequest(m_slot_id, request_id_list, length, priority);
}

inline
void RfxController::unregisterToHandleRequest(const int *request_id_list,
            size_t length, HANDLER_PRIORITY priority) {
    unregisterToHandleRequest(m_slot_id, request_id_list, length, priority);
}

inline
void RfxController::registerToHandleUrc(const int *urc_id_list, size_t length) {
    registerToHandleUrc(m_slot_id, urc_id_list, length);
}

inline
void RfxController::unregisterToHandleUrc(const int *urc_id_list, size_t length) {
    unregisterToHandleUrc(m_slot_id, urc_id_list, length);
}

inline
void RfxController::registerRequestToCsRild(const int *request_id_list, size_t length) {
    registerRequestToCsRild(m_slot_id, request_id_list, length);
}

inline
void RfxController::unregisterRequestToCsRild(const int *request_id_list, size_t length) {
    unregisterRequestToCsRild(m_slot_id, request_id_list, length);
}

inline
void RfxController::registerRequestToPsRild(const int *request_id_list, size_t length) {
    registerRequestToPsRild(m_slot_id, request_id_list, length);
}

inline
void RfxController::unregisterRequestToPsRild(const int *request_id_list, size_t length) {
    unregisterRequestToPsRild(m_slot_id, request_id_list, length);
}

#endif /* __RFX_CONTROLLER_H__ */

