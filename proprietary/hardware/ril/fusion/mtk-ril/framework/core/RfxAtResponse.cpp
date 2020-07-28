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

#include "RfxAtResponse.h"

/*void RfxAtResponse::setFinalResponse(char* finalResponse) {
    RfxAtLine final = new RfxAtLine(finalResponse, NULL);
    m_finalResponse = final;
}*/

void RfxAtResponse::setFinalResponse(RfxAtLine* finalResponse) {
    m_finalResponse = finalResponse;
}

/*void RfxAtResponse::setIntermediates(char* line) {
    RfxAtLine intermediates = new RfxAtLine(line, NULL);
    if (m_pIntermediates == NULL) {
        m_pIntermediates = intermediates;
    } else {
        // find the last one position
        RfxAtLine* tmp = m_pIntermediates;
        while ((tmp=tmp->getNext()) != NULL) {
        }
        tmp->setNext(intermediates);
    }
}*/

void RfxAtResponse::setIntermediates(RfxAtLine* line) {
    if (m_pIntermediates == NULL) {
        m_pIntermediates = line;
    } else {
        // find the last one position
        RfxAtLine* tmp = m_pIntermediates;
        while (tmp->getNext() != NULL) {
            tmp = tmp->getNext();
        }
        tmp->setNext(line);
    }
}

AT_CME_Error RfxAtResponse::atGetCmeError() {
    int ret;
    int err;
    char *p_cur;

    if (m_success > 0)
        return CME_SUCCESS;

    if (m_finalResponse == NULL)
        return CME_ERROR_NON_CME;

    if (RfxMisc::strStartsWith(m_finalResponse->getLine(), "ERROR"))
        return CME_UNKNOWN;

    if (!RfxMisc::strStartsWith(m_finalResponse->getLine(), "+CME ERROR:"))
        return CME_ERROR_NON_CME;

    // make use of RfxAtLine
    m_finalResponse->atTokStart(&err);

    if (err < 0)
        return CME_ERROR_NON_CME;

    ret = m_finalResponse->atTokNextint(&err);

    if (err < 0)
        return CME_ERROR_NON_CME;

    return (AT_CME_Error)ret;
}

int RfxAtResponse::isATCmdRspErr() {
    return (m_err < 0 || 0 == m_success) ? 1: 0;
}

void RfxAtResponse::reverseIntermediates() {
    RfxAtLine *pcur, *pnext;

    pcur = m_pIntermediates;
    m_pIntermediates = NULL;

    while (pcur != NULL) {
        pnext = pcur->getNext();
        pcur->setNext(m_pIntermediates);
        m_pIntermediates = pcur;
        pcur = pnext;
    }
}

/*
** Indicate command send to mux or not
*/
bool RfxAtResponse::isAtSent() {
    return (m_err > 0);
}

/*
** Command has sent to modem and get response.
** This API will return success or failure of response
*/
bool RfxAtResponse::isAtResponseFail() {
    return (m_err < 0) ||
            (m_success <= 0);
}
