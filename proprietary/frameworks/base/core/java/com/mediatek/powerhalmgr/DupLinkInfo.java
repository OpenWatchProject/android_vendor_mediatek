/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.powerhalmgr;

import android.os.Parcel;
import android.os.Parcelable;

import java.util.Arrays;

/**
 * Information about a CPU
 *
 * @hide
 */
public class DupLinkInfo implements Parcelable {

    private String mSrcIp;
    private String mDstIp;
    private int mSrcPort;
    private int mDstPort;
    private int mProto;

    @Override
    public int describeContents() {
        return 0;
    }

    public String getSrcIp() {
        return mSrcIp;
    }

    public String getDstIp() {
        return mDstIp;
    }

    public int getSrcPort() {
        return mSrcPort;
    }

    public int getDstPort() {
        return mDstPort;
    }

    public int getProto() {
        return mProto;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeString(mSrcIp);
        out.writeString(mDstIp);
        out.writeInt(mSrcPort);
        out.writeInt(mDstPort);
        out.writeInt(mProto);
    }

    public void readFromParcel(Parcel in) {
        mSrcIp = in.readString();
        mDstIp = in.readString();
        mSrcPort = in.readInt();
        mDstPort = in.readInt();
        mProto = in.readInt();
    }

    public static final Parcelable.Creator<DupLinkInfo> CREATOR =
            new Parcelable.Creator<DupLinkInfo>() {
        @Override
        public DupLinkInfo createFromParcel(Parcel in) {
            return new DupLinkInfo(in);
        }

        @Override
        public DupLinkInfo[] newArray(int size) {
            return new DupLinkInfo[size];
        }
    };

    public DupLinkInfo(String src_ip, String dst_ip,
        int src_port, int dst_port, int proto) {
        mSrcIp = src_ip;
        mDstIp = dst_ip;
        mSrcPort = src_port;
        mDstPort = dst_port;
        mProto = proto;
    }

    private DupLinkInfo(Parcel in) {
        mSrcIp = in.readString();
        mDstIp = in.readString();
        mSrcPort = in.readInt();
        mDstPort = in.readInt();
        mProto = in.readInt();
    }

    @Override
    public String toString() {
        final StringBuilder sb = new StringBuilder();
        sb.append("DupLinkInfo(");
        sb.append(mSrcIp);
        sb.append(",");
        sb.append(mDstIp);
        sb.append(",");
        sb.append(mSrcPort);
        sb.append(",");
        sb.append(mDstPort);
        sb.append(",");
        sb.append(mProto);
        sb.append(")");
        return sb.toString();
    }
}
