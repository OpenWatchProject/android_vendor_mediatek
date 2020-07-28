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

/* types
 * -------------------------------------------------------------------------- */
typedef enum {
	WIFI_TEST_PREAMBLE_LONG,
	WIFI_TEST_PREAMBLE_SHORT,
} WIFI_PreambleType_t;

/* WIFI_TEST_TxDataRate */
#define WIFI_TEST_RATE_AUTO         0
#define WIFI_TEST_RATE_1MBPS		1
#define WIFI_TEST_RATE_2MBPS		2
#define WIFI_TEST_RATE_5_5MBPS		3
#define WIFI_TEST_RATE_6MBPS		4
#define WIFI_TEST_RATE_9MBPS		5
#define WIFI_TEST_RATE_11MBPS		6
#define WIFI_TEST_RATE_12MBPS		7
#define WIFI_TEST_RATE_18MBPS		8
#define WIFI_TEST_RATE_24MBPS		9
#define WIFI_TEST_RATE_36MBPS		10
#define WIFI_TEST_RATE_48MBPS		11
#define WIFI_TEST_RATE_54MBPS		12

#define MAX_EEPROM_BUFFER_SIZE 1200
#define EFUSE_BYTES_PER_LINE 16
#define BUFFER_BIN_MODE 0x0
#define EFUSE_MODE 0x02

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
    RF_AT_FUNCID_COMMAND,
    RF_AT_FUNCID_POWER,
    RF_AT_FUNCID_RATE,
    RF_AT_FUNCID_PREAMBLE,
    RF_AT_FUNCID_ANTENNA,
    RF_AT_FUNCID_PKTLEN,
    RF_AT_FUNCID_PKTCNT,
    RF_AT_FUNCID_PKTINTERVAL,
    RF_AT_FUNCID_TEMP_COMPEN,
    RF_AT_FUNCID_TXOPLIMIT,
    RF_AT_FUNCID_ACKPOLICY,
    RF_AT_FUNCID_PKTCONTENT,
    RF_AT_FUNCID_RETRYLIMIT,
    RF_AT_FUNCID_QUEUE,
    RF_AT_FUNCID_BANDWIDTH,
    RF_AT_FUNCID_GI,
    RF_AT_FUNCID_STBC,
    RF_AT_FUNCID_CHNL_FREQ,
    RF_AT_FUNCID_RIFS,
    RF_AT_FUNCID_TRSW_TYPE,
    RF_AT_FUNCID_RF_SX_SHUTDOWN,
    RF_AT_FUNCID_PLL_SHUTDOWN,
    RF_AT_FUNCID_SLOW_CLK_MODE,
    RF_AT_FUNCID_ADC_CLK_MODE,
    RF_AT_FUNCID_MEASURE_MODE,
    RF_AT_FUNCID_VOLT_COMPEN,
    RF_AT_FUNCID_DPD_TX_GAIN,
    RF_AT_FUNCID_DPD_MODE,
    RF_AT_FUNCID_TSSI_MODE,
    RF_AT_FUNCID_TX_GAIN_CODE,
    RF_AT_FUNCID_TX_PWR_MODE,

    /* Query command */
    RF_AT_FUNCID_TXED_COUNT = 32,
    RF_AT_FUNCID_TXOK_COUNT,
    RF_AT_FUNCID_RXOK_COUNT,
    RF_AT_FUNCID_RXERROR_COUNT,
    RF_AT_FUNCID_RESULT_INFO,
    RF_AT_FUNCID_TRX_IQ_RESULT,
    RF_AT_FUNCID_TSSI_RESULT,
    RF_AT_FUNCID_DPD_RESULT,
    RF_AT_FUNCID_RXV_DUMP,
    RF_AT_FUNCID_RX_PHY_STATIS,
    RF_AT_FUNCID_MEASURE_RESULT,
    RF_AT_FUNCID_TEMP_SENSOR,
    RF_AT_FUNCID_VOLT_SENSOR,
    RF_AT_FUNCID_READ_EFUSE,
    RF_AT_FUNCID_RX_RSSI,

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
    RF_AT_FUNCID_SET_J_MODE_SETTING = 75,

    /*ICAP command*/
    RF_AT_FUNCID_SET_ICAP_CONTENT = 80,
    RF_AT_FUNCID_SET_ICAP_MODE,
    RF_AT_FUNCID_SET_ICAP_STARTCAP,
    RF_AT_FUNCID_SET_ICAP_SIZE = 83,
    RF_AT_FUNCID_SET_ICAP_TRIGGER_OFFSET,
    RF_AT_FUNCID_QUERY_ICAP_DUMP_FILE = 85,
    RF_AT_FUNCID_QUERY_ICAP_STARTING_ADDR = 86,
    RF_AT_FUNCID_QUERY_ICAP_END_ADDR = 87,
    RF_AT_FUNCID_QUERY_ICAP_WAPPER_ADDR = 88,

	/*2G 5G Band*/
	RF_AT_FUNCID_SET_BAND = 90,

	/*Reset Counter*/
	RF_AT_FUNCID_RESETTXRXCOUNTER = 91,

	/*FAGC RSSI Path*/
	RF_AT_FUNCID_FAGC_RSSI_PATH = 92,

	/*Set RX Filter Packet Length*/
	RF_AT_FUNCID_RX_FILTER_PKT_LEN = 93,

	/*Tone*/
	RF_AT_FUNCID_SET_TONE_RF_GAIN = 96,
	RF_AT_FUNCID_SET_TONE_DIGITAL_GAIN = 97,
	RF_AT_FUNCID_SET_TONE_TYPE = 98,
	RF_AT_FUNCID_SET_TONE_DC_OFFSET = 99,
	RF_AT_FUNCID_SET_TONE_BW = 100,

	/*MT6632 Add*/
	RF_AT_FUNCID_SET_MAC_HEADER = 101,
	RF_AT_FUNCID_SET_SEQ_CTRL = 102,
	RF_AT_FUNCID_SET_PAYLOAD = 103,
	RF_AT_FUNCID_SET_DBDC_BAND_IDX = 104,
	RF_AT_FUNCID_SET_BYPASS_CAL_STEP = 105,

	/*Set RX Path*/
	RF_AT_FUNCID_SET_RX_PATH = 106,

	/*Set Frequency Offset*/
	RF_AT_FUNCID_SET_FRWQ_OFFSET = 107,

	/*Get Frequency Offset*/
	RF_AT_FUNCID_GET_FREQ_OFFSET = 108,

	/*Set RXV Debug Index*/
	RF_AT_FUNCID_SET_RXV_INDEX = 109,

	/*Set Test Mode DBDC Enable*/
	RF_AT_FUNCID_SET_DBDC_ENABLE = 110,

	/*Get Test Mode DBDC Enable*/
	RF_AT_FUNCID_GET_DBDC_ENABLE = 111,

	/*Set ICAP Ring Capture*/
	RF_AT_FUNCID_SET_ICAP_RING = 112,

	/*Set TX Path*/
	RF_AT_FUNCID_SET_TX_PATH = 113,

    /*Set Nss*/
    RF_AT_FUNCID_SET_NSS = 114,

	/*Set TX Antenna Mask*/
	RF_AT_FUNCID_SET_ANTMASK = 115,


	/*TMR set command*/
	RF_AT_FUNCID_SET_TMR_ROLE=116,
	RF_AT_FUNCID_SET_TMR_MODULE=117,
	RF_AT_FUNCID_SET_TMR_DBM=118,
	RF_AT_FUNCID_SET_TMR_ITER=119,

	/* Set ADC For IRR Feature */
    RF_AT_FUNCID_SET_ADC = 120,

	/* Set RX Gain For IRR Feature */
    RF_AT_FUNCID_SET_RX_GAIN = 121,

	/* Set TTG For IRR Feature */
    RF_AT_FUNCID_SET_TTG = 122,

	/* Set TTG ON/OFF For IRR Feature */
    RF_AT_FUNCID_TTG_ON_OFF = 123,

	/* Set TSSI for QA Tool Setting */
	RF_AT_FUNCID_SET_TSSI = 124,

	/* Set Recal Cal Step*/
	RF_AT_FUNCID_SET_RECAL_CAL_STEP = 125,

	/* Set iBF/eBF enable */
	RF_AT_FUNCID_SET_IBF_ENABLE = 126,
	RF_AT_FUNCID_SET_EBF_ENABLE = 127,

	/* Set MPS Setting */
	RF_AT_FUNCID_SET_MPS_SIZE = 128,
	RF_AT_FUNCID_SET_MPS_SEQ_DATA = 129,
	RF_AT_FUNCID_SET_MPS_PAYLOAD_LEN = 130,
	RF_AT_FUNCID_SET_MPS_PKT_CNT = 131,
	RF_AT_FUNCID_SET_MPS_PWR_GAIN = 132,
	RF_AT_FUNCID_SET_MPS_NSS = 133,
	RF_AT_FUNCID_SET_MPS_PACKAGE_BW = 134,
	RF_AT_FUNCID_SET_MPS_SYSTEM_BW = 135,
	RF_AT_FUNCID_GET_CH_TX_PWR_OFFSET = 136,
	RF_AT_FUNCID_SET_CH_TX_PWR_OFFSET = 137,
	RF_AT_FUNCID_GET_EEPROM_ADDR = 138,
	RF_AT_FUNCID_GET_FREQ_OFFSET_EEPROM_ADDR = 139,
	RF_AT_FUNCID_GET_AUTO_ISOLATION_VALUE = 140
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
    RF_AT_COMMAND_AUTOISO = 17,
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
#define DBGLOG(_fmt...)  \
    if(fgDebugMode) { \
        printf("<DBG> " _fmt); \
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
bool WIFI_TEST_SetPreamble(WIFI_PreambleType_t PreambleType);
bool WIFI_TEST_Channel(int ChannelNo);
bool WIFI_TEST_TxGain(float TxGain);
bool WIFI_TEST_TxBurstInterval(int SIFS);
bool WIFI_TEST_TxPayloadLength(int TxPayLength);
bool WIFI_TEST_TxBurstFrames(int Frames);
bool WIFI_TEST_TxDestAddress(unsigned char *addr);
bool WIFI_TEST_TxStart(void);
bool WIFI_TEST_TxStop(void);
bool WIFI_TEST_SetRxPath(uint32_t u4Path);
bool WIFI_TEST_SetFreqOffset(uint32_t u4FreqOffset);
bool WIFI_TEST_RxStart(void);
bool WIFI_TEST_RxStop(void);
bool WIFI_TEST_AutoIso(void);
bool WIFI_TEST_FRError(int *FError);
bool WIFI_TEST_FRGood(int *FRGood);
bool WIFI_TEST_RSSI(int *RSSI);
bool WIFI_TEST_IsRunning(void);
bool WIFI_TEST_IsUp(void);
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

int WIFI_TEST_set_HQA(uint16_t offset, uint16_t sequence, unsigned char *data);
int WIFI_TEST_set_Eeprom_Mode(unsigned char data);
int WIFI_TEST_set_Efuse_Buffer_Mode(unsigned char data);
bool WIFI_TEST_SetTxPowerCompensation(uint32_t u4TxPowerCompensation);
bool WIFI_TEST_GetTxPowerCompensationEepromAddr(unsigned int offset, unsigned int *val);
bool WIFI_TEST_SetTxPath(uint32_t u4Path);
bool WIFI_TEST_SetNss(uint32_t u4Nss);
bool WIFI_TEST_SetDBDC(uint32_t u4Dbdc);
bool WIFI_TEST_SetDBDCBand(uint32_t u4Dbdcband);
bool WIFI_TEST_SetCBW(uint32_t u4Cbw);
bool WIFI_TEST_SetDBW(uint32_t u4Dbw);
bool WIFI_TEST_SetBand(uint32_t u4Band);
bool WIFI_TEST_SetMacHeader(uint32_t u4MacHeader);
bool WIFI_TEST_SetPayLoad(uint32_t u4Payload);
bool WIFI_TEST_SetTA(uint32_t u4TA);
bool WIFI_TEST_SetStbc(uint32_t u4Stbc);
bool WIFI_TEST_SetIbf(uint32_t u4Ibf);
bool WIFI_TEST_SetEbf(uint32_t u4Ebf);
bool WIFI_TEST_SetCbw(uint32_t u4Cbw);
bool WIFI_TEST_SetDbw(uint32_t u4Dbw);
bool WIFI_TEST_SetPriCh(uint32_t u4PriCh);

bool WIFI_TEST_GetCalFreqOffsetEepromAddr(unsigned int offset, unsigned int *val);
int
WIFI_TEST_get_HQA(
      uint16_t offset,
      uint16_t sequence,
      unsigned char *data
    );
int
WIFI_TEST_GetFreeEfuseBlock(
      unsigned int *data
    );
bool WIFI_TEST_MAC_Write(unsigned int addr, unsigned char* mac_bytes, unsigned char mac_bytes_len);
bool WIFI_TEST_GetCalIsolationValue(uint32_t *val);

#endif /* __LIBWIFITEST_H__ */
