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

package com.mediatek.mms.ext;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageView;
import android.widget.SearchView;
import android.widget.TextView;

import com.mediatek.mms.callback.IConversationListHost;

public interface IOpConversationListExt {

    /**
     * init conversation plugin.
     *
     * @param host    when click menu, from host can change mode or show sim message
     * @return
     * @internal
     */
    void init(IConversationListHost host);

    /**
     * add option menu in plugin
     *
     * @param menu     the option menu of conversation list
     * @param base      the base option menu id of conversation, operator can add menus
     *                  these ids must bigger than base id.
     * @return
     * @internal
     */
    void onCreateOptionsMenu(Menu menu);

    /**
     * @param menu     the option menu of conversation list
     * method is called in activity's onPrepareOptionsMenu
     * Plugin side can control the status of menus added by itself.
     * @internal
     */
    void onPrepareOptionsMenu(Menu menu);

    /**
     * Returns whether the menu item is handled.
     *
     * @param menu     the option menu of conversation list
     * @internal
     */
    boolean onOptionsItemSelected(MenuItem item, int actionSettingsId);

    /**
     *
     * @param activity Activity
     * @param savedInstanceState Bundle
     * @return boolean
     * @internal
     */
    boolean onCreate(Activity activity, Bundle savedInstanceState);

    /**
     * @internal
     */
    boolean onClickSmsPromoBanner();

    /**
     * @internal
     */
    public Intent onSmsPromoBannerViewClick(Intent intent);

    /**
     * @internal
     */
    public void initSmsPromoBanner(ImageView imageView, TextView smsPromoBannerTitle,
            TextView smsPromoBannermessage, ApplicationInfo appInfo, PackageManager pm);

    /**
     * @internal
     */
    void onResume(boolean isSmsEnabled);
}

