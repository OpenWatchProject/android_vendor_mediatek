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


package com.mediatek.internal.telephony;

import com.android.internal.telephony.WspTypeDecoder;

import java.util.HashMap;

/**
 * Implement the WSP data type decoder.
 *
 */
public class MtkWspTypeDecoder extends WspTypeDecoder {
    // MTK-START
    public static final int CONTENT_TYPE_B_CONNECTIVITY = 0x35;
    public static final String CONTENT_MIME_TYPE_B_CONNECTIVITY =
            "application/vnd.wap.connectivity-wbxml";
    // Add For AGPS exchanging SUPL init Message between SLP(SUPL Locaiton Platform) and UE.
    // Reference to specification OMA-TS-ULP-v1.0-20070615-A.pdf page 50-51 for detailed infromtion.
    public static final String CONTENT_MIME_TYPE_B_VND_SULP_INIT =
            "application/vnd.omaloc-supl-init";
    // MTK-END

    // MTK-START
    private final static HashMap<Integer, String> WELL_KNOWN_HEADERS =
            new HashMap<Integer, String>();
    // MTK-END

    // MTK-START
    HashMap<String, String> mHeaders;
    // MTK-END

    static {
        WELL_KNOWN_HEADERS.put(0x00, "Accept");
        WELL_KNOWN_HEADERS.put(0x01, "Accept-Charset");
        WELL_KNOWN_HEADERS.put(0x02, "Accept-Encoding");
        WELL_KNOWN_HEADERS.put(0x03, "Accept-Language");
        WELL_KNOWN_HEADERS.put(0x04, "Accept-Ranges");
        WELL_KNOWN_HEADERS.put(0x05, "Age");
        WELL_KNOWN_HEADERS.put(0x06, "Allow");
        WELL_KNOWN_HEADERS.put(0x07, "Authorization");
        WELL_KNOWN_HEADERS.put(0x08, "Cache-Control");
        WELL_KNOWN_HEADERS.put(0x09, "Connection");
        WELL_KNOWN_HEADERS.put(0x0A, "Content-Base");
        WELL_KNOWN_HEADERS.put(0x0B, "Content-Encoding");
        WELL_KNOWN_HEADERS.put(0x0C, "Content-Language");
        WELL_KNOWN_HEADERS.put(0x0D, "Content-Length");
        WELL_KNOWN_HEADERS.put(0x0E, "Content-Location");
        WELL_KNOWN_HEADERS.put(0x0F, "Content-MD5");

        WELL_KNOWN_HEADERS.put(0x10, "Content-Range");
        WELL_KNOWN_HEADERS.put(0x11, "Content-Type");
        WELL_KNOWN_HEADERS.put(0x12, "Date");
        WELL_KNOWN_HEADERS.put(0x13, "Etag");
        WELL_KNOWN_HEADERS.put(0x14, "Expires");
        WELL_KNOWN_HEADERS.put(0x15, "From");
        WELL_KNOWN_HEADERS.put(0x16, "Host");
        WELL_KNOWN_HEADERS.put(0x17, "If-Modified-Since");
        WELL_KNOWN_HEADERS.put(0x18, "If-Match");
        WELL_KNOWN_HEADERS.put(0x19, "If-None-Match");
        WELL_KNOWN_HEADERS.put(0x1A, "If-Range");
        WELL_KNOWN_HEADERS.put(0x1B, "If-Unmodified-Since");
        WELL_KNOWN_HEADERS.put(0x1C, "Location");
        WELL_KNOWN_HEADERS.put(0x1D, "Last-Modified");
        WELL_KNOWN_HEADERS.put(0x1E, "Max-Forwards");
        WELL_KNOWN_HEADERS.put(0x1F, "Pragma");

        WELL_KNOWN_HEADERS.put(0x20, "Proxy-Authenticate");
        WELL_KNOWN_HEADERS.put(0x21, "Proxy-Authorization");
        WELL_KNOWN_HEADERS.put(0x22, "Public");
        WELL_KNOWN_HEADERS.put(0x23, "Range");
        WELL_KNOWN_HEADERS.put(0x24, "Referer");
        WELL_KNOWN_HEADERS.put(0x25, "Retry-After");
        WELL_KNOWN_HEADERS.put(0x26, "Server");
        WELL_KNOWN_HEADERS.put(0x27, "Transfer-Encoding");
        WELL_KNOWN_HEADERS.put(0x28, "Upgrade");
        WELL_KNOWN_HEADERS.put(0x29, "User-Agent");
        WELL_KNOWN_HEADERS.put(0x2A, "Vary");
        WELL_KNOWN_HEADERS.put(0x2B, "Via");
        WELL_KNOWN_HEADERS.put(0x2C, "Warning");
        WELL_KNOWN_HEADERS.put(0x2D, "WWW-Authenticate");
        WELL_KNOWN_HEADERS.put(0x2E, "Content-Disposition");
        WELL_KNOWN_HEADERS.put(0x2F, "X-Wap-Application-Id");

        WELL_KNOWN_HEADERS.put(0x30, "X-Wap-Content-URI");
        WELL_KNOWN_HEADERS.put(0x31, "X-Wap-Initiator-URI");
        WELL_KNOWN_HEADERS.put(0x32, "Accept-Application");
        WELL_KNOWN_HEADERS.put(0x33, "Bearer-Indication");
        WELL_KNOWN_HEADERS.put(0x34, "Push-Flag");
        WELL_KNOWN_HEADERS.put(0x35, "Profile");
        WELL_KNOWN_HEADERS.put(0x36, "Profile-Diff");
        WELL_KNOWN_HEADERS.put(0x37, "Profile-Warning");
        WELL_KNOWN_HEADERS.put(0x38, "Expect");
        WELL_KNOWN_HEADERS.put(0x39, "TE");
        WELL_KNOWN_HEADERS.put(0x3A, "Trailer");
        WELL_KNOWN_HEADERS.put(0x3B, "Accept-Charset");
        WELL_KNOWN_HEADERS.put(0x3C, "Accept-Encoding");
        WELL_KNOWN_HEADERS.put(0x3D, "Cache-Control");
        WELL_KNOWN_HEADERS.put(0x3E, "Content-Range");
        WELL_KNOWN_HEADERS.put(0x3F, "X-Wap-Tod");

        WELL_KNOWN_HEADERS.put(0x40, "Content-ID");
        WELL_KNOWN_HEADERS.put(0x41, "Set-Cookie");
        WELL_KNOWN_HEADERS.put(0x42, "Cookie");
        WELL_KNOWN_HEADERS.put(0x43, "Encoding-Version");
        WELL_KNOWN_HEADERS.put(0x44, "Profile-Warning");
        WELL_KNOWN_HEADERS.put(0x45, "Content-Disposition");
        WELL_KNOWN_HEADERS.put(0x46, "X-WAP-Security");
        WELL_KNOWN_HEADERS.put(0x47, "Cache-Control");
        WELL_KNOWN_HEADERS.put(0x48, "Expect");
        WELL_KNOWN_HEADERS.put(0x49, "X-Wap-Loc-Invocation");
        WELL_KNOWN_HEADERS.put(0x4A, "X-Wap-Loc-Delivery");
    }

    public MtkWspTypeDecoder(byte[] pdu) {
        super(pdu);
    }

    // MTK-START
    /**
     * Decode the "Headers" for WSP pdu headers
     *
     * @param startIndex The starting position of the "Headers" in this pdu
     *        headerLength The Headers Length
     *
     * @return a map of headers keyed by their names, which can be get by
     *         getHeaders().
     *
     * @Notes  The headers name may be encoded with integer value or in text format, and the
     *         well known name will be expanded to a string. If it can not be expanded to a string,
     *         the integer value will be transformed to a string.
     *
     *         The headers value may be encoded with integer value or in text format, and the
     *         integer value will be
     *         transformed to a string.
     *
     *
     */
    public void decodeHeaders(int startIndex, int headerLength) {
        mHeaders = new HashMap<String, String>();
        String headerName = null;
        String headerValue = null;
        int intValues;

        int index = startIndex;
        while (index < startIndex + headerLength) {
            decodeHeaderFieldName(index);
            index += getDecodedDataLength();
            expandWellKnownHeadersName();
            intValues = (int) mUnsigned32bit;
            if (mStringValue != null) {
                headerName = mStringValue;
            } else if (intValues >= 0) {
                headerName = String.valueOf(intValues);
            } else {
                continue;
            }

            decodeHeaderFieldValues(index);
            index += getDecodedDataLength();
            intValues = (int) mUnsigned32bit;
            if (mStringValue != null) {
                headerValue = mStringValue;
            } else if (intValues >= 0) {
                headerValue = String.valueOf(intValues);
            } else {
                continue;
            }

            mHeaders.put(headerName, headerValue);
        }
    }

    /**
     * Decode the "Field Name" for WSP pdu headers
     *
     * @param startIndex The starting position of the "Field Name" in this pdu
     *
     * @return return value can be retrieved by getValueString() or getValue32() method
     *
     * @Notes  According OMA-WAP-TS-WSP-V1_0-20020290-C,
     *         the well-known header field name should be binany-coded.
     *         Page code shifting is currently not supported.
     *
     */
    public boolean decodeHeaderFieldName(int startIndex) {
        if (decodeShortInteger(startIndex) == true) {
            mStringValue = null;
            return true;
        } else {
            return decodeTextString(startIndex);
        }
    }

    /**
     * Decode the "Field values" for WSP pdu headers
     *
     * @param startIndex The starting position of the "Field values" in this pdu
     *
     * @return return value can be retrieved by getValueString() or getValue32() method
     *
     * @Notes  See OMA-WAP-TS-WSP-V1_0-20020290-C, Page 81, Field values.
     *
     *
     */
    public boolean decodeHeaderFieldValues(int startIndex) {
        byte first = mWspData[startIndex];
        if ((first == WAP_PDU_LENGTH_QUOTE)
                && decodeUintvarInteger(startIndex + 1)) {
            mStringValue = null;
            mDataLength++;
            return true;
        }

        if (decodeIntegerValue(startIndex) == true) {
            mStringValue = null;
            return true;
        }
        return decodeTextString(startIndex);
    }

    public void expandWellKnownHeadersName() {
        if (mStringValue == null) {
            int binaryHeadersName = (int) mUnsigned32bit;
            mStringValue = WELL_KNOWN_HEADERS.get(binaryHeadersName);
        } else {
            mUnsigned32bit = -1;
        }
    }

    public HashMap<String, String> getHeaders() {
        expandWellKnownXWapApplicationIdName();
        return mHeaders;
    }

    /*
     * Notes: use for expand well known x-application-id
     *
     * See OMNA Push Application ID
     */
    private final static HashMap<Integer, String> WELL_KNOWN_X_WAP_APPLICATION_ID =
        new HashMap<Integer, String>();
    static {
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x00, "x-wap-application:*");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x01, "x-wap-application:push.sia");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x02, "x-wap-application:wml.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x03, "x-wap-application:wta.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x04, "x-wap-application:mms.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x05, "x-wap-application:push.syncml");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x06, "x-wap-application:loc.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x07, "x-wap-application:syncml.dm");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x08, "x-wap-application:drm.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x09, "x-wap-application:emn.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x0A, "x-wap-application:wv.ua");
        //0x0B~0F unused

        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x10, "x-oma-application:ulp.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x11, "x-oma-application:dlota.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x12, "x-oma-application:java-ams");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x13, "x-oma-application:bcast.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x14, "x-oma-application:dpe.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x15, "x-oma-application:cpm:ua");

        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x8000, "x-wap-microsoft:localcontent.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x8001, "x-wap-microsoft:IMclient.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x8002, "x-wap-docomo:imode.mail.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x8003, "x-wap-docomo:imode.mr.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x8004, "x-wap-docomo:imode.mf.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x8005, "x-motorola:location.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x8006, "x-motorola:now.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x8007, "x-motorola:otaprov.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x8008, "x-motorola:browser.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x8009, "x-motorola:splash.ua");
        //0x800A not used
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x800B, "x-wap-nai:mvsw.command");
        //0800C~0800F not used
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x8010, "x-wap-openwave:iota.ua");

        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9000, "x-wap-docomo:imode.mail2.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9001, "x-oma-nec:otaprov.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9002, "x-oma-nokia:call.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9003, "x-oma-coremobility:sqa.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9004, "x-oma-docomo:doja.jam.ua");

        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9010, "x-oma-nokia:sip.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9011, "x-oma-vodafone:otaprov.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9012, "x-hutchison:ad.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9013, "x-oma-nokia:voip.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9014, "x-oma-docomo:voice.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9015, "x-oma-docomo:browser.ctl");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9016, "x-oma-docomo:dan.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9017, "x-oma-nokia:vs.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9018, "x-oma-nokia:voip.ext1.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9019, "x-wap-vodafone:casting.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x901A, "x-oma-docomo:imode.data.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x901B, "x-oma-snapin:otaprov.ctl");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x901C, "x-oma-nokia:vrs.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x901D, "x-oma-nokia:vrpg.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x901E, "x-oma-motorola:screen3.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x901F, "x-oma-docomo:device.ctl");

        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9020, "x-oma-nokia:msc.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9021, "x-3gpp2:lcs.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9022, "x-wap-vodafone:dcd.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9023, "x-3gpp:mbms.service.announcement.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9024, "x-oma-vodafone:dltmtbl.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9025, "x-oma-vodafone:dvcctl.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9026, "x-oma-cmcc:mail.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9027, "x-oma-nokia:vmb.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9028, "x-oma-nokia:ldapss.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9029, "x-hutchison:al.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x902A, "x-oma-nokia:uma.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x902B, "x-oma-nokia:news.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x902C, "x-oma-docomo:pf");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x902D, "x-oma-docomo:ub>");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x902E, "x-oma-nokia:nat.traversal.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x902F, "x-oma-intromobile:intropad.ua");

        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9030, "x-oma-docomo:uin.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9031, "x-oma-nokia:iptv.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9032, "x-hutchison:il.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9033, "x-oma-nokia:voip.general.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9034, "x-microsoft:drm.meter");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9035, "x-microsoft:drm.license");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9036, "x-oma-docomo:ic.ctl");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9037, "x-oma-slingmedia:SPM.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9038, "x-cibenix:odp.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9039, "x-oma-motorola:voip.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x903A, "x-oma-motorola:ims");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x903B, "x-oma-docomo:imode.remote.ctl");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x903C, "x-oma-docomo:device.ctl.um");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x903D, "x-microsoft:playready.drm.initiator");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x903E, "x-microsoft:playready.drm");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x903F, "x-oma-sbm:ms.mexa.ua");

        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9040, "urn:oma:drms:org-LGE:L650V");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9041, "x-oma-docomo:um");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9042, "x-oma-docomo:uin.um");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9043, "urn:oma:drms:org-LGE:KU450");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9044, "x-wap-microsoft:cfgmgr.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9045, "x-3gpp:mbms.download.delivery.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9046, "x-oma-docomo:star.ctl");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9047, "urn:oma:drms:org-LGE:KU380");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9048, "x-oma-docomo:pf2");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9049, "x-oma-motorola:blogcentral.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x904A, "x-oma-docomo:imode.agent.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x904B, "x-wap-application:push.sia");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x904C, "x-oma-nokia:destination.network.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x904D, "x-oma-sbm:mid2.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x904E, "x-carrieriq:avm.ctl");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x904F, "x-oma-sbm:ms.xml.ua");

        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9050, "urn:dvb:ipdc:notification:2008");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9051, "x-oma-docomo:imode.mvch.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9052, "x-oma-motorola:webui.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9053, "x-oma-sbm:cid.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9054, "x-oma-nokia:vcc.v1.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9055, "x-oma-docomo:open.ctl");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9056, "x-oma-docomo:sp.mail.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9057, "x-essoy-application:push.erace");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9058, "x-oma-docomo:open.fu");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9059, "x-samsung:osp.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x905A, "x-oma-docomo:imode.mchara.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x905B,
                "X-Wap-Application-Id:x-oma-application: scidm.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x905C, "x-oma-docomo:xmd.mail.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x905D, "x-oma-application:pal.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x905E, "x-oma-docomo:imode.relation.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x905F, "x-oma-docomo:xmd.storage.ua");
        WELL_KNOWN_X_WAP_APPLICATION_ID.put(0x9060, "x-oma-docomo:xmd.lcsapp.ua");
    }

    /*
     * Notes: The well known X-Wap-Application-Id will be expand to String.
     */

    public void expandWellKnownXWapApplicationIdName() {
        String X_WAP_APPLICATION_ID = "X-Wap-Application-Id";
        int binaryCode = -1;
        try {
            binaryCode = Integer.valueOf(mHeaders.get(X_WAP_APPLICATION_ID));
        } catch (Exception e) {
            return;
        }
        if (binaryCode != -1) {
            String value = WELL_KNOWN_X_WAP_APPLICATION_ID.get(binaryCode);
            if (value != null) {
                mHeaders.put(X_WAP_APPLICATION_ID, value);
            }
        }
    }
    // MTK-END
}
