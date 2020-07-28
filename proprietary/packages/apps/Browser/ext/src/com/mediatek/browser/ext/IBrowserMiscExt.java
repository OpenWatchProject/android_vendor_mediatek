package com.mediatek.browser.ext;

import android.app.Activity;
import android.content.Intent;
import android.webkit.WebView;

public interface IBrowserMiscExt {

    /**
     * Handle the activity result
     * @param requestCode the request code
     * @param resultCode the result code
     * @param data the intent data
     * @param obj the different object for each requestCode
     */
    void onActivityResult(int requestCode, int resultCode, Intent data, Object obj);

    /**
     * Process the network notify
     * @param the webview
     * @param the activity
     * @param whether the network is up or not
     */
    void processNetworkNotify(WebView view, Activity activity, boolean isNetworkUp);

}