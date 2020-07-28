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
import android.widget.LinearLayout;

import com.mediatek.camera.R;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;

import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.ConcurrentSkipListMap;

/**
 * A manager for {@link IndicatorView}.
 */
public class IndicatorViewManager extends AbstractViewManager {
    private static final Tag TAG = new Tag(
            IndicatorViewManager.class.getSimpleName());
    private static final int PADDING_IN_DP = 20;
    private static final int ITEM_LIMIT = 5;
    private LinearLayout mIndicatorViewLayout;
    private ConcurrentSkipListMap<Integer, View> mIndicatorItems = new ConcurrentSkipListMap<>();
    /**
     * constructor of IndicatorViewManager.
     * @param app The {@link IApp} implementer.
     * @param parentView the root view of ui.
     */
    public IndicatorViewManager(IApp app, ViewGroup parentView) {
        super(app, parentView);
    }

    @Override
    protected View getView() {
        mIndicatorViewLayout = (LinearLayout) mParentView.findViewById(R.id.indicator_view);
        updateQuickItems();
        return mIndicatorViewLayout;
    }

    @Override
    public void setEnabled(boolean enabled) {
        if (mIndicatorViewLayout != null) {
            int count = mIndicatorViewLayout.getChildCount();
            for (int i = 0; i < count; i++) {
                View view = mIndicatorViewLayout.getChildAt(i);
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
    public void addToIndicatorView(View view, int priority) {
        LogHelper.d(TAG, "[registerToIndicatorView] priority = " + priority);
        if (mIndicatorItems.size() > ITEM_LIMIT) {
            LogHelper.w(TAG, "already reach to limit number : " + ITEM_LIMIT);
            return;
        }
        if (!mIndicatorItems.containsValue(view)) {
            mIndicatorItems.put(priority, view);
            updateQuickItems();
        }
    }

    /**
     * remove view from quick switcher.
     * @param view The view removed from quick switcher.
     */
    public void removeFromIndicatorView(View view) {
        if (mIndicatorItems.containsValue(view)) {
            Iterator iterator = mIndicatorItems.entrySet().iterator();
            int priority;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                View v = (View) map.getValue();
                if (v == view) {
                    priority = (Integer) map.getKey();
                    mIndicatorItems.remove(priority, v);
                }
            }
            updateQuickItems();
        }
    }
    /**
     * Register indicator icon view, layout position will be decided by the priority.
     */
    public void registerQuickIconDone() {
        updateQuickItems();
    }

    private void updateQuickItems() {
        int count = 0;
        float density = mApp.getActivity().getResources().getDisplayMetrics().density;
        int paddingInPix = (int) (PADDING_IN_DP * density);
        if (mIndicatorViewLayout != null && mIndicatorViewLayout.getChildCount() != 0) {
            mIndicatorViewLayout.removeAllViews();
        }
        if (mIndicatorViewLayout != null) {
            Iterator iterator = mIndicatorItems.entrySet().iterator();
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                View view = (View) map.getValue();
                view.setLayoutParams(new ViewGroup.LayoutParams(
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT));
                if (count != 0) {
                    view.setPadding(0, paddingInPix, 0, 0);
                }
                mIndicatorViewLayout.addView(view);
                count++;
            }
            updateViewOrientation();
        }
    }
}
