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
import android.content.Intent;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcelable;
import android.provider.MediaStore;
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

import com.android.camera.FeatureSwitcher;
import com.android.camera.R;
import com.android.camera.v2.ui.RotateImageView;
import com.android.camera.v2.ui.SettingListLayout;
import com.android.camera.v2.uimanager.preference.CameraPreference;
import com.android.camera.v2.uimanager.preference.ListPreference;
import com.android.camera.v2.uimanager.preference.PreferenceGroup;
import com.android.camera.v2.uimanager.preference.PreferenceManager;
import com.android.camera.v2.util.CameraUtil;
import com.android.camera.v2.util.SettingKeys;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import java.util.ArrayList;
import java.util.List;


public class SettingManager extends AbstractUiManager implements View.OnClickListener,
        OnTabChangeListener, SettingListLayout.Listener {
    private static final Tag TAG = new Tag(SettingManager.class.getSimpleName());
    protected static final int                MSG_REMOVE_SETTING = 0;
    protected static final int                DELAY_MSG_REMOVE_SETTING_MS = 3000;
    private static final String               TAB_INDICATOR_KEY_PREVIEW = "preview";
    private static final String               TAB_INDICATOR_KEY_COMMON = "common";
    private static final String               TAB_INDICATOR_KEY_CAMERA = "camera";
    private static final String               TAB_INDICATOR_KEY_VIDEO = "video";

    private PreferenceManager                 mPreferenceManager;
    private ListPreference                    mPreference;
    private RotateImageView                   mIndicator;
    private MyPagerAdapter                    mAdapter;
    private ViewGroup                         mSettingLayout;
    private ViewGroup                         mSettingViewLayer;
    private ViewPager                         mPager;
    private TabHost                           mTabHost;
    private MainHandler                       mMainHandler;
    private Activity                          mActivity;
    private Intent                            mIntent;
    private Animation                         mFadeIn;
    private Animation                         mFadeOut;
    private OnSettingChangedListener           mSettingChangedListener;
    private OnSettingStatusListener           mOnSettingStatusListener;
    private boolean                           mShowingContainer = false;

    public interface OnSettingChangedListener {
        public void onSettingChanged(String key, String value);
        public void onSettingRestored();
    }

    public interface OnSettingStatusListener {
        public void onShown();
        public void onHidden();
    }

    public SettingManager(Activity activity, ViewGroup parent,
            PreferenceManager preferenceManager) {
        super(activity, parent);
        mActivity = activity;
        mSettingViewLayer = parent;
        mMainHandler = new MainHandler(activity.getMainLooper());
        mPreferenceManager = preferenceManager;
        mIntent = mActivity.getIntent();
    }

    @Override
    protected View getView() {
        View view = inflate(R.layout.setting_indicator_v2);
        mIndicator = (RotateImageView) view.findViewById(R.id.setting_indicator);
        mIndicator.setOnClickListener(this);
        return view;
    }

    @Override
    protected void onRefresh() {
        LogHelper.i(TAG, "onRefresh(), mShowingContainer=" + mShowingContainer);
        if (mShowingContainer && mAdapter != null) {
            mAdapter.notifyDataSetChanged();
        }
    }

    @Override
    public void onClick(View view) {
        // TODO Auto-generated method stub
        if (view == mIndicator) {
            if (!mShowingContainer) {
                showSetting();
            } else {
                collapse(true);
            }
        }
    }

    @Override
    public void onTabChanged(String key) {
        int currentIndex = -1;
        if (mTabHost != null && mPager != null) {
            currentIndex = mTabHost.getCurrentTab();
            mPager.setCurrentItem(currentIndex);
        }
        LogHelper.i(TAG, "onTabChanged(" + key + ") currentIndex=" + currentIndex);
    }

    @Override
    public void onSettingChanged(SettingListLayout settingList,
            ListPreference preference) {
        if (mSettingChangedListener != null) {
            if (preference == null) {
                return;
            }
            LogHelper.i(TAG, "[onSettingChanged], key:" + preference.getKey() +
                    ", value:" + preference.getValue());
            mSettingChangedListener.onSettingChanged(preference.getKey(), preference.getValue());
        }

    }

    @Override
    public void onRestoreSetting() {
        LogHelper.i(TAG, "[onRestoreSetting]...");
        if (mSettingChangedListener != null) {
            mSettingChangedListener.onSettingRestored();
        }
    }

    public boolean onBackPressed() {
        LogHelper.i(TAG, "[onBackPressed]...");
        return collapse(false);
    }

    public void setSettingChangedListener(OnSettingChangedListener listener) {
        mSettingChangedListener = listener;
    }

    public void setSettingStatusListener(OnSettingStatusListener listener) {
        mOnSettingStatusListener = listener;
    }

    public boolean collapse(boolean force) {
        boolean collapsechild = false;
        if (mShowingContainer && mAdapter != null) {
            if (!mAdapter.collapse(force)) {
                hideSetting();
            }
            collapsechild = true;
        }
        LogHelper.i(TAG, "collapse(" + force + ") mShowingContainer="
                + mShowingContainer + ", return " + collapsechild);
        return collapsechild;
    }

    /**
     * This method must be called in UI thread.
     */
    public void collapseImmediately() {
        LogHelper.i(TAG, "[collapseImmediately]...");
        mMainHandler.removeMessages(MSG_REMOVE_SETTING);
        if (mAdapter != null) {
            mAdapter.collapse(true);
        }

        if (mSettingLayout != null && mSettingViewLayer != null) {
            mSettingViewLayer.removeView(mSettingLayout);
        }
        mShowingContainer = false;
        mSettingLayout = null;
    }

    private void showSetting() {
        LogHelper.i(TAG, "showSetting() mShowingContainer=" + mShowingContainer);
        if (!mShowingContainer) {
            mMainHandler.removeMessages(MSG_REMOVE_SETTING);
            mShowingContainer = true;
            initializeSettings();
            refresh();
            highlightCurrentSetting(mPager.getCurrentItem());
            mSettingLayout.setVisibility(View.VISIBLE);
            if (mSettingLayout.getParent() == null) {
                mSettingViewLayer.addView(mSettingLayout);
            }
            if (mOnSettingStatusListener != null) {
                mOnSettingStatusListener.onShown();
            }
            startFadeInAnimation(mSettingLayout);
            mIndicator.setImageResource(R.drawable.ic_setting_focus);
        }
        //setChildrenClickable(true);
    }

    private void hideSetting() {
        LogHelper.i(TAG, "hideSetting() mShowingContainer="
                + mShowingContainer + ", mSettingLayout=" + mSettingLayout);
        //setChildrenClickable(false);
        if (mShowingContainer && mSettingLayout != null) {
            mMainHandler.removeMessages(MSG_REMOVE_SETTING);
            startFadeOutAnimation(mSettingLayout);
            mSettingLayout.setVisibility(View.GONE);
            mShowingContainer = false;
            if (mOnSettingStatusListener != null) {
                mOnSettingStatusListener.onHidden();
            }
            mIndicator.setImageResource(R.drawable.ic_setting_normal);
            mMainHandler.sendEmptyMessageDelayed(MSG_REMOVE_SETTING, DELAY_MSG_REMOVE_SETTING_MS);
        }
    }

    private void initializeSettings() {
        if (mSettingLayout == null) {
            mSettingLayout = (ViewGroup)
                    mActivity.getLayoutInflater().inflate(R.layout.setting_container_v2,
                    mSettingViewLayer, false);
            mTabHost = (TabHost) mSettingLayout.findViewById(R.id.tab_title);
            mTabHost.setup();
            String action = mIntent.getAction();
            LogHelper.i(TAG, "intent.action:" + action);
            List<Holder> list = new ArrayList<Holder>();
            // default setting
            int commonKeys[] = SettingKeys.SETTING_GROUP_COMMON_FOR_TAB;
            int cameraKeys[] = SettingKeys.SETTING_GROUP_CAMERA_FOR_TAB;
            int videoKeys[] = SettingKeys.SETTING_GROUP_VIDEO_FOR_TAB;

            if (FeatureSwitcher.isSubSettingEnabled()) {
                // For tablet
                commonKeys = SettingKeys.SETTING_GROUP_MAIN_COMMON_FOR_TAB;
            } else if (FeatureSwitcher.isLomoEffectEnabled()) {
                commonKeys = SettingKeys.SETTING_GROUP_COMMON_FOR_LOMOEFFECT;
            }
            // image capture setting, compared to default setting,
            // common settings and video setting may be different.
            if (MediaStore.ACTION_IMAGE_CAPTURE.equals(action)) {
                videoKeys = null;
            }
            // image capture setting, compared to default setting,
            // common settings and video setting
            // may be different.
            if (MediaStore.ACTION_VIDEO_CAPTURE.equals(action)) {
                cameraKeys = null;
            }

            if (commonKeys != null) {
                list.add(new Holder(TAB_INDICATOR_KEY_COMMON, R.drawable.ic_tab_common_setting,
                        commonKeys));
            }
            if (cameraKeys != null) {
                list.add(new Holder(TAB_INDICATOR_KEY_CAMERA, R.drawable.ic_tab_camera_setting,
                        cameraKeys));
            }
            if (videoKeys != null) {
                list.add(new Holder(TAB_INDICATOR_KEY_VIDEO, R.drawable.ic_tab_video_setting,
                        videoKeys));
            }

            int size = list.size();
            List<SettingListLayout> pageViews = new ArrayList<SettingListLayout>();
            for (int i = 0; i < size; i++) {
                Holder holder = list.get(i);
                // new page view
                SettingListLayout pageView = (SettingListLayout)
                        mActivity.getLayoutInflater().inflate(
                        R.layout.setting_list_layout_v2, mSettingViewLayer, false);
                ArrayList<ListPreference> listItems = new ArrayList<ListPreference>();
                pageView.setRootView(mSettingViewLayer);
                pageView.initialize(getListPreferences(holder.mSettingKeys, i == 0));
                pageView.setSettingChangedListener(SettingManager.this);
                pageViews.add(pageView);
                // new indicator view
                ImageView indicatorView = new ImageView(mActivity);
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
        int orientation = (Integer) mSettingViewLayer.getTag();
        CameraUtil.setOrientation(mSettingLayout, orientation, false);
    }

    private void highlightCurrentSetting(int position) {
        if (mTabHost != null) {
            mTabHost.setCurrentTab(position);
        }
    }

    private void startFadeInAnimation(View view) {
        if (mFadeIn == null) {
            mFadeIn = AnimationUtils.loadAnimation(mActivity, R.anim.setting_popup_grow_fade_in);
        }
        if (view != null && mFadeIn != null) {
            view.startAnimation(mFadeIn);
        }
    }

    private void startFadeOutAnimation(View view) {
        if (mFadeOut == null) {
            mFadeOut = AnimationUtils.loadAnimation(mActivity,
                    R.anim.setting_popup_shrink_fade_out);
        }
        if (view != null && mFadeOut != null) {
            view.startAnimation(mFadeOut);
        }
    }

    protected void setChildrenClickable(boolean clickable) {
        LogHelper.i(TAG, "setChildrenClickable(" + clickable + ") ");
        PreferenceGroup group = mPreferenceManager.getPreferenceGroup();
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

    private ArrayList<ListPreference> getListPreferences(int[] keys, boolean addrestore) {
        ArrayList<ListPreference> listItems = new ArrayList<ListPreference>();
        for (int i = 0; i < keys.length; i++) {
            String key = SettingKeys.getSettingKey(keys[i]);
            ListPreference pref = mPreferenceManager.getListPreference(key);
            if (pref != null && pref.isShowInSetting() && pref.isVisibled()) {
                listItems.add(pref);
            }
        }

        if (addrestore) {
            listItems.add(null);
        }
        return listItems;
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

    private class MainHandler extends Handler {

        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            LogHelper.i(TAG, "msg:" + msg);
            switch(msg.what) {
            case MSG_REMOVE_SETTING:
                // If we removeView and addView frequently, drawing cache may be
                // wrong.
                // Here avoid do this action frequently to workaround that
                // issue.
                if (mSettingLayout != null && mSettingViewLayer != null) {
                    mSettingViewLayer.removeView(mSettingLayout);
                }
                mSettingLayout = null;
                break;
            }
        }

    }

    private class MyPagerAdapter extends PagerAdapter implements OnPageChangeListener {
        private final List<SettingListLayout> mPageViews;

        public MyPagerAdapter(List<SettingListLayout> pageViews) {
            mPageViews = new ArrayList<SettingListLayout>(pageViews);
        }

        @Override
        public void destroyItem(View view, int position, Object object) {
            LogHelper.i(TAG, "MyPagerAdapter.destroyItem(" + position + ")");
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
            LogHelper.i(TAG, "MyPagerAdapter.instantiateItem(" + position + ")");
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
                if (key.equals(SettingKeys.KEY_EDGE)
                        || key.equals(SettingKeys.KEY_HUE)
                        || key.equals(SettingKeys.KEY_SATURATION)
                        || key.equals(SettingKeys.KEY_BRIGHTNESS)
                        || key.equals(SettingKeys.KEY_CONTRAST)) {
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
            LogHelper.i(TAG, "[notifyDataSetChanged]...");
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
            LogHelper.i(TAG, "MyPagerAdapter.collapse(" + force + ") return " + collapse);
            return collapse;
        }
    }
}
