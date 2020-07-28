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

import android.content.pm.PackageManager;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;

import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.IccInternalInterface;
import com.android.internal.telephony.IccPhoneBookInterfaceManager;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.uicc.AdnRecordCache;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppType;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccRecords;

import com.mediatek.internal.telephony.uicc.MtkRuimRecords;
import com.mediatek.internal.telephony.uicc.MtkSIMRecords;

import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * SimPhoneBookInterfaceManager to provide an inter-process communication to
 * access ADN-like SIM records.
 */
public class MtkIccPhoneBookInterfaceManager extends IccPhoneBookInterfaceManager {
    static final String LOG_TAG = "MtkIccPhoneBookIM";
    protected static final boolean DBG = !SystemProperties.get("ro.build.type").equals("user");


    private int mErrorCause;
    private int mSlotId = -1;
    private List<MtkAdnRecord> mRecords;
    private IccRecords mIccRecords;

    public MtkIccPhoneBookInterfaceManager(Phone phone) {
        super(phone);
    }

    protected Handler mMtkBaseHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            IccPhoneBookInterfaceManager.Request request
                    = (IccPhoneBookInterfaceManager.Request) ar.userObj;

            switch (msg.what) {
                case EVENT_GET_SIZE_DONE:
                    mBaseHandler.handleMessage(msg);
                    break;
                case EVENT_UPDATE_DONE:
                    logd("EVENT_UPDATE_DONE");
                    ar = (AsyncResult) msg.obj;
                    boolean success = (ar.exception == null);
                    logd("EVENT_UPDATE_DONE" + " success:" + success);
                    if (!success) {
                        if (ar.exception instanceof CommandException) {
                            mErrorCause = getErrorCauseFromException(
                                    (CommandException) ar.exception);
                        } else {
                            loge("Error : Unknow exception instance");
                            mErrorCause =
                                    IccInternalInterface.ERROR_ICC_PROVIDER_GENERIC_FAILURE;
                        }
                    } else {
                        mErrorCause = IccInternalInterface.ERROR_ICC_PROVIDER_SUCCESS;
                    }
                    logi("update done result: " + mErrorCause);
                    notifyPending(request, success);
                    break;
                case EVENT_LOAD_DONE:
                    List<MtkAdnRecord> records = null;
                    if (ar.exception == null) {
                        records = (List<MtkAdnRecord>) ar.result;
                    } else {
                        loge("EVENT_LOAD_DONE: Cannot load ADN records; ex=" +
                                ar.exception);
                    }
                    notifyPending(request, records);
                    break;
                default:
                    break;
            }
        }

        private void notifyPending(IccPhoneBookInterfaceManager.Request request, Object result) {
            if (request != null) {
                synchronized (request) {
                    request.mResult = result;
                    request.mStatus.set(true);
                    request.notifyAll();
                }
            }
        }
    };


    public void updateIccRecords(IccRecords iccRecords) {
        mIccRecords = iccRecords;
        if (iccRecords != null) {
            mAdnCache = iccRecords.getAdnCache();
            if (mAdnCache != null && mAdnCache instanceof MtkAdnRecordCache) {
                mSlotId = ((MtkAdnRecordCache) mAdnCache).getSlotId();
            } else {
                mSlotId = -1;
            }
            logi("[updateIccRecords] Set mAdnCache value");
        } else {
            mAdnCache = null;
            logi("[updateIccRecords] Set mAdnCache value to null");
            mSlotId = -1;
        }
    }

    protected void logd(String msg) {
        Rlog.d(LOG_TAG, "[IccPbInterfaceManager] " + msg + "(slot " + mSlotId + ")");
    }

    protected void loge(String msg) {
        Rlog.e(LOG_TAG, "[IccPbInterfaceManager] " + msg + "(slot " + mSlotId + ")");
    }

    protected void logi(String msg) {
        Rlog.i(LOG_TAG, "[IccPbInterfaceManager] " + msg + "(slot " + mSlotId + ")");
    }

    /**
     * Replace oldAdn with newAdn in ADN-like record in EF
     *
     * getAdnRecordsInEf must be called at least once before this function,
     * otherwise an error will be returned. Currently the email field
     * if set in the ADN record is ignored.
     * throws SecurityException if no WRITE_CONTACTS permission
     *
     * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
     * @param oldTag adn tag to be replaced
     * @param oldPhoneNumber adn number to be replaced
     *        Set both oldTag and oldPhoneNubmer to "" means to replace an
     *        empty record, aka, insert new record
     * @param newTag adn tag to be stored
     * @param newPhoneNumber adn number to be stored
     *        Set both newTag and newPhoneNubmer to "" means to replace the old
     *        record with empty one, aka, delete old record
     * @param pin2 required to update EF_FDN, otherwise must be null
     * @return true for success
     */
    public boolean
    updateAdnRecordsInEfBySearch(int efid,
            String oldTag, String oldPhoneNumber,
            String newTag, String newPhoneNumber, String pin2) {
        int result;

        result = updateAdnRecordsInEfBySearchWithError(
                efid, oldTag, oldPhoneNumber,
                newTag, newPhoneNumber, pin2);

        return result == IccInternalInterface.ERROR_ICC_PROVIDER_SUCCESS;
    }

    /**
     * Replace oldAdn with newAdn in ADN-like record in EF
     *
     * getAdnRecordsInEf must be called at least once before this function,
     * otherwise an error will be returned. Currently the email field
     * if set in the ADN record is ignored.
     * throws SecurityException if no WRITE_CONTACTS permission
     * This method will return why the error occurs.
     *
     * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
     * @param oldTag adn tag to be replaced
     * @param oldPhoneNumber adn number to be replaced Set both oldTag and
     *            oldPhoneNubmer to "" means to replace an empty record, aka,
     *            insert new record
     * @param newTag adn tag to be stored
     * @param newPhoneNumber adn number to be stored Set both newTag and
     *            newPhoneNubmer to "" means to replace the old record with
     *            empty one, aka, delete old record
     * @param pin2 required to update EF_FDN, otherwise must be null
     * @return ERROR_ICC_PROVIDER_* defined in the IccInternalInterface
     */
    public synchronized int updateAdnRecordsInEfBySearchWithError(int efid,
            String oldTag, String oldPhoneNumber,
            String newTag, String newPhoneNumber, String pin2) {

        int index = -1;
        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS)
            != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        if (mAdnCache == null) {
            loge("updateAdnRecordsInEfBySearchWithError mAdnCache is null");
            return IccInternalInterface.ERROR_ICC_PROVIDER_UNKNOWN;
        }

        if (DBG) logd("updateAdnRecordsInEfBySearch: efid=0x" +
                Integer.toHexString(efid).toUpperCase());


        efid = updateEfForIccType(efid);
        checkThread();
        IccPhoneBookInterfaceManager.Request updateRequest
                = new IccPhoneBookInterfaceManager.Request();
        synchronized (updateRequest) {
            Message response = mMtkBaseHandler.obtainMessage(EVENT_UPDATE_DONE, updateRequest);
            MtkAdnRecord oldAdn = new MtkAdnRecord(oldTag, oldPhoneNumber);
            if (null == newPhoneNumber) {
                newPhoneNumber = "";
            }
            if (null == newTag) {
                newTag = "";
            }
            MtkAdnRecord newAdn = new MtkAdnRecord(newTag, newPhoneNumber);
            if (mAdnCache != null) {
                index = ((MtkAdnRecordCache) mAdnCache).updateAdnBySearch(
                        efid, oldAdn, newAdn, pin2, response, null);
                waitForResult(updateRequest);
            } else {
                loge("Failure while trying to update by search due to uninitialised adncache");
            }
        }
        if (mErrorCause == IccInternalInterface.ERROR_ICC_PROVIDER_SUCCESS) {
            logi("updateAdnRecordsInEfBySearchWithError success index is " + index);
            return index;
        }
        return mErrorCause;
    }

    /**
     * Replace oldAdn with newAdn in ADN-like record in EF
     *
     * getAdnRecordsInEf must be called at least once before this function,
     * otherwise an error will be returned. Currently the email field
     * if set in the ADN record is ignored.
     * throws SecurityException if no WRITE_CONTACTS permission
     * This method will return why the error occurs.
     *
     * @param efid must be EF_PBR
     * @param oldTag adn tag to be replaced
     * @param oldPhoneNumber adn number to be replaced
     *        Set both oldTag and oldPhoneNubmer to "" means to replace an
     *        empty record, aka, insert new record
     * @param oldAnr additional number to be replaced
     * @param oldGrpIds group ids to be replaced
     * @param oldEmails emails to be replaced
     * @param newTag adn tag to be stored
     * @param newPhoneNumber adn number to be stored
     *        Set both newTag and newPhoneNubmer to "" means to replace the old
     *        record with empty one, aka, delete old record
     * @param newAnr additional number to be stored
     * @param newGrpIds group ids to be stored
     * @param newEmails emails to be stored
     * @return ERROR_ICC_PROVIDER_* defined in the IccInternalInterface
     */
    public synchronized int updateUsimPBRecordsInEfBySearchWithError(int efid,
            String oldTag, String oldPhoneNumber, String oldAnr, String oldGrpIds,
            String[] oldEmails,
            String newTag, String newPhoneNumber, String newAnr, String newGrpIds,
            String[] newEmails) {

        int index = -1;
        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        if (mAdnCache == null) {
            loge("updateUsimPBRecordsInEfBySearchWithError mAdnCache is null");
            return IccInternalInterface.ERROR_ICC_PROVIDER_UNKNOWN;
        }

        if (DBG) {
            logd("updateUsimPBRecordsInEfBySearchWithError: efid=" + efid +
                    "oldAnr" + oldAnr + " oldGrpIds " + oldGrpIds +
                    "==>"  + " newAnr= " + newAnr + " newGrpIds = " + newGrpIds);
        }
        checkThread();
        IccPhoneBookInterfaceManager.Request updateRequest
                = new IccPhoneBookInterfaceManager.Request();
        synchronized (updateRequest) {
            Message response = mMtkBaseHandler.obtainMessage(EVENT_UPDATE_DONE, updateRequest);
            MtkAdnRecord oldAdn = new MtkAdnRecord(oldTag, oldPhoneNumber);
            if (null == newPhoneNumber) {
                newPhoneNumber = "";
            }
            if (null == newTag) {
                newTag = "";
            }
            MtkAdnRecord newAdn = new MtkAdnRecord(0, 0, newTag, newPhoneNumber, newAnr, newEmails,
                    newGrpIds);
            index = ((MtkAdnRecordCache) mAdnCache).updateAdnBySearch(
                    efid, oldAdn, newAdn, null, response, null);
            waitForResult(updateRequest);
        }
        if (mErrorCause == IccInternalInterface.ERROR_ICC_PROVIDER_SUCCESS) {
            logi("updateUsimPBRecordsInEfBySearchWithError success index is " + index);
            return index;
        }
        return mErrorCause;
    }

    /**
     * Replace oldAdn with newAdn in ADN-like record in EF
     *
     * getAdnRecordsInEf must be called at least once before this function,
     * otherwise an error will be returned. Currently the email field
     * if set in the ADN record is ignored.
     * throws SecurityException if no WRITE_CONTACTS permission
     * This method will return why the error occurs.
     *
     * @param efid must be EF_PBR
     * @param oldAdn adn to be replaced with all fields
     * @param newAdn adn to be stored with all fields
     * @return ERROR_ICC_PROVIDER_* defined in the IccInternalInterface
     */
    public synchronized int updateUsimPBRecordsBySearchWithError(int efid, MtkAdnRecord oldAdn,
            MtkAdnRecord newAdn) {
        int index = -1;
        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        if (mAdnCache == null) {
            loge("updateUsimPBRecordsBySearchWithError mAdnCache is null");
            return IccInternalInterface.ERROR_ICC_PROVIDER_UNKNOWN;
        }

        if (DBG) {
            logd("updateUsimPBRecordsBySearchWithError: efid=" + efid +
                    " (" + oldAdn + ")" + "==>" + "(" + newAdn + ")");
        }
        checkThread();
        IccPhoneBookInterfaceManager.Request updateRequest
                = new IccPhoneBookInterfaceManager.Request();
        synchronized (updateRequest) {
            Message response = mMtkBaseHandler.obtainMessage(EVENT_UPDATE_DONE, updateRequest);
            if (newAdn.getNumber() == null) {
                newAdn.setNumber("");
            }
            index = ((MtkAdnRecordCache) mAdnCache).updateAdnBySearch(
                    efid, oldAdn, newAdn, null, response, null);
            waitForResult(updateRequest);
        }
        if (mErrorCause == IccInternalInterface.ERROR_ICC_PROVIDER_SUCCESS) {
            logi("updateUsimPBRecordsBySearchWithError success index is " + index);
            return index;
        }
        return mErrorCause;
    }

    /**
     * Update an ADN-like EF record by record index
     *
     * This is useful for iteration the whole ADN file, such as write the whole
     * phone book or erase/format the whole phonebook. Currently the email field
     * if set in the ADN record is ignored.
     * throws SecurityException if no WRITE_CONTACTS permission
     *
     * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
     * @param newTag adn tag to be stored
     * @param newPhoneNumber adn number to be stored
     *        Set both newTag and newPhoneNubmer to "" means to replace the old
     *        record with empty one, aka, delete old record
     * @param index is 1-based adn record index to be updated
     * @param pin2 required to update EF_FDN, otherwise must be null
     * @return true for success
     */
    public boolean
    updateAdnRecordsInEfByIndex(int efid, String newTag,
            String newPhoneNumber, int index, String pin2) {

        int result;

        result = updateAdnRecordsInEfByIndexWithError(
                efid, newTag,
                newPhoneNumber, index, pin2);

        return result == IccInternalInterface.ERROR_ICC_PROVIDER_SUCCESS;
    }

    /**
     * Update an ADN-like EF record by record index
     *
     * This is useful for iteration the whole ADN file, such as write the whole
     * phone book or erase/format the whole phonebook. Currently the email field
     * if set in the ADN record is ignored.
     * throws SecurityException if no WRITE_CONTACTS permission
     * This method will return why the error occurs.
     *
     * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
     * @param newTag adn tag to be stored
     * @param newPhoneNumber adn number to be stored Set both newTag and
     *            newPhoneNubmer to "" means to replace the old record with
     *            empty one, aka, delete old record
     * @param index is 1-based adn record index to be updated
     * @param pin2 required to update EF_FDN, otherwise must be null
     * @return ERROR_ICC_PROVIDER_* defined in the IccInternalInterface
     */
    public synchronized int updateAdnRecordsInEfByIndexWithError(int efid, String newTag,
            String newPhoneNumber, int index, String pin2) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        if (mAdnCache == null) {
            loge("updateAdnRecordsInEfByIndex mAdnCache is null");
            return IccInternalInterface.ERROR_ICC_PROVIDER_UNKNOWN;
        }
        if (DBG) logd("updateAdnRecordsInEfByIndex: efid=0x" +
                Integer.toHexString(efid).toUpperCase() + " Index=" + index);
        checkThread();
        IccPhoneBookInterfaceManager.Request updateRequest
                = new IccPhoneBookInterfaceManager.Request();
        synchronized (updateRequest) {
            Message response = mMtkBaseHandler.obtainMessage(EVENT_UPDATE_DONE, updateRequest);
            if (null == newPhoneNumber) {
                newPhoneNumber = "";
            }
            if (null == newTag) {
                newTag = "";
            }
            MtkAdnRecord newAdn = new MtkAdnRecord(newTag, newPhoneNumber);
            if (mAdnCache != null) {
                ((MtkAdnRecordCache) mAdnCache).updateAdnByIndex(
                        efid, newAdn, index, pin2, response);
                waitForResult(updateRequest);
            } else {
                loge("Failure while trying to update by index due to uninitialised adncache");
            }
        }
        return mErrorCause;
    }

    /**
     * Update an ADN-like EF record by record index
     *
     * This is useful for iteration the whole ADN file, such as write the whole
     * phone book or erase/format the whole phonebook. Currently the email field
     * if set in the ADN record is ignored.
     * throws SecurityException if no WRITE_CONTACTS permission
     * This method will return why the error occurs.
     *
     * @param efid must be EF_PBR
     * @param newTag adn tag to be stored
     * @param newPhoneNumber adn number to be stored Set both newTag and
     *            newPhoneNubmer to "" means to replace the old record with
     *            empty one, aka, delete old record
     * @param newAnr additional number to be stored
     * @param newGrpIds group ids to be stored
     * @param newEmails emails to be stored
     * @param index is 1-based adn record index to be updated
     * @return ERROR_ICC_PROVIDER_* defined in the IccInternalInterface
     */
    public synchronized int updateUsimPBRecordsInEfByIndexWithError(int efid, String newTag,
            String newPhoneNumber, String newAnr, String newGrpIds, String[] newEmails, int index) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        if (mAdnCache == null) {
            loge("updateUsimPBRecordsInEfByIndexWithError mAdnCache is null");
            return IccInternalInterface.ERROR_ICC_PROVIDER_UNKNOWN;
        }

        if (DBG) {
            logd("updateUsimPBRecordsInEfByIndexWithError: efid=" + efid +
                    " Index=" + index + " ==> " + " newAnr= " + newAnr +
                    " newGrpIds = " + newGrpIds);
        }
        checkThread();
        IccPhoneBookInterfaceManager.Request updateRequest
                = new IccPhoneBookInterfaceManager.Request();
        synchronized (updateRequest) {
            Message response = mMtkBaseHandler.obtainMessage(EVENT_UPDATE_DONE, updateRequest);
            if (null == newPhoneNumber) {
                newPhoneNumber = "";
            }
            if (null == newTag) {
                newTag = "";
            }
            MtkAdnRecord newAdn = new MtkAdnRecord(efid, index, newTag, newPhoneNumber, newAnr,
                    newEmails, newGrpIds);
            ((MtkAdnRecordCache) mAdnCache).updateAdnByIndex(efid, newAdn, index, null, response);
            waitForResult(updateRequest);
        }
        return mErrorCause;
    }

    /**
     * Update an ADN-like EF record by record index
     *
     * This is useful for iteration the whole ADN file, such as write the whole
     * phone book or erase/format the whole phonebook. Currently the email field
     * if set in the ADN record is ignored.
     * throws SecurityException if no WRITE_CONTACTS permission
     * This method will return why the error occurs.
     *
     * @param efid must be EF_PBR
     * @param record adn to be stored with all fields
     * @param index is 1-based adn record index to be updated
     * @return ERROR_ICC_PROVIDER_* defined in the IccInternalInterface
     */
    public synchronized int updateUsimPBRecordsByIndexWithError(int efid, MtkAdnRecord record,
            int index) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        if (mAdnCache == null) {
            loge("updateUsimPBRecordsByIndexWithError mAdnCache is null");
            return IccInternalInterface.ERROR_ICC_PROVIDER_UNKNOWN;
        }

        if (DBG) {
            logd("updateUsimPBRecordsByIndexWithError: efid=" + efid +
                    " Index=" + index + " ==> " + record);
        }
        checkThread();
        IccPhoneBookInterfaceManager.Request updateRequest
                = new IccPhoneBookInterfaceManager.Request();
        synchronized (updateRequest) {
            Message response = mMtkBaseHandler.obtainMessage(EVENT_UPDATE_DONE, updateRequest);
            ((MtkAdnRecordCache) mAdnCache).updateAdnByIndex(efid, record, index, null, response);
            waitForResult(updateRequest);
        }
        return mErrorCause;

    }

    // Handle ADN EF path which is same as AdnRecordLoader
    private String getAdnEFPath(int efid) {
        if (efid == IccConstants.EF_ADN) {
            return IccConstants.MF_SIM + IccConstants.DF_TELECOM;
        }

        return null;
    }

    /**
     * Get the capacity of records in efid.
     *
     * @param efid the EF id of a ADN-like ICC
     * @return  int[3] array
     *            recordSizes[0]  is the single record length
     *            recordSizes[1]  is the total length of the EF file
     *            recordSizes[2]  is the number of records in the EF file
     */
    public int[] getAdnRecordsSize(int efid) {
        if (DBG) logd("getAdnRecordsSize: efid=" + efid);
        checkThread();
        IccPhoneBookInterfaceManager.Request getSizeRequest
                = new IccPhoneBookInterfaceManager.Request();
        synchronized (getSizeRequest) {
            //Using mMtkBaseHandler, no difference in EVENT_GET_SIZE_DONE handling
            Message response = mMtkBaseHandler.obtainMessage(EVENT_GET_SIZE_DONE, getSizeRequest);

            IccFileHandler fh = mPhone.getIccFileHandler();
            if (fh != null) {
                if (getAdnEFPath(efid) != null) {
                    fh.getEFLinearRecordSize(efid, getAdnEFPath(efid), response);
                } else {
                    fh.getEFLinearRecordSize(efid, response);
                }
                waitForResult(getSizeRequest);
            }
        }

        return getSizeRequest.mResult == null ? new int[3] : (int[]) getSizeRequest.mResult;
    }


    /**
     * Loads the AdnRecords in efid and returns them as a
     * List of AdnRecords.
     *
     * throws SecurityException if no READ_CONTACTS permission
     *
     * @param efid the EF id of a ADN-like ICC
     * @return List of MtkAdnRecord
     */
    public synchronized List<MtkAdnRecord> getAdnRecordsInEf(int efid, Object object) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        efid = updateEfForIccType(efid);
        if (DBG) logd("getAdnRecordsInEF: efid=0x" + Integer.toHexString(efid).toUpperCase());

        if (mAdnCache == null) {
            loge("getAdnRecordsInEF mAdnCache is null");
            return null;
        }
        checkThread();
        IccPhoneBookInterfaceManager.Request loadRequest
                = new IccPhoneBookInterfaceManager.Request();
        synchronized (loadRequest) {
            Message response = mMtkBaseHandler.obtainMessage(EVENT_LOAD_DONE, loadRequest);
            if (mAdnCache != null) {
                mAdnCache.requestLoadAllAdnLike(efid, mAdnCache.extensionEfForEf(efid), response);
                waitForResult(loadRequest);
            } else {
                loge("Failure while trying to load from SIM due to uninitialised adncache");
            }
        }
        return (List<MtkAdnRecord>) loadRequest.mResult;
    }

    protected void checkThread() {
            // Make sure this isn't the UI thread, since it will block
            if (mMtkBaseHandler.getLooper().equals(Looper.myLooper())) {
                loge("query() called on the main UI thread!");
                throw new IllegalStateException(
                        "You cannot call query on this provder from the main UI thread.");
            }
    }


    private int getErrorCauseFromException(CommandException e) {

        int ret;

        if (e == null) {
            return IccInternalInterface.ERROR_ICC_PROVIDER_SUCCESS;
        }

        switch (e.getCommandError()) {
            case GENERIC_FAILURE: /* occurs when Extension file is full(?) */
                ret = IccInternalInterface.ERROR_ICC_PROVIDER_GENERIC_FAILURE;
                break;
            case OEM_ERROR_1:
                ret = IccInternalInterface.ERROR_ICC_PROVIDER_NUMBER_TOO_LONG;
                break;
            case SIM_PUK2:
            case PASSWORD_INCORRECT:
                ret = IccInternalInterface.ERROR_ICC_PROVIDER_PASSWORD_ERROR;
                break;
            case OEM_ERROR_2:
                ret = IccInternalInterface.ERROR_ICC_PROVIDER_TEXT_TOO_LONG;
                break;
            case OEM_ERROR_3:
                ret = IccInternalInterface.ERROR_ICC_PROVIDER_STORAGE_FULL;
                break;
            case OEM_ERROR_4:
                ret = IccInternalInterface.ERROR_ICC_PROVIDER_NOT_READY;
                break;
            case OEM_ERROR_5:
                ret = IccInternalInterface.ERROR_ICC_PROVIDER_ANR_TOO_LONG;
                break;
            case OEM_ERROR_6:
                ret = IccInternalInterface.ERROR_ICC_PROVIDER_ANR_SAVE_FAILURE;
                break;
            case RADIO_NOT_AVAILABLE:
            case OEM_ERROR_7:
                ret = IccInternalInterface.ERROR_ICC_PROVIDER_ADN_LIST_NOT_EXIST;
                break;
            case OEM_ERROR_8:
                ret = IccInternalInterface.ERROR_ICC_PROVIDER_EMAIL_FULL;
                break;
            case OEM_ERROR_9:
                ret = IccInternalInterface.ERROR_ICC_PROVIDER_EMAIL_TOO_LONG;
                break;
            case OEM_ERROR_10:
                ret = IccInternalInterface.ERROR_ICC_PROVIDER_SNE_FULL;
                break;
            case OEM_ERROR_11:
                ret = IccInternalInterface.ERROR_ICC_PROVIDER_SNE_TOO_LONG;
                break;
            default:
                ret = IccInternalInterface.ERROR_ICC_PROVIDER_UNKNOWN;
                break;
        }

        return ret;
    }

    public void onPhbReady() {
        if (mAdnCache != null) {
            mAdnCache.requestLoadAllAdnLike(IccConstants.EF_ADN,
                    mAdnCache.extensionEfForEf(IccConstants.EF_ADN), null);
        }
    }

    /**
     * Check if PHB ready.
     *
     * @return  true for PHB ready
     */
    public boolean isPhbReady() {
        boolean phbReady = false;

        if (mAdnCache == null) {
            return false;
        }

        if (SubscriptionManager.isValidSlotIndex(mSlotId) == true) {
            if (mIccRecords != null) {
                if (mIccRecords instanceof MtkSIMRecords) {
                    phbReady = ((MtkSIMRecords) mIccRecords).isPhbReady();
                } else if (mIccRecords instanceof MtkRuimRecords) {
                    phbReady = ((MtkRuimRecords) mIccRecords).isPhbReady();
                }
            }
        }
        return phbReady;
    }

    /**
     * Get all Groups.
     *
     * @return  List of all Groups.
     * @see     UsimGroup
     */
    public List<UsimGroup> getUsimGroups() {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        return ((mAdnCache == null) ? null : ((MtkAdnRecordCache) mAdnCache).getUsimGroups());
    }

    /**
     * Get Group name by index.
     *
     * @param nGasId Group index
     * @return  Group name string of the group index
     */
    public String getUsimGroupById(int nGasId) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        return ((mAdnCache == null) ? null : ((MtkAdnRecordCache) mAdnCache).getUsimGroupById(
                nGasId));
    }

    /**
     * Remove a Group.
     *
     * @param nGasId Group index to be removed
     * @return  true for remove successfully
     */
    public boolean removeUsimGroupById(int nGasId) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        return ((mAdnCache == null) ? false : ((MtkAdnRecordCache) mAdnCache).removeUsimGroupById(
                nGasId));
    }

    /**
     * Add a new Group.
     *
     * @param grpName Group name string
     * @return  index for success, USIM_ERROR_* defined in UsimPhoneBookManager for error,
     *          -1 for other exception
     */
    public int insertUsimGroup(String grpName) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        return ((mAdnCache == null) ? -1 : ((MtkAdnRecordCache) mAdnCache).insertUsimGroup(
                grpName));
    }

    /**
     * Update Group name(GAS) by index.
     *
     * @param nGasId Group name(GAS) index to be updated
     * @param grpName Group name string
     * @return  0 for success, USIM_ERROR_* defined in UsimPhoneBookManager for error,
     *          -1 for other exception
     */
    public int updateUsimGroup(int nGasId, String grpName) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        return ((mAdnCache == null) ? -1 : ((MtkAdnRecordCache) mAdnCache).updateUsimGroup(
                nGasId, grpName));
    }

    /**
     * Add contact to a group.
     *
     * @param adnIndex ADN index
     * @param grpIndex Group index to be added
     * @return  true for add successfully
     */
    public boolean addContactToGroup(int adnIndex, int grpIndex) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        return ((mAdnCache == null) ? false : ((MtkAdnRecordCache) mAdnCache).addContactToGroup(
                adnIndex, grpIndex));
    }

    /**
     * Remove contact from group.
     *
     * @param adnIndex ADN index
     * @param grpIndex removed group index for the ADN
     * @return  true for remove successfully
     */
    public boolean removeContactFromGroup(int adnIndex, int grpIndex) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        return ((mAdnCache == null) ? false : ((MtkAdnRecordCache) mAdnCache).
                removeContactFromGroup(adnIndex, grpIndex));
    }

    /**
     * Update contact's group.
     *
     * @param adnIndex updated ADN index
     * @param grpIdList new group id list for the ADN
     * @return  true for update successfully
     */
    public boolean updateContactToGroups(int adnIndex, int[] grpIdList) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        return ((mAdnCache == null) ? false : ((MtkAdnRecordCache) mAdnCache).
                updateContactToGroups(adnIndex, grpIdList));
    }

    /**
     * Move contact in group to another group.
     *
     * @param adnIndex moved ADN index
     * @param fromGrpIdList old group id list for the ADN
     * @param toGrpIdList new group id list for the ADN
     * @return  true for move successfully
     */
    public boolean moveContactFromGroupsToGroups(int adnIndex,
                                                 int[] fromGrpIdList, int[] toGrpIdList) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        return ((mAdnCache == null) ? false : ((MtkAdnRecordCache) mAdnCache).
                moveContactFromGroupsToGroups(adnIndex, fromGrpIdList, toGrpIdList));
    }

    /**
     * Query group exist or not by group name.
     * @param grpName the group name to be checked
     * @return  group index for exist, -1 for not exist
     */
    public int hasExistGroup(String grpName) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        return ((mAdnCache == null) ? -1 : ((MtkAdnRecordCache) mAdnCache).hasExistGroup(grpName));
    }

    /**
     * Get GAS's max input length in bytes.
     *
     * @return  GAS's max input length in bytes
     */
    public int getUsimGrpMaxNameLen() {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        return ((mAdnCache == null) ? -1 : ((MtkAdnRecordCache) mAdnCache).getUsimGrpMaxNameLen());
    }

    /**
     * Get Group(GAS) max entries numbers.
     *
     * @return  Group(GAS) max entries numbers
     */
    public int getUsimGrpMaxCount() {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        return ((mAdnCache == null) ? -1 : ((MtkAdnRecordCache) mAdnCache).getUsimGrpMaxCount());
    }

    /**
     * Get all AAS.
     *
     * @return  List of all AAS
     * @see     AlphaTag
     */
    public List<AlphaTag> getUsimAasList() {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        return ((mAdnCache == null) ? null : ((MtkAdnRecordCache) mAdnCache).getUsimAasList());
    }

    /**
     * Get AAS by index.
     *
     * @param index AAS index
     * @return  AAS string of inputed AAS index
     */
    public String getUsimAasById(int index) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        return ((mAdnCache == null) ? null : ((MtkAdnRecordCache) mAdnCache).getUsimAasById(
                index));
    }

    /**
     * Remove AAS by index.
     *
     * @param index removed AAS index
     * @param pbrIndex PBR index of the AAS
     * @return  true for remove successfully
     */
    public boolean removeUsimAasById(int index, int pbrIndex) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        return ((mAdnCache == null) ? false : ((MtkAdnRecordCache) mAdnCache).removeUsimAasById(
                index, pbrIndex));
    }

    /**
     * Insert a new AAS string.
     *
     * @param aasName AAS String to insert
     * @return  AAS index if success, 0 for string empty or too long,
     *          -2 for no entries(fulled), -1 for other exception
     */
    public int insertUsimAas(String aasName) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        return ((mAdnCache == null) ? -1 : ((MtkAdnRecordCache) mAdnCache).insertUsimAas(aasName));
    }

    /**
     * Update AAS string by index.
     *
     * @param index updated AAS index
     * @param pbrIndex PBR index of the AAS
     * @param aasName AAS string to update, null or "" means remove
     * @return  true for update successfully
     */
    public boolean updateUsimAas(int index, int pbrIndex, String aasName) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        return ((mAdnCache == null) ? false : ((MtkAdnRecordCache) mAdnCache).updateUsimAas(
                index, pbrIndex, aasName));
    }

    /**
     * Update AAS index of the ADN.
     *
     * @param adnIndex ADN index
     * @param aasIndex change AAS to the value refered by aasIndex, -1 means remove
     * @return  true for update successfully
     */
    public boolean updateAdnAas(int adnIndex, int aasIndex) {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.WRITE_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.WRITE_CONTACTS permission");
        }

        return ((mAdnCache == null) ? false : ((MtkAdnRecordCache) mAdnCache).updateAdnAas(
                adnIndex, aasIndex));
    }

    /**
     * Get PHB support ANR count.
     *
     * @return  PHB support ANR count
     */
    public int getAnrCount() {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        return ((mAdnCache == null) ? 0 : ((MtkAdnRecordCache) mAdnCache).getAnrCount());
    }

    /**
     * Get PHB support Email count.
     *
     * @return  PHB support Email count
     */
    public int getEmailCount() {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        return ((mAdnCache == null) ? 0 : ((MtkAdnRecordCache) mAdnCache).getEmailCount());
    }

    /**
     * Get AAS's max entries numbers.
     *
     * @return  AAS's max entries numbers
     */
    public int getUsimAasMaxCount() {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        return ((mAdnCache == null) ? -1 : ((MtkAdnRecordCache) mAdnCache).getUsimAasMaxCount());
    }

    /**
     * Get AAS's max input length in bytes.
     *
     * @return  AAS's max input length in bytes
     */
    public int getUsimAasMaxNameLen() {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        return ((mAdnCache == null) ? -1 : ((MtkAdnRecordCache) mAdnCache).getUsimAasMaxNameLen());
    }

    /**
     * Check if the PHB support SNE.
     *
     * @return  true for SNE support
     */
    public boolean hasSne() {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        return ((mAdnCache == null) ? false : ((MtkAdnRecordCache) mAdnCache).hasSne());
    }

    /**
     * Get SNE's max input length in bytes.
     *
     * @return  SNE's max input length in bytes
     */
    public int getSneRecordLen() {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        return ((mAdnCache == null) ? -1 : ((MtkAdnRecordCache) mAdnCache).getSneRecordLen());
    }

    /**
     * Judge if the PHB ADN is accessible or not.
     *
     * @return  true for ready
     */
    public boolean isAdnAccessible() {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        return ((mAdnCache == null) ? false : ((MtkAdnRecordCache) mAdnCache).isAdnAccessible());
    }

    public synchronized UsimPBMemInfo[] getPhonebookMemStorageExt() {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        return ((mAdnCache == null) ? null : ((MtkAdnRecordCache) mAdnCache).
                getPhonebookMemStorageExt());
    }

    public int getUpbDone() {
        return ((mAdnCache == null) ? -1 : ((MtkAdnRecordCache) mAdnCache).getUpbDone());
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
     *          null if error happen
     */
    public int[] getAdnRecordsCapacity() {

        if (mPhone.getContext().checkCallingOrSelfPermission(
                android.Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Requires android.permission.READ_CONTACTS permission");
        }

        return (mAdnCache == null) ? null : ((MtkAdnRecordCache) mAdnCache).getAdnRecordsCapacity();
    }
}
