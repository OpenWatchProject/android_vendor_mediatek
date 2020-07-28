/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.android.browser;

import com.android.browser.UI.ComboViews;

import android.animation.Animator;
import android.animation.Animator.AnimatorListener;
import android.animation.ObjectAnimator;
import android.content.Context;
import android.content.res.Resources;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.DecelerateInterpolator;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;


/**
 * Base class for a bottom bar used by the browser.
 */
public class BottomBar extends LinearLayout {

    private static final float ANIM_TITLEBAR_DECELERATE = 2.5f;
    private static final String TAG = "BottomBar";
    private Context mContext;
    private UiController mUiController;
    private BaseUi mBaseUi;
    private FrameLayout mContentView;
    private TabControl mTabControl;

    private boolean mUseQuickControls;
    private boolean mUseFullScreen;
    protected LinearLayout mBottomBar;
    protected ImageView mBottomBarBack;
    protected ImageView mBottomBarForward;
    protected ImageView mBottomBarTabs;
    protected ImageView mBottomBarBookmarks;
    protected TextView mBottomBarTabCount;
    //state
    private boolean mShowing;
    private Animator mBottomBarAnimator;
    private int mContentViewHeight;

    public BottomBar(Context context, UiController controller,
                     BaseUi ui, final TabControl tabControl,
                     FrameLayout contentView) {
        super(context, null);
        mContext = context;
        mUiController = controller;
        mBaseUi = ui;
        mTabControl = tabControl;
        mContentView = contentView;
        initLayout(context);
        setupBottomBar();
    }

    private void initLayout(Context context) {
        LayoutInflater factory = LayoutInflater.from(context);
        factory.inflate(R.layout.bottom_bar, this);
        mBottomBar = (LinearLayout) findViewById(
                R.id.bottombar);
        mBottomBarBack = (ImageView) findViewById(
                R.id.back);
        mBottomBarForward = (ImageView) findViewById(
                R.id.forward);
        mBottomBarTabs = (ImageView) findViewById(
                R.id.tabs);
        mBottomBarBookmarks = (ImageView) findViewById(
                R.id.bookmarks);
        mBottomBarTabCount = (TextView) findViewById(
                R.id.tabcount);

        mBottomBarBack.setOnClickListener(new OnClickListener() {
            public void onClick(View arg0) {
                ((Controller) mUiController).onBackKey();
            }
        });
        mBottomBarBack.setOnLongClickListener(new OnLongClickListener() {
            @Override
            public boolean onLongClick(View arg0) {
                Toast.makeText(mUiController.getActivity(),
                        mUiController.getActivity().getResources()
                        .getString(R.string.back), 0).show();
                return false;
            }
        });
        mBottomBarForward.setOnClickListener(new OnClickListener() {
            public void onClick(View arg0) {
                if (mUiController != null && mUiController.getCurrentTab() != null) {
                    mUiController.getCurrentTab().goForward();
                }
            }
        });
        mBottomBarForward.setOnLongClickListener(new OnLongClickListener() {
            @Override
            public boolean onLongClick(View arg0) {
                Toast.makeText(mUiController.getActivity(),
                        mUiController.getActivity().getResources()
                        .getString(R.string.forward), 0).show();
                return false;
            }
        });
        mBottomBarTabs.setOnClickListener(new OnClickListener() {
            public void onClick(View arg0) {
                ((PhoneUi) mBaseUi).toggleNavScreen();
            }
        });
        mBottomBarTabs.setOnLongClickListener(new OnLongClickListener() {
            @Override
            public boolean onLongClick(View arg0) {
                Toast.makeText(mUiController.getActivity(),
                        mUiController.getActivity().getResources()
                        .getString(R.string.tabs), 0).show();
                return false;
            }
        });
        mBottomBarBookmarks.setOnClickListener(new OnClickListener() {
            public void onClick(View arg0) {
                mUiController.bookmarksOrHistoryPicker(ComboViews.Bookmarks);
            }
        });
        mBottomBarBookmarks.setOnLongClickListener(new OnLongClickListener() {
            @Override
            public boolean onLongClick(View arg0) {
                Toast.makeText(mUiController.getActivity(),
                        mUiController.getActivity().getResources()
                        .getString(R.string.bookmarks), 0).show();
                return false;
            }
        });
        mBottomBarTabCount.setText(
                Integer.toString(mUiController.getTabControl().getTabCount()));
        mTabControl.setOnTabCountChangedListener(new TabControl.OnTabCountChangedListener() {
            public void onTabCountChanged() {
                mBottomBarTabCount.setText(
                        Integer.toString(mTabControl.getTabCount()));
            }
        });
    }

    private void setupBottomBar() {
        ViewGroup parent = (ViewGroup) getParent();
        show();
        if (parent != null) {
            parent.removeView(this);
        }
        mContentView.addView(this, makeLayoutParams());
        mBaseUi.setContentViewMarginBottom(0);
    }

    public void setFullScreen(boolean use) {
        mUseFullScreen = use;
        if (use) {
            this.setVisibility(View.GONE);
        } else {
            this.setVisibility(View.VISIBLE);
        }
    }

    public void setUseQuickControls(boolean use) {
        mUseQuickControls = use;
        if (use) {
            this.setVisibility(View.GONE);
        } else {
            this.setVisibility(View.VISIBLE);
        }
    }

    void setupBottomBarAnimator(Animator animator) {
        Resources res = mContext.getResources();
        int duration = res.getInteger(R.integer.titlebar_animation_duration);
        animator.setInterpolator(new DecelerateInterpolator(
                ANIM_TITLEBAR_DECELERATE));
        animator.setDuration(duration);
    }

    void show() {
        //Xlog.i(TAG, "bottom bar show(), showing: " + mShowing + "IME: " + mIMEShow);
        cancelBottomBarAnimation();
        if (mUseQuickControls) {
            this.setVisibility(View.GONE);
            mShowing = false;
            return;
        } else if (!mShowing/* && !mIMEShow*/) {
            this.setVisibility(View.VISIBLE);
            int visibleHeight = getVisibleBottomHeight();
            float startPos = getTranslationY();
            this.setLayerType(View.LAYER_TYPE_HARDWARE, null);
            mBottomBarAnimator = ObjectAnimator.ofFloat(this,
                    "translationY", startPos, startPos - visibleHeight);
            setupBottomBarAnimator(mBottomBarAnimator);
            mBottomBarAnimator.start();
            mShowing = true;
        }
    }

    void hide() {
        //Xlog.i(TAG, "bottom bar hide(), showing: " + mShowing);
        if (mUseQuickControls || mUseFullScreen) {
            cancelBottomBarAnimation();
            int visibleHeight = getVisibleBottomHeight();
            float startPos = getTranslationY();
            this.setLayerType(View.LAYER_TYPE_HARDWARE, null);
            mBottomBarAnimator = ObjectAnimator.ofFloat(this,
                    "translationY", startPos, startPos + visibleHeight);
            mBottomBarAnimator.addListener(mHideBottomBarAnimatorListener);
            setupBottomBarAnimator(mBottomBarAnimator);
            mBottomBarAnimator.start();
            this.setVisibility(View.GONE);
            mShowing = false;
            return;
        } else {
            this.setVisibility(View.VISIBLE);
            cancelBottomBarAnimation();
            int visibleHeight = getVisibleBottomHeight();
            float startPos = getTranslationY();
            //Xlog.i(TAG, "hide(): visibleHeight: " + visibleHeight);
            //Xlog.i(TAG, "hide(): startPos: " + startPos);
            this.setLayerType(View.LAYER_TYPE_HARDWARE, null);
            mBottomBarAnimator = ObjectAnimator.ofFloat(this,
                    "translationY", startPos, startPos + visibleHeight);
            mBottomBarAnimator.addListener(mHideBottomBarAnimatorListener);
            setupBottomBarAnimator(mBottomBarAnimator);
            mBottomBarAnimator.start();
        }
        mShowing = false;
    }

    boolean isShowing() {
        return mShowing;
    }

    void cancelBottomBarAnimation() {
        if (mBottomBarAnimator != null) {
            mBottomBarAnimator.cancel();
            mBottomBarAnimator = null;
        }
    }

    private AnimatorListener mHideBottomBarAnimatorListener = new AnimatorListener() {

        @Override
        public void onAnimationStart(Animator animation) {
        }

        @Override
        public void onAnimationRepeat(Animator animation) {
        }

        @Override
        public void onAnimationEnd(Animator animation) {
            // update position
            onScrollChanged();
            BottomBar.this.setLayerType(View.LAYER_TYPE_NONE, null);
        }

        @Override
        public void onAnimationCancel(Animator animation) {
        }
    };

    private int getVisibleBottomHeight() {
        return mBottomBar.getHeight();
    }

    public boolean useQuickControls() {
        return mUseQuickControls;
    }

    private ViewGroup.MarginLayoutParams makeLayoutParams() {
        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(LayoutParams.MATCH_PARENT,
                LayoutParams.WRAP_CONTENT);
        params.gravity = Gravity.BOTTOM;
        return params;
    }

    public void onScrollChanged() {
        if (!mShowing) {
            setTranslationY(getVisibleBottomHeight());
        }
    }

    public void changeBottomBarState(boolean back, boolean forward) {
        mBottomBarBack.setEnabled(back);
        mBottomBarForward.setEnabled(forward);
    }

}
