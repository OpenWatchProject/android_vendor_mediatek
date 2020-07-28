package com.android.mms.draft;

import android.content.Context;
import android.net.Uri;
import android.util.Log;

import com.mediatek.opmsg.util.OpMessageUtils;

public class LoadReq extends TaskReq {

    private static final String TAG = "[Mms][Draft][LoadReq]";

    /**
    * Constructor
    */
    public LoadReq(int type, long threadId, Uri uri, Context context,
            /*Handler handler*/IDraftInterface callback) {
        Log.d(TAG, "[LoadReq] the threadId is :" + threadId);
        if (type == DraftManager.ASYNC_LOAD_ACTION && threadId <= 0) {
            Log.d(TAG, "[LoadReq] req is async load action ,but thread id <= 0, return");
            return;
        }
        mThreadId = threadId;
        mMessageUri = uri;
        mContext = context;
        mHandler = callback;
        mHandlerWhat = type;
        if (type == DraftManager.SYNC_LOAD_ACTION) {
            mSyncObject = new Object();
        }
    }

    /**
    * return the load request type
    */
    public int getType() {
        return mHandlerWhat;
    }

    public IDraftInterface getHandler() {
        return mHandler;
    }

    public int getWhat() {
        return mHandlerWhat;
    }

    /**
    * execute the load request
    */
    public void executeReq() {
        MmsDraftData returnValue = new MmsDraftData();
        DraftAction da = new DraftAction();
        if (mHandlerWhat == DraftManager.SYNC_LOAD_ACTION) {
            if (mMessageUri == null) {
                Log.d(TAG, "[LoadReq.executeReq] Sync load mMessageUri is null");
                returnValue.setBooleanResult(false);
                mResult = returnValue;
                return;
            }
            returnValue.setMessageUri(mMessageUri);
            boolean res = da.loadFromUri(mContext, mMessageUri);
            returnValue.setBooleanResult(res);
            returnValue.setSlideshow(da.getSlideshow());
            Log.d(TAG, "[LoadReq.executeReq] Sync load return result : " + res);
        } else if (mHandlerWhat == DraftManager.ASYNC_LOAD_ACTION) {
            StringBuilder sb = new StringBuilder();
            Uri msgUri = da.readDraftMmsMessage(mContext, mThreadId, sb);
            if (sb != null && sb.toString().length() != 0) {
                Log.d(TAG, "[LoadReq.executeReq] draft subject is : " + sb.toString());
                returnValue.setSubject(sb.toString());
            }
            if (msgUri != null) {
                Log.d(TAG, "[LoadReq.executeReq] msgUri : " + msgUri);

                if (da.loadFromUri(mContext, msgUri)) {
                    Log.d(TAG, "[LoadReq.executeReq] load from uri finished");
                    returnValue.setBooleanResult(true);
                    returnValue.setMessageUri(msgUri);
                    returnValue.setSlideshow(da.getSlideshow());
                } else {
                    returnValue.setBooleanResult(false);
                    Log.d(TAG, "[LoadReq.executeReq] load from uri failed");
                    mResult = returnValue;
                    return;
                }
            } else {
                Log.d(TAG, "[LoadReq.executeReq] The MessageUri is null, cannot load");
                returnValue.setBooleanResult(false);
                mResult = returnValue;
                return;
            }
        } else {
            returnValue.setBooleanResult(false);
            mResult = returnValue;
            Log.d(TAG, "[LoadReq.executeReq] unknown load type");
            return;
        }

        mResult = returnValue;
    }
}
