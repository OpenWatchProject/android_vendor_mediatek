package com.mediatek.rcse.plugin.message;

import java.lang.ref.SoftReference;

import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;

import com.mediatek.mms.ipmessage.DefaultIpMessageItemExt;
import com.mediatek.rcse.api.Logger;
import com.mediatek.mms.callback.IColumnsMapCallback;
import com.mediatek.mms.ipmessage.IIpColumnsMapExt;
import com.mediatek.mms.callback.IMessageItemCallback;

public class RcseMessageItem extends DefaultIpMessageItemExt {
    private static String TAG = "RcseMessageItem";

    static final int COLUMN_SMS_IP_MESSAGE_ID = 22;

    IpMessage mIpMessage = null;
    Context mContext;
    long mIpMessageId;
    long mMsgId;
    int mSubId;
    String mType;
    String mAddress;
    int mBoxId;
    long mSentDate;
    long mDate;

    private SoftReference<Bitmap> mBitmapCache = new SoftReference<Bitmap>(null);
    private int mCacheBitmapWidth;
    private int mCacheBitmapHeight;

    public Bitmap getIpMessageBitmap() {
        return mBitmapCache.get();
    }

    public void setIpMessageBitmapCache(Bitmap bitmap) {
        if (null != bitmap) {
            mBitmapCache = new SoftReference<Bitmap>(bitmap);
        }
    }

    // / M: these 3 methods is matched with setIpMessageBitmapCache.
    public void setIpMessageBitmapSize(int width, int height) {
        mCacheBitmapWidth = width;
        mCacheBitmapHeight = height;
    }

    public int getIpMessageBitmapWidth() {
        return mCacheBitmapWidth;
    }

    public int getIpMessageBitmapHeight() {
        return mCacheBitmapHeight;
    }

    @Override
    public boolean onIpCreateMessageItem(Context context, Cursor cursor, long msgId, String type,
            int subId, IIpColumnsMapExt ipColumnsMap, IMessageItemCallback msgItemCallback,
            boolean isDrawTimeDivider) {
        /// M: add for ipmessage
        mContext = context;
        int ipsgColumIndex = cursor.getColumnIndex("ipmsg_id");
        Logger.v(TAG, "onIpCreateMessageItem ipsgColumIndex! " + ipsgColumIndex);
        if(ipsgColumIndex == -1)
            return false;
        mIpMessageId = cursor.getLong(COLUMN_SMS_IP_MESSAGE_ID);
        mMsgId = msgId;
        mType = type;
        mSubId = subId;
        mAddress = cursor.getString(RcseMessageListAdapter.COLUMN_SMS_ADDRESS);
        int mBoxId;
        mSentDate = cursor.getLong(RcseMessageListAdapter.COLUMN_SMS_DATE_SENT);
        mDate = cursor.getLong(RcseMessageListAdapter.COLUMN_SMS_DATE);
        if (mIpMessageId > 0 && IpMmsConfig.isServiceEnabled(context)) {
            mIpMessage = IpMessageManager.getInstance(context).getIpMsgInfo(mMsgId);
            if (mIpMessage == null) {
                Logger.v(TAG, "MessageItem.init(): ip message is null!");
            }
            return true;
        }
        return false;
    }

    /**
     * add for mms
     */
    public boolean onIpCreateMessageItem(Context context, long msgId, String type,
            int subId, int boxId, String subject,
                        int messageType, String address) {
        mBoxId = boxId;
        return false;
    }


}
