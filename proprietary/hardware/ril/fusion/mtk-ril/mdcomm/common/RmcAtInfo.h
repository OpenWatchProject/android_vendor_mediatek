/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef __RMC_AT_INFO_H__
#define __RMC_AT_INFO_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxBaseHandler.h"

/*****************************************************************************
 * Class RmcAtSendInfo
 *****************************************************************************/
class RmcAtSendInfo {
// External Method
public:
    // Get the AT comman string sent to modem
    //
    // RETURNS: the AT command string
    const String8 &getCmd() const {
        return m_cmd;
    }

// Constructor / Destructor
public:
    // Constructor with the at command string
    RmcAtSendInfo(
        String8 cmd    // [IN] The at command string sent to modem
    ) : m_cmd(cmd) {
    }

    // Destructor
    virtual ~RmcAtSendInfo() {
    }

// Overriable
public:
    virtual sp<RfxAtResponse> sendAt(
        RfxBaseHandler *handler    // [IN] the pointer of the RfxBaseHandler
    ) = 0;

// Implementation
private:
    String8 m_cmd;
};

/*****************************************************************************
 * Class RmcNoLineAtSendInfo
 *****************************************************************************/
class RmcNoLineAtSendInfo : public RmcAtSendInfo {
// Constructor / Desctrucotr
public:
    RmcNoLineAtSendInfo(String8 cmd) : RmcAtSendInfo(cmd) {
    }

    virtual ~RmcNoLineAtSendInfo() {
    }

// Override
public:
    virtual sp<RfxAtResponse> sendAt(RfxBaseHandler *handler) {
         return handler->atSendCommand(getCmd());
    }
};


/*****************************************************************************
 * Class RmcNumericAtSendInfo
 *****************************************************************************/
class RmcNumericAtSendInfo : public RmcAtSendInfo {
// Constructor / Desctrucotr
public:
    RmcNumericAtSendInfo(String8 cmd) : RmcAtSendInfo(cmd) {
    }

    virtual ~RmcNumericAtSendInfo() {
    }

// Override
public:
    virtual sp<RfxAtResponse> sendAt(RfxBaseHandler *handler) {
         return handler->atSendCommandNumeric(getCmd());
    }
};


/*****************************************************************************
 * Class RmcRawAtSendInfo
 *****************************************************************************/
class RmcRawAtSendInfo : public RmcAtSendInfo {
// Constructor / Desctrucotr
public:
    RmcRawAtSendInfo(String8 cmd) : RmcAtSendInfo(cmd) {
    }

    virtual ~RmcRawAtSendInfo() {
    }

// Override
public:
    virtual sp<RfxAtResponse> sendAt(RfxBaseHandler *handler) {
         return handler->atSendCommandRaw(getCmd());
    }
};


/*****************************************************************************
 * Class RmcSingleLineAtSendInfo
 *****************************************************************************/
class RmcSingleLineAtSendInfo : public RmcNoLineAtSendInfo {
// External Method
public:
    // get prefix of the at response
    //
    // RETURNS: the prefix of at response
    const String8 &getResponsePrefix() const {
        return m_responsePrefix;
    }

// Constructor / Desctructor
public:
    RmcSingleLineAtSendInfo(String8 cmd, String8 responsePrefix) :
            RmcNoLineAtSendInfo(cmd), m_responsePrefix(responsePrefix) {
    }

    virtual ~RmcSingleLineAtSendInfo() {
    }

// Override
public:
    virtual sp<RfxAtResponse> sendAt(RfxBaseHandler * handler) {
        return handler->atSendCommandSingleline(getCmd(), getResponsePrefix().string());
    }

// Implementation
private:
    String8 m_responsePrefix;
};


/*****************************************************************************
 * Class RmcMultiLineAtSendInfo
 *****************************************************************************/
class RmcMultiLineAtSendInfo : public RmcSingleLineAtSendInfo {
// Constructor / Desctructor
public:
    RmcMultiLineAtSendInfo(String8 cmd, String8 responsePrefix) :
            RmcSingleLineAtSendInfo(cmd, responsePrefix) {
    }

    virtual ~RmcMultiLineAtSendInfo() {
    }

// Override
public:
    virtual sp<RfxAtResponse> sendAt(RfxBaseHandler * handler) {
        return handler->atSendCommandMultiline(getCmd(), getResponsePrefix().string());
    }
};


/*****************************************************************************
 * Class RmcAtRspInfo
 *****************************************************************************/
class RmcAtRspInfo {
// External Method
public:
    // Get the AT command
    //
    // RETURNS: the AT command
    const String8 &getCmd() const {
        return m_cmd;
    }

    // Get the response of the AT
    //
    // RETURNS: the response of the AT
    const sp<RfxAtResponse> & getResponse() {
        return m_response;
    }

// Constructor / Destructor
public:
    RmcAtRspInfo(
        String8 cmd,                   // [IN] the AT command
        sp<RfxAtResponse> response     // [IN] the response of the AT command
    ) : m_cmd(cmd), m_response(response) {
    }

    virtual ~RmcAtRspInfo() {
    }

// Implementation
private:
    String8 m_cmd;
    sp<RfxAtResponse> m_response;
};


#endif /* __RMC_AT_INFO_H__ */
