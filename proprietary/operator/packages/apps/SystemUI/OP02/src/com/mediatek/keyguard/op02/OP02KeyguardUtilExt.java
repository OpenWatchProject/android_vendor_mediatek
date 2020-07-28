/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.keyguard.op02;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.keyguard.ext.DefaultKeyguardUtilExt;
import com.mediatek.systemui.op02.R;

/**
 * Customize keyguard utils.
 */
public class OP02KeyguardUtilExt extends DefaultKeyguardUtilExt {
    public static final String TAG = "OP02KeyguardUtilExt";

    private Context mContext;

    /**
     * The constructor and to save the plugin's context for resource access.
     *
     * @param context the context of plugin.
     */
    public OP02KeyguardUtilExt(Context context) {
        super();
        mContext = context;
    }

    /**
     * Customize PIN/PUK lock view.
     *
     * @param phoneId the current phone ID.
     * @param imageView the default icon image view.
     * @param textView the default slot text view.
     */
    @Override
    public void customizePinPukLockView(int phoneId,
                    ImageView imageView, TextView textView) {
        Log.d(TAG, "customizePinPukLockView");
        if (TelephonyManager.getDefault().getPhoneCount() > 1) {
            textView.setVisibility(View.VISIBLE);
            if (phoneId == PhoneConstants.SIM_ID_1) {
                imageView.setImageDrawable(mContext.getResources().
                            getDrawable(R.drawable.ic_lockscreen_sim1));
            } else if (phoneId == PhoneConstants.SIM_ID_2) {
                imageView.setImageDrawable(mContext.getResources().
                            getDrawable(R.drawable.ic_lockscreen_sim2));
            }
        }
    }

    /**
     * Set the carrier text gravity.
     *
     * @param view the view that need to set gravity.
     */
    @Override
    public void customizeCarrierTextGravity(TextView view) {
        Log.d(TAG, "customizeCarrierTextGravity view = " + view);
        view.setGravity(Gravity.CENTER);
    }
}
