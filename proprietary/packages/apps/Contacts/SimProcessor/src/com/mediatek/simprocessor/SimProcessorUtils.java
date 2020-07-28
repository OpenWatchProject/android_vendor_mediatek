/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.simprocessor;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.ContactsContract.CommonDataKinds.Nickname;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Groups;
import android.text.TextUtils;

import com.mediatek.internal.telephony.phb.IMtkIccPhoneBook;
import com.mediatek.internal.telephony.phb.UsimGroup;
import com.mediatek.simprocessor.SimServiceUtils.ServiceWorkData;

public class SimProcessorUtils {
    private static final boolean DEBUG = true;
    private static final String TAG = "SimProcessorUtils";

    public static final String ACCOUNT_TYPE_SIM = "SIM Account";
    public static final String ACCOUNT_TYPE_USIM = "USIM Account";
    public static final String ACCOUNT_TYPE_LOCAL_PHONE = "Local Phone Account";
    public static final String ACCOUNT_TYPE_RUIM = "RUIM Account";
    public static final String ACCOUNT_TYPE_CSIM = "CSIM Account";
    /** Define a new Phone label type */
    public static final int TYPE_AAS = 101;

    /**
     * @param subId
     *            sub id
     * @return the account name for this sub id
     */
    public static String getAccountNameUsingSubId(int subId) {
        String accountName = null;
        String iccCardType = SimCardUtils.getIccCardType(subId);
        if (iccCardType != null) {
            accountName = iccCardType + subId;
        }

        return accountName;
    }

    /**
     * @param subId
     * @return the account type for this sub id
     */
    public static String getAccountTypeUsingSubId(int subId) {
        int simtype = -1;
        String simAccountType = null;

        simtype = SimCardUtils.getSimTypeBySubId(subId);
        if (SimCardUtils.SimType.SIM_TYPE_USIM == simtype) {
            simAccountType = ACCOUNT_TYPE_USIM;
        } else if (SimCardUtils.SimType.SIM_TYPE_SIM == simtype) {
            simAccountType = ACCOUNT_TYPE_SIM;
        } else if (SimCardUtils.SimType.SIM_TYPE_RUIM == simtype) {
            simAccountType = ACCOUNT_TYPE_RUIM;
        } else if (SimCardUtils.SimType.SIM_TYPE_CSIM == simtype) {
            simAccountType = ACCOUNT_TYPE_CSIM;
        }

        return simAccountType;
    }

    public static class NamePhoneTypePair {
        public String name;
        public int phoneType;
        public String phoneTypeSuffix;

        public NamePhoneTypePair(String nameWithPhoneType) {
            // Look for /W /H /M or /O at the end of the name signifying the
            // type
            int nameLen = nameWithPhoneType.length();
            if (nameLen - 2 >= 0 && nameWithPhoneType.charAt(nameLen - 2) == '/') {
                char c = Character.toUpperCase(nameWithPhoneType.charAt(nameLen - 1));
                phoneTypeSuffix = String.valueOf(nameWithPhoneType.charAt(nameLen - 1));
                if (c == 'W') {
                    phoneType = Phone.TYPE_WORK;
                } else if (c == 'M' || c == 'O') {
                    phoneType = Phone.TYPE_MOBILE;
                } else if (c == 'H') {
                    phoneType = Phone.TYPE_HOME;
                } else {
                    phoneType = Phone.TYPE_OTHER;
                }
                name = nameWithPhoneType.substring(0, nameLen - 2);
            } else {
                phoneTypeSuffix = "";
                phoneType = Phone.TYPE_OTHER;
                name = nameWithPhoneType;
            }
        }
    }

    public static final class USIMGroup {
        private static final String TAG = "SimProcessorUtils.USIMGroup";
        public static final String SIM_TYPE_USIM = "USIM";
        public static final String SIM_TYPE_CSIM = "CSIM";

        private static final HashMap<Integer, ArrayList<UsimGroup>> UGRP_LISTARRAY =
                new HashMap<Integer, ArrayList<UsimGroup>>() {
            @Override
            public ArrayList<UsimGroup> get(Object key) {
                Integer subId = (Integer) key;
                if (super.get(subId) == null) {
                    put(subId, new ArrayList<UsimGroup>());
                }
                return super.get(key);
            }
        };

        /**
         * Sync USIM group
         *
         * @param context
         * @param grpIdMap
         *            The pass in varible must not be null.
         */
        public static synchronized void syncUSIMGroupContactsGroup(Context context,
                final ServiceWorkData workData, HashMap<Integer, Integer> grpIdMap) {
            Log.d(TAG, "[syncUSIMGroupContactsGroup] begin");
            String simTypeTag = "UNKNOWN";
            if (workData.mSimType == SimCardUtils.SimType.SIM_TYPE_USIM) {
                simTypeTag = "USIM";
            } else if (workData.mSimType == SimCardUtils.SimType.SIM_TYPE_CSIM) {
                simTypeTag = "CSIM";
            } else {
                Log.w(TAG, "[syncUSIMGroupContactsGroup]wrong type workData.mSimType : "
                        + workData.mSimType);
                return;
            }
            final int subId = workData.mSubId;

            ArrayList<UsimGroup> ugrpList = UGRP_LISTARRAY.get(subId);

            // Get All groups in USIM
            ugrpList.clear();
            final IMtkIccPhoneBook iIccPhb = getIMtkIccPhoneBook();
            if (iIccPhb == null) {
                Log.w(TAG, "[syncUSIMGroupContactsGroup]iIccPhb is null,return!");
                return;
            }

            try {
                List<UsimGroup> uList = iIccPhb.getUsimGroups(subId);
                if (uList == null) {
                    return;
                }
                for (UsimGroup ug : uList) {
                    String gName = ug.getAlphaTag();
                    int gIndex = ug.getRecordIndex();
                    Log.i(TAG, "[syncUSIMGroupContactsGroup]gName:" + Log.anonymize(gName)
                            + "|gIndex: " + gIndex);

                    if (!TextUtils.isEmpty(gName) && gIndex > 0) {
                        ugrpList.add(new UsimGroup(gIndex, gName));
                    }
                }
            } catch (android.os.RemoteException e) {
                Log.e(TAG, "[syncUSIMGroupContactsGroup]catched exception:");
                e.printStackTrace();
            }

            // Query SIM info to get simId
            // Query to get all groups in Phone
            ContentResolver cr = context.getContentResolver();
            Cursor c = cr.query(Groups.CONTENT_SUMMARY_URI, null, Groups.DELETED + "=0 AND "
                    + Groups.ACCOUNT_TYPE + "='" + simTypeTag + " Account' AND "
                    + Groups.ACCOUNT_NAME + "=" + "'" + simTypeTag + subId + "'", null, null);
            // Query all Group including deleted group

            HashMap<String, Integer> noneMatchedMap = new HashMap<String, Integer>();
            if (c != null) {
                c.moveToPosition(-1);
                while (c.moveToNext()) {
                    String grpName = c.getString(c.getColumnIndexOrThrow(Groups.TITLE));
                    int grpId = c.getInt(c.getColumnIndexOrThrow(Groups._ID));
                    if (!noneMatchedMap.containsKey(grpName)) {
                        noneMatchedMap.put(grpName, grpId);
                    }
                }
                c.close();
            }

            if (ugrpList != null) {
                boolean hasMerged = false;
                for (UsimGroup ugrp : ugrpList) {
                    String ugName = ugrp.getAlphaTag();
                    hasMerged = false;
                    long groupId = -1;
                    if (!TextUtils.isEmpty(ugName)) {
                        int ugId = ugrp.getRecordIndex();
                        if (noneMatchedMap.containsKey(ugName)) {
                            groupId = noneMatchedMap.get(ugName);
                            noneMatchedMap.remove(ugName);
                            hasMerged = true;
                        }

                        if (!hasMerged) {
                            // Need to create on phone
                            ContentValues values = new ContentValues();
                            values.put(Groups.TITLE, ugName);
                            values.put(Groups.GROUP_VISIBLE, 1);
                            values.put(Groups.SYSTEM_ID, 0);
                            values.put(Groups.ACCOUNT_NAME, simTypeTag + subId);
                            values.put(Groups.ACCOUNT_TYPE, simTypeTag + " Account");
                            Uri uri = cr.insert(Groups.CONTENT_URI, values);
                            groupId = (uri == null) ? 0 : ContentUris.parseId(uri);
                        }
                        if (groupId > 0) {
                            grpIdMap.put(ugId, (int) groupId);
                        }
                    }
                }

                if (noneMatchedMap.size() > 0) {
                    Integer[] groupIdArray = noneMatchedMap.values().toArray(new Integer[0]);
                    StringBuilder delGroupIdStr = new StringBuilder();
                    for (Integer i : groupIdArray) {
                        int delGroupId = i;
                        delGroupIdStr.append(delGroupId).append(",");
                    }
                    if (delGroupIdStr.length() > 0) {
                        delGroupIdStr.deleteCharAt(delGroupIdStr.length() - 1);
                    }
                    if (delGroupIdStr.length() > 0) {
                        cr.delete(Groups.CONTENT_URI,
                                Groups._ID + " IN (" + delGroupIdStr.toString() + ")", null);
                    }
                }
                Log.i(TAG, "[syncUSIMGroupContactsGroup] end.");
            } else {
                deleteUSIMGroupOnPhone(context, subId);
            }
        }

        public static void deleteUSIMGroupOnPhone(Context context, int subId) {
            ContentResolver cr = context.getContentResolver();
            cr.delete(Groups.CONTENT_URI, Groups.ACCOUNT_TYPE + "='USIM Account' AND "
                    + Groups.ACCOUNT_NAME + "=" + "'USIM" + subId + "'", null);
            cr.delete(Groups.CONTENT_URI, Groups.ACCOUNT_TYPE + "='CSIM Account' AND "
                    + Groups.ACCOUNT_NAME + "=" + "'CSIM" + subId + "'", null);
        }
    }

    public static IMtkIccPhoneBook getIMtkIccPhoneBook() {
        String serviceName = SubInfoUtils.getMtkPhoneBookServiceName();
        final IMtkIccPhoneBook iIccPhb = IMtkIccPhoneBook.Stub.asInterface(ServiceManager
                .getService(serviceName));
        return iIccPhb;
    }

    public static int importSimSne(
            ArrayList<ContentProviderOperation> operationList,
            final Cursor cursor, int index) {
        // build SNE ContentProviderOperation from cursor
        int sneColumnIdx = cursor.getColumnIndex("sne");
        if (sneColumnIdx != -1) {
            String nickname = cursor.getString(sneColumnIdx);
            Log.d(TAG, "[buildOperationFromCursor] sneColumnIdx:" + sneColumnIdx
                    + ". nickname:" + Log.anonymize(nickname));
            if (!TextUtils.isEmpty(nickname)) {
                ContentProviderOperation.Builder builder = ContentProviderOperation
                        .newInsert(Data.CONTENT_URI);
                builder.withValueBackReference(Nickname.RAW_CONTACT_ID, index);
                builder.withValue(Data.MIMETYPE, Nickname.CONTENT_ITEM_TYPE);
                builder.withValue(Nickname.DATA, nickname);
                Log.d(TAG, "[buildOperationFromCursor] nickname added" + Log.anonymize(nickname));
                operationList.add(builder.build());
                return 1;
            }
        }
        return 0;
    }
}
