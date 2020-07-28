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

import android.annotation.Nullable;
import android.annotation.ProductApi;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffColorFilter;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.os.Build;
import android.os.Parcel;
import android.os.Parcelable;
import android.telephony.Rlog;
import android.telephony.SubscriptionInfo;
import android.telephony.UiccAccessRule;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.DisplayMetrics;

/**
 * A Parcelable class for Subscription Information.
 */
public class MtkSubscriptionInfo extends SubscriptionInfo {

    /**
     * Size of text to render on the icon.
     */
    private static final int TEXT_SIZE = 16;

    /**
     * SIM Icon bitmap
     */
    private Bitmap mIconBitmap;

    private static final boolean IS_DEBUG_BUILD =
            Build.TYPE.equals("eng") || Build.TYPE.equals("userdebug");

    private static final String LOG_TAG = "MtkSubscriptionInfo";
    /**
     * Constructor.
     * @param id subid
     * @param iccId iccid
     * @param simSlotIndex slot index
     * @param displayName display name
     * @param carrierName carrier name
     * @param nameSource name source
     * @param iconTint icon number
     * @param number phone number
     * @param roaming roaming state
     * @param icon bitmap
     * @param mcc mcc
     * @param mnc mnc
     * @param countryIso countryIso
     * @param isEmbedded is embedded
     * @param accessRules access Rules
     * @param cardString card string
     * @hide
     */
    public MtkSubscriptionInfo(int id, String iccId, int simSlotIndex, CharSequence displayName,
            CharSequence carrierName, int nameSource, int iconTint, String number, int roaming,
            Bitmap icon, String mcc, String mnc, String countryIso, boolean isEmbedded,
            @Nullable UiccAccessRule[] accessRules, String cardString) {
        super(id, iccId, simSlotIndex, displayName, carrierName, nameSource, iconTint, number,
                roaming, icon, mcc, mnc, countryIso, isEmbedded, accessRules, cardString);
        mIconBitmap = icon;
    }

    /**
     * Constructor.
     * @param id subid
     * @param iccId iccid
     * @param simSlotIndex slot index
     * @param displayName display name
     * @param carrierName carrier name
     * @param nameSource name source
     * @param iconTint icon number
     * @param number phone number
     * @param roaming roaming state
     * @param icon bitmap
     * @param mcc mcc
     * @param mnc mnc
     * @param countryIso country Iso
     * @param isEmbedded is embedded
     * @param accessRules access Rules
     * @param cardString card string
     * @param isOpportunistic whether opportunistic
     * @param groupUUID group UUID
     * @param carrierId carrier ID
     * @param profileClass profile class
     * @hide
     */
    public MtkSubscriptionInfo(int id, String iccId, int simSlotIndex, CharSequence displayName,
            CharSequence carrierName, int nameSource, int iconTint, String number, int roaming,
            Bitmap icon, String mcc, String mnc, String countryIso, boolean isEmbedded,
            @Nullable UiccAccessRule[] accessRules, String cardString, boolean isOpportunistic,
            @Nullable String groupUUID, int carrierId, int profileClass) {
        super(id, iccId, simSlotIndex, displayName, carrierName, nameSource, iconTint, number,
                roaming, icon, mcc, mnc, countryIso, isEmbedded, accessRules, cardString,
                isOpportunistic, groupUUID, carrierId, profileClass);
        mIconBitmap = icon;
    }

    /**
     * Constructor.
     * @param id subid
     * @param iccId iccid
     * @param simSlotIndex slot index
     * @param displayName display name
     * @param carrierName carrier name
     * @param nameSource name source
     * @param iconTint icon number
     * @param number phone number
     * @param roaming roaming state
     * @param icon bitmap
     * @param mcc mcc
     * @param mnc mnc
     * @param countryIso country Iso
     * @param isEmbedded is embedded
     * @param accessRules access Rules
     * @param cardString card string
     * @param cardId card id
     * @param isOpportunistic whether opportunistic
     * @param groupUUID group UUID
     * @param isGroupDisabled whether group disabled
     * @param carrierId carrier ID
     * @param profileClass profile class
     * @param subType type of subscription
     * @param groupOwner Group owner
     * @hide
     */
    public MtkSubscriptionInfo(int id, String iccId, int simSlotIndex, CharSequence displayName,
            CharSequence carrierName, int nameSource, int iconTint, String number, int roaming,
            Bitmap icon, String mcc, String mnc, String countryIso, boolean isEmbedded,
            @Nullable UiccAccessRule[] accessRules, String cardString, int cardId,
            boolean isOpportunistic, @Nullable String groupUUID, boolean isGroupDisabled,
            int carrierId, int profileClass, int subType, @Nullable String groupOwner) {
        super(id, iccId, simSlotIndex, displayName, carrierName, nameSource, iconTint, number,
                roaming, icon, mcc, mnc, countryIso, isEmbedded, accessRules, cardString, cardId,
                isOpportunistic, groupUUID, isGroupDisabled, carrierId, profileClass,
                subType, groupOwner);
        mIconBitmap = icon;
    }

    /**
     * Creates and returns an icon {@code Bitmap} to represent this {@code SubscriptionInfo}
     * in a user interface.
     *
     * @param context A {@code Context} to get the {@code DisplayMetrics}s from.
     *
     * @return A bitmap icon for this {@code SubscriptionInfo}.
     */
    @ProductApi
    @Override
    public Bitmap createIconBitmap(Context context) {
        return createIconBitmap(context, -1, true);
    }

    /**
     * We need to set the icon color for disabled account @{
     * @param context
     * @param color the new color for the icon.
     * @return A bitmap icon.
     * @hide
     */
    @ProductApi
    public Bitmap createIconBitmap(Context context, int color) {
        return createIconBitmap(context, color, true);
    }

    /**
     * We need to set the icon color for disabled account @{
     * @param context
     * @param color the new color for the icon.
     * @param showSlotIndex use to indicate if slot index is needed
     * @return A bitmap icon.
     * @hide
     */
    @ProductApi
    public Bitmap createIconBitmap(Context context, int color, boolean showSlotIndex) {
        synchronized(this) {
            int width = mIconBitmap.getWidth();
            int height = mIconBitmap.getHeight();
            DisplayMetrics metrics = context.getResources().getDisplayMetrics();
            if (IS_DEBUG_BUILD) {
                Rlog.d(LOG_TAG, "mIconBitmap width:" + width + " height:" + height + " metrics:"
                        + metrics.toString());
            }
            // Create a new bitmap of the same size because it will be modified.
            Bitmap workingBitmap = Bitmap.createBitmap(metrics, width, height,
                    mIconBitmap.getConfig());

            Canvas canvas = new Canvas(workingBitmap);
            Paint paint = new Paint();

            // Tint the icon with the color.
            paint.setColorFilter(new PorterDuffColorFilter((color == -1 ? getIconTint() : color),
                    PorterDuff.Mode.SRC_ATOP));
            canvas.drawBitmap(mIconBitmap, 0, 0, paint);
            paint.setColorFilter(null);

            // Single SIM project might prefer no show slot index.
            if (showSlotIndex) {
                // Write the sim slot index.
                paint.setAntiAlias(true);
                paint.setTypeface(Typeface.create("sans-serif", Typeface.NORMAL));
                paint.setColor(Color.WHITE);
                // Set text size scaled by density
                paint.setTextSize(TEXT_SIZE * metrics.density);
                // Convert sim slot index to localized string
                final String index = String.format("%d", getSimSlotIndex() + 1);
                final Rect textBound = new Rect();
                paint.getTextBounds(index, 0, 1, textBound);
                final float xOffset = (width / 2.f) - textBound.centerX();
                final float yOffset = (height / 2.f) - textBound.centerY();
                canvas.drawText(index, xOffset, yOffset, paint);
            }

            return workingBitmap;
        }
    }

    public static final Parcelable.Creator<MtkSubscriptionInfo> CREATOR
            = new Parcelable.Creator<MtkSubscriptionInfo>() {
        @Override
        public MtkSubscriptionInfo createFromParcel(Parcel source) {
            int id = source.readInt();
            String iccId = source.readString();
            int simSlotIndex = source.readInt();
            CharSequence displayName = source.readCharSequence();
            CharSequence carrierName = source.readCharSequence();
            int nameSource = source.readInt();
            int iconTint = source.readInt();
            String number = source.readString();
            int dataRoaming = source.readInt();
            String mcc = source.readString();
            String mnc = source.readString();
            String countryIso = source.readString();
            Bitmap iconBitmap = source.readParcelable(Bitmap.class.getClassLoader());
            boolean isEmbedded = source.readBoolean();
            UiccAccessRule[] accessRules = source.createTypedArray(UiccAccessRule.CREATOR);
            String cardString = source.readString();
            int cardId = source.readInt();
            boolean isOpportunistic = source.readBoolean();
            String groupUUID = source.readString();
            boolean isGroupDisabled = source.readBoolean();
            int carrierid = source.readInt();
            int profileClass = source.readInt();
            int subType = source.readInt();
            String[] ehplmns = source.readStringArray();
            String[] hplmns = source.readStringArray();
            String groupOwner = source.readString();

            MtkSubscriptionInfo info = new MtkSubscriptionInfo(id, iccId, simSlotIndex,
                    displayName, carrierName, nameSource, iconTint, number, dataRoaming,
                    iconBitmap, mcc, mnc, countryIso, isEmbedded, accessRules, cardString,
                    cardId, isOpportunistic, groupUUID, isGroupDisabled, carrierid, profileClass,
                    subType, groupOwner);
            info.setAssociatedPlmns(ehplmns, hplmns);
            return info;
        }

        @Override
        public MtkSubscriptionInfo[] newArray(int size) {
            return new MtkSubscriptionInfo[size];
        }
    };

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        synchronized(this) {
            super.writeToParcel(dest, flags);
        }
    }

    @Override
    public int describeContents() {
        return 0;
    }

    /**
     * MTK accelerates updateSubscriptionInfoByIccId in MtkSubscriptionInfoUpdater.java, so
     * getCountryIso() is null when we updateSubscriptionInfo for the first time. And we don't
     * update the mCountryIso in SubscriptionInfo any more. So we can getCountryIso from
     * PROPERTY_ICC_OPERATOR_ISO_COUNTRY which is set when all sim records are loaded.
     * @return the name displayed to the user that identifies Subscription provider name
     * @hide
     */
    @Override
    public String getCountryIso() {
        String tmpCountryIso = super.getCountryIso();
        if (TextUtils.isEmpty(tmpCountryIso)) {
            tmpCountryIso = TelephonyManager.getDefault().getSimCountryIso(
                    super.getSubscriptionId());
        }
        return tmpCountryIso;
    }

    public static boolean isPrintableFullIccId() {
        return true;
    }
}
