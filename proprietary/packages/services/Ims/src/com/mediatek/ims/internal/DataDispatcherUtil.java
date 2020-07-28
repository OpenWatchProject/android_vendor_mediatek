
package com.mediatek.ims.internal;

import android.telephony.Rlog;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.ims.ImsAdapter.VaEvent;
import com.mediatek.ims.VaConstants;

public class DataDispatcherUtil {

    private static final String TAG = DataDispatcherUtil.class.getSimpleName();

    public DataDispatcherUtil() {
    }

    public ImsBearerRequest deCodeEvent(VaEvent event) {

        ImsBearerRequest request = null;
        switch(event.getRequestID()) {
            case VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_REQ:
                request = deCodeNwRequest(event);
                break;
            case VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_REQ:
                request = deCodeNwRelease(event);
                break;
            case VaConstants.MSG_ID_REQUEST_PCSCF_DISCOVERY:
                request = deCodePcscfDiscovery(event);
                break;
        }

        return request;
    }

    private ImsBearerRequest deCodeNwRequest(VaEvent event) {
        int reqId = event.getRequestID();
        int transId = event.getByte();
        int rat_type = event.getByte();
        String capability = PhoneConstants.APN_TYPE_IMS;
        if(event.getByte() == 1) {
            capability = PhoneConstants.APN_TYPE_EMERGENCY;
        }

        return new ImsBearerRequest(transId, reqId, capability);
    }

    private ImsBearerRequest deCodeNwRelease(VaEvent event) {
        int reqId = event.getRequestID();
        int transId = event.getByte();
        int abortId = event.getByte();
        boolean isValid = (event.getByte() == 1);
        if (isValid == false) {
            reqId = DataDispatcher.MSG_ID_WRAP_IMSM_IMSPA_PDN_ABORT;
        }
        String capability = PhoneConstants.APN_TYPE_IMS;
        if (event.getByte() == 1) {
            capability = PhoneConstants.APN_TYPE_EMERGENCY;
        }

        return new ImsBearerRequest(transId, reqId, capability);
    }

    private ImsBearerRequest deCodePcscfDiscovery(VaEvent event) {
        //imcf_uint8 transaction_id
        //imcf_uint8 context_id
        //imcf_uint8 pad [2]
        //char nw_if_name [IMC_MAXIMUM_NW_IF_NAME_STRING_SIZE]
        //imc_pcscf_acquire_method_enum pcscf_aqcuire_method

        int reqId = event.getRequestID();
        int transId = event.getByte();
        int cid = event.getByte();
        event.getBytes(2); //padding
        String capability = PhoneConstants.APN_TYPE_IMS;
        //TODO need to parsing interface to decide capablity

        return new ImsBearerRequest(transId, reqId, capability);
    }

    public VaEvent enCodeEvent(int respId, String data) {
        switch(respId) {
            case VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_ACT_ACK_RESP:
                return enCodeNwRequested(data);
            case VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_ACT_REJ_RESP:
                return enCodeRejectNwRequested(data);
            case VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_DEACT_ACK_RESP:
                return enCodeNwReleased(data);
            case VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_DEACT_REJ_RESP:
                return enCodeRejectNwReleased(data);
            case VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_DEACT_IND:
                return enCodeNotifyNwLost(data);
            case VaConstants.MSG_ID_REJECT_PCSCF_DISCOVERY:
                return enCodeRejectPcscfDiscovery(data);
            case VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_NETWORK_HANDLE_NOTIFY:
                return enCodeNotifyNetworkHandle(data);
        }

        loge("not support event: " + respId);
        return null;
    }

    private VaEvent enCodeNwRequested(String data) {
        logd("enCodeNwRequested: " + data);
        String[] contents = data.split(",");
        int transId = Integer.parseInt(contents[0]);
        int phoneId = Integer.parseInt(contents[1]);
        int netId = Integer.parseInt(contents[2]);
        String ifaceName = "";
        if (contents.length > 3) {
            ifaceName = contents[3];
        }

        VaEvent event = new VaEvent(phoneId,
            VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_ACT_ACK_RESP);
        event.putByte(transId);
        event.putBytes(new byte[3]);
        event.putInt(netId);
        event.putString(ifaceName, 16);
        return event;
    }

    private VaEvent enCodeRejectNwRequested(String data) {
        logd("enCodeRejectNwRequested: " + data);
        String[] contents = data.split(",");
        int transId = Integer.parseInt(contents[0]);
        int phoneId = Integer.parseInt(contents[1]);
        int cause = Integer.parseInt(contents[2]);

        VaEvent event = new VaEvent(phoneId,
            VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_ACT_REJ_RESP);
        event.putByte(transId);
        event.putBytes(new byte[3]);
        event.putInt(cause);
        return event;
    }

    private VaEvent enCodeNwReleased(String data) {
        logd("enCodeNwReleased: " + data);
        String[] contents = data.split(",");
        int transId = Integer.parseInt(contents[0]);
        int phoneId = Integer.parseInt(contents[1]);
        int cause = Integer.parseInt(contents[2]);
        String ifaceName = "";
        if (contents.length > 3) {
            ifaceName = contents[3];
        }

        VaEvent event = new VaEvent(phoneId,
            VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_DEACT_ACK_RESP);
        event.putByte(transId);
        event.putByte(cause);
        event.putBytes(new byte[2]);
        event.putString(ifaceName, 16);
        return event;
    }

    private VaEvent enCodeRejectNwReleased(String data) {
        logd("enCodeRejectNwReleased: " + data);
        String[] contents = data.split(",");
        int transId = Integer.parseInt(contents[0]);
        int phoneId = Integer.parseInt(contents[1]);
        int cause = Integer.parseInt(contents[2]);

        VaEvent event = new VaEvent(phoneId,
            VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_DEACT_REJ_RESP);
        event.putByte(transId);
        event.putBytes(new byte[3]);
        event.putInt(cause);
        return event;
    }

    private VaEvent enCodeNotifyNwLost(String data) {
        logd("enCodeNotifyNwLost: " + data);
        String[] contents = data.split(",");
        int phoneId = Integer.parseInt(contents[0]);
        int cause = Integer.parseInt(contents[1]);
        String ifaceName = "";
        if (contents.length > 2) {
            ifaceName = contents[2];
        }

        VaEvent event = new VaEvent(phoneId,
            VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_DEACT_IND);
        event.putByte(cause);
        event.putBytes(new byte[3]);
        event.putString(ifaceName, 16);
        return event;
    }

    private VaEvent enCodeRejectPcscfDiscovery(String data) {
        logd("enCodeRejectPcscfDiscovery: " + data);
        String[] contents = data.split(",");
        int transId = Integer.parseInt(contents[0]);
        int phoneId = Integer.parseInt(contents[1]);
        int cause = Integer.parseInt(contents[2]);

        VaEvent event = new VaEvent(phoneId,
            VaConstants.MSG_ID_REJECT_PCSCF_DISCOVERY);
        event.putByte(transId);
        event.putByte(cause);
        event.putBytes(new byte[2]);
        return event;
    }

    private VaEvent enCodeNotifyNetworkHandle(String data) {
        logd("enCodeNotifyNetworkHandle: " + data);
        String[] contents = data.split(",");
        int phoneId = Integer.parseInt(contents[0]);
        long networkHandle = Long.parseLong(contents[1]);

        VaEvent event = new VaEvent(phoneId,
                VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_NETWORK_HANDLE_NOTIFY);
        event.putLong(networkHandle);
        return event;
    }

    public void logd(String s) {
            Rlog.d(TAG, s);
        }

    public void logi(String s) {
        Rlog.i(TAG, s);
    }

    public void loge(String s) {
        Rlog.e(TAG, s);
    }

    public class ImsBearerRequest {

        private int mTransId, mRequest;
        private String mCapability;

        private ImsBearerRequest(int transId, int request, String capability) {
            mTransId = transId;
            mRequest = request;
            mCapability = capability;
        }

        public int getTransId() {
            return mTransId;
        }

        public int getRequestID() {
            return mRequest;
        }

        public String getCapability() {
            return mCapability;
        }

        public String toString() {
            return "{ transId: " + mTransId + " Request: " + mRequest
                + " Capbility: " + mCapability + " }";
        }
    }
}
