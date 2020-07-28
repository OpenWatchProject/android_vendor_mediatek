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

package com.mediatek.internal.telephony;

import android.app.AppOpsManager;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Binder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.provider.Settings;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.RadioAccessFamily;
import android.telephony.Rlog;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.UiccAccessRule;
import android.text.TextUtils;
import android.text.format.Time;
import android.util.Log;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.ITelephonyRegistry;
import com.android.internal.telephony.MultiSimSettingController;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.ProxyController;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyPermissions;
import com.android.internal.telephony.metrics.TelephonyMetrics;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.UiccController;

import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.uicc.MtkSpnOverride;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Objects;
import java.util.Set;

/**
 * SubscriptionController to provide an inter-process communication to
 * access Sms in Icc.
 *
 * Any setters which take subId, slotIndex or phoneId as a parameter will throw an exception if the
 * parameter equals the corresponding INVALID_XXX_ID or DEFAULT_XXX_ID.
 *
 * All getters will lookup the corresponding default if the parameter is DEFAULT_XXX_ID. Ie calling
 * getPhoneId(DEFAULT_SUB_ID) will return the same as getPhoneId(getDefaultSubId()).
 *
 * Finally, any getters which perform the mapping between subscriptions, slots and phones will
 * return the corresponding INVALID_XXX_ID if the parameter is INVALID_XXX_ID. All other getters
 * will fail and return the appropriate error value. Ie calling
 * getSlotIndex(INVALID_SUBSCRIPTION_ID) will return INVALID_SIM_SLOT_INDEX and calling
 * getSubInfoForSubscriber(INVALID_SUBSCRIPTION_ID) will return null.
 *
 */
public class MtkSubscriptionController extends SubscriptionController {
    private static final String LOG_TAG = "MtkSubCtrl";
    private static final boolean ENGDEBUG = TextUtils.equals(android.os.Build.TYPE, "eng");
    static final int MAX_LOCAL_LOG_LINES = 500; // TODO: Reduce to 100 when 17678050 is fixed
    private static final boolean sIsOP01
                = "OP01".equals(SystemProperties.get("persist.vendor.operator.optr", ""));
    private static final boolean sIsOP02
                = "OP02".equals(SystemProperties.get("persist.vendor.operator.optr", ""));

    /** The singleton instance. */
    private static MtkSubscriptionController sMtkInstance = null;

    private static Intent sStickyIntent = null;

    private boolean mIsReady = false;

    private int lastPhoneId = SubscriptionManager.DEFAULT_SIM_SLOT_INDEX;

    private static final int PROJECT_SIM_NUM = TelephonyManager.getDefault().getPhoneCount();
    // Record the iccid when MtkSubscriptionController is initialized.
    static String initIccid[] = new String[PROJECT_SIM_NUM];

    // This function creates a single instance of this class, this mapping AOSP init
    protected static MtkSubscriptionController mtkInit(Phone phone) {
        synchronized (MtkSubscriptionController.class) {
            if (sMtkInstance == null) {
                sMtkInstance = new MtkSubscriptionController(phone);
                if (DBG) Rlog.d(LOG_TAG, "mtkInit, sMtkInstance = " + sMtkInstance);
            } else {
                Log.wtf(LOG_TAG, "init() called multiple times!  sMtkInstance = " + sMtkInstance);
            }
            return sMtkInstance;
        }
    }

    // This function creates a single instance of this class, this mapping AOSP init
    protected static MtkSubscriptionController mtkInit(Context c, CommandsInterface[] ci) {
        synchronized (MtkSubscriptionController.class) {
            if (sMtkInstance == null) {
                sMtkInstance = new MtkSubscriptionController(c);
                if (DBG) Rlog.d(LOG_TAG, "mtkInit, sMtkInstance = " + sMtkInstance);
                MtkSubscriptionControllerEx.MtkInitStub(c);
            } else {
                Log.wtf(LOG_TAG, "init() called multiple times!  sMtkInstance = " + sMtkInstance);
            }
            return sMtkInstance;
        }
    }

    protected MtkSubscriptionController(Context c) {
        super(c);
    }

    @Override
    protected /*private*/ void clearSlotIndexForSubInfoRecords() {
        // Record old subinfo to set detect type.
        for (int i = 0; i < PROJECT_SIM_NUM; i++) {
            Cursor cursor = mContext.getContentResolver().query(SubscriptionManager.CONTENT_URI,
                    null, SubscriptionManager.SIM_SLOT_INDEX + "=?",
                    new String[]{String.valueOf(i)}, null);

            try {
                if (cursor != null && cursor.moveToFirst()) {
                    initIccid[i] = cursor.getString(cursor.getColumnIndexOrThrow(
                            SubscriptionManager.ICC_ID));
                } else {
                    initIccid[i] = "N/A";
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        }
        super.clearSlotIndexForSubInfoRecords();
    }

    public static MtkSubscriptionController getMtkInstance() {
        synchronized (MtkSubscriptionController.class) {
            return sMtkInstance;
        }
    }

    protected MtkSubscriptionController(Phone phone) {
        super(phone);
    }

    @Override
    public void notifySubscriptionInfoChanged() {
        ITelephonyRegistry tr = ITelephonyRegistry.Stub.asInterface(ServiceManager.getService(
                "telephony.registry"));
        try {
            if (DBG) {
                logd("notifySubscriptionInfoChanged:");
            }
            tr.notifySubscriptionInfoChanged();
        } catch (RemoteException ex) {
            // Should never happen because its always available.
        }

        // FIXME: Remove if listener technique accepted.
        broadcastSimInfoContentChanged(null);

        MultiSimSettingController.getInstance().notifySubscriptionInfoChanged();
        TelephonyMetrics metrics = TelephonyMetrics.getInstance();
        List<SubscriptionInfo> subInfoList = getActiveSubscriptionInfoList(
                mContext.getOpPackageName());
        if (subInfoList == null) {
            subInfoList = new ArrayList<>();
        }
        metrics.updateActiveSubscriptionInfoList(
                Collections.unmodifiableList(subInfoList));
    }

    /**
     * New SubInfoRecord instance and fill in detail info
     * @param cursor
     * @return the query result of desired SubInfoRecord
     */
    @Override
    protected SubscriptionInfo getSubInfoRecord(Cursor cursor) {
        int id = cursor.getInt(cursor.getColumnIndexOrThrow(
                SubscriptionManager.UNIQUE_KEY_SUBSCRIPTION_ID));
        String iccId = cursor.getString(cursor.getColumnIndexOrThrow(
                SubscriptionManager.ICC_ID));
        int simSlotIndex = cursor.getInt(cursor.getColumnIndexOrThrow(
                SubscriptionManager.SIM_SLOT_INDEX));
        String displayName = cursor.getString(cursor.getColumnIndexOrThrow(
                SubscriptionManager.DISPLAY_NAME));
        String carrierName = cursor.getString(cursor.getColumnIndexOrThrow(
                SubscriptionManager.CARRIER_NAME));
        int nameSource = cursor.getInt(cursor.getColumnIndexOrThrow(
                SubscriptionManager.NAME_SOURCE));
        int iconTint = cursor.getInt(cursor.getColumnIndexOrThrow(
                SubscriptionManager.COLOR));
        String number = cursor.getString(cursor.getColumnIndexOrThrow(
                SubscriptionManager.NUMBER));
        int dataRoaming = cursor.getInt(cursor.getColumnIndexOrThrow(
                SubscriptionManager.DATA_ROAMING));
        // Get the blank bitmap for this SubInfoRecord
        Bitmap iconBitmap = BitmapFactory.decodeResource(mContext.getResources(),
                com.android.internal.R.drawable.ic_sim_card_multi_24px_clr);
        String mcc = cursor.getString(cursor.getColumnIndexOrThrow(
                SubscriptionManager.MCC_STRING));
        String mnc = cursor.getString(cursor.getColumnIndexOrThrow(
                SubscriptionManager.MNC_STRING));
        String ehplmnsRaw = cursor.getString(cursor.getColumnIndexOrThrow(
                SubscriptionManager.EHPLMNS));
        String hplmnsRaw = cursor.getString(cursor.getColumnIndexOrThrow(
                SubscriptionManager.HPLMNS));
        String[] ehplmns = ehplmnsRaw == null ? null : ehplmnsRaw.split(",");
        String[] hplmns = hplmnsRaw == null ? null : hplmnsRaw.split(",");

        // cardId is the private ICCID/EID string, also known as the card string
        String cardId = cursor.getString(cursor.getColumnIndexOrThrow(
                SubscriptionManager.CARD_ID));
        String countryIso = cursor.getString(cursor.getColumnIndexOrThrow(
                SubscriptionManager.ISO_COUNTRY_CODE));
        // publicCardId is the publicly exposed int card ID
        int publicCardId = mUiccController.convertToPublicCardId(cardId);
        boolean isEmbedded = cursor.getInt(cursor.getColumnIndexOrThrow(
                SubscriptionManager.IS_EMBEDDED)) == 1;
        int carrierId = cursor.getInt(cursor.getColumnIndexOrThrow(
                SubscriptionManager.CARRIER_ID));
        UiccAccessRule[] accessRules;
        if (isEmbedded) {
            accessRules = UiccAccessRule.decodeRules(cursor.getBlob(
                    cursor.getColumnIndexOrThrow(SubscriptionManager.ACCESS_RULES)));
        } else {
            accessRules = null;
        }
        boolean isOpportunistic = cursor.getInt(cursor.getColumnIndexOrThrow(
                SubscriptionManager.IS_OPPORTUNISTIC)) == 1;
        String groupUUID = cursor.getString(cursor.getColumnIndexOrThrow(
                SubscriptionManager.GROUP_UUID));
        int profileClass = cursor.getInt(cursor.getColumnIndexOrThrow(
                SubscriptionManager.PROFILE_CLASS));
        int subType = cursor.getInt(cursor.getColumnIndexOrThrow(
                SubscriptionManager.SUBSCRIPTION_TYPE));
        String groupOwner = getOptionalStringFromCursor(cursor, SubscriptionManager.GROUP_OWNER,
                /*defaultVal*/ null);
        if (VDBG) {
            String iccIdToPrint = MtkSubscriptionInfo.givePrintableIccid(iccId);
            String cardIdToPrint = MtkSubscriptionInfo.givePrintableIccid(cardId);
            logd("[getSubInfoRecord] id:" + id + " iccid:" + iccIdToPrint + " simSlotIndex:"
                    + simSlotIndex + " carrierid:" + carrierId + " displayName:" + displayName
                    + " nameSource:" + nameSource + " iconTint:" + iconTint
                    + " dataRoaming:" + dataRoaming + " mcc:" + mcc + " mnc:" + mnc
                    + " countIso:" + countryIso + " isEmbedded:"
                    + isEmbedded + " accessRules:" + Arrays.toString(accessRules)
                    + " cardId:" + cardIdToPrint + " publicCardId:" + publicCardId
                    + " isOpportunistic:" + isOpportunistic + " groupUUID:" + groupUUID
                    + " profileClass:" + profileClass + " subscriptionType: " + subType);
        }

        // If line1number has been set to a different number, use it instead.
        String line1Number = mTelephonyManager.getLine1Number(id);
        if (!TextUtils.isEmpty(line1Number) && !line1Number.equals(number)) {
            number = line1Number;
        }

        MtkSubscriptionInfo info = new MtkSubscriptionInfo(id, iccId, simSlotIndex, displayName,
                carrierName, nameSource, iconTint, number, dataRoaming, iconBitmap, mcc, mnc,
                countryIso, isEmbedded, accessRules, cardId, publicCardId, isOpportunistic,
                groupUUID, false /* isGroupDisabled */, carrierId, profileClass, subType,
                groupOwner);
        info.setAssociatedPlmns(ehplmns, hplmns);
        return info;
    }

    /**
     * Add a new subscription info record, if needed.
     * @param uniqueId This is the unique identifier for the subscription within the specific
     *                 subscription type.
     * @param displayName human-readable name of the device the subscription corresponds to.
     * @param slotIndex value for {@link SubscriptionManager#SIM_SLOT_INDEX}
     * @param subscriptionType the type of subscription to be added.
     * @return 0 if success, < 0 on error.
     */
    @Override
    public int addSubInfo(String uniqueId, String displayName, int slotIndex,
            int subscriptionType) {
        if (DBG) {
            String iccIdStr = uniqueId;
            if (!isSubscriptionForRemoteSim(subscriptionType)) {
                iccIdStr = MtkSubscriptionInfo.givePrintableIccid(uniqueId);
            }
            logdl("[addSubInfoRecord]+ iccid: " + iccIdStr
                    + ", slotIndex: " + slotIndex
                    + ", subscriptionType: " + subscriptionType);
        }

        enforceModifyPhoneState("addSubInfo");

        // Now that all security checks passes, perform the operation as ourselves.
        final long identity = Binder.clearCallingIdentity();
        try {
            if (uniqueId == null) {
                if (DBG) {
                    logdl("[addSubInfo]- null iccId");
                }
                return -1;
            }

            ContentResolver resolver = mContext.getContentResolver();
            String selection = SubscriptionManager.ICC_ID + "=?";
            String[] args;
            if (isSubscriptionForRemoteSim(subscriptionType)) {
                selection += " AND " + SubscriptionManager.SUBSCRIPTION_TYPE + "=?";
                args = new String[]{uniqueId, Integer.toString(subscriptionType)};
            } else {
                selection += " OR " + SubscriptionManager.ICC_ID + "=?" + " OR "
                        + SubscriptionManager.ICC_ID + "=?";
                args = new String[]{uniqueId, IccUtils.getDecimalSubstring(uniqueId),
                        uniqueId.toLowerCase()};
            }
            Cursor cursor = resolver.query(SubscriptionManager.CONTENT_URI,
                    new String[]{SubscriptionManager.UNIQUE_KEY_SUBSCRIPTION_ID,
                            SubscriptionManager.SIM_SLOT_INDEX, SubscriptionManager.NAME_SOURCE,
                            SubscriptionManager.ICC_ID, SubscriptionManager.CARD_ID},
                    selection, args, null);

            boolean setDisplayName = false;
            try {
                boolean recordsDoNotExist = (cursor == null || !cursor.moveToFirst());
                if (isSubscriptionForRemoteSim(subscriptionType)) {
                    if (recordsDoNotExist) {
                        // create a Subscription record
                        slotIndex = SubscriptionManager.SLOT_INDEX_FOR_REMOTE_SIM_SUB;
                        Uri uri = insertEmptySubInfoRecord(uniqueId, displayName,
                                slotIndex, subscriptionType);
                        if (DBG) {
                            logd("[addSubInfoRecord] New record created: " + uri);
                        }
                    } else {
                        if (DBG) {
                            logdl("[addSubInfoRecord] Record already exists");
                        }
                    }
                } else {  // Handle Local SIM devices
                    if (recordsDoNotExist) {
                        setDisplayName = true;
                        Uri uri = insertEmptySubInfoRecord(uniqueId, slotIndex);
                        if (DBG) {
                            logdl("[addSubInfoRecord] New record created: " + uri);
                        }
                    } else { // there are matching records in the database for the given ICC_ID
                        int subId = cursor.getInt(0);
                        int oldSimInfoId = cursor.getInt(1);
                        int nameSource = cursor.getInt(2);
                        String oldIccId = cursor.getString(3);
                        String oldCardId = cursor.getString(4);
                        ContentValues value = new ContentValues();

                        if (slotIndex != oldSimInfoId) {
                            value.put(SubscriptionManager.SIM_SLOT_INDEX, slotIndex);
                        }

                        if (oldIccId != null && ((oldIccId.length() < uniqueId.length()
                                && (oldIccId.equals(IccUtils.getDecimalSubstring(uniqueId))))
                                || ((!uniqueId.toLowerCase().equals(uniqueId)) && (
                                uniqueId.toLowerCase().equals(oldIccId))))) {
                            value.put(SubscriptionManager.ICC_ID, uniqueId);
                        }

                        UiccCard card = mUiccController.getUiccCardForPhone(slotIndex);
                        if (card != null) {
                            String cardId = card.getCardId();
                            if (cardId != null && cardId != oldCardId) {
                                value.put(SubscriptionManager.CARD_ID, cardId);
                            }
                        }

                        if (value.size() > 0) {
                            resolver.update(SubscriptionManager.getUriForSubscriptionId(subId),
                                    value, null, null);
                        }

                        if (DBG) {
                            logdl("[addSubInfoRecord] Record already exists");
                        }
                    }
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }

            selection = SubscriptionManager.SIM_SLOT_INDEX + "=?";
            args = new String[] {String.valueOf(slotIndex)};
            if (isSubscriptionForRemoteSim(subscriptionType)) {
                selection = SubscriptionManager.ICC_ID + "=? AND "
                        + SubscriptionManager.SUBSCRIPTION_TYPE + "=?";
                args = new String[]{uniqueId, Integer.toString(subscriptionType)};
            }
            cursor = resolver.query(SubscriptionManager.CONTENT_URI, null,
                    selection, args, null);
            try {
                if (cursor != null && cursor.moveToFirst()) {
                    do {
                        int subId = cursor.getInt(cursor.getColumnIndexOrThrow(
                                SubscriptionManager.UNIQUE_KEY_SUBSCRIPTION_ID));
                        // If sSlotIndexToSubIds already has the same subId for a slotIndex/phoneId,
                        // do not add it.
                        if (addToSubIdList(slotIndex, subId, subscriptionType)) {
                            // TODO While two subs active, if user deactivats first
                            // one, need to update the default subId with second one.

                            // FIXME: Currently we assume phoneId == slotIndex which in the future
                            // may not be true, for instance with multiple subs per slot.
                            // But is true at the moment.
                            int subIdCountMax = getActiveSubInfoCountMax();
                            int defaultSubId = getDefaultSubId();
                            if (DBG) {
                                logdl("[addSubInfoRecord]"
                                        + " sSlotIndexToSubIds.size=" + sSlotIndexToSubIds.size()
                                        + " slotIndex=" + slotIndex + " subId=" + subId
                                        + " defaultSubId=" + defaultSubId
                                        + " simCount=" + subIdCountMax);
                            }

                            // Set the default sub if not set or if single sim device
                            if (!isSubscriptionForRemoteSim(subscriptionType)) {
                                if (!SubscriptionManager.isValidSubscriptionId(defaultSubId)
                                        || subIdCountMax == 1
                                        || !isActiveSubId(defaultSubId)
                                        || !isActiveSubId(mDefaultFallbackSubId)
                                        || (mDefaultFallbackSubId == subId && lastPhoneId !=
                                        slotIndex)) {
                                    logdl("setting default fallback subid to " + subId);
                                    setDefaultFallbackSubId(subId, subscriptionType);
                                    lastPhoneId = slotIndex;
                                }
                                // If single sim device, set this subscription as the default for
                                // everything
                                if (subIdCountMax == 1) {
                                    if (DBG) {
                                        logdl("[addSubInfoRecord] one sim set defaults to subId="
                                                + subId);
                                    }
                                    setDefaultDataSubId(subId);
                                    setDefaultSmsSubId(subId);
                                    setDefaultVoiceSubId(subId);
                                }
                            } else {
                                updateDefaultSubIdsIfNeeded(subId, subscriptionType);
                            }
                        } else {
                            if (DBG) {
                                logdl("[addSubInfoRecord] current SubId is already known, "
                                        + "IGNORE");
                            }
                        }
                        if (DBG) {
                            logdl("[addSubInfoRecord] hashmap(" + slotIndex + "," + subId + ")");
                        }
                    } while (cursor.moveToNext());
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }

            // Refresh the Cache of Active Subscription Info List
            refreshCachedActiveSubscriptionInfoList();

            if (isSubscriptionForRemoteSim(subscriptionType)) {
                notifySubscriptionInfoChanged();
            } else {  // Handle Local SIM devices
                // Set Display name after sub id is set above so as to get valid simCarrierName
                int subId = getSubIdUsingPhoneId(slotIndex);
                if (!SubscriptionManager.isValidSubscriptionId(subId)) {
                    if (DBG) {
                        logdl("[addSubInfoRecord]- getSubId failed invalid subId = " + subId);
                    }
                    return -1;
                }
                if (setDisplayName) {
                    String simCarrierName = mTelephonyManager.getSimOperatorName(subId);
                    String nameToSet;

                    // Take MVNO into account.
                    String simNumeric = mTelephonyManager.getSimOperatorNumeric(subId);
                    String simMvnoName;
                    // SPN file may not get and wait to update in handleSimLoaded.
                    if ("20404".equals(simNumeric) && TextUtils.isEmpty(simCarrierName)) {
                        simMvnoName = "";
                    } else {
                        simMvnoName = MtkSpnOverride.getInstance().lookupOperatorNameForDisplayName(
                                subId, simNumeric, true, mContext);
                    }
                    if (ENGDEBUG && DBG) {
                        logd("[addSubInfoRecord]- simNumeric: " + simNumeric +
                                ", simMvnoName: " + simMvnoName);
                    }
                    if (!TextUtils.isEmpty(simMvnoName)) {
                        nameToSet = simMvnoName;
                    } else {
                        if (!TextUtils.isEmpty(simCarrierName)) {
                            nameToSet = simCarrierName;
                        } else {
                            nameToSet = "CARD " + Integer.toString(slotIndex + 1);
                        }
                    }

                    ContentValues value = new ContentValues();
                    value.put(SubscriptionManager.DISPLAY_NAME, nameToSet);
                    resolver.update(SubscriptionManager.getUriForSubscriptionId(subId), value,
                            null, null);

                    if (DBG) {
                        logdl("[addSubInfoRecord] sim name = " + nameToSet);
                    }
                }

                // Refresh the Cache of Active Subscription Info List
                refreshCachedActiveSubscriptionInfoList();

                // Once the records are loaded, notify DcTracker
                sPhones[slotIndex].updateDataConnectionTracker();

                if (DBG) {
                    logdl("[addSubInfoRecord]- info size=" + sSlotIndexToSubIds.size());
                }
            }

        } finally {
            Binder.restoreCallingIdentity(identity);
        }
        return 0;
    }

    @Override
    public int getSlotIndex(int subId) {
        if (VDBG) printStackTrace("[getSlotIndex] subId=" + subId);

        if (subId == SubscriptionManager.DEFAULT_SUBSCRIPTION_ID) {
            logd("[getSlotIndex]+ subId == SubscriptionManager.DEFAULT_SUBSCRIPTION_ID");
            subId = getDefaultSubId();
        }
        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            if (DBG) logd("[getSlotIndex]- subId invalid");
            return SubscriptionManager.INVALID_SIM_SLOT_INDEX;
        }

        int size = sSlotIndexToSubIds.size();

        if (size == 0) {
            if (DBG) {
                logd("[getSlotIndex]- size == 0, return SIM_NOT_INSERTED instead, subId =" + subId);
            }
            return SubscriptionManager.SIM_NOT_INSERTED;
        }

        for (Entry<Integer, ArrayList<Integer>> entry : sSlotIndexToSubIds.entrySet()) {
            int sim = entry.getKey();
            ArrayList<Integer> subs = entry.getValue();

            if (subs != null && subs.contains(subId)) {
                if (VDBG) {
                    logv("[getSlotIndex]- return =" + sim + ", subId = " + subId);
                }
                return sim;
            }
        }

        if (DBG) {
            logd("[getSlotIndex]- return INVALID_SIM_SLOT_INDEX, subId = " + subId);
        }
        return SubscriptionManager.INVALID_SIM_SLOT_INDEX;
    }

    @Override
    public int getPhoneId(int subId) {
        if (VDBG) printStackTrace("[getPhoneId] subId=" + subId);
        int phoneId;

        if (subId == SubscriptionManager.DEFAULT_SUBSCRIPTION_ID) {
            subId = getDefaultSubId();
            if (DBG) logdl("[getPhoneId] asked for default subId=" + subId);
        }

        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            if (subId > SubscriptionManager.DUMMY_SUBSCRIPTION_ID_BASE
                    - getActiveSubInfoCountMax()) {
                phoneId = (int) (SubscriptionManager.DUMMY_SUBSCRIPTION_ID_BASE  - subId);
            } else {
                phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
            }

            if (VDBG) {
                //logdl("[getPhoneId]- invalid subId return="
                //        + SubscriptionManager.INVALID_PHONE_INDEX);
                logdl("[getPhoneId]- invalid subId = " + subId + " return = " + phoneId);
            }
            //return SubscriptionManager.INVALID_PHONE_INDEX;
            return phoneId;
        }

        int size = sSlotIndexToSubIds.size();
        if (size == 0) {
            phoneId = mDefaultPhoneId;
            //if (DBG) logdl("[getPhoneId]- no sims, returning default phoneId=" + phoneId);
            if (DBG) {
                logd("[getPhoneId]- no sims, returning default phoneId=" + phoneId +
                    ", subId" + subId);
            }
            return phoneId;
        }

        // FIXME: Assumes phoneId == slotIndex
        for (Entry<Integer, ArrayList<Integer>> entry: sSlotIndexToSubIds.entrySet()) {
            int sim = entry.getKey();
            ArrayList<Integer> subs = entry.getValue();

            if (subs != null && subs.contains(subId)) {
                if (VDBG) logdl("[getPhoneId]- found subId=" + subId + " phoneId=" + sim);
                return sim;
            }
        }

        phoneId = mDefaultPhoneId;
        if (DBG) {
            logdl("[getPhoneId]- subId=" + subId + " not found return default phoneId=" + phoneId);
        }
        return phoneId;

    }

    /**
     * @return the number of records cleared
     */
    @Override
    public int clearSubInfo() {
        enforceModifyPhoneState("clearSubInfo");

        // Now that all security checks passes, perform the operation as ourselves.
        final long identity = Binder.clearCallingIdentity();
        try {
            int size = sSlotIndexToSubIds.size();

            if (size == 0) {
                if (DBG) logdl("[clearSubInfo]- no simInfo size=" + size);
                return 0;
            }

            setReadyState(false);

            sSlotIndexToSubIds.clear();
            if (DBG) logdl("[clearSubInfo]- clear size=" + size);
            return size;
        } finally {
            Binder.restoreCallingIdentity(identity);
        }
    }

    @Override
    public void setDefaultDataSubId(int subId) {
        setDefaultDataSubIdWithResult(subId);
    }

    /**
     * Add notify notifySubscriptionInfoChanged with propKey.
     * @param subId Subscription Id of Subscription
     * @param propKey Column name in database associated with SubscriptionInfo
     * @param propValue Value to store in DB for particular subId & column name
     * @return number of rows updated.
     * @hide
     */
    @Override
    public int setSubscriptionProperty(int subId, String propKey, String propValue) {
        int ret = super.setSubscriptionProperty(subId, propKey, propValue);

        if (ENGDEBUG) {
        logdl("[setSubscriptionProperty] propKey=" + propKey + ", propValue = " + propValue +
                ", subId=" + subId + ", Binder.getCallingPid and Binder.getCallingUid are "
                + Binder.getCallingPid() + "," + Binder.getCallingUid());
        }

        final long token = Binder.clearCallingIdentity();

        try {
            if (ret != 0) {
                Intent intent = new Intent(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
                int phoneId = getPhoneId(subId);
                List<SubscriptionInfo> subInfoList = getActiveSubscriptionInfoList(
                        mContext.getOpPackageName());
                int subCount = (subInfoList == null) ? 0 : subInfoList.size();

                putSubinfoRecordUpdatedExtra(intent, phoneId,
                        MtkSubscriptionManager.EXTRA_VALUE_NOCHANGE, subCount, propKey);

                notifySubscriptionInfoChanged(intent);
            }

            return ret;
        } finally {
            Binder.restoreCallingIdentity(token);
        }
    }

    // Please add override APIs before this API and add MTK APIs after this API.
    // Notes, please place the MTK code by functions, inner class and member order

    // This function broadcast when SubscriptionInfo has changed, this extend AOSP
    // broadcastSimInfoContentChanged function
    private void broadcastSimInfoContentChanged(Intent intentExt) {
        Intent intent = new Intent(TelephonyIntents.ACTION_SUBINFO_CONTENT_CHANGE);
        mContext.sendBroadcast(intent);
        intent = new Intent(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        // mContext.sendBroadcast(intent);

        // Data module expect only receive this message when mapping relationship changed, but
        // not content changed. We add one more extra field to indicated no changed.
        if (intentExt == null) {
            List<SubscriptionInfo> subInfoList = getActiveSubscriptionInfoList(
                    mContext.getOpPackageName());
            int subCount = (subInfoList == null) ? 0 : subInfoList.size();

            putSubinfoRecordUpdatedExtra(intent, -1, MtkSubscriptionManager.EXTRA_VALUE_NOCHANGE,
                    subCount, null);
        }

        synchronized (MtkSubscriptionController.class) {
            sStickyIntent = ((intentExt == null) ? intent : intentExt);
            int detectedType = sStickyIntent.getIntExtra(
                    MtkSubscriptionManager.INTENT_KEY_DETECT_STATUS, 0);
            int phoneId = sStickyIntent.getIntExtra(PhoneConstants.PHONE_KEY, -1);
            if (ENGDEBUG) {
                logd("broadcast intent ACTION_SUBINFO_RECORD_UPDATED with detectType:" +
                        detectedType + ", phoneId:" + phoneId);
            }
            mContext.sendStickyBroadcast(sStickyIntent);
        }
    }

    /**
     * Clear an subscriptionInfo to subinfo database if needed by updating slot index to invalid.
     * @param slotIndex the slot which the SIM is removed
     */
    public void clearSubInfoRecord(int slotIndex) {
        setReadyState(false);
        super.clearSubInfoRecord(slotIndex);
    }

    /**
     * Set the default data subscription id.
     * @param subId sub id
     * @return set status
     */
    // This function is a substitiute of setDefaultDataSubId
    public boolean setDefaultDataSubIdWithResult(int subId) {
        enforceModifyPhoneState("setDefaultDataSubIdWithResult");

        final long identity = Binder.clearCallingIdentity();
        try {
            if (subId == SubscriptionManager.DEFAULT_SUBSCRIPTION_ID) {
                throw new RuntimeException(
                        "setDefaultDataSubIdWithResult called with DEFAULT_SUB_ID");
            }

            ProxyController proxyController = ProxyController.getInstance();
            int len = sPhones.length;
            logdl("[setDefaultDataSubIdWithResult] num phones=" + len + ", subId=" + subId
                    + ", Binder.getCallingPid and Binder.getCallingUid are "
                    + Binder.getCallingPid() + "," + Binder.getCallingUid());

            if (proxyController != null && SubscriptionManager.isValidSubscriptionId(subId)) {
                // Only re-map modems if the new default data sub is valid
                RadioAccessFamily[] rafs = new RadioAccessFamily[len];
                int targetPhoneId = 0;
                boolean atLeastOneMatch = false;
                for (int phoneId = 0; phoneId < len; phoneId++) {
                    Phone phone = sPhones[phoneId];
                    int raf;
                    int id = phone.getSubId();
                    if (id == subId) {
                        // TODO Handle the general case of N modems and M subscriptions.
                        raf = proxyController.getMaxRafSupported();
                        atLeastOneMatch = true;
                        targetPhoneId = phoneId;
                    } else {
                        // TODO Handle the general case of N modems and M subscriptions.
                        raf = proxyController.getMinRafSupported();
                    }
                    logdl("[setDefaultDataSubIdWithResult] phoneId=" + phoneId + " subId=" + id
                            + " RAF=" + raf);
                    rafs[phoneId] = new RadioAccessFamily(phoneId, raf);
                }
                if (atLeastOneMatch) {
                    proxyController.setRadioCapability(rafs);
                } else {
                    if (DBG) {
                        logdl("[setDefaultDataSubIdWithResult] no valid subId's found"
                                + " - not updating.");
                    }
                }
            }

            // FIXME is this still needed?
            updateAllDataConnectionTrackers();

            int previousDefaultSub = getDefaultSubId();
            Settings.Global.putInt(mContext.getContentResolver(),
                    Settings.Global.MULTI_SIM_DATA_CALL_SUBSCRIPTION, subId);
            MultiSimSettingController.getInstance().notifyDefaultDataSubChanged();
            broadcastDefaultDataSubIdChanged(subId);
            if (previousDefaultSub != getDefaultSubId()) {
                sendDefaultChangedBroadcast(getDefaultSubId());
            }

            return true;
        } finally {
            Binder.restoreCallingIdentity(identity);
        }
    }

    /**
     * Get the SubscriptionInfo with the subId key.
     * @param subId The unique SubscriptionInfo key in database
     * @return SubscriptionInfo, maybe null if not found
     */
    public MtkSubscriptionInfo getSubscriptionInfo(String callingPackage, int subId) {
        String pkgName = callingPackage;

        if (callingPackage == null) {
            pkgName = mContext.getOpPackageName();
        }

        if (!TelephonyPermissions.checkCallingOrSelfReadPhoneState(mContext, subId, pkgName,
                "getSubscriptionInfo")) {
            return null;
        }

        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            logd("[getSubscriptionInfo]- invalid subId, subId =" + subId);
            return null;
        }

        // Now that all security checks passes, perform the operation as ourselves.
        final long identity = Binder.clearCallingIdentity();
        try {
            Cursor cursor = mContext.getContentResolver().query(SubscriptionManager.CONTENT_URI,
                    null, SubscriptionManager.UNIQUE_KEY_SUBSCRIPTION_ID + "=?",
                    new String[] {Long.toString(subId)}, null);
            try {
                if (cursor != null) {
                    if (cursor.moveToFirst()) {
                        MtkSubscriptionInfo si = (MtkSubscriptionInfo)getSubInfoRecord(cursor);
                        if (si != null) {
                            if (DBG) {
                                logd("[getSubscriptionInfo]+ subId=" + subId + ", subInfo=" + si);
                            }
                            return si;
                        }
                    }
                } else {
                    logd("[getSubscriptionInfo]- Query fail");
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        } finally {
            Binder.restoreCallingIdentity(identity);
        }

        if (DBG) {
            logd("[getSubscriptionInfo]- subId=" + subId + ",subInfo=null");
        }
        return null;
    }


    /**
     * Get the active SubscriptionInfo associated with the iccId.
     * @param iccId the IccId of SIM card
     * @return SubscriptionInfo, maybe null if not found
     */
    public MtkSubscriptionInfo getSubscriptionInfoForIccId(String callingPackage, String iccId) {
        String pkgName = callingPackage;

        if (callingPackage == null) {
            pkgName = mContext.getOpPackageName();
        }

        if (!TelephonyPermissions.checkCallingOrSelfReadPhoneState(mContext,
                SubscriptionManager.INVALID_SUBSCRIPTION_ID, pkgName,
                "getSubscriptionInfoForIccId")) {
            return null;
        }

        if (iccId == null) {
            logd("[getSubscriptionInfoForIccId]- null iccid");
            return null;
        }

        // Now that all security checks passes, perform the operation as ourselves.
        final long identity = Binder.clearCallingIdentity();
        try {
            Cursor cursor = mContext.getContentResolver().query(SubscriptionManager.CONTENT_URI,
                    null, SubscriptionManager.ICC_ID + "=?", new String[] {iccId}, null);

            try {
                if (cursor != null) {
                    while (cursor.moveToNext()) {
                        MtkSubscriptionInfo si = (MtkSubscriptionInfo)getSubInfoRecord(cursor);
                        if (si != null) {
                            if (DBG) {
                                logd("[getSubscriptionInfoForIccId]+ iccId="
                                        + MtkSubscriptionInfo.givePrintableIccid(iccId)
                                        + ", subInfo=" + si);
                            }
                            return si;
                        }
                    }
                } else {
                    logd("[getSubscriptionInfoForIccId]- Query fail");
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        } finally {
            Binder.restoreCallingIdentity(identity);
        }

        if (DBG) {
            logd("[getSubscriptionInfoForIccId]- iccId=" + iccId + ",subInfo=null");
        }
        return null;
    }

    /**
     * Set deafult data sub ID without invoking capability switch.
     * @param subId the default data sub ID
     */
    public void setDefaultDataSubIdWithoutCapabilitySwitch(int subId) {
        if (subId == SubscriptionManager.DEFAULT_SUBSCRIPTION_ID) {
            throw new RuntimeException(
                    "setDefaultDataSubIdWithoutCapabilitySwitch called with DEFAULT_SUB_ID");
        }
        if (ENGDEBUG && DBG) {
            logd("[setDefaultDataSubIdWithoutCapabilitySwitch] subId=" + subId
                    + ", Binder.getCallingPid and Binder.getCallingUid are "
                    + Binder.getCallingPid() + "," + Binder.getCallingUid());
        }
        // FIXME is this still needed?
        updateAllDataConnectionTrackers();

        int previousDefaultSub = getDefaultSubId();
        Settings.Global.putInt(mContext.getContentResolver(),
                Settings.Global.MULTI_SIM_DATA_CALL_SUBSCRIPTION, subId);
        MultiSimSettingController.getInstance().notifyDefaultDataSubChanged();
        broadcastDefaultDataSubIdChanged(subId);
        if (previousDefaultSub != getDefaultSubId()) {
            sendDefaultChangedBroadcast(getDefaultSubId());
        }
    }

    /**
     * Notify the changed of subscription info.
     * @param intent intent message.
     */
    public void notifySubscriptionInfoChanged(Intent intent) {
        ITelephonyRegistry tr = ITelephonyRegistry.Stub.asInterface(ServiceManager.getService(
                "telephony.registry"));
        try {
            // if (DBG) logd("notifySubscriptionInfoChanged:");
            setReadyState(true);
            tr.notifySubscriptionInfoChanged();
        } catch (RemoteException ex) {
            // Should never happen because its always available.
        }

        // FIXME: Remove if listener technique accepted.
        broadcastSimInfoContentChanged(intent);

        MultiSimSettingController.getInstance().notifySubscriptionInfoChanged();
        TelephonyMetrics metrics = TelephonyMetrics.getInstance();

        List<SubscriptionInfo> subInfoList = getActiveSubscriptionInfoList(
                mContext.getOpPackageName());
        if (subInfoList == null) {
            subInfoList = new ArrayList<>();
        }
        metrics.updateActiveSubscriptionInfoList(
                Collections.unmodifiableList(subInfoList));
    }

    /**
     * Query if sub module always initialization done.
     * @return ready or not
     */
    public boolean isReady() {
        if (ENGDEBUG) {
            logd("[isReady]- " + mIsReady);
        }
        return mIsReady;
    }

    /**
     * Set sub module initialization state.
     *  @param isReady state
     */
    public void setReadyState(boolean isReady) {
        if (ENGDEBUG) {
            logd("[setReadyState]- " + isReady);
        }
        mIsReady = isReady;
    }

    /**
     * Get default fall back sub Id.
     *  @return mDefaultFallbackSubId
     */
    public int getDefaultFallbackSubId() {
        return mDefaultFallbackSubId;
    }

    /**
     * Get default fall back sub Id.
     *  @param intent intent message.
     *  @param phoneId phone Id.
     *  @param detectedType card detected type.
     *  @param subCount SUB count.
     *  @param propKey prop key value.
     */
    public void putSubinfoRecordUpdatedExtra(Intent intent, int phoneId, int detectedType,
            int subCount, String propKey) {
        logd("putSubinfoRecordUpdatedExtra: phoneId = " + phoneId + " detectedType = "
                + detectedType + " subCount = " + subCount + " propKey = " + propKey);

        // Put phone id and sub Id.
        int[] subIds = SubscriptionManager.getSubId(phoneId);
        if (subIds != null && subIds.length > 0) {
            SubscriptionManager.putPhoneIdAndSubIdExtra(intent, phoneId, subIds[0]);
        } else {
            logd("putSubinfoRecordUpdatedExtra: no valid subs");
            intent.putExtra(PhoneConstants.PHONE_KEY, phoneId);
            intent.putExtra(PhoneConstants.SLOT_KEY, phoneId);
        }

        // Put detected type.
        intent.putExtra(MtkSubscriptionManager.INTENT_KEY_DETECT_STATUS, detectedType);

        // Put sub count.
        intent.putExtra(MtkSubscriptionManager.INTENT_KEY_SIM_COUNT, subCount);

        // Put propkey
        if (propKey != null) {
            intent.putExtra(MtkSubscriptionManager.INTENT_KEY_PROP_KEY, propKey);
        } else {
            intent.putExtra(MtkSubscriptionManager.INTENT_KEY_PROP_KEY, "");
        }
    }

    private void logv(String msg) {
        Rlog.v(LOG_TAG, msg);
    }

    private void logd(String msg) {
        Rlog.d(LOG_TAG, msg);
    }

    private void loge(String msg) {
        Rlog.e(LOG_TAG, msg);
    }

    private void logdl(String msg) {
        logd(msg);
        mLocalLog.log(msg);
    }

    @Override
    public void setDefaultVoiceSubId(int subId) {
        super.setDefaultVoiceSubId(subId);

        PhoneAccountHandle newHandle =
                subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID
                        ? null : mTelephonyManager.getPhoneAccountHandleForSubscriptionId(
                        subId);
        TelecomManager telecomManager = mContext.getSystemService(TelecomManager.class);
        PhoneAccountHandle currentHandle = telecomManager.getUserSelectedOutgoingPhoneAccount();
        if (newHandle == null && currentHandle == null) {
            telecomManager.setUserSelectedOutgoingPhoneAccount(null);
            logd("[setDefaultVoiceSubId] setUserSelectedOutgoingPhoneAccount(null)" +
                    " when SIM plug out");
        }
    }
}
