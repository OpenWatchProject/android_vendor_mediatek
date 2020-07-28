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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.mediatek.voicecommand.ui.settings;

import android.content.Context;
import android.preference.Preference;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.mediatek.voicecommand.R;
import com.mediatek.voicecommand.util.Log;

/**
 * This class provides the View to be displayed in the VoiceUiCommandPlay and
 * associates with a SharedPreferences to store/retrieve the preference data.
 * 
 */
public class CommandPlayPreference extends Preference {
    private static final String TAG = "CommandPlayPreference";

    private TextView mPreferenceTitle = null;

    private CharSequence mTitleValue = "";
    private LayoutInflater mInflater;
    private Context mContext;

    /**
     * Constructor of CommandPlayPreference.
     * 
     * @param context
     *            the Context this is associated with
     * @param attrs
     *            the attributes of the XML tag that is inflating the preference
     * @param defStyle
     *            the default style to apply to this preference
     * @param title
     *            preference title
     */
    public CommandPlayPreference(Context context, AttributeSet attrs, int defStyle, String title) {
        super(context, attrs, defStyle);
        mContext = context;

        if (super.getTitle() != null) {
            mTitleValue = super.getTitle().toString();
        }
    }

    /**
     * Constructor of CommandPlayPreference.
     * 
     * @param context
     *            the Context this is associated with
     * @param attrs
     *            the attributes of the XML tag that is inflating the preference
     */
    public CommandPlayPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
    }

    /**
     * Constructor of CommandPlayPreference.
     * 
     * @param context
     *            the Context this is associated with
     */
    public CommandPlayPreference(Context context) {
        super(context);
        mContext = context;
    }

    /**
     * Constructor of CommandPlayPreference.
     * 
     * @param context
     *            the Context this is associated with
     * @param title
     *            preference title
     */
    public CommandPlayPreference(Context context, String title) {
        super(context);
        mContext = context;

        if (title != null) {
            mTitleValue = title;
        }
    }

    /**
     * Constructor of CommandPlayPreference.
     * 
     * @param title
     *            preference title
     */
    public void setShowTitle(String title) {
        mTitleValue = title;
    }

    @Override
    public View onCreateView(ViewGroup parent) {
        Log.i(TAG, "[onCreateView]...");

        mInflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View view = mInflater.inflate(R.layout.voice_ui_preference_title, null);

        mPreferenceTitle = (TextView) view.findViewById(R.id.command_preference_title);
        if (mPreferenceTitle != null) {
            mPreferenceTitle.setText(mTitleValue);
        }
        return view;
    }
}
