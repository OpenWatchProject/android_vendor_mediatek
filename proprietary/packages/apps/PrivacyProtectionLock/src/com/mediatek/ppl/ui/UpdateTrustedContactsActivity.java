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

package com.mediatek.ppl.ui;

import android.content.Intent;
import android.os.Bundle;
import android.view.MenuItem;
import android.widget.TextView;

import com.mediatek.ppl.PplService;
import com.mediatek.ppl.R;

public class UpdateTrustedContactsActivity extends SetupTrustedContactsActivity {

    private TextView mTitle;

    public void onDestroy() {
        if (null != mBinder) {
            mBinder.unregisterSensitiveActivity(this);
        }
        super.onDestroy();
    }

    @Override
    protected void onPropertyConfig() {
        setProperty(PROPERTY_NEED_SERVICE | PROPERTY_HAS_ACTIONBAR | PROPERTY_QUIT_BACKGROUND);
    }

    @Override
    protected void onRegisterEvent() {
        mEventReceiver.addAction(Intent.ACTION_SCREEN_OFF);
        mEventReceiver.addAction(PplService.Intents.UI_NO_SIM);
    }

    @Override
    protected void onPrepareLayout() {

        super.onPrepareLayout();
        mTitle = (TextView) findViewById(R.id.tv_setup_trusted_title);
    }

    @Override
    protected void onInitLayout() {
        mTitle.setText(R.string.title_update_trusted_contact);
        mNextButton.setText(R.string.button_confirm);
        super.onInitLayout();
    }

    @Override
    protected void onPplServiceConnected(Bundle saveInstanceState) {
        mBinder.registerSensitiveActivity(this);
        super.onPplServiceConnected(saveInstanceState);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (android.R.id.home == item.getItemId()) {
            gotoActivity(this, ControlPanelActivity.class);
            finish();
            return true;
        }
        return false;
    }

    @Override
    public void onBackPressed() {
        gotoActivity(this, ControlPanelActivity.class);
        finish();
    }

    @Override
    protected void onButtonClicked() {

        mBinder.saveTustedNumberList(getNumberList(), PplService.EDIT_TYPE_UPDATE);
        gotoActivity(this, ControlPanelActivity.class);
        finish();
    }
}
