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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "Rtst.h"
#include <libmtkrilutils.h>
extern "C" {
    #include "sms.h"
}
/*****************************************************************************
 * Test Cases
 *****************************************************************************/
TEST(CdmaSmsTest, Sort) {
    int a[] = {10, 8, 5, 11, 2};
    int b[] = {2,  5, 8, 10, 11};
    sort(a, 5);
    for (int i = 0; i < 5; i++) {
        ASSERT_EQ(a[i], b[i]);
    }
}

TEST(CdmaSmsTest, getRange) {
    int b[] = {2,  5, 8, 10, 11};
    Range r[5];
    int n = getRange(b, 5, r);
    ASSERT_EQ(n, 4);
    ASSERT_EQ(r[0].start, 2);
    ASSERT_EQ(r[0].end, 2);

    ASSERT_EQ(r[1].start, 5);
    ASSERT_EQ(r[1].end, 5);

    ASSERT_EQ(r[2].start, 8);
    ASSERT_EQ(r[2].end, 8);

    ASSERT_EQ(r[3].start, 10);
    ASSERT_EQ(r[3].end, 11);
}

TEST(CdmaSmsTest, sorAndgetRange) {
    int b[] = {10, 9, 8, 6, 5};
    sort(b, 5);
    Range r[5];
    int n = getRange(b, 5, r);
    ASSERT_EQ(n, 2);
    ASSERT_EQ(r[0].start, 5);
    ASSERT_EQ(r[0].end, 6);

    ASSERT_EQ(r[1].start, 8);
    ASSERT_EQ(r[1].end, 10);
}

TEST(CdmaSmsTest, singleNumbersorAndgetRange) {
    int c[] = {5};
    sort(c, 1);
    Range r[5];
    int n = getRange(c, 1, r);
    ASSERT_EQ(n, 1);
    ASSERT_EQ(r[0].start, 5);
    ASSERT_EQ(r[0].end, 5);
}

TEST(CdmaSmsTest, sequenceNumbersorAndgetRange) {
    int d[] = {5, 6, 8, 10, 15, 16};
    sort(d, 6);
    Range r[5];
    int n = getRange(d, 6, r);
    ASSERT_EQ(n, 4);
    ASSERT_EQ(r[0].start, 5);
    ASSERT_EQ(r[0].end, 6);

    ASSERT_EQ(r[1].start, 8);
    ASSERT_EQ(r[1].end, 8);

    ASSERT_EQ(r[2].start, 10);
    ASSERT_EQ(r[2].end, 10);

    ASSERT_EQ(r[3].start, 15);
    ASSERT_EQ(r[3].end, 16);
}

TEST(CdmaSmsTest, split) {
    char src[] = "4096-4096,4098-4101";
    char dest[10][20];
    int num = 0;
    split(src, ",", dest, &num);
    ASSERT_EQ(0, strcmp(dest[0], "4096-4096"));
    ASSERT_EQ(0, strcmp(dest[1], "4098-4101"));
}

TEST(CdmaSmsTest, testTrimAndSkipQuote) {
    char xxx[] = " \"4096-4096\" ";
    ASSERT_EQ(0, strcmp(trim(xxx), "\"4096-4096\""));
    ASSERT_EQ(0, strcmp(skipQuote(trim(xxx)), "4096-4096"));
}

TEST(CdmaSmsTest, testSplitTrimAndSkipQuote) {
    char xxx[] = " \"4096-4096\" ";
    char dest2[2][20];
    int num = 0;
    split(skipQuote(trim(xxx)), "-", dest2, &num);
    ASSERT_EQ(num, 2);
    ASSERT_EQ(0, strcmp(dest2[0], "4096"));
    ASSERT_EQ(0, strcmp(dest2[1], "4096"));
}


TEST(CdmaSmsTest, testGetRangeFromModem) {
    Range range[10];
    int n;
    char yyy[]= " \"4096-4096\", \"4098-4101\" ";
    n = getRangeFromModem(yyy, range);
    ASSERT_EQ(n, 2);
    ASSERT_EQ(range[0].start, 4096);
    ASSERT_EQ(range[0].end, 4096);
    ASSERT_EQ(range[1].start, 4098);
    ASSERT_EQ(range[1].end, 4101);
}

TEST(CdmaSmsTest, RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION_1) {
    if (isSvlteSupport() == 1) {
        RTST_CASE_BEGIN();
        RTST_RIL_REQUEST(RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION, 1, RTST_INT32, "1");
        RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION,
                RIL_E_REQUEST_NOT_SUPPORTED);
        RTST_CASE_END();
    } else {
        RTST_CASE_BEGIN();
        RTST_RIL_REQUEST(RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION, 1, RTST_INT32, "0");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCB=1", 1, "OK");
        RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION, RIL_E_SUCCESS);
        RTST_CASE_END();
    }
}

TEST(CdmaSmsTest, RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION_2) {
    if (isSvlteSupport() == 1) {
        RTST_CASE_BEGIN();
        RTST_RIL_REQUEST(RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION, 1, RTST_INT32, "1");
        RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION,
                RIL_E_REQUEST_NOT_SUPPORTED);
        RTST_CASE_END();
    } else {

        RTST_CASE_BEGIN();
        RTST_RIL_REQUEST(RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION, 1, RTST_INT32, "1");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCB=0", 1, "OK");
        RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION, RIL_E_SUCCESS);
        RTST_CASE_END();
    }
}

TEST(CdmaSmsTest, RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG_1) {
    if (isSvlteSupport() == 1) {
        RTST_CASE_BEGIN();
        RTST_RIL_VOID_REQUEST(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG);
        RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG,
                RIL_E_REQUEST_NOT_SUPPORTED);
        RTST_CASE_END();
    } else {
        RTST_CASE_BEGIN();
        RTST_RIL_VOID_REQUEST(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG);
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCB?", 2, "+ECSCB:0", "OK");
        RTST_EXPECTED_RIL_RESPONSE_NC(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG, RIL_E_SUCCESS, 4,
             RTST_INT32, "1",
             RTST_INT32, "0",
             RTST_INT32, "0",
             RTST_INT32, "0"
             );
        RTST_CASE_END();
    }
}

TEST(CdmaSmsTest, RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG_2) {
    if (isSvlteSupport() == 1) {
        RTST_CASE_BEGIN();
        RTST_RIL_VOID_REQUEST(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG);
        RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG,
                RIL_E_REQUEST_NOT_SUPPORTED);
        RTST_CASE_END();
    } else {

        RTST_CASE_BEGIN();
        RTST_RIL_VOID_REQUEST(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG);
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCB?", 2, "+ECSCB:1", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBCHA?", 2, "+ECSCBCHA:0", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBLAN?", 2, "+ECSCBLAN:0", "OK");
        RTST_EXPECTED_RIL_RESPONSE_NC(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG, RIL_E_SUCCESS, 4,
             RTST_INT32, "1",
             RTST_INT32, "0",
             RTST_INT32, "0",
             RTST_INT32, "0"
             );
        RTST_CASE_END();
    }
}

TEST(CdmaSmsTest, RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG_3) {
    if (isSvlteSupport() == 1) {
        RTST_CASE_BEGIN();
        RTST_RIL_VOID_REQUEST(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG);
        RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG,
                RIL_E_REQUEST_NOT_SUPPORTED);
        RTST_CASE_END();
    } else {

        RTST_CASE_BEGIN();
        RTST_RIL_VOID_REQUEST(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG);
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCB?", 2, "+ECSCB:1", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBCHA?", 2, "+ECSCBCHA:1,\"4096-4096,4098-4100\"", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBLAN?", 2, "+ECSCBLAN:1,\"1-1,3-3\"", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBCHA?", 2, "+ECSCBCHA:1,\"4096-4096,4098-4100\"", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBLAN?", 2, "+ECSCBLAN:1,\"1-1,3-3\"", "OK");

        RTST_EXPECTED_RIL_RESPONSE_NC(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG, RIL_E_SUCCESS, 19,
             RTST_INT32, "6",
             RTST_INT32, "4096",
             RTST_INT32, "0",
             RTST_INT32, "1",
             RTST_INT32, "4098",
             RTST_INT32, "0",
             RTST_INT32, "1",
             RTST_INT32, "4099",
             RTST_INT32, "0",
             RTST_INT32, "1",
             RTST_INT32, "4100",
             RTST_INT32, "0",
             RTST_INT32, "1",
             RTST_INT32, "0",
             RTST_INT32, "1",
             RTST_INT32, "1",
             RTST_INT32, "0",
             RTST_INT32, "3",
             RTST_INT32, "1"
             );
        RTST_CASE_END();
    }
}


TEST(CdmaSmsTest, RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG_4) {
    if (isSvlteSupport() == 1) {
        RTST_CASE_BEGIN();
        RTST_RIL_VOID_REQUEST(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG);
        RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG,
                RIL_E_REQUEST_NOT_SUPPORTED);
        RTST_CASE_END();
    } else {

        RTST_CASE_BEGIN();
        RTST_RIL_VOID_REQUEST(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG);
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCB?", 2, "+ECSCB:1", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBCHA?", 2, "+ECSCBCHA:0", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBLAN?", 2, "+ECSCBLAN:1,\"1-1,3-3\"", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBCHA?", 2, "+ECSCBCHA:0", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBLAN?", 2, "+ECSCBLAN:1,\"1-1,3-3\"", "OK");
        RTST_EXPECTED_RIL_RESPONSE_NC(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG, RIL_E_SUCCESS, 7,
             RTST_INT32, "2",
             RTST_INT32, "0",
             RTST_INT32, "1",
             RTST_INT32, "1",
             RTST_INT32, "0",
             RTST_INT32, "3",
             RTST_INT32, "1"
             );
        RTST_CASE_END();
    }
}


TEST(CdmaSmsTest, RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG_5) {
    if (isSvlteSupport() == 1) {
        RTST_CASE_BEGIN();
        RTST_RIL_VOID_REQUEST(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG);
        RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG,
                RIL_E_REQUEST_NOT_SUPPORTED);
        RTST_CASE_END();
    } else {

        RTST_CASE_BEGIN();
        RTST_RIL_VOID_REQUEST(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG);
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCB?", 2, "+ECSCB:1", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBCHA?", 2, "+ECSCBCHA:1,\"4096-4096,4098-4100\"", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBLAN?", 2, "+ECSCBLAN:0", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBCHA?", 2, "+ECSCBCHA:1,\"4096-4096,4098-4100\"", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBLAN?", 2, "+ECSCBLAN:0", "OK");
        RTST_EXPECTED_RIL_RESPONSE_NC(RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG, RIL_E_SUCCESS,
                13,
                RTST_INT32, "4",
                RTST_INT32, "4096",
                RTST_INT32, "0",
                RTST_INT32, "1",
                RTST_INT32, "4098",
                RTST_INT32, "0",
                RTST_INT32, "1",
                RTST_INT32, "4099",
                RTST_INT32, "0",
                RTST_INT32, "1",
                RTST_INT32, "4100",
                RTST_INT32, "0",
                RTST_INT32, "1");
        RTST_CASE_END();
    }
}


TEST(CdmaSmsTest, RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG_1) {
    if (isSvlteSupport() == 1) {
        RTST_CASE_BEGIN();
        RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, 10,
             RTST_INT32, "3",
             RTST_INT32, "4096",
             RTST_INT32, "1",
             RTST_INT32, "0",
             RTST_INT32, "4098",
             RTST_INT32, "1",
             RTST_INT32, "0",
             RTST_INT32, "4099",
             RTST_INT32, "1",
             RTST_INT32, "0"
             );
        RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG,
                RIL_E_REQUEST_NOT_SUPPORTED);
        RTST_CASE_END();
    } else {
        RTST_CASE_BEGIN();
        RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, 10,
             RTST_INT32, "3",
             RTST_INT32, "4096",
             RTST_INT32, "1",
             RTST_INT32, "0",
             RTST_INT32, "4098",
             RTST_INT32, "1",
             RTST_INT32, "0",
             RTST_INT32, "4099",
             RTST_INT32, "1",
             RTST_INT32, "0"
             );

        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBCHA=0,\"4096-4096\"", 1, "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBCHA=0,\"4098-4099\"", 1, "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBLAN=0,\"1-1\"", 1, "OK");
        RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, RIL_E_SUCCESS);
        RTST_CASE_END();
    }
}


TEST(CdmaSmsTest, RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG_2) {
    if (isSvlteSupport() == 1) {
        RTST_CASE_BEGIN();
        RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, 10,
             RTST_INT32, "3",
             RTST_INT32, "4096",
             RTST_INT32, "1",
             RTST_INT32, "1",
             RTST_INT32, "4098",
             RTST_INT32, "1",
             RTST_INT32, "1",
             RTST_INT32, "4099",
             RTST_INT32, "1",
             RTST_INT32, "1"
             );
        RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG,
                RIL_E_REQUEST_NOT_SUPPORTED);
        RTST_CASE_END();
    } else {

        RTST_CASE_BEGIN();
        RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, 10,
             RTST_INT32, "3",
             RTST_INT32, "4096",
             RTST_INT32, "1",
             RTST_INT32, "1",
             RTST_INT32, "4098",
             RTST_INT32, "1",
             RTST_INT32, "1",
             RTST_INT32, "4099",
             RTST_INT32, "1",
             RTST_INT32, "1"
             );

        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBCHA?", 2, "+ECSCBCHA:1,\"4096-4097\"", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBCHA=0,\"4096-4097\"", 1, "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBLAN?", 2, "+ECSCBLAN:1,\"1-1\"", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBLAN=0,\"1-1\"", 1, "OK");

        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBCHA=1,\"4096-4096\"", 1, "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBCHA=1,\"4098-4099\"", 1, "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBLAN=1,\"1-1\"", 1, "OK");
        RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, RIL_E_SUCCESS);
        RTST_CASE_END();
    }
}

TEST(CdmaSmsTest, RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG_3) {
    if (isSvlteSupport() == 1) {
        RTST_CASE_BEGIN();
        RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, 4,
             RTST_INT32, "1",
             RTST_INT32, "4096",
             RTST_INT32, "1",
             RTST_INT32, "1"
             );
        RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG,
                RIL_E_REQUEST_NOT_SUPPORTED);
        RTST_CASE_END();

    } else {
        RTST_CASE_BEGIN();
        RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, 4,
             RTST_INT32, "1",
             RTST_INT32, "4096",
             RTST_INT32, "1",
             RTST_INT32, "1"
             );
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBCHA?", 2, "+ECSCBCHA:0", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBLAN?", 2, "+ECSCBLAN:0", "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBCHA=1,\"4096-4096\"", 1, "OK");
        RTST_AT_CMD(SMS_CHANNEL, "AT+ECSCBLAN=1,\"1-1\"", 1, "OK");
        RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, RIL_E_SUCCESS);

        RTST_CASE_END();
    }
}

TEST(CdmaSmsTest, RIL_REQUEST_GET_SMSC_ADDRESS_1){
    RTST_CASE_BEGIN();
    RTST_RIL_VOID_REQUEST(RIL_REQUEST_GET_SMSC_ADDRESS);
    RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_GET_SMSC_ADDRESS, RIL_E_REQUEST_NOT_SUPPORTED);
    RTST_CASE_END();
}

TEST(CdmaSmsTest, RIL_REQUEST_SET_SMSC_ADDRESS_1){
    RTST_CASE_BEGIN();
    RTST_RIL_REQUEST(RIL_REQUEST_SET_SMSC_ADDRESS, 1, RTST_STRING, "+316540942002");
    RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_SET_SMSC_ADDRESS, RIL_E_REQUEST_NOT_SUPPORTED);
    RTST_CASE_END();
}

TEST(CdmaSmsTest, RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM_1) {
    RTST_CASE_BEGIN();
    RTST_RIL_REQUEST(RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM, 1, RTST_INT32, "1");
    RTST_AT_CMD(SMS_CHANNEL, "AT+CPMS=\"SM\"", 1, "OK");
    RTST_AT_CMD(SMS_CHANNEL, "AT+CMGD=0", 1, "OK");
    RTST_AT_CMD(SMS_CHANNEL, "AT+CPMS=\"ME\"", 1, "OK");
    RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM, RIL_E_SUCCESS);
    RTST_CASE_END();
}

TEST(CdmaSmsTest, RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM_2) {
    RTST_CASE_BEGIN();
    RTST_RIL_REQUEST(RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM, 1, RTST_INT32, "-1");
    RTST_AT_CMD(SMS_CHANNEL, "AT+CPMS=\"SM\"", 1, "OK");
    RTST_AT_CMD(SMS_CHANNEL, "AT+CMGD=,4", 1, "OK");
    RTST_AT_CMD(SMS_CHANNEL, "AT+CPMS=\"ME\"", 1, "OK");
    RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM, RIL_E_SUCCESS);
    RTST_CASE_END();
}

TEST(CdmaSmsTest, RIL_REQUEST_IMS_SEND_SMS_1) {
    RTST_CASE_BEGIN();
    RTST_RIL_REQUEST_NC(RIL_REQUEST_IMS_SEND_SMS, 37,
        RTST_INT32, "2",     // Cdma phone
        RTST_INT32, "0",     // retry
        RTST_INT32, "32",    // message ref
        RTST_INT32, "4098",  // teleServiceId
        RTST_INT32, "0",     // servicePresent
        RTST_INT32, "0",     // serviceCategory
        RTST_INT32, "0",     // address_digit_mode
        RTST_INT32, "0",     // address_nbr_mode
        RTST_INT32, "0",     // address_nbr_type
        RTST_INT32, "0",     // address_nbr_plan
        RTST_INT32, "11",    // address_nbr_of_digits
        RTST_INT32, "1",     // number 13522866822
        RTST_INT32, "3",
        RTST_INT32, "5",
        RTST_INT32, "2",
        RTST_INT32, "2",
        RTST_INT32, "8",
        RTST_INT32, "6",
        RTST_INT32, "6",
        RTST_INT32, "8",
        RTST_INT32, "2",
        RTST_INT32, "2",
        RTST_INT32, "0",    // sub address type
        RTST_INT32, "0",    // subaddr_odd
        RTST_INT32, "0",    // sub address digit
        RTST_INT32, "11",    // bearer data length
        RTST_INT32, "0",
        RTST_INT32, "3",
        RTST_INT32, "32",
        RTST_INT32, "0",
        RTST_INT32, "16",
        RTST_INT32, "1",
        RTST_INT32, "4",
        RTST_INT32, "16",
        RTST_INT32, "18",
        RTST_INT32, "68",
        RTST_INT32, "128"
        );
    RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_IMS_SEND_SMS, RIL_E_REQUEST_NOT_SUPPORTED);
    RTST_CASE_END();
}


TEST(CdmaSmsTest, RIL_REQUEST_CDMA_SEND_SMS_0) {
    RTST_CASE_BEGIN();
    RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_SEND_SMS, 67,
        RTST_INT32, "4098",  // teleServiceId
        RTST_INT32, "0",     // servicePresent
        RTST_INT32, "0",     // serviceCategory
        RTST_INT32, "0",     // address_digit_mode
        RTST_INT32, "0",     // address_nbr_mode
        RTST_INT32, "0",     // address_nbr_type
        RTST_INT32, "0",     // address_nbr_plan
        RTST_INT32, "11",    // address_nbr_of_digits
        RTST_INT32, "1",     // number 13522866822
        RTST_INT32, "3",
        RTST_INT32, "5",
        RTST_INT32, "2",
        RTST_INT32, "2",
        RTST_INT32, "8",
        RTST_INT32, "6",
        RTST_INT32, "6",
        RTST_INT32, "8",
        RTST_INT32, "2",
        RTST_INT32, "2",
        RTST_INT32, "0",    // sub address type
        RTST_INT32, "0",    // subaddr_odd
        RTST_INT32, "0",    // sub address digit
        RTST_INT32, "44",    // bearer data length
        RTST_INT32, "0",
        RTST_INT32, "3",
        RTST_INT32, "32",
        RTST_INT32, "0",
        RTST_INT32, "240",
        RTST_INT32, "1",
        RTST_INT32, "37",
        RTST_INT32, "17",
        RTST_INT32, "60",
        RTST_INT32, "76",
        RTST_INT32, "217",
        RTST_INT32, "243",
        RTST_INT32, "243",
        RTST_INT32, "201",
        RTST_INT32, "155",
        RTST_INT32, "70",
        RTST_INT32, "142",
        RTST_INT32, "121",
        RTST_INT32, "52",
        RTST_INT32, "104",
        RTST_INT32, "241",
        RTST_INT32, "143",
        RTST_INT32, "70",
        RTST_INT32, "140",
        RTST_INT32, "94",
        RTST_INT32, "178",
        RTST_INT32, "104",
        RTST_INT32, "197",
        RTST_INT32, "147",
        RTST_INT32, "38",
        RTST_INT32, "109",
        RTST_INT32, "26",
        RTST_INT32, "50",
        RTST_INT32, "102",
        RTST_INT32, "207",
        RTST_INT32, "163",
        RTST_INT32, "38",
        RTST_INT32, "125",
        RTST_INT32, "26",
        RTST_INT32, "50",
        RTST_INT32, "99",
        RTST_INT32, "209",
        RTST_INT32, "168",
        RTST_INT32, "00"
        );
    RTST_AT_CMD(SMS_CHANNEL, "at+cmgf = 0", 1, "OK");
    RTST_AT_CMD(SMS_CHANNEL,
        "AT+CMGS=\"0\", \"0000021002040702c4d48a19a088060100082c00032000f00125113c4cd9f3f3c99b468e793468f18f468c5eb268c593266d1a3266cfa3267d1a3263d1a800\"",
        1,
        "^HCMGSS:32");
    RTST_EXPECTED_RIL_RESPONSE_NC(RIL_REQUEST_CDMA_SEND_SMS, RIL_E_SUCCESS, 3,
            RTST_INT32, "32",
            RTST_INT32, "-1",
            RTST_INT32, "0"
            );
    RTST_CASE_END();
}



TEST(CdmaSmsTest, RIL_REQUEST_CDMA_SEND_SMS_1) {
    RTST_CASE_BEGIN();
    RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_SEND_SMS, 34,
        RTST_INT32, "4098",  // teleServiceId
        RTST_INT32, "0",     // servicePresent
        RTST_INT32, "0",     // serviceCategory
        RTST_INT32, "0",     // address_digit_mode
        RTST_INT32, "0",     // address_nbr_mode
        RTST_INT32, "0",     // address_nbr_type
        RTST_INT32, "0",     // address_nbr_plan
        RTST_INT32, "11",    // address_nbr_of_digits
        RTST_INT32, "1",     // number 13522866822
        RTST_INT32, "3",
        RTST_INT32, "5",
        RTST_INT32, "2",
        RTST_INT32, "2",
        RTST_INT32, "8",
        RTST_INT32, "6",
        RTST_INT32, "6",
        RTST_INT32, "8",
        RTST_INT32, "2",
        RTST_INT32, "2",
        RTST_INT32, "0",    // sub address type
        RTST_INT32, "0",    // subaddr_odd
        RTST_INT32, "0",    // sub address digit
        RTST_INT32, "11",    // bearer data length
        RTST_INT32, "0",
        RTST_INT32, "3",
        RTST_INT32, "32",
        RTST_INT32, "0",
        RTST_INT32, "16",
        RTST_INT32, "1",
        RTST_INT32, "4",
        RTST_INT32, "16",
        RTST_INT32, "18",
        RTST_INT32, "68",
        RTST_INT32, "128"
        );
    RTST_AT_CMD(SMS_CHANNEL, "at+cmgf = 0", 1, "OK");
    RTST_AT_CMD(SMS_CHANNEL,
        "AT+CMGS=\"0\", \"0000021002040702c4d48a19a088060100080b0003200010010410124480\"",
        1,
        "^HCMGSS:32");
    RTST_EXPECTED_RIL_RESPONSE_NC(RIL_REQUEST_CDMA_SEND_SMS, RIL_E_SUCCESS, 3,
            RTST_INT32, "32",
            RTST_INT32, "-1",
            RTST_INT32, "0"
            );
    RTST_CASE_END();
}


TEST(CdmaSmsTest, RIL_REQUEST_CDMA_SEND_SMS_2) {
    RTST_CASE_BEGIN();
    RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_SEND_SMS, 34,
        RTST_INT32, "4098",  // teleServiceId
        RTST_INT32, "0",     // servicePresent
        RTST_INT32, "0",     // serviceCategory
        RTST_INT32, "0",     // address_digit_mode
        RTST_INT32, "0",     // address_nbr_mode
        RTST_INT32, "0",     // address_nbr_type
        RTST_INT32, "0",     // address_nbr_plan
        RTST_INT32, "11",    // address_nbr_of_digits
        RTST_INT32, "1",     // number 13522866822
        RTST_INT32, "3",
        RTST_INT32, "5",
        RTST_INT32, "2",
        RTST_INT32, "2",
        RTST_INT32, "8",
        RTST_INT32, "6",
        RTST_INT32, "6",
        RTST_INT32, "8",
        RTST_INT32, "2",
        RTST_INT32, "2",
        RTST_INT32, "0",    // sub address type
        RTST_INT32, "0",    // subaddr_odd
        RTST_INT32, "0",    // sub address digit
        RTST_INT32, "11",    // bearer data length
        RTST_INT32, "0",
        RTST_INT32, "3",
        RTST_INT32, "32",
        RTST_INT32, "0",
        RTST_INT32, "16",
        RTST_INT32, "1",
        RTST_INT32, "4",
        RTST_INT32, "16",
        RTST_INT32, "18",
        RTST_INT32, "68",
        RTST_INT32, "128"
        );
    RTST_AT_CMD(SMS_CHANNEL, "at+cmgf = 0", 1, "OK");
    RTST_AT_CMD(SMS_CHANNEL,
        "AT+CMGS=\"0\", \"0000021002040702c4d48a19a088060100080b0003200010010410124480\"",
        1,
        "^HCMGSF:15,2");
    RTST_EXPECTED_RIL_RESPONSE_NC(RIL_REQUEST_CDMA_SEND_SMS, RIL_E_SYSTEM_ERR, 3,
            RTST_INT32, "0",
            RTST_INT32, "-1",
            RTST_INT32, "15"
            );
    RTST_CASE_END();
}


TEST(CdmaSmsTest, RIL_REQUEST_CDMA_SEND_SMS_3) {
    RTST_CASE_BEGIN();
    RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_SEND_SMS, 34,
        RTST_INT32, "4098",  // teleServiceId
        RTST_INT32, "0",     // servicePresent
        RTST_INT32, "0",     // serviceCategory
        RTST_INT32, "0",     // address_digit_mode
        RTST_INT32, "0",     // address_nbr_mode
        RTST_INT32, "0",     // address_nbr_type
        RTST_INT32, "0",     // address_nbr_plan
        RTST_INT32, "11",    // address_nbr_of_digits
        RTST_INT32, "1",     // number 13522866822
        RTST_INT32, "3",
        RTST_INT32, "5",
        RTST_INT32, "2",
        RTST_INT32, "2",
        RTST_INT32, "8",
        RTST_INT32, "6",
        RTST_INT32, "6",
        RTST_INT32, "8",
        RTST_INT32, "2",
        RTST_INT32, "2",
        RTST_INT32, "0",    // sub address type
        RTST_INT32, "0",    // subaddr_odd
        RTST_INT32, "0",    // sub address digit
        RTST_INT32, "11",    // bearer data length
        RTST_INT32, "0",
        RTST_INT32, "3",
        RTST_INT32, "32",
        RTST_INT32, "0",
        RTST_INT32, "16",
        RTST_INT32, "1",
        RTST_INT32, "4",
        RTST_INT32, "16",
        RTST_INT32, "18",
        RTST_INT32, "68",
        RTST_INT32, "128"
        );
    RTST_AT_CMD(SMS_CHANNEL, "at+cmgf = 0", 1, "OK");
    RTST_AT_CMD(SMS_CHANNEL,
        "AT+CMGS=\"0\", \"0000021002040702c4d48a19a088060100080b0003200010010410124480\"",
        1,
        "^HCMGSF:16,1");
    RTST_EXPECTED_RIL_RESPONSE_NC(RIL_REQUEST_CDMA_SEND_SMS, RIL_E_SYSTEM_ERR, 3,
            RTST_INT32, "0",
            RTST_INT32, "-1",
            RTST_INT32, "16"
            );
    RTST_CASE_END();
}

TEST(CdmaSmsTest, RIL_REQUEST_CDMA_SEND_SMS_4) {
    RTST_CASE_BEGIN();
    RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_SEND_SMS, 34,
        RTST_INT32, "4098",  // teleServiceId
        RTST_INT32, "0",     // servicePresent
        RTST_INT32, "0",     // serviceCategory
        RTST_INT32, "0",     // address_digit_mode
        RTST_INT32, "0",     // address_nbr_mode
        RTST_INT32, "0",     // address_nbr_type
        RTST_INT32, "0",     // address_nbr_plan
        RTST_INT32, "11",    // address_nbr_of_digits
        RTST_INT32, "1",     // number 13522866822
        RTST_INT32, "3",
        RTST_INT32, "5",
        RTST_INT32, "2",
        RTST_INT32, "2",
        RTST_INT32, "8",
        RTST_INT32, "6",
        RTST_INT32, "6",
        RTST_INT32, "8",
        RTST_INT32, "2",
        RTST_INT32, "2",
        RTST_INT32, "0",    // sub address type
        RTST_INT32, "0",    // subaddr_odd
        RTST_INT32, "0",    // sub address digit
        RTST_INT32, "11",    // bearer data length
        RTST_INT32, "0",
        RTST_INT32, "3",
        RTST_INT32, "32",
        RTST_INT32, "0",
        RTST_INT32, "16",
        RTST_INT32, "1",
        RTST_INT32, "4",
        RTST_INT32, "16",
        RTST_INT32, "18",
        RTST_INT32, "68",
        RTST_INT32, "128"
        );
    RTST_AT_CMD(SMS_CHANNEL, "at+cmgf = 0", 1, "OK");
    RTST_AT_CMD(SMS_CHANNEL,
        "AT+CMGS=\"0\", \"0000021002040702c4d48a19a088060100080b0003200010010410124480\"",
        1,
        "+CMS ERROR: 513");
    RTST_EXPECTED_RIL_RESPONSE_NC(RIL_REQUEST_CDMA_SEND_SMS, RIL_E_SYSTEM_ERR, 3,
            RTST_INT32, "0",
            RTST_INT32, "-1",
            RTST_INT32, "513"
            );
    RTST_CASE_END();
}


TEST(CdmaSmsTest, RIL_REQUEST_CDMA_SEND_SMS_5) {
    RTST_CASE_BEGIN();
    RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_SEND_SMS, 34,
        RTST_INT32, "4098",  // teleServiceId
        RTST_INT32, "0",     // servicePresent
        RTST_INT32, "0",     // serviceCategory
        RTST_INT32, "0",     // address_digit_mode
        RTST_INT32, "0",     // address_nbr_mode
        RTST_INT32, "0",     // address_nbr_type
        RTST_INT32, "0",     // address_nbr_plan
        RTST_INT32, "11",    // address_nbr_of_digits
        RTST_INT32, "1",     // number 13522866822
        RTST_INT32, "3",
        RTST_INT32, "5",
        RTST_INT32, "2",
        RTST_INT32, "2",
        RTST_INT32, "8",
        RTST_INT32, "6",
        RTST_INT32, "6",
        RTST_INT32, "8",
        RTST_INT32, "2",
        RTST_INT32, "2",
        RTST_INT32, "0",    // sub address type
        RTST_INT32, "0",    // subaddr_odd
        RTST_INT32, "0",    // sub address digit
        RTST_INT32, "11",    // bearer data length
        RTST_INT32, "0",
        RTST_INT32, "3",
        RTST_INT32, "32",
        RTST_INT32, "0",
        RTST_INT32, "16",
        RTST_INT32, "1",
        RTST_INT32, "4",
        RTST_INT32, "16",
        RTST_INT32, "18",
        RTST_INT32, "68",
        RTST_INT32, "128"
        );
    RTST_AT_CMD(SMS_CHANNEL, "at+cmgf = 0", 1, "OK");
    RTST_AT_CMD(SMS_CHANNEL,
        "AT+CMGS=\"0\", \"0000021002040702c4d48a19a088060100080b0003200010010410124480\"",
        1,
        "+CMS ERROR: 518");
    RTST_EXPECTED_RIL_RESPONSE_NC(RIL_REQUEST_CDMA_SEND_SMS, RIL_E_FDN_CHECK_FAILURE, 3,
            RTST_INT32, "0",
            RTST_INT32, "-1",
            RTST_INT32, "518"
            );
    RTST_CASE_END();
}

TEST(CdmaSmsTest, RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM_1) {
    RTST_CASE_BEGIN();
    RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM, 35,
        RTST_INT32, "1",     // status
        RTST_INT32, "4098",  // teleServiceId
        RTST_INT32, "0",     // servicePresent
        RTST_INT32, "0",     // serviceCategory
        RTST_INT32, "0",     // address_digit_mode
        RTST_INT32, "0",     // address_nbr_mode
        RTST_INT32, "0",     // address_nbr_type
        RTST_INT32, "0",     // address_nbr_plan
        RTST_INT32, "11",    // address_nbr_of_digits
        RTST_INT32, "1",     // number 13522866822
        RTST_INT32, "3",
        RTST_INT32, "5",
        RTST_INT32, "2",
        RTST_INT32, "2",
        RTST_INT32, "8",
        RTST_INT32, "6",
        RTST_INT32, "6",
        RTST_INT32, "8",
        RTST_INT32, "2",
        RTST_INT32, "2",
        RTST_INT32, "0",    // sub address type
        RTST_INT32, "0",    // subaddr_odd
        RTST_INT32, "0",    // sub address digit
        RTST_INT32, "11",    // bearer data length
        RTST_INT32, "0",
        RTST_INT32, "3",
        RTST_INT32, "32",
        RTST_INT32, "0",
        RTST_INT32, "16",
        RTST_INT32, "1",
        RTST_INT32, "4",
        RTST_INT32, "16",
        RTST_INT32, "18",
        RTST_INT32, "68",
        RTST_INT32, "128"
        );
    RTST_AT_CMD(SMS_CHANNEL, "AT+CPMS=\"ME\", \"SM\"", 1, "OK");
    RTST_AT_CMD(SMS_CHANNEL,
        "AT+CMGW=\"0\",\"0000021002040702c4d48a19a088060100080b0003200010010410124480\",1",
        2,
        "+CMGW:\"SM\",32",
        "OK");
    RTST_AT_CMD(SMS_CHANNEL, "AT+CPMS=\"ME\", \"ME\"", 1, "OK");
    RTST_EXPECTED_RIL_RESPONSE(RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM, RIL_E_SUCCESS, 1,
            RTST_INT32, "32");
    RTST_CASE_END();
}

TEST(CdmaSmsTest, RIL_UNSOL_RESPONSE_CDMA_NEW_SMS_1) {
    RTST_CASE_BEGIN();
    RTST_URC_STRING("+CMT:30,\"0000021002020702c4d48a19a088060100080b0003200010010410124480\"");
    RTST_EXPECTED_RIL_URC_NC(RIL_UNSOL_RESPONSE_CDMA_NEW_SMS, 34,
        RTST_INT32, "4098",  // teleServiceId
        RTST_INT32, "0",     // servicePresent
        RTST_INT32, "0",     // serviceCategory
        RTST_INT32, "1",     // address_digit_mode
        RTST_INT32, "0",     // address_nbr_mode
        RTST_INT32, "0",     // address_nbr_type
        RTST_INT32, "0",     // address_nbr_plan
        RTST_INT32, "11",    // address_nbr_of_digits
        RTST_INT32, "49",     // number 13522866822
        RTST_INT32, "51",
        RTST_INT32, "53",
        RTST_INT32, "50",
        RTST_INT32, "50",
        RTST_INT32, "56",
        RTST_INT32, "54",
        RTST_INT32, "54",
        RTST_INT32, "56",
        RTST_INT32, "50",
        RTST_INT32, "50",
        RTST_INT32, "0",    // sub address type
        RTST_INT32, "0",    // subaddr_odd
        RTST_INT32, "0",    // sub address digit
        RTST_INT32, "11",    // bearer data length
        RTST_INT32, "0",
        RTST_INT32, "3",
        RTST_INT32, "32",
        RTST_INT32, "0",
        RTST_INT32, "16",
        RTST_INT32, "1",
        RTST_INT32, "4",
        RTST_INT32, "16",
        RTST_INT32, "18",
        RTST_INT32, "68",
        RTST_INT32, "128");
    RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, 2,
        RTST_INT32, "0",
        RTST_INT32, "0");
    RTST_AT_CMD(SMS_CHANNEL, "at+cmgf = 0", 1, "OK");
    RTST_AT_CMD(SMS_CHANNEL, "AT+CNMA=13, \"02040702c4d48a19a088070100\"", 1, "OK");
    RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, RIL_E_SUCCESS);

    RTST_CASE_END();
}


// For CR ALPS03120853
TEST(CdmaSmsTest, RIL_UNSOL_RESPONSE_CDMA_NEW_SMS_5) {
    RTST_CASE_BEGIN();
    RTST_URC_STRING("+CMT:171, \"000002fdea0206020699660840060120089900031132c0018704280018d85d800bf01f00004008fc02e10e8189185c1c1b1a58d85d1a5bdb8bdd9b990b9dd85c0b9b5b5ccb5b595cdcd859d9402d21ebe12320a60cde93cc0c0c18594b5d11cc0023642243600c4dcccc4c0c4ccc8e0c0e4022a02381000018b462016040c0fd2020da1d1d1c0e8bcbcc4c0b8c8ccccb8ccb8dcd0e8e0c0bd7d1ccde1e4c00000306170111191844080140\"");
    RTST_EXPECTED_RIL_URC_NC(RIL_UNSOL_RESPONSE_CDMA_NEW_SMS, 164,
        RTST_INT32, "4100",  // teleServiceId
        RTST_INT32, "0",     // servicePresent
        RTST_INT32, "0",     // serviceCategory
        RTST_INT32, "1",     // address_digit_mode
        RTST_INT32, "0",     // address_nbr_mode
        RTST_INT32, "0",     // address_nbr_type
        RTST_INT32, "0",     // address_nbr_plan
        RTST_INT32, "8",    // address_nbr_of_digits
        RTST_INT32, "49",     // number 10659821
        RTST_INT32, "48",
        RTST_INT32, "54",
        RTST_INT32, "53",
        RTST_INT32, "57",
        RTST_INT32, "56",
        RTST_INT32, "50",
        RTST_INT32, "49",
        RTST_INT32, "0",    // sub address type
        RTST_INT32, "0",    // subaddr_odd
        RTST_INT32, "0",    // sub address digit
        RTST_INT32, "145",    // bearer data length
        RTST_INT32, "0",
        RTST_INT32, "3",
        RTST_INT32, "17",
        RTST_INT32, "50",
        RTST_INT32, "192",
        RTST_INT32, "1",
        RTST_INT32, "127",
        RTST_INT32, "3",
        RTST_INT32, "224",
        RTST_INT32, "0",
        RTST_INT32, "8",
        RTST_INT32, "1",
        RTST_INT32, "31",
        RTST_INT32, "128",
        RTST_INT32, "92",
        RTST_INT32, "33",
        RTST_INT32, "208",
        RTST_INT32, "49",
        RTST_INT32, "35",
        RTST_INT32, "11",
        RTST_INT32, "131",
        RTST_INT32, "131",
        RTST_INT32, "99",
        RTST_INT32, "75",
        RTST_INT32, "27",
        RTST_INT32, "11",
        RTST_INT32, "163",
        RTST_INT32, "75",
        RTST_INT32, "123",
        RTST_INT32, "113",
        RTST_INT32, "123",
        RTST_INT32, "179",
        RTST_INT32, "115",
        RTST_INT32, "33",
        RTST_INT32, "115",
        RTST_INT32, "187",
        RTST_INT32, "11",
        RTST_INT32, "129",
        RTST_INT32, "115",
        RTST_INT32, "107",
        RTST_INT32, "107",
        RTST_INT32, "153",
        RTST_INT32, "107",
        RTST_INT32, "107",
        RTST_INT32, "43",
        RTST_INT32, "155",
        RTST_INT32, "155",
        RTST_INT32, "11",
        RTST_INT32, "59",
        RTST_INT32, "40",
        RTST_INT32, "5",
        RTST_INT32, "164",
        RTST_INT32, "61",
        RTST_INT32, "124",
        RTST_INT32, "36",
        RTST_INT32, "100",
        RTST_INT32, "20",
        RTST_INT32, "193",
        RTST_INT32, "155",
        RTST_INT32, "210",
        RTST_INT32, "121",
        RTST_INT32, "129",
        RTST_INT32, "129",
        RTST_INT32, "131",
        RTST_INT32, "11",
        RTST_INT32, "41",
        RTST_INT32, "107",
        RTST_INT32, "162",
        RTST_INT32, "57",
        RTST_INT32, "128",
        RTST_INT32, "4",
        RTST_INT32, "108",
        RTST_INT32, "132",
        RTST_INT32, "72",
        RTST_INT32, "108",
        RTST_INT32, "1",
        RTST_INT32, "137",
        RTST_INT32, "185",
        RTST_INT32, "153",
        RTST_INT32, "137",
        RTST_INT32, "129",
        RTST_INT32, "137",
        RTST_INT32, "153",
        RTST_INT32, "145",
        RTST_INT32, "193",
        RTST_INT32, "129",
        RTST_INT32, "200",
        RTST_INT32, "4",
        RTST_INT32, "84",
        RTST_INT32, "4",
        RTST_INT32, "112",
        RTST_INT32, "32",
        RTST_INT32, "0",
        RTST_INT32, "3",
        RTST_INT32, "22",
        RTST_INT32, "140",
        RTST_INT32, "64",
        RTST_INT32, "44",
        RTST_INT32, "8",
        RTST_INT32, "24",
        RTST_INT32, "31",
        RTST_INT32, "164",
        RTST_INT32, "4",
        RTST_INT32, "27",
        RTST_INT32, "67",
        RTST_INT32, "163",
        RTST_INT32, "163",
        RTST_INT32, "129",
        RTST_INT32, "209",
        RTST_INT32, "121",
        RTST_INT32, "121",
        RTST_INT32, "137",
        RTST_INT32, "129",
        RTST_INT32, "113",
        RTST_INT32, "145",
        RTST_INT32, "153",
        RTST_INT32, "153",
        RTST_INT32, "113",
        RTST_INT32, "153",
        RTST_INT32, "113",
        RTST_INT32, "185",
        RTST_INT32, "161",
        RTST_INT32, "209",
        RTST_INT32, "193",
        RTST_INT32, "129",
        RTST_INT32, "122",
        RTST_INT32, "250",
        RTST_INT32, "57",
        RTST_INT32, "155",
        RTST_INT32, "195",
        RTST_INT32, "201",
        RTST_INT32, "128",
        RTST_INT32, "0",
        RTST_INT32, "0",
        RTST_INT32, "3",
        RTST_INT32, "6",
        RTST_INT32, "23",
        RTST_INT32, "1",
        RTST_INT32, "17",
        RTST_INT32, "25",
        RTST_INT32, "24",
        RTST_INT32, "68",
        RTST_INT32, "8",
        RTST_INT32, "1",
        RTST_INT32, "64"
        );
    RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, 2,
        RTST_INT32, "0",
        RTST_INT32, "0");
    RTST_AT_CMD(SMS_CHANNEL, "at+cmgf = 0", 1, "OK");
    RTST_AT_CMD(SMS_CHANNEL, "AT+CNMA=12, \"020406020699660840070120\"", 1, "OK");
    RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, RIL_E_SUCCESS);
    RTST_CASE_END();
}


// For Broadcast SMS
TEST(CdmaSmsTest, RIL_UNSOL_RESPONSE_CDMA_NEW_SMS_6) {
    RTST_CASE_BEGIN();
    RTST_URC_STRING("+CMT:42,\"010102000108230003100040011910d41850e2458d1e449952e64d9d3e851a54ea55ad5ec59b400c0100\"");
    RTST_EXPECTED_RIL_URC_NC(RIL_UNSOL_RESPONSE_CDMA_NEW_SMS, 47,
        RTST_INT32, "0", // no teleServicd Id
        RTST_INT32, "1", // servicePresent
        RTST_INT32, "1", // serviceCategory
        RTST_INT32, "0", // address_digit_mode
        RTST_INT32, "0", // address_nbr_mode
        RTST_INT32, "0", // address_nbr_type
        RTST_INT32, "0", // address_nbr_plan
        RTST_INT32, "0", // address_nbr_of_digits
        RTST_INT32, "0", // sub address type
        RTST_INT32, "0", // subaddr_odd
        RTST_INT32, "0", // sub address digit
        RTST_INT32, "35",// bearer data length
        RTST_INT32, "0",
        RTST_INT32, "3",
        RTST_INT32, "16",
        RTST_INT32, "0",
        RTST_INT32, "64",
        RTST_INT32, "1",
        RTST_INT32, "25",
        RTST_INT32, "16",
        RTST_INT32, "212",
        RTST_INT32, "24",
        RTST_INT32, "80",
        RTST_INT32, "226",
        RTST_INT32, "69",
        RTST_INT32, "141",
        RTST_INT32, "30",
        RTST_INT32, "68",
        RTST_INT32, "153",
        RTST_INT32, "82",
        RTST_INT32, "230",
        RTST_INT32, "77",
        RTST_INT32, "157",
        RTST_INT32, "62",
        RTST_INT32, "133",
        RTST_INT32, "26",
        RTST_INT32, "84",
        RTST_INT32, "234",
        RTST_INT32, "85",
        RTST_INT32, "173",
        RTST_INT32, "94",
        RTST_INT32, "197",
        RTST_INT32, "155",
        RTST_INT32, "64",
        RTST_INT32, "12",
        RTST_INT32, "1",
        RTST_INT32, "0");
    RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, 2,
        RTST_INT32, "0",
        RTST_INT32, "0");
    RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, RIL_E_SUCCESS);
    RTST_CASE_END();
}

TEST(CdmaSmsTest, RIL_UNSOL_RESPONSE_CDMA_NEW_SMS_7) {
    RTST_CASE_BEGIN();
    RTST_URC_STRING("+CMT:30,\"0000021002020702c4d48a19a088060100080b0003200010010410124480\"");
    RTST_EXPECTED_RIL_URC_NC(RIL_UNSOL_RESPONSE_CDMA_NEW_SMS, 34,
        RTST_INT32, "4098",  // teleServiceId
        RTST_INT32, "0",     // servicePresent
        RTST_INT32, "0",     // serviceCategory
        RTST_INT32, "1",     // address_digit_mode
        RTST_INT32, "0",     // address_nbr_mode
        RTST_INT32, "0",     // address_nbr_type
        RTST_INT32, "0",     // address_nbr_plan
        RTST_INT32, "11",    // address_nbr_of_digits
        RTST_INT32, "49",     // number 13522866822
        RTST_INT32, "51",
        RTST_INT32, "53",
        RTST_INT32, "50",
        RTST_INT32, "50",
        RTST_INT32, "56",
        RTST_INT32, "54",
        RTST_INT32, "54",
        RTST_INT32, "56",
        RTST_INT32, "50",
        RTST_INT32, "50",
        RTST_INT32, "0",    // sub address type
        RTST_INT32, "0",    // subaddr_odd
        RTST_INT32, "0",    // sub address digit
        RTST_INT32, "11",    // bearer data length
        RTST_INT32, "0",
        RTST_INT32, "3",
        RTST_INT32, "32",
        RTST_INT32, "0",
        RTST_INT32, "16",
        RTST_INT32, "1",
        RTST_INT32, "4",
        RTST_INT32, "16",
        RTST_INT32, "18",
        RTST_INT32, "68",
        RTST_INT32, "128");
    RTST_RIL_REQUEST_NC(RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, 2,
        RTST_INT32, "2",
        RTST_INT32, "33");
    RTST_AT_CMD(SMS_CHANNEL, "at+cmgf = 0", 1, "OK");
    RTST_AT_CMD(SMS_CHANNEL, "AT+CNMA=14, \"02040702c4d48a19a08807020221\"", 1, "OK");
    RTST_EXPECTED_RIL_VOID_RESPONSE(RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, RIL_E_SUCCESS);


    RTST_CASE_END();
}


TEST(CdmaSmsTest, RIL_UNSOL_SIM_SMS_STORAGE_FULL_1) {
    RTST_CASE_BEGIN();
    RTST_URC_STRING("^SMMEMFULL:\"SM\"");
    RTST_EXPECTED_RIL_VOID_URC(RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL);
    RTST_CASE_END();
}
