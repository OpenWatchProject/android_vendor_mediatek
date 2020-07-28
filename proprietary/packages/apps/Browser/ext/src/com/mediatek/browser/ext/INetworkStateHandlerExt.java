package com.mediatek.browser.ext;

import android.app.Activity;

/**
 * Interface for operator feature.
 */
public interface INetworkStateHandlerExt {

    /**
     * Show pop up on need.
     * @param activity activity
     */
    void promptUserToEnableData(Activity activity);
}