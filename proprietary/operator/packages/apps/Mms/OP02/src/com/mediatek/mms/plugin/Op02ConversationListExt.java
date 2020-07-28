package com.mediatek.mms.plugin;

import android.content.Context;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.view.Menu;
import android.view.MenuItem;
import android.util.Log;

//import android.widget.SearchView;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.mms.ext.DefaultOpConversationListExt;
import com.mediatek.mms.plugin.R;

public class Op02ConversationListExt extends DefaultOpConversationListExt {

    private static final String TAG = "Op02ConversationListExt";
    private int mMenuSimSms = 101;

    public static final String ACTION_VIEW_SELECT_CARD =
                                    "com.mediatek.setting.SubSelectActivity";
    public static final String ACTION_VIEW_SIM_MESSAGES =
                                    "com.mediatek.simmessage.ManageSimMessages";
    public static final String PREFERENCE_KEY = "PREFERENCE_KEY";
    public static final String SMS_MANAGE_SIM_MESSAGES = "pref_key_manage_sim_messages";

    public Op02ConversationListExt(Context context) {
        super(context);
    }

    @Override
    public void onCreateOptionsMenu(Menu menu) {
        Drawable sMenuIcon =
            getResources().getDrawable(R.drawable.ic_menu_sim_sms);

        menu.add(0, mMenuSimSms, 0, getString(R.string.menu_sim_sms)).setIcon(
                    sMenuIcon);

        Log.d(TAG, "Add Menu: " + getString(R.string.menu_sim_sms));
    }

    @Override
    public void onPrepareOptionsMenu(Menu menu) {
        MenuItem item = menu.findItem(mMenuSimSms);
        if (item == null) {
            Log.e(TAG, "onPrepareOptionsMenu: menu item should not be null");
            return;
        }
        if (!Op02MmsUtils.isSmsEnabled(this) ||
           !Op02MmsUtils.isSimInserted(this) ||
            Op02MmsUtils.isAirplaneOn(this) ||
            !Op02MmsUtils.isSmsReady(this)) {
            item.setEnabled(false);
            Log.d(TAG, "Menu sim sms entry is disabled");
        } else {
            item.setEnabled(true);
            Log.d(TAG, "Menu sim sms entry is enabled");
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item, int actionSettingsId) {
        if (item.getItemId() == mMenuSimSms) {
            int count = Op02MmsUtils.getSimCount(this);
            if (count > 1) {
                Intent simSmsIntent = new Intent(ACTION_VIEW_SELECT_CARD);
                simSmsIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                simSmsIntent.putExtra(PREFERENCE_KEY, SMS_MANAGE_SIM_MESSAGES);
                startActivity(simSmsIntent);
            } else if (count == 1) {
                Intent simSmsIntent = new Intent(ACTION_VIEW_SIM_MESSAGES);
                simSmsIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                simSmsIntent.putExtra(PhoneConstants.SUBSCRIPTION_KEY,
                                        Op02MmsUtils.getActiveSubId(this));
                startActivity(simSmsIntent);
            }
            return true;
        }
        return false;
    }
}
