package com.mediatek.contacts.ext;

import android.app.Fragment;
import android.content.Context;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.preference.PreferenceFragment;
import android.view.Menu;
import android.widget.EditText;

public class DefaultOp01Extension implements IOp01Extension {
    //--------------for QuickContact----------------//
    /**
     * Op01 will check video button visibility or not.
     * @param enabled Host Video Enabled
     * @param uri Contact Uri
     * @param params Extend Parameters
     * @return True or False
     */
    @Override
    public boolean isVideoButtonEnabled(boolean enabled, Uri uri, Object...params) {
        return enabled;
    }

    /**
     * Op01 will reset values of video state.
     */
    @Override
    public void resetVideoState() {
        //do-nothing
    }
}
