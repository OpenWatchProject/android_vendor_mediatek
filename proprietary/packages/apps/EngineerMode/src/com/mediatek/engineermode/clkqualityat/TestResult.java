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

package com.mediatek.engineermode.clkqualityat;

import com.mediatek.engineermode.Elog;

import java.util.ArrayList;

/**
 * Test result.
 */
public class TestResult {
    protected static final String TAG = "ClkQualityAt/TestResult";
    public static final int CONNECTION_FAIL = -2;
    public static final int NONE = -1;
    public static final int PASS = 0;
    public static final int VENIAL = 1;
    public static final int SERIOUS = 2;
    public static final int FAIL = 3;
    public static final int MODEM_FAIL = 4;
    private static final float EMPTY_FAIL_COMP_DRIFT_RATE = 2.5f;
    private static final float OTHER_FAIL_COMP_DRIFT_RATE = 30f;
    private static final float OTHER_SERIOUS_COMP_DRIFT_RATE = 20f;
    private static final float OTHER_VENIAL_COMP_DRIFT_RATE = 10f;
    private static final float OTHER_FAIL_COMP_DRIFT = 4f;
    private static final int CHECK_SLOT = 12;
    public CharSequence x = "Time";
    public CharSequence y = "PPB/S";
    /**
     * State for a whole flow.
     *
     */
    enum ResultState {
        STATE_NOT_START,
        STATE_REF_DONE,
        STATE_DONE,
        STATE_NA
    };

    /**
     * Rule type to check if the value is pass the criterion.
     *
     */
    enum PassCriterion {
        EmptyItem,
        OtherItem
    };

    public ResultState state = ResultState.STATE_NOT_START;

    private ArrayList<Float> mCompclkdriftrate = new ArrayList<Float>();
    private ArrayList<Float> mClkdriftrate = new ArrayList<Float>();
    private ArrayList<Float> mCompClkdrift = new ArrayList<Float>();
    private float mMaxCompClkDriftRate = 0;
    private float mMaxClkDriftRate = 0;

    void addCompclkdriftrate(float time, float data) {
        mCompclkdriftrate.add(time);
        mCompclkdriftrate.add(data);
        if (Math.abs(data) > Math.abs(mMaxCompClkDriftRate)) {
            mMaxCompClkDriftRate = data;
        }
    }

    void addClkdriftrate(float time, float data) {
        mClkdriftrate.add(time);
        mClkdriftrate.add(data);
        if (Math.abs(data) > Math.abs(mMaxClkDriftRate)) {
            mMaxClkDriftRate = data;
        }
    }

    void addCompclkdrift(float data) {
        mCompClkdrift.add(data);
    }

    float getMaxCompclkdriftrate() {
        return mMaxCompClkDriftRate;
    }

    float getMaxClkDriftRate() {
        return mMaxClkDriftRate;
    }

    void calPassResult(PassCriterion criterion) {
        Elog.d(TAG, "TestResult toString: " + this.toString());
        Elog.d(TAG, "@calPassResult, maxCompclkdriftrate: " + mMaxCompClkDriftRate + " count: " + mCompclkdriftrate.size()/2);
        float absMaxCompclkdriftrate = Math.abs(mMaxCompClkDriftRate);
        // for EmptyItem
        if (criterion == PassCriterion.EmptyItem) {
            Elog.d(TAG, "@calPassResult, result for EmptyItem");
            if (absMaxCompclkdriftrate < EMPTY_FAIL_COMP_DRIFT_RATE)
                result = PASS;
            else
                result = FAIL;
            Elog.d(TAG, "@calPassResult, EmptyItem's test result: " + result);
            return;
        }

        // for OtherItem
        // get max CompClkdrift value
        float maxCalculation = 0;
        int loopCnt = mCompClkdrift.size() - CHECK_SLOT;
        for (int i=0; i<loopCnt; ++i) {
            float val = Math.abs(mCompClkdrift.get(i+CHECK_SLOT) - mCompClkdrift.get(i))/CHECK_SLOT;
            if (val > maxCalculation)
                maxCalculation = val;
        }
        Elog.d(TAG, "@calPassResult, maxCalculation: " + maxCalculation + "count: " + mCompClkdrift.size());
        // check result
        if (absMaxCompclkdriftrate < OTHER_VENIAL_COMP_DRIFT_RATE
                && maxCalculation < OTHER_FAIL_COMP_DRIFT)
            result = PASS;
        else if (absMaxCompclkdriftrate < OTHER_SERIOUS_COMP_DRIFT_RATE)
            result = VENIAL;
        else if (absMaxCompclkdriftrate < OTHER_FAIL_COMP_DRIFT_RATE)
            result = SERIOUS;
        else
            result = FAIL;
        Elog.d(TAG, "@calPassResult, OtherItem's test result: " + result);
    }

    // overall result, pass or fail
    public int result = NONE;

    public ArrayList<Float> getClkdriftrate() {
        return mClkdriftrate;
    }

   public ArrayList<Float> getCompClkDriftRate() {
        return mCompclkdriftrate;
    }

    @Override
    public String toString() {
        // TODO Auto-generated method stub
        return "mCompclkdriftrate: " + mCompclkdriftrate.toString() + "\r\n mClkdriftrate: "
                + mClkdriftrate.toString() + "\r\n mCompClkdrift: "
                + mCompClkdrift.toString() + "\r\n mMaxClkDriftRate: " + mMaxClkDriftRate +
                "\r\n mMaxCompClkDriftRate: " + mMaxCompClkDriftRate;
    }
}
