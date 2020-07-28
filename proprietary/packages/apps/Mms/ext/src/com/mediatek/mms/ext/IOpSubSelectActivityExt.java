package com.mediatek.mms.ext;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.view.View;
import android.widget.CheckBox;

public interface IOpSubSelectActivityExt {
    /**
     * @internal
     */
    void onCreate(Activity hostActivity);
    /**
     * @internal
     */
    boolean onListItemClick(Activity hostActivity, final int subId);
    /**
     * @internal
     */
    String [] setSaveLocation();

    /**
     * @internal
     */
    boolean isSimSupported(int subId);

    /**
     * @internal
     */
    View getView(String preferenceKey, View view);

    /**
     * @internal
     */
    boolean getView(CheckBox subCheckBox, String preferenceKey);

    boolean getView(Context context, int position, CheckBox subCheckBox, String preferenceKey);

    /**
     * @internal
     */
    void onActivityResult(int requestCode, int resultCode, Intent data);

    /**
     * @internal
     */
    void onSimStateChanged();
}
