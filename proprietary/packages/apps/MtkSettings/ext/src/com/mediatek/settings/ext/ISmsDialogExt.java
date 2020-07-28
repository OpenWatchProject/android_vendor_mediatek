package com.mediatek.settings.ext;

import android.content.Context;
import android.view.KeyEvent;
import android.content.Intent;
import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;



public interface ISmsDialogExt {

    /**
    * Called when yes/No is clicked on dialog from SmsDefaultDialog
    * @param newPackageName Package name of the new sms application
    * @param activity Alertactivity instance
    * @param context of the current acitivity
    * @param which yes/No is pressed
    * @return whether to set default application or not.
    * @internal
    */
    public boolean onClick(String newPackageName, AlertActivity activity, Context context,
                           int which);

    /**
    * Called on Keydown is pressed on default dialog from SmsDefaultDialog
    * @param keyCode Keycode of key pressed
    * @param event of key pressed
    * @param context of AlertActivity
    * @return whether to set default application or not.
    * @internal
    */
    public boolean onKeyDown(int keyCode, KeyEvent event, AlertActivity context) ;

    /**
    * Called to alter the dialog message to be shown from SmsDefaultDialog
    * @param param parameters of the Alert Dialog to be shown
    * @param packageName Old Package name of the sms application
    * @param intent Intent to be thrown
    * @param newName appname of the sms application needs to be set
    * @param oldName Old appname of sms application set
    * @internal
    */
    public void buildMessage(AlertController.AlertParams  param, String packageName, Intent intent,
                             String newName, String oldName) ;

}
