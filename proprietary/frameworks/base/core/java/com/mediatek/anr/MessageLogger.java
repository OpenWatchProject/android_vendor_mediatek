/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
package com.mediatek.anr;

import android.os.Looper;
import android.os.Message;
import android.os.MessageQueue;
import android.os.Process;
import android.os.RemoteException;
import android.os.SystemClock;

import android.util.Log;
import android.util.Printer;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Use to dump message history and message queue.
 *
 * @hide
 */
public class MessageLogger implements Printer {
    private static final String TAG = "MessageLogger";
    // Enable message history debugging log
    public static boolean mEnableLooperLog = false;
    private CircularMessageInfoArray mMessageHistory;
    private CircularMessageInfoArray mLongTimeMessageHistory;
    final static int MESSAGE_COUNT = 20;
    final static int LONGER_TIME_MESSAGE_COUNT = 20;
    final static int LONGER_TIME = 200;             //Unit: ms
    private  String mLastRecord = null;
    private  long mLastRecordKernelTime;            //Unit: Milli
    private  long mNonSleepLastRecordKernelTime;    //Unit: Milli
    private  long mLastRecordDateTime;              //Unit: Micro
    private  int mState = 0;
    private  long mMsgCnt = 0;
    private String mName = null;
    // Fix LongMsgHistory to __exp_main.txt
    private String MSL_Warn = "MSL Waraning:";
    private String sInstNotCreated = MSL_Warn +
            "!!! MessageLoggerInstance might not be created !!!\n";
    private StringBuilder messageInfo;
    private long mProcessId;

    // Currently, we dump the first 20 messages
    private static final int MESSAGE_DUMP_SIZE_MAX = 20;

    private static Method sGetCurrentTimeMicro = getSystemClockMethod("currentTimeMicro");
    private Method mGetMessageQueue = getLooperMethod("getQueue");

    private Field mMessageQueueField = getMessageQueueField("mMessages");
    private Field mMessageField = getMessageField("next");

    private static Method getSystemClockMethod(String func) {
        try {
            Class<?> systemClock = Class.forName("android.os.SystemClock");
            return systemClock.getDeclaredMethod(func);
        } catch (Exception e) {
            return null;
        }
    }

    private Method getLooperMethod(String func) {
        try {
            Class<?> looper = Class.forName("android.os.Looper");
            return looper.getDeclaredMethod(func);
        } catch (Exception e) {
            return null;
        }
    }

    private Field getMessageQueueField(String var) {
        try {
            Class<?> messageQueue = Class.forName("android.os.MessageQueue");
            Field field = messageQueue.getDeclaredField(var);
            field.setAccessible(true);
            return field;
        } catch (Exception e) {
            return null;
        }
    }

    private Field getMessageField(String var) {
        try {
            Class<?> message = Class.forName("android.os.Message");
            Field field = message.getDeclaredField(var);
            field.setAccessible(true);
            return field;
        } catch (Exception e) {
            return null;
        }
    }

    public MessageLogger() {
        init();
    }

    public MessageLogger(boolean mValue) {
        mEnableLooperLog = mValue;
        init();
    }

    public MessageLogger(boolean mValue, String Name) {
        mName = Name;
        mEnableLooperLog = mValue;
        init();
    }

    private void init() {
        mMessageHistory = new CircularMessageInfoArray(MESSAGE_COUNT);
        mLongTimeMessageHistory = new CircularMessageInfoArray(LONGER_TIME_MESSAGE_COUNT);

        messageInfo = new StringBuilder(20 * 1024);
        mProcessId = Process.myPid();
    }

    public long wallStart;                //Unit:Micro
    public long wallTime;                 //Unit:Micro
    public long nonSleepWallStart;        //Unit:Milli
    public long nonSleepWallTime;         //Unit:Milli

    public void println(String s) {
        synchronized (this) {
            mState++;
            mMsgCnt++;

            mLastRecordKernelTime = SystemClock.elapsedRealtime();
            mNonSleepLastRecordKernelTime = SystemClock.uptimeMillis();

            try {
                if (null != sGetCurrentTimeMicro) {
                    mLastRecordDateTime = (long) sGetCurrentTimeMicro.invoke(null);
                }
            } catch (Exception e) { }

            if (mState == 1) {
                MessageInfo msgInfo = mMessageHistory.add();
                msgInfo.init();
                msgInfo.startDispatch = s;
                msgInfo.msgIdStart = mMsgCnt;
                msgInfo.startTimeElapsed = mLastRecordDateTime;
                msgInfo.startTimeUp = mNonSleepLastRecordKernelTime;
            } else {
                mState = 0;

                MessageInfo msgInfo = mMessageHistory.getLast();
                msgInfo.finishDispatch = s;
                msgInfo.msgIdFinish = mMsgCnt;
                msgInfo.durationElapsed = mLastRecordDateTime - msgInfo.startTimeElapsed;
                msgInfo.durationUp = mNonSleepLastRecordKernelTime - msgInfo.startTimeUp;
                wallTime = msgInfo.durationElapsed;
                if (msgInfo.durationElapsed >= (LONGER_TIME * 1000)) {
                    MessageInfo longMsgInfo = mLongTimeMessageHistory.add();
                    longMsgInfo.copy(msgInfo);
                }
            }

            if (mEnableLooperLog) {
                if (mState == 1) {
                    Log.d(TAG, "Debugging_MessageLogger: " + s + " start");
                } else {
                    Log.d(TAG, "Debugging_MessageLogger: " + s + " spent " +
                          wallTime / 1000 + "ms");
                }
            }
        }
    }

    public void setInitStr(String str_tmp) {
         messageInfo.delete(0, messageInfo.length());
         messageInfo.append(str_tmp);
    }

    private void log(String info) {
        messageInfo.append(info).append("\n");
        //Log.d(TAG, info);
    }

    public void dumpMessageQueue() {
        Looper looper = null;
        MessageQueue messageQueue = null;
        try {
            do {
                if (null == (looper = Looper.getMainLooper())) {
                    log(MSL_Warn + "!!! Current MainLooper is Null !!!");
                    break;
                } else if (null == (messageQueue =
                        (MessageQueue) mGetMessageQueue.invoke(looper))) {
                    log(MSL_Warn + "!!! Current MainLooper's MsgQueue is Null !!!");
                    break;
                }
                dumpMessageQueueImpl(messageQueue);
            } while (false);
        } catch (Exception e) { }

        log(String.format(MSL_Warn +
                "!!! Calling thread from PID:%d's TID:%d(%s),Thread's type is %s!!!",
                android.os.Process.myPid(),
                Thread.currentThread().getId(),
                Thread.currentThread().getName(),
                Thread.currentThread().getClass().getName()));

        StackTraceElement[] stkTrace = Thread.currentThread().getStackTrace();
        log(String.format(MSL_Warn + "!!! get StackTrace: !!!"));
        for (int index = 0;  index < stkTrace.length; index++) {
            log(String.format(MSL_Warn + "File:%s's Linenumber:%d, Class:%s, Method:%s",
                    stkTrace[index].getFileName(),
                    stkTrace[index].getLineNumber(),
                    stkTrace[index].getClassName(),
                    stkTrace[index].getMethodName()));
        }
    }

    public void dumpMessageQueueImpl(MessageQueue messageQueue) throws Exception {
        synchronized (messageQueue) {
            Message mMessages = null;
            if (null != mMessageQueueField) {
                mMessages = (Message) mMessageQueueField.get(messageQueue);
            }
            if (null != mMessages) {
                log("Dump first " + MESSAGE_DUMP_SIZE_MAX + " messages in Queue: ");
                Message message = mMessages;
                int count = 0;
                while (null != message) {
                    count++;
                    if (count <= MESSAGE_DUMP_SIZE_MAX) {
                            log("Dump Message in Queue (" + count + "): " + message);
                    }
                    message = (Message) mMessageField.get(message);
                }

                log("Total Message Count: " + count);
            } else {
                log("mMessages is null");
            }
        }
    }

    public void dumpMessageHistory() {
        synchronized (this) {
            log(">>> Entering MessageLogger.dump. to Dump MSG HISTORY <<<");
            if (null == mMessageHistory || 0 == mMessageHistory.size()) {
                log(sInstNotCreated);
                dumpMessageQueue();
                try {
                    AnrManagerNative.getDefault().informMessageDump(
                             messageInfo.toString(), Process.myPid());
                } catch (RemoteException ex) {
                    Log.d(TAG, "informMessageDump exception " + ex);
                }
                return ;
            }

            log("MSG HISTORY IN MAIN THREAD:");
            log("Current kernel time : " + SystemClock.uptimeMillis() + "ms PID=" + mProcessId);
            SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
            //State = 1 means the current dispatching message has not been finished
            int msgIdx = mMessageHistory.size() - 1;

            if (mState == 1) {
                Date date = new Date((long) mLastRecordDateTime / 1000);
                long spent = SystemClock.elapsedRealtime() - mLastRecordKernelTime;
                long nonSleepSpent = SystemClock.uptimeMillis() - mNonSleepLastRecordKernelTime;
                MessageInfo msgInfo = mMessageHistory.getLast();

                log("Last record : Msg#:" + msgInfo.msgIdStart + " " + msgInfo.startDispatch);
                log("Last record dispatching elapsedTime:" + spent + " ms/upTime:" +
                     nonSleepSpent + " ms");
                log("Last record dispatching time : " + simpleDateFormat.format(date));

                --msgIdx;
            }

            for (; msgIdx >= 0; msgIdx--) {
                MessageInfo info = mMessageHistory.get(msgIdx);
                Date date = new Date(info.startTimeElapsed / 1000);
                log("Msg#:" + info.msgIdFinish + " " + info.finishDispatch + " elapsedTime:" +
                        (info.durationElapsed / 1000) + " ms/upTime:" + info.durationUp + " ms");
                log("Msg#:" + info.msgIdStart + " " + info.startDispatch + " from " +
                        simpleDateFormat.format(date));
            }

            log("=== Finish Dumping MSG HISTORY===");

            log("=== LONGER MSG HISTORY IN MAIN THREAD ===");
            msgIdx = mLongTimeMessageHistory.size() - 1;
            for (; msgIdx >= 0; msgIdx--) {
                MessageInfo info = mLongTimeMessageHistory.get(msgIdx);

                Date date = new Date(info.startTimeElapsed / 1000);
                log("Msg#:" + info.msgIdStart + " " + info.startDispatch + " from " +
                    simpleDateFormat.format(date) + " elapsedTime:" +
                    (info.durationElapsed / 1000) + " ms/upTime:" + info.durationUp + "ms");
            }
            log("=== Finish Dumping LONGER MSG HISTORY===");

            try {
                dumpMessageQueue();
                AnrManagerNative.getDefault().informMessageDump(
                    new String(messageInfo.toString()), Process.myPid());
                messageInfo.delete(0, messageInfo.length());
            } catch (RemoteException ex) {
                Log.d(TAG, "informMessageDump exception " + ex);
            }
        }
    }

    public class MessageInfo {
        public String startDispatch;
        public String finishDispatch;
        public long msgIdStart;
        public long msgIdFinish;
        public long startTimeUp;
        public long durationUp;
        public long startTimeElapsed;
        public long durationElapsed;

        public MessageInfo() {
            init();
        }

        public void init() {
            startDispatch = null;
            finishDispatch = null;
            msgIdStart = -1;
            msgIdFinish = -1;
            startTimeUp = 0;
            durationUp = -1;
            startTimeElapsed = 0;
            durationElapsed = -1;
        }

        public void copy(MessageInfo info) {
            startDispatch = info.startDispatch;
            finishDispatch = info.finishDispatch;
            msgIdStart = info.msgIdStart;
            msgIdFinish = info.msgIdFinish;
            startTimeUp = info.startTimeUp;
            durationUp = info.durationUp;
            startTimeElapsed = info.startTimeElapsed;
            durationElapsed = info.durationElapsed;
        }
    }

    public class CircularMessageInfoArray {
        private MessageInfo[] mElem;
        private MessageInfo mLastElem;
        private int mHead;
        private int mTail;
        private int mSize;

        public CircularMessageInfoArray(int size) {
            int capacity = size + 1;
            mElem = new MessageInfo[capacity];
            for (int i = 0; i < capacity; ++i) {
                mElem[i] = new MessageInfo();
            }
            mHead = 0;
            mTail = 0;
            mLastElem = null;
            mSize = capacity;
        }

        public boolean empty() {
            return (mHead == mTail) || (mElem == null);
        }

        public boolean full() {
            return (mTail == mHead - 1) || (mTail - mHead == mSize - 1);
        }

        public int size() {
            if (mTail - mHead >= 0) {
                return mTail - mHead;
            } else {
                return mSize + mTail - mHead;
            }
        }

        private MessageInfo getLocked(int n) {
            if (mHead + n <= mSize - 1) {
                return mElem[mHead + n];
            } else {
                return mElem[mHead + n - mSize];
            }
        }

        public synchronized MessageInfo get(int n) {
            if (n >= 0 && n < size()) {
                return getLocked(n);
            } else {
                return null;
            }
        }

        public MessageInfo getLast() {
            return mLastElem;
        }

        public synchronized MessageInfo add() {
            if (full()) {
                mHead++;
                if (mHead == mSize) {
                    mHead = 0;
                }
            }

            mLastElem = mElem[mTail];

            mTail++;
            if (mTail == mSize) {
                mTail = 0;
            }

            return mLastElem;
        }
    }
}

