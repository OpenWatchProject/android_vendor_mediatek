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
 * MediaTek Inc. (C) 2015. All rights reserved.
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


package com.mediatek.internal.telephony.dataconnection;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.RetryManager;

import android.os.SystemProperties;
import android.text.TextUtils;
import android.telephony.DataFailCause;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.ServiceState;
import android.telephony.Rlog;
import android.util.SparseArray;

import java.util.HashMap;
import java.util.EnumSet;



public class DcFailCauseManager {
    static public final String LOG_TAG = "DcFcMgr";
    static public final boolean DBG = true;
    static public final boolean VDBG = false;

    //***** Class Variables
    private static final SparseArray<DcFailCauseManager> sDcFailCauseManager =
            new SparseArray<DcFailCauseManager>();
    private Phone mPhone;

    /**
     * The default retry configuration for default type APN. See above for the syntax.
     */
    public static final String DEFAULT_DATA_RETRY_CONFIG_OP19 =
            "max_retries=10, 720000,1440000,2880000,5760000,11520000,23040000,23040000,23040000,"
            + "23040000,46080000";

    private static final String[][] specificPLMN = {
        {"50501"}, // Telstra, OP19
        {"732101"} // Claro, OP120
    };

    public enum Operator {
        NONE(-1),
        OP19(0),
        OP120(1);

        private static final HashMap<Integer,Operator> lookup
            = new HashMap<Integer,Operator>();

        static {
            for(Operator op : EnumSet.allOf(Operator.class)) {
                lookup.put(op.getValue(), op);
            }
        }

        private int value;

        private Operator(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }

        public static Operator get(int value) {
            return lookup.get(value);
        }
    }

    // CC33: For Telcel, Claro
    private static final int[] CC33_FAIL_CAUSE_TABLE = {
        DataFailCause.USER_AUTHENTICATION,
        DataFailCause.SERVICE_OPTION_NOT_SUBSCRIBED
    };

    private enum retryConfigForDefault {
        maxRetryCount(1),
        retryTime(0),
        randomizationTime(0);

        private final int value;

        private retryConfigForDefault(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }
    }

    // CC33: For Telcel, Claro
    public enum retryConfigForCC33 {
        maxRetryCount(2),
        retryTime(45000),
        randomizationTime(0);

        private final int value;

        private retryConfigForCC33(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }
    }

    //***** Class Methods
    public static DcFailCauseManager getInstance(Phone phone) {
        if (phone != null) {
            int phoneId = phone.getPhoneId();
            if (phoneId < 0) {
                Rlog.e(LOG_TAG, "PhoneId[" + phoneId + "] is invalid!");
                return null;
            }
            DcFailCauseManager dcFcMgr = sDcFailCauseManager.get(phoneId);
            if (dcFcMgr == null) {
                Rlog.d(LOG_TAG, "For phoneId:" + phoneId + " doesn't exist, create it");
                dcFcMgr = new DcFailCauseManager(phone);
                sDcFailCauseManager.put(phoneId, dcFcMgr);
            }
            return dcFcMgr;
        }

        Rlog.e(LOG_TAG, "Can't get phone to init!");
        return null;
    }

    private DcFailCauseManager(Phone phone) {
        if (DBG) log("DcFcMgr.constructor");

        mPhone = phone;
    }

    public void dispose() {
        if (DBG) log("DcFcMgr.dispose");
        sDcFailCauseManager.remove(mPhone.getPhoneId());
    }

    public long getSuggestedRetryDelayByOp(@DataFailCause.FailCause int cause) {
        long suggestedRetryTime = RetryManager.NO_SUGGESTED_RETRY_DELAY;
        Operator operator = getSpecificNetworkOperator();

        switch (operator) {
            case OP120:
                for (int tempCause : CC33_FAIL_CAUSE_TABLE) {
                    if (cause == tempCause) {
                        suggestedRetryTime = retryConfigForCC33.retryTime.getValue();
                    }
                }
                break;
            default:
                // Do nothing
                break;
        }

        return suggestedRetryTime;
    }

    public long getRetryTimeByIndex(int idx, Operator op) {
        long retryTime = RetryManager.NO_RETRY;
        String configStr = null;

        switch (op) {
            case OP19:
                configStr = DEFAULT_DATA_RETRY_CONFIG_OP19;
                break;
            default:
                // Do nothing
                break;
        }

        if (configStr != null) {
            String[] strArray = configStr.split(",");
            try {
                retryTime = Long.parseLong(strArray[idx]);
            } catch (IndexOutOfBoundsException e) {
                if (DBG) {
                    loge("get retry time by index fail");
                }
                e.printStackTrace();
            }
        }

        return retryTime;
    }

    public boolean isSpecificNetworkAndSimOperator(Operator op) {
        if (op == null) {
            if (DBG) {
                loge("op is null, return false!");
            }
            return false;
        }

        Operator networkOp = getSpecificNetworkOperator();
        Operator simOp = getSpecificSimOperator();

        return op == networkOp && op == simOp;
    }

    public boolean isSpecificNetworkOperator(Operator op) {
        if (op == null) {
            if (DBG) {
                loge("op is null, return false!");
            }
            return false;
        }

        Operator networkOp = getSpecificNetworkOperator();
        return op == networkOp;
    }

    public boolean isNetworkOperatorForCC33() {
        Operator networkOp = getSpecificNetworkOperator();

        switch (networkOp) {
            case OP120:
                return true;
            default:
                // Do nothing
                break;
        }
        return false;
    }

    private Operator getSpecificNetworkOperator() {
        Operator operator = Operator.NONE;
        String plmn = "";
        try {
            plmn = TelephonyManager.getDefault().getNetworkOperatorForPhone(mPhone.getPhoneId());
            if (DBG) {
                log("Check PLMN=" + plmn);
            }
        } catch (Exception e) {
            if (DBG) {
                loge("get plmn fail");
            }
            e.printStackTrace();
        }

        for (int i = 0; i < specificPLMN.length; i++) {
            //reset flag
            boolean isServingInSpecificPlmn = false;

            //check if serving in specific plmn
            for (int j = 0; j < specificPLMN[i].length; j++) {
                if (plmn.equals(specificPLMN[i][j])) {
                    isServingInSpecificPlmn = true;
                    break;
                }
            }

            if (isServingInSpecificPlmn == true) {
                operator = Operator.get(i);
                log("Serving in specific network op=" + operator + "(" + i + ")");
                break;
            }
        }

        return operator;
    }

    private Operator getSpecificSimOperator() {
        Operator operator = Operator.NONE;
        String hplmn = "";
        try {
            hplmn = TelephonyManager.getDefault().getSimOperatorNumericForPhone(
                    mPhone.getPhoneId());
            if (DBG) {
                log("Check HPLMN=" + hplmn);
            }
        } catch (Exception e) {
            if (DBG) {
                loge("get hplmn fail");
            }
            e.printStackTrace();
        }

        for (int i = 0; i < specificPLMN.length; i++) {
            //reset flag
            boolean isServingInSpecificPlmn = false;

            //check if serving in specific plmn
            for (int j = 0; j < specificPLMN[i].length; j++) {
                if (hplmn.equals(specificPLMN[i][j])) {
                    isServingInSpecificPlmn = true;
                    break;
                }
            }

            if (isServingInSpecificPlmn == true) {
                operator = Operator.get(i);
                log("Serving in specific sim op=" + operator + "(" + i + ")");
                break;
            }
        }

        return operator;
    }

    @Override
    public String toString() {
        String ret = "sDcFailCauseManager: " + sDcFailCauseManager;
        return ret;
    }

    private void loge(String s) {
        Rlog.e(LOG_TAG, s);
    }

    private void log(String s) {
        Rlog.d(LOG_TAG, s);
    }
}
