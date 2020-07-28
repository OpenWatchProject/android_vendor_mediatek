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
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>

#include "bt_hw_test.h"


#define VERSION "5.0"

#define for_each_opt(opt, long, short) while ((opt=getopt_long(argc, argv, short ? short:"+", long, NULL)) != -1)

/**************************************************************************
 *                 G L O B A L   V A R I A B L E S                        *
***************************************************************************/
//static int bt_init = false;
static bool g_inquiry_complete = false;
static bool g_pdn_complete = false;

/**************************************************************************
 *                         F U N C T I O N S                              *
***************************************************************************/
static pthread_t pdnLoopThread;

static int check_hex_str(char *str)
{
    int i = 0;
    int len = strlen(str);

    for (; i < len; i++) {
        if ((('a' <= str[i]) && (str[i] <= 'f')) || (('A' <= str[i]) && (str[i] <= 'F')) ||
            (('0' <= str[i]) && (str[i] <= '9')))
            continue;
        else
            return -1;
    }
    return 0;
}

void print_info(char *info)
{
    printf("%s", info);
    if (0 == strcmp(info, "---Inquiry completed---\n"))
        g_inquiry_complete = true;
}

static void wait_user_exit(void)
{
    /* To receive terminal input */
    char buf[5] = {0};
    int tmp;
    int i = 0;
    /* Loop in waiting for user type "exit" */

    while ((tmp = getchar()) != EOF) {
        if (i >= 5)
            i = 0; /* Rollback */
        buf[i] = (char)tmp;

        if (tmp != '\r' && tmp != '\n') {
            i++;
        }
        else {
            buf[i] = '\0';
            if (0 != strcmp(buf, "exit"))
                i = 0; /* Discard this string */
            else
                break;
        }
    }
}

#if 0
static struct option poweron_options[] = {
    { "help",	0, 0, 'h' },
    { 0, 0, 0, 0 }
};

static const char *poweron_help =
    "Usage:\n"
    "\tpoweron [no option]\n";

static void cmd_power_on(int argc, char **argv)
{
    int opt;

    for_each_opt(opt, poweron_options, "+h") {
        switch (opt) {
        case 'h':
        default:
            printf("%s", poweron_help);
            return;
        }
    }

    if (bt_init == true) {
        printf("BT device is already on\n");
    }
    else {
        if (HW_TEST_BT_init()) {
            printf("BT device power on success\n");
            bt_init = true;
        }
        else {
            printf("BT device power on failed\n");
        }
    }
    return;
}

static struct option poweroff_options[] = {
    { "help",	0, 0, 'h' },
    { 0, 0, 0, 0 }
};

static const char *poweroff_help =
    "Usage:\n"
    "\tpoweroff [no option]\n";

static void cmd_power_off(int argc, char **argv)
{
    int opt;

    for_each_opt(opt, poweroff_options, "+h") {
        switch (opt) {
        case 'h':
        default:
            printf("%s", poweroff_help);
            return;
        }
    }

    if (bt_init == false) {
        printf("BT device is already off\n");
    }
    else {
        HW_TEST_BT_deinit();
        printf("BT device power off\n");
        bt_init = false;
    }
    return;
}

static struct option reset_options[] = {
    { "help",	0, 0, 'h' },
    { 0, 0, 0, 0 }
};

static const char *reset_help =
    "Usage:\n"
    "\treset [no option]\n";

static void cmd_reset(int argc, char **argv)
{
    int opt;

    for_each_opt(opt, reset_options, "+h") {
        switch (opt) {
        case 'h':
        default:
            printf("%s", reset_help);
            return;
        }
    }

    if (bt_init == false) {
        printf("BT device is off, run \"autobt poweron\" first\n");
    }
    else {
        if (HW_TEST_BT_reset()) {
            printf("BT device reset success\n");
        }
        else {
            printf("BT device reset failed\n");
        }
    }
    return;
}
#endif

static struct option tx_options[] = {
    { "help",	0, 0, 'h' },
    { "pattern", 1, 0, 'p' },
    { "hopping", 1, 0, 'o' },
    { "channel", 1, 0, 'c' },
    { "type", 1, 0, 't' },
    { "length", 1, 0, 'l' },
    { 0, 0, 0, 0 }
};

static const char *tx_help =
    "Usage:\n"
    "\ttx [--pattern] 0x01:\tTx 0000 pattern\n"
    "\t               0x02:\tTx 1111 pattern\n"
    "\t               0x03:\tTx 1010 pattern\n"
    "\t               0x04:\tTx pseudo random bit sequence\n"
    "\t               0x09:\tTx 11110000 pattern\n"
    "\t               0x0A:\tTx single tone\n"
    "\n"
    "\t   [--hopping] 0x00:\tSingle frequency\n"
    "\t               0x01:\t79 channels frequency hopping\n"
    "\n"
    "\t   [--channel] Integer (0~78 channel for single frequency)\n"
    "\n"
    "\t   [--type]    0x00:\tNULL\n"
    "\t               0x01:\tPOLL\n"
    "\t               0x02:\tFHS\n"
    "\t               0x03:\tDM1\n"
    "\t               0x04:\tDH1\n"
    "\t               0x05:\tHV1\n"
    "\t               0x06:\tHV2\n"
    "\t               0x07:\tHV3\n"
    "\t               0x08:\tDV\n"
    "\t               0x09:\tAUX\n"
    "\t               0x0A:\tDM3\n"
    "\t               0x0B:\tDH3\n"
    "\t               0x0E:\tDM5\n"
    "\t               0x0F:\tDH5\n"
    "\t               0x17:\tEV3\n"
    "\t               0x1C:\tEV4\n"
    "\t               0x1D:\tEV5\n"
    "\t               0x24:\t2-DH1\n"
    "\t               0x28:\t3-DH1\n"
    "\t               0x2A:\t2-DH3\n"
    "\t               0x2B:\t3-DH3\n"
    "\t               0x2E:\t2-DH5\n"
    "\t               0x2F:\t3-DH5\n"
    "\t               0x36:\t2-EV3\n"
    "\t               0x37:\t3-EV3\n"
    "\t               0x3C:\t2-EV5\n"
    "\t               0x3D:\t3-EV5\n"
    "\n"
    "\t   [--length]  integer\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt tx --pattern 0x01 --hopping 0x00 --channel 7 --type 0x04 --length 27\n"
    "\n\n"
    "Notice:\n"
    "\tThis command is to start Tx only test, Controller will send out the specified Tx packet continuously\n"
    "\tTo end test, please type in \"exit\"\n"
    "\n";

static void cmd_tx_test(int argc, char **argv)
{
    int opt;
    unsigned char opt_bits = 0;
    unsigned char pattern = 0, hopping = 0, type = 0;
    int channel = 0;
    unsigned int length = 0;

    for_each_opt(opt, tx_options, "+p:o:c:t:l:h") {
        switch (opt) {
        case 'p':
            pattern = (unsigned char)strtoul(optarg, NULL, 16);
            opt_bits |= BIT(0);
            break;

        case 'o':
            hopping = (unsigned char)strtoul(optarg, NULL, 16);
            opt_bits |= BIT(1);
            break;

        case 'c':
            channel = atoi(optarg);
            if (channel < 0 || channel > 78) {
                printf("Invalid command option parameter!\n");
                printf("%s", tx_help);
                return;
            }
            opt_bits |= BIT(2);
            break;

        case 't':
            type = (unsigned char)strtoul(optarg, NULL, 16);
            opt_bits |= BIT(3);
            break;

        case 'l':
            length = (unsigned int)strtoul(optarg, NULL, 10);
            opt_bits |= BIT(4);
            break;

        case 'h':
        default:
            printf("%s", tx_help);
            return;
        }
    }

    if (~opt_bits & BITS(0, 4)) {
        printf("Incomplete command options!\n");
        printf("%s", tx_help);
        return;
    }

    /* BT power on & Initialize */
    if (HW_TEST_BT_init()) {
        printf("BT device power on success\n");
    }
    else {
        printf("BT device power on failed\n");
        return;
    }

    printf("Tx pattern: 0x%02x\n", pattern);
    printf("Hopping: 0x%02x\n", hopping);
    printf("Tx channel: %d\n", channel);
    printf("Packet type: 0x%02x\n", type);
    printf("Packet length: %d\n", length);

    /* Test start */
    if (HW_TEST_BT_TxOnlyTest_start(
          pattern,
          hopping,
          channel,
          type,
          length) == true) {
        printf("Tx test start...\n");
    }
    else{
        printf("Try to start Tx test, failed\n");
        HW_TEST_BT_deinit();
        printf("BT device power off\n");
        return;
    }

    /* Wait user type "exit" */
    wait_user_exit();

    /* Test end */
    HW_TEST_BT_TxOnlyTest_end();
    printf("Tx test complete\n");
    /* BT power off */
    HW_TEST_BT_deinit();
    printf("BT device power off\n");

    return;
}

static struct option nsrx_options[] = {
    { "help",	0, 0, 'h' },
    { "pattern", 1, 0, 'p' },
    { "channel", 1, 0, 'c' },
    { "type", 1, 0, 't' },
    { "addr", 1, 0, 'a' },
    { 0, 0, 0, 0 }
};

static const char *nsrx_help =
    "Usage:\n"
    "\tnsrx [--pattern] 0x01:\tRx 0000 pattern\n"
    "\t                 0x02:\tRx 1111 pattern\n"
    "\t                 0x03:\tRx 1010 pattern\n"
    "\t                 0x04:\tRx pseudo random bit sequence\n"
    "\t                 0x09:\tRx 11110000 pattern\n"
    "\n"
    "\t     [--channel] Integer (0~78 channel for single frequency)\n"
    "\n"
    "\t     [--type]    -- BR packet --\n"
    "\t                 0x03:\tDM1\n"
    "\t                 0x04:\tDH1\n"
    "\t                 0x0A:\tDM3\n"
    "\t                 0x0B:\tDH3\n"
    "\t                 0x0E:\tDM5\n"
    "\t                 0x0F:\tDH5\n"
    "\t                 -- EDR packet --\n"
    "\t                 0x24:\t2-DH1\n"
    "\t                 0x28:\t3-DH1\n"
    "\t                 0x2A:\t2-DH3\n"
    "\t                 0x2B:\t3-DH3\n"
    "\t                 0x2E:\t2-DH5\n"
    "\t                 0x2F:\t3-DH5\n"
    "\n"
    "\t     [--addr]    Hex XXXXXXXX (UAP+LAP 4 bytes)\n"
    "\t                 if set 0, use default value 0x00A5F0C3\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt nsrx --pattern 0x02 --channel 5 --type 0x2A --addr 88C0FFEE\n"
    "\n\n"
    "Notice:\n"
    "\tThis command is to start Non-Signal-Rx test on a specified channel\n"
    "\tTo end test, please type in \"exit\", then the PER & BER during test are returned\n"
    "\n";

static void cmd_non_signal_rx(int argc, char **argv)
{
    int opt;
    unsigned char opt_bits = 0;
    unsigned char pattern = 0, type = 0;
    int channel = 0;
    unsigned int addr = 0; /* UAP+LAP 4 bytes */
    unsigned int pkt_count;
    unsigned int byte_count;
    float PER, BER;

    for_each_opt(opt, nsrx_options, "+p:c:t:a:h") {
        switch (opt) {
        case 'p':
            pattern = (unsigned char)strtoul(optarg, NULL, 16);
            opt_bits |= BIT(0);
            break;

        case 'c':
            channel = atoi(optarg);
            if (channel < 0 || channel > 78) {
                printf("Invalid command option parameter!\n");
                printf("%s", nsrx_help);
                return;
            }
            opt_bits |= BIT(1);
            break;

        case 't':
            type = (unsigned char)strtoul(optarg, NULL, 16);
            opt_bits |= BIT(2);
            break;

        case 'a':
            if (0 != strcmp(optarg, "0")) {
                if (strlen(optarg) != 8) {
                    printf("Invalid command option parameter!\n");
                    printf("%s", nsrx_help);
                    return;
                }
                else {
                    if (check_hex_str(optarg)) {
                        printf("Invalid command option parameter!\n");
                        printf("%s", nsrx_help);
                        return;
                    }
                    else {
                        addr = (unsigned int)strtoul(optarg, NULL, 16);
                    }
                }
            } else {
                addr = 0x00A5F0C3;
            }
            opt_bits |= BIT(3);
            break;

        case 'h':
        default:
            printf("%s", nsrx_help);
            return;
        }
    }

    if (~opt_bits & BITS(0, 3)) {
        printf("Incomplete command options!\n");
        printf("%s", nsrx_help);
        return;
    }

    /* BT power on & Initialize */
    if (HW_TEST_BT_init()) {
        printf("BT device power on success\n");
    }
    else {
        printf("BT device power on failed\n");
        return;
    }

    printf("Rx pattern: 0x%02x\n", pattern);
    printf("Rx channel: %d\n", channel);
    printf("Packet type: 0x%02x\n", type);
    printf("Tester address: %08x\n", addr);

    /* Test start */
    if (HW_TEST_BT_NonSignalRx_start(
          pattern,
          channel,
          type,
          addr) == true) {
        printf("Non-Signal-Rx test start...\n");
    }
    else {
        printf("Try to start Non-Signal-Rx test, failed\n");
        HW_TEST_BT_deinit();
        printf("BT device power off\n");
        return;
    }

    /* Wait user type "exit" */
    wait_user_exit();

    /* Test end */
    if (HW_TEST_BT_NonSignalRx_end(
          &pkt_count,
          &PER,
          &byte_count,
          &BER) == true) {
        printf("Non-Signal-Rx test complete\n");
        printf("Total received packet: %d\n", pkt_count);
        printf("Packet Error Rate: %f%%\n", PER);
        printf("Total received payload byte: %d\n", byte_count);
        printf("Bit Error Rate: %f%%\n", BER);
    }
    else {
        printf("Try to end Non-Signal-Rx test, failed\n");
    }

    /* BT power off */
    HW_TEST_BT_deinit();
    printf("BT device power off\n");
    return;
}

static struct option testmode_options[] = {
    { "help",	0, 0, 'h' },
    { "power", 1, 0, 'p' },
    { 0, 0, 0, 0 }
};

static const char *testmode_help =
    "Usage:\n"
    "\ttestmode [--power] integer (range: 0~7)\n"
    "\t                   if not set, use default value 7\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt testmode --power 6\n"
    "\n\n"
    "Notice:\n"
    "\tThis command is to enable BT device under test mode\n"
    "\tTo exit test mode, please type in \"exit\"\n"
    "\n";

static void cmd_test_mode(int argc, char **argv)
{
    int opt;
    int power = 7;

    for_each_opt(opt, testmode_options, "+p:h") {
        switch (opt) {
        case 'p':
            power = atoi(optarg);
            if (power < 0 || power > 7) {
                printf("Invalid command option parameter!\n");
                printf("%s", testmode_help);
                return;
            }
            break;

        case 'h':
        default:
            printf("%s", testmode_help);
            return;
        }
    }

    /* BT power on & Initialize */
    if (HW_TEST_BT_init()) {
        printf("BT device power on success\n");
    }
    else {
        printf("BT device power on failed\n");
        return;
    }

    /* Test start */
    if (HW_TEST_BT_TestMode_enter(power) == true) {
        printf("Test mode entered, you can start to test...\n");
    }
    else {
        printf("Enable BT device under test mode failed\n");
        HW_TEST_BT_deinit();
        printf("BT device power off\n");
        return;
    }

    /* Wait user type "exit" */
    wait_user_exit();

    /* Test end */
    HW_TEST_BT_TestMode_exit();
    printf("Test mode exit\n");
    /* BT power off */
    HW_TEST_BT_deinit();
    printf("BT device power off\n");

    return;
}

static struct option inquiry_options[] = {
    { "help",	0, 0, 'h' },
    { 0, 0, 0, 0 }
};

static const char *inquiry_help =
    "Usage:\n"
    "\tinquiry [no command option]\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt inquiry\n"
    "\n";

static void cmd_inquiry(int argc, char **argv)
{
    int opt;

    for_each_opt(opt, inquiry_options, "+h") {
        switch (opt) {
        case 'h':
        default:
            printf("%s", inquiry_help);
            return;
        }
    }

    /* BT power on & initialize */
    if (HW_TEST_BT_init()) {
        printf("BT device power on success\n");
    }
    else {
        printf("BT device power on failed\n");
        return;
    }

    if (HW_TEST_BT_Inquiry(print_info) == true) {
        printf("Inquiry remote devices...\n");
    }
    else {
        printf("Start inquiry procedure failed\n");
        HW_TEST_BT_deinit();
        printf("BT device power off\n");
        return;
    }

    /* Loop in waiting for inquiry complete */
    g_inquiry_complete = false;
    do {
        sleep(7); /* Since the inquiry length is 6.4s set in driver */
    } while (!g_inquiry_complete);


    /* BT power off */
    HW_TEST_BT_deinit();
    printf("BT device power off\n");

    return;
}

static struct option bletx_options[] = {
    { "help",	0, 0, 'h' },
    { "pattern", 1, 0, 'p' },
    { "channel", 1, 0, 'c' },
    { 0, 0, 0, 0 }
};

static const char *bletx_help =
    "Usage:\n"
    "\tbletx [--pattern] 0x00:\tTx pseudo random bit sequence 9\n"
    "\t                  0x01:\tTx 11110000 pattern\n"
    "\t                  0x02:\tTx 10101010 pattern\n"
    "\n"
    "\t      [--channel] Integer (0~39 channel for frequency range 2402MHz~2480MHz)\n"
    "\t                  channel = (frequency-2402)/2\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt bletx --pattern 0x00 --channel 20\n"
    "\n\n"
    "Notice:\n"
    "\tThis command is to start LE Tx test, LE Controller will send out the specified Tx packet continuously\n"
    "\tTo end test, please type in \"exit\"\n"
    "\n";

static void cmd_ble_tx(int argc, char **argv)
{
    int opt;
    unsigned char opt_bits = 0;
    unsigned char pattern = 0;
    int channel = 0;

    for_each_opt(opt, bletx_options, "+p:c:h") {
        switch (opt) {
        case 'p':
            pattern = (unsigned char)strtoul(optarg, NULL, 16);
            opt_bits |= BIT(0);
            break;

        case 'c':
            channel = atoi(optarg);
            if (channel < 0 || channel > 39) {
                printf("Invalid command option parameter!\n");
                printf("%s", bletx_help);
                return;
            }
            opt_bits |= BIT(1);
            break;

        case 'h':
        default:
            printf("%s", bletx_help);
            return;
        }
    }

    if (~opt_bits & BITS(0, 1)) {
        printf("Incomplete command options!\n");
        printf("%s", bletx_help);
        return;
    }

    /* BT power on & Initialize */
    if (HW_TEST_BT_init()) {
        printf("BT device power on success\n");
    }
    else {
        printf("BT device power on failed\n");
        return;
    }

    printf("Tx pattern: 0x%02x\n", pattern);
    printf("Tx channel: %d\n", channel);

    /* Test start */
    if (HW_TEST_BT_LE_Tx_start(pattern, channel) == true) {
        printf("LE Tx test start...\n");
    }
    else{
        printf("Try to start LE Tx test, failed\n");
        HW_TEST_BT_deinit();
        printf("BT device power off\n");
        return;
    }
    /* Wait user type "exit" */
    wait_user_exit();

    /* Test end */
    if (HW_TEST_BT_LE_Tx_end() == true) {
        printf("LE Tx test complete\n");
    }
    else {
        printf("Try to end LE Tx test, failed\n");
    }

    /* BT power off */
    HW_TEST_BT_deinit();
    printf("BT device power off\n");

    return;
}

static struct option blerx_options[] = {
    { "help",	0, 0, 'h' },
    { "channel", 1, 0, 'c' },
    { 0, 0, 0, 0 }
};

static const char *blerx_help =
    "Usage:\n"
    "\tblerx [--channel] Integer (0~39 channel for frequency range 2402MHz~2480MHz)\n"
    "\t                  channel = (frequency-2402)/2\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt blerx --channel 20\n"
    "\n\n"
    "Notice:\n"
    "\tThis command is to start LE Rx test on a specified channel\n"
    "\tTo end test, please type in \"exit\", then the total received packet count during test are returned\n"
    "\n";

static void cmd_ble_rx(int argc, char **argv)
{
    int opt;
    unsigned char opt_bits = 0;
    int channel = 0;
    unsigned short pkt_count;

    for_each_opt(opt, blerx_options, "+c:h") {
        switch (opt) {
        case 'c':
            channel = atoi(optarg);
            if (channel < 0 || channel > 39) {
                printf("Invalid command option parameter!\n");
                printf("%s", blerx_help);
                return;
            }
            opt_bits |= BIT(0);
            break;

        case 'h':
        default:
            printf("%s", blerx_help);
            return;
        }
    }

    if (~opt_bits & BITS(0, 0)) {
        printf("Incomplete command options!\n");
        printf("%s", blerx_help);
        return;
    }

    /* BT power on & Initialize */
    if (HW_TEST_BT_init()) {
        printf("BT device power on success\n");
    }
    else {
        printf("BT device power on failed\n");
        return;
    }

    printf("Rx pattern: PRBS(pseudo random bit sequence)\n");
    printf("Rx channel: %d\n", channel);

    /* Test start */
    if (HW_TEST_BT_LE_Rx_start(channel) == true) {
        printf("LE Rx test start...\n");
    }
    else {
        printf("Try to start LE Rx test, failed\n");
        HW_TEST_BT_deinit();
        printf("BT device power off\n");
        return;
    }

    /* Wait user type "exit" */
    wait_user_exit();

    /* Test end */
    if (HW_TEST_BT_LE_Rx_end(&pkt_count) == true) {
        printf("LE Rx test complete\n");
        printf("Total received packet: %d\n", pkt_count);
    }
    else {
        printf("Try to end LE Rx test, failed\n");
    }

    /* BT power off */
    HW_TEST_BT_deinit();
    printf("BT device power off\n");
    return;
}


static struct option bleetx_options[] = {
    { "help", 0, 0, 'h' },
    { "channel", 1, 0, 'c' },
    { "length", 1, 0, 'l' },
    { "pattern", 1, 0, 'p' },
    { "phy", 1, 0, 'r' },
    { 0, 0, 0, 0 }
};

static const char *bleetx_help =
    "Notice: Only for Chipset which support BT Core Spec 5.0\n"
    "\n"
    "Usage:\n"
    "\tbleetx [--channel] Integer (0~39 channel for frequency range 2402MHz~2480MHz)\n"
    "\t                   channel = (frequency-2402)/2\n"
    "\n"
    "\t       [--length]  Length in bytes of payload data in each packet, range 0x00~0xFF\n"
    "\n"
    "\t       [--pattern] 0x00:\tPRBS9 sequence '11111111100000111101...'\n"
    "\t                   0x01:\tRepeated '11110000' sequence\n"
    "\t                   0x02:\tRepeated '10101010' sequence\n"
    "\t                   0x03:\tPRBS15 sequence\n"
    "\t                   0x04:\tRepeated '11111111' sequence\n"
    "\t                   0x05:\tRepeated '00000000' sequence\n"
    "\t                   0x06:\tRepeated '00001111' sequence\n"
    "\t                   0x07:\tRepeated '01010101' sequence\n"
    "\t                   0x08~0xFF:\tReserved for future use\n"
    "\n"
    "\t       [--phy]     0x00:\tReserved for future use\n"
    "\t                   0x01:\tTrasmitter set to use the LE 1M PHY\n"
    "\t                   0x02:\tTrasmitter set to use the LE 2M PHY\n"
    "\t                   0x03:\tTrasmitter set to use the LE Coded PHY with S=8 data coding\n"
    "\t                   0x04:\tTrasmitter set to use the LE Coded PHY with S=2 data coding\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt bleetx --channel 39 --length 2 --pattern 0x01 --phy 0x01\n"
    "\n\n"
    "Notice:\n"
    "\tThis command is to start LE Enhanced Tx test, LE Controller will send out the specified Tx packet continuously\n"
    "\tTo end test, please type in \"exit\"\n"
    "\n";

static void cmd_ble_enhanced_tx(int argc, char **argv)
{
    int opt;
    unsigned char opt_bits = 0;
    int channel = 0;
    unsigned int length = 0;
    unsigned char pattern = 0, phy_rate = 0;

    for_each_opt(opt, bleetx_options, "+c:l:p:r:h") {
        switch (opt) {
        case 'c':
            channel = atoi(optarg);
            opt_bits |= BIT(0);
            if (channel < 0 || channel > 39) {
                printf("Invalid command option parameter!\n");
                printf("%s", bleetx_help);
                return;
            }
            break;
        case 'l':
            length = (unsigned int)strtoul(optarg, NULL, 10);
            opt_bits |= BIT(1);
            break;
        case 'p':
            pattern = (unsigned char)strtoul(optarg, NULL, 16);
            opt_bits |= BIT(2);
            break;
        case 'r':
            phy_rate = (unsigned char)strtoul(optarg, NULL, 16);
            opt_bits |= BIT(3);
            break;
        case 'h':
        default:
            printf("%s", bleetx_help);
            return;
        }
    }

    if (~opt_bits & BITS(0, 3)) {
        printf("Incomplete command options!\n");
        printf("%s", bleetx_help);
        return;
    }

    /* BT power on & Initialize */
    if (HW_TEST_BT_init()) {
        printf("BT device power on success\n");
    }
    else {
        printf("BT device power on failed\n");
        return;
    }

    printf("Tx channel: %d\n", channel);
    printf("Tx payload length: %d\n", length);
    printf("Tx pattern: 0x%02x\n", pattern);
    printf("Tx PHY rate: 0x%02x\n", phy_rate);

    /* Test start */
    if (HW_TEST_BT_LE_Enhanced_Tx_start(channel, length, pattern, phy_rate) == true) {
        printf("LE Enhanced Tx test start...\n");
    }
    else {
        printf("Try to start LE Enhanced Tx test, failed\n");
        HW_TEST_BT_deinit();
        printf("BT device power off\n");
        return;
    }

    /* Wait user type "exit" */
    wait_user_exit();

    /* Test end */
    if (HW_TEST_BT_LE_Tx_end() == true) {
        printf("LE Enhanced Tx test complete\n");
    }
    else {
        printf("Try to end LE Enhanced Tx test, failed\n");
    }

    /* BT power off */
    HW_TEST_BT_deinit();
    printf("BT device power off\n");

    return;
}

static struct option bleerx_options[] = {
    { "help", 0, 0, 'h' },
    { "channel", 1, 0, 'c' },
    { "phy", 1, 0, 'r' },
    { "index", 1, 0, 'i' },
    { 0, 0, 0, 0 }
};

static const char *bleerx_help =
    "Notice: Only for Chipset which support BT Core Spec 5.0\n"
    "\n"
    "Usage:\n"
    "\tbleerx [--channel] Integer (0~39 channel for frequency range 2402MHz~2480MHz)\n"
    "\t                   channel = (frequency-2402)/2\n"
    "\n"
    "\t       [--phy]     0x00: Reserved for future use\n"
    "\t                   0x01: Receiver set to use the LE 1Mbps PHY\n"
    "\t                   0x02: Receiver set to use the LE 2Mbps PHY\n"
    "\t                   0x03: Receiver set to use the LE Coded PHY\n"
    "\t                   0x04~0xFF: Reserved for future use\n"
    "\n"
    "\t       [--index]   0x00: Standard mudulation index\n"
    "\t                   0x01: Stable mudulation index\n"
    "\t                   0x02~0xFF: Reserved for future use\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt bleerx --channel 20 --phy 0x02 --index 0x00\n"
    "\n\n"
    "Notice:\n"
    "\tThis command is to start LE Enhanced Rx test on a specified channel\n"
    "\tTo end test, please type in \"exit\", then the total received packet count during test are returned\n"
    "\n";

static void cmd_ble_enhanced_rx(int argc, char **argv)
{
    int opt;
    unsigned char opt_bits = 0;
    int channel = 0;
    unsigned char phy_rate = 0, index = 0;
    unsigned short pkt_count;

    for_each_opt(opt, bleerx_options, "+c:r:i:h") {
        switch (opt) {
        case 'c':
            channel = atoi(optarg);
            if (channel < 0 || channel > 39) {
                printf("Invalid command option parameter!\n");
                printf("%s", bleerx_help);
                return;
            }
            opt_bits |= BIT(0);
            break;
        case 'r':
            phy_rate = (unsigned char)strtoul(optarg, NULL, 16);
            opt_bits |= BIT(1);
            break;
        case 'i' :
            index = (unsigned char)strtoul(optarg, NULL, 16);
            opt_bits |= BIT(2);
            break;
        case 'h':
        default:
            printf("%s", bleerx_help);
            return;
        }
    }

    if (~opt_bits & BITS(0, 2)) {
        printf("Incomplete command options!\n");
        printf("%s", bleerx_help);
        return;
    }

    /* BT power on & Initialize */
    if (HW_TEST_BT_init()) {
        printf("BT device power on success\n");
    }
    else {
        printf("BT device power on failed\n");
        return;
    }

    printf("Rx channel: %d\n", channel);
    printf("Rx PHY rate: 0x%02x\n", phy_rate);
    printf("Rx modulation index: 0x%02x\n", index);

    /* Test start */
    if (HW_TEST_BT_LE_Enhanced_Rx_start(channel, phy_rate, index) == true) {
        printf("LE Enhanced Rx test start...\n");
    }
    else {
        printf("Try to start LE Enhanced Rx test, failed\n");
        HW_TEST_BT_deinit();
        printf("BT device power off\n");
        return;
    }

    /* Wait user type "exit" */
    wait_user_exit();

    /* Test end */
    if (HW_TEST_BT_LE_Rx_end(&pkt_count) == true) {
        printf("LE Enhanced Rx test complete\n");
        printf("Total received packet: %d\n", pkt_count);
    }
    else {
        printf("Try to end LE Enhanced Rx test, failed\n");
    }

    /* BT power off */
    HW_TEST_BT_deinit();
    printf("BT device power off\n");
    return;
}


static struct option relayer_options[] = {
    { "help", 0, 0, 'h' },
    { "port", 1, 0, 'p' },
    { "speed", 1, 0, 's' },
    { 0, 0, 0, 0 }
};

static const char *relayer_help =
    "Usage:\n"
    "\trelayer [--port]  Interface to communicate with PC\n"
    "\t                  0:\tUART1\n"
    "\t                  1:\tUART2\n"
    "\t                  2:\tUART3\n"
    "\t                  3:\tUART4\n"
    "\t                  4:\tUSB\n"
    "\n"
    "\t        [--speed] Speed of UART serial port, if not set, use default value 115200\n"
    "\t                  Not required for USB\n"
    "\t                  9600\n"
    "\t                  19200\n"
    "\t                  38400\n"
    "\t                  57600\n"
    "\t                  115200\n"
    "\t                  230400\n"
    "\t                  460800\n"
    "\t                  500000\n"
    "\t                  576000\n"
    "\t                  921600\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt relayer --port 0 --speed 115200\n"
    "\t\tautobt relayer --port 4\n"
    "\n\n"
    "Notice:\n"
    "\tThis command is to start BT relayer function for Controller Certification\n"
    "\tBT relayer mode is a bridge for data transmission between PC simulation tool and Bluetooth Controller."
    " If it is configured to be running via UART serial port (port 0~3), H/W rework may be needed;"
    " if it is configured to be running via USB interface (port 4), be noticed to set \"vendor.usb.config\""
    " property to \"acm_third\" first to open a VCOM.\n"
    "\tTo end test, please type in \"exit\"\n"
    "\n";

extern BOOL RELAYER_start(int serial_port, int serial_speed);
extern void RELAYER_exit(void);

static void cmd_relayer(int argc, char **argv)
{
    int opt;
    int port = -1, speed = 115200;

    for_each_opt(opt, relayer_options, "+p:s:h") {
        switch (opt) {
        case 'p':
            port = atoi(optarg);
            break;

        case 's':
            speed = atoi(optarg);
            break;

        case 'h':
        default:
            printf("%s", relayer_help);
            return;
        }
    }

    if (port < 0 || port > 4) {
        printf("Invalid command option parameter!\n");
        printf("%s", relayer_help);
        return;
    }

    printf("Serial port number: %d\n", port);
    printf("Serial port speed: %d\n", speed);

    /* Test start */
    if (RELAYER_start(port, speed) == true) {
        printf("BT relayer mode start, you can run test script from PC simulation tool now...\n");
    }
    else {
        printf("Start BT relayer mode failed\n");
        return;
    }

    /* Wait user type "exit" */
    wait_user_exit();

    /* Test end */
    RELAYER_exit();
    printf("BT relayer mode exit\n");

    return;
}

void pdn_loop_stop(char *info)
{
    (void) info;
    printf("---pdn_loop_stop---\n");
    g_pdn_complete = true;
}

static struct option pdn_loop_options[] = {
    { "clock", 1, 0, 'c' },
    { "length", 1, 0, 'l' },
    { "interval", 1, 0, 'i' },
    { 0, 0, 0, 0 }
};

static void cmd_pdn_loop_start(int argc, char **argv)
{
    int opt;
    int clk_rate = 0x00;
    int pdn_length = 0x10;
    int pdn_interval_ms = 500;

    for_each_opt(opt, pdn_loop_options, "+c:l:i") {
        switch (opt) {
        case 'c':
            clk_rate = atoi(optarg);
            break;
        case 'l':
            pdn_length = atoi(optarg);
            break;
        case 'i' :
            pdn_interval_ms = atoi(optarg);
            break;
        }
    }
    pdn_arg_struct args = {pdn_loop_stop, (UINT8)pdn_length, (UINT16)pdn_interval_ms};
    printf("pdn_length[%d], pdn_interval_ms[%d] \n", pdn_length, pdn_interval_ms);

    /* BT power on & initialize */
    if (HW_TEST_BT_init()) {
        printf("BT device power on success\n");
    }
    else {
        printf("BT device power on failed\n");
        return;
    }
    if(clk_rate > 0) {
        HW_TEST_BT_PDN_SET_CLOCK(clk_rate);
    }
    printf("cmd_pdn_loop_start...\n");
    pthread_create(&pdnLoopThread, NULL, HW_TEST_BT_PDN_LOOP, (void*)&args);
    /* Loop in waiting for inquiry complete */
    g_pdn_complete = false;
    do {
        sleep(5);
    } while (!g_pdn_complete);
    /* BT power off */
    HW_TEST_BT_deinit();
    printf("BT device power off\n");

    return;
}

static struct option pdn_ut_options[] = {
    { "clock", 1, 0, 'c' },
    { "length", 1, 0, 'l' },
    { 0, 0, 0, 0 }
};

static void cmd_pdn_ut(int argc, char **argv)
{
    int opt;
    int clk_rate = 0x00;
    int pdn_length = 0x10;

    for_each_opt(opt, pdn_ut_options, "+c:l") {
        switch (opt) {
        case 'c':
            clk_rate = atoi(optarg);
            break;
        case 'l':
            pdn_length = atoi(optarg);
            break;
        }
    }

    if (HW_TEST_BT_init()) {
        printf("BT device power on success\n");
    } else {
        printf("BT device power on failed\n");
        printf("Fail!\n");
        return;
    }
    if(clk_rate > 0) {
        HW_TEST_BT_PDN_SET_CLOCK(clk_rate);
    }
    if (HW_TEST_BT_PDN_UT(pdn_length) == true) {
        printf("Pass\n");
    } else {
        printf("Fail\n");
    }
    HW_TEST_BT_deinit();
    printf("BT device power off\n");
    return;
}

static struct {
    char *cmd;
    void (*func)(int argc, char **argv);
    char *doc;
} command[] = {
    { "tx",        cmd_tx_test,        "Tx only test"                      },
    { "nsrx",      cmd_non_signal_rx,  "Non-Signal-Rx test"                },
    { "testmode",  cmd_test_mode,      "Enable BT device under test mode"  },
    { "inquiry",   cmd_inquiry,        "Inquiry remote devices"            },
    { "bletx",     cmd_ble_tx,         "LE Tx test"                        },
    { "blerx",     cmd_ble_rx,         "LE Rx test"                        },
    { "bleetx",    cmd_ble_enhanced_tx,"(BT 5.0) LE Enhanced Tx test"      },
    { "bleerx",    cmd_ble_enhanced_rx,"(BT 5.0) LE Enhanced Rx test"      },
    { "relayer",   cmd_relayer,        "BT relayer mode"                   },
    { "pdnut",     cmd_pdn_ut,         "PDN UT test"                       },
    { "pdnstart",  cmd_pdn_loop_start, "PDN LOOP test start"               },
    { NULL, NULL, 0 }
};

static void usage(void)
{
    int i;

    printf("autobt test tool - ver %s\n", VERSION);
    printf("Usage:\n"
        "\tautobt <command> [command options] [command parameters]\n");
    printf("Commands:\n");
    for (i = 0; command[i].cmd; i++) {
        printf("\t%-8s\t%s\n", command[i].cmd, command[i].doc);
    }
    printf("\n"
        "For more information on the usage of each command use:\n"
        "\tautobt <command> --help\n\n\n");
}

static struct option main_options[] = {
    { "help",	0, 0, 'h' },
    { 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
    int opt, i;

    while ((opt=getopt_long(argc, argv, "+h", main_options, NULL)) != -1) {
        switch (opt) {
        case 'h':
        default:
            usage();
            exit(0);
        }
    }

    argc -= optind;
    argv += optind;
    optind = 0;

    if (argc < 1) {
        usage();
        exit(0);
    }

    for (i = 0; command[i].cmd; i++) {
        if (strcmp(command[i].cmd, argv[0]))
           continue;
        command[i].func(argc, argv);
        break;
    }

    return 0;
}
