
package com.mediatek.phone;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.view.ContextThemeWrapper;
import android.view.WindowManager;

import com.android.phone.R;
import com.android.services.telephony.Log;
import com.mediatek.services.telephony.MtkTelephonyConnectionServiceUtil;

import java.util.ArrayList;

public class MtkSimErrorDialog extends AlertDialog implements OnClickListener {

    private static final int DIALOG_INFORMATON_SIZE = 4;

    public MtkSimErrorDialog(Context context, ArrayList<String> dialogInformation) {
//        super(new ContextThemeWrapper(context, R.style.SimErrorDialogTheme));
        super(new ContextThemeWrapper(context, null));
        getWindow().setType(WindowManager.LayoutParams.TYPE_PHONE);

        if (dialogInformation == null || dialogInformation.size() != DIALOG_INFORMATON_SIZE) {
            Log.d(this, "Finish this with illegle dialog information : ", dialogInformation);
            return;
        }

        setTitle(dialogInformation.get(0));
        setMessage(dialogInformation.get(1));
        setButton(DialogInterface.BUTTON_POSITIVE, dialogInformation.get(2), this);
        setButton(DialogInterface.BUTTON_NEGATIVE, dialogInformation.get(3), this);
        setOnDismissListener(new DialogInterface.OnDismissListener() {
            public void onDismiss(DialogInterface dialog) {
                MtkTelephonyConnectionServiceUtil.getInstance().cellConnMgrShowAlertingFinalize();
                setMtkSimErrorDialog(null);
                Log.d(MtkSimErrorDialog.this, "SimErrorDialog dismissed: " + MtkSimErrorDialog.this);
            }
        });

        setMtkSimErrorDialog(this);
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:
                if (MtkTelephonyConnectionServiceUtil.getInstance().isCellConnMgrAlive()) {
                    MtkTelephonyConnectionServiceUtil.getInstance().cellConnMgrHandleEvent();
                }
                Log.d(this, "SimErrorDialog onClick ok.");
                break;
            case DialogInterface.BUTTON_NEGATIVE:
                MtkTelephonyConnectionServiceUtil.getInstance().cellConnMgrShowAlertingFinalize();
                Log.d(this, "SimErrorDialog onClick cancel.");
                break;
            default:
                Log.d(this, "SimErrorDialog onClick.");
                break;
        }
    }

    private void setMtkSimErrorDialog(MtkSimErrorDialog dialog) {
        MtkTelephonyConnectionServiceUtil.getInstance().cellConnMgrSetSimErrorDialogActivity(dialog);
    }

    // When user want to leave this by press home key, we
    // should cancel the current dialog.
    public IntentFilter mIntentFilter = new IntentFilter(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
    public BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(this, "onReceive: cancel the request dialog. action = " + action);
            if (action.equals(Intent.ACTION_CLOSE_SYSTEM_DIALOGS)) {
                String reason = intent.getStringExtra("reason");
                if (reason != null && reason.equals("homekey")) {
                    dismiss();
                }
            }
        }
    };

    @Override
    protected void onStart() {
        super.onStart();
        getContext().registerReceiver(mReceiver, mIntentFilter);
    }

    @Override
    protected void onStop() {
        super.onStop();
        getContext().unregisterReceiver(mReceiver);
    }
}
