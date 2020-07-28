/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.ui;

import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.LinearLayout;

import com.mediatek.camera.R;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.utils.CameraUtil;

import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.ConcurrentSkipListMap;

/**
 * A manager for {@link QuickSwitcher}.
 */
public class QuickSwitcherManager extends AbstractViewManager {
    private static final Tag TAG = new Tag(
            QuickSwitcherManager.class.getSimpleName());
    private static final int MARGIN_IN_DP = 46;
    private static final int ITEM_LIMIT = 4;
    private LinearLayout mQuickSwitcherLayout;
    private ConcurrentSkipListMap<Integer, View> mQuickItems = new ConcurrentSkipListMap<>();
    private final OnOrientationChangeListenerImpl mOrientationChangeListener;
    private View mTopBar;
    private ViewGroup mOptionRoot;
    /**
     * constructor of QuickSwitcherManager.
     * @param app The {@link IApp} implementer.
     * @param parentView the root view of ui.
     */
    public QuickSwitcherManager(IApp app, ViewGroup parentView) {
        super(app, parentView);
        mTopBar = app.getActivity().findViewById(R.id.top_bar);
        mOptionRoot = (ViewGroup) mApp.getActivity().findViewById(R.id.quick_switcher_option);
        mOrientationChangeListener = new OnOrientationChangeListenerImpl();
    }

    @Override
    protected View getView() {
        mQuickSwitcherLayout = (LinearLayout) mParentView.findViewById(R.id.quick_switcher);
        updateQuickItems();
        return mQuickSwitcherLayout;
    }

    @Override
    public void setEnabled(boolean enabled) {
        if (mQuickSwitcherLayout != null) {
            int count = mQuickSwitcherLayout.getChildCount();
            for (int i = 0; i < count; i++) {
                View view = mQuickSwitcherLayout.getChildAt(i);
                view.setEnabled(enabled);
                view.setClickable(enabled);
            }
        }
    }

    /**
     * add view to quick switcher with specified priority.
     * @param view The view register to quick switcher.
     * @param priority The priority that the registered view sort order.
     */
    public void addToQuickSwitcher(View view, int priority) {
        LogHelper.d(TAG, "[registerToQuickSwitcher] priority = " + priority);
        if (mQuickItems.size() > ITEM_LIMIT) {
            LogHelper.w(TAG, "already reach to limit number : " + ITEM_LIMIT);
            return;
        }
        if (!mQuickItems.containsValue(view)) {
            mQuickItems.put(priority, view);
        }
    }

    /**
     * remove view from quick switcher.
     * @param view The view removed from quick switcher.
     */
    public void removeFromQuickSwitcher(View view) {
        LogHelper.d(TAG, "[removeFromQuickSwitcher]");
        if (mQuickItems.containsValue(view)) {
            Iterator iterator = mQuickItems.entrySet().iterator();
            int priority;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                View v = (View) map.getValue();
                if (v == view) {
                    priority = (Integer) map.getKey();
                    LogHelper.d(TAG, "[removeFromQuickSwitcher] priority = " + priority);
                    mQuickItems.remove(priority, v);
                }
            }
        }
    }
    /**
     * Register quick switcher icon view, layout position will be decided by the priority.
     */
    public void registerQuickIconDone() {
        updateQuickItems();
    }

    /**
     * Show quick switcher option view, mode picker and quick switch will disappear.
     * @param optionView the option view, it should not attach to any parent view.
     */
    public void showQuickSwitcherOption(View optionView) {
        if (mOptionRoot.getChildCount() != 0) {
            LogHelper.e(TAG, "[showQuickSwitcherOption] Already has options to be shown!");
            return;
        }
        Animation inAnim = AnimationUtils.loadAnimation(mApp.getActivity(), R.anim.anim_top_in);
        mOptionRoot.addView(optionView);
        int orientation = mApp.getGSensorOrientation();
        CameraUtil.rotateRotateLayoutChildView(mApp.getActivity(), mOptionRoot, orientation, true);
        mOptionRoot.setVisibility(View.VISIBLE);
        mOptionRoot.setClickable(true);
        mOptionRoot.startAnimation(inAnim);
        mTopBar.setVisibility(View.GONE);
        mApp.registerOnOrientationChangeListener(mOrientationChangeListener);
    }

    /**
     * Hide quick switcher option view, it will remove from the option parent view.
     */
    public void hideQuickSwitcherOption() {
        Animation outAnim = AnimationUtils.loadAnimation(mApp.getActivity(), R.anim.anim_top_out);
        outAnim.setAnimationListener(new Animation.AnimationListener() {
            @Override
            public void onAnimationStart(Animation animation) {

            }

            @Override
            public void onAnimationEnd(Animation animation) {
                mOptionRoot.setVisibility(View.GONE);
                mOptionRoot.setClickable(false);
                mOptionRoot.removeAllViews();
                mTopBar.setVisibility(View.VISIBLE);
                mApp.unregisterOnOrientationChangeListener(mOrientationChangeListener);
            }

            @Override
            public void onAnimationRepeat(Animation animation) {

            }
        });

        mOptionRoot.startAnimation(outAnim);
        outAnim.setFillAfter(true);
    }

    /**
     * Hide quick switcher without animation.
     */
    public void hideQuickSwitcherImmediately() {
        mOptionRoot.setVisibility(View.GONE);
        mOptionRoot.removeAllViews();
        mTopBar.setVisibility(View.VISIBLE);
        mApp.unregisterOnOrientationChangeListener(mOrientationChangeListener);
    }

    private void updateQuickItems() {
        float density = mApp.getActivity().getResources().getDisplayMetrics().density;
        int marginInPix = (int) (MARGIN_IN_DP * density);
        if (mQuickSwitcherLayout != null && mQuickSwitcherLayout.getChildCount() != 0) {
            mQuickSwitcherLayout.removeAllViews();
        }
        if (mQuickSwitcherLayout != null) {
            Iterator iterator = mQuickItems.entrySet().iterator();
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                View view = (View) map.getValue();
                LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT);
                params.setMargins(marginInPix, 0, 0, 0);
                view.setLayoutParams(params);
                mQuickSwitcherLayout.addView(view);
            }
            updateViewOrientation();
        }
    }
    /**
     * Implementer of OnOrientationChangeListener.
     */
    private class OnOrientationChangeListenerImpl implements IApp.OnOrientationChangeListener {

        @Override
        public void onOrientationChanged(int orientation) {
            if (mOptionRoot != null && mOptionRoot.getChildCount() != 0) {
                CameraUtil.rotateRotateLayoutChildView(mApp.getActivity(), mOptionRoot,
                        orientation, true);
            }
        }
    }
}
