package com.mediatek.op18.mms;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.widget.Toast;
import com.mediatek.op18.mms.R;
import com.mediatek.mms.ext.DefaultOpSettingListActivityExt;
import com.mediatek.op18.mms.utils.SDCardUtils;



public class Op18SettingListActivityExt
        extends DefaultOpSettingListActivityExt {

    private static final String TAG = "OP18SettingListActivity";

    private Context mPluginContext;
    private Context mAppContext;

    public static Context mHostActivity;
    private static Toast mToast = null;
    //InitMessageTask mInitMessageTask = null;

    public Op18SettingListActivityExt(Context pluginContext) {
        mPluginContext = pluginContext;
    }

    @Override
    public boolean onOpCreate(Context hostActivity, Context appContext) {
        Log.d(TAG, "onOpCreate ");
        mHostActivity = hostActivity;
        mAppContext = appContext;
        return super.onOpCreate(hostActivity, appContext);
    }

    @Override
    public void onListItemClick(int position) {
        Log.d(TAG, "onListItemClick ");
        if (SDCardUtils.getExternalStoragePath(mPluginContext) == null) {
            Log.d(TAG, "No SD card ");
            if (mToast != null) {
                mToast.setText(R.string.nosdcard_notice);
                mToast.show();
                return;
            }
            mToast = Toast.makeText(mPluginContext, R.string.nosdcard_notice, Toast.LENGTH_SHORT);
            mToast.show();
            return;
        }

        //Intent intent = new Intent(mAppContext, MainActivity.class);
        Intent intent = new Intent();
        intent.setClassName("com.mediatek.op18.plugin", "com.mediatek.op18.plugin.MainActivity");
        //Intent intent = new Intent(mPluginContext, MainActivity.class);

        //intent.putExtra("host_context", mPluginContext);
        //intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        //intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK|
            //Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        mHostActivity.startActivity(intent);
        //mPluginContext.startActivity(intent);

    /*
        CharSequence options[] = new CharSequence[] {
            mPluginContext.getString(R.string.dialog_item_backup),
            mPluginContext.getString(R.string.dialog_item_restore)
            };

        AlertDialog.Builder builder = new AlertDialog.Builder(mHostActivity);
        builder.setTitle(mPluginContext.getString(R.string.dialog_title));
        builder.setItems(options, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                // the user clicked on option[which]
                switch(which) {
                    //Backup
                    case 0 :
                        // 1.Check whether backup is already running
                        break;
                    case 1 :
                        break;
                    default:
                        break;
                }
            }
        });
        builder.show();

        startInit();*/

    }

/*
    public void startInit() {
        mInitMessageTask = new InitMessageTask();
        mInitMessageTask.execute();
    }*/

}


