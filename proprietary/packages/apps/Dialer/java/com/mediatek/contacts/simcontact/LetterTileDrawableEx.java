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
package com.mediatek.contacts.simcontact;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Build;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionManager.OnSubscriptionsChangedListener;
import android.util.Log;

import com.android.dialer.contactphoto.ContactPhotoManager.DefaultImageRequest;
import com.android.contacts.common.R;
import com.android.dialer.lettertile.LetterTileDrawable;

import com.mediatek.contacts.util.ContactsCommonListUtils;

import java.util.HashMap;

public class LetterTileDrawableEx extends LetterTileDrawable {
    private static final String TAG = "LetterTileDrawableEx";

    /**
     * This height ratio is just a experience value. Avatar icon will take up
     * the ratio height of View.
     */
    private static float SIM_AVATAR_HEIGHT_RATIO = 0.32f;
    private static float SIM_AVATAR_WIDTH_RATIO = 0.32f;
    private static float SDN_LOCKED_RATIO = 0.3f;

    private static Bitmap DEFAULT_SIM_AVATAR;
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
        mSimPaint.setDither(true);
        //mBackgroundColor = res.getColor(R.color.background_primary);
        //Feng , tmp fix @{
        mBackgroundColor = 0;
        // @}
        if (DEFAULT_SIM_AVATAR == null) {
            DEFAULT_SIM_AVATAR = BitmapFactory.decodeResource(res,
                    R.drawable.sim_indicator_default_small);
            DEFAULT_SIM_SDN_AVATAR_LOCKED = BitmapFactory.decodeResource(res,
                    R.drawable.sim_indicator_sim_locked);
        }
    }

    public void setSIMProperty(DefaultImageRequest request) {
        if (request.subId > 0) {
            mSubId = request.subId;
            mSdnPhotoId = request.photoId;
        }
        Log.d(TAG, "[setSIMProperty]request subId = " + request.subId + ",request photoId: "
                + request.photoId);
    }

    private static HashMap<Integer, Bitmap> BITMAP_ICONS = new HashMap<Integer, Bitmap>();

    private void initSimIconBitmaps() {
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
            BITMAP_ICONS.put(subIds[i], SubInfoUtils.getIconBitmap(subIds[i]));
        }
    }

    private Bitmap getIconBitmapUsingSubId(int subId) {
        return BITMAP_ICONS.get(subId);
    }

    private Bitmap getIconBitmapCache(int subId) {
        if (BITMAP_ICONS.isEmpty()) {
            initSimIconBitmaps();
        }
        return getIconBitmapUsingSubId(subId);
    }

    @Override
    public void draw(Canvas canvas) {
        super.draw(canvas);
        if (SubInfoUtils.checkSubscriber(mSubId)) {
            Bitmap bitmap = getIconBitmapCache(mSubId);
            if (bitmap != null) {
                drawSimAvatar(bitmap, bitmap.getWidth(), bitmap.getHeight(), canvas);
            } else {
                Log.e(TAG, "[draw]bitmap is null !!!");
            }
            // For SDN icon.
            if (mSdnPhotoId == ContactsCommonListUtils.SIM_PHOTO_ID_SDN_LOCKED) {
                bitmap = DEFAULT_SIM_SDN_AVATAR_LOCKED;
                drawSdnAvatar(bitmap, bitmap.getWidth(), bitmap.getHeight(), canvas);
            }
        }
    }

    private void drawSimAvatar(final Bitmap bitmap, final int width, final int height,
            final Canvas canvas) {
        // rect for sim avatar
        final Rect destRect = copyBounds();
        destRect.set((int) (destRect.right - scale * SIM_AVATAR_WIDTH_RATIO * destRect.width()),
                (int) (destRect.bottom - scale * SIM_AVATAR_HEIGHT_RATIO * destRect.height()),
                destRect.right, destRect.bottom);
        sPaint.setColor(mBackgroundColor);
        sPaint.setAntiAlias(true);
        float radius = destRect.width() / 2 * 1.2f;
        Log.d(TAG, "bitmap: " + bitmap + ", width: " + width + ", radius: " + radius);
        canvas.drawCircle(destRect.centerX(), destRect.centerY(), radius, sPaint);
        canvas.drawBitmap(bitmap, null, destRect, mSimPaint);
    }

    private void drawSdnAvatar(final Bitmap bitmap, final int width, final int height,
            final Canvas canvas) {
        // rect for sim avatar
        final Rect destRect = copyBounds();

        destRect.set((int) (destRect.left), (int) (destRect.top + scale * SDN_LOCKED_RATIO
                * destRect.height()),
                (int) (destRect.left + scale * SDN_LOCKED_RATIO * destRect.width()),
                (int) (destRect.top + 2.0f * scale * SDN_LOCKED_RATIO * destRect.height()));

        canvas.drawBitmap(bitmap, null, destRect, mSimPaint);
    }

    //For dev
    private int getSlotIndex(int subId) {
        int slotId = -1;
        //M: Build version will greater than O @{
        //if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            slotId = SubscriptionManager.getSlotIndex(subId);
       /* } else {
            Context context = GlobalEnv.getApplicationContext();
            Object result = CompatUtils.invokeMethod(SubscriptionManager.from(context), "getSlotId",
                    new Class<?>[]{int.class}, new Object[]{subId});
            if (result != null) {
                slotId = (int) result;
            }
        }*/
        /// @}
        return slotId;
    }

    ///M: add to listen subinfo changed, clear cache when it happens. @{
    private static OnSubscriptionsChangedListener sListener = new OnSubscriptionsChangedListener() {
        private boolean mIsFirst = true; //ignore the first changed.
        @Override
        public void onSubscriptionsChanged() {
            if (mIsFirst) {
                Log.d(TAG, "Subscriptions changed, ignore first callback");
                mIsFirst = false;
                return;
            }
            Log.d(TAG, "Subscriptions changed, clear bitmap icons cache");
            BITMAP_ICONS.clear();
        }
    };

    static {
        Context context = GlobalEnv.getApplicationContext();
        SubscriptionManager.from(context).addOnSubscriptionsChangedListener(sListener);

        /* sometimes onSubscriptionsChanged will not be invoked when language changed, so listen
        locale changed broadcast */
        IntentFilter filter = new IntentFilter(Intent.ACTION_LOCALE_CHANGED);
        context.registerReceiver(new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Log.d(TAG, "Local changed, clear bitmap icons cache");
                BITMAP_ICONS.clear();
            }
        }, filter);
    }
    /// @}
}
