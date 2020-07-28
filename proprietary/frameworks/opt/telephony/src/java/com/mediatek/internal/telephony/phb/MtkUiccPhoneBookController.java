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

import android.os.ServiceManager;
import android.telephony.Rlog;

import com.android.internal.telephony.IccPhoneBookInterfaceManager;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.SubscriptionController;

import java.util.List;

public class MtkUiccPhoneBookController extends IMtkIccPhoneBook.Stub {
    private static final String TAG = "MtkUiccPhoneBookController";
    private Phone[] mPhone;

    /* only one MtkUiccPhoneBookController exists */
    public MtkUiccPhoneBookController(Phone[] phone) {
        if (ServiceManager.getService("mtksimphonebook") == null) {
               ServiceManager.addService("mtksimphonebook", this);
        }
        mPhone = phone;
    }

    @Override
    public List<MtkAdnRecord> getAdnRecordsInEf(int efid) throws android.os.RemoteException {
        return getAdnRecordsInEfForSubscriber(getDefaultSubscription(), efid);
    }

    @Override
    public List<MtkAdnRecord> getAdnRecordsInEfForSubscriber(int subId, int efid)
           throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).getAdnRecordsInEf(efid, null);
        } else {
            Rlog.e(TAG, "getAdnRecordsInEf iccPbkIntMgr is" +
                      "null for Subscription:" + subId);
            return null;
        }
    }

    public int
    updateAdnRecordsInEfBySearchWithError(int subId, int efid,
            String oldTag, String oldPhoneNumber,
            String newTag, String newPhoneNumber, String pin2) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).
                    updateAdnRecordsInEfBySearchWithError(efid, oldTag, oldPhoneNumber,
                    newTag, newPhoneNumber, pin2);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return 0;
        }
    }

    public int
    updateUsimPBRecordsInEfBySearchWithError(int subId, int efid,
            String oldTag, String oldPhoneNumber, String oldAnr, String oldGrpIds,
            String[] oldEmails, String newTag, String newPhoneNumber, String newAnr,
            String newGrpIds, String[] newEmails) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).
                    updateUsimPBRecordsInEfBySearchWithError(efid, oldTag, oldPhoneNumber, oldAnr,
                    oldGrpIds, oldEmails, newTag, newPhoneNumber, newAnr, newGrpIds, newEmails);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return 0;
        }
    }

    public int
    updateAdnRecordsInEfByIndexWithError(int subId, int efid, String newTag,
            String newPhoneNumber, int index, String pin2) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).
                    updateAdnRecordsInEfByIndexWithError(efid, newTag, newPhoneNumber, index,
                    pin2);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return 0;
        }
    }

    public int
    updateUsimPBRecordsInEfByIndexWithError(int subId, int efid, String newTag,
            String newPhoneNumber, String newAnr,  String newGrpIds, String[] newEmails, int index)
            throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).
                    updateUsimPBRecordsInEfByIndexWithError(efid, newTag, newPhoneNumber, newAnr,
                    newGrpIds, newEmails, index);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return 0;
        }
    }

    public int updateUsimPBRecordsByIndexWithError(int subId, int efid, MtkAdnRecord record,
            int index) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).
                    updateUsimPBRecordsByIndexWithError(efid, record, index);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return 0;
        }
    }

    public int updateUsimPBRecordsBySearchWithError(int subId, int efid, MtkAdnRecord oldAdn,
            MtkAdnRecord newAdn) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).
                    updateUsimPBRecordsBySearchWithError(efid, oldAdn, newAdn);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return 0;
        }
    }

    public boolean isPhbReady(int subId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).isPhbReady();
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return false;
        }
    }

    public List<UsimGroup> getUsimGroups(int subId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).getUsimGroups();
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return null;
        }
    }

    public String getUsimGroupById(int subId, int nGasId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).getUsimGroupById(nGasId);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return null;
        }
    }

    public boolean removeUsimGroupById(int subId, int nGasId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).removeUsimGroupById(nGasId);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return false;
        }
    }

    public int insertUsimGroup(int subId, String grpName) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).insertUsimGroup(grpName);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return -1;
        }
    }

    public int updateUsimGroup(int subId, int nGasId, String grpName)
            throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).
                    updateUsimGroup(nGasId, grpName);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return -1;
        }
    }

    public boolean addContactToGroup(int subId, int adnIndex, int grpIndex)
            throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).
                    addContactToGroup(adnIndex, grpIndex);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return false;
        }
    }

    public boolean removeContactFromGroup(int subId, int adnIndex, int grpIndex)
            throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).
                    removeContactFromGroup(adnIndex, grpIndex);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return false;
        }
    }

    public boolean updateContactToGroups(int subId, int adnIndex, int[] grpIdList)
            throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).
                    updateContactToGroups(adnIndex, grpIdList);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return false;
        }
    }

    public boolean moveContactFromGroupsToGroups(int subId, int adnIndex, int[] fromGrpIdList,
                                             int[] toGrpIdList) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).
                    moveContactFromGroupsToGroups(adnIndex, fromGrpIdList, toGrpIdList);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return false;
        }
    }

    public int hasExistGroup(int subId, String grpName) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).hasExistGroup(grpName);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return -1;
        }
    }

    public int getUsimGrpMaxNameLen(int subId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).getUsimGrpMaxNameLen();
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return -1;
        }
    }

    public int getUsimGrpMaxCount(int subId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).getUsimGrpMaxCount();
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return -1;
        }
    }

    public List<AlphaTag> getUsimAasList(int subId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).getUsimAasList();
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return null;
        }
    }

    public String getUsimAasById(int subId, int index) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).getUsimAasById(index);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return null;
        }
    }

    public int insertUsimAas(int subId, String aasName) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).insertUsimAas(aasName);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return 0;
        }
    }

    public int getAnrCount(int subId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).getAnrCount();
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return 0;
        }
    }

    public int getEmailCount(int subId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).getEmailCount();
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return 0;
        }
    }

    public int getUsimAasMaxCount(int subId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).getUsimAasMaxCount();
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return 0;
        }
    }

    public int getUsimAasMaxNameLen(int subId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).getUsimAasMaxNameLen();
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return 0;
        }
    }

    public boolean updateUsimAas(int subId, int index, int pbrIndex, String aasName)
            throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).
                    updateUsimAas(index, pbrIndex, aasName);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return false;
        }
    }

    public boolean removeUsimAasById(int subId, int index, int pbrIndex)
            throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).
                    removeUsimAasById(index, pbrIndex);
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return false;
        }
    }

    public boolean hasSne(int subId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).hasSne();
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return false;
        }
    }

    public int getSneRecordLen(int subId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).getSneRecordLen();
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return 0;
        }
    }

    public boolean isAdnAccessible(int subId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).isAdnAccessible();
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return true;
        }
    }

    public UsimPBMemInfo[] getPhonebookMemStorageExt(int subId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).getPhonebookMemStorageExt();
        } else {
            Rlog.e(TAG, "updateAdnRecordsInEfBySearch iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return null;
        }
    }

    public int getUpbDone(int subId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager) iccPbkIntMgr).getUpbDone();
        } else {
            Rlog.e(TAG, "getUpbDone iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return -1;
        }
    }

    @Override
    public int[] getAdnRecordsCapacity() throws android.os.RemoteException {
        return getAdnRecordsCapacityForSubscriber(getDefaultSubscription());
    }

    @Override
    public int[] getAdnRecordsCapacityForSubscriber(int subId) throws android.os.RemoteException {
        IccPhoneBookInterfaceManager iccPbkIntMgr =
                             getIccPhoneBookInterfaceManager(subId);
        if (iccPbkIntMgr != null) {
            return ((MtkIccPhoneBookInterfaceManager)iccPbkIntMgr).getAdnRecordsCapacity();
        } else {
            Rlog.e(TAG, "getAdnRecordsCapacity iccPbkIntMgr is" +
                      " null for Subscription:" + subId);
            return null;
        }
    }

    /**
     * get phone book interface manager object based on subscription.
     **/
    private IccPhoneBookInterfaceManager
            getIccPhoneBookInterfaceManager(int subId) {

        int phoneId = SubscriptionController.getInstance().getPhoneId(subId);
        try {
            return mPhone[phoneId].getIccPhoneBookInterfaceManager();
        } catch (NullPointerException e) {
            Rlog.e(TAG, "Exception is :" + e.toString() + " For subscription :" + subId);
            e.printStackTrace(); //To print stack trace
            return null;
        } catch (ArrayIndexOutOfBoundsException e) {
            Rlog.e(TAG, "Exception is :" + e.toString() + " For subscription :" + subId);
            e.printStackTrace();
            return null;
        }
    }

    private int getDefaultSubscription() {
        return PhoneFactory.getDefaultSubscription();
    }
}
