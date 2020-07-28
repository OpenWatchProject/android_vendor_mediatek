package verizon.net.sip;

import verizon.net.sip.EABService.Listener;

class EabRequest {
    public String[] mId;
    public String mServiceId;
    public Listener mListener;

    public EabRequest(String[] id, String serviceId,
        Listener listener) {
        mId = id;
        mServiceId = serviceId;
        mListener = listener;
    }
}
