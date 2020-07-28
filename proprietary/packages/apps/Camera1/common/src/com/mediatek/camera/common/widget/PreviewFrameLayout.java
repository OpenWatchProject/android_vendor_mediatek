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
package com.mediatek.camera.common.widget;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.view.View;
import android.widget.FrameLayout;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentSkipListMap;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * It is layout container for focus and face detection.
 * It's size is same with preview area, and locate at the top of preview surface.
 */

public class PreviewFrameLayout extends FrameLayout {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(PreviewFrameLayout.class.getSimpleName());
    private int mPreviewWidth;
    private int mPreviewHeight;
    private int mCurrentPriority = -1;
    private ConcurrentSkipListMap<Integer, List<View>> mPriorityMap = new ConcurrentSkipListMap<>();


    /**
     * The PreviewFrameLayout constructor.
     * @param context
     *            The Context the view is running in, through which it can
     *            access the current theme, resources, etc.
     * @param attrs
     *            The attributes of the XML tag that is inflating the view.
     */
    public PreviewFrameLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void dispatchDraw(Canvas canvas) {
        int currentPriority = -1;
        Iterator iterator = mPriorityMap.entrySet().iterator();
        List viewList;
        ALL:
        while (iterator.hasNext()) {
            Map.Entry map = (Map.Entry) iterator.next();
            viewList = (List<View>) map.getValue();
            LogHelper.d(TAG, "[dispatchDraw] While loop priority = " + map.getKey());
            for (Object v : viewList) {
                LogHelper.d(TAG, "[dispatchDraw] for loop view = " + v);
                if (((View) v).getVisibility() == View.VISIBLE) {
                    currentPriority = (int) map.getKey();
                    break ALL;
                }
            }
        }

        if (mCurrentPriority != currentPriority) {
            LogHelper.d(TAG, "[dispatchDraw] currentPriority = " + currentPriority);
            mCurrentPriority = currentPriority;
        }
        //found the current highest visible view priority.
        if (currentPriority >= 0) {
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                if ((int) map.getKey() <= currentPriority) {
                    break;
                }
                viewList = (List<View>) map.getValue();
                for (Object v : viewList) {
                    ((View) v).setVisibility(View.INVISIBLE);
                }
            }
        }
        super.dispatchDraw(canvas);
    }

    /**
     * Register preview layout view to the priority list.
     * @param child Preview layout child view.
     * @param priority Preview layout child priority.
     */
    public void registerView(View child, int priority) {
        LogHelper.d(TAG, "registerView child = " + child + " priority " + priority);
        if (mPriorityMap.containsKey(priority)) {
            List viewList = mPriorityMap.get(priority);
            viewList.add(child);
        } else {
            List viewList = new CopyOnWriteArrayList<View>();
            viewList.add(child);
            mPriorityMap.put(priority, viewList);
        }
    }

    /**
     * Unregister view from priority list, after call remove, do not show the view again.
     * @param view The unregister view.
     */
    public void unRegisterView(View view) {
        LogHelper.d(TAG, "unRegisterView view = " + view);
        Iterator iterator = mPriorityMap.entrySet().iterator();
        List viewList;
        ALL:
        while (iterator.hasNext()) {
            Map.Entry map = (Map.Entry) iterator.next();
            viewList = (List<View>) map.getValue();
            LogHelper.d(TAG, "unRegisterView While loop priority = " + map.getKey());
            if (viewList.contains(view)) {
                viewList.remove(view);
                LogHelper.d(TAG, "unRegisterView remove success");
            }
        }
    }

    /**
     * Set current preview area size.
     * @param width Preview area width.
     * @param height Preview area height.
     */
    public void setPreviewSize(int width, int height) {
        LogHelper.d(TAG, "setPreviewSize width = " + width + " height = " + height);
        mPreviewWidth = width;
        mPreviewHeight = height;
        requestLayout();
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(MeasureSpec.makeMeasureSpec(mPreviewWidth, MeasureSpec.EXACTLY),
                MeasureSpec.makeMeasureSpec(mPreviewHeight, MeasureSpec.EXACTLY));
    }
}
