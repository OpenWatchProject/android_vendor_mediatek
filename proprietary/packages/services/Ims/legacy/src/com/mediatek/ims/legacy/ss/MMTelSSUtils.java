/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.ims.legacy.ss;

import android.content.Context;
import android.net.Network;

import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
/// SS OP01 Ut @{
import java.util.Arrays;
import java.util.List;
import java.util.Random;
/// @}
import javax.net.SocketFactory;

import android.os.Build;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;

import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.ims.ImsException;
import com.android.ims.ImsManager;

import com.google.i18n.phonenumbers.PhoneNumberUtil;

import com.mediatek.internal.telephony.dataconnection.MtkDcHelper;
import com.mediatek.internal.telephony.MtkSuppServHelper;
import com.mediatek.ims.internal.ImsXuiManager;
import com.mediatek.ims.SuppSrvConfig;
import com.mediatek.simservs.client.SimServs;
import com.mediatek.telephony.MtkTelephonyManagerEx;


/**
 * Implementation for MMTel SS Utils.
 *
 * {@hide}
 *
 */
public class MMTelSSUtils {
    private static final String LOG_TAG = "MMTelSSUtils";

    // Sensitive log task
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean SENLOG = TextUtils.equals(Build.TYPE, "user");
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    //Following Constants definition must be same with EngineerMode/ims/ImsActivity.java
    private final static String PROP_SS_MODE = "persist.vendor.radio.ss.mode";
    private final static String MODE_SS_XCAP = "Prefer XCAP";
    private final static String MODE_SS_CS = "Prefer CS";

    private static final String SS_SERVICE_CLASS_PROP = "vendor.gsm.radio.ss.sc";

    private static SimServs mSimservs = null;

    public static SimServs initSimserv(Context context, int phoneId) {
        mSimservs = SimServs.getInstance();
        if (mSimservs.getPhoneId() != phoneId) {
            Rlog.d(LOG_TAG, "[initSimserv] old PhoneId:" + mSimservs.getPhoneId()
                    + " new PhoneId:" + phoneId);
            mSimservs.resetParameters();
            mSimservs.setPhoneId(phoneId);
        }

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(context);

        mSimservs.setUseHttpProtocolScheme(ssConfig.isUseHttpProtocolScheme());
        if (ssConfig.getElementContentType() != null && !ssConfig.getElementContentType().isEmpty()) {
            mSimservs.setElementUpdateContentType(false, ssConfig.getElementContentType());
        }
        mSimservs.setHandleError409(ssConfig.isHandleError409());
        mSimservs.setFillCompleteForwardTo(ssConfig.isFillCompleteForwardTo());
        mSimservs.setXcapNSPrefixSS(ssConfig.isXcapNsPrefixSS());
        mSimservs.setSimservQueryWhole(ssConfig.isQueryWholeSimServ());
        mSimservs.setETagDisable(ssConfig.isDisableEtag());
        mSimservs.setAttrNeedQuotationMark(ssConfig.isAttrNeedQuotationMark());
        if (ssConfig.getAUID() != null && !ssConfig.getAUID().isEmpty()) {
            mSimservs.setAUID(ssConfig.getAUID());
        }
        if (ssConfig.getXcapRoot() != null && !ssConfig.getXcapRoot().isEmpty()) {
            mSimservs.setXcapRoot(ssConfig.getXcapRoot());
        }

        mSimservs.setXui(getXui(phoneId, context));

        String xcapRoot = getXcapRootUri(phoneId, context);
        if (xcapRoot != null && !xcapRoot.isEmpty()) {
            mSimservs.setXcapRoot(addXcapRootPort(xcapRoot, phoneId, context));
        }

        mSimservs.setIntendedId(getXIntendedId(phoneId, context));
        mSimservs.setContext(context);
        mSimservs.setPhoneId(phoneId);

        XcapMobileDataNetworkManager.setKeepAliveTimer(ssConfig.getDataKeepAliveTimer());
        if (ssConfig.getRequestDataTimer() > 0) {
            XcapMobileDataNetworkManager.setRequestDataTimer(ssConfig.getRequestDataTimer());
        }
        if (ssConfig.getDataCoolDownTimer() > 0) {
            XcapMobileDataNetworkManager.setDataCoolDownTimer(ssConfig.getDataCoolDownTimer());
        }

        return mSimservs;
    }

    /**
     * Get the XCAP Root URI for the specific phone ID.
     *
     * @param phoneId phone index
     * @return the Root URI String
     */
    public static String getXcapRootUri(int phoneId, Context context) {
        SimServs simSrv = SimServs.getInstance();
        String rootUri = simSrv.getXcapRoot();
        Rlog.d(LOG_TAG, "getXcapRootUri():" + rootUri);
        int subId = getSubIdUsingPhoneId(phoneId);
        if (rootUri == null) {

            // still null, assemble it
            String impi = null;
            impi = MtkTelephonyManagerEx.getDefault().getIsimImpi(subId);

            if (isValidIMPI(impi)) {  // ISIM
                Rlog.d(LOG_TAG, "getXcapRootUri():get APP_FAM_IMS and impi=" +
                        MtkSuppServHelper.encryptString(impi));
                mSimservs.setXcapRootByImpi(impi);
            } else {
                String mccMnc = null;
                if (MtkDcHelper.isCdma4GDualModeCard(phoneId)) {
                    mccMnc = MtkTelephonyManagerEx.getDefault()
                              .getSimOperatorNumericForPhoneEx(phoneId)[0];
                    if (mccMnc == null || mccMnc.length() <= 0) {
                        mccMnc = TelephonyManager.getDefault().getSimOperator(subId);
                    }
                } else {
                    mccMnc = TelephonyManager.getDefault().getSimOperator(subId);
                }

                String mcc = "";
                String mnc = "";
                if (!TextUtils.isEmpty(mccMnc)) {
                    mcc = mccMnc.substring(0, 3);
                    mnc = mccMnc.substring(3);
                }

                if (mnc.length() == 2) {
                    mccMnc = mcc + 0 + mnc;
                    Rlog.d(LOG_TAG, "add 0 to mnc =" + mnc);
                }
                Rlog.d(LOG_TAG, "get mccMnc=" + mccMnc + " from the IccRecrods");

                if (!TextUtils.isEmpty(mccMnc)) {
                    if (mccMnc.equals("460000") || mccMnc.equals("460002")
                            || mccMnc.equals("460007") || mccMnc.equals("460008")
                            || mccMnc.equals("460004")) {
                        mSimservs.setXcapRootByMccMnc("460", "000");
                    } else {
                        mSimservs.setXcapRootByMccMnc(mccMnc.substring(0, 3),
                                mccMnc.substring(3));
                    }
                }
            }
            rootUri = mSimservs.getXcapRoot();
            Rlog.d(LOG_TAG, "getXcapRoot():rootUri=" + rootUri);
        }

        return rootUri;
    }

    /**
     * Get the XCAP XUI for the specific phone ID.
     * @param phoneId phone index
     * @return the XUI String
     */
    public static String getXui(int phoneId, Context context) {
        String  sXui = ImsXuiManager.getInstance().getXui(phoneId);
        Rlog.d(LOG_TAG, "getXui():sXui from XuiManager = " + MtkSuppServHelper.encryptString(sXui));
        int subId = getSubIdUsingPhoneId(phoneId);
        if (sXui == null) {
            //Get XUI (P-Associated-URI header) from storage (updated by IMSA) (IR.92) - wait for
            // XuiManager check-in by AF10
            //[ALPS01654778] As XCAP User Identity (XUI) the UE must use the default public user
            // identity received in P-Associated-URI header in 200 OK for REGISTER
            //IMS Stack will update IMPUs to ImsSimservsDispatcher's handleXuiUpdate(): Store
            // IMPUs to ImsXuiManager
            //Example (from main log):ImsSimservsDispatcher: [ims] ImsSimservsDispatcher
            // handleXuiUpdate xui=sip:14253269846@msg.pc.t-mobile.com,
            //sip:+14253269846@msg.pc.t-mobile.com,
            //sip:+14253269846@ims.mnc260.mcc310.3gppnetwork.org,
            //sip:310260007540985@ims.mnc260.mcc310.3gppnetwork.org
            //XUI = sip:+8613810290014@bj.ims.mnc000.mcc460.3gppnetwork.org,tel:+8613810290014
            //XUI= tel:+8613810290014,sip:+8613810290014@bj.ims.mnc000.mcc460.3gppnetwork.org
            SimServs simSrv = SimServs.getInstance();
            sXui = simSrv.getXui();
            Rlog.d(LOG_TAG, "getXui():sXui from simSrv=" + MtkSuppServHelper.encryptString(sXui));
            if (sXui != null) {
                sXui = getSipUriFromXui(sXui);
                simSrv.setXui(sXui);
                return sXui;
            } else {
                //Check if ISIM or SIM is inserted or not
                //ISIM is ready but it may not read all records successfully at this time
                //[TODO] getImpu() from ISIM:API is not available (Set IMPU if ISIM inserted)
                String sImpu = "";
                String[] impu = null;
                impu = MtkTelephonyManagerEx.getDefault().getIsimImpu(subId);

                if (impu != null && impu[0] != null && !impu[0].isEmpty()) {
                    sImpu = impu[0];
                    Rlog.d(LOG_TAG, "getXui():sImpu=" + MtkSuppServHelper.encryptString(sImpu));
                    mSimservs.setXuiByImpu(sImpu);
                } else {
                    //SIM/USIM is ready but it may not read all records successfully at this time
                    TelephonyManager telephonyManager =
                        (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
                    String sImsi = telephonyManager.getSubscriberId(getSubIdUsingPhoneId(phoneId));
                    Rlog.d(LOG_TAG, "getXui():IMS uiccApp is null, try to select USIM uiccApp");

                    String mccMnc = "";

                    if (MtkDcHelper.isCdma4GDualModeCard(phoneId)) {
                        mccMnc = MtkTelephonyManagerEx.getDefault()
                                  .getSimOperatorNumericForPhoneEx(phoneId)[0];
                        if (mccMnc == null || mccMnc.length() <= 0) {
                            mccMnc = TelephonyManager.getDefault().getSimOperator(subId);
                        }
                    } else {
                        mccMnc = TelephonyManager.getDefault().getSimOperator(subId);
                    }

                    Rlog.d(LOG_TAG, "getXui():Imsi=" + MtkSuppServHelper.encryptString(sImsi) + ", mccMnc=" + mccMnc);

                    if (!TextUtils.isEmpty(mccMnc)) {
                        mSimservs.setXuiByImsiMccMnc(sImsi, mccMnc.substring(0, 3),
                                mccMnc.substring(3));
                    }
                }

                //Originally:sXui is null. Now:re-obtain sXui again
                sXui = mSimservs.getXui();
                Rlog.d(LOG_TAG, "getXui():sXui=" + MtkSuppServHelper.encryptString(sXui));
                return sXui;
            }
        } else {
            sXui = getSipUriFromXui(sXui);
            return sXui;
        }
    }

    /**
     * Get the XIntended Id for the specific phone ID.
     * @param phoneId phone index
     * @return the XIntendedId String
     */
    public static String getXIntendedId(int phoneId, Context context) {
        //[ALPS01654778] Modify for TMO-US XCAP Test: It requires that XUI and
        // X-3GPP-Intended-Identify must be same
        return getXui(phoneId, context);
    }

    /**
     * Check if we use Ut/XCAP.
     * @param phoneId phone index
     * @return ture if use Ut/XCAP
     */
    public static boolean isPreferXcap(int phoneId, Context context) {
        boolean r = true;
        String ssMode = MODE_SS_CS;
        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(context);

        if (!SystemProperties.get("persist.vendor.ims_support").equals("1")
                || !SystemProperties.get("persist.vendor.volte_support").equals("1")) {
            Rlog.d(LOG_TAG, "isPreferXcap(): Not Enable VOLTE feature!");
            return false;
        }

        if (SystemProperties.get("persist.vendor.ims_support").equals("1") &&
                SystemProperties.get("persist.vendor.volte_support").equals("1")) {
            ssMode = SystemProperties.get(PROP_SS_MODE, MODE_SS_XCAP);
            if (ssConfig.isNotSupportXcap()) {
                ssMode = MODE_SS_CS;
            }
        } else {
            ssMode = SystemProperties.get(PROP_SS_MODE, MODE_SS_CS);
        }

        if (MODE_SS_CS.equals(ssMode)) {
            r = false;
        }

        Rlog.d(LOG_TAG, "isPreferXcap() " + ssMode);
        return r;
    }

    public static String addXcapRootPort(String xcapRoot, int phoneId, Context context) {
        //xcapRoot without specific port number
        if ("http".equals(xcapRoot.substring(0, xcapRoot.lastIndexOf(':')))
               || "https".equals(
               xcapRoot.substring(0, xcapRoot.lastIndexOf(':')))) {
            if (xcapRoot.charAt(xcapRoot.length() - 1) == '/') {
                xcapRoot = xcapRoot.substring(0, xcapRoot.length() - 1);
            }

            SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(context);

            int port = ssConfig.getPort();
            if (port != 0) {
                xcapRoot += ":" + port;
            }

            xcapRoot += "/";
        }


        return xcapRoot;
    }


    public static String appendCountryCode(String dialNumber, int phoneId) {
        // if country code is 886:
        // Case 1: input: 123456789, output: +886123456789
        // Case 2: input: 886123456789, output: +886886123456789
        // Case 3: input: +886123456789, output: +886123456789  (no change)
        // Case 4: input: +86123456789, output: +86123456789 (no change)
        // Case 5: input: 86123456789, output: +88686123456789

        String currIso = TelephonyManager.getDefault().getNetworkCountryIsoForPhone(phoneId);
        Rlog.d(LOG_TAG, "currIso: " + currIso);

        int countryCode = PhoneNumberUtil.getInstance().getCountryCodeForRegion(currIso.toUpperCase());

        if (countryCode == 0) {
            Rlog.d(LOG_TAG, "Country code not found.");
            return dialNumber;
        }

        String countryCodeStr = Integer.toString(countryCode);

        if (dialNumber == null || dialNumber.isEmpty() || countryCodeStr == null) {
            return dialNumber;
        }

        if (dialNumber.substring(0, 1).equals("+")) {
            Rlog.d(LOG_TAG, "No need to append country code: " + MtkSuppServHelper.encryptString(dialNumber));
            return dialNumber;
        } else {
            String dialNumberWithCountryCode = "+" + countryCodeStr + dialNumber;
            Rlog.d(LOG_TAG, "dialNumberWithCountryCode: " + MtkSuppServHelper.encryptString(dialNumberWithCountryCode));
            return dialNumberWithCountryCode;
        }
    }

    /**
     * Check valid IMPI.
     * @param impi IMPI
     * @return ture if it is valid IMPI
     */
    public static boolean isValidIMPI(String impi) {
        Rlog.d(LOG_TAG, "isValidIMPI, impi= " + MtkSuppServHelper.encryptString(impi));

        boolean validIMPI = (impi != null) && (!impi.isEmpty()) && (impi.contains("@"));
        return validIMPI;
    }

    public static int getServiceClass() {
        return Integer.parseInt(SystemProperties.get(SS_SERVICE_CLASS_PROP, "-1"));
    }

    public static void resetServcieClass() {
        SystemProperties.set(SS_SERVICE_CLASS_PROP, "-1");
    }

    /**
     * prefter to get the sip uri from Xui.
     * @param sXui Xui
     * @return sip uri from Xui if it contains
     */
    public static String getSipUriFromXui(String sXui) {
        String  sipXui = null;
        String[]  sXuiArray = sXui.split(",");
        boolean isContainSipUri = false;
        for (int i = 0; i < sXuiArray.length; i++) {
           if (sXuiArray[i] != null && !sXuiArray[i].isEmpty() &&
                   sXuiArray[i].contains("sip:")) {
               sipXui = sXuiArray[i];
               isContainSipUri = true;
               break;
           }
        }
        if (isContainSipUri == false) {
            sipXui = sXuiArray[0];
        }
        Rlog.d(LOG_TAG, "getSipUriFromXui: " + MtkSuppServHelper.encryptString(sipXui));
        return sipXui;
    }

    private static int getSubIdUsingPhoneId(int phoneId) {
        int [] values = SubscriptionManager.getSubId(phoneId);
        if(values == null || values.length <= 0) {
            return SubscriptionManager.getDefaultSubscriptionId();
        }
        else {
            return values[0];
        }
    }
}
