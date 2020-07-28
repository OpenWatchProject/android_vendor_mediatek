package com.android.mms.draft;

import android.content.Context;
import android.net.Uri;
import android.util.Log;

import com.android.mms.model.SlideshowModel;
import com.android.mms.ui.MessageUtils;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.PduBody;
import com.mediatek.android.mms.pdu.MtkPduPersister;


/*******************************************************************************

The class is used to save parts which will be called in onPause() method which in
SlideEditorActity or SlideshowActivity.

Which means you edit the slide, after edit finished, will call this class to save
the parts, which you edit in the slideeditoractivity.

********************************************************************************/
public class UpdateReq extends TaskReq {

    private static final String TAG = "[Mms][Draft][UpdateReq]";

    public UpdateReq(int type, long threadId, Uri uri, Context context, SlideshowModel slideshow) {
        if (uri == null || slideshow == null || context == null) {
            Log.w(TAG, "[UpdateReq] Constructor, the parameters maybe is wrong");
            return;
        }
        mMessageUri = uri;
        mContext = context;
        mSlideshowModel = slideshow;
        mHandlerWhat = type;
        mSyncObject = new Object();
        mThreadId = threadId;
    }

    public int getType() {
        return mHandlerWhat;
    }

    public void executeReq() {
        Log.d(TAG, "[executeReq] enter and begin to update");
        try {
            PduBody pb = mSlideshowModel.toPduBody();
//            mdd.setPduBody(pb);
            MessageUtils.updatePartsIfNeeded(mSlideshowModel,
                    MtkPduPersister.getPduPersister(mContext), mMessageUri, pb, null);
            //MtkPduPersister.getPduPersister(mContext).updateParts(mMessageUri, pb, null);
            if (pb != null) {
                mSlideshowModel.sync(pb);
            }
        } catch (MmsException e) {
            Log.e(TAG, "[executeReq] happened exception when update");
        }
    }
}

