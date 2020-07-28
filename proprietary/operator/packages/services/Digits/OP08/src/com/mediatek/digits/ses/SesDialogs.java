package com.mediatek.digits.ses;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.Toast;

public class SesDialogs {
    private static final String TAG = "SesDialogs";

    private SesDialogs() {
    }

    static public void createToast(Context context, int resId, int duration) {
        Toast.makeText(context, resId, duration).show();
    }

    static public AlertDialog createOkDialog(Context context, int titleResId, int textResId, int btnResId) {
        Log.d(TAG, "createOkDialog  titleResId=" + titleResId +
              ", textResId=" + textResId +
              ", btnResId=" + btnResId);

        AlertDialog dialog = new AlertDialog.Builder(context)
        .setIcon(android.R.drawable.ic_dialog_alert)
        .setTitle(titleResId)
        .setMessage(textResId)
        .setNegativeButton(btnResId, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.cancel();
            }
        })
        .create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        return dialog;
    }

    static public AlertDialog createOkDial611Dialog(Context context, int titleResId, int textResId,
            int btn1ResId, int btn2ResId) {
        Log.d(TAG, "createOkDial611Dialog  titleResId=" + titleResId +
              ", textResId=" + textResId +
              ", btn1ResId=" + btn1ResId +
              ", btn2ResId=" + btn2ResId);
        AlertDialog dialog = new AlertDialog.Builder(context)
        .setIcon(android.R.drawable.ic_dialog_alert)
        .setTitle(titleResId)
        .setMessage(textResId)
        .setPositiveButton(btn1ResId, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Intent intent = new Intent(Intent.ACTION_CALL, Uri.parse("tel:611")).setFlags(
                    Intent.FLAG_ACTIVITY_NO_HISTORY | Intent.FLAG_FROM_BACKGROUND);
                context.startActivity(intent);
                //dialog.dismiss();
            }
        })
        .setNegativeButton(btn2ResId, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.cancel();
            }
        })
        .create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        return dialog;
    }

    static public AlertDialog createLineInfoOkDialog(Context context, int titleResId, int textResId, int btnResId,
            String msisdn) {
        Log.d(TAG, "createLineInfoOkDialog  titleResId=" + titleResId +
              ", textResId=" + textResId +
              ", btnResId=" + btnResId +
              ", msisdn=" + msisdn);
        Resources res = context.getResources();
        AlertDialog dialog = new AlertDialog.Builder(context)
        .setIcon(android.R.drawable.ic_dialog_alert)
        .setTitle(titleResId)
        .setMessage(String.format(res.getString(textResId), msisdn))
        .setNegativeButton(btnResId, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.cancel();
            }
        })
        .create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        return dialog;
    }

    static public AlertDialog createLineInfoOkDial611Dialog(Context context, int titleResId, int textResId,
            int btn1ResId, int btn2ResId, String msisdn) {
        Log.d(TAG, "createLineInfoOkDial611Dialog  titleResId=" + titleResId +
              ", textResId=" + textResId +
              ", btn1ResId=" + btn1ResId +
              ", btn2ResId=" + btn2ResId +
              ", msisdn=" + msisdn);
        Resources res = context.getResources();
        AlertDialog dialog = new AlertDialog.Builder(context)
        .setIcon(android.R.drawable.ic_dialog_alert)
        .setTitle(titleResId)
        .setMessage(String.format(res.getString(textResId), msisdn))
        .setPositiveButton(btn1ResId, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Intent intent = new Intent(Intent.ACTION_CALL, Uri.parse("tel:611")).setFlags(
                    Intent.FLAG_ACTIVITY_NO_HISTORY | Intent.FLAG_FROM_BACKGROUND);
                context.startActivity(intent);
                //dialog.dismiss();
            }
        })
        .setNegativeButton(btn2ResId, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.cancel();
            }
        })
        .create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        return dialog;
    }

    static public AlertDialog createOkLogoutDialog(Context context, int titleResId, int textResId,
            int btn1ResId, int btn2ResId, Handler handler) {
        Log.d(TAG, "createOkLogoutDialog  titleResId=" + titleResId +
              ", textResId=" + textResId +
              ", btn1ResId=" + btn1ResId +
              ", btn2ResId=" + btn2ResId);
        AlertDialog dialog = new AlertDialog.Builder(context)
        .setIcon(android.R.drawable.ic_dialog_alert)
        .setTitle(titleResId)
        .setMessage(textResId)
        .setPositiveButton(btn1ResId, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Message message = handler.obtainMessage(SesManager.CMD_USER_LOGOUT);
                message.sendToTarget();
                //dialog.dismiss();
            }
        })
        .setNegativeButton(btn2ResId, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.cancel();
            }
        })
        .create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        return dialog;
    }

}
