package verizon.net.sip;

import verizon.net.sip.EABService.Listener;

class ContactTuple {
    public Listener mListener;
    public String mService;
    public SipProfile mProfile;
    public boolean mIsUpdate;
    public boolean mIsSupportService;

    public ContactTuple(Listener l, String s) {
        mListener = l;
        mService = s;
        mIsUpdate = false;
        mIsSupportService = false;
        mProfile = null;
    }

    public Listener getListener() {
        return mListener;
    }

    public String getService() {
        return mService;
    }

    public SipProfile getSipProfile() {
        return mProfile;
    }

    public boolean isSupportService() {
        return mIsSupportService;
    }

    public boolean isUpdate() {
        return mIsUpdate;
    }
}
