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

#ifndef __RFX_AT_LINE_H__
#define __RFX_AT_LINE_H__

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "RfxDefs.h"
#include "RfxLog.h"

class RfxAtLine {
    public:
        RfxAtLine() : m_line(NULL), m_pNext(NULL) {
        }

        RfxAtLine(const char* line, RfxAtLine* next);

        // copy constructor
        RfxAtLine(const RfxAtLine &other);

        ~RfxAtLine();

        RfxAtLine &operator=(const RfxAtLine &other);
    public:
        RfxAtLine* getNext() const {
            return m_pNext;
        }
        void setNext(RfxAtLine* next) {
            m_pNext = next;
        }
        char *getLine() const {
            return m_line;
        }
        void setLine(char* line) {
            m_line = line;
        }
        char *getCurrentLine() {
            return m_pCur;
        }
        void atTokStart(int *err);
        int atTokNextint(int *err);
        int atTokNexthexint(int *err);
        bool atTokNextbool(int *err);
        char* atTokNextstr(int *err);
        int atTokHasmore();
        char* atTokChar(int *err);
        void atTokEqual(int *err);
        long long atTokNextlonglong(int *err);
        int isFinalResponseSuccess();
        int isFinalResponseErrorEx(int channel_id);
        int isIntermediatePattern();
        bool isFinalResponseSuccessInNumeric();
        bool isFinalResponseErrorInNumeric();
        bool isAckResponse();

    private:
        void skipWhiteSpace();
        void skipNextComma();
        int atTokNextintBase(int base, int  uns, int *err);
        long long atTokNextlonglongBase(int base, int  uns, int *err);
        char* nextTok();

    private:
        char *m_line; // should dynamic allocate memory?
        RfxAtLine *m_pNext;
        char *m_pCur; // current position, initialize at atTokStart
};
#endif
