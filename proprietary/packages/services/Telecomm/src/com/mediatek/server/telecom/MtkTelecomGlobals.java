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

package com.mediatek.server.telecom;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.widget.Toast;
import com.mediatek.server.telecom.ext.ExtensionManager;

/**
 * Supply functions of show toast.
 */
public class MtkTelecomGlobals {
    private static final String TAG = MtkTelecomGlobals.class.getSimpleName();
    private static final int SHOW_TOAST = 1;
    private static MtkTelecomGlobals sInstance;
    private final Context mContext;
    private Handler mMainThreadHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == SHOW_TOAST) {
                Toast.makeText(mContext, ((String) msg.obj), Toast.LENGTH_SHORT).show();
            }
        }
    };

    private MtkTelecomGlobals(Context applicationContext) {
        mContext = applicationContext;
        ExtensionManager.registerApplicationContext(mContext);
    }

    private void onCreate() {
        ExtensionManager.registerApplicationContext(mContext);
    }

    /**
     * Get Context.
     * @return Context The current Context.
     */
    public Context getContext() {
        return mContext;
    }

    /**
     * Show toast for the string resource.
     * @param resId The resource id.
     */
    public void showToast(int resId) {
        String text = mContext.getString(resId);
        showToast(text);
    }

    /**
     * Show toast for string resource.
     * @param text The toast content.
     */
    public void showToast(String text) {
        mMainThreadHandler.obtainMessage(SHOW_TOAST, text).sendToTarget();
    }

    /**
     * Create MtkTelecomGlobals instance.
     * @param applicationContext The current Context.
     */
    synchronized public static void createInstance(Context applicationContext) {
        if (sInstance != null) {
            return;
        }
        sInstance = new MtkTelecomGlobals(applicationContext);
        sInstance.onCreate();
    }

    /**
     * Return MtkTelecomGlobals instance.
     * @return text The MtkTelecomGlobals instance.
     */
    public static MtkTelecomGlobals getInstance() {
        return sInstance;
    }
}
