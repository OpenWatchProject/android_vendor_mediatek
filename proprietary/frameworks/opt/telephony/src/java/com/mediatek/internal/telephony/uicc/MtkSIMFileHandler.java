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

import android.telephony.Rlog;

import android.os.Message;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.uicc.SIMFileHandler;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.mediatek.internal.telephony.uicc.MtkIccFileHandler;
import com.mediatek.internal.telephony.uicc.MtkIccConstants;
import android.telephony.Rlog;

/**
 * {@hide}
 */
public final class MtkSIMFileHandler extends SIMFileHandler {
    static final String LOG_TAG_EX = "MtkSIMFH";
    MtkIccFileHandler mMtkIccFh = null;
    //***** Instance Variables

    //***** Constructor

    public MtkSIMFileHandler(UiccCardApplication app, String aid, CommandsInterface ci) {
        super(app, aid, ci);
        mMtkIccFh = new MtkIccFileHandler(app, aid, ci);
    }

    @Override
    protected String getEFPath(int efid) {
        switch(efid) {
            case MtkIccConstants.EF_SMSP:   // [ALPS01206315] Support EF_SMSP
                return MF_SIM + DF_TELECOM;
            case MtkIccConstants.EF_ECC:
            case EF_OPL:
            case EF_FPLMN:
                return MF_SIM + DF_GSM;
            case MtkIccConstants.EF_RAT: // ALPS00302702 RAT balancing (ADF(USIM)/7F66/5F30/EF_RAT)
                return DF_ADF + "7F66" + "5F30";
            case EF_CSIM_IMSIM:
                return MF_SIM + DF_CDMA;
            default:
                Rlog.d(LOG_TAG_EX, "SIM aosp default getEFPath.");
                return super.getEFPath(efid);
        }
    }
    @Override
    public void loadEFLinearFixedAll(int fileid, Message onLoaded, boolean is7FFF) {
        mMtkIccFh.loadEFLinearFixedAllByPath(fileid, onLoaded, is7FFF);
    }
    @Override
    public void loadEFLinearFixedAll(int fileid, int mode , Message onLoaded) {
        mMtkIccFh.loadEFLinearFixedAllByMode(fileid, mode, onLoaded);
    }
    @Override
    public void loadEFTransparent(int fileid, String path, Message onLoaded) {
        mMtkIccFh.loadEFTransparent(fileid, path, onLoaded);
    }
    @Override
    public void updateEFTransparent(int fileid, String path, byte[] data, Message onComplete) {
        mMtkIccFh.updateEFTransparent(fileid, path, data, onComplete);
    }
    // PHB START
    @Override
    public void readEFLinearFixed(int fileid, int recordNum, int recordSize, Message onLoaded) {
        mMtkIccFh.readEFLinearFixed(fileid, recordNum, recordSize, onLoaded);
    }
    @Override
    public void selectEFFile(int fileid, Message onLoaded) {
        mMtkIccFh.selectEFFile(fileid, onLoaded);
    }
    // PHB END
}
