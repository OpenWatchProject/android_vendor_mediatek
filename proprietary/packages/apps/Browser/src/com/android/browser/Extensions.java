package com.android.browser;

import android.content.Context;

import com.mediatek.browser.ext.DefaultBrowserBookmarkExt;
import com.mediatek.browser.ext.DefaultBrowserDownloadExt;
import com.mediatek.browser.ext.DefaultBrowserHistoryExt;
import com.mediatek.browser.ext.DefaultBrowserMiscExt;
import com.mediatek.browser.ext.DefaultBrowserRegionalPhoneExt;
import com.mediatek.browser.ext.DefaultBrowserSettingExt;
import com.mediatek.browser.ext.DefaultBrowserSiteNavigationExt;
import com.mediatek.browser.ext.DefaultBrowserUrlExt;
import com.mediatek.browser.ext.DefaultNetworkStateHandlerExt;
import com.mediatek.browser.ext.IBrowserBookmarkExt;
import com.mediatek.browser.ext.IBrowserDownloadExt;
import com.mediatek.browser.ext.IBrowserHistoryExt;
import com.mediatek.browser.ext.IBrowserMiscExt;
import com.mediatek.browser.ext.IBrowserRegionalPhoneExt;
import com.mediatek.browser.ext.IBrowserSettingExt;
import com.mediatek.browser.ext.IBrowserSiteNavigationExt;
import com.mediatek.browser.ext.IBrowserUrlExt;
import com.mediatek.browser.ext.INetworkStateHandlerExt;
import com.mediatek.browser.ext.OpBrowserCustomizationFactoryBase;

/**
 * Helper class to create plugin instance.
 */
public class Extensions {
    private static volatile IBrowserBookmarkExt sBookmarkPlugin = null;
    private static volatile IBrowserDownloadExt sDownloadPlugin = null;
    private static volatile IBrowserHistoryExt sHistoryPlugin = null;
    private static volatile IBrowserMiscExt sMiscPlugin = null;
    private static volatile IBrowserRegionalPhoneExt sRegionalPhonePlugin = null;
    private static volatile IBrowserSettingExt sSettingPlugin = null;
    private static volatile IBrowserSiteNavigationExt sSiteNavigationPlugin = null;
    private static volatile IBrowserUrlExt sUrlPlugin = null;
    private static volatile INetworkStateHandlerExt sNetworkPlugin = null;

    private Extensions() {
    };

    /**
     * get browser bookmark plugin instance.
     * @param context browser context
     * @return browser bookmark plugin instance
     */
    public static IBrowserBookmarkExt getBookmarkPlugin(Context context) {
        if (sBookmarkPlugin == null) {
            synchronized (Extensions.class) {
                if (sBookmarkPlugin == null) {
                    sBookmarkPlugin = OpBrowserCustomizationFactoryBase
                            .getOpFactory(context).makeBrowserBookmarkExt();
                }
            }
        }
        return sBookmarkPlugin;
    }

    /**
     * get browser download plugin instance.
     * @param context browser context
     * @return browser download plugin instance
     */
    public static IBrowserDownloadExt getDownloadPlugin(Context context) {
        if (sDownloadPlugin == null) {
            synchronized (Extensions.class) {
                if (sDownloadPlugin == null) {
                    sDownloadPlugin = OpBrowserCustomizationFactoryBase
                            .getOpFactory(context).makeBrowserDownloadExt();
                }
            }
        }
        return sDownloadPlugin;
    }

    /**
     * get browser history plugin instance.
     * @param context browser context
     * @return browser history plugin instance
     */
    public static IBrowserHistoryExt getHistoryPlugin(Context context) {
        if (sHistoryPlugin == null) {
            synchronized (Extensions.class) {
                if (sHistoryPlugin == null) {
                    sHistoryPlugin = OpBrowserCustomizationFactoryBase
                            .getOpFactory(context).makeBrowserHistoryExt();
                }
            }
        }
        return sHistoryPlugin;
    }

    /**
     * get browser misc plugin instance.
     * @param context browser context
     * @return browser misc plugin instance
     */
    public static IBrowserMiscExt getMiscPlugin(Context context) {
        if (sMiscPlugin == null) {
            synchronized (Extensions.class) {
                if (sMiscPlugin == null) {
                    sMiscPlugin = OpBrowserCustomizationFactoryBase
                            .getOpFactory(context).makeBrowserMiscExt();
                }
            }
        }
        return sMiscPlugin;
    }

    /**
     * get browser regional phone plugin instance.
     * @param context browser context
     * @return browser regional phone plugin instance
     */
    public static IBrowserRegionalPhoneExt getRegionalPhonePlugin(Context context) {
        if (sRegionalPhonePlugin == null) {
            synchronized (Extensions.class) {
                if (sRegionalPhonePlugin == null) {
                    sRegionalPhonePlugin = OpBrowserCustomizationFactoryBase
                            .getOpFactory(context).makeBrowserRegionalPhoneExt();
                }
            }
        }
        return sRegionalPhonePlugin;
    }

    /**
     * get browser setting plugin instance.
     * @param context browser context
     * @return browser setting plugin instance
     */
    public static IBrowserSettingExt getSettingPlugin(Context context) {
        if (sSettingPlugin == null) {
            synchronized (Extensions.class) {
                if (sSettingPlugin == null) {
                    sSettingPlugin = OpBrowserCustomizationFactoryBase
                            .getOpFactory(context).makeBrowserSettingExt();
                }
            }
        }
        return sSettingPlugin;
    }

    /**
     * get browser site navigation plugin instance.
     * @param context browser context
     * @return browser site navigation plugin instance
     */
    public static IBrowserSiteNavigationExt getSiteNavigationPlugin(Context context) {
        if (sSiteNavigationPlugin == null) {
            synchronized (Extensions.class) {
                if (sSiteNavigationPlugin == null) {
                    sSiteNavigationPlugin = OpBrowserCustomizationFactoryBase
                            .getOpFactory(context).makeBrowserSiteNavigationExt();
                }
            }
        }
        return sSiteNavigationPlugin;
    }

    /**
     * get browser url plugin instance.
     * @param context browser context
     * @return browser url plugin instance
     */
    public static IBrowserUrlExt getUrlPlugin(Context context) {
        if (sUrlPlugin == null) {
            synchronized (Extensions.class) {
                if (sUrlPlugin == null) {
                    sUrlPlugin = OpBrowserCustomizationFactoryBase
                            .getOpFactory(context).makeBrowserUrlExt();
                }
            }
        }
        return sUrlPlugin;
    }
    /**
     * get browser network state handler plugin instance.
     * @param context browser context
     * @return browser network state handler plugin instance
     */
    public static INetworkStateHandlerExt getNetworkStateHandlerPlugin(Context context) {
        if (sNetworkPlugin == null) {
            synchronized (Extensions.class) {
                if (sNetworkPlugin == null) {
                    sNetworkPlugin = OpBrowserCustomizationFactoryBase
                            .getOpFactory(context).makeNetworkStateHandlerExt();
                }
            }
        }
        return sNetworkPlugin;
    }

    static void resetPlugins() {
        synchronized (Extensions.class) {
            sBookmarkPlugin = null;
            sDownloadPlugin = null;
            sHistoryPlugin = null;
            sMiscPlugin = null;
            sRegionalPhonePlugin = null;
            sSettingPlugin = null;
            sSiteNavigationPlugin = null;
            sUrlPlugin = null;
            sNetworkPlugin = null;
            OpBrowserCustomizationFactoryBase.resetOpFactory();
        }
    }

}
