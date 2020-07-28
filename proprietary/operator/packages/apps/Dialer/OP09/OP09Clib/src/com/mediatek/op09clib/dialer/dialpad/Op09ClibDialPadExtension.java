package com.mediatek.op09clib.dialer.dialpad;

import android.content.Context;
import android.content.Intent;
import android.os.UserManager;
import android.util.Log;
import android.widget.Toast;
import com.mediatek.dialer.ext.DefaultDialPadExtension;
import com.mediatek.op09clib.dialer.R;

/**
 * dialpad extension plugin for op09.
*/
public class Op09ClibDialPadExtension extends DefaultDialPadExtension {
    private static final String TAG = "OP09ClibDialPadExtension";

    private static final String PRL_VERSION_DISPLAY = "*#0000#";
    private static final String CDMAINFO = "android.intent.action.CdmaInfoSpecification";

    private Context mPluginContext;

    public Op09ClibDialPadExtension(Context cnx) {
        super();
        mPluginContext = cnx;
    }

    /**
     * handle special chars from user input on dial pad.
     *
     * @param context from host app.
     * @param input from user input in dial pad.
     * @return boolean, check if the input string is handled.
     */
    public boolean handleChars(Context context, String input) {
        if (input.equals(PRL_VERSION_DISPLAY)) {
            showPRLVersionSetting(context);
            return true;
        }
        return false;
    }

    /**
     * show version by cdma phone provider info.
     *
     * @param context from host app.
     * @param slot indicator which slot is cdma phone.
     * @return void.
     */
    private void showPRLVersionSetting(Context context) {
        final UserManager userManager = UserManager.get(context);
        if (!userManager.isPrimaryUser()) {
            Toast.makeText(
                    mPluginContext,
                    mPluginContext.getResources().getString(
                        R.string.error_account_access), Toast.LENGTH_LONG)
                    .show();

            log("not primary user, just return.");
            return;
        }

        Intent intentCdma = new Intent(CDMAINFO);
        intentCdma.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intentCdma);
    }

    /**
     * simple log info.
     *
     * @param msg need print out string.
     * @return void.
     */
    private static void log(String msg) {
        Log.d(TAG, msg);
    }
}
