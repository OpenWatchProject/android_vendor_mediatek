
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
 * @file TrapSender.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c TrapSender class used to send traps to remote receivers inter process.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */
#ifndef __TRAP_SENDER_H__
#define __TRAP_SENDER_H__
#include <pthread.h>
#include <set>
#include <vendor/mediatek/hardware/mdmonitor/1.0/IMDMonitorClientCallback.h>
#include "libmdmonitor.h"
#include "CircularBufEx.h"
namespace libmdmonitor {
class Transmitter;
using ::vendor::mediatek::hardware::mdmonitor::V1_0::IMDMonitorClientCallback;


/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class is used to send traps to remote receivers inter process.
 */
class TrapSender 
{
public:
    /**
     * @brief Create a empty trap sender.
     */
    TrapSender();

    /**
     * @brief Cleanup.
     */
    virtual ~TrapSender();

    /**
     * @brief Check-in traps into the trap sender.<br>
     *        The function only copy @a data into internal data queue @a m_txQueue and so is non-blocking.
     *
     * @param trapType [IN] The trap type.
     * @param data [IN] The trap data to send.
     * @param nLen [IN] The length of @a data.
     */
    void CheckInTrap(TRAP_TYPE trapType, const unsigned char *data, size_t nLen);

    /**
     * @brief Check whether the remote receivers is connected or not. 
     *
     * @return true: The remote receiver is connected.<br>
     *        false: The remote receiver is disconnected.
     */
    bool IsConnected() const { return m_isConnected; }

    /**
     * @brief Check whether the given trap type and message was subscribed. 
     *
     * @param frameInfo [IN] The frame info contains the trap type and message ID.
     *
     * @return true: The trap type and message was subscribed.<br>
     *        false: Otherwise.
     */
    bool IsSubscribed(const FRAME_INFO &frameInfo) const; 

    /**
     * @brief Subscribe message. 
     *
     * @param type [IN] The trap type.
     * @param msgId [IN] The message ID.
     *
     * @return true.
     */
    bool Subscribe(TRAP_TYPE type, MSGID msgId);

    /**
     * @brief Unsubscribe message. 
     *
     * @param type [IN] The trap type.
     * @param msgId [IN] The message ID.
     *
     * @return true.
     */
    bool Unsubscribe(TRAP_TYPE type, MSGID msgId);

    /**
     * @brief Start sending traps. 
     *
     * @return true: Success.<br>
     *        false: Errors.
     */
    bool StartTx();

    /**
     * @brief Stop sending traps. 
     */
    void StopTx();

    /**
     * @brief Setup a specific trap sender. 
     *
     * @param type [IN] The transmitter type.
     * @param szServerName [IN] The host name to connect.
     *
     * @return true: Success.<br>
     *        false: Errors.
     */
    bool SetupTx(TRANSMITTER_TYPE type, const char *szServerName, const android::sp<IMDMonitorClientCallback>& callback);

    void GetSubscriptionList(TRAP_TYPE trapType, std::vector<MSGID> &vList);
    const char *GetServerName() const { return m_serverName; }
protected:
    /**
     * @brief The flag that indicates whether the receiver is connected.
     */
    bool m_isConnected;

#define BULK_READ_BUFFER_CHECKOUT_POLLING_INTERVAL 10
    /**
     * @brief The buffer for sender thread to store the data ready for sending out.
     */
    unsigned char m_sendBuffer[BULK_READ_BUFFER_SIZE];

    /**
     * @brief The data size in @a m_sendBuffer.
     */
    size_t m_sendBufferDataSize;

    /**
     * @brief The internal data queue.
     */
    CircularBufEx m_txQueue;

    /**
     * @brief The sender thread ID.
     */
    pthread_t m_senderThrId;

    /**
     * @brief The lock for accessing internal data queue @a m_txQueue.
     */
    pthread_mutex_t m_queueLock;

    /**
     * @brief notify when data comes @a m_txQueue.
    */
    pthread_cond_t m_queueCondition;

    /**
     * @brief The lock for operatrion on @a m_transmitter.
     */
    pthread_mutex_t m_transmitterLock;

    /**
     * @brief The main loop for sender thread to send data out. 
     */
    void Send();

    /**
     * @brief Create the sender thread. 
     *
     * @return true: Success.
     *        false: Errors.
     */
    bool CreateThread();

    /**
     * @brief Wait the sender thread terminated. 
     *
     * @return true.
     */
    bool WaitThreadExit();

    /**
     * @brief Thr wrapper function of @b Send for sender thread. 
     *
     * @param arg [IN] The trap sender instance.
     *
     * @return NULL
     */
    static void *ThrTxWrapper(void *arg);

    /**
     * @brief The subscription table.
     */
    std::set<MSGID> m_trap[TRAP_TYPE_SIZE];

    /**
     * @brief The address of the transmitter.
     */
    Transmitter *m_transmitter;

    /**
     * @brief The flag to notify sender thread to terminate.
     */
    volatile bool m_notifyThreadStop;

    /**
     * @brief The discard counts of each trap types.
     */
    uint32_t m_discardCount[TRAP_TYPE_SIZE];


    /**
     * @brief The server name of the host to connect.
     */
    char m_serverName[MAX_SOCK_ADDR_LEN];
};
} // namespace libmdmonitor {

#endif
