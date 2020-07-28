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
import android.util.SparseArray;

import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.uicc.AdnRecordCache;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.UiccCardApplication;

import com.mediatek.internal.telephony.uicc.MtkUiccCardApplication;

import com.mediatek.internal.telephony.phb.CsimPhbUtil;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * {@hide}
 */
public class MtkAdnRecordCache extends AdnRecordCache {
    private static final String LOG_TAG = "MtkAdnRecordCache";
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean DBG =
            SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1 &&
            !SystemProperties.get("ro.build.type").equals("user");
    private MtkUsimPhoneBookManager mUsimPhoneBookManager;

    //***** Instance Variables
    private CommandsInterface mCi;
    private UiccCardApplication mCurrentApp;
    private int mSlotId = -1;

    // Indexed by EF ID
    protected SparseArray<ArrayList<MtkAdnRecord>> mMtkAdnLikeFiles
        = new SparseArray<ArrayList<MtkAdnRecord>>();

    public static final int MAX_PHB_NAME_LENGTH = 60;
    public static final int MAX_PHB_NUMBER_LENGTH = 40;
    public static final int MAX_PHB_NUMBER_ANR_LENGTH = 20;
    public static final int MAX_PHB_NUMBER_ANR_COUNT = 1;  // Max is 3, support 1 currently.

    private final Object mLock = new Object();
    private boolean mSuccess = false;
    private boolean mNeedToWait = false;

    private static final int ADN_FILE_SIZE = 250;

    //***** Constructor
    public MtkAdnRecordCache(IccFileHandler fh, CommandsInterface ci, UiccCardApplication app) {
        super(fh);
        mCi = ci;
        mCurrentApp = app;
        mUsimPhoneBookManager = new MtkUsimPhoneBookManager(mFh, this, ci, app);
        if (app != null) mSlotId = ((MtkUiccCardApplication) app).getPhoneId();
    }

    //***** Called from SIMRecords

    /**
     * Called from SIMRecords.onRadioNotAvailable and SIMRecords.handleSimRefresh.
     */
    public void reset() {
        logi("reset");
        mMtkAdnLikeFiles.clear();
        mUsimPhoneBookManager.reset();

        clearWaiters();
        clearUserWriters();

        /// M: CSIM PHB handling @{
        if (!CsimPhbUtil.hasModemPhbEnhanceCapability(mFh)) {
            CsimPhbUtil.clearAdnRecordSize();
        }
        /// @}
    }

    public int getSlotId() {
        return mSlotId;
    }

    private void clearUserWriters() {
        logi("clearUserWriters");
        synchronized (mLock) {
            logi("mNeedToWait " + mNeedToWait);
            if (mNeedToWait) {
                mNeedToWait = false;
                mLock.notifyAll();
            }
        }
        int size = mUserWriteResponse.size();
        for (int i = 0; i < size; i++) {
            sendErrorResponse(mUserWriteResponse.valueAt(i), "AdnCace reset "
                    + mUserWriteResponse.valueAt(i));
        }
        mUserWriteResponse.clear();
    }


    private void sendErrorResponse(Message response, String errString) {
        sendErrorResponse(
                response,
                errString,
                RILConstants.GENERIC_FAILURE);
    }

    private void sendErrorResponse(Message response, String errString, int ril_errno) {

        CommandException e = CommandException.fromRilErrno(ril_errno);

        if (response != null) {
            logw(errString);
            AsyncResult.forMessage(response).exception = e;
            response.sendToTarget();
        }
    }

    /**
     * Update an ADN-like record in EF by record index.
     *
     * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
     * @param adn is the new adn to be stored
     * @param recordIndex is the 1-based adn record index
     * @param pin2 is required to update EF_FDN, otherwise must be null
     * @param response message to be posted when done
     *        response.exception hold the exception in error
     */
    public synchronized void updateAdnByIndex(int efid, MtkAdnRecord adn, int recordIndex,
            String pin2, Message response) {
        logd("updateAdnByIndex efid:" + efid + ", recordIndex:" + recordIndex
                + ", adn [" + adn + "]");
        int extensionEF = extensionEfForEf(efid);
        int i = 0;
        String anr = null;

        if (extensionEF < 0) {
            sendErrorResponse(response, "EF is not known ADN-like EF:0x" +
                    Integer.toHexString(efid).toUpperCase());
            return;
        }
        // MTK-START
        if (null == adn.mAlphaTag) {
            adn.mAlphaTag = "";
        }
        if (adn.mAlphaTag.length() > MAX_PHB_NAME_LENGTH) {

            sendErrorResponse(
                    response,
                    "the input length of mAlphaTag is too long: " + adn.mAlphaTag,
                    RILConstants.OEM_ERROR_2);
            return;
        }
        if (null == adn.mNumber) {
            adn.mNumber = "";
        }
        int numLength = adn.mNumber.length();
        if (adn.mNumber.indexOf('+') != -1) {
            numLength--;
        }

        if (numLength > MAX_PHB_NUMBER_LENGTH) {
            sendErrorResponse(
                    response,
                    "the input length of phoneNumber is too long: " + adn.mNumber,
                    RILConstants.OEM_ERROR_1);

            return;
        }

        for (i = 0; i < MAX_PHB_NUMBER_ANR_COUNT; i++) {
            anr = adn.getAdditionalNumber(i);
            if (anr != null) {
                numLength = anr.length();
                if (anr.indexOf('+') != -1) {
                    numLength--;
                }

                if (numLength > MAX_PHB_NUMBER_ANR_LENGTH) {
                    sendErrorResponse(
                            response,
                            "the input length of additional number is too long: "
                                + anr,
                            RILConstants.OEM_ERROR_5);
                    return;
                }
            }
        }

        if (!mUsimPhoneBookManager.checkEmailLength(adn.mEmails)) {
            sendErrorResponse(
                    response,
                    "the email string is too long",
                    RILConstants.OEM_ERROR_9);
            return;
        }

        //add for uicc card start. the efid is EF_PBR if only it is uicc card.
        ArrayList<MtkAdnRecord>  oldAdnList;
        MtkAdnRecord foundAdn = null;

        if (efid == IccConstants.EF_PBR) {
            oldAdnList = mUsimPhoneBookManager.loadEfFilesFromUsim(null);

            if (oldAdnList == null) {
                sendErrorResponse(
                        response,
                        "Adn list not exist for EF:" + efid,
                        RILConstants.OEM_ERROR_7);
                return;
            }

            foundAdn = oldAdnList.get(recordIndex - 1);
            efid = foundAdn.mEfid; //change into adn file id
            extensionEF = foundAdn.mExtRecord;

            adn.mEfid = efid; //update efid in adn
        }
        //add for uicc card end.

        if (!mUsimPhoneBookManager.checkEmailCapacityFree(recordIndex, adn.mEmails, foundAdn)) {
            sendErrorResponse(
                    response,
                    "drop the email for the limitation of the SIM card",
                    RILConstants.OEM_ERROR_8);
            return;
        }

        for (i = 0; i < MAX_PHB_NUMBER_ANR_COUNT; i++) {
            anr = adn.getAdditionalNumber(i);
            if (!mUsimPhoneBookManager.isAnrCapacityFree(anr, recordIndex, i, foundAdn)) {
                sendErrorResponse(
                        response,
                        "drop the additional number for the update fail: " + anr,
                        RILConstants.OEM_ERROR_6);
                return;
            }
        }

        if (!mUsimPhoneBookManager.checkSneCapacityFree(recordIndex, adn.mSne, foundAdn)) {
            sendErrorResponse(
                    response,
                    "drop the sne for the limitation of the SIM card",
                    RILConstants.OEM_ERROR_10);
            return;
        }

        // MTK-END
        Message pendingResponse = mUserWriteResponse.get(efid);
        if (pendingResponse != null) {
            sendErrorResponse(response, "Have pending update for EF:0x" +
                    Integer.toHexString(efid).toUpperCase());
            return;
        }

        mUserWriteResponse.put(efid, response);
        if (efid == IccConstants.EF_ADN || efid == IccConstants.EF_PBR || efid == 0x4F3A
                || efid == 0x4F3B || efid == 0x4F3C || efid == 0x4F3D) {
            if (adn.mAlphaTag.length() == 0 && adn.mNumber.length() == 0) {
                // delete the group info
                mUsimPhoneBookManager.removeContactGroup(recordIndex);
            }
        }

        // Add for ALPS02528193, during run removeContactGroup and get AdnRecordCache reset.
        if (mUserWriteResponse.size() == 0) {
            return;
        }

        synchronized (mLock) {
            mSuccess = false;
            mNeedToWait = true;

            new MtkAdnRecordLoader(mFh).updateEF(adn, efid, extensionEF,
                recordIndex, pin2,
                obtainMessage(EVENT_UPDATE_ADN_DONE, efid, recordIndex, adn));
            // MTK-START

            try {
                while (mNeedToWait) {
                    mLock.wait();
                }
            } catch (InterruptedException e) {
                return;
            }
        }
        if (!mSuccess) {
            return;
        }
        // update anr/grpIds/emails if necessary
        if (efid == IccConstants.EF_ADN || efid == IccConstants.EF_PBR ||
                efid == 0x4F3A || efid == 0x4F3B || efid == 0x4F3C || efid == 0x4F3D) {
            try {
                int mResult = mUsimPhoneBookManager.updateSneByAdnIndex(adn.mSne,
                        recordIndex, foundAdn);
                if (MtkUsimPhoneBookManager.USIM_ERROR_CAPACITY_FULL == mResult) {
                    sendErrorResponse(
                            response,
                            "drop the SNE for the limitation of the SIM card",
                            RILConstants.OEM_ERROR_10);
                } else if (MtkUsimPhoneBookManager.USIM_ERROR_STRING_TOOLONG == mResult) {
                    sendErrorResponse(
                            response,
                            "the sne string is too long",
                            RILConstants.OEM_ERROR_11);
                } else {
                    for (i = 0; i < MAX_PHB_NUMBER_ANR_COUNT; i++) {
                        anr = adn.getAdditionalNumber(i);
                        mUsimPhoneBookManager.updateAnrByAdnIndex(anr, recordIndex, i, foundAdn);
                    }
                    int success = mUsimPhoneBookManager.
                            updateEmailsByAdnIndex(adn.mEmails, recordIndex, foundAdn);
                    if (MtkUsimPhoneBookManager.USIM_ERROR_CAPACITY_FULL == success) {
                        sendErrorResponse(
                                response,
                                "drop the email for the limitation of the SIM card",
                                RILConstants.OEM_ERROR_8);
                    } else if (MtkUsimPhoneBookManager.USIM_ERROR_STRING_TOOLONG == success) {
                        sendErrorResponse(
                                response,
                                "the email string is too long",
                                RILConstants.OEM_ERROR_9);
                    } else if (MtkUsimPhoneBookManager.USIM_ERROR_OTHERS == success) {
                        sendErrorResponse(
                                response,
                                "Unkown error occurs when update email",
                                RILConstants.GENERIC_FAILURE);
                    } else {
                        AsyncResult.forMessage(response, null, null);
                        response.sendToTarget();
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
                // Rlog.e(LOG_TAG, "exception occured when update anr and email "
                // + e);
                return;
            }
        } else if (efid == IccConstants.EF_FDN) {
            AsyncResult.forMessage(response, null, null);
            response.sendToTarget();
        }
        // MTK-END
    }

    /**
     * Replace oldAdn with newAdn in ADN-like record in EF.
     *
     * The ADN-like records must be read through requestLoadAllAdnLike() before
     *
     * @param efid must be one of EF_ADN, EF_FDN, and EF_SDN
     * @param oldAdn is the adn to be replaced
     *        If oldAdn.isEmpty() is ture, it insert the newAdn
     * @param newAdn is the adn to be stored
     *        If newAdn.isEmpty() is true, it delete the oldAdn
     * @param pin2 is required to update EF_FDN, otherwise must be null
     * @param response message to be posted when done
     *        response.exception hold the exception in error
     * @param object no use currently
     * @return index > 0 for success
     */
    public synchronized int updateAdnBySearch(int efid, MtkAdnRecord oldAdn, MtkAdnRecord newAdn,
            String pin2, Message response, Object object) {
        logd("updateAdnBySearch efid:" + efid + ", oldAdn [" + oldAdn
                + "], new Adn[" + newAdn + "]");
        int index = -1;
        int extensionEF;
        int i = 0;
        String anr = null;
        extensionEF = extensionEfForEf(efid);

        if (extensionEF < 0) {
            sendErrorResponse(response, "EF is not known ADN-like EF:0x" +
                    Integer.toHexString(efid).toUpperCase());
            return index;
        }
        // MTK-START
        if (null == newAdn.mAlphaTag) {
            newAdn.mAlphaTag = "";
        }
        if (newAdn.mAlphaTag.length() > MAX_PHB_NAME_LENGTH) {

            sendErrorResponse(
                    response,
                    "the input length of mAlphaTag is too long: " + newAdn.mAlphaTag,
                    RILConstants.OEM_ERROR_2);
            return index;
        }
        if (null == newAdn.mNumber) {
            newAdn.mNumber = "";
        }
        int numLength = newAdn.mNumber.length();
        if (newAdn.mNumber.indexOf('+') != -1) {
            numLength--;
        }

        if (numLength > MAX_PHB_NUMBER_LENGTH) {

            sendErrorResponse(
                    response,
                    "the input length of phoneNumber is too long: " + newAdn.mNumber,
                    RILConstants.OEM_ERROR_1);

            return index;
        }

        for (i = 0; i < MAX_PHB_NUMBER_ANR_COUNT; i++) {
            anr = newAdn.getAdditionalNumber(i);
            if (anr != null) {
                numLength = anr.length();
                if (anr.indexOf('+') != -1) {
                numLength--;
                }

                if (numLength > MAX_PHB_NUMBER_ANR_LENGTH) {
                    sendErrorResponse(
                            response,
                            "the input length of additional number is too long: "
                                + anr,
                            RILConstants.OEM_ERROR_5);
                    return index;
                }
            }
        }
        // MTK-END

        if (!mUsimPhoneBookManager.checkEmailLength(newAdn.mEmails)) {
            sendErrorResponse(
                    response,
                    "the email string is too long",
                    RILConstants.OEM_ERROR_9);
            return index;
        }

        ArrayList<MtkAdnRecord>  oldAdnList;

        if (efid == EF_PBR) {
            oldAdnList = mUsimPhoneBookManager.loadEfFilesFromUsim(null);
        } else {
            oldAdnList = getRecordsIfLoaded(efid, null);
        }

        if (oldAdnList == null) {
            sendErrorResponse(
                    response,
                    "Adn list not exist for EF:" + efid,
                    RILConstants.OEM_ERROR_7);
            return index;
        }

        /* Move to upper for more error handle.
        int index = -1;
        */
        int count = 1;
        for (Iterator<MtkAdnRecord> it = oldAdnList.iterator(); it.hasNext(); ) {
            if (oldAdn.isEqual(it.next())) {
                index = count;
                break;
            }
            count++;
        }
        logi("updateAdnBySearch index " + index);
        if (index == -1) {
            if (oldAdn.mAlphaTag.length() == 0 && oldAdn.mNumber.length() == 0) {
                sendErrorResponse(
                        response,
                        "Adn record don't exist for " + oldAdn,
                        RILConstants.OEM_ERROR_3);
            } else {
                sendErrorResponse(response, "Adn record don't exist for " + oldAdn);
            }
            return index;
        }

        MtkAdnRecord foundAdn = null;
        if (efid == EF_PBR) {
            //if the index is more than 250, the the adn is belong to the file which EFid is 4F3B
            foundAdn = oldAdnList.get(index - 1);
            efid = foundAdn.mEfid;
            extensionEF = foundAdn.mExtRecord;
            index = foundAdn.mRecordNumber; //so we change the corrected index. ex the index is 251,
                                            //then it will be the first adn in the file 4F3B,
                                            //so change it into mRecordNumber which is 1.

            newAdn.mEfid = efid;
            newAdn.mExtRecord = extensionEF;
            newAdn.mRecordNumber = index;
        }

        Message pendingResponse = mUserWriteResponse.get(efid);

        if (pendingResponse != null) {
            sendErrorResponse(response, "Have pending update for EF:0x" +
                    Integer.toHexString(efid).toUpperCase());
            return index;
        }
        if (0 == efid) {
            sendErrorResponse(response, "Abnormal efid: " + efid);
            return index;
        }
        if (!mUsimPhoneBookManager.checkEmailCapacityFree(index, newAdn.mEmails, foundAdn)) {
            sendErrorResponse(
                    response,
                    "drop the email for the limitation of the SIM card",
                    RILConstants.OEM_ERROR_8);
            return index;
        }
        for (i = 0; i < MAX_PHB_NUMBER_ANR_COUNT; i++) {
            anr = newAdn.getAdditionalNumber(i);
            if (!mUsimPhoneBookManager.isAnrCapacityFree(anr, index, i, foundAdn)) {
                sendErrorResponse(
                        response,
                        "drop the additional number for the write fail: " + anr,
                        RILConstants.OEM_ERROR_6);
                return index;
            }
        }

        if (!mUsimPhoneBookManager.checkSneCapacityFree(index, newAdn.mSne, foundAdn)) {
            sendErrorResponse(
                    response,
                    "drop the sne for the limitation of the SIM card",
                    RILConstants.OEM_ERROR_10);
            return index;
        }

        mUserWriteResponse.put(efid, response);

        synchronized (mLock) {
            mSuccess = false;
            mNeedToWait = true;

            new MtkAdnRecordLoader(mFh).updateEF(newAdn, efid, extensionEF,
                index, pin2,
                obtainMessage(EVENT_UPDATE_ADN_DONE, efid, index, newAdn));

            // MTK-START

            try {
                while (mNeedToWait) {
                    mLock.wait();
                }
            } catch (InterruptedException e) {
                return index;
            }
        }
        if (!mSuccess) {
            loge("updateAdnBySearch mSuccess:" + mSuccess);
            return index;
        }
        int success = 0;
        if (efid == EF_ADN || efid == EF_PBR || efid == 0x4F3A || efid == 0x4F3B ||
                efid == 0x4F3C || efid == 0x4F3D) {
            int mResult = mUsimPhoneBookManager.updateSneByAdnIndex(newAdn.mSne, index, foundAdn);
            if (MtkUsimPhoneBookManager.USIM_ERROR_CAPACITY_FULL == mResult) {
                sendErrorResponse(
                        response,
                        "drop the SNE for the limitation of the SIM card",
                        RILConstants.OEM_ERROR_10);
            } else if (MtkUsimPhoneBookManager.USIM_ERROR_STRING_TOOLONG == mResult) {
                sendErrorResponse(
                        response,
                        "the sne string is too long",
                        RILConstants.OEM_ERROR_11);
            } else {
                for (i = 0; i < MAX_PHB_NUMBER_ANR_COUNT; i++) {
                    anr = newAdn.getAdditionalNumber(i);
                    mUsimPhoneBookManager.updateAnrByAdnIndex(anr, index, i, foundAdn);
                }
                success = mUsimPhoneBookManager.updateEmailsByAdnIndex(newAdn.mEmails,
                        index, foundAdn);

                if (MtkUsimPhoneBookManager.USIM_ERROR_CAPACITY_FULL == success) {
                    sendErrorResponse(response,
                            "drop the email for the limitation of the SIM card",
                            RILConstants.OEM_ERROR_8);
                } else if (MtkUsimPhoneBookManager.USIM_ERROR_STRING_TOOLONG == success) {
                    sendErrorResponse(
                            response,
                            "the email string is too long",
                            RILConstants.OEM_ERROR_9);
                } else if (MtkUsimPhoneBookManager.USIM_ERROR_OTHERS == success) {
                    sendErrorResponse(
                            response,
                            "Unkown error occurs when update email",
                            RILConstants.GENERIC_FAILURE);
                } else {
                    logd("updateAdnBySearch response:" + response);
                    AsyncResult.forMessage(response, null, null);
                    response.sendToTarget();
                }
            }
        } else if (efid == IccConstants.EF_FDN) {
            logd("updateAdnBySearch FDN response:" + response);
            AsyncResult.forMessage(response, null, null);
            response.sendToTarget();
        }
        return index;
        // MTK-END
    }

    /**
     * Responds with exception(in response) if efid is not a known ADN-like record.
     */
    public void
    requestLoadAllAdnLike(int efid, int extensionEf, Message response) {
        ArrayList<Message> waiters;
        ArrayList<MtkAdnRecord> result;
        logd("requestLoadAllAdnLike efid = " + efid + ", extensionEf = " + extensionEf);
        if (efid == EF_PBR) {
            result = mUsimPhoneBookManager.loadEfFilesFromUsim(null);
        } else {
            result = getRecordsIfLoaded(efid, null);
        }
        logi("requestLoadAllAdnLike efid = " + efid + ", result = null ?" + (result == null));

        // Have we already loaded this efid?
        if (result != null) {
            if (response != null) {
                AsyncResult.forMessage(response).result = result;
                response.sendToTarget();
            }

            return;
        } else if (result == null && efid == EF_PBR) {
            sendErrorResponse(
                    response,
                    "Error occurs when query PBR",
                    RILConstants.GENERIC_FAILURE);
            return;
        }

        // Have we already *started* loading this efid?

        waiters = mAdnLikeWaiters.get(efid);

        if (waiters != null) {
            // There's a pending request for this EF already
            // just add ourselves to it

            waiters.add(response);
            return;
        }

        // Start loading efid

        waiters = new ArrayList<Message>();
        waiters.add(response);

        mAdnLikeWaiters.put(efid, waiters);

        if (extensionEf < 0) {
            // respond with error if not known ADN-like record

            if (response != null) {
                AsyncResult.forMessage(response).exception
                    = new RuntimeException("EF is not known ADN-like EF:0x" +
                        Integer.toHexString(efid).toUpperCase());
                response.sendToTarget();
            }

            return;
        }

        new MtkAdnRecordLoader(mFh).loadAllFromEF(efid, extensionEf,
            obtainMessage(EVENT_LOAD_ALL_ADN_LIKE_DONE, efid, 0));
    }

    //***** Private methods

    protected void
    notifyWaiters(ArrayList<Message> waiters, AsyncResult ar) {

        if (waiters == null) {
            return;
        }

        for (int i = 0, s = waiters.size() ; i < s ; i++) {
            Message waiter = waiters.get(i);

            if (waiter != null) {
                logi("NotifyWaiters: " + waiter);
                AsyncResult.forMessage(waiter, ar.result, ar.exception);
                waiter.sendToTarget();
            }
        }
    }

    /**
     * @return List of AdnRecords for efid if we've already loaded them this
     * radio session, or null if we haven't
     */
    public ArrayList<MtkAdnRecord>
    getRecordsIfLoaded(int efid, Object object) {
        return mMtkAdnLikeFiles.get(efid);
    }

    //***** Overridden from Handler

    @Override
    public void
    handleMessage(Message msg) {
        AsyncResult ar;
        int efid;

        switch (msg.what) {
            case EVENT_LOAD_ALL_ADN_LIKE_DONE:
                /* arg1 is efid, obj.result is ArrayList<MtkAdnRecord>*/
                ar = (AsyncResult) msg.obj;
                efid = msg.arg1;
                ArrayList<Message> waiters;

                waiters = mAdnLikeWaiters.get(efid);
                mAdnLikeWaiters.delete(efid);

                if (ar.exception == null) {
                    mMtkAdnLikeFiles.put(efid, (ArrayList<MtkAdnRecord>) ar.result);
                } else {
                    Rlog.w(LOG_TAG, "EVENT_LOAD_ALL_ADN_LIKE_DONE exception"
                            + "(slot " + mSlotId + ")", ar.exception);
                }
                notifyWaiters(waiters, ar);
                break;
            case EVENT_UPDATE_ADN_DONE:
                logd("EVENT_UPDATE_ADN_DONE");
                synchronized (mLock) {
                    if (mNeedToWait) {
                        ar = (AsyncResult) msg.obj;
                        efid = msg.arg1;
                        int index = msg.arg2;
                        MtkAdnRecord adn = (MtkAdnRecord) (ar.userObj);

                        if (ar.exception == null && adn != null) {

                            adn.setRecordIndex(index);
                            if (adn.mEfid <= 0) {
                                adn.mEfid = efid;
                            }

                            logd("mMtkAdnLikeFiles changed index:" + index + ",adn:" + adn +
                                    "  efid:" + efid);

                            if (null != mMtkAdnLikeFiles && null != mMtkAdnLikeFiles.get(efid)) {
                                /// M: CSIM PHB handling @{
                                if (efid == 0x4F3B &&
                                        !CsimPhbUtil.hasModemPhbEnhanceCapability(mFh)) {
                                    index -= ADN_FILE_SIZE;
                                }
                                /// @}
                                mMtkAdnLikeFiles.get(efid).set(index - 1, adn);
                                logd(" index:" + index + "   efid:"
                                        + efid);
                            }
                            if ((null != mUsimPhoneBookManager) && (efid != IccConstants.EF_FDN)) {
                                if (efid == 0x4F3B) {
                                    // the value of index in
                                    // mUsimPhoneBookManager is 250+index, the
                                    // 1~250 records come form 0x4F3A
                                    index += ADN_FILE_SIZE;
                                    logd(" index2:" + index);

                                }
                                mUsimPhoneBookManager
                                        .updateUsimPhonebookRecordsList(index - 1, adn);
                            }
                        }

                        Message response = mUserWriteResponse.get(efid);
                        mUserWriteResponse.delete(efid);

                        logi("MtkAdnRecordCache: " + ar.exception);

                        if (ar.exception != null && response != null) {
                            AsyncResult.forMessage(response, null, ar.exception);
                            response.sendToTarget();
                        }
                        mSuccess = ar.exception == null;
                        mNeedToWait = false;
                        mLock.notifyAll();
                    }
                }
                break;
            default:
                break;
        }
    }

    protected void logd(String msg) {
        if (DBG) Rlog.d(LOG_TAG, msg + "(slot " + mSlotId + ")");
    }

    protected void loge(String msg) {
        Rlog.e(LOG_TAG, msg + "(slot " + mSlotId + ")");
    }

    protected void logi(String msg) {
        Rlog.i(LOG_TAG, msg + "(slot " + mSlotId + ")");
    }

    protected void logw(String msg) {
        Rlog.w(LOG_TAG, msg + "(slot " + mSlotId + ")");
    }

    public List<UsimGroup> getUsimGroups() {
        return mUsimPhoneBookManager.getUsimGroups();
    }

    public String getUsimGroupById(int nGasId) {
        return mUsimPhoneBookManager.getUsimGroupById(nGasId);
    }

    public boolean removeUsimGroupById(int nGasId) {
        return mUsimPhoneBookManager.removeUsimGroupById(nGasId);
    }

    public int insertUsimGroup(String grpName) {
        return mUsimPhoneBookManager.insertUsimGroup(grpName);
    }

    public int updateUsimGroup(int nGasId, String grpName) {
        return mUsimPhoneBookManager.updateUsimGroup(nGasId, grpName);
    }

    public boolean addContactToGroup(int adnIndex, int grpIndex) {
        return mUsimPhoneBookManager.addContactToGroup(adnIndex, grpIndex);
    }

    public boolean removeContactFromGroup(int adnIndex, int grpIndex) {
        return mUsimPhoneBookManager.removeContactFromGroup(adnIndex, grpIndex);
    }

    public boolean updateContactToGroups(int adnIndex, int[] grpIdList) {
        return mUsimPhoneBookManager.updateContactToGroups(adnIndex, grpIdList);
    }

    public boolean moveContactFromGroupsToGroups(int adnIndex, int[] fromGrpIdList,
                                                 int[] toGrpIdList) {
        return mUsimPhoneBookManager.moveContactFromGroupsToGroups(adnIndex, fromGrpIdList,
                                                                   toGrpIdList);
    }

    public int hasExistGroup(String grpName) {
        return mUsimPhoneBookManager.hasExistGroup(grpName);
    }

    public int getUsimGrpMaxNameLen() {
        return mUsimPhoneBookManager.getUsimGrpMaxNameLen();
    }

    public int getUsimGrpMaxCount() {
        return mUsimPhoneBookManager.getUsimGrpMaxCount();
    }

    // currently not used, for debug.
    private void dumpAdnLikeFile() {
        int size = mMtkAdnLikeFiles.size();
        logd("dumpAdnLikeFile size " + size);
        int key;
        for (int i = 0; i < size; i++) {
            key = mMtkAdnLikeFiles.keyAt(i);

            ArrayList<MtkAdnRecord> records = mMtkAdnLikeFiles.get(key);
            logd("dumpAdnLikeFile index " + i + " key " + key + "records size " + records.size());
            for (int j = 0; j < records.size(); j++) {
                MtkAdnRecord record = records.get(j);
                logd("mMtkAdnLikeFiles[" + j + "]=" + record);
            }
        }
    }

    public ArrayList<AlphaTag> getUsimAasList() {
        return mUsimPhoneBookManager.getUsimAasList();
    }

    public String getUsimAasById(int index) {
        // TODO
        return mUsimPhoneBookManager.getUsimAasById(index, 0);
    }

    public boolean removeUsimAasById(int index, int pbrIndex) {
        return mUsimPhoneBookManager.removeUsimAasById(index, pbrIndex);
    }

    public int insertUsimAas(String aasName) {
        return mUsimPhoneBookManager.insertUsimAas(aasName);
    }

    public boolean updateUsimAas(int index, int pbrIndex, String aasName) {
        return mUsimPhoneBookManager.updateUsimAas(index, pbrIndex, aasName);
    }

    /**
     * @param adnIndex ADN index
     * @param aasIndex change AAS to the value refered by aasIndex, -1 means
     *            remove
     * @return true for update success
     */
    public boolean updateAdnAas(int adnIndex, int aasIndex) {
        return mUsimPhoneBookManager.updateAdnAas(adnIndex, aasIndex);
    }

    public int getAnrCount() {
        return mUsimPhoneBookManager.getAnrCount();
    }

    public int getEmailCount() {
        return mUsimPhoneBookManager.getEmailCount();
    }

    public int getUsimAasMaxCount() {
        return mUsimPhoneBookManager.getUsimAasMaxCount();
    }

    public int getUsimAasMaxNameLen() {
        return mUsimPhoneBookManager.getUsimAasMaxNameLen();
    }

    public boolean hasSne() {
        return mUsimPhoneBookManager.hasSne();
    }

    public int getSneRecordLen() {
        return mUsimPhoneBookManager.getSneRecordLen();
    }

    public boolean isAdnAccessible() {
        return mUsimPhoneBookManager.isAdnAccessible();
    }

    public boolean isUsimPhbEfAndNeedReset(int fileId) {
        return mUsimPhoneBookManager.isUsimPhbEfAndNeedReset(fileId);
    }

    public UsimPBMemInfo[] getPhonebookMemStorageExt() {
        return mUsimPhoneBookManager.getPhonebookMemStorageExt();
    }

    public int getUpbDone() {
        return mUsimPhoneBookManager.getUpbDone();
    }

    public int[] getAdnRecordsCapacity() {
        return mUsimPhoneBookManager.getAdnRecordsCapacity();
    }
}
