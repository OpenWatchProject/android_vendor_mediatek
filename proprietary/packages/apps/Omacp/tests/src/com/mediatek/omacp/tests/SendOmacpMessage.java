/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.omacp.tests;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.provider.Telephony;
import com.android.internal.telephony.PhoneConstants;
import java.util.HashMap;

public class SendOmacpMessage {

    public static void sendOmacpMessage(Context context, String content) {
        String data = DOCUMENT_START;
        data += content;
        data += DOCUMENT_END;
        // create a intent, set action, put extras data
        Intent i = new Intent();
        i.setAction("android.provider.Telephony.WAP_PUSH_RECEIVED");
        i.setType("text/vnd.wap.connectivity-xml");
        i.putExtra("data", data.getBytes());
        Bundle bundle = new Bundle();
        //bundle.putInt(PhoneConstants.GEMINI_SIM_ID_KEY, 0);
        bundle.putString(Telephony.WapPush.ADDR, "+8613612345678");
        bundle.putString(Telephony.WapPush.SERVICE_ADDR, "13812345678");
        i.putExtras(bundle);

        // send order broadcast and set the last receiver
        context.sendBroadcast(i);
    }

    public static void sendOmacpMessageWithPin(int eventType, Context context, byte[] content) {
        byte[] data = content;
        // create a intent, set action, put extras data
        Intent intent = new Intent();
        intent.setAction("android.provider.Telephony.WAP_PUSH_RECEIVED");
        intent.setType("application/vnd.wap.connectivity-wbxml");
        intent.putExtra("data", data);
        if (eventType == 0) {
            HashMap<String, String> contentTypeParamaters = new HashMap<String, String>();
            String mac = "82C5D3279F51B5B4A048D7754515386764D2F985";
            String sec = "";
            contentTypeParamaters.put("MAC", mac);
            contentTypeParamaters.put("SEC", sec);
            intent.putExtra("contentTypeParameters", contentTypeParamaters);
        }
        Bundle bundle = new Bundle();
        //bundle.putInt(PhoneConstants.GEMINI_SIM_ID_KEY, 0);
        bundle.putString(Telephony.WapPush.ADDR, "15603027251");
        bundle.putString(Telephony.WapPush.SERVICE_ADDR, "+8613010888500");
        intent.putExtras(bundle);

        // send order broadcast and set the last receiver
        context.sendBroadcast(intent);
    }

    public static final String DOCUMENT_START = "<wap-provisioningdoc>"
        + "<characteristic type=\"BOOTSTRAP\">"
        + "<parm name=\"NAME\" value=\"settings\" /> "
        + "</characteristic>";

    public static final String BROWSER_DATA = "<characteristic type=\"APPLICATION\">" +
            "<parm name=\"APPID\" value=\"w2\"/>" +
            "<parm name=\"NAME\" value=\"bookmark folder name\"/>" +
            "<parm name=\"TO-PROXY\" value=\"PROXY\"/>" +
            "<parm name=\"TO-NAPID\" value=\"INTERNET\"/>" +
            "<characteristic type=\"RESOURCE\">" +
            "<parm name=\"AAUTHNAME\" value=\"username\"/>" +
            "<parm name=\"AAUTHTYPE\" value=\"HTTP-BASIC\"/>" +
            "<parm name=\"AAUTHSECRET\" value=\"password\"/>" +
            "<parm name=\"URI\" value=\"www.somecompany.com/startpage/index.wml\"/>" +
            "<parm name=\"NAME\" value=\"Some Company WAP Service\"/>" +
            "<parm name=\"STARTPAGE\"/>" +
            "</characteristic>" +
            "<characteristic type=\"RESOURCE\">" +
            "<parm name=\"AAUTHNAME\" value=\"username2\"/>" +
            "<parm name=\"AAUTHTYPE\" value=\"HTTP-BASIC\"/>" +
            "<parm name=\"AAUTHSECRET\" value=\"password2\"/>" +
            "<parm name=\"URI\" value=\"www.sina.com/startpage/index.wml\"/>" +
            "<parm name=\"NAME\" value=\"Sina WAP Service\"/>" +
            "</characteristic>" +
            "</characteristic>";

    public static final String MMS_DATA = "<characteristic type=\"APPLICATION\">" +
            "<parm name=\"APPID\" value=\"w4\"/>" +
            "<parm name=\"NAME\" value=\"Operator MMS Server\"/>" +
            "<parm name=\"ADDR\" value=\"http://mms.operator.com/MMS\"/>" +
            "<parm name=\"CM\" value=\"R\"/>" +
            "</characteristic>" +
            "<characteristic type=\"APPLICATION\">" +
            "<parm name=\"APPID\" value=\"ap0005\"/>" +
            "<parm name=\"NAME\" value=\"Operator MMS Server\"/>" +
            "<parm name=\"TO-PROXY\" value=\"PROXY\"/>" +
            "<parm name=\"TO-NAPID\" value=\"NAPID\"/>" +
            "<parm name=\"ADDR\" value=\"http://mms.operator.com/MMS\"/>" +
            "<parm name=\"CM\" value=\"R\"/>" +
            "<parm name=\"RM\" value=\"F\"/>" +
            "<parm name=\"MS\" value=\"300\"/>" +
            "<parm name=\"PC-ADDR\" value=\"http://www.mms.pc-addr.com\"/>" +
            "<parm name=\"Ma\" value=\"Disable\"/>" +
            "</characteristic>";

    public static final String MMSAPN_DATA = "<wap-provisioningdoc version=\"1.0\">" +
            "<characteristic type=\"BOOTSTRAP\">" +
            "<parm name=\"PROXY-ID\" value=\"mms-pxy\"/>" +
            "<parm name=\"NAME\" value=\"T-Mobile MMS\"/>" +
            "</characteristic>" +
            "<characteristic type=\"PXLOGICAL\">" +
            "<parm name=\"PROXY-ID\" value=\"mms-pxy\"/>" +
            "<parm name=\"NAME\" value=\"T-Mobile MMS\"/>" +
            "<characteristic type=\"PXPHYSICAL\">" +
            "<parm name=\"PHYSICAL-PROXY-ID\" value=\"MMS-Proxy\"/>" +
            "<parm name=\"PXADDRTYPE\" value=\"IPV4\"/>" +
            "<parm name=\"PXADDR\" value=\"172.28.23.131\"/>" +
            "<parm name=\"TO-NAPID\" value=\"mms-apn\"/>" +
            "<characteristic type=\"PORT\">" +
            "<parm name=\"PORTNBR\" value=\"8008\"/>" +
            "</characteristic>" +
            "</characteristic>" +
            "</characteristic>" +
            "<characteristic type=\"NAPDEF\">" +
            "<parm name=\"NAPID\" value=\"mms-apn\"/>" +
            "<parm name=\"BEARER\" value=\"GSM-GPRS\"/>" +
            "<parm name=\"NAME\" value=\"T-Mobile MMS\"/>" +
            "<parm name=\"NAP-ADDRESS\" value=\"internet.t-mobile\"/>" +
            "<parm name=\"NAP-ADDRTYPE\" value=\"APN\"/>" +
            "<parm name=\"LINGER\" value=\"180\"/>" +
            "<characteristic type=\"NAPAUTHINFO\">" +
            "<parm name=\"AUTHTYPE\" value=\"PAP\"/>" +
            "<parm name=\"AUTHNAME\" value=\"t-mobile\"/>" +
            "<parm name=\"AUTHSECRET\" value=\"tm\"/>" +
            "</characteristic>" +
            "</characteristic>" +
            "<characteristic type=\"APPLICATION\">" +
            "<parm name=\"APPID\" value=\"w4\"/>" +
            "<parm name=\"NAME\" value=\"T-Mobile MMS\"/>" +
            "<parm name=\"TO-PROXY\" value=\"mms-pxy\"/>" +
            "<parm name=\"ADDR\" value=\"http://mms.t-mobile.de/servlets/mms\"/>" +
            "</characteristic>" +
            "</wap-provisioningdoc>";

    public static final String APN_DATA = "<wap-provisioningdoc>" +
            "<characteristic type=\"BOOTSTRAP\">" +
            "<parm name=\"NAME\" value=\"Orange Settings\"/>" +
            "</characteristic>" +
            "<characteristic type=\"PXLOGICAL\">" +
            "<parm name=\"PROXY-ID\" value=\"orange1.co.il\"/>" +
            "<parm name=\"NAME\" value=\"Orange 3G\"/>" +
            "<characteristic type=\"PXPHYSICAL\">" +
            "<parm name=\"PHYSICAL-PROXY-ID\" value=\"proxy 1\"/>" +
            "<parm name=\"PXADDR\" value=\"192.118.011.056\"/>" +
            "<parm name=\"PXADDRTYPE\" value=\"IPV4\"/>" +
            "<parm name=\"TO-NAPID\" value=\"Always on\"/>" +
            "<characteristic type=\"PORT\">" +
            "<parm name=\"PORTNBR\" value=\"80\"/>" +
            "</characteristic>" +
            "</characteristic>" +
            "</characteristic>" +
            "<characteristic type=\"NAPDEF\">" +
            "<parm name=\"NAPID\" value=\"Always on\"/>" +
            "<parm name=\"BEARER\" value=\"GSM-GPRS\"/>" +
            "<parm name=\"NAME\" value=\"3G Browse\"/>" +
            "<parm name=\"NAP-ADDRESS\" value=\"uwap.orange.co.il\"/>" +
            "<parm name=\"NAP-ADDRTYPE\" value=\"APN\"/>" +
            "<characteristic type=\"NAPAUTHINFO\">" +
            "<parm name=\"AUTHTYPE\" value=\"CHAP\"/>" +
            "<parm name=\"AUTHNAME\" value=\"orange\"/>" +
            "<parm name=\"AUTHSECRET\" value=\"mobile54\"/>" +
            "</characteristic>" +
            "</characteristic>" +
            "</wap-provisioningdoc>";

    public static final String SUPL_DATA = "<characteristic type=\"APPLICATION\">" +
            "<parm name=\"APPID\" value=\"ap0004\"/>" +
            "<parm name=\"PROVIDER-ID\" value=\"Some operator\"/>" +
            "<parm name=\"NAME\" value=\"Some SLP Server\"/>" +
            "<parm name=\"TO-NAPID\" value=\"INTERNET\"/>" +
            "<characteristic type=\"APPADDR\">" +
            "<parm name=\"ADDR\" value=\"123.56.78.90\"/>" +
            "<parm name=\"ADDRTYPE\" value=\"FQDN\"/>" +
            "</characteristic>" +
            "</characteristic>";

    public static final String RTSP_DATA = "<characteristic type=\"APPLICATION\">" +
            "<parm name=\"APPID\" value=\"554\"/>" +
            "<parm name=\"PROVIDER-ID\" value=\"MyRtsp\"/>" +
            "<parm name=\"NAME\" value=\"Operator streaming settings\"/>" +
            "<parm name=\"TO-PROXY\" value=\"streaming-internet\"/>" +
            "<parm name=\"TO-NAPID\" value=\"IAP1\"/>" +
            "<parm name=\"MAX-BANDWIDTH\" value=\"128000\"/>" +
            "<parm name=\"MIN-UDP-PORT\" value=\"6970\"/>" +
            "<parm name=\"MAX-UDP-PORT\" value=\"32000\"/>" +
            "</characteristic>";

    public static final String EMAIL_GMAIL_DATA = "<characteristic type=\"APPLICATION\">" +
            "<parm name=\"APPID\" value=\"25\" />" +
            "<parm name=\"PROVIDER-ID\" value=\"settings\" />" +
            "<parm name=\"TO-NAPID\" value=\"settings_NAPID\" />" +
            "<parm name=\"FROM\" value=\"stbtest@gmail.com\" />" +
            "<characteristic type=\"APPADDR\">" +
            "<parm name=\"ADDR\" value=\"smtp.gmail.com\" />" +
            "<characteristic type=\"PORT\">" +
            "<parm name=\"PORTNBR\" value=\"465\" />" +
            "<parm name=\"SERVICE\" value=\"465\" />" +
            "</characteristic>" +
            "</characteristic>" +
            "<characteristic type=\"APPAUTH\">" +
            "<parm name=\"AAUTHTYPE\" value=\"LOGIN\" />" +
            "<parm name=\"AAUTHNAME\" value=\"stbtest\" />" +
            "<parm name=\"AAUTHSECRET\" value=\"wps2000wps2000\" />" +
            "</characteristic>" +
            "</characteristic>" +
            "<characteristic type=\"APPLICATION\">" +
            "<parm name=\"APPID\" value=\"143\" />" +
            "<parm name=\"PROVIDER-ID\" value=\"settings\" />" +
            "<parm name=\"TO-NAPID\" value=\"settings_NAPID\" />" +
            "<characteristic type=\"APPADDR\">" +
            "<parm name=\"ADDR\" value=\"imap.gmail.com\" />" +
            "<characteristic type=\"PORT\">" +
            "<parm name=\"PORTNBR\" value=\"993\" />" +
            "<parm name=\"SERVICE\" value=\"993\" />" +
            "</characteristic>" +
            "</characteristic>" +
            "<characteristic type=\"APPAUTH\">" +
            "<parm name=\"AAUTHNAME\" value=\"stbtest\" />" +
            "<parm name=\"AAUTHSECRET\" value=\"wps2000wps2000\" />" +
            "</characteristic>" +
            "</characteristic>";

    public static final String EMAIL_SINA_DATA = "<characteristic type=\"APPLICATION\">" +
            "<parm name=\"APPID\" value=\"25\" />" +
            "<parm name=\"PROVIDER-ID\" value=\"settings\" />" +
            "<parm name=\"TO-NAPID\" value=\"settings_NAPID\" />" +
            "<parm name=\"FROM\" value=\"stbtest@sina.com\" />" +
            "<characteristic type=\"APPADDR\">" +
            "<parm name=\"ADDR\" value=\"smtp.sina.com.cn\" />" +
            "<characteristic type=\"PORT\">" +
            "<parm name=\"PORTNBR\" value=\"25\" />" +
            "<parm name=\"SERVICE\" value=\"25\" />" +
            "</characteristic>" +
            "</characteristic>" +
            "<characteristic type=\"APPAUTH\">" +
            "<parm name=\"AAUTHTYPE\" value=\"LOGIN\" />" +
            "<parm name=\"AAUTHNAME\" value=\"stbtest\" />" +
            "<parm name=\"AAUTHSECRET\" value=\"wps2000\" />" +
            "</characteristic>" +
            "</characteristic>" +
            "<characteristic type=\"APPLICATION\">" +
            "<parm name=\"APPID\" value=\"110\" />" +
            "<parm name=\"PROVIDER-ID\" value=\"settings\" />" +
            "<parm name=\"TO-NAPID\" value=\"settings_NAPID\" />" +
            "<characteristic type=\"APPADDR\">" +
            "<parm name=\"ADDR\" value=\"pop.sina.com.cn\" />" +
            "<characteristic type=\"PORT\">" +
            "<parm name=\"PORTNBR\" value=\"110\" />" +
            "<parm name=\"SERVICE\" value=\"110\" />" +
            "</characteristic>" +
            "</characteristic>" +
            "<characteristic type=\"APPAUTH\">" +
            "<parm name=\"AAUTHNAME\" value=\"stbtest\" />" +
            "<parm name=\"AAUTHSECRET\" value=\"wps2000\" />" +
            "</characteristic>" +
            "</characteristic>";

    public static final String DM_DATA = "<characteristic type=\"APPLICATION\">" +
            "<parm name=\"APPID\" value=\"w7\"/>" +
            "<parm name=\"PROVIDER-ID\" value=\"com.mgmtsrv.manage\"/>" +
            "<parm name=\"NAME\" value=\"Mgmt Server\"/>" +
            "<parm name=\"ADDR\" value=\"http://www.mgmtserver.com:8080/manage\"/>" +
            "<parm name=\"TO-NAPID\" value=\"INTERNET\"/>" +
            "<parm name=\"INIT\"/>" +
            "<characteristic type=\"APPAUTH\">" +
            "<parm name=\"AAUTHTYPE\" value=\"HTTP-DIGEST\"/>" +
            "<parm name=\"AAUTHNAME\" value=\"client_http_id\"/>" +
            "<parm name=\"AAUTHSECRET\" value=\"client_http_secret\"/>" +
            "</characteristic>" +
            "<characteristic type=\"APPAUTH\">" +
            "<parm name=\"AAUTHLEVEL\" value=\"CLIENT\"/>" +
            "<parm name=\"AAUTHTYPE\" value=\"DIGEST,BASIC\"/>" +
            "<parm name=\"AAUTHNAME\" value=\"server_id\"/>" +
            "<parm name=\"AAUTHSECRET\" value=\"serversecret\"/>" +
            "<parm name=\"AAUTHDATA\" value=\"servernonce\"/>" +
            "</characteristic>" +
            "<characteristic type=\"APPAUTH\">" +
            "<parm name=\"AAUTHLEVEL\" value=\"APPSRV\"/>" +
            "<parm name=\"AAUTHNAME\" value=\"client_id\"/>" +
            "<parm name=\"AAUTHSECRET\" value=\"clientsecret\"/>" +
            "<parm name=\"AAUTHDATA\" value=\"clientnonce\"/>" +
            "</characteristic>" +
            "</characteristic>";

    public static final String IMPS_DATA = "<wap-provisioningdoc>" +
            "<characteristic type=\"BOOTSTRAP\">" +
            "<parm name=\"NAME\" value=\"settings\" />" +
            "</characteristic>" +
            "<characteristic type=\"APPLICATION\">" +
            "<parm name=\"APPID\" value=\"wA\"/>" +
            "<parm name=\"PROVIDER-ID\" value=\"operator\"/>" +
            "<parm name=\"NAME\" value=\"IMPS Server\"/>" +
            "<characteristic type=\"APPAUTH\">" +
            "<parm name=\"AAUTHLEVEL\" value=\"APPSRV\"/>" +
            "<parm name=\"AAUTHNAME\" value=\"username\"/>" +
            "<parm name=\"AAUTHSECRET\" value=\"password />" +
            "</characteristic>" +
            "<parm name=\"AACCEPT\" value=\"application/vnd.wv.csp+xml;1.3;1.2;1.1;1.0,application/vnd.wv.csp+wbxml;1.3;1.2;1.1;1.0\"/>" +
            "<parm name=\"ADDR\" value=\"http://123.56.78.90\"/>" +
            "<parm name=\"TO-NAPID\" value=\"INTERNET\"/>" +
            "<parm name=\"TO-PROXY\" value=\"123.56.78.90\"/>" +
            "<parm name=\"SERVICES\" value=\"IM;PR;GR\"/>" +
            "</characteristic>" +
            "</wap-provisioningdoc>";

    public static final String DOCUMENT_END = "</wap-provisioningdoc>";

    public static final byte[] WEBXMLAPNDATA = {
            3, 11, 106, 0, 69, 3, 45, 32, 0, -58, 86, 1, -121, 7, 6, 3, 115, 101, 116, 116, 105,
            110, 103, 115, 0, 1, 1, 3, 45, 32, 0, -58, 0, 1, 85, 1, -121, 54, 0, 0, 6, 3, 97, 112,
            48, 48, 48, 52, 0, 1, -121, 0, 1, 56, 0, 0, 6, 3, 83, 111, 109, 101, 32, 111, 112, 101,
            114, 97, 116, 111, 114, 0, 1, -121, 7, 6, 3, 83, 111, 109, 101, 32, 83, 76, 80, 32, 83,
            101, 114, 118, 101, 114, 0, 1, -121, 34, 6, 3, 73, 78, 84, 69, 82, 78, 69, 84, 0, 1,
            -58, 0, 1, 86, 1, -121, 52, 0, 0, 6, 3, 49, 50, 51, 46, 53, 54, 46, 55, 56, 46, 57, 48,
            0, 1, -121, 0, 1, 53, 0, 0, 6, 3, 70, 81, 68, 78, 0, 1, 1, 1, 1};

}
