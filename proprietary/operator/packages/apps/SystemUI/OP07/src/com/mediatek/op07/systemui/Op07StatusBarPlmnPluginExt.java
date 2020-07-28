package com.mediatek.op07.systemui;

import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.ColorStateList;
import android.widget.TextView;
import android.view.View;
import android.widget.LinearLayout;
import android.util.Log;
import com.mediatek.systemui.ext.DefaultStatusBarPlmnPlugin;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.op07.systemui.R;

public class Op07StatusBarPlmnPluginExt extends DefaultStatusBarPlmnPlugin {
    static final String TAG = "Op07StatusBarPlmnPluginExt";
    private String mPlmn = null;
    private static TextView sPlmnTextView = null;
    Context mconxt = null;
    Context mContext = null;

    public Op07StatusBarPlmnPluginExt(Context context) {
        super(context);
        mContext = context;
        // receive broadcasts
        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.SPN_STRINGS_UPDATED_ACTION);
        context.registerReceiver(mBroadcastReceiver, filter);
        if (sPlmnTextView == null)
            sPlmnTextView = new TextView(context);
            sPlmnTextView.setTextAppearance(android.R.style.TextAppearance_Small_Inverse);
    }

    /********************************************************************************/
    /********* Broadcast receiver for change in network *****************************/
    /****
     * Receives the intent to display and change the operator name on status bar
     ***/
    /********************************************************************/

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "into broadcast recevier");
            String action = intent.getAction();

            if (TelephonyIntents.SPN_STRINGS_UPDATED_ACTION.equals(action)) {
                Log.d(TAG, "SPN_STRINGS_UPDATED_ACTION");
                updateNetworkName(intent.getBooleanExtra(
                        TelephonyIntents.EXTRA_SHOW_SPN, false),
                        intent.getStringExtra(TelephonyIntents.EXTRA_SPN),
                        intent.getBooleanExtra(
                                TelephonyIntents.EXTRA_SHOW_PLMN, false),
                        intent.getStringExtra(TelephonyIntents.EXTRA_PLMN));

            }
        }

    };

    /*****************************************************************************/
    /*************************** updateNetworkName *********************************/
    /*************************** receive the textview and add the text *************/
    /*****************************************************************************/

    private String mOldPlmn = null;

    private void updateNetworkName(boolean showSpn, String spn,
            boolean showPlmn, String plmn) {
        Log.d(TAG, "For AT&T updateNetworkName, showSpn=" + showSpn + " spn="
                + spn + " showPlmn=" + showPlmn + " plmn=" + plmn);

        StringBuilder str = new StringBuilder();
        boolean something = false;
        if (showPlmn && plmn != null) {
            str.append(plmn);
            something = true;
        }
        if (showSpn && spn != null) {
            if (something) {
                str.append(mContext.getString(
                                 com.android.internal.R.string.kg_text_message_separator));
            }
            str.append(spn);
            something = true;
        }
        if (something) {
            mOldPlmn = str.toString();
        } else {
            mOldPlmn = mContext.getResources().getString(
                    com.android.internal.R.string.lockscreen_carrier_default);
        }
        sPlmnTextView.setText(str);
    }

    public TextView getPlmnTextView(Context context) {
        Log.d(TAG, "return sPlmnTextView");
        return sPlmnTextView;
    }

    /*
     * * update the plmn when in search mode
     */
    private void updatePLMNSearchingStateView(boolean searching) {
        if (searching) {
            sPlmnTextView.setText(R.string.plmn_searching);
            Log.d(TAG, "updatePLMNSearchingStateView");
        } else {
            sPlmnTextView.setText(mOldPlmn);
            Log.d(TAG, "updatePLMNSearchingStateView");
        }
    }

    /*
     * * add plmn text view to status bar
     */
    public void addPlmn(LinearLayout statusBarContents, Context sysContx) {
        Log.d(TAG, "addPlmn, set visible");
        if(statusBarContents != null) {
            statusBarContents.addView(sPlmnTextView, 0);
            sPlmnTextView.setVisibility(View.VISIBLE);
        } else {
            Log.e(TAG, "statusBarContents is null");
        }
    }

    public void setPlmnVisibility(int visibility) {
        sPlmnTextView.setVisibility(visibility);
    }

    public static void setTint(int tint) {
        if (sPlmnTextView != null) {
            sPlmnTextView.setTextColor(ColorStateList.valueOf(tint));
        }
    }

}