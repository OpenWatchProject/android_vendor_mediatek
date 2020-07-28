package com.android.mms.draft;

import android.net.Uri;
import android.util.Log;

import com.android.mms.model.SlideshowModel;
import com.google.android.mms.pdu.PduBody;

import com.mediatek.opmsg.util.OpMessageUtils;


/*******************************************************************************

Class MmsDraftData which used as return object which contains slideshowmodel,subject
and messageuri.
The mMessageUri is create from save request

********************************************************************************/
public class MmsDraftData {

    private static final String TAG = "[Mms][Draft][MmsDraftData]";

    private SlideshowModel mSlideshowModel;

    private String mSubject;

    private Uri mMessageUri;

    private PduBody mBody;

    private boolean mBooleanResult;

    private int mCreateOrUpdate;

    public MmsDraftData(SlideshowModel slideshow, String subject, Uri uri) {
        if (slideshow == null || uri == null) {
            Log.w(TAG, "[MmsDraftData]Constructor parameters maybe is wrong");
            return ;
        }
        mSlideshowModel = slideshow;
        mSubject = subject;
        mMessageUri = uri;
    }

    public MmsDraftData() {
    }

    public MmsDraftData(SlideshowModel slideshow, String subject) {
        if (slideshow == null) {
            Log.w(TAG, "slideshow is null");
            return ;
        }
        mSlideshowModel = slideshow;
        mSubject = subject;
    }

    public void setSlideshow(SlideshowModel slideshow) {
        if (slideshow != null) {
            mSlideshowModel = slideshow;
        }
    }

    public void setSubject(String subject) {
        if (subject != null && subject.length() != 0) {
            mSubject = subject;
        }
    }

    public void setPduBody(PduBody pb) {
        if (pb != null) {
            mBody = pb;
        }
    }

    public void setMessageUri(Uri uri) {
        if (uri != null) {
            mMessageUri = uri;
        }
    }

    public void setBooleanResult(boolean result) {
        mBooleanResult = result;
    }

    public SlideshowModel getSlideshow() {
        return mSlideshowModel;
    }

    public String getSubject() {
        return mSubject;
    }

    public Uri getMessageUri() {
        return mMessageUri;
    }

    public PduBody getPduBody() {
        return mBody;
    }

    public boolean getBooleanResult() {
        return mBooleanResult;
    }

    public void setCreateOrUpdate(int create) {
        mCreateOrUpdate = create;
    }

    public int getCreateOrUpdate() {
        return mCreateOrUpdate;
    }

}
