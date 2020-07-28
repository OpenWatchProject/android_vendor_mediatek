package com.mediatek.internal.telephony.ims;

import android.os.Parcel;
import android.os.Parcelable;

public class MtkPacketFilterInfo implements Parcelable {
    public static final int IMC_BMP_NONE = 0x00000000;
    public static final int IMC_BMP_V4_ADDR = 0x00000001;
    public static final int IMC_BMP_V6_ADDR = 0x00000002;
    public static final int IMC_BMP_PROTOCOL = 0x00000004;
    public static final int IMC_BMP_LOCAL_PORT_SINGLE = 0x00000008;
    public static final int IMC_BMP_LOCAL_PORT_RANGE = 0x00000010;
    public static final int IMC_BMP_REMOTE_PORT_SINGLE = 0x00000020;
    public static final int IMC_BMP_REMOTE_PORT_RANGE = 0x00000040;
    public static final int IMC_BMP_SPI = 0x00000080;
    public static final int IMC_BMP_TOS = 0x00000100;
    public static final int IMC_BMP_FLOW_LABEL = 0x00000200;

    public int mId;
    public int mPrecedence;
    public int mDirection;
    public int mNetworkPfIdentifier;
    public int mBitmap; // bit mask that use to check if the the following member is valid
    public String mAddress;
    public String mMask;
    public int mProtocolNextHeader;
    public int mLocalPortLow;
    public int mLocalPortHigh;
    public int mRemotePortLow;
    public int mRemotePortHigh;
    public int mSpi;
    public int mTos;
    public int mTosMask;
    public int mFlowLabel;

    public MtkPacketFilterInfo(int id, int precedence, int direction,
                                     int networkPfIdentifier,int bitmap,
                                     String address, String mask,
                                     int protocolNextHeader, int localPortLow,
                                     int localPortHigh, int remotePortLow,
                                     int remotePortHigh, int spi, int tos,
                                     int tosMask, int flowLabel) {
        mId = id;
        mPrecedence = precedence;
        mDirection = direction;
        mNetworkPfIdentifier = networkPfIdentifier;
        mBitmap = bitmap;
        mAddress = address;
        mMask = mask;
        mProtocolNextHeader = protocolNextHeader;
        mLocalPortLow = localPortLow;
        mLocalPortHigh = localPortHigh;
        mRemotePortLow = remotePortLow;
        mRemotePortHigh = remotePortHigh;
        mSpi = spi;
        mTos = tos;
        mTosMask = tosMask;
        mFlowLabel = flowLabel;
    }

    public static MtkPacketFilterInfo readFrom(Parcel p) {
        int id = p.readInt();
        int precedence = p.readInt();
        int direction = p.readInt();
        int networkPfIdentifier = p.readInt();
        int bitmap = p.readInt();
        String address = p.readString();
        String mask = p.readString();
        int protocolNextHeader = p.readInt();
        int localPortLow = p.readInt();
        int localPortHigh = p.readInt();
        int remotePortLow = p.readInt();
        int remotePortHigh = p.readInt();
        int spi = p.readInt();
        int tos = p.readInt();
        int tosMask = p.readInt();
        int flowLabel = p.readInt();

        return new MtkPacketFilterInfo(id, precedence, direction, networkPfIdentifier,
                                       bitmap, address, mask, protocolNextHeader, localPortLow,
                                       localPortHigh, remotePortLow, remotePortHigh, spi, tos,
                                       tosMask, flowLabel);
    }

    public void writeTo(Parcel p) {
        p.writeInt(mId);
        p.writeInt(mPrecedence);
        p.writeInt(mDirection);
        p.writeInt(mNetworkPfIdentifier);
        p.writeInt(mBitmap);
        p.writeString(mAddress == null ? "" : mAddress);
        p.writeString(mMask == null ? "" : mMask);
        p.writeInt(mProtocolNextHeader);
        p.writeInt(mLocalPortLow);
        p.writeInt(mLocalPortHigh);
        p.writeInt(mRemotePortLow);
        p.writeInt(mRemotePortHigh);
        p.writeInt(mSpi);
        p.writeInt(mTos);
        p.writeInt(mTosMask);
        p.writeInt(mFlowLabel);
    }

    @Override
    public String toString() {
        return "[id=" + mId + ", precedence=" + mPrecedence + ", direction=" + mDirection
                + ", networkPfIdentifier=" + mNetworkPfIdentifier +
                ", bitmap=" + Integer.toHexString(mBitmap) + ", address=" + mAddress + ", mask="
                + mMask + ", protocolNextHeader=" + mProtocolNextHeader +
                ", localPortLow=" + mLocalPortLow + ", localPortHigh=" + mLocalPortHigh
                + ", remotePortLow=" + mRemotePortLow +
                ", remotePortHigh=" + mRemotePortHigh + ", spi=" + Integer.toHexString(mSpi)
                + ", tos=" + mTos + ", tosMask=" + mTosMask + ", flowLabel="
                + Integer.toHexString(mFlowLabel) + "]";
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        writeTo(dest);
    }

    public static final Parcelable.Creator<MtkPacketFilterInfo> CREATOR =
            new Parcelable.Creator<MtkPacketFilterInfo>() {
        @Override
        public MtkPacketFilterInfo createFromParcel(Parcel source) {
            MtkPacketFilterInfo packetFilterInfo = MtkPacketFilterInfo.readFrom(source);
            return packetFilterInfo;
        }

        @Override
        public MtkPacketFilterInfo[] newArray(int size) {
            return new MtkPacketFilterInfo[size];
        }
    };
}

