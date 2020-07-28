/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

package com.android.browser;

import android.app.AppGlobals;
import android.app.Application;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
/// M: import DisplayList
import android.graphics.RenderNode;
import android.util.AttributeSet;
import android.util.Log;
import android.view.ViewRootImpl;
import android.view.View;
import android.webkit.WebChromeClient;
import android.webkit.WebStorage;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import java.lang.reflect.Method;
import java.util.Map;

/**
 * Manage WebView scroll events
 */
public class BrowserWebView extends WebView {

    public interface OnScrollChangedListener {
        void onScrollChanged(int l, int t, int oldl, int oldt);
    }

    private boolean mBackgroundRemoved = false;
    private TitleBar mTitleBar;
    private OnScrollChangedListener mOnScrollChangedListener;
    private WebChromeClient mWebChromeClient;
    private WebViewClient mWebViewClient;
    private String LOGTAG = "BrowserWebView";
    private static final String INCLUDE_SAVE_PAGE_WEBVIEW_PACKAGE = "com.mediatek.webview";
    private String mSiteNavHitURL;

    /**
     * @param context
     * @param attrs
     * @param defStyle
     * @param javascriptInterfaces
     */
    public BrowserWebView(Context context, AttributeSet attrs, int defStyle,
            Map<String, Object> javascriptInterfaces, boolean privateBrowsing) {
        super(context, attrs, defStyle, javascriptInterfaces, privateBrowsing);
    }

    /**
     * @param context
     * @param attrs
     * @param defStyle
     */
    public BrowserWebView(
            Context context, AttributeSet attrs, int defStyle, boolean privateBrowsing) {
        super(context, attrs, defStyle, privateBrowsing);
    }

    /**
     * @param context
     * @param attrs
     */
    public BrowserWebView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    /**
     * @param context
     */
    public BrowserWebView(Context context) {
        super(context);
    }

    @Override
    public void setWebChromeClient(WebChromeClient client) {
        mWebChromeClient = client;
        super.setWebChromeClient(client);
    }

    public WebChromeClient getWebChromeClient() {
      return mWebChromeClient;
    }

    @Override
    public void setWebViewClient(WebViewClient client) {
        mWebViewClient = client;
        super.setWebViewClient(client);
    }

    public WebViewClient getWebViewClient() {
      return mWebViewClient;
    }

    public void setTitleBar(TitleBar title) {
        mTitleBar = title;
    }

    public int getTitleHeight() {
        return (mTitleBar != null) ? mTitleBar.getEmbeddedHeight() : 0;
    }

    public boolean hasTitleBar() {
        return (mTitleBar != null);
    }

    @Override
    protected void onDraw(Canvas c) {
        super.onDraw(c);
        if (!mBackgroundRemoved && getRootView().getBackground() != null) {
            mBackgroundRemoved = true;
            post(new Runnable() {
                public void run() {
                    getRootView().setBackgroundDrawable(null);
                }
            });
        }
    }

    public void drawContent(Canvas c) {
        onDraw(c);
    }

    @Override
    protected void onScrollChanged(int l, int t, int oldl, int oldt) {
        super.onScrollChanged(l, t, oldl, oldt);
        if (mTitleBar != null) {
            mTitleBar.onScrollChanged();
        }
        if (mOnScrollChangedListener != null) {
            mOnScrollChangedListener.onScrollChanged(l, t, oldl, oldt);
        }
    }

    public void setOnScrollChangedListener(OnScrollChangedListener listener) {
        mOnScrollChangedListener = listener;
    }

    @Override
    public boolean showContextMenuForChild(View originalView) {
        return false;
    }

    @Override
    public void destroy() {
        BrowserSettings.getInstance().stopManagingSettings(getSettings());
        super.destroy();
        /// M: Reset the display list.
        RenderNode displayList = updateDisplayListIfDirty();
        if (displayList != null) {
            displayList.discardDisplayList();
        }
    }


    /// M: add interface for save page @{
    /**
     * The interface of save page.
     *
     * @return true if it can be saved, false otherwise
     * @hide
     * @internal
     */
    public boolean savePage() {
        Log.d(LOGTAG , "savePage");
        initChromiumClassIfNeccessary();
        if (mCls == null) {
            Log.e(LOGTAG, "Can't get WebViewChromium Save Page Interface");
            return false;
        }
        try {
            Method savePageMethod = mCls.getDeclaredMethod("savePage");
            if (savePageMethod == null) {
                Log.e(LOGTAG, "Get Null from webviewchromium savePage method");
                return false;
            }
            return (Boolean) savePageMethod.invoke(getWebViewProvider());
        } catch (ReflectiveOperationException ex) {
            Log.e(LOGTAG, "get Save Page Interface Exception->" + ex);
            return false;
        }
    }

    private Class<?> mCls;

    private void initChromiumClassIfNeccessary() {
        if (mCls != null) {
            return;
        }
        try {
            Application initialApplication = AppGlobals.getInitialApplication();
            if (initialApplication == null) {
                throw new ReflectiveOperationException("Applicatin not found");
            }
            Context webViewContext = initialApplication.createPackageContext(
                    INCLUDE_SAVE_PAGE_WEBVIEW_PACKAGE,
                    Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
            initialApplication.getAssets().addAssetPath(
                    webViewContext.getApplicationInfo().sourceDir);
            ClassLoader clazzLoader = webViewContext.getClassLoader();

            String className = "com.android.webview.chromium.WebViewChromium";
            mCls = Class.forName(className, true, clazzLoader);
        } catch (android.content.pm.PackageManager.NameNotFoundException ex) {
            Log.e(LOGTAG, "get Webview Class Exception->" + ex);
        } catch (ReflectiveOperationException ex) {
            Log.e(LOGTAG, "get Webview Class Exception->" + ex);
        }
    }
    /// @}


    /// M: add save page client @{
    /**
     * Set save page client.
     *
     * @param client the object of the client
     * @hide
     * @internal
     */
    public void setSavePageClient(SavePageClient client) {
        initChromiumClassIfNeccessary();
        if (mCls == null) {
            Log.e(LOGTAG, "Can't get WebViewChromium Save Page Interface");
            return;
        }
        try {
            Class[] p = new Class[1];
            p[0] = Object.class;
            Method setSavePageClientMethod = mCls.getDeclaredMethod("setSavePageClient", p);
            if (setSavePageClientMethod == null) {
                Log.e(LOGTAG, "Get Null from the webviewchromium setSavePageClient method");
                return;
            }
            setSavePageClientMethod.invoke(getWebViewProvider(), (Object)client);
        } catch (ReflectiveOperationException ex) {
            Log.e(LOGTAG, "get set Save Page Client Interface Exception->" + ex);
        }
    }
    /// @}

    @Override
    protected void onDetachedFromWindowInternal() {
        super.onDetachedFromWindowInternal();
        /// M: detach functor.
        ViewRootImpl viewRootImpl = getViewRootImpl();

        if (viewRootImpl != null) {
            viewRootImpl.detachFunctor(0);
        }
    }

    @Override
    public void setLayerType(int layerType, Paint paint) {
        if (getWebViewProvider() == null) return;
        super.setLayerType(layerType, paint);
    }


    /// M: for site navigation hit test result @{
    public synchronized void setSiteNavHitURL(String url) {
        mSiteNavHitURL = url;
    }
    public synchronized String getSiteNavHitURL() {
        return mSiteNavHitURL;
    }
    /// @}
}
