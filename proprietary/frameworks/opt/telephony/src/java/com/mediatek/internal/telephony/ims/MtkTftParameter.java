package com.mediatek.internal.telephony.ims;

import android.os.Parcel;
import android.os.Parcelable;

import java.util.ArrayList;
import java.lang.StringBuffer;

public class MtkTftParameter implements Parcelable {
    public ArrayList<Integer> mLinkedPacketFilterIdList;

    public MtkTftParameter (ArrayList<Integer> linkedPacketFilterIdList) {
        mLinkedPacketFilterIdList = linkedPacketFilterIdList;
    }

    public static MtkTftParameter readFrom(Parcel p) {

        int linkedPfNumber = p.readInt();
        ArrayList<Integer> linkedPacketFilterIdList = new ArrayList<Integer>();
        for (int i = 0; i < linkedPfNumber; i++)
            linkedPacketFilterIdList.add(p.readInt());

        return new MtkTftParameter(linkedPacketFilterIdList);
    }

    public void writeTo(Parcel p) {
        p.writeInt(mLinkedPacketFilterIdList.size());
        for (Integer pfId : mLinkedPacketFilterIdList)
            p.writeInt(pfId);
    }

    @Override
    public String toString() {
        StringBuffer buf = new StringBuffer("LinkedPacketFilterIdList[");
        for (Integer linkedPacketFilterId : mLinkedPacketFilterIdList)
            buf.append(linkedPacketFilterId + " ");
        buf.append("]");
        return buf.toString();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        writeTo(dest);
    }

    public static final Parcelable.Creator<MtkTftParameter> CREATOR =
            new Parcelable.Creator<MtkTftParameter>() {
        @Override
        public MtkTftParameter createFromParcel(Parcel source) {
            MtkTftParameter tftParameter = MtkTftParameter.readFrom(source);
            return tftParameter;
        }

        @Override
        public MtkTftParameter[] newArray(int size) {
            return new MtkTftParameter[size];
        }
    };
}