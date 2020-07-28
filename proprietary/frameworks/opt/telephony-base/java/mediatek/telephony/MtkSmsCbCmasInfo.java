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

package mediatek.telephony;

import android.os.Parcel;
import android.os.Parcelable;
import android.telephony.SmsCbCmasInfo;

/**
 * Contains CMAS warning notification Type 1 elements for a {@link SmsCbMessage}.
 * Supported values for each element are defined in TIA-1149-0-1 (CMAS over CDMA) and
 * 3GPP TS 23.041 (for GSM/UMTS).
 *
 * {@hide}
 */
public class MtkSmsCbCmasInfo extends SmsCbCmasInfo {

    // MTK-Start
    /**
     * CMAS expiration is unkown. The expiration is available for CDMA warining alerts
     * containing a type 2 elements record.
     */
    public static final long CMAS_EXPIRATION_UNKNOWN = 0L;
    // MTK-End

    /** CMAS Class Public Safety ALert (4396-4397). */
    public static final int CMAS_CLASS_PUBLIC_SAFETY = 0x07;

    /** CMAS Class State/Local WEA Tests  (4398-4399). */
    public static final int CMAS_CLASS_WEA_TEST = 0x08;

    // MTK-Start
    /** CMAS expiration. */
    private long mExpiration;
    // MTK-END

    /** Create a new SmsCbCmasInfo object with the specified values.
     *
     * @param messageClass  message class of the CMAS
     * @param category      category of the CMAS
     * @param responseType  response type of the CMAS
     * @param severity      severity of the CMAS
     * @param urgency       urgency of the CMAS
     * @param certainty     certainty of the CMAS
     * @param expiration    expiration of the CMAS
     */
    public MtkSmsCbCmasInfo(int messageClass, int category, int responseType, int severity,
            int urgency, int certainty, long expiration) {
        super(messageClass, category, responseType, severity, urgency, certainty);
        mExpiration = expiration;
    }

    /** Create a new SmsCbCmasInfo object from a Parcel. */
    public MtkSmsCbCmasInfo(Parcel in) {
        super(in.readInt(), in.readInt(), in.readInt(), in.readInt(), in.readInt(), in.readInt());
        mExpiration = in.readLong();
    }

    /**
     * Flatten this object into a Parcel.
     *
     * @param dest  The Parcel in which the object should be written.
     * @param flags Additional flags about how the object should be written (ignored).
     */
    @Override
    public void writeToParcel(Parcel dest, int flags) {
        super.writeToParcel(dest, flags);
        dest.writeLong(mExpiration);
    }

    @Override
    public String toString() {
        return super.toString() + "{" + mExpiration + "}";
    }

    /**
     * Get the expiration data of the CMAS alert.
     * @return the timestamp(UTC) of the expiration data of the CMAS alert
     */
    public long getExpiration() {
        return mExpiration;
    }

    /** Creator for unparcelling objects. */
    public static final Parcelable.Creator<SmsCbCmasInfo>
            CREATOR = new Parcelable.Creator<SmsCbCmasInfo>() {
        @Override
        public SmsCbCmasInfo createFromParcel(Parcel in) {
            return new MtkSmsCbCmasInfo(in);
        }

        @Override
        public SmsCbCmasInfo[] newArray(int size) {
            return new MtkSmsCbCmasInfo[size];
        }
    };
}
