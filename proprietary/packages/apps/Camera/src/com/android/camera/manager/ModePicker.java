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
package com.android.camera.manager;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.DisplayMetrics;
import android.view.View;
import android.widget.LinearLayout;

import com.android.camera.CameraActivity;
import com.android.camera.FeatureSwitcher;
import com.android.camera.Log;
import com.android.camera.ModeChecker;
import com.android.camera.R;
import com.android.camera.ui.ModePickerScrollView;
import com.android.camera.ui.RotateImageView;

import com.mediatek.camera.setting.preference.ListPreference;

public class ModePicker extends ViewManager implements View.OnClickListener,
        View.OnLongClickListener {
    private static final String TAG = "ModePicker";

    private ListPreference mModePreference;

    public interface OnModeChangedListener {
        void onModeChanged(int newMode);
    }

    // can not change this sequence
    // Before MODE_VIDEO is "capture mode" for UI,switch "capture mode"
    // remaining view should not show
    public static final int MODE_PHOTO = 0;
    public static final int MODE_HDR = 1;
    public static final int MODE_FACE_BEAUTY = 2;
    public static final int MODE_PANORAMA = 3;
    public static final int MODE_ASD = 4;
    public static final int MODE_PHOTO_PIP = 5;
    public static final int MODE_STEREO_CAMERA = 6;
    // TODO there is for stereo photo feature
    // Now only denoise, would include 6 later.
    public static final int MODE_PHOTO_STEREO = 7;
    public static final int MODE_VIDEO = 8;
    public static final int MODE_VIDEO_PIP = 9;
    public static final int MODE_VIDEO_STEREO = 10;

    public static final int MODE_NUM_ALL = 11;
    public static final int OFFSET = 100;
    private static final int OFFSET_STEREO_PREVIEW = OFFSET;
    private static final int OFFSET_STEREO_SINGLE = OFFSET * 2;

    public static final int MODE_PHOTO_3D = OFFSET_STEREO_PREVIEW + MODE_PHOTO;
    public static final int MODE_VIDEO_3D = OFFSET_STEREO_PREVIEW + MODE_VIDEO;

    public static final int MODE_PHOTO_SGINLE_3D = OFFSET_STEREO_SINGLE + MODE_PHOTO;
    public static final int MODE_PANORAMA_SINGLE_3D = OFFSET_STEREO_SINGLE + MODE_PANORAMA;

    private static final int DELAY_MSG_HIDE_MS = 3000; // 3s
    private static final int MODE_DEFAULT_MARGINBOTTOM = 100;
    private static final int MODE_DEFAULT_PADDING = 20;
    private static final int MODE_MIN_COUNTS = 4;
    private LinearLayout.LayoutParams mLayoutParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);

    private static final int[] MODE_ICONS_HIGHTLIGHT = new int[MODE_NUM_ALL];
    private static final int[] MODE_ICON_ORDER = {
        MODE_PHOTO, MODE_STEREO_CAMERA, MODE_PHOTO_STEREO, MODE_PHOTO_PIP,
        MODE_FACE_BEAUTY, MODE_PANORAMA};
    static {
        MODE_ICONS_HIGHTLIGHT[MODE_PHOTO] = R.drawable.ic_mode_photo_focus;
        MODE_ICONS_HIGHTLIGHT[MODE_FACE_BEAUTY] = R.drawable.ic_mode_facebeauty_focus;
        MODE_ICONS_HIGHTLIGHT[MODE_PANORAMA] = R.drawable.ic_mode_panorama_focus;
        MODE_ICONS_HIGHTLIGHT[MODE_PHOTO_PIP] = R.drawable.ic_mode_pip_focus;
        MODE_ICONS_HIGHTLIGHT[MODE_STEREO_CAMERA] = R.drawable.ic_mode_refocus_focus;
        MODE_ICONS_HIGHTLIGHT[MODE_PHOTO_STEREO] = R.drawable.ic_mode_denoise_focus;
    };
    private static final int[] MODE_ICONS_NORMAL = new int[MODE_NUM_ALL];
    static {
        MODE_ICONS_NORMAL[MODE_PHOTO] = R.drawable.ic_mode_photo_normal;
        MODE_ICONS_NORMAL[MODE_FACE_BEAUTY] = R.drawable.ic_mode_facebeauty_normal;
        MODE_ICONS_NORMAL[MODE_PANORAMA] = R.drawable.ic_mode_panorama_normal;
        MODE_ICONS_NORMAL[MODE_PHOTO_PIP] = R.drawable.ic_mode_pip_normal;
        MODE_ICONS_NORMAL[MODE_STEREO_CAMERA] = R.drawable.ic_mode_refocus_normal;
        MODE_ICONS_NORMAL[MODE_PHOTO_STEREO] = R.drawable.ic_mode_denoise_normal;
    };

    private final RotateImageView[] mModeViews = new RotateImageView[MODE_NUM_ALL];
    private ModePickerScrollView mScrollView;
    private int mCurrentMode = -1;
    private OnModeChangedListener mModeChangeListener;
    private OnScreenToast mModeToast;
    private int mDisplayWidth;
    private int mModeWidth;
    private int mModeMarginBottom = MODE_DEFAULT_MARGINBOTTOM;

    public ModePicker(CameraActivity context) {
        super(context);
    }

    public int getCurrentMode() {
        return mCurrentMode;
    }

    private void setRealMode(int mode) {
        Log.d(TAG, "setRealMode(" + mode + ") mCurrentMode=" + mCurrentMode);

        if (mode == MODE_PHOTO_PIP) {
        }
        // in photo mode, if the hdr, asd,is on, we
        // should set the current mode is hdr or asd. in hdr, asd mode if its values is off in
        // sharepreference,
        // we should set the current mode
        // as photo mode
       /* if (mode == MODE_PHOTO || mode == MODE_HDR || mode == MODE_ASD) {
            mode = getRealMode(mModePreference);
        }*/

        if (mCurrentMode != mode) {
            mCurrentMode = mode;
            highlightCurrentMode();
            notifyModeChanged();
            if (mModeToast != null) {
                mModeToast.cancel();
            }
        } else {
            // if mode do not change, we should reset ModePicker view enabled
            setEnabled(true);
        }
    }

    public void setCurrentMode(int mode) {
        int realmode = getModeIndex(mode);
        if (getContext().isStereoMode()) {
            if (FeatureSwitcher.isStereoSingle3d()) {
                realmode += OFFSET_STEREO_SINGLE;
            } else {
                realmode += OFFSET_STEREO_PREVIEW;
            }
        }
        Log.d(TAG, "setCurrentMode(" + mode + ") realmode=" + realmode);
        setRealMode(realmode);
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
            if (MODE_HDR == index || MODE_ASD == index
                    || (FeatureSwitcher.isVfbEnable() && MODE_FACE_BEAUTY == index)) {
                mModeViews[MODE_PHOTO].setImageResource(MODE_ICONS_HIGHTLIGHT[MODE_PHOTO]);
            }
        }
    }

    public int getModeIndex(int mode) {
        int index = mode % OFFSET;
        Log.d(TAG, "getModeIndex(" + mode + ") return " + index);
        return index;
    }

    public void setListener(OnModeChangedListener l) {
        mModeChangeListener = l;
    }

    @Override
    protected View getView() {
        clearListener();
        View view = inflate(R.layout.mode_picker);
        mScrollView = (ModePickerScrollView) view.findViewById(R.id.mode_picker_scroller);
        mModeViews[MODE_PHOTO] = (RotateImageView) view.findViewById(R.id.mode_photo);
        mModeViews[MODE_PHOTO_PIP] = (RotateImageView) view.findViewById(R.id.mode_photo_pip);
        mModeViews[MODE_STEREO_CAMERA] = (RotateImageView) view
                .findViewById(R.id.mode_stereo_camera);
        mModeViews[MODE_PHOTO_STEREO] = (RotateImageView) view
                .findViewById(R.id.mode_photo_stereo);
        mModeViews[MODE_FACE_BEAUTY] = (RotateImageView) view.findViewById(R.id.mode_face_beauty);
        mModeViews[MODE_PANORAMA] = (RotateImageView) view.findViewById(R.id.mode_panorama);
        DisplayMetrics metrics = getContext().getResources().getDisplayMetrics();
        mDisplayWidth = Math.min(metrics.widthPixels, metrics.heightPixels);
        mModeWidth = getModeWidth();
        mModeMarginBottom = getDefaultMarginBottom();
        applyListener();
        highlightCurrentMode();
        return view;
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

    @Override
    public void onClick(View view) {
        Log.d(TAG, "onClick(" + view + ") isEnabled()=" + isEnabled() + ", view.isEnabled()="
                + view.isEnabled() + ", getContext().isFullScreen()=" + getContext().isFullScreen()
                + ",mCurrentMode = " + mCurrentMode);
        if (FeatureSwitcher.isVfbEnable() && mCurrentMode == MODE_FACE_BEAUTY
                && view == mModeViews[MODE_PHOTO]) {
            Log.i(TAG, "onClick(,will return");
            return;
        }
        setEnabled(false);
        if (getContext().isFullScreen()) {
            for (int i = 0; i < MODE_NUM_ALL; i++) {
                if (mModeViews[i] == view) {
                    setCurrentMode(i);
                    break;
                }
            }
            Log.d(TAG, "onClick,isCameraOpened:" + getContext().isCameraOpened());
            if (getContext().isCameraOpened()) {
                setEnabled(true);
            }
        } else {
            // if the is not full screen, we should reset PickMode view enable
            setEnabled(true);
        }

        if (view.getContentDescription() != null) {
            if (mModeToast == null) {
                mModeToast = OnScreenToast.makeText(getContext(), view.getContentDescription());
            } else {
                mModeToast.setText(view.getContentDescription());
            }
            mModeToast.showToast();
        }
    }

    public void hideToast() {
        Log.d(TAG, "hideToast(), mModeToast:" + mModeToast);
        if (mModeToast != null) {
            mModeToast.hideToast();
        }
    }

    private void notifyModeChanged() {
        if (mModeChangeListener != null) {
            mModeChangeListener.onModeChanged(getCurrentMode());
        }
    }

    public void onRefresh() {
        Log.d(TAG, "onRefresh() mCurrentMode=" + mCurrentMode);
        // get counts of mode supported by back camera and compute the margin
        // bottom between mode icon.
        int supportModes = ModeChecker.modesShowInPicker(getContext(), 0);
        if (supportModes < MODE_MIN_COUNTS && supportModes > 1) {
            mModeMarginBottom = (mDisplayWidth - supportModes * mModeWidth) / (supportModes - 1);
        }
        mLayoutParams.setMargins(0, 0, 0, mModeMarginBottom);

        int visibleCount = 0;
        for (int i = 0; i < MODE_NUM_ALL; i++) {
            if (mModeViews[i] != null) {
                boolean visible = ModeChecker.getModePickerVisible(getContext(), getContext()
                        .getCameraId(), i);
                // check vFB
                // if vFB support, FB not need show in the mode picker line
                if (MODE_FACE_BEAUTY == i && FeatureSwitcher.isVfbEnable()) {
                    visible = false;
                }
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
    public boolean onLongClick(View view) {
        if (view.getContentDescription() != null) {
            if (mModeToast == null) {
                mModeToast = OnScreenToast.makeText(getContext(), view.getContentDescription());
            } else {
                mModeToast.setText(view.getContentDescription());
            }
            mModeToast.showToast();
        }
        // don't consume long click event
        return false;
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
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
    protected void onRelease() {
        super.onRelease();
        mModeToast = null;
    }

    private int getModeWidth() {
        Bitmap bitmap = BitmapFactory.decodeResource(getContext().getResources(),
                MODE_ICONS_NORMAL[MODE_PHOTO]);
        int bitmapWidth = bitmap.getWidth();
        return bitmapWidth + MODE_DEFAULT_PADDING * 2;

    }

    private int getDefaultMarginBottom() {
        // default show three and half mode icons
        return (mDisplayWidth - MODE_MIN_COUNTS * mModeWidth) / (MODE_MIN_COUNTS - 1)
                + (mModeWidth / (2 * (MODE_MIN_COUNTS - 1)));
    }

    public void setModePreference(ListPreference pref) {
        mModePreference = pref;
    }
}
