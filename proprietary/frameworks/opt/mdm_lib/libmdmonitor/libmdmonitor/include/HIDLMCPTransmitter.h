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
 * @file HIDLMCPTransmitter.h
 *
 * @author mtk03685 (Roger Lo)
 *
 * @brief The header file defines @c HIDLMCPTransmitter class used to send MCP data out.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __HIDL_MCP_TRANSMITTER_H__
#define __HIDL_MCP_TRANSMITTER_H__
#include <unistd.h>
#include <pthread.h>
#include <vendor/mediatek/hardware/mdmonitor/1.0/IMDMonitorService.h>
#include "libmdmonitor.h"
#include "Transmitter.h"
#include "DataQueue.h"
#include <utils/Mutex.h>

namespace libmdmonitor {

using ::vendor::mediatek::hardware::mdmonitor::V1_0::IMDMonitorService;
using ::android::Mutex;


/**
 * @author mtk03685 (Roger Lo)
 *
 * @brief The class is used to send out MCP data through HIDL
 *
 * @see MCPHandler
 * @see Transmitter
 */
class HIDLMCPTransmitter : public Transmitter
{
public:
    /**
     * @brief Crate @c HIDLMCPTransmitter instance that with HIDL service function.
     *
     * @param callback [IN] The callback HIDL function.
     */
    HIDLMCPTransmitter(const char *szServiceName);

    /**
     * @brief Cleanup.
     */
    virtual ~HIDLMCPTransmitter();

    /**
     * @brief Send out MCP data through @a m_connection.
     * It is synchronized, so that one send operation at one time
     * to ensure @ppReply is correct.
     *
     * @param cmd [IN] The command in MCP format to send out.
     * @param ppReply [OUT] The address of a pointer to the reply in MCP format.
     * @param bStopFlag [IN] The address of a variable indicating the write operation should be stopped immediately.
     *
     * @return true: All MCP data was successfully sent.<br>
     *        false: Error or the write operation was cancelled.
     */
    virtual bool Send(const MCPInfo &cmd, MCPInfo **ppReply, volatile bool *bStopFlag = NULL);



    /**
     * @brief Check whether the connection is valid.
     * @return true: The connection is valid.<br>
     *        false: Otherwise.
     */
    virtual bool CheckValid();


    void serviceDied();    

    class HIDLMCPTransmitterRecipient : public android::hardware::hidl_death_recipient
    {
    public:
        HIDLMCPTransmitterRecipient(HIDLMCPTransmitter *transmitter):mTransmitter(transmitter){}
        virtual void serviceDied(uint64_t cookie, const android::wp<::android::hidl::base::V1_0::IBase>& who) {
            if(mTransmitter){
                mTransmitter->serviceDied();
            }
        }

    private:
        HIDLMCPTransmitter *mTransmitter;
    };

protected:

    void connectMDMHIDL();

    /**
     * @brief The server name of the host to connect.
     */
    char m_serviceName[MAX_SOCK_ADDR_LEN];


    // HIDL Hanlder
    android::sp<IMDMonitorService> m_connection;
    Mutex mLock;  // mutex lock for m_connection
    android::sp<HIDLMCPTransmitterRecipient> m_deathRecipient;

    // HIDL data queue
    DataQueue m_mcpRAWData;

    pthread_mutex_t m_sendLock;
};


} //namespace libmdmonitor {

#endif
