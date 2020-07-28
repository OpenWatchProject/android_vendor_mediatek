package com.mediatek.rcse.plugin.message;

import com.mediatek.mms.ipmessage.DefaultIpMessageListAdapterExt;
import com.mediatek.rcse.api.Logger;

import android.content.Context;
import android.database.Cursor;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Sms;
import android.telephony.SmsManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.mediatek.rcs.R;

public class RcseMessageListAdapter extends DefaultIpMessageListAdapterExt {
    private static String TAG = "RcseMessageListAdapter";

    /// M: add for ipmessage
    private static final int INCOMING_ITEM_TYPE_IPMSG = 2;
    private static final int OUTGOING_ITEM_TYPE_IPMSG = 3;

    /// M: add for ipmessage
    static final int COLUMN_SMS_IP_MESSAGE_ID   = 22;
    static final int COLUMN_SMS_SPAM            = 23;

    static final int COLUMN_MSG_TYPE            = 0;
    static final int COLUMN_SMS_TYPE            = 10;
    static final int COLUMN_SMS_STATUS          = 11;
    static final int COLUMN_SMS_ADDRESS         = 8;
    static final int COLUMN_SMS_DATE            = 4;
    static final int COLUMN_SMS_DATE_SENT       = 5;

    static final int IP_VIEW_TYPE_COUNT = 4;

    private Context mPluginContext;

    public RcseMessageListAdapter(Context pluginContext) {
        mPluginContext = pluginContext;
    }


    @Override
    public View onIpNewView(LayoutInflater inflater, Cursor cursor, ViewGroup parent) {
       LayoutInflater pluginInflater = LayoutInflater.from(mPluginContext);
        View retView = null;
        // / M: add for ipmessage
        switch (getIpItemViewType(cursor)) {
            case INCOMING_ITEM_TYPE_IPMSG:
                retView =  pluginInflater.inflate(R.layout.message_list_item_recv_ipmsg, parent, false);
                break;
            case OUTGOING_ITEM_TYPE_IPMSG:
               retView =  pluginInflater.inflate(R.layout.message_list_item_send_ipmsg, parent, false);
               Logger.v(TAG, "onIpNewView(): message sent retView = " + retView);
        }
        return retView;
    }

    @Override
    public int getIpItemViewType(Cursor cursor) {
        /// M: add for ipmessage
        String type = cursor.getString(COLUMN_MSG_TYPE);
        Logger.v(TAG, "getIpItemViewType(): message type = " + type);

        int boxId;
        if ("sms".equals(type)) {
            /// M: check sim sms and set box id
            long status = cursor.getLong(COLUMN_SMS_STATUS);
            boolean isSimMsg = false;
            if (status == SmsManager.STATUS_ON_ICC_SENT
                    || status == SmsManager.STATUS_ON_ICC_UNSENT) {
                isSimMsg = true;
                boxId = Sms.MESSAGE_TYPE_SENT;
            } else if (status == SmsManager.STATUS_ON_ICC_READ
                    || status == SmsManager.STATUS_ON_ICC_UNREAD) {
                isSimMsg = true;
                boxId = Sms.MESSAGE_TYPE_INBOX;
            } else {
                boxId = cursor.getInt(COLUMN_SMS_TYPE);
            }
            Logger.v(TAG, "getIpItemViewType(): boxid  = " + boxId + "isSimMsg:" + isSimMsg);
            long ipMessageId = cursor.getLong(COLUMN_SMS_IP_MESSAGE_ID);
            Logger.v(TAG, "getItemViewType(): ipMessageId = " + ipMessageId);
            if (ipMessageId > 0 && !isSimMsg) {
                return boxId == Mms.MESSAGE_BOX_INBOX ? INCOMING_ITEM_TYPE_IPMSG : OUTGOING_ITEM_TYPE_IPMSG;
            }
        }
        return -1;
    }

    @Override
    public int getIpViewTypeCount() {
        return IP_VIEW_TYPE_COUNT;
    }
}
