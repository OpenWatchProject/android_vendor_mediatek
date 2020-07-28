package com.android.mms.draft;

import android.content.Context;
import android.net.Uri;
import android.util.Log;

import com.android.mms.model.SlideshowModel;
import com.android.mms.util.MmsLog;

import com.mediatek.android.mms.pdu.MtkSendReq;


/*******************************************************************************
 *
 * Class : TaskReq
 * Base class which implement basic info for different req
 *
*******************************************************************************/
public class TaskReq {

    private static final String TAG = "[Mms][Draft][TaskReq]";

    protected /*Handler*/IDraftInterface mHandler;

    protected int mHandlerWhat;

    /// the object which used synchronized when doing sync action(save | load | update) @{
    protected Object mSyncObject;
    /// @}

    protected int mNotiCount = 0;

    /// the save | load | update result should be store in the object @{
    protected MmsDraftData mResult;
    /// @}

    protected SlideshowModel mSlideshowModel;

    protected Uri mMessageUri;

    protected Context mContext;

    protected long mThreadId;

    /**
    * A factory which used to create SaveReq or LoadReq or UpdateReq
    */
    public static TaskReq createReq(int type, long threadId, Uri uri,
            SlideshowModel slideshow, MtkSendReq sendReq,
            Context context, IDraftInterface callback/*Handler handler*/) {
        Log.d(TAG, "[createReq] type : " + type + ", threadId : " + threadId);
        TaskReq tr = null;
        switch (type) {
            case DraftManager.ASYNC_SAVE_ACTION:
            case DraftManager.SYNC_SAVE_ACTION:
                MmsLog.dpi(TAG, "[createReq] save uri : " + uri);
                tr = new SaveReq(type, threadId, uri, slideshow, sendReq, context, callback);
                break;

            case DraftManager.ASYNC_LOAD_ACTION:
                if (threadId <= 0) {
                    Log.d(TAG, "[createReq] request is async load action, threadid <= 0");
                    break;
                }
            case DraftManager.SYNC_LOAD_ACTION:
                tr = new LoadReq(type, threadId, uri, context, callback);
                break;

            case DraftManager.SYNC_UPDATE_ACTION:
                tr = new UpdateReq(type, threadId, uri, context, slideshow);
                break;

            default:
                Log.d(TAG, "Constructor unKnown type to create Req");
                break;
        }
        return tr;
    }

    /**
    * return the type.
    * The method should be implement in the sub-class
    */
    public int getType() {
        return 0;
    }

    public MmsDraftData getResult() {
        return mResult;
    }

    public long getThreadId() {
        return mThreadId;
    }

    public void setMessageUri(Uri uri) {
        MmsLog.dpi(TAG, "set message uri: " + uri);
        mMessageUri = uri;
    }

    public Uri getMessageUri() {
        return mMessageUri;
    }

    public void waitExecute() {
        if (mSyncObject != null) {
            MmsLog.dpi(TAG, "[notifyFinished] enter and to wait, this " + this);
            try {
                synchronized (mSyncObject) {
                    if (mNotiCount > 0) {
                        Log.d(TAG, "[waitExecute] mNotiCount " + mNotiCount);
                        mNotiCount = 0;
                        return;
                    }
                    mSyncObject.wait();
                    mNotiCount = 0;
                }
            } catch (InterruptedException ex) {
                MmsLog.epi(TAG, "[waitExecute] InterruptedException happened while wait object");
            }
        }
    }

    public void notifyFinished() {
        if (mSyncObject != null) {
            MmsLog.dpi(TAG, "[notifyFinished] enter and to notify, this " + this);
            synchronized (mSyncObject) {
                mNotiCount ++;
                mSyncObject.notify();
            }
        }
    }

    /**
     * execute Save or load request according type
     */
    public void executeReq() {
        return;
    }

    public IDraftInterface getHandler() {
        return null;
    }

    public int getWhat() {
        return 0;
    }
}
