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

import android.os.Handler;
import android.os.Message;
import android.os.Parcelable;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.ViewPager;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.ImageView;
import android.widget.ImageView.ScaleType;
import android.widget.TabHost;
import android.widget.TabHost.OnTabChangeListener;

import com.android.camera.CameraActivity;
import com.android.camera.FeatureSwitcher;
import com.android.camera.Log;
import com.android.camera.R;
import com.android.camera.Util;
import com.android.camera.ui.RotateImageView;
import com.android.camera.ui.SettingListLayout;

import com.mediatek.camera.platform.ICameraAppUi.CommonUiType;
import com.mediatek.camera.platform.ICameraAppUi.ViewState;
import com.mediatek.camera.setting.preference.CameraPreference;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.setting.preference.PreferenceGroup;
import com.mediatek.camera.setting.SettingConstants;

import java.util.ArrayList;
import java.util.List;

public class SettingManager extends ViewManager implements View.OnClickListener,
        SettingListLayout.Listener, CameraActivity.OnPreferenceReadyListener, OnTabChangeListener {
    private static final String TAG = "SettingManager";

    public interface SettingListener {
        void onSharedPreferenceChanged(ListPreference preference);
        void onRestorePreferencesClicked();
        void onSettingContainerShowing(boolean show);
        void onStereoCameraPreferenceChanged(ListPreference preference, int type);
    }

    protected static final int SETTING_PAGE_LAYER = VIEW_LAYER_SETTING;
    private static final String TAB_INDICATOR_KEY_PREVIEW = "preview";
    private static final String TAB_INDICATOR_KEY_COMMON = "common";
    private static final String TAB_INDICATOR_KEY_CAMERA = "camera";
    private static final String TAB_INDICATOR_KEY_VIDEO = "video";

    protected static final int MSG_REMOVE_SETTING = 0;
    protected static final int DELAY_MSG_REMOVE_SETTING_MS = 3000; // delay
                                                                   // remove
                                                                   // setting

    private MyPagerAdapter mAdapter;
    protected ViewGroup mSettingLayout;
    private ViewPager mPager;
    private TabHost mTabHost;

    protected RotateImageView mIndicator;
    protected boolean mShowingContainer;
    private boolean mIsStereoFeatureSwitch;
    protected ISettingCtrl mSettingController;
    protected SettingListener mListener;
    private Animation mFadeIn;
    private Animation mFadeOut;
    private ListPreference mPreference;
    private boolean mCancleHideAnimation = false;
    private boolean mPreferenceReady;

    protected Handler mMainHandler = new Handler() {
        public void handleMessage(Message msg) {
            Log.d(TAG, "handleMessage(" + msg + ")");
            switch (msg.what) {
            case MSG_REMOVE_SETTING:
                // If we removeView and addView frequently, drawing cache may be
                // wrong.
                // Here avoid do this action frequently to workaround that
                // issue.
                if (mSettingLayout != null && mSettingLayout.getParent() != null) {
                    getContext().removeView(mSettingLayout, SETTING_PAGE_LAYER);
                }
                break;
            default:
                break;
            }
        };
    };

    public SettingManager(CameraActivity context) {
        super(context);
        context.addOnPreferenceReadyListener(this);
    }

    @Override
    protected View getView() {
        View view = inflate(R.layout.setting_indicator);
        mIndicator = (RotateImageView) view.findViewById(R.id.setting_indicator);
        mIndicator.setOnClickListener(this);
        return view;
    }

    @Override
    public void onRefresh() {
        Log.d(TAG, "onRefresh() isShowing()=" + isShowing() + ", mShowingContainer="
                + mShowingContainer);
        if (mShowingContainer && mAdapter != null) { // just apply checker when
                                                     // showing settings
            mAdapter.notifyDataSetChanged();
        }
    }

    @Override
    public void hide() {
        collapse(true);
        super.hide();
    }

    @Override
    protected void onRelease() {
        super.onRelease();
        releaseSettingResource();
    }

    @Override
    public boolean collapse(boolean force) {
        boolean collapsechild = false;
        if (mShowingContainer && mAdapter != null) {
            if (!mAdapter.collapse(force)) {
                hideSetting();
            }
            collapsechild = true;
        }
        Log.d(TAG, "collapse(" + force + ") mShowingContainer=" + mShowingContainer + ", return "
                + collapsechild);
        return collapsechild;
    }

    @Override
    public void onClick(View view) {
        if (mPreferenceReady && view == mIndicator) {
            if (!mShowingContainer) {
                showSetting();
            } else {
                collapse(true);
            }
        }
    }

    @Override
    public void onOrientationChanged(int orientation) {
        super.onOrientationChanged(orientation);
        Util.setOrientation(mSettingLayout, orientation, true);
    }

    public void superOrientationChanged(int orientation) {
        super.onOrientationChanged(orientation);
    }

    @Override
    public void onRestorePreferencesClicked() {
        Log.d(TAG, "onRestorePreferencesClicked() mShowingContainer=" + mShowingContainer);
        if (mListener != null && mShowingContainer) {
            mListener.onRestorePreferencesClicked();
        }
    }

    @Override
    public void onSettingChanged(SettingListLayout settingList, ListPreference preference) {
        Log.d(TAG, "onSettingChanged(" + settingList + ")");
        if (mListener != null) {
            mListener.onSharedPreferenceChanged(preference);
            mPreference = preference;
        }
        refresh();
    }

    @Override
    public void onStereoCameraSettingChanged(SettingListLayout settingList,
            ListPreference preference, int index, boolean showing) {
        Log.d(TAG, "onStereo3dSettingChanged(" + settingList + ")" + ", type = " + index);
        if (mListener != null) {
            mIsStereoFeatureSwitch = true;
            mListener.onStereoCameraPreferenceChanged(preference, index);
            mPreference = preference;
        }
        if (getContext().getCurrentMode() == ModePicker.MODE_STEREO_CAMERA
                || (getContext().getCurrentMode() != ModePicker.MODE_STEREO_CAMERA && index == 2)) {
            refresh();
            return;
        }
        if (mShowingContainer && mAdapter != null) {
            if (!mAdapter.collapse(true)) {
                if (mShowingContainer && mSettingLayout != null) {
                    mMainHandler.removeMessages(MSG_REMOVE_SETTING);
                    mSettingLayout.setVisibility(View.GONE);
                    getContext().getCameraAppUI().restoreViewState();
                    mIndicator.setImageResource(R.drawable.ic_setting_normal);
                    mMainHandler.sendEmptyMessageDelayed(MSG_REMOVE_SETTING,
                            DELAY_MSG_REMOVE_SETTING_MS);
                }
                setChildrenClickable(false);
            }
        }
        if (getContext().isFullScreen()) {
                mMainHandler.removeMessages(MSG_REMOVE_SETTING);
                initializeSettings();
                refresh();
                highlightCurrentSetting(mPager.getCurrentItem());
                mSettingLayout.setVisibility(View.VISIBLE);
                if (mSettingLayout.getParent() == null) {
                    getContext().addView(mSettingLayout, SETTING_PAGE_LAYER);
                }
                getContext().getCameraAppUI().setViewState(ViewState.VIEW_STATE_SETTING);
                mIndicator.setImageResource(R.drawable.ic_setting_focus);
            setChildrenClickable(true);
        }
    }

    @Override
    public void onPreferenceReady() {
        releaseSettingResource();
        mPreferenceReady = true;
    }

    @Override
    public void onTabChanged(String key) {
        int currentIndex = -1;
        if (mTabHost != null && mPager != null) {
            currentIndex = mTabHost.getCurrentTab();
            mPager.setCurrentItem(currentIndex);
        }
        Log.d(TAG, "onTabChanged(" + key + ") currentIndex=" + currentIndex);
    }

    public void setListener(SettingListener listener) {
        mListener = listener;
    }

    public void setSettingController(ISettingCtrl settingController) {
        mSettingController = settingController;
    }

    public boolean handleMenuEvent() {
        boolean handle = false;
        if (isEnabled() && isShowing() && mIndicator != null) {
            mIndicator.performClick();
            handle = true;
        }
        Log.d(TAG, "handleMenuEvent() isEnabled()=" + isEnabled() + ", isShowing()=" + isShowing()
                + ", mIndicator=" + mIndicator + ", return " + handle);
        return handle;
    }

    protected void releaseSettingResource() {
        Log.i(TAG, "releaseSettingResource()");
        if (mIsStereoFeatureSwitch) {
            mIsStereoFeatureSwitch = false;
            Log.d(TAG, "releaseSettingResource is stereo feature, no need release");
            return;
        }
        collapse(true);
        if (mSettingLayout != null) {
            mAdapter = null;
            mPager = null;
            mSettingLayout = null;
        }
    }

    public void showSetting() {
        Log.d(TAG, "showSetting() mShowingContainer=" + mShowingContainer
                + ", getContext().isFullScreen()=" + getContext().isFullScreen());
        if (getContext().isFullScreen()) {
            if (!mShowingContainer
                    && mSettingController.isSettingsInitialized()
                    && getContext().getCameraAppUI().isNormalViewState()) {
                mMainHandler.removeMessages(MSG_REMOVE_SETTING);
                mShowingContainer = true;
                mListener.onSettingContainerShowing(mShowingContainer);
                initializeSettings();
                refresh();
                highlightCurrentSetting(mPager.getCurrentItem());
                mSettingLayout.setVisibility(View.VISIBLE);
                if (mSettingLayout.getParent() == null) {
                    getContext().addView(mSettingLayout, SETTING_PAGE_LAYER);
                }
                getContext().getCameraAppUI().setViewState(ViewState.VIEW_STATE_SETTING);
                startFadeInAnimation(mSettingLayout);
                mIndicator.setImageResource(R.drawable.ic_setting_focus);
            }
            setChildrenClickable(true);
        }
    }

    public void resetSettings() {
        if (mSettingLayout != null && mSettingLayout.getParent() != null) {
            getContext().removeView(mSettingLayout, SETTING_PAGE_LAYER);
        }
        mSettingLayout = null;
    }

    private void initializeSettings() {
        if (mSettingLayout == null && mSettingController.getPreferenceGroup() != null) {
            mSettingLayout = (ViewGroup) getContext().inflate(R.layout.setting_container,
                    SETTING_PAGE_LAYER);
            mTabHost = (TabHost) mSettingLayout.findViewById(R.id.tab_title);
            mTabHost.setup();

            // For tablet
            int settingKeys[] = SettingConstants.SETTING_GROUP_COMMON_FOR_TAB;
            if (FeatureSwitcher.isSubSettingEnabled()) {
                settingKeys = SettingConstants.SETTING_GROUP_MAIN_COMMON_FOR_TAB;
            } else if (FeatureSwitcher.isLomoEffectEnabled() && getContext().isNonePickIntent()) {
                settingKeys = SettingConstants.SETTING_GROUP_COMMON_FOR_LOMOEFFECT;
            }
            List<Holder> list = new ArrayList<Holder>();
            list.add(new Holder(TAB_INDICATOR_KEY_COMMON, R.drawable.ic_tab_common_setting,
                    settingKeys));
            if (getContext().isNonePickIntent()
                    && getContext().getCurrentMode() != ModePicker.MODE_PHOTO_STEREO) {
                list.add(new Holder(TAB_INDICATOR_KEY_CAMERA, R.drawable.ic_tab_camera_setting,
                        SettingConstants.SETTING_GROUP_CAMERA_FOR_TAB));
                list.add(new Holder(TAB_INDICATOR_KEY_VIDEO, R.drawable.ic_tab_video_setting,
                        SettingConstants.SETTING_GROUP_VIDEO_FOR_TAB));
            } else if (getContext().isImageCaptureIntent()
                    || getContext().getCurrentMode() == ModePicker.MODE_PHOTO_STEREO) {
                list.add(new Holder(TAB_INDICATOR_KEY_CAMERA, R.drawable.ic_tab_camera_setting,
                        SettingConstants.SETTING_GROUP_CAMERA_FOR_TAB));
            } else if (getContext().isVideoCaptureIntent()) {
                list.add(new Holder(TAB_INDICATOR_KEY_VIDEO, R.drawable.ic_tab_video_setting,
                        SettingConstants.SETTING_GROUP_VIDEO_FOR_TAB));
            }

            int size = list.size();
            List<SettingListLayout> pageViews = new ArrayList<SettingListLayout>();
            for (int i = 0; i < size; i++) {
                Holder holder = list.get(i);
                // new page view
                SettingListLayout pageView = (SettingListLayout) getContext().inflate(
                        R.layout.setting_list_layout, SETTING_PAGE_LAYER);
                ArrayList<ListPreference> listItems = new ArrayList<ListPreference>();
                pageView.initialize(getListPreferences(holder.mSettingKeys, i == 0));
                pageViews.add(pageView);
                // new indicator view
                ImageView indicatorView = new ImageView(getContext());
                if (indicatorView != null) {
                    indicatorView.setBackgroundResource(R.drawable.bg_tab_title);
                    indicatorView.setImageResource(holder.mIndicatorIconRes);
                    indicatorView.setScaleType(ScaleType.CENTER);
                }
                mTabHost.addTab(mTabHost.newTabSpec(holder.mIndicatorKey)
                        .setIndicator(indicatorView).setContent(android.R.id.tabcontent));
            }

            mAdapter = new MyPagerAdapter(pageViews);
            mPager = (ViewPager) mSettingLayout.findViewById(R.id.pager);
            mPager.setAdapter(mAdapter);
            mPager.setOnPageChangeListener(mAdapter);
            mTabHost.setOnTabChangedListener(this);
        }
        Util.setOrientation(mSettingLayout, getOrientation(), false);
    }

    private ArrayList<ListPreference> getListPreferences(int[] keys, boolean addrestore) {
        ArrayList<ListPreference> listItems = new ArrayList<ListPreference>();
        for (int i = 0; i < keys.length; i++) {
            String key = SettingConstants.getSettingKey(keys[i]);
            ListPreference pref = mSettingController.getListPreference(key);
            if (pref != null && pref.isVisibled() && pref.isShowInSetting()) {
                listItems.add(pref);
            }
        }
        if ("on".equals(mSettingController
                .getSettingValue(SettingConstants.KEY_SLOW_MOTION))
                || "on".equals(mSettingController
                        .getSettingValue(SettingConstants.KEY_REFOCUS))) {
            listItems.remove(mSettingController
                    .getListPreference(SettingConstants.KEY_VIDEO_QUALITY));
        }
        if (!("on".equals(mSettingController
                .getSettingValue(SettingConstants.KEY_REFOCUS)))) {
            listItems.remove(mSettingController
                    .getListPreference(SettingConstants.KEY_REFOCUS_VIDEO_QUALITY));
        }
        if (addrestore) {
            listItems.add(null);
        }
        return listItems;
    }

    public void cancleHideAnimation() {
        mCancleHideAnimation = true;
    }

    public void hideSetting() {
        Log.d(TAG, "hideSetting() mShowingContainer=" + mShowingContainer + ", mSettingLayout="
                + mSettingLayout);
        setChildrenClickable(false);
        if (mShowingContainer && mSettingLayout != null) {
            mMainHandler.removeMessages(MSG_REMOVE_SETTING);
            if (!mCancleHideAnimation) {
                startFadeOutAnimation(mSettingLayout);
            }
            mSettingLayout.setVisibility(View.GONE);
            mShowingContainer = false;
            //because into setting,ViewState will set mode picker false
            getContext().getCameraAppUI().getCameraView(CommonUiType.MODE_PICKER).setEnabled(true);
            getContext().getCameraAppUI().restoreViewState();
            mListener.onSettingContainerShowing(mShowingContainer);
            mIndicator.setImageResource(R.drawable.ic_setting_normal);
            mMainHandler.sendEmptyMessageDelayed(MSG_REMOVE_SETTING, DELAY_MSG_REMOVE_SETTING_MS);
        }
        mCancleHideAnimation = false;
    }

    protected void setChildrenClickable(boolean clickable) {
        Log.d(TAG, "setChildrenClickable(" + clickable + ") ");
        PreferenceGroup group = mSettingController.getPreferenceGroup();
        if (group != null) {
            int len = group.size();
            for (int i = 0; i < len; i++) {
                CameraPreference pref = group.get(i);
                if (pref instanceof ListPreference) {
                    ((ListPreference) pref).setClickable(clickable);
                }
            }
        }
    }

    protected void startFadeInAnimation(View view) {
        if (mFadeIn == null) {
            mFadeIn = AnimationUtils.loadAnimation(getContext(), R.anim.setting_popup_grow_fade_in);
        }
        if (view != null && mFadeIn != null) {
            view.startAnimation(mFadeIn);
        }
    }

    protected void startFadeOutAnimation(View view) {
        if (mFadeOut == null) {
            mFadeOut = AnimationUtils.loadAnimation(getContext(),
                    R.anim.setting_popup_shrink_fade_out);
        }
        if (view != null && mFadeOut != null) {
            view.startAnimation(mFadeOut);
        }
    }

    private void highlightCurrentSetting(int position) {
        if (mTabHost != null) {
            mTabHost.setCurrentTab(position);
        }
    }

    private class Holder {
        String mIndicatorKey;
        int mIndicatorIconRes;
        int[] mSettingKeys;

        public Holder(String key, int res, int[] keys) {
            mIndicatorKey = key;
            mIndicatorIconRes = res;
            mSettingKeys = keys;
        }
    }

    public boolean isShowSettingContainer() {
        return mShowingContainer;
    }

    private class MyPagerAdapter extends PagerAdapter implements OnPageChangeListener {
        private final List<SettingListLayout> mPageViews;

        public MyPagerAdapter(List<SettingListLayout> pageViews) {
            mPageViews = new ArrayList<SettingListLayout>(pageViews);
        }

        @Override
        public void destroyItem(View view, int position, Object object) {
            Log.d(TAG, "MyPagerAdapter.destroyItem(" + position + ")");
            ((ViewPager) view).removeView(mPageViews.get(position));
        }

        @Override
        public void finishUpdate(View view) {
        }

        @Override
        public int getCount() {
            return mPageViews.size();
        }

        @Override
        public Object instantiateItem(View view, int position) {
            Log.i(TAG, "MyPagerAdapter.instantiateItem(" + position + ")");
            ((ViewPager) view).addView(mPageViews.get(position), 0);
            return mPageViews.get(position);
        }

        @Override
        public boolean isViewFromObject(View view, Object object) {
            return view == (object);
        }

        @Override
        public void restoreState(Parcelable state, ClassLoader loader) {
        }

        @Override
        public Parcelable saveState() {
            return null;
        }

        @Override
        public void startUpdate(View container) {
        }

        // for page event @
        @Override
        public void onPageScrollStateChanged(int state) {
        }

        @Override
        public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {
            if (mPreference != null) {
                String key = mPreference.getKey();
                if (key.equals(SettingConstants.KEY_EDGE)
                        || key.equals(SettingConstants.KEY_HUE)
                        || key.equals(SettingConstants.KEY_SATURATION)
                        || key.equals(SettingConstants.KEY_BRIGHTNESS)
                        || key.equals(SettingConstants.KEY_CONTRAST)) {
                    return;
                }
            }

            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    collapse(true);
                }
            });
        }

        @Override
        public void onPageSelected(int position) {
            highlightCurrentSetting(position);
            collapse(true);
        }

        @Override
        public void notifyDataSetChanged() {
            super.notifyDataSetChanged();
            for (SettingListLayout page : mPageViews) {
                if (page != null) {
                    page.setSettingChangedListener(SettingManager.this);
                    page.reloadPreference();
                }
            }
        }

        public boolean collapse(boolean force) {
            boolean collapse = false;
            int size = mPageViews.size();
            for (int i = 0; i < size; i++) {
                SettingListLayout pageView = mPageViews.get(i);
                if (pageView != null && pageView.collapseChild() && !force) {
                    collapse = true;
                    break;
                }
            }
            Log.d(TAG, "MyPagerAdapter.collapse(" + force + ") return " + collapse);
            return collapse;
        }
    }
}
