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

#include "RfxReader.h"
#include "RfxRilUtils.h"
#include "RfxChannelContext.h"
#include "RfxMclDispatcherThread.h"
#include "RfxMclMessage.h"
#include "RfxChannelManager.h"
#include "RfxMclStatusManager.h"
#include "RfxMessageId.h"
#include "RfxRawData.h"
#include "RfxStringData.h"
#include <libmtkrilutils.h>

static const char *s_smsUnsoliciteds[] = {
    "+CMT:",
    "+CDS:",
    "+CBM:",
    "+EIMSCMT:",
    "+EIMSCDS:",
    "+ECBMWAC:"
};

#define RFX_LOG_TAG "AT"

RfxReader::RfxReader(int fd, int channel_id, RfxChannelContext *context) :
        m_fd(fd), m_channel_id(channel_id), m_context(context),
        mName(NULL), m_pATBufferCur(NULL) {
        memset(&m_threadId, 0, sizeof(pthread_t));
        memset(m_aTBuffer, 0, (MAX_AT_RESPONSE+1));
}

bool RfxReader::threadLoop() {
    m_threadId = pthread_self();
    mName = RfxChannelManager::channelIdToString(m_channel_id);

    // start to read data from m_fd
    printLog(INFO, String8::format("threadLoop. RfxReader %s init, channel id = %d, fd = %d",
            mName, m_channel_id, m_fd));

    if ((m_channel_id%RIL_CHANNEL_OFFSET )== RIL_CMD_IMS) {
        readerLoopForFragData();
    } else {
        readerLoop();
    }
    return true;
}

void RfxReader::readerLoop() {
    int err = 0;

    m_pATBufferCur = m_aTBuffer;
    for (;; ) {
        const char *line;
        line = readline(m_aTBuffer);

        //RFX_LOG_D(LOG_TAG, "%s:%s", readerName, line);
        if (line == NULL)
            break;
        while((err = m_context->m_readerMutex.tryLock()) != 0){
            usleep(200 * 1000);
        }
        if (isSMSUnsolicited(line)) {
            char *line1;
            const char *line2;
            printLog(DEBUG, String8::format("SMS Urc Received!"));
            // The scope of string returned by 'readline()' is valid only
            // till next call to 'readline()' hence making a copy of line
            // before calling readline again.
            line1 = strdup(line);
            if (line1 == NULL) {
                printLog(ERROR, String8::format("malloc failed"));
                m_context->m_readerMutex.unlock();
                break;
            }
            line2 = readline(m_aTBuffer);

            if (line2 == NULL) {
                printLog(ERROR, String8::format("NULL line found in %s", mName));
                m_context->m_readerMutex.unlock();
                free(line1);
                break;
            }
            int index = 0;
            if ((index = needToHidenLog(line1)) >= 0) {
                printLog(INFO, String8::format("%s: line1:%s:***,line2:***", mName,
                        getHidenLogPreFix(index)));
            } else {
                printLog(INFO, String8::format("%s: line1:%s,line2:%s", mName, line1, line2));
            }
            RfxAtLine* atLine1 = new RfxAtLine(line1, NULL);
            RfxAtLine* atLine2 = new RfxAtLine(line2, NULL);
            handleUnsolicited(atLine1, atLine2);
            // free at RfxMclMessage deconstructor
            //delete(atLine1);
            //delete(atLine2);
            free(line1);
        } else {
            while((err = m_context->m_commandMutex.tryLock()) != 0){
                usleep(200 * 1000);
            }

            int index = 0;
            if ((index = needToHidenLog(line)) >= 0) {
                printLog(INFO, String8::format("AT< %s=*** (%s, tid:%lu)\n",
                        getHidenLogPreFix(index), mName, m_threadId));
            } else if (!strstr(line, ":") && isdigit(line[0])) {
                printLog(INFO, String8::format("AT< *** (%s, tid:%lu)\n", mName, m_threadId));
            } else {
                if (isLogReductionCmd(line)) {
                    printLog(DEBUG, String8::format("AT< %s (%s, tid:%lu)\n", line, mName,
                            m_threadId));
                } else {
                    printLog(INFO, String8::format("AT< %s (%s, tid:%lu)\n", line, mName,
                            m_threadId));
                }
            }

            processLine(line);
            m_context->m_commandMutex.unlock();
        }
        m_context->m_readerMutex.unlock();
        if (RfxRilUtils::isSimSwitchUrc(line)) {
            // Wait for sim switch to handle this urc before processing the next one
            while (RfxMclStatusManager::getNonSlotMclStatusManager()->getBoolValue(
                    RFX_STATUS_KEY_CAPABILITY_SWITCH_URC_CHANNEL, false)) {
                usleep(5 * 1000);
            }
        }
    }
    static Mutex isTrmMutex;
    static bool isTrm = false;
    printLog(ERROR, String8::format("%s Closed, trigger TRM! %d", mName, isTrm));
    // trigger TRM to reset telephony
    isTrmMutex.lock();
    if (isTrm == false) {
        isTrm = true;
        rfx_property_set("vendor.ril.mux.report.case", "2");
        rfx_property_set("vendor.ril.muxreport", "1");
    }
    isTrmMutex.unlock();
}

char* RfxReader::readline(char *buffer) {
    ssize_t count;

    char *p_read = NULL;
    char *p_eol = NULL;
    char *ret;

    /* this is a little odd. I use *s_ATBufferCur == 0 to
     * mean "buffer consumed completely". If it points to a character, than
     * the buffer continues until a \0
     */
    if (*m_pATBufferCur == '\0') {
        /* empty buffer */
        m_pATBufferCur = buffer;
        *m_pATBufferCur = '\0';
        p_read = buffer;
    } else { /* *s_ATBufferCur != '\0' */
        /* there's data in the buffer from the last read */
        // skip over leading newlines
        while (*m_pATBufferCur == '\r' || *m_pATBufferCur == '\n') {
            m_pATBufferCur++;
        }
        p_eol = findNextEOL(m_pATBufferCur);
        if (p_eol == NULL) {
            /* a partial line. move it up and prepare to read more */
            size_t len;

            len = strlen(m_pATBufferCur);
            memmove(buffer, m_pATBufferCur, len + 1);
            p_read = buffer + len;
            m_pATBufferCur = buffer;
        }
        /* Otherwise, (p_eol !- NULL) there is a complete line  */
        /* that will be returned the while () loop below        */
    }
    while (p_eol == NULL) {
        if (0 == MAX_AT_RESPONSE - (p_read - buffer)) {
            printLog(ERROR, String8::format("ERROR: Input line exceeded buffer\n"));
            /* ditch buffer and start over again */
            m_pATBufferCur = buffer;
            *m_pATBufferCur = '\0';
            p_read = buffer;
        }
        do {
            if (RfxRilUtils::isUserLoad() != 1) {
                printLog(DEBUG, String8::format("AT read start\n"));
            }
            count = read(m_fd, p_read, MAX_AT_RESPONSE - (p_read - buffer));
            if (RfxRilUtils::isUserLoad() != 1) {
                if (count < 0) {
                    printLog(DEBUG, String8::format("AT read end: %zd (err: %d - %s)\n", count,
                            errno, strerror(errno)));
                } else {
                    printLog(DEBUG, String8::format("AT read end: %zd\n", count));
                }
            }
        } while (count < 0 && errno == EINTR);

        if (count > 0) {
            // AT_DUMP("<< ", p_read, count);
            // m_context->readCount += count;

            p_read[count] = '\0';
            // skip over leading newlines
            while (*m_pATBufferCur == '\r' || *m_pATBufferCur == '\n') {
                m_pATBufferCur++;
            }

            p_eol = findNextEOL(m_pATBufferCur);
            p_read += count;
        } else if (count <= 0) {
            /* read error encountered or EOF reached */
            if (count == 0)
                printLog(ERROR, String8::format("atchannel: EOF reached"));
            else
                printLog(ERROR, String8::format("atchannel: read error %s", strerror(errno)));
            return NULL;
        }
    }

    /* a full line in the buffer. Place a \0 over the \r and return */

    ret = m_pATBufferCur;
    *p_eol = '\0';
    if (m_pATBufferCur[0] == '>' && m_pATBufferCur[1] == ' ' &&
            m_pATBufferCur[2] == '\0') {
        printLog(DEBUG, String8::format("atchannel: This is sms prompt!"));
        m_pATBufferCur = p_eol + 1; /* this will always be <= p_read,    */
        m_pATBufferCur[0] = '\0';
    } else {
        m_pATBufferCur = p_eol + 1; /* this will always be <= p_read,    */
    }
    /* and there will be a \0 at *p_read */

    return ret;
}

void RfxReader::processLine(const char *line) {
    sp<RfxAtResponse> response = m_context->getResponse();
    RfxAtLine* atLine = new RfxAtLine(line, NULL);

    int isIntermediateResult = 0;
    bool isNumericSet = false;
    int slotId = m_channel_id/RIL_PROXY_OFFSET;
    int isRequestAck = 0;

    if (RFX_MSG_TYPE_NONE == m_context->getType()) {
        /* no command pending */
        handleUnsolicited(atLine, NULL);
        // free at RfxMclMessage deconstructor
        // delete(atLine);
        return;
    } else if (atLine->isAckResponse()) {
        isRequestAck = 1;
        RFX_LOG_D(RFX_LOG_TAG, "receive ACK (%s)\n", line);
    } else {
        switch (response->getCommandType()) {
        case NO_RESULT:
            break;
        case NUMERIC:
            if (response->getIntermediates() == NULL
                && isdigit(atLine->getLine()[0])
                ) {
                response->setIntermediates(atLine);
                isIntermediateResult = 1;
            } else {
                /* either we already have an intermediate response or
                 *     the line doesn't begin with a digit */
                //handleUnsolicited(line,p_channel);
            }
            break;
        case SINGLELINE:
            if (response->getIntermediates() == NULL
                && RfxMisc::strStartsWith(atLine->getLine(), response->getResponsePrefix())
                ) {
                response->setIntermediates(atLine);
                isIntermediateResult = 1;
            } else {
                /* we already have an intermediate response */
                //handleUnsolicited(line,p_channel);
            }
            break;
        case MULTILINE:
            if (RfxMisc::strStartsWith(atLine->getLine(), response->getResponsePrefix())) {
                response->setIntermediates(atLine);
                isIntermediateResult = 1;
            } else {
                //handleUnsolicited(line,p_channel);
            }
            break;
        /* atci start */
        case RAW:
            isNumericSet = RfxMclStatusManager::getMclStatusManager(slotId)->getBoolValue(
                    RFX_STATUS_KEY_ATCI_IS_NUMERIC, false);
            if (isNumericSet) {
                if (!atLine->isFinalResponseSuccessInNumeric() &&
                        !atLine->isFinalResponseErrorInNumeric()) {
                    response->setIntermediates(atLine);
                    isIntermediateResult = 1;
                }
            } else {    //isNumericSet is false
                if (!atLine->isFinalResponseSuccess() &&
                        !atLine->isFinalResponseErrorEx(m_channel_id) &&
                        !atLine->isIntermediatePattern()) {
                    response->setIntermediates(atLine);
                    isIntermediateResult = 1;
                }
            }
            break;
        /* atci end */
        default: /* this should never be reached */
            printLog(ERROR, String8::format("Unsupported AT command type %d\n",
                    response->getCommandType()));
            //handleUnsolicited(line,p_channel);
            break;
        }
    }

    if (isRequestAck) {
        response->setAck(1);
        handleRequestAck();
    } else if (isIntermediateResult) {
        /* No need to run the following code*/
    } else if (atLine->isFinalResponseSuccess()) {
        response->setSuccess(1);
        handleFinalResponse(atLine);
    } else if (atLine->isFinalResponseErrorEx(m_channel_id)) {
        response->setSuccess(0);
        handleFinalResponse(atLine);
    } else if (atLine->isIntermediatePattern()) {
        response->setSuccess(1);
        handleFinalResponse(atLine);
    } else if (isNumericSet == 1) {
        if (atLine->isFinalResponseSuccessInNumeric()) {
            response->setSuccess(1);
            handleFinalResponse(atLine);
        } else if (atLine->isFinalResponseErrorInNumeric()) {
            response->setSuccess(0);
            handleFinalResponse(atLine);
        }
    } else {
        handleUnsolicited(atLine, NULL);
    }
    // free at RfxAtResponse deconstructor
    // delete(atLine);
}

void RfxReader::handleUnsolicited(RfxAtLine* line1, RfxAtLine* line2) {
    // create Message and set to MclDispatcherThread
    sp<RfxMclMessage> msg = RfxMclMessage::obtainRawUrc(m_channel_id, line1, line2);
    RfxMclDispatcherThread::waitLooper();
    RfxMclDispatcherThread::enqueueMclMessage(msg);

    // send each raw urc to atci module
    int slotId = RfxMclStatusManager::getNonSlotMclStatusManager()->
            getIntValue(RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT,0);
    sp<RfxMclMessage> atciMsg = RfxMclMessage::obtainEvent(RFX_MSG_EVENT_RAW_URC,
            RfxStringData(msg->getRawUrc()->getLine(), strlen(msg->getRawUrc()->getLine())),
            RIL_CMD_PROXY_6, m_channel_id/RIL_CHANNEL_OFFSET);
    RfxMclDispatcherThread::enqueueMclMessage(atciMsg);
}

void RfxReader::handleFinalResponse(RfxAtLine* line) {
    sp<RfxAtResponse> outResponse = m_context->getResponse();
    outResponse->setFinalResponse(line);
    m_context->m_commandCondition.signal();
}

int RfxReader::isSMSUnsolicited(const char *line) {
    size_t i;
    for (i = 0; i < NUM_ELEMS(s_smsUnsoliciteds); i++) {
        if (RfxMisc::strStartsWith(line, s_smsUnsoliciteds[i])) {
            return 1;
        }
    }
    return 0;
}

char* RfxReader::findNextEOL(char *cur) {
    if (cur[0] == '>' && cur[1] == ' ' && cur[2] == '\0')
        /* SMS prompt character...not \r terminated */
        return cur + 2;

    // Find next newline
    while (*cur != '\0' && *cur != '\r' && *cur != '\n') {
        cur++;
    }

    return *cur == '\0' ? NULL : cur;
}

void RfxReader::readerLoopForFragData() {
    int err = 0;
    int count = 0;
    unsigned char *header = (unsigned char *) calloc(RfxFragmentEncoder::HEADER_SIZE+1, sizeof(char));
    if (header == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "OOM");
        return;
    }
    char *aggregateData = NULL;
    char *tmpData;

    RfxFragmentData fragData;
    size_t dataReceived = 0;
    int writeP = 0;

    for (;;) {
        do {
            count = read(m_fd, header, RfxFragmentEncoder::HEADER_SIZE);
            if (count < 0 && errno != EINTR) {
                printLog(ERROR, String8::format("ViLTE read end: %d (err: %d - %s)\n", count,
                        errno, strerror(errno)));
                free(header);
                return;
            } else {
                printLog(DEBUG, String8::format("ViLTE read end: %d, %s\n", count, header));
            }
        } while (count < 0 && errno == EINTR);

        // For non-ViLte project, imspty will transform to dev/null
        // It will trigger read with count=0, the data can be ignored
        if (count == 0) {
            continue;
        }

        fragData = RfxFragmentEncoder::decodeHeader(header);
        aggregateData = (char *) calloc(fragData.getDataLength(), sizeof(char));
        if (aggregateData == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "OOM");
            free(header);
            return;
        }
        tmpData = (char *) calloc(RfxFragmentEncoder::MAX_FRAME_SIZE, sizeof(char));
        if (tmpData == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "OOM");
            free(aggregateData);
            free(header);
            return;
        }
        writeP = 0;
        printLog(INFO, String8::format("fragData.getDataLength(): %zu", fragData.getDataLength()));

        size_t readTimes = (fragData.getDataLength()/RfxFragmentEncoder::MAX_FRAME_SIZE);
        size_t remain = fragData.getDataLength() % RfxFragmentEncoder::MAX_FRAME_SIZE;
        size_t readTmp = 0;
        while (readTimes) {
            while (readTmp < RfxFragmentEncoder::MAX_FRAME_SIZE) {
                count = read(m_fd, tmpData, RfxFragmentEncoder::MAX_FRAME_SIZE-readTmp);
                printLog(DEBUG, String8::format("read count:%d, readTmp:%zu, writeP: %d",
                        count, readTmp, writeP));
                if (count < 0 && errno == EINTR) {
                    printLog(ERROR, String8::format("ViLTE read end: %d (err: %d - %s)\n", count,
                            errno, strerror(errno)));
                    continue;
                } else if (count < 0) {
                    printLog(ERROR, String8::format("ViLTE read end: %d (err: %d - %s)\n", count,
                            errno, strerror(errno)));
                    free(aggregateData);
                    free(tmpData);
                    free(header);
                    return;
                } else {
                    memcpy((void*)(aggregateData+writeP), (void*)tmpData, count);
                    writeP += count;
                    readTmp += count;
                    memset(tmpData, 0, RfxFragmentEncoder::MAX_FRAME_SIZE);

                    printLog(INFO, String8::format("ViLTE read readtimes: %zu, remain: %zu, count: %d",
                            readTimes, remain, count));
                }
            }
            readTmp = 0;
            readTimes--;
        }

        while (readTmp < remain) {
            count = read(m_fd, tmpData, remain-readTmp);
            printLog(DEBUG, String8::format("remain read count:%d, readTmp:%zu", count, readTmp));
            if (count < 0 && errno == EINTR) {
                printLog(ERROR, String8::format("ViLTE read end: %d (err: %d - %s)\n", count,
                        errno, strerror(errno)));
                continue;
            } else if (count < 0) {
                printLog(ERROR, String8::format("ViLTE read end: %d (err: %d - %s)\n", count,
                        errno, strerror(errno)));
                free(aggregateData);
                free(tmpData);
                free(header);
                return;
            } else {
                memcpy((void*)(aggregateData+writeP), (void*)tmpData, count);
                writeP += count;
                readTmp += count;
                printLog(INFO, String8::format("ViLTE read count: %d", count));
            }
        }

        printLog(INFO, String8::format("ViLTE read total count: %d", writeP));
        handleUserDataEvent(fragData.getClientId(), aggregateData, fragData.getDataLength());
        free(aggregateData);
        free(tmpData);
    }
    free(header);
}

void RfxReader::handleUserDataEvent(int clientId, char *data, size_t length) {
    printLog(INFO, String8::format("handleUserDataEvent, len:%zu, client %d", length, clientId));
    // create Message and set to MclDispatcherThread
    sp<RfxMclMessage> msg = RfxMclMessage::obtainEvent(
            RFX_MSG_EVENT_IMS_DATA,
            RfxRawData((void *)data, length),
            RIL_CMD_PROXY_IMS,
            RIL_CMD_PROXY_IMS/RIL_CHANNEL_OFFSET,
            clientId);
    RfxMclDispatcherThread::enqueueMclMessage(msg);
}

void RfxReader::handleRequestAck() {
    m_context->m_commandCondition.signal();
}

void RfxReader::setChannelId(int channelId) {
    m_channel_id = channelId;
    mName = RfxChannelManager::channelIdToString(m_channel_id);
}

void RfxReader::printLog(int level, String8 log) {
    RfxRilUtils::printLog(level, String8::format("%s", RFX_LOG_TAG), log,
            m_channel_id/RIL_CHANNEL_OFFSET);
}
