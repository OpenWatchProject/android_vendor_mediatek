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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libwifitest.h"

void rx_test(void) {
    int i, rssi;
    uint32_t rxOk, rxErr;
    bool retval;
    float fRssi;

    printf("[RX] entering RF testing mode ..\n");

    retval = WIFI_TEST_OpenDUT();
    printf("(%d) entered RF testing mode ..\n", retval);

    retval = WIFI_TEST_SetMode(WIFI_TEST_MODE_80211N_ONLY);
    printf("(%d) changed mode to 802.11n ..\n", retval);

    retval = WIFI_TEST_SetFrequency(2412, 0);
    printf("(%d) changed channel to #1..\n", retval);

    retval = WIFI_TEST_SetBandwidthV2(WIFI_TEST_CH_BW_20MHZ);
    printf("(%d) changed channel bandwidth 20M..\n", retval);

    retval = WIFI_TEST_SetTxBandwidth(WIFI_TEST_CH_BW_20MHZ);
    printf("(%d) changed data bandwidth 20M..\n", retval);

    retval = WIFI_TEST_SetPriChannelSetting(WIFI_TEST_PRI_CH_SETTING_0);
    printf("(%d) changed primary channel offset to 0..\n", retval);

    retval = WIFI_TEST_RxStart();
    printf("(%d) RX test started..\n", retval);

    for(i = 0 ; i < 10 ; i++) {
        retval = WIFI_TEST_GetResult(&rxOk, &rxErr);
        printf("(%d) RX OK: %d / RX ERR: %d\n", retval, rxOk, rxErr);
        sleep(1);
        retval = WIFI_TEST_RSSI(&rssi);
        printf("(%d) RSSI: %d\n", retval, rssi);
        sleep(1);
        retval = WIFI_TEST_RSSI_05_DBM(&fRssi);
        printf("(%d) RSSI: %f\n", retval, fRssi);
        sleep(1);
    }

    retval = WIFI_TEST_RxStop();
    printf("(%d) RX test stopped ..\n", retval);

    retval = WIFI_TEST_CloseDUT();
    printf("(%d) left RF testing mode ..\n", retval);
}

void tx_test(void) {
    bool retval;
    int i;

    printf("[TX] entering RF testing mode ..\n");

    retval = WIFI_TEST_OpenDUT();
    printf("(%d) entered RF testing mode ..\n", retval);

    retval = WIFI_TEST_SetFrequency(2412, 0);
    printf("(%d) changed channel to #1..\n", retval);

    retval = WIFI_TEST_GI(WIFI_TEST_GI_TYPE_NORMAL_GI);
    printf("(%d) changed normal GI 800ns..\n", retval);

    retval = WIFI_TEST_SetBandwidthV2(WIFI_TEST_CH_BW_20MHZ);
    printf("(%d) changed channel bandwidth 20M..\n", retval);

    retval = WIFI_TEST_SetTxBandwidth(WIFI_TEST_CH_BW_20MHZ);
    printf("(%d) changed data bandwidth 20M..\n", retval);

    retval = WIFI_TEST_SetPriChannelSetting(WIFI_TEST_PRI_CH_SETTING_0);
    printf("(%d) changed primary channel offset to 0..\n", retval);

    /* RF_AT_FUNCID_TX_PWR_MODE; Power Mode */
    //@FIXME

    retval = WIFI_TEST_TxGain(16);
    printf("(%d) changed tw power to 16dBm..\n", retval);

    retval = WIFI_TEST_TxDataRate(5); //WIFI_TEST_MCS_RATE_5
    printf("(%d) changed data rate to HT_MCS5..\n", retval);

    retval = WIFI_TEST_SetPreamble(RF_AT_PREAMBLE_11N_MM);
    printf("(%d) changed preamble to 11n mm..\n", retval);

    retval = WIFI_TEST_TxPayloadLength(1023);
    printf("(%d) changed packet len to 1023..\n", retval);

    retval = WIFI_TEST_TxBurstFrames(0);
    printf("(%d) changed packet count to 0..\n", retval);

    retval = WIFI_TEST_SetTX(true);
    printf("(%d) TX test started..\n", retval);

    printf("Tx ing ...\n");
    for(i = 0 ; i < 10 ; i++) {
        sleep(1);
    }

    retval = WIFI_TEST_SetTX(false);
    printf("(%d) TX test stopped ..\n", retval);

    retval = WIFI_TEST_CloseDUT();
    printf("(%d) left RF testing mode ..\n", retval);
}

int main(void) {
    printf("=== RX Test===\n");
    rx_test();
    printf("=== TX Test===\n");
    tx_test();
    return 0;
}
