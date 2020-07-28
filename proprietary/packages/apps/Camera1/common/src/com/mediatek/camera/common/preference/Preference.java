/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.common.preference;

import android.content.Context;
import android.preference.PreferenceScreen;
import android.support.annotation.IdRes;
import android.util.AttributeSet;
import android.view.View;

/**
 * Define a new preference class extends {@link android.preference.Preference}
 * to set view's id and content description.
 */

public class Preference extends android.preference.Preference {
    /**
     * This parameter define that setting view is disable or hidden when it isn't selectable.
     * True means its view should be hidden when it isn't selectable, false means its view
     * should be disable when it isn't selectable.
     */
    public static final boolean JUST_DISABLE_UI_WHEN_NOT_SELECTABLE = false;

    private CharSequence mContentDescription;
    @IdRes
    private int mID = View.NO_ID;
    private PreferenceScreen mRootPreference;
    private boolean mRemoved = false;

    /**
     * Perform inflation from XML and apply a class-specific base style. This
     * constructor of Preference allows subclasses to use their own base style
     * when they are inflating.
     * @param context The Context this is associated with, through which it can
     *            access the current theme, resources.
     * @param attrs The attributes of the XML tag that is inflating the
     *            preference.
     * @param defStyleAttr An attribute in the current theme that contains a
     *            reference to a style resource that supplies default values for
     *            the view. Can be 0 to not look for defaults.
     * @param defStyleRes A resource identifier of a style resource that
     *            supplies default values for the view, used only if
     *            defStyleAttr is 0 or can not be found in the theme. Can be 0
     *            to not look for defaults.
     */
    public Preference(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    /**
     * Perform inflation from XML and apply a class-specific base style. This
     * constructor of Preference allows subclasses to use their own base style
     * when they are inflating.
     *
     * @param context The Context this is associated with, through which it can
     *            access the current theme, resources.
     * @param attrs The attributes of the XML tag that is inflating the
     *            preference.
     * @param defStyleAttr An attribute in the current theme that contains a
     *            reference to a style resource that supplies default values for
     *            the view. Can be 0 to not look for defaults.
     */
    public Preference(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    /**
     * Constructor that is called when inflating a Preference from XML. This is
     * called when a Preference is being constructed from an XML file, supplying
     * attributes that were specified in the XML file. This version uses a
     * default style of 0, so the only attribute values applied are those in the
     * Context's Theme and the given AttributeSet.
     *
     * @param context The Context this is associated with, through which it can
     *            access the current theme, resources.
     * @param attrs The attributes of the XML tag that is inflating the
     *            preference.
     */
    public Preference(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    /**
     * Constructor to create a Preference.
     *
     * @param context The Context in which to store Preference values.
     */
    public Preference(Context context) {
        super(context);
    }

    @Override
    protected void onBindView(View view) {
        if (mContentDescription != null) {
            view.setContentDescription(mContentDescription);
        }
        if (mID != View.NO_ID) {
            view.setId(mID);
        }
        super.onBindView(view);
    }

    @Override
    public void setEnabled(boolean enabled) {
        if (JUST_DISABLE_UI_WHEN_NOT_SELECTABLE) {
            super.setEnabled(enabled);
            return;
        }

        if (enabled) {
            mRootPreference.addPreference(this);
            mRemoved = false;
        } else if (!mRemoved) {
            mRootPreference.removePreference(this);
            mRemoved = true;
        }
    }

    /**
     * Set view content description.
     *
     * @param contentDescription The content description.
     */
    public void setContentDescription(CharSequence contentDescription) {
        mContentDescription = contentDescription;
    }

    /**
     * Sets the identifier for this view. The identifier does not have to be
     * unique in this view's hierarchy. The identifier should be a positive
     * number.
     *
     * @param id a number used to identify the view
     */
    public void setId(@IdRes int id) {
        mID = id;
    }

    /**
     * Set the root preference of this preference.
     *
     * @param rootPreference The root preference of this preference.
     */
    public void setRootPreference(PreferenceScreen rootPreference) {
        mRootPreference = rootPreference;
    }
}
