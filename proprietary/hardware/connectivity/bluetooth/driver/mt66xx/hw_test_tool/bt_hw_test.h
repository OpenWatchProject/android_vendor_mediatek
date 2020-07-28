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

#ifndef __BT_HW_TEST_H__
#define __BT_HW_TEST_H__

#include <stdbool.h>
#include <cutils/properties.h>
#ifndef FALSE
#define FALSE     0
#endif
#ifndef TRUE
#define TRUE      1
#endif
#ifndef BOOL
#define BOOL      bool
#endif

typedef unsigned char UCHAR;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;

#define UNUSED_ATTR __attribute__((unused))

/* LOG_TAG must be defined before log.h */
#define LOG_TAG         "BT_HW_TEST "
#include <log/log.h>

#define BT_HW_TEST_DEBUG     1
#define ERR(f, ...)     ALOGE("%s: " f, __FUNCTION__, ##__VA_ARGS__)
#define WAN(f, ...)     ALOGW("%s: " f, __FUNCTION__, ##__VA_ARGS__)
#if BT_HW_TEST_DEBUG
#define DBG(f, ...)     ALOGD("%s: " f, __FUNCTION__, ##__VA_ARGS__)
#define TRC(f)          ALOGW("%s #%d", __FUNCTION__, __LINE__)
#else
#define DBG(...)        ((void)0)
#define TRC(f)          ((void)0)
#endif


#ifndef BIT
#define BIT(n)                          ((UINT8) 1 << (n))
#endif /* BIT */

#ifndef BITS
/* bits range: for example BITS(0,3) = 0x0F
 *   ==>  (BIT(m)-1)   = 0x00     ~(BIT(m)-1)   => 0xFF
 *   ==>  (BIT(n+1)-1) = 0x0F
 */

#define BITS(m, n)                       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))
#endif /* BIT */

/*
* Exported Function Declaration
*/
#ifdef __cplusplus
extern "C"
{
#endif
BOOL HW_TEST_BT_init(void);
void HW_TEST_BT_deinit(void);
BOOL HW_TEST_BT_reset(void);
BOOL HW_TEST_BT_PDN_UT(UINT8 pdn_length);
BOOL HW_TEST_BT_PDN_SET_CLOCK(UINT8 clk_rate);
void * HW_TEST_BT_PDN_LOOP(void *ptr);

BOOL HW_TEST_BT_TxOnlyTest_start(
  UINT8 tx_pattern,
  UINT8 hopping,
  int channel,
  UINT8 packet_type,
  UINT32 packet_len
);

BOOL HW_TEST_BT_TxOnlyTest_end(void);

BOOL HW_TEST_BT_NonSignalRx_start(
  UINT8 rx_pattern,
  int channel,
  UINT8 packet_type,
  UINT32 tester_addr
);

BOOL HW_TEST_BT_NonSignalRx_end(
  UINT32 *pu4RxPktCount,
  float  *pftPktErrRate,
  UINT32 *pu4RxByteCount,
  float  *pftBitErrRate
);

BOOL HW_TEST_BT_TestMode_enter(int power);
BOOL HW_TEST_BT_TestMode_exit(void);

typedef void (*FUNC_CB)(char *info);
typedef struct {
    FUNC_CB info_cb;
    UINT8 pdn_length;
    UINT16 pdn_interval_ms;
}pdn_arg_struct;

BOOL HW_TEST_BT_Inquiry(FUNC_CB info_cb);

BOOL HW_TEST_BT_LE_Tx_start(UINT8 tx_pattern, int channel);
BOOL HW_TEST_BT_LE_Enhanced_Tx_start(int channel, UINT32 payload_len, UINT8 pattern, UINT8 phy_rate);
BOOL HW_TEST_BT_LE_Tx_end(void);

BOOL HW_TEST_BT_LE_Rx_start(int channel);
BOOL HW_TEST_BT_LE_Enhanced_Rx_start(int channel, UINT8 phy_rate, UINT8 index);
BOOL HW_TEST_BT_LE_Rx_end(UINT16 *pu2RxPktCount);


#ifdef __cplusplus
}
#endif

#endif
