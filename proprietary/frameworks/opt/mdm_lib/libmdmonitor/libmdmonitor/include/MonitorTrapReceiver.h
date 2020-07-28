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
 * @file MonitorTrapReceiver.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c MonitorTrapReceiver class used as a trap receiver service in the background thread.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __MONITOR_TRAP_RECEIVER_H__
#define __MONITOR_TRAP_RECEIVER_H__
#include "libmdmonitor.h"
#include "MCPReceiver.h"
#include "SocketListener.h"
#include "MDMonitorClientCallback.h"
namespace libmdmonitor {

using vendor::mediatek::hardware::mdmonitor::V1_0::implementation::MDMonitorClientCallback;


/**
 * @brief The prototype of callback function when @c MonitorTrapReceiver receives a trap.<br>
 *
 * @param param [IN] The data managed by the callee and<br>
 *                   should be set using @b MonitorTrapReceiver::SetTrapHandler.
 * @param timestamp [IN] The UNIX timestamp of the trap when modem monitor received.
 * @param type [IN] The trap type.
 * @param len [IN] The length of trap data @a data.
 * @param data [IN] Trap data.
 */
typedef void (*TrapHandler)(void *param, uint32_t timestamp, TRAP_TYPE type, size_t len,\
                            const uint8_t *data, size_t discard_count);

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class is used to run a trap receiver service in the background.<br>
 * Use @a SetTrapHandler to register the callback function to process the traps incoming.<br>
 *
 * @see PayloadHandlerInterface
 */
class MonitorTrapReceiver : public PayloadHandlerInterface
{
public:
    /**
     * @brief Create a trap receiver with the specified host name for specified session.
     * Session and receiver are a one-to-one correspondence.
     *
     * @param sessionId [IN] The session ID that this receiver belongs to.
     * @param szServerName [IN] The host name of the receiver.
     */
    MonitorTrapReceiver(SID sessionId, const char *szServerName);

    /**
     * @brief Cleanup
     */
    virtual ~MonitorTrapReceiver(); 

    /**
     * @brief Register the callback function to process the traps incoming. 
     *
     * @param handler [IN] The callback function to register.
     * @param param [IN] The parameter which will be passed into @a handler.
     */
	void SetTrapHandler(TrapHandler handler, void *param);

    /**
     * @brief The implementation of @b PayloadHandlerInterface::ProcessPayload<br>
     *        as a callback function to process the MCP payload data which will be<br>
     *        called by @a m_mcpReceiver.
     *
     * @param trap [IN] The trap received from @c TrapSender consists of timestamp, trap data and discard info.
     * @param not_used [OUT] Not used.
     *
     * @return true will be returned.
     */
	bool ProcessPayload(const MCPInfo &trap, MCPInfo &not_used);

protected:
    /**
     * @brief The callback function registered for trap processing.
     */
	TrapHandler m_trapHandler;

    /**
     * @brief The parameters managed by the callee and will be passed into @a m_trapHandler.
     */
	void *m_handlerParam;

    /**
     * @brief The socket listener instance to handle the socket connection from trap sender.
     */
    SocketListener m_socketListener;

    /**
     * @brief The instance is in charge of receiving data from socket connection<br>
     *        and decompose MCP data. 
     */
    MCPReceiver m_mcpReceiver;


    android::sp<MDMonitorClientCallback> m_pHidlCallback;
};
} //namespace libmdmonitor {

#endif
