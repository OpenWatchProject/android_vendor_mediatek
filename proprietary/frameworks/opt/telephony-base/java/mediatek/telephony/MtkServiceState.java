/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package mediatek.telephony;

import android.content.Intent;
import android.os.Bundle;
import android.os.Parcel;
import android.os.SystemProperties;
import android.telephony.AccessNetworkConstants;
import android.telephony.AccessNetworkConstants.TransportType;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.NetworkRegistrationInfo.Domain;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.telephony.Rlog;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;

import java.util.ArrayList;
import java.util.Arrays;

import android.annotation.ProductApi;

/**
 * Contains MTK proprietary phone state and service related information.
 *
 * The following phone information is included in returned ServiceState:
 *
 * <ul>
 *   <li>Service state: IN_SERVICE, OUT_OF_SERVICE, EMERGENCY_ONLY, POWER_OFF
 *   <li>Roaming indicator
 *   <li>Operator name, short name and numeric id
 *   <li>Network selection mode
 * </ul>
 */
public class MtkServiceState extends ServiceState {

    static final String LOG_TAG = "MTKSS";
    static final boolean DBG = false;

    // For HSPAP detail radio technology START
    /** @hide */
    public static final int RIL_RADIO_TECHNOLOGY_MTK = 128;
    /** @hide */
    public static final int RIL_RADIO_TECHNOLOGY_HSDPAP = RIL_RADIO_TECHNOLOGY_MTK + 1;
    /** @hide */
    public static final int RIL_RADIO_TECHNOLOGY_HSDPAP_UPA = RIL_RADIO_TECHNOLOGY_MTK + 2;
    /** @hide */
    public static final int RIL_RADIO_TECHNOLOGY_HSUPAP = RIL_RADIO_TECHNOLOGY_MTK + 3;
    /** @hide */
    public static final int RIL_RADIO_TECHNOLOGY_HSUPAP_DPA = RIL_RADIO_TECHNOLOGY_MTK + 4;
    /** @hide */
    public static final int RIL_RADIO_TECHNOLOGY_DC_DPA = RIL_RADIO_TECHNOLOGY_MTK + 5;
    /** @hide */
    public static final int RIL_RADIO_TECHNOLOGY_DC_UPA = RIL_RADIO_TECHNOLOGY_MTK + 6;
    /** @hide */
    public static final int RIL_RADIO_TECHNOLOGY_DC_HSDPAP = RIL_RADIO_TECHNOLOGY_MTK + 7;
    /** @hide */
    public static final int RIL_RADIO_TECHNOLOGY_DC_HSDPAP_UPA = RIL_RADIO_TECHNOLOGY_MTK + 8;
    /** @hide */
    public static final int RIL_RADIO_TECHNOLOGY_DC_HSDPAP_DPA = RIL_RADIO_TECHNOLOGY_MTK + 9;
    /** @hide */
    public static final int RIL_RADIO_TECHNOLOGY_DC_HSPAP = RIL_RADIO_TECHNOLOGY_MTK + 10;
    // For HSPAP detail radio technology END

    /**
     * MTK proprietary registration states for GSM, UMTS and CDMA.
     */
    /** @hide */
    public static final int
            REGISTRATION_STATE_NOT_REGISTERED_AND_NOT_SEARCHING_EMERGENCY_CALL_ENABLED = 10;
    /** @hide */
    public static final int REGISTRATION_STATE_NOT_REGISTERED_AND_SEARCHING_EMERGENCY_CALL_ENABLED
            = 12;
    /** @hide */
    public static final int REGISTRATION_STATE_REGISTRATION_DENIED_EMERGENCY_CALL_ENABLED = 13;
    /** @hide */
    public static final int REGISTRATION_STATE_UNKNOWN_EMERGENCY_CALL_ENABLED = 14;

    //MTK-START
    private int mRilVoiceRegState = NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
    private int mRilDataRegState  = NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
    //[ALPS01675318] -START
    private int mProprietaryDataRadioTechnology;
    //[ALPS01675318] -END
    private int mVoiceRejectCause = -1;
    private int mDataRejectCause = -1;
    //MTK-END

    // CellularDataRegState
    private int mRilCellularDataRegState = NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
    private int mCellularDataNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;

    /**
     * Create a new ServiceState from a intent notifier Bundle
     *
     * This method is used by PhoneStateIntentReceiver and maybe by
     * external applications.
     *
     * @param m Bundle from intent notifier
     * @return newly created ServiceState
     * @hide
     */
    public static ServiceState newFromBundle(Bundle m) {
        MtkServiceState ret;
        ret = new MtkServiceState();
        ret.setFromNotifierBundle(m);
        return ret;
    }

    /**
     * Empty constructor
     */
    public MtkServiceState() {
        // initialize all mtk's variable.
        // follow AOSP, use OOS as default.
        setStateOutOfService();
    }

    /**
     * Copy constructors
     *
     * @param s Source service state
     */
    public MtkServiceState(MtkServiceState s) {
        copyFrom(s);
    }


    /**
     * Copy constructors
     *
     * This function is for create SS instance in makeServiceState() of TelephonyRegistry.
     *
     * @param s Source service state
     */
    public MtkServiceState(ServiceState s) {
        copyFrom((MtkServiceState)s);
    }

    protected void copyFrom(MtkServiceState s) {
        mVoiceRegState = s.mVoiceRegState;
        mDataRegState = s.mDataRegState;
        mVoiceOperatorAlphaLong = s.mVoiceOperatorAlphaLong;
        mVoiceOperatorAlphaShort = s.mVoiceOperatorAlphaShort;
        mVoiceOperatorNumeric = s.mVoiceOperatorNumeric;
        mDataOperatorAlphaLong = s.mDataOperatorAlphaLong;
        mDataOperatorAlphaShort = s.mDataOperatorAlphaShort;
        mDataOperatorNumeric = s.mDataOperatorNumeric;
        mIsManualNetworkSelection = s.mIsManualNetworkSelection;
        mCssIndicator = s.mCssIndicator;
        mNetworkId = s.mNetworkId;
        mSystemId = s.mSystemId;
        mCdmaRoamingIndicator = s.mCdmaRoamingIndicator;
        mCdmaDefaultRoamingIndicator = s.mCdmaDefaultRoamingIndicator;
        mCdmaEriIconIndex = s.mCdmaEriIconIndex;
        mCdmaEriIconMode = s.mCdmaEriIconMode;
        mIsEmergencyOnly = s.mIsEmergencyOnly;
        mChannelNumber = s.mChannelNumber;
        mCellBandwidths = s.mCellBandwidths == null ? null :
                Arrays.copyOf(s.mCellBandwidths, s.mCellBandwidths.length);
        mLteEarfcnRsrpBoost = s.mLteEarfcnRsrpBoost;
        synchronized (mNetworkRegistrationInfos) {
            mNetworkRegistrationInfos.clear();
            mNetworkRegistrationInfos.addAll(s.getNetworkRegistrationInfoList());
        }
        mNrFrequencyRange = s.mNrFrequencyRange;
        mOperatorAlphaLongRaw = s.mOperatorAlphaLongRaw;
        mOperatorAlphaShortRaw = s.mOperatorAlphaShortRaw;
        mIsIwlanPreferred = s.mIsIwlanPreferred;
        //MTK-START
        mRilVoiceRegState = s.mRilVoiceRegState;
        mRilDataRegState = s.mRilDataRegState;
        mProprietaryDataRadioTechnology = s.mProprietaryDataRadioTechnology;
        mVoiceRejectCause = s.mVoiceRejectCause;
        mDataRejectCause = s.mDataRejectCause;
        mRilCellularDataRegState = s.mRilCellularDataRegState;
        mCellularDataNetworkType = s.mCellularDataNetworkType;
        //MTK-END
    }

    /**
     * Construct a MtkServiceState object from the given parcel.
     */
    public MtkServiceState(Parcel in) {
        mVoiceRegState = in.readInt();
        mDataRegState = in.readInt();
        mVoiceOperatorAlphaLong = in.readString();
        mVoiceOperatorAlphaShort = in.readString();
        mVoiceOperatorNumeric = in.readString();
        mDataOperatorAlphaLong = in.readString();
        mDataOperatorAlphaShort = in.readString();
        mDataOperatorNumeric = in.readString();
        mIsManualNetworkSelection = in.readInt() != 0;
        mCssIndicator = (in.readInt() != 0);
        mNetworkId = in.readInt();
        mSystemId = in.readInt();
        mCdmaRoamingIndicator = in.readInt();
        mCdmaDefaultRoamingIndicator = in.readInt();
        mCdmaEriIconIndex = in.readInt();
        mCdmaEriIconMode = in.readInt();
        mIsEmergencyOnly = in.readInt() != 0;
        mLteEarfcnRsrpBoost = in.readInt();
        synchronized (mNetworkRegistrationInfos) {
            in.readList(mNetworkRegistrationInfos, NetworkRegistrationInfo.class.getClassLoader());
        }
        mChannelNumber = in.readInt();
        mCellBandwidths = in.createIntArray();
        mNrFrequencyRange = in.readInt();
        mOperatorAlphaLongRaw = in.readString();
        mOperatorAlphaShortRaw = in.readString();
        mIsIwlanPreferred = in.readBoolean();
        // MTK START
        mRilVoiceRegState = in.readInt();
        mRilDataRegState = in.readInt();
        mProprietaryDataRadioTechnology = in.readInt();
        mVoiceRejectCause = in.readInt();
        mDataRejectCause = in.readInt();
        mRilCellularDataRegState = in.readInt();
        mCellularDataNetworkType = in.readInt();
        // MTK END
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mVoiceRegState);
        out.writeInt(mDataRegState);
        out.writeString(mVoiceOperatorAlphaLong);
        out.writeString(mVoiceOperatorAlphaShort);
        out.writeString(mVoiceOperatorNumeric);
        out.writeString(mDataOperatorAlphaLong);
        out.writeString(mDataOperatorAlphaShort);
        out.writeString(mDataOperatorNumeric);
        out.writeInt(mIsManualNetworkSelection ? 1 : 0);
        out.writeInt(mCssIndicator ? 1 : 0);
        out.writeInt(mNetworkId);
        out.writeInt(mSystemId);
        out.writeInt(mCdmaRoamingIndicator);
        out.writeInt(mCdmaDefaultRoamingIndicator);
        out.writeInt(mCdmaEriIconIndex);
        out.writeInt(mCdmaEriIconMode);
        out.writeInt(mIsEmergencyOnly ? 1 : 0);
        out.writeInt(mLteEarfcnRsrpBoost);
        synchronized (mNetworkRegistrationInfos) {
            out.writeList(mNetworkRegistrationInfos);
        }
        out.writeInt(mChannelNumber);
        out.writeIntArray(mCellBandwidths);
        out.writeInt(mNrFrequencyRange);
        out.writeString(mOperatorAlphaLongRaw);
        out.writeString(mOperatorAlphaShortRaw);
        out.writeBoolean(mIsIwlanPreferred);
        // MTK START
        out.writeInt(mRilVoiceRegState);
        out.writeInt(mRilDataRegState);
        out.writeInt(mProprietaryDataRadioTechnology);
        out.writeInt(mVoiceRejectCause);
        out.writeInt(mDataRejectCause);
        out.writeInt(mRilCellularDataRegState);
        out.writeInt(mCellularDataNetworkType);
        // MTK END
    }

    @Override
    public boolean equals (Object o) {
        MtkServiceState s;

        try {
            s = (MtkServiceState) o;
        } catch (ClassCastException ex) {
            return false;
        }

        if (o == null) {
            return false;
        }
        synchronized (mNetworkRegistrationInfos) {
            return (mVoiceRegState == s.mVoiceRegState
                    && mDataRegState == s.mDataRegState
                    && mIsManualNetworkSelection == s.mIsManualNetworkSelection
                    && mChannelNumber == s.mChannelNumber
                    && Arrays.equals(mCellBandwidths, s.mCellBandwidths)
                    && equalsHandlesNulls(mVoiceOperatorAlphaLong, s.mVoiceOperatorAlphaLong)
                    && equalsHandlesNulls(mVoiceOperatorAlphaShort, s.mVoiceOperatorAlphaShort)
                    && equalsHandlesNulls(mVoiceOperatorNumeric, s.mVoiceOperatorNumeric)
                    && equalsHandlesNulls(mDataOperatorAlphaLong, s.mDataOperatorAlphaLong)
                    && equalsHandlesNulls(mDataOperatorAlphaShort, s.mDataOperatorAlphaShort)
                    && equalsHandlesNulls(mDataOperatorNumeric, s.mDataOperatorNumeric)
                    && equalsHandlesNulls(mCssIndicator, s.mCssIndicator)
                    && equalsHandlesNulls(mNetworkId, s.mNetworkId)
                    && equalsHandlesNulls(mSystemId, s.mSystemId)
                    && equalsHandlesNulls(mCdmaRoamingIndicator, s.mCdmaRoamingIndicator)
                    && equalsHandlesNulls(mCdmaDefaultRoamingIndicator,
                            s.mCdmaDefaultRoamingIndicator)
                    && mIsEmergencyOnly == s.mIsEmergencyOnly
                    && equalsHandlesNulls(mOperatorAlphaLongRaw, s.mOperatorAlphaLongRaw)
                    && equalsHandlesNulls(mOperatorAlphaShortRaw, s.mOperatorAlphaShortRaw)
                    && mNetworkRegistrationInfos.size() == s.mNetworkRegistrationInfos.size()
                    && mNetworkRegistrationInfos.containsAll(s.mNetworkRegistrationInfos)
                    && mNrFrequencyRange == s.mNrFrequencyRange
                    && mIsIwlanPreferred == s.mIsIwlanPreferred)
                    //MTK START
                    && mRilVoiceRegState == s.mRilVoiceRegState
                    && mRilDataRegState == s.mRilDataRegState
                    && equalsHandlesNulls(mProprietaryDataRadioTechnology,
                            s.mProprietaryDataRadioTechnology)
                    && mVoiceRejectCause == s.mVoiceRejectCause
                    && mDataRejectCause == s.mDataRejectCause
                    && mRilCellularDataRegState == s.mRilCellularDataRegState
                    && mCellularDataNetworkType == s.mCellularDataNetworkType;
                    //MTK END
        }
    }

    @Override
    public String toString() {
        synchronized (mNetworkRegistrationInfos) {
            return new StringBuilder().append("{mVoiceRegState=").append(mVoiceRegState)
                .append("(" + rilServiceStateToString(mVoiceRegState) + ")")
                .append(", mDataRegState=").append(mDataRegState)
                .append("(" + rilServiceStateToString(mDataRegState) + ")")
                .append(", mChannelNumber=").append(mChannelNumber)
                .append(", duplexMode()=").append(getDuplexMode())
                .append(", mCellBandwidths=").append(Arrays.toString(mCellBandwidths))
                .append(", mVoiceOperatorAlphaLong=").append(mVoiceOperatorAlphaLong)
                .append(", mVoiceOperatorAlphaShort=").append(mVoiceOperatorAlphaShort)
                .append(", mDataOperatorAlphaLong=").append(mDataOperatorAlphaLong)
                .append(", mDataOperatorAlphaShort=").append(mDataOperatorAlphaShort)
                .append(", isManualNetworkSelection=").append(mIsManualNetworkSelection)
                .append(mIsManualNetworkSelection ? "(manual)" : "(automatic)")
                .append(", getRilVoiceRadioTechnology=").append(getRilVoiceRadioTechnology())
                .append("(" + rilRadioTechnologyToString(getRilVoiceRadioTechnology()) + ")")
                .append(", getRilDataRadioTechnology=").append(getRilDataRadioTechnology())
                .append("(" + rilRadioTechnologyToString(getRilDataRadioTechnology()) + ")")
                .append(", mCssIndicator=").append(mCssIndicator ? "supported" : "unsupported")
                .append(", mNetworkId=").append(mNetworkId)
                .append(", mSystemId=").append(mSystemId)
                .append(", mCdmaRoamingIndicator=").append(mCdmaRoamingIndicator)
                .append(", mCdmaDefaultRoamingIndicator=").append(mCdmaDefaultRoamingIndicator)
                .append(", mIsEmergencyOnly=").append(mIsEmergencyOnly)
                .append(", isUsingCarrierAggregation=").append(isUsingCarrierAggregation())
                .append(", mLteEarfcnRsrpBoost=").append(mLteEarfcnRsrpBoost)
                .append(", mNetworkRegistrationInfos=").append(mNetworkRegistrationInfos)
                .append(", mNrFrequencyRange=").append(mNrFrequencyRange)
                .append(", mOperatorAlphaLongRaw=").append(mOperatorAlphaLongRaw)
                .append(", mOperatorAlphaShortRaw=").append(mOperatorAlphaShortRaw)
                .append(", mIsIwlanPreferred=").append(mIsIwlanPreferred)
                //MTK START
                .append(", Ril Voice Regist state=").append(mRilVoiceRegState)
                .append(", Ril Data Regist state=").append(mRilDataRegState)
                .append(", mProprietaryDataRadioTechnology=").append(mProprietaryDataRadioTechnology)
                .append(", VoiceRejectCause=").append(mVoiceRejectCause)
                .append(", DataRejectCause=").append(mDataRejectCause)
                .append(", IwlanRegState=").append(getIwlanRegState())
                .append(", CellularVoiceRegState=").append(getCellularVoiceRegState())
                .append(", CellularDataRegState=").append(getCellularDataRegState())
                .append(", RilCellularDataRegState=").append(getRilCellularDataRegState())
                .append(", CellularDataRoamingType=").append(getCellularDataRoamingType())
                .append(", CellularDataNetworkType=").append(getCellularDataNetworkType())
                //MTK END
                .append("}").toString();
        }
    }

    @Override
    protected void init() {
        if (DBG) Rlog.d(LOG_TAG, "[MtkServiceState] init");
        super.init();
        //MTK-START
        mRilVoiceRegState = NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
        mRilDataRegState  = NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
        mProprietaryDataRadioTechnology = 0;
        mVoiceRejectCause = -1;
        mDataRejectCause = -1;
        mRilCellularDataRegState = NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
        mCellularDataNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        //MTK-END
    }

    /**
     * Set MtkServiceState based on intent notifier map.
     *
     * @param m intent notifier map
     * @hide
     */
    @Override
    protected void setFromNotifierBundle(Bundle m) {
        MtkServiceState ssFromBundle = m.getParcelable(Intent.EXTRA_SERVICE_STATE);
        if (ssFromBundle != null) {
            copyFrom(ssFromBundle);
        }
    }

    /**
     * Set intent notifier Bundle based on service state.
     *
     * @param m intent notifier Bundle
     * @hide
     */
    @Override
    public void fillInNotifierBundle(Bundle m) {
        m.putParcelable(Intent.EXTRA_SERVICE_STATE, this);
        // serviceState already consists of below entries.
        // for backward compatibility, we continue fill in below entries.
        m.putInt("voiceRegState", mVoiceRegState);
        m.putInt("dataRegState", mDataRegState);
        m.putInt("dataRoamingType", getDataRoamingType());
        m.putInt("voiceRoamingType", getVoiceRoamingType());
        m.putString("operator-alpha-long", mVoiceOperatorAlphaLong);
        m.putString("operator-alpha-short", mVoiceOperatorAlphaShort);
        m.putString("operator-numeric", mVoiceOperatorNumeric);
        m.putString("data-operator-alpha-long", mDataOperatorAlphaLong);
        m.putString("data-operator-alpha-short", mDataOperatorAlphaShort);
        m.putString("data-operator-numeric", mDataOperatorNumeric);
        m.putBoolean("manual", mIsManualNetworkSelection);
        m.putInt("radioTechnology", getRilVoiceRadioTechnology());
        m.putInt("dataRadioTechnology", getRadioTechnology());
        m.putBoolean("cssIndicator", mCssIndicator);
        m.putInt("networkId", mNetworkId);
        m.putInt("systemId", mSystemId);
        m.putInt("cdmaRoamingIndicator", mCdmaRoamingIndicator);
        m.putInt("cdmaDefaultRoamingIndicator", mCdmaDefaultRoamingIndicator);
        m.putBoolean("emergencyOnly", mIsEmergencyOnly);
        m.putBoolean("isDataRoamingFromRegistration", getDataRoamingFromRegistration());
        m.putBoolean("isUsingCarrierAggregation", isUsingCarrierAggregation());
        m.putInt("LteEarfcnRsrpBoost", mLteEarfcnRsrpBoost);
        m.putInt("ChannelNumber", mChannelNumber);
        m.putIntArray("CellBandwidths", mCellBandwidths);
        m.putInt("mNrFrequencyRange", mNrFrequencyRange);
        //MTK-START
        m.putInt("RilVoiceRegState", mRilVoiceRegState);
        m.putInt("RilDataRegState", mRilDataRegState);
        m.putInt("proprietaryDataRadioTechnology", mProprietaryDataRadioTechnology);
        m.putInt("VoiceRejectCause", mVoiceRejectCause);
        m.putInt("DataRejectCause", mDataRejectCause);
        m.putInt("RilCellularDataRegState", mRilCellularDataRegState);
        m.putInt("CellularDataNetworkType", mCellularDataNetworkType);
        //MTK-END
    }

    /**
     * Returns a merged ServiceState consisting of the base SS with voice settings from the
     * voice SS. The voice SS is only used if it is IN_SERVICE (otherwise the base SS is returned).
     * @hide
     * */
    public static MtkServiceState mergeMtkServiceStates(MtkServiceState baseSs,
            MtkServiceState voiceSs) {
        if (voiceSs.mVoiceRegState != STATE_IN_SERVICE) {
            return baseSs;
        }

        MtkServiceState newSs = new MtkServiceState(baseSs);

        // voice overrides
        newSs.mVoiceRegState = voiceSs.mVoiceRegState;
        newSs.mIsEmergencyOnly = false; // only get here if voice is IN_SERVICE

        return newSs;
    }

    // M: MTK Added methods START

    /**
     * Get current voice network registration reject cause.
     * See 3GPP TS 24.008,section 10.5.3.6 and Annex G.
     * @return registration reject cause or INVALID value (-1)
     * @hide
     */
    public int getVoiceRejectCause() {
        return mVoiceRejectCause;
    }

    /**
     * Get current data network registration reject cause.
     * See 3GPP TS 24.008 Annex G.6 "Additional cause codes for GMM".
     * @return registration reject cause or INVALID value (-1)
     * @hide
     */
    public int getDataRejectCause() {
        return mDataRejectCause;
    }

    /** @hide */
    public void setVoiceRejectCause(int cause) {
        mVoiceRejectCause = cause;
    }

    /** @hide */
    public void setDataRejectCause(int cause) {
        mDataRejectCause = cause;
    }

    // [ALPS01675318] -START
    /** @hide */
    public int getProprietaryDataRadioTechnology() {
        return this.mProprietaryDataRadioTechnology;
    }

    /** @hide */
    public void setProprietaryDataRadioTechnology(int rt) {
        if (DBG) Rlog.d(LOG_TAG, "[MtkServiceState] setProprietaryDataRadioTechnology = " + rt);
        mProprietaryDataRadioTechnology = rt;
    }
    //[ALPS01675318] -END

    /**
     * @hide
     */
    public int rilRadioTechnologyToNetworkTypeEx(int rt) {
        return rilRadioTechnologyToNetworkType(rt);
    }

    /** @hide */
    public int getRilVoiceRegState() {
        return mRilVoiceRegState;
    }

    /** @hide */
    public int getRilDataRegState() {
        return mRilDataRegState;
    }

    /**
     * @hide
     */
    public void setRilVoiceRegState(int nRegState) {
        mRilVoiceRegState = nRegState;
    }

    /**
     * @hide
     */
    public void setRilDataRegState(int nDataRegState) {
        mRilDataRegState = nDataRegState;
    }

    /**
     * @hide
     */
    public boolean compareTwoRadioTechnology(int nRadioTechnology1, int nRadioTechnology2) {
        if (nRadioTechnology1 == nRadioTechnology2) {
            return false;
        } else if (nRadioTechnology1 == RIL_RADIO_TECHNOLOGY_LTE) {
            return true;
        } else if (nRadioTechnology2 == RIL_RADIO_TECHNOLOGY_LTE) {
            return false;
        } else if (nRadioTechnology1 == RIL_RADIO_TECHNOLOGY_GSM) {
            // ALPS02230032-START
            if (nRadioTechnology2 == RIL_RADIO_TECHNOLOGY_UNKNOWN) {
                return true;
            }
            // ALPS00230032-END
            return false;
        } else if (nRadioTechnology2 == RIL_RADIO_TECHNOLOGY_GSM) {
            // ALPS02230032-START
            if (nRadioTechnology1 == RIL_RADIO_TECHNOLOGY_UNKNOWN) {
                return false;
            }
            // ALPS00230032-END
            return true;
        } else if (nRadioTechnology1 > nRadioTechnology2) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Get current Cellular data network roaming type.
     * @return roaming type
     * @hide
     */
    public boolean getCellularDataRoaming() {
        final NetworkRegistrationInfo regState = getNetworkRegistrationInfo(
                NetworkRegistrationInfo.DOMAIN_PS, AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        if (regState != null) {
            return regState.getRoamingType() != ROAMING_TYPE_NOT_ROAMING;
        }
        return false;
    }

    /**
     * Get cellur data network type.
     * @return Cellur data network type
     * @hide
     */
    public int getCellularDataNetworkType() {
        return mCellularDataNetworkType;
    }

    /**
     * Get current Cellular service state.
     *
     * @see #STATE_IN_SERVICE
     * @see #STATE_OUT_OF_SERVICE
     * @see #STATE_EMERGENCY_ONLY
     * @see #STATE_POWER_OFF
     * @return if voice and data is in service, return in service.
     * @hide
     */
    @ProductApi
    public int getCellularRegState() {
        if (getCellularVoiceRegState() == STATE_POWER_OFF) return STATE_POWER_OFF;
        else if (getCellularDataRegState() == STATE_IN_SERVICE) return STATE_IN_SERVICE;
        else return getCellularVoiceRegState();
    }

    /**
     * Get current Cellular voice service state.
     *
     * @see #STATE_IN_SERVICE
     * @see #STATE_OUT_OF_SERVICE
     * @see #STATE_EMERGENCY_ONLY
     * @see #STATE_POWER_OFF
     * @return cellular voice reg state
     * @hide
     */
    public int getCellularVoiceRegState() {
        final NetworkRegistrationInfo regCsState = getNetworkRegistrationInfo(
                NetworkRegistrationInfo.DOMAIN_CS, AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        if (mVoiceRegState == STATE_POWER_OFF) return mVoiceRegState;
        if (regCsState != null && regCsState.isInService()) return STATE_IN_SERVICE;
        return STATE_OUT_OF_SERVICE;
    }


    /**
     * Get current Cellular data service state.
     *
     * @see #STATE_IN_SERVICE
     * @see #STATE_OUT_OF_SERVICE
     * @see #STATE_EMERGENCY_ONLY
     * @see #STATE_POWER_OFF
     * @return cellular data reg state
     * @hide
     */
    public int getCellularDataRegState() {
        if (mVoiceRegState == STATE_POWER_OFF) return mVoiceRegState;
        if (mRilCellularDataRegState == NetworkRegistrationInfo.REGISTRATION_STATE_HOME ||
            mRilCellularDataRegState == NetworkRegistrationInfo.REGISTRATION_STATE_ROAMING)
            return STATE_IN_SERVICE;
        return STATE_OUT_OF_SERVICE;
    }

    public int getRilCellularDataRegState() {
        return mRilCellularDataRegState;
    }

    /**
     * Get current Cellular data network roaming type.
     * @return roaming type
     * @hide
     */
    public int getCellularDataRoamingType() {
        final NetworkRegistrationInfo regPsState = getNetworkRegistrationInfo(
                NetworkRegistrationInfo.DOMAIN_PS, AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        // Use Voice to check power state because mDataRegState considers IWLAN
        if (mVoiceRegState == STATE_POWER_OFF) return ROAMING_TYPE_NOT_ROAMING;
        if (regPsState != null) return regPsState.getRoamingType();
        return ROAMING_TYPE_NOT_ROAMING;
    }

    /**
     * @return check cellular data CA state
     * @hide
     */
    public boolean isUsingCellularCarrierAggregation() {
        return isUsingCarrierAggregation();
    }

    /**
     * @see #STATE_IN_SERVICE
     * @see #STATE_OUT_OF_SERVICE
     * @see #STATE_EMERGENCY_ONLY
     * @see #STATE_POWER_OFF
     * @return iwlan state
     * @hide
     */
    public int getIwlanRegState() {
        final NetworkRegistrationInfo regIwlanState = getNetworkRegistrationInfo(
                NetworkRegistrationInfo.DOMAIN_PS, AccessNetworkConstants.TRANSPORT_TYPE_WLAN);
        if (regIwlanState != null && regIwlanState.isInService()) return STATE_IN_SERVICE;
        return STATE_OUT_OF_SERVICE;

    }

    /**
     * When we support legacy mode, we lost cellular data state
     * we keep it for other usage
     * @hide
     */
    public void keepCellularDataServiceState() {
        final NetworkRegistrationInfo regPsState = getNetworkRegistrationInfo(
                NetworkRegistrationInfo.DOMAIN_PS, AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        if (regPsState != null) {
            mRilCellularDataRegState = regPsState.getRegistrationState();
            mCellularDataNetworkType = regPsState.getAccessNetworkTechnology();
        }
    }
    // M: MTK Added methods END

    @Override
    public int getRilDataRadioTechnology() {
        return mtkNetworkTypeToRilRadioTechnology(getDataNetworkType());
    }

    @Override
    public int getRilVoiceRadioTechnology() {
        NetworkRegistrationInfo wwanRegInfo = getNetworkRegistrationInfo(
                NetworkRegistrationInfo.DOMAIN_CS, AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        if (wwanRegInfo != null) {
            // M: for 5G
            return mtkNetworkTypeToRilRadioTechnology(wwanRegInfo.getAccessNetworkTechnology());
        }
        return RIL_RADIO_TECHNOLOGY_UNKNOWN;
    }

    private static int mtkNetworkTypeToRilRadioTechnology(int networkType) {
        switch(networkType) {
            case TelephonyManager.NETWORK_TYPE_GPRS:
                return ServiceState.RIL_RADIO_TECHNOLOGY_GPRS;
            case TelephonyManager.NETWORK_TYPE_EDGE:
                return ServiceState.RIL_RADIO_TECHNOLOGY_EDGE;
            case TelephonyManager.NETWORK_TYPE_UMTS:
                return ServiceState.RIL_RADIO_TECHNOLOGY_UMTS;
            case TelephonyManager.NETWORK_TYPE_HSDPA:
                return ServiceState.RIL_RADIO_TECHNOLOGY_HSDPA;
            case TelephonyManager.NETWORK_TYPE_HSUPA:
                return ServiceState.RIL_RADIO_TECHNOLOGY_HSUPA;
            case TelephonyManager.NETWORK_TYPE_HSPA:
                return ServiceState.RIL_RADIO_TECHNOLOGY_HSPA;
            case TelephonyManager.NETWORK_TYPE_CDMA:
                return ServiceState.RIL_RADIO_TECHNOLOGY_IS95A;
            case TelephonyManager.NETWORK_TYPE_1xRTT:
                return ServiceState.RIL_RADIO_TECHNOLOGY_1xRTT;
            case TelephonyManager.NETWORK_TYPE_EVDO_0:
                return ServiceState.RIL_RADIO_TECHNOLOGY_EVDO_0;
            case TelephonyManager.NETWORK_TYPE_EVDO_A:
                return ServiceState.RIL_RADIO_TECHNOLOGY_EVDO_A;
            case TelephonyManager.NETWORK_TYPE_EVDO_B:
                return ServiceState.RIL_RADIO_TECHNOLOGY_EVDO_B;
            case TelephonyManager.NETWORK_TYPE_EHRPD:
                return ServiceState.RIL_RADIO_TECHNOLOGY_EHRPD;
            case TelephonyManager.NETWORK_TYPE_LTE:
                return ServiceState.RIL_RADIO_TECHNOLOGY_LTE;
            case TelephonyManager.NETWORK_TYPE_HSPAP:
                return ServiceState.RIL_RADIO_TECHNOLOGY_HSPAP;
            case TelephonyManager.NETWORK_TYPE_GSM:
                return ServiceState.RIL_RADIO_TECHNOLOGY_GSM;
            case TelephonyManager.NETWORK_TYPE_TD_SCDMA:
                return ServiceState.RIL_RADIO_TECHNOLOGY_TD_SCDMA;
            case TelephonyManager.NETWORK_TYPE_IWLAN:
                return ServiceState.RIL_RADIO_TECHNOLOGY_IWLAN;
            case TelephonyManager.NETWORK_TYPE_LTE_CA:
                return ServiceState.RIL_RADIO_TECHNOLOGY_LTE_CA;
            case TelephonyManager.NETWORK_TYPE_NR:
                return ServiceState.RIL_RADIO_TECHNOLOGY_NR;
            default:
                return ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN;
        }
    }

    @Override
    public NetworkRegistrationInfo getNetworkRegistrationInfo(@Domain int domain,
                                                              @TransportType int transportType) {
        synchronized (mNetworkRegistrationInfos) {
            for (NetworkRegistrationInfo networkRegistrationInfo : mNetworkRegistrationInfos) {
                // MTK null check
                if (networkRegistrationInfo == null) {
                    // if it's null, we skip it
                    Rlog.e(LOG_TAG, "getNetworkRegistrationInfo find null nris="
                            + mNetworkRegistrationInfos);
                } else if (networkRegistrationInfo.getTransportType() == transportType
                        && networkRegistrationInfo.getDomain() == domain) {
                    return new NetworkRegistrationInfo(networkRegistrationInfo);
                }
            }
        }

        return null;
    }
}
