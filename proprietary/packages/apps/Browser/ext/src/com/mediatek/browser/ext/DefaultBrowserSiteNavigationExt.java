package com.mediatek.browser.ext;

import android.util.Log;

public class DefaultBrowserSiteNavigationExt implements IBrowserSiteNavigationExt {

    private static final String TAG = "DefaultBrowserSiteNavigationExt";

    @Override
    public CharSequence[] getPredefinedWebsites() {
        Log.i("@M_" + TAG, "Enter: " + "getPredefinedWebsites" + " --default implement");
        return null;
    }

    @Override
    public int getSiteNavigationCount() {
        Log.i("@M_" + TAG, "Enter: " + "getSiteNavigationCount" + " --default implement");
        return 0;
    }

}
