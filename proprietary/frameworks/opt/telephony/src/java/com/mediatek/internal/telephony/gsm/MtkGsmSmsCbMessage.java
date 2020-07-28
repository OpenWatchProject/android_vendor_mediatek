/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

package com.mediatek.internal.telephony.gsm;

import android.content.Context;
import android.telephony.Rlog;
import android.telephony.SmsCbLocation;
import android.telephony.SmsCbMessage;
import android.util.Pair;

import com.android.internal.telephony.gsm.GsmSmsCbMessage;
import com.android.internal.telephony.gsm.SmsCbHeader;
import com.android.internal.telephony.uicc.IccUtils;

import com.mediatek.internal.telephony.gsm.cbutil.Circle;
import com.mediatek.internal.telephony.gsm.cbutil.Polygon;
import com.mediatek.internal.telephony.gsm.cbutil.Shape;
import com.mediatek.internal.telephony.gsm.cbutil.Vertex;
import com.mediatek.internal.telephony.gsm.cbutil.WhamTuple;
import com.mediatek.internal.telephony.MtkSmsCbMessage;

import java.util.ArrayList;
import java.util.Arrays;

/**
 * Parses a UMTS format SMS-CB message into an MtkSmsCbMessage object.
 */
public class MtkGsmSmsCbMessage {
    private static final String TAG = "MtkGsmSmsCbMessage";

    public MtkGsmSmsCbMessage() {}

    /**
     * Create a new SmsCbMessage object from a header object plus one or more received PDUs.
     *
     * @param pdus PDU bytes
     */
    public static SmsCbMessage createSmsCbMessage(Context context, SmsCbHeader header,
                                                  SmsCbLocation location, byte[][] pdus)
            throws IllegalArgumentException {
        int pageCount = header.getNumberOfPages();
        String language = null;
        StringBuilder sb = new StringBuilder();
        MtkSmsCbHeader mtkHeader = (MtkSmsCbHeader)header;
        int priority = mtkHeader.isEmergencyMessage() ?
                    SmsCbMessage.MESSAGE_PRIORITY_EMERGENCY
                    : SmsCbMessage.MESSAGE_PRIORITY_NORMAL;
        byte[] wac = null;

        if (pageCount > 1) {
            // wea 2.0 cb with more segments
            for (byte[] pdu : pdus) {
                Pair<String, String> p = GsmSmsCbMessage.parseBody(header, pdu);
                language = p.first;
                sb.append(p.second);
            }
         } else {
            // only one segment
            if (mtkHeader.isWHAMMessage()) {
                wac = getWhamData(header, pdus[0]);
            } else {
                Pair<String, String> p = GsmSmsCbMessage.parseBody(header, pdus[0]);
                language = p.first;
                sb.append(p.second);

                wac = getWacData(header, pdus[0]);
            }
         }

         return new MtkSmsCbMessage(SmsCbMessage.MESSAGE_FORMAT_3GPP,
                    header.getGeographicalScope(), header.getSerialNumber(), location,
                    header.getServiceCategory(), language, sb.toString(), priority,
                    header.getEtwsInfo(), header.getCmasInfo(), wac);
    }

     /**
     * get Wham raw Data .
     * After completing successfully this method will have assigned the Wham raw data
     * into mWac
     *
     * @param header the message header to use
     * @param pdu the PDU to decode
     * @return the WAC raw data
     */
    private static byte[] getWhamData(SmsCbHeader header, byte[] pdu) {
        int offset = SmsCbHeader.PDU_HEADER_LENGTH + 1;
        int length = pdu[offset + GsmSmsCbMessage.PDU_BODY_PAGE_LENGTH];
        byte[] wham = Arrays.copyOfRange(pdu, offset, offset + length);
        Rlog.d(TAG, "getWhamData length=" + length);
        return wham;
    }

     /**
     * get WAC raw Data .
     * After completing successfully this method will have assigned the WAC raw data
     * into mWac
     *
     * @param header the message header to use
     * @param pdu the PDU to decode
     * @return the WAC raw data
     */
    private static byte[] getWacData(SmsCbHeader header, byte[] pdu) {
        if (!header.isUmtsFormat()) {
            return null;
        }
        int nrPages = pdu[SmsCbHeader.PDU_HEADER_LENGTH];
        int offset = SmsCbHeader.PDU_HEADER_LENGTH + 1 +
                (GsmSmsCbMessage.PDU_BODY_PAGE_LENGTH + 1) * nrPages;
        if (offset < pdu.length - 1) {
            int length = ((pdu[offset + 1] & 0xFF) << 8) | (pdu[offset] & 0xFF);
            byte[] wac = Arrays.copyOfRange(pdu, offset + 2, offset + 2 + length);
            Rlog.d(TAG, "WAC length = " + length);
            return wac;
        } else {
            Rlog.d(TAG, "No WAC info.");
            return null;
        }
    }

    public static ArrayList<Shape> parseWac(MtkSmsCbMessage msg) {
        ArrayList<Shape> result = new ArrayList<Shape>();
        byte[] wacBytes = msg.getWac();
        if (wacBytes == null) {
            return result;
        }
        int len = wacBytes.length;
        int i = 0;

        while (i < len) {
            int tag = (wacBytes[i] & 0xFF) >>> 4;
            switch (tag) {
                case 0x01: {
                    int maxWaitTime = wacBytes[i + 2] & 0xFF;
                    Rlog.d(TAG, "maxWaitTime = " + maxWaitTime);
                    msg.setMaxWaitTime(maxWaitTime);
                    i += 3;
                    break;
                }
                case 0x02: {
                    int polyLen = ((wacBytes[i] & 0x0F) << 6) | ((wacBytes[i + 1] & 0xFF) >>> 2);
                    Polygon polygon = new Polygon();

                    int count = (polyLen - 2) / 11;
                    i += 2;
                    for (int k = 0; k < count; k++) {
                        int lati1 = ((wacBytes[i] & 0xFF) << 14) | ((wacBytes[i + 1] & 0xFF) << 6) |
                               ((wacBytes[i + 2] & 0xFC) >>> 2);
                        int longi1 = ((wacBytes[i + 2] & 0x03) << 20) |
                                ((wacBytes[i + 3] & 0xFF) << 12) | ((wacBytes[i + 4] & 0xFF) << 4) |
                                ((wacBytes[i + 5] & 0xF0) >>> 4);
                        polygon.addVertex(
                                new Vertex(lati1 * 180.0 / 4194304 - 90,
                                longi1 * 360.0 / 4194304 - 180));

                        int lati2 = ((wacBytes[i + 5] & 0x0F) << 18) |
                                ((wacBytes[i + 6] & 0xFF) << 10) | ((wacBytes[i + 7] & 0xFF) << 2) |
                                ((wacBytes[i + 8] & 0xC0) >>> 6);
                        int longi2 = ((wacBytes[i + 8] & 0x3F) << 16) |
                                ((wacBytes[i + 9] & 0xFF) << 8) | (wacBytes[i + 10] & 0xFF);
                        polygon.addVertex(
                                new Vertex(lati2 * 180.0 / 4194304 - 90,
                                longi2 * 360.0 / 4194304 - 180));

                        i += 11;
                    }

                    // odd
                    if ((polyLen - 2) % 11 != 0) {
                        int lati1 = ((wacBytes[i] & 0xFF) << 14) | ((wacBytes[i + 1] & 0xFF) << 6) |
                               ((wacBytes[i + 2] & 0xFC) >>> 2);
                        int longi1 = ((wacBytes[i + 2] & 0x03) << 20) |
                                ((wacBytes[i + 3] & 0xFF) << 12) | ((wacBytes[i + 4] & 0xFF) << 4) |
                                ((wacBytes[i + 5] & 0xF0) >>> 4);
                        polygon.addVertex(
                                new Vertex(lati1 * 180.0 / 4194304 - 90,
                                longi1 * 360.0 / 4194304 - 180));
                        i += 6;
                    }
                    result.add(polygon);
                    break;
                }
                case 0x03: {
                    int lati = ((wacBytes[i + 2] & 0xFF) << 14) | ((wacBytes[i + 3] & 0xFF) << 6) |
                            ((wacBytes[i + 4] & 0xFC) >>> 2);
                    int longi = ((wacBytes[i + 4] & 0x03) << 20) |
                            ((wacBytes[i + 5] & 0xFF) << 12) | ((wacBytes[i + 6] & 0xFF) << 4) |
                            ((wacBytes[i + 7] & 0xF0) >>> 4);
                    int radius = ((wacBytes[i + 7] & 0x0F) << 16) |
                            ((wacBytes[i + 8] & 0xFF) << 8) |
                            (wacBytes[i + 9] & 0xFF);
                    Circle circle = new Circle(
                            new Vertex(lati * 180.0 / 4194304 - 90, longi * 360.0 / 4194304 - 180),
                            radius / 64.0);
                    result.add(circle);
                    i += 10;
                    break;
                }
                default: {
                    Rlog.d(TAG, "not expected tag:" + tag);
                    i = len;
                    break;
                }
            }
        }

        for (Shape item : result) {
            Rlog.d(TAG, "result=" + item.toString());
        }

        return result;
    }

    public static ArrayList<ArrayList<WhamTuple>> parseWHAMTupleList(MtkSmsCbMessage msg) {
        ArrayList<ArrayList<WhamTuple>> result = new ArrayList<ArrayList<WhamTuple>>();
        byte[] whamBytes = msg.getWac();
        if (whamBytes == null) {
            return result;
        }
        int len = whamBytes.length;
        int i = 0;

        ArrayList<WhamTuple> singleList = null;

        while (i < len) {
            int tag = (whamBytes[i] & 0xFF) >>> 4;
            switch (tag) {
                case 0x01: {
                    if (singleList == null) {
                        singleList = new ArrayList<WhamTuple>();
                    }
                    int length = ((whamBytes[i] & 0x0F) << 3) | ((whamBytes[i + 1] & 0xFF) >>> 5);
                    int count = (length - 2) / 4;
                    i += 2;
                    for (int k = 0; k < count; k++) {
                        int msgId = ((whamBytes[i] & 0xFF) << 8) | (whamBytes[i + 1] & 0xFF);
                        int serialNumber = ((whamBytes[i + 2] & 0xFF) << 8) |
                                (whamBytes[i + 3] & 0xFF);
                        WhamTuple tuple = new WhamTuple(WhamTuple.WHAM_TUPLE_TYPE_UNIQUE,
                                msgId, serialNumber);
                        singleList.add(tuple);
                        i += 4;
                    }
                    break;
                }
                case 0x02: {
                    ArrayList<WhamTuple> commonList = new ArrayList<WhamTuple>();
                    int length = ((whamBytes[i] & 0x0F) << 3) | ((whamBytes[i + 1] & 0xFF) >>> 5);
                    int count = (length - 2) / 4;
                    i += 2;
                    for (int k = 0; k < count; k++) {
                        int msgId = ((whamBytes[i] & 0xFF) << 8) | (whamBytes[i + 1] & 0xFF);
                        int serialNumber = ((whamBytes[i + 2] & 0xFF) << 8) |
                                (whamBytes[i + 3] & 0xFF);
                        WhamTuple tuple = new WhamTuple(WhamTuple.WHAM_TUPLE_TYPE_SHARE,
                                msgId, serialNumber);
                        commonList.add(tuple);
                        i += 4;
                    }
                    result.add(commonList);
                    break;
                }
                default: {
                    Rlog.d(TAG, "not expected tag:" + tag);
                    i = len;
                    break;
                }
            }
        }

        if (singleList != null) {
            result.add(singleList);
        }

        for (ArrayList<WhamTuple> onelist : result) {
            for (WhamTuple item : onelist) {
                Rlog.d(TAG, "result=" + item.toString());
            }
        }
        return result;
    }
}
