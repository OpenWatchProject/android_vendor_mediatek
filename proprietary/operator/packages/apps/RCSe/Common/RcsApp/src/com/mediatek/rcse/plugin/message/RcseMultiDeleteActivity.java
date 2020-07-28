package com.mediatek.rcse.plugin.message;

import java.util.Collection;
import java.util.HashSet;

import com.mediatek.mms.ipmessage.DefaultIpMultiDeleteActivityExt;
import com.mediatek.mms.callback.IMultiDeleteActivityCallback;
import com.mediatek.mms.ipmessage.IIpMessageItemExt;
import android.database.Cursor;

import android.app.Activity;
import android.content.AsyncQueryHandler;
import android.content.Context;

import com.mediatek.rcse.api.Logger;

public class RcseMultiDeleteActivity extends DefaultIpMultiDeleteActivityExt {
    private static String TAG = "RcseMultiDeleteActivity";
    
    public static final String IPMSG_IDS = "forward_ipmsg_ids";
    
    /// M: add for ipmessage, record the ipmessage id.
    private HashSet<Long> mSelectedIpMessageIds = new HashSet<Long>();
    private Activity mContext;
    
    @Override
    public boolean multiDeleteActivityInit(Activity context,
            IMultiDeleteActivityCallback callback) {
        mContext = context;
        return true;
    }
    
    @Override
    public boolean onIpMultiDeleteClick(AsyncQueryHandler handler, int token, Object cookie,
            int deleteRunningCount, boolean deleteLocked) {
        /// M: delete ipmessage in external db
        if (mSelectedIpMessageIds.size() > 0) {
            long [] ids = new long[mSelectedIpMessageIds.size()];
            int k = 0;
            for (Long id : mSelectedIpMessageIds) {
                ids[k++] = id;
                Logger.d(TAG, "delete ipmessage, id:" + ids[k - 1]);
            }
            IpMessageManager.getInstance(mContext).deleteIpMsg(ids, false);
            if (deleteLocked) {
                mSelectedIpMessageIds.clear();
            }
            return true;
        }
        return false;
    }
    
    @Override
    public boolean onIpHandleItemClick(IIpMessageItemExt item, long ipMessageId, boolean isSelected,
            long msgId) {
        if (ipMessageId > 0) {
            if (isSelected) {
                mSelectedIpMessageIds.add(msgId);
            } else {
                mSelectedIpMessageIds.remove(msgId);
            }
        }
        return true;
    }
    
    @Override
    public boolean onIpMarkCheckedState(Cursor cursor, boolean checkedState) {
        // / M: add for ipmessage
        mSelectedIpMessageIds.clear();
        return true;
    }
    
    @Override
    public boolean onAddSelectedIpMessageId(boolean checkedState, long msgId, long ipMessageId) {
        /// M: add for ipmessage
        if (checkedState && ipMessageId > 0) {
            mSelectedIpMessageIds.add(msgId);
        }
        return true;
    }

    @Override
    public boolean onIpDeleteThread(Collection<Long> threads, int maxSmsId) {
        IpMessageUtils.deleteIpMessage(mContext, threads, maxSmsId, false);
        return true;
    }

}
