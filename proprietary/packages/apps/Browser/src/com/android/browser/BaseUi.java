/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
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

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.LayerDrawable;
import android.graphics.drawable.PaintDrawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.webkit.WebChromeClient;
import android.webkit.WebView;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.Toast;

import com.android.browser.Tab.SecurityState;
import com.android.internal.view.menu.MenuBuilder;
import com.mediatek.browser.ext.IBrowserUrlExt;

import java.util.List;

/**
 * UI interface definitions
 */
public abstract class BaseUi implements UI {

    /// M: add for debug @ {
    private static final boolean DEBUG = Browser.DEBUG;
    private static final String TAG = "browser";
    /// @ }
    private static final String LOGTAG = "BaseUi";

    protected static final FrameLayout.LayoutParams COVER_SCREEN_PARAMS =
        new FrameLayout.LayoutParams(
        ViewGroup.LayoutParams.MATCH_PARENT,
        ViewGroup.LayoutParams.MATCH_PARENT);

    protected static final FrameLayout.LayoutParams COVER_SCREEN_GRAVITY_CENTER =
        new FrameLayout.LayoutParams(
        ViewGroup.LayoutParams.MATCH_PARENT,
        ViewGroup.LayoutParams.MATCH_PARENT,
        Gravity.CENTER);

    private static final int MSG_HIDE_TITLEBAR = 1;
    private static final int MSG_HIDE_BOTTOMBAR = 2;
    private static final int MSG_CLOSE_TABLEVIEW = 3;
    public static final int HIDE_TITLEBAR_DELAY = 2000; // in ms

    Activity mActivity;
    UiController mUiController;
    TabControl mTabControl;
    protected Tab mActiveTab;
    private InputMethodManager mInputManager;

    private Drawable mLockIconSecure;
    private Drawable mLockIconMixed;
    protected Drawable mGenericFavicon;

    protected FrameLayout mFrameLayout;
    protected FrameLayout mContentView;
    protected FrameLayout mCustomViewContainer;
    protected FrameLayout mFullscreenContainer;
    protected FrameLayout mFixedTitlebarContainer;

    private View mCustomView;
    private WebChromeClient.CustomViewCallback mCustomViewCallback;
    private int mOriginalOrientation;

    private LinearLayout mErrorConsoleContainer = null;

    private UrlBarAutoShowManager mUrlBarAutoShowManager;

    private Toast mStopToast;

    // the default <video> poster
    private Bitmap mDefaultVideoPoster;
    // the video progress view
    private View mVideoProgressView;

    private boolean mActivityPaused;
    protected boolean mUseQuickControls;
    protected TitleBar mTitleBar;
    /// M: add bottom bar
    protected BottomBar mBottomBar;
    protected boolean mNeedBottomBar;
    private NavigationBarBase mNavigationBar;
    protected PieControl mPieControl;
    private boolean mBlockFocusAnimations;

    //inputURL flag
    //launcher browser and show edit url
    private boolean mInputUrlFlag = false;
    public BaseUi(Activity browser, UiController controller) {
        mActivity = browser;
        mUiController = controller;
        mTabControl = controller.getTabControl();
        Resources res = mActivity.getResources();
        mInputManager = (InputMethodManager)
                browser.getSystemService(Activity.INPUT_METHOD_SERVICE);
        mLockIconSecure = res.getDrawable(R.drawable.ic_secure_holo_dark);
        mLockIconMixed = res.getDrawable(R.drawable.ic_secure_partial_holo_dark);
        mFrameLayout = (FrameLayout) mActivity.getWindow()
                .getDecorView().findViewById(android.R.id.content);
        LayoutInflater.from(mActivity)
                .inflate(R.layout.custom_screen, mFrameLayout);
        mFixedTitlebarContainer = (FrameLayout) mFrameLayout.findViewById(
                R.id.fixed_titlebar_container);
        mContentView = (FrameLayout) mFrameLayout.findViewById(
                R.id.main_content);
        mCustomViewContainer = (FrameLayout) mFrameLayout.findViewById(
                R.id.fullscreen_custom_content);
        mErrorConsoleContainer = (LinearLayout) mFrameLayout
                .findViewById(R.id.error_console);

        mGenericFavicon = res.getDrawable(
                R.drawable.app_web_browser_sm);
        mTitleBar = new TitleBar(mActivity, mUiController, this,
                mContentView);
        /// M: add bottom bar @{
        mNeedBottomBar = BrowserFeatureOption.BROWSER_BOTTOM_BAR_SUPPORT
                         && !BrowserActivity.isTablet(mActivity);
        if (mNeedBottomBar) {
            mBottomBar = new BottomBar(mActivity, mUiController, this, mTabControl,
                    mContentView);
        }
        /// @}
        setFullscreen(BrowserSettings.getInstance().useFullscreen());
        mTitleBar.setProgress(100);
        mNavigationBar = mTitleBar.getNavigationBar();
        mUrlBarAutoShowManager = new UrlBarAutoShowManager(this);
    }

    private void cancelStopToast() {
        if (mStopToast != null) {
            mStopToast.cancel();
            mStopToast = null;
        }
    }

    // lifecycle

    public void onPause() {
        if (isCustomViewShowing()) {
            onHideCustomView();
        }
        cancelStopToast();
        mActivityPaused = true;
    }

    public void onResume() {
        mActivityPaused = false;
        // check if we exited without setting active tab
        // b: 5188145
        final Tab ct = mTabControl.getCurrentTab();
        if (ct != null) {
            setActiveTab(ct);
        }
        mTitleBar.onResume();
    }

    protected boolean isActivityPaused() {
        return mActivityPaused;
    }

    public void onConfigurationChanged(Configuration config) {
    }

    public Activity getActivity() {
        return mActivity;
    }

    // key handling

    @Override
    public boolean onBackKey() {
        if (mCustomView != null) {
            mUiController.hideCustomView();
            return true;
        }
        return false;
    }

    @Override
    public boolean onMenuKey() {
        return false;
    }

    @Override
    public void setUseQuickControls(boolean useQuickControls) {
        mUseQuickControls = useQuickControls;
        /// M: set use QC @{
        if (mNeedBottomBar) {
            mBottomBar.setUseQuickControls(mUseQuickControls);
        }
        /// @}
        if (useQuickControls) {
            mPieControl = new PieControl(mActivity, mUiController, this);
            mPieControl.attachToContainer(mContentView);
        } else {
            if (mPieControl != null) {
                mPieControl.removeFromContainer(mContentView);
            }
        }
        updateUrlBarAutoShowManagerTarget();
    }

    // Tab callbacks
    @Override
    public void onTabDataChanged(Tab tab) {
        if (DEBUG) {
            Log.d(TAG, "BaseUi.onTabDataChanged()--->tab = " + tab);
        }
        setUrlTitle(tab);
        setFavicon(tab);
        updateLockIconToLatest(tab);
        updateNavigationState(tab);
        mTitleBar.onTabDataChanged(tab);
        mNavigationBar.onTabDataChanged(tab);
        onProgressChanged(tab);
    }

    @Override
    public void onProgressChanged(Tab tab) {
        int progress = tab.getLoadProgress();
        if (tab.inForeground()) {
            mTitleBar.setProgress(progress);
        }
    }

    @Override
    public void bookmarkedStatusHasChanged(Tab tab) {
        if (tab.inForeground()) {
            boolean isBookmark = tab.isBookmarkedSite();
            mNavigationBar.setCurrentUrlIsBookmark(isBookmark);
        }
    }

    @Override
    public void onPageStopped(Tab tab) {
        cancelStopToast();
        if (tab.inForeground()) {
            mStopToast = Toast
                    .makeText(mActivity, R.string.stopping, Toast.LENGTH_SHORT);
            mStopToast.show();
        }
    }

    @Override
    public boolean needsRestoreAllTabs() {
        return true;
    }

    @Override
    public void addTab(Tab tab) {
        if (DEBUG) {
            Log.d(TAG, "BaseUi.addTab()--->empty implemetion " + tab);
        }
    }

    @Override
    public void setActiveTab(final Tab tab) {
        if (DEBUG) {
            Log.d(TAG, "BaseUi.setActiveTab()--->tab = " + tab);
        }
        if (tab == null) return;
        // block unnecessary focus change animations during tab switch
        mBlockFocusAnimations = true;
        mHandler.removeMessages(MSG_HIDE_TITLEBAR);
        if ((tab != mActiveTab) && (mActiveTab != null)) {
            WebView view = mActiveTab.getTopWindow();
            if (view != null && view.hasFocus()) {
                mTitleBar.getNavigationBar().getUrlInputView().ignoreIME(true);
            }
            removeTabFromContentView(mActiveTab);
            WebView web = mActiveTab.getWebView();
            if (web != null) {
                web.setOnTouchListener(null);
            }
        }
        mActiveTab = tab;
        BrowserWebView web = (BrowserWebView) mActiveTab.getWebView();
        updateUrlBarAutoShowManagerTarget();
        attachTabToContentView(tab);
        if (web != null) {
            // Request focus on the top window.
            if (mUseQuickControls) {
                mPieControl.forceToTop(mContentView);
            }
            web.setTitleBar(mTitleBar);
            mTitleBar.onScrollChanged();
        }
        mTitleBar.bringToFront();
        /// M: bottom bar bring to front @{
        if (mNeedBottomBar) {
            mBottomBar.bringToFront();
        }
        /// @}
        tab.getTopWindow().requestFocus();
        mTitleBar.getNavigationBar().getUrlInputView().ignoreIME(false);
        setShouldShowErrorConsole(tab, mUiController.shouldShowErrorConsole());
        onTabDataChanged(tab);
        onProgressChanged(tab);
        mNavigationBar.setIncognitoMode(tab.isPrivateBrowsingEnabled());
        updateAutoLogin(tab, false);
        mBlockFocusAnimations = false;
    }

    protected void updateUrlBarAutoShowManagerTarget() {
        WebView web = mActiveTab != null ? mActiveTab.getWebView() : null;
        if (!mUseQuickControls && web instanceof BrowserWebView) {
            mUrlBarAutoShowManager.setTarget((BrowserWebView) web);
        } else {
            mUrlBarAutoShowManager.setTarget(null);
        }
    }

    Tab getActiveTab() {
        return mActiveTab;
    }

    @Override
    public void updateTabs(List<Tab> tabs) {
    }

    @Override
    public void removeTab(Tab tab) {
        if (DEBUG) {
            Log.d(TAG, "BaseUi.removeTab()--->tab = " + tab);
        }
        if (mActiveTab == tab) {
            removeTabFromContentView(tab);
            mActiveTab = null;
        }
    }

    @Override
    public void detachTab(Tab tab) {
        if (DEBUG) {
            Log.d(TAG, "BaseUi.detachTab()--->tab = " + tab);
        }
        removeTabFromContentView(tab);
    }

    @Override
    public void attachTab(Tab tab) {
        if (DEBUG) {
            Log.d(TAG, "BaseUi.attachTab()--->tab = " + tab);
        }
        attachTabToContentView(tab);
    }

    protected void attachTabToContentView(Tab tab) {
        if (DEBUG) {
            Log.d(TAG, "BaseUi.attachTabToContentView()--->tab = " + tab);
        }
        if ((tab == null) || (tab.getWebView() == null)) {
            return;
        }
        View container = tab.getViewContainer();
        WebView mainView  = tab.getWebView();
        // Attach the WebView to the container and then attach the
        // container to the content view.
        FrameLayout wrapper =
                (FrameLayout) container.findViewById(R.id.webview_wrapper);
        ViewGroup parent = (ViewGroup) mainView.getParent();
        if (parent != wrapper) {
            if (parent != null) {
                parent.removeView(mainView);
            }
            wrapper.addView(mainView);
        }
        parent = (ViewGroup) container.getParent();
        if (parent != mContentView) {
            if (parent != null) {
                parent.removeView(container);
            }
            mContentView.addView(container, COVER_SCREEN_PARAMS);
        }
        mUiController.attachSubWindow(tab);
    }

    private void removeTabFromContentView(Tab tab) {
        if (DEBUG) {
            Log.d(TAG, "BaseUi.removeTabFromContentView()--->tab = " + tab);
        }
        hideTitleBar();
        /// M: avoid null pointer exception @ {
        if (tab == null) {
            return;
        }
        /// @ }
        // Remove the container that contains the main WebView.
        WebView mainView = tab.getWebView();
        View container = tab.getViewContainer();
        if (mainView == null) {
            return;
        }
        // Remove the container from the content and then remove the
        // WebView from the container. This will trigger a focus change
        // needed by WebView.
        FrameLayout wrapper =
                (FrameLayout) container.findViewById(R.id.webview_wrapper);
        wrapper.removeView(mainView);
        mContentView.removeView(container);
        mUiController.endActionMode();
        mUiController.removeSubWindow(tab);
        ErrorConsoleView errorConsole = tab.getErrorConsole(false);
        if (errorConsole != null) {
            mErrorConsoleContainer.removeView(errorConsole);
        }
    }

    @Override
    public void onSetWebView(Tab tab, WebView webView) {
        if (DEBUG) {
            Log.d(TAG, "BaseUi.onSetWebView()--->tab = " + tab + ", webView = " + webView);
        }
        View container = tab.getViewContainer();
        if (container == null) {
            // The tab consists of a container view, which contains the main
            // WebView, as well as any other UI elements associated with the tab.
            container = mActivity.getLayoutInflater().inflate(R.layout.tab,
                    mContentView, false);
            tab.setViewContainer(container);
        }
        if (tab.getWebView() != webView) {
            // Just remove the old one.
            FrameLayout wrapper =
                    (FrameLayout) container.findViewById(R.id.webview_wrapper);
            wrapper.removeView(tab.getWebView());
        }
    }

    /**
     * create a sub window container and webview for the tab
     * Note: this methods operates through side-effects for now
     * it sets both the subView and subViewContainer for the given tab
     * @param tab tab to create the sub window for
     * @param subView webview to be set as a subwindow for the tab
     */
    @Override
    public void createSubWindow(Tab tab, WebView subView) {
        if (DEBUG && subView != null) {
            Log.d(TAG, "BaseUi.createSubWindow()--->subView()--->width = "
                + subView.getWidth() + ", view.height = " + subView.getHeight());
        }
        View subViewContainer = mActivity.getLayoutInflater().inflate(
                R.layout.browser_subwindow, null);
        ViewGroup inner = (ViewGroup) subViewContainer
                .findViewById(R.id.inner_container);
        inner.addView(subView, new LayoutParams(LayoutParams.MATCH_PARENT,
                LayoutParams.MATCH_PARENT));
        final ImageButton cancel = (ImageButton) subViewContainer
                .findViewById(R.id.subwindow_close);
        final WebView cancelSubView = subView;
        cancel.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                ((BrowserWebView) cancelSubView).getWebChromeClient().onCloseWindow(cancelSubView);
            }
        });
        tab.setSubWebView(subView);
        tab.setSubViewContainer(subViewContainer);
    }

    /**
     * Remove the sub window from the content view.
     */
    @Override
    public void removeSubWindow(View subviewContainer) {
        if (DEBUG) {
            Log.d(TAG, "BaseUi.removeSubWindow()--->");
        }
        mContentView.removeView(subviewContainer);
        mUiController.endActionMode();
    }

    /**
     * Attach the sub window to the content view.
     */
    @Override
    public void attachSubWindow(View container) {
        if (DEBUG) {
            Log.d(TAG, "BaseUi.attachSubWindow()--->");
        }
        if (container.getParent() != null) {
            // already attached, remove first
            ((ViewGroup) container.getParent()).removeView(container);
        }
        mContentView.addView(container, COVER_SCREEN_PARAMS);
    }

    protected void refreshWebView() {
        WebView web = getWebView();
        if (web != null) {
            web.invalidate();
        }
    }

    public void editUrl(boolean clearInput, boolean forceIME) {
        if (DEBUG) {
            Log.d(TAG, "BaseUi.editUrl()--->editUrl = " + clearInput
                + ", forceIME = " + forceIME);
        }
        if (mUiController.isInCustomActionMode()) {
            mUiController.endActionMode();
        }
        showTitleBar();
        if ((getActiveTab() != null) && !getActiveTab().isSnapshot()) {
            mNavigationBar.startEditingUrl(clearInput, forceIME);
        }
    }

    boolean canShowTitleBar() {
        return !isTitleBarShowing()
                && !isActivityPaused()
                && (getActiveTab() != null)
                && (getWebView() != null)
                && !mUiController.isInCustomActionMode();
    }

    protected void showTitleBar() {
        mHandler.removeMessages(MSG_HIDE_TITLEBAR);
        if (canShowTitleBar()) {
            mTitleBar.show();
        }
    }

    protected void hideTitleBarOnly() {
        if (mTitleBar.isShowing()) {
            mTitleBar.hide();
        }
    }

    protected void hideTitleBar() {
        if (mTitleBar.isShowing()) {
            mTitleBar.hide();
        }
        /// M: hide bottom bar when title bar hide
        hideBottomBar();
    }

    /// M: add bottom bar feature interface @{
    protected void showBottomBar() {
        if (mNeedBottomBar && mBottomBar != null && !mBottomBar.isShowing()) {
            if (getWebView() != null/* && !(getWebView().canScrollVertically(-1)
                    || getWebView().canScrollVertically(1))*/) {
                mBottomBar.hide();
            } else {
                mBottomBar.show();
            }
        }
    }

    protected void showBottomBarMust() {
        if (mNeedBottomBar && mBottomBar != null && !mBottomBar.isShowing()) {
            mBottomBar.show();
        }
    }

    protected void hideBottomBar() {
        if (mNeedBottomBar && mBottomBar != null && mBottomBar.isShowing()) {
            mBottomBar.hide();
        }
    }
    /// @}

    protected boolean isTitleBarShowing() {
        return View.VISIBLE == mTitleBar.getVisibility();
    }

    public boolean isEditingUrl() {
        return mTitleBar.isEditingUrl();
    }

    public void stopEditingUrl() {
        mTitleBar.getNavigationBar().stopEditingUrl();
    }

    public TitleBar getTitleBar() {
        return mTitleBar;
    }

    @Override
    public void showVoiceTitleBar(String title, List<String> results) {
        //mNavigationBar.setInVoiceMode(true, results);
        mNavigationBar.setDisplayTitle(title);
    }

    @Override
    public void revertVoiceTitleBar(Tab tab) {
        //mNavigationBar.setInVoiceMode(false, null);
        String url = tab.getUrl();
        String title = tab.getTitle();
        //mNavigationBar.setDisplayTitle(url);

        /// M: Set the navigation bar title. @{
        mBrowserUrlExt = Extensions.getUrlPlugin(mActivity);
        mNavigationBar.setDisplayTitle(mBrowserUrlExt.getNavigationBarTitle(title, url));
        /// @}

    }

    @Override
    public void showComboView(ComboViews startingView, Bundle extras) {
        if (DEBUG && startingView != null) {
            Log.d(TAG, "BaseUi.showComboView()--->startingView = " + startingView.toString());
        }
        Intent intent = new Intent(mActivity, ComboViewActivity.class);
        intent.putExtra(ComboViewActivity.EXTRA_INITIAL_VIEW, startingView.name());
        intent.putExtra(ComboViewActivity.EXTRA_COMBO_ARGS, extras);
        Tab t = getActiveTab();
        if (t != null) {
            intent.putExtra(ComboViewActivity.EXTRA_CURRENT_URL, t.getUrl());
        }
        mActivity.startActivityForResult(intent, Controller.COMBO_VIEW);
    }

    @Override
    public void showCustomView(View view, int requestedOrientation,
            WebChromeClient.CustomViewCallback callback) {
        // if a view already exists then immediately terminate the new one
        if (mCustomView != null) {
            callback.onCustomViewHidden();
            return;
        }

        mOriginalOrientation = mActivity.getRequestedOrientation();
        FrameLayout decor = (FrameLayout) mActivity.getWindow().getDecorView();
        mFullscreenContainer = new FullscreenHolder(mActivity);
        mFullscreenContainer.addView(view, COVER_SCREEN_PARAMS);
        /// M: Apply the full screen for FrameLayout only.
        //decor.addView(mFullscreenContainer, COVER_SCREEN_PARAMS);
        mFrameLayout.addView(mFullscreenContainer, COVER_SCREEN_PARAMS);
        mCustomView = view;
        setFullscreen(true);
        mFixedTitlebarContainer.setVisibility(View.INVISIBLE);
        mTitleBar.getNavigationBar().getUrlInputView().setVisibility(View.INVISIBLE);
        /// M: add for ALPS03422406
        //((BrowserWebView) getWebView()).setVisibility(View.INVISIBLE);
        mCustomViewCallback = callback;
        mActivity.setRequestedOrientation(requestedOrientation);
    }

    @Override
    public void onHideCustomView() {
        BrowserWebView view = (BrowserWebView) getWebView();
        /// M: for ALPS03422406
        if (view != null && view.getVisibility() == View.INVISIBLE) {
            view.setVisibility(View.VISIBLE);
        }
        mFixedTitlebarContainer.setVisibility(View.VISIBLE);
        mTitleBar.getNavigationBar().getUrlInputView().setVisibility(View.VISIBLE);
        if (mCustomView == null)
            return;
        setFullscreen(BrowserSettings.getInstance().useFullscreen());
        FrameLayout decor = (FrameLayout) mActivity.getWindow().getDecorView();
        /// M: Remove the full screen from FrameLayout only.
        //decor.removeView(mFullscreenContainer);
        mFrameLayout.removeView(mFullscreenContainer);
        mFullscreenContainer = null;
        mCustomView = null;
        mCustomViewCallback.onCustomViewHidden();
        // Show the content view.
        mActivity.setRequestedOrientation(mOriginalOrientation);
        view.clearFocus();
        view.requestFocus();
    }

    @Override
    public boolean isCustomViewShowing() {
        return mCustomView != null;
    }

    protected void dismissIME() {
        if (mInputManager.isActive()) {
            mInputManager.hideSoftInputFromWindow(mContentView.getWindowToken(),
                    0);
        }
    }

    @Override
    public boolean isWebShowing() {
        return mCustomView == null;
    }

    @Override
    public void showAutoLogin(Tab tab) {
        updateAutoLogin(tab, true);
    }

    @Override
    public void hideAutoLogin(Tab tab) {
        updateAutoLogin(tab, true);
    }

    // -------------------------------------------------------------------------

    protected void updateNavigationState(Tab tab) {
    }

    protected void updateAutoLogin(Tab tab, boolean animate) {
        mTitleBar.updateAutoLogin(tab, animate);
    }

    /**
     * Update the lock icon to correspond to our latest state.
     */
    protected void updateLockIconToLatest(Tab t) {
        if (t != null && t.inForeground()) {
            updateLockIconImage(t.getSecurityState());
        }
    }

    /**
     * Updates the lock-icon image in the title-bar.
     */
    private void updateLockIconImage(SecurityState securityState) {
        Drawable d = null;
        if (securityState == SecurityState.SECURITY_STATE_SECURE) {
            d = mLockIconSecure;
        } else if (securityState == SecurityState.SECURITY_STATE_MIXED
                || securityState == SecurityState.SECURITY_STATE_BAD_CERTIFICATE) {
            // TODO: It would be good to have different icons for insecure vs mixed content.
            // See http://b/5403800
            d = mLockIconMixed;
        }
        mNavigationBar.setLock(d);
    }

    /// M: Browser url plugin.
    private IBrowserUrlExt mBrowserUrlExt = null;

    protected void setUrlTitle(Tab tab) {
        String url = tab.getUrl();
        String title = tab.getTitle();
        Log.i(LOGTAG, "Load Progress: " + tab.getLoadProgress() +
              "inPageLoad: " + tab.inPageLoad());
        if (TextUtils.isEmpty(title) ||
                (!tab.inPageLoad() &&
                title.equals(mActivity.getString(R.string.title_bar_loading)))) {
            title = url;
        }
        if (tab.inForeground()) {
            if (url.startsWith("file://")) {
                mNavigationBar.setDisplayTitle(title);
            } else {
                /// M: Set the navigation bar title. @{
                mBrowserUrlExt = Extensions.getUrlPlugin(mActivity);
                mNavigationBar.setDisplayTitle(mBrowserUrlExt.getNavigationBarTitle(title, url));
                /// @}
            }
        }
    }

    // Set the favicon in the title bar.
    protected void setFavicon(Tab tab) {
        if (tab.inForeground()) {
            Bitmap icon = tab.getFavicon();
            mNavigationBar.setFavicon(icon);
        }
    }

    @Override
    public void onActionModeFinished(boolean inLoad) {
    }

    // active tabs page

    public void showActiveTabsPage() {
    }

    /**
     * Remove the active tabs page.
     */
    public void removeActiveTabsPage() {
    }

    // menu handling callbacks

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        return true;
    }

    @Override
    public void updateMenuState(Tab tab, Menu menu) {
    }

    @Override
    public void onOptionsMenuOpened() {
    }

    @Override
    public void onExtendedMenuOpened() {
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        return false;
    }

    @Override
    public void onOptionsMenuClosed(boolean inLoad) {
    }

    @Override
    public void onExtendedMenuClosed(boolean inLoad) {
    }

    @Override
    public void onContextMenuCreated(Menu menu) {
    }

    @Override
    public void onContextMenuClosed(Menu menu, boolean inLoad) {
    }

    // error console

    @Override
    public void setShouldShowErrorConsole(Tab tab, boolean flag) {
        if (tab == null) return;
        ErrorConsoleView errorConsole = tab.getErrorConsole(true);
        if (flag) {
            // Setting the show state of the console will cause it's the layout
            // to be inflated.
            if (errorConsole.numberOfErrors() > 0) {
                errorConsole.showConsole(ErrorConsoleView.SHOW_MINIMIZED);
            } else {
                errorConsole.showConsole(ErrorConsoleView.SHOW_NONE);
            }
            if (errorConsole.getParent() != null) {
                mErrorConsoleContainer.removeView(errorConsole);
            }
            // Now we can add it to the main view.
            mErrorConsoleContainer.addView(errorConsole,
                    new LinearLayout.LayoutParams(
                            ViewGroup.LayoutParams.MATCH_PARENT,
                            ViewGroup.LayoutParams.WRAP_CONTENT));
        } else {
            mErrorConsoleContainer.removeView(errorConsole);
        }
    }

    // -------------------------------------------------------------------------
    // Helper function for WebChromeClient
    // -------------------------------------------------------------------------

    @Override
    public Bitmap getDefaultVideoPoster() {
        if (mDefaultVideoPoster == null) {
            mDefaultVideoPoster = BitmapFactory.decodeResource(
                    mActivity.getResources(), R.drawable.default_video_poster);
        }
        return mDefaultVideoPoster;
    }

    @Override
    public View getVideoLoadingProgressView() {
        if (mVideoProgressView == null) {
            LayoutInflater inflater = LayoutInflater.from(mActivity);
            mVideoProgressView = inflater.inflate(
                    R.layout.video_loading_progress, null);
        }
        return mVideoProgressView;
    }

    @Override
    public void showMaxTabsWarning() {
        Toast warning = Toast.makeText(mActivity,
                mActivity.getString(R.string.max_tabs_warning),
                Toast.LENGTH_SHORT);
        warning.show();
    }

    protected WebView getWebView() {
        if (mActiveTab != null) {
            return mActiveTab.getWebView();
        } else {
            return null;
        }
    }

    protected Menu getMenu() {
        MenuBuilder menu = new MenuBuilder(mActivity);
        mActivity.getMenuInflater().inflate(R.menu.browser, menu);
        return menu;
    }

    public void setFullscreen(boolean enabled) {
        if (DEBUG) {
            Log.d(TAG, "BaseUi.setFullscreen()--->" + enabled);
        }
        Window win = mActivity.getWindow();
        WindowManager.LayoutParams winParams = win.getAttributes();
        final int bits = WindowManager.LayoutParams.FLAG_FULLSCREEN;
        if (enabled) {
            winParams.flags |=  bits;
        } else {
            winParams.flags &= ~bits;
            if (mCustomView != null) {
                mCustomView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
            } else {
                mContentView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
            }
        }
        if (mNeedBottomBar) {
            mBottomBar.setFullScreen(enabled);
        }
        win.setAttributes(winParams);
    }

    public Drawable getFaviconDrawable(Bitmap icon) {
        Drawable[] array = new Drawable[3];
        array[0] = new PaintDrawable(Color.BLACK);
        PaintDrawable p = new PaintDrawable(Color.WHITE);
        array[1] = p;
        if (icon == null) {
            array[2] = mGenericFavicon;
        } else {
            array[2] = new BitmapDrawable(icon);
        }
        LayerDrawable d = new LayerDrawable(array);
        d.setLayerInset(1, 1, 1, 1, 1);
        d.setLayerInset(2, 2, 2, 2, 2);
        return d;
    }

    public boolean isLoading() {
        return mActiveTab != null ? mActiveTab.inPageLoad() : false;
    }

    /**
     * Suggest to the UI that the title bar can be hidden. The UI will then
     * decide whether or not to hide based off a number of factors, such
     * as if the user is editing the URL bar or if the page is loading
     */
    public void suggestHideTitleBar() {
        if (!isLoading() && !isEditingUrl() && !mTitleBar.wantsToBeVisible()
                && !mNavigationBar.isMenuShowing()) {
            hideTitleBarOnly();
        }
    }

    protected final void showTitleBarForDuration() {
        showTitleBarForDuration(HIDE_TITLEBAR_DELAY);
    }

    protected final void showTitleBarForDuration(long duration) {
        showTitleBar();
        Message msg = Message.obtain(mHandler, MSG_HIDE_TITLEBAR);
        mHandler.sendMessageDelayed(msg, duration);
    }

    protected final void showBottomBarForDuration(long duration) {
        if (getWebView() != null) {
            mHandler.removeMessages(MSG_HIDE_BOTTOMBAR);
            showBottomBarMust();
            Message msg = Message.obtain(mHandler, MSG_HIDE_BOTTOMBAR);
            mHandler.sendMessageDelayed(msg, duration);
        }
    }

    /// M: black screen when back to launcher:// @ {
    /**
     * Call this function to remove a tab delay.
     * @param tab tab to close.
     */
    public void closeTableDelay(Tab tab) {
        tab.clearTabData();
        Message msg = Message.obtain(mHandler, MSG_CLOSE_TABLEVIEW, tab);
        mHandler.sendMessageDelayed(msg, HIDE_TITLEBAR_DELAY);
    }
    /// @ }

    protected Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            if (msg.what == MSG_HIDE_TITLEBAR) {
                suggestHideTitleBar();
            }
            if (msg.what == MSG_HIDE_BOTTOMBAR) {
                if ((mUiController != null) &&
                    (mUiController.getCurrentTab() != null) &&
                    (!mUiController.getCurrentTab().inPageLoad())) {
                    hideBottomBar();
                }
            }
            /// M: black screen when back to launcher:// @ {
            // If it is last webview, make sure delay to remove it.
            if (msg.what == MSG_CLOSE_TABLEVIEW) {
                if (mUiController != null) {
                     mUiController.closeTab((Tab) msg.obj);
                }
            }
            /// @ }
            BaseUi.this.handleMessage(msg);
        }
    };

    protected void handleMessage(Message msg) {}

    @Override
    public void showWeb(boolean animate) {
        mUiController.hideCustomView();
    }

    static class FullscreenHolder extends FrameLayout {

        public FullscreenHolder(Context ctx) {
            super(ctx);
            setBackgroundColor(ctx.getResources().getColor(R.color.black));
        }

        @Override
        public boolean onTouchEvent(MotionEvent evt) {
            return true;
        }

    }

    /*packages*/ void setInputUrlFlag(boolean flag) {
        mInputUrlFlag = flag;
    }

    /*packages*/ boolean getInputUrlFlag() {
        return mInputUrlFlag;
    }
    public void addFixedTitleBar(View view) {
        if (DEBUG && view != null) {
            Log.d(TAG, "BaseUi.addFixedTitleBar()--->width = "
                + view.getWidth() + ", height = " + view.getHeight());
        }
        mFixedTitlebarContainer.addView(view);
    }

    public void setContentViewMarginTop(int margin) {
        LinearLayout.LayoutParams params =
                (LinearLayout.LayoutParams) mContentView.getLayoutParams();
        if (params.topMargin != margin) {
            params.topMargin = margin;
            mContentView.setLayoutParams(params);
        }
    }

    public void setContentViewMarginBottom(int margin) {
        LinearLayout.LayoutParams params =
                (LinearLayout.LayoutParams) mContentView.getLayoutParams();
        if (params.bottomMargin != margin) {
            params.bottomMargin = margin;
            mContentView.setLayoutParams(params);
        }
    }

    @Override
    public boolean blockFocusAnimations() {
        return mBlockFocusAnimations;
    }

    @Override
    public void onVoiceResult(String result) {
        mNavigationBar.onVoiceResult(result);
    }

    /// M: update bottom bar state
    @Override
    public void updateBottomBarState(boolean pageCanScroll, boolean back, boolean forward) {
    }
}
