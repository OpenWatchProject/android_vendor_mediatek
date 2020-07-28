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

package com.mediatek.engineermode.wifi;

/**
 * EM Wifi native interface.
 *
 */
public class EMWifi {

    static boolean sIsInitialed = false;

    private static final long IOCTL_CMD_ID_CTIA_ON          = hex2Long("FFFF1234");
    private static final long IOCTL_CMD_ID_CTIA_OFF         = hex2Long("FFFF1235");
    private static final long IOCTL_CMD_ID_CTIA_SET_RATE    = hex2Long("FFFF0123");

    /**
     * Initialize EM wifi.
     *
     * @return Chip ID, like 0x6620 or 0x5921
     */
    public static native int initial();

    /**
     * Un-initialize EM wifi after test complete.
     *
     * @return ERROR_RFTEST_XXXX
     */
    public static native int unInitial();


    /**
     * Set wifi chip to test mode.
     *
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setTestMode();

    /**
     * Set the driver to normal mode.
     *
     * @return 0, if successful, -1, otherwise
     */
    public static native int setNormalMode();

    /**
     * stop BB continuous Tx mode.
     *
     * @return 0, if successful -1, Failed to set stop pattern
     */
    public static native int setStandBy();

    /**
     * Set EEPROM size.
     *
     * @param i4EepromSz
     *            Size of the EEPROM
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setEEPRomSize(long i4EepromSz);


    /**
     * Retrieve TX Power from EEPROM.
     *
     * @param i4ChnFreg
     *            Frequency
     * @param i4Rate
     *            Rate in unit of 500K
     * @param powerStatus
     *            Power status array
     * @param arraylen
     *            Power status array length
     * @return ERROR_RFTEST_XXXX
     */
    public static native int readTxPowerFromEEPromEx(long i4ChnFreg,
            long i4Rate, long[] powerStatus, int arraylen);

    /**
     * Update EEPROM checksum.
     *
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setEEPromCKSUpdated();


    /**
     * Query Rx status.
     *
     * @param i4Init
     *            Result array
     * @param arraylen
     *            Array length
     * @return ERROR_RFTEST_XXXX
     */
    public static native int getPacketRxStatus(long[] i4Init, int arraylen);


    /**
     * Set RF channel.
     *
     * @param i4ChFreqkHz
     *            Channel frequence
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setChannel(long i4ChFreqkHz);


    /**
     * Read 16-bit data to EEPRom.
     *
     * @param u4Offset
     *            Index of the EEPRom offset
     * @param pu4Value
     *            Value read from the EEPRom offset (size: 2 Byte)
     * @return ERROR_RFTEST_XXXX
     */
    public static native int readEEPRom16(long u4Offset, long[] pu4Value);


    /**
     * Read 16-bit data from EEPROM.
     *
     * @param u4Offset
     *            Index of the EEPROM offset
     * @param u4Value
     *            Value read from EEPROM
     * @return ERROR_RFTEST_XXXX
     */
    public static native int writeEEPRom16(long u4Offset, long u4Value);

    /**
     * Read string data from EEPROM.
     *
     * @param u4Addr
     *            Index of the EEPROM offset
     * @param u4Length
     *            Data length
     * @param paucStr
     *            String get from the EEPROM offset
     * @return ERROR_RFTEST_XXXX
     */
    public static native int eepromReadByteStr(long u4Addr, long u4Length,
            byte[] paucStr);

    /**
     * Write string data to EEPROM.
     *
     * @param u4Addr
     *            index of the EEPROM offset
     * @param u4Length
     *            Data length
     * @param paucStr
     *            Value to set
     * @return ERROR_RFTEST_XXXX
     */
    public static native int eepromWriteByteStr(long u4Addr, long u4Length,
            String paucStr);

    /**
     * Set AT parameter.
     *
     * @param u4FuncIndex
     *            Function index
     * @param u4FuncData
     *            Function data to set
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setATParam(long u4FuncIndex, long u4FuncData);

    /**
     * Get AT parameter.
     *
     * @param u4FuncIndex
     *            Function index
     * @param pu4FuncData
     *            Function data
     * @return ERROR_RFTEST_XXXX
     */
    public static native int getATParam(long u4FuncIndex, long[] pu4FuncData);


    /**
     * Set PNP power.
     *
     * @param i4PowerMode
     *            Power mode
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setPnpPower(long i4PowerMode);


    /**
     * Write 32-bit data to MCR.
     *
     * @param offset
     *            Address offset of the MCR
     * @param value
     *            Value set to the MCR
     * @return ERROR_RFTEST_XXXX
     */
    public static native int writeMCR32(long offset, long value);

    /**
     * Read 32-bit data from MCR.
     *
     * @param offset
     *            Address offset of the MCR
     * @param value
     *            Value read from the MCR
     * @return RROR_RFTEST_XXXX
     */
    public static native int readMCR32(long offset, long[] value);

    /**
     * Get support channel list. Added by mtk54046 @ 2012-01-05 for get support
     * channel list.
     *
     * @param value
     *            Channel buffer
     * @return RROR_RFTEST_XXXX
     */
    public static native int getSupportChannelList(long[] value);

    /**
     * CTIA test setting. Added by mtk54046 @ 2012-11-15 for CTIA test.
     *
     * @param id
     *            Address
     * @param value
     *            Value to set
     * @return RROR_RFTEST_XXXX
     */
    public static native int doCTIATestSet(long id, long value);

    /**
     * CTIA test getting. Added by mtk54046 @ 2012-11-15 for CTIA test.
     *
     * @param id
     *            Address
     * @param value
     *            Buffer to get
     * @return RROR_RFTEST_XXXX
     */
    public static native int doCTIATestGet(long id, long[] value);

    /**
     * CTIA test on.
     *
     * @return Boolean
     */
    public static boolean doCtiaTestOn() {
        return doCTIATestSet(IOCTL_CMD_ID_CTIA_ON, 0) > 0;
    }

    /**
     * CTIA test off.
     *
     * @return Boolean
     */
    public static boolean doCtiaTestOff() {
        return doCTIATestSet(IOCTL_CMD_ID_CTIA_OFF, 0) > 0;
    }

    /**
     * CTIA test set rate.
     *
     * @param rate
     *            Rate for CTIA
     * @return Boolean
     */
    public static boolean doCtiaTestRate(int rate) {
        return doCTIATestSet(IOCTL_CMD_ID_CTIA_SET_RATE, rate) > 0;
    }

    private static long hex2Long(String str) {
        return Long.parseLong(str, 16);
    }

    /**
     * Query wifi log level UI enable.
     *
     * @param module
     *     0: Driver
     *     1: FW
     *
     * @return
     *     true: enable
     *     false: otherwise
     */
    public static native boolean isWifiLogUiEnable(int module);

    /**
     * Query wifi log level.
     *
     * @param module
     *     0: Driver
     *     1: FW
     *
     * @return
     *     0: Off
     *     1: Default
     *     2: Extreme
     */
    public static native int getWifiLogLevel(int module);

    /**
     * Set wifi log level.
     *
     * @param module
     *     0: Driver
     *     1: FW
     *
     * @param level
     *     0: Off
     *     1: Default
     *     2: Extreme
     *
     * @return true for settings successful
     */
    public static native boolean setWifiLogLevel(int module, int level);

    /**
     * Query wifi manifest version.
     *
     * @return manifest version string. Null or empty string for cmd not supported
     */
    public static native String getFwManifestVersion();

    /**
     * Query if Ant swap feature supported.
     * @return true if supported
     */
    public static native boolean isAntSwapSupport();

    static {
        System.loadLibrary("em_wifi_jni");
    }

}
