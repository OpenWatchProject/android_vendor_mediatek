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

package com.ct.selfregister.dm;

import android.os.Build;
import android.util.Log;

import com.ct.selfregister.dm.utils.PlatformManager;
import com.ct.selfregister.dm.utils.Utils;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * Responsible for compose the register message.
 */
public class RegisterMessage {
    private static final String TAG = Const.TAG_PREFIX + "RegisterMessage";

    /// M: Fields of register message.
    private static final String FIELD_REG_VERSION = "REGVER";
    private static final String FIELD_MEID = "MEID";
    private static final String FIELD_MODEL = "MODEL";
    private static final String FIELD_SW_VERSION = "SWVER";
    private static final String FIELD_UE_TYPE = "UETYPE";
    private static final String FIELD_MACID = "MACID";

    private static final String FIELD_OS_VERSION = "OSVER";
    private static final String FIELD_ROM = "ROM";
    private static final String FIELD_RAM = "RAM";
    private static final String FIELD_IMEI_1 = "IMEI1";
    private static final String FIELD_IMEI_2 = "IMEI2";

    private static final String FIELD_SIM1_CDMA_IMSI = "SIM1CDMAIMSI";
    private static final String FIELD_SIM1_ICCID = "SIM1ICCID";
    private static final String FIELD_SIM1_LTE_IMSI = "SIM1LTEIMSI";

    private static final String FIELD_SIM2_CDMA_IMSI = "SIM2CDMAIMSI";
    private static final String FIELD_SIM2_LTE_IMSI = "SIM2LTEIMSI";
    private static final String FIELD_SIM2_ICCID = "SIM2ICCID";

    private static final String FIELD_DATA_SIM = "DATASIM";

    private static final String FIELD_SIM1_CELLID = "SIM1CELLID";
    private static final String FIELD_SIM2_CELLID = "SIM2CELLID";

    private static final String FIELD_REG_DATE = "REGDATE";

    // specified values
    private static final String OPERATING_SYSTEM = "android";

    private static final int LENGTH_MAX_MANUFACTURE = 3;
    private static final int LENGTH_MAX_MODEL = 20;
    private static final int LENGTH_MAX_VERSION = 60;
    private static final int VALUE_UE_TYPE = 1;
    private static final String VALUE_REG_VERSION = "8.0";

    private Op09SelfRegister mRegisterService;
    private PlatformManager mPlatformManager;
    private String mMessage;

    /**
     * Constructor method.
     * @param service The service which invokes this class.
     */
    public RegisterMessage(Op09SelfRegister service) {
        mRegisterService = service;
        mPlatformManager = mRegisterService.getPlatformManager();

        mMessage = generateMessageData();
        Log.d(TAG, "Generate data (hide sensitive): " + generateEncryptMessage(mMessage));
    }

    public String getRealMessage() {
        return mMessage;
    }

    public String getBase64Message() {
        return Utils.encodeBase64(mMessage);
    }

    private String generateMessageData() {
        JSONObject data = new JSONObject();
        try {
            appendVersionInfo(data);
            data.put(FIELD_MEID, getMeid());
            data.put(FIELD_MODEL, getModel());
            data.put(FIELD_SW_VERSION, getSoftwareVersion());
            data.put(FIELD_UE_TYPE, VALUE_UE_TYPE);
            data.put(FIELD_MACID, getMacId());

            data.put(FIELD_OS_VERSION, getOSVersion());
            data.put(FIELD_ROM, getRom());
            data.put(FIELD_RAM, getRam());

            appendImeiInfo(data);
            appendSimInfo(data);
            appendLocationInfo(data);
            appendSpecialInfo(data);

        } catch (JSONException e) {
            Log.e(TAG, "JSONException " + e);
            e.printStackTrace();
        }

        return data.toString();
    }

    private String generateEncryptMessage(String message) {
        try {
            JSONObject copyData = new JSONObject(message);
            String[] array = {FIELD_MEID, FIELD_MACID, FIELD_IMEI_1, FIELD_IMEI_2,
                    FIELD_SIM1_CDMA_IMSI, FIELD_SIM1_ICCID, FIELD_SIM1_LTE_IMSI,
                    FIELD_SIM2_CDMA_IMSI, FIELD_SIM2_LTE_IMSI, FIELD_SIM2_ICCID,
                    FIELD_SIM1_CELLID, FIELD_SIM2_CELLID};
            for (String key: array) {
                copyData.put(key, encryptMessage((String) copyData.get(key)));
            }
            return copyData.toString();
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return "";
    }


    private void appendVersionInfo(JSONObject data) throws JSONException {
        data.put(FIELD_REG_VERSION, VALUE_REG_VERSION);
    }

    private void appendImeiInfo(JSONObject data) throws JSONException {
        data.put(FIELD_IMEI_1, getImei(Const.SLOT_ID_0));

        if (mPlatformManager.isSingleLoad()) {
            data.put(FIELD_IMEI_2, Const.VALUE_EMPTY);
        } else {
            data.put(FIELD_IMEI_2, getImei(Const.SLOT_ID_1));
        }
    }

    private void appendSimInfo(JSONObject data) throws JSONException {
        appendFirstSlotInfo(data);
        appendSecondSlotInfo(data);
    }

    private void appendFirstSlotInfo(JSONObject data) throws JSONException {
        int slot = Const.SLOT_ID_0;
        if (mPlatformManager.hasIccCard(slot)) {
            data.put(FIELD_SIM1_CDMA_IMSI, getCdmaImsi(slot));
            data.put(FIELD_SIM1_ICCID, getIccId(slot));
            data.put(FIELD_SIM1_LTE_IMSI, getLteImsi(slot));
        } else {
            data.put(FIELD_SIM1_CDMA_IMSI, Const.VALUE_EMPTY);
            data.put(FIELD_SIM1_ICCID, Const.VALUE_EMPTY);
            data.put(FIELD_SIM1_LTE_IMSI, Const.VALUE_EMPTY);
        }
    }

    private void appendSecondSlotInfo(JSONObject data) throws JSONException {
        int slot = Const.SLOT_ID_1;
        if (!mPlatformManager.isSingleLoad()
                && mPlatformManager.hasIccCard(slot)) {
            data.put(FIELD_SIM2_CDMA_IMSI, getCdmaImsi(slot));
            data.put(FIELD_SIM2_LTE_IMSI, getLteImsi(slot));
            data.put(FIELD_SIM2_ICCID, getIccId(slot));
        } else {
            data.put(FIELD_SIM2_CDMA_IMSI, Const.VALUE_EMPTY);
            data.put(FIELD_SIM2_LTE_IMSI, Const.VALUE_EMPTY);
            data.put(FIELD_SIM2_ICCID, Const.VALUE_EMPTY);
        }
    }

    private void appendLocationInfo(JSONObject data) throws JSONException {
        appendCellIdInfo(data);
    }

    private void appendCellIdInfo(JSONObject data) throws JSONException {
        int slot = Const.SLOT_ID_0;
        if (mPlatformManager.hasIccCard(slot)) {
            data.put(FIELD_SIM1_CELLID, getCellId(slot));
        } else {
            data.put(FIELD_SIM1_CELLID, Const.VALUE_EMPTY);
        }

        slot = Const.SLOT_ID_1;
        if (!mPlatformManager.isSingleLoad()
                && mPlatformManager.hasIccCard(slot)) {
            data.put(FIELD_SIM2_CELLID, getCellId(slot));
        } else {
            data.put(FIELD_SIM2_CELLID, Const.VALUE_EMPTY);
        }
    }

    private void appendSpecialInfo(JSONObject data) throws JSONException {
        data.put(FIELD_DATA_SIM, getDataSim());
        data.put(FIELD_REG_DATE, getRegDate());
    }

    private String getRegDate() {
        return PlatformManager.getDate();
    }

    private String getDataSim() {
        // Map slot to {1, 2}
        return "" + (mRegisterService.getDataSim() + 1);
    }

    private String getIccId(int slotId) {
        return mRegisterService.getIccIdFromCard(slotId);
    }

    private String getCdmaImsi(int slotId) {
        return mRegisterService.getComplexImsi(slotId)[0];
    }

    private String getLteImsi(int slotId) {
        return mRegisterService.getComplexImsi(slotId)[1];
    }

    private String getCellId(int slot) {
        int cellId = mRegisterService.getCellId(slot);
        if (cellId <= 0) {
            return Const.VALUE_EMPTY;
        } else {
            return "" + cellId;
        }
    }

    /**
     * For device not support CDMA, there's no MEID info.
     * In this scenario, use IMEI for slot 0
     * @return MEID, or IMEI for slot 0 (no CDMA)
     */
    private String getMeid() {
        String meid = mRegisterService.getMeid();

        // Not support CDMA, or not burn this info
        if (mPlatformManager.isInvalidMeid(meid)) {
            String imei = mPlatformManager.getImei(Const.SLOT_ID_0);

            // IMEI for slot 0 must be valid
            if (!mPlatformManager.isInvalidImei(imei)) {
                return imei;
            }
        }
        return meid;
    }

    private String getRom() {
        return PlatformManager.getRom();
    }

    private String getRam() {
        return mPlatformManager.getRam();
    }

    private String getImei(int slotId) {
        return mRegisterService.getImei(slotId);
    }

    private String getOSVersion() {
        return OPERATING_SYSTEM + Build.VERSION.RELEASE;
    }

    private String getMacId() {
        return mRegisterService.getMacId();
    }

    private String getModel() {
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

    private String getSoftwareVersion() {
        String result = PlatformManager.getSoftwareVersion();
        if (result.length() > LENGTH_MAX_VERSION) {
            Log.w(TAG, "Software version length > " + LENGTH_MAX_VERSION + ", cut it!");
            result = result.substring(0, LENGTH_MAX_VERSION);
        }

        return result;
    }

    private String encryptMessage(String message) {
        return PlatformManager.encryptMessage(message);
    }
}
