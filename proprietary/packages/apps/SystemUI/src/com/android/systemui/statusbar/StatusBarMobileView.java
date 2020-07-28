/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.systemui.statusbar;

import static com.android.systemui.plugins.DarkIconDispatcher.getTint;
import static com.android.systemui.plugins.DarkIconDispatcher.isInArea;
import static com.android.systemui.statusbar.StatusBarIconView.STATE_DOT;
import static com.android.systemui.statusbar.StatusBarIconView.STATE_HIDDEN;
import static com.android.systemui.statusbar.StatusBarIconView.STATE_ICON;

import android.content.Context;
import android.content.res.ColorStateList;
import android.graphics.Color;
import android.graphics.Rect;
import android.os.SystemProperties;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;

import com.android.internal.annotations.VisibleForTesting;
import com.android.settingslib.graph.SignalDrawable;
import com.android.systemui.DualToneHandler;
import com.android.systemui.R;
import com.android.systemui.plugins.DarkIconDispatcher.DarkReceiver;
import com.android.systemui.statusbar.phone.StatusBarSignalPolicy.MobileIconState;

import com.mediatek.systemui.ext.ISystemUIStatusBarExt;
import com.mediatek.systemui.ext.OpSystemUICustomizationFactoryBase;
import com.mediatek.systemui.statusbar.util.FeatureOptions;

public class StatusBarMobileView extends FrameLayout implements DarkReceiver,
        StatusIconDisplayable {
    private static final String TAG = "StatusBarMobileView";
    private static final boolean DEBUG = Log.isLoggable(TAG, Log.DEBUG)
            || FeatureOptions.LOG_ENABLE;

    /// Used to show etc dots
    private StatusBarIconView mDotView;
    /// The main icon view
    private LinearLayout mMobileGroup;
    private String mSlot;
    private MobileIconState mState;
    private SignalDrawable mMobileDrawable;
    private View mInoutContainer;
    private ImageView mIn;
    private ImageView mOut;
    private ImageView mMobile, mMobileType, mMobileRoaming;
    //private View mMobileRoamingSpace;
    private int mVisibleState = -1;
    private DualToneHandler mDualToneHandler;
    /// M: Add for new features @{
    // Add for [Network Type and volte on Statusbar]
    private ImageView mNetworkType;
    private ImageView mVolteType;
    /// @}
    /// M: for vowifi
    private boolean mIsWfcEnable;
    private boolean mIsWfcCase;

    /// M: Add for Plugin feature @ {
    private ISystemUIStatusBarExt mStatusBarExt;
    /// @ }

    public static StatusBarMobileView fromContext(Context context, String slot) {
        LayoutInflater inflater = LayoutInflater.from(context);
        StatusBarMobileView v = (StatusBarMobileView)
                inflater.inflate(R.layout.status_bar_mobile_signal_group, null);

        v.setSlot(slot);
        v.init();
        v.setVisibleState(STATE_ICON);
        return v;
    }

    public StatusBarMobileView(Context context) {
        super(context);
    }

    public StatusBarMobileView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public StatusBarMobileView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public StatusBarMobileView(Context context, AttributeSet attrs, int defStyleAttr,
            int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    @Override
    public void getDrawingRect(Rect outRect) {
        super.getDrawingRect(outRect);
        float translationX = getTranslationX();
        float translationY = getTranslationY();
        outRect.left += translationX;
        outRect.right += translationX;
        outRect.top += translationY;
        outRect.bottom += translationY;
    }

    private void init() {
        mDualToneHandler = new DualToneHandler(getContext());
        mMobileGroup = findViewById(R.id.mobile_group);
        mMobile = findViewById(R.id.mobile_signal);
        mMobileType = findViewById(R.id.mobile_type);
        mMobileRoaming = findViewById(R.id.mobile_roaming);
        //mMobileRoamingSpace = findViewById(R.id.mobile_roaming_space);
        mIn = findViewById(R.id.mobile_in);
        mOut = findViewById(R.id.mobile_out);
        mInoutContainer = findViewById(R.id.inout_container);
        /// M: Add for [Network Type and volte on Statusbar] @{
        mNetworkType    = findViewById(R.id.network_type);
        mVolteType      = findViewById(R.id.volte_indicator_ext);
        /// @}

        mMobileDrawable = new SignalDrawable(getContext());
        mMobile.setImageDrawable(mMobileDrawable);

        initDotView();

        mIsWfcEnable = SystemProperties.get("persist.vendor.mtk_wfc_support").equals("1");

        /// M: Add for Plugin feature @ {
        mStatusBarExt = OpSystemUICustomizationFactoryBase.getOpFactory(mContext)
                                     .makeSystemUIStatusBar(mContext);
        /// @ }
    }

    private void initDotView() {
        mDotView = new StatusBarIconView(mContext, mSlot, null);
        mDotView.setVisibleState(STATE_DOT);

        int width = mContext.getResources().getDimensionPixelSize(R.dimen.status_bar_icon_size);
        LayoutParams lp = new LayoutParams(width, width);
        lp.gravity = Gravity.CENTER_VERTICAL | Gravity.START;
        addView(mDotView, lp);
    }

    public void applyMobileState(MobileIconState state) {
        if (DEBUG) {
            Log.d(getMobileTag(), "[" + this.hashCode() + "][visibility=" + getVisibility()
                + "] applyMobileState: state = " + state);
        }
        boolean requestLayout = false;
        if (state == null) {
            requestLayout = getVisibility() != View.GONE;
            setVisibility(View.GONE);
            mState = null;
        } else if (mState == null) {
            requestLayout = true;
            mState = state.copy();
            initViewState();
        } else if (!mState.equals(state)) {
            requestLayout = updateState(state.copy());
        }

        if (requestLayout) {
            requestLayout();
        }
    }

    private void initViewState() {
        setContentDescription(mState.contentDescription);
        if (!mState.visible) {
            mMobileGroup.setVisibility(View.GONE);
        } else {
            mMobileGroup.setVisibility(View.VISIBLE);
        }
        mMobileDrawable.setLevel(mState.strengthId);
        if (mState.typeId > 0) {
            if (!mStatusBarExt.disableHostFunction()) {
                mMobileType.setContentDescription(mState.typeContentDescription);
                mMobileType.setImageResource(mState.typeId);
            }
            mMobileType.setVisibility(View.VISIBLE);
        } else {
            mMobileType.setVisibility(View.GONE);
        }

        mMobileRoaming.setVisibility(mState.roaming ? View.VISIBLE : View.GONE);
        //mMobileRoamingSpace.setVisibility(mState.roaming ? View.VISIBLE : View.GONE);
        mIn.setVisibility(mState.activityIn ? View.VISIBLE : View.GONE);
        mOut.setVisibility(mState.activityOut ? View.VISIBLE : View.GONE);
        mInoutContainer.setVisibility((mState.activityIn || mState.activityOut)
                ? View.VISIBLE : View.GONE);

        /// M: Add for [Network Type and volte on Statusbar] @{
        setCustomizeViewProperty();
        /// @}
        showWfcIfAirplaneMode();

        /// M: Add data group for plugin feature. @ {
        mStatusBarExt.addCustomizedView(mState.subId, mContext, mMobileGroup);
        setCustomizedOpViews();
        /// @ }
    }

    private boolean updateState(MobileIconState state) {
        boolean needsLayout = false;

        setContentDescription(state.contentDescription);
        if (mState.visible != state.visible) {
            mMobileGroup.setVisibility(state.visible ? View.VISIBLE : View.GONE);
            needsLayout = true;
        }
        if (mState.strengthId != state.strengthId) {
            mMobileDrawable.setLevel(state.strengthId);
        }
        if (mState.typeId != state.typeId) {
            needsLayout |= state.typeId == 0 || mState.typeId == 0;
            if (state.typeId != 0) {
                if (!mStatusBarExt.disableHostFunction()) {
                    mMobileType.setContentDescription(state.typeContentDescription);
                    mMobileType.setImageResource(state.typeId);
                }
                mMobileType.setVisibility(View.VISIBLE);
            } else {
                mMobileType.setVisibility(View.GONE);
            }
        }

        mMobileRoaming.setVisibility(state.roaming ? View.VISIBLE : View.GONE);
        //mMobileRoamingSpace.setVisibility(state.roaming ? View.VISIBLE : View.GONE);
        mIn.setVisibility(state.activityIn ? View.VISIBLE : View.GONE);
        mOut.setVisibility(state.activityOut ? View.VISIBLE : View.GONE);
        mInoutContainer.setVisibility((state.activityIn || state.activityOut)
                ? View.VISIBLE : View.GONE);

        needsLayout |= state.roaming != mState.roaming
                || state.activityIn != mState.activityIn
                || state.activityOut != mState.activityOut;

        /// M: Add for [Network Type and volte on Statusbar] @{
        if (mState.networkIcon != state.networkIcon) {
            setNetworkIcon(state.networkIcon);
        }
        if (mState.volteIcon != state.volteIcon) {
            setVolteIcon(state.volteIcon);
        }
        if (mState.mCustomizedState != state.mCustomizedState
                || mState.networkIcon != state.networkIcon) {
            // if cs reg state has changed or network icon change to LTE,need to update.
            mStatusBarExt.setDisVolteView(mState.subId, state.volteIcon, mVolteType);
        }
        /// @}

        mState = state;
        // should added after set mState
        showWfcIfAirplaneMode();
        setCustomizedOpViews();

        return needsLayout;
    }

    @Override
    public void onDarkChanged(Rect area, float darkIntensity, int tint) {
        if (!isInArea(area, this)) {
            return;
        }
        mMobileDrawable.setTintList(
                ColorStateList.valueOf(mDualToneHandler.getSingleColor(darkIntensity)));
        ColorStateList color = ColorStateList.valueOf(getTint(area, this, tint));
        mIn.setImageTintList(color);
        mOut.setImageTintList(color);
        mMobileType.setImageTintList(color);
        mMobileRoaming.setImageTintList(color);
        mNetworkType.setImageTintList(color);
        mVolteType.setImageTintList(color);
        mDotView.setDecorColor(tint);
        mDotView.setIconColor(tint, false);
        /// M: Add for plugin items tint handling. @{
        mMobile.setImageTintList(color);
        mStatusBarExt.setCustomizedPlmnTextTint(tint);
        mStatusBarExt.setIconTint(color);
        /// @}
    }

    @Override
    public String getSlot() {
        return mSlot;
    }

    public void setSlot(String slot) {
        mSlot = slot;
    }

    @Override
    public void setStaticDrawableColor(int color) {
        ColorStateList list = ColorStateList.valueOf(color);
        float intensity = color == Color.WHITE ? 0 : 1;
        // We want the ability to change the theme from the one set by SignalDrawable in certain
        // surfaces. In this way, we can pass a theme to the view.
        mMobileDrawable.setTintList(
                ColorStateList.valueOf(mDualToneHandler.getSingleColor(intensity)));
        mIn.setImageTintList(list);
        mOut.setImageTintList(list);
        mMobileType.setImageTintList(list);
        mMobileRoaming.setImageTintList(list);
        mNetworkType.setImageTintList(list);
        mVolteType.setImageTintList(list);
        mDotView.setDecorColor(color);
        /// M: Add for plugin items tint handling. @{
        mMobile.setImageTintList(list);
        mStatusBarExt.setCustomizedPlmnTextTint(color);
        mStatusBarExt.setIconTint(list);
        /// @}
    }

    @Override
    public void setDecorColor(int color) {
        mDotView.setDecorColor(color);
    }

    @Override
    public boolean isIconVisible() {
        return mState.visible || needShowWfcInAirplaneMode();
    }

    @Override
    public void setVisibleState(int state, boolean animate) {
        if (state == mVisibleState) {
            return;
        }

        mVisibleState = state;
        switch (state) {
            case STATE_ICON:
                mMobileGroup.setVisibility(View.VISIBLE);
                mDotView.setVisibility(View.GONE);
                break;
            case STATE_DOT:
                mMobileGroup.setVisibility(View.INVISIBLE);
                mDotView.setVisibility(View.VISIBLE);
                break;
            case STATE_HIDDEN:
            default:
                mMobileGroup.setVisibility(View.INVISIBLE);
                mDotView.setVisibility(View.INVISIBLE);
                break;
        }
    }

    @Override
    public int getVisibleState() {
        return mVisibleState;
    }

    @VisibleForTesting
    public MobileIconState getState() {
        return mState;
    }

    @Override
    public String toString() {
        return "StatusBarMobileView(slot=" + mSlot + ", hash=" + this.hashCode()
                + ", state=" + mState + ")";
    }

    /// M: Set all added or customised view. @ {
    private void setCustomizeViewProperty() {
        // Add for [Network Type on Statusbar], the place to set network type icon.
        setNetworkIcon(mState.networkIcon);
        /// M: Add for volte icon.
        setVolteIcon(mState.volteIcon);
    }

    /// M: Add for volte icon on Statusbar @{
    private void setVolteIcon(int volteIcon) {
        if (volteIcon > 0) {
            mVolteType.setImageResource(volteIcon);
            mVolteType.setVisibility(View.VISIBLE);
        } else {
            mVolteType.setVisibility(View.GONE);
        }
        /// M: customize VoLTE icon. @{
        mStatusBarExt.setCustomizedVolteView(volteIcon, mVolteType);
        mStatusBarExt.setDisVolteView(mState.subId, volteIcon, mVolteType);
        /// M: customize VoLTE icon. @}
    }
    ///@}

    /// M : Add for [Network Type on Statusbar] @{
    private void setNetworkIcon(int networkIcon) {
        // Network type is CTA feature, so non CTA project should not set this.
        if ((!FeatureOptions.MTK_CTA_SET) /* TODO || mIsWfcCase*/) {
            return;
        }
        if (networkIcon > 0) {
            if (!mStatusBarExt.disableHostFunction()) {
                mNetworkType.setImageResource(networkIcon);
            }
            mNetworkType.setVisibility(View.VISIBLE);
        } else {
            mNetworkType.setVisibility(View.GONE);
        }
    }
    ///@}

    /// M: Add for plugin features. @{
    private void setCustomizedOpViews() {
        mStatusBarExt.SetHostViewInvisible(mMobileRoaming);
        mStatusBarExt.SetHostViewInvisible(mIn);
        mStatusBarExt.SetHostViewInvisible(mOut);
        if (mState.visible) {
            mStatusBarExt.getServiceStateForCustomizedView(mState.subId);
            mStatusBarExt.setCustomizedNetworkTypeView(
                    mState.subId, mState.networkIcon, mNetworkType);
            mStatusBarExt.setCustomizedDataTypeView(
                mState.subId, mState.typeId, mState.mDataActivityIn, mState.mDataActivityOut);
            mStatusBarExt.setCustomizedSignalStrengthView(
                mState.subId, mState.strengthId, mMobile);
            mStatusBarExt.setCustomizedMobileTypeView(
                mState.subId, mMobileType);
            mStatusBarExt.setCustomizedView(mState.subId);
        }
    }
    /// @}

    /**
     * If in airplane mode, and in wifi calling state, should show wfc icon and
     * hide other icons
     */
    private void showWfcIfAirplaneMode() {
        if (needShowWfcInAirplaneMode()) {
            if (DEBUG) {
                Log.d(getMobileTag(), "showWfcIfAirplaneMode: show wfc in airplane mode");
            }
            mMobileGroup.setVisibility(View.VISIBLE);
            mMobile.setVisibility(View.GONE);
            mMobileType.setVisibility(View.GONE);
            mNetworkType.setVisibility(View.GONE);
            mMobileRoaming.setVisibility(View.GONE);
            mIsWfcCase = true;
            requestLayout();
        } else {
            if (mIsWfcCase) {
                if (DEBUG) {
                    Log.d(getMobileTag(), "showWfcIfAirplaneMode: recover to show mobile view");
                }
                mMobile.setVisibility(View.VISIBLE);
                mIsWfcCase = false;
                requestLayout();
            }
        }
    }

    private boolean needShowWfcInAirplaneMode() {
        return mIsWfcEnable && !mState.visible && mState.volteIcon != 0;
    }

    /**
     * get tag with subscription id
     * @return tag for logcat
     */
    private String getMobileTag() {
        return String.format(TAG + "(%d)", mState != null ? mState.subId : -1);
    }
}
