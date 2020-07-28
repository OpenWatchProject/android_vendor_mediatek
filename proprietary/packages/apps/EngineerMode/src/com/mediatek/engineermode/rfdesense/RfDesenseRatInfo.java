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
 * BY OPENING THIS Rat, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
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

package com.mediatek.engineermode.rfdesense;

public class RfDesenseRatInfo {
    public final int DEFAULT_BAND_WIDTH = 3;
    public final int DEFAULT_VRB_LENGTH = 1;
    private String RatName;
    private String RatCmdStart;
    private String RatCmdStop;
    private String Ratband;
    private String RatPowerSet;
    private String RatCmdSwitch;
    private String RatCmdPowerRead;
    private String RatCmdAntSwitch;
    private Boolean RatCheckState;
    private Boolean RatSendState;

    private int RatCmdLteRb;
    private int RatCmdLteBw;
    private int RatTxtimes;

    public String getRatName() {
        return RatName;
    }

    public void setRatName(String mRatname) {
        if (mRatname != null) {
            RatName = mRatname;
        }
    }

    public Boolean getRatCheckState() {
        return RatCheckState;
    }

    public void setRatCheckState(Boolean mRatCheckState) {
        RatCheckState = mRatCheckState;
    }

    public Boolean getRatSendState() {
        return RatSendState;
    }

    public void setRatSendState(Boolean mRatSendState) {
        RatSendState = mRatSendState;
    }

    public String getRatCmdStart() {
        return RatCmdStart;
    }

    public String getRatCmdAntSwitch() {
        return RatCmdAntSwitch;
    }

    public void setRatCmdAntSwitch(String ratCmdAntSwitch) {
        RatCmdAntSwitch = ratCmdAntSwitch;
    }

    public void setRatCmdStart(String mRatCmdStart) {
        if (mRatCmdStart != null) {
            RatCmdStart = mRatCmdStart;
        }
    }

    public String getRatCmdStop() {
        return RatCmdStop;
    }

    public void setRatCmdStop(String mRatCmdStop) {
        if (mRatCmdStop != null) {
            RatCmdStop = mRatCmdStop;
        }
    }

    public String getRatCmdSwitch() {
        return RatCmdSwitch;
    }

    public void setRatCmdSwitch(String mRatCmdSwitch) {
        RatCmdSwitch = mRatCmdSwitch;
    }

    public String getRatCmdPowerRead() {
        return RatCmdPowerRead;
    }

    public void setRatCmdLteBwRb(int ratCmdLteBw, int ratCmdLteRb) {
        if (ratCmdLteBw == -1) {
            RatCmdLteBw = DEFAULT_BAND_WIDTH;
        } else {
            RatCmdLteBw = ratCmdLteBw;
        }
        if (ratCmdLteRb == -1) {
            RatCmdLteRb = DEFAULT_VRB_LENGTH;
        } else {
            RatCmdLteRb = ratCmdLteRb;
        }
    }

    public void setRatCmdStart(String rat, int channel, int power, int band) {
        String command = "";
        if (rat.equals(RfDesenseTxTest.mRatName[0])) {  //GSM
            command = "AT+ERFTX=2,1," + channel + "," + 4100 + "," + band + "," + 0 + ","
                    + power + "," + 0;
        } else if (rat.equals(RfDesenseTxTest.mRatName[1])) {    //TDSCDMA
            command = "AT+ERFTX=0,0," + band + "," + channel + "," + power;
        } else if (rat.equals(RfDesenseTxTest.mRatName[2])) {    //WCDMA
            command = "AT+ERFTX=0,0," + band + "," + channel + "," + power;
        } else if (rat.equals(RfDesenseTxTest.mRatName[3])) {    //LTE(FDD)
            command = "AT+ERFTX=6,0,2," + band + "," + RatCmdLteBw + "," + channel + ",1,0,0,0," +
                    RatCmdLteRb + "," + "0," + power;
        } else if (rat.equals(RfDesenseTxTest.mRatName[4])) {    //LTE(TDD)
            command = "AT+ERFTX=6,0,2," + band + "," + RatCmdLteBw + "," + channel + ",0,0,0,0," +
                    RatCmdLteRb + "," + "0," + power;
        } else if (rat.equals(RfDesenseTxTest.mRatName[5])) {    //CDMA(EVDO)
            command = "AT+ERFTX=13,4," + channel + "," + band + "," + power;
        } else if (rat.equals(RfDesenseTxTest.mRatName[6])) {    //CDMA(1x)
            command = "AT+ECRFTX=1," + channel + "," + band + "," + power + ",0";
        }
        RatCmdStart = command;
    }

    public void setRatPowerRead(String mRatCmdPowerRead) {
        RatCmdPowerRead = mRatCmdPowerRead;
    }

    public String getRatband() {
        return Ratband;
    }

    public void setRatband(String ratband) {
        Ratband = ratband;
    }

    public String getRatPowerSet() {
        return RatPowerSet;
    }

    public void setRatPowerSet(String ratPowerSet) {
        RatPowerSet = ratPowerSet;
    }

    public int getRatTxtimes() {
        return RatTxtimes;
    }

    public void setRatTxtimes(int ratTxtimes) {
        RatTxtimes = ratTxtimes;
    }
}
