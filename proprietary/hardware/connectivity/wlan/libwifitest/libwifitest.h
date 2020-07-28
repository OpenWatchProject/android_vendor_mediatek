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

/*
 * This is sample code for WLAN Test Mode Control
 */

#ifndef __LIBWIFITEST_H__
#define __LIBWIFITEST_H__

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/wireless.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <netdb.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <unistd.h>

/* basic definitions
 * -------------------------------------------------------------------------- */
#ifndef bool
#define bool int
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define PROC_NET_DEV "/proc/net/dev"

/* types
 * -------------------------------------------------------------------------- */
typedef enum {
	WIFI_TEST_PREAMBLE_LONG,
	WIFI_TEST_PREAMBLE_SHORT,
} WIFI_PreambleType_t;

/* WIFI_TEST_TxDataRate */
typedef enum _tagENUM_WIFI_TEST_TX_DATA_RATE {
    WIFI_TEST_RATE_AUTO          = -1,
    WIFI_TEST_RATE_CCK_1MBPS     = 0,
    WIFI_TEST_RATE_CCK_2MBPS     = 1,
    WIFI_TEST_RATE_CCK_5_5MBPS   = 2,
    WIFI_TEST_RATE_CCK_11MBPS    = 3,
    WIFI_TEST_RATE_OFDM_6MBPS    = 4,
    WIFI_TEST_RATE_OFDM_9MBPS    = 5,
    WIFI_TEST_RATE_OFDM_12MBPS   = 6,
    WIFI_TEST_RATE_OFDM_18MBPS   = 7,
    WIFI_TEST_RATE_OFDM_24MBPS   = 8,
    WIFI_TEST_RATE_OFDM_36MBPS   = 9,
    WIFI_TEST_RATE_OFDM_48MBPS   = 10,
    WIFI_TEST_RATE_OFDM_54MBPS   = 11,
} ENUM_WIFI_TEST_TX_DATA_RATE, *P_ENUM_WIFI_TEST_TX_DATA_RATE;


/* Supported MCS rates */
typedef enum _tagENUM_WIFI_TEST_MCS_RATE {
    WIFI_TEST_MCS_RATE_0 = 0,
    WIFI_TEST_MCS_RATE_1 = 1,
    WIFI_TEST_MCS_RATE_2 = 2,
    WIFI_TEST_MCS_RATE_3 = 3,
    WIFI_TEST_MCS_RATE_4 = 4,
    WIFI_TEST_MCS_RATE_5 = 5,
    WIFI_TEST_MCS_RATE_6 = 6,
    WIFI_TEST_MCS_RATE_7 = 7,
    WIFI_TEST_MCS_RATE_8 = 8,
    WIFI_TEST_MCS_RATE_9 = 9,
    WIFI_TEST_MCS_RATE_10 = 10,
    WIFI_TEST_MCS_RATE_11 = 11,
    WIFI_TEST_MCS_RATE_12 = 12,
    WIFI_TEST_MCS_RATE_13 = 13,
    WIFI_TEST_MCS_RATE_14 = 14,
    WIFI_TEST_MCS_RATE_15 = 15,
    WIFI_TEST_MCS_RATE_32= 32
} ENUM_WIFI_TEST_MCS_RATE, *P_ENUM_WIFI_TEST_MCS_RATE;

/* Preamble Type */
typedef enum _tagENUM_WIFI_TEST_PREAMBLE_TYPE {
    WIFI_TEST_PREAMBLE_TYPE_MIXED_MODE = 0,
    WIFI_TEST_PREAMBLE_TYPE_GREENFIELD = 1
} ENUM_WIFI_TEST_PREAMBLE_TYPE, *P_ENUM_WIFI_TEST_PREAMBLE_TYPE;

/* Guard Interval Type */
typedef enum _tagENUM_WIFI_TEST_GI_TYPE {
    WIFI_TEST_GI_TYPE_NORMAL_GI = 0,
    WIFI_TEST_GI_TYPE_SHORT_GI  = 1
} ENUM_WIFI_TEST_GI_TYPE, *P_ENUM_WIFI_TEST_GI_TYPE;

/* Legacy/11n Bandwidth Type */
typedef enum _tagENUM_WIFI_BANDWIDTH {
    WIFI_TEST_BW_20MHZ  = 0,
    WIFI_TEST_BW_40MHZ  = 1,
    WIFI_TEST_BW_U20MHZ = 2,
    WIFI_TEST_BW_D20MHZ = 3,
    WIFI_TEST_BW_80MHZ  = 4,
    WIFI_TEST_BW_160MHZ = 5,
    WIFI_TEST_BW_NUM    = 6
} ENUM_WIFI_BANDWIDTH, *P_ENUM_WIFI_BANDWIDTH;

/* Channel Bandwidth Type */
typedef enum _tagENUM_WIFI_CHANNEL_BANDWIDTH {
    WIFI_TEST_CH_BW_20MHZ = 0,
    WIFI_TEST_CH_BW_40MHZ,
    WIFI_TEST_CH_BW_80MHZ,
    WIFI_TEST_CH_BW_160MHZ,
    WIFI_TEST_CH_BW_NUM
}ENUM_WIFI_CHANNEL_BANDWIDTH, *P_ENUM_WIFI_CHANNEL_BANDWIDTH;

/* Primary channel offset (in unit of 20MHZ) */
typedef enum _tagENUM_WIFI_PRI_CHANNEL_SETTING {
    WIFI_TEST_PRI_CH_SETTING_0 = 0,
    WIFI_TEST_PRI_CH_SETTING_1,
    WIFI_TEST_PRI_CH_SETTING_2,
    WIFI_TEST_PRI_CH_SETTING_3,
    WIFI_TEST_PRI_CH_SETTING_4,
    WIFI_TEST_PRI_CH_SETTING_5,
    WIFI_TEST_PRI_CH_SETTING_6,
    WIFI_TEST_PRI_CH_SETTING_7,
    WIFI_TEST_PRI_CH_SETTING_NUM
}ENUM_WIFI_PRI_CHANNEL_SETTING, *P_ENUM_WIFI_PRI_CHANNEL_SETTING;

/* Wi-Fi TEST MODE */
typedef enum _tagENUM_WIFI_TEST_MODE {
    WIFI_TEST_MODE_BY_API_CONTROL   = 0,
    WIFI_TEST_MODE_CW_ONLY          = 1,
    WIFI_TEST_MODE_80211A_ONLY      = 2,
    WIFI_TEST_MODE_80211B_ONLY      = 3,
    WIFI_TEST_MODE_80211G_ONLY      = 4,
    WIFI_TEST_MODE_80211N_ONLY      = 5,
    WIFI_TEST_MODE_80211AC_ONLY     = 6,
    WIFI_TEST_MODE_NUM              = 7
} ENUM_WIFI_TEST_MODE, *P_ENUM_WIFI_TEST_MODE;

/* J MODE setting */
typedef enum _tagENUM_WIFI_J_MODE {
    WIFI_TEST_J_MODE_DISABLE            = 0,
    WIFI_TEST_J_MODE_5M_SINGLE_TONE     = 1,
    WIFI_TEST_J_MODE_10M_SINGLE_TINE    = 2,
    WIFI_TEST_J_MODE_NUM                = 3
} ENUM_WIFI_J_MODE, *P_ENUM_WIFI_J_MODE;

typedef enum _ENUM_RF_AT_FUNCID_T {

    RF_AT_FUNCID_VERSION = 0,
    RF_AT_FUNCID_COMMAND,       /* 01 */
    RF_AT_FUNCID_POWER,         /* 02 */
    RF_AT_FUNCID_RATE,          /* 03 */
    RF_AT_FUNCID_PREAMBLE,      /* 04 */
    RF_AT_FUNCID_ANTENNA,       /* 05 */
    RF_AT_FUNCID_PKTLEN,        /* 06 */
    RF_AT_FUNCID_PKTCNT,        /* 07 */
    RF_AT_FUNCID_PKTINTERVAL,   /* 08 */
    RF_AT_FUNCID_TEMP_COMPEN,   /* 09 */
    RF_AT_FUNCID_TXOPLIMIT,     /* 10 */
    RF_AT_FUNCID_ACKPOLICY,     /* 11 */
    RF_AT_FUNCID_PKTCONTENT,    /* 12 */
    RF_AT_FUNCID_RETRYLIMIT,    /* 13 */
    RF_AT_FUNCID_QUEUE,         /* 14 */
    RF_AT_FUNCID_BANDWIDTH,     /* 15 */
    RF_AT_FUNCID_GI,            /* 16 */
    RF_AT_FUNCID_STBC,          /* 17 */
    RF_AT_FUNCID_CHNL_FREQ,     /* 18 */
    RF_AT_FUNCID_RIFS,          /* 19 */
    RF_AT_FUNCID_TRSW_TYPE,     /* 20 */
    RF_AT_FUNCID_RF_SX_SHUTDOWN,/* 21 */
    RF_AT_FUNCID_PLL_SHUTDOWN,  /* 22 */
    RF_AT_FUNCID_SLOW_CLK_MODE, /* 23 */
    RF_AT_FUNCID_ADC_CLK_MODE,  /* 24 */
    RF_AT_FUNCID_MEASURE_MODE,  /* 25 */
    RF_AT_FUNCID_VOLT_COMPEN,   /* 26 */
    RF_AT_FUNCID_DPD_TX_GAIN,   /* 27 */
    RF_AT_FUNCID_DPD_MODE,      /* 28 */
    RF_AT_FUNCID_TSSI_MODE,     /* 29 */
    RF_AT_FUNCID_TX_GAIN_CODE,  /* 30 */
    RF_AT_FUNCID_TX_PWR_MODE,   /* 31 */

    /* Query command */
    RF_AT_FUNCID_TXED_COUNT = 32,
    RF_AT_FUNCID_TXOK_COUNT,    /* 33 */
    RF_AT_FUNCID_RXOK_COUNT,    /* 34 */
    RF_AT_FUNCID_RXERROR_COUNT, /* 35 */
    RF_AT_FUNCID_RESULT_INFO,   /* 36 */
    RF_AT_FUNCID_TRX_IQ_RESULT, /* 37 */
    RF_AT_FUNCID_TSSI_RESULT,   /* 38 */
    RF_AT_FUNCID_DPD_RESULT,    /* 39 */
    RF_AT_FUNCID_RXV_DUMP,      /* 40 */
    RF_AT_FUNCID_RX_PHY_STATIS, /* 41 */
    RF_AT_FUNCID_MEASURE_RESULT,/* 42 */
    RF_AT_FUNCID_TEMP_SENSOR,   /* 43 */
    RF_AT_FUNCID_VOLT_SENSOR,   /* 44 */
    RF_AT_FUNCID_READ_EFUSE,    /* 45 */
    RF_AT_FUNCID_RX_RSSI,       /* 46 */
    RF_AT_FUNCID_FW_INFO,       /* 47 */
    RF_AT_FUNCID_DRV_INFO,      /* 48 */
    RF_AT_FUNCID_PWR_DETECTOR,  /* 49 */
    RF_AT_FUNCID_WBRSSI_IBSSI,  /* 50 */
    RF_AT_FUNCID_TX1_POWER,     /* 51 */
    RF_AT_FUNCID_TX_PATH,       /* 52 */
    RF_AT_FUNCID_RX_PATH,       /* 53 */
    RF_AT_FUNCID_RX_RCPI,       /* 54 */

    /* Set command */
    RF_AT_FUNCID_SET_RX_DEF_ANT = 63,
    RF_AT_FUNCID_SET_DPD_RESULT = 64,
    RF_AT_FUNCID_SET_CW_MODE,
    RF_AT_FUNCID_SET_JAPAN_CH14_FILTER,
    RF_AT_FUNCID_WRITE_EFUSE,
    RF_AT_FUNCID_SET_MAC_DST_ADDRESS,
    RF_AT_FUNCID_SET_MAC_SRC_ADDRESS,
    RF_AT_FUNCID_SET_RXOK_MATCH_RULE,
    RF_AT_FUNCID_SET_CHANNEL_BANDWIDTH = 71,
    RF_AT_FUNCID_SET_DATA_BANDWIDTH = 72,
    RF_AT_FUNCID_SET_PRI_SETTING = 73,
    RF_AT_FUNCID_SET_TX_ENCODE_MODE = 74,
    RF_AT_FUNCID_SET_J_MODE_SETTING = 75
} ENUM_RF_AT_FUNCID_T;

typedef enum _ENUM_RF_AT_COMMAND_T {

    RF_AT_COMMAND_STOPTEST = 0,
    RF_AT_COMMAND_STARTTX,
    RF_AT_COMMAND_STARTRX,
    RF_AT_COMMAND_RESET,
    RF_AT_COMMAND_OUTPUT_POWER,     /* Payload */
    RF_AT_COMMAND_LO_LEAKAGE,       /* Local freq is renamed to Local leakage */
    RF_AT_COMMAND_CARRIER_SUPPR,    /* OFDM (LTF/STF), CCK (PI,PI/2) */
    RF_AT_COMMAND_TRX_IQ_CAL,
    RF_AT_COMMAND_TSSI_CAL,
    RF_AT_COMMAND_DPD_CAL,
    RF_AT_COMMAND_CW,
    RF_AT_COMMAND_NUM

} ENUM_RF_AT_COMMAND_T;

typedef enum _ENUM_RF_AT_PREAMBLE_T {

    RF_AT_PREAMBLE_NORMAL = 0,
    RF_AT_PREAMBLE_CCK_SHORT,
    RF_AT_PREAMBLE_11N_MM,
    RF_AT_PREAMBLE_11N_GF,
    RF_AT_PREAMBLE_11AC,
    RF_AT_PREAMBLE_NUM

} ENUM_RF_AT_PREAMBLE_T;

typedef enum _ENUM_CHNL_EXT_T {
    CHNL_EXT_SCN = 0,
    CHNL_EXT_SCA = 1,
    CHNL_EXT_RES = 2,
    CHNL_EXT_SCB = 3
} ENUM_CHNL_EXT_T, *P_ENUM_CHNL_EXT_T;

typedef enum _ENUM_CHNL_DEF_T {
    CHNL_DEFINE_PRIMARY_CHANNEL = 0,
    CHNL_DEFINE_CENTER_CHANNEL = 1,
} ENUM_CHNL_DEF_T, *P_ENUM_CHNL_DEF_T;


typedef struct _PARAM_MTK_WIFI_CHANNEL_T {
    ENUM_CHNL_DEF_T eDefine;
    uint32_t u4No;
    ENUM_WIFI_BANDWIDTH eBW; /* record bw is because primary channel needs BW to tranfrom to center channel*/
} PARAM_MTK_WIFI_CHANNEL_T, *P_PARAM_MTK_WIFI_CHANNEL_T;

#ifndef BIT
#define BIT(n)          ((uint32_t) 1 << (n))
#define BITS(m,n)       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))
#endif /* BIT */

/* RF Test Properties */
#define RF_AT_PARAM_RATE_MCS_MASK   BIT(31)
#define RF_AT_PARAM_RATE_MASK       BITS(0,7)
#define RF_AT_PARAM_RATE_1M         0
#define RF_AT_PARAM_RATE_2M         1
#define RF_AT_PARAM_RATE_5_5M       2
#define RF_AT_PARAM_RATE_11M        3
#define RF_AT_PARAM_RATE_6M         4
#define RF_AT_PARAM_RATE_9M         5
#define RF_AT_PARAM_RATE_12M        6
#define RF_AT_PARAM_RATE_18M        7
#define RF_AT_PARAM_RATE_24M        8
#define RF_AT_PARAM_RATE_36M        9
#define RF_AT_PARAM_RATE_48M        10
#define RF_AT_PARAM_RATE_54M        11

/* DBG mode log */
#define DBGLOG(fmt, arg ...)  \
    if(fgDebugMode) { \
        ALOGD("%s: " fmt, __FUNCTION__ , ##arg); \
    }

/* Wi-Fi test Library version */
#define LIB_WIFI_TEST_VER_MAJOR     0
#define LIB_WIFI_TEST_VER_MINOR     1

/* functions
 * -------------------------------------------------------------------------- */
bool WIFI_TEST_OpenDUT(void);
bool WIFI_TEST_CloseDUT(void);

/* Style #A - Control API */
bool WIFI_TEST_TxDataRate(int TxDataRate);
bool WIFI_TEST_SetPreamble(ENUM_RF_AT_PREAMBLE_T PreambleType);
bool WIFI_TEST_Channel(int ChannelNo);
bool WIFI_TEST_TxGain(int TxGain);
bool WIFI_TEST_TxBurstInterval(int SIFS);
bool WIFI_TEST_TxPayloadLength(int TxPayLength);
bool WIFI_TEST_TxBurstFrames(int Frames);
bool WIFI_TEST_TxDestAddress(unsigned char *addr);
bool WIFI_TEST_TxStart(void);
bool WIFI_TEST_TxStop(void);
bool WIFI_TEST_RxStart(void);
bool WIFI_TEST_RxStop(void);
bool WIFI_TEST_FRError(int *FError);
bool WIFI_TEST_FRGood(int *FRGood);
bool WIFI_TEST_RSSI(int *RSSI);
bool WIFI_TEST_RSSI_05_DBM(float *RSSI);
bool WIFI_TEST_IsRunning(void);
bool WIFI_TEST_IsUp(void);
bool WIFI_TEST_GI(ENUM_WIFI_TEST_GI_TYPE eGIType);
bool WIFI_TEST_TxDataRate11n(ENUM_WIFI_TEST_MCS_RATE eDataRate11n, ENUM_WIFI_TEST_PREAMBLE_TYPE ePreambleType, ENUM_WIFI_TEST_GI_TYPE eGIType);
bool WIFI_TEST_TxDataRate11ac(ENUM_WIFI_TEST_MCS_RATE eTxMcsRate,
    ENUM_WIFI_TEST_GI_TYPE eGIType);
bool WIFI_TEST_FrequencyAccuracy(int ChannelNo);
bool WIFI_TEST_FrequencyAccuracy_Stop();

/* Style #B - Control API */
bool WIFI_TEST_SetMode(ENUM_WIFI_TEST_MODE eMode);
bool WIFI_TEST_GetSupportedMode(uint32_t *pu4SupportedMode);
bool WIFI_TEST_SetBandwidth(ENUM_WIFI_BANDWIDTH eBandwidth);
bool WIFI_TEST_SetFrequency(uint32_t u4FreqInMHz, uint32_t u4OffsetInKHz);
bool WIFI_TEST_SetRate(uint32_t u4Rate);
bool WIFI_TEST_SetTXPower(uint32_t u4Gain);
bool WIFI_TEST_SetTX(bool fgEnable);
bool WIFI_TEST_SetRX(bool fgEnable, char *aucSrcAddr, char *aucDstAddr);
bool WIFI_TEST_ClearResult(void);
bool WIFI_TEST_GetResult(uint32_t *pu4GoodFrameCount, uint32_t *pu4BadFrameCount);

bool WIFI_TEST_CW_MODE(int mode);
bool WIFI_TEST_DPD_MODE(int mode);
bool WIFI_TEST_TEMP_COMPEN_MODE(int mode);
bool WIFI_TEST_TX_POWER_MODE(int mode);
bool WIFI_TEST_CW_MODE_START(void);

bool WIFI_TEST_init(void);
void WIFI_TEST_deinit(void);

bool WIFI_TEST_EFUSE_Read(unsigned int offset, unsigned int *val);
bool WIFI_TEST_EFUSE_Write(unsigned int offset, unsigned int val);
bool WIFI_TEST_MCR_Read(unsigned int addr, unsigned int *val);
bool WIFI_TEST_MCR_Write(unsigned int addr, unsigned int val);

bool WIFI_TEST_SetBandwidthV2(ENUM_WIFI_CHANNEL_BANDWIDTH eBandwidth);
bool WIFI_TEST_SetPriChannelSetting(uint8_t ucPrimaryChSetting);
bool WIFI_TEST_SetTxBandwidth(ENUM_WIFI_CHANNEL_BANDWIDTH eBandwidth);
bool WIFI_TEST_SetTxCodingMode(uint8_t ucLdpc);
bool WIFI_TEST_SetJMode(ENUM_WIFI_J_MODE eJModeSetting);

bool WIFI_TEST_SetRxDefaultAnt(uint8_t ucUseAuxAnt);

bool WIFI_TEST_TxCount(uint32_t *TxCount);
bool WIFI_TEST_TxGoodCount(uint32_t *TxGoodCount);
/* WIFI_TEST_PrimaryChannel : Set the channel (for primary channel )*/
bool WIFI_TEST_PrimaryChannel(int ChannelNo);
/* WIFI_TEST_Set_PrimaryChannel_Bandwidth : Set the bandwith , channel (for primary channel ) , SCO (secondary channel offset) */
bool WIFI_TEST_Set_PrimaryChannel_Bandwidth(int ChannelNo , ENUM_WIFI_BANDWIDTH eWifiRfBw,ENUM_CHNL_EXT_T eRfSco);

int WIFI_TEST_set(uint32_t u4FuncIndex, uint32_t u4FuncData, uint32_t *pu4FuncIndex, uint32_t *pu4FuncData);
int WIFI_TEST_get(uint32_t u4FuncIndex, uint32_t u4FuncData, uint32_t *pu4FuncIndex, uint32_t *pu4FuncData);

#endif /* __LIBWIFITEST_H__ */
