package com.android.mms.widget;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.UserManager;
import android.widget.Toast;

import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.data.Conversation;
import com.android.mms.ui.ComposeMessageActivity;
import com.android.mms.ui.ConversationList;
import com.android.mms.util.MmsLog;
import com.mediatek.cb.cbmsg.CbMessageListActivity;
import com.mediatek.wappush.ui.WPMessageActivity;

import mediatek.telephony.MtkTelephony;

public class MmsWidgetProxyActivity extends Activity {
    private static final String TAG = "MmsWidgetProxyActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Intent intent = getIntent();
        Context context = getApplicationContext();
        MmsLog.d(TAG, "onResume Action is " + intent.getAction());
        if (MmsWidgetService.ACTION_CONVERSATION_MODE.equals(intent.getAction())) {
            startActivityForConv(context, intent);
        } else if (MmsWidgetProvider.ACTION_COMPOSE_NEW_MESSAGE.equals(intent.getAction())) {
            startActivityForNew(context);
        }
        finish();
    }

    private void startActivityForNew(Context context) {
        MmsLog.d(TAG, "compose new message isSmsEnabled" + MmsConfig.isSmsEnabled(this));

        if (MmsConfig.isSmsEnabled(this)) {
            Intent ic = new Intent(context, ComposeMessageActivity.class);
            startActivityInternal(context, ic);
        } else if (UserManager.get(getApplicationContext()).hasUserRestriction(
                  UserManager.DISALLOW_SMS)){
            Toast.makeText(this, R.string.user_has_no_premission, Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(this, R.string.compose_disabled_toast, Toast.LENGTH_SHORT).show();
        }
    }

    private void startActivityForConv(Context context, Intent intent) {
        Intent ic = null;
        int msgType = intent.getIntExtra(MmsWidgetService.EXTRA_KEY_CONVSATION_TYPE, 0);
        long threadId = intent.getLongExtra("thread_id", 0);
        MmsLog.d(TAG, "conversation mode -- msgType=" + msgType + "  thread_id=" + threadId);
        Conversation conv = Conversation.createNew(context);
        switch (msgType) {
        case MtkTelephony.MtkThreads.CELL_BROADCAST_THREAD:
            MmsLog.d(TAG, "conversation mode -- CB");
            ic = CbMessageListActivity.createIntent(context, threadId);
            break;
        case MtkTelephony.MtkThreads.WAPPUSH_THREAD:
            MmsLog.d(TAG, "conversation mode -- push");
            ic = WPMessageActivity.createIntent(context, threadId);
            break;
        case MmsWidgetService.MORE_MESSAGES:
            MmsLog.d(TAG, "conversation mode -- more message");
            ic = new Intent(context, ConversationList.class);
            break;
        default:
            MmsLog.d(TAG, "conversation mode -- normal message");
            ic = ComposeMessageActivity.createIntent(context, threadId);
            break;
        }
        startActivityInternal(context, ic);
    }

    private void startActivityInternal(Context context, Intent intent) {
        MmsLog.d(TAG, "startActivityInternal is called");
        try {
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
            context.startActivity(intent);
        } catch (Exception e) {
            MmsLog.e(TAG, "Failed to start intent activity", e);
        }
    }
}
