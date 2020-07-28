/*
 * Copyright (C) 2012 The Android Open Source Project
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

package com.mediatek.internal.telephony;

import android.os.Parcel;
import android.os.Parcelable;

import java.util.ArrayList;

/**
 * Immutable cell information from a point in time.
 */
public class PseudoCellInfo implements Parcelable {
    public int mApcMode;
    public boolean mApcReport;
    public int mReportInterval;
    public int mCellCount;;
    public ArrayList<CellInfo> mCellInfos;

    class CellInfo {
        public int type;
        public int plmn;
        public int lac;
        public int cid;
        public int arfcn;
        public int bsic;
    }

    /** @hide */
    protected PseudoCellInfo() {
        this.mApcMode = 0;
        this.mApcReport = false;
        this.mReportInterval = 0;
        this.mCellCount = 0;
        this.mCellInfos = null;
    }

    protected PseudoCellInfo(int[] msgs) {
        mApcMode        =  msgs[0];
        mApcReport      =  msgs[1] == 1 ? true : false;;
        mReportInterval =  msgs[2];
        mCellCount      =  msgs[3];
        mCellInfos = new ArrayList<CellInfo>();
        for (int i = 0; i < mCellCount; i++) {
            CellInfo cell = new CellInfo();
            cell.type  =  msgs[i*6 + 4];
            cell.plmn  =  msgs[i*6 + 5];
            cell.lac   =  msgs[i*6 + 6];
            cell.cid   =  msgs[i*6 + 7];
            cell.arfcn =  msgs[i*6 + 8];
            cell.bsic  =  msgs[i*6 + 9];
            mCellInfos.add(cell);
        }
    }

    protected PseudoCellInfo(int apcMode, boolean reportEnable,
            int interVal, int[] cellinfo) {
        this.mApcMode = apcMode;
        this.mApcReport = reportEnable;
        this.mReportInterval = interVal;
        setCellInfo(cellinfo);
    }

    public void updateApcSetting(int apcMode, boolean reportEnable, int interVal) {
        mApcMode = apcMode;
        mApcReport = reportEnable;
        mReportInterval = interVal;
    }

    public void setCellInfo(int[] cellInfo) {
        mCellCount = cellInfo[0];
        mCellInfos = new ArrayList<CellInfo>();
        for (int i = 0; i < mCellCount; i++) {
            CellInfo cell = new CellInfo();
            cell.type  =  cellInfo[i*6 + 1];
            cell.plmn  =  cellInfo[i*6 + 2];
            cell.lac   =  cellInfo[i*6 + 3];
            cell.cid   =  cellInfo[i*6 + 4];
            cell.arfcn =  cellInfo[i*6 + 5];
            cell.bsic  =  cellInfo[i*6 + 6];
            mCellInfos.add(cell);
        }
    }

    public int getApcMode() {
        return mApcMode;
    }

    public boolean getReportEnable() {
        return mApcReport;
    }

    public int getReportInterval() {
        return mReportInterval;
    }

    public int getCellCount() {
        return mCellCount;
    }

    public int getType(int index) {
        if (index < 0 || index >= mCellCount) {
            return 0;
        }
        if (mCellInfos == null || mCellInfos.get(index) == null) {
            return 0;
        }
        return mCellInfos.get(index).type;
    }

    public int getPlmn(int index) {
        if (index < 0 || index >= mCellCount) {
            return 0;
        }
        if (mCellInfos == null || mCellInfos.get(index) == null) {
            return 0;
        }
        return mCellInfos.get(index).plmn;
    }

    public int getLac(int index) {
        if (index < 0 || index >= mCellCount) {
            return 0;
        }
        if (mCellInfos == null || mCellInfos.get(index) == null) {
            return 0;
        }
        return mCellInfos.get(index).lac;
    }

    public int getCid(int index) {
        if (index < 0 || index >= mCellCount) {
            return 0;
        }
        if (mCellInfos == null || mCellInfos.get(index) == null) {
            return 0;
        }
        return mCellInfos.get(index).cid;
    }

    public int getArfcn(int index) {
        if (index < 0 || index >= mCellCount) {
            return 0;
        }
        if (mCellInfos == null || mCellInfos.get(index) == null) {
            return 0;
        }
        return mCellInfos.get(index).arfcn;
    }

    public int getBsic(int index) {
        if (index < 0 || index >= mCellCount) {
            return 0;
        }
        if (mCellInfos == null || mCellInfos.get(index) == null) {
            return 0;
        }
        return mCellInfos.get(index).bsic;
    }

    @Override
    public String toString() {
        StringBuffer sb = new StringBuffer();
        sb.append("[");
        sb.append(mApcMode);
        sb.append(", ");
        sb.append(mApcReport);
        sb.append(", ");
        sb.append(mReportInterval);
        sb.append(", ");
        sb.append(mCellCount);
        sb.append("]");

        for (int i = 0; i < mCellCount; i++) {
            if (mCellInfos == null || mCellInfos.get(i) == null) {
                break;
            }
            sb.append("[");
            sb.append(mCellInfos.get(i).type);
            sb.append(", ");
            sb.append(mCellInfos.get(i).plmn);
            sb.append(", ");
            sb.append(mCellInfos.get(i).lac);
            sb.append(", ");
            sb.append(mCellInfos.get(i).cid);
            sb.append(", ");
            sb.append(mCellInfos.get(i).arfcn);
            sb.append(", ");
            sb.append(mCellInfos.get(i).bsic);
            sb.append("]");
        }
        return sb.toString();
    }

    /**
     * Implement the Parcelable interface
     */
    @Override
    public int describeContents() {
        return 0;
    }

    /** Implement the Parcelable interface */
    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mApcMode);
        dest.writeInt(mApcReport? 1 : 0);
        dest.writeInt(mReportInterval);
        dest.writeInt(mCellCount);
        for (int i = 0; i < mCellCount; i++) {
            if (mCellInfos == null || mCellInfos.get(i) == null) {
                return;
            }
            dest.writeInt(mCellInfos.get(i).type);
            dest.writeInt(mCellInfos.get(i).plmn);
            dest.writeInt(mCellInfos.get(i).lac);
            dest.writeInt(mCellInfos.get(i).cid);
            dest.writeInt(mCellInfos.get(i).arfcn);
            dest.writeInt(mCellInfos.get(i).bsic);
        }
    }

    /**
     * Used by child classes for parceling
     *
     * @hide
     */
    protected PseudoCellInfo(Parcel in) {
        mApcMode = in.readInt();
        mApcReport = (in.readInt() == 1? true : false);
        mReportInterval = in.readInt();
        mCellCount = in.readInt();
        mCellInfos = new ArrayList<CellInfo>();
        for (int i = 0; i < mCellCount; i++) {
            CellInfo cell = new CellInfo();
            cell.type  =  in.readInt();
            cell.plmn  =  in.readInt();
            cell.lac   =  in.readInt();
            cell.cid   =  in.readInt();
            cell.arfcn =  in.readInt();
            cell.bsic = in.readInt();
            mCellInfos.add(cell);

        }
    }

    /** Implement the Parcelable interface */
    public static final Creator<PseudoCellInfo> CREATOR = new Creator<PseudoCellInfo>() {
        @Override
        public PseudoCellInfo createFromParcel(Parcel in) {
            return new PseudoCellInfo(in);
        }

        @Override
        public PseudoCellInfo[] newArray(int size) {
            return new PseudoCellInfo[size];
        }
    };
}
