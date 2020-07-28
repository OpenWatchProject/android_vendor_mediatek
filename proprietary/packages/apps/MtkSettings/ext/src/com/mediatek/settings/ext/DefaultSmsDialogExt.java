
package com.mediatek.settings.ext;

import android.content.Context;
import android.content.ContextWrapper;

import android.view.KeyEvent;
import android.content.Intent;
import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

/*import com.android.settings.SmsDefaultDialog;
import com.android.settings.R;

*/


public class DefaultSmsDialogExt extends ContextWrapper implements ISmsDialogExt {
    private static final String TAG = "DefaultSmsDialogExt";

    public DefaultSmsDialogExt(Context context) {
           super(context);
    }

    public boolean onClick(String newPackageName, AlertActivity activity, Context context,
                           int which) {
        return true;
    }

    public boolean onKeyDown(int keyCode, KeyEvent event, AlertActivity context) {
            return false;
        }

    public void buildMessage(AlertController.AlertParams  param, String packageName, Intent intent,
                             String newName, String oldName){}
}
