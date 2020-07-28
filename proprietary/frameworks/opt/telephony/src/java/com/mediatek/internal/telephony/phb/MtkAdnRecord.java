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
package com.mediatek.internal.telephony.phb;

import android.os.Parcel;
import android.os.Parcelable;
import android.telephony.Rlog;
import android.text.TextUtils;

import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.IccInternalInterface;
import com.android.internal.telephony.uicc.AdnRecord;
import com.android.internal.telephony.uicc.IccUtils;
import com.mediatek.internal.telephony.MtkPhoneNumberUtils;

import java.io.UnsupportedEncodingException;
import java.util.regex.Pattern;



/**
 *
 * Used to load or store ADNs (Abbreviated Dialing Numbers).
 *
 * {@hide}
 *
 */
public class MtkAdnRecord extends AdnRecord {
    static final String LOG_TAG = "MtkAdnRecord";

    //***** Instance Variables

    String mAdditionalNumber = null;
    String mAdditionalNumber2 = null;
    String mAdditionalNumber3 = null;
    String mGrpIds;
    // The index of aas
    int mAas = 0;
    String mSne = null;

    int mResult = IccInternalInterface.ERROR_ICC_PROVIDER_SUCCESS;

    //***** Constants

    private static final String SIM_NUM_PATTERN = "[+]?[[0-9][*#pw,;]]+[[0-9][*#pw,;]]*";

    //***** Static Methods

    public static final Parcelable.Creator<MtkAdnRecord> CREATOR
            = new Parcelable.Creator<MtkAdnRecord>() {
        @Override
        public MtkAdnRecord createFromParcel(Parcel source) {
            int efid;
            int recordNumber;
            String alphaTag;
            String number;
            String anr;
            String anr2;
            String anr3;
            String grpIds;
            String[] emails;

            efid = source.readInt();
            recordNumber = source.readInt();
            alphaTag = source.readString();
            number = source.readString();
            emails = source.readStringArray();
            anr = source.readString();
            anr2 = source.readString();
            anr3 = source.readString();
            grpIds = source.readString();
            int aas = source.readInt();
            String sne = source.readString();
            MtkAdnRecord adn = new MtkAdnRecord(efid, recordNumber, alphaTag, number, anr, anr2,
                                          anr3, emails, grpIds);
            adn.setAasIndex(aas);
            adn.setSne(sne);
            return adn;
        }

        @Override
        public MtkAdnRecord[] newArray(int size) {
            return new MtkAdnRecord[size];
        }
    };


    //***** Constructor
    public MtkAdnRecord(byte[] record) {
        super(record);
    }

    public MtkAdnRecord(int efid, int recordNumber, byte[] record) {
        super(efid, recordNumber, record);
    }

    public MtkAdnRecord(String alphaTag, String number) {
        super(alphaTag, number);
    }

    public MtkAdnRecord(String alphaTag, String number, String anr) {
        this(0, 0, alphaTag, number, anr);
    }

    public MtkAdnRecord(String alphaTag, String number, String[] emails) {
        super(alphaTag, number, emails);
    }

    public MtkAdnRecord(int efid, int recordNumber, String alphaTag, String number,
            String[] emails) {
        super(efid, recordNumber, alphaTag, number, emails);
        this.mAdditionalNumber = "";
        this.mAdditionalNumber2 = "";
        this.mAdditionalNumber3 = "";
        this.mGrpIds = null;
    }

    public MtkAdnRecord(int efid, int recordNumber, String alphaTag, String number) {
        super(efid, recordNumber, alphaTag, number, null);
        this.mAdditionalNumber = "";
        this.mAdditionalNumber2 = "";
        this.mAdditionalNumber3 = "";
        this.mGrpIds = null;
    }

    public MtkAdnRecord(int efid, int recordNumber, String alphaTag, String number, String anr) {
        super(efid, recordNumber, alphaTag, number, null);
        this.mAdditionalNumber = anr;
        this.mAdditionalNumber2 = "";
        this.mAdditionalNumber3 = "";
        this.mGrpIds = null;
    }

    public MtkAdnRecord(int efid, int recordNumber, String alphaTag, String number, String anr,
            String[] emails, String grps) {
        super(efid, recordNumber, alphaTag, number, emails);
        this.mAdditionalNumber = anr;
        this.mAdditionalNumber2 = "";
        this.mAdditionalNumber3 = "";
        this.mGrpIds = grps;
    }

    public MtkAdnRecord(int efid, int recordNumber, String alphaTag, String number, String anr,
            String anr2, String anr3, String[] emails, String grps) {
        super(efid, recordNumber, alphaTag, number, emails);
        this.mAdditionalNumber = anr;
        this.mAdditionalNumber2 = anr2;
        this.mAdditionalNumber3 = anr3;
        this.mGrpIds = grps;
    }

    //***** Instance Methods
    public String getAdditionalNumber() {
        return mAdditionalNumber;
    }

    public String getAdditionalNumber(int index) {
        String number = null;
        if (index == 0) {
            number = this.mAdditionalNumber;
        } else if (index == 1) {
            number = this.mAdditionalNumber2;
        } else if (index == 2) {
            number = this.mAdditionalNumber3;
        } else {
            Rlog.e(LOG_TAG, "getAdditionalNumber Error:" + index);
    }
        return number;
    }

    public int getAasIndex() {
        return mAas;
    }

    public String getSne() {
        return mSne;
    }

    public String getGrpIds() {
        return mGrpIds;
    }

    public void setAnr(String anr) {
        this.mAdditionalNumber = anr;
    }

    public void setAnr(String anr, int index) {
        if (index == 0) {
            this.mAdditionalNumber = anr;
        } else if (index == 1) {
            this.mAdditionalNumber2 = anr;
        } else if (index == 2) {
            this.mAdditionalNumber3 = anr;
        } else {
            Rlog.e(LOG_TAG, "setAnr Error:" + index);
        }
    }

    public void setAasIndex(int aas) {
        this.mAas = aas;
    }

    public void setSne(String sne) {
        this.mSne = sne;
    }

    public void setGrpIds(String grps) {
        this.mGrpIds = grps;
    }

    public void setRecordIndex(int nIndex) {
        this.mRecordNumber = nIndex;
    }

    private String getMaskString(String str) {
        if (str == null) {
            return "null";
        } else if (str.length() > 2) {
            return str.substring(0, str.length() >> 1) + "xxxxx";
        } else {
            return "xx";
        }
    }

    @Override
    public String toString() {
        return "ADN Record:" + mRecordNumber
                + ",alphaTag:" + getMaskString(mAlphaTag)
                + ",number:" + getMaskString(mNumber)
                + ",aas:" + mAas
                + ",emails:" + ((mEmails == null) ? "null" : getMaskString(mEmails[0]))
                + ",grpIds:" + mGrpIds
                + ",sne:" + mSne;
    }

    public boolean isEmpty() {
        return TextUtils.isEmpty(mAlphaTag) && TextUtils.isEmpty(mNumber)
                && TextUtils.isEmpty(mAdditionalNumber) && mEmails == null;
    }

    public boolean isEqual(MtkAdnRecord adn) {
        return (stringCompareNullEqualsEmpty(mAlphaTag, adn.mAlphaTag) &&
                stringCompareNullEqualsEmpty(mNumber, adn.mNumber));
    }

    //***** Parcelable Implementation

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        super.writeToParcel(dest, flags);
        dest.writeString(mAdditionalNumber);
        dest.writeString(mAdditionalNumber2);
        dest.writeString(mAdditionalNumber3);
        dest.writeString(mGrpIds);
        dest.writeInt(mAas);
        dest.writeString(mSne);
    }

    /**
     * Build adn hex byte array based on record size
     * The format of byte array is defined in 51.011 10.5.1
     *
     * @param recordSize is the size X of EF record
     * @return hex byte[recordSize] to be written to EF record
     *          return null for wrong format of dialing number or tag
     */
    public byte[] buildAdnString(int recordSize) {
        Rlog.i(LOG_TAG, "in BuildAdnString");
        byte[] bcdNumber;
        byte[] byteTag;
        byte[] adnString;
        int footerOffset = recordSize - FOOTER_SIZE_BYTES;
        int alphaIdLength = 0;

        // create an empty record
        adnString = new byte[recordSize];
        for (int i = 0; i < recordSize; i++) {
            adnString[i] = (byte) 0xFF;
        }
        if (isPhoneNumberInvaild(mNumber)) {
            Rlog.w(LOG_TAG, "[buildAdnString] invaild number");
            mResult = IccInternalInterface.ERROR_ICC_PROVIDER_WRONG_ADN_FORMAT;
            return null;
        }
        if (TextUtils.isEmpty(mNumber)) {
            Rlog.w(LOG_TAG, "[buildAdnString] Empty dialing number");
            mResult = IccInternalInterface.ERROR_ICC_PROVIDER_SUCCESS;
            //return adnString;   // return the empty record (for delete)
        } else if (mNumber.length()
                > (ADN_DIALING_NUMBER_END - ADN_DIALING_NUMBER_START + 1) * 2) {
            mResult = IccInternalInterface.ERROR_ICC_PROVIDER_NUMBER_TOO_LONG;
            Rlog.w(LOG_TAG,
                    "[buildAdnString] Max length of dialing number is 20");
            return null;
        } else if (mAlphaTag != null && mAlphaTag.length() > footerOffset) {
            mResult = IccInternalInterface.ERROR_ICC_PROVIDER_TEXT_TOO_LONG;
            Rlog.w(LOG_TAG,
                    "[buildAdnString] Max length of tag is " + footerOffset);
            return null;
        } else {
            mResult = IccInternalInterface.ERROR_ICC_PROVIDER_SUCCESS;
            try {
                bcdNumber = MtkPhoneNumberUtils.numberToCalledPartyBCD(mNumber);
                if (bcdNumber == null) return null;
            } catch (RuntimeException exc) {
                CommandException cmdEx =
                  new CommandException(CommandException.Error.OEM_ERROR_12);
                throw new RuntimeException("invalid number for BCD ", cmdEx);
            }
            System.arraycopy(bcdNumber, 0, adnString,
                    footerOffset + ADN_TON_AND_NPI, bcdNumber.length);

            adnString[footerOffset + ADN_BCD_NUMBER_LENGTH]
                    = (byte) (bcdNumber.length);
            adnString[footerOffset + ADN_CAPABILITY_ID]
                    = (byte) 0xFF; // Capability Id
            adnString[footerOffset + ADN_EXTENSION_ID]
                    = (byte) 0xFF; // Extension Record Id
        }

        if (!TextUtils.isEmpty(mAlphaTag)) {
            if (isContainChineseChar(mAlphaTag)) {
                Rlog.i(LOG_TAG, "[buildAdnString] getBytes,alphaTag:" + mAlphaTag);
                try {
                    Rlog.i(LOG_TAG, "call getBytes");
                    byteTag = mAlphaTag.getBytes("utf-16be");
                    Rlog.i(LOG_TAG, "byteTag," + IccUtils.bytesToHexString(byteTag));
                } catch (UnsupportedEncodingException ex) {
                    Rlog.w(LOG_TAG, "[buildAdnString] getBytes exception");
                    return null;
                }
                byte[] header = new byte[1];
                header[0] = (byte) 0x80;
                System.arraycopy(header, 0, adnString, 0, 1);
                if (byteTag.length > adnString.length - 1) {
                      mResult = IccInternalInterface.ERROR_ICC_PROVIDER_TEXT_TOO_LONG;
                    Rlog.w(LOG_TAG, "[buildAdnString] after getBytes byteTag.length:" +
                        byteTag.length + " adnString.length:" + adnString.length);
                    return null;
                }
                System.arraycopy(byteTag, 0, adnString, 1, byteTag.length);
                alphaIdLength = byteTag.length + 1;
                Rlog.i(LOG_TAG, "arrarString" + IccUtils.bytesToHexString(adnString));
            } else {
                Rlog.i(LOG_TAG, "[buildAdnString] stringToGsm8BitPacked");
                byteTag = GsmAlphabet.stringToGsm8BitPacked(mAlphaTag);
                alphaIdLength = byteTag.length;
                if (alphaIdLength > adnString.length) {
                      mResult = IccInternalInterface.ERROR_ICC_PROVIDER_TEXT_TOO_LONG;
                    Rlog.w(LOG_TAG, "[buildAdnString] after stringToGsm8BitPacked byteTag.length:" +
                            byteTag.length + " adnString.length:" + adnString.length);
                    return null;
                }
                System.arraycopy(byteTag, 0, adnString, 0, byteTag.length);
            }
        }

        if (mAlphaTag != null && alphaIdLength > footerOffset) {
            mResult = IccInternalInterface.ERROR_ICC_PROVIDER_TEXT_TOO_LONG;
            Rlog.w(LOG_TAG, "[buildAdnString] Max length of tag is " + footerOffset +
                ",alphaIdLength:" + alphaIdLength);
            return null;
        }

        return adnString;
    }

    /**
     * Get error code when do buildAdnString.
     *
     * @return ERROR_ICC_PROVIDER_* defined in the IccInternalInterface
     * @see IccInternalInterface
     */
    public int getErrorNumber() {
        return mResult;
    }
    /**
     * See TS 51.011 10.5.10
     */
    public void
    appendExtRecord(byte[] extRecord) {
        try {
            if (extRecord.length != EXT_RECORD_LENGTH_BYTES) {
                return;
            }

            if ((extRecord[0] & EXT_RECORD_TYPE_MASK)
                    != EXT_RECORD_TYPE_ADDITIONAL_DATA) {
                return;
            }

            if ((0xff & extRecord[1]) > MAX_EXT_CALLED_PARTY_LENGTH) {
                // invalid or empty record
                return;
            }

            mNumber += MtkPhoneNumberUtils.calledPartyBCDFragmentToString(
                                        extRecord, 2, 0xff & extRecord[1]);

            // We don't support ext record chaining.

        } catch (RuntimeException ex) {
            Rlog.w(LOG_TAG, "Error parsing AdnRecordEx ext record", ex);
        }
    }

    //***** Private Methods

    /**
     * alphaTag and number are set to null on invalid format.
     */
    private void
    parseRecord(byte[] record) {
        try {
            mAlphaTag = IccUtils.adnStringFieldToString(
                            record, 0, record.length - FOOTER_SIZE_BYTES);

            int footerOffset = record.length - FOOTER_SIZE_BYTES;

            int numberLength = 0xff & record[footerOffset];

            if (numberLength > MAX_NUMBER_SIZE_BYTES) {
                // Invalid number length
                mNumber = "";
                return;
            }

            // Please note 51.011 10.5.1:
            //
            // "If the Dialling Number/SSC String does not contain
            // a dialling number, e.g. a control string deactivating
            // a service, the TON/NPI byte shall be set to 'FF' by
            // the ME (see note 2)."

            mNumber = MtkPhoneNumberUtils.calledPartyBCDToString(
                            record, footerOffset + 1, numberLength);


            mExtRecord = 0xff & record[record.length - 1];

            mEmails = null;
            mAdditionalNumber = "";
            mAdditionalNumber2 = "";
            mAdditionalNumber3 = "";
            mGrpIds = null;

        } catch (RuntimeException ex) {
            Rlog.w(LOG_TAG, "Error parsing AdnRecordEx", ex);
            mNumber = "";
            mAlphaTag = "";
            mEmails = null;
            mAdditionalNumber = "";
            mAdditionalNumber2 = "";
            mAdditionalNumber3 = "";
            mGrpIds = null;
        }
    }
    /// M: judge alphat whether contains chinese character
    private boolean isContainChineseChar(String alphTag) {
        boolean result = false;
        int length = alphTag.length();

        for (int i = 0; i < length; i++) {
            if (Pattern.matches("[\u4E00-\u9FA5]", alphTag.substring(i, i + 1))) {
                result = true;
                break;
            }
        }

        return result;
    }
    private boolean isPhoneNumberInvaild(String phoneNumber) {
        String tempPhoneNumber = null;
        if (!TextUtils.isEmpty(phoneNumber)) {
            tempPhoneNumber = MtkPhoneNumberUtils.stripSeparators(phoneNumber);

            if (!Pattern.matches(SIM_NUM_PATTERN,
                    MtkPhoneNumberUtils.extractCLIRPortion(tempPhoneNumber))) {
                return true;
            }
        }
        return false;
    }
}
