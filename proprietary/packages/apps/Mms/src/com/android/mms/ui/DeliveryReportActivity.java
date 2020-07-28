/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2008 Esmertec AG.
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.mms.ui;

import android.app.ListActivity;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.os.Bundle;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Sms;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.Window;
import android.widget.ListView;

import com.android.mms.R;
import com.android.mms.data.Contact;
import com.android.mms.util.MmsLog;
import com.google.android.mms.pdu.CharacterSets;
import com.google.android.mms.pdu.EncodedStringValue;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.android.mms.pdu.MtkPduPersister;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.opmsg.util.OpMessageUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

/*
 * This is the UI for displaying a delivery report: This activity can handle the
 * following parameters from the intent by which it is launched: thread_id long
 * The id of the conversation from which to get the recipients for the report.
 * message_id long The id of the message about which a report should be
 * displayed. message_type String The type of message (Sms or Mms). This is used
 * in conjunction with the message id to retrive the particular message that the
 * report will be about.
 */
public class DeliveryReportActivity extends ListActivity {
    private static final String LOG_TAG = "DeliveryReportActivity";

    static final String[] MMS_REPORT_REQUEST_PROJECTION = new String[] {
        Mms.Addr.ADDRESS,       //0
        Mms.DELIVERY_REPORT,    //1
        Mms.READ_REPORT,        //2
        Mms.Addr.CHARSET        //3
    };

    static final String[] MMS_REPORT_STATUS_PROJECTION = new String[] {
        Mms.Addr.ADDRESS,       //0
        "delivery_status",      //1
        "read_status"           //2
    };

    static final String[] SMS_REPORT_STATUS_PROJECTION = new String[] {
        Sms.ADDRESS,            //0
        Sms.STATUS,             //1
        Sms.DATE_SENT,          //2
        Sms.TYPE                //3
    };

    // These indices must sync up with the projections above.
    static final int COLUMN_RECIPIENT           = 0;
    static final int COLUMN_DELIVERY_REPORT     = 1;
    static final int COLUMN_READ_REPORT         = 2;
    static final int COLUMN_CHARSET             = 3;
    static final int COLUMN_DELIVERY_STATUS     = 1;
    static final int COLUMN_READ_STATUS         = 2;
    static final int COLUMN_DATE_SENT           = 2;
    static final int COLUMN_MESSAGE_TYPE        = 3;

    private long mMessageId;
    private String mMessageType;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        if (PermissionCheckUtil.requestRequiredPermissions(this)) {
            return;
        }

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.delivery_report_activity);
        Intent intent = getIntent();
        mMessageId = getMessageId(icicle, intent);
        mMessageType = getMessageType(icicle, intent);
        initListView();
        initListAdapter();
    }

    private void initListView() {
        // Add the header for the list view.
        LayoutInflater inflater = getLayoutInflater();
        View header = inflater.inflate(R.layout.delivery_report_header, null);
        getListView().addHeaderView(header, null, true);
    }

    private void initListAdapter() {
        List<DeliveryReportItem> items = getReportItems();
        if (items == null) {
            items = new ArrayList<DeliveryReportItem>(1);
            items.add(new DeliveryReportItem("", getString(R.string.status_none), null));
            Log.w(LOG_TAG, "items == null");
        }
        setListAdapter(new DeliveryReportAdapter(this, items));
    }

    @Override
    public void onResume() {
        super.onResume();
        refreshDeliveryReport();
    }

    private void refreshDeliveryReport() {
        ListView list = getListView();
        list.invalidateViews();
        list.requestFocus();
    }

    private long getMessageId(Bundle icicle, Intent intent) {
        long msgId = 0L;

        if (icicle != null) {
            msgId = icicle.getLong("message_id");
        }

        if (msgId == 0L) {
            msgId = intent.getLongExtra("message_id", 0L);
        }

        return msgId;
    }

    private String getMessageType(Bundle icicle, Intent intent) {
        String msgType = null;

        if (icicle != null) {
            msgType = icicle.getString("message_type");
        }

        if (msgType == null) {
            msgType = intent.getStringExtra("message_type");
        }

        return msgType;
    }

    private List<DeliveryReportItem> getReportItems() {
        if (mMessageType.equals("sms")) {
            return getSmsReportItems();
        } else {
            return getMmsReportItems();
        }
    }

    private List<DeliveryReportItem> getSmsReportItems() {
        String selection = "_id = " + mMessageId;
        Cursor c = SqliteWrapper.query(this, getContentResolver(), Sms.CONTENT_URI,
                    SMS_REPORT_STATUS_PROJECTION, selection, null, null);
        if (c == null) {
            return null;
        }

        try {
            if (c.getCount() <= 0) {
                return null;
            }

            List<DeliveryReportItem> items = new ArrayList<DeliveryReportItem>();
            while (c.moveToNext()) {
                // For sent messages with delivery reports, we stick the delivery time in the
                // date_sent column (see MessageStatusReceiver).
                String deliveryDateString = null;
                long deliveryDate = c.getLong(COLUMN_DATE_SENT);
                int messageType = c.getInt(COLUMN_MESSAGE_TYPE);
                if (messageType == Sms.MESSAGE_TYPE_SENT && deliveryDate > 0) {
                    deliveryDateString = getString(R.string.delivered_label) +
                            MessageUtils.formatTimeStampString(this,
                                    deliveryDate, true);
                }

                items.add(new DeliveryReportItem(
                                /// M: Code analyze 001, fix bug ALPS00116721, It can't
                                // show the name when receive the delivery report. @{
                                getString(R.string.recipient_label)
                                + Contact.get(c.getString(COLUMN_RECIPIENT), false).getName(),
                                /// @}
                                getString(R.string.status_label) +
                                        getSmsStatusText(c.getInt(COLUMN_DELIVERY_STATUS)),
                                        deliveryDateString));
            }
            return items;
        } finally {
            c.close();
        }
    }

    private String getMmsReportStatusText(
            MmsReportRequest request,
            Map<String, MmsReportStatus> reportStatus) {
        if (reportStatus == null) {
            MmsLog.d(LOG_TAG, "getMmsReportStatusText(): reportStatus is null.");
            // haven't received any reports.
            return getString(R.string.status_pending);
        }

        String recipient = request.getRecipient();
        recipient = (Mms.isEmailAddress(recipient)) ?
                Mms.extractAddrSpec(recipient) : PhoneNumberUtils.stripSeparators(recipient);
        MmsReportStatus status = queryStatusByRecipient(reportStatus, recipient);
        if (status == null) {
            // haven't received any reports.
            MmsLog.d(LOG_TAG, "getMmsReportStatusText(): MmsReportStatus is null.");
            return getString(R.string.status_pending);
        }

        if (request.isReadReportRequested()) {
            if (status.readStatus != 0) {
                switch (status.readStatus) {
                    case MtkPduHeaders.READ_STATUS_READ:
                        return getString(R.string.status_read);
                    case MtkPduHeaders.READ_STATUS__DELETED_WITHOUT_BEING_READ:
                        return getString(R.string.status_unread);
                }
            }
        }

        switch (status.deliveryStatus) {
            case 0: // No delivery report received so far.
                MmsLog.d(LOG_TAG, "getMmsReportStatusText(): sms deliveryStatus is 0.");
                return getString(R.string.status_pending);
                /// M: Code analyze 002, new feature, add more status of the
                // delivery report. @{
            case MtkPduHeaders.STATUS_EXPIRED:
                return getString(R.string.status_expired);
            case MtkPduHeaders.STATUS_DEFERRED:
                return getString(R.string.status_deferred);
                /// @}
            case MtkPduHeaders.STATUS_FORWARDED:
            case MtkPduHeaders.STATUS_RETRIEVED:
                return getString(R.string.status_received);
            case MtkPduHeaders.STATUS_REJECTED:
                return getString(R.string.status_rejected);
                /// M: Code analyze 002, new feature, add more status of the
                // delivery report. @{
            case MtkPduHeaders.STATUS_UNRECOGNIZED:
            case MtkPduHeaders.STATUS_INDETERMINATE:
            case MtkPduHeaders.STATUS_UNREACHABLE:
                /// @}
            default:
                return getString(R.string.status_failed);
        }
    }

    private static MmsReportStatus queryStatusByRecipient(
            Map<String, MmsReportStatus> status, String recipient) {
        MmsLog.dpi(LOG_TAG, "queryStatusByRecipient(): status.size = "
                + status.size() + ", recipient = " + recipient);
        Set<String> recipientSet = status.keySet();
        Iterator<String> iterator = recipientSet.iterator();
        while (iterator.hasNext()) {
            String r = iterator.next();
            MmsLog.dpi(LOG_TAG, "queryStatusByRecipient(): has report recipinet = " + r);
            if (Mms.isEmailAddress(recipient)) {
                if (TextUtils.equals(r, recipient)) {
                    MmsLog.d(LOG_TAG, "queryStatusByRecipient(): return email recipinet status");
                    return status.get(r);
                }
            } else if (PhoneNumberUtils.compare(r, recipient)) {
                MmsLog.d(LOG_TAG, "queryStatusByRecipient(): return number recipinet status");
                return status.get(r);
            } else {
                /// M: add for special number comparing like +8869123456789 and 09123456789
                if (!TextUtils.isEmpty(r) && !TextUtils.isEmpty(recipient)) {
                    String specialStr = "0";
                    if ((recipient.startsWith(specialStr) &&
                            r.endsWith(recipient.substring(recipient.indexOf(specialStr)
                                    + specialStr.length())))
                                || (r.startsWith(specialStr) &&
                                        recipient.endsWith(r.substring(r.indexOf(specialStr)
                                                + specialStr.length())))) {
                        MmsLog.d(LOG_TAG, "queryStatusByRecipient():" +
                                " return special number recipinet status");
                        return status.get(r);
                    }
                }
            }
        }
        MmsLog.d(LOG_TAG, "queryStatusByRecipient(): return null");
        return null;
    }

    private List<DeliveryReportItem> getMmsReportItems() {
        List<MmsReportRequest> reportReqs = getMmsReportRequests();
        if (null == reportReqs) {
            return null;
        }

        if (reportReqs.size() == 0) {
            return null;
        }

        Map<String, MmsReportStatus> reportStatus = getMmsReportStatus();
        List<DeliveryReportItem> items = new ArrayList<DeliveryReportItem>();
        for (MmsReportRequest reportReq : reportReqs) {
            String statusText = getString(R.string.status_label) +
                getMmsReportStatusText(reportReq, reportStatus);
            /// M: Code analyze 001, fix bug ALPS00116721, It can't show the
            // name when receive the delivery report. @{
            items.add(new DeliveryReportItem(getString(R.string.recipient_label)
                    + Contact.get(reportReq.getRecipient(), false).getName(), statusText, null));
            /// @}
        }
        return items;
    }

    private Map<String, MmsReportStatus> getMmsReportStatus() {
        Uri uri = Uri.withAppendedPath(Mms.REPORT_STATUS_URI,
                                       String.valueOf(mMessageId));
        Cursor c = SqliteWrapper.query(this, getContentResolver(), uri,
                       MMS_REPORT_STATUS_PROJECTION, null, null, null);

        if (c == null) {
            return null;
        }

        try {
            Map<String, MmsReportStatus> statusMap =
                    new HashMap<String, MmsReportStatus>();

            while (c.moveToNext()) {
                String recipient = c.getString(COLUMN_RECIPIENT);
                recipient = (Mms.isEmailAddress(recipient)) ?
                                        Mms.extractAddrSpec(recipient) :
                                            PhoneNumberUtils.stripSeparators(recipient);
                MmsReportStatus status = new MmsReportStatus(
                                        c.getInt(COLUMN_DELIVERY_STATUS),
                                        c.getInt(COLUMN_READ_STATUS));
                statusMap.put(recipient, status);
            }
            return statusMap;
        } finally {
            c.close();
        }
    }

    private List<MmsReportRequest> getMmsReportRequests() {
        Uri uri = Uri.withAppendedPath(Mms.REPORT_REQUEST_URI,
                                       String.valueOf(mMessageId));
        Cursor c = SqliteWrapper.query(this, getContentResolver(), uri,
                      MMS_REPORT_REQUEST_PROJECTION, null, null, null);

        if (c == null) {
            return null;
        }

        try {
            if (c.getCount() <= 0) {
                return null;
            }

            List<MmsReportRequest> reqList = new ArrayList<MmsReportRequest>();
            while (c.moveToNext()) {
                int charset = c.getInt(COLUMN_CHARSET);
                String addressData = c.getString(COLUMN_RECIPIENT);
                String address = null;
                if (TextUtils.isEmpty(addressData)) {
                    address = "";
                } else if (charset == CharacterSets.ANY_CHARSET) {
                    address = addressData;
                } else {
                    address = new EncodedStringValue(charset,
                            MtkPduPersister.getBytes(addressData)).getString();
                }

                reqList.add(new MmsReportRequest(
                                address,
                                c.getInt(COLUMN_DELIVERY_REPORT),
                                c.getInt(COLUMN_READ_REPORT)));
            }
            return reqList;
        } finally {
            c.close();
        }
    }

    private String getSmsStatusText(int status) {
        MmsLog.d(LOG_TAG, "getSmsStatusText(): status = " + status);
        if (status == Sms.STATUS_NONE) {
            // No delivery report requested
            return getString(R.string.status_none);
        } else if (status >= Sms.STATUS_FAILED) {
            // Failure
            return getString(R.string.status_failed);
        } else if (status >= Sms.STATUS_PENDING) {
            // Pending
            return getString(R.string.status_pending);
        } else {
            // Success
            return getString(R.string.status_received);
        }
    }

    public static final class MmsReportStatus {
        final int deliveryStatus;
        final int readStatus;

        public MmsReportStatus(int drStatus, int rrStatus) {
            deliveryStatus = drStatus;
            readStatus = rrStatus;
        }
    }

    public static final class MmsReportRequest {
        private final String mRecipient;
        private final boolean mIsDeliveryReportRequsted;
        private final boolean mIsReadReportRequested;

        public MmsReportRequest(String recipient, int drValue, int rrValue) {
            mRecipient = recipient;
            mIsDeliveryReportRequsted = drValue == MtkPduHeaders.VALUE_YES;
            mIsReadReportRequested = rrValue == MtkPduHeaders.VALUE_YES;
        }

        public String getRecipient() {
            return mRecipient;
        }

        public boolean isDeliveryReportRequested() {
            return mIsDeliveryReportRequsted;
        }

        public boolean isReadReportRequested() {
            return mIsReadReportRequested;
        }
    }
}
