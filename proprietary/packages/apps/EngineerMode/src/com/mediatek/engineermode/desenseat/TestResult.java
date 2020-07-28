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

package com.mediatek.engineermode.desenseat;

import android.annotation.SuppressLint;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/**
 * Test result.
 */
public class TestResult {
    public static final int CONNECTION_FAIL = -2;
    public static final int NONE = -1;
    public static final int FAIL = 0;
    public static final int PASS = 1;
    public static final int CNR_FAIL = 2;
    public static final int MODEM_FAIL = 3;
    /**
     * State for a whole flow.
     *
     */
    enum ResultState {
        STATE_NOT_START,
        STATE_REF_DONE,
        STATE_DONE,
        STATE_NA
    } ;
    public ResultState state = ResultState.STATE_NOT_START;


    /**
     * Data for one test.
     *
     */
    class Entry {
        public float desense;
        public float average;
        public float averageBase;
        public float averageTest;
        public float max;
        public float min;
        public ArrayList<Float> data = new ArrayList<Float>();
    }

    // overall result, pass or fail
    public int result = NONE;

    // detail result for each sv id
    @SuppressLint("UseSparseArrays")
    public Map<Integer, Entry> subResults = new HashMap<Integer, Entry>();

    public CharSequence x = "Time";
    public CharSequence y = "CNR";

    void addData(int svid, float data) {
        if (subResults.get(svid) == null) {
            subResults.put(svid, new Entry());
        }
        subResults.get(svid).data.add(data);
    }

    void setAverageCNR(int svid, float data, boolean base) {
        if (subResults.get(svid) == null) {
            subResults.put(svid, new Entry());
        }

        Entry result = subResults.get(svid);
        if (base) {
            result.averageBase = data;
            result.average = data;
        } else {
            result.averageTest = data;
            result.average = (result.averageBase + result.averageTest) / 2;
        }
    }

    float getAverage(int svid) {

        if (subResults.get(svid) == null) {
            return 0;
        } else {
            return subResults.get(svid).average;
        }
    }

    void setDesenseValue(int svid, float data) {
        if (subResults.get(svid) == null) {
            subResults.put(svid, new Entry());
        }
        subResults.get(svid).desense = data;
    }

    // get the desense value of a specified sv id
    Float getDesenseValue(int svid) {
        if (subResults.get(svid) != null) {
            return subResults.get(svid).desense;
        }
        return null;
    }

    String getSummary() {
        float min = Float.POSITIVE_INFINITY;
        float max = Float.NEGATIVE_INFINITY;
        float average = 0;
        for (Entry entry : subResults.values()) {
            for (int i = 1; i < entry.data.size(); i += 2) {
                if (entry.data.get(i) < min) {
                    min = entry.data.get(i);
                }
                if (entry.data.get(i) > max) {
                    max = entry.data.get(i);
                }
            }
            average += entry.average;
        }
        average /= subResults.size();
        return "Average: " + average + ", Min: " + min + ", Max: " + max;
    }

    @Override
    public String toString() {
        return "TestResult [result=" + result + ", subResults=" + subResults
                + ", x=" + x + ", y=" + y + ", getSummary()=" + getSummary()
                + "]";
    }
}
