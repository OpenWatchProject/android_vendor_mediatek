/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.dialer.ext;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;
import android.widget.PopupMenu;

import java.util.List;

public interface IDialPadExtension {

    /**
     * for OP09.
     * called when handle special chars, plug-in should do customize chars handling here
     * @param context Context
     * @param input the input digits (text)
     * @return whether plug-in handled the chars
     * @internal
     */
    boolean handleChars(Context context, String input);

    /**
     * for OP09.
     * called when dialpad create view, plug-in should customize view here
     * @param inflater the host inflater
     * @param container parent ViewGroup
     * @param savedState previous state saved for create view
     * @param resultView the host view created
     * @return customized view by plug-in
     * @internal
     */
    View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedState,
            View resultView);

    /**
     * for OP01 SpeedDialMenu.
     * called when init option menu
     * @param context the activity who call this func
     * @param menu the activity option menu
     * @internal
     */
    void buildOptionsMenu(Context context, Menu menu);

    /**
     * for OP01.
     * called onViewCreated
     * @param activity the activity who call this func
     * @param view the dialpad view
     * @internal
     */
    void onViewCreated(Activity activity, View view);

    /** Define all the parameters order that pass to plugin. */
    public static final int VIDEO_BUTTON_PARAMS_INDEX_QUERY_STRING = 0;
    public static final int VIDEO_BUTTON_PARAMS_INDEX_CONTACT_LOOKUP_URI = 1;

   /**
     * for OP07.
     *@param appContext Context
     *@param intent Intent
     * @return false or true
     */
    boolean checkVideoSetting(Context appContext, Intent intent);

   /**
     * for OP18.
     *@param object Object
     */
    void customizeDefaultTAB(Object object);

    /**
     * for OP01 IMEI+ IMED.
     * Called when create device info list item view
     * @param viewGroup the view holder
     * @param showDecimal if need to show Decimal
     * @param showBarcode if need to show Barcode
     * @return if handled in plugin
     */
    public boolean handleDeviceIdDisplay(
            ViewGroup viewGroup, boolean showDecimal, boolean showBarcode);
}
