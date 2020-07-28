package com.mediatek.mms.appservice.mwi;

import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.text.format.DateUtils;
import android.text.format.Time;
import android.util.Log;

import com.mediatek.mms.appservice.R;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import mediatek.telephony.MtkTelephony.MtkMwi;

public class MwiMessage {
    private static String TAG = "Mms/Mwi/MwiMessage";

    // Structure of Mwi
    private String mTo = "";
    private String mFrom = "";
    private String mSubject = "";
    private long mDate;
    private String mPriority = "";
    private String mMsgId = "";
    private String mMsgContext = "";
    private boolean mSeen;
    private boolean mGotContent;

    // Account from MwiHeader
    private String mMsgAccount = "";

    private int mId;
    private int mPriorityId;
    private String mPriorityShow = "";      // string to show
    private int mMsgContextId;
    private String mMsgContextShow = "";     // string to show
    private String mTimestamp = "";
    private String mTimeDetail = "";
    private Context mContext;
    private Uri mUri;
    private boolean mRead;
    private boolean mIsChecked;

    public static final int MSG_READ = 1;
    public static final int MSG_UNREAD = 0;
    private static final String TYPE_VOICE_MSG = "voice-message";
    private static final String TYPE_VIDEO_MSG = "video-message";
    private static final String TYPE_FAX_MSG = "fax-message";
    private static final String TYPE_TEXT_MSG = "text-message";
    private static final int TYPE_VOICE_MSG_ID = 1;
    private static final int TYPE_VIDEO_MSG_ID = 2;
    private static final int TYPE_FAX_MSG_ID = 3;
    private static final int TYPE_TEXT_MSG_ID = 4;
    private static final String PRIORITY_URGENT = "urgent";
    private static final String PRIORITY_NORMAL = "normal";
    private static final String PRIORITY_LOW = "low";
    private static final int PRIORITY_URGENT_ID = 1;
    private static final int PRIORITY_NORMAL_ID = 2;
    private static final int PRIORITY_LOW_ID = 3;

    public MwiMessage(Context context) {
        mContext = context;
    }

    public MwiMessage(Context context, String to, String from, String subject, String date,
            String priority, String msgId, String msgContext, String msgAccount,
            boolean seen, boolean gotContent) {
        mContext = context;
        mTo = to;
        mFrom = from;
        mSubject = subject;
        setDate(date);
        setPriority(priority);
        mMsgId = msgId;
        setMsgContext(msgContext);
        mMsgAccount = msgAccount;
        mSeen = seen;
        mGotContent = gotContent;
        mId = -1;
    }

    public MwiMessage(Context context, Cursor cursor) {
        mContext = context;
        mId = cursor.getInt(cursor.getColumnIndex(Columns.Id.getColumnName()));
        mTo = cursor.getString(cursor.getColumnIndex(Columns.To.getColumnName()));
        mFrom = cursor.getString(cursor.getColumnIndex(Columns.From.getColumnName()));
        mSubject = cursor.getString(cursor.getColumnIndex(Columns.Subject.getColumnName()));

        setDate(cursor.getLong(cursor.getColumnIndex(Columns.Date.getColumnName())));
        setPriorityId(cursor.getInt(cursor.getColumnIndex(Columns.Priority.getColumnName())));
        mMsgId = cursor.getString(cursor.getColumnIndex(Columns.MsgId.getColumnName()));
        setMsgContextId(cursor.getInt(cursor.getColumnIndex(Columns.MsgContext.getColumnName())));
        mMsgAccount = cursor.getString(cursor.getColumnIndex(Columns.MsgAccount.getColumnName()));
        mSeen = cursor.getInt(cursor.getColumnIndex(Columns.Seen.getColumnName())) == 1
                ? true : false;
        mRead = cursor.getInt(cursor.getColumnIndex(Columns.Read.getColumnName())) == 1
                ? true : false;
        mGotContent = cursor.getInt(cursor.getColumnIndex(Columns.GotContent.getColumnName())) == 1
                ? true : false;
    }

    public String getTo() {
        return mTo;
    }
    public void setTo(String to) {
        this.mTo = to;
    }
    public String getFrom() {
        return mFrom;
    }
    public void setFrom(String from) {
        this.mFrom = from;
    }
    public String getSubject() {
        return mSubject;
    }
    public void setSubject(String subject) {
        this.mSubject = subject;
    }
    public Long getDate() {
        return mDate;
    }
    public void setDate(String date) {
        //Date format dd MMM yyyy HH:mm:ss Z
        String datePattern =
            "[0-9]{2} [a-zA-Z]{3} [0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2} [\\+\\-][0-9]{4}";
        Pattern pattern = Pattern.compile(datePattern);
        Matcher matcher = pattern.matcher(date);
        if (!matcher.find()) {
            Log.d(TAG, "date String not find " + date);
            Date d = new Date();
            mDate = d.getTime();
            mTimestamp = formatTimeStampString(mContext, mDate);
            mTimeDetail = formatTimeStampString(mContext, mDate, true);
            return;
        }
        date = date.substring(matcher.start(), matcher.end());
        Log.d(TAG, "date String: " + date);
        String dateFormat = "dd MMM yyyy HH:mm:ss Z";
        SimpleDateFormat sdf = new SimpleDateFormat(dateFormat, Locale.US);
        try {
            Date d = sdf.parse(date);
            // local time zone
            Calendar nowCal = Calendar.getInstance();
            TimeZone localZone = nowCal.getTimeZone();
            // set time zone
            sdf.setTimeZone(localZone);
            mDate = d.getTime();
            mTimestamp = formatTimeStampString(mContext, mDate);
            mTimeDetail = formatTimeStampString(mContext, mDate, true);
        } catch (ParseException ex) {
            ex.printStackTrace();
        }
    }

    private String formatTimeStampString(Context context, long when) {
        return formatTimeStampString(context, when, false);
    }

    private String formatTimeStampString(Context context, long when, boolean fullFormat) {
        Time then = new Time();
        then.set(when);
        Time now = new Time();
        now.setToNow();

        // Basic settings for formatDateTime() we want for all cases.
        int format_flags = DateUtils.FORMAT_NO_NOON_MIDNIGHT |
        /// M: Fix ALPS00419488 to show 12:00, so mark DateUtils.FORMAT_ABBREV_ALL
                           //DateUtils.FORMAT_ABBREV_ALL |
                           DateUtils.FORMAT_CAP_AMPM;

        // If the message is from a different year, show the date and year.
        if (then.year != now.year) {
            format_flags |= DateUtils.FORMAT_SHOW_YEAR | DateUtils.FORMAT_SHOW_DATE;
        } else if (then.yearDay != now.yearDay) {
            // If it is from a different day than today, show only the date.
            format_flags |= DateUtils.FORMAT_SHOW_DATE;
        } else {
            // Otherwise, if the message is from today, show the time.
            format_flags |= DateUtils.FORMAT_SHOW_TIME;
        }

        // If the caller has asked for full details, make sure to show the date
        // and time no matter what we've determined above (but still make showing
        // the year only happen if it is a different year from today).
        if (fullFormat) {
            format_flags |= (DateUtils.FORMAT_SHOW_DATE | DateUtils.FORMAT_SHOW_TIME);
        }
        /* mark by hongduo
        String dateTime = sOpMessageUtilsExt.formatTimeStampString(context, when, format_flags);
        if (dateTime != null) {
            return dateTime;
        }
        */
        return DateUtils.formatDateTime(context, when, format_flags);
    }

    public void setDate(long date) {
        this.mDate = date;
        mTimestamp = formatTimeStampString(mContext, mDate);
        mTimeDetail = formatTimeStampString(mContext, mDate, true);
    }

    public String getPriority() {
        return mPriority;
    }
    public void setPriority(String priority) {
        this.mPriority = priority;
        if (priority == null) {
            Log.w(TAG, "priority is null");
            return;
        }
        if (priority.equals(PRIORITY_URGENT)) {
            mPriorityId = PRIORITY_URGENT_ID;
            mPriorityShow = mContext.getString(R.string.priority_urgent);
        } else if (priority.equals(PRIORITY_NORMAL)) {
            mPriorityId = PRIORITY_NORMAL_ID;
            mPriorityShow = mContext.getString(R.string.priority_normal);
        } else if (priority.equals(PRIORITY_LOW)) {
            mPriorityId = PRIORITY_LOW_ID;
            mPriorityShow = mContext.getString(R.string.priority_low);
        }
    }
    public String getMsgId() {
        return mMsgId;
    }
    public void setMsgId(String msgId) {
        this.mMsgId = msgId;
    }
    public String getMsgContext() {
        return mMsgContext;
    }
    public void setMsgContext(String msgContext) {
        this.mMsgContext = msgContext;
        if (mMsgContext == null) {
            Log.w(TAG, "MsgContext is null");
        }
        if (mMsgContext.equals(TYPE_VOICE_MSG)) {
            mMsgContextId = TYPE_VOICE_MSG_ID;
            mMsgContextShow = mContext.getString(R.string.type_voice_msg);
        } else if (mMsgContext.equals(TYPE_VIDEO_MSG)) {
            mMsgContextId = TYPE_VIDEO_MSG_ID;
            mMsgContextShow = mContext.getString(R.string.type_video_msg);
        } else if (mMsgContext.equals(TYPE_FAX_MSG)) {
            mMsgContextId = TYPE_FAX_MSG_ID;
            mMsgContextShow = mContext.getString(R.string.type_fax_msg);
        } else if (mMsgContext.equals(TYPE_TEXT_MSG)) {
            mMsgContextId = TYPE_TEXT_MSG_ID;
            mMsgContextShow = mContext.getString(R.string.type_text_msg);
        }
    }

    public boolean getSeen() {
        return mSeen;
    }

    public void setSeen(boolean seen) {
        this.mSeen = seen;
    }

    public boolean getGotContent() {
        return mGotContent;
    }

    public void setGotContent(boolean gotContent) {
        this.mGotContent = gotContent;
    }

    public String getMsgAccount() {
        return mMsgAccount;
    }
    public void setMsgAccount(String msgAccount) {
        this.mMsgAccount = msgAccount;
    }

    public String getTimestamp() {
        return mTimestamp;
    }

    public String getTimeDetail() {
        return mTimeDetail;
    }

    public int getPriorityId() {
        return mPriorityId;
    }

    public int getMsgContextId() {
        return mMsgContextId;
    }

    public String getPriorityShow() {
        return mPriorityShow;
    }

    public String getMsgContextShow() {
        return mMsgContextShow;
    }

    public boolean isRead() {
        return mRead;
    }

    public void setRead(boolean read) {
        this.mRead = read;
    }

    public Uri getUri() {
        return mUri;
    }

    public void setUri(Uri uri) {
        this.mUri = uri;
    }

    public boolean isChecked() {
        return mIsChecked;
    }

    public void setChecked(boolean checked) {
        this.mIsChecked = checked;
    }

    public void setPriorityId(int PriorityId) {
        this.mPriorityId = PriorityId;
        if (mPriorityId == PRIORITY_URGENT_ID) {
            mPriority = PRIORITY_URGENT;
            mPriorityShow = mContext.getString(R.string.priority_urgent);
        } else if (mPriorityId == PRIORITY_NORMAL_ID) {
            mPriority = PRIORITY_NORMAL;
            mPriorityShow = mContext.getString(R.string.priority_normal);
        } else if (mPriorityId == PRIORITY_LOW_ID) {
            mPriority = PRIORITY_LOW;
            mPriorityShow = mContext.getString(R.string.priority_low);
        }
    }

    public void setMsgContextId(int msgContextId) {
        this.mMsgContextId = msgContextId;
        if (mMsgContextId == TYPE_VOICE_MSG_ID) {
            mMsgContext = TYPE_VOICE_MSG;
            mMsgContextShow = mContext.getString(R.string.type_voice_msg);
        } else if (mMsgContextId == TYPE_VIDEO_MSG_ID) {
            mMsgContext = TYPE_VIDEO_MSG;
            mMsgContextShow = mContext.getString(R.string.type_video_msg);
        } else if (mMsgContextId == TYPE_FAX_MSG_ID) {
            mMsgContext = TYPE_FAX_MSG;
            mMsgContextShow = mContext.getString(R.string.type_fax_msg);
        } if (mMsgContextId == TYPE_TEXT_MSG_ID) {
            mMsgContext = TYPE_TEXT_MSG;
            mMsgContextShow = mContext.getString(R.string.type_text_msg);
        }
    }

    public void markAsRead() {
        final Uri mwiMsgUri = ContentUris.withAppendedId(MtkMwi.CONTENT_URI, mId);
        new Thread(new Runnable() {
            public void run() {
                if (mwiMsgUri != null) {
                    buildReadContentValues();

                    /**
                     * M: Check the read flag first. It's much faster to do a
                     * query than to do an update. Timing this function show
                     * it's about 10x faster to do the query compared to the
                     * update, even when there's nothing to update.
                     */
                    boolean needUpdate = true;

                    Cursor c = mContext.getContentResolver().query(mwiMsgUri, UNREAD_PROJECTION,
                            UNREAD_SELECTION, null, null);
                    if (c != null) {
                        try {
                            needUpdate = c.getCount() > 0;
                        } finally {
                            c.close();
                        }
                    }

                    if (needUpdate) {
                        mContext.getContentResolver().update(mwiMsgUri, sReadContentValues,
                                UNREAD_SELECTION, null);
                        mSeen = false;
                    }
                }
            }
        }).start();
    }

    private static final String[] UNREAD_PROJECTION = new String[]{MtkMwi.READ};
    private static ContentValues sReadContentValues;
    private static final String UNREAD_SELECTION = "(read=0)";
    private void buildReadContentValues() {
        if (sReadContentValues == null) {
            sReadContentValues = new ContentValues(2);
            sReadContentValues.put("seen", 1);
            sReadContentValues.put("read", 1);
        }
    }
    /*
     * Label in data which is in content of intent from framework.
     */
    public enum Label {
        To("To"), From("From"), Subject("Subject"), Date("Date"), Priority("Priority"),
        MsgId("Message-ID"), MsgContext("Message-Context"), MsgAccount("Msg-Account");

        private String mLabel;
        private Label(String label) {
            mLabel = label;
        }
        public String getLabel() {
            return mLabel;
        }
    }

    /*
     * Columns in database.
     */
    public enum Columns {
        Id("_id"), MsgAccount("msg_account"), To("to_account"),
        From("from_account"), Subject("subject"),
        Date("msg_date"), Priority("priority"), MsgId("msg_id"), MsgContext("msg_context"),
        Seen("seen"), Read("read"), GotContent("got_content");

        private String mColumn;
        private Columns(String column) {
            mColumn = column;
        }
        public String getColumnName() {
            return mColumn;
        }
    }
}
