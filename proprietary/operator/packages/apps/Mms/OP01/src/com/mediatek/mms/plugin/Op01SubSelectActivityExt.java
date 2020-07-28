package com.mediatek.mms.plugin;

import android.app.Activity;
import android.content.Context;
import android.view.View;

import com.mediatek.mms.ext.DefaultOpSubSelectActivityExt;

/**
 * Op01SubSelectActivityExt.
 *
 */
public class Op01SubSelectActivityExt extends DefaultOpSubSelectActivityExt {

    private Op01MmsPreference mMmsPreferenceExt = null;

    /**
     * Construction.
     * @param context Context
     */
    public Op01SubSelectActivityExt(Context context) {
        mMmsPreferenceExt = new Op01MmsPreference(context);
    }

    @Override
    public void onCreate(Activity hostActivity) {
        mMmsPreferenceExt.configSelectCardPreferenceTitle(hostActivity);
    }
    /* q0 migration, phase out sms validate*/
    /*
    @Override
    public boolean onListItemClick(Activity hostActivity, final int subId) {
        return mMmsPreferenceExt.handleSelectCardPreferenceTreeClick(hostActivity, subId);
    }
*/
    @Override
    public View getView(String preferenceKey, View view) {
        return mMmsPreferenceExt.getView(preferenceKey, view);
    }

    @Override
    public String[] setSaveLocation() {
        return mMmsPreferenceExt.getSaveLocationString();
    }
}
