/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef __RFX_AT_RESPONSE_H__
#define __RFX_AT_RESPONSE_H__

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "utils/RefBase.h"
#include "RfxAtLine.h"
#include "RfxDefs.h"
#include "RfxMisc.h"

using ::android::RefBase;

class RfxAtResponse : public virtual RefBase {
    public:
        RfxAtResponse() :
                m_type(INIT),
                m_success(-1),
                m_ack(0),
                m_finalResponse(NULL),
                m_pIntermediates(NULL),
                m_err(0),
                m_responsePrefix(NULL) {
        }
        RfxAtResponse(AtCommandType type, int success, RfxAtLine* finalResponse, RfxAtLine* line,
                int err, const char *responsePrefix) :
                m_type(type),
                m_success(success),
                m_ack(0),
                m_finalResponse(finalResponse),
                m_pIntermediates(line),
                m_err(err),
                m_responsePrefix(responsePrefix) {
        }
        RfxAtResponse(AtCommandType type, const char *responsePrefix) :
            m_type(type),
            m_success(-1),
            m_ack(0),
            m_finalResponse(NULL),
            m_pIntermediates(NULL),
            m_err(0),
            m_responsePrefix(responsePrefix) {
        }
        ~RfxAtResponse() {
            if (m_pIntermediates) {
                delete(m_pIntermediates);
            }
            if (m_finalResponse) {
                delete(m_finalResponse);
            }
        };

    public:
        AtCommandType getCommandType() const {
            return m_type;
        }
        void setCommandType(AtCommandType type) {
            m_type = type;
        }
        int getSuccess() const {
            return m_success;
        }
        void setSuccess(int success) {
            m_success = success;
        }
        RfxAtLine* getFinalResponse() const {
            return m_finalResponse;
        }
        // void setFinalResponse(char* finalResponse);
        void setFinalResponse(RfxAtLine* finalResponse);
        RfxAtLine* getIntermediates() const  {
            return m_pIntermediates;
        }
        // void setIntermediates(char* line);
        void setIntermediates(RfxAtLine *line);
        int getError() const {
            return m_err;
        }
        void setError(int err) {
            m_err = err;
        }
        int getIsAck() const {
            return m_ack;
        }
        void setAck(int ack) {
            m_ack = ack;
        }

        const char *getResponsePrefix() const {
            return m_responsePrefix;
        }
        /*void setResponsePrefix(char *responsePrefix) {
            m_responsePrefix = responsePrefix;
        }*/
        AT_CME_Error atGetCmeError();
        int isATCmdRspErr();
        void reverseIntermediates();
        bool isAtSent();
        bool isAtResponseFail();

    private:
        AtCommandType m_type;
        int m_success; /* true if final response indicates success (eg "OK") */
        int m_ack; /* true if it is Ack response */
        RfxAtLine *m_finalResponse; /* eg OK, ERROR */
        RfxAtLine *m_pIntermediates; /* any intermediate responses */
        int m_err;
        const char *m_responsePrefix;
};
#endif
