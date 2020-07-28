/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.ims.rcsua.service;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.os.SystemProperties;

import com.mediatek.ims.rcsua.Configuration;
import com.mediatek.ims.rcsua.service.utils.Logger;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.HashSet;

/**
 * The Class RcsProxySipHandler.
 */
public class SipHandler {

    static final String TCP_PROTOCOL = "tcp";
    static final String UDP_PROTOCOL = "udp";
    static final String TLS_PROTOCOL = "tls";
    static final String SCTP_PROTOCOL = "sctp";

    private static HashMap<String, Integer> headerCache;

    static {
        headerCache = new HashMap<String, Integer>();

        headerCache.put(SipHeader.FNAME_VIA, SipHeader.VIA);
        headerCache.put(SipHeader.CNAME_VIA, SipHeader.VIA);

        headerCache.put(SipHeader.FNAME_CALLID, SipHeader.CALLID);
        headerCache.put(SipHeader.CNAME_CALLID, SipHeader.CALLID);

        headerCache.put(SipHeader.FNAME_EVENT, SipHeader.EVENT);
        headerCache.put(SipHeader.CNAME_EVENT, SipHeader.EVENT);

        headerCache.put(SipHeader.NAME_CSEQ, SipHeader.CSEQ);

        headerCache.put(SipHeader.FNAME_CONTENT_LENGTH, SipHeader.CONTENT_LENGTH);
        headerCache.put(SipHeader.CNAME_CONTENT_LENGTH, SipHeader.CONTENT_LENGTH);
    }

    static SipHandler createInstance(RuaAdapter adapter) {
        if (INSTANCE == null) {
            synchronized (SipHandler.class) {
                if (INSTANCE == null) {
                    INSTANCE = new SipHandler(adapter);
                }
            }
        }

        return INSTANCE;
    }

    static void destroyInstance() {
        if (INSTANCE != null) {
            INSTANCE.dispose();
            INSTANCE.spThread.quitSafely();
            INSTANCE = null;
        }
    }

    void handleSipMessageReceived(RuaAdapter.RcsUaEvent event) {
        int reqId = event.getRequestId();
        int msgId = event.getInt();
        int connId = event.getInt();
        int msgLen = event.getInt();
        // discard C string terminate character
        byte[] rawMessage = event.getBytes(msgLen-1);

        logger.debug("messageReceived: "
                        + "requestId[" + String.format("%#x", msgId) + "],"
                        + "connId[" + String.format("%#x", connId) + "],"
                        + "length[" + msgLen + "]");

        if (processor != null) {
            processor.handleSipReceived(msgId, connId, rawMessage);
        }
    }

    /**
     * Send sip msg.
     *
     * @param message the sip msg buffer
     * @return true, if successful
     */
    void sendMessage(byte[] message, int channelId) {

        if (processor != null) {
            processor.handleSipSending(message, channelId);
        }
    }

    void registerListener(Listener listener) {
        listeners.add(listener);
        if (listener.standalonePresence && stanaloneListener == null)
            stanaloneListener = listener;
    }

    void unregisterListener(Listener listener) {
        listeners.remove(listener);
        if (stanaloneListener == listener)
            stanaloneListener = null;
    }

    /**
     * Instantiates a new rcs proxy sip handler.
     *
     * @param adapter the context
     */
    SipHandler(RuaAdapter adapter) {
        this.adapter = adapter;
        if (sipTraceEnabled) {
            if (SystemProperties.get("ro.build.type").equals("user"))
                sipTraceEnabled = false;
        }
        spThread = new HandlerThread("SipProcessorThread");
        spThread.start();
        processor = new Processor(spThread);
        transactMgr = new TransactionManager(adapter, processor, logger);
    }

    void dispose() {
        logger.debug("dispose");
        transactMgr.dump();
        transactMgr.clearAllTransactions();
    }

    private void notifySipMessageReceived(SipMessage sip) {
        notifySipMessageReceived(sip, 0);
    }

    private void notifySipMessageReceived(SipMessage sip, int channelId) {
        boolean presenceMessage = sip.isPresenceEvent();

        for (Listener listener : listeners) {
            // If standalone presence client exist, presence only
            // request will only route to it.
            if (presenceMessage
                    && stanaloneListener != null
                    && listener != stanaloneListener)
                continue;
            // Non-presence message never route to standalone presence client
            if (listener.standalonePresence && sip.isRestricted())
                continue;
            // Response only route to the client which originate requset
            if (channelId != 0 && listener.hashCode() != channelId)
                continue;
            listener.messageReceived(sip.transport, sip.retouching());
        }
    }

    private void printSipData(byte[] sipData, boolean incoming) {
        if (sipTraceEnabled) {
            String sip = new String(sipData);
            // remove body part
            String[] buf = sip.split("\r\n\r\n");
            if (buf.length == 0) {
                return;
            }
            String headers = buf[0];
            if (incoming)
                SIPlogger.info("Incoming SIP");
            else
                SIPlogger.info("Outgoing SIP");
            for (String header : headers.split("\r\n")) {
                SIPlogger.info(header.replaceAll(numberPattern, "*"));
            }
        }
    }

    class Processor extends Handler {

        Processor(HandlerThread thread) {
            super(thread.getLooper());
        }

        void handleSipReceived(int reqId, int connId, byte[] msgData) {
            Message.obtain(this, INCOMING_NEW_SIP, reqId, connId, msgData)
                    .sendToTarget();
        }

        void handleSipSending(byte[] msgData, int channelId) {
            Message.obtain(this, OUTGOING_NEW_SIP, channelId, 0, msgData)
                    .sendToTarget();
        }

        @Override
        public void handleMessage(Message msg) {
            int reqId = msg.arg1;
            int connId = msg.arg2;
            byte[] msgData = null;
            Transaction transaction = null;
            SipMessage sip = null;

            switch (msg.what) {
            case INCOMING_NEW_SIP:

                msgData = (byte[]) msg.obj;
                printSipData(msgData, true);

                sip = SipMessage.create(msgData, logger);
                if (sip != null) {
                    if (sip.isRequest()) {
                        transactMgr.incomingRequest(sip, reqId, connId);
                    } else {
                        transaction = transactMgr.incomingResponse(sip, reqId, connId);
                    }
                    if (transaction != null && transaction.channelId != 0)
                        notifySipMessageReceived(sip, transaction.channelId);
                    else
                        notifySipMessageReceived(sip);
                }

                break;

            case OUTGOING_NEW_SIP:

                msgData = (byte[]) msg.obj;
                printSipData(msgData, false);

                sip = SipMessage.create(msgData, logger);
                if (sip != null) {
                    if (sip.isRequest()) {
                        int channelId = msg.arg1;
                        transaction = transactMgr.outgoingRequest(sip, channelId);
                    } else {
                        transaction = transactMgr.outgoingResponse(sip);
                    }

                    // Check transaction here due to might be possible
                    // droping sending request after transaction processing
                    if (transaction != null) {
                        RuaAdapter.RcsUaEvent event = new RuaAdapter.RcsUaEvent(
                                RuaAdapter.CMD_SEND_SIP_MSG);
                        event.putInt(transaction.requestId);
                        event.putInt(transaction.connId);
                        // "+1" is MUST for IMS stack!
                        event.putInt(msgData.length + 1);
                        event.putBytes(msgData);

                        adapter.sendMsgToRCSUAProxy(event);
                    }
                }

                break;

            case TRANSACTION_TIMEOUT:

                transaction = (Transaction)msg.obj;
                if (transaction != null)
                    logger.debug("TRANSACTION_TIMEOUT:" + transaction.ID);
                transactMgr.removeTransaction(transaction);

                break;
            }
        }

        static final int INCOMING_NEW_SIP = 1;
        static final int OUTGOING_NEW_SIP = 2;
        static final int TRANSACTION_TIMEOUT = 3;
    }

    /**
     * RcsSIPEventListener
     */
    public static abstract class Listener {

        /**
         * Notify sip message.
         *
         * @param sipMessage the sip msg response
         */
        public abstract void messageReceived(int transport, byte[] sipMessage);

        protected boolean standalonePresence;
    }

    /**
     * The Class to wrapping SIP message parsing result.
     */
    private static class SipMessage {

        static final int INVITE = 1;
        static final int BYE = 2;
        static final int ACK = 3;
        static final int CANCEL = 4;
        static final int REGISTER = 5;
        static final int UPDATE = 6;
        static final int SUBSCRIBE = 7;
        static final int PUBLISH = 8;
        static final int OPTIONS = 9;
        static final int MESSAGE = 10;
        static final int NOTIFY = 11;

        SipMessage() {
            request = true;
            needRetouch = false;
            method = 0;
            statusCode = 0;
            cseqMethod = 0;
            transport = Configuration.TCP;
        }

        SipMessage(boolean request) {
            this();
            this.request = request;
        }

        static SipMessage create(byte[] sipData, Logger logger) {
            SipMessage sip = null;
            LineReader reader = new LineReader(sipData, logger);
            Line line;
            boolean viaFound = false, callIdFound = false;

            line = reader.read();
            if (line == null || line.getLength() <= 0)
                return sip;

            sip = line.processFirstLine();

            while ((line = reader.read()) != null && line.getLength() >= 0) {
                if (line.getLength() == 0) {
                    // empty line is the separator between header and body,
                    // we want to check if Content-Length is the last header here
                    sip.setHeadersEndPos(reader.getCurrPosition());
                    break;
                }

                switch (line.getHeaderName()) {
                    case SipHeader.VIA:
                        if (viaFound)
                            break;

                        viaFound = true;
                        String transport = line.getTransport();
                        if (TCP_PROTOCOL.equalsIgnoreCase(transport))
                            sip.transport = Configuration.TCP;
                        else if (UDP_PROTOCOL.equalsIgnoreCase(transport))
                            sip.transport = Configuration.UDP;
                        else if (TLS_PROTOCOL.equalsIgnoreCase(transport))
                            sip.transport = Configuration.TLS;
                        else if (SCTP_PROTOCOL.equalsIgnoreCase(transport))
                            sip.transport = Configuration.SCTP;
                        String branch = line.getBranchId();
                        if (branch != null) {
                            sip.branchId = branch;
                        }

                        break;

                    case SipHeader.CALLID:

                        if (callIdFound)
                            break;
                        String callId = line.getCallId();
                        if (callId != null) {
                            callIdFound = true;
                            sip.callId = callId;
                        }

                        break;

                    case SipHeader.EVENT:

                        sip.event = line.getEvent();

                        break;

                    case SipHeader.CSEQ:

                        sip.cseqMethod = line.getCSeqMethod();

                        break;

                    case SipHeader.CONTENT_LENGTH:

                        sip.setClHeaderPos(reader.getCurrPosition(), reader.getNextPosition());

                        break;
                }
            }

            sip.rawData = sipData;
            logger.debug(sip.toString());

            return sip;
        }

        byte[] retouching() {
            if (!needRetouch)
                return rawData;

            ByteBuffer bb = ByteBuffer.allocate(rawData.length);
            bb.put(rawData, 0, clStart);
            bb.put(rawData, clNext, hdrEnd - clNext);
            bb.put(rawData, clStart, clNext - clStart);
            bb.put(rawData, hdrEnd, rawData.length - hdrEnd);

            return bb.array();
        }

        boolean isRequest() {
            return request;
        }

        boolean isProvisionalResponse() {
            return (!request && (statusCode / 100 == 1));
        }

        boolean isFinalResponse() {
            return (!request && (statusCode / 100 != 1));
        }

        boolean is2XXResponse() {
            return (!request && (statusCode / 100 == 2));
        }

        boolean isRestricted() {
            boolean restricted = false;

            if (request) {
                switch (method) {
                case INVITE:
                case CANCEL:
                case ACK:
                case BYE:
                case MESSAGE:
                    restricted = true;
                    break;
                default:
                    break;
                }
            }

            return restricted;
        }

        boolean isPresenceEvent() {
            return request && method == NOTIFY && "presence".equalsIgnoreCase(event);
        }

        public int getClHederPos() {
            return clStart;
        }

        void setClHeaderPos(int clStart, int clNext) {
            this.clStart = clStart;
            this.clNext = clNext;
        }

        void setHeadersEndPos(int hdrEnd) {
            this.hdrEnd = hdrEnd;
            if (clNext > 0 && clNext < hdrEnd)
                needRetouch = true;
        }

        @Override
        public String toString() {
            StringBuilder builder = new StringBuilder();
            if (isRequest()) {
                builder.append("Request of method[");
                builder.append(method);
                builder.append("]:");
            } else {
                builder.append("Response of code[");
                builder.append(statusCode);
                builder.append("]:");
            }
            builder.append("branchId[");
            builder.append(branchId);
            builder.append("]");

            builder.append(";callId[");
            builder.append(callId);
            builder.append("]");

            builder.append(";event[");
            builder.append(event);
            builder.append("]");

            builder.append(";transport[");
            builder.append(transport);
            builder.append("]");

            return builder.toString();
        }

        int method;
        int statusCode;
        String branchId;
        String callId;
        String event;
        int transport;
        int cseqMethod;
        private int clStart;
        private int clNext;
        private int hdrEnd;
        private boolean request;
        private boolean needRetouch;
        private byte[] rawData;
    }

    private static class LineReader {

        LineReader(byte[] data, Logger logger) {
            this.buffer = data;
            this.currPos = 0;
            this.nextPos = 0;
            this.logger = logger;
        }

        Line read() {
            if (this.nextPos >= buffer.length)
                return null;

            this.currPos = this.nextPos;
            final int MAX_LINES = 8;
            int[] bofs = new int[MAX_LINES];
            int[] eofs = new int[MAX_LINES];
            int lineIdx = 0;
            int dataLen = 0;
            int nextOffset = 0;

            for (int i = 0; i < bofs.length; i++) {
                bofs[i] = -1;
                eofs[i] = -1;
            }

            bofs[lineIdx] = this.currPos;

            for (int i = this.currPos; i < buffer.length; i++) {
                if (nextOffset > 0)
                    break;

                switch (buffer[i]) {
                    case '\r':
                        if (bofs[lineIdx] < 0) bofs[lineIdx] = i;
                        if (eofs[lineIdx] < 0) eofs[lineIdx] = i;
                        else
                            nextOffset = i;
                        break;

                    case '\n':
                        if (bofs[lineIdx] < 0) bofs[lineIdx] = i;
                        if (eofs[lineIdx] < 0) eofs[lineIdx] = i;
                        break;

                    case ' ':
                    case '\t':
                        if (eofs[lineIdx] >= 0) {
                            if (lineIdx+1 < bofs.length) {
                                lineIdx++;
                                int j;
                                for (j = i+1; j < buffer.length
                                        && (buffer[j] == ' ' || buffer[j] == '\t'); j++);
                                if (j < buffer.length) {
                                    i = j - 1;
                                    dataLen++;
                                } else {
                                    lineIdx--;
                                    nextOffset = j;
                                }
                            }
                        } else {
                            dataLen++;
                        }
                        break;

                    default:
                        if (eofs[lineIdx] >= 0) {
                            nextOffset = i;
                            break;
                        }

                        if (bofs[lineIdx] < 0)
                            bofs[lineIdx] = i;
                        dataLen++;
                        break;
                }
            }

            // update offset for next search
            if (nextOffset > 0) {
                this.nextPos = nextOffset;
            } else {
                if (bofs[lineIdx] >= 0 && eofs[lineIdx] < 0)
                    eofs[lineIdx] = buffer.length;
                this.nextPos = buffer.length;
            }

            if (dataLen == 0) {
                return new Line(new char[0], logger);
            }

            //ByteBuffer bb = ByteBuffer.allocate(dataLen);
            byte[] bb = new byte[dataLen];
            for (int i = 0, pos = 0; i < lineIdx + 1; i++) {
                int begin = bofs[i];
                if (begin < 0)
                    break;
                int end = eofs[i];
                if (i > 0)
                    bb[pos++] = (byte)' ';
                //bb.put((byte)' ');

                System.arraycopy(buffer, begin, bb, pos, end - begin);
                pos += end - begin;
                //bb.put(buffer, begin, end - begin);
            }
            char[] chars = new char[bb.length];
            for (int i = 0; i < chars.length; i++)
                chars[i] = (char)(bb[i] & 0xFF);

            return new Line(chars, logger);
/*
            bb.rewind();
            return new Line(StandardCharsets.US_ASCII.decode(bb).array(), logger);
*/
        }

        int getCurrPosition() { return this.currPos; }

        int getNextPosition() { return this.nextPos; }

        private byte[] buffer;
        private int currPos;
        private int nextPos;
        private Logger logger;
    }

    private static class Line {

        Line(char[] base, Logger logger) {
            this.dataBuffer = base;
            this.dataOffset = 0;
            this.bufSize = base.length;
            this.logger = logger;
        }

        SipMessage processFirstLine() {
            SipMessage sip = null;

            // Process first line
            if (isRequest()) {
                sip = new SipMessage(true);
                sip.method = getMethod();
            } else {
                sip = new SipMessage(false);
                sip.statusCode = getStatusCode();
            }

            return sip;
        }

        boolean startsWith(final String to) {
            int i, j;
            for (i = dataOffset, j = 0;
                 i < bufSize && j < to.length() && to.charAt(j) == dataBuffer[i]; i++, j++);
            return j == to.length();
        }

        boolean isRequest() {
            return !startsWith("SIP/2.0");
        }

        int getMethod() {
            int method = 0;

            if (startsWith(METHOD_INVITE))
                method = SipMessage.INVITE;
            else
            if (startsWith(METHOD_ACK))
                method = SipMessage.ACK;
            else
            if (startsWith(METHOD_CANCEL))
                method = SipMessage.CANCEL;
            else
            if (startsWith(METHOD_BYE))
                method = SipMessage.BYE;
            else
            if (startsWith(METHOD_OPTIONS))
                method = SipMessage.OPTIONS;
            else
            if (startsWith(METHOD_REGISTER))
                method = SipMessage.REGISTER;
            else
            if (startsWith(METHOD_MESSAGE))
                method = SipMessage.MESSAGE;
            else
            if (startsWith(METHOD_SUBSCRIBE))
                method = SipMessage.SUBSCRIBE;
            else
            if (startsWith(METHOD_PUBLISH))
                method = SipMessage.PUBLISH;
            else
            if (startsWith(METHOD_NOTIFY))
                method = SipMessage.NOTIFY;

            return method;
        }

        int getStatusCode() {
            int code = 0;
            for (int i = 0, sp = 0; i < bufSize; i++) {
                if (sp == 0 && dataBuffer[i] == ' ')
                    sp = i;
                else
                if (sp > 0 && dataBuffer[i] == ' ') {
                    code = Integer.valueOf(
                            String.valueOf(dataBuffer, sp + 1, i - sp - 1));
                    break;
                }
            }

            return code;
        }

        String getCopy() {
            return String.valueOf(dataBuffer, 0, bufSize);
        }

        int getLength() {
            return bufSize;
        }

        String getBranchId() {
            final char[] prefix = "branch=".toCharArray();
            final char[] magic = "z9hG4bK".toCharArray();
            final char first = 'b';
            int start = 0;
            int max = bufSize - prefix.length;

            for (int i = dataOffset; i <= max; i++) {
                if (dataBuffer[i] != first) {
                    while (++i <= max && dataBuffer[i] != first);
                }
                if (i <= max) {
                    int j = i + 1;
                    int end = j + prefix.length - 1;
                    for (int k = 1; j < end && dataBuffer[j] == prefix[k]; j++, k++);

                    if (j == end) {
                        start = j;
                        break;
                    }
                }
            }

            if (start > 0) {
                dataOffset = start;

                for (int i = start; i < bufSize
                        && isValidToken(dataBuffer[i]); i++, dataOffset++);

                if (dataOffset > start) {
                    int i, j;
                    for (i = start, j = 0;
                         i < dataOffset && j < magic.length && dataBuffer[i] == magic[j]; i++, j++);
                    if (j == magic.length) {
                        return String.valueOf(dataBuffer, start, dataOffset-start);
                    }
                }
            }

            return null;
        }

        String getTransport() {
            int i, start;

            // WS
            for (i = dataOffset; i < bufSize
                    && (dataBuffer[i] == ' ' || dataBuffer[i] == '\t');
                 i++);
            // Protocol
            for (dataOffset = i;
                 i < bufSize && isValidToken(dataBuffer[i]); i++, dataOffset++);
            // WS
            for (; i < bufSize
                    && (dataBuffer[i] == ' ' || dataBuffer[i] == '\t');
                 i++);
            if (i == bufSize || dataBuffer[i] != '/')
                return null;
            // WS
            for (i++; i < bufSize
                    && (dataBuffer[i] == ' ' || dataBuffer[i] == '\t');
                 i++);
            // Version
            for (dataOffset = i;
                 i < bufSize && isValidToken(dataBuffer[i]); i++, dataOffset++);
            // WS
            for (; i < bufSize
                    && (dataBuffer[i] == ' ' || dataBuffer[i] == '\t');
                 i++);
            if (i == bufSize || dataBuffer[i] != '/')
                return null;

            // WS
            for (i++; i < bufSize
                    && (dataBuffer[i] == ' ' || dataBuffer[i] == '\t');
                 i++);
            // Transport
            for (start = i, dataOffset = i;
                 i < bufSize && isValidToken(dataBuffer[i]); i++, dataOffset++);

            return (dataOffset > start) ? String.valueOf(dataBuffer, start, dataOffset-start) : null;
        }

        int getCSeqMethod() {
            int i;

            // WS
            for (i = dataOffset; i < bufSize
                    && (dataBuffer[i] == ' ' || dataBuffer[i] == '\t');
                 i++);

            // Sequence
            for (dataOffset = i;
                 i < bufSize && isDigit(dataBuffer[i]); i++, dataOffset++);

            // WS
            for (; i < bufSize
                    && (dataBuffer[i] == ' ' || dataBuffer[i] == '\t');
                 i++);

            dataOffset = i;

            return getMethod();
        }

        String getCallId() {
            int i, start;

            // eat leading white space
            for (i = dataOffset; i < bufSize
                    && (dataBuffer[i] == ' ' || dataBuffer[i] == '\t');
                 i++);
            for (start = i, dataOffset = i;
                 i < bufSize && isValidWord(dataBuffer[i]); i++, dataOffset++);
            if (dataOffset > start && dataOffset < bufSize) {
                if (dataBuffer[dataOffset] == '@') {
                    dataOffset++;
                    for (i = dataOffset;
                         i < bufSize && isValidWord(dataBuffer[i]); i++, dataOffset++);
                }
            }

            return (dataOffset > start) ? String.valueOf(dataBuffer, start, dataOffset-start) : null;
        }

        String getEvent() {
            int i, start;

            // WS
            for (i = dataOffset; i < bufSize
                    && (dataBuffer[i] == ' ' || dataBuffer[i] == '\t');
                 i++);

            // Event
            for (start = i, dataOffset = i;
                 i < bufSize && isValidToken(dataBuffer[i]); i++, dataOffset++);

            return (dataOffset > start) ? String.valueOf(dataBuffer, start, dataOffset-start) : null;
        }

        int getHeaderName() {
            int i,j, start, end;
            boolean equals = false;

            // eat leading white space
            for (i = dataOffset; i < bufSize && dataBuffer[i] <= ' '; i++);

            if (i < bufSize) {
                for (start = i, end = i; i < bufSize
                        && isValidToken(dataBuffer[i]); i++, end++);
                // eat leading white space
                for (i = end; i < bufSize && dataBuffer[i] <= ' '; i++);

                if (i < bufSize && dataBuffer[i] == ':') {
                    String name = new String(dataBuffer, start, end-start);
                    Integer header = headerCache.get(name.toLowerCase());
                    if (header != null) {
                        dataOffset = i + 1;
                        return header;
                    }
                }
            }

            return 0;
        }

        private boolean isDigit(char c) {
            return (c >= '0' && c <= '9');
        }

        private boolean isAlphanum(char c) {
            return (c >= '0' && c <= '9')
                    || (c >= 'a' && c <= 'z')
                    || (c >= 'A' && c <= 'Z');
        }

        private boolean isValidToken(char c) {
            if ( isAlphanum(c) ) return true;
            else switch (c) {
                case '-':
                case '.':
                case '!':
                case '%':
                case '*':
                case '_':
                case '+':
                case '`':
                case '\'':
                case '~':
                    return true;
                default:
                    return false;
            }
        }

        private boolean isValidWord(char c) {
            if ( isAlphanum(c) ) return true;
            else switch (c) {
                case '-':
                case '.':
                case '!':
                case '%':
                case '*':
                case '_':
                case '+':
                case '`':
                case '\'':
                case '~':
                case '(':
                case ')':
                case '<':
                case '>':
                case ':':
                case '\\':
                case '\"':
                case '/':
                case '[':
                case ']':
                case '?':
                case '{':
                case '}':
                    return true;
                default:
                    return false;
            }
        }

        private char[] dataBuffer;
        private int bufSize;
        private int dataOffset;
        private Logger logger;

        private static final String METHOD_REGISTER = "REGISTER";
        private static final String METHOD_INVITE = "INVITE";
        private static final String METHOD_ACK = "ACK";
        private static final String METHOD_OPTIONS = "OPTIONS";
        private static final String METHOD_BYE = "BYE";
        private static final String METHOD_CANCEL = "CANCEL";
        private static final String METHOD_MESSAGE = "MESSAGE";
        private static final String METHOD_SUBSCRIBE = "SUBSCRIBE";
        private static final String METHOD_PUBLISH = "PUBLISH";
        private static final String METHOD_NOTIFY = "NOTIFY";
    }

    static class SipHeader {
        static final String FNAME_VIA = "via";
        static final String CNAME_VIA = "v";
        static final String FNAME_CALLID = "call-id";
        static final String CNAME_CALLID = "i";
        static final String FNAME_EVENT = "event";
        static final String CNAME_EVENT = "o";
        static final String NAME_CSEQ = "cseq";
        static final String FNAME_CONTENT_LENGTH = "content-length";
        static final String CNAME_CONTENT_LENGTH = "l";

        static final int VIA = 1;
        static final int CALLID = 2;
        static final int CSEQ = 3;
        static final int EVENT = 4;
        static final int CONTENT_LENGTH = 5;
    }

    /**
     * TransactionManager
     *
     * To maintain all ongoing transactions and active sessions.
     */
    static class TransactionManager {

        TransactionManager(RuaAdapter adapter, Processor processor, Logger logger) {
            this.adapter = adapter;
            this.processor = processor;
            this.logger = logger;
        }

        /**
         * Process incoming request.
         */
        void incomingRequest(SipMessage request, int reqId, int connId) {
            Transaction transaction = null;
            Transaction transact = null;

            switch (request.method) {
            case SipMessage.INVITE:

                transaction = serverTransactions.get(request.branchId);
                if (transaction != null) {
                    // Should be retransmission
                } else {
                    transaction = newServerTransaction(request, reqId, connId);
                }

                break;

            case SipMessage.ACK:

                transaction = serverTransactions.get(request.branchId);
                if (transaction != null) {
                    if (transaction.method == SipMessage.INVITE
                            && transaction.state == Transaction.COMPLETED) {
                        transactionConfirmed(transaction);
                    }
                } else {
                    transact = activeSessions.get(request.callId);
                }

                break;

            case SipMessage.CANCEL:

                transact = serverTransactions.get(request.branchId);
                if (transact != null) {
                    // According to RFC3261, only INVITE transaction need handle CANCEL request
                    if (transact.state == Transaction.PROCEEDING) {
                        transactionCancelled(transact);
                        break;
                    }
                }

                transaction = newServerTransaction(request, reqId, connId);

                break;

            default:

                transaction = newServerTransaction(request, reqId, connId);

                break;
            }

            if (transaction != null)
                logger.debug("incomingRequest:" + transaction);
            else
                logger.debug("incomingRequest:" + null);
        }

        /**
         * Process incoming response.
         */
        Transaction incomingResponse(SipMessage response, int reqId, int connId) {
            Transaction transaction = null;
            String transactionId = response.branchId;

            if (response.cseqMethod == SipMessage.CANCEL)
                transactionId += response.cseqMethod;

            transaction = clientTransactions.get(transactionId);
            if (transaction == null) {
                transaction = clientTransactionsForNoVia.get(response.callId);
            }

            if (transaction != null) {
                if (transaction.requestId != reqId)
                    transaction.requestId = reqId;
                if (transaction.connId != connId)
                    transaction.connId = connId;

                if (transaction.method == SipMessage.INVITE) {
                    switch (transaction.state) {
                    case Transaction.CALLING:
                        if (response.isProvisionalResponse()) {
                            transactionProceeding(transaction);
                        } else if (response.is2XXResponse()) {
                            transactionTerminated(transaction);
                            sessionUp(transaction);
                        } else {
                            transactionCompleted(transaction);
                        }
                        break;

                    case Transaction.PROCEEDING:
                        if (response.is2XXResponse()) {
                            transactionTerminated(transaction);
                            sessionUp(transaction);
                        } else if (response.isFinalResponse()) {
                            transactionCompleted(transaction);
                        }
                        break;

                    case Transaction.CANCELLED:
                        if (response.isFinalResponse())
                            transactionTerminated(transaction);
                        break;
                    }
                } else {
                    switch (transaction.state) {
                    case Transaction.TRYING:

                        if (response.isProvisionalResponse())
                            transactionProceeding(transaction);

                    case Transaction.PROCEEDING:

                        if (response.isFinalResponse()) {
                            transactionCompleted(transaction);
                        }
                        break;

                    default:
                        break;
                    }
                }
            } else {
                // Should be retransmit
            }

            if (transaction != null)
                logger.debug("incomingResponse:" + transaction);
            else
                logger.debug("incomingResponse:" + null);

            return transaction;
        }

        /**
         * Process outgoing request.
         */
        Transaction outgoingRequest(SipMessage request, int channelId) {
            Transaction transaction = null;

            switch (request.method) {
            case SipMessage.INVITE:

                transaction = clientTransactions.get(request.branchId);
                if (transaction != null ||
                        clientTransactionsForNoVia.get(request.callId) != null) {
                    // Should be retransmission
                } else {
                    transaction = newClientTransaction(request, channelId);
                }

                break;

            case SipMessage.ACK:

                transaction = clientTransactions.get(request.branchId);
                if (transaction != null) {
                    if (transaction.method == SipMessage.INVITE
                            && transaction.state == Transaction.COMPLETED) {
                        // This is the ACK for 300-699 response of INVITE, Platform IMS
                        // stack will be responsible to send ACK. Just drop it.
                        return null;
                    }

                    return transaction;
                }
                // ACK for 2XX response should start new Transaction, thus
                // connId need to be updated via Call-ID.
                Transaction transact = activeSessions.get(request.callId);
                if (transact != null) {
                    transaction = newClientTransaction(request, 0, transact.connId, channelId);
                } else {
                    transaction = newClientTransaction(request, channelId);
                }

                break;

            case SipMessage.CANCEL:

                transact = clientTransactions.get(request.branchId);
                if (transact == null) {
                    transact = clientTransactionsForNoVia.get(request.callId);
                }
                if (transact != null) {
                    // According to RFC3261, only INVITE transaction need handle CANCEL request
                    if (transact.state == Transaction.PROCEEDING) {
                        transactionCancelled(transact);
                        transaction = newClientTransaction(
                                request, transact.requestId, transact.connId, channelId);
                    }
                } else {
                    transact = activeSessions.get(request.branchId);
                    if (transact != null)
                        transaction = newClientTransaction(
                                request, transact.requestId, transact.connId, channelId);
                }

                break;

            case SipMessage.BYE:

                sessionDown(request.callId);
                transaction = newClientTransaction(request, channelId);

                break;

            default:
                transaction = newClientTransaction(request, channelId);
                break;
            }

            if (transaction != null)
                logger.debug("outgoingRequest:" + transaction);
            else
                logger.debug("outgoingRequest:" + null);

            return transaction;
        }

        /**
         * Process outgoing response.
         */
        Transaction outgoingResponse(SipMessage response) {
            Transaction transaction = null;
            String transactionId = response.branchId;

            if (response.cseqMethod == SipMessage.CANCEL)
                transactionId += response.cseqMethod;

            transaction = serverTransactions.get(transactionId);
            if (transaction == null) {
                // Could be retransmit for 200 OK
                if (response.cseqMethod == SipMessage.INVITE && response.is2XXResponse()) {
                    transaction = activeSessions.get(response.callId);
                }
                if (transaction == null) {
                    return transaction;
                }
            }

            if (transaction.method == SipMessage.INVITE) {
                switch (transaction.state) {
                case Transaction.CALLING:

                    if (response.isProvisionalResponse()) {
                        transactionProceeding(transaction);
                        if (response.statusCode == 100) {
                            // IMS stack will be responsible for 100 Trying, drop redundant.
                            transaction = null;
                        }
                    } else if (response.is2XXResponse()) {
                        transactionTerminated(transaction);
                        sessionUp(transaction);
                    } else {
                        transactionCompleted(transaction);
                    }

                    break;

                case Transaction.PROCEEDING:

                    if (response.is2XXResponse()) {
                        transactionTerminated(transaction);
                        sessionUp(transaction);
                    } else if (response.isFinalResponse()) {
                        transactionCompleted(transaction);
                    }

                    break;

                case Transaction.CANCELLED:

                    if (response.isFinalResponse())
                        transactionTerminated(transaction);

                    break;
                }
            } else {
                switch (transaction.state) {
                case Transaction.TRYING:

                    if (response.isProvisionalResponse())
                        transactionProceeding(transaction);

                case Transaction.PROCEEDING:

                    if (response.isFinalResponse()) {
                        transactionCompleted(transaction);
                        if (response.cseqMethod == SipMessage.BYE) {
                            sessionDown(response.callId);
                        }
                    }

                    break;

                default:
                    break;
                }
            }

            if (transaction != null)
                logger.debug("outgoingResponse:" + transaction);
            else
                logger.debug("outgoingResponse:" + null);

            return transaction;
        }

        Transaction newClientTransaction(SipMessage message, int channelId) {
            return newClientTransaction(message, 0, 0, channelId);
        }

        Transaction newClientTransaction(SipMessage message, int reqId, int connId, int channelId) {
            int requestId = (reqId == 0) ? globalReqId++ : reqId;
            int protocolId = connId == 0 ? getProtocol(message.transport) : connId;

            Transaction transaction = new Transaction(
                    message, false, requestId, protocolId, channelId);
            // Only add the transaction into clientTransactions when there is branch.id in VIA header
            // sent from client
            if (message.branchId != null) {
                clientTransactions.put(transaction.ID, transaction);
            }
            clientTransactionsForNoVia.put(transaction.callId, transaction);

            Message timeout = processor.obtainMessage(Processor.TRANSACTION_TIMEOUT, transaction);
            processor.sendMessageDelayed(timeout, transaction.getTimeout());

            return transaction;
        }

        Transaction newServerTransaction(SipMessage message, int reqId, int connId) {
            int protocolId = connId == 0 ? getProtocol(message.transport) : connId;

            Transaction transaction = new Transaction(
                    message, true, reqId, protocolId, 0);
            serverTransactions.put(transaction.ID, transaction);

            Message timeout = processor.obtainMessage(Processor.TRANSACTION_TIMEOUT, transaction);
            processor.sendMessageDelayed(timeout, transaction.getTimeout());

            return transaction;
        }

        void transactionProceeding(Transaction transaction) {
            transaction.state = Transaction.PROCEEDING;
            processor.removeMessages(Processor.TRANSACTION_TIMEOUT, transaction);
        }

        void transactionCompleted(Transaction transaction) {
            transaction.state = Transaction.COMPLETED;
            processor.removeMessages(Processor.TRANSACTION_TIMEOUT, transaction);

            Message message = processor.obtainMessage(Processor.TRANSACTION_TIMEOUT, transaction);
            processor.sendMessageDelayed(message, transaction.getTimeout());
        }

        void transactionConfirmed(Transaction transaction) {
            transaction.state = Transaction.CONFIRMED;
            processor.removeMessages(Processor.TRANSACTION_TIMEOUT, transaction);

            Message message = processor.obtainMessage(Processor.TRANSACTION_TIMEOUT, transaction);
            processor.sendMessageDelayed(message, transaction.getTimeout());
        }

        void transactionTerminated(Transaction transaction) {
            transaction.state = Transaction.TERMINATED;
            if (transaction.UAC) {
                clientTransactions.remove(transaction.ID);
                clientTransactionsForNoVia.remove(transaction.callId);
            } else {
                serverTransactions.remove(transaction.ID);
            }
            processor.removeMessages(Processor.TRANSACTION_TIMEOUT, transaction);
        }

        void transactionCancelled(Transaction transaction) {
            transaction.state = Transaction.CANCELLED;
        }

        void sessionUp(Transaction transaction) {
            if (activeSessions.containsKey(transaction.callId)) {
                // Should be session modify INVITE
            } else {
                activeSessions.put(transaction.callId, transaction);
                adapter.notifySessionIncreased();
            }
        }

        void sessionDown(String sessionId) {
            if (activeSessions.containsKey(sessionId)) {
                activeSessions.remove(sessionId);
                adapter.notifySessionDecreased();
            }
        }

        /**
         * Gets the tcp/udp from sip message.
         *
         * @param transport SIP transport
         * @return udp = 0x10000, tcp = 0x20001
         */
        int getProtocol(int transport) {
            int protocol;

            switch (transport) {
                case Configuration.TCP:
                    protocol = 131073; // 0x20001
                    break;
                case Configuration.UDP:
                    protocol = 65536; // 0x10000
                    break;
                default:
                    protocol = 131073;
                    break;
            }

            return protocol;
        }

        void removeTransaction(Transaction transaction) {
            if (transaction.UAC) {
                clientTransactions.remove(transaction.ID);
                clientTransactionsForNoVia.remove(transaction.callId);
            } else {
                serverTransactions.remove(transaction.ID);
            }
        }

        void clearAllTransactions() {
            clientTransactions.clear();
            clientTransactionsForNoVia.clear();
            serverTransactions.clear();
            processor.removeMessages(Processor.TRANSACTION_TIMEOUT);
            activeSessions.clear();
        }

        void dump() {
            logger.debug("dump client transactions begin");
            for (Transaction transaction : clientTransactions.values()) {
                logger.debug(transaction.toString());
            }
            logger.debug("dump client transactions for no VIA begin");
            for (Transaction transaction : clientTransactionsForNoVia.values()) {
                logger.debug(transaction.toString());
            }
            logger.debug("dump server transactions begin");
            for (Transaction transaction : serverTransactions.values()) {
                logger.debug(transaction.toString());
            }
            logger.debug("dump active sessions begin");
            for (Transaction transaction : activeSessions.values()) {
                logger.debug(transaction.toString());
            }
            logger.debug("dump end");
        }

        private final RuaAdapter adapter;
        private final Processor processor;
        Logger logger;
        private HashMap<String, Transaction> clientTransactions = new HashMap<>();
        private HashMap<String, Transaction> clientTransactionsForNoVia = new HashMap<>();
        private HashMap<String, Transaction> serverTransactions = new HashMap<>();
        private HashMap<String, Transaction> activeSessions = new HashMap<>();
        private int globalReqId = 0x0FFF0000;
    }

    /**
     * The Class represent SIP Transaction.
     */
    static class Transaction {

        /**
         * Instantiates a new sip transaction.
         *
         * @param msg SIP message data
         * @param incoming incoming SIP message
         * @param reqId the req id
         * @param connId the conn id
         */
        private Transaction(SipMessage msg, boolean incoming, int reqId, int connId, int channelId) {
            this.requestId = reqId;
            this.connId = connId;
            // CANCEL has same branch ID as the request to be cancelled
            // need modify to make a distinct one
            if (msg.method == SipMessage.CANCEL)
                this.ID = msg.branchId + msg.method;
            else
                this.ID = msg.branchId;
            this.transport = msg.transport;
            this.callId = msg.callId;
            this.method = msg.method;
            this.channelId = channelId;
            this.UAC = !incoming;
            if (msg.method == SipMessage.INVITE) {
                this.state = CALLING;
            } else {
                this.state = TRYING;
            }
        }

        boolean isTerminated() {
            return state == TERMINATED;
        }

        long getTimeout() {
            long timeout = 0;

            switch (state) {
            case COMPLETED:

                // enlarge timeout to match redundant ACK
                if (method == SipMessage.INVITE && UAC)
                    timeout = 10 * 1000;
                else if (UAC) // Timer K
                    timeout = (transport == Configuration.UDP) ? 5 * 1000 : 0;
                else if (method == SipMessage.INVITE) // Timer H
                    timeout = 32 * 1000;
                else // Timer J
                    timeout = (transport == Configuration.UDP) ? 32 * 1000 : 0;

                break;

            case CONFIRMED:

                // Timer I
                timeout = (transport == Configuration.UDP) ? 5 * 1000 : 0;

                break;

            default:
                // Timer B
                timeout = 32 * 1000;

                break;
            }

            return timeout;
        }

        @Override
        public String toString() {
            StringBuilder builder = new StringBuilder();
            if (UAC)
                builder.append("ClientTransaction{");
            else
                builder.append("ServerTransaction{");
            builder.append("state[" + state + "],");
            builder.append("method[" + method + "],");
            builder.append("requestId[" + String.format("%#x", requestId) + "],");
            builder.append("connId[" + String.format("%#x", connId) + "],");
            builder.append("transport[" + transport + "],");
            builder.append("ID[" + ID + "],");
            builder.append("callId[" + callId + "]");
            builder.append("}");

            return builder.toString();
        }

        private int requestId;
        // udp = 65536, tcp = 131073
        private int connId;
        private int method;
        private int channelId;
        private int state;
        private int transport;
        private String ID;
        private String callId;
        private boolean UAC;

        private static final int CALLING = 1;
        private static final int TRYING = 2;
        private static final int PROCEEDING = 3;
        private static final int COMPLETED = 4;
        private static final int CONFIRMED = 5;
        private static final int TERMINATED = 6;
        private static final int CANCELLED = 7;
    };

    private static SipHandler INSTANCE;
    private static boolean sipTraceEnabled = true;
    private static final String numberPattern = "(?<=[\\d]{2})\\d(?=[\\d]{4})";

    private RuaAdapter adapter;
    private HandlerThread spThread;
    private Processor processor;
    private TransactionManager transactMgr;
    private HashSet<Listener> listeners = new HashSet<Listener>();
    private Listener stanaloneListener = null;
    private Logger logger = Logger.getLogger(this.getClass().getName());
    private Logger SIPlogger = Logger.getLogger("RuaSipTrace");
}
