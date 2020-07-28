package com.mediatek.mms.ext;

import android.content.Context;
import android.content.ContextWrapper;
import android.database.Cursor;
import android.util.Log;

import com.mediatek.mms.callback.IColumnsMapCallback;

import java.util.HashMap;
import java.util.Map;

public class DefaultOpMessageListAdapterExt extends ContextWrapper implements
        IOpMessageListAdapterExt {

    private static final String TAG = "Mms/DefaultOpMessageListAdapterExt";
    //add for multi-forward
    private Map<Long, BodyandAddress> mBodyandAddressItem;
    class BodyandAddress {
        String mAddress;
        String mBody;
        int mBoxType;
        int mIpMsgId;
        public BodyandAddress(String mAddress, String mBody, int boxType, int mIpMsgId) {
            super();
            this.mAddress = mAddress;
            this.mBody = mBody;
            this.mBoxType = boxType;
            this.mIpMsgId = mIpMsgId;
        }
    }

    public DefaultOpMessageListAdapterExt(Context base) {
        super(base);
        initBodyandAddress();
    }

    @Override
    public void initListMap(Cursor cursor, IColumnsMapCallback columnsMap) {
        setBodyandAddress(cursor, columnsMap);
    }

    @Override
    public void clearList() {
        clearBodyandAddressList();
    }

    public void setBodyandAddress(Cursor cursor, IColumnsMapCallback columnsMap) {
        try {
            if (!cursor.getString(columnsMap.getColumnMsgType()).equals("mms")) {
                long msgId = cursor.getLong(columnsMap.getColumnMsgId());
                String address = cursor.getString(columnsMap.getColumnSmsAddress());
                String body    = cursor.getString(columnsMap.getColumnSmsBody());
                int boxType = cursor.getInt(columnsMap.getColumnSmsType());
                int ipMsgId = cursor.getInt(columnsMap.getColumnSmsIpMessageId());
//                Log.d(TAG, "initListMap mAddress = " + address + "mBody" + body +
//                    ", boxid = " + boxType + ", ipMsgId = " + ipMsgId);
                BodyandAddress  ba = new BodyandAddress(address, body, boxType, ipMsgId);
                mBodyandAddressItem.put(msgId, ba);
            }
        } catch (Exception e) {
            Log.e(TAG, "initListMap error", e);
        }
    }

    public void setForwardMenuEnabled(boolean enabled) {
        return;
    }

    public void clearBodyandAddressList() {
        if (mBodyandAddressItem != null) {
            mBodyandAddressItem.clear();
        }
    }

    public void initBodyandAddress() {
        mBodyandAddressItem = new HashMap<Long, BodyandAddress>();
    }

    public  String getBody(long id) {
        if (mBodyandAddressItem.size() > 0 && mBodyandAddressItem.get(id) != null) {
            return mBodyandAddressItem.get(id).mBody;
        } else {
            return null;
        }
    }

    public  String getAddress(long id) {
        if (mBodyandAddressItem.size() > 0 && mBodyandAddressItem.get(id) != null) {
            return mBodyandAddressItem.get(id).mAddress;
        } else {
            return null;
        }
    }

    public int getBoxType(long id) {
        if (mBodyandAddressItem.size() > 0 && mBodyandAddressItem.get(id) != null) {
            return mBodyandAddressItem.get(id).mBoxType;
        } else {
            return -1;
        }
    }
}
