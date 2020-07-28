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

package com.mediatek.engineermode.mdmcomponent;

import android.app.Activity;

import com.mediatek.engineermode.Elog;

import java.nio.ByteBuffer;


class EUTRAMeasurementReport extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContentICD.MSG_TYPE_ICD_RECORD,
            MDMContentICD.MSG_ID_ERRC_SERVING_CELL_INFO,
            MDMContentICD.MSG_ID_ERRC_MEAS_REPORT_INFO
    };
    int wMCC = 0;
    int wMNC = 0;
    int ucFreqBandInd = 0;
    int ucDlBandWidth = 0;
    int ucUlBandWidth = 0;

    int wServEarfcn = 0;
    int wServPhysCellId = 0;
    int wServRsrp = 0;
    int wServRsrq = 0;
    int wServRssnr = 0;


    public EUTRAMeasurementReport(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "EUTRAMeasurementReport";
    }

    @Override
    String getGroup() {
        return "8.NR EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"MCC", "MNC", "ucFreqBandInd", "ucDlBandwidth", "ucUlBandwidth",
                "wServEarfcn",
                "wServPhysCellId",
                "ucServRsrp",
                "ucServRsrq", "ucServRssnr"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    void update(String name, Object msg) {
        ByteBuffer icdPacket = (ByteBuffer) msg;
        clearData();
        Elog.d(TAG, "EUTRAMeasurementReport update,name id = " + name);

        int version = getFieldValueIcdVersion(icdPacket);
        Elog.d(TAG, "version = " + version);

        if (name.equals(MDMContentICD.MSG_ID_ERRC_SERVING_CELL_INFO)) {
            wMCC = getFieldValueIcd(icdPacket, 56, 16);
            wMNC = getFieldValueIcd(icdPacket, 72, 16);
            ucFreqBandInd = getFieldValueIcd(icdPacket, 8, 16);
            ucDlBandWidth = getFieldValueIcd(icdPacket, 96, 8);
            ucUlBandWidth = getFieldValueIcd(icdPacket, 104, 8);
        } else if (name.equals(MDMContentICD.MSG_ID_ERRC_MEAS_REPORT_INFO)) {
            wServEarfcn = getFieldValueIcd(icdPacket, 24, 32);
            wServPhysCellId = getFieldValueIcd(icdPacket, 8, 16);
            wServRsrp = getFieldValueIcd(icdPacket, 56, 8);
            wServRsrq = getFieldValueIcd(icdPacket, 64, 8);
            wServRssnr = getFieldValueIcd(icdPacket, 72, 8);
        }
        addData(wMCC);
        addData(wMNC);
        addData(ucFreqBandInd);
        addData(ucDlBandWidth);
        addData(ucUlBandWidth);

        addData(wServEarfcn);
        addData(wServPhysCellId);
        addData(wServRsrp);
        addData(wServRsrq);
        addData(wServRssnr);
        // notifyDataSetChanged();
    }
}