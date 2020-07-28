package com.mediatek.mms.appcallback;

import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;
import java.util.ArrayList;
import android.app.PendingIntent;

public class SmsEntry implements Parcelable {
    public int subId;
    public String destAddr;
    public String scAddr;
    public ArrayList<String> parts = new ArrayList<String>();
    public int encodingType;
    public ArrayList<PendingIntent> sentIntents = new ArrayList<PendingIntent>();
    public ArrayList<PendingIntent> deliveryIntents = new ArrayList<PendingIntent>();

    private SmsEntry(Parcel in) {
        readFromParcel(in);
    }

    public SmsEntry(int subid, String dest, String sc,
            ArrayList<String> textParts, int encType, ArrayList<PendingIntent> sIntents,
            ArrayList<PendingIntent> dIntents) {
        subId = subid;
        destAddr = dest;
        scAddr = sc;
        parts = textParts;
        encodingType = encType;
        sentIntents = sIntents;
        deliveryIntents = dIntents;
    }

    public static final Parcelable.Creator<SmsEntry> CREATOR = new
    Parcelable.Creator<SmsEntry>() {

      public SmsEntry createFromParcel(Parcel in) {
        return new SmsEntry(in);
      }

      public SmsEntry[] newArray(int size) {
        return new SmsEntry[size];
      }

    };

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
      dest.writeInt(subId);
      dest.writeString(destAddr);
      dest.writeString(scAddr);
      dest.writeStringList(parts);
      dest.writeInt(encodingType);
      dest.writeTypedList(sentIntents);
      dest.writeTypedList(deliveryIntents);
    }

    public void readFromParcel(Parcel in) {
        subId = in.readInt();
        destAddr = in.readString();
        scAddr = in.readString();
        in.readStringList(parts);
        encodingType = in.readInt();

        in.readTypedList(sentIntents, PendingIntent.CREATOR);
        in.readTypedList(deliveryIntents, PendingIntent.CREATOR);
    }
}
