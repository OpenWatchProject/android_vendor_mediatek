/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
package com.mediatek.internal.telephony.devreg;

import android.app.PendingIntent;
import android.content.Context;

import com.android.internal.telephony.Phone;
import com.mediatek.internal.telephony.MtkUiccSmsController;
import com.mediatek.internal.telephony.OpTelephonyCustomizationFactoryBase;
import com.mediatek.internal.telephony.OpTelephonyCustomizationUtils;

/**
 * Controller for device register.
 */
public class DeviceRegisterController {

    private static IDeviceRegisterExt sDeviceRegisterExt = null;
    private MtkUiccSmsController mSmsController = null;
    private DeviceRegisterHandler[] mHandler = null;

    /**
     * Constructor for DeviceRegisterController.
     *
     * @param context the context of phone app
     * @param phone the array of phone
     * @param controller the sms controller
     */
    public DeviceRegisterController(Context context,
            Phone[] phone, MtkUiccSmsController controller) {
        mSmsController = controller;
        try {
            OpTelephonyCustomizationFactoryBase factoryBase =
                    OpTelephonyCustomizationUtils.getOpFactory(context);
            sDeviceRegisterExt = factoryBase.makeDeviceRegisterExt(context, this);
        } catch (Exception e) {
            e.printStackTrace();
            sDeviceRegisterExt = new DefaultDeviceRegisterExt(context, this);
        }
        mHandler = new DeviceRegisterHandler[phone.length];
        for (int i = 0; i < phone.length; i++) {
            mHandler[i] = new DeviceRegisterHandler(phone[i], this);
        }
    }

    /**
     * Send a data message with original port without permission check.
     *
     * @param subId subscription identity
     * @param destAddr the destination address
     * @param scAddr the SMSC to send the message through, or NULL for the
     *  default SMSC
     * @param destPort destination port
     * @param originalPort origianl sender port
     * @param data the body of the message to send
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is sucessfully sent, or failed.
     *  The result code will be <code>Activity.RESULT_OK</code> for success,
     *  or one of these errors:<br>
     *  <code>RESULT_ERROR_GENERIC_FAILURE</code><br>
     *  <code>RESULT_ERROR_RADIO_OFF</code><br>
     *  <code>RESULT_ERROR_NULL_PDU</code><br>
     *  For <code>RESULT_ERROR_GENERIC_FAILURE</code> the sentIntent may include
     *  the extra "errorCode" containing a radio technology specific value,
     *  generally only useful for troubleshooting.<br>
     *  The per-application based SMS control checks sentIntent. If sentIntent
     *  is NULL the caller will be checked against all unknown applicaitons,
     *  which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     */
    public void sendDataSms(int subId,
            String destAddr, String scAddr, int destPort, int originalPort, byte[] data,
            PendingIntent sentIntent, PendingIntent deliveryIntent) {
        mSmsController.sendData(
                subId,
                destAddr,
                scAddr,
                destPort,
                originalPort,
                data,
                sentIntent,
                deliveryIntent);
    }

    private static IDeviceRegisterExt getDeviceRegisterExt() {
        return sDeviceRegisterExt;
    }

    /**
     * Set esnMeid for device register.
     *
     * @param esnMeid esn or meid
     */
    public void setCdmaCardEsnOrMeid(String esnMeid) {
        if (sDeviceRegisterExt != null) {
            sDeviceRegisterExt.setCdmaCardEsnOrMeid(esnMeid);
        }
    }

    /**
     * Handle the auto register message reply (for CDMA)
     *
     * @param pdu the SMS PDU
     */
    public void handleAutoRegMessage(byte[] pdu) {
        if (sDeviceRegisterExt != null) {
            sDeviceRegisterExt.handleAutoRegMessage(pdu);
        }
    }


    /**
     * Handle the auto register message reply (for IMS)
     *
     * @param subId the sub Id of related SIM
     * @param format the format of message, like SmsMessage.FORMAT_3GPP2
     * @param pdu ths SMS PDU
     */
    public void handleAutoRegMessage(int subId, String format, byte[] pdu) {
        if (sDeviceRegisterExt != null) {
            sDeviceRegisterExt.handleAutoRegMessage(subId, format, pdu);
        }
    }
}
