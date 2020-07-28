/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 * the prior written permission of MediaTek inc. and/or its licensor, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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
package com.mediatek.camera.common.setting;

import android.preference.PreferenceFragment;

/**
 * Setting that has entry in the setting view tree must implement this interface.
 * This interface defines the Api to load setting view when setting view tree is created,
 * refresh setting view when setting view tree is resumed, and unload view resource when
 * setting view tree is destroyed.
 */
public interface ICameraSettingView {
    /**
     * This parameter define that setting view is disable or hidden when it isn't selectable.
     * True means its view should be hidden when it isn't selectable, false means its view
     * should be disable when it isn't selectable.
     */
    public static final boolean JUST_DISABLE_UI_WHEN_NOT_SELECTABLE = false;

    /**
     * In this method, setting should add its setting view layout to setting view
     * tree. Setting view layout is defined the in the xml by using {@link
     * android.preference.Preference}, so using {@code addPreferencesFromResource} in
     * {@link PreferenceFragment} class to add setting view to setting view tree. This
     * method will be called when preference fragment is created.
     *
     * @param fragment The instance of {@link PreferenceFragment}.
     */
    void loadView(PreferenceFragment fragment);

    /**
     * In this method, setting can refresh its view, like refresh summary. This
     * method will be called when preference fragment is resumed.
     */
    void refreshView();

    /**
     * In this method, setting can unload view resource. This method will be
     * called when preference fragment is destroyed.
     */
    void unloadView();

    /**
     * Sets whether this Preference is enabled.
     *
     * @param enable Set true to enable it.
     */
    void setEnabled(boolean enable);

    /**
     * Checks whether this view should be enabled in the list.
     *
     * @return True if this view is enabled, false otherwise.
     */
    boolean isEnabled();
}
