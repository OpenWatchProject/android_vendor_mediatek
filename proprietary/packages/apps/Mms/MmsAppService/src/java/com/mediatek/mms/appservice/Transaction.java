/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2007-2008 Esmertec AG.
 * Copyright (C) 2007-2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.mms.appservice;

import android.content.Context;
import android.net.Uri;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

/**
 * Transaction is an abstract class for notification transaction, send transaction
 * and other transactions described in MMS spec.
 * It provides the interfaces of them and some common methods for them.
 */
public abstract class Transaction extends Observable {
    // should be same with MessageUtils.MMS_SHARED_FILES_FOLDER_NAME;
    protected static final String PDU_FILES_FOLDER_NAME = "voicemails";

    // need same with android authorities defined in AndroidManifest;
    protected static final String MMS_FILE_PROVIDER_AUTHORITIES =
            "com.mediatek.mms.appservice.files";

    private final int mServiceId;
    protected Context mContext;
    protected String mId;
    protected TransactionState mTransactionState;
    protected int mSubId;
    protected Uri mUri;
    protected File mPduFile;
    protected String mContentLocation;

    protected static final Executor sExecutor = Executors.newFixedThreadPool(2);

    /**
     * Identifies push requests.
     */
    public static final int NOTIFICATION_TRANSACTION = 0;
    /**
     * Identifies deferred retrieve requests.
     */
    public static final int RETRIEVE_TRANSACTION     = 1;
    /**
     * Identifies send multimedia message requests.
     */
    public static final int SEND_TRANSACTION         = 2;
    /**
     * Identifies send read report requests.
     */
    public static final int READREC_TRANSACTION      = 3;

    public Transaction(Context context, int serviceId, int subId) {
        mContext = context;
        mTransactionState = new TransactionState();
        mServiceId = serviceId;
        mSubId = subId;
    }

    public File createPduFile(byte[] pduData, String fileName) {
        File pduFile = null;
        try {
            File baseDir = mContext.getFilesDir();
            File pduFolders = new File(baseDir.toString() + "/" + PDU_FILES_FOLDER_NAME);
            if (!pduFolders.exists()) {
                pduFolders.mkdirs();
                pduFolders.setExecutable(true, false);
            }
            pduFile = new File(pduFolders.toString() + "/" + fileName);
            /*
             * if (pduFile.exists()) { pduFile.delete(); }
             * pduFile.createNewFile();
             */
            FileOutputStream fos = null;
            try {
                fos = new FileOutputStream(pduFile, false);
                if (pduData != null) {
                    fos.write(pduData);
                }
            } finally {
                if (fos != null) {
                    fos.close();
                }
            }
            pduFile.setReadable(true, false);
        } catch (IOException ex) {
            ex.printStackTrace();
            return null;
        }
        return pduFile;
    }

    /**
     * Returns the transaction state of this transaction.
     *
     * @return Current state of the Transaction.
     */
    @Override
    public TransactionState getState() {
        return mTransactionState;
    }

    /**
     * An instance of Transaction encapsulates the actions required
     * during a MMS Client transaction.
     */
    public abstract void process();

    /**
     * Used to determine whether a transaction is equivalent to this instance.
     *
     * @param transaction the transaction which is compared to this instance.
     * @return true if transaction is equivalent to this instance, false otherwise.
     */
    public boolean isEquivalent(Transaction transaction) {
        return mId.equals(transaction.mId);
    }

    /**
     * Get the service-id of this transaction which was assigned by the framework.
     * @return the service-id of the transaction
     */
    public int getServiceId() {
        return mServiceId;
    }

    public long getSubId() {
        return mSubId;
    }

    public Uri getUri() {
        return mUri;
    }

    public String getId() {
        return mId;
    }

    @Override
    public String toString() {
        return getClass().getName() + ": serviceId=" + mServiceId;
    }

    /**
     * Get the type of the transaction.
     *
     * @return Transaction type in integer.
     */
    abstract public int getType();

    public String getContentLocation() {
        return mContentLocation;
    }

    public void setPduFile(File file) {
        mPduFile = file;
    }
}
