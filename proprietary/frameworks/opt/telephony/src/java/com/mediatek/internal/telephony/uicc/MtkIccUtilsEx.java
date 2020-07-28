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

package com.mediatek.internal.telephony.uicc;

import android.os.SystemProperties;
import android.telephony.Rlog;
import android.text.TextUtils;

import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.cdma.sms.UserData;
import com.android.internal.telephony.gsm.SimTlv;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.PersoSubState;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.UiccController;

import java.nio.charset.Charset;
import java.util.Arrays;
/**
 * Various methods, useful for dealing with SIM data.
 */
public class MtkIccUtilsEx extends IccUtils {
    static final String MTK_LOG_TAG = "MtkIccUtilsEx";

    // Full Name IEI from TS 24.008
    protected static final int TAG_FULL_NETWORK_NAME = 0x43;

    // From 3GPP TS 31.103
    private static final int TAG_ISIM_VALUE = 0x80;

    // CDMA 3G card type define
    public static final int CDMA_CARD_TYPE_NOT_3GCARD = 0;
    public static final int CDMA_CARD_TYPE_UIM_ONLY = 1;
    public static final int CDMA_CARD_TYPE_RUIM_SIM = 2;

    protected static final String[]  PROPERTY_RIL_FULL_UICC_TYPE = {
        "vendor.gsm.ril.fulluicctype",
        "vendor.gsm.ril.fulluicctype.2",
        "vendor.gsm.ril.fulluicctype.3",
        "vendor.gsm.ril.fulluicctype.4",
    };

    protected static final String[] PROPERTY_RIL_CT3G = {
        "vendor.gsm.ril.ct3g",
        "vendor.gsm.ril.ct3g.2",
        "vendor.gsm.ril.ct3g.3",
        "vendor.gsm.ril.ct3g.4",
    };

    /**
     * Parse SPN byte array to string according to indicated family and raw data.
     *
     * @param family refer to UiccController.APP_FAM_3GPP, UiccController.APP_FAM_3GPP2
     * @param data EF_SPN raw data
     * @return parsed EF_SPN data, string type
     *
     */
    public static String parseSpnToString(int family, byte[] data) {
        if (data == null) {
            return null;
        }

        if (UiccController.APP_FAM_3GPP == family) {
            return IccUtils.adnStringFieldToString(data, 1, data.length - 1);
        } else if (UiccController.APP_FAM_3GPP2 == family) {
            int encoding = data[1];
            int language = data[2];
            byte[] spnData = new byte[32];
            int len = ((data.length - 3) < 32) ? (data.length - 3) : 32;
            System.arraycopy(data, 3, spnData, 0, len);

            int numBytes;
            for (numBytes = 0; numBytes < spnData.length; numBytes++) {
                if ((spnData[numBytes] & 0xFF) == 0xFF) {
                    break;
                }
            }

            if (numBytes == 0) {
                return "";
            }
            try {
                switch (encoding) {
                case UserData.ENCODING_OCTET:
                case UserData.ENCODING_LATIN:
                    return (new String(spnData, 0, numBytes, "ISO-8859-1"));
                case UserData.ENCODING_IA5:
                case UserData.ENCODING_GSM_7BIT_ALPHABET:
                    return (GsmAlphabet.gsm7BitPackedToString(spnData, 0, (numBytes * 8) / 7));
                case UserData.ENCODING_7BIT_ASCII:
                    String spn = new String(spnData, 0, numBytes, "US-ASCII");
                    // To address issues with incorrect encoding scheme
                    // programmed in some commercial CSIM cards, the decoded
                    // SPN is checked to have characters in printable ASCII
                    // range. If not, they are decoded with
                    // ENCODING_GSM_7BIT_ALPHABET scheme.
                    if (TextUtils.isPrintableAsciiOnly(spn)) {
                        return (spn);
                    } else {
                        return (GsmAlphabet.gsm7BitPackedToString(spnData, 0, (numBytes * 8) / 7));
                    }
                case UserData.ENCODING_UNICODE_16:
                    return (new String(spnData, 0, numBytes, "utf-16"));
                default:
                    Rlog.d(MTK_LOG_TAG, "spn decode error: " + encoding);
                }
            } catch (Exception e) {
                Rlog.d(MTK_LOG_TAG, "spn decode error: " + e);
            }

        }
        return null;
    }

    /**
     * Parse PNN byte array to string according to indicated raw data.
     *
     * @param data EF_PNN raw data
     * @return parsed EF_PNN data, string type
     *
     */
    public static String parsePnnToString(byte[] data) {
        if (data == null) {
            return null;
        }

        SimTlv tlv = new SimTlv(data, 0, data.length);
        for (; tlv.isValidObject(); tlv.nextObject()) {
            if (tlv.getTag() == TAG_FULL_NETWORK_NAME) {
                return networkNameToString(tlv.getData(), 0, tlv.getData().length);
            }
        }

        return null;
    }

    /** @hide
     * Parse IMPI  byte array to string according to indicated raw data.
     *
     * @param data EF_IMPI raw data
     * @return parsed IMPI data, string type
     *
     */
    public static String parseImpiToString(byte[] data) {
        if (data == null) {
            return null;
        }

        SimTlv tlv = new SimTlv(data, 0, data.length);
        do {
           if (tlv.isValidObject() && tlv.getTag() == TAG_ISIM_VALUE) {
               return new String(tlv.getData(), Charset.forName("UTF-8"));
           }
        } while (tlv.nextObject());
        Rlog.d(MTK_LOG_TAG, "[ISIM] can't find TLV. record = " + IccUtils.bytesToHexString(data));
        return null;
    }

    /**
     * Check CDMA 3g card type.
     * @param slotId current slot id.
     * @return int Uim only card, RUIM+SIM, or not CDMA 3g card.
     */
    public static int checkCdma3gCard(int slotId) {
        String prop = null;
        String values[] = null;
        int cdma3gCardType = -1;

        if (slotId < 0 || slotId >= PROPERTY_RIL_FULL_UICC_TYPE.length) {
            Rlog.d(MTK_LOG_TAG, "checkCdma3gCard: invalid slotId " + slotId);
            return cdma3gCardType;
        }

        prop = SystemProperties.get(PROPERTY_RIL_FULL_UICC_TYPE[slotId]);
        if ((prop != null) && (prop.length() > 0)) {
            values = prop.split(",");
        }

        if (values != null) {
            if (Arrays.asList(values).contains("RUIM") && Arrays.asList(values).contains("SIM")) {
                cdma3gCardType = CDMA_CARD_TYPE_RUIM_SIM;
            } else if ((!(Arrays.asList(values).contains("USIM")
                    || Arrays.asList(values).contains("SIM")))
                    || (Arrays.asList(values).contains("SIM")
                    && ("1".equals(SystemProperties.get(PROPERTY_RIL_CT3G[slotId]))))) {
                cdma3gCardType = CDMA_CARD_TYPE_UIM_ONLY;
            } else {
                cdma3gCardType = CDMA_CARD_TYPE_NOT_3GCARD;
            }
        }

        Rlog.d(MTK_LOG_TAG, "checkCdma3gCard slotId " + slotId + ", prop value = " + prop
                + ", size = " + ((values != null) ? values.length : 0) + ", cdma3gCardType = "
                + cdma3gCardType);

        return cdma3gCardType;
    }

    public static String getPrintableString(String str, int length) {
        String strToPrint = null;
        if (str != null) {
            if (str.length() > length) {
                strToPrint = str.substring(0, length) + Rlog.pii(false, str.substring(length));
            } else {
                strToPrint = str;
            }
        }
        return strToPrint;
    }

    /**
     * Get SIM ME lock persosubstate from integer.
     * @param substate for cardType index.
     * @return newSubState.
     */
    public static PersoSubState PersoSubstateFromRILInt(int substate) {
        PersoSubState newSubState;

        switch (substate) {
            case 25: newSubState = PersoSubState.PERSOSUBSTATE_SIM_NS_SP; break;
            case 26: newSubState = PersoSubState.PERSOSUBSTATE_SIM_NS_SP_PUK; break;
            case 27: newSubState = PersoSubState.PERSOSUBSTATE_SIM_SIM_C; break;
            case 28: newSubState = PersoSubState.PERSOSUBSTATE_SIM_SIM_C_PUK; break;
            default: newSubState = PersoSubState.PERSOSUBSTATE_UNKNOWN;
        }

        return newSubState;
    }
}
