package com.mediatek.browser.ext;

import android.app.Activity;
import android.content.Intent;
import android.util.Log;
import android.webkit.WebView;


public class DefaultBrowserMiscExt implements IBrowserMiscExt {

    private static final String TAG = "DefaultBrowserMiscExt";

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data, Object obj) {
        Log.i("@M_" + TAG, "Enter: " + "onActivityResult" + " --default implement");
        return;
    }

    @Override
    public void processNetworkNotify(WebView view, Activity activity, boolean isNetworkUp) {
        Log.i("@M_" + TAG, "Enter: " + "processNetworkNotify" + " --default implement");
        if (!isNetworkUp) {
            view.setNetworkAvailable(false);
        }
    }

}