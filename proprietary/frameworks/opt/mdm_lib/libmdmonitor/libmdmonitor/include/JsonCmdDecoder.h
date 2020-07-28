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
 * @file JsonCmdDecoder.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c JsonCmdDecoder class used to decode Json objects as modem monitor commands.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __JSON_CMD_DECODER_H__
#define __JSON_CMD_DECODER_H__
#include <map>
#include "ApiInfo.h"
#include "MCPInfo.h"
#include "MCPReceiver.h"
#include "json.h"
class CommandInterface;

using namespace std;

namespace libmdmonitor {
/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class is used to decode Json objects as modem monitor commands.
 *
 * @see CmdDecoder
 * @see JsonCmd
 */
class JsonCmdDecoder : public PayloadHandlerInterface
{
private:
    JsonCmdDecoder() {};
public:
    /**
     * @brief Create a empty Json command decoder and set the command runner instance to @a cmdRunnerInstance. 
     *
     * @param cmdRunnerInstance [IN] The pointer to command runner instance. Should not be NULL.
     */
    JsonCmdDecoder(CommandInterface *cmdRunner);

    /**
     * @brief Destructor.
     */
    virtual ~JsonCmdDecoder();

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
    bool ProcessPayload(const MCPInfo &cmd, MCPInfo &reply);
private:
    MONITOR_CMD_CODE GetCmdCode();
    bool GetIntParam(string paramNames, uint64_t *intParam);
    bool GetIntArrayParam(string paramNames, vector<uint64_t>* &intArrayParam);
    bool GetStringParam(string paramNames, string *stringParam);
    bool ComposeMCPReply(MONITOR_CMD_RESP errNo, const Json::Value &jData);
    void ParseMCPInfo();
    void HandleCmd();
    void Clear();
private:
    const MCPInfo *m_MCPdata;
    MCPInfo *m_MCPReply;
    ApiInfo *m_apiInfo;
    CommandInterface *m_CmdInterface;
    map<string, vector<uint64_t>> m_vecIntArrayParams;
    map<string, string> m_vecCStringParams;
};
} //namespace libmdmonitor {

#endif
