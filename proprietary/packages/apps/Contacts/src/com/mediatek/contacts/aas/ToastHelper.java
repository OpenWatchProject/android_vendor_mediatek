package com.mediatek.contacts.aas;

import android.content.Context;
import android.widget.Toast;

public class ToastHelper {

    private Context mContext = null;
    private Toast mToast = null;

    public ToastHelper(Context context) {
        if (context == null) {
            throw new IllegalArgumentException();
        }
        mContext = context;
    }

    /**
     * Show a Toast(Toast.LENGTH_SHORT).
     *
     * @param text
     */
    public void showToast(String text) {
        if (mToast == null) {
            mToast = Toast.makeText(mContext, text, Toast.LENGTH_SHORT);
        } else {
            mToast.setText(text);
        }
        mToast.show();
    }

    /**
     * Show a Toast(Toast.LENGTH_SHORT).
     *
     * @param text
     */
    public void showToast(int resId) {
        if (mToast == null) {
            mToast = Toast.makeText(mContext, resId, Toast.LENGTH_SHORT);
        } else {
            mToast.setText(resId);
        }
        mToast.show();
    }

}
