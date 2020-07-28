package com.mediatek.op08.dialer.setting;

import android.content.Context;
import android.database.Cursor;
import android.content.Intent;
import android.preference.PreferenceActivity.Header;
import android.util.Log;

import com.mediatek.dialer.ext.DefaultDialerSettingExt;
import com.mediatek.op08.dialer.R;
import com.mediatek.op08.dialer.setting.SettingConstant;

import java.util.List;

public class Op08DialerSettingExt extends DefaultDialerSettingExt {
    private static final String TAG = "Op08DialerSettingExt ";
    private static final boolean sIsRcsOn = true;
    private Context mHostContext;
    private Context mPluginContext;

    public Op08DialerSettingExt(Context pluginContext, Context hostContext) {
        Log.d(TAG, "Op08DialerSettingExt");
        mHostContext = hostContext;
        mPluginContext = pluginContext;
    }

    @Override
    public void onBuildHeaders(List<Header> target){
        Log.d(TAG, "onBuildHeaders ");
        if (!sIsRcsOn) {
            Log.d(TAG, "sIsRcsOn false return directely");
            return;
        }
        Header enrichCallSettingHeader = new Header();
        Intent enrichCallSettingIntent =
                new Intent("com.mediatek.dialer.setting.ACTION_RCS_CALL_SETTING");
        enrichCallSettingHeader.title = mPluginContext.getString(R.string.rcs_enrich_call_setting);
        enrichCallSettingHeader.intent = enrichCallSettingIntent;
        target.add(enrichCallSettingHeader);

        /*for test provider*/
        /*Cursor cursor = SettingConstant.getDataFromProvider(mHostContext);
        if (cursor != null) {
            cursor.close();
        }*/
    }
}
