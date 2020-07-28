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

package com.mediatek.camera.ui.shutter;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUiListener.OnShutterButtonListener;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.utils.PriorityConcurrentSkipListMap;
import com.mediatek.camera.ui.AbstractViewManager;


import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.ConcurrentSkipListMap;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * A manager for {@link ShutterButton}.
 */
public class ShutterButtonManager extends AbstractViewManager implements
                                                    ShutterRootLayout.OnShutterChangeListener {
    /**
     * Shutter type change listener.
     */
    public interface OnShutterChangeListener {
        /**
         * When current valid shutter changed, invoke the listener to notify.
         * @param newShutterName The new valid shutter name.
         */
       void onShutterTypeChanged(String newShutterName);
    }
    private final static int SHUTTER_GESTURE_PRIORITY = 20;
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
                              ShutterButtonManager.class.getSimpleName());
    private ShutterButton.OnShutterButtonListener mShutterButtonListener;

    private PriorityConcurrentSkipListMap<String, OnShutterButtonListener> mShutterButtonListeners
            = new PriorityConcurrentSkipListMap<>(true);

    /**
     * Used to store a shutter information.
     */
    private static class ShutterItem {
        public Drawable mShutterDrawable;
        public String mShutterType;
        public String mShutterName;
        public ShutterView mShutterView;

    }
    private ConcurrentSkipListMap<Integer, ShutterItem> mShutterButtons =
            new ConcurrentSkipListMap<>();

    private ShutterRootLayout mShutterLayout;
    private LayoutInflater mInflater;
    private OnShutterChangeListener mListener;

    /**
     * constructor of ShutterButtonManager.
     * @param app The {@link IApp} implementer.
     * @param parentView the root view of ui.
     */
    public ShutterButtonManager(IApp app, ViewGroup parentView) {
        super(app, parentView);
        mShutterButtonListener = new ShutterButtonListenerImpl();
        mInflater = (LayoutInflater) app.getActivity()
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @Override
    protected View getView() {
        mShutterLayout = (ShutterRootLayout) mApp.getActivity().findViewById(R.id.shutter_root);
        mShutterLayout.setOnShutterChangedListener(this);
        mApp.getAppUi().registerGestureListener(mShutterLayout.getGestureListener(),
                SHUTTER_GESTURE_PRIORITY);
        // [Add for CCT tool] Receive keycode and switch photo/video mode @{
        mApp.registerKeyEventListener(mShutterLayout.getKeyEventListener(), IApp.DEFAULT_PRIORITY);
        // @}
        return mShutterLayout;
    }

    public void setOnShutterChangedListener(OnShutterChangeListener listener) {
        mListener = listener;
    }

    @Override
    public void onShutterChangedStart(String newShutterName) {
        if (mListener != null) {
            mListener.onShutterTypeChanged(newShutterName);
        }
    }

    @Override
    public void onShutterChangedEnd() {
    }

    @Override
    public void setEnabled(boolean enabled) {
        if (mShutterLayout != null) {
            mShutterLayout.setEnabled(enabled);
            int count = mShutterLayout.getChildCount();
            for (int i = 0; i < count; i++) {
                View view = mShutterLayout.getChildAt(i);
                view.setEnabled(enabled);
            }
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mShutterLayout != null) {
            mShutterLayout.onResume();
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        if (mShutterLayout != null) {
            mShutterLayout.onPause();
        }
    }

    /**
     * Set Shutter text can be clicked or not.
     * @param enabled True shutter text can be clicked.
     *                False shutter text can not be clicked.
     */
    public void setTextEnabled(boolean enabled) {
        if (mShutterLayout != null) {
            int count = mShutterLayout.getChildCount();
            for (int i = 0; i < count; i++) {
                View view = mShutterLayout.getChildAt(i);
                ((ShutterView) view).setTextEnabled(enabled);
            }
        }
    }

    /**
     * Register shutterButton listener.
     * @param listener
     *            the listener set to shutterButtonManager.
     * @param priority The listener priority.
     */
    public void registerOnShutterButtonListener(OnShutterButtonListener listener, int priority) {
        if (listener == null) {
            LogHelper.e(TAG, "registerOnShutterButtonListener error [why null]");
        }
        mShutterButtonListeners.put(mShutterButtonListeners.getPriorityKey(priority, listener),
                listener);
    }

    /**
     * Unregister shutter button listener.
     *
     * @param listener The listener to be unregistered.
     */
    public void unregisterOnShutterButtonListener(OnShutterButtonListener listener) {
        if (listener == null) {
            LogHelper.e(TAG, "unregisterOnShutterButtonListener error [why null]");
        }
        if (mShutterButtonListeners.containsValue(listener)) {
            mShutterButtonListeners.remove(mShutterButtonListeners.findKey(listener));
        }
    }

    /**
     * Register shutter button UI.
     * @param drawable The shutter button icon drawable.
     * @param type The shutter type, such as "Picture" or "Photo", the type will be shown
     *             above the shutter icon as a text.
     * @param priority The shutter ui priority, the smaller the value, the higher the priority.
     *                 the high priority icon will be located on the left.
     */
    public void registerShutterButton(Drawable drawable, String type, int priority) {
        if (mShutterButtons.containsKey(priority)) {
            return;
        }
        ShutterItem item = new ShutterItem();
        item.mShutterDrawable = drawable;
        item.mShutterType = type;
        if ("Picture".equals(type)) {
            item.mShutterName =
                    (String) mApp.getActivity().getResources().getText(R.string.shutter_type_photo);
        } else if ("Video".equals(type)) {
            item.mShutterName =
                    (String) mApp.getActivity().getResources().getText(R.string.shutter_type_video);
        }

        mShutterButtons.put(priority, item);
    }

    /**
     * Register shutter done, it will trigger to refresh the ui.
     */
    public void registerDone() {

        ShutterItem shutter;
        ShutterView shutterView;
        ShutterItem prevShutter = null;

        if (mShutterLayout.getChildCount() != 0) {
            return;
        }

        mShutterLayout.removeAllViews();
        int index = 0;
        //when registerDone, add the shutter view to root layout.
        for (Integer key: mShutterButtons.keySet()) {
            shutter = mShutterButtons.get(key);

                    //inflate the view
            shutterView = (ShutterView) mInflater.inflate(
                    R.layout.shutter_item, mShutterLayout, false);
            shutterView.setType(shutter.mShutterType);
            shutterView.setName(shutter.mShutterName);
            shutterView.setDrawable(shutter.mShutterDrawable);
            shutterView.setId(generateViewId());
            shutterView.setOnShutterTextClickedListener(mShutterLayout);
            shutterView.setTag(index);

            mShutterLayout.addView(shutterView);
            shutterView.setOnShutterButtonListener(mShutterButtonListener);
            shutter.mShutterView = shutterView;
            RelativeLayout.LayoutParams params =
                    (RelativeLayout.LayoutParams) shutterView.getLayoutParams();
            if (prevShutter == null) {
                params.addRule(RelativeLayout.CENTER_IN_PARENT);
            } else {
                params.addRule(RelativeLayout.RIGHT_OF, prevShutter.mShutterView.getId());
            }
            prevShutter = shutter;
            index++;
        }
    }

    /**
     * Invoke the onShutterButtonCLicked listener to start a capture event.
     * @param currentPriority Trigger module shutter button listener priority,
     *                        the trigger event will be pass to the modules which shutter listener
     *                        priority is lower than currentPriority value.
     *                        The zero value will pass the click event to all listeners.
     */
    public void triggerShutterButtonClicked(final int currentPriority) {
        // shutter button may be trigger manually by mode or setting,
        // here should judge whether is enabled, if not enabled ignore this trigger.
        if (isEnabled()) {
            Iterator iterator = mShutterButtonListeners.entrySet().iterator();
            OnShutterButtonListener listener;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                listener = (OnShutterButtonListener) map.getValue();
                int priority = mShutterButtonListeners.getPriorityByKey(
                        (String) map.getKey());
                if (priority > currentPriority
                        && listener != null
                        && listener.onShutterButtonClick()) {
                    return;
                }
            }
        }
    }

    /**
     * Invoke the triggerShutterButtonLongPressed listener to start a capture event.
     * @param currentPriority Trigger module shutter button listener priority,
     *                        the trigger event will be pass to the modules which shutter listener
     *                        priority is lower than currentPriority value.
     *                        The zero value will pass the click event to all listeners.
     */
    public void triggerShutterButtonLongPressed(final int currentPriority) {
        // shutter button may be trigger manually by mode or setting,
        // here should judge whether is enabled, if not enabled ignore this trigger.
        if (isEnabled()) {
            Iterator iterator = mShutterButtonListeners.entrySet().iterator();
            OnShutterButtonListener listener;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                listener = (OnShutterButtonListener) map.getValue();
                int priority = mShutterButtonListeners.getPriorityByKey(
                        (String) map.getKey());
                if (priority > currentPriority
                        && listener != null
                        && listener.onShutterButtonLongPressed()) {
                    return;
                }
            }
        }
    }

    /**
     * Update current mode support shutter types.
     * @param currentType Current mode type.
     * @param types Support type list.
     */
    public void updateModeSupportType(String currentType, String[] types) {

        for (int i = 0; i < mShutterLayout.getChildCount(); i++) {
            boolean isSupported = false;
            ShutterView shutter = (ShutterView) mShutterLayout.getChildAt(i);
            for (int j = 0; j < types.length; j++) {
                if (types[j].equals(shutter.getType())) {
                    isSupported = true;
                }
            }
            if (isSupported) {
                shutter.setVisibility(View.VISIBLE);
            } else {
                shutter.setVisibility(View.INVISIBLE);
            }
        }

        String targetType = null;

        if (types.length == 1) {
            targetType = types[0];
        } else {
            targetType = currentType;
        }
        LogHelper.d(TAG, "currentType = " + currentType + " targetType = " + targetType);
        for (int i = 0; i < mShutterLayout.getChildCount(); i++) {
            ShutterView shutter = (ShutterView) mShutterLayout.getChildAt(i);
            if (targetType.equals(shutter.getType())) {
                mShutterLayout.updateCurrentShutterIndex(i);
            }

        }
    }

    /**
     * Update current mode shutter info.
     * @param type Current mode's shutter type.
     * @param drawable Current mode's shutter drawable.
     */
    public void updateCurrentModeShutter(String type, Drawable drawable) {
        if (drawable != null) {
            for (int i = 0; i < mShutterLayout.getChildCount(); i++) {
                ShutterView shutter = (ShutterView) mShutterLayout.getChildAt(i);
                if (shutter.getType().equals(type)) {
                    shutter.setDrawable(drawable);
                }
            }
        } else {
            for (int i = 0; i < mShutterLayout.getChildCount(); i++) {
                ShutterView shutter = (ShutterView) mShutterLayout.getChildAt(i);
                ShutterItem item;
                for (Integer key: mShutterButtons.keySet()) {
                    item = mShutterButtons.get(key);
                    if (shutter.getType().equals(item.mShutterType)) {
                        shutter.setDrawable(item.mShutterDrawable);
                    }
                }
            }
        }
    }

    /**
     * Get shutter root view.
     * @return the shutter root view.
     */
    public View getShutterRootView() {
        return mShutterLayout;
    }

    /**
     * Implementer of {@link OnShutterButtonListener}, receiver the UI event and notify the event
     * by the priority.
     */
    private class ShutterButtonListenerImpl implements ShutterButton.OnShutterButtonListener {

        @Override
        public void onShutterButtonFocused(boolean pressed) {
            Iterator iterator = mShutterButtonListeners.entrySet().iterator();
            OnShutterButtonListener listener;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                listener = (OnShutterButtonListener) map.getValue();
                if (listener != null && listener.onShutterButtonFocus(pressed)) {
                    return;
                }
            }
        }

        @Override
        public void onShutterButtonClicked() {
            Iterator iterator = mShutterButtonListeners.entrySet().iterator();
            OnShutterButtonListener listener;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                listener = (OnShutterButtonListener) map.getValue();
                if (listener != null && listener.onShutterButtonClick()) {
                    return;
                }
            }
        }

        @Override
        public void onShutterButtonLongPressed() {
            Iterator iterator = mShutterButtonListeners.entrySet().iterator();
            OnShutterButtonListener listener;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                listener = (OnShutterButtonListener) map.getValue();
                if (listener != null && listener.onShutterButtonLongPressed()) {
                    return;
                }
            }
        }
    }
    private static final AtomicInteger sNextGenerateId = new AtomicInteger(1);

    private static int generateViewId() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR1) {
            for (; ; ) {
                final int result = sNextGenerateId.get();
                //aapt-generated IDs have the high byte nonzero; clamp to the range under that.
                int newValue = result + 1;
                if (newValue > 0x00FFFFFF) {
                    newValue = 1; //Roll over to 1, not 0.
                }
                if (sNextGenerateId.compareAndSet(result, newValue)) {
                    return result;
                }
            }
        } else {
            return View.generateViewId();
        }
    }
}
