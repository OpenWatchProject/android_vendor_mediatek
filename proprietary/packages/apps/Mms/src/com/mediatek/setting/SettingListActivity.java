/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.setting;

import android.app.ActionBar;
import android.app.ListActivity;
import android.app.role.RoleManager;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.android.mms.MmsConfig;
import com.android.mms.R;

import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.ext.IOpSettingListActivityExt;
import com.mediatek.mms.ipmessage.IIpSettingListActivityExt;
import com.mediatek.opmsg.util.OpMessageUtils;


public class SettingListActivity extends ListActivity {
    private static final String TAG = "SettingListActivity";

    /// Move preference strings from Google file to MTK SettingListActivity. @{
    public static final String TEXT_SIZE = "message_font_size";
    public static final String SMS_MANAGE_SIM_MESSAGES  = "pref_key_manage_sim_messages";
    public static final String SDCARD_MESSAGE_DIR_PATH = "//message//";
    public static final String SMS_DELIVERY_REPORT_MODE = "pref_key_sms_delivery_reports";
    public static final String AUTO_RETRIEVAL           = "pref_key_mms_auto_retrieval";
    /// @}

    private boolean inited = false;
    // KK migration, for default MMS function. @{
    private boolean mIsSmsEnabled = true;
    private View mHeaderView;
    private TextView mSetDeafultMmsTile;
    private TextView mSetDeafultMmsSummary;
    /// @}

    private IOpSettingListActivityExt mOpSettingListActivityExt;

    private IIpSettingListActivityExt mIpSettingListActivity;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setTitle(getResources().getString(R.string.menu_preferences));
        setContentView(R.layout.setting_list);
        /// KK migration, for default MMS function. @{
        LayoutInflater inflater = (LayoutInflater) this
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mHeaderView = (View) inflater.inflate(R.layout.setting_header, getListView(), false);
        mSetDeafultMmsTile = (TextView) mHeaderView.findViewById(R.id.set_default_mms_title);
        mSetDeafultMmsSummary = (TextView) mHeaderView.findViewById(R.id.set_default_mms_summary);
        getListView().addHeaderView(mHeaderView);
        /// @}
        // add for ipmessage
        mIpSettingListActivity =
            IpMessageUtils.getIpMessagePlugin(this).getIpSettingListActivity();
        mIpSettingListActivity.onIpCreate(this);

        Log.d(TAG, "new mOpSettingListActivityExt");
        mOpSettingListActivityExt = OpMessageUtils.getOpMessagePlugin().
            getOpSettingListActivityExt();
        Log.d(TAG, "new mOpSettingListActivityExt = " + mOpSettingListActivityExt);
        mOpSettingListActivityExt.onOpCreate(this, getApplicationContext());
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume ");
        if (isNeedUpdateView()) {
            setAdapter();
            // Update the views state, enable or disable.
            mUpdateViewStateHandler.sendEmptyMessage(0);
        }
    }

    private boolean isNeedUpdateView() {
        boolean needUpdate = false;
        if (!inited) {
            needUpdate = true;
            inited = true;
        }
        boolean isSmsEnable = MmsConfig.isSmsEnabled(this);
        if (isSmsEnable != mIsSmsEnabled) {
            mIsSmsEnabled = isSmsEnable;
            needUpdate = true;
        }
        // add for ipmessage
        needUpdate = mIpSettingListActivity.isIpNeedUpdateView(needUpdate);
        return needUpdate;
    }

    private void setAdapter() {
        String[] settingList = new String[] {
                getResources().getString(R.string.pref_setting_sms),
                getResources().getString(R.string.pref_setting_mms),
                getResources().getString(R.string.pref_setting_notification),
                getResources().getString(R.string.pref_setting_general)
        };
        // add for ipmessage
        settingList = mIpSettingListActivity.setIpAdapter(settingList);


        Log.d(TAG, "before addImportExportItem");
        settingList = mOpSettingListActivityExt.setAdapter(settingList);

        setListAdapter(new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1,
                settingList));

    }

    private final Handler mUpdateViewStateHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            int lastItemPosition = getListView().getChildCount() - 1;
            boolean isListDrawCompleted = (getListView().getChildAt(lastItemPosition) != null);
            Log.d(TAG, "draw completed? " + isListDrawCompleted + " enable: " + mIsSmsEnabled);
            if (!isListDrawCompleted) {
                mUpdateViewStateHandler.sendEmptyMessageDelayed(0, 5);
            } else {
                if (mIsSmsEnabled) {
                    mSetDeafultMmsTile.setText(R.string.pref_title_sms_enabled);
                    mSetDeafultMmsSummary.setText(R.string.pref_summary_sms_enabled);
                } else {
                    mSetDeafultMmsTile.setText(R.string.pref_title_sms_disabled);
                    mSetDeafultMmsSummary.setText(R.string.pref_summary_sms_disabled);
                }
                for (int i = 1; i <= lastItemPosition; i++) {
                    if (getListView().getChildAt(i) != null) {
                        getListView().getChildAt(i).setEnabled(mIsSmsEnabled);
                    }
                }
                // add for ipmessage
                mIpSettingListActivity.handleIpMessage();
            }
        }
    };
    /// @}

    @Override
    protected void onListItemClick(ListView parent, View view, int position, long id) {
        /// KK migration, for default MMS function. @{
        if (view == mHeaderView) {
            setDefaultMms();
            return;
        } else if (mIsSmsEnabled) {
            // add for ipmessage
            if (mIpSettingListActivity.onIpListItemClick(position)) {
                return;
            }
            switch (position) {
                case 1:
                    Intent smsPreferenceIntent = new Intent(SettingListActivity.this,
                            SmsPreferenceActivity.class);
                    startActivity(smsPreferenceIntent);
                    break;
                case 2:
                    Intent mmsPreferenceIntent = new Intent(SettingListActivity.this,
                            MmsPreferenceActivity.class);
                    startActivity(mmsPreferenceIntent);
                    break;
                case 3:
                    Intent notificationPreferenceIntent = new Intent(SettingListActivity.this,
                            NotificationPreferenceActivity.class);
                    startActivity(notificationPreferenceIntent);
                    break;
                case 4:
                    Intent generalPreferenceIntent = new Intent(SettingListActivity.this,
                            GeneralPreferenceActivity.class);
                    startActivity(generalPreferenceIntent);
                    break;
                case 5:
                    mOpSettingListActivityExt.onListItemClick(position);
                    break;
                default:
                    break;
            }
        }

        /// @}
    }

    /// KK migration, for default MMS function. @{
    public void setDefaultMms() {
        Log.d(TAG, "setDefaultMms mIsSmsEnabled: " + mIsSmsEnabled);
        final RoleManager roleManager = this.getSystemService(RoleManager.class);
        Intent intent = roleManager.createRequestRoleIntent(RoleManager.ROLE_SMS); //new Intent();
        //Log.d(TAG, "setDefaultMms startActivityForResult intent: " + intent);
        startActivityForResult(intent, 0);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case android.R.id.home:
            // The user clicked on the Messaging icon in the action bar. Take them back from
            // wherever they came from
            finish();
            return true;
        default:
            break;
        }
        return false;
    }
}
