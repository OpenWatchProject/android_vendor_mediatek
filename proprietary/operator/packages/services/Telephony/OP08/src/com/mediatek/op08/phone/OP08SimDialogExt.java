package com.mediatek.op08.phone;

import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.mediatek.phone.ext.DefaultSimDialogExt;
import com.mediatek.phone.ext.ISimDialogExt;

public class OP08SimDialogExt extends DefaultSimDialogExt {
    private static final String TAG = "OP08SimDialogExt";
    private Context mContext;
    public static boolean isTMOLoad = false;
    public OP08SimDialogExt(Context context) {
        super();
        mContext = context;
    }

    @Override
    public boolean isNeedShowRoamingDialog() {
        Log.i(TAG, "isNeedShowRoamingDialog true");
        return true;
    }
}
