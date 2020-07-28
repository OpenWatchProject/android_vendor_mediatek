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
package com.mediatek.contacts.util;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Parcel;
import android.os.Parcelable;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.ContactsContract.CommonDataKinds.GroupMembership;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Groups;
import android.provider.ContactsContract.RawContacts;
import android.text.TextUtils;

import com.android.contacts.R;

import com.mediatek.contacts.simcontact.SimCardUtils;
import com.mediatek.contacts.simservice.SimServiceUtils.ServiceWorkData;
import com.mediatek.contacts.simcontact.PhbInfoUtils;
import com.mediatek.contacts.simcontact.SubInfoUtils;
import com.mediatek.internal.telephony.phb.UsimGroup;
import com.mediatek.internal.telephony.phb.IMtkIccPhoneBook;
import com.mediatek.internal.telephony.phb.MtkUsimPhoneBookManager;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

public class ContactsGroupUtils {
    private static final String TAG = "ContactsGroupUtils";

    public static int sArrayData;
    public static final String CONTACTS_IN_GROUP_SELECT = " IN " + "(SELECT "
            + RawContacts.CONTACT_ID + " FROM " + "raw_contacts" + " WHERE " + "raw_contacts._id"
            + " IN " + "(SELECT " + "data." + Data.RAW_CONTACT_ID + " FROM " + "data "
            + "JOIN mimetypes ON (data.mimetype_id = mimetypes._id)" + " WHERE " + Data.MIMETYPE
            + "='" + GroupMembership.CONTENT_ITEM_TYPE + "' AND " + GroupMembership.GROUP_ROW_ID
            + "=" + "(SELECT " + "groups" + "." + Groups._ID + " FROM " + "groups" + " WHERE "
            + Groups.DELETED + "=0 AND " + Groups.TITLE + "=?))" + " AND " + RawContacts.DELETED
            + "=0)";

    public static IMtkIccPhoneBook getIMtkIccPhoneBook() {
        String serviceName = SubInfoUtils.getMtkPhoneBookServiceName();
        final IMtkIccPhoneBook iIccPhb = IMtkIccPhoneBook.Stub.asInterface(ServiceManager
                .getService(serviceName));
        return iIccPhb;
    }

    public static final String SELECTION_MOVE_GROUP_DATA = RawContacts.CONTACT_ID + " IN (%1) AND "
            + Data.MIMETYPE + "='" + GroupMembership.CONTENT_ITEM_TYPE + "' AND "
            + GroupMembership.GROUP_ROW_ID + "='%2'";

    private static final int MAX_OP_COUNT_IN_ONE_BATCH = 150;

    /**
     * Move contacts from one USIM group to another
     *
     * @param data
     *            Contacts data.
     * @param ugrpIdArray
     *            Must be created before calling, and the array length must 4.
     *            the first one indicates old USIM group id, and the second one
     *            indicates the target USIM group id.
     * @param isInTargetGroup
     *            This variable indicates whether a contacts is already in
     *            target group.
     */
    public static boolean moveUSIMGroupMember(ContactsGroupArrayData data, int subId,
            boolean isInTargetGroup, int fromUgrpId, int toUgrpId) {
        boolean ret = false;
        Log.i(TAG, "[moveUSIMGroupMember]sub id: " + subId);
        if (subId > 0) {
            // Add group data into new USIM group if it is not in new USIM group
            boolean moveSucess = false;
            moveSucess = USIMGroup.moveContactToGroups(subId, data.mSimIndex,
                    new int[] { fromUgrpId }, new int[] { toUgrpId });
            Log.i(TAG, "[moveUSIMGroupMember]moveSucess : " + moveSucess + ",data.mSimIndex:"
                    + data.mSimIndex + ",fromUgrpId:" + fromUgrpId + ",toUgrpId:" + toUgrpId);
            ret = moveSucess;
        }

        return ret;
    }

    public static final class USIMGroup {
        private static final String TAG = "ContactsGroupUtils.USIMGroup";
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

        public static void addGroupItemToLocal(int subId, UsimGroup usimGroup) {
            ArrayList<UsimGroup> groupList = UGRP_LISTARRAY.get(subId);
            groupList.add(usimGroup);
            Log.d(TAG, "[addGroupItemToLocal]: usimGroup: " + Log.anonymize(usimGroup));
        }

        public static boolean updateLocalGroupName(int subId, int groupId, String newName) {
            ArrayList<UsimGroup> groupList = UGRP_LISTARRAY.get(subId);
            UsimGroup needUpateGroup = null;
            for (UsimGroup usimGroup : groupList) {
                if (usimGroup.getRecordIndex() == groupId) {
                    needUpateGroup = usimGroup;
                }
            }
            Log.d(TAG, "[updateLocalGroupName]needUpateGroup is null = "
                    + (needUpateGroup == null));
            if (needUpateGroup != null) {
                needUpateGroup.setAlphaTag(newName);
                return true;
            }
            return false;
        }

        public static boolean removeLocalGroupItem(int subId, int groupId) {
            ArrayList<UsimGroup> groupList = UGRP_LISTARRAY.get(subId);
            UsimGroup dirtyGroup = null;
            for (UsimGroup usimGroup : groupList) {
                if (usimGroup.getRecordIndex() == groupId) {
                    dirtyGroup = usimGroup;
                }
            }
            Log.d(TAG, "[removeLocalGroupItem]: dirtyGroup: " + Log.anonymize(dirtyGroup));
            if (dirtyGroup != null) {
                groupList.remove(dirtyGroup);
                return true;
            }
            return false;
        }

        public static int hasExistGroup(int subId, String grpName) throws RemoteException {
            int grpId = -1;
            final IMtkIccPhoneBook iIccPhb = getIMtkIccPhoneBook();
            Log.d(TAG, "[hasExistGroup]grpName:" + Log.anonymize(grpName) + "|iIccPhb:" + iIccPhb);
            if (TextUtils.isEmpty(grpName) || iIccPhb == null) {
                return grpId;
            }

            ArrayList<UsimGroup> groupList = UGRP_LISTARRAY.get(subId);
            if (groupList.isEmpty()) {
                List<UsimGroup> usimGroupList = iIccPhb.getUsimGroups(subId);
                if (usimGroupList == null) {
                    Log.w(TAG, "[hasExistGroup] can't get usimGroupList from iIccPhb!");
                    return grpId;
                }
                for (UsimGroup usimGroup : usimGroupList) {
                    String groupName = usimGroup.getAlphaTag();
                    int groupIndex = usimGroup.getRecordIndex();
                    if (!TextUtils.isEmpty(groupName) && groupIndex > 0) {
                        groupList.add(new UsimGroup(groupIndex, groupName));
                        if (groupName.equals(grpName)) {
                            grpId = groupIndex;
                        }
                    }
                }
            } else {
                for (UsimGroup usimGroup : groupList) {
                    if (grpName.equals(usimGroup.getAlphaTag())) {
                        grpId = usimGroup.getRecordIndex();
                        break;
                    }
                }
            }
            Log.d(TAG, "[hasExistGroup]grpId:" + grpId);

            return grpId;
        }

        public static int syncUSIMGroupNewIfMissing(int subId, String name) throws RemoteException,
                USIMGroupException {
            int nameLen = 0;
            Log.i(TAG, "[syncUSIMGroupNewIfMissing]name:" + Log.anonymize(name) + ",subId:" + subId);
            if (TextUtils.isEmpty(name)) {
                Log.w(TAG, "[syncUSIMGroupNewIfMissing]name is null,return.");
                return -1;
            }
            try {
                nameLen = name.getBytes("GBK").length;
            } catch (java.io.UnsupportedEncodingException e) {
                Log.w(TAG, "[syncUSIMGroupNewIfMissing]UnsupportedEncodingException:" + e);
                nameLen = name.length();
            }

            /** hints generic error when phonebook is not ready. */
            int usimGrpMaxNameLen = PhbInfoUtils.getUsimGroupMaxNameLength(subId);
            Log.d(TAG, "[syncUSIMGroupNewIfMissing]nameLen:" + nameLen
                    + " ||usimGrpMaxNameLen:" + usimGrpMaxNameLen);

            if (usimGrpMaxNameLen == -1) {
                Log.e(TAG, "[syncUSIMGroupNewIfMissing]nameLen:" + nameLen
                        + " ||getUSIMGrpMaxNameLen(subId) is -1.");
                throw new USIMGroupException(USIMGroupException.ERROR_STR_GRP_GENERIC_ERROR,
                        USIMGroupException.GROUP_GENERIC_ERROR, subId);
            }
            /***/
            if (nameLen > usimGrpMaxNameLen) {
                throw new USIMGroupException(USIMGroupException.ERROR_STR_GRP_NAME_OUTOFBOUND,
                        USIMGroupException.GROUP_NAME_OUT_OF_BOUND, subId);
            }
            final IMtkIccPhoneBook iIccPhb = getIMtkIccPhoneBook();
            int grpId = -1;
            grpId = hasExistGroup(subId, name);
            if (grpId < 1 && iIccPhb != null) {
                grpId = iIccPhb.insertUsimGroup(subId, name);
                Log.i(TAG, "[syncUSIMGroupNewIfMissing]inserted grpId:" + grpId);
                if (grpId > 0) {
                    addGroupItemToLocal(subId, new UsimGroup(grpId, name));
                }
            }
            Log.d(TAG, "[syncUSIMGroupNewIfMissing]grpId:" + grpId);
            if (grpId < 1) {
                switch (grpId) {
                case USIMGroupException.USIM_ERROR_GROUP_COUNT:
                    throw new USIMGroupException(USIMGroupException.ERROR_STR_GRP_COUNT_OUTOFBOUND,
                            USIMGroupException.GROUP_NUMBER_OUT_OF_BOUND, subId);

                    // Name len has been check before new group.
                    // However, do protect here just for full logic.
                case USIMGroupException.USIM_ERROR_NAME_LEN:
                    throw new USIMGroupException(USIMGroupException.ERROR_STR_GRP_NAME_OUTOFBOUND,
                            USIMGroupException.GROUP_NAME_OUT_OF_BOUND, subId);
                default:
                    throw new USIMGroupException(USIMGroupException.ERROR_STR_GRP_GENERIC_ERROR,
                            USIMGroupException.GROUP_GENERIC_ERROR, subId);
                }
            }
            return grpId;
        }

        /**
         * If a group has to change name, the mapping group of USIM card should
         * also be changed
         *
         * @return
         */
        public static int syncUSIMGroupUpdate(int subId, String oldName, String newName)
                throws RemoteException, USIMGroupException {
            final IMtkIccPhoneBook iIccPhb = getIMtkIccPhoneBook();
            int grpId = hasExistGroup(subId, oldName);
            Log.d(TAG, "[syncUSIMGroupUpdate]grpId:" + grpId + "|subId:" + subId + "|oldName:"
                    + Log.anonymize(oldName) + "|newName:" + Log.anonymize(newName));
            if (grpId > 0) {
                int nameLength = 0;
                try {
                    if (!TextUtils.isEmpty(newName)) {
                        nameLength = newName.getBytes("GBK").length;
                    } else {
                        return grpId;
                    }
                } catch (java.io.UnsupportedEncodingException e) {
                    Log.w(TAG, "[syncUSIMGroupUpdate]UnsupportedEncodingException:" + e);
                    nameLength = newName.length();
                }
                if (nameLength > PhbInfoUtils.getUsimGroupMaxNameLength(subId)) {
                    Log.e(TAG, "[syncUSIMGroupUpdate]nameLength:" + nameLength
                                    + ",getUsimGroupMaxNameLength(subId):"
                                    + PhbInfoUtils.getUsimGroupMaxNameLength(subId));
                    throw new USIMGroupException(USIMGroupException.ERROR_STR_GRP_NAME_OUTOFBOUND,
                            USIMGroupException.GROUP_NAME_OUT_OF_BOUND, subId);
                }
                int ret = iIccPhb.updateUsimGroup(subId, grpId, newName);
                Log.d(TAG, "[syncUSIMGroupUpdate]updateUsimGroup ret:" + ret);
                if (ret < 0) {
                    switch (ret) {
                    case USIMGroupException.USIM_ERROR_GROUP_COUNT:
                        throw new USIMGroupException(
                                USIMGroupException.ERROR_STR_GRP_COUNT_OUTOFBOUND,
                                USIMGroupException.GROUP_NUMBER_OUT_OF_BOUND, subId);
                    case USIMGroupException.USIM_ERROR_NAME_LEN:
                        throw new USIMGroupException(
                                USIMGroupException.ERROR_STR_GRP_NAME_OUTOFBOUND,
                                USIMGroupException.GROUP_NAME_OUT_OF_BOUND, subId);
                    default:
                        throw new USIMGroupException(
                                USIMGroupException.ERROR_STR_GRP_GENERIC_ERROR,
                                USIMGroupException.GROUP_GENERIC_ERROR, subId);
                    }
                } else {//success
                    updateLocalGroupName(subId, grpId, newName);
                }
            }
            return grpId;
        }

        public static int deleteUSIMGroup(int subId, String name) {
            final IMtkIccPhoneBook iIccPhb = getIMtkIccPhoneBook();
            int errCode = -2;
            try {
                int grpId = hasExistGroup(subId, name);
                if (grpId > 0) {
                    if (iIccPhb.removeUsimGroupById(subId, grpId)) {
                        removeLocalGroupItem(subId, grpId);
                        errCode = 0;
                    } else {
                        errCode = -1;
                    }
                }
            } catch (android.os.RemoteException e) {
                Log.e(TAG, "[deleteUSIMGroup]RemoteException:" + e);
            }
            Log.d(TAG, "[deleteUSIMGroup]errCode:" + errCode);

            return errCode;
        }

        /**
         * Time consuming, run it in background.
         *
         * @param slotId
         * @param simIndex
         * @param grpId
         * @return
         */
        public static boolean addUSIMGroupMember(int subId, int simIndex, int grpId) {
            boolean succFlag = false;
            try {
                if (grpId > 0) {
                    final IMtkIccPhoneBook iIccPhb = getIMtkIccPhoneBook();
                    if (iIccPhb != null) {
                        succFlag = iIccPhb.addContactToGroup(subId, simIndex, grpId);
                    }
                }
            } catch (android.os.RemoteException e) {
                Log.e(TAG, "[deleteUSIMGroup]RemoteException:" + e);
                succFlag = false;
            }
            Log.i(TAG, "[addUSIMGroupMember]succFlag:" + succFlag);
            return succFlag;
        }

        /**
         * Time consuming, run in background
         *
         * @param subId
         * @param simIndex
         * @param grpId
         * @return
         */
        public static boolean deleteUSIMGroupMember(int subId, int simIndex, int grpId) {
            Log.d(TAG, "[deleteUSIMGroupMember]subId:" + subId + "|simIndex:" + simIndex
                    + "|grpId:" + grpId);
            boolean succFlag = false;
            try {
                if (grpId > 0) {
                    final IMtkIccPhoneBook iIccPhb = getIMtkIccPhoneBook();
                    if (iIccPhb != null) {
                        succFlag = iIccPhb.removeContactFromGroup(subId, simIndex, grpId);
                    }
                }
            } catch (android.os.RemoteException e) {
                Log.e(TAG, "[deleteUSIMGroup]RemoteException:" + e);
                succFlag = false;
            }
            Log.d(TAG, "[deleteUSIMGroupMember]result:" + succFlag + ",subId:" + subId
                    + ",simIndex:" + simIndex + ",grpId:" + grpId);

            return succFlag;
        }

        /**
         * Move one member from group (fromGrpIds) to other groups (toGrpIds).
         *
         * @param subId
         * @param simIndex
         * @param fromGrpIds
         * @param toGrpIds
         * @return true succeed, otherwise false.
         */
        public static boolean moveContactToGroups(int subId, int simIndex, int[] fromGrpIds,
                int[] toGrpIds) {
            boolean succFlag = false;
            try {
                Log.d(TAG, "[moveUSIMGroupMember]subId:" + subId + "|simIndex:" + simIndex
                                + "|fromGrpIds:" + Arrays.toString(fromGrpIds) + "|toGrpIds:"
                                + Arrays.toString(toGrpIds));

                if (fromGrpIds != null && fromGrpIds.length > 0 && toGrpIds != null
                        && toGrpIds.length > 0) {
                    final IMtkIccPhoneBook iIccPhb = getIMtkIccPhoneBook();
                    if (iIccPhb != null) {
                        succFlag = iIccPhb.moveContactFromGroupsToGroups(subId, simIndex,
                                fromGrpIds, toGrpIds);
                    }
                } else {
                    Log.d(TAG, "moveUSIMGroupMember: illegal Group information");
                }
            } catch (android.os.RemoteException e) {
                Log.e(TAG, "[deleteUSIMGroup]RemoteException:" + e);
                succFlag = false;
            }
            Log.d(TAG, "[deleteUSIMGroup]result:" + succFlag + ",subId:" + subId
                    + ",simIndex:" + simIndex);

            return succFlag;
        }

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
            if (!SimCardUtils.isUsimType(workData.mSimType)
                    && !SimCardUtils.isCsimType(workData.mSimType)) {
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
                    Log.i(TAG, "[syncUSIMGroupContactsGroup]gName:" + Log.anonymize(gName) + "|gIndex: "
                            + gIndex);

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
                    + Groups.ACCOUNT_TYPE + "='" + workData.mSimType + " Account' AND "
                    + Groups.ACCOUNT_NAME + "=" + "'" + workData.mSimType+ subId + "'", null, null);
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
                            values.put(Groups.ACCOUNT_NAME, workData.mSimType + subId);
                            values.put(Groups.ACCOUNT_TYPE, workData.mSimType + " Account");
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

    public static class USIMGroupException extends Exception {
        private static final long serialVersionUID = 1L;

        public static final String ERROR_STR_GRP_NAME_OUTOFBOUND = "Group name out of bound";
        public static final String ERROR_STR_GRP_COUNT_OUTOFBOUND = "Group count out of bound";
        public static final String ERROR_STR_GRP_GENERIC_ERROR = "Group generic error";

        public static final int GROUP_NAME_OUT_OF_BOUND = 1;
        public static final int GROUP_NUMBER_OUT_OF_BOUND = 2;
        public static final int GROUP_GENERIC_ERROR = 3;
        public static final int GROUP_SIM_ABSENT = 4;
        // Exception type definination in framework.
        public static final int USIM_ERROR_NAME_LEN = MtkUsimPhoneBookManager.USIM_ERROR_NAME_LEN;
        public static final int USIM_ERROR_GROUP_COUNT =
                MtkUsimPhoneBookManager.USIM_ERROR_GROUP_COUNT;

        int mErrorType;
        int mSubId;

        public USIMGroupException(String msg, int errorType, int subId) {
            super(msg);
            mErrorType = errorType;
            mSubId = subId;
        }

        public int getErrorType() {
            return mErrorType;
        }

        public int getErrorSubId() {
            return mSubId;
        }

        @Override
        public String getMessage() {
            return "Details message: errorType:" + mErrorType + "\n" + super.getMessage();
        }

        public static int getErrorToastId(int errorType) {
            int retMsgId;
            switch (errorType) {
            case GROUP_NAME_OUT_OF_BOUND:
                retMsgId = R.string.usim_group_name_exceed_limit;
                break;
            case GROUP_NUMBER_OUT_OF_BOUND:
                retMsgId = R.string.usim_group_count_exceed_limit;
                break;
            case GROUP_SIM_ABSENT:
                retMsgId = R.string.callFailed_simError;
                break;
            default:
                retMsgId = R.string.generic_failure;
            }

            return retMsgId;
        }
    }

    /**
     * the group data can be stored in Bundle. add for ALPS01889745.
     */
    public static class ContactsGroupArrayData implements Parcelable {
        private int mSimIndex;
        private int mSimIndexPhoneOrSim;

        public int getmSimIndex() {
            return mSimIndex;
        }

        public int getmSimIndexPhoneOrSim() {
            return mSimIndexPhoneOrSim;
        }

        public int describeContents() {
            return 0;
        }

        public void writeToParcel(Parcel out, int flags) {
            out.writeInt(mSimIndex);
            out.writeInt(mSimIndexPhoneOrSim);
        }

        public static final Parcelable.Creator<ContactsGroupArrayData> CREATOR =
                new Parcelable.Creator<ContactsGroupArrayData>() {

            public ContactsGroupArrayData createFromParcel(Parcel in) {
                return new ContactsGroupArrayData(in);
            }

            public ContactsGroupArrayData[] newArray(int size) {
                return new ContactsGroupArrayData[size];
            }
        };

        public ContactsGroupArrayData(Parcel in) {
            mSimIndex = in.readInt();
            mSimIndexPhoneOrSim = in.readInt();
        }

        public ContactsGroupArrayData(int simIndex, int simIndexPhoneOrSim) {
            mSimIndex = simIndex;
            mSimIndexPhoneOrSim = simIndexPhoneOrSim;
        }

        public ContactsGroupArrayData initData(int simIndex, int simIndexPhoneorSim) {
            mSimIndex = simIndex;
            mSimIndexPhoneOrSim = simIndexPhoneorSim;
            return this;
        }
    }

    /**
     * the Pacelable HashMap. add for ALPS01889745.
     */
    public static class ParcelableHashMap implements Parcelable {
        private HashMap<Long, ContactsGroupArrayData> mMap;

        public void writeToParcel(Parcel out, int flags) {
            out.writeMap(mMap);
        }

        public HashMap<Long, ContactsGroupArrayData> getHashMap() {
            return mMap;
        }

        public int describeContents() {
            return 0;
        }

        public static final Parcelable.Creator<ParcelableHashMap> CREATOR =
                new Parcelable.Creator<ParcelableHashMap>() {

            public ParcelableHashMap createFromParcel(Parcel in) {
                return new ParcelableHashMap(in);
            }

            public ParcelableHashMap[] newArray(int size) {
                return new ParcelableHashMap[size];
            }
        };

        public ParcelableHashMap(HashMap<Long, ContactsGroupArrayData> map) {
            mMap = map;
        }

        public ParcelableHashMap(Parcel in) {
            mMap = in.readHashMap(ParcelableHashMap.class.getClassLoader());
        }

    }

}
