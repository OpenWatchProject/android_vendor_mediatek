package com.mediatek.contacts.ext;

import android.app.Fragment;
import android.content.Context;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.preference.PreferenceFragment;
import android.view.Menu;
import android.widget.EditText;

/**
 * for op01 plugin
 */
public interface IOp01Extension {
    //--------------for QuickContact----------------//
    /**
     * Op01 will check video button visibility or not.
     * @param enabled Host Video Enabled
     * @param uri Contact Uri
     * @param params Extend Parameters
     * @return True or False
     */
    boolean isVideoButtonEnabled(boolean enabled, Uri uri, Object...params);

    /**
     * Op01 will reset values of video state.
     */
    void resetVideoState();
}
