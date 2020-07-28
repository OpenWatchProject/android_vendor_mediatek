/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.mms.plugin;

import android.app.Activity;
import android.content.Context;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.SearchView;

import com.mediatek.mms.ext.DefaultOpConversationListExt;
import com.mediatek.mms.plugin.R;

/**
 * Op01ConversationListExt.
 *
 */
public class Op01ConversationListExt extends DefaultOpConversationListExt {
    private static final String TAG = "Mms/Op01MmsConversationExt";

    private int mMenuSimSms = 102;

    private boolean mSmsEnabledStatus = true;

    /**
     * Construction.
     * @param context Context
     */
    public Op01ConversationListExt(Context context) {
        super(context);
    }

    @Override
    public void onCreateOptionsMenu(Menu menu) {
        Drawable sMenuIcon =
            getResources().getDrawable(R.drawable.ic_menu_sim_sms);

        menu.add(0, mMenuSimSms, 0, getString(R.string.menu_sim_sms)).setIcon(
                    sMenuIcon);

        Log.d(TAG, "Add Menu: " + getString(R.string.menu_sim_sms));
        return;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item, int actionSettingsId) {
        if (item.getItemId() == mMenuSimSms) {
            getHost().showSimSms();
            return true;
        } else {
            return false;
        }
    }

    @Override
    public void onPrepareOptionsMenu(Menu menu) {
        Log.d(TAG, "onPrepareOptionsMenu ");
        MenuItem item = menu.findItem(mMenuSimSms);
        if (item == null) {
            Log.e(TAG, "onPrepareOptionsMenu: menu item should not be null");
            return;
        }
        if (Op01MmsUtils.isSmsEnabled(this) && Op01MmsUtils.isSimInserted(this) &&
                !Op01MmsUtils.isAirplaneOn(this) && Op01MmsUtils.isSmsReady(this)) {
            item.setVisible(true);
            Log.d(TAG, "Menu: " + getString(R.string.menu_sim_sms) + " is visible");
        } else {
            item.setVisible(false);
            Log.d(TAG, "Menu: " + getString(R.string.menu_sim_sms) + " is invisible");
        }
    }

    @Override
    public boolean onCreate(Activity activity, Bundle savedInstanceState) {
        return true;
    }

    @Override
    public void onResume(boolean isSmsEnabled) {
        if (isSmsEnabled == mSmsEnabledStatus) {
            Log.d(TAG, "onResume, isSmsEnabled == mOldSmsEnabledStatus," +
                          "no need refresh");
            return ;
        } else {
            Log.d(TAG, "onResume, isSmsEnabled != mOldSmsEnabledStatus," +
                "need refresh");
            mSmsEnabledStatus = isSmsEnabled;
        }
        return;
    }
}

