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
package com.mediatek.contacts.lettertiles;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.telephony.SubscriptionManager;
//import android.util.Log;

import com.android.contacts.ContactPhotoManager.DefaultImageRequest;
import com.android.contacts.R;
import com.android.contacts.lettertiles.LetterTileDrawable;

import com.mediatek.contacts.simcontact.SubInfoUtils;
import com.mediatek.contacts.util.ContactsCommonListUtils;
import com.mediatek.contacts.util.Log;

import java.util.HashMap;

public class LetterTileDrawableEx extends LetterTileDrawable {
    private static final String TAG = LetterTileDrawableEx.class.getSimpleName();

    /**
     * This height ratio is just a experience value. Avatar icon will take up
     * the ratio height of View.
     */
    private static float SIM_AVATAR_HEIGHT_RATIO = 0.32f;
    private static float SIM_AVATAR_WIDTH_RATIO = 0.32f;
    private static float SDN_LOCKED_RATIO = 0.3f;
    /**
     * This width ratio is just a experience value. Avatar icon will take up the
     * ratio width of View.
     */
    private static int SIM_ALPHA = 240;

    private static Bitmap DEFAULT_SIM_SDN_AVATAR_LOCKED;

    private long mSdnPhotoId = 0;
    private Paint mSimPaint;
    private static final Paint sPaint = new Paint();
    private int mBackgroundColor;
    private int mSubId = 0;

    public LetterTileDrawableEx(Resources res) {
        super(res);
        mSimPaint = new Paint();
        mSimPaint.setAntiAlias(true);
        // mSimPaint.setAlpha(SIM_ALPHA);
        mSimPaint.setDither(true);
        mBackgroundColor = res.getColor(R.color.background_primary);
        if (DEFAULT_SIM_SDN_AVATAR_LOCKED == null) {
            DEFAULT_SIM_SDN_AVATAR_LOCKED = BitmapFactory.decodeResource(res,
                    R.drawable.sim_indicator_sim_locked);
        }
    }

    public void setSIMProperty(DefaultImageRequest request) {
        if (request.subId > 0) {
            mSubId = request.subId;
            mSdnPhotoId = request.photoId;
        }
    }

    class IconEntry {
        public int iconTint;
        public Bitmap iconBitmap;

    }

    private static HashMap<Integer, IconEntry> BITMAP_ICONS = new HashMap<Integer, IconEntry>();

    public static void clearSimIconBitmaps() {
        BITMAP_ICONS.clear();
    }

    public void initSimIconBitmaps() {
        BITMAP_ICONS.clear();
        int[] subIds = SubInfoUtils.getActiveSubscriptionIdList();
        /// M: ALPS02807938. subIds maybe null when no basic perimission.@{
        if (subIds == null) {
            Log.e(TAG, "[initSimIconBitmaps] maybe has no basic permissions!");
            return;
        }
        /// @}
        int size = subIds.length;
        for (int i = 0; i < size; i++) {
            IconEntry icon = new IconEntry();
            icon.iconBitmap = SubInfoUtils.getIconBitmap(subIds[i]);
            icon.iconTint = SubInfoUtils.getColorUsingSubId(subIds[i]);
            int soltId = SubscriptionManager.getSlotIndex(subIds[i]);
            BITMAP_ICONS.put(soltId, icon);
        }
    }

    private Bitmap getIconBitmapUsingSubId(int subId) {
        int soltId = SubscriptionManager.getSlotIndex(subId);
        IconEntry iconEntry = BITMAP_ICONS.get(soltId);
        Bitmap bitmap = null;
        if (iconEntry != null) {
            bitmap = iconEntry.iconBitmap;
        }
        return bitmap;
    }

    public Bitmap getIconBitmapCache(int subId) {
        // Icon color change by setting, we refresh bitmaps icon cache.
        Bitmap bitmap = null;
        int soltId = SubscriptionManager.getSlotIndex(subId);
        IconEntry iconEntry = BITMAP_ICONS.get(soltId);
        if (iconEntry == null || SubInfoUtils.iconTintChange(iconEntry.iconTint, subId)) {
            Log.d(TAG, "icon tint changed need to re-get sim icons bitmap");
            initSimIconBitmaps();
        }
        bitmap = getIconBitmapUsingSubId(subId);
        return bitmap;
    }

    @Override
    public void draw(Canvas canvas) {
        super.draw(canvas);
        if (SubInfoUtils.checkSubscriber(mSubId)) {
            Bitmap bitmap = getIconBitmapCache(mSubId);
            if (bitmap != null) {
                drawSimAvatar(bitmap, bitmap.getWidth(), bitmap.getHeight(), canvas);
            } else {
                Log.e(TAG, "[draw]bitmap is null !!! subId=" + mSubId);
            }
            // [SDN] For SDN icon.
            if (ContactsCommonListUtils.isSdnPhotoId(mSdnPhotoId)) {
                bitmap = DEFAULT_SIM_SDN_AVATAR_LOCKED;
                drawSdnAvatar(bitmap, bitmap.getWidth(), bitmap.getHeight(), canvas);
            }
        }
    }

    private void drawSimAvatar(final Bitmap bitmap, final int width, final int height,
            final Canvas canvas) {
        // rect for sim avatar
        final Rect destRect = copyBounds();
        destRect.set((int) (destRect.right -getScale() * SIM_AVATAR_WIDTH_RATIO * destRect.width()),
                (int) (destRect.bottom - getScale() * SIM_AVATAR_HEIGHT_RATIO * destRect.height()),
                destRect.right, destRect.bottom);
        sPaint.setColor(mBackgroundColor);
        sPaint.setAntiAlias(true);
        float radius = destRect.width() / 2 * 1.2f;
        canvas.drawCircle(destRect.centerX(), destRect.centerY(), radius, sPaint);
        canvas.drawBitmap(bitmap, null, destRect, mSimPaint);
    }

    private void drawSdnAvatar(final Bitmap bitmap, final int width, final int height,
            final Canvas canvas) {
        // rect for sim avatar
        final Rect destRect = copyBounds();

        destRect.set((int) (destRect.left), (int) (destRect.top + getScale() * SDN_LOCKED_RATIO
                * destRect.height()),
                (int) (destRect.left + getScale() * SDN_LOCKED_RATIO * destRect.width()),
                (int) (destRect.top + 2.0f * getScale() * SDN_LOCKED_RATIO * destRect.height()));

        canvas.drawBitmap(bitmap, null, destRect, mSimPaint);
    }
}
