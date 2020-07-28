/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
 */

package com.mediatek.email.util;

import java.util.ArrayList;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

import android.content.Context;

import com.android.email.mail.Store;
import com.android.email.provider.Utilities;
import com.android.emailcommon.Logging;
import com.android.emailcommon.internet.MimeUtility;
import com.android.emailcommon.mail.FetchProfile;
import com.android.emailcommon.mail.Folder;
import com.android.emailcommon.mail.Message;
import com.android.emailcommon.mail.MessagingException;
import com.android.emailcommon.mail.Part;
import com.android.emailcommon.mail.Folder.OpenMode;
import com.android.emailcommon.provider.Account;
import com.android.emailcommon.provider.EmailContent;
import com.android.emailcommon.provider.Mailbox;
import com.android.mail.utils.LogUtils;

public class ImapMailDownloader {
    private static final String PLAINTEXT_MIMETYPE = "text/plain";
    private static final String HTMLTEXT_MIMETYPE = "text/html";

    private static final int MAX_SMALL_MESSAGE_SIZE = (5 * 1024);

    //Create thread pool for limit threads using.
    private static final int CORE_POOL_SIZE = 40;
    private static final int MAXIMUM_POOL_SIZE = 40;
    private static final int QUEUE_SIZE = 120;
    private static final int KEEP_ALIVE = 1;

    //Define the maximum threads count of one account.
    private static final int MAX_ACCOUNT_SYNC_THREADS = 8;

    private static final ThreadFactory THREAD_FACTORY = new ThreadFactory() {
        private final AtomicInteger mCount = new AtomicInteger(1);
        public Thread newThread(Runnable r) {
            String name = "ImapService #" + mCount.getAndIncrement();
            return new Thread(r, name);
        }
    };

    private static final BlockingQueue<Runnable> POOL_WORK_QUEUE =
            new LinkedBlockingQueue<Runnable>(QUEUE_SIZE);

    /**
     * An {@link Executor} that can be used to execute messages sync related runnable in parallel.
     */
    private static final ThreadPoolExecutor MESSAGE_SYNC_THREAD_POOL
            = new ThreadPoolExecutor(CORE_POOL_SIZE, MAXIMUM_POOL_SIZE, KEEP_ALIVE,
                    TimeUnit.SECONDS, POOL_WORK_QUEUE, THREAD_FACTORY);
    static {
        MESSAGE_SYNC_THREAD_POOL.allowCoreThreadTimeOut(true);
    }

    // Save unsynced messages as global to access it in multi-thread
    private ArrayList<Message> mUnsyncedMessages = new ArrayList<Message>();
    // Save MessagingException in thread to send it out
    private MessagingException mMessagingException;
    // Save the count of running sync threads
    private int mRunningSyncThreadCount = 0;

    /**
     * M: Load the structure and body of messages not yet synced in multithread
     * @param context the context
     * @param account the account we're syncing
     * @param remoteStore the Store we're working on
     * @param unsyncedMessages an array of Message's we've got headers for
     * @param toMailbox the destination mailbox we're syncing
     * @throws MessagingException
     */
    public void loadUnsyncedMessagesInMultiThread(Context context, final Account account, Store remoteStore,
            ArrayList<Message> unsyncedMessages, final Mailbox toMailbox)
            throws MessagingException {
        LogUtils.d(Logging.LOG_TAG, "loadUnsyncedMessagesInMultiThread message: " + unsyncedMessages.size());
        /** M: Put IMAP synchronize process into Multi-Thread,  left POP3 run as used to @{ */
        mUnsyncedMessages = unsyncedMessages;

        /** M: Start {@link MAX_ACCOUNT_SYNC_THREADS} threads to synchronize Messages concurrently */
        synchronized (mUnsyncedMessages) {
            mRunningSyncThreadCount = 0;
            final int unsyncedMessagesCount = unsyncedMessages.size();
            while (unsyncedMessagesCount > mRunningSyncThreadCount
                    && mRunningSyncThreadCount < MAX_ACCOUNT_SYNC_THREADS) {
                    Logging.v("unsyncedMessages size: " + unsyncedMessagesCount
                            + " threadIndex: " + mRunningSyncThreadCount);
                MESSAGE_SYNC_THREAD_POOL.execute(new LoadUnsyncMessageTask(context, account,
                        remoteStore, toMailbox));
                mRunningSyncThreadCount++;
            }
            // wait until all messages has been fetched.
            while (mRunningSyncThreadCount > 0) {
                try {
                    mUnsyncedMessages.wait();
                } catch (InterruptedException e) {
                    Logging.e("loadUnsyncedMessages " + e.getMessage(), e);
                }
            }
        }
        if (mMessagingException != null) {
            MessagingException me = mMessagingException;
            mMessagingException = null;
            throw me;
        }
    }

    /** M: haveMessages
     * Check if there is any unsyncedMessage left
     */
    private Message getUnsyncedMessage() {
        synchronized (mUnsyncedMessages) {
            if (mUnsyncedMessages.isEmpty()) {
                return null;
            }
            return mUnsyncedMessages.remove(0);
        }
    }

    /** M: LoadUnsyncMessageTask is a task for loading unsynced messages
     * Load messages by a separate IMAP connection
     * Check {@link mUnsyncedMessages} to iterate every unsynced message
     * Send notify to wake up {@link loadUnsyncedMessages} to callback sync finished
     * Save MessagingException happened in sync task thread and send it out
     */
    private class LoadUnsyncMessageTask implements Runnable {
        private Account mAccountInner;
        private Store mRemoteStoreInner;
        private Message mUnsyncedMessageInner;
        private Mailbox mToMailboxInner;
        private Context mContext;

        public LoadUnsyncMessageTask(Context context, final Account account, Store remoteStore,
                final Mailbox toMailbox) {
            mAccountInner = account;
            mRemoteStoreInner = remoteStore;
            mToMailboxInner = toMailbox;
            mContext = context;
        }

        public void run() {
            try {
                final Folder remoteFolder = mRemoteStoreInner.getFolder(mToMailboxInner.mServerId);
                remoteFolder.open(OpenMode.READ_WRITE);
                while ((mUnsyncedMessageInner = getUnsyncedMessage()) != null) {
                    ArrayList<Message> messages = new ArrayList<Message>();
                    messages.add(mUnsyncedMessageInner);
                    loadUnsyncedMessages(mContext, mAccountInner, remoteFolder,
                            messages, mToMailboxInner);
                }
                remoteFolder.close(false);
            } catch (MessagingException me) {
                Logging.d("LoadUnsyncMessageAsyncTask", me);
                /** M: Save MessagingException to send exception out */
                mMessagingException = me;
            } finally {
                synchronized (mUnsyncedMessages) {
                    mRunningSyncThreadCount--;
                    if (mRunningSyncThreadCount == 0) {
                        mUnsyncedMessages.notify();
                    }
                }
            }
        }
    }

    /**
     * M: Load the structure and body of messages not yet synced
     * @param account the account we're syncing
     * @param remoteFolder the (open) Folder we're working on
     * @param messages an array of Messages we've got headers for
     * @param toMailbox the destination mailbox we're syncing
     * @throws MessagingException
     */
    public static void loadUnsyncedMessages(final Context context, final Account account,
            Folder remoteFolder, ArrayList<Message> unsyncedMessages, final Mailbox toMailbox)
            throws MessagingException {
        FetchProfile fp = new FetchProfile();

        for (Message message : unsyncedMessages) {
            // Also need to pay attention to the 0 RFC822.SIZE that may occurs in using Gmail Imap.
            // Cope with it as partial download in case that the mail has a big size attachment
            // which can lead to potential OutOfMemory

            // Download messages. We ask the server to give us the message structure,
            // but not all of the attachments.
            fp.clear();
            fp.add(FetchProfile.Item.STRUCTURE);
            remoteFolder.fetch(new Message[] { message }, fp, null);

            boolean isPartialDownload = imapPartialFetchMessage(message, remoteFolder);
            // Store the updated message locally and mark it fully loaded
            Utilities.copyOneMessageToProvider(context, message, account, toMailbox,
                    isPartialDownload ? EmailContent.Message.FLAG_LOADED_PARTIAL
                            : EmailContent.Message.FLAG_LOADED_COMPLETE);
        }
    }

    /** M: Imap partial download implementation.
     *  If MIME contain html parts, we only download html part, otherwise we use text parts.
     * @param message
     * @param remoteFolder
     * @return if partial download success.
     */
    public static boolean imapPartialFetchMessage(Message message, Folder remoteFolder)
            throws MessagingException {
        boolean isPartialDownload = false;
        // We have a structure to deal with, from which
        // we can pull down the parts we want to actually store.
        // Build a list of parts we are interested in. Text parts
        // will be downloaded right now, attachments will be left for later.
        ArrayList<Part> viewables = new ArrayList<Part>();
        ArrayList<Part> attachments = new ArrayList<Part>();

        MimeUtility.collectParts(message, viewables, attachments);

        // Separate plain & html viewable parts
        ArrayList<Part> plainTexts = new ArrayList<Part>();
        ArrayList<Part> htmlTexts = new ArrayList<Part>();
        int plainTextsSize = 0;
        int htmlTextsSize = 0;
        for (Part part : viewables) {
            if (PLAINTEXT_MIMETYPE.equalsIgnoreCase(part.getMimeType())) {
                plainTexts.add(part);
                plainTextsSize += part.getSize();
            }
            if (HTMLTEXT_MIMETYPE.equalsIgnoreCase(part.getMimeType())) {
                htmlTexts.add(part);
                htmlTextsSize += part.getSize();
            }
        }

        if (!htmlTexts.isEmpty()) {
            isPartialDownload = fetchTextParts(htmlTexts, htmlTextsSize, remoteFolder,
                    message);
        } else if (!plainTexts.isEmpty()) {
            isPartialDownload = fetchTextParts(plainTexts, plainTextsSize, remoteFolder,
                    message);
        }
        return isPartialDownload;
    }

    /**
     * M: Fetch text parts partially if it was large enough.
     * @param textParts
     * @param textSize
     * @param remoteFolder
     * @param message
     * @return true if it was a partial fetching, false for totally fetching.
     * @throws MessagingException
     */
    private static boolean fetchTextParts(ArrayList<Part> textParts, int textSize,
            Folder remoteFolder, Message message) throws MessagingException {
        FetchProfile fp = new FetchProfile();
        if (textSize <= MAX_SMALL_MESSAGE_SIZE) {
            for (Part part : textParts) {
                fp.clear();
                fp.add(part);
                remoteFolder.fetch(new Message[] { message }, fp, null);
            }
            return false;
        } else {
            int totalSize = 0;
            int partSize = 0;
            for (Part part : textParts) {
                partSize = 0;
                partSize = part.getSize();
                totalSize = partSize + totalSize;
                fp.clear();
                fp.add(part);
                if (totalSize <= MAX_SMALL_MESSAGE_SIZE) {
                    remoteFolder.fetch(new Message[] { message }, fp, null);
                } else {
                    remoteFolder.fetch(new Message[] { message }, fp, null,
                            MAX_SMALL_MESSAGE_SIZE - totalSize + partSize);
                    break;
                }
            }
            return true;
        }
    }
}
