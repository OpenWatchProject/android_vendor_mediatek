package com.mediatek.op01.dialer.speeddial;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.os.Build;
import android.os.Bundle;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MenuItem.OnMenuItemClickListener;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;

import com.android.dialer.dialpadview.SpecialCharSequenceMgr;
import com.mediatek.dialer.ext.DefaultDialPadExtension;
import com.mediatek.internal.telephony.ratconfiguration.RatConfiguration;
import com.mediatek.op01.dialer.R;

import java.util.ArrayList;

public class Op01DialPadExtension extends DefaultDialPadExtension
    implements View.OnLongClickListener {

    private static final String TAG = "OP01DialPadExtension";

    private static final boolean DEBUG = ("eng".equals(Build.TYPE) ||
                                        "userdebug".equals(Build.TYPE));

    private String mHostPackage;
    private Resources mHostResources;
    private EditText mEditText;
    private Context mHostContext;
    private Context mContext;

    /**
     * for op01
     * @param durationView the duration text
     */

    public Op01DialPadExtension (Context context) {
        super();
        mContext = context;
    }

    @Override
    public void buildOptionsMenu(Context context, Menu menu){
        Log.d(TAG, "buildOptionsMenu.");
        int index = menu.size();
        MenuItem speedDialMenu = menu.add(Menu.NONE,
                index, 0, mContext.getText(R.string.call_speed_dial));
        speedDialMenu.setOnMenuItemClickListener(new OnMenuItemClickListener() {
            public boolean onMenuItemClick(MenuItem item) {
                Log.d(TAG, "SpeedDial onMenuItemClick");
                SpeedDialController.getInstance().enterSpeedDial(context);
                return true;
            }
        });
    }

    @Override
    public void onViewCreated(Activity activity, View view) {
        Log.d(TAG, "onViewCreated.");
        mHostContext = activity;
        mHostPackage = activity.getPackageName();
        mHostResources = activity.getResources();

        View two = (View) view.findViewById(mHostResources.getIdentifier("two",
                                "id", mHostPackage));
        two.setOnLongClickListener(this);

        View three = (View) view.findViewById(mHostResources.getIdentifier("three",
                                "id", mHostPackage));
        three.setOnLongClickListener(this);

        View four = (View) view.findViewById(mHostResources.getIdentifier("four",
                                "id", mHostPackage));
        four.setOnLongClickListener(this);

        View five = (View) view.findViewById(mHostResources.getIdentifier("five",
                                "id", mHostPackage));
        five.setOnLongClickListener(this);

        View six = (View) view.findViewById(mHostResources.getIdentifier("six",
                                "id", mHostPackage));
        six.setOnLongClickListener(this);

        View seven = (View) view.findViewById(mHostResources.getIdentifier("seven",
                                "id", mHostPackage));
        seven.setOnLongClickListener(this);

        View eight = (View) view.findViewById(mHostResources.getIdentifier("eight",
                                "id", mHostPackage));
        eight.setOnLongClickListener(this);

        View nine = (View) view.findViewById(mHostResources.getIdentifier("nine",
                                "id", mHostPackage));
        nine.setOnLongClickListener(this);

        mEditText = (EditText) view.findViewById(mHostResources.getIdentifier("digits",
                                "id", mHostPackage));
    }

    @Override
    public boolean onLongClick(View view) {
        int id = view.getId();

        int key = 0;
        if (id == mHostResources.getIdentifier("two","id", mHostPackage)) {
            key = 2;
        }
        else if (id == mHostResources.getIdentifier("three","id", mHostPackage)) {
            key = 3;
        }
        else if (id == mHostResources.getIdentifier("four","id", mHostPackage)) {
            key = 4;
        }
        else if (id == mHostResources.getIdentifier("five","id", mHostPackage)) {
            key = 5;
        }
        else if (id == mHostResources.getIdentifier("six","id", mHostPackage)) {
            key = 6;
        }
        else if (id == mHostResources.getIdentifier("seven","id", mHostPackage)) {
            key = 7;
        }
        else if (id == mHostResources.getIdentifier("eight","id", mHostPackage)) {
            key = 8;
        }
        else if (id == mHostResources.getIdentifier("nine","id", mHostPackage)) {
            key = 9;
        }

        if (key > 0 && key < 10 && mEditText.getText().length() <= 1) {
            SpeedDialController.getInstance().handleKeyLongProcess(mHostContext, mContext, key);
            mEditText.getText().clear();
            return true;
        }
        return false;
    }

    private boolean isC2KSupport() {
        return RatConfiguration.isC2kSupported();
    }

    @Override
    public boolean handleDeviceIdDisplay(
            ViewGroup viewGroup, boolean showDecimal, boolean showBarcode) {
        if (isC2KSupport()) {
            String meid = "";
            int count = TelephonyManager.getDefault().getPhoneCount();
            for (int i = 0; i < count; i++) {
                String imei = TelephonyManager.getDefault().getImei(i);
                if (DEBUG) {
                    if (imei != null) {
                        Log.d(TAG, "length of imei = " + imei.length());
                    }
                }
                if (!TextUtils.isEmpty(imei)) {
                    imei = "IMEI:" + imei;
                    SpecialCharSequenceMgr.addDeviceIdRow(
                            viewGroup, imei, showDecimal, showBarcode);
                }

                if (TextUtils.isEmpty(meid)) {
                    meid = TelephonyManager.getDefault().getMeid(i);
                    if (DEBUG) {
                        if(meid != null) {
                            Log.d(TAG, "length of meid = " + meid.length());
                        }
                    }
                }
            }
            if (!TextUtils.isEmpty(meid)) {
                meid = "MEID:" + meid;
                SpecialCharSequenceMgr.addDeviceIdRow(viewGroup, meid, showDecimal, showBarcode);
            }
            return true;
        }
        return false;
    }
}
