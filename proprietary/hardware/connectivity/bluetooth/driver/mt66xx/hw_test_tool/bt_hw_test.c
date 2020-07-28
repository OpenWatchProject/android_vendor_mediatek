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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include "bt_hw_test.h"


/**************************************************************************
 *                 G L O B A L   V A R I A B L E S                        *
***************************************************************************/

static int bt_fd = -1;

/* Used to read serial port */
static pthread_t rxThread;
static pthread_t inqThread;
static BOOL fgKillThread = FALSE;

/* mtk bt library */
static void *glib_handle = NULL;
typedef int (*INIT)(void);
typedef int (*DEINIT)(int fd);
typedef int (*WRITE)(int fd, unsigned char *buf, unsigned int len);
typedef int (*READ)(int fd, unsigned char *buf, unsigned int len);

INIT    bt_init = NULL;
DEINIT  bt_restore = NULL;
WRITE   bt_send_data = NULL;
READ    bt_receive_data = NULL;

/**************************************************************************
  *                         F U N C T I O N S                             *
***************************************************************************/

BOOL HW_TEST_BT_init(void)
{
    const char *errstr;

    TRC();

    glib_handle = dlopen("libbluetooth_mtk_pure.so", RTLD_LAZY);
    if (!glib_handle) {
        ERR("%s\n", dlerror());
        goto error;
    }

    dlerror(); /* Clear any existing error */

    bt_init = dlsym(glib_handle, "bt_init");
    bt_restore = dlsym(glib_handle, "bt_restore");
    bt_send_data = dlsym(glib_handle, "bt_send_data");
    bt_receive_data = dlsym(glib_handle, "bt_receive_data");

    if ((errstr = dlerror()) != NULL) {
        ERR("Can't find function symbols %s\n", errstr);
        goto error;
    }

    bt_fd = bt_init();
    if (bt_fd < 0)
        goto error;

    DBG("BT is enabled success\n");

    return TRUE;

error:
    if (glib_handle) {
        dlclose(glib_handle);
        glib_handle = NULL;
    }

    return FALSE;
}

void HW_TEST_BT_deinit(void)
{
    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
    }
    else {
        if (bt_fd < 0) {
            ERR("bt driver fd is invalid!\n");
        }
        else {
            bt_restore(bt_fd);
            bt_fd = -1;
        }
        dlclose(glib_handle);
        glib_handle = NULL;
    }

    return;
}

BOOL HW_TEST_BT_reset(void)
{
    UINT8 HCI_RESET[] = {0x01, 0x03, 0x0C, 0x0};
    UINT8 ucAckEvent[7];
    /* Event expected */
    UINT8 ucEvent[] = {0x04, 0x0E, 0x04, 0x01, 0x03, 0x0C, 0x00};
    UINT32 i;

    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    if (bt_send_data(bt_fd, HCI_RESET, sizeof(HCI_RESET)) < 0) {
        ERR("Send HCI reset command fails errno %d\n", errno);
        return FALSE;
    }

    DBG("write:\n");
    for (i = 0; i < sizeof(HCI_RESET); i++) {
        DBG("%02x\n", HCI_RESET[i]);
    }

    if (bt_receive_data(bt_fd, ucAckEvent, sizeof(ucAckEvent)) < 0) {
        ERR("Receive command complete event fails errno %d\n", errno);
        return FALSE;
    }

    DBG("read:\n");
    for (i = 0; i < sizeof(ucAckEvent); i++) {
        DBG("%02x\n", ucAckEvent[i]);
    }

    if (memcmp(ucAckEvent, ucEvent, sizeof(ucEvent))) {
        ERR("Receive unexpected event\n");
        return FALSE;
    }

    return TRUE;
}

void * HW_TEST_BT_PDN_LOOP(void *ptr)
{
    bool bRet = TRUE;
    int i = 0;
    char *ucSTART = "start";
    char *ucSTOP = "stop";
    char ucProp[PROPERTY_VALUE_MAX];
    property_set("sys.bt.pdn.status", ucSTART);
    FUNC_CB info_cb = ((pdn_arg_struct*)ptr)->info_cb;
    UINT8 pdn_length = ((pdn_arg_struct*)ptr)->pdn_length;
    UINT16 pdn_interval_ms = ((pdn_arg_struct*)ptr)->pdn_interval_ms;

    printf("HW_TEST_BT_PDN_LOOP ENTER\n");
    while(1)
    {
        property_get("sys.bt.pdn.status", ucProp, NULL);
        if(!memcmp(ucProp, ucSTOP, strlen(ucSTOP))) {
            printf("sys.bt.pdn.status = %s, stop pdn loop\n", ucProp);
            info_cb(NULL);
            break;
        }
        bRet = HW_TEST_BT_PDN_UT(pdn_length);
        if(bRet) {
            printf("[%4d]:pdn sucess\n" , i+1);
        }
        else {
            printf("[%4d]:pdn fail\n" , i+1);
        }
        i++;
        usleep(pdn_interval_ms*1000);
    }
    printf("HW_TEST_BT_PDN_LOOP leave\n");
    return NULL;
}

BOOL HW_TEST_BT_PDN_UT(UINT8 pdn_length)
{
    UINT8 HCI_PDN_TX_CMD[] = {0x01, 0xA6, 0xFC, 0x01, pdn_length};
    UINT8 ucAckEvent[13];
    UINT8 ucEvent[] = {0x04, 0x0E, 0x0A, 0x01, 0xA1, 0xFC, 0x26, 0xCC, 0x0F, 0x00, 0x00, 0x00, 0x00}; /* PDN test expected event */
    UINT32 i = 0;

    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    if (bt_send_data(bt_fd, HCI_PDN_TX_CMD, sizeof(HCI_PDN_TX_CMD)) < 0) {
        ERR("Send Tx test command fails errno %d\n", errno);
        printf("HW_TEST_BT_PDN_UT send command fail, errno %d\n", errno);
        return FALSE;
    }
    printf("send command:\n");
    for (i = 0; i < sizeof(HCI_PDN_TX_CMD); i++) {
        printf("%02x ", HCI_PDN_TX_CMD[i]);
    }
    printf("\n");

    if (bt_receive_data(bt_fd, ucAckEvent, sizeof(ucAckEvent)) < 0) {
        ERR("Receive PDN test event fails errno %d\n", errno);
        printf("Receive PDN test event fails errno %d\n", errno);
        return FALSE;
    }
    printf("read event:\n");
    for (i = 0; i < sizeof(ucAckEvent); i++) {
        printf("%02x ", ucAckEvent[i]);
    }
    printf("\n");
    if (memcmp(ucAckEvent, ucEvent, 6)) {
        printf("Receive unexpected event!\n");
        return FALSE;
    }
    if (ucAckEvent[9] != 0) {
        printf("PDN status error!\n");
        return FALSE;
    }

    return TRUE;
}

BOOL HW_TEST_BT_PDN_SET_CLOCK(UINT8 clk_rate)
{
    /* After connac20, set clock rate to mcu before imax test */
    UINT8 HCI_PDN_SET_CLK_CMD[] = {0x01, 0x6F, 0xFC, 0x08, 0x01, 0x0A, 0x04, 0x00, 0x09, 0x02, clk_rate, 00};
    UINT8 ucAckEvent[8];
    UINT8 ucEvent[] = {0x04, 0xE4, 0x05, 0x02, 0x0A, 0x01, 0x00, 0x00};
    UINT32 i = 0;

    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    if (bt_send_data(bt_fd, HCI_PDN_SET_CLK_CMD, sizeof(HCI_PDN_SET_CLK_CMD)) < 0) {
        ERR("Send Tx test command fails errno %d\n", errno);
        printf("HW_TEST_BT_PDN_SET_CLOCK send command fail, errno %d\n", errno);
        return FALSE;
    }
    printf("send command:\n");
    for (i = 0; i < sizeof(HCI_PDN_SET_CLK_CMD); i++) {
        printf("%02x ", HCI_PDN_SET_CLK_CMD[i]);
    }
    printf("\n");

    if (bt_receive_data(bt_fd, ucAckEvent, sizeof(ucAckEvent)) < 0) {
        ERR("Receive PDN test event fails errno %d\n", errno);
        printf("Receive PDN test event fails errno %d\n", errno);
        return FALSE;
    }
    printf("read event:\n");
    for (i = 0; i < sizeof(ucAckEvent); i++) {
        printf("%02x ", ucAckEvent[i]);
    }
    printf("\n");
    if (memcmp(ucAckEvent, ucEvent, 8)) {
        printf("Receive unexpected event!\n");
        return FALSE;
    }

    return TRUE;
}


BOOL HW_TEST_BT_TxOnlyTest_start(
    UINT8 tx_pattern,
    UINT8 hopping,
    int channel,
    UINT8 packet_type,
    UINT32 packet_len
    )
{
    UINT8 HCI_VS_TX_TEST[] = {0x01, 0x0D, 0xFC, 0x17, 0x00,
                              0x00,
                              0x00, /* Tx pattern */
                              0x00, /* Single frequency or 79 channels hopping */
                              0x00, /* Tx channel */
                              0x00,
                              0x00, 0x01,
                              0x00, /* Packet type */
                              0x00, 0x00, /* Packet length */
                              0x02, 0x00, 0x01, 0x00,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00};
    UINT8 ucAckEvent1[14];
    UINT8 ucAckEvent2[7];
    /* Event expected */
    UINT8 ucEvent1[] = {0x04, 0x03, 0x0B, 0x00}; /* Connection complete */
    UINT8 ucEvent2[] = {0x04, 0x0E, 0x04, 0x01, 0x0D, 0xFC, 0x00}; /* Command complete */
    UINT32 i;

    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    /* Prepare Tx test command */
    HCI_VS_TX_TEST[6] = tx_pattern;
    HCI_VS_TX_TEST[7] = hopping;
    HCI_VS_TX_TEST[8] = (UINT8)channel;
    HCI_VS_TX_TEST[12] = packet_type;
    HCI_VS_TX_TEST[13] = (UINT8)(packet_len & 0xFF);
    HCI_VS_TX_TEST[14] = (UINT8)((packet_len >> 8) & 0xFF);

    if (bt_send_data(bt_fd, HCI_VS_TX_TEST, sizeof(HCI_VS_TX_TEST)) < 0) {
        ERR("Send Tx test command fails errno %d\n", errno);
        return FALSE;
    }

    DBG("write:\n");
    for (i = 0; i < sizeof(HCI_VS_TX_TEST); i++) {
        DBG("%02x\n", HCI_VS_TX_TEST[i]);
    }

    if (tx_pattern != 0x0A) {
        /* Receive connection complete event */
        if (bt_receive_data(bt_fd, ucAckEvent1, sizeof(ucAckEvent1)) < 0) {
            ERR("Receive connection complete event fails errno %d\n", errno);
            return FALSE;
        }

        DBG("read:\n");
        for (i = 0; i < sizeof(ucAckEvent1); i++) {
            DBG("%02x\n", ucAckEvent1[i]);
        }

        if (memcmp(ucAckEvent1, ucEvent1, sizeof(ucEvent1))) {
            ERR("Receive unexpected event\n");
            return FALSE;
        }
    }

    /* Receive command complete event */
    if (bt_receive_data(bt_fd, ucAckEvent2, sizeof(ucAckEvent2)) < 0) {
        ERR("Receive command complete event fails errno %d\n", errno);
        return FALSE;
    }

    DBG("read:\n");
    for (i = 0; i < sizeof(ucAckEvent2); i++) {
        DBG("%02x\n", ucAckEvent2[i]);
    }

    if (memcmp(ucAckEvent2, ucEvent2, sizeof(ucEvent2))) {
        ERR("Receive unexpected event\n");
        return FALSE;
    }

    return TRUE;
}

BOOL HW_TEST_BT_TxOnlyTest_end(void)
{
    //return HW_TEST_BT_reset();
    return TRUE;
}

BOOL HW_TEST_BT_NonSignalRx_start(
    UINT8 rx_pattern,
    int channel,
    UINT8 packet_type,
    UINT32 tester_addr
    )
{
    UINT8 HCI_VS_RX_TEST[] = {0x01, 0x0D, 0xFC, 0x17, 0x00,
                              0x00, /* Rx pattern */
                              0x0B, /* Rx test mode */
                              0x00,
                              0x00,
                              0x00, /* Rx channel */
                              0x00, 0x01,
                              0x00, /* Packet type */
                              0x00, 0x00,
                              0x02, 0x00, 0x01, 0x00,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* Tester address */
                              0x00, 0x00};
    UINT8 ucAckEvent1[14];
    UINT8 ucAckEvent2[7];
    /* Event expected */
    UINT8 ucEvent1[] = {0x04, 0x03, 0x0B, 0x00}; /* Connection complete */
    UINT8 ucEvent2[] = {0x04, 0x0E, 0x04, 0x01, 0x0D, 0xFC, 0x00}; /* Command complete */
    UINT32 i;

    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    /* Prepare Non-Signal-Rx test command */
    HCI_VS_RX_TEST[5] = rx_pattern;
    HCI_VS_RX_TEST[9] = (UINT8)channel;
    HCI_VS_RX_TEST[12] = packet_type;
    HCI_VS_RX_TEST[21] = (UINT8)((tester_addr >> 24) & 0xFF);
    HCI_VS_RX_TEST[22] = (UINT8)((tester_addr >> 16) & 0xFF);
    HCI_VS_RX_TEST[23] = (UINT8)((tester_addr >> 8) & 0xFF);
    HCI_VS_RX_TEST[24] = (UINT8)(tester_addr & 0xFF);

    if (bt_send_data(bt_fd, HCI_VS_RX_TEST, sizeof(HCI_VS_RX_TEST)) < 0) {
        ERR("Send Non-Signal-Rx test command fails errno %d\n", errno);
        return FALSE;
    }

    DBG("write:\n");
    for (i = 0; i < sizeof(HCI_VS_RX_TEST); i++) {
        DBG("%02x\n", HCI_VS_RX_TEST[i]);
    }

    /* Receive connection complete event */
    if (bt_receive_data(bt_fd, ucAckEvent1, sizeof(ucAckEvent1)) < 0) {
        ERR("Receive connection complete event fails errno %d\n", errno);
        return FALSE;
    }

    DBG("read:\n");
    for (i = 0; i < sizeof(ucAckEvent1); i++) {
        DBG("%02x\n", ucAckEvent1[i]);
    }

    if (memcmp(ucAckEvent1, ucEvent1, sizeof(ucEvent1))) {
        ERR("Receive unexpected event\n");
        return FALSE;
    }

    /* Receive command complete event */
    if (bt_receive_data(bt_fd, ucAckEvent2, sizeof(ucAckEvent2)) < 0){
        ERR("Receive command complete event fails errno %d\n", errno);
        return FALSE;
    }

    DBG("read:\n");
    for (i = 0; i < sizeof(ucAckEvent2); i++) {
        DBG("%02x\n", ucAckEvent2[i]);
    }

    if (memcmp(ucAckEvent2, ucEvent2, sizeof(ucEvent2))) {
        ERR("Receive unexpected event\n");
        return FALSE;
    }

    return TRUE;
}

BOOL HW_TEST_BT_NonSignalRx_end(
    UINT32 *pu4RxPktCount,
    float  *pftPktErrRate,
    UINT32 *pu4RxByteCount,
    float  *pftBitErrRate
    )
{
    UINT8 HCI_VS_TEST_END[] = {0x01, 0x0D, 0xFC, 0x17, 0x00,
                               0x00,
                               0xFF, /* test end */
                               0x00,
                               0x00,
                               0x00,
                               0x00, 0x01,
                               0x00,
                               0x00, 0x00,
                               0x02, 0x00, 0x01, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00};
    UINT8 ucAckEvent[23];
    /* Event expected, the remaining bytes contain the test result */
    UINT8 ucEvent[] = {0x04, 0x0E, 0x14, 0x01, 0x0D, 0xFC, 0x00};
    UINT32 i;

    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    /* Non-Signal-Rx test end command */
    if (bt_send_data(bt_fd, HCI_VS_TEST_END, sizeof(HCI_VS_TEST_END)) < 0) {
        ERR("Send test end command fails errno %d\n", errno);
        return FALSE;
    }

    DBG("write:\n");
    for (i = 0; i < sizeof(HCI_VS_TEST_END); i++) {
        DBG("%02x\n", HCI_VS_TEST_END[i]);
    }

    /* Receive command complete event */
    if (bt_receive_data(bt_fd, ucAckEvent, sizeof(ucAckEvent)) < 0) {
        ERR("Receive command complete event fails errno %d\n", errno);
        return FALSE;
    }

    DBG("read:\n");
    for (i = 0; i < sizeof(ucAckEvent); i++) {
        DBG("%02x\n", ucAckEvent[i]);
    }

    if (memcmp(ucAckEvent, ucEvent, sizeof(ucEvent))) {
        ERR("Receive unexpected event\n");
        return FALSE;
    }
    else {
        /*
        * Parsing the test result:
        *   received packet count + PER + received payload byte count + BER
        */
        *pu4RxPktCount = *((UINT32*)&ucAckEvent[7]);
        *pftPktErrRate = (float)(*((UINT32*)&ucAckEvent[11]))/1000000;
        *pu4RxByteCount = *((UINT32*)&ucAckEvent[15]);
        *pftBitErrRate = (float)(*((UINT32*)&ucAckEvent[19]))/1000000;
    }

    return TRUE;
}


static void *BT_Rx_Thread(void *ptr);

BOOL HW_TEST_BT_TestMode_enter(int power)
{
    UINT8 HCI_VS_SET_RADIO[] =
        {0x01, 0x79, 0xFC, 0x06, 0x07, 0x80, 0x00, 0x06, 0x05, 0x07};
    UINT8 HCI_TEST_MODE_ENABLE[] =
        {0x01, 0x03, 0x18, 0x00};
    UINT8 HCI_WRITE_SCAN_ENABLE[] =
        {0x01, 0x1A, 0x0C, 0x01, 0x03};
    UINT8 HCI_SET_EVENT_FILTER[] =
        {0x01, 0x05, 0x0C, 0x03, 0x02, 0x00, 0x02};

    UINT8 ucAckEvent[7];
    /* Event expected */
    UINT8 ucEvent1[] = {0x04, 0x0E, 0x04, 0x01, 0x79, 0xFC, 0x00};
    UINT8 ucEvent2[] = {0x04, 0x0E, 0x04, 0x01, 0x03, 0x18, 0x00};
    UINT8 ucEvent3[] = {0x04, 0x0E, 0x04, 0x01, 0x1A, 0x0C, 0x00};
    UINT8 ucEvent4[] = {0x04, 0x0E, 0x04, 0x01, 0x05, 0x0C, 0x00};
    UINT32 i;

    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    /*
    * First command: Set Tx power
    */

    if (power >= 0 && power <= 7) {
        HCI_VS_SET_RADIO[4] = (UINT8)power;
        HCI_VS_SET_RADIO[9] = (UINT8)power;
    }

    if (bt_send_data(bt_fd, HCI_VS_SET_RADIO, sizeof(HCI_VS_SET_RADIO)) < 0) {
        ERR("Send set Tx power command fails errno %d\n", errno);
        return FALSE;
    }

    DBG("write:\n");
    for (i = 0; i < sizeof(HCI_VS_SET_RADIO); i++) {
        DBG("%02x\n", HCI_VS_SET_RADIO[i]);
    }

    if (bt_receive_data(bt_fd, ucAckEvent, sizeof(ucAckEvent)) < 0) {
        ERR("Receive command complete event fails errno %d\n", errno);
        return FALSE;
    }

    DBG("read:\n");
    for (i = 0; i < sizeof(ucAckEvent); i++) {
        DBG("%02x\n", ucAckEvent[i]);
    }

    if (memcmp(ucAckEvent, ucEvent1, sizeof(ucEvent1))){
        ERR("Receive unexpected event\n");
        return FALSE;
    }

    /*
    * Second command: HCI_Enable_Device_Under_Test_Mode
    */

    if (bt_send_data(bt_fd, HCI_TEST_MODE_ENABLE, sizeof(HCI_TEST_MODE_ENABLE)) < 0) {
        ERR("Send test mode enable command fails errno %d\n", errno);
        return FALSE;
    }

    DBG("write:\n");
    for (i = 0; i < sizeof(HCI_TEST_MODE_ENABLE); i++) {
        DBG("%02x\n", HCI_TEST_MODE_ENABLE[i]);
    }

    if (bt_receive_data(bt_fd, ucAckEvent, sizeof(ucAckEvent)) < 0) {
        ERR("Receive command complete event fails errno %d\n", errno);
        return FALSE;
    }

    DBG("read:\n");
    for (i = 0; i < sizeof(ucAckEvent); i++) {
        DBG("%02x\n", ucAckEvent[i]);
    }

    if (memcmp(ucAckEvent, ucEvent2, sizeof(ucEvent2))) {
        ERR("Receive unexpected event\n");
        return FALSE;
    }

    /*
    * Third command: HCI_Write_Scan_Enable
    */

    if (bt_send_data(bt_fd, HCI_WRITE_SCAN_ENABLE, sizeof(HCI_WRITE_SCAN_ENABLE)) < 0) {
        ERR("Send write scan enable command fails errno %d\n", errno);
        return FALSE;
    }

    DBG("write:\n");
    for (i = 0; i < sizeof(HCI_WRITE_SCAN_ENABLE); i++) {
        DBG("%02x\n", HCI_WRITE_SCAN_ENABLE[i]);
    }

    if (bt_receive_data(bt_fd, ucAckEvent, sizeof(ucAckEvent)) < 0) {
        ERR("Receive command complete event fails errno %d\n", errno);
        return FALSE;
    }

    DBG("read:\n");
    for (i = 0; i < sizeof(ucAckEvent); i++) {
        DBG("%02x\n", ucAckEvent[i]);
    }

    if(memcmp(ucAckEvent, ucEvent3, sizeof(ucEvent3))){
        ERR("Receive unexpected event\n");
        return FALSE;
    }

    /*
    * Fourth command: HCI_Set_Event_Filter
    */

    if (bt_send_data(bt_fd, HCI_SET_EVENT_FILTER, sizeof(HCI_SET_EVENT_FILTER)) < 0) {
        ERR("Send set event filter command fails errno %d\n", errno);
        return FALSE;
    }

    DBG("write:\n");
    for (i = 0; i < sizeof(HCI_SET_EVENT_FILTER); i++) {
        DBG("%02x\n", HCI_SET_EVENT_FILTER[i]);
    }

    if (bt_receive_data(bt_fd, ucAckEvent, sizeof(ucAckEvent)) < 0) {
        ERR("Receive command complete event fails errno %d\n", errno);
        return FALSE;
    }

    DBG("read:\n");
    for (i = 0; i < sizeof(ucAckEvent); i++) {
        DBG("%02x\n", ucAckEvent[i]);
    }

    if (memcmp(ucAckEvent, ucEvent4, sizeof(ucEvent4))) {
        ERR("Receive unexpected event\n");
        return FALSE;
    }

    /*
    * Create thread to receive events from Controller during the test
    */
    fgKillThread = FALSE;
    pthread_create(&rxThread, NULL, BT_Rx_Thread, NULL);

    return TRUE;
}

BOOL HW_TEST_BT_TestMode_exit(void)
{
    fgKillThread = TRUE;
    /* Wait until thread exit */
    pthread_join(rxThread, NULL);

    //return HW_TEST_BT_reset();
    return TRUE;
}

static void *BT_Rx_Thread(UNUSED_ATTR void *ptr)
{
    UINT8 ucRxBuf[512];
    UINT8 ucHeader = 0;
    UINT32 u4Len = 0, pkt_len = 0;
    UINT32 i;

    TRC();

    while (!fgKillThread) {

        if (!glib_handle) {
            ERR("mtk bt library is unloaded!\n");
            break;
        }
        if (bt_fd < 0) {
            ERR("bt driver fd is invalid!\n");
            break;
        }

        if (bt_receive_data(bt_fd, &ucHeader, sizeof(ucHeader)) < 0) {
            ERR("Zero byte read\n");
            continue;
        }

        memset(ucRxBuf, 0, sizeof(ucRxBuf));
        ucRxBuf[0] = ucHeader;
        u4Len = 1;

        switch (ucHeader) {
          case 0x04:
            DBG("Receive HCI event\n");
            if (bt_receive_data(bt_fd, &ucRxBuf[1], 2) < 0) {
                ERR("Read event header fails\n");
                goto CleanUp;
            }

            u4Len += 2;
            pkt_len = (UINT32)ucRxBuf[2];
            if ((u4Len + pkt_len) > sizeof(ucRxBuf)) {
                ERR("Read buffer overflow! packet len %d\n", u4Len + pkt_len);
                goto CleanUp;
            }

            if (bt_receive_data(bt_fd, &ucRxBuf[3], pkt_len) < 0) {
                ERR("Read event param fails\n");
                goto CleanUp;
            }

            u4Len += pkt_len;
            break;

          case 0x02:
            DBG("Receive ACL data\n");
            if (bt_receive_data(bt_fd, &ucRxBuf[1], 4) < 0) {
                ERR("Read ACL header fails\n");
                goto CleanUp;
            }

            u4Len += 4;
            pkt_len = (((UINT32)ucRxBuf[4]) << 8);
            pkt_len += (UINT32)ucRxBuf[3]; /*little endian*/
            if ((u4Len + pkt_len) > sizeof(ucRxBuf)) {
                ERR("Read buffer overflow! packet len %d\n", u4Len + pkt_len);
                goto CleanUp;
            }

            if (bt_receive_data(bt_fd, &ucRxBuf[5], pkt_len) < 0) {
                ERR("Read ACL data fails\n");
                goto CleanUp;
            }

            u4Len += pkt_len;
            break;

          case 0x03:
            DBG("Receive SCO data\n");
            if (bt_receive_data(bt_fd, &ucRxBuf[1], 3) < 0) {
                ERR("Read SCO header fails\n");
                goto CleanUp;
            }

            u4Len += 3;
            pkt_len = (UINT32)ucRxBuf[3];
            if ((u4Len + pkt_len) > sizeof(ucRxBuf)) {
                ERR("Read buffer overflow! packet len %d\n", u4Len + pkt_len);
                goto CleanUp;
            }

            if (bt_receive_data(bt_fd, &ucRxBuf[4], pkt_len) < 0) {
                ERR("Read SCO data fails\n");
                goto CleanUp;
            }

            u4Len += pkt_len;
            break;

          default:
            ERR("Unexpected BT packet header %02x\n", ucHeader);
            goto CleanUp;
        }

        /* Dump rx packet */
        DBG("read:\n");
        for (i = 0; i < u4Len; i++) {
            DBG("%02x\n", ucRxBuf[i]);
        }
    }

CleanUp:
    return NULL;
}

static BOOL BT_Inquiry(void)
{
    UINT8 HCI_INQUIRY[] =
        {0x01, 0x01, 0x04, 0x05, 0x33, 0x8B, 0x9E, 0x05, 0x0A};
    UINT32 i;

    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    if (bt_send_data(bt_fd, HCI_INQUIRY, sizeof(HCI_INQUIRY)) < 0) {
        ERR("Send inquiry command fails errno %d\n", errno);
        return FALSE;
    }

    DBG("write:\n");
    for (i = 0; i < sizeof(HCI_INQUIRY); i++) {
        DBG("%02x\n", HCI_INQUIRY[i]);
    }

    return TRUE;
}

static void *BT_Inquiry_Thread(void *ptr)
{
    UINT8 ucRxBuf[256];
    UINT8 ucHeader = 0;
    UINT32 u4Len = 0, pkt_len = 0;
    UINT32 i;
    FUNC_CB info_cb = (FUNC_CB)ptr;
    UCHAR btaddr[6];
    char  str[30];

    TRC();

    while (!fgKillThread) {

        if (!glib_handle) {
            ERR("mtk bt library is unloaded!\n");
            break;
        }
        if (bt_fd < 0) {
            ERR("bt driver fd is invalid!\n");
            break;
        }

        if (bt_receive_data(bt_fd, &ucHeader, sizeof(ucHeader)) < 0) {
            ERR("Zero byte read\n");
            continue;
        }

        memset(ucRxBuf, 0, sizeof(ucRxBuf));
        ucRxBuf[0] = ucHeader;
        u4Len = 1;

        switch (ucHeader) {
          case 0x04:
            DBG("Receive HCI event\n");
            if (bt_receive_data(bt_fd, &ucRxBuf[1], 2) < 0) {
                ERR("Read event header fails\n");
                goto CleanUp;
            }

            u4Len += 2;
            pkt_len = (UINT32)ucRxBuf[2];
            if ((u4Len + pkt_len) > sizeof(ucRxBuf)) {
                ERR("Read buffer overflow! packet len %d\n", u4Len + pkt_len);
                goto CleanUp;
            }

            if (bt_receive_data(bt_fd, &ucRxBuf[3], pkt_len) < 0) {
                ERR("Read event param fails\n");
                goto CleanUp;
            }

            u4Len += pkt_len;

            /* Dump rx packet */
            DBG("read:\n");
            for (i = 0; i < u4Len; i++) {
                 DBG("%02x\n", ucRxBuf[i]);
            }

            if (ucRxBuf[1] == 0x0F) {
                /* Command status event */
                if (pkt_len != 0x04) {
                    ERR("Unexpected command status event len %d", pkt_len);
                    goto CleanUp;
                }

                if (ucRxBuf[3] != 0x00) {
                    ERR("Unexpected command status %02x", ucRxBuf[3]);
                    goto CleanUp;
                }
            }
            else if (ucRxBuf[1] == 0x01) {
                /* Inquiry complete event */
                if (pkt_len != 0x01) {
                    ERR("Unexpected inquiry complete event len %d", pkt_len);
                    goto CleanUp;
                }

                if (ucRxBuf[3] != 0x00) {
                    ERR("Unexpected inquiry complete status %02x", ucRxBuf[3]);
                    goto CleanUp;
                }

                info_cb("---Inquiry completed---\n");
                fgKillThread = TRUE;
            }
            else if (ucRxBuf[1] == 0x02) {
                /* Inquiry result event */
                if (pkt_len != 0x0F) {
                    ERR("Unexpected inquiry result event len %d", pkt_len);
                    goto CleanUp;
                }

                /* Retrieve BD addr */
                btaddr[0] = ucRxBuf[9];
                btaddr[1] = ucRxBuf[8];
                btaddr[2] = ucRxBuf[7];
                btaddr[3] = ucRxBuf[6];
                btaddr[4] = ucRxBuf[5];
                btaddr[5] = ucRxBuf[4];

                /* Inquiry result callback */
                memset(str, 0, sizeof(str));
                sprintf(str, "    %02x:%02x:%02x:%02x:%02x:%02x\n",
                    btaddr[0], btaddr[1], btaddr[2], btaddr[3], btaddr[4], btaddr[5]);
                info_cb(str);
            }
            else {
                /* simply ignore it? */
                DBG("Unexpected event %02x\n", ucRxBuf[1]);
            }
            break;

          default:
            ERR("Unexpected BT packet header %02x\n", ucHeader);
            goto CleanUp;
        }
    }

CleanUp:
    return NULL;
}

BOOL HW_TEST_BT_Inquiry(FUNC_CB info_cb)
{
    if (BT_Inquiry() == TRUE) {
        /*
        * Create thread to receive events during the Inquiry procedure
        */
        fgKillThread = FALSE;
        pthread_create(&inqThread, NULL, BT_Inquiry_Thread, (void*)info_cb);
        return TRUE;
    }
    else {
        return FALSE;
    }
}

BOOL HW_TEST_BT_LE_Tx_start(UINT8 tx_pattern, int channel)
{
    UINT8 HCI_LE_TX_TEST[] = {0x01, 0x1E, 0x20, 0x03,
                              0x00, /* Tx channel */
                              0x25, /* Packet payload data length */
                              0x00};/* Tx payload pattern */
    UINT8 ucAckEvent[7];
    /* Event expected */
    UINT8 ucEvent[] = {0x04, 0x0E, 0x04, 0x01, 0x1E, 0x20, 0x00};
    UINT32 i;

    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    /* Prepare LE Tx test command */
    HCI_LE_TX_TEST[4] = (UINT8)channel;
    HCI_LE_TX_TEST[6] = tx_pattern;

    if (bt_send_data(bt_fd, HCI_LE_TX_TEST, sizeof(HCI_LE_TX_TEST)) < 0) {
        ERR("Send LE Tx test command fails errno %d\n", errno);
        return FALSE;
    }

    DBG("write:\n");
    for (i = 0; i < sizeof(HCI_LE_TX_TEST); i++) {
        DBG("%02x\n", HCI_LE_TX_TEST[i]);
    }

    /* Receive command complete event */
    if (bt_receive_data(bt_fd, ucAckEvent, sizeof(ucAckEvent)) < 0) {
        ERR("Receive command complete event fails errno %d\n", errno);
        return FALSE;
    }

    DBG("read:\n");
    for (i = 0; i < sizeof(ucAckEvent); i++) {
        DBG("%02x\n", ucAckEvent[i]);
    }

    if (memcmp(ucAckEvent, ucEvent, sizeof(ucEvent))) {
        ERR("Receive unexpected event\n");
        return FALSE;
    }

    return TRUE;
}

BOOL HW_TEST_BT_LE_Enhanced_Tx_start(int channel, UINT32 payload_len, UINT8 pattern, UINT8 phy_rate)
{
    UINT8 HCI_LE_ENHANCED_TX_TEST[] = {0x01, 0x34, 0x20, 0x04,
                                       0x00, /* Tx channel */
                                       0x00, /* Payload length */
                                       0x00, /* Tx pattern */
                                       0x00};/* PHY rate */
    UINT8 ucAckEvent[7];
    /* Event expected */
    UINT8 ucEvent[] = {0x04, 0x0E, 0x04, 0x01, 0x34, 0x20, 0x00};
    UINT32 i;

    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    /* Prepare LE Tx test command */
    HCI_LE_ENHANCED_TX_TEST[4] = (UINT8)channel;
    HCI_LE_ENHANCED_TX_TEST[5] = (UINT8)payload_len;
    HCI_LE_ENHANCED_TX_TEST[6] = pattern;
    HCI_LE_ENHANCED_TX_TEST[7] = phy_rate;


    if (bt_send_data(bt_fd, HCI_LE_ENHANCED_TX_TEST, sizeof(HCI_LE_ENHANCED_TX_TEST)) < 0) {
        ERR("Send LE Enhanced Tx test command fails errno %d\n", errno);
        return FALSE;
    }

    DBG("write:\n");
    for (i = 0; i < sizeof(HCI_LE_ENHANCED_TX_TEST); i++) {
        DBG("%02x\n", HCI_LE_ENHANCED_TX_TEST[i]);
    }

    /* Receive command complete event */
    if (bt_receive_data(bt_fd, ucAckEvent, sizeof(ucAckEvent)) < 0) {
        ERR("Receive command complete event fails errno %d\n", errno);
        return FALSE;
    }

    DBG("read:\n");
    for (i = 0; i < sizeof(ucAckEvent); i++) {
        DBG("%02x\n", ucAckEvent[i]);
    }

    if (memcmp(ucAckEvent, ucEvent, sizeof(ucEvent))) {
        ERR("Receive unexpected event\n");
        return FALSE;
    }

    return TRUE;
}


BOOL HW_TEST_BT_LE_Tx_end(void)
{
    UINT8 HCI_LE_TEST_END[] = {0x01, 0x1F, 0x20, 0x00};
    UINT8 ucAckEvent[9];
    /* Event expected, for tx test, the last two bytes are 0x0000 */
    UINT8 ucEvent[] = {0x04, 0x0E, 0x06, 0x01, 0x1F, 0x20, 0x00, 0x00, 0x00};
    UINT32 i;

    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    /* LE test end command */
    if (bt_send_data(bt_fd, HCI_LE_TEST_END, sizeof(HCI_LE_TEST_END)) < 0) {
        ERR("Send LE test end command fails errno %d\n", errno);
        return FALSE;
    }

    DBG("write:\n");
    for (i = 0; i < sizeof(HCI_LE_TEST_END); i++) {
        DBG("%02x\n", HCI_LE_TEST_END[i]);
    }

    /* Receive command complete event */
    if (bt_receive_data(bt_fd, ucAckEvent, sizeof(ucAckEvent)) < 0) {
        ERR("Receive command complete event fails errno %d\n", errno);
        return FALSE;
    }

    DBG("read:\n");
    for (i = 0; i < sizeof(ucAckEvent); i++) {
        DBG("%02x\n", ucAckEvent[i]);
    }

    if (memcmp(ucAckEvent, ucEvent, sizeof(ucEvent))) {
        ERR("Receive unexpected event\n");
        return FALSE;
    }

    return TRUE;
}

BOOL HW_TEST_BT_LE_Rx_start(int channel)
{
    UINT8 HCI_LE_RX_TEST[] = {0x01, 0x1D, 0x20, 0x01,
                              0x00};/* Rx channel */
    UINT8 ucAckEvent[7];
    /* Event expected */
    UINT8 ucEvent[] = {0x04, 0x0E, 0x04, 0x01, 0x1D, 0x20, 0x00};
    UINT32 i;

    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    /* Prepare LE Rx test command */
    HCI_LE_RX_TEST[4] = (UINT8)channel;

    if (bt_send_data(bt_fd, HCI_LE_RX_TEST, sizeof(HCI_LE_RX_TEST)) < 0) {
        ERR("Send LE Rx test command fails errno %d\n", errno);
        return FALSE;
    }

    DBG("write:\n");
    for (i = 0; i < sizeof(HCI_LE_RX_TEST); i++) {
        DBG("%02x\n", HCI_LE_RX_TEST[i]);
    }

    /* Receive command complete event */
    if (bt_receive_data(bt_fd, ucAckEvent, sizeof(ucAckEvent)) < 0){
        ERR("Receive command complete event fails errno %d\n", errno);
        return FALSE;
    }

    DBG("read:\n");
    for (i = 0; i < sizeof(ucAckEvent); i++) {
        DBG("%02x\n", ucAckEvent[i]);
    }

    if (memcmp(ucAckEvent, ucEvent, sizeof(ucEvent))) {
        ERR("Receive unexpected event\n");
        return FALSE;
    }

    return TRUE;
}

BOOL HW_TEST_BT_LE_Enhanced_Rx_start(int channel, UINT8 phy_rate, UINT8 index)
{
    UINT8 HCI_LE_ENHANCED_RX_TEST[] = {0x01, 0x33, 0x20, 0x03,
                                       0x00,  /* Rx channel */
                                       0x00,  /* PHY rate */
                                       0x00}; /* Modulation index */
    UINT8 ucAckEvent[7];
    /* Event expected */
    UINT8 ucEvent[] = {0x04, 0x0E, 0x04, 0x01, 0x33, 0x20, 0x00};
    UINT32 i;

    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    /* Prepare LE Rx test command */
    HCI_LE_ENHANCED_RX_TEST[4] = (UINT8)channel;
    HCI_LE_ENHANCED_RX_TEST[5] = phy_rate;
    HCI_LE_ENHANCED_RX_TEST[6] = index;


    if (bt_send_data(bt_fd, HCI_LE_ENHANCED_RX_TEST, sizeof(HCI_LE_ENHANCED_RX_TEST)) < 0) {
        ERR("Send LE Rx test command fails errno %d\n", errno);
        return FALSE;
    }

    DBG("write:\n");
    for (i = 0; i < sizeof(HCI_LE_ENHANCED_RX_TEST); i++) {
        DBG("%02x\n", HCI_LE_ENHANCED_RX_TEST[i]);
    }

    /* Receive command complete event */
    if (bt_receive_data(bt_fd, ucAckEvent, sizeof(ucAckEvent)) < 0){
        ERR("Receive command complete event fails errno %d\n", errno);
        return FALSE;
    }

    DBG("read:\n");
    for (i = 0; i < sizeof(ucAckEvent); i++) {
        DBG("%02x\n", ucAckEvent[i]);
    }

    if (memcmp(ucAckEvent, ucEvent, sizeof(ucEvent))) {
        ERR("Receive unexpected event\n");
        return FALSE;
    }

    return TRUE;
}

BOOL HW_TEST_BT_LE_Rx_end(UINT16 *pu2RxPktCount)
{
    UINT8 HCI_LE_TEST_END[] = {0x01, 0x1F, 0x20, 0x00};
    UINT8 ucAckEvent[9];
    /* Event expected, for rx test, the last two bytes are the total received packet count */
    UINT8 ucEvent[] = {0x04, 0x0E, 0x06, 0x01, 0x1F, 0x20, 0x00};
    UINT32 i;

    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    /* LE test end command */
    if (bt_send_data(bt_fd, HCI_LE_TEST_END, sizeof(HCI_LE_TEST_END)) < 0) {
        ERR("Send LE test end command fails errno %d\n", errno);
        return FALSE;
    }

    DBG("write:\n");
    for (i = 0; i < sizeof(HCI_LE_TEST_END); i++) {
        DBG("%02x\n", HCI_LE_TEST_END[i]);
    }

    /* Receive command complete event */
    if (bt_receive_data(bt_fd, ucAckEvent, sizeof(ucAckEvent)) < 0) {
        ERR("Receive command complete event fails errno %d\n", errno);
        return FALSE;
    }

    DBG("read:\n");
    for (i = 0; i < sizeof(ucAckEvent); i++) {
        DBG("%02x\n", ucAckEvent[i]);
    }

    if (memcmp(ucAckEvent, ucEvent, sizeof(ucEvent))) {
        ERR("Receive unexpected event\n");
        return FALSE;
    }
    else {
        *pu2RxPktCount = *((UINT16*)&ucAckEvent[7]);
    }

    return TRUE;
}
