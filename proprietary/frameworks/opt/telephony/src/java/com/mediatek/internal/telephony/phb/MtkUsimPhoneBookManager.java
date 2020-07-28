/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.internal.telephony.phb;

import android.os.AsyncResult;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.SparseArray;
import android.util.SparseIntArray;

import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.TelephonyProperties;

import com.android.internal.telephony.gsm.SimTlv;
import com.android.internal.telephony.gsm.UsimPhoneBookManager;
import com.android.internal.telephony.uicc.AdnRecordCache;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppType;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.uicc.IccException;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccIoResult;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.mediatek.internal.telephony.MtkPhoneNumberUtils;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.MtkRILConstants;
import com.mediatek.internal.telephony.uicc.EFResponseData;
import com.mediatek.internal.telephony.uicc.MtkUiccCardApplication;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

import com.mediatek.internal.telephony.phb.CsimPhbUtil;

/**
 * This class implements reading and parsing USIM records.
 * Refer to Spec 3GPP TS 31.102 for more details.
 *
 * {@hide}
 */
public class MtkUsimPhoneBookManager extends UsimPhoneBookManager {
    private static final String LOG_TAG = "MtkUsimPhoneBookManager";
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean DBG =
            SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1 &&
            !SystemProperties.get("ro.build.type").equals("user");
    private ArrayList<PbrRecord> mPbrRecords;
    private MtkAdnRecordCache mAdnCache;
    private MtkRIL mCi;
    private UiccCardApplication mCurrentApp;
    private int mSlotId = -1;
    private final Object mGasLock = new Object();
    private final Object mUPBCapabilityLock = new Object();
    private final Object mAasLock = new Object();

    private ArrayList<MtkAdnRecord> mPhoneBookRecords;
    private int mEmailRecordSize = -1;
    private int mEmailFileSize = USIM_DEFAULT_MAX_EMAIL_FILE_SIZE;
    private int mAdnFileSize = USIM_DEFAULT_MAX_ADN_FILE_SIZE;
    private int mAnrRecordSize = -1;
    private int[] mAdnRecordSize;
    private SparseArray<int[]> mRecordSize;
    private int mSliceCount = 0;
    private int mUpbDone = -1;
    private boolean mIsReset = false;
    private static final int PBR_NOT_NEED_NOTIFY = -1;
    private int mPbrNeedNotify = PBR_NOT_NEED_NOTIFY;
    private int mReadEFLinerRecordSizeNum = 0;

    private ArrayList<UsimGroup> mGasForGrp;
    private ArrayList<String> mAasForAnr;
    private ArrayList<ArrayList<byte[]>> mIapFileList = null;
    private ArrayList<ArrayList<byte[]>> mExt1FileList;

    private boolean mRefreshEmailInfo = false;
    private boolean mRefreshAnrInfo = false;
    private boolean mRefreshAdnInfo = false;
    private int[] mEmailRecTable = new int[USIM_DEFAULT_MAX_EMAIL_FILE_SIZE * 4];
    private int[] mEmailInfo;
    private int[] mSneInfo;
    private ArrayList<int[]> mAnrInfo;
    private int[] mUpbCap = new int[8];
    private int mResult = -1;

    private static final int EVENT_AAS_LOAD_DONE = 5;
    private static final int EVENT_GAS_LOAD_DONE = 6;
    private static final int EVENT_IAP_UPDATE_DONE = 7;
    private static final int EVENT_EMAIL_UPDATE_DONE = 8;
    private static final int EVENT_ANR_UPDATE_DONE = 9;
    private static final int EVENT_AAS_UPDATE_DONE = 10;
    private static final int EVENT_SNE_UPDATE_DONE = 11;
    private static final int EVENT_GRP_UPDATE_DONE = 12;
    private static final int EVENT_GAS_UPDATE_DONE = 13;
    private static final int EVENT_IAP_RECORD_LOAD_DONE = 14;
    private static final int EVENT_EMAIL_RECORD_LOAD_DONE = 15;
    private static final int EVENT_ANR_RECORD_LOAD_DONE = 16;
    private static final int EVENT_GRP_RECORD_LOAD_DONE = 17;
    private static final int EVENT_SNE_RECORD_LOAD_DONE = 18;
    private static final int EVENT_UPB_CAPABILITY_QUERY_DONE = 19;
    private static final int EVENT_SELECT_EF_FILE_DONE = 20;
    private static final int EVENT_QUERY_PHB_ADN_INFO = 21;
    //OPTMZ
    private static final int EVENT_EMAIL_RECORD_LOAD_OPTMZ_DONE = 22;
    private static final int EVENT_ANR_RECORD_LOAD_OPTMZ_DONE = 23;
    private static final int EVENT_SNE_RECORD_LOAD_OPTMZ_DONE = 24;
    private static final int EVENT_QUERY_EMAIL_AVAILABLE_OPTMZ_DONE = 25;
    private static final int EVENT_QUERY_ANR_AVAILABLE_OPTMZ_DONE = 26;
    private static final int EVENT_QUERY_SNE_AVAILABLE_OPTMZ_DONE = 27;
    private static final int EVENT_AAS_LOAD_DONE_OPTMZ = 28;
    // EXT APIs
    private static final int EVENT_GET_RECORDS_SIZE_DONE = 1000;
    private static final int EVENT_EXT1_LOAD_DONE = 1001;
    //USIM type2 conditional bytes length (refer to 3GPP TS31.102)
    private static final int USIM_TYPE2_CONDITIONAL_LENGTH = 2;

    // Error code for USIM Group
    public static final int USIM_ERROR_NAME_LEN = -10; // the input group name is too long!
    public static final int USIM_ERROR_GROUP_COUNT = -20; // outnumber the max count of groups
    public static final int USIM_ERROR_CAPACITY_FULL = -30;
    public static final int USIM_ERROR_STRING_TOOLONG = -40;
    public static final int USIM_ERROR_OTHERS = -50;

    public static final int USIM_MAX_ANR_COUNT = 3;
    private static final int USIM_MAX_AAS_ENTRIES_COUNT = 5;
    private static final int USIM_DEFAULT_MAX_EMAIL_FILE_SIZE = 100;
    private static final int USIM_DEFAULT_MAX_ADN_FILE_SIZE = 250;

    private static final int UPB_EF_ANR = 0;
    private static final int UPB_EF_EMAIL = 1;
    private static final int UPB_EF_SNE = 2;
    private static final int UPB_EF_AAS = 3;
    private static final int UPB_EF_GAS = 4;
    private static final int UPB_EF_GRP = 5;

    private AtomicInteger mReadingAnrNum = new AtomicInteger(0);
    private AtomicInteger mReadingEmailNum = new AtomicInteger(0);
    private AtomicInteger mReadingGrpNum = new AtomicInteger(0);
    private AtomicInteger mReadingSneNum = new AtomicInteger(0);
    private AtomicInteger mReadingIapNum = new AtomicInteger(0);
    private AtomicBoolean mNeedNotify = new AtomicBoolean(false);

    protected EFResponseData mEfData = null;

    // class File represent a PBR record TLV object which points to the rest of the phonebook EFs
    private class File {
        // Phonebook reference file constructed tag defined in 3GPP TS 31.102
        // section 4.4.2.1 table 4.1
        private final int mParentTag;
        // EFID of the file
        private final int mEfid;
        // SFI (Short File Identification) of the file. 0xFF indicates invalid SFI.
        private final int mSfi;
        // The order of this tag showing in the PBR record.
        private final int mIndex;

        // MTK-START
        public int mPbrRecord;
        public int mTag;
        public int mAnrIndex;
        // MTK-END

        File(int parentTag, int efid, int sfi, int index) {
            mParentTag = parentTag;
            mEfid = efid;
            mSfi = sfi;
            mIndex = index;
        }

        public int getParentTag() { return mParentTag; }
        public int getEfid() { return mEfid; }
        public int getSfi() { return mSfi; }
        public int getIndex() { return mIndex; }
        public String toString() {
            return "mParentTag:" + Integer.toHexString(mParentTag).toUpperCase() + ",mEfid:" +
                    Integer.toHexString(mEfid).toUpperCase() + ",mSfi:" +
                    Integer.toHexString(mSfi).toUpperCase() + ",mIndex:" + mIndex + ",mPbrRecord:" +
                    mPbrRecord + ",mAnrIndex" + mAnrIndex + ",mTag:" +
                    Integer.toHexString(mTag).toUpperCase();
        }
    }

    public MtkUsimPhoneBookManager(IccFileHandler fh, AdnRecordCache cache) {
        super(fh, cache);
    }

    public MtkUsimPhoneBookManager(IccFileHandler fh, AdnRecordCache cache,
            CommandsInterface ci, UiccCardApplication app) {
        super(fh, cache);
        mFh = fh;
        mPhoneBookRecords = new ArrayList<MtkAdnRecord>();
        mGasForGrp = new ArrayList<UsimGroup>();
        mIapFileList = new ArrayList<ArrayList<byte[]>>();
        mPbrRecords = null;
        // We assume its present, after the first read this is updated.
        // So we don't have to read from UICC if its not present on subsequent reads.
        mIsPbrPresent = true;
        mAdnCache = (MtkAdnRecordCache) cache;
        mCi = (MtkRIL) ci;
        mCurrentApp = app;
        mSlotId = (app == null) ? -1 : app.getPhoneId();
        mEmailsForAdnRec = new SparseArray<ArrayList<String>>();
        mSfiEfidTable = new SparseIntArray();
        for (int i = 0; i < 8; i++) {
            mUpbCap[i] = -1;
        }
        logi("constructor finished. ");
    }

    public void reset() {
        // Better not call super.reset() here
        // because some field is declared here with same name as parents'.
        mIsReset = true;
        mPhoneBookRecords.clear();
        mIapFileRecord = null;
        mEmailFileRecord = null;
        mPbrRecords = null;
        mIsPbrPresent = true;
        mRefreshCache = false;
        mEmailsForAdnRec.clear();
        mSfiEfidTable.clear();
        mGasForGrp.clear();
        mIapFileList = null;
        mAasForAnr = null;
        mExt1FileList = null;
        mSliceCount = 0;
        mEmailRecTable = new int[400];
        mEmailInfo = null;
        mSneInfo = null;
        mAnrInfo = null;
        for (int i = 0; i < 8; i++) {
            mUpbCap[i] = -1;
        }
        mEmailRecordSize = -1;
        mAnrRecordSize = -1;
        mUpbDone = -1;
        mAdnRecordSize = null;
        mRefreshEmailInfo = false;
        mRefreshAnrInfo = false;
        mRefreshAdnInfo = false;
        synchronized (mLock) {
            mLock.notifyAll();
        }
        mPbrNeedNotify = PBR_NOT_NEED_NOTIFY;
        logi("reset finished. ");
    }

    // Load all phonebook related EFs from the SIM.
    public ArrayList<MtkAdnRecord> loadEfFilesFromUsim(Object object) {
        long prevTime = System.currentTimeMillis();
        synchronized (mLock) {
            mIsReset = false;
            if (!mPhoneBookRecords.isEmpty()) {
                if (mRefreshCache) {
                    mRefreshCache = false;
                    refreshCache();
                }
                return mPhoneBookRecords;
            }

            if (!mIsPbrPresent) return null;

            // Check if the PBR file is present in the cache, if not read it
            // from the USIM.
            if (mPbrRecords == null || mPbrRecords.size() == 0) {
                mPbrNeedNotify++;
                readPbrFileAndWait();
            }

            if (mPbrRecords == null || mPbrRecords.size() == 0) {
                // Reset may happen during read Pbr file, we need to distinguish between
                // being reset or PBR really not exist in the USIM.
                if (checkIsPhbReady() && !mIsReset) {
                    if (true == readAdnFileAndWait(0)) {
                        mIsPbrPresent = false;
                        mEmailRecordSize = 0;
                        mAnrRecordSize = 0;
                        mUpbDone = 1;
                    }
                    logi("loadEfFilesFromUsim getRecordIfLoaded EF_ADN pbrP:" + mIsPbrPresent);
                    return mAdnCache.getRecordsIfLoaded(IccConstants.EF_ADN, null);
                } else {
                    // PBR null due to reset, do nothing.
                    logi("loadEfFilesFromUsim phb not ready and Reset");
                    return null;
                }
            }
            logi("loadEfFilesFromUsim mPbrNeedNotify:" + mPbrNeedNotify);

            if (mEmailRecordSize < 0) {
                readEmailRecordSize();
            }

            if (mAnrRecordSize < 0) {
                readAnrRecordSize();
            }

            int adnEf = mPbrRecords.get(0).mFileIds.get(USIM_EFADN_TAG).getEfid();
            if (adnEf > 0) {
                int [] size = readEFLinearRecordSize(adnEf);
                if (size != null && size.length == 3) {
                    mAdnFileSize = size[2];
                }
            }

            if (null != mPbrRecords.get(0).mFileIds.get(USIM_EFSNE_TAG)) {
                int [] size = readEFLinearRecordSize(
                      mPbrRecords.get(0).mFileIds.get(USIM_EFSNE_TAG).getEfid());
            }

            int numRecs = mPbrRecords.size();
            // CSIM card need CRSM, not CPBR. and need read for every pbr record.
            if (!CsimPhbUtil.hasModemPhbEnhanceCapability(mFh)) {
                for (int i = 0; i < numRecs; i++) {
                    readAASFileAndWait(i);
                    readAdnFileAndWaitForUICC(i);
                }
            } else {
                // read adn by CPBR, not by read record from EF , So we needn't read
                // for every pbr record.
                readAasFileAndWaitOptmz();
                readAdnFileAndWait(0);
            }

            if (mPhoneBookRecords.isEmpty()) {
                logi("loadEfFilesFromUsim mPhoneBookRecords Empty");
                return mPhoneBookRecords;
            }

            // MD3 doesn't support the command, so need to use SimIo
            // Actually CSIM only need support Email & Anr
            if (!CsimPhbUtil.hasModemPhbEnhanceCapability(mFh)) {
                for (int i = 0; i < numRecs; i++) {
                    readSneFileAndWait(i);
                    readAnrFileAndWait(i);
                    readEmailFileAndWait(i);
                }
            } else {
                // Speed up.
                logi("loadEfFilesFromUsim Speed up read begin");
                readSneFileAndWaitOptmz();
                readAnrFileAndWaitOptmz();
                readEmailFileAndWaitOptmz();
                logi("loadEfFilesFromUsim Speed up read end");
            }

            readGrpIdsAndWait();
            // Check if reset happened.
            if (mPbrRecords != null) {
                mUpbDone = 1;
            }
            // All EF files are loaded, return all the records
        }

        // mCurrentApp.getType() will grab self lock which maybe deadlock with
        // UsimPhoneBookManager lock, such as API reset() which is called from
        // UiccCardApplication, so UsimPhoneBookManager lock does not protect
        // UiccCardApplication interface.
        if (mPbrRecords != null) {
            long endTime = System.currentTimeMillis();
            logi("loadEfFilesFromUsim Time: " + (endTime - prevTime) +
                    " AppType: " + mCurrentApp.getType());
        } else {
            logi("loadEfFilesFromUsim end");
        }

        return mPhoneBookRecords;
    }

    // Read EF_EMAIL which contains the email records.
    private void readEmailFileAndWait(int recId) {
        logi("readEmailFileAndWait " + recId);
        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            return;
        }

        SparseArray<File> files;
        files = mPbrRecords.get(recId).mFileIds;
        if (files == null) return;

        File emailFile = files.get(USIM_EFEMAIL_TAG);
        if (emailFile != null) {
            int emailEfid = emailFile.getEfid();

            if (emailFile.getParentTag() == USIM_TYPE1_TAG) {
                readType1Ef(emailFile, 0);
                return;
            } else if (emailFile.getParentTag() == USIM_TYPE2_TAG) {
                readType2Ef(emailFile);
                return;
            }
        }
    }

    // Read Phonebook Index Admistration EF_IAP file
    private void readIapFileAndWait(int pbrIndex, int efid, boolean forceRefresh) {
        logi("readIapFileAndWait pbrIndex :" + pbrIndex + ",efid:" + efid +
                ",forceRefresh:" + forceRefresh);

        if (efid <= 0) return;

        if (mIapFileList == null) {
            logi("readIapFileAndWait IapFileList is null !!!! recreate it !");
            mIapFileList = new ArrayList<ArrayList<byte[]>>();
        }

        int [] size = null;
        if (mRecordSize != null && mRecordSize.get(efid) != null) {
            size = mRecordSize.get(efid);
        } else {
            size = readEFLinearRecordSize(efid);
        }

        if (size == null || size.length != 3) {
            Rlog.e(LOG_TAG, "readIapFileAndWait: read record size error.");

            ArrayList<byte[]> iapList = new ArrayList<byte[]>();
            mIapFileList.add(pbrIndex, iapList);
            return;
        }

        if (mIapFileList.size() <= pbrIndex) {
            log("Create IAP first!");
            ArrayList<byte[]> iapList = new ArrayList<byte[]>();
            byte[] value = null;

            for (int i = 0; i < mAdnFileSize; i++) {
                value = new byte[size[0]];
                for (byte tem : value) {
                    tem = (byte) 0xFF;
                }
                iapList.add(value);
            }
            mIapFileList.add(pbrIndex, iapList);
        } else {
            log("This IAP has been loaded!");
            if (!forceRefresh) {
                return;
            }
        }

        int numAdnRecs = mPhoneBookRecords.size();
        int nOffset = pbrIndex * mAdnFileSize;
        int nMax = nOffset + mAdnFileSize;
        nMax = numAdnRecs < nMax ? numAdnRecs : nMax;

        log("readIapFileAndWait nOffset " + nOffset + ", nMax " + nMax);
        int totalReadingNum = 0;
        for (int i = nOffset; i < nMax; i++) {
            MtkAdnRecord rec;
            try {
                rec = mPhoneBookRecords.get(i);
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG, "readIapFileAndWait: mPhoneBookRecords " +
                        "IndexOutOfBoundsException numAdnRecs is " + numAdnRecs + "index is " + i);
                break;
            }

            if (rec.getAlphaTag().length() > 0 || rec.getNumber().length() > 0) {
                mReadingIapNum.addAndGet(1);
                int[] data = new int[2];
                data[0] = pbrIndex;
                data[1] = i - nOffset;
                mFh.readEFLinearFixed(efid, i + 1 - nOffset, size[0], obtainMessage(
                        EVENT_IAP_RECORD_LOAD_DONE, data));
                totalReadingNum++;
            }
        }

        if (mReadingIapNum.get() == 0) {
            mNeedNotify.set(false);
            return;
        } else {
            mNeedNotify.set(true);
        }
        logi("readIapFileAndWait before mLock.wait " + mNeedNotify.get()
            + " total:" + totalReadingNum);

        synchronized (mLock) {
            try {
                mLock.wait();
            } catch (InterruptedException e) {
                Rlog.e(LOG_TAG, "Interrupted Exception in readIapFileAndWait");
            }
        }

        logi("readIapFileAndWait after mLock.wait after mLock.wait:" + mNeedNotify.get());

        if (true == mNeedNotify.get()) {
            mNeedNotify.set(false);
        }
    }

    //TODO: type3, doesn't need to run twice.
    private void readAASFileAndWait(int recId) {
        logi("readAASFileAndWait " + recId);

        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            return;
        }

        SparseArray<File> files;
        files = mPbrRecords.get(recId).mFileIds;
        if (files == null) return;

        File aasFile = files.get(USIM_EFAAS_TAG);
        if (aasFile == null) return;

        int aasEfid = aasFile.getEfid();
        log("readAASFileAndWait-get AAS EFID " + aasEfid);
        if (mAasForAnr != null) {
            logi("AAS has been loaded for Pbr number " + recId);
            // return or not?
        }

        if (mFh != null) {
            Message msg = obtainMessage(EVENT_AAS_LOAD_DONE);
            msg.arg1 = recId;
            mFh.loadEFLinearFixedAll(aasEfid, msg);

            try {
                mLock.wait();
            } catch (InterruptedException e) {
                Rlog.e(LOG_TAG, "Interrupted Exception in readAASFileAndWait");
            }
        } else {
            Rlog.e(LOG_TAG, "readAASFileAndWait-IccFileHandler is null");
            return;
        }
    }

    private void readSneFileAndWait(int recId) {
        logi("readSneFileAndWait " + recId);

        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            return;
        }

        SparseArray<File> files;
        files = mPbrRecords.get(recId).mFileIds;
        if (files == null) return;

        File sneFile = files.get(USIM_EFSNE_TAG);
        if (sneFile == null) return;

        int sneEfid = sneFile.getEfid();
        log("readSneFileAndWait: EFSNE id is " + sneEfid);

        if (sneFile.getParentTag() == USIM_TYPE2_TAG) {
            readType2Ef(sneFile);
            return;
        } else if (sneFile.getParentTag() == USIM_TYPE1_TAG) {
            readType1Ef(sneFile, 0);
            return;
        }
    }

    private void readAnrFileAndWait(int recId) {
        logi("readAnrFileAndWait: recId is " + recId);

        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            return;
        }

        SparseArray<File> files;
        files = mPbrRecords.get(recId).mFileIds;
        if (files == null) {
            log("readAnrFileAndWait: No anr tag in pbr record " + recId);
            return;
        }

        for (int index = 0; index < mPbrRecords.get(recId).mAnrIndex; index++) {
            File anrFile = files.get(USIM_EFANR_TAG + index * 0x100);

            if (anrFile == null) continue;

            if (anrFile.getParentTag() == USIM_TYPE2_TAG) {
                anrFile.mAnrIndex = index;
                readType2Ef(anrFile);
                //return; // if return, it will only get one anr
            } else if (anrFile.getParentTag() == USIM_TYPE1_TAG) {
                anrFile.mAnrIndex = index;
                readType1Ef(anrFile, index);
                //return;
            }

            break;
        }
    }

    private void readGrpIdsAndWait() {
        logi("readGrpIdsAndWait begin");

        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            return;
        }

        SparseArray<File> files;
        files = mPbrRecords.get(0).mFileIds;
        if (files == null) return;

        File grpFile = files.get(USIM_EFGRP_TAG);
        if (grpFile == null) return;

        int totalReadingNum = 0;
        int numAdnRecs = mPhoneBookRecords.size();
        for (int i = 0; i < numAdnRecs; i++) {
            MtkAdnRecord rec;

            try {
                rec = mPhoneBookRecords.get(i);
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG, "readGrpIdsAndWait: mPhoneBookRecords " +
                        "IndexOutOfBoundsException numAdnRecs is " + numAdnRecs + "index is " + i);
                break;
            }

            if (rec.getAlphaTag().length() > 0 || rec.getNumber().length() > 0) {
                mReadingGrpNum.incrementAndGet();
                int adnIndex = rec.getRecId();
                int[] data = new int[2];

                data[0] = i;
                data[1] = adnIndex;
                mCi.readUPBGrpEntry(adnIndex, obtainMessage(EVENT_GRP_RECORD_LOAD_DONE, data));
                totalReadingNum++;
            }
        }

        if (mReadingGrpNum.get() == 0) {
            mNeedNotify.set(false);
            return;
        } else {
            mNeedNotify.set(true);
        }

        logi("readGrpIdsAndWait before mLock.wait " + mNeedNotify.get() +
                " total:" + totalReadingNum);

        try {
            mLock.wait();
        } catch (InterruptedException e) {
            Rlog.e(LOG_TAG, "Interrupted Exception in readGrpIdsAndWait");
        }

        logi("readGrpIdsAndWait after mLock.wait after mLock.wait " + mNeedNotify.get());

        if (true == mNeedNotify.get()) {
            mNeedNotify.set(false);
        }
    }

    // Read EF_ADN file
    private boolean readAdnFileAndWait(int recId) {
        logi("readAdnFileAndWait begin: recId is " + recId + ",mIsReset:" + mIsReset);

        int previousSize = mPhoneBookRecords.size();

        mAdnCache.requestLoadAllAdnLike(IccConstants.EF_ADN,
        mAdnCache.extensionEfForEf(IccConstants.EF_ADN), obtainMessage(EVENT_USIM_ADN_LOAD_DONE));

        try {
            mLock.wait();
        } catch (InterruptedException e) {
            Rlog.e(LOG_TAG, "Interrupted Exception in readAdnFileAndWait");
        }

        /**
         * The recent added ADN record # would be the reference record size
         * for the rest of EFs associated within this PBR.
         */
        if (mPbrRecords != null && mPbrRecords.size() > recId) {
            mPbrRecords.get(recId).mMasterFileRecordNum = mPhoneBookRecords.size() - previousSize;
        }

        logi("readAdnFileAndWait end: recId is " + recId + ",mIsReset:" + mIsReset);
        // if reset during reading, it will return false
        if (mIsReset == false) {
            return true;
        } else {
            return false;
        }
    }

    // Create the phonebook reference file based on EF_PBR
    private void createPbrFile(ArrayList<byte[]> records) {
        if (records == null) {
            mPbrRecords = null;
            mIsPbrPresent = false;
            return;
        }

        mPbrRecords = new ArrayList<PbrRecord>();
        mSliceCount = 0;
        for (int i = 0; i < records.size(); i++) {
            // Some cards have two records but the 2nd record is filled with all invalid char 0xff.
            // So we need to check if the record is valid or not before adding into the PBR records.
            if (records.get(i)[0] != INVALID_BYTE) {
                mPbrRecords.add(new PbrRecord(records.get(i)));
            }
        }

        for (PbrRecord record : mPbrRecords) {
            File file = record.mFileIds.get(USIM_EFADN_TAG);
            // If the file does not contain EF_ADN, we'll just skip it.
            if (file != null) {
                int sfi = file.getSfi();
                if (sfi != INVALID_SFI) {
                    mSfiEfidTable.put(sfi, record.mFileIds.get(USIM_EFADN_TAG).getEfid());
                }
            }
        }
    }

    // OPTMZ Start
    private void readAasFileAndWaitOptmz() {
        logi("readAasFileAndWaitOptmz begin");

        if (mAasForAnr == null || mAasForAnr.size() == 0) {
            int aasRecNum = 0;

            if (mUpbCap[3] < 0) {
                if (mPbrRecords == null || mPbrRecords.size() == 0) {
                    return;
                }

                SparseArray<File> files;
                files = mPbrRecords.get(0).mFileIds;
                if (files == null) return;

                File aasFile = files.get(USIM_EFAAS_TAG);
                if (aasFile == null) return;

                int [] size = readEFLinearRecordSize(aasFile.getEfid());
                if (size != null && size.length == 3) {
                    aasRecNum = size[2];
                }
            } else {
                aasRecNum = mUpbCap[3];
            }

            if (aasRecNum > USIM_MAX_AAS_ENTRIES_COUNT) {
                aasRecNum = USIM_MAX_AAS_ENTRIES_COUNT;
            }

            mCi.readUPBAasList(1, aasRecNum, obtainMessage(EVENT_AAS_LOAD_DONE_OPTMZ));

            try {
                mLock.wait();
            } catch (InterruptedException e) {
                Rlog.e(LOG_TAG, "Interrupted Exception in readAasFileAndWaitOptmz");
            }
        }
        logi("readAasFileAndWaitOptmz end");
    }

    // Read EF_EMAIL which contains the email records.
    private void readEmailFileAndWaitOptmz() {
        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            return;
        }

        SparseArray<File> files;
        files = mPbrRecords.get(0).mFileIds;

        if (files == null) return;

        File emailFile = files.get(USIM_EFEMAIL_TAG);
        if (emailFile == null) return;

        int totalReadingNum = 0;
        int numAdnRecs = mPhoneBookRecords.size();
        for (int i = 0; i < numAdnRecs; i++) {
            MtkAdnRecord rec;

            try {
                rec = mPhoneBookRecords.get(i);
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG, "readEmailFileAndWaitOptmz: mPhoneBookRecords IndexOutOfBounds"
                        + "numAdnRecs is " + numAdnRecs + "index is " + i);
                break;
            }

            if (rec.getAlphaTag().length() > 0 || rec.getNumber().length() > 0) {
                int[] data = new int[2];
                data[0] = 0;
                data[1] = i;
                int loadWhat = 0;

                loadWhat = EVENT_EMAIL_RECORD_LOAD_OPTMZ_DONE;
                mReadingEmailNum.incrementAndGet();
                mCi.readUPBEmailEntry(i + 1, 1, obtainMessage(loadWhat, data));
                totalReadingNum++;
            }
        }

        if (mReadingEmailNum.get() == 0) {
            mNeedNotify.set(false);
            return;
        } else {
            mNeedNotify.set(true);
        }

        logi("readEmailFileAndWaitOptmz before mLock.wait " + mNeedNotify.get()
            + " total:" + totalReadingNum);

        synchronized (mLock) {
            try {
                mLock.wait();
            } catch (InterruptedException e) {
                Rlog.e(LOG_TAG, "Interrupted Exception in readEmailFileAndWaitOptmz");
            }
        }

        logi("readEmailFileAndWaitOptmz after mLock.wait: " + mNeedNotify.get());

        if (true == mNeedNotify.get()) {
            mNeedNotify.set(false);
        }
    }

    // Read EF_ANR which contains the email records.
    private void readAnrFileAndWaitOptmz() {
        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            return;
        }

        SparseArray<File> files;
        files = mPbrRecords.get(0).mFileIds; //mPbrFile.mFileIds.get(recId);
        if (files == null) return;

        int anrIndex = 0;
        File anrFile = files.get(USIM_EFANR_TAG + 0x100 * anrIndex);
        if (anrFile == null) return;

        int totalReadingNum = 0;
        int numAdnRecs = mPhoneBookRecords.size();
        //TODO: read muliple ANR file
        for (int i = 0; i < numAdnRecs; i++) {
            MtkAdnRecord rec;

            try {
                rec = mPhoneBookRecords.get(i);
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG, "readAnrFileAndWaitOptmz: mPhoneBookRecords IndexOutOfBounds"
                        + "numAdnRecs is " + numAdnRecs + "index is " + i);
                break;
            }

            if (rec.getAlphaTag().length() > 0 || rec.getNumber().length() > 0) {
                int[] data = new int[3];
                data[0] = 0;
                data[1] = i;
                data[2] = anrIndex;
                int loadWhat = 0;

                loadWhat = EVENT_ANR_RECORD_LOAD_OPTMZ_DONE;
                mReadingAnrNum.addAndGet(1);
                mCi.readUPBAnrEntry(i + 1, anrIndex + 1, obtainMessage(loadWhat, data));
                totalReadingNum++;
            }
        }

        if (mReadingAnrNum.get() == 0) {
            mNeedNotify.set(false);
            return;
        } else {
            mNeedNotify.set(true);
        }

        logi("readAnrFileAndWaitOptmz before mLock.wait " + mNeedNotify.get()
            + " total:" + totalReadingNum);

        synchronized (mLock) {
            try {
                mLock.wait();
            } catch (InterruptedException e) {
                Rlog.e(LOG_TAG, "Interrupted Exception in readAnrFileAndWaitOptmz");
            }
        }

        logi("readAnrFileAndWaitOptmz after mLock.wait:" + mNeedNotify.get());

        if (true == mNeedNotify.get()) {
            mNeedNotify.set(false);
        }
    }

    // Read EF_SNE which contains the email records.
    private void readSneFileAndWaitOptmz() {
        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            return;
        }

        SparseArray<File> files;
        files = mPbrRecords.get(0).mFileIds;
        if (files == null) return;

        File sneFile = files.get(USIM_EFSNE_TAG);
        if (sneFile == null) return;

        int totalReadingNum = 0;
        int numAdnRecs = mPhoneBookRecords.size();
        for (int i = 0; i < numAdnRecs; i++) {
            MtkAdnRecord rec;

            try {
                rec = mPhoneBookRecords.get(i);
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG, "readSneFileAndWaitOptmz: mPhoneBookRecords IndexOutOfBounds"
                        + "numAdnRecs is " + numAdnRecs + "index is " + i);
                break;
            }

            if (rec.getAlphaTag().length() > 0 || rec.getNumber().length() > 0) {
                int[] data = new int[2];
                data[0] = 0;
                data[1] = i;
                int loadWhat = 0;

                loadWhat = EVENT_SNE_RECORD_LOAD_OPTMZ_DONE;
                mReadingSneNum.incrementAndGet();
                mCi.readUPBSneEntry(i + 1, 1, obtainMessage(loadWhat, data));
                totalReadingNum++;
            }
        }

        if (mReadingSneNum.get() == 0) {
            mNeedNotify.set(false);
            return;
        } else {
            mNeedNotify.set(true);
        }

        logi("readSneFileAndWaitOptmz before mLock.wait " + mNeedNotify.get()
            + " total:" + totalReadingNum);

        synchronized (mLock) {
            try {
                mLock.wait();
            } catch (InterruptedException e) {
                Rlog.e(LOG_TAG, "Interrupted Exception in readSneFileAndWaitOptmz");
            }
        }

        logi("readSneFileAndWaitOptmz after mLock.wait: " + mNeedNotify.get());

        if (true == mNeedNotify.get()) {
            mNeedNotify.set(false);
        }
    }

    // data[0], data[1], payload
    private void updatePhoneAdnRecordWithEmailByIndexOptmz(
            int emailIndex, int adnIndex, String email) {
        log("updatePhoneAdnRecordWithEmailByIndex emailIndex = " + emailIndex +
                ",adnIndex = " + adnIndex);

        if (email == null) {
            return;
        }

        try {
            if (email != null && !email.equals("")) {
                MtkAdnRecord rec = mPhoneBookRecords.get(adnIndex);
                rec.setEmails(new String[] {
                    email
                });
            }
        } catch (IndexOutOfBoundsException e) {
            Rlog.e(LOG_TAG, "[JE]updatePhoneAdnRecordWithEmailByIndex " + e.getMessage());
        }
    }

    //data[0], data[1], data[2], payload
    private void updatePhoneAdnRecordWithAnrByIndexOptmz(
            int recId, int adnIndex, int anrIndex, PhbEntry anrData) {
        log("updatePhoneAdnRecordWithAnrByIndexOptmz the " + adnIndex + " anr record:" + anrData);

        if (anrData != null && anrData.number != null && !anrData.number.equals("")) {
            String anr;

            if (anrData.ton == MtkPhoneNumberUtils.TOA_International) {
                anr = MtkPhoneNumberUtils.prependPlusToNumber(anrData.number);
            } else {
                anr = anrData.number;
            }

            // replace ? with N
            anr = anr.replace('?', MtkPhoneNumberUtils.WILD);
            // replace p with ,
            anr = anr.replace('p', MtkPhoneNumberUtils.PAUSE);
            // replace w with ;
            anr = anr.replace('w', MtkPhoneNumberUtils.WAIT);

            int anrAas = anrData.index;

            if (anr != null && !anr.equals("")) {
                String aas = null;
                if (anrAas > 0 && anrAas != 0xFF) {
                    if (mAasForAnr != null && anrAas <= mAasForAnr.size()) {
                        aas = mAasForAnr.get(anrAas - 1);
                    }
                }
                log(" updatePhoneAdnRecordWithAnrByIndex "
                        + adnIndex + " th anr is " + anr + " the anrIndex is " + anrIndex);
                // SIM record numbers are 1 based.
                MtkAdnRecord rec;

                try {
                    rec = mPhoneBookRecords.get(adnIndex);
                } catch (IndexOutOfBoundsException e) {
                    Rlog.e(LOG_TAG, "updatePhoneAdnRecordWithAnrByIndex: mPhoneBookRecords " +
                            "IndexOutOfBoundsException size: " + mPhoneBookRecords.size() +
                            "index: " + adnIndex);
                    return;
                }

                rec.setAnr(anr, anrIndex);
                if (aas != null && aas.length() > 0) {
                    rec.setAasIndex(anrAas);
                }
                mPhoneBookRecords.set(adnIndex, rec);
            }
        }
    }

    private String[] buildAnrRecordOptmz(String number, int aas) {
        int ton = 0x81;

        // eliminate '+' from number
        if (number.indexOf('+') != -1) {
            if (number.indexOf('+') != number.lastIndexOf('+')) {
                // there are multiple '+' in the String
                Rlog.w(LOG_TAG, "There are multiple '+' in the number: " + number);
            }
            ton = 0x91;
            number = number.replace("+", "");
        }
        // replace N with ?
        number = number.replace(MtkPhoneNumberUtils.WILD, '?');
        // replace , with p
        number = number.replace(MtkPhoneNumberUtils.PAUSE, 'p');
        // replace ; with w
        number = number.replace(MtkPhoneNumberUtils.WAIT, 'w');

        // Add by mtk80995 replace \ to \5c and replace " to \22 for MTK modem
        // the order is very important! for "\\" is substring of "\\22"
        //alphaId = alphaId.replace("\\", "\\5c");
        //alphaId = alphaId.replace("\"", "\\22");
        // end Add by mtk80995
        String[] res = new String[3];
        res[0] = number;
        res[1] = Integer.toString(ton);
        res[2] = Integer.toString(aas);

        return res;
    }

    //data[1], sne
    private void updatePhoneAdnRecordWithSneByIndexOptmz(int adnIndex, String sne) {
        if (sne == null) {
            return;
        }
        //String sne = IccUtils.adnStringFieldToString(recData, 0, recData.length);
        log("updatePhoneAdnRecordWithSneByIndex index " + adnIndex
                + " recData file is " + sne);
        if (sne != null && !sne.equals("")) {
            MtkAdnRecord rec;

            try {
                rec = mPhoneBookRecords.get(adnIndex);
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG, "updatePhoneAdnRecordWithSneByIndex: mPhoneBookRecords " +
                        "IndexOutOfBoundsException size() is " + mPhoneBookRecords.size() +
                        "index is " + adnIndex);
                return;
            }

            rec.setSne(sne);
        }
    }

    @Override
    public void handleMessage(Message msg) {
        AsyncResult ar;
        int[] userData = null;
        boolean isNotify;
        switch (msg.what) {
            case EVENT_QUERY_PHB_ADN_INFO:
                logi("EVENT_QUERY_PHB_ADN_INFO");

                ar = (AsyncResult) msg.obj;

                if (ar.exception == null) {
                    int[] info = (int[]) (ar.result);
                    if (info != null && info.length == 4) {
                        mAdnRecordSize = new int[4];
                        mAdnRecordSize[0] = info[0]; // # of used entries
                        mAdnRecordSize[1] = info[1]; // # of total entries
                        mAdnRecordSize[2] = info[2]; // # max length of number
                        mAdnRecordSize[3] = info[3]; // # max length of alpha id
                        log("recordSize[0]=" + mAdnRecordSize[0] +
                            ",recordSize[1]=" + mAdnRecordSize[1] +
                            ",recordSize[2]=" + mAdnRecordSize[2] +
                            ",recordSize[3]=" + mAdnRecordSize[3]);
                    } else {
                        mAdnRecordSize = new int[4];
                        mAdnRecordSize[0] = 0; // # of used entries
                        mAdnRecordSize[1] = 0; // # of total entries
                        mAdnRecordSize[2] = 0; // # max length of number
                        mAdnRecordSize[3] = 0; // # max length of alpha id
                    }
                }

                synchronized (mLock) {
                    mLock.notify();
                }
                break;
            case EVENT_PBR_LOAD_DONE:
                logi("handleMessage: EVENT_PBR_LOAD_DONE:" + mPbrNeedNotify);

                ar = (AsyncResult) msg.obj;

                if (ar.exception == null) {
                    createPbrFile((ArrayList<byte[]>) ar.result);
                }
                // To prevent no need notify.
                if (mPbrNeedNotify == PBR_NOT_NEED_NOTIFY) {
                    break;
                }

                synchronized (mLock) {
                    mLock.notify();
                }
                mPbrNeedNotify--;
                break;
            case EVENT_USIM_ADN_LOAD_DONE:
                logi("Loading USIM ADN records done");

                ar = (AsyncResult) msg.obj;

                if (ar.exception == null && mPhoneBookRecords != null) {
                    /// M: CSIM PHB handling @{
                    if (!CsimPhbUtil.hasModemPhbEnhanceCapability(mFh)
                            && mPhoneBookRecords.size() > 0
                            && ar.result != null) {
                        ArrayList<MtkAdnRecord> adnList = changeAdnRecordNumber(
                                mPhoneBookRecords.size(),(ArrayList<MtkAdnRecord>) ar.result);
                        mPhoneBookRecords.addAll(adnList);
                        //add for check csim phb storage
                        CsimPhbUtil.initPhbStorage(adnList);
                        //add for check csim phb storage
                    } else {
                        if (ar.result != null) {
                            mPhoneBookRecords.addAll((ArrayList<MtkAdnRecord>) ar.result);
                            //add for check csim phb storage
                            if (!CsimPhbUtil.hasModemPhbEnhanceCapability(mFh)) {
                               CsimPhbUtil.initPhbStorage((ArrayList<MtkAdnRecord>)
                                       ar.result);
                            }
                            //add for check csim phb storage
                            log("Loading USIM ADN records " + mPhoneBookRecords.size());
                        } else {
                            log("Loading USIM ADN records ar.result:" + ar.result);
                        }
                    }
                    /// @}
                } else {
                    Rlog.w(LOG_TAG, "Loading USIM ADN records fail.");
                }

                synchronized (mLock) {
                    mLock.notify();
                }
                break;
            case EVENT_ANR_RECORD_LOAD_DONE:
                ar = (AsyncResult) msg.obj;
                userData = (int[]) (ar.userObj);
                IccIoResult result = (IccIoResult) ar.result;

                if (result != null) {
                    IccException iccException = result.getException();

                    if (iccException == null) {
                        updatePhoneAdnRecordWithAnrByIndex(userData[0], userData[1],
                                                           userData[2], result.payload);
                    }
                }

                mReadingAnrNum.decrementAndGet();
                log("haman, mReadingAnrNum when load done after minus: " + mReadingAnrNum.get() +
                        ", mNeedNotify:" + mNeedNotify.get());
                if (mReadingAnrNum.get() == 0) {
                    if (mNeedNotify.get()) {
                        mNeedNotify.set(false);
                        synchronized (mLock) {
                            mLock.notify();
                        }
                    }
                    logi("EVENT_ANR_RECORD_LOAD_DONE end mLock.notify");
                }
                break;
            case EVENT_IAP_LOAD_DONE:
                logi("Loading USIM IAP records done");

                ar = (AsyncResult) msg.obj;

                if (ar.exception == null) {
                    mIapFileRecord = ((ArrayList<byte[]>) ar.result);
                }

                synchronized (mLock) {
                    mLock.notify();
                }
                break;
            case EVENT_EMAIL_LOAD_DONE:
                logi("Loading USIM Email records done");

                ar = (AsyncResult) msg.obj;

                if (ar.exception == null) {
                    mEmailFileRecord = ((ArrayList<byte[]>) ar.result);
                }

                synchronized (mLock) {
                    mLock.notify();
                }
                break;
            case EVENT_EMAIL_RECORD_LOAD_DONE:
                ar = (AsyncResult) msg.obj;
                userData = (int[]) (ar.userObj);
                IccIoResult em = (IccIoResult) ar.result;

                log("Loading USIM email record done email index:" + userData[0] +
                        ", adn i:" + userData[1]);
                if (em != null) {
                    IccException iccException = em.getException();

                    if (iccException == null) {
                        updatePhoneAdnRecordWithEmailByIndex(userData[0], userData[1], em.payload);
                    }
                }

                mReadingEmailNum.decrementAndGet();
                log("haman, mReadingEmailNum when load done after minus: "
                        + mReadingEmailNum.get() + ", mNeedNotify:" + mNeedNotify.get());

                if (mReadingEmailNum.get() == 0) {
                    if (mNeedNotify.get()) {
                        mNeedNotify.set(false);
                        synchronized (mLock) {
                            mLock.notify();
                        }
                    }
                    logi("EVENT_EMAIL_RECORD_LOAD_DONE end mLock.notify");
                }
                break;
            case EVENT_EMAIL_UPDATE_DONE:
                logi("Updating USIM Email records done");

                ar = (AsyncResult) msg.obj;

                if (ar.exception == null) {
                    log("Updating USIM Email records successfully!");
                    mRefreshEmailInfo = true;
                } else {
                    Rlog.e(LOG_TAG, "EVENT_EMAIL_UPDATE_DONE exception", ar.exception);
                }

                synchronized (mLock) {
                    mLock.notify();
                }
                break;
            case EVENT_IAP_UPDATE_DONE:
                logi("Updating USIM IAP records done");

                ar = (AsyncResult) msg.obj;

                if (ar.exception == null) {
                    log("Updating USIM IAP records successfully!");
                }
                break;
            case EVENT_ANR_UPDATE_DONE:
                logi("Updating USIM ANR records done");

                ar = (AsyncResult) msg.obj;

                IccIoResult res = (IccIoResult) ar.result;

                if (ar.exception != null)  {
                    Rlog.e(LOG_TAG, "EVENT_ANR_UPDATE_DONE exception", ar.exception);
                } else {
                    if (res != null) {
                        IccException exception = res.getException();

                        if (exception == null) {
                            log("Updating USIM ANR records successfully!");
                            mRefreshAnrInfo = true;
                        }
                    } else {
                        mRefreshAnrInfo = true;
                    }
                }

                synchronized (mLock) {
                    mLock.notify();
                }
                break;
            case EVENT_GRP_RECORD_LOAD_DONE:
                ar = (AsyncResult) msg.obj;
                userData = (int[]) (ar.userObj);
                isNotify = mNeedNotify.get();

                if (ar.result != null) {
                    int[] grpIds = (int[]) ar.result;

                    if (grpIds.length > 0) {
                        updatePhoneAdnRecordWithGrpByIndex(userData[0], userData[1], grpIds);
                    }
                }

                mReadingGrpNum.decrementAndGet();
                log("haman, mReadingGrpNum when load done after minus: " +
                        mReadingGrpNum.get() + ",mNeedNotify:" + isNotify);

                if (mReadingGrpNum.get() == 0) {
                    if (mNeedNotify.get()) {
                        mNeedNotify.set(false);
                        synchronized (mLock) {
                            mLock.notify();
                        }
                    }
                    logi("EVENT_GRP_RECORD_LOAD_DONE end mLock.notify:" + isNotify);
                }
                break;
            case EVENT_UPB_CAPABILITY_QUERY_DONE:
                logi("Query UPB capability done");

                ar = (AsyncResult) msg.obj;

                if (ar.exception == null) {
                    mUpbCap = ((int[]) ar.result);
                }

                synchronized (mUPBCapabilityLock) {
                    mUPBCapabilityLock.notify();
                }
                break;
            case EVENT_GAS_LOAD_DONE:
                logi("Load UPB GAS done");

                ar = (AsyncResult) msg.obj;

                if (ar.exception == null) {
                    String[] gasList = ((String[]) ar.result);
                    if (gasList != null && gasList.length > 0) {
                        mGasForGrp = new ArrayList<UsimGroup>();
                        for (int i = 0; i < gasList.length; i++) {
                            String gas = decodeGas(gasList[i]);

                            UsimGroup uGasEntry = new UsimGroup(i + 1, gas);
                            mGasForGrp.add(uGasEntry);
                            log("Load UPB GAS done i is " + i + ", gas is " + gas);
                        }
                    }
                }

                synchronized (mGasLock) {
                    mGasLock.notify();
                }
                break;
            case EVENT_GAS_UPDATE_DONE:
                logi("update UPB GAS done");

                ar = (AsyncResult) msg.obj;

                if (ar.exception == null) {
                    mResult = 0;
                } else {
                    CommandException e = (CommandException) ar.exception;

                    if (e.getCommandError() == CommandException.Error.OEM_ERROR_2) {
                        mResult = USIM_ERROR_NAME_LEN;
                    } else if (e.getCommandError() == CommandException.Error.OEM_ERROR_3) {
                        mResult = USIM_ERROR_GROUP_COUNT;
                    } else {
                        mResult = -1;
                    }
                }

                logi("update UPB GAS done mResult is " + mResult);

                synchronized (mGasLock) {
                    mGasLock.notify();
                }
                break;
            case EVENT_GRP_UPDATE_DONE:
                logi("update UPB GRP done");

                ar = (AsyncResult) msg.obj;

                if (ar.exception == null) {
                    mResult = 0;
                } else {
                    mResult = -1; // todo: set the error code
                }

                synchronized (mLock) {
                    mLock.notify();
                }
                break;
            case EVENT_AAS_LOAD_DONE:

                ar = (AsyncResult) msg.obj;
                int pbrIndexAAS = msg.arg1;

                logi("EVENT_AAS_LOAD_DONE done pbr " + pbrIndexAAS);

                ArrayList<byte[]> aasFileRecords;
                if (ar.exception == null) {
                    aasFileRecords = ((ArrayList<byte[]>) ar.result);

                    if (aasFileRecords != null) {
                        int size = aasFileRecords.size();
                        ArrayList<String> list = new ArrayList<String>();

                        for (int i = 0; i < size; i++) {
                            byte[] aas = aasFileRecords.get(i);
                            if (aas == null) {
                                list.add(null);
                                continue;
                            }
                            String aasAlphaTag =
                                     IccUtils.adnStringFieldToString(aas, 0, aas.length);
                            list.add(aasAlphaTag);
                        }
                        mAasForAnr = list;
                    }
                }

                synchronized (mLock) {
                    mLock.notify();
                }
                break;
            case EVENT_AAS_UPDATE_DONE:
                logi("EVENT_AAS_UPDATE_DONE done.");

                synchronized (mAasLock) {
                    mAasLock.notify();
                }
                break;
            case EVENT_GET_RECORDS_SIZE_DONE:
                ar = (AsyncResult) (msg.obj);
                int efid = msg.arg1;

                logi("EVENT_GET_RECORDS_SIZE_DONE done, recNum:" + mReadEFLinerRecordSizeNum +
                        ", ef_id:" + efid);

                if (ar.exception == null) {
                    int[] recordSize = (int[]) ar.result;
                    if (recordSize.length == 3) {
                        if (mRecordSize == null) {
                            mRecordSize = new SparseArray<int[]>();
                        }
                        mRecordSize.put(efid, recordSize);
                    } else {
                        Rlog.e(LOG_TAG, "get wrong record size format" + ar.exception);
                    }
                } else {
                    Rlog.e(LOG_TAG, "get EF record size failed" + ar.exception);
                }
                if (mReadEFLinerRecordSizeNum > 0) {
                    synchronized (mLock) {
                        mLock.notify();
                    }
                }
                break;
            case EVENT_EXT1_LOAD_DONE:
                ar = (AsyncResult) msg.obj;
                int pbrIndexExt1 = msg.arg1;

                logi("EVENT_EXT1_LOAD_DONE done pbr " + pbrIndexExt1);

                if (ar.exception == null) {
                    ArrayList<byte[]> record = ((ArrayList<byte[]>) ar.result);

                    if (record != null) {
                        log("EVENT_EXT1_LOAD_DONE done size " + record.size());
                        if (mExt1FileList == null) {
                            mExt1FileList = new ArrayList<ArrayList<byte[]>>();
                        }
                        mExt1FileList.add(record);
                    }
                }

                synchronized (mLock) {
                    mLock.notify();
                }
                break;
            case EVENT_SNE_RECORD_LOAD_DONE:
                logi("Loading USIM SNE record done");
                ar = (AsyncResult) msg.obj;
                userData = (int[]) (ar.userObj);
                IccIoResult r = (IccIoResult) ar.result;

                if (r != null) {
                    IccException iccException = r.getException();

                    if (iccException == null) {
                        log("Loading USIM SNE record done result is "
                                + IccUtils.bytesToHexString(r.payload));
                        updatePhoneAdnRecordWithSneByIndex(userData[0], userData[1], r.payload);
                    }
                }

                mReadingSneNum.decrementAndGet();
                log("haman, mReadingSneNum when load done after minus: " + mReadingSneNum.get() +
                         ",mNeedNotify:" + mNeedNotify.get());
                if (mReadingSneNum.get() == 0) {
                    if (mNeedNotify.get()) {
                        mNeedNotify.set(false);
                        synchronized (mLock) {
                            mLock.notify();
                        }
                    }
                    logi("EVENT_SNE_RECORD_LOAD_DONE end mLock.notify");
                }
                break;
            case EVENT_SNE_UPDATE_DONE:
                logi("update UPB SNE done");
                ar = (AsyncResult) msg.obj;
                if (ar.exception != null) {
                    Rlog.e(LOG_TAG, "EVENT_SNE_UPDATE_DONE exception", ar.exception);
                    CommandException e = (CommandException) ar.exception;

                    if (e.getCommandError() == CommandException.Error.OEM_ERROR_2) {
                        mResult = USIM_ERROR_STRING_TOOLONG;
                    } else if (e.getCommandError() == CommandException.Error.OEM_ERROR_3) {
                        mResult = USIM_ERROR_CAPACITY_FULL;
                    } else {
                        mResult = USIM_ERROR_OTHERS;
                    }
                } else {
                    mResult = 0;
                }

                synchronized (mLock) {
                    mLock.notify();
                }
                break;
            case EVENT_IAP_RECORD_LOAD_DONE:
                ar = (AsyncResult) msg.obj;
                userData = (int[]) (ar.userObj);
                IccIoResult re = (IccIoResult) ar.result;
                isNotify = mNeedNotify.get();
                if (re != null && mIapFileList != null) {
                    IccException iccException = re.getException();

                    if (iccException == null) {
                        log("Loading USIM Iap record done result is " +
                                IccUtils.bytesToHexString(re.payload));

                        ArrayList<byte[]> iapList;

                        try {
                            iapList = mIapFileList.get(userData[0]);

                            if (iapList.size() > 0) {
                                iapList.set(userData[1], re.payload);
                            } else {
                                Rlog.w(LOG_TAG, "Warning: IAP size is 0");
                            }
                        } catch (IndexOutOfBoundsException e) {
                            Rlog.e(LOG_TAG, "Index out of bounds.");
                        }
                    }
                }

                mReadingIapNum.decrementAndGet();
                log("haman, mReadingIapNum when load done after minus: " +
                    mReadingIapNum.get() + ",mNeedNotify " + mNeedNotify.get() +
                    ", Iap pbr:" + userData[0] + ", adn i:" + userData[1]);

                if (mReadingIapNum.get() == 0) {
                    if (mNeedNotify.get()) {
                        mNeedNotify.set(false);
                        synchronized (mLock) {
                            mLock.notify();
                        }
                    }
                    logi("EVENT_IAP_RECORD_LOAD_DONE end mLock.notify:" + isNotify);
                }
                break;
            case EVENT_SELECT_EF_FILE_DONE:
                ar = (AsyncResult) msg.obj;

                if (ar.exception == null) {
                    mEfData = (EFResponseData) ar.result;
                } else {
                    Rlog.w(LOG_TAG, "Select EF file fail" + ar.exception);
                }

                synchronized (mLock) {
                    mLock.notify();
                }
                break;
            case EVENT_EMAIL_RECORD_LOAD_OPTMZ_DONE:
                ar = (AsyncResult) msg.obj;
                userData = (int[]) (ar.userObj);
                String emailResult = (String) ar.result;
                isNotify = mNeedNotify.get();

                if (emailResult != null && ar.exception == null) {
                    log("Loading USIM Email record done result is " + emailResult);
                    updatePhoneAdnRecordWithEmailByIndexOptmz(
                            userData[0], userData[1], emailResult);
                }

                mReadingEmailNum.decrementAndGet();
                log("haman, mReadingEmailNum when load done after minus: " +
                        mReadingEmailNum.get() + ", mNeedNotify:" + mNeedNotify.get() +
                        ", email index:" + userData[0] + ", adn i:" + userData[1]);

                if (mReadingEmailNum.get() == 0) {
                    if (mNeedNotify.get()) {
                        mNeedNotify.set(false);
                        synchronized (mLock) {
                            mLock.notify();
                        }
                    }
                    logi("EVENT_EMAIL_RECORD_LOAD_OPTMZ_DONE end mLock.notify:" + isNotify);
                }
                break;
            case EVENT_QUERY_EMAIL_AVAILABLE_OPTMZ_DONE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    mEmailInfo = (int[]) ar.result;
                    if (mEmailInfo == null) {
                        log("mEmailInfo Null!");
                    } else {
                        logi("mEmailInfo = " + mEmailInfo[0] + " " + mEmailInfo[1] + " " +
                                mEmailInfo[2]);
                    }
                }

                synchronized (mLock) {
                    mLock.notify();
                }
                break;
            case EVENT_QUERY_ANR_AVAILABLE_OPTMZ_DONE:
                ar = (AsyncResult) msg.obj;
                int[] tmpAnrInfo = (int[]) ar.result;

                if (ar.exception == null) {
                    if (tmpAnrInfo == null) {
                        log("tmpAnrInfo Null!");
                    } else {
                        logi("tmpAnrInfo = " + tmpAnrInfo[0] + " " + tmpAnrInfo[1] + " " +
                                tmpAnrInfo[2]);
                        if (mAnrInfo == null) {
                            mAnrInfo = new ArrayList<int[]>();
                        } else if (mAnrInfo.size() > 0) {
                            mAnrInfo.clear();
                        }
                        mAnrInfo.add(tmpAnrInfo);
                    }
                }

                synchronized (mLock) {
                    mLock.notify();
                }
                break;
            case EVENT_QUERY_SNE_AVAILABLE_OPTMZ_DONE:
                ar = (AsyncResult) msg.obj;

                if (ar.exception == null) {
                    mSneInfo = (int[]) ar.result;
                    if (mSneInfo == null) {
                        log("mSneInfo Null!");
                    } else {
                        logi("mSneInfo = " + mSneInfo[0] + " " + mSneInfo[1] + " " +
                                mSneInfo[2]);
                    }
                }

                synchronized (mLock) {
                    mLock.notify();
                }
                break;
            case EVENT_SNE_RECORD_LOAD_OPTMZ_DONE:
                ar = (AsyncResult) msg.obj;
                userData = (int[]) (ar.userObj);
                String sneResult = (String) ar.result;

                isNotify = mNeedNotify.get();
                if (sneResult != null && ar.exception == null) {
                    sneResult = decodeGas(sneResult);
                    log("Loading USIM Sne record done result is " + sneResult);
                    updatePhoneAdnRecordWithSneByIndexOptmz(userData[1], sneResult);
                }

                mReadingSneNum.decrementAndGet();
                log("haman, mReadingSneNum when load done after minus: " +
                        mReadingSneNum.get() + ", mNeedNotify:" + mNeedNotify.get() +
                        ", sne index:" + userData[0] + ", adn i:" + userData[1]);

                if (mReadingSneNum.get() == 0) {
                    if (mNeedNotify.get()) {
                        mNeedNotify.set(false);
                        synchronized (mLock) {
                            mLock.notify();
                        }
                    }
                    logi("EVENT_SNE_RECORD_LOAD_OPTMZ_DONE end mLock.notify:" + isNotify);
                }
                break;
            case EVENT_ANR_RECORD_LOAD_OPTMZ_DONE:
                ar = (AsyncResult) msg.obj;
                userData = (int[]) (ar.userObj);
                PhbEntry[] anrResult = (PhbEntry[]) ar.result;
                isNotify = mNeedNotify.get();

                if (anrResult != null && ar.exception == null) {
                    log("Loading USIM Anr record done result is " + anrResult[0]);
                    updatePhoneAdnRecordWithAnrByIndexOptmz(
                            userData[0], userData[1], userData[2], anrResult[0]);
                }

                mReadingAnrNum.decrementAndGet();
                log("haman, mReadingAnrNum when load done after minus: " +
                        mReadingAnrNum.get() + ", mNeedNotify:" + mNeedNotify.get() +
                        ", anr index:" + userData[2] + ", adn i:" + userData[1]);
                if (mReadingAnrNum.get() == 0) {
                    if (mNeedNotify.get()) {
                        mNeedNotify.set(false);
                        synchronized (mLock) {
                            mLock.notify();
                        }
                    }
                    logi("EVENT_ANR_RECORD_LOAD_OPTMZ_DONE end mLock.notify:" + isNotify);
                }
                break;
            case EVENT_AAS_LOAD_DONE_OPTMZ:
                logi("Load UPB AAS done");

                ar = (AsyncResult) msg.obj;

                if (ar.exception == null) {
                    String[] aasList = ((String[]) ar.result);
                    if (aasList != null && aasList.length > 0) {
                        mAasForAnr = new ArrayList<String>();
                        for (int i = 0; i < aasList.length; i++) {
                            String aas = decodeGas(aasList[i]);
                            mAasForAnr.add(aas);
                            log("Load UPB AAS done i is " + i + ", aas is " + aas);
                        }
                    }
                }

                synchronized (mLock) {
                    mLock.notify();
                }
                break;
            default:
                Rlog.e(LOG_TAG, "UnRecognized Message : " + msg.what);
                break;
        }
    }

    // PbrRecord represents a record in EF_PBR
    private class PbrRecord {
        // TLV tags
        private SparseArray<File> mFileIds;
        private int mAnrIndex = 0;
        /**
         * 3GPP TS 31.102 4.4.2.1 EF_PBR (Phone Book Reference file)
         * If this is type 1 files, files that contain as many records as the
         * reference/master file (EF_ADN, EF_ADN1) and are linked on record number
         * bases (Rec1 -> Rec1). The master file record number is the reference.
         */
        private int mMasterFileRecordNum;

        PbrRecord(byte[] record) {
            mFileIds = new SparseArray<File>();
            SimTlv recTlv;
            logi("PBR rec: " + IccUtils.bytesToHexString(record));
            recTlv = new SimTlv(record, 0, record.length);
            parseTag(recTlv);
        }

        void parseTag(SimTlv tlv) {
            SimTlv tlvEfSfi;
            int tag;
            byte[] data;

            do {
                tag = tlv.getTag();
                switch (tag) {
                case USIM_TYPE1_TAG: // A8
                case USIM_TYPE3_TAG: // AA
                case USIM_TYPE2_TAG: // A9
                    data = tlv.getData();
                    tlvEfSfi = new SimTlv(data, 0, data.length);
                    parseEfAndSFI(tlvEfSfi, tag);
                    break;
                default:
                    break;
                }
            } while (tlv.nextObject());
            mSliceCount++;
        }

        void parseEfAndSFI(SimTlv tlv, int parentTag) {
            int tag;
            byte[] data;
            int tagNumberWithinParentTag = 0;
            do {
                tag = tlv.getTag();
                // log("parseEf tag is " + tag);
                switch (tag) {
                    case USIM_EFEMAIL_TAG:
                    case USIM_EFADN_TAG:
                    case USIM_EFEXT1_TAG:
                    case USIM_EFANR_TAG:
                    case USIM_EFPBC_TAG:
                    case USIM_EFGRP_TAG:
                    case USIM_EFAAS_TAG:
                    case USIM_EFGSD_TAG:
                    case USIM_EFUID_TAG:
                    case USIM_EFCCP1_TAG:
                    case USIM_EFIAP_TAG:
                    case USIM_EFSNE_TAG:
                        /** 3GPP TS 31.102, 4.4.2.1 EF_PBR (Phone Book Reference file)
                         *
                         * The SFI value assigned to an EF which is indicated in EF_PBR shall
                         * correspond to the SFI indicated in the TLV object in EF_PBR.

                         * The primitive tag identifies clearly the type of data, its value
                         * field indicates the file identifier and, if applicable, the SFI
                         * value of the specified EF. That is, the length value of a primitive
                         * tag indicates if an SFI value is available for the EF or not:
                         * - Length = '02' Value: 'EFID (2 bytes)'
                         * - Length = '03' Value: 'EFID (2 bytes)', 'SFI (1 byte)'
                         */

                        int sfi = INVALID_SFI;
                        data = tlv.getData();

                        if (data.length < 2 || data.length > 3) {
                            Rlog.w(LOG_TAG, "Invalid TLV length: " + data.length);
                            break;
                        }

                        if (data.length == 3) {
                            sfi = data[2] & 0xFF;
                        }

                        int efid = ((data[0] & 0xFF) << 8) | (data[1] & 0xFF);
                        //M:
                        if (tag == USIM_EFANR_TAG) { //for handle multiple ANR, we should modify tag
                            tag += 0x100 * mAnrIndex;
                            mAnrIndex++;
                        }

                        File object = new File(parentTag, efid, sfi, tagNumberWithinParentTag);
                        object.mTag = tag; //TODO: add to constructor
                        object.mPbrRecord = mSliceCount;
                        logi("pbr " + object);

                        mFileIds.put(tag, object);
                        break;
                    default:
                        break;
                }
                tagNumberWithinParentTag++;
            } while (tlv.nextObject());
        }
    }

    private void queryUpbCapablityAndWait() {
        logi("queryUpbCapablityAndWait begin");

        synchronized (mUPBCapabilityLock) {
            for (int i = 0; i < 8; i++) {
                mUpbCap[i] = -1;
            }

            if (checkIsPhbReady()) {
                mCi.queryUPBCapability(obtainMessage(EVENT_UPB_CAPABILITY_QUERY_DONE));

                try {
                    mUPBCapabilityLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in queryUpbCapablityAndWait");
                }
            }
        }

        logi("queryUpbCapablityAndWait done:" +
                "N_Anr :" + mUpbCap[0] + ",N_Email:" + mUpbCap[1] + ",N_Sne:" + mUpbCap[2] +
                ",N_Aas:" + mUpbCap[3] + ",L_Aas:" + mUpbCap[4] + ",N_Gas:" + mUpbCap[5] +
                ",L_Gas:" + mUpbCap[6] + ",N_Grp:" + mUpbCap[7]);
    }

    private void readGasListAndWait() {
        logi("readGasListAndWait begin");

        synchronized (mGasLock) {
            if (mUpbCap[5] <= 0) {
                log("readGasListAndWait no need to read. return");
                return;
            }

            mCi.readUPBGasList(1, mUpbCap[5], obtainMessage(EVENT_GAS_LOAD_DONE));

            try {
                mGasLock.wait();
            } catch (InterruptedException e) {
                Rlog.e(LOG_TAG, "Interrupted Exception in readGasListAndWait");
            }
        }
        logi("readGasListAndWait end");
    }

    //data[0], data[1], data[2], payload
    private void updatePhoneAdnRecordWithAnrByIndex(int recId, int adnIndex,
                                                    int anrIndex, byte[] anrRecData) {
        log("updatePhoneAdnRecordWithAnrByIndex the " + adnIndex + "th anr record is "
                + IccUtils.bytesToHexString(anrRecData));
        /* mantdatory if and only if the file is not type 1 */
        // int adnRecNum = anrRec[anrRec.length - 1];
        // f***
        int anrRecLength = anrRecData[1];
        int anrAas = anrRecData[0];

        if (anrRecLength > 0 && anrRecLength <= 11) {
            String anr = MtkPhoneNumberUtils.calledPartyBCDToString(anrRecData, 2, anrRecData[1]);
            /*
             * String anr = IccUtils.bcdToString(anrRecData, 3, anrRecData[1] -
             * 1); if (anrRecData[2] == 0x91){ anr = "+" + anr; }
             */
            if (anr != null && !anr.equals("")) {
                String aas = null;

                if (anrAas > 0 && anrAas != 0xFF) {
                    if (mAasForAnr != null) {
                        ArrayList<String> aasList = mAasForAnr;

                        if (aasList != null && anrAas <= aasList.size()) {
                            aas = aasList.get(anrAas - 1);
                        }
                    }
                }
                logi(" updatePhoneAdnRecordWithAnrByIndex "
                        + adnIndex + " th anr is " + anr + " the anrIndex is " + anrIndex);
                // SIM record numbers are 1 based.
                MtkAdnRecord rec;
                try {
                    rec = mPhoneBookRecords.get(adnIndex);
                } catch (IndexOutOfBoundsException e) {
                    Rlog.e(LOG_TAG, "updatePhoneAdnRecordWithAnrByIndex: mPhoneBookRecords " +
                            "IndexOutOfBoundsException mPhoneBookRecords.size() is " +
                            mPhoneBookRecords.size() + "index is " + adnIndex);
                    return;
                }

                rec.setAnr(anr, anrIndex);
                if (aas != null && aas.length() > 0) {
                    rec.setAasIndex(anrAas);
                }
                mPhoneBookRecords.set(adnIndex, rec);
            }
        }
    }

    public ArrayList<UsimGroup> getUsimGroups() {
        logi("getUsimGroups begin");

        synchronized (mGasLock) {
            if (!mGasForGrp.isEmpty()) {
                return mGasForGrp;
            }
        }

        queryUpbCapablityAndWait();
        readGasListAndWait();
        logi("getUsimGroups end");
        return mGasForGrp;
    }

    public String getUsimGroupById(int nGasId) {
        String grpName = null;

        logi("getUsimGroupById nGasId is " + nGasId);

        if (mGasForGrp != null && nGasId <= mGasForGrp.size()) {
            UsimGroup uGas = mGasForGrp.get(nGasId - 1);
            if (uGas != null) {
                grpName = uGas.getAlphaTag();
                log("getUsimGroupById index is " + uGas.getRecordIndex() +
                        ", name is " + grpName);
            }
        }

        logi("getUsimGroupById grpName is " + grpName);
        return grpName;
    }

    public synchronized boolean removeUsimGroupById(int nGasId) {
        boolean ret = false;

        logi("removeUsimGroupById nGasId is " + nGasId);

        synchronized (mGasLock) {
            if (mGasForGrp == null || nGasId > mGasForGrp.size()) {
                Rlog.e(LOG_TAG, "removeUsimGroupById fail ");
            } else {
                UsimGroup uGas = mGasForGrp.get(nGasId - 1);

                if (uGas != null) {
                    log(" removeUsimGroupById index is " + uGas.getRecordIndex());
                }

                if (uGas != null && uGas.getAlphaTag() != null) {
                    mCi.deleteUPBEntry(UPB_EF_GAS, 0, nGasId,
                            obtainMessage(EVENT_GAS_UPDATE_DONE));

                    try {
                        mGasLock.wait();
                    } catch (InterruptedException e) {
                        Rlog.e(LOG_TAG, "Interrupted Exception in removeUsimGroupById");
                    }

                    if (mResult == 0) {
                        ret = true;
                        uGas.setAlphaTag(null);
                        mGasForGrp.set(nGasId - 1, uGas);
                    }
                } else {
                    Rlog.w(LOG_TAG, "removeUsimGroupById fail: this gas doesn't exist ");
                }
            }
        }
        logi("removeUsimGroupById result is " + ret);
        return ret;
    }

    private String decodeGas(String srcGas) {
        log("[decodeGas] gas string is " + ((srcGas == null) ? "null" : srcGas));

        if (srcGas == null || TextUtils.isEmpty(srcGas) || srcGas.length() % 2 != 0) {
            return null;
        }

        String retGas = null;

        try {
            byte[] ba = IccUtils.hexStringToBytes(srcGas);
            if (ba == null) {
                Rlog.w(LOG_TAG, "gas string is null");
                return retGas;
            }
            retGas = new String(ba, 0, srcGas.length() / 2, "utf-16be");
        } catch (UnsupportedEncodingException ex) {
            Rlog.e(LOG_TAG, "[decodeGas] implausible UnsupportedEncodingException", ex);
        } catch (RuntimeException ex) {
            Rlog.e(LOG_TAG, "[decodeGas] RuntimeException", ex);
        }
        return retGas;
    }

    private String encodeToUcs2(String input) {
        byte[] textPart;
        StringBuilder output = new StringBuilder();

        for (int i = 0; i < input.length(); i++) {
            String hexInt = Integer.toHexString(input.charAt(i));
            for (int j = 0; j < (4 - hexInt.length()); j++) {
                output.append("0");
            }
            output.append(hexInt);
        }

        return output.toString();
    }

    public synchronized int insertUsimGroup(String grpName) {
        int index = -1;

        logi("insertUsimGroup grpName is " + grpName);

        synchronized (mGasLock) {
            if (mGasForGrp == null || mGasForGrp.size() == 0) {
                Rlog.w(LOG_TAG, "insertUsimGroup fail ");
            } else {
                UsimGroup gasEntry = null;
                int i = 0;

                for (i = 0; i < mGasForGrp.size(); i++) {
                    gasEntry = mGasForGrp.get(i);
                    if (gasEntry != null && gasEntry.getAlphaTag() == null) {
                        index = gasEntry.getRecordIndex();
                        log("insertUsimGroup index is " + index);
                        break;
                    }
                }

                if (index < 0) {
                    Rlog.w(LOG_TAG, "insertUsimGroup fail: gas file is full.");
                    index = USIM_ERROR_GROUP_COUNT; // too many groups
                    return index;
                }

                String temp = encodeToUcs2(grpName);

                mCi.editUPBEntry(UPB_EF_GAS, 0, index, temp, null,
                        obtainMessage(EVENT_GAS_UPDATE_DONE));

                try {
                    mGasLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in insertUsimGroup");
                }

                if (mResult < 0) {
                    Rlog.e(LOG_TAG, "result is negative. insertUsimGroup");
                    return mResult;
                } else {
                    gasEntry.setAlphaTag(grpName);
                    mGasForGrp.set(i, gasEntry);
                }
            }
        }
        return index;
    }

    public synchronized int updateUsimGroup(int nGasId, String grpName) {
        int ret = -1;
        logi("updateUsimGroup nGasId is " + nGasId);

        synchronized (mGasLock) {
            mResult = -1;

            if (mGasForGrp == null || nGasId > mGasForGrp.size()) {
                Rlog.w(LOG_TAG, "updateUsimGroup fail ");
            } else if (grpName != null) {
                String temp = encodeToUcs2(grpName);
                mCi.editUPBEntry(UPB_EF_GAS, 0, nGasId, temp, null,
                        obtainMessage(EVENT_GAS_UPDATE_DONE));

                try {
                    mGasLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in updateUsimGroup");
                }
            }

            if (mResult == 0) {
                ret = nGasId;
                UsimGroup uGasEntry = mGasForGrp.get(nGasId - 1);
                if (uGasEntry != null) {
                    log("updateUsimGroup index is " + uGasEntry.getRecordIndex());
                    uGasEntry.setAlphaTag(grpName);
                } else {
                    Rlog.w(LOG_TAG, "updateUsimGroup the entry doesn't exist ");
                }
            } else {
                ret = mResult;
            }
        }

        return ret;
    }

    public boolean addContactToGroup(int adnIndex, int grpIndex) {
        boolean ret = false;

        logi("addContactToGroup begin adnIndex is " + adnIndex + " to grp " + grpIndex);

        if (mPhoneBookRecords == null || adnIndex <= 0 || adnIndex > mPhoneBookRecords.size()) {
            Rlog.e(LOG_TAG, "addContactToGroup no records or invalid index.");
            return false;
        }

        synchronized (mLock) {
            MtkAdnRecord rec;

            try {
                rec = mPhoneBookRecords.get(adnIndex - 1);
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG, "addContactToGroup: mPhoneBookRecords " +
                        "IndexOutOfBoundsException mPhoneBookRecords.size() is " +
                        mPhoneBookRecords.size() + "index is " + (adnIndex - 1));
                return false;
            }

            if (rec != null) {
                log(" addContactToGroup the adn index is " + rec.getRecId()
                        + " old grpList is " + rec.getGrpIds());
                String grpList = rec.getGrpIds();
                boolean bExist = false;
                int nOrder = -1;
                // mUpbCap[7] (N_Grp), maximum number of groups in an entry of EF_GRP
                // mUpbCap[5] (N_Gas), maximum number of entries in the EF_GAS
                int grpCount = mUpbCap[7];
                int grpMaxCount = ((mUpbCap[7] > mUpbCap[5]) ? mUpbCap[5] : mUpbCap[7]);
                int[] grpIdArray = new int[grpCount];

                for (int i = 0; i < grpCount; i++) {
                    grpIdArray[i] = 0;
                }

                if (grpList != null) {
                    String[] grpIds = rec.getGrpIds().split(",");
                    for (int i = 0; i < grpMaxCount; i++) {
                        grpIdArray[i] = Integer.parseInt(grpIds[i]);
                        if (grpIndex == grpIdArray[i]) {
                            bExist = true;
                            log(" addContactToGroup the adn is already in the group. i is " + i);
                            break;
                        }

                        if ((nOrder < 0) && (grpIdArray[i] == 0 || grpIdArray[i] == 255)) {
                            nOrder = i;
                            log(" addContactToGroup found an unsed position in the group list." +
                                " i is " + i);
                        }
                    }
                } else {
                    nOrder = 0;
                }

                if (!bExist && nOrder >= 0) {
                    grpIdArray[nOrder] = grpIndex;
                    mCi.writeUPBGrpEntry(adnIndex, grpIdArray,
                            obtainMessage(EVENT_GRP_UPDATE_DONE));

                    try {
                        mLock.wait();
                    } catch (InterruptedException e) {
                        Rlog.e(LOG_TAG, "Interrupted Exception in addContactToGroup");
                    }

                    if (mResult == 0) {
                        ret = true;
                        updatePhoneAdnRecordWithGrpByIndex(adnIndex - 1, adnIndex, grpIdArray);
                        logi(" addContactToGroup the adn index is "
                                + rec.getRecId());
                        mResult = -1;
                    }
                }
            }
        }
        logi("addContactToGroup end adnIndex is " +
                adnIndex + " to grp " + grpIndex);
        return ret;
    }

    public synchronized boolean removeContactFromGroup(int adnIndex, int grpIndex) {
        boolean ret = false;

        logi("removeContactFromGroup begin adnIndex is " + adnIndex + " to grp " + grpIndex);

        if (mPhoneBookRecords == null || adnIndex <= 0 || adnIndex > mPhoneBookRecords.size()) {
            Rlog.e(LOG_TAG, "removeContactFromGroup no records or invalid index.");
            return false;
        }

        synchronized (mLock) {
            MtkAdnRecord rec;

            try {
                rec = mPhoneBookRecords.get(adnIndex - 1);
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG, "removeContactFromGroup: mPhoneBookRecords " +
                        "IndexOutOfBoundsException mPhoneBookRecords.size() is " +
                        mPhoneBookRecords.size() + "index is " + (adnIndex - 1));
                return false;
            }

            if (rec != null) {
                String grpList = rec.getGrpIds();
                if (grpList == null) {
                    Rlog.e(LOG_TAG, " the adn is not in any group. ");
                    return false;
                }

                String[] grpIds = grpList.split(",");
                boolean bExist = false;
                int nOrder = -1;
                int[] grpIdArray = new int[grpIds.length];

                for (int i = 0; i < grpIds.length; i++) {
                    grpIdArray[i] = Integer.parseInt(grpIds[i]);
                    if (grpIndex == grpIdArray[i]) {
                        bExist = true;
                        nOrder = i;
                        log(" removeContactFromGroup the adn is in the group. i is "  + i);
                    }
                }

                if (bExist && nOrder >= 0) {
                    grpIdArray[nOrder] = 0;
                    mCi.writeUPBGrpEntry(adnIndex, grpIdArray,
                            obtainMessage(EVENT_GRP_UPDATE_DONE));
                    try {
                        mLock.wait();
                    } catch (InterruptedException e) {
                        Rlog.e(LOG_TAG, "Interrupted Exception in removeContactFromGroup");
                    }

                    if (mResult == 0) {
                        ret = true;
                        updatePhoneAdnRecordWithGrpByIndex(adnIndex - 1, adnIndex, grpIdArray);
                        mResult = -1;
                    }

                } else {
                    Rlog.e(LOG_TAG, " removeContactFromGroup the adn is not in the group. ");
                }
            }
        }
        logi("removeContactFromGroup end adnIndex is " + adnIndex + " to grp " + grpIndex);
        return ret;
    }

    public boolean updateContactToGroups(int adnIndex, int[] grpIdList) {
        boolean ret = false;

        if (mPhoneBookRecords == null || adnIndex <= 0 ||
                adnIndex > mPhoneBookRecords.size() || grpIdList == null) {
            Rlog.e(LOG_TAG, "updateContactToGroups no records or invalid index.");
            return false;
        }

        logi("updateContactToGroups begin grpIdList is " +
                adnIndex + " to grp list count " + grpIdList.length);

        synchronized (mLock) {
            MtkAdnRecord rec = mPhoneBookRecords.get(adnIndex - 1);

            if (rec != null) {
                log(" updateContactToGroups the adn index is " + rec.getRecId() +
                        " old grpList is " + rec.getGrpIds());

                int grpCount = mUpbCap[7];

                if (grpIdList.length > grpCount) {
                    Rlog.e(LOG_TAG, "updateContactToGroups length of grpIdList > grpCount.");
                    return false;
                }

                int[] grpIdArray = new int[grpCount];

                for (int i = 0; i < grpCount; i++) {
                    grpIdArray[i] = ((i < grpIdList.length) ? grpIdList[i] : 0);
                    log("updateContactToGroups i:" + i + ",grpIdArray[" + i + "]:" + grpIdArray[i]);
                }

                mCi.writeUPBGrpEntry(adnIndex, grpIdArray,
                        obtainMessage(EVENT_GRP_UPDATE_DONE));
                try {
                    mLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in updateContactToGroups");
                }

                if (mResult == 0) {
                    ret = true;
                    updatePhoneAdnRecordWithGrpByIndex(adnIndex - 1, adnIndex, grpIdArray);
                    logi(" updateContactToGroups the adn index is "
                            + rec.getRecId());
                    mResult = -1;
                }
            }
        }

        logi("updateContactToGroups end grpIdList is " +
                adnIndex + " to grp list count " + grpIdList.length);
        return ret;

    }

    public boolean moveContactFromGroupsToGroups(int adnIndex,
                                                 int[] fromGrpIdList, int[] toGrpIdList) {
        boolean ret = false;

        if (mPhoneBookRecords == null || adnIndex <= 0 || adnIndex > mPhoneBookRecords.size()) {
            Rlog.e(LOG_TAG, "moveContactFromGroupsToGroups no records or invalid index.");
            return false;
        }

        synchronized (mLock) {
            MtkAdnRecord rec = mPhoneBookRecords.get(adnIndex - 1);

            if (rec != null) {
                // mUpbCap[7] (N_Grp), maximum number of groups in an entry of EF_GRP
                // mUpbCap[5] (N_Gas), maximum number of entries in the EF_GAS
                int grpCount = mUpbCap[7];
                int grpMaxCount = ((mUpbCap[7] > mUpbCap[5]) ? mUpbCap[5] : mUpbCap[7]);
                String grpIds = rec.getGrpIds();

                logi(" moveContactFromGroupsToGroups the adn index is " + rec.getRecId() +
                               " original grpIds is " + grpIds +
                               ", fromGrpIdList: " +
                                ((fromGrpIdList == null) ? "null" : fromGrpIdList) +
                               ", toGrpIdList: " + ((toGrpIdList == null) ? "null" : toGrpIdList));

                int[] grpIdIntArray = new int[grpCount];

                for (int i = 0; i < grpCount; i++) {
                    grpIdIntArray[i] = 0;
                }

                // Prepare original group IDs.
                if (grpIds != null) {
                    String[] grpIdStrArray = grpIds.split(",");
                    for (int i = 0; i < grpMaxCount; i++) {
                        grpIdIntArray[i] = Integer.parseInt(grpIdStrArray[i]);
                    }
                }

                // Remove from groups.
                if (fromGrpIdList != null) {
                    for (int i = 0; i < fromGrpIdList.length; i++) {
                        for (int j = 0; j < grpMaxCount; j++) {
                            if (grpIdIntArray[j] == fromGrpIdList[i]) {
                                grpIdIntArray[j] = 0;
                            }
                        }
                    }
                }

                // Add to groups.
                if (toGrpIdList != null) {
                    for (int i = 0; i < toGrpIdList.length; i++) {
                        boolean bEmpty = false;
                        boolean bExist = false;

                        // Check if contact is in to-group already.
                        for (int k = 0; k < grpMaxCount; k++) {
                          if (grpIdIntArray[k] == toGrpIdList[i]) {
                              bExist = true;
                              break;
                          }
                        }

                        if (bExist == true) {
                            Rlog.w(LOG_TAG, "moveContactFromGroupsToGroups the adn is"
                                    + "already in the group.");
                            continue;
                        }

                        // Add to gropup.
                        for (int j = 0; j < grpMaxCount; j++) {
                            if ((grpIdIntArray[j] == 0) || (grpIdIntArray[j] == 255)) {
                                bEmpty = true;
                                grpIdIntArray[j] = toGrpIdList[i];
                                break;
                            }
                        }

                        if (bEmpty == false) {
                            Rlog.e(LOG_TAG, "moveContactFromGroupsToGroups no empty to add.");
                            return false;
                        }
                    }
                }

                mCi.writeUPBGrpEntry(adnIndex, grpIdIntArray,
                        obtainMessage(EVENT_GRP_UPDATE_DONE));
                try {
                    mLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in moveContactFromGroupsToGroups");
                }

                if (mResult == 0) {
                    ret = true;
                    updatePhoneAdnRecordWithGrpByIndex(adnIndex - 1, adnIndex, grpIdIntArray);
                    logi("moveContactFromGroupsToGroups the adn index is "
                            + rec.getRecId());
                    mResult = -1;
                }
            }
        }
        return ret;
    }

    /**
     *
     * @param adnIndex adn index 1 based.
     * @return true for remove success
     */
    public boolean removeContactGroup(int adnIndex) {
        boolean ret = false;

        logi("removeContactsGroup adnIndex is " + adnIndex);

        if (mPhoneBookRecords == null || mPhoneBookRecords.isEmpty()) {
            return ret;
        }

        synchronized (mLock) {
            MtkAdnRecord rec;

            try {
                rec = mPhoneBookRecords.get(adnIndex - 1);
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG, "removeContactGroup: mPhoneBookRecords " +
                        "IndexOutOfBoundsException mPhoneBookRecords.size() is " +
                        mPhoneBookRecords.size() + "index is " + (adnIndex - 1));
                return false;
            }

            if (rec == null) {
                return ret;
            }

            log("removeContactsGroup rec is " + rec);

            String grpList = rec.getGrpIds();

            if (grpList == null) {
                return ret;
            }

            String[] grpIds = grpList.split(",");
            boolean hasGroup = false;

            for (int i = 0; i < grpIds.length; i++) {
                int value = Integer.parseInt(grpIds[i]);
                if (value > 0 && value < 255) {
                    hasGroup = true;
                    break;
                }
            }

            if (hasGroup) {
                mCi.writeUPBGrpEntry(adnIndex, new int[0],
                        obtainMessage(EVENT_GRP_UPDATE_DONE));

                try {
                    mLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in removeContactGroup");
                }

                if (mResult == 0) {
                    ret = true;
                    int[] grpIdArray = new int[grpIds.length];

                    for (int i = 0; i < grpIds.length; i++) {
                        grpIdArray[i] = 0;
                    }
                    updatePhoneAdnRecordWithGrpByIndex(adnIndex - 1, adnIndex, grpIdArray);
                    logi(" removeContactGroup the adn index is "
                            + rec.getRecId());
                    mResult = -1;
                }
            }
            return ret;
        }
    }

    public int hasExistGroup(String grpName) {
        int grpId = -1;

        logi("hasExistGroup grpName is " + grpName);

        if (grpName == null) {
            return grpId;
        }

        if (mGasForGrp != null && mGasForGrp.size() > 0) {
            for (int i = 0; i < mGasForGrp.size(); i++) {
                UsimGroup uGas = mGasForGrp.get(i);

                if (uGas != null && grpName.equals(uGas.getAlphaTag())) {
                    log("getUsimGroupById index is " + uGas.getRecordIndex() +
                            ", name is " + grpName);
                    grpId = uGas.getRecordIndex();
                    break;
                }
            }

        }
        logi("hasExistGroup grpId is " + grpId);
        return grpId;
    }

    public int getUsimGrpMaxNameLen() {
        int ret = -1;

        logi("getUsimGrpMaxNameLen begin");

        synchronized (mUPBCapabilityLock) {
            if (checkIsPhbReady()) {
                if (mUpbCap[6] < 0) {
                    queryUpbCapablityAndWait();
                }
                ret = mUpbCap[6];
            } else {
                ret = -1;
            }
            logi("getUsimGrpMaxNameLen done: " + "L_Gas is " + ret);
        }
        return ret;
    }

    public int getUsimGrpMaxCount() {
        // Maybe should compare N_Gas and N_Grp if different.
        int ret = -1;

        logi("getUsimGrpMaxCount begin");

        synchronized (mUPBCapabilityLock) {
            if (checkIsPhbReady()) {
                if (mUpbCap[5] < 0) {
                    queryUpbCapablityAndWait();
                }
                ret = mUpbCap[5];
            } else {
                ret = -1;
            }
            logi("getUsimGrpMaxCount done: " + "N_Gas is " + ret);
        }
        return ret;
    }

    private void log(String msg) {
        if (DBG) Rlog.d(LOG_TAG, msg + "(slot " + mSlotId + ")");
    }

    private void logi(String msg) {
        Rlog.i(LOG_TAG, msg + "(slot " + mSlotId + ")");
    }

    /**
     * Check if EF_ANR's space available to insert,
     * only type2 & insert needs to check capacity.
     * CSIM and USIM use different way to check.
     *
     * @param anr ANR string to be insertd/updated, null or "" means remove
     * @param adnIndex the index of ADN to be updated
     * @param anrIndex the i-th ANR to be checked (0-based)
     * @param oldAdn to check insert or update
     * @return true for space available
     */
    public boolean isAnrCapacityFree(String anr, int adnIndex, int anrIndex, MtkAdnRecord oldAdn) {
        String oldAnr = null;

        if (oldAdn != null) {
            oldAnr = oldAdn.getAdditionalNumber(anrIndex);
        }

        if (anr == null || anr.equals("") || anrIndex < 0 ||
            (getUsimEfType(USIM_EFANR_TAG) == USIM_TYPE1_TAG) ||
            (oldAnr != null && !oldAnr.equals(""))) {
            // return anr capacity free in this case when:
            // 1. no anr in new adn record or
            // 2. type1 (type1 always have same count as ADN) or
            // 3. type2 and replace old with new one
            return true;
        }

        if (!CsimPhbUtil.hasModemPhbEnhanceCapability(mFh)) {
            ArrayList<Integer> fileIds;
            int pbrRecNum = (adnIndex - 1) / mAdnFileSize;
            int anrRecNum = (adnIndex - 1) % mAdnFileSize;

            //only check anr2 and anr3
            try {
                log("isAnrCapacityFree anr: " + anr);

                if (mRecordSize == null || mRecordSize.size() == 0) {
                    log("isAnrCapacityFree: mAnrFileSize is empty");
                    return false;
                }

                File anrFile = mPbrRecords.get(pbrRecNum).mFileIds.get(
                        USIM_EFANR_TAG + anrIndex * 0x100);

                if (anrFile == null) {
                    return false;
                }

                int anrFileId = anrFile.getEfid();
                int[] sizeInfo = mRecordSize.get(anrFileId);
                int size = sizeInfo[2];
                log("isAnrCapacityFree size: " + size);

                if (size < anrRecNum + 1) {
                    log("isAnrCapacityFree: anrRecNum out of size: " + anrRecNum);
                    return false;
                }
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG, "isAnrCapacityFree Index out of bounds.");
                return false;
            } catch (java.lang.NullPointerException e) {
                Rlog.e(LOG_TAG, "isAnrCapacityFree exception:" + e.toString());
                return false;
            }

            return true;

        } else {
            synchronized (mLock) {
                if (mAnrInfo == null || anrIndex >= mAnrInfo.size()) {
                    mCi.queryUPBAvailable(UPB_EF_ANR, anrIndex + 1,
                            obtainMessage(EVENT_QUERY_ANR_AVAILABLE_OPTMZ_DONE));

                    try {
                        mLock.wait();
                    } catch (InterruptedException e) {
                        Rlog.e(LOG_TAG, "Interrupted Exception in isAnrCapacityFree");
                    }
                }
            }

            if (mAnrInfo != null && mAnrInfo.get(anrIndex) != null
                    && mAnrInfo.get(anrIndex)[1] > 0) {
                return true;
            } else {
                return false;
            }
        }
    }

    /**
     * Update a contact's additional number.
     * CSIM and USIM use different way to update.
     *
     * @param anr additional number string to be updated
     * @param adnIndex ADN index
     * @param anrIndex the i-th ANR (0-based)
     */
    public void updateAnrByAdnIndex(String anr, int adnIndex, int anrIndex, MtkAdnRecord oldAdn) {
        // TODO: return update success or not.
        SparseArray<File> fileIds;
        int pbrRecNum = (adnIndex - 1) / mAdnFileSize;
        int anrRecNum = (adnIndex - 1) % mAdnFileSize;

        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            return;
        }

        fileIds = mPbrRecords.get(pbrRecNum).mFileIds;

        if (fileIds == null) {
            log("updateAnrByAdnIndex: No anr tag in pbr record 0");
            return;
        }

        if (mPhoneBookRecords == null || mPhoneBookRecords.isEmpty()) {
            Rlog.w(LOG_TAG, "updateAnrByAdnIndex: mPhoneBookRecords is empty");
            return;
        }

        File anrFile = fileIds.get(USIM_EFANR_TAG + anrIndex * 0x100);

        if (anrFile == null) {
            log("updateAnrByAdnIndex no efFile anrIndex: " + anrIndex);
            return;
        }

        logi("updateAnrByAdnIndex begin effile " + anrFile);
        // To reduce unnecessary request, we check whether anr and aas index value changed.
        String oldAnr = null;
        int oldAas = 0;

        if (oldAdn != null) {
            oldAnr = oldAdn.getAdditionalNumber(anrIndex);
            oldAas = oldAdn.getAasIndex();
        }

        if (!CsimPhbUtil.hasModemPhbEnhanceCapability(mFh)) {
            int efid = anrFile.getEfid();
            log("updateAnrByAdnIndex recId: " + pbrRecNum + " EF_ANR id is " +
                Integer.toHexString(efid).toUpperCase());

            if (anrFile.getParentTag() == USIM_TYPE2_TAG) {
                updateType2Anr(anr, adnIndex, anrFile);
                return;
            }
            //update Type1 ANR
            MtkAdnRecord rec;
            try {
                rec = mPhoneBookRecords.get(adnIndex - 1);
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG, "updateAnrByAdnIndex: mPhoneBookRecords " +
                        "IndexOutOfBoundsException mPhoneBookRecords.size() is " +
                        mPhoneBookRecords.size() + "index is " + (adnIndex - 1));
                return;
            }

            int aas = rec.getAasIndex();
            byte[] data = buildAnrRecord(anr, mAnrRecordSize, aas);

            if (data != null) {
                mFh.updateEFLinearFixed(efid, anrRecNum + 1, data, null,
                        obtainMessage(EVENT_ANR_UPDATE_DONE));
            }
        } else {
            MtkAdnRecord rec;

            try {
                rec = mPhoneBookRecords.get(adnIndex - 1);
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG, "updateAnrByAdnIndexOptmz: mPhoneBookRecords " +
                        "IndexOutOfBoundsException size() is " + mPhoneBookRecords.size() +
                        "index is " + (adnIndex - 1));
                return;
            }

            int aas = rec.getAasIndex();
            Message msg = obtainMessage(EVENT_ANR_UPDATE_DONE);

            synchronized (mLock) {
                if (anr == null || anr.length() == 0) {  // delete
                    if (oldAnr == null || oldAnr.length() == 0) {
                        return;
                    }
                    mCi.deleteUPBEntry(UPB_EF_ANR, anrIndex + 1, adnIndex, msg);
                } else {
                    /*
                    if (anr.equals(oldAnr) && oldAas == aas) {
                        return;
                    }*/
                    String param[] = buildAnrRecordOptmz(anr, aas);
                    mCi.editUPBEntry(UPB_EF_ANR, anrIndex + 1, adnIndex,
                            param[0], param[1], param[2], msg);
                }
                try {  //Make sure whether need to wait here.
                    mLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in updateAnrByAdnIndexOptmz");
                }
            }
        }
        logi("updateAnrByAdnIndex end effile " + anrFile);
    }

    //For Email Type2 update
    private int getEmailRecNum(String[] emails, int pbrRecNum, int nIapRecNum,
                               byte[] iapRec, int tagNum) {
        boolean hasEmail = false;
        int recNum = iapRec[tagNum] & 0xFF;

        log("getEmailRecNum recNum:" + recNum);
        if (null == emails) {
            if (recNum < 255 && recNum > 0) {
                mEmailRecTable[recNum - 1] = 0;
            }
            return -1;
        }

        for (int i = 0; i < emails.length; i++) {
            if (null != emails[i] && !emails[i].equals("")) {
                hasEmail = true;
                break;
            }
        }

        if (!hasEmail) {
            if (recNum < 255 && recNum > 0) {
                mEmailRecTable[recNum - 1] = 0;
            }
            return -1;
        }
        // no email record before find
        if (recNum > mEmailFileSize || recNum >= 255 || recNum <= 0) {
            // find a index to save the email and update iap record.
            int nOffset = pbrRecNum * mEmailFileSize;
            for (int i = nOffset; i < nOffset + mEmailFileSize; i++) {
                log("updateEmailsByAdnIndex: mEmailRecTable[" + i + "] is "
                        + mEmailRecTable[i]);
                if (mEmailRecTable[i] == 0) {
                    recNum = i + 1 - nOffset;
                    mEmailRecTable[i] = nIapRecNum;
                    break;
                }
            }
        }

        if (recNum > mEmailFileSize) {
            recNum = 255;
        }

        if (recNum == -1) {
            return -2;
        }

        return recNum;
    }

    /**
     * Check if EF_Email space available to insert,
     * only type2 & insert needs to check capacity.
     * CSIM and USIM use different way to check.
     *
     * @param adnIndex the index of ADN to be updated
     * @param emails to be insertd/updated, null or "" means remove
     * @param oldAdn to check insert or update
     * @return true for space available
     */
    public boolean checkEmailCapacityFree(int adnIndex, String[] emails, MtkAdnRecord oldAdn) {
        boolean hasEmail = false;

        if (null == emails ||
            (getUsimEfType(USIM_EFEMAIL_TAG) == USIM_TYPE1_TAG) ||
            (oldAdn != null && oldAdn.getEmails() != null)) {
            // return anr capacity free in this case when:
            // 1. no email in new adn record or
            // 2. type1 (type1 always have same count as ADN) or
            // 3. type2 and replace old with new one
            return true;
        }

        for (int i = 0; i < emails.length; i++) {
            if (null != emails[i] && !emails[i].equals("")) {
                hasEmail = true;
                break;
            }
        }

        if (!hasEmail) { //Delete
            return true;
        }

        if (!CsimPhbUtil.hasModemPhbEnhanceCapability(mFh)) {
            int pbrRecNum = (adnIndex - 1) / mAdnFileSize;
            int nOffset = pbrRecNum * mEmailFileSize;

            for (int i = nOffset; i < nOffset + mEmailFileSize; i++) {
                if (mEmailRecTable[i] == 0) {
                    return true;
                }
            }
            return false;
        } else {
            synchronized (mLock) {
                if (mEmailInfo == null || mEmailInfo.length != 3) {
                    mCi.queryUPBAvailable(UPB_EF_EMAIL, 1,
                            obtainMessage(EVENT_QUERY_EMAIL_AVAILABLE_OPTMZ_DONE));

                    try {
                        mLock.wait();
                    } catch (InterruptedException e) {
                        Rlog.e(LOG_TAG, "Interrupted Exception in CheckEmailCapacityFree");
                    }

                    if (mUpbDone == -1) {
                        return true;
                    }
                    mEmailFileSize = countEmailFileSize();
                }
            }
            int used = countEmailCapacity(adnIndex);
            logi("CheckEmailCapacityFree: mEmailFileSize: " + mEmailFileSize +
                    " used: " + used + " adnIndex: " + adnIndex);

            if (used < mEmailFileSize) {
                return true;
            } else {
                return false;
            }
        }
    }

    private int countEmailFileSize() {
        int numAdnRecs = mPhoneBookRecords.size();
        int totalPbrRecNum = numAdnRecs/mAdnFileSize;

        if (numAdnRecs % mAdnFileSize > 0) {
            totalPbrRecNum ++;
        }

        if (mEmailInfo != null && mEmailInfo.length == 3 && totalPbrRecNum > 0) {
            return mEmailInfo[0]/totalPbrRecNum;
        } else {
            return USIM_DEFAULT_MAX_EMAIL_FILE_SIZE;
        }
    }

    private int countEmailCapacity(int adnIndex) {
        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            return -1;
        }

        int pbrRecNum = (adnIndex - 1) / mAdnFileSize;
        int nOffset = pbrRecNum * mAdnFileSize;
        int numAdnRecs = mPhoneBookRecords.size();
        int nMax = nOffset + mAdnFileSize;
        nMax = numAdnRecs < nMax ? numAdnRecs : nMax;
        SparseArray<File> files = mPbrRecords.get(pbrRecNum).mFileIds;
        File emailFile = files.get(USIM_EFEMAIL_TAG);
        int used = 0;

        if (emailFile != null) {
            MtkAdnRecord rec = null;
            String[] emails = null;

            for (int i = nOffset; i < nMax; i++) {
                try {
                    rec = mPhoneBookRecords.get(i);
                } catch (IndexOutOfBoundsException e) {
                    Rlog.e(LOG_TAG, "countEmailCapacity: mPhoneBookRecords " +
                            "IndexOutOfBoundsException mPhoneBookRecords.size() is " +
                            mPhoneBookRecords.size() + "index is " + i);
                }

                if (null == rec) {
                    // log("null email rec ");
                    continue;
                }

                emails = rec.getEmails();
                if (null != emails && emails.length > 0 && emails[0].length() > 0) {
                    // log("email: " + emails[0]);
                    used++;
                }
            }
            log("countEmailCapacity: email used: " + used);
        }else {
            return -1;
        }
        return used;
    }

    /**
     * Check if EF_SNE space available to insert,
     * only type2 & insert needs to check capacity.
     * Currently not support CSIM to do this check.
     *
     * @param adnIndex the index of ADN to be updated, currently not be used
     * @param sne SNE string to be insertd/updated, null or "" means remove
     * @param oldAdn to check insert or update
     * @return true for space available
     */
    public boolean checkSneCapacityFree(int adnIndex, String sne, MtkAdnRecord oldAdn) {
        String oldSne = null;

        if (oldAdn != null) {
            oldSne = oldAdn.getSne();
        }

        if (null == sne || sne.equals("") ||
            (getUsimEfType(USIM_EFSNE_TAG) == USIM_TYPE1_TAG) ||
            (null != oldSne && !oldSne.equals(""))) {
            // return anr capacity free in this case when:
            // 1. no sne in new adn record or
            // 2. type1 (type1 always have same count as ADN) or
            // 3. type2 and replace old with new one
            return true;
        }

        if (!CsimPhbUtil.hasModemPhbEnhanceCapability(mFh)) {
            // TODO: implement Sne capacity check when CSIM
            return true;
        } else {
            synchronized (mLock) {
                if (mSneInfo == null) {
                    mCi.queryUPBAvailable(UPB_EF_SNE, 1,
                            obtainMessage(EVENT_QUERY_SNE_AVAILABLE_OPTMZ_DONE));
                    try {
                        mLock.wait();
                    } catch (InterruptedException e) {
                        Rlog.e(LOG_TAG, "Interrupted Exception in checkSneCapacityFree");
                    }
                }
            }

            if (mSneInfo != null && mSneInfo[1] > 0) {
                return true;
            } else {
                return false;
            }
        }
    }

    private int getUsimEfType(int efTag) {
        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            return 0;
        }

        SparseArray<File> files;
        files = mPbrRecords.get(0).mFileIds;

        if (files == null) {
            return 0;
        }

        File efFile = files.get(efTag);

        if (efFile != null) {
            Rlog.d(LOG_TAG, "[getUsimEfType] efTag: " + efTag + ", type: " +
                efFile.getParentTag());
            return efFile.getParentTag();
        }
        return 0;
    }

    /**
     * Check if input emails not exceed max length.
     *
     * @param emails email list to be checked length
     * @return true for not exceed max length
     */
    public boolean checkEmailLength(String[] emails) {
        if (emails != null && emails[0] != null) {

            if (mPbrRecords == null || mPbrRecords.size() == 0) {
                return true;
            }

            SparseArray<File> files = mPbrRecords.get(0).mFileIds;
            if (files == null) {
                return true;
            }

            File emailFile = files.get(USIM_EFEMAIL_TAG);
            if (emailFile == null) {
                return true;
            }

            boolean emailType2 = emailFile.getParentTag() == USIM_TYPE2_TAG;
            int maxDataLength = (((mEmailRecordSize != -1) && emailType2) ?
                             (mEmailRecordSize - USIM_TYPE2_CONDITIONAL_LENGTH) : mEmailRecordSize);
            byte[] eMailData = GsmAlphabet.stringToGsm8BitPacked(emails[0]);

            logi("checkEmailLength eMailData.length=" + eMailData.length +
                    ", maxDataLength=" + maxDataLength);

            if ((maxDataLength != -1) && (eMailData.length > maxDataLength)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Update a contact's Emails.
     * CSIM and USIM use different way to update.
     *
     * @param emails email list to be updated
     * @param adnIndex ADN index
     * @return 0 for success, USIM_ERROR_* for error
     */
    public int updateEmailsByAdnIndex(String[] emails, int adnIndex, MtkAdnRecord oldAdn) {
        int pbrRecNum = (adnIndex - 1) / mAdnFileSize;
        int adnRecNum = (adnIndex - 1) % mAdnFileSize;
        SparseArray<File> files;

        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            return 0;
        }

        files = mPbrRecords.get(pbrRecNum).mFileIds;

        if (files == null || files.size() == 0) {
            return 0;
        }

        if (mPhoneBookRecords == null || mPhoneBookRecords.isEmpty()) {
            return 0;
        }

        File efFile = files.get(USIM_EFEMAIL_TAG);
        if (efFile == null) {
            log("updateEmailsByAdnIndex: No email tag in pbr record 0");
            return 0;
        }
        // To reduce unnecessary request, we check if email value changed.
        String[] oldEmails = null;
        String oldEmail = null;

        if (oldAdn != null) {
            oldEmails = oldAdn.getEmails();
        }

        if (oldEmails != null && oldEmails.length != 0 && !TextUtils.isEmpty(oldEmails[0])) {
            oldEmail = oldEmails[0];
        }

        int efid = efFile.getEfid();
        boolean emailType2 = efFile.getParentTag() == USIM_TYPE2_TAG;
        int emailType2Index = efFile.getIndex();

        logi("updateEmailsByAdnIndex: pbrrecNum is " + pbrRecNum
                + " EF_EMAIL id is " + Integer.toHexString(efid).toUpperCase());

        if (!CsimPhbUtil.hasModemPhbEnhanceCapability(mFh)) {
            if (emailType2 && mIapFileList != null) {
                return updateType2Email(emails, adnIndex, efFile);
            } else {
                log("updateEmailsByAdnIndex file: " + efFile);
                // handle type1 email
                String email = (emails == null || emails.length <= 0) ? null : emails[0];

                if (0 >= mEmailRecordSize) {
                    return USIM_ERROR_OTHERS;
                }

                byte[] data = buildEmailRecord(email, adnIndex, mEmailRecordSize, emailType2);

                if (data == null) {
                    return USIM_ERROR_STRING_TOOLONG;
                }

                mFh.updateEFLinearFixed(efid, adnRecNum + 1, data, null,
                            obtainMessage(EVENT_EMAIL_UPDATE_DONE));
                return 0;
            }
        } else {
            int emailIndex = 1;  // Currently only support 1 Email
            Message msg = obtainMessage(EVENT_EMAIL_UPDATE_DONE);

            synchronized (mLock) {
                if (emails == null || emails.length == 0 || TextUtils.isEmpty(emails[0])) {
                    if (oldEmail == null) {
                        return 0;
                    }
                    // delete
                    mCi.deleteUPBEntry(UPB_EF_EMAIL, emailIndex, adnIndex, msg);
                } else {
                    if (emails[0].equals(oldEmail)) {
                        return 0;
                    }
                    String temp = encodeToUcs2(emails[0]);  // make sure encode method
                    mCi.editUPBEntry(UPB_EF_EMAIL, emailIndex, adnIndex, temp, null, msg);
                }

                try {
                    mLock.wait();  // make sure whether need to wait
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in updateEmailsByAdnIndex");
                }
            }
            return 0;
        }
    }

    private int updateType2Email(String[] emails, int adnIndex, File emailFile) {
        // Get iap record by index
        // if the index is valid, update the email record
        int pbrRecNum = (adnIndex - 1) / mAdnFileSize;
        int adnRecNum = (adnIndex - 1) % mAdnFileSize;
        int emailType2Index = emailFile.getIndex();
        int efid = emailFile.getEfid();
        int recNum = -3;
        byte[] iapRec = null;

        try {
            ArrayList<byte[]> iapFile = mIapFileList.get(pbrRecNum);

            if (iapFile.size() > 0) {
                iapRec = iapFile.get(adnRecNum);
            } else {
                Rlog.w(LOG_TAG, "Warning: IAP size is 0");
                return USIM_ERROR_OTHERS;
            }
        } catch (IndexOutOfBoundsException e) {
            Rlog.e(LOG_TAG, "Index out of bounds.");
            return USIM_ERROR_OTHERS;
        }

        recNum = getEmailRecNum(emails, pbrRecNum, adnRecNum + 1, iapRec, emailType2Index);

        log("updateEmailsByAdnIndex: Email recNum is " + recNum);

        if (-2 == recNum) {
            return USIM_ERROR_CAPACITY_FULL;
        }

        log("updateEmailsByAdnIndex: found Email recNum is " + recNum);

        iapRec[emailType2Index] = (byte) recNum;
        SparseArray<File> files = mPbrRecords.get(pbrRecNum).mFileIds;

        if (files.get(USIM_EFIAP_TAG) != null) {
            efid = files.get(USIM_EFIAP_TAG).getEfid();
        } else {
            //Some USIM card does not confrim to SPEC, there is no IAP file in the USIM card.
            Rlog.e(LOG_TAG, "updateEmailsByAdnIndex Error: No IAP file!");
            return USIM_ERROR_OTHERS;
        }
        //update IAP
        mFh.updateEFLinearFixed(efid, adnRecNum + 1, iapRec, null,
                obtainMessage(EVENT_IAP_UPDATE_DONE));

        if ((recNum != 255) && (recNum != -1)) {
            String eMailAd = null;

            if (emails != null) {
                try {
                    eMailAd = emails[0];
                } catch (IndexOutOfBoundsException e) {
                    Rlog.e(LOG_TAG,
                           "Error: updateEmailsByAdnIndex no email address, continuing");
                }

                if (0 >= mEmailRecordSize) {
                    return USIM_ERROR_OTHERS;
                }

                byte[] eMailRecData = buildEmailRecord(eMailAd, adnIndex, mEmailRecordSize, true);

                if (eMailRecData == null) {
                    return USIM_ERROR_STRING_TOOLONG;
                }

                // to be replaced by the record size
                efid = files.get(USIM_EFEMAIL_TAG).getEfid();
                mFh.updateEFLinearFixed(efid, recNum, eMailRecData,
                        null, obtainMessage(EVENT_EMAIL_UPDATE_DONE));
            }
        }
        return 0;
    }

    private byte[] buildAnrRecord(String anr, int recordSize, int aas) {
        log("buildAnrRecord anr:" + anr + ",recordSize:" + recordSize + ",aas:" + aas);

        if (recordSize <= 0) {
            readAnrRecordSize();
        }

        byte[] bcdNumber;
        byte[] byteTag;
        byte[] anrString;
        anrString = new byte[recordSize];

        for (int i = 0; i < recordSize; i++) {
            anrString[i] = (byte) 0xFF;
        }

        String updatedAnr = MtkPhoneNumberUtils.convertPreDial(anr);

        if (TextUtils.isEmpty(updatedAnr)) {
            Rlog.w(LOG_TAG, "[buildAnrRecord] Empty dialing number");
            return anrString; // return the empty record (for delete)
        } else if (updatedAnr.length() > (13 - 4 + 1) * 2) {
            Rlog.w(LOG_TAG,
                    "[buildAnrRecord] Max length of dialing number is 20");
            return null;
        } else {
            bcdNumber = MtkPhoneNumberUtils.numberToCalledPartyBCD(updatedAnr);
            if (bcdNumber != null) {
                anrString[0] = (byte) aas;
                System.arraycopy(bcdNumber, 0, anrString,
                        2, bcdNumber.length);
                anrString[1] = (byte) (bcdNumber.length);
                // anrString[2] = (byte) 0x81;
            }
            return anrString;
        }
    }

    private byte[] buildEmailRecord(String strEmail, int adnIndex, int recordSize,
            boolean emailType2) {

        byte[] eMailRecData = new byte[recordSize]; // to be replaced by the record size

        for (int i = 0; i < recordSize; i++) {
            eMailRecData[i] = (byte) 0xFF;
        }

        if (strEmail != null && !strEmail.equals("")) {
            byte[] eMailData = GsmAlphabet.stringToGsm8BitPacked(strEmail);
            int maxDataLength = (((mEmailRecordSize != -1) && emailType2) ?
                                 (eMailRecData.length - USIM_TYPE2_CONDITIONAL_LENGTH) :
                                  eMailRecData.length);
            log("buildEmailRecord eMailData.length=" + eMailData.length +
                ", maxDataLength=" + maxDataLength);

            if (eMailData.length > maxDataLength) {
                return null;
            }

            System.arraycopy(eMailData, 0, eMailRecData, 0, eMailData.length);

            if (emailType2 && mPbrRecords != null) {
                int pbrIndex = (adnIndex - 1) / mAdnFileSize;
                int adnRecId = ((adnIndex % mAdnFileSize) & 0xFF);
                SparseArray<File> files = mPbrRecords.get(pbrIndex).mFileIds;
                File adnFile = files.get(USIM_EFADN_TAG);
                eMailRecData[recordSize - 2] = (byte) adnFile.getSfi();
                eMailRecData[recordSize - 1] = (byte) adnRecId;
                log("buildEmailRecord x+1=" + adnFile.getSfi() + ", x+2=" + adnRecId);
            }
        }
        return eMailRecData;
    }

    /**
     * Update phonebook list.
     * When a contact update done and success,
     * AdnRecordCache should call this to update phoneBook cache.
     *
     * @param index of the mPhoneBookRecords to be updated
     * @param newAdn to update the cache
     */
    public void updateUsimPhonebookRecordsList(int index, MtkAdnRecord newAdn) {
        logi("updateUsimPhonebookRecordsList update the " + index + "th record.");

        if (index < mPhoneBookRecords.size()) {
            MtkAdnRecord oldAdn = mPhoneBookRecords.get(index);
            if (oldAdn != null && oldAdn.getGrpIds() != null) {
                newAdn.setGrpIds(oldAdn.getGrpIds());
            }
            mPhoneBookRecords.set(index, newAdn);
            mRefreshAdnInfo = true;
        }
    }

    private void updatePhoneAdnRecordWithGrpByIndex(int recIndex, int adnIndex, int[] grpIds) {
        log("updatePhoneAdnRecordWithGrpByIndex the " + recIndex + "th grp ");

        if (recIndex > mPhoneBookRecords.size()) {
            return;
        }

        int grpSize = grpIds.length;

        if (grpSize > 0) {
            MtkAdnRecord rec;

            try {
                rec = mPhoneBookRecords.get(recIndex);
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG, "updatePhoneAdnRecordWithGrpByIndex: mPhoneBookRecords " +
                        "IndexOutOfBoundsException mPhoneBookRecords.size() is " +
                        mPhoneBookRecords.size() + "index is " + recIndex);
                return;
            }

            log("updatePhoneAdnRecordWithGrpByIndex the adnIndex is " + adnIndex
                    + "; the original index is " + rec.getRecId());
            StringBuilder grpIdsSb = new StringBuilder();

            for (int i = 0; i < grpSize - 1; i++) {
                grpIdsSb.append(grpIds[i]);
                grpIdsSb.append(",");
            }

            grpIdsSb.append(grpIds[grpSize - 1]);
            rec.setGrpIds(grpIdsSb.toString());
            log("updatePhoneAdnRecordWithGrpByIndex grpIds is " + grpIdsSb.toString());
            mPhoneBookRecords.set(recIndex, rec);
            log("updatePhoneAdnRecordWithGrpByIndex the rec:" + rec);
        }
    }

    //Email, ANR, SNE type1 use.
    private void readType1Ef(File file, int anrIndex) {
        log("readType1Ef:" + file);

        if (file.getParentTag() != USIM_TYPE1_TAG) {
            return;
        }

        int pbrIndex = file.mPbrRecord;
        int numAdnRecs = mPhoneBookRecords.size();
        int nOffset = pbrIndex * mAdnFileSize;
        int nMax = nOffset + mAdnFileSize;
        nMax = numAdnRecs < nMax ? numAdnRecs : nMax;
        int what = 0;
        int [] size = null;

        if (mRecordSize != null && mRecordSize.get(file.getEfid()) != null) {
            size = mRecordSize.get(file.getEfid());
        } else {
            size = readEFLinearRecordSize(file.getEfid());
        }

        if (size == null || size.length != 3) {
            Rlog.e(LOG_TAG, "readType1Ef: read record size error.");
                return;
        }

        int recordSize = size[0];
        int tag = file.mTag % 0x100;
        int fileIndex = file.mTag / 0x100;
        log("readType1Ef: RecordSize = " + recordSize);

        if (tag == USIM_EFEMAIL_TAG) {  // reset mEmailRecTable
            for (int i = nOffset; i < nOffset + mEmailFileSize; i++) {
                try {
                    mEmailRecTable[i] = 0;
                } catch (ArrayIndexOutOfBoundsException e) {
                    Rlog.e(LOG_TAG, "init RecTable error " + e.getMessage());
                    break;
                }
            }
        }

        if (recordSize == 0) {
            Rlog.w(LOG_TAG, "readType1Ef: recordSize is 0. ");
            return;
        }

        int totalReadingNum = 0;

        for (int i = nOffset; i < nMax; i++) {
            MtkAdnRecord rec;

            try {
                rec = mPhoneBookRecords.get(i);
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG,
                        "readType1Ef: mPhoneBookRecords IndexOutOfBoundsException numAdnRecs is "
                                + numAdnRecs + "index is " + i);
                break;
            }

            if (rec.getAlphaTag().length() > 0 || rec.getNumber().length() > 0) {
                int[] data = new int[3];
                data[0] = file.mPbrRecord;
                data[1] = i;
                data[2] = anrIndex;
                int loadWhat = 0;

                switch (tag) {
                    case USIM_EFANR_TAG:
                        loadWhat = EVENT_ANR_RECORD_LOAD_DONE;
                        mReadingAnrNum.addAndGet(1);
                        break;
                    case USIM_EFEMAIL_TAG:
                        //the email index
                        data[0] = i + 1 - nOffset + nOffset * mEmailFileSize;
                        loadWhat = EVENT_EMAIL_RECORD_LOAD_DONE;
                        mReadingEmailNum.incrementAndGet();
                        break;
                    case USIM_EFSNE_TAG:
                        loadWhat = EVENT_SNE_RECORD_LOAD_DONE;
                        mReadingSneNum.incrementAndGet();
                        break;
                    default:
                        Rlog.e(LOG_TAG, "not support tag " + file.mTag);
                        break;
                }

                mFh.readEFLinearFixed(file.getEfid(), i + 1 - nOffset, recordSize, obtainMessage(
                        loadWhat, data));
                totalReadingNum++;
            }
        }

        switch (tag) {
            case USIM_EFANR_TAG:
                if (mReadingAnrNum.get() == 0) {
                    mNeedNotify.set(false);
                    return;
                } else {
                    mNeedNotify.set(true);
                }
                break;
            case USIM_EFEMAIL_TAG:
                if (mReadingEmailNum.get() == 0) {
                    mNeedNotify.set(false);
                    return;
                } else {
                    mNeedNotify.set(true);
                }
                break;
            case USIM_EFSNE_TAG:
                if (mReadingSneNum.get() == 0) {
                    mNeedNotify.set(false);
                    return;
                } else {
                    mNeedNotify.set(true);
                }
                break;
            default:
                Rlog.e(LOG_TAG, "not support tag " + Integer.toHexString(file.mTag).toUpperCase());
                break;
        }

        logi("readType1Ef before mLock.wait " + mNeedNotify.get() + " total:" + totalReadingNum);

        synchronized (mLock) {
            try {
                mLock.wait();
            } catch (InterruptedException e) {
                Rlog.e(LOG_TAG, "Interrupted Exception in readType1Ef");
            }
        }

        logi("readType1Ef after mLock.wait " + mNeedNotify.get());
    }

    //Email ANR, SNE type2 use.
    private void readType2Ef(File file) {
        log("readType2Ef:" + file);

        if (file.getParentTag() != USIM_TYPE2_TAG) {
            return;
        }

        int recId = file.mPbrRecord;
        SparseArray<File> files = mPbrRecords.get(file.mPbrRecord).mFileIds;

        if (files == null) {
            Rlog.e(LOG_TAG, "Error: no fileIds");
            return;
        }

        File iapFile = files.get(USIM_EFIAP_TAG);
        if (iapFile == null) {
            Rlog.e(LOG_TAG, "Can't locate EF_IAP in EF_PBR.");
            return;
        }

        readIapFileAndWait(recId, iapFile.getEfid(), false);

        if (mIapFileList == null || mIapFileList.size() <= recId ||
            mIapFileList.get(recId).size() == 0) {
            Rlog.e(LOG_TAG, "Error: IAP file is empty");
            return;
        }

        int numAdnRecs = mPhoneBookRecords.size();
        int nOffset = recId * mAdnFileSize;
        int nMax = nOffset + mAdnFileSize;
        nMax = numAdnRecs < nMax ? numAdnRecs : nMax;

        switch (file.mTag) {
            case USIM_EFANR_TAG:
                break;
            case USIM_EFEMAIL_TAG:
                for (int i = nOffset; i < nOffset + mEmailFileSize; i++) {
                    try {
                        mEmailRecTable[i] = 0;
                    } catch (ArrayIndexOutOfBoundsException e) {
                        Rlog.e(LOG_TAG, "init RecTable error " + e.getMessage());
                        break;
                    }
                }
                break;
            case USIM_EFSNE_TAG:
                break;
            default:
                Rlog.e(LOG_TAG, "no implement type2 EF " + file.mTag);
                return;
        }

        int [] size = null;
        int efid = file.getEfid();
        if (mRecordSize != null && mRecordSize.get(efid) != null) {
            size = mRecordSize.get(efid);
        } else {
            size = readEFLinearRecordSize(efid);
        }

        if (size == null || size.length != 3) {
            Rlog.e(LOG_TAG, "readType2: read record size error.");
            return;
        }

        log("readType2: RecordSize = " + size[0]);
        ArrayList<byte[]> iapList = mIapFileList.get(recId);

        if (iapList.size() == 0) { //duplicate check
            Rlog.e(LOG_TAG, "Warning: IAP size is 0");
            return;
        }

        int type2Index = file.getIndex();
        int totalReadingNum = 0;

        for (int i = nOffset; i < nMax; i++) {
            MtkAdnRecord arec;

            try {
                arec = mPhoneBookRecords.get(i);
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG, "readType2Ef: mPhoneBookRecords " +
                        "IndexOutOfBoundsException numAdnRecs is " + numAdnRecs + "index is " + i);
                break;
            }

            if (arec.getAlphaTag().length() > 0 || arec.getNumber().length() > 0) {
                byte[] iapRecord = iapList.get(i - nOffset);
                int index = iapRecord[type2Index] & 0xFF;

                if (index <= 0 || index >= 255) {
                    continue;
                }

                log("Type2 iap[" + (i - nOffset) + "]=" + index);
                //data[0]: pbrIndex
                //data[1]: Adn recId
                int[] data = new int[3];
                data[0] = recId;
                data[1] = i;
                int loadWhat = 0;

                switch (file.mTag) {
                    case USIM_EFANR_TAG:
                        loadWhat = EVENT_ANR_RECORD_LOAD_DONE;
                        data[2] = file.mAnrIndex;
                        mReadingAnrNum.addAndGet(1);
                        break;
                    case USIM_EFEMAIL_TAG:
                        //the email index
                        data[0] = i + 1 - nOffset + nOffset * mEmailFileSize;
                        loadWhat = EVENT_EMAIL_RECORD_LOAD_DONE;
                        mReadingEmailNum.incrementAndGet();
                        break;
                    case USIM_EFSNE_TAG:
                        loadWhat = EVENT_SNE_RECORD_LOAD_DONE;
                        mReadingSneNum.incrementAndGet();
                        break;
                    default:
                        Rlog.e(LOG_TAG, "not support tag " + file.mTag);
                        break;
                }

                mFh.readEFLinearFixed(efid, index, size[0], obtainMessage(
                        loadWhat, data));
                totalReadingNum++;
            }
        }

        switch (file.mTag) {
        case USIM_EFANR_TAG:
            if (mReadingAnrNum.get() == 0) {
                mNeedNotify.set(false);
                return;
            } else {
                mNeedNotify.set(true);
            }
            break;
        case USIM_EFEMAIL_TAG:
            if (mReadingEmailNum.get() == 0) {
                mNeedNotify.set(false);
                return;
            } else {
                mNeedNotify.set(true);
            }
            break;
        case USIM_EFSNE_TAG:
            if (mReadingSneNum.get() == 0) {
                mNeedNotify.set(false);
                return;
            } else {
                mNeedNotify.set(true);
            }
            break;
        default:
            Rlog.e(LOG_TAG, "not support tag " + file.mTag);
            break;
        }

        logi("readType2Ef before mLock.wait " + mNeedNotify.get() + " total:" + totalReadingNum);

        synchronized (mLock) {
            try {
                mLock.wait();
            } catch (InterruptedException e) {
                Rlog.e(LOG_TAG, "Interrupted Exception in readType2Ef");
            }
        }
        logi("readType2Ef after mLock.wait " + mNeedNotify.get());
    }

    //data[0], data[1], payload
    private void updatePhoneAdnRecordWithEmailByIndex(int emailIndex, int adnIndex,
                                                      byte[] emailRecData) {
        log("updatePhoneAdnRecordWithEmailByIndex emailIndex = " + emailIndex +
                ",adnIndex = " + adnIndex);

        if (emailRecData == null) {
            return;
        }

        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            return;
        }

        boolean emailType2 =
                mPbrRecords.get(0).mFileIds.get(USIM_EFEMAIL_TAG).getParentTag() == USIM_TYPE2_TAG;
        log("updatePhoneAdnRecordWithEmailByIndex: Type2: " + emailType2 +
                " emailData: " + IccUtils.bytesToHexString(emailRecData));
        int length = emailRecData.length;

        if (emailType2 && emailRecData.length >= USIM_TYPE2_CONDITIONAL_LENGTH) {
            length = emailRecData.length - USIM_TYPE2_CONDITIONAL_LENGTH;
        }

        log("updatePhoneAdnRecordWithEmailByIndex length = " + length);

        byte[] validEMailData = new byte[length];

        for (int i = 0; i < length; i++) {
            validEMailData[i] = (byte) 0xFF;
        }

        System.arraycopy(emailRecData, 0, validEMailData, 0, length);

        try {
            String email = IccUtils.adnStringFieldToString(validEMailData, 0, length);
            if (email != null && !email.equals("")) {
                MtkAdnRecord rec = mPhoneBookRecords.get(adnIndex);
                rec.setEmails(new String[] {
                    email
                });
            }
            mEmailRecTable[emailIndex - 1] = adnIndex + 1;
        } catch (IndexOutOfBoundsException e) {
            Rlog.e(LOG_TAG, "[JE]updatePhoneAdnRecordWithEmailByIndex " + e.getMessage());
        }
    }

    private void updateType2Anr(String anr, int adnIndex, File file) {
        logi("updateType2Ef anr:" + anr + ",adnIndex:" + adnIndex + ",file:" + file);

        int pbrRecNum = (adnIndex - 1) / mAdnFileSize;
        int iapRecNum = (adnIndex - 1) % mAdnFileSize;

        log("updateType2Ef pbrRecNum:" + pbrRecNum + ",iapRecNum:" + iapRecNum);

        if (mIapFileList == null) {
            return;
        }
        if (file == null) {
            return;
        }
        SparseArray<File> files;
        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            return;
        }
        files = mPbrRecords.get(file.mPbrRecord).mFileIds;
        if (files == null) {
            return;
        }

        ArrayList<byte[]> list;

        try {
            list = mIapFileList.get(file.mPbrRecord);
        } catch (IndexOutOfBoundsException e) {
            Rlog.e(LOG_TAG, "Index out of bounds.");
            return;
        }

        if (list == null) {
            return;
        }
        if (list.size() == 0) {
            Rlog.e(LOG_TAG, "Warning: IAP size is 0");
            return;
        }

        byte[] iap = list.get(iapRecNum);

        if (iap == null) {
            return;
        }

        int index = iap[file.getIndex()] & 0xFF;
        log("updateType2Ef orignal index :" + index);

        if (anr == null || anr.length() == 0) {  //For Delete, update IAP
            if (index > 0) {
                iap[file.getIndex()] = (byte) 255;
                if (files.get(USIM_EFIAP_TAG) != null) {
                    mFh.updateEFLinearFixed(files.get(USIM_EFIAP_TAG).getEfid(),
                        iapRecNum + 1, iap, null, obtainMessage(EVENT_IAP_UPDATE_DONE));
                } else {
                    // Some USIM card does not confrim to SPEC,
                    // there is no IAP file in the USIM card.
                    Rlog.e(LOG_TAG, "updateType2Anr Error: No IAP file!");
                    return;
                }
            }
            return;
        }
        // found the index
        int recNum = 0;
        int[] tmpSize = mRecordSize.get(file.getEfid());
        int size = tmpSize[2];

        log("updateType2Anr size :" + size);
        if (index > 0 && index <= size) {
            recNum = index;
        } else {
            // insert
            int[] indexArray = new int[size + 1];

            for (int i = 1; i <= size; i++) {
                indexArray[i] = 0;
            }

            for (int i = 0; i < list.size(); i++) {
                byte[] value = list.get(i);
                if (value != null) {
                    int tem = value[file.getIndex()] & 0xFF;
                    if (tem > 0 && tem < 255 && tem <= size) {
                        indexArray[tem] = 1;
                    }
                }
            }
            // handle shared ANR Case begin
            boolean sharedAnr = false;
            File file2 = null;

            for (int i = 0; i < mPbrRecords.size(); i++) {
                if (i != file.mPbrRecord) {
                    file2 = mPbrRecords.get(i).mFileIds.get(USIM_EFANR_TAG + adnIndex * 0x100);
                    if (file2 == null) continue;
                    if (file2.getEfid() == file.getEfid()) {
                        sharedAnr = true;
                    }
                    break;
                }
            }

            if (sharedAnr) {
                ArrayList<byte[]> relatedList;

                try {
                    relatedList = mIapFileList.get(file2.mPbrRecord);

                    if (relatedList != null && relatedList.size() > 0) {
                        for (int i = 0; i < relatedList.size(); i++) {
                            byte[] value = relatedList.get(i);
                            if (value != null) {
                                int tem = value[file2.getIndex()] & 0xFF;
                                if (tem > 0 && tem < 255 && tem <= size) {
                                    indexArray[tem] = 1;
                                }
                            }
                        }
                    }
                } catch (IndexOutOfBoundsException e) {
                    Rlog.e(LOG_TAG, "Index out of bounds.");
                    return;
                }
            }
            // handle shared ANR Case end
            for (int i = 1; i <= size; i++) {
                if (indexArray[i] == 0) {
                    recNum = i;
                    break;
                }
            }
        }

        log("updateType2Anr final index :" + recNum);

        if (recNum == 0) {
            return;
        }

        byte[] data = null;
        int what = 0;
        int fileId = 0;
        MtkAdnRecord rec = null;

        try {
            rec = mPhoneBookRecords.get(adnIndex - 1);
        } catch (IndexOutOfBoundsException e) {
            Rlog.e(LOG_TAG, "updateType2Anr: mPhoneBookRecords " +
                    "IndexOutOfBoundsException mPhoneBookRecords.size() is " +
                    mPhoneBookRecords.size() + "index is " + (adnIndex - 1));
        }

        if (rec == null) {
            return;
        }

        int aas = rec.getAasIndex();
        data = buildAnrRecord(anr, mAnrRecordSize, aas);
        what = EVENT_ANR_UPDATE_DONE;
        fileId = file.getEfid();

        if (data != null) {
            mFh.updateEFLinearFixed(fileId, recNum, data, null, obtainMessage(what));
            if (recNum != index) {
                iap[file.getIndex()] = (byte) recNum;
                if (files.get(USIM_EFIAP_TAG) != null) {
                    mFh.updateEFLinearFixed(files.get(USIM_EFIAP_TAG).getEfid(),
                        iapRecNum + 1, iap, null, obtainMessage(EVENT_IAP_UPDATE_DONE));
                } else {
                    // Some USIM card does not confrim to SPEC,
                    // there is no IAP file in the USIM card.
                    Rlog.e(LOG_TAG, "updateType2Anr Error: No IAP file!");
                    return;
                }
            }
        }
    }

    private void readAnrRecordSize() {
        logi("readAnrRecordSize");

        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            Rlog.w(LOG_TAG, "readAnrRecordSize: PBR null ");
            return;
        }

        SparseArray<File> fileIds = mPbrRecords.get(0).mFileIds;

        if (fileIds == null) {
            Rlog.w(LOG_TAG, "readAnrRecordSize: fileIds null ");
            return;
        }

        File anrFile = fileIds.get(USIM_EFANR_TAG);

        if (fileIds.size() == 0 || anrFile == null) {
            mAnrRecordSize = 0;
            Rlog.w(LOG_TAG, "readAnrRecordSize: No anr tag in pbr file ");
            return;
        }

        int efid = anrFile.getEfid();
        int[] size = readEFLinearRecordSize(efid);

        if (size == null || size.length != 3) {
            Rlog.e(LOG_TAG, "readAnrRecordSize: read record size error.");
            return;
        }

        mAnrRecordSize = size[0];
        logi("readAnrRecordSize end size = " + mAnrRecordSize);
    }

    private void readEmailRecordSize() {
        logi("readEmailRecordSize");

        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            Rlog.w(LOG_TAG, "readEmailRecordSize: PBR null");
            return;
        }

        SparseArray<File> fileIds = mPbrRecords.get(0).mFileIds;

        if (fileIds == null) {
            Rlog.w(LOG_TAG, "readEmailRecordSize: fileId null");
            return;
        }

        File emailFile = fileIds.get(USIM_EFEMAIL_TAG);

        if (fileIds.size() == 0 || emailFile == null) {
            mEmailRecordSize = 0;
            Rlog.w(LOG_TAG, "readEmailRecordSize: No email tag in pbr file ");
            return;
        }

        int efid = emailFile.getEfid();
        int[] size = readEFLinearRecordSize(efid);

        if (size == null || size.length != 3) {
            Rlog.e(LOG_TAG, "readEmailRecordSize: read record size error.");
            return;
        }

        mEmailFileSize = size[2];
        mEmailRecordSize = size[0];
        logi("readEmailRecordSize Size:" + mEmailFileSize + "," + mEmailRecordSize);
    }

    private boolean loadAasFiles() {
        synchronized (mLock) {
            if (mAasForAnr == null || mAasForAnr.size() == 0) {
                if (!mIsPbrPresent) {
                    Rlog.e(LOG_TAG, "No PBR files");
                    return false;
                }

                loadPBRFiles();

                if (mPbrRecords == null) {
                    return false;
                }

                int numRecs = mPbrRecords.size();

                if (mAasForAnr == null) {
                    mAasForAnr = new ArrayList<String>();
                }

                mAasForAnr.clear();
                logi("loadAasFiles read num:" + numRecs + ", " + mPbrNeedNotify);
                if (!CsimPhbUtil.hasModemPhbEnhanceCapability(mFh)) {
                    for (int i = 0; i < numRecs; i++) {
                        readAASFileAndWait(i);
                    }
                } else {
                    readAasFileAndWaitOptmz();
                }
            }
            return true;
        }
    }

    public ArrayList<AlphaTag> getUsimAasList() {
        logi("getUsimAasList start");

        ArrayList<AlphaTag> results = new ArrayList<AlphaTag>();
        if (loadAasFiles() == false) return results;

        ArrayList<String> allAas = mAasForAnr;
        if (allAas == null) return results;

        for (int i = 0; i < 1; i++) {
            for (int j = 0; j < allAas.size(); j++) {
                String value = allAas.get(j);
                logi("aasIndex:" + (j + 1) + ",pbrIndex:" + i + ",value:" + value);
                AlphaTag tag = new AlphaTag(j + 1, value, i);
                results.add(tag);
            }
        }
        return results;
    }

    public String getUsimAasById(int index, int pbrIndex) {
        logi("getUsimAasById by id " + index + ",pbrIndex " + pbrIndex);

        if (loadAasFiles() == false) return null;

        ArrayList<String> map = mAasForAnr;

        if (map != null) {
            logi("getUsimAasById NonNULL by id " + index + ",pbrIndex " + pbrIndex);
            return map.get(index - 1);
        }

        logi("getUsimAasById NULL by id " + index + ",pbrIndex " + pbrIndex);
        return null;
    }

    public boolean removeUsimAasById(int index, int pbrIndex) {
        logi("removeUsimAasById by id " + index + ",pbrIndex " + pbrIndex);

        if (loadAasFiles() == false) return false;   // return true or false here?

        int aasIndex = index;
        SparseArray<File> files = mPbrRecords.get(pbrIndex).mFileIds;

        if (files == null || files.get(USIM_EFAAS_TAG) == null) {
            Rlog.e(LOG_TAG, "removeUsimAasById-PBR have no AAS EF file");
            return false;
        }

        int efid = files.get(USIM_EFAAS_TAG).getEfid();
        log("removeUsimAasById result,efid:" + efid);

        if (mFh != null) {
            Message msg = obtainMessage(EVENT_AAS_UPDATE_DONE);
            int len = getUsimAasMaxNameLen();
            byte[] aasString = new byte[len];

            for (int i = 0; i < len; i++) {
                aasString[i] = (byte) 0xFF;
            }

            synchronized (mAasLock) {
                //mFh.updateEFLinearFixed(efid, aasIndex, aasString, null, msg);
                mCi.deleteUPBEntry(UPB_EF_AAS, 1, aasIndex, msg);
                try {
                    mAasLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in removesimAasById");
                }
            }

            AsyncResult ar = (AsyncResult) msg.obj;

            if (ar == null || ar.exception == null) {
                ArrayList<String> list = mAasForAnr;
                if (list != null) {
                    log("remove aas done " + list.get(aasIndex - 1));
                    list.set(aasIndex - 1, null);
                } else {
                    log("remove aas mAasForAnr is null ");
                }
                return true;
            } else {
                Rlog.e(LOG_TAG, "removeUsimAasById exception " + ar.exception);
                return false;
            }
        } else {
            Rlog.e(LOG_TAG, "removeUsimAasById-IccFileHandler is null");
            return false;
        }
    }

    public int insertUsimAas(String aasName) {
        logi("insertUsimAas begin" + aasName);

        if (aasName == null || aasName.length() == 0) {
            return 0;
        }

        if (loadAasFiles() == false) return -1;

        int limit = getUsimAasMaxNameLen();
        int len = aasName.length();

        if (len > limit) {
            return 0;
        }

        int index = -1;

        synchronized (mAasLock) {
            int aasIndex = 0;
            boolean found = false;
            // find empty position.
            ArrayList<String> allAas = mAasForAnr;

            for (int j = 0; j < allAas.size(); j++) {
                String value = allAas.get(j);
                if (value == null || value.length() == 0) {
                    found = true;
                    aasIndex = j + 1;
                    break;
                }
            }

            log("insertUsimAas aasIndex:" + aasIndex + ",found:" + found);
            if (!found) {
                // TODO full
                return -2;
            }

            String temp = encodeToUcs2(aasName);
            Message msg = obtainMessage(EVENT_AAS_UPDATE_DONE);
            mCi.editUPBEntry(UPB_EF_AAS, 0, aasIndex, temp, null, msg);

            try {
                mAasLock.wait();
            } catch (InterruptedException e) {
                Rlog.e(LOG_TAG, "Interrupted Exception in insertUsimAas");
            }

            AsyncResult ar = (AsyncResult) msg.obj;
            logi("insertUsimAas UPB_EF_AAS: ar " + ar);

            if (ar == null || ar.exception == null) {
                ArrayList<String> list = mAasForAnr;
                if (list != null) {
                    list.set(aasIndex - 1, aasName);
                    logi("insertUsimAas update mAasForAnr done");
                } else {
                    logi("insertUsimAas mAasForAnr is null");
                }
                return aasIndex;
            } else {
                Rlog.e(LOG_TAG, "insertUsimAas exception " + ar.exception);
                return -1;
            }
        }
    }

    public boolean updateUsimAas(int index, int pbrIndex, String aasName) {
        logi("updateUsimAas index " + index + ",pbrIndex " + pbrIndex + ",aasName " + aasName);

        if (loadAasFiles() == false) return false;

        ArrayList<String> map = mAasForAnr;

        if (index <= 0 || index > map.size()) {
            Rlog.e(LOG_TAG, "updateUsimAas not found aas index " + index);
            return false;
        }

        String aas = map.get(index - 1);
        log("updateUsimAas old aas " + aas);

        if (aasName == null || aasName.length() == 0) {  // delete
            return removeUsimAasById(index, pbrIndex);
        } else {  // update
            int limit = getUsimAasMaxNameLen();
            int len = aasName.length();

            log("updateUsimAas aas limit " + limit);

            if (len > limit) {
                return false;
            }

            int offset = 0;
            log("updateUsimAas offset " + offset);
            int aasIndex = index + offset;
            String temp = encodeToUcs2(aasName);
            Message msg = obtainMessage(EVENT_AAS_UPDATE_DONE);

            synchronized (mAasLock) {
                mCi.editUPBEntry(UPB_EF_AAS, 0, aasIndex, temp, null, msg);
                try {
                    mAasLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in updateUsimAas");
                }
            }

            AsyncResult ar = (AsyncResult) msg.obj;

            if (ar == null || ar.exception == null) {
                ArrayList<String> list = mAasForAnr;
                if (list != null) {
                    list.set(index - 1, aasName);
                    logi("updateUsimAas update mAasForAnr done");
                } else {
                    logi("updateUsimAas mAasForAnr is null");
                }
                return true;
            } else {
                Rlog.e(LOG_TAG, "updateUsimAas exception " + ar.exception);
                return false;
            }
        }
    }

    /**
     * @param adnIndex ADN index
     * @param aasIndex change AAS to the value refered by aasIndex, -1 means remove
     * @return true for update success
     */
    public boolean updateAdnAas(int adnIndex, int aasIndex) {
        int pbrRecNum = (adnIndex - 1) / mAdnFileSize;
        int index = (adnIndex - 1) % mAdnFileSize;
        MtkAdnRecord rec;

        try {
            rec = mPhoneBookRecords.get(adnIndex - 1);
        } catch (IndexOutOfBoundsException e) {
            Rlog.e(LOG_TAG, "updateADNAAS: mPhoneBookRecords " +
                    "IndexOutOfBoundsException mPhoneBookRecords.size() is " +
                    mPhoneBookRecords.size() + "index is " + (adnIndex - 1));
            return false;
        }

        rec.setAasIndex(aasIndex);
        // TODO update aas
        for (int i = 0; i < 3; i++) {
            String anr = rec.getAdditionalNumber(i);
            updateAnrByAdnIndex(anr, adnIndex, i, rec);
        }

        return true;
    }

    public int getUsimAasMaxNameLen() {
        logi("getUsimAasMaxNameLen begin");

        synchronized (mUPBCapabilityLock) {
            if (mUpbCap[4] < 0 && checkIsPhbReady()) {
                mCi.queryUPBCapability(obtainMessage(EVENT_UPB_CAPABILITY_QUERY_DONE));
                try {
                    mUPBCapabilityLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in getUsimAasMaxNameLen");
                }
            }
        }

        logi("getUsimAasMaxNameLen done: " + "L_AAS is " + mUpbCap[4]);

        return mUpbCap[4];
    }

    public int getUsimAasMaxCount() {
        logi("getUsimAasMaxCount begin");

        synchronized (mUPBCapabilityLock) {
            if (mUpbCap[3] < 0 && checkIsPhbReady()) {
                mCi.queryUPBCapability(obtainMessage(EVENT_UPB_CAPABILITY_QUERY_DONE));
                try {
                    mUPBCapabilityLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in getUsimAasMaxCount");
                }
            }
        }

        logi("getUsimAasMaxCount done: " + "N_AAS is " + mUpbCap[3]);

        return mUpbCap[3];
    }

    public void loadPBRFiles() {
        if (!mIsPbrPresent) {
            return;
        }

        synchronized (mLock) {
            // Check if the PBR file is present in the cache, if not read it
            // from the USIM.
            if (mPbrRecords == null) {
                mPbrNeedNotify++;
                readPbrFileAndWait();
            }
        }
    }

    public int getAnrCount() {
        logi("getAnrCount begin");

        synchronized (mUPBCapabilityLock) {
            if (mUpbCap[0] < 0 && checkIsPhbReady()) {
                mCi.queryUPBCapability(obtainMessage(EVENT_UPB_CAPABILITY_QUERY_DONE));
                try {
                    mUPBCapabilityLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in getAnrCount");
                }
            }
        }

        if (0 >= mAnrRecordSize) {
            logi("getAnrCount end mAnrRecordSize:" + mAnrRecordSize);
            return mAnrRecordSize;
        }
        logi("getAnrCount done: " + "N_ANR is " + mUpbCap[0]);

        // TODO Support all ANRs
        return mUpbCap[0] > 0 ? 1 : 0;
    }

    public int getEmailCount() {
        logi("getEmailCount begin");

        synchronized (mUPBCapabilityLock) {
            if (mUpbCap[1] < 0 && checkIsPhbReady()) {
                mCi.queryUPBCapability(obtainMessage(EVENT_UPB_CAPABILITY_QUERY_DONE));

                try {
                    mUPBCapabilityLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in getEmailCount");
                }
            }
        }

        if (0 >= mEmailRecordSize) {
            logi("getEmailCount end mEmailRecordSize:" + mEmailRecordSize);
            return mEmailRecordSize;
        }

        logi("getEmailCount done: " + "N_EMAIL is " + mUpbCap[1]);

        return mUpbCap[1] > 0 ? 1 : 0;
    }

    public boolean hasSne() {
        log("hasSne begin");

        synchronized (mUPBCapabilityLock) {
            if (mUpbCap[2] < 0 && checkIsPhbReady()) {
                mCi.queryUPBCapability(obtainMessage(EVENT_UPB_CAPABILITY_QUERY_DONE));

                try {
                    mUPBCapabilityLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in hasSne");
                }
            }
        }

        log("hasSne done: " + "N_Sne is " + mUpbCap[2]);
        return mUpbCap[2] > 0 ? true : false;
    }

    public int getSneRecordLen() {  // equals to mSneInfo[2]
        int resultSize = 0;

        if (!hasSne()) {
            return 0;
        }

        if (mPbrRecords == null || mPbrRecords.size() == 0 ||  mPbrRecords.get(0) == null) {
            return -1;
        }

        SparseArray<File> files = mPbrRecords.get(0).mFileIds;
        if (files == null) {
            return -1;
        }

        File sneFile = files.get(USIM_EFSNE_TAG);
        if (sneFile == null) {
            return 0;
        }

        int efid = sneFile.getEfid();
        boolean sneType2 = sneFile.getParentTag() == USIM_TYPE2_TAG;
        logi("getSneRecordLen: EFSNE id is " + efid);
        int [] size = null;

        if (mRecordSize != null && mRecordSize.get(efid) != null) {
            size = mRecordSize.get(efid);
        } else {
            size = readEFLinearRecordSize(efid);
        }

        if (size != null) {
            if (sneType2) {
                resultSize = size[0] - 2;
            } else {
                resultSize = size[0];
            }
        }

        return resultSize;
    }

    public int getUpbDone() {
        return mUpbDone;
    }

    private void updatePhoneAdnRecordWithSneByIndex(int recNum, int adnIndex, byte[] recData) {
        if (recData == null) {
            return;
        }

        String sne = IccUtils.adnStringFieldToString(recData, 0, recData.length);
        log("updatePhoneAdnRecordWithSneByIndex index " + adnIndex + " recData file is " + sne);

        if (sne != null && !sne.equals("")) {
            MtkAdnRecord rec;

            try {
                rec = mPhoneBookRecords.get(adnIndex);
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG, "updatePhoneAdnRecordWithSneByIndex: mPhoneBookRecords " +
                        "IndexOutOfBoundsException mPhoneBookRecords.size() is " +
                        mPhoneBookRecords.size() + "index is " + adnIndex);
                return;
            }
            rec.setSne(sne);
        }
    }

    /**
     * Update a contact's second name entry.
     * CSIM and USIM use different way to update.
     *
     * @param sne sne string to be updated
     * @param adnIndex ADN index
     * @return 0 for success, USIM_ERROR_* for error
     */
    public int updateSneByAdnIndex(String sne, int adnIndex, MtkAdnRecord oldAdn) {
        logi("updateSneByAdnIndex begin sne is " + sne + ",adnIndex " + adnIndex);

        int pbrRecNum = (adnIndex - 1) / mAdnFileSize;
        int nIapRecNum = (adnIndex - 1) % mAdnFileSize;

        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            return -1;
        }

        Message msg = obtainMessage(EVENT_SNE_UPDATE_DONE);
        SparseArray<File> files = mPbrRecords.get(pbrRecNum).mFileIds;

        if (files == null || files.get(USIM_EFSNE_TAG) == null) {
            log("updateSneByAdnIndex: No SNE tag in pbr file 0");
            return -1;
        }

        if (mPhoneBookRecords == null || mPhoneBookRecords.isEmpty()) {
            return -1;
        }
        // To reduce unnecessary request, we check if sne value changed.
        String oldSne = null;

        if (oldAdn != null) {
            oldSne = oldAdn.getSne();
        }

        File sneFile = files.get(USIM_EFSNE_TAG);
        int efid = sneFile.getEfid();
        log("updateSneByAdnIndex: EF_SNE id is " + Integer.toHexString(efid).toUpperCase());
        int efIndex = 1; // We assume Icc only have one SNE.

        log("updateSneByAdnIndex: efIndex is " + efIndex);

        synchronized (mLock) {
            if (sne == null || sne.length() == 0) {
                if (oldSne == null || oldSne.length() == 0) {
                    return 0;
                }
                // delete
                mCi.deleteUPBEntry(UPB_EF_SNE, efIndex, adnIndex, msg);
            } else {
                if (sne.equals(oldSne)) {
                    return 0;
                }
                String temp = encodeToUcs2(sne);

                mCi.editUPBEntry(UPB_EF_SNE, efIndex, adnIndex, temp, null, msg);
            }

            try {
                mLock.wait();
            } catch (InterruptedException e) {
                Rlog.e(LOG_TAG, "Interrupted Exception in updateSneByAdnIndex");
            }
        }
        logi("updateSneByAdnIndex end sne is " + sne + ",adnIndex "
                + adnIndex);
        return mResult;
    }

    /**
     * Get the capacity of ADN records.
     *
     * @return  int[6] array
     *            capacity[0]  is the max count of ADN
     *            capacity[1]  is the used count of ADN
     *            capacity[2]  is the max count of EMAIL
     *            capacity[3]  is the used count of EMAIL
     *            capacity[4]  is the max count of ANR
     *            capacity[5]  is the used count of ANR
     */
    public int[] getAdnRecordsCapacity() {
        int[] capacity = new int[6];

        // ADN
        if (mRefreshAdnInfo || mRefreshEmailInfo || mRefreshAnrInfo || mAdnRecordSize == null
                || mAdnRecordSize.length != 4) {
            getAdnStorageInfo();
            mRefreshAdnInfo = false;
        }

        if (mAdnRecordSize == null || mAdnRecordSize.length != 4) {
            return null;
        } else {
            capacity[0] = mAdnRecordSize[1];
            capacity[1] = mAdnRecordSize[0];
        }

        // Email
        if (mRefreshEmailInfo || mEmailInfo == null || mEmailInfo.length != 3) {
            mCi.queryUPBAvailable(UPB_EF_EMAIL, 1,
                    obtainMessage(EVENT_QUERY_EMAIL_AVAILABLE_OPTMZ_DONE));
            synchronized (mLock) {
                try {
                    mLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in getAdnRecordsCapacity");
                }
            }
            mRefreshEmailInfo = false;
        }

        if (mEmailInfo == null || mEmailInfo.length != 3) {
            return null;
        } else {
            capacity[2] = mEmailInfo[0];
            capacity[3] = mEmailInfo[0] - mEmailInfo[1];
        }

        // Anr
        if (mRefreshAnrInfo || mAnrInfo == null || mAnrInfo.get(0) == null
                || mAnrInfo.get(0).length != 3) {
            mCi.queryUPBAvailable(UPB_EF_ANR, 1,
                    obtainMessage(EVENT_QUERY_ANR_AVAILABLE_OPTMZ_DONE));
            synchronized (mLock) {
                try {
                    mLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in getAdnRecordsCapacity");
                }
            }
            mRefreshAnrInfo = false;
        }

        if (mAnrInfo == null || mAnrInfo.get(0) == null || mAnrInfo.get(0).length != 3) {
            return null;
        } else {
            capacity[4] = mAnrInfo.get(0)[0];
            capacity[5] = mAnrInfo.get(0)[0] - mAnrInfo.get(0)[1];
        }

        logi("getAdnRecordsCapacity: max adn=" + capacity[0] +
                ", used adn=" + capacity[1] + ", max email=" + capacity[2] +
                ", used email=" + capacity[3] + ", max anr=" + capacity[4] +
                ", used anr=" + capacity[5]);

        return capacity;
    }

    private int[] getAdnStorageInfo() {
        logi("getAdnStorageInfo");

        if (mCi != null) {
            mCi.queryPhbStorageInfo(MtkRILConstants.PHB_ADN,
                    obtainMessage(EVENT_QUERY_PHB_ADN_INFO));

            synchronized (mLock) {
                try {
                    mLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in getAdnStorageInfo");
                }
            }
        } else {
            Rlog.w(LOG_TAG, "GetAdnStorageInfo: filehandle is null.");
            return null;
        }

        return mAdnRecordSize;
    }

    public UsimPBMemInfo[] getPhonebookMemStorageExt() {
        boolean is3G = mCurrentApp.getType() == AppType.APPTYPE_USIM;

        logi("getPhonebookMemStorageExt isUsim " + is3G);

        if (!is3G) {
            return getPhonebookMemStorageExt2G();
        }

        if (mPbrRecords == null) {
            loadPBRFiles();
        }

        if (mPbrRecords == null) {
            return null;
        }

        log("getPhonebookMemStorageExt slice " + mPbrRecords.size());
        UsimPBMemInfo[] response = new UsimPBMemInfo[mPbrRecords.size()];

        for (int i = 0; i < mPbrRecords.size(); i++) {
            response[i] = new UsimPBMemInfo();
        }

        if (mPhoneBookRecords.isEmpty()) {
            Rlog.w(LOG_TAG, "mPhoneBookRecords has not been loaded.");
            return response;
        }

        int[] size = null;
        int used = 0;
        int currentTotal = 0; //get totoal size of current Nth EF file.

        for (int pbrIndex = 0; pbrIndex < mPbrRecords.size(); pbrIndex++) {
            SparseArray<File> files = mPbrRecords.get(pbrIndex).mFileIds;
            int numAdnRecs = mPhoneBookRecords.size();
            int nOffset = pbrIndex * mAdnFileSize;
            int nMax = nOffset + mAdnFileSize;
            nMax = numAdnRecs < nMax ? numAdnRecs : nMax;

            // Adn
            File adnFile = files.get(USIM_EFADN_TAG);

            if (adnFile != null) {
                size = readEFLinearRecordSize(adnFile.getEfid());
                if (size != null) {
                    response[pbrIndex].setAdnLength(size[0]);
                    if (0 < currentTotal) {
                        response[pbrIndex].setAdnTotal(currentTotal + size[2]);
                    } else {
                        response[pbrIndex].setAdnTotal(size[2]);
                    }
                }

                response[pbrIndex].setAdnType(adnFile.getParentTag());
                response[pbrIndex].setSliceIndex(pbrIndex + 1);

                used = 0;

                MtkAdnRecord rec = null;

                for (int j = nOffset; j < nMax; j++) {
                    try {
                    rec = mPhoneBookRecords.get(j);
                    } catch (IndexOutOfBoundsException e) {
                        Rlog.e(LOG_TAG, "getPhonebookMemStorageExt: mPhoneBookRecords " +
                                "IndexOutOfBoundsException mPhoneBookRecords.size() is " +
                                mPhoneBookRecords.size() + "index is " + j);
                    }
                    if (rec != null &&
                        ((rec.getAlphaTag() != null && rec.getAlphaTag().length() > 0) ||
                         (rec.getNumber() != null && rec.getNumber().length() > 0))) {
                        log("Adn: " + rec.toString());
                        used++;
                        rec = null;
                    }
                }
                log("adn used " + used);
                response[pbrIndex].setAdnUsed(used);
            }

            // ANR
            File anrFile = files.get(USIM_EFANR_TAG);

            if (anrFile != null) {
                size = readEFLinearRecordSize(anrFile.getEfid());
                if (size != null) {
                    response[pbrIndex].setAnrLength(size[0]);
                    response[pbrIndex].setAnrTotal(size[2]);
                }
                response[pbrIndex].setAnrType(anrFile.getParentTag());
                used = 0;

                MtkAdnRecord rec = null;
                String anrStr = null;

                for (int i = nOffset; i < nMax; i++) {
                    try {
                        rec = mPhoneBookRecords.get(i);
                    } catch (IndexOutOfBoundsException e) {
                        Rlog.e(LOG_TAG, "getPhonebookMemStorageExt: mPhoneBookRecords " +
                                "IndexOutOfBoundsException mPhoneBookRecords.size() is " +
                                mPhoneBookRecords.size() + "index is " + i);
                    }

                    if (null == rec) {
                        log("null anr rec ");
                        continue;
                    }

                    anrStr = rec.getAdditionalNumber();

                    if (null != anrStr && 0 < anrStr.length()) {
                        log("anrStr: " + anrStr);
                        used++;
                    }
                }
                log("anr used: " + used);
                response[pbrIndex].setAnrUsed(used);
            }

            // Email
            File emailFile = files.get(USIM_EFEMAIL_TAG);

            if (emailFile != null) {
                size = readEFLinearRecordSize(emailFile.getEfid());
                if (size != null) {
                    response[pbrIndex].setEmailLength(size[0]);
                    response[pbrIndex].setEmailTotal(size[2]);
                }

                response[pbrIndex].setEmailType(emailFile.getParentTag());
                used = 0;
                MtkAdnRecord rec = null;
                String[] emails = null;

                for (int i = nOffset; i < nMax; i++) {
                    try {
                        rec = mPhoneBookRecords.get(i);
                    } catch (IndexOutOfBoundsException e) {
                        Rlog.e(LOG_TAG, "getPhonebookMemStorageExt: mPhoneBookRecords " +
                                "IndexOutOfBoundsException mPhoneBookRecords.size() is " +
                                mPhoneBookRecords.size() + "index is " + i);
                    }

                    if (null == rec) {
                        log("null email rec ");
                        continue;
                    }

                    emails = rec.getEmails();

                    if (null != emails && emails.length > 0 && emails[0].length() > 0) {
                        log("email: " + emails[0]);
                        used++;
                    }
                }
                log("email used: " + used);
                response[pbrIndex].setEmailUsed(used);
            }

            // Ext1
            File ext1File = files.get(USIM_EFEXT1_TAG);

            if (ext1File != null) {
                size = readEFLinearRecordSize(ext1File.getEfid());
                if (size != null) {
                    response[pbrIndex].setExt1Length(size[0]);
                    response[pbrIndex].setExt1Total(size[2]);
                }

                response[pbrIndex].setExt1Type(ext1File.getParentTag());
                synchronized (mLock) {
                    readExt1FileAndWait(pbrIndex);
                }

                used = 0;

                if (mExt1FileList != null && pbrIndex < mExt1FileList.size()) {
                    ArrayList<byte[]> ext1 = mExt1FileList.get(pbrIndex);
                    if (ext1 != null) {
                        int len = ext1.size();
                        for (int i = 0; i < len; i++) {
                            byte[] arr = ext1.get(i);
                            log("ext1[" + i + "]=" + IccUtils.bytesToHexString(arr));
                            if (arr != null && arr.length > 0) {
                                if (arr[0] == 1 || arr[0] == 2) {
                                    used++;
                                }
                            }
                        }
                    }
                }
                response[pbrIndex].setExt1Used(used);
            }

            // Gas
            File gasFile = files.get(USIM_EFGSD_TAG);
            if (gasFile != null) {
                size = readEFLinearRecordSize(gasFile.getEfid());
                if (size != null) {
                    response[pbrIndex].setGasLength(size[0]);
                    response[pbrIndex].setGasTotal(size[2]);
                }
                response[pbrIndex].setGasType(gasFile.getParentTag());
            }

            // Aas
            File aasFile = files.get(USIM_EFAAS_TAG);
            if (aasFile != null) {
                size = readEFLinearRecordSize(aasFile.getEfid());
                if (size != null) {
                    response[pbrIndex].setAasLength(size[0]);
                    response[pbrIndex].setAasTotal(size[2]);
                }
                response[pbrIndex].setAasType(aasFile.getParentTag());
            }

            // Sne
            File sneFile = files.get(USIM_EFSNE_TAG);
            if (sneFile != null) {
                size = readEFLinearRecordSize(sneFile.getEfid());
                if (size != null) {
                    response[pbrIndex].setSneLength(size[0]);
                    response[pbrIndex].setSneTotal(size[0]);
                }
                response[pbrIndex].setSneType(sneFile.getParentTag());
            }

            // Ccp
            File ccpFile = files.get(USIM_EFCCP1_TAG);
            if (ccpFile != null) {
                size = readEFLinearRecordSize(ccpFile.getEfid());
                if (size != null) {
                    response[pbrIndex].setCcpLength(size[0]);
                    response[pbrIndex].setCcpTotal(size[0]);
                }
                response[pbrIndex].setCcpType(ccpFile.getParentTag());
            }
        }

        for (int i = 0; i < mPbrRecords.size(); i++) {
            log("getPhonebookMemStorageExt[" + i + "]:" + response[i]);
        }

        return response;
    }

    public UsimPBMemInfo[] getPhonebookMemStorageExt2G() {
        UsimPBMemInfo[] response = new UsimPBMemInfo[1];
        response[0] = new UsimPBMemInfo();
        int[] size = null;
        size = readEFLinearRecordSize(IccConstants.EF_ADN);

        if (size != null) {
            response[0].setAdnLength(size[0]);
            if (isAdnAccessible() == true) {
                response[0].setAdnTotal(size[2]);
            } else {
                response[0].setAdnTotal(0);
            }
        }

        response[0].setAdnType(USIM_TYPE1_TAG);
        response[0].setSliceIndex(1);
        size = readEFLinearRecordSize(IccConstants.EF_EXT1);

        if (size != null) {
            response[0].setExt1Length(size[0]);
            response[0].setExt1Total(size[2]);
        }

        response[0].setExt1Type(USIM_TYPE3_TAG);

        synchronized (mLock) {
            if (mFh != null) {
                Message msg = obtainMessage(EVENT_EXT1_LOAD_DONE);
                msg.arg1 = 0;
                mFh.loadEFLinearFixedAll(IccConstants.EF_EXT1, msg);
                try {
                    mLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in readExt1FileAndWait");
                }
            } else {
                Rlog.e(LOG_TAG, "readExt1FileAndWait-IccFileHandler is null");
                return response;
            }
        }

        int used = 0;

        if (mExt1FileList != null && mExt1FileList.size() > 0) {
            ArrayList<byte[]> ext1 = mExt1FileList.get(0);

            if (ext1 != null) {
                int len = ext1.size();

                for (int i = 0; i < len; i++) {
                    byte[] arr = ext1.get(i);
                    log("ext1[" + i + "]=" + IccUtils.bytesToHexString(arr));
                    if (arr != null && arr.length > 0) {
                        if (arr[0] == 1 || arr[0] == 2) {
                            used++;
                        }
                    }
                }
            }
        }

        response[0].setExt1Used(used);
        logi("getPhonebookMemStorageExt2G:" + response[0]);
        return response;
    }

    /**
     * Get record size for a linear fixed EF.
     *
     * @param fileId EF id
     * @return is the recordSize[]
     *         int[0] is the record length
     *         int[1] is the total length of the EF file
     *         int[2] is the number of records in the EF file
     *         So int[0] * int[2] = int[1]
     */
    public int[] readEFLinearRecordSize(int fileId) {
        logi("readEFLinearRecordSize fileid:" + Integer.toHexString(fileId).toUpperCase() +
                ",recordNum:" + mReadEFLinerRecordSizeNum);

        Message msg = obtainMessage(EVENT_GET_RECORDS_SIZE_DONE);
        msg.arg1 = fileId;

        synchronized (mLock) {
            if (mFh != null) {
                // to record if reset to notify all
                mReadEFLinerRecordSizeNum++;
                mFh.getEFLinearRecordSize(fileId, msg);

                try {
                    mLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in readEFLinearRecordSize");
                }
            } else {
                Rlog.e(LOG_TAG, "readEFLinearRecordSize-IccFileHandler is null");
            }

            if (mFh != null) {
                mReadEFLinerRecordSizeNum--;
            }

            int[] recordSize = mRecordSize != null ? mRecordSize.get(fileId) : null;

            if (recordSize != null) {
                logi("readEFLinearRecordSize fileid:" + Integer.toHexString(fileId).toUpperCase() +
                        ",len:" + recordSize[0] + ",total:" + recordSize[1] +
                        ",count:" + recordSize[2] + ",recordNum:" + mReadEFLinerRecordSizeNum);
            } else {
                logi("readEFLinearRecordSize fileid:" + Integer.toHexString(fileId).toUpperCase() +
                        ",recordSize: null");
            }
            return recordSize;
        }
    }

    private void readExt1FileAndWait(int recId) {
        logi("readExt1FileAndWait " + recId);

        if (mPbrRecords == null || mPbrRecords.size() == 0 || mPbrRecords.get(recId) == null) {
            return;
        }

        SparseArray<File> files = mPbrRecords.get(recId).mFileIds;

        if (files == null || files.get(USIM_EFEXT1_TAG) == null) {
            Rlog.e(LOG_TAG, "readExt1FileAndWait-PBR have no Ext1 record");
            return;
        }

        int efid = files.get(USIM_EFEXT1_TAG).getEfid();
        log("readExt1FileAndWait-get EXT1 EFID " + efid);

        if (mExt1FileList != null) {
            if (recId < mExt1FileList.size()) {
                log("EXT1 has been loaded for Pbr number " + recId);
                return;
            }
        }

        if (mFh != null) {
            Message msg = obtainMessage(EVENT_EXT1_LOAD_DONE);
            msg.arg1 = recId;
            mFh.loadEFLinearFixedAll(efid, msg);
            try {
                mLock.wait();
            } catch (InterruptedException e) {
                Rlog.e(LOG_TAG, "Interrupted Exception in readExt1FileAndWait");
            }
        } else {
            Rlog.e(LOG_TAG, "readExt1FileAndWait-IccFileHandler is null");
            return;
        }
    }

    private boolean checkIsPhbReady() {
        String strPhbReady = "false";
        String strAllSimState = "";
        String strCurSimState = "";
        boolean isSimLocked = false;

        if (SubscriptionManager.isValidSlotIndex(mSlotId) == false) {
            log("[isPhbReady] InvalidSlotId slotId: " + mSlotId);
            return false;
        }

        strPhbReady = TelephonyManager.getTelephonyProperty(mSlotId,
                "vendor.gsm.sim.ril.phbready", "false");
        // after Gen93 chip, phb should not care about sim state since modem will care
        if (SystemProperties.get("ro.vendor.mtk_ril_mode").equals("c6m_1rild")) {
            logi("[isPhbReady] isPhbReady: " + strPhbReady);
            return (strPhbReady.equals("true"));
        }

        strAllSimState = SystemProperties.get(TelephonyProperties.PROPERTY_SIM_STATE);
        if ((strAllSimState != null) && (strAllSimState.length() > 0)) {
            String values[] = strAllSimState.split(",");
            if ((mSlotId >= 0) && (mSlotId < values.length) && (values[mSlotId] != null)) {
                strCurSimState = values[mSlotId];
            }
        }

        isSimLocked = (strCurSimState.equals("NETWORK_LOCKED") ||
                       strCurSimState.equals("PIN_REQUIRED"));
                       // In PUK_REQUIRED state, phb can be accessed.
        logi("[isPhbReady] isPhbReady: " + strPhbReady + ",strSimState: " + strAllSimState);
        return (strPhbReady.equals("true") && !isSimLocked);
    }

    public boolean isAdnAccessible() {
        /* For SIM, need check ADN is accessible or not. */
        if ((mFh != null) && (mCurrentApp.getType() == AppType.APPTYPE_SIM)) {
            synchronized (mLock) {
                Message response = obtainMessage(EVENT_SELECT_EF_FILE_DONE);

                mFh.selectEFFile(IccConstants.EF_ADN, response);
                try {
                    mLock.wait();
                } catch (InterruptedException e) {
                    Rlog.e(LOG_TAG, "Interrupted Exception in isAdnAccessible");
                }
            }

            if (mEfData != null) {
                int fs = mEfData.getFileStatus();
                /*
                    b1=0: invalidated
                    b1=1: not invalidated

                    b3=0: not readable or updatable when invalidated
                    b3=1: readable and updatable when invalidated
                */
                if ((fs & 0x05) > 0) {
                    return true;
                } else {
                    return false;
                }
            }
        }
        return true;
    }

    /**
     * Check if need reset ADN cache when receive SIM refresh with file update.
     * @return true for need reset
     */
    public boolean isUsimPhbEfAndNeedReset(int fileId) {
        logi("isUsimPhbEfAndNeedReset, fileId: " + Integer.toHexString(fileId).toUpperCase());

        if (mPbrRecords == null) {
            Rlog.e(LOG_TAG, "isUsimPhbEfAndNeedReset, No PBR files");
            return false;
        }

        int numRecs = mPbrRecords.size();

        for (int i = 0; i < numRecs; i++) {
            SparseArray<File> files = mPbrRecords.get(i).mFileIds;
            for (int j = USIM_EFADN_TAG; j <= USIM_EFCCP1_TAG; j++) {
                if ((j == USIM_EFPBC_TAG) || (j == USIM_EFUID_TAG) || (j == USIM_EFCCP1_TAG)) {
                    // Modem will not reset PHB with these EFs.
                    logi("isUsimPhbEfAndNeedReset, not reset EF: " + j);
                    continue;
                } else if (files.get(j) != null && (fileId == files.get(j).getEfid())) {
                    logi("isUsimPhbEfAndNeedReset, return true with EF: " + j);
                    return true;
                }
            }
        }

        log("isUsimPhbEfAndNeedReset, return false.");
        return false;
    }

    private void readAdnFileAndWaitForUICC(int recId) {
        logi("readAdnFileAndWaitForUICC begin" + recId);

        if (mPbrRecords == null || mPbrRecords.size() == 0) {
            return;
        }

        SparseArray<File> files;
        files = mPbrRecords.get(recId).mFileIds;

        if (files == null || files.size() == 0) return;

        if (files.get(USIM_EFADN_TAG) == null){
            Rlog.e(LOG_TAG, "readAdnFileAndWaitForUICC: No ADN tag in pbr record " + recId);
            return;
        }

        int efid = files.get(USIM_EFADN_TAG).getEfid();

        log("readAdnFileAndWaitForUICC: EFADN id is " + efid);
        log("UiccPhoneBookManager readAdnFileAndWaitForUICC: recId is " + recId + "");
        //change extensionEfForEf for L0 3g merge
        mAdnCache.requestLoadAllAdnLike(efid,
            mAdnCache.extensionEfForEf(IccConstants.EF_ADN),
                                        obtainMessage(EVENT_USIM_ADN_LOAD_DONE));
        try {
            mLock.wait();
        } catch (InterruptedException e) {
            Rlog.e(LOG_TAG, "Interrupted Exception in readAdnFileAndWait");
        }

        int previousSize = mPhoneBookRecords.size();
        /**
         * The recent added ADN record # would be the reference record size
         * for the rest of EFs associated within this PBR.
         */
        if (mPbrRecords != null && mPbrRecords.size() > recId) {
            mPbrRecords.get(recId).mMasterFileRecordNum = mPhoneBookRecords.size() - previousSize;
        }
        logi("readAdnFileAndWaitForUICC end" + recId);
    }

    private ArrayList<MtkAdnRecord> changeAdnRecordNumber(int baseNumber,
            ArrayList<MtkAdnRecord> adnList) {
        int size = adnList.size();
        int i = 0;
        MtkAdnRecord adnRecord;

        for (i = 0; i < size; i++) {
            adnRecord = adnList.get(i);
            if (adnRecord != null) {
                adnRecord.setRecordIndex(adnRecord.getRecId() + baseNumber);
            }
        }
        return adnList;
    }

}
