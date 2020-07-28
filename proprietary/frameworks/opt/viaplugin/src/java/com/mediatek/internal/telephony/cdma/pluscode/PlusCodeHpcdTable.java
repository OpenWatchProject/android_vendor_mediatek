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

package com.mediatek.internal.telephony.cdma.pluscode;

import android.telephony.Rlog;

import java.util.ArrayList;
import java.util.List;

/**
 * Plus Code HPCD table.
 * @hide
 */
public class PlusCodeHpcdTable {

    private static PlusCodeHpcdTable sInstance;

    static final Object sInstSync = new Object();

    static final String LOG_TAG = "PlusCodeHpcdTable";
    private static final boolean DBG = true;

    static final int PARAM_FOR_OFFSET = 2;

    private static final MccIddNddSid[] MccIddNddSidMap = TelephonyPlusCode.MCC_IDD_NDD_SID_MAP;

    private static final MccSidLtmOff[] MccSidLtmOffMap = TelephonyPlusCode.MCC_SID_LTM_OFF_MAP;

    /**
     * @return the single instance.
     */
    public static PlusCodeHpcdTable getInstance() {
        synchronized (sInstSync) {
            if (sInstance == null) {
                sInstance = new PlusCodeHpcdTable();
            }
        }
        return sInstance;
    }

    private PlusCodeHpcdTable() {
        // Do nothing.
    }

    /**
     * Get CC from MccIddNddSidMap by mcc value.
     * @param sMcc the MCC
     * @return CC
     */
    public static MccIddNddSid getCcFromTableByMcc(String sMcc) {
        Rlog.d(LOG_TAG, " getCcFromTableByMcc mcc = " + sMcc);
        if (sMcc == null || sMcc.length() == 0) {
            Rlog.d(LOG_TAG, "[getCcFromTableByMcc] please check the param ");
            return null;
        }

        int mcc;
        try {
            mcc = Integer.parseInt(sMcc);
        } catch (NumberFormatException e) {
            Rlog.e(LOG_TAG, android.util.Log.getStackTraceString(e));
            return null;
        }
        int size = MccIddNddSidMap.length;

        MccIddNddSid mccIddNddSid = null;

        /*int high = size - 1, low = 0, guess;
        while (high - low > 1) {
            guess = (high + low) / 2;
            mccIddNddSid = (MccIddNddSid) MccIddNddSidMap[guess];

            int temMcc = mccIddNddSid.getMcc();
            if (temMcc < mcc) {
                low = guess;
            } else {
                high = guess;
            }
        }*/

        Rlog.d(LOG_TAG, " getCcFromTableByMcc size = " + size);
        int find = -1;
        for (int i = 0; i < size; i++) {
            mccIddNddSid = (MccIddNddSid) MccIddNddSidMap[i];
            int tempMcc = mccIddNddSid.getMcc();
            //Rlog.d(LOG_TAG, " getCcFromTableByMcc tempMcc = " + tempMcc);
            if (tempMcc == mcc) {
                find = i;
                break;
            }
        }

        Rlog.d(LOG_TAG, " getCcFromTableByMcc find = " + find);
        if (find > -1 && find < size) {
            mccIddNddSid = (MccIddNddSid) MccIddNddSidMap[find];
            Rlog.d(LOG_TAG, "Now find Mcc = " + mccIddNddSid.mMcc + ", Mcc = "
                    + mccIddNddSid.mCc + ", SidMin = " + Rlog.pii(LOG_TAG, mccIddNddSid.mSidMin)
                    + ", SidMax = " + Rlog.pii(LOG_TAG, mccIddNddSid.mSidMax) + ", Idd = "
                    + mccIddNddSid.mIdd + ", Ndd = " + mccIddNddSid.mNdd);
            return mccIddNddSid;
        } else {
            Rlog.d(LOG_TAG, "can't find one that match the Mcc");
            return null;
        }
    }

    /**
     * Get MCC from conflicts table by sid. If Conlicts, there was more than one value,
     * so add into list. If not, there was only one value in the list.
     * @param sSid the SID
     * @return MCC
     */
    public static ArrayList<String> getMccFromConflictTableBySid(String sSid) {
        Rlog.d(LOG_TAG, " [getMccFromConflictTableBySid] sid = " + Rlog.pii(LOG_TAG, sSid));
        if (sSid == null || sSid.length() == 0 || sSid.length() > 5) {
            Rlog.d(LOG_TAG, "[getMccFromConflictTableBySid] please check the param ");
            return null;
        }

        int sid;
        try {
            sid = Integer.parseInt(sSid);
        } catch (NumberFormatException e) {
            Rlog.e(LOG_TAG, android.util.Log.getStackTraceString(e));
            return null;
        }

        if (sid < 0) {
            return null;
        }

        ArrayList<String> mccArrays = new ArrayList<String>();
        MccSidLtmOff mccSidLtmOff = null;
        int mccSidMapSize = MccSidLtmOffMap.length;
        Rlog.d(LOG_TAG, " [getMccFromConflictTableBySid] mccSidMapSize = " + mccSidMapSize);
        for (int i = 0; i < mccSidMapSize; i++) {
            mccSidLtmOff = (MccSidLtmOff) MccSidLtmOffMap[i];
            if (mccSidLtmOff != null && mccSidLtmOff.mSid == sid) {
                mccArrays.add(Integer.toString(mccSidLtmOff.mMcc));
                Rlog.d(LOG_TAG, "mccSidLtmOff  Mcc = " + mccSidLtmOff.mMcc
                        + ", Sid = " + Rlog.pii(LOG_TAG, mccSidLtmOff.mSid) + ", LtmOffMin = "
                        + mccSidLtmOff.mLtmOffMin + ", LtmOffMax = "
                        + mccSidLtmOff.mLtmOffMax);
            }
        }

        return mccArrays;
    }

    /**
     * Get CC from MccIddNddSidMap by sid.
     * @param sSid the SID.
     * @return the CC.
     */
    public static MccIddNddSid getCcFromMINSTableBySid(String sSid) {
        Rlog.d(LOG_TAG, " [getCcFromMINSTableBySid] sid = " + Rlog.pii(LOG_TAG, sSid));
        if (sSid == null || sSid.length() == 0 || sSid.length() > 5) {
            Rlog.d(LOG_TAG, "[getCcFromMINSTableBySid] please check the param ");
            return null;
        }

        int sid;
        try {
            sid = Integer.parseInt(sSid);
        } catch (NumberFormatException e) {
            Rlog.e(LOG_TAG, android.util.Log.getStackTraceString(e));
            return null;
        }
        if (sid < 0) {
            return null;
        }

        MccIddNddSid mccIddNddSid = null;
        MccIddNddSid findMccIddNddSid = null;

        int size = MccIddNddSidMap.length;
        for (int i = 0; i < size; i++) {
            mccIddNddSid = (MccIddNddSid) MccIddNddSidMap[i];
            if (sid <= mccIddNddSid.mSidMax && sid >= mccIddNddSid.mSidMin) {
                findMccIddNddSid = mccIddNddSid;
                break;
            }
        }

        if (DBG) {
            Rlog.d(LOG_TAG, " getCcFromMINSTableBySid findMccIddNddSid = "
                    + Rlog.pii(LOG_TAG, findMccIddNddSid));
        }
        return findMccIddNddSid;

    }

    /**
     * Get CC from MccIddNddSidMap by ltm_off.
     * @param mccArray the MCC array.
     * @param ltmOff the LTM off.
     * @return the CC
     */
    public String getCcFromMINSTableByLTM(List<String> mccArray, String ltmOff) {
        Rlog.d(LOG_TAG, " getCcFromMINSTableByLTM sLtm_off = " + ltmOff);
        if (ltmOff == null || ltmOff.length() == 0 || mccArray == null || mccArray.size() == 0) {
            Rlog.d(LOG_TAG, "[getCcFromMINSTableByLTM] please check the param ");
            return null;
        }

        String findMcc = null;

        int ltmoff;
        try {
            ltmoff = Integer.parseInt(ltmOff);
        } catch (NumberFormatException e) {
            Rlog.e(LOG_TAG, android.util.Log.getStackTraceString(e));
            return null;
        }

        Rlog.d(LOG_TAG, "[getCcFromMINSTableByLTM]  ltm_off =  " + ltmoff);

        int findOutMccSize = mccArray.size();
        if (findOutMccSize > 1 && MccSidLtmOffMap != null) {
            int mccSidMapSize = MccSidLtmOffMap.length;
            if (DBG) {
                Rlog.d(LOG_TAG, " Conflict FindOutMccSize = " + findOutMccSize);
            }

            MccSidLtmOff mccSidLtmOff = null;
            int mcc = -1;
            for (int i = 0; i < findOutMccSize; i++) {
                try {
                    mcc = Integer.parseInt(mccArray.get(i));
                } catch (NumberFormatException e) {
                    Rlog.e(LOG_TAG, android.util.Log.getStackTraceString(e));
                    return null;
                }

                Rlog.d(LOG_TAG, " Conflict mcc = " + mcc + ",index = " + i);
                for (int j = 0; j < mccSidMapSize; j++) {
                    mccSidLtmOff = (MccSidLtmOff) MccSidLtmOffMap[j];
                    if (mccSidLtmOff.mMcc == mcc) {

                        int max = (mccSidLtmOff.mLtmOffMax) * PARAM_FOR_OFFSET;
                        int min = (mccSidLtmOff.mLtmOffMin) * PARAM_FOR_OFFSET;

                        Rlog.d(LOG_TAG, "mccSidLtmOff LtmOffMin = "
                                + mccSidLtmOff.mLtmOffMin + ", LtmOffMax = "
                                + mccSidLtmOff.mLtmOffMax);
                        if (ltmoff <= max && ltmoff >= min) {
                            findMcc = mccArray.get(i);
                            break;
                        }
                    }
                }
            }
        } else {
            findMcc = mccArray.get(0);
        }

        Rlog.d(LOG_TAG, "find one that match the ltm_off mcc = " + findMcc);
        return findMcc;
    }

    /**
     * Get mcc from conflict table by sid ltm off.
     * @param sSid the SID
     * @param sLtmOff the LTM off
     * @return MCC
     */
    public static String getMccFromConflictTableBySidLtmOff(String sSid, String sLtmOff) {
        Rlog.d(LOG_TAG, " [getMccFromConflictTableBySidLtmOff] sSid = " + Rlog.pii(LOG_TAG, sSid)
                + ", sLtm_off = " + sLtmOff);
        if (sSid == null || sSid.length() == 0 || sSid.length() > 5
                || sLtmOff == null || sLtmOff.length() == 0) {
            Rlog.d(LOG_TAG, "[getMccFromConflictTableBySidLtmOff] please check the param ");
            return null;
        }

        int sid;
        try {
            sid = Integer.parseInt(sSid);
        } catch (NumberFormatException e) {
            Rlog.e(LOG_TAG, android.util.Log.getStackTraceString(e));
            return null;
        }
        if (sid < 0) {
            return null;
        }

        int ltmoff;
        try {
            ltmoff = Integer.parseInt(sLtmOff);
        } catch (NumberFormatException e) {
            Rlog.e(LOG_TAG, android.util.Log.getStackTraceString(e));
            return null;
        }

        Rlog.d(LOG_TAG, " [getMccFromConflictTableBySidLtmOff] sid = " + Rlog.pii(LOG_TAG, sid));

        int mccSidMapSize = MccSidLtmOffMap.length;
        Rlog.d(LOG_TAG, " [getMccFromConflictTableBySidLtmOff] mccSidMapSize = " + mccSidMapSize);

        MccSidLtmOff mccSidLtmOff = null;
        for (int i = 0; i < mccSidMapSize; i++) {
            mccSidLtmOff = MccSidLtmOffMap[i];

            int max = (mccSidLtmOff.mLtmOffMax) * PARAM_FOR_OFFSET;
            int min = (mccSidLtmOff.mLtmOffMin) * PARAM_FOR_OFFSET;

            Rlog.d(LOG_TAG,
                    "[getMccFromConflictTableBySidLtmOff] mccSidLtmOff.Sid = "
                    + Rlog.pii(LOG_TAG, mccSidLtmOff.mSid) + ", sid = " + Rlog.pii(LOG_TAG, sid)
                    + ", ltm_off = " + ltmoff + ", max = " + max
                    + ", min = " + min);

            if (mccSidLtmOff.mSid == sid && (ltmoff <= max && ltmoff >= min)) {
                String mccStr = Integer.toString(mccSidLtmOff.mMcc);
                Rlog.d(LOG_TAG, "[getMccFromConflictTableBySidLtmOff] Mcc = " + mccStr);
                return mccStr;
            }
        }

        return null;
    }

    /**
     * Get mcc from MINS table by sid.
     * @param sSid the SID
     * @return MCC
     */
    public static String getMccFromMINSTableBySid(String sSid) {
        Rlog.d(LOG_TAG, " [getMccFromMINSTableBySid] sid = " + Rlog.pii(LOG_TAG, sSid));
        if (sSid == null || sSid.length() == 0 || sSid.length() > 5) {
            Rlog.d(LOG_TAG, "[getMccFromMINSTableBySid] please check the param ");
            return null;
        }

        int sid;
        try {
            sid = Integer.parseInt(sSid);
        } catch (NumberFormatException e) {
            Rlog.e(LOG_TAG, android.util.Log.getStackTraceString(e));
            return null;
        }
        if (sid < 0) {
            return null;
        }

        MccIddNddSid mccIddNddSid = null;

        int size = MccIddNddSidMap.length;
        Rlog.d(LOG_TAG, " [getMccFromMINSTableBySid] size = " + size);

        for (int i = 0; i < size; i++) {
            mccIddNddSid = MccIddNddSidMap[i];

            Rlog.d(LOG_TAG, " [getMccFromMINSTableBySid] sid = " + Rlog.pii(LOG_TAG, sid)
                    + ", mccIddNddSid.SidMin = " + Rlog.pii(LOG_TAG, mccIddNddSid.mSidMin)
                    + ", mccIddNddSid.SidMax = " + Rlog.pii(LOG_TAG, mccIddNddSid.mSidMax));

            if (sid >= mccIddNddSid.mSidMin && sid <= mccIddNddSid.mSidMax) {
                String mccStr = Integer.toString(mccIddNddSid.mMcc);
                Rlog.d(LOG_TAG, "[queryMccFromConflictTableBySid] Mcc = " + mccStr);
                return mccStr;
            }

        }

        return null;
    }

    /**
     * Get mccmnc from SidMccMncList by sid.
     * @param sSid the SID
     * @return MCCMNC
     */
    public static String getMccMncFromSidMccMncListBySid(String sSid) {
        Rlog.d(LOG_TAG, " [getMccMncFromSidMccMncListBySid] sid = " + Rlog.pii(LOG_TAG, sSid));
        if (sSid == null || sSid.length() == 0 || sSid.length() > 5) {
            Rlog.d(LOG_TAG, "[getMccMncFromSidMccMncListBySid] please check the param ");
            return null;
        }

        int sid;
        try {
            sid = Integer.parseInt(sSid);
        } catch (NumberFormatException e) {
            Rlog.e(LOG_TAG, android.util.Log.getStackTraceString(e));
            return null;
        }
        if (sid < 0) {
            return null;
        }

        List<SidMccMnc> mSidMccMncList = TelephonyPlusCode.getSidMccMncList();
        SidMccMnc mSidMccMnc = null;
        int left = 0;
        int right = mSidMccMncList.size() - 1;
        int mid;
        int mccMnc = 0;

        while (left <= right) {
            mid = (left + right) / 2;
            mSidMccMnc = mSidMccMncList.get(mid);
            if (sid < mSidMccMnc.mSid) {
                right = mid - 1;
            } else if (sid > mSidMccMnc.mSid) {
                left = mid + 1;
            } else {
                mccMnc = mSidMccMnc.mMccMnc;
                break;
            }
        }

        if (mccMnc != 0) {
            String mccMncStr = Integer.toString(mccMnc);
            Rlog.d(LOG_TAG, "[getMccMncFromSidMccMncListBySid] MccMncStr = " + mccMncStr);
            return mccMncStr;
        } else {
            return null;
        }
    }
}
