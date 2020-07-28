package com.mediatek.mms.appserviceproxy;

import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;
//import android.app.PendingIntent;
import android.content.Intent;


public class AppServiceProxyParam implements Parcelable {
    private Intent mIntent;

    private AppServiceProxyParam(Parcel in) {
        readFromParcel(in);
    }

    public AppServiceProxyParam(Intent intent) {
        mIntent = intent;
    }

    public static final Parcelable.Creator<AppServiceProxyParam> CREATOR = new
    Parcelable.Creator<AppServiceProxyParam>() {

      public AppServiceProxyParam createFromParcel(Parcel in) {
        return new AppServiceProxyParam(in);
      }

      public AppServiceProxyParam[] newArray(int size) {
        return new AppServiceProxyParam[size];
      }

    };

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
      dest.writeParcelable(mIntent, 0);
    }

    public void readFromParcel(Parcel in) {
        mIntent = in.readParcelable(null);
    }

    public Intent getIntent() {
        return mIntent;
    }
}
