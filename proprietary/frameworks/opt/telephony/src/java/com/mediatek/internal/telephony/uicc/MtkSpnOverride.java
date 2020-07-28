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

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import android.os.Environment;
import android.telephony.Rlog;
import android.util.Xml;

import com.android.internal.util.XmlUtils;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.PhoneConstants;
//import com.android.internal.telephony.uicc.SpnOverride;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkGsmCdmaPhone;

import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.content.Context;
import android.os.SystemProperties;
import java.util.ArrayList;

public class MtkSpnOverride {

    private static MtkSpnOverride sInstance;
    protected HashMap<String, String> mCarrierSpnMap;
    static final String LOG_TAG = "SpnOverride";
    protected static final String PARTNER_SPN_OVERRIDE_PATH ="etc/spn-conf.xml";
    protected static final String OEM_SPN_OVERRIDE_PATH = "telephony/spn-conf.xml";
    static final Object sInstSync = new Object();
    static final String LOG_TAG_EX = "MtkSpnOverride";
    // MTK-START: MVNO
    // MVNO-API START
    // EF_SPN
    private static HashMap<String, String> CarrierVirtualSpnMapByEfSpn;
    private static final String PARTNER_VIRTUAL_SPN_BY_EF_SPN_OVERRIDE_PATH =
            "etc/virtual-spn-conf-by-efspn.xml";

    // IMSI
    private ArrayList CarrierVirtualSpnMapByImsi;
    private static final String PARTNER_VIRTUAL_SPN_BY_IMSI_OVERRIDE_PATH =
            "etc/virtual-spn-conf-by-imsi.xml";

    // EF_PNN
    private static HashMap<String, String> CarrierVirtualSpnMapByEfPnn;
    private static final String PARTNER_VIRTUAL_SPN_BY_EF_PNN_OVERRIDE_PATH =
            "etc/virtual-spn-conf-by-efpnn.xml";

    // EF_GID1
    private static HashMap<String, String> CarrierVirtualSpnMapByEfGid1;
    private static final String PARTNER_VIRTUAL_SPN_BY_EF_GID1_OVERRIDE_PATH =
            "etc/virtual-spn-conf-by-efgid1.xml";

    public class VirtualSpnByImsi {
        public String pattern;
        public String name;
        public VirtualSpnByImsi(String pattern, String name) {
            this.pattern = pattern;
            this.name = name;
        }
    }
    // MVNO-API END

    public static MtkSpnOverride getInstance() {
        synchronized (sInstSync) {
            if (sInstance == null) {
                sInstance = new MtkSpnOverride();
            }
            return sInstance;
        }
    }

    MtkSpnOverride () {
        mCarrierSpnMap = new HashMap<String, String>();
        loadSpnOverrides();

        // MTK-START
        // MVNO-API
        // EF_SPN
        CarrierVirtualSpnMapByEfSpn = new HashMap<String, String>();
        loadVirtualSpnOverridesByEfSpn();

        // IMSI
        this.CarrierVirtualSpnMapByImsi = new ArrayList();
        this.loadVirtualSpnOverridesByImsi();

        // EF_PNN
        CarrierVirtualSpnMapByEfPnn = new HashMap<String, String>();
        loadVirtualSpnOverridesByEfPnn();

        // EF_GID1
        CarrierVirtualSpnMapByEfGid1 = new HashMap<String, String>();
        loadVirtualSpnOverridesByEfGid1();
        // MTK-END
    }

    protected void loadSpnOverrides() {
        FileReader spnReader;
        // MTK-START
        Rlog.d(LOG_TAG_EX, "loadSpnOverrides");
        File spnFile = null;

        if ("OP09".equals(SystemProperties.get("persist.vendor.operator.optr", ""))) {
            spnFile = new File(Environment.getVendorDirectory(),
                    "etc/spn-conf-op09.xml");
            if (!spnFile.exists()) {
                Rlog.d(LOG_TAG_EX, "No spn-conf-op09.xml file");
                spnFile = new File(Environment.getRootDirectory(),
                        PARTNER_SPN_OVERRIDE_PATH);
            }
        } else {
            spnFile = new File(Environment.getRootDirectory(),
                    PARTNER_SPN_OVERRIDE_PATH);
        }
        File oemSpnFile = new File(Environment.getOemDirectory(),
                OEM_SPN_OVERRIDE_PATH);

        if (oemSpnFile.exists()) {
            // OEM image exist SPN xml, get the timestamp from OEM & System image for comparison.
            long oemSpnTime = oemSpnFile.lastModified();
            long sysSpnTime = spnFile.lastModified();
            Rlog.d(LOG_TAG_EX, "SPN Timestamp: oemTime = " + oemSpnTime + " sysTime = " +
                    sysSpnTime);

            // To get the newer version of SPN from OEM image
            if (oemSpnTime > sysSpnTime) {
                Rlog.d(LOG_TAG_EX, "SPN in OEM image is newer than System image");
                spnFile = oemSpnFile;
            }
        } else {
            // No SPN in OEM image, so load it from system image.
            Rlog.d(LOG_TAG_EX, "No SPN in OEM image = " + oemSpnFile.getPath() +
                " Load SPN from system image");
        }

        try {
            spnReader = new FileReader(spnFile);
        } catch (FileNotFoundException e) {
            Rlog.w(LOG_TAG_EX, "Can not open " + spnFile.getAbsolutePath());
            return;
        }

        try {
            XmlPullParser parser = Xml.newPullParser();
            parser.setInput(spnReader);

            XmlUtils.beginDocument(parser, "spnOverrides");

            while (true) {
                XmlUtils.nextElement(parser);

                String name = parser.getName();
                if (!"spnOverride".equals(name)) {
                    break;
                }

                String numeric = parser.getAttributeValue(null, "numeric");
                String data    = parser.getAttributeValue(null, "spn");

                mCarrierSpnMap.put(numeric, data);
            }
            spnReader.close();
        } catch (XmlPullParserException e) {
            Rlog.w(LOG_TAG_EX, "Exception in spn-conf parser " + e);
        } catch (IOException e) {
            Rlog.w(LOG_TAG_EX, "Exception in spn-conf parser " + e);
        }
    }

    // MTK-START
    // MVNO-API START
    private static void loadVirtualSpnOverridesByEfSpn() {
        FileReader spnReader;
        Rlog.d(LOG_TAG_EX, "loadVirtualSpnOverridesByEfSpn");
        final File spnFile = new File(Environment.getVendorDirectory(),
                PARTNER_VIRTUAL_SPN_BY_EF_SPN_OVERRIDE_PATH);

        try {
            spnReader = new FileReader(spnFile);
        } catch (FileNotFoundException e) {
            Rlog.w(LOG_TAG_EX, "Can't open " +
                    Environment.getVendorDirectory() + "/" +
                    PARTNER_VIRTUAL_SPN_BY_EF_SPN_OVERRIDE_PATH);
            return;
        }

        try {
            XmlPullParser parser = Xml.newPullParser();
            parser.setInput(spnReader);

            XmlUtils.beginDocument(parser, "virtualSpnOverridesByEfSpn");

            while (true) {
                XmlUtils.nextElement(parser);

                String name = parser.getName();
                if (!"virtualSpnOverride".equals(name)) {
                    break;
                }

                String mccmncspn = parser.getAttributeValue(null, "mccmncspn");
                String spn = parser.getAttributeValue(null, "name");
                Rlog.w(LOG_TAG_EX, "test mccmncspn = " + mccmncspn + ", name = " + spn);
                CarrierVirtualSpnMapByEfSpn.put(mccmncspn, spn);
            }
            spnReader.close();
        } catch (XmlPullParserException e) {
            Rlog.w(LOG_TAG_EX, "Exception in virtual-spn-conf-by-efspn parser " + e);
        } catch (IOException e) {
            Rlog.w(LOG_TAG_EX, "Exception in virtual-spn-conf-by-efspn parser " + e);
        }
    }

    public String getSpnByEfSpn(String mccmnc, String spn) {
        if (mccmnc == null || spn == null || mccmnc.isEmpty() || spn.isEmpty())
            return null;

        return CarrierVirtualSpnMapByEfSpn.get(mccmnc + spn);
    }

    private void loadVirtualSpnOverridesByImsi() {
        FileReader spnReader;
        Rlog.d(LOG_TAG_EX, "loadVirtualSpnOverridesByImsi");
        final File spnFile = new File(Environment.getVendorDirectory(),
                PARTNER_VIRTUAL_SPN_BY_IMSI_OVERRIDE_PATH);

        try {
            spnReader = new FileReader(spnFile);
        } catch (FileNotFoundException e) {
            Rlog.w(LOG_TAG_EX, "Can't open " +
                    Environment.getVendorDirectory() + "/" +
                    PARTNER_VIRTUAL_SPN_BY_IMSI_OVERRIDE_PATH);
            return;
        }

        try {
            XmlPullParser parser = Xml.newPullParser();
            parser.setInput(spnReader);

            XmlUtils.beginDocument(parser, "virtualSpnOverridesByImsi");

            while (true) {
                XmlUtils.nextElement(parser);

                String name = parser.getName();
                if (!"virtualSpnOverride".equals(name)) {
                    break;
                }

                String imsipattern = parser.getAttributeValue(null, "imsipattern");
                String spn = parser.getAttributeValue(null, "name");
                Rlog.w(LOG_TAG_EX, "test imsipattern = " + imsipattern + ", name = " + spn);
                this.CarrierVirtualSpnMapByImsi.add(new VirtualSpnByImsi(imsipattern, spn));
            }
            spnReader.close();
        } catch (XmlPullParserException e) {
            Rlog.w(LOG_TAG_EX, "Exception in virtual-spn-conf-by-imsi parser " + e);
        } catch (IOException e) {
            Rlog.w(LOG_TAG_EX, "Exception in virtual-spn-conf-by-imsi parser " + e);
        }
    }

    public String getSpnByImsi(String mccmnc, String imsi) {
        if (mccmnc == null || imsi == null || mccmnc.isEmpty() || imsi.isEmpty())
            return null;

        VirtualSpnByImsi vsbi;
        for (int i = 0; i < this.CarrierVirtualSpnMapByImsi.size(); i++) {
            vsbi = (VirtualSpnByImsi) (this.CarrierVirtualSpnMapByImsi.get(i));
            Rlog.d(LOG_TAG_EX, "getSpnByImsi(): mccmnc = " + mccmnc + ", imsi = " +
                    ((imsi.length() >= 6) ? imsi.substring(0, 6) : "xx")
                    + ", pattern = " + vsbi.pattern);

            if (imsiMatches(vsbi.pattern, mccmnc + imsi) == true) {
                 return vsbi.name;
            }
        }
        return null;
    }

    public String isOperatorMvnoForImsi(String mccmnc, String imsi) {
        if (mccmnc == null || imsi == null || mccmnc.isEmpty() || imsi.isEmpty())
            return null;

        VirtualSpnByImsi vsbi;
        String pattern;
        for (int i = 0; i < this.CarrierVirtualSpnMapByImsi.size(); i++) {
            vsbi = (VirtualSpnByImsi) (this.CarrierVirtualSpnMapByImsi.get(i));
            Rlog.w(LOG_TAG_EX, "isOperatorMvnoForImsi(): mccmnc = " + mccmnc +
                    ", imsi = " + ((imsi.length() >= 6) ?
                    imsi.substring(0, 6) : "xx") + ", pattern = " + vsbi.pattern);

            if (imsiMatches(vsbi.pattern, mccmnc + imsi) == true) {
                return vsbi.pattern;
            }
        }
        return null;
    }

    private boolean imsiMatches(String imsiDB, String imsiSIM) {
        // Note: imsiDB value has digit number or 'x' character for seperating USIM information
        // for MVNO operator. And then digit number is matched at same order and 'x' character
        // could replace by any digit number.
        // ex) if imsiDB inserted '310260x10xxxxxx' for GG Operator,
        //     that means first 6 digits, 8th and 9th digit
        //     should be set in USIM for GG Operator.
        int len = imsiDB.length();
        int idxCompare = 0;

        Rlog.d(LOG_TAG_EX, "mvno match imsi = " +
            ((imsiSIM == null) ? "" : ((imsiSIM.length() >= 6) ? imsiSIM.substring(0, 6) : "xx"))
            + "pattern = " + imsiDB);
        if (len <= 0 || imsiSIM == null) return false;
        if (len > imsiSIM.length()) return false;

        for (int idx = 0; idx < len; idx++) {
            char c = imsiDB.charAt(idx);
            if ((c == 'x') || (c == 'X') || (c == imsiSIM.charAt(idx))) {
                continue;
            } else {
                return false;
            }
        }
        return true;
    }

    private static void loadVirtualSpnOverridesByEfPnn() {
        FileReader spnReader;
        Rlog.d(LOG_TAG_EX, "loadVirtualSpnOverridesByEfPnn");
        final File spnFile = new File(Environment.getVendorDirectory(),
                PARTNER_VIRTUAL_SPN_BY_EF_PNN_OVERRIDE_PATH);

        try {
            spnReader = new FileReader(spnFile);
        } catch (FileNotFoundException e) {
            Rlog.w(LOG_TAG_EX, "Can't open " +
                    Environment.getVendorDirectory() + "/" +
                    PARTNER_VIRTUAL_SPN_BY_EF_PNN_OVERRIDE_PATH);
            return;
        }

        try {
            XmlPullParser parser = Xml.newPullParser();
            parser.setInput(spnReader);

            XmlUtils.beginDocument(parser, "virtualSpnOverridesByEfPnn");

            while (true) {
                XmlUtils.nextElement(parser);

                String name = parser.getName();
                if (!"virtualSpnOverride".equals(name)) {
                    break;
                }

                String mccmncpnn = parser.getAttributeValue(null, "mccmncpnn");
                String spn = parser.getAttributeValue(null, "name");
                Rlog.w(LOG_TAG_EX, "test mccmncpnn = " + mccmncpnn + ", name = " + spn);
                CarrierVirtualSpnMapByEfPnn.put(mccmncpnn, spn);
            }
            spnReader.close();
        } catch (XmlPullParserException e) {
            Rlog.w(LOG_TAG_EX, "Exception in virtual-spn-conf-by-efpnn parser " + e);
        } catch (IOException e) {
            Rlog.w(LOG_TAG_EX, "Exception in virtual-spn-conf-by-efpnn parser " + e);
        }
    }

    public String getSpnByEfPnn(String mccmnc, String pnn) {
        if (mccmnc == null || pnn == null || mccmnc.isEmpty() || pnn.isEmpty())
            return null;

        return CarrierVirtualSpnMapByEfPnn.get(mccmnc + pnn);
    }

    private static void loadVirtualSpnOverridesByEfGid1() {
        FileReader spnReader;
        Rlog.d(LOG_TAG_EX, "loadVirtualSpnOverridesByEfGid1");
        final File spnFile = new File(Environment.getVendorDirectory(),
                PARTNER_VIRTUAL_SPN_BY_EF_GID1_OVERRIDE_PATH);

        try {
            spnReader = new FileReader(spnFile);
        } catch (FileNotFoundException e) {
            Rlog.w(LOG_TAG_EX, "Can't open " +
                    Environment.getVendorDirectory() + "/" +
                    PARTNER_VIRTUAL_SPN_BY_EF_GID1_OVERRIDE_PATH);
            return;
        }

        try {
            XmlPullParser parser = Xml.newPullParser();
            parser.setInput(spnReader);

            XmlUtils.beginDocument(parser, "virtualSpnOverridesByEfGid1");

            while (true) {
                XmlUtils.nextElement(parser);

                String name = parser.getName();
                if (!"virtualSpnOverride".equals(name)) {
                    break;
                }

                String mccmncgid1 = parser.getAttributeValue(null, "mccmncgid1");
                String spn = parser.getAttributeValue(null, "name");
                Rlog.w(LOG_TAG_EX, "test mccmncgid1 = " + mccmncgid1 + ", name = " + spn);
                CarrierVirtualSpnMapByEfGid1.put(mccmncgid1, spn);
            }
            spnReader.close();
        } catch (XmlPullParserException e) {
            Rlog.w(LOG_TAG_EX, "Exception in virtual-spn-conf-by-efgid1 parser " + e);
        } catch (IOException e) {
            Rlog.w(LOG_TAG_EX, "Exception in virtual-spn-conf-by-efgid1 parser " + e);
        }
    }

    public String getSpnByEfGid1(String mccmnc, String gid1) {
        if (mccmnc == null || gid1 == null || mccmnc.isEmpty() || gid1.isEmpty())
            return null;

        return CarrierVirtualSpnMapByEfGid1.get(mccmnc + gid1);
    }

    /*
     * Get service provider name by MVNO parttern in pre-configured XML.
     *
     * There are four MVNO reference XML as below:
     * 1. virtual-spn-conf-by-efspn.xml: match by mcc/mnc + EF_SPN value combination.
     * 2. virtual-spn-conf-by-imsi.xml: match by mcc/mnc + EF_IMSI value combination.
     * 3. virtual-spn-conf-by-efpnn.xml: match by mcc/mnc + EF_PNN value combination.
     * 4. virtual-spn-conf-by-efgid.xml: match by mcc/mnc + EF_GID1 value combination.
     *
     * The check has prioity with 1->2->3->4 sequence.
     *
     * @param subId subscription id to indicate the target SIM card.
     * @param numeric MCC/MNC
     *
     * @return operation name (if didn't match any pattern, will return null)
     *
     */
    public String getSpnByPattern(int subId, String numeric) {
        Phone phone = PhoneFactory.getPhone(SubscriptionManager.getPhoneId(subId));
        String mvnoOperName = null;

        mvnoOperName = getSpnByEfSpn(numeric,
                 ((MtkGsmCdmaPhone)phone).getMvnoPattern(MtkPhoneConstants.MVNO_TYPE_SPN));
        Rlog.d(LOG_TAG_EX, "the result of searching mvnoOperName by EF_SPN: " + mvnoOperName);

        if (mvnoOperName == null) {
            mvnoOperName = getSpnByImsi(numeric, phone.getSubscriberId());
            Rlog.d(LOG_TAG_EX, "the result of searching mvnoOperName by IMSI: " + mvnoOperName);
        }

        if (mvnoOperName == null) {
            mvnoOperName = getSpnByEfPnn(numeric,
                    ((MtkGsmCdmaPhone)phone).getMvnoPattern(MtkPhoneConstants.MVNO_TYPE_PNN));
            Rlog.d(LOG_TAG_EX, "the result of searching mvnoOperName by EF_PNN: " + mvnoOperName);
        }

        if (mvnoOperName == null) {
            mvnoOperName = getSpnByEfGid1(numeric,
                    ((MtkGsmCdmaPhone)phone).getMvnoPattern(MtkPhoneConstants.MVNO_TYPE_GID));
            Rlog.d(LOG_TAG_EX, "the result of searching mvnoOperName by EF_GID1: " + mvnoOperName);
        }

        return mvnoOperName;
    }

    private boolean isForceGetCtSpnFromRes(
            int subId, String numeric, Context context, String mvnoOperName) {

        boolean getFromResource = false;
        Phone phone = PhoneFactory.getPhone(SubscriptionManager.getPhoneId(subId));
        String ctName = context.getText(com.mediatek.internal.R.string.ct_name).toString();
        String simCarrierName = TelephonyManager.from(context).getSimOperatorName(subId);
        Rlog.d(LOG_TAG_EX, "ctName:" + ctName + ", simCarrierName:" + simCarrierName
                + ", subId:" + subId);
        if (ctName != null && (ctName.equals(mvnoOperName) || ctName.equals(simCarrierName))) {
            Rlog.d(LOG_TAG_EX, "Get from resource.");
            getFromResource = true;
        }

        if (("20404".equals(numeric) || "45403".equals(numeric))
                && phone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA
                && ctName != null && ctName.equals(simCarrierName)) {
            Rlog.d(LOG_TAG_EX, "Special handle for roaming case!");
            getFromResource = true;
        }

        return getFromResource;
    }

    /*
     * Get service provider name by MCC/MNC from pre-configured operator name resource file and
     * spn-conf.xml.
     *
     * Please notes that if you query long name, we will search spn-conf.xml in case of there is
     * no pre-configured operator name resource file matched.
     *
     * @param numeric MCC/MNC
     * @param desireLongName indicate to get long name or short name.
     * @param context context used to get resource
     *
     * @return operation name (if didn't match any pre-configured mcc/mnc, will return null)
     */
    public String getSpnByNumeric(String numeric, boolean desireLongName,
            Context context) {
        return getSpnByNumeric(numeric, desireLongName, context, false, true);
    }

    private String getSpnByNumeric(String numeric, boolean desireLongName,
            Context context, boolean getCtSpn, boolean getDefaultSpn) {
        String operName = null;

        if (desireLongName) { // MVNO-API
           // ALFMS00040828 - add "46008"
           if ((numeric.equals("46000")) || (numeric.equals("46002")) ||
                   (numeric.equals("46004")) || (numeric.equals("46007")) ||
                   (numeric.equals("46008"))) {
              operName = context.getText(com.mediatek.R.string.oper_long_46000).toString();
           } else if ((numeric.equals("46001")) || (numeric.equals("46009")) ||
                   (numeric.equals("45407"))) {
               operName = context.getText(com.mediatek.R.string.oper_long_46001).toString();
           } else if ((numeric.equals("46003")) || (numeric.equals("46011")) || getCtSpn) {
               operName = context.getText(com.mediatek.R.string.oper_long_46003).toString();
           } else if (numeric.equals("46601")) {
               operName = context.getText(com.mediatek.R.string.oper_long_46601).toString();
           } else if (numeric.equals("46692")) {
               operName = context.getText(com.mediatek.R.string.oper_long_46692).toString();
           } else if (numeric.equals("46697")) {
               operName = context.getText(com.mediatek.R.string.oper_long_46697).toString();
           } else if (numeric.equals("99998")) {
               operName = context.getText(com.mediatek.R.string.oper_long_99998).toString();
           } else if (numeric.equals("99999")) {
               operName = context.getText(com.mediatek.R.string.oper_long_99999).toString();
           } else {
               // If can't found corresspoding operator in string resource,
               // lookup from spn_conf.xml
               if (getDefaultSpn && containsCarrier(numeric)) {
                   operName = getSpn(numeric);
               } else {
                   Rlog.d(LOG_TAG_EX, "Can't find long operator name for " + numeric);
               }
           }
       }
       else if (desireLongName == false) // MVNO-API
       {
           // ALFMS00040828 - add "46008"
           if ((numeric.equals("46000")) || (numeric.equals("46002")) ||
                   (numeric.equals("46004")) || (numeric.equals("46007")) ||
                   (numeric.equals("46008"))) {
               operName = context.getText(com.mediatek.R.string.oper_short_46000).toString();
           } else if ((numeric.equals("46001")) || (numeric.equals("46009")) ||
                   (numeric.equals("45407"))) {
               operName = context.getText(com.mediatek.R.string.oper_short_46001).toString();
           } else if ((numeric.equals("46003")) || (numeric.equals("46011")) || getCtSpn) {
               operName = context.getText(com.mediatek.R.string.oper_short_46003).toString();
           } else if (numeric.equals("46601")) {
               operName = context.getText(com.mediatek.R.string.oper_short_46601).toString();
           } else if (numeric.equals("46692")) {
               operName = context.getText(com.mediatek.R.string.oper_short_46692).toString();
           } else if (numeric.equals("46697")) {
               operName = context.getText(com.mediatek.R.string.oper_short_46697).toString();
           } else if (numeric.equals("99997")) {
               operName = context.getText(com.mediatek.R.string.oper_short_99997).toString();
           } else if (numeric.equals("99999")) {
               operName = context.getText(com.mediatek.R.string.oper_short_99999).toString();
           } else {
               // If can't found corresspoding operator in string resource,
               // lookup from spn_conf.xml
               if (getDefaultSpn && containsCarrier(numeric)) {
                   operName = getSpn(numeric);
               } else {
                   Rlog.d(LOG_TAG_EX, "Can't find short operator name for " + numeric);
               }
           }
       }

       return operName;
    }

    /*
     * Use to query SIM operator name for MVNO feature.
     *
     * @param subId subscription id
     * @param numeric MCC/MNC
     * @param desireLongName if get long name
     * @param context context used to get resource
     * @param defaultName default name in case of fail to find operator name
     * @return operator name
     *
     */
    public String lookupOperatorName(int subId, String numeric,
            boolean desireLongName, Context context, String defaultName) {
        String operName = null;

        // Step 1. check if phone is available.
        Phone phone = PhoneFactory.getPhone(SubscriptionManager.getPhoneId(subId));
        if (phone == null) {
            Rlog.w(LOG_TAG_EX, "lookupOperatorName getPhone null");
            return defaultName;
        }

        // Step 2. get spn from mvno parrtern.
        operName = getSpnByPattern(subId, numeric);

        // Step 3. check if need special handling (CT only).
        boolean getCtSpn = isForceGetCtSpnFromRes(subId, numeric, context, operName);

        // Step 4. get Spn by numeric from resource and spn_conf.xml
        if (operName == null || getCtSpn) {
            operName = getSpnByNumeric(numeric, desireLongName, context, getCtSpn, true);
        }

        // Step5. if didn't found any spn, return default name.
        return ((operName == null) ? defaultName : operName);
    }

    /*
     * Use to query SIM operator name for MVNO feature with default name numeric.
     *
     */
    public String lookupOperatorName(int subId, String numeric,
            boolean desireLongName, Context context) {
        return lookupOperatorName(subId, numeric, desireLongName, context, numeric);
    }

    /*
     * Use to query SIM operator name for MVNO feature with default empty name.
     *
     */
    public String lookupOperatorNameForDisplayName(int subId,
            String numeric, boolean desireLongName, Context context) {
        return lookupOperatorName(subId, numeric, desireLongName, context, null);
    }
    // MVNO-API END

    public boolean containsCarrier(String carrier) {
        return mCarrierSpnMap.containsKey(carrier);
    }

    public String getSpn(String carrier) {
        return mCarrierSpnMap.get(carrier);
    }

    // MTK-END
    public boolean containsCarrierEx(String carrier) {
        // MTK-START
        //return true;
        return containsCarrier(carrier);
        // MTK-END
    }
    public String getSpnEx(String carrier) {
        // MTK-START
        //return "";
        return getSpn(carrier);
        // MTK-END
    }
}
