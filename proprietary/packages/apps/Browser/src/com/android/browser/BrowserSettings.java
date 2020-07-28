/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2011 The Android Open Source Project
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

import android.app.ActivityManager;
import android.content.ContentResolver;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.content.res.Configuration;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Build;
import android.os.Message;
import android.preference.PreferenceManager;
import com.android.browser.provider.Browser;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.Log;
import android.webkit.CookieManager;
import android.webkit.GeolocationPermissions;
import android.webkit.WebIconDatabase;
import android.webkit.WebSettings;
import android.webkit.WebSettings.LayoutAlgorithm;
import android.webkit.WebSettings.PluginState;
import android.webkit.WebSettings.TextSize;
import android.webkit.WebSettings.ZoomDensity;
import android.webkit.WebStorage;
import android.webkit.WebView;
import android.webkit.WebViewDatabase;

import com.android.browser.BrowserHistoryPage.ClearHistoryTask;
import com.android.browser.homepages.HomeProvider;
import com.android.browser.preferences.GeneralPreferencesFragment;
import com.android.browser.provider.BrowserProvider;
import com.android.browser.search.SearchEngine;
import com.android.browser.search.SearchEngines;
import com.android.browser.sitenavigation.SiteNavigation;
/// M: Add for Regional Phone support.
import com.mediatek.browser.ext.IBrowserRegionalPhoneExt;
import com.mediatek.browser.ext.IBrowserSettingExt;
import com.mediatek.search.SearchEngineManager;
import com.mediatek.custom.CustomProperties;

import java.lang.ref.WeakReference;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.WeakHashMap;

/**
 * Class for managing settings
 */
public class BrowserSettings implements OnSharedPreferenceChangeListener,
        PreferenceKeys {

    // TODO: Do something with this UserAgent stuff
    private static final String DESKTOP_USERAGENT = "Mozilla/5.0 (X11; " +
        "Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) " +
        "Chrome/66.0.3359.158 Safari/537.36";

    private static final String IPHONE_USERAGENT = "Mozilla/5.0 (iPhone; U; " +
        "CPU iPhone OS 4_0 like Mac OS X; en-us) AppleWebKit/532.9 " +
        "(KHTML, like Gecko) Version/4.0.5 Mobile/8A293 Safari/6531.22.7";

    private static final String IPAD_USERAGENT = "Mozilla/5.0 (iPad; U; " +
        "CPU OS 3_2 like Mac OS X; en-us) AppleWebKit/531.21.10 " +
        "(KHTML, like Gecko) Version/4.0.4 Mobile/7B367 Safari/531.21.10";

    private static final String FROYO_USERAGENT = "Mozilla/5.0 (Linux; U; " +
        "Android 2.2; en-us; Nexus One Build/FRF91) AppleWebKit/533.1 " +
        "(KHTML, like Gecko) Version/4.0 Mobile Safari/533.1";

    private static final String HONEYCOMB_USERAGENT = "Mozilla/5.0 (Linux; U; " +
        "Android 3.1; en-us; Xoom Build/HMJ25) AppleWebKit/534.13 " +
        "(KHTML, like Gecko) Version/4.0 Safari/534.13";

    private static final String USER_AGENTS[] = { null,
            DESKTOP_USERAGENT,
            IPHONE_USERAGENT,
            IPAD_USERAGENT,
            FROYO_USERAGENT,
            HONEYCOMB_USERAGENT,
    };

    // The minimum min font size
    // Aka, the lower bounds for the min font size range
    // which is 1:5..24
    private static final int MIN_FONT_SIZE_OFFSET = 5;
    // The initial value in the text zoom range
    // This is what represents 100% in the SeekBarPreference range
    private static final int TEXT_ZOOM_START_VAL = 10;
    // The size of a single step in the text zoom range, in percent
    private static final int TEXT_ZOOM_STEP = 5;
    // The initial value in the double tap zoom range
    // This is what represents 100% in the SeekBarPreference range
    private static final int DOUBLE_TAP_ZOOM_START_VAL = 5;
    // The size of a single step in the double tap zoom range, in percent
    private static final int DOUBLE_TAP_ZOOM_STEP = 5;

    public static final String XLOGTAG = "Browser/Settings";
    /// M: add for debug @ {
    private static final boolean DEBUG = com.android.browser.Browser.DEBUG;
    private static final String TAG = "browser";
    /// @ }

    private static BrowserSettings sInstance;

    private Context mContext;
    private SharedPreferences mPrefs;
    private LinkedList<WeakReference<WebSettings>> mManagedSettings;
    private Controller mController;
    private WebStorageSizeManager mWebStorageSizeManager;
    private WeakHashMap<WebSettings, String> mCustomUserAgents;
    private static boolean sInitialized = false;
    private boolean mNeedsSharedSync = true;
    private float mFontSizeMult = 1.0f;

    // Current state of network-dependent settings
    private boolean mLinkPrefetchAllowed = true;

    // Cached values
    private int mPageCacheCapacity = 1;
    private String mAppCachePath;

    // Cached settings
    private SearchEngine mSearchEngine;

    private static String sFactoryResetUrl;

    public static void initialize(final Context context) {
        sInstance = new BrowserSettings(context);
    }

    public static BrowserSettings getInstance() {
        return sInstance;
    }

    private BrowserSettings(Context context) {
        mContext = context.getApplicationContext();
        mPrefs = PreferenceManager.getDefaultSharedPreferences(mContext);
        mManagedSettings = new LinkedList<WeakReference<WebSettings>>();
        mCustomUserAgents = new WeakHashMap<WebSettings, String>();
        BackgroundHandler.execute(mSetup);
    }

    public void setController(Controller controller) {
        mController = controller;
        if (sInitialized) {
            syncSharedSettings();
        }
        sBrowserSettingExt = Extensions.getSettingPlugin(mContext);
        sBrowserSettingExt.setOnlyLandscape(mPrefs, mController.getActivity());
    }

    public void startManagingSettings(WebSettings settings) {

        if (mNeedsSharedSync) {
            syncSharedSettings();
        }

        synchronized (mManagedSettings) {
            syncStaticSettings(settings);
            syncSetting(settings);
            mManagedSettings.add(new WeakReference<WebSettings>(settings));
        }
    }

    /// M: Browser setting plugin.
    private static IBrowserSettingExt sBrowserSettingExt = null;

    public void stopManagingSettings(WebSettings settings) {
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.stopManagingSettings()--->");
        }
        Iterator<WeakReference<WebSettings>> iter = mManagedSettings.iterator();
        while (iter.hasNext()) {
            WeakReference<WebSettings> ref = iter.next();
            if (ref.get() == settings) {
                iter.remove();
                return;
            }
        }
    }

    private Runnable mSetup = new Runnable() {

        @Override
        public void run() {
            DisplayMetrics metrics = mContext.getResources().getDisplayMetrics();
            mFontSizeMult = metrics.scaledDensity / metrics.density;
            // the cost of one cached page is ~3M (measured using nytimes.com). For
            // low end devices, we only cache one page. For high end devices, we try
            // to cache more pages, currently choose 5.
            if (ActivityManager.staticGetMemoryClass() > 16) {
                mPageCacheCapacity = 5;
            }
            mWebStorageSizeManager = new WebStorageSizeManager(mContext,
                    new WebStorageSizeManager.StatFsDiskInfo(getAppCachePath()),
                    new WebStorageSizeManager.WebKitAppCacheInfo(getAppCachePath()));
            // Workaround b/5254577
            mPrefs.registerOnSharedPreferenceChangeListener(BrowserSettings.this);
            if (Build.VERSION.CODENAME.equals("REL")) {
                // This is a release build, always startup with debug disabled
                setDebugEnabled(false);
            }
            if (mPrefs.contains(PREF_TEXT_SIZE)) {
                /*
                 * Update from TextSize enum to zoom percent
                 * SMALLEST is 50%
                 * SMALLER is 75%
                 * NORMAL is 100%
                 * LARGER is 150%
                 * LARGEST is 200%
                 */
                switch (getTextSize()) {
                case SMALLEST:
                    setTextZoom(50);
                    break;
                case SMALLER:
                    setTextZoom(75);
                    break;
                case LARGER:
                    setTextZoom(150);
                    break;
                case LARGEST:
                    setTextZoom(200);
                    break;
                }
                mPrefs.edit().remove(PREF_TEXT_SIZE).apply();
            }

            /// M: Get the customer homepage. @{
            sBrowserSettingExt = Extensions.getSettingPlugin(mContext);
            sFactoryResetUrl = sBrowserSettingExt.getCustomerHomepage();
            /// @}

            if (sFactoryResetUrl == null) {
                /// M: add for site navigation @{
                if (BrowserFeatureOption.BROWSER_SITE_NAVIGATION_SUPPORT) {
                    sFactoryResetUrl = mContext.getResources()
                                               .getString(R.string.homepage_base_site_navigation);
                } else {
                    sFactoryResetUrl = mContext.getResources().getString(R.string.homepage_base);
                    if (sFactoryResetUrl.indexOf("{CID}") != -1) {
                        sFactoryResetUrl = sFactoryResetUrl.replace("{CID}",
                                BrowserProvider.getClientId(mContext.getContentResolver()));
                    }
                }
                // @}
            }

            if (DEBUG) {
                Log.d(TAG, "BrowserSettings.mSetup()--->run()--->sFactoryResetUrl : "
                            + sFactoryResetUrl);
            }
            synchronized (BrowserSettings.class) {
                sInitialized = true;
                BrowserSettings.class.notifyAll();
            }
        }
    };

    public static String getFactoryResetUrlFromRes(Context context) {
        /// M: Get the customer homepage. @{
        sBrowserSettingExt = Extensions.getSettingPlugin(context);
        sFactoryResetUrl = sBrowserSettingExt.getCustomerHomepage();

        if (sFactoryResetUrl == null) {
            sFactoryResetUrl = context.getResources().getString(R.string.homepage_base);
        }

        if (sFactoryResetUrl.indexOf("{CID}") != -1) {
            sFactoryResetUrl = sFactoryResetUrl.replace("{CID}",
                BrowserProvider.getClientId(context.getContentResolver()));
        }
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.getFactoryResetUrlFromRes()--->sFactoryResetUrl : "
                        + sFactoryResetUrl);
        }
        return sFactoryResetUrl;
        /// @}
    }

    private static void requireInitialization() {
        synchronized (BrowserSettings.class) {
            while (!sInitialized) {
                try {
                    BrowserSettings.class.wait();
                } catch (InterruptedException e) {
                }
            }
        }
    }

    /**
     * Syncs all the settings that have a Preference UI
     */
    private void syncSetting(WebSettings settings) {
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.syncSetting()--->");
        }
        settings.setGeolocationEnabled(enableGeolocation());
        settings.setJavaScriptEnabled(enableJavascript());
        settings.setLightTouchEnabled(enableLightTouch());
        settings.setNavDump(enableNavDump());
        /// M: From Android O, doesn't support text encoding, Blink do auto-detect.
        //settings.setDefaultTextEncodingName(getDefaultTextEncoding());
        settings.setMinimumFontSize(getMinimumFontSize());
        settings.setMinimumLogicalFontSize(getMinimumFontSize());
        settings.setPluginState(getPluginState());
        settings.setTextZoom(getTextZoom());
        setDoubleTapZoom(settings, getDoubleTapZoom());
        settings.setLayoutAlgorithm(getLayoutAlgorithm());
        settings.setJavaScriptCanOpenWindowsAutomatically(!blockPopupWindows());
        settings.setLoadsImagesAutomatically(loadImages());
        settings.setLoadWithOverviewMode(loadPageInOverviewMode());
        settings.setSavePassword(rememberPasswords());
        settings.setSaveFormData(saveFormdata());
        settings.setUseWideViewPort(isWideViewport());

        /// M: Set the font family and the Operator UA. @{
        sBrowserSettingExt = Extensions.getSettingPlugin(mContext);
        sBrowserSettingExt.setStandardFontFamily(settings, mPrefs);

        String ua = mCustomUserAgents.get(settings);
        if (ua != null) {
            settings.setUserAgentString(ua);
        } else {
            String operatorUA = CustomProperties.getString(CustomProperties.MODULE_BROWSER,
                                   CustomProperties.USER_AGENT);
            if (operatorUA == null || operatorUA.length() == 0) {
                String pluginUA = Extensions.getSettingPlugin(mContext)
                        .getOperatorUA(settings.getUserAgentString());
                if (pluginUA != null && pluginUA.length() > 0) {
                    operatorUA = pluginUA;
                }
            }
            if (getUserAgent() == 0 && operatorUA != null) {
                settings.setUserAgentString(operatorUA);
            } else {
                settings.setUserAgentString(USER_AGENTS[getUserAgent()]);
            }
        }
        /// @}
    }

    /**
     * Syncs all the settings that have no UI
     * These cannot change, so we only need to set them once per WebSettings
     */
    private void syncStaticSettings(WebSettings settings) {
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.syncStaticSettings()--->");
        }
        settings.setDefaultFontSize(16);
        settings.setDefaultFixedFontSize(13);

        // WebView inside Browser doesn't want initial focus to be set.
        settings.setNeedInitialFocus(false);
        // Browser supports multiple windows
        settings.setSupportMultipleWindows(true);
        // enable smooth transition for better performance during panning or
        // zooming
        settings.setEnableSmoothTransition(true);
        // disable content url access
        settings.setAllowContentAccess(true);

        // HTML5 API flags
        settings.setAppCacheEnabled(true);
        settings.setDatabaseEnabled(true);
        settings.setDomStorageEnabled(true);

        // HTML5 configuration parametersettings.
        settings.setAppCacheMaxSize(getWebStorageSizeManager().getAppCacheMaxSize());
        settings.setAppCachePath(getAppCachePath());
        settings.setDatabasePath(mContext.getDir("databases", 0).getPath());
        settings.setGeolocationDatabasePath(mContext.getDir("geolocation", 0).getPath());
        // origin policy for file access
        settings.setAllowUniversalAccessFromFileURLs(false);
        settings.setAllowFileAccessFromFileURLs(false);

        /// M: Set the mixed content mode
        settings.setMixedContentMode(WebSettings.MIXED_CONTENT_COMPATIBILITY_MODE);
    }

    private void syncSharedSettings() {
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.syncSharedSettings()--->");
        }
        mNeedsSharedSync = false;
        CookieManager.getInstance().setAcceptCookie(acceptCookies());
        if (mController != null) {
            for (Tab tab : mController.getTabs()) {
                tab.setAcceptThirdPartyCookies(acceptCookies());
            }
            mController.setShouldShowErrorConsole(enableJavascriptConsole());
        }
    }

    private void syncManagedSettings() {
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.syncManagedSettings()--->");
        }
        syncSharedSettings();
        synchronized (mManagedSettings) {
            Iterator<WeakReference<WebSettings>> iter = mManagedSettings.iterator();
            while (iter.hasNext()) {
                WeakReference<WebSettings> ref = iter.next();
                WebSettings settings = ref.get();
                if (settings == null) {
                    iter.remove();
                    continue;
                }
                syncSetting(settings);
            }
        }
    }

    @Override
    public void onSharedPreferenceChanged(
            SharedPreferences sharedPreferences, String key) {
        syncManagedSettings();
        if (PREF_SEARCH_ENGINE.equals(key)) {
            updateSearchEngine(false);
        }
        /// M: If mController == null(sometimes the request coming from Settings),
        /// BrowserActivity is not started, do not set UI full screen,
        /// just save setttings. @{
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.onSharedPreferenceChanged()--->"
                        + key + " mControll is null:" + (mController == null));
        }
        if (mController == null) {
            return;
        /// @}
        } else if (PREF_FULLSCREEN.equals(key)) {
            if (mController != null && mController.getUi() != null) {
                mController.getUi().setFullscreen(useFullscreen());
            }
        } else if (PREF_ENABLE_QUICK_CONTROLS.equals(key)) {
            if (mController != null && mController.getUi() != null) {
                mController.getUi().setUseQuickControls(sharedPreferences.getBoolean(key, false));
            }
        } else if (PREF_LINK_PREFETCH.equals(key)) {
            updateConnectionType();
        /// M: add for landscape only browser mode. @{
        } else if (PREF_LANDSCAPEONLY.equals(key)) {
            sBrowserSettingExt = Extensions.getSettingPlugin(mContext);
            sBrowserSettingExt.setOnlyLandscape(sharedPreferences, mController.getActivity());
        /// @}
        }
    }

    public static String getFactoryResetHomeUrl(Context context) {
        requireInitialization();
        return sFactoryResetUrl;
    }

    public LayoutAlgorithm getLayoutAlgorithm() {
        LayoutAlgorithm layoutAlgorithm = LayoutAlgorithm.NORMAL;
        final LayoutAlgorithm autosize = Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT ?
                LayoutAlgorithm.TEXT_AUTOSIZING : LayoutAlgorithm.NARROW_COLUMNS;

        if (autofitPages()) {
            layoutAlgorithm = autosize;
        }
        if (isDebugEnabled()) {
            if (isNormalLayout()) {
                layoutAlgorithm = LayoutAlgorithm.NORMAL;
            } else {
                layoutAlgorithm = autosize;
            }
        }
        return layoutAlgorithm;
    }

    public int getPageCacheCapacity() {
        requireInitialization();
        return mPageCacheCapacity;
    }

    public WebStorageSizeManager getWebStorageSizeManager() {
        requireInitialization();
        return mWebStorageSizeManager;
    }

    private String getAppCachePath() {
        if (mAppCachePath == null) {
            mAppCachePath = mContext.getDir("appcache", 0).getPath();
        }
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.getAppCachePath()--->mAppCachePath:" + mAppCachePath);
        }
        return mAppCachePath;
    }

    private void updateSearchEngine(boolean force) {
        String searchEngineName = getSearchEngineName();
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.updateSearchEngine()--->searchEngineName:"
                        + searchEngineName);
        }
        if (force || mSearchEngine == null || searchEngineName == null ||
                !mSearchEngine.getName().equals(searchEngineName)) {
            mSearchEngine = SearchEngines.get(mContext, searchEngineName);
         }
    }

    public SearchEngine getSearchEngine() {
        if (mSearchEngine == null) {
            updateSearchEngine(false);
        }
        return mSearchEngine;
    }

    public boolean isDebugEnabled() {
        requireInitialization();
        return mPrefs.getBoolean(PREF_DEBUG_MENU, false);
    }

    public void setDebugEnabled(boolean value) {
        Editor edit = mPrefs.edit();
        edit.putBoolean(PREF_DEBUG_MENU, value);
        if (!value) {
            // Reset to "safe" value
            edit.putBoolean(PREF_ENABLE_HARDWARE_ACCEL_SKIA, false);
        }
        edit.apply();
    }

    public void clearCache() {
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.clearCache()--->");
        }
        WebIconDatabase.getInstance().removeAllIcons();
        if (mController != null) {
            WebView current = mController.getCurrentWebView();
            if (current != null) {
                current.clearCache(true);
            }
        }
    }

    public void clearCookies() {
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.clearCookies()--->");
        }
        CookieManager.getInstance().removeAllCookie();
    }

    public void clearHistory() {
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.clearHistory()--->");
        }
        final ContentResolver resolver = mContext.getContentResolver();
        final ClearHistoryTask clear = new BrowserHistoryPage.ClearHistoryTask(resolver);
        if (!clear.isAlive()) {
            clear.start();
        }
    }

    public void clearFormData() {
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.clearFormData()--->");
        }
        WebViewDatabase.getInstance(mContext).clearFormData();
        if (mController!= null) {
            WebView currentTopView = mController.getCurrentTopWebView();
            if (currentTopView != null) {
                currentTopView.clearFormData();
            }
        }
    }

    public void clearPasswords() {
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.clearPasswords()--->");
        }
        WebViewDatabase db = WebViewDatabase.getInstance(mContext);
        db.clearUsernamePassword();
        db.clearHttpAuthUsernamePassword();
    }

    public void clearDatabases() {
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.clearDatabases()--->");
        }
        WebStorage.getInstance().deleteAllData();
    }

    public void clearLocationAccess() {
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.clearLocationAccess()--->");
        }
        GeolocationPermissions.getInstance().clearAll();
    }

    public void resetDefaultPreferences() {
        // Preserve autologin setting
        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.resetDefaultPreferences()--->");
        }
        long gal = mPrefs.getLong(GoogleAccountLogin.PREF_AUTOLOGIN_TIME, -1);
        mPrefs.edit()
                .clear()
                .putLong(GoogleAccountLogin.PREF_AUTOLOGIN_TIME, gal)
                .apply();
        resetCachedValues();
        syncManagedSettings();
    }

    private void resetCachedValues() {
        updateSearchEngine(false);
    }

    public void toggleDebugSettings() {
        setDebugEnabled(!isDebugEnabled());
    }

    public boolean hasDesktopUseragent(WebView view) {
        return view != null && mCustomUserAgents.get(view.getSettings()) != null;
    }

    /// M: add different interface to support change smartbook UA
    public boolean isDesktopUserAgent(WebView view) {
        WebSettings settings = view.getSettings();
        String ua = settings.getUserAgentString();
        if (ua != null) {
            return ua.equals(DESKTOP_USERAGENT);
        }
        return false;
    }

    public void changeUserAgent(WebView view, boolean isDesktop) {
        if (view == null) {
            return;
        }
        WebSettings settings = view.getSettings();
        if (!isDesktop) {
            Log.i(XLOGTAG, "UA restore");
            if (mCustomUserAgents.get(settings) != null) {
                // this flow only for user has used desktop ua before using/plug/unplug SmartBook,
                // so use toglled ua.
                return;
            }
            String operatorUA = CustomProperties.getString(CustomProperties.MODULE_BROWSER,
                    CustomProperties.USER_AGENT);
            if (operatorUA == null || operatorUA.length() == 0) {
                String pluginUA = Extensions.getSettingPlugin(mContext)
                        .getOperatorUA(settings.getUserAgentString());
                if (pluginUA != null && pluginUA.length() > 0) {
                    operatorUA = pluginUA;
                }
            }
            if (getUserAgent() == 0 && operatorUA != null) {
                settings.setUserAgentString(operatorUA);
            } else {
                settings.setUserAgentString(USER_AGENTS[getUserAgent()]);
            }
        } else {
            Log.i(XLOGTAG, "UA change to desktop");
            settings.setUserAgentString(DESKTOP_USERAGENT);
        }
    }

    public void toggleDesktopUseragent(WebView view) {
        if (view == null) {
            return;
        }
        WebSettings settings = view.getSettings();
        if (mCustomUserAgents.get(settings) != null) {
            mCustomUserAgents.remove(settings);
            settings.setUserAgentString(USER_AGENTS[getUserAgent()]);
        } else {
            mCustomUserAgents.put(settings, DESKTOP_USERAGENT);
            settings.setUserAgentString(DESKTOP_USERAGENT);
        }
    }

    public static int getAdjustedMinimumFontSize(int rawValue) {
        rawValue++; // Preference starts at 0, min font at 1
        if (rawValue > 1) {
            rawValue += (MIN_FONT_SIZE_OFFSET - 2);
        }
        return rawValue;
    }

    public int getAdjustedTextZoom(int rawValue) {
        rawValue = (rawValue - TEXT_ZOOM_START_VAL) * TEXT_ZOOM_STEP;
        return (int) ((rawValue + 100) * mFontSizeMult);
    }

    static int getRawTextZoom(int percent) {
        return (percent - 100) / TEXT_ZOOM_STEP + TEXT_ZOOM_START_VAL;
    }

    public int getAdjustedDoubleTapZoom(int rawValue) {
        rawValue = (rawValue - DOUBLE_TAP_ZOOM_START_VAL) * DOUBLE_TAP_ZOOM_STEP;
        return (int) ((rawValue + 100) * mFontSizeMult);
    }

    static int getRawDoubleTapZoom(int percent) {
        return (percent - 100) / DOUBLE_TAP_ZOOM_STEP + DOUBLE_TAP_ZOOM_START_VAL;
    }

    public SharedPreferences getPreferences() {
        return mPrefs;
    }

    // update connectivity-dependent options
    public void updateConnectionType() {
        ConnectivityManager cm = (ConnectivityManager)
            mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        String linkPrefetchPreference = getLinkPrefetchEnabled();
        boolean linkPrefetchAllowed = linkPrefetchPreference.
            equals(getLinkPrefetchAlwaysPreferenceString(mContext));
        NetworkInfo ni = cm.getActiveNetworkInfo();
        if (ni != null) {
            switch (ni.getType()) {
                case ConnectivityManager.TYPE_WIFI:
                case ConnectivityManager.TYPE_ETHERNET:
                case ConnectivityManager.TYPE_BLUETOOTH:
                    linkPrefetchAllowed |= linkPrefetchPreference.
                        equals(getLinkPrefetchOnWifiOnlyPreferenceString(mContext));
                    break;
                case ConnectivityManager.TYPE_MOBILE:
                case ConnectivityManager.TYPE_MOBILE_DUN:
                case ConnectivityManager.TYPE_MOBILE_MMS:
                case ConnectivityManager.TYPE_MOBILE_SUPL:
                case ConnectivityManager.TYPE_WIMAX:
                default:
                    break;
            }
        }
        if (mLinkPrefetchAllowed != linkPrefetchAllowed) {
            mLinkPrefetchAllowed = linkPrefetchAllowed;
            syncManagedSettings();
        }
    }

    // -----------------------------
    // getter/setters for accessibility_preferences.xml
    // -----------------------------

    @Deprecated
    private TextSize getTextSize() {
        String textSize = mPrefs.getString(PREF_TEXT_SIZE, "NORMAL");
        return TextSize.valueOf(textSize);
    }

    public int getMinimumFontSize() {
        int minFont = mPrefs.getInt(PREF_MIN_FONT_SIZE, 0);
        return getAdjustedMinimumFontSize(minFont);
    }

    public boolean forceEnableUserScalable() {
        return mPrefs.getBoolean(PREF_FORCE_USERSCALABLE, false);
    }

    public int getTextZoom() {
        requireInitialization();
        int textZoom = mPrefs.getInt(PREF_TEXT_ZOOM, 10);
        return getAdjustedTextZoom(textZoom);
    }

    public void setTextZoom(int percent) {
        mPrefs.edit().putInt(PREF_TEXT_ZOOM, getRawTextZoom(percent)).apply();
    }

    public int getDoubleTapZoom() {
        requireInitialization();
        int doubleTapZoom = mPrefs.getInt(PREF_DOUBLE_TAP_ZOOM, 5);
        return getAdjustedDoubleTapZoom(doubleTapZoom);
    }

    public void setDoubleTapZoom(int percent) {
        mPrefs.edit().putInt(PREF_DOUBLE_TAP_ZOOM, getRawDoubleTapZoom(percent)).apply();
    }

    // -----------------------------
    // getter/setters for advanced_preferences.xml
    // -----------------------------

    public String getSearchEngineName() {
        /// M: Operator Feature get the customer's search engine. @{
        SearchEngineManager searchEngineManager = (SearchEngineManager) mContext
                .getSystemService(SearchEngineManager.SEARCH_ENGINE_SERVICE);
        List<com.mediatek.common.search.SearchEngine> searchEngines =
                                                            searchEngineManager.getAvailables();
        if (searchEngines == null || searchEngines.size() <= 0) {
            return null;
        }
        String defaultSearchEngine = SearchEngine.GOOGLE;
        com.mediatek.common.search.SearchEngine searchEngine = searchEngineManager.getDefault();
        if (searchEngine != null) {
            defaultSearchEngine = searchEngine.getName();
        }

        sBrowserSettingExt = Extensions.getSettingPlugin(mContext);
        String searchEngineName = sBrowserSettingExt.getSearchEngine(mPrefs, mContext);
        String searchEngineFavicon = "";

        searchEngine = searchEngineManager.getByName(searchEngineName);
        if (searchEngine != null) {
            searchEngineFavicon = searchEngine.getFaviconUri();
        } else {
            searchEngineFavicon = mPrefs.getString(BrowserSettings.PREF_SEARCH_ENGINE_FAVICON, "");
        }

        int selectedItem = -1;
        boolean need_refresh = false;
        int len = searchEngines.size();
        String[] entryValues = new String[len];
        String[] entryFavicon = new String[len];

        searchEngine = searchEngineManager.getBestMatch("", searchEngineFavicon);
        if (searchEngine != null) {
            if (!searchEngineName.equals(searchEngine.getName())) {
                searchEngineName = searchEngine.getName();
                need_refresh = true;
            }
        }

        for (int i = 0; i < len; i++) {
            entryValues[i] = searchEngines.get(i).getName();
            entryFavicon[i] = searchEngines.get(i).getFaviconUri();
            if (entryValues[i].equals(searchEngineName)) {
                selectedItem = i;
            }
        }

        if (selectedItem == -1) {
            selectedItem = 0;
            for (int i = 0; i < len; i++) {
                if (entryValues[i].equals(defaultSearchEngine)) {
                    selectedItem = i;
                }
            }
            need_refresh = true;
        }

        if (DEBUG) {
            Log.d(TAG, "BrowserSettings.getSearchEngineName-->selectedItem = " + selectedItem
                + "entryValues[" + selectedItem + "]=" + entryValues);
        }
        if (need_refresh && selectedItem != -1) {
            SharedPreferences.Editor editor = mPrefs.edit();
            editor.putString(BrowserSettings.PREF_SEARCH_ENGINE, entryValues[selectedItem]);
            editor.putString(BrowserSettings.PREF_SEARCH_ENGINE_FAVICON,
                             entryFavicon[selectedItem]);
            editor.commit();
        }

        return entryValues[selectedItem];
        /// @}
    }

    public boolean allowAppTabs() {
        return mPrefs.getBoolean(PREF_ALLOW_APP_TABS, false);
    }

    public boolean openInBackground() {
        return mPrefs.getBoolean(PREF_OPEN_IN_BACKGROUND, false);
    }

    public boolean enableJavascript() {
        return mPrefs.getBoolean(PREF_ENABLE_JAVASCRIPT, true);
    }

    // TODO: Cache
    public PluginState getPluginState() {
        String state = mPrefs.getString(PREF_PLUGIN_STATE, "ON");
        return PluginState.valueOf(state);
    }

    public boolean loadPageInOverviewMode() {
        boolean loadMode = mPrefs.getBoolean(PREF_LOAD_PAGE, true);
        Log.i(XLOGTAG, "loadMode: " + loadMode);
        return loadMode;
    }

    public boolean autofitPages() {
        return mPrefs.getBoolean(PREF_AUTOFIT_PAGES, true);
    }

    public boolean blockPopupWindows() {
        return mPrefs.getBoolean(PREF_BLOCK_POPUP_WINDOWS, true);
    }

    public boolean loadImages() {
        return mPrefs.getBoolean(PREF_LOAD_IMAGES, true);
    }
    /// M: From Android O, doesn't support text encoding, Blink do auto-detect.
    /*
    public String getDefaultTextEncoding() {
        String encoding = mPrefs.getString(PREF_DEFAULT_TEXT_ENCODING, null);
        if (TextUtils.isEmpty(encoding)) {
            encoding = mContext
                    .getString(R.string.pref_default_text_encoding_default_choice);
        }
        return encoding;
    }
    */
    /**
     *  M: Add download path get function integrated with File Manager
     *  Don't need set function because set is done in AdvancedPreferencesFragment onActivityResult
     * @return
     */
    public String getDownloadPath() {
        return mPrefs.getString(PREF_DOWNLOAD_DIRECTORY_SETTING,
                                getDefaultDownloadPathWithMultiSDcard());
    }

    /**
     *  M: In OP01 project, device has multi SDcard storage, we need to
     *  set sdcard2 as default download path.
     * @return
     */
    public String getDefaultDownloadPathWithMultiSDcard() {
        sBrowserSettingExt = Extensions.getSettingPlugin(mContext);
        if (DEBUG) {
            Log.d(TAG, "Default Download Path:" + sBrowserSettingExt.getDefaultDownloadFolder());
        }
        return sBrowserSettingExt.getDefaultDownloadFolder();
    }

    // -----------------------------
    // getter/setters for general_preferences.xml
    // -----------------------------

    public String getHomePage() {
        return mPrefs.getString(PREF_HOMEPAGE, getFactoryResetHomeUrl(mContext));
    }

    public void setHomePage(String value) {
        mPrefs.edit().putString(PREF_HOMEPAGE, value).apply();
        if (DEBUG) {
            Log.i(XLOGTAG, "BrowserSettings: setHomePage : " + value);
        }
    }

    public void setHomePagePicker(String value) {
        mPrefs.edit().putString(GeneralPreferencesFragment.PREF_HOMEPAGE_PICKER, value).apply();
        Log.i(XLOGTAG, "BrowserSettings: setHomePagePicker : " + value);
    }


    public boolean isAutofillEnabled() {
        return mPrefs.getBoolean(PREF_AUTOFILL_ENABLED, true);
    }

    public void setAutofillEnabled(boolean value) {
        mPrefs.edit().putBoolean(PREF_AUTOFILL_ENABLED, value).apply();
    }

    // -----------------------------
    // getter/setters for debug_preferences.xml
    // -----------------------------

    public boolean isHardwareAccelerated() {
        if (!isDebugEnabled()) {
            return true;
        }
        return mPrefs.getBoolean(PREF_ENABLE_HARDWARE_ACCEL, true);
    }

    public boolean isSkiaHardwareAccelerated() {
        if (!isDebugEnabled()) {
            return false;
        }
        return mPrefs.getBoolean(PREF_ENABLE_HARDWARE_ACCEL_SKIA, false);
    }

    public int getUserAgent() {
        if (!isDebugEnabled()) {
            return 0;
        }
        return Integer.parseInt(mPrefs.getString(PREF_USER_AGENT, "0"));
    }

    // -----------------------------
    // getter/setters for hidden_debug_preferences.xml
    // -----------------------------

    public boolean enableVisualIndicator() {
        if (!isDebugEnabled()) {
            return false;
        }
        return mPrefs.getBoolean(PREF_ENABLE_VISUAL_INDICATOR, false);
    }

    public boolean enableCpuUploadPath() {
        if (!isDebugEnabled()) {
            return false;
        }
        return mPrefs.getBoolean(PREF_ENABLE_CPU_UPLOAD_PATH, false);
    }

    public boolean enableJavascriptConsole() {
        if (!isDebugEnabled()) {
            return false;
        }
        return mPrefs.getBoolean(PREF_JAVASCRIPT_CONSOLE, true);
    }

    public boolean isWideViewport() {
        if (!isDebugEnabled()) {
            return true;
        }
        return mPrefs.getBoolean(PREF_WIDE_VIEWPORT, true);
    }

    public boolean isNormalLayout() {
        if (!isDebugEnabled()) {
            return false;
        }
        return mPrefs.getBoolean(PREF_NORMAL_LAYOUT, false);
    }

    public boolean isTracing() {
        if (!isDebugEnabled()) {
            return false;
        }
        return mPrefs.getBoolean(PREF_ENABLE_TRACING, false);
    }

    public boolean enableLightTouch() {
        if (!isDebugEnabled()) {
            return false;
        }
        return mPrefs.getBoolean(PREF_ENABLE_LIGHT_TOUCH, false);
    }

    public boolean enableNavDump() {
        if (!isDebugEnabled()) {
            return false;
        }
        return mPrefs.getBoolean(PREF_ENABLE_NAV_DUMP, false);
    }

    public String getJsEngineFlags() {
        if (!isDebugEnabled()) {
            return "";
        }
        return mPrefs.getString(PREF_JS_ENGINE_FLAGS, "");
    }

    // -----------------------------
    // getter/setters for lab_preferences.xml
    // -----------------------------

    public boolean useQuickControls() {
        return mPrefs.getBoolean(PREF_ENABLE_QUICK_CONTROLS, false);
    }

    public boolean useMostVisitedHomepage() {
        return HomeProvider.MOST_VISITED.equals(getHomePage());
    }

    /**
     * M: If homepage settings is site navigation, return site navigation.
     */
    public boolean useSiteNavigationHomepage() {
        if (BrowserFeatureOption.BROWSER_SITE_NAVIGATION_SUPPORT) {
            return SiteNavigation.SITE_NAVIGATION.equals(getHomePage());
        } else {
            return false;
        }
    }

    public boolean useFullscreen() {
        return mPrefs.getBoolean(PREF_FULLSCREEN, false);
    }

    // -----------------------------
    // getter/setters for privacy_security_preferences.xml
    // -----------------------------

    public boolean showSecurityWarnings() {
        return mPrefs.getBoolean(PREF_SHOW_SECURITY_WARNINGS, true);
    }

    public boolean acceptCookies() {
        return mPrefs.getBoolean(PREF_ACCEPT_COOKIES, true);
    }

    public boolean saveFormdata() {
        return mPrefs.getBoolean(PREF_SAVE_FORMDATA, true);
    }

    public boolean enableGeolocation() {
        return mPrefs.getBoolean(PREF_ENABLE_GEOLOCATION, true);
    }

    public boolean rememberPasswords() {
        return mPrefs.getBoolean(PREF_REMEMBER_PASSWORDS, true);
    }

    // -----------------------------
    // getter/setters for bandwidth_preferences.xml
    // -----------------------------

    public static String getPreloadOnWifiOnlyPreferenceString(Context context) {
        return context.getResources().getString(R.string.pref_data_preload_value_wifi_only);
    }

    public static String getPreloadAlwaysPreferenceString(Context context) {
        return context.getResources().getString(R.string.pref_data_preload_value_always);
    }

    private static final String DEAULT_PRELOAD_SECURE_SETTING_KEY =
            "browser_default_preload_setting";

    public String getDefaultPreloadSetting() {
        String preload = Settings.Secure.getString(mContext.getContentResolver(),
                DEAULT_PRELOAD_SECURE_SETTING_KEY);
        if (preload == null) {
            preload = mContext.getResources().getString(R.string.pref_data_preload_default_value);
        }
        return preload;
    }

    public String getPreloadEnabled() {
        return mPrefs.getString(PREF_DATA_PRELOAD, getDefaultPreloadSetting());
    }

    public static String getLinkPrefetchOnWifiOnlyPreferenceString(Context context) {
        return context.getResources().getString(R.string.pref_link_prefetch_value_wifi_only);
    }

    public static String getLinkPrefetchAlwaysPreferenceString(Context context) {
        return context.getResources().getString(R.string.pref_link_prefetch_value_always);
    }

    private static final String DEFAULT_LINK_PREFETCH_SECURE_SETTING_KEY =
            "browser_default_link_prefetch_setting";

    public String getDefaultLinkPrefetchSetting() {
        String preload = Settings.Secure.getString(mContext.getContentResolver(),
            DEFAULT_LINK_PREFETCH_SECURE_SETTING_KEY);
        if (preload == null) {
            preload = mContext.getResources().getString(R.string.pref_link_prefetch_default_value);
        }
        return preload;
    }

    public String getLinkPrefetchEnabled() {
        return mPrefs.getString(PREF_LINK_PREFETCH, getDefaultLinkPrefetchSetting());
    }

    // -----------------------------
    // getter/setters for browser recovery
    // -----------------------------
    /**
     * The last time browser was started.
     * @return The last browser start time as System.currentTimeMillis. This
     * can be 0 if this is the first time or the last tab was closed.
     */
    public long getLastRecovered() {
        return mPrefs.getLong(KEY_LAST_RECOVERED, 0);
    }

    /**
     * Sets the last browser start time.
     * @param time The last time as System.currentTimeMillis that the browser
     * was started. This should be set to 0 if the last tab is closed.
     */
    public void setLastRecovered(long time) {
        mPrefs.edit()
            .putLong(KEY_LAST_RECOVERED, time)
            .apply();
    }

    /**
     * Used to determine whether or not the previous browser run crashed. Once
     * the previous state has been determined, the value will be set to false
     * until a pause is received.
     * @return true if the last browser run was paused or false if it crashed.
     */
    public boolean wasLastRunPaused() {
        return mPrefs.getBoolean(KEY_LAST_RUN_PAUSED, false);
    }

    /**
     * Sets whether or not the last run was a pause or crash.
     * @param isPaused Set to true When a pause is received or false after
     * resuming.
     */
    public void setLastRunPaused(boolean isPaused) {
        mPrefs.edit()
            .putBoolean(KEY_LAST_RUN_PAUSED, isPaused)
            .apply();
    }

    /// M: Add for update the settings. @{
    public void onConfigurationChanged(Configuration config) {
        updateSearchEngine(false);
    }
    /// @}

    /// M: Add for Regional Phone support. @{
    public void updateSearchEngineSetting() {
        IBrowserRegionalPhoneExt browserRegionalPhone = Extensions.getRegionalPhonePlugin(mContext);
        String searchEngineName = browserRegionalPhone.getSearchEngine(mPrefs, mContext);
        if (searchEngineName == null) {
            Log.i(XLOGTAG, "updateSearchEngineSetting ---no change");
            return;
        }

        SearchEngineManager searchEngineManager = (SearchEngineManager) mContext
                .getSystemService(SearchEngineManager.SEARCH_ENGINE_SERVICE);
        com.mediatek.common.search.SearchEngine searchEngineInfo =
                                                   searchEngineManager.getByName(searchEngineName);
        if (searchEngineInfo == null) {
            Log.i(XLOGTAG, "updateSearchEngineSetting ---" + searchEngineName + " not found");
            return;
        }

        String searchEngineFavicon = searchEngineInfo.getFaviconUri();
        SharedPreferences.Editor editor = mPrefs.edit();
        editor.putString(BrowserSettings.PREF_SEARCH_ENGINE, searchEngineName);
        editor.putString(BrowserSettings.PREF_SEARCH_ENGINE_FAVICON, searchEngineFavicon);
        editor.commit();
        Log.i(XLOGTAG, "updateSearchEngineSetting --" +
                        searchEngineName + "--" + searchEngineFavicon);
    }
    /// @}

    /**
     * M: Set the double-tap zoom of the page in percent. Default is 100.
     * @param doubleTapZoom A percent value for increasing or decreasing the double-tap zoom.
     * @hide
     * @internal
     */
    public void setDoubleTapZoom(WebSettings settings, int doubleTapZoom) {
        // Because others will extend WebSettings, don't throw exception.
        /// M: Add to fix 3169690.
        /// Deliver the double-tap setting to AwSettings. @{
        try {
            Method getAwSettings= settings.getClass().getDeclaredMethod("getAwSettings");
            getAwSettings.setAccessible(true);
            Object awSettings = getAwSettings.invoke(settings);
            Method setMethod = awSettings.getClass().getMethod("setDoubleTapZoom", int.class);
            setMethod.invoke(awSettings, doubleTapZoom);
        } catch (NoSuchMethodException e) {
            Log.e("WebSettings", "No such method for setDoubleTapZoom: " + e);
        } catch (IllegalAccessException e) {
            Log.e("WebSettings", "Illegal access for setDoubleTapZoom:" + e);
        } catch (InvocationTargetException e) {
            Log.e("WebSettings", "Invocation target exception for setDoubleTapZoom: " + e);
        } catch (NullPointerException e) {
            Log.e("WebSettings", "Null pointer for setDoubleTapZoom: " + e);
        }
        /// @}
    }

}
