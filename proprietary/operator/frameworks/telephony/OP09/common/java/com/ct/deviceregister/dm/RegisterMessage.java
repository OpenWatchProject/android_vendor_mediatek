/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.ct.deviceregister.dm;

import android.os.Build;
import android.util.Log;

import com.ct.deviceregister.dm.utils.PlatformManager;

import java.util.zip.CRC32;

public class RegisterMessage {
    private static final String TAG = Const.TAG_PREFIX + "RegisterMessage";

    // Value specified by CT
    private static final byte PROTOCOL_VERSION_ESN = 0x01;
    private static final byte PROTOCOL_VERSION_MEID = 0x02;

    private static final byte COMMAND_TYPE_SEND = 0x03;
    public static final byte COMMAND_TYPE_RECEIVED = 0x04;

    private static final int LENGTH_MAX_MESSAGE = 127;
    private static final int LENGTH_MAX_MANUFACTURE = 3;
    private static final int LENGTH_MAX_MODEL = 20;
    private static final int LENGTH_MAX_VERSION = 60;
    private static final int LENGTH_CHECKSUM = 8;

    private static final String TAG_MSG_START = "<a1>";
    private static final String TAG_MSG_END = "</a1>";

    private static byte FILL_BYTE = 0x0;

    private Op09DeviceRegisterExt mService;

    public RegisterMessage() {
    }

    public RegisterMessage(Op09DeviceRegisterExt service) {
        mService = service;
    }

    protected byte[] getRegisterMessage() {
        String data = generateMessageData();
        int byteArrayLength = 4 + data.length();

        byte[] message = new byte[byteArrayLength];

        // 1. message prefix
        message[0] = getProtocolVersion();
        message[1] = COMMAND_TYPE_SEND;
        message[2] = (byte) data.length();
        message[3] = FILL_BYTE;

        // 2. message content
        byte[] dataByte = data.getBytes();
        int i = 4;
        for (int j = 0; j < dataByte.length; j++) {
            message[i] = dataByte[j];
            i++;
        }

        // 3. message check sum
        String checksum = generateChecksum(message);
        byte[] crcByte = checksum.getBytes();

        byte[] messageFinal = new byte[message.length + crcByte.length];

        int k = 0;
        for (int j = 0; j < message.length; j++) {
            messageFinal[k] = message[j];
            k++;
        }

        for (int j = 0; j < crcByte.length; j++) {
            messageFinal[k] = crcByte[j];
            k++;
        }
        return messageFinal;
    }

    private String generateMessageData() {
        StringBuilder data = new StringBuilder();
        data.append(TAG_MSG_START);

        // construct body
        String[] startTags = getStartTags();
        String[] endTags = getEndTags();
        String[] contents = getContent();
        for (int i = 0; i < startTags.length; ++i) {
            data.append(startTags[i]);
            data.append(contents[i]);
            data.append(endTags[i]);
        }

        data.append(TAG_MSG_END);

        return trimIfNeed(data);
    }

    public String encryptMeidImsi() {
        String message = String.format("<a1><b1>%s</b1><b2>%s</b2><b3>%s</b3><b4>%s</b4></a1>",
                getModel(), PlatformManager.encryptMessage(getMeid()),
                PlatformManager.encryptMessage(getCDMAIMSI()), getSoftwareVersion());
        return trimIfNeed(new StringBuilder(message));
    }

    protected String trimIfNeed(StringBuilder data) {
        // Message has a length limit
        if (data.length() > getMaxLength()) {
            Log.w(TAG, "Message length > " + getMaxLength() + ", cut it!");
            int exceedLength = data.length() - getMaxLength();
            int endTagLength = getLengthOfEndTag();
            data = data.delete(data.length() - endTagLength - exceedLength,
                    data.length() - endTagLength);
        }
        return data.toString();
    }

    protected String[] getStartTags() {
        return new String[]{"<b1>", "<b2>", "<b3>", "<b4>"};
    }

    protected String[] getEndTags() {
        return new String[]{"</b1>", "</b2>", "</b3>", "</b4>"};
    }

    protected int getLengthOfEndTag() {
        // length of </b1></a1>
        return 10;
    }

    protected String[] getContent() {
        return new String[]{getModel(), getMeid(), getCDMAIMSI(), getSoftwareVersion()};
    }

    protected String getModel() {
        String manufacturer = PlatformManager.getManufacturer();
        if (manufacturer.length() > LENGTH_MAX_MANUFACTURE) {
            Log.w(TAG, "Manufacturer length > " + LENGTH_MAX_MANUFACTURE + ", cut it!");
            manufacturer = manufacturer.substring(0, LENGTH_MAX_MANUFACTURE);
        }

        String model = Build.MODEL;
        model = model.replaceAll("-", " ");
        if (model.indexOf(manufacturer) != -1) {
            model = model.replaceFirst(manufacturer, "");
        }

        String result = manufacturer + "-" + model;
        if (result.length() > LENGTH_MAX_MODEL) {
            Log.w(TAG, "Model length > " + LENGTH_MAX_MODEL + ", cut it!");
            result = result.substring(0, LENGTH_MAX_MODEL);
        }

        return result;
    }

    private String getMeid() {
        String result = "";
        String meid = mService.getDeviceMeid();
        String imeiSlot0 = mService.getImei(0);
        Log.i(TAG,"[getMeid] meid/imei0: " + PlatformManager.encryptMessage(meid) + "/"
                + PlatformManager.encryptMessage(imeiSlot0));
        if (imeiSlot0.startsWith(meid)) {
            result = imeiSlot0;
        } else {
            result = meid;
        }
        return result;
    }

    private String getCDMAIMSI() {
        return mService.getCurrentCDMAImsi();
    }

    protected String getSoftwareVersion() {
        String result = PlatformManager.getSoftwareVersion();
        if (result.length() > LENGTH_MAX_VERSION) {
            Log.w(TAG, "Software version length > " + LENGTH_MAX_VERSION + ", cut it!");
            result = result.substring(0, LENGTH_MAX_VERSION);
        }

        return result;
    }

    protected int getMaxLength() {
        return LENGTH_MAX_MESSAGE;
    }

    protected byte getProtocolVersion() {
        return PROTOCOL_VERSION_MEID;
    }

    protected String generateChecksum(byte[] data) {
        CRC32 checksum = new CRC32();
        checksum.update(data);
        long value = checksum.getValue();

        String crcString = Long.toHexString(value);
        int crcStringLength = crcString.length();
        if (crcStringLength < LENGTH_CHECKSUM) {
            String prefix = "";
            for (int i = crcStringLength; i < LENGTH_CHECKSUM; i++) {
                prefix += "0";
            }
            crcString = prefix + crcString;
        }
        return crcString;
    }

}
