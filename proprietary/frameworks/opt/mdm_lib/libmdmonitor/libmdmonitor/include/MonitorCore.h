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
/**
 * @file MonitorCore.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c MonitorCore class used to execute commands and dispatch DHL frames to trap receivers.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __MONITOR_CORE_H__
#define __MONITOR_CORE_H__
#include <pthread.h>
#include <list>
#include "libmdmonitor.h"
#include "CommandInterface.h"
#include "FrameDecoder.h"
#include "PrivilegeManager.h"
#include "Session.h"
#include "EMFilterManager.h"
#include "RawDataDumperVirtual.h"

namespace libmdmonitor {

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The command set for modem control.
 */
typedef enum {
    MDM_LOG_CONTROL_CMD_START,
    MDM_LOG_CONTROL_CMD_PAUSE,
    MDM_LOG_CONTROL_CMD_STOP,
    MDM_LOG_CONTROL_CMD_SET_IP_PKT_LEN,
    MDM_LOG_CONTROL_CMD_SEND_EM_FILTER,
    MDM_LOG_CONTROL_CMD_SEND_ICD_FILTER
} MDM_LOG_CONTROL_CMD;


typedef enum {
    MDM_LOG_CONTROL_OK,
    MDM_LOG_CONTROL_ERROR,
    MDM_LOG_CONTROL_CMD_RESPONSE_TIMEOUT
} MDM_LOG_CONTROL_RETCODE;

/**
 * @brief The prototype of modem log controller which will be called back in MonitorCore.<br>
 *
 * @param logCtrlCmd [IN] The command for modem log control.
 * @param param_version [IN] The version of @a param.
 * @param param [IN] The parameters passed by monitor core.
 * @param private_data [IN] The private data managed by the callee and<br>
 *                          should be set using @b MonitorCore::RegisterLogController.
 *
 * @return true: The command for modem log control was successfully processed.
 *        false: Errors.
 */
typedef MDM_LOG_CONTROL_RETCODE (*MDMLogController)(MDM_LOG_CONTROL_CMD logCtrlCmd, unsigned param_version, const void *param, void *private_data);
/**
 * @brief The buffer structure as parameter definition .<br>
 *
 */
struct DataBuffer
{
    char *bufferPtr;
    unsigned int bufferSize;
};

struct ICDFilterData
{
    bool type;  // FALSE: event, TRUE:record
    unsigned int startId;
    unsigned int endId;
    char *bufferPtr;
    unsigned int bufferSize;
    bool enableAsyncResponse;
};

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class is used to execute commands and dispatch DHL frames to trap receivers.
 */
class MonitorCore : public CommandInterface
{
public:
    /**
     * @brief Create a monitor core.
     */
    MonitorCore();

    /**
     * @brief Cleanup.
     */
    virtual ~MonitorCore();

    /**
     * @brief Check if the client is authorized to access MDM framework.
     *        Now is only support to check if APK client can access MDML.
     *
     * @param client [IN] Information of MDM framework client, e.g. package name of APK client
     */
    bool isAuthorized(const char *client);

    /**
     * @brief The main API for modem info collector to check-in DHL frames into monitor core.
     *
     * @param dhlFrameType [IN] DHL frame type. Should be either DHL_INDEX_TRACE or DHL_PRIMITIVE.
     * @param pData [IN] DHL frame data.
     * @param len [IN] The length of @a pData.
     * @param bCheckSubscribed [IN] true: Only those trap receivers subscribing the type of<br>
     *                                    DHL frame can receive the frame by default.<br>
     *                             false: No matter the trap receivers subscribing the type of<br>
     *                                    DHL frame can receive the frame.
     * @return true: The DHL frame was successfully dispatched.<br>
     *        false: Errors.
     */
    bool onRecvData(unsigned char dhlFrameType, const unsigned char *pData, size_t len, bool bCheckSubscribed = true);

    //void BenchmarkTest();
    
    /**
     * @brief The API is used to replay DHL stream in the data unit of ELT log file.
     *
     * @param szPath [IN] The file path of extracted data unit from ELG log file.<br>
     *                    Should not be NULL.
     */
    void PlayDHLStream(const char *szPath);

    /**
     * @brief The API is used by modem info collector to register modem log controller.
     *
     * @param logCtrl [IN] The model log controller will be called back in monitor core.
     * @param param_version [IN] The version of parameters which are passed to @a logCtrl.
     * @param private_data [IN] The private data managed by modem info collector.
     */
    void RegisterLogController(MDMLogController logCtrl, unsigned param_version, void *private_data);

    /**
     * @brief The API is used to register RawDataDumper to write debug infos.
     *
     */
    void RegisterRawDataDumper(RawDataDumperVirtual *dumper);
    /* Interface */
    /**
     * @brief The core implementation of session creation.
     * @see CommandInterface::onCreateSession 
     */
    SID onCreateSession(const char *secret = NULL);

    /**
     * @brief The core implementation of session close.
     * @see CommandInterface::onCloseSession
     */
    MONITOR_CMD_RESP onCloseSession(SID doerId, SID sessionId);

    /**
     * @brief The core implementation of trap subscription.
     * @see CommandInterface::onSubscribeTrap
     */
    MONITOR_CMD_RESP onSubscribeTrap(SID doerId, SID sessionId, TRAP_TYPE type, MSGID msgId, bool enableAsyncResponse = false);
    MONITOR_CMD_RESP onSubscribeMultiTrap(SID doerId, SID sessionId, TRAP_TYPE type, MSGID *msgId, unsigned int msgIdSize, bool enableAsyncResponse = false);

    /**
     * @brief The core implementation of trap unsubscription.
     * @see CommandInterface::onUnsubscribeTrap
     */
    MONITOR_CMD_RESP onUnsubscribeTrap(SID doerId, SID sessionId, TRAP_TYPE type, MSGID msgId, bool enableAsyncResponse = false);
    MONITOR_CMD_RESP onUnsubscribeMultiTrap(SID doerId, SID sessionId, TRAP_TYPE type, MSGID *msgId, unsigned int msgIdSize, bool enableAsyncResponse = false);

    /**
     * @brief The core implementation of starting trap sending.
     * @see CommandInterface::onEnableTrap
     */

    MONITOR_CMD_RESP onEnableTrap(SID doerId, SID sessionId);

    /**
     * @brief The core implementation of stopping trap sending.
     * @see CommandInterface::onDisableTrap
     */
    MONITOR_CMD_RESP onDisableTrap(SID doerId, SID sessionId);

    /**
     * @brief The core implementation of trap receiver setting.
     * @see CommandInterface::onSetTrapReceiver
     */
    MONITOR_CMD_RESP onSetTrapReceiver(SID doerId, SID sessionId, const char *szServerName);


    // TODO: Add Comment
    MONITOR_CMD_RESP onSetHIDLTrapCallback(const char *szServerName, const android::sp<IMDMonitorClientCallback>& callback);

    /**
     * @brief The core implementation of trap receiver setting.
     * @see CommandInterface::onSetIPRawSize
     */
    MONITOR_CMD_RESP onSetIPRawSize(SID doerId, SID sessionId, IP_TYPE type, unsigned int rawSize);

    /**
     * @brief The core implementation of getting session status.
     * @see CommandInterface::onGetSession
     */
    MONITOR_CMD_RESP onGetSession(SID doerId, SID sessionId, MONITOR_SESSION_STATUS &status);

    /**
     * @brief The core implementation of listing sessions.
     * @see CommandInterface::onListSession
     */
    MONITOR_CMD_RESP onListSession(SID doerId, std::vector<SID> &list);

    /**
     * @brief The core implementation of set debug level.
     * @see CommandInterface::onSetDebugLevel
     */
    MONITOR_CMD_RESP onSetDebugLevel(SID doerId, MONITOR_DEBUG_LEVEL level);
protected:
    /**
     * @brief Get current count of connections to trap receivers.
     *
     * @return The count of connections to trap receivers.
     */
    size_t GetConnectionCount() const;
    /**
     * @brief Get current subscription count of specific  EM id and SIM index.
     *
     * @return The count of connections to trap receivers.
     */
    size_t GetEMSubscribeCount(MSGID msgId, uint32_t simIdx) const;
    size_t GetICDSubscribeCount(TRAP_TYPE type, MSGID msgId) const;

    /**
     * @brief Add a subscription count of specific EM id and SIM index.
     *
     * @return The count of connections to trap receivers.
     */
    void AddEMSubscribeCount(MSGID msgId, SID sessionId, uint32_t simIdx);
    void AddICDSubscribeCount(TRAP_TYPE type, MSGID msgId, SID sessionId);

    /**
     * @brief Remove a subscription count of specific EM id and SIM index.
     *
     * @return The count of connections to trap receivers.
     */
    void RemoveEMSubscribeCount(MSGID msgId, SID sessionId, uint32_t simIdx);
    void RemoveICDSubscribeCount(TRAP_TYPE type, MSGID msgId, SID sessionId);
    MSGID GetICDSubscribeMinIDValue(TRAP_TYPE type);
    MSGID GetICDSubscribeMaxIDValue(TRAP_TYPE type);

    /**
     * @brief Update current EM filter buffer to DHL via MAL APIs.
     *
     * @return The count of connections to trap receivers.
     */
    void UpdateCurrentEMFilterToDHL();
    MDM_LOG_CONTROL_RETCODE UpdateCurrentICDFilterToDHL(TRAP_TYPE type, MSGID start_id, MSGID end_id, bool enableAsyncResponse = false);

    /**
     * @brief The wrapper to callback @a m_logCtrl which is registered by modem info collector.
     *
     * @param logCtrlCmd [IN] The command for log control.
     *
     * @return the result of this operation, true:success
     *         false: fail.
     */
    MDM_LOG_CONTROL_RETCODE LogControl(MDM_LOG_CONTROL_CMD logCtrlCmd) const;

    /**
     * @brief The lock of session table.<br>
     *        Should request the lock before accessing @a m_sessionTable.
     */
    pthread_mutex_t m_sessionLock;

    /**
     * @brief The table which associates SIDs to the sessions.
     */
    std::map<SID, Session*> m_sessionTable;
    /**
     * @brief The table which associates MSGID to the used sessions.
     */
	std::map<MSGID, std::list<SID>> m_EMSubscriptionSessionList[4];   // <EM msg id, subsrcibed session ids> [SIM idx]

    /**
     * @brief The table which associates MSGID to the used sessions.
     */
	std::map<MSGID, std::list<SID>> m_ICDRecordSubscriptionSessionList;   // <ICD id, subsrcibed session ids>   (All SIM index together now)
	std::map<MSGID, std::list<SID>> m_ICDEventSubscriptionSessionList;   // <ICD id, subsrcibed session ids>   (All SIM index together now)



    std::map<std::string, android::sp<IMDMonitorClientCallback>> m_unmappingCallBackTable;

    /**
     * @brief The operation manager for modem EM filter operation.
     */
    EMFilterManager m_emFilterManager;

    /**
     * @brief The privilege manager for sessions.
     */
    PrivilegeManager m_privManager;

    /**
     * @brief Get session by SID
     *
     * @param sessionId [IN] Session ID.
     *
     * @return NULL: No such session.<br>
     *        !NULL: The pointer to the session in @a m_sessionTable.
     */
    Session *GetSession(SID sessionId);

    /**
     * @brief List Session ID.
     *
     * @param vSIDList [OUT] The session id list to return.
     */
    void ListSession(std::vector<SID> &vSIDList);

    /**
     * @brief The modem log controller registered by model info collector.
     */
    MDMLogController m_logCtrl;

    /**
     * @brief The version of parameters that @a m_logCtrl can accept.
     */
    unsigned m_logCtrlParamVer;

    /**
     * @brief The private data managed by modem info collector.
     */
    void *m_logCtrlPrivateData;

    /**
     * @brief used for debug.
    */
    RawDataDumperVirtual *m_rawDataDumper;
};
} //namespace libmdmonitor {

#endif
