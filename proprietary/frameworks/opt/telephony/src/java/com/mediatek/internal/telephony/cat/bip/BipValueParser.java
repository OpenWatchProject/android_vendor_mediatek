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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony.cat;

import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.cat.Duration.TimeUnit;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.cat.ComprehensionTlv;
import com.android.internal.telephony.cat.ResultException;

import android.content.res.Resources;
import android.content.res.Resources.NotFoundException;
import java.io.UnsupportedEncodingException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;

import com.android.internal.telephony.cat.ResultCode;

abstract class BipValueParser {

    static BearerDesc retrieveBearerDesc(ComprehensionTlv ctlv) throws ResultException {
        byte[] rawValue = ctlv.getRawValue();
        int valueIndex = ctlv.getValueIndex();
        int length = ctlv.getLength();
        BearerDesc bearerDesc = null;
        GPRSBearerDesc gprsbearerDesc = null;
        EUTranBearerDesc euTranbearerDesc = null;
        UTranBearerDesc uTranbearerDesc = null;
        DefaultBearerDesc defaultbearerDesc = null;
        try {
            int bearerType = rawValue[valueIndex++] & 0xff;
            MtkCatLog.d("CAT", "retrieveBearerDesc: bearerType:"
                    +bearerType + ", length: " + length);
            if (BipUtils.BEARER_TYPE_GPRS == bearerType) {
                gprsbearerDesc = new GPRSBearerDesc();
                gprsbearerDesc.precedence = rawValue[valueIndex++] & 0xff;
                gprsbearerDesc.delay = rawValue[valueIndex++] & 0xff;
                gprsbearerDesc.reliability = rawValue[valueIndex++] & 0xff;
                gprsbearerDesc.peak = rawValue[valueIndex++] & 0xff;
                gprsbearerDesc.mean = rawValue[valueIndex++] & 0xff;
                gprsbearerDesc.pdpType = rawValue[valueIndex++] & 0xff;
                return gprsbearerDesc;
            } else if (BipUtils.BEARER_TYPE_UTRAN == bearerType) {
                uTranbearerDesc = new UTranBearerDesc();
                uTranbearerDesc.trafficClass = rawValue[valueIndex++] & 0xff;
                uTranbearerDesc.maxBitRateUL_High = rawValue[valueIndex++] & 0xff;
                uTranbearerDesc.maxBitRateUL_Low = rawValue[valueIndex++] & 0xff;
                uTranbearerDesc.maxBitRateDL_High = rawValue[valueIndex++] & 0xff;
                uTranbearerDesc.maxBitRateDL_Low = rawValue[valueIndex++] & 0xff;
                uTranbearerDesc.guarBitRateUL_High = rawValue[valueIndex++] & 0xff;
                uTranbearerDesc.guarBitRateUL_Low = rawValue[valueIndex++] & 0xff;
                uTranbearerDesc.guarBitRateDL_High = rawValue[valueIndex++] & 0xff;
                uTranbearerDesc.guarBitRateDL_Low = rawValue[valueIndex++] & 0xff;
                uTranbearerDesc.deliveryOrder = rawValue[valueIndex++] & 0xff;
                uTranbearerDesc.maxSduSize = rawValue[valueIndex++] & 0xff;
                uTranbearerDesc.sduErrorRatio = rawValue[valueIndex++] & 0xff;
                uTranbearerDesc.residualBitErrorRadio = rawValue[valueIndex++] & 0xff;
                uTranbearerDesc.deliveryOfErroneousSdus = rawValue[valueIndex++] & 0xff;
                uTranbearerDesc.transferDelay = rawValue[valueIndex++] & 0xff;
                uTranbearerDesc.trafficHandlingPriority = rawValue[valueIndex++] & 0xff;
                uTranbearerDesc.pdpType = rawValue[valueIndex++] & 0xff;
                return uTranbearerDesc;
            } else if (BipUtils.BEARER_TYPE_EUTRAN == bearerType) {
                euTranbearerDesc = new EUTranBearerDesc();
                euTranbearerDesc.QCI = rawValue[valueIndex++] & 0xff;
                euTranbearerDesc.maxBitRateU = rawValue[valueIndex++] & 0xff;
                euTranbearerDesc.maxBitRateD = rawValue[valueIndex++] & 0xff;
                euTranbearerDesc.guarBitRateU = rawValue[valueIndex++] & 0xff;
                euTranbearerDesc.guarBitRateD = rawValue[valueIndex++] & 0xff;
                euTranbearerDesc.maxBitRateUEx = rawValue[valueIndex++] & 0xff;
                euTranbearerDesc.maxBitRateDEx = rawValue[valueIndex++] & 0xff;
                euTranbearerDesc.guarBitRateUEx = rawValue[valueIndex++] & 0xff;
                euTranbearerDesc.guarBitRateDEx = rawValue[valueIndex++] & 0xff;
                euTranbearerDesc.pdnType = rawValue[valueIndex++] & 0xff;
                return euTranbearerDesc;
            } else if (BipUtils.BEARER_TYPE_DEFAULT == bearerType) {
                defaultbearerDesc = new DefaultBearerDesc();
                return defaultbearerDesc;
            } else if (BipUtils.BEARER_TYPE_CSD == bearerType) {
                MtkCatLog.d("CAT", "retrieveBearerDesc: unsupport CSD");
                throw new ResultException(ResultCode.BEYOND_TERMINAL_CAPABILITY);
            } else {
                MtkCatLog.d("CAT", "retrieveBearerDesc: un-understood bearer type");
                throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
            }
        } catch (IndexOutOfBoundsException e) {
            MtkCatLog.d("CAT", "retrieveBearerDesc: out of bounds");
            throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
        }
    }

    static int retrieveBufferSize(ComprehensionTlv ctlv) throws ResultException {
        byte[] rawValue = ctlv.getRawValue();
        int valueIndex = ctlv.getValueIndex();
        int size = 0;

        try {
            size = ((rawValue[valueIndex] & 0xff) << 8) + (rawValue[valueIndex + 1] & 0xff);
        } catch (IndexOutOfBoundsException e) {
            MtkCatLog.d("CAT", "retrieveBufferSize: out of bounds");
            throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
        }

        return size;
    }

        static String retrieveNetworkAccessName(ComprehensionTlv ctlv) throws ResultException {
            byte[] rawValue = ctlv.getRawValue();
            int valueIndex = ctlv.getValueIndex();
            String networkAccessName = null;

            try {
            // int len = ctlv.getLength() - ctlv.getValueIndex() + 1;
                int totalLen = ctlv.getLength();
                String stkNetworkAccessName = new String(rawValue, valueIndex, totalLen);
                String stkNetworkIdentifier = null;
                String stkOperatorIdentifier = null;

                if (stkNetworkAccessName != null && totalLen > 0) {
                //Get network identifier
                    int len = rawValue[valueIndex++];
                    if (totalLen > len) {
                          stkNetworkIdentifier = new String(rawValue, valueIndex, len);
                          valueIndex += len;
                    }
                    MtkCatLog.d("CAT", "totalLen:" + totalLen + ";" + valueIndex + ";" + len);
                    //Get operator identififer
                    String tmp_string = null;
                    while (totalLen > (len + 1)) {
                        totalLen -= (len + 1);
                        len = rawValue[valueIndex++];
                        MtkCatLog.d("CAT", "next len: " + len);
                        if (totalLen > len) {
                            tmp_string = new String(rawValue, valueIndex, len);
                            if (stkOperatorIdentifier == null)
                                stkOperatorIdentifier = tmp_string;
                            else
                                stkOperatorIdentifier = stkOperatorIdentifier + "." + tmp_string;
                            tmp_string = null;
                        }
                        valueIndex += len;
                        MtkCatLog.d("CAT", "totalLen:" + totalLen + ";" + valueIndex + ";" + len);
                    }

                    if (stkNetworkIdentifier != null && stkOperatorIdentifier != null) {
                        networkAccessName = stkNetworkIdentifier + "." + stkOperatorIdentifier;
                    } else if (stkNetworkIdentifier != null) {
                        networkAccessName = stkNetworkIdentifier;
                    }
                    MtkCatLog.d("CAT", "nw:" + stkNetworkIdentifier + ";" + stkOperatorIdentifier);
                }
            } catch (IndexOutOfBoundsException e) {
                MtkCatLog.d("CAT", "retrieveNetworkAccessName: out of bounds");
                throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
            }

            return networkAccessName;
        }

    static TransportProtocol retrieveTransportProtocol(ComprehensionTlv ctlv)
            throws ResultException {
        byte[] rawValue = ctlv.getRawValue();
        int valueIndex = ctlv.getValueIndex();
        int protocolType = 0;
        int portNumber = 0;

        try {
            protocolType = rawValue[valueIndex++];
            portNumber = ((rawValue[valueIndex] & 0xff) << 8) + (rawValue[valueIndex + 1] & 0xff);
        } catch (IndexOutOfBoundsException e) {
            MtkCatLog.d("CAT", "retrieveTransportProtocol: out of bounds");
            throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
        }

        return new TransportProtocol(protocolType, portNumber);
    }

    static OtherAddress retrieveOtherAddress(ComprehensionTlv ctlv) throws ResultException {
        byte[] rawValue = ctlv.getRawValue();
        int valueIndex = ctlv.getValueIndex();
        int addressType = 0;
        OtherAddress otherAddress = null;

        try {
            addressType = rawValue[valueIndex++];
            if (BipUtils.ADDRESS_TYPE_IPV4 == addressType) {
                otherAddress = new OtherAddress(addressType, rawValue, valueIndex);
            } else if (BipUtils.ADDRESS_TYPE_IPV6 == addressType) {
                otherAddress = new OtherAddress(addressType, rawValue, valueIndex);
                // throw new
                // ResultException(ResultCode.BEYOND_TERMINAL_CAPABILITY);
            } else {
                return null;
                // throw new
                // ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
            }
        } catch (IndexOutOfBoundsException e) {
            MtkCatLog.d("CAT", "retrieveOtherAddress: out of bounds");
            return null;
            // throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
        } catch (UnknownHostException e2) {
            MtkCatLog.d("CAT", "retrieveOtherAddress: unknown host");
            return null;
            // throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
        }

        return otherAddress;
    }

    static int retrieveChannelDataLength(ComprehensionTlv ctlv) throws ResultException {
        byte[] rawValue = ctlv.getRawValue();
        int valueIndex = ctlv.getValueIndex();
        int length = 0;

        MtkCatLog.d("CAT", "valueIndex:" + valueIndex);

        try {
            length = rawValue[valueIndex] & 0xFF;
        } catch (IndexOutOfBoundsException e) {
            MtkCatLog.d("CAT", "retrieveTransportProtocol: out of bounds");
            throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
        }

        return length;
    }

    static byte[] retrieveChannelData(ComprehensionTlv ctlv) throws ResultException {
        byte[] rawValue = ctlv.getRawValue();
        int valueIndex = ctlv.getValueIndex();
        byte[] channelData = null;

        try {
            channelData = new byte[ctlv.getLength()];
            System.arraycopy(rawValue, valueIndex, channelData, 0, channelData.length);
        } catch (IndexOutOfBoundsException e) {
            MtkCatLog.d("CAT", "retrieveChannelData: out of bounds");
            throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
        }

        return channelData;
    }

    static byte[] retrieveNextActionIndicator(ComprehensionTlv ctlv) throws ResultException {
        byte[] nai;

        byte[] rawValue = ctlv.getRawValue();
        int valueIndex = ctlv.getValueIndex();
        int length = ctlv.getLength();

        nai = new byte[length];
        try {
            for (int index = 0; index < length; ) {
                nai[index++] = rawValue[valueIndex++];
            }
        } catch (IndexOutOfBoundsException e) {
            throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
        }

        return nai;
    }
}
