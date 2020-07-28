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
package com.android.camera.v2.uimanager;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.DisplayMetrics;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;

import com.android.camera.R;
import com.android.camera.v2.ui.ModePickerScrollView;
import com.android.camera.v2.ui.RotateImageView;
import com.android.camera.v2.uimanager.preference.ListPreference;
import com.android.camera.v2.uimanager.preference.PreferenceManager;
import com.android.camera.v2.util.SettingKeys;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import java.util.LinkedHashMap;
import java.util.Map;

public class ModePicker extends AbstractUiManager implements View.OnClickListener,
        View.OnLongClickListener {
    private static final Tag TAG = new Tag(ModePicker.class.getSimpleName());
    private static final int MODE_NUM_ALL              = 4;
    private static final int MODE_PANORAMA             = 0;
    private static final int MODE_PHOTO_PIP            = 1;
    private static final int MODE_STEREO_CAMERA        = 2;
    // Add for Cross Mount
    private static final int MODE_PHOTO                = 3;

    private static final int MODE_DEFAULT_MARGINBOTTOM = 100;
    private static final int MODE_DEFAULT_PADDING      = 20;
    private static final int MODE_MIN_COUNTS           = 4;
    private static final int OFFSET                    = 100;

    private LinearLayout.LayoutParams             mLayoutParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
    private static final int[] MODE_ICON_ORDER = {MODE_PHOTO, MODE_STEREO_CAMERA, MODE_PHOTO_PIP,
            MODE_PANORAMA};

    private static final String[] KEY_OF_MODES = new String[MODE_NUM_ALL - 1];
    static {
        KEY_OF_MODES[MODE_PANORAMA]       = SettingKeys.KEY_PANORAMA;
        KEY_OF_MODES[MODE_PHOTO_PIP]      = SettingKeys.KEY_PHOTO_PIP;
        KEY_OF_MODES[MODE_STEREO_CAMERA]  = SettingKeys.KEY_DUAL_CAMERA_MODE;
    }

    private static final int[] MODE_ICONS_HIGHTLIGHT = new int[MODE_NUM_ALL];
    static {
        MODE_ICONS_HIGHTLIGHT[MODE_PHOTO]           = R.drawable.ic_mode_photo_focus;
        MODE_ICONS_HIGHTLIGHT[MODE_PANORAMA]        = R.drawable.ic_mode_panorama_focus;
        MODE_ICONS_HIGHTLIGHT[MODE_PHOTO_PIP]       = R.drawable.ic_mode_pip_focus;
        MODE_ICONS_HIGHTLIGHT[MODE_STEREO_CAMERA]   = R.drawable.ic_mode_refocus_focus;
    };
    private static final int[] MODE_ICONS_NORMAL = new int[MODE_NUM_ALL];
    static {
        MODE_ICONS_NORMAL[MODE_PHOTO]               = R.drawable.ic_mode_photo_normal;
        MODE_ICONS_NORMAL[MODE_PANORAMA]            = R.drawable.ic_mode_panorama_normal;
        MODE_ICONS_NORMAL[MODE_PHOTO_PIP]           = R.drawable.ic_mode_pip_normal;
        MODE_ICONS_NORMAL[MODE_STEREO_CAMERA]       = R.drawable.ic_mode_refocus_normal;
    };

    private Activity                              mActivity;
    private ViewGroup                             mModeLayer;
    private PreferenceManager                     mPreferenceManager;
    private final RotateImageView[]               mModeViews = new RotateImageView[MODE_NUM_ALL];
    private ModePickerScrollView                  mScrollView;
    private OnModeChangedListener                 mListener;
    private OnScreenToast                         mModeToast;
    private int                                   mDisplayWidth;
    private int                                   mModeWidth;
    private int                                   mModeMarginBottom = MODE_DEFAULT_MARGINBOTTOM;
    private int                                   mCurrentMode = MODE_PHOTO;

    public interface OnModeChangedListener {
        public void onModeChanged(Map<String, String> changedModes);
        public void onRestoreToNomalMode(Map<String, String> changedModes);
    }

    public ModePicker(Activity activity, ViewGroup parent,
            PreferenceManager preferenceManager) {
        super(activity, parent);
        mActivity = activity;
        mModeLayer = parent;
        mPreferenceManager = preferenceManager;
    }

    @Override
    protected View getView() {
        // TODO Auto-generated method stub
        clearListener();
        View view = inflate(R.layout.mode_picker_v2);
        mScrollView = (ModePickerScrollView) view.findViewById(R.id.mode_picker_scroller);

        mModeViews[MODE_PHOTO]         = (RotateImageView) view.findViewById(R.id.mode_photo);
        mModeViews[MODE_PHOTO_PIP]     = (RotateImageView) view.findViewById(R.id.mode_photo_pip);
        mModeViews[MODE_STEREO_CAMERA] = (RotateImageView)
                view.findViewById(R.id.mode_stereo_camera);
        mModeViews[MODE_PANORAMA]      = (RotateImageView) view.findViewById(R.id.mode_panorama);

        DisplayMetrics metrics = mActivity.getResources().getDisplayMetrics();
        mDisplayWidth = Math.min(metrics.widthPixels, metrics.heightPixels);
        mModeWidth = getModeWidth();
        mModeMarginBottom = getDefaultMarginBottom();
        applyListener();
        highlightCurrentMode();
        return view;
    }

    @Override
    protected void onRefresh() {
        LogHelper.d(TAG, "onRefresh() mCurrentMode=" + mCurrentMode);
        // get counts of mode supported by back camera and compute the margin
        // bottom
        // between mode icon.
        int supportModes = getCountsOfSupportedModes();
        if (supportModes < MODE_MIN_COUNTS && supportModes > 1) {
            mModeMarginBottom = (mDisplayWidth - supportModes * mModeWidth) / (supportModes - 1);
        }
        LogHelper.d(TAG, "mModeMarginBottom:" + mModeMarginBottom);
        mLayoutParams.setMargins(0, 0, 0, mModeMarginBottom);

        int visibleCount = 0;
        for (int i = 0; i < MODE_NUM_ALL; i++) {
            if (mModeViews[i] != null) {
                boolean visible = isModeVisible(i);
                mModeViews[i].setVisibility(visible ? View.VISIBLE : View.GONE);
                mModeViews[i].setLayoutParams(mLayoutParams);
                mModeViews[i].setPadding(MODE_DEFAULT_PADDING, MODE_DEFAULT_PADDING,
                        MODE_DEFAULT_PADDING, MODE_DEFAULT_PADDING);
                if (visible) {
                    visibleCount++;
                }
            }
        }
        // set margin botton of the last mode icon as 0.
        for (int i = MODE_ICON_ORDER.length - 1; i >= 0; i--) {
            int index = MODE_ICON_ORDER[i];
            if (mModeViews[index] != null && mModeViews[index].getVisibility() == View.VISIBLE) {
                LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                        LinearLayout.LayoutParams.WRAP_CONTENT,
                        LinearLayout.LayoutParams.WRAP_CONTENT);
                params.setMargins(0, 0, 0, 0);
                mModeViews[index].setLayoutParams(params);
                break;
            }
        }

        if (visibleCount <= 1) { // to enable/disable background
            mScrollView.setVisibility(View.GONE);
        } else {
            mScrollView.setVisibility(View.VISIBLE);
        }
        highlightCurrentMode();
    }

    @Override
    public void setEnable(boolean enabled) {
        super.setEnable(enabled);
        if (mScrollView != null) {
            mScrollView.setEnabled(enabled);
        }
        for (int i = 0; i < MODE_NUM_ALL; i++) {
            if (mModeViews[i] != null) {
                mModeViews[i].setEnabled(enabled);
                mModeViews[i].setClickable(enabled);
            }
        }
    }

    @Override
    public boolean onLongClick(View v) {
        if (mModeToast == null) {
            mModeToast = new OnScreenToast(mActivity, mModeLayer);
        }
        mModeToast.showToast(v.getContentDescription());
        return false;
    }

    @Override
    public void onClick(View v) {
        LogHelper.i(TAG, "[onClick], view:" + v);
        if (mModeToast == null) {
            mModeToast = new OnScreenToast(mActivity, mModeLayer);
        }
        mModeToast.showToast(v.getContentDescription());
        int mode = -1;
        for (int i = 0; i < MODE_NUM_ALL; i++) {
            if (mModeViews[i] == v) {
                mode = i;
                break;
            }
        }

        if (mCurrentMode != mode) {
            LinkedHashMap<String, String> changedModes = new LinkedHashMap<String, String>();
            if (mode == MODE_PHOTO) {
                // changed mode to photo mode from other modes.
                String key = KEY_OF_MODES[mCurrentMode];
                changedModes.put(key, "off");
            } else if (mode != MODE_PHOTO && mCurrentMode == MODE_PHOTO) {
                // changed mode from photo mode to other modes
                String key = KEY_OF_MODES[mode];
                changedModes.put(key, "on");
            } else {
                // changed mode between other modes.
                String oldKey = KEY_OF_MODES[mCurrentMode];
                String newKey = KEY_OF_MODES[mode];

                changedModes.put(oldKey, "off");
                changedModes.put(newKey, "on");
            }
            mCurrentMode = mode;
            highlightCurrentMode();

            if (mListener != null) {
                mListener.onModeChanged(changedModes);
            }
        }


    }

    public void setOnModeChangedListener(OnModeChangedListener listener) {
        mListener = listener;
    }

    /**
     * Reset to default mode, this method is called when restore setting.
     */
    public void restoreToNormalMode() {
        LogHelper.i(TAG, "[restoreToNormalMode], mCurrentMode:" + mCurrentMode);
        if (mCurrentMode == MODE_PHOTO) {
            return;
        }

        LinkedHashMap<String, String> changedModes = new LinkedHashMap<String, String>();
        String key = KEY_OF_MODES[mCurrentMode];
        changedModes.put(key, "off");
        mCurrentMode = MODE_PHOTO;
        highlightCurrentMode();
        if (mListener != null) {
            mListener.onRestoreToNomalMode(changedModes);
        }
    }

    private void highlightCurrentMode() {
        int index = getModeIndex(mCurrentMode);
        for (int i = 0; i < MODE_NUM_ALL; i++) {
            if (mModeViews[i] != null) {
                if (i == index) {
                    mModeViews[i].setImageResource(MODE_ICONS_HIGHTLIGHT[i]);
                } else {
                    mModeViews[i].setImageResource(MODE_ICONS_NORMAL[i]);
                }
            }
        }
    }

    private int getModeIndex(int mode) {
        int index = mode % OFFSET;
        LogHelper.d(TAG, "getModeIndex(" + mode + ") return " + index);
        return index;
    }

    private int getCountsOfSupportedModes() {
        int countsOfSupportedMode = 1;
        for (String key : KEY_OF_MODES) {
            ListPreference pref = mPreferenceManager.getListPreference(key);
            if (pref != null && pref.isVisibled()) {
                countsOfSupportedMode ++;
            }
        }
        return countsOfSupportedMode;
    }

    private boolean isModeVisible(int mode) {
        if (mode == MODE_PHOTO) {
            return true;
        }

        String key = KEY_OF_MODES[mode];
        ListPreference pref = mPreferenceManager.getListPreference(key);
        if (pref != null && pref.isVisibled()) {
            return true;
        }
        return false;
    }

    private void applyListener() {
        for (int i = 0; i < MODE_NUM_ALL; i++) {
            if (mModeViews[i] != null) {
                mModeViews[i].setOnClickListener(this);
                mModeViews[i].setOnLongClickListener(this);
            }
        }
    }

    private void clearListener() {
        for (int i = 0; i < MODE_NUM_ALL; i++) {
            if (mModeViews[i] != null) {
                mModeViews[i].setOnClickListener(null);
                mModeViews[i].setOnLongClickListener(null);
                mModeViews[i] = null;
            }
        }
    }

    private int getModeWidth() {
        Bitmap bitmap = BitmapFactory.decodeResource(mActivity.getResources(),
                MODE_ICONS_NORMAL[MODE_PHOTO]);
        int bitmapWidth = bitmap.getWidth();
        return bitmapWidth + MODE_DEFAULT_PADDING * 2;

    }

    private int getDefaultMarginBottom() {
        // default show three and half mode icons
        return (mDisplayWidth - MODE_MIN_COUNTS * mModeWidth) / (MODE_MIN_COUNTS - 1)
                + (mModeWidth / (2 * (MODE_MIN_COUNTS - 1)));
    }

}
