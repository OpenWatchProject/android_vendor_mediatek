/*
 * Copyright (C) 2006 The Android Open Source Project
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

import android.telephony.CellSignalStrengthCdma;
import android.telephony.CellSignalStrengthGsm;
import android.telephony.CellSignalStrengthWcdma;
import android.telephony.CellSignalStrengthTdscdma;
import android.telephony.CellSignalStrengthLte;
import android.telephony.CellSignalStrengthNr;
import android.telephony.SignalStrength;
import android.telephony.ServiceState;
/**
 * {@hide}
 */
public class FemtoCellInfo implements Parcelable {
    /* This CSG is Not in EFACSGL nor in EFOCSGL */
    public static final int CSG_ICON_TYPE_NOT_ALLOWED = 0;

    /* This CSG is in Allowed CSG List (EFACSGL) */
    public static final int CSG_ICON_TYPE_ALLOWED = 1;

    /* This CSG is in Operator CSG List (EFOCSGL) */
    public static final int CSG_ICON_TYPE_OPERATOR = 2;

    /* This CSG is in Operator CSGs. However, it is unauthorized, meaning that UE's registration
        has been rejected by cause #25.*/
    public static final int CSG_ICON_TYPE_OPERATOR_UNAUTHORIZED = 3;

    public static final int STATE_CONNECTED    = 0;
    public static final int STATE_DISCONNECTED = 1;

    public static final int SIGNAL_STRENGTH_NONE_OR_UNKNOWN = 0;
    public static final int SIGNAL_STRENGTH_POOR = 1;
    public static final int SIGNAL_STRENGTH_MODERATE = 2;
    public static final int SIGNAL_STRENGTH_GOOD = 3;
    public static final int SIGNAL_STRENGTH_GREAT = 4;

    private int csgId;
    private int csgIconType; /* FemtoCellInfo.CSG_ICON_TYPE_xxx */
    private String homeNodeBName;
    private String operatorNumeric; /* ex: "46000" */
    private String operatorAlphaLong; /* ex: "China Mobile" */
    private int rat = 0; /* ServiceState.RIL_RADIO_TECHNOLOGY_xxx */

    private boolean con = false;
    private int sig;

    /**
     * Get registered CSG Id.
     * @return csgId CSG id.
     */
    public int getCsgId() {
        return csgId;
    }

    /**
     * Get registered CSG Icon Type.
     * @return csgIconType CSG icon type.
     */
    public int getCsgIconType() {
        return csgIconType;
    }

    /**
     * Get Home NodeB Name(if exist).
     * @return homeNodeBName node B name.
     */
    public String getHomeNodeBName() {
        return homeNodeBName;
    }

    /**
     * Get registered CSG Rat information.
     * @return rat operator rat.
     */
    public int getCsgRat() {
        return rat;
    }

    /**
     * Get registered operator numeric code.
     * @return operatorNumeric operator mcc/mnc.
     */
    public String getOperatorNumeric() {
        return operatorNumeric;
    }

    /**
     * Get registered operator alphanumeric long name.
     * @return operatorAlphaLong operator name.
     */
    public String getOperatorAlphaLong() {
        return operatorAlphaLong;
    }

    /**
     * Get connection state
     * @return STATE_CONNECTED or STATE_DISCONNECTED
     */
    public int getConnectionState() {
        if (con) return STATE_CONNECTED;
        else return STATE_DISCONNECTED;
    }

    /**
     * Get signal level
     * @return
     */
    public int getSignalLevel() {
        return updateLevel();
    }

    /**
     * Calculate signal level
     * @return 0~4
     */
    private int updateLevel() {
        if (rat == ServiceState.RIL_RADIO_TECHNOLOGY_LTE) {
            SignalStrength ss = new SignalStrength(new CellSignalStrengthCdma(),
                new CellSignalStrengthGsm(),
                new CellSignalStrengthWcdma(),
                new CellSignalStrengthTdscdma(),
                new CellSignalStrengthLte(99, sig, Integer.MAX_VALUE, Integer.MAX_VALUE, Integer.MAX_VALUE, Integer.MAX_VALUE),
                new CellSignalStrengthNr());
            return ss.getLevel();
        } else if (rat == ServiceState.RIL_RADIO_TECHNOLOGY_UMTS) {
            SignalStrength ss = new SignalStrength(new CellSignalStrengthCdma(),
                new CellSignalStrengthGsm(),
                new CellSignalStrengthWcdma(sig, Integer.MAX_VALUE, Integer.MAX_VALUE, Integer.MAX_VALUE),
                new CellSignalStrengthTdscdma(),
                new CellSignalStrengthLte(),
                new CellSignalStrengthNr());
            return ss.getLevel();
        }
        return SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
    }

    /**
     * @param csgId CSG ID.
     * @param csgIconType CSG icon type.
     * @param homeNodeBName node B name.
     * @param operatorNumeric operator mcc/mnc.
     * @param operatorAlphaLong operator name.
     * @param rat operator rat.
     */
    public FemtoCellInfo(int csgId, int csgIconType, String homeNodeBName,
        String operatorNumeric, String operatorAlphaLong, int rat,
        boolean con, int sig) {
        this.csgId = csgId;
        this.csgIconType = csgIconType;
        this.homeNodeBName = homeNodeBName;
        this.operatorNumeric = operatorNumeric;
        this.operatorAlphaLong = operatorAlphaLong;
        this.rat = rat;
        this.con = con;
        this.sig = sig;
    }

    /**
     * @return a string debug representation of this instance.
     */
    public String toString() {
        return "FemtoCellInfo " + csgId
                + "/" + csgIconType
                + "/" + homeNodeBName
                + "/" + operatorNumeric
                + "/" + operatorAlphaLong
                + "/" + rat
                + "/" + con
                + "/" + sig
                + "/" + getSignalLevel();
    }

    /**
     * Parcelable interface implemented below.
     * This is a simple effort to make FemtoCellInfo parcelable rather than
     * trying to make the conventional containing object (AsyncResult),
     * implement parcelable.
     */

    /**
     * @return 0 sfalse.
     */
    public int describeContents() {
        return 0;
    }

    /**
     * Implement the Parcelable interface.
     * Method to serialize a FemtoCellInfo object.
     * @param dest femtocell data.
     * @param flags no use.
     */
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(csgId);
        dest.writeInt(csgIconType);
        dest.writeString(homeNodeBName);
        dest.writeString(operatorNumeric);
        dest.writeString(operatorAlphaLong);
        dest.writeInt(rat);
        dest.writeInt(con?1:0);
        dest.writeInt(sig);
    }

    /**
     * Implement the Parcelable interface
     * Method to deserialize a FemtoCellInfo object, or an array thereof.
     */
    public static final Creator<FemtoCellInfo> CREATOR =
        new Creator<FemtoCellInfo>() {
            public FemtoCellInfo createFromParcel(Parcel in) {
                FemtoCellInfo femtoCellInfo = new FemtoCellInfo(
                        in.readInt(), /*csgId*/
                        in.readInt(), /*csgIconType*/
                        in.readString(), /*homeNodeBName*/
                        in.readString(), /*operatorNumeric*/
                        in.readString(), /*operatorAlphaLong*/
                        in.readInt(), /*rat*/
                        in.readInt() == 1? true:false, /*con*/
                        in.readInt()); /*sig*/
                return femtoCellInfo;
            }

            public FemtoCellInfo[] newArray(int size) {
                return new FemtoCellInfo[size];
            }
        };
}

