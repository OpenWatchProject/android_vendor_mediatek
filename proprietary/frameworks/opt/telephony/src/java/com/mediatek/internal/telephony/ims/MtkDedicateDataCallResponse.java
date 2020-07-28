package com.mediatek.internal.telephony.ims;

import java.util.ArrayList;
import android.os.Parcelable;
import android.os.Parcel;


public class MtkDedicateDataCallResponse implements Parcelable {
    public int mInterfaceId;
    public int mDefaultCid;
    public int mCid;
    public int mActive;
    public int mSignalingFlag;
    public int mBearerId;
    public int mFailCause;
    public MtkQosStatus mMtkQosStatus;
    public MtkTftStatus mMtkTftStatus;
    public String mPcscfAddress;

    public enum SetupResult {
        SUCCESS,
        FAIL;
        public int failCause = 0;
    }

    public static final String REASON_BEARER_ACTIVATION = "activation";
    public static final String REASON_BEARER_DEACTIVATION = "deactivation";
    public static final String REASON_BEARER_MODIFICATION = "modification";

    public MtkDedicateDataCallResponse(int interfaceId, int defaultId, int cid, int active,
                                                int signalingFlag, int bearerId, int faileCause,
                                                MtkQosStatus qosStatus, MtkTftStatus tftStatus,
                                                String pcscf) {
        mInterfaceId = interfaceId;
        mDefaultCid = defaultId;
        mCid = cid;
        mActive = active;
        mSignalingFlag = signalingFlag;
        mBearerId = bearerId;
        mFailCause = faileCause;
        mMtkQosStatus = qosStatus;
        mMtkTftStatus = tftStatus;
        mPcscfAddress = pcscf;
    }

    public static MtkDedicateDataCallResponse readFrom(Parcel p) {
        int interfaceId = p.readInt();
        int defaultCid = p.readInt();
        int cid = p.readInt();
        int active = p.readInt();
        int signalingFlag = p.readInt();
        int bearerId = p.readInt();
        int failCause = p.readInt();
        MtkQosStatus qosStatus = null;
        MtkTftStatus tftStatus = null;
        String pcscf = null;

        if (p.readInt() == 1) { // has QOS
            qosStatus = MtkQosStatus.readFrom(p);
        }
        if (p.readInt() == 1) { // has TFT
            tftStatus = MtkTftStatus.readFrom(p);
        }
        if (p.readInt() == 1) { // has PCSCF
            pcscf = p.readString();
        }

        return new MtkDedicateDataCallResponse(interfaceId, defaultCid, cid, active,
                                               signalingFlag, bearerId, failCause,
                                               qosStatus, tftStatus, pcscf);
    }

    @Override
    public String toString() {
        return "[interfaceId=" + mInterfaceId + ", defaultCid=" + mDefaultCid + ", cid="
            + mCid + ", active=" + mActive + ", signalingFlag=" + mSignalingFlag
            + ", bearerId=" + mBearerId + ", failCause=" + mFailCause
            + ", QOS=" + mMtkQosStatus + ", TFT=" + mMtkTftStatus
            + ", PCSCF=" + mPcscfAddress + "]";
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mInterfaceId);
        dest.writeInt(mDefaultCid);
        dest.writeInt(mCid);
        dest.writeInt(mActive);
        dest.writeInt(mSignalingFlag);
        dest.writeInt(mBearerId);
        dest.writeInt(mFailCause);
        dest.writeInt(mMtkQosStatus == null ? 0 : 1); // has QOS or not
        if (mMtkQosStatus != null)
            mMtkQosStatus.writeTo(dest);
        dest.writeInt(mMtkTftStatus == null ? 0 : 1); // has TFT or not
        if (mMtkTftStatus != null)
            mMtkTftStatus.writeTo(dest);
        dest.writeInt(mPcscfAddress == null ? 0 : 1); // has PCSCF or not
        dest.writeString(mPcscfAddress);
    }

    public static final Creator<MtkDedicateDataCallResponse> CREATOR =
            new Creator<MtkDedicateDataCallResponse>() {
        public MtkDedicateDataCallResponse createFromParcel(Parcel p) {
            MtkDedicateDataCallResponse dataInfo
                    = MtkDedicateDataCallResponse.readFrom(p);
            return dataInfo;
        }

        public MtkDedicateDataCallResponse[] newArray(int size) {
            return new MtkDedicateDataCallResponse[size];
        }
    };
}
