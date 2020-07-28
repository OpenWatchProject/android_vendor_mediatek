package com.android.mms.draft;

import android.content.Context;
import android.net.Uri;
import android.provider.Telephony.Mms;
import android.util.Log;

import com.android.mms.model.SlideshowModel;
import com.mediatek.android.mms.pdu.MtkSendReq;

import java.io.InputStream;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

/*******************************************************************************
 *
 * Class : DraftManager which is single tone class
 *
 * used to provide interface to save | load | update
 *
 * @author mtk54453
 *
 *******************************************************************************/


public class DraftManager {

    private static final String TAG = "[Mms][Draft][DraftManager]";

    public static final int ASYNC_SAVE_ACTION = 0x01;

    public static final int SYNC_SAVE_ACTION = 0x10;

    public static final int ASYNC_LOAD_ACTION = 0x02;

    public static final int SYNC_LOAD_ACTION = 0x20;

    public static final int SYNC_UPDATE_ACTION = 0x30;

    private ConcurrentMap<Long, DraftTask> mTaskMap;

    private static final long FAKE_THREAD_ID = 0;

    private static DraftManager sInstance;

    public static final HashSet<Long> sEditingThread = new HashSet<Long>();

    /**
     * private Constructor
     */
    private DraftManager() {
        mTaskMap = new ConcurrentHashMap<Long, DraftTask>();
    }

    /**
     * the single tone class to get the single instance
     * @return
     */
    public static DraftManager getInstance() {
        if (sInstance == null) {
            sInstance = new DraftManager();
        }
        return sInstance;
    }

    /**
     * method : update
     *      used to update parts which will be called in slide editor or slide show edit activity
     *
     *      The uri must be starts with Mms.Draft.CONTENT_URI(content://mms-sms/drafts)
     *
     * @param type SYNC_UPDATE_ACTION
     * @param threadId thread id which used to fill into mTaskMap to diff the different threadid req
     * @param context the context which used to getPdupersister
     * @param uri the parts which will be save into the uri
     * @param slideshow slideshow which contains the PduBody
     * @param preOpenedFiles maybe always null
     */
    public void update(int type, long threadId, Context context, Uri uri, SlideshowModel slideshow,
                HashMap<Uri, InputStream> preOpenedFiles) {
        if (context == null || uri == null || slideshow == null) {
            Log.d(TAG, "[update] The parameters maybe is not satisfied the request");
            return;
        }
        if (uri == null || !(uri.toString().startsWith(Mms.Draft.CONTENT_URI.toString()))) {
            Log.d(TAG, "[update] uri is null or The uri is not start with "
                    + Mms.Draft.CONTENT_URI.toString());
            return;
        }
        if (type != SYNC_UPDATE_ACTION) {
            Log.d(TAG, "[update] the request type is not update request");
            return;
        }

        DraftTask dt = getDraftTask(type, threadId);

        if (dt != null) {
            TaskReq tr = TaskReq.createReq(type, threadId, uri, slideshow, null, context, null);
            if (tr != null) {
                dt.addReq(tr);
                Log.d("slideshow+++", "[update] begin to execute and wait here");
                tr.waitExecute();
                Log.d("slideshow+++", "[update] execute finished, and just return");
                return;
            }
        }
        return;
    }

    /**
     *
     * @param type
     * @param threadId
     * @param uri
     * @param slideshow
     * @param sendReq
     * @param context
     * @param handler
     * @return
     */
    public MmsDraftData saveDraft(int type, long threadId, Uri uri,
                SlideshowModel slideshow, MtkSendReq sendReq, Context context,
                /*Handler handler*/IDraftInterface callback) {
        if (slideshow == null || sendReq == null || context == null) {
            Log.d(TAG, "[saveDraft] The parameters maybe is not satisfied the request");
            return null;
        }
        if (type != ASYNC_SAVE_ACTION && type != SYNC_SAVE_ACTION) {
            Log.d(TAG, "[saveDraft] the request is not save request");
            return null;
        }
        Log.d(TAG, "[saveDraft] begin, threadid : " + threadId);

        DraftTask dt = getDraftTask(type, threadId);

        if (dt != null) {
            TaskReq tr = TaskReq.createReq(type, threadId, uri,
                    slideshow, sendReq, context, callback);
            if (tr != null) {
                dt.addReq(tr);
                if (type == SYNC_SAVE_ACTION) {
                    Log.d(TAG, "[saveDraft] do SYNC_SAVE_ACTION");
                    tr.waitExecute();
                    Log.d(TAG, "[saveDraft] return from waitExecute");
                    return tr.getResult();
                } else if (type == ASYNC_SAVE_ACTION) {
                    Log.d(TAG, "[saveDraft] do ASYNC_SAVE_ACTION");
                    return null;
                }
            }
        }
        return null;
    }

    /**
     *
     * @param type
     * @param threadId
     * @param uri
     * @param context
     * @param handler
     * @return
     */
    public MmsDraftData loadDraft(int type, long threadId, Uri uri,
            Context context, IDraftInterface callback/*Handler handler*/) {
        if (type == ASYNC_LOAD_ACTION && threadId <= 0) {
            Log.d(TAG, "[loadDraft] req is async load action and threadId <= 0, just return");
            return null;
        }
        if (context == null) {
            Log.d(TAG, "[loadDraft] context is null");
            return null;
        }
        if (type != ASYNC_LOAD_ACTION && type != SYNC_LOAD_ACTION) {
            Log.d(TAG, "[loadDraft] the type is not load request");
            return null;
        }
        /// Sync load action, if the uri is null or the uri is not start with content:
        /// mms-sms/drafts sync load must has uri, async load can read uri from
        /// readDraftMmsMessage return null @{
        if (type == SYNC_LOAD_ACTION) {
            if (uri == null || !(uri.toString().startsWith(Mms.Draft.CONTENT_URI.toString()))) {
                Log.d(TAG,"[loadDraft] load uri is null or" +
                        " uri is not starts wicht Mms.Draft.CONTENT_URI");
                return null;
            }
        }
        /// @}

        DraftTask dt = getDraftTask(type, threadId);

        if (dt != null) {
            if (dt.preLoad(type)) {
                Log.d(TAG, "[loadDraft] preLoad return true, and return last save req result");
                return dt.getPreLoadResult();
            }
            TaskReq tr = TaskReq.createReq(type, threadId, uri, null, null, context, callback);
            if (tr != null) {
                dt.addReq(tr);
                if (type == SYNC_LOAD_ACTION) {
                    Log.d(TAG, "[loadDraft] do SYNC_LOAD_ACTION");
                    tr.waitExecute();
                    return tr.getResult();
                } else if (type == ASYNC_LOAD_ACTION) {
                    Log.d(TAG, "[loadDraft] do ASYNC_LOAD_ACTION");
                    return null;
                } else {
                    Log.d(TAG, "[loadDraft] unknow load req action");
                    return null;
                }
            }
        }
        return null;
    }

    /**
     *
     * @param type
     * @param threadId
     * @return
     */
    private DraftTask getDraftTask(int type, long threadId) {
        Log.d(TAG, "[getDraftTask] type : " + type + ", threadId : " + threadId);
        if (mTaskMap == null) {
            mTaskMap = new ConcurrentHashMap<Long, DraftTask>();
        }

        removeTask();

        DraftTask dtRe = null;
        if (type == ASYNC_LOAD_ACTION) {
            if (threadId <= 0) {
                Log.d(TAG, "[getDraftTask] cannot do async load action when threadId <= 0");
                return null;
            } else {
                dtRe = mTaskMap.get(threadId);
                if (dtRe == null) {
                    Log.d(TAG, "[getDraftTask] DraftTask" +
                            " is not exist and new one to put into hashmap");
                    dtRe = new DraftTask();
                    mTaskMap.put(threadId, dtRe);
                }
            }
        } else if (type == SYNC_SAVE_ACTION
                    || type == ASYNC_SAVE_ACTION
                    || type == SYNC_UPDATE_ACTION
                    || type == SYNC_LOAD_ACTION) {
            if (threadId <= 0) {
                Log.d(TAG, "[getDraftTask] threadId <= 0 and use 0 to be the id");
                dtRe = mTaskMap.get(FAKE_THREAD_ID);
                if (dtRe == null) {
                    dtRe = new DraftTask();
                    mTaskMap.put(FAKE_THREAD_ID, dtRe);
                }
            } else {
                dtRe = mTaskMap.get(threadId);
                if (dtRe == null) {
                    Log.d(TAG, "[getDraftTask] DraftTask is null and create a new one");
                    dtRe = new DraftTask();
                    mTaskMap.put(threadId, dtRe);
                }
            }
        } else {
            Log.d(TAG, "[getDraftTask] unknow type to create DraftTask");
            return null;
        }
        return dtRe;
    }

    /**
     * which used to remove task in the HashMap
     */
    private void removeTask() {
        if (mTaskMap != null && !mTaskMap.isEmpty()) {
            Iterator iter = mTaskMap.entrySet().iterator();
            while (iter.hasNext()) {
                Map.Entry<Long, DraftTask> entry = (Map.Entry<Long, DraftTask>) iter.next();
                if (entry.getValue().isCleared()) {
                    Log.d(TAG, "[removeTask] remove task : " + entry.getKey() + " in task map");
                    iter.remove();
                }
            }
        }
    }

}
