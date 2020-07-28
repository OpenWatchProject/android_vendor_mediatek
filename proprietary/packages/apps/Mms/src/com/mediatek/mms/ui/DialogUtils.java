package com.mediatek.mms.ui;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.view.KeyEvent;

import com.android.mms.R;

public class DialogUtils {

}

class NewProgressDialog extends ProgressDialog {
    private boolean mIsDismiss = false;
    public NewProgressDialog(Context context) {
        super(context);
    }

    public void dismiss() {
       if (isDismiss()) {
           super.dismiss();
       }
    }

    public synchronized void setDismiss(boolean isDismiss) {
        this.mIsDismiss = isDismiss;
    }

    public synchronized boolean isDismiss() {
        return mIsDismiss;
    }
}

class DeleteProgressDialogUtil {
    /**
     * Gets a delete progress dialog.
     * @param context the activity context.
     * @return the delete progress dialog.
     */
    public static NewProgressDialog getProgressDialog(Context context) {
        NewProgressDialog dialog = new NewProgressDialog(context);
        dialog.setCancelable(false);
        dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        dialog.setMessage(context.getString(R.string.deleting));
        dialog.setMax(1); /* default is one complete */
        // ignore the search key, when deleting we do not want the search bar come out.
        dialog.setOnKeyListener(new DialogInterface.OnKeyListener() {
            public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                return (keyCode == KeyEvent.KEYCODE_SEARCH);
            }
        });
        return dialog;
    }
}