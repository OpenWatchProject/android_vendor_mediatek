package com.android.mms.draft;

import android.util.Log;

import java.util.Vector;

/*******************************************************************************
 *
 * Class DraftTask :
 *      Used to execute different thread id save load update request
 *
 *
 ********************************************************************************/

public class DraftTask {

    private static final String TAG = "[Mms][Draft][DraftTask]";

    // the latest save req which used to return the latest slideshowmodel
    // subject and recipients
    private SaveReq mLatestSaveReq;

    // save & load request list
    private Vector<TaskReq> mReqList;

    // The thread to execute the request
    private TaskThread mThread;

    // the last request type
    private int mLastReqType;

    private boolean mStarted = false;

    private int mCurrentRunningType = 0;

    private long mClearThreadId = 0;

    /**
     * Constructor to init req list and thread
     */
    public DraftTask() {
        mReqList = new Vector<TaskReq>();
        mThread = new TaskThread();
    }

    /**
     * method : preLoad
     * if the last req is save action, and current req is load action, and last
     * save action result is not null, then just return the last save action result
     * @param type current type
     * @return if the last save action result is ok, return true
     *         then return false
     */
    //S+L
    public boolean preLoad(int type) {
        // S+L
        Log.d(TAG, "[preLoad] S+L, mLastReqType = " + mLastReqType + ", and type  = " + type);
        if (mLastReqType == DraftManager.ASYNC_SAVE_ACTION) {
            if (type == DraftManager.ASYNC_LOAD_ACTION) {
                if (mLatestSaveReq != null) {
                    MmsDraftData result = mLatestSaveReq.getResult();
                    if (result != null) {
                        Log.d(TAG, "[preLoad] S+L, just return the last save req result");
                        return false;
                    } else {
                        Log.d(TAG, "[preLoad] S+L, result which from mLatestSaveReq is null");
                    }
                } else {
                    Log.d(TAG, "[preLoad] S+L, mLatestSaveReq is null");
                }
            }
        }
        return false;
    }

    /**
     * Method : getPreLoadResult
     * return the last save action result
     * @return the last save action result
     */
    public MmsDraftData getPreLoadResult() {
        if (mLatestSaveReq != null) {
            MmsDraftData result = mLatestSaveReq.getResult();
            if (result != null) {
                return result;
            } else {
                Log.d(TAG, "[getPreLoadResult] result is null return null");
            }
        }
        return null;
    }

    /**
     * method : addReq
     * @param req save(async)
     */
    @SuppressWarnings("unchecked")
    public void addReq(TaskReq req) {
        if (mReqList == null) {
            mReqList = new Vector<TaskReq>();
        }
        if (req == null) {
            Log.d(TAG, "[addReq] Request is null");
            return;
        }
        mReqList.add(req);
        mClearThreadId = req.getThreadId();
        if (req.getType() == DraftManager.ASYNC_SAVE_ACTION) {
            mLatestSaveReq = (SaveReq) req;
            mLastReqType = DraftManager.ASYNC_SAVE_ACTION;
            Log.d(TAG, "[addReq] type is ASYNC_SAVE_ACTION, save the result");
        }

        if (mReqList.size() == 1) {
            execute();
        }
    }

    /**
     * remove request from mReqList
     */
    public void removeReq(TaskReq req) {
        if (mReqList == null || req == null) {
            Log.d(TAG, "[removeReq] mReqList or parameter req is null, check them");
            return;
        }

    }

    /**
     * check the async task is running or not.
     */
    public boolean isExcuting() {
        return mStarted;
    }

    /**
     * ask async task to execute
     */
    private void execute() {
        if (mThread == null) {
            mThread = new TaskThread();
            mStarted = true;
            mThread.start();
            Log.d(TAG, "execute new and start a thread to run the req");
        }
        if (!mStarted) {
            mStarted = true;
            mThread.start();
        }
    }

    /**
     * method : sendResult
     *      send execute result back to caller which through handler
     *      used to send result when task req is async req
     * @param handler the handler to send message back to caller
     * @param what to get the type
     * @param data the result to send back
     */
    private void sendResult(IDraftInterface handler, int what, MmsDraftData data) {

        if (data != null) {
            Log.d(TAG, "[sendResult] result uri : " + data.getMessageUri());
        }
        Log.d(TAG, "[sendResult] what : " + what);
        if (handler != null) {
            if (what == DraftManager.ASYNC_LOAD_ACTION) {
                if (data != null) {
                    handler.loadFinished(data);
                }
            } else if (what == DraftManager.ASYNC_SAVE_ACTION) {
                if (data != null) {
                    handler.updateAfterSaveDraftFinished(data.getMessageUri(),
                            data.getCreateOrUpdate(), data.getBooleanResult());
                }
            }
        }
    }

    /**
     * method : sendResult
     *      used to notify execute finished
     *      used to send result when task req is sync req
     * @param tr the task req to notify
     */
    private void sendResult(TaskReq tr) {
        Log.d(TAG, "[sendResult] notify to get result");
        tr.notifyFinished();
    }

    /**
     * method : getNextReq
     *      used to get next req to execute
     *      if the mReqList size is 1, just return the first one
     *      if not, foreach the mReqList circle.
     *          if current req is save ,get next req, and check the next req is save
     *          just execute the last save req
     * @return task req
     */
    private TaskReq getNextReq() {
        if (mReqList != null) {
            if (mReqList.size() == 1) {
                return mReqList.get(0);
            } else {
                for (int index = 0; index < mReqList.size(); index++) {
                    int j = index;
                    TaskReq tr = mReqList.get(index);
                    if (tr.getType() == DraftManager.ASYNC_SAVE_ACTION) {
                        if ((++j) < mReqList.size()
                                && mReqList.get(j).getType() == DraftManager.ASYNC_SAVE_ACTION) {
                            continue;
                        }
                    }
                    return tr;
                }
            }
        }
        return null;
    }

    /**
     * if mReqList is empty, means every request has been executed.
     * which can be removed from TaskMap in DraftManager
     * @return if mReqList isEmpty, return true, else return false
     *         if mReqList == null, return true, means error, no need to stay in TaskMap
     */
    public boolean isCleared() {
        if (mReqList != null) {
            Log.d(TAG, "[isCleared] ReqList isEmpty : " + mReqList.isEmpty());
            return mReqList.isEmpty();
        }
        return true;
    }

    /**
     * The thread to execute the req according getNextReq
     *
     */
    private class TaskThread extends Thread {
        public TaskThread() {
            this.setName("DraftTask.executeReqThread");
        }

        public void run() {
            boolean tobeStopped = false;
            while (!tobeStopped) {
                TaskReq tr = getNextReq();
                if (tr == null) {
                    mCurrentRunningType = 0;
                    break;
                }
                mCurrentRunningType = tr.getType();
                tr.executeReq();
                updateNextReqUriIfSave();

                int type = tr.getType();
                Log.d(TAG, "[run] type : " + type);
                if (type == DraftManager.ASYNC_SAVE_ACTION
                        || type == DraftManager.ASYNC_LOAD_ACTION) {
                    IDraftInterface handler = tr.getHandler();
                    sendResult(handler, tr.getWhat(), tr.getResult());
                    Log.d(TAG, "[run] async save | load execute, return result");
                    if (type == DraftManager.ASYNC_SAVE_ACTION) {
                        Log.d(TAG, "[run] current action is save, and save the action result");
                        mLatestSaveReq = (SaveReq) tr;
                    }
                } else if (type == DraftManager.SYNC_SAVE_ACTION
                        || type == DraftManager.SYNC_LOAD_ACTION
                        || type == DraftManager.SYNC_UPDATE_ACTION) {
                    sendResult(tr);
                    Log.d(TAG, "[run] sync save | load | update execute, return result");
                } else {
                    Log.d(TAG, "[run] unknown type to execute");
                }
                mLastReqType = type;
                mReqList.remove(tr);
                if (mReqList.isEmpty()) {
                    mThread = null;
                    tobeStopped = true;
                    mCurrentRunningType = 0;
                    mStarted = false;
                    Log.d(TAG, "[run] the reqlist is empty and break the while true circle");
                }
            }
            Log.d(TAG, "[run] the while circle is finised");
        }
    }

    /*
     * If two save reqs are nearby in list and their uri are inited null,
     * the second req uri should be updated after inited.
     */
    private void updateNextReqUriIfSave() {
        Log.d(TAG, "updateNextReqUriIfSave()");
        if (mReqList == null || mReqList.size() < 2) return;
        TaskReq curReq = mReqList.get(0);
        TaskReq nextReq = mReqList.get(1);

        if ((curReq.getType() != DraftManager.SYNC_SAVE_ACTION
                && curReq.getType() != DraftManager.ASYNC_SAVE_ACTION)
                || (nextReq.getType() != DraftManager.SYNC_SAVE_ACTION
                && nextReq.getType() != DraftManager.ASYNC_SAVE_ACTION)
                || curReq.getMessageUri() == null
                || nextReq.getMessageUri() != null) {
            return;
        }

        nextReq.setMessageUri(curReq.getMessageUri());
    }
}
