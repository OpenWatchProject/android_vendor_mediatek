/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef _CFG_WIFI_FILE_H
#define _CFG_WIFI_FILE_H

// the record structure define of wifi nvram file
/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
typedef signed char             INT_8, *PINT_8, **PPINT_8;
typedef unsigned char           UINT_8, *PUINT_8, **PPUINT_8, *P_UINT_8;
typedef unsigned short          UINT_16, *PUINT_16, **PPUINT_16;
typedef unsigned int      UINT_32, *PUINT_32, **PPUINT_32;


/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/
/*----------------------------------------------------------------------------*/
/*!
*  NVRAM data structure defination---------Start
*  Part1OwnVersion:0001
*  Part1PeerVersion:0008
*  Release Date : 2018.07.17
*/
/*----------------------------------------------------------------------------*/
// duplicated from nic_cmd_event.h to avoid header dependency
typedef struct _TX_PWR_PARAM_T {
    INT_8       cTxPwr2G4Cck;		/* signed, in unit of 0.5dBm */
#if defined(MT6620)
    INT_8       acReserved[3];
#else/*For MT6628 and later chip*/
     INT_8       cTxPwr2G4Dsss;      /* signed, in unit of 0.5dBm */
     INT_8       acReserved[2];
#endif

    INT_8       cTxPwr2G4OFDM_BPSK;
    INT_8       cTxPwr2G4OFDM_QPSK;
    INT_8       cTxPwr2G4OFDM_16QAM;
    INT_8       cTxPwr2G4OFDM_Reserved;
    INT_8       cTxPwr2G4OFDM_48Mbps;
    INT_8       cTxPwr2G4OFDM_54Mbps;

    INT_8       cTxPwr2G4HT20_BPSK;
    INT_8       cTxPwr2G4HT20_QPSK;
    INT_8       cTxPwr2G4HT20_16QAM;
    INT_8       cTxPwr2G4HT20_MCS5;
    INT_8       cTxPwr2G4HT20_MCS6;
    INT_8       cTxPwr2G4HT20_MCS7;

    INT_8       cTxPwr2G4HT40_BPSK;
    INT_8       cTxPwr2G4HT40_QPSK;
    INT_8       cTxPwr2G4HT40_16QAM;
    INT_8       cTxPwr2G4HT40_MCS5;
    INT_8       cTxPwr2G4HT40_MCS6;
    INT_8       cTxPwr2G4HT40_MCS7;

    INT_8       cTxPwr5GOFDM_BPSK;
    INT_8       cTxPwr5GOFDM_QPSK;
    INT_8       cTxPwr5GOFDM_16QAM;
    INT_8       cTxPwr5GOFDM_Reserved;
    INT_8       cTxPwr5GOFDM_48Mbps;
    INT_8       cTxPwr5GOFDM_54Mbps;

    INT_8       cTxPwr5GHT20_BPSK;
    INT_8       cTxPwr5GHT20_QPSK;
    INT_8       cTxPwr5GHT20_16QAM;
    INT_8       cTxPwr5GHT20_MCS5;
    INT_8       cTxPwr5GHT20_MCS6;
    INT_8       cTxPwr5GHT20_MCS7;

    INT_8       cTxPwr5GHT40_BPSK;
    INT_8       cTxPwr5GHT40_QPSK;
    INT_8       cTxPwr5GHT40_16QAM;
    INT_8       cTxPwr5GHT40_MCS5;
    INT_8       cTxPwr5GHT40_MCS6;
    INT_8       cTxPwr5GHT40_MCS7;
} TX_PWR_PARAM_T, *P_TX_PWR_PARAM_T;

typedef struct _WIFI_NVRAM_2G4_TX_POWER_T
{
    UINT_8 uc2G4TxPwrCck1M;
    UINT_8 uc2G4TxPwrCck2M;
    UINT_8 uc2G4TxPwrCck5M;
    UINT_8 uc2G4TxPwrCck11M;
    UINT_8 uc2G4TxPwrOfdm6M;
    UINT_8 uc2G4TxPwrOfdm9M;
    UINT_8 uc2G4TxPwrOfdm12M;
    UINT_8 uc2G4TxPwrOfdm18M;
    UINT_8 uc2G4TxPwrOfdm24M;
    UINT_8 uc2G4TxPwrOfdm36M;
    UINT_8 uc2G4TxPwrOfdm48M;
    UINT_8 uc2G4TxPwrOfdm54M;
    UINT_8 uc2G4TxPwrHt20Mcs0;
    UINT_8 uc2G4TxPwrHt20Mcs1;
    UINT_8 uc2G4TxPwrHt20Mcs2;
    UINT_8 uc2G4TxPwrHt20Mcs3;
    UINT_8 uc2G4TxPwrHt20Mcs4;
    UINT_8 uc2G4TxPwrHt20Mcs5;
    UINT_8 uc2G4TxPwrHt20Mcs6;
    UINT_8 uc2G4TxPwrHt20Mcs7;
    UINT_8 uc2G4TxPwrHt40Mcs0;
    UINT_8 uc2G4TxPwrHt40Mcs1;
    UINT_8 uc2G4TxPwrHt40Mcs2;
    UINT_8 uc2G4TxPwrHt40Mcs3;
    UINT_8 uc2G4TxPwrHt40Mcs4;
    UINT_8 uc2G4TxPwrHt40Mcs5;
    UINT_8 uc2G4TxPwrHt40Mcs6;
    UINT_8 uc2G4TxPwrHt40Mcs7;
    UINT_8 uc2G4TxPwrVht20Mcs0;
    UINT_8 uc2G4TxPwrVht20Mcs1;
    UINT_8 uc2G4TxPwrVht20Mcs2;
    UINT_8 uc2G4TxPwrVht20Mcs3;
    UINT_8 uc2G4TxPwrVht20Mcs4;
    UINT_8 uc2G4TxPwrVht20Mcs5;
    UINT_8 uc2G4TxPwrVht20Mcs6;
    UINT_8 uc2G4TxPwrVht20Mcs7;
    UINT_8 uc2G4TxPwrVht20Mcs8;
    UINT_8 uc2G4TxPwrVht20Mcs9;
    UINT_8 uc2G4TxPwrVht40Mcs0;
    UINT_8 uc2G4TxPwrVht40Mcs1;
    UINT_8 uc2G4TxPwrVht40Mcs2;
    UINT_8 uc2G4TxPwrVht40Mcs3;
    UINT_8 uc2G4TxPwrVht40Mcs4;
    UINT_8 uc2G4TxPwrVht40Mcs5;
    UINT_8 uc2G4TxPwrVht40Mcs6;
    UINT_8 uc2G4TxPwrVht40Mcs7;
    UINT_8 uc2G4TxPwrVht40Mcs8;
    UINT_8 uc2G4TxPwrVht40Mcs9;
    UINT_8 uc2G4TxPwrHt40Mcs32;
    UINT_8 uc2G4TxPwrLGBW40DuplucateMode;

} WIFI_NVRAM_2G4_TX_POWER_T, *P_WIFI_NVRAM_2G4_TX_POWER_T;

typedef struct _WIFI_NVRAM_5G_TX_POWER_T
{
    UINT_8 uc5GTxPwrOfdm6M;
    UINT_8 uc5GTxPwrOfdm9M;
    UINT_8 uc5GTxPwrOfdm12M;
    UINT_8 uc5GTxPwrOfdm18M;
    UINT_8 uc5GTxPwrOfdm24M;
    UINT_8 uc5GTxPwrOfdm36M;
    UINT_8 uc5GTxPwrOfdm48M;
    UINT_8 uc5GTxPwrOfdm54M;
    UINT_8 uc5GTxPwrHt20Mcs0;
    UINT_8 uc5GTxPwrHt20Mcs1;
    UINT_8 uc5GTxPwrHt20Mcs2;
    UINT_8 uc5GTxPwrHt20Mcs3;
    UINT_8 uc5GTxPwrHt20Mcs4;
    UINT_8 uc5GTxPwrHt20Mcs5;
    UINT_8 uc5GTxPwrHt20Mcs6;
    UINT_8 uc5GTxPwrHt20Mcs7;
    UINT_8 uc5GTxPwrHt40Mcs0;
    UINT_8 uc5GTxPwrHt40Mcs1;
    UINT_8 uc5GTxPwrHt40Mcs2;
    UINT_8 uc5GTxPwrHt40Mcs3;
    UINT_8 uc5GTxPwrHt40Mcs4;
    UINT_8 uc5GTxPwrHt40Mcs5;
    UINT_8 uc5GTxPwrHt40Mcs6;
    UINT_8 uc5GTxPwrHt40Mcs7;
    UINT_8 uc5GTxPwrVht20Mcs0;
    UINT_8 uc5GTxPwrVht20Mcs1;
    UINT_8 uc5GTxPwrVht20Mcs2;
    UINT_8 uc5GTxPwrVht20Mcs3;
    UINT_8 uc5GTxPwrVht20Mcs4;
    UINT_8 uc5GTxPwrVht20Mcs5;
    UINT_8 uc5GTxPwrVht20Mcs6;
    UINT_8 uc5GTxPwrVht20Mcs7;
    UINT_8 uc5GTxPwrVht20Mcs8;
    UINT_8 uc5GTxPwrVht20Mcs9;
    UINT_8 uc5GTxPwrVht40Mcs0;
    UINT_8 uc5GTxPwrVht40Mcs1;
    UINT_8 uc5GTxPwrVht40Mcs2;
    UINT_8 uc5GTxPwrVht40Mcs3;
    UINT_8 uc5GTxPwrVht40Mcs4;
    UINT_8 uc5GTxPwrVht40Mcs5;
    UINT_8 uc5GTxPwrVht40Mcs6;
    UINT_8 uc5GTxPwrVht40Mcs7;
    UINT_8 uc5GTxPwrVht40Mcs8;
    UINT_8 uc5GTxPwrVht40Mcs9;
    UINT_8 uc5GTxPwrVht80Mcs0;
    UINT_8 uc5GTxPwrVht80Mcs1;
    UINT_8 uc5GTxPwrVht80Mcs2;
    UINT_8 uc5GTxPwrVht80Mcs3;
    UINT_8 uc5GTxPwrVht80Mcs4;
    UINT_8 uc5GTxPwrVht80Mcs5;
    UINT_8 uc5GTxPwrVht80Mcs6;
    UINT_8 uc5GTxPwrVht80Mcs7;
    UINT_8 uc5GTxPwrVht80Mcs8;
    UINT_8 uc5GTxPwrVht80Mcs9;
    UINT_8 uc5GTxPwrVht160Mcs0;
    UINT_8 uc5GTxPwrVht160Mcs1;
    UINT_8 uc5GTxPwrVht160Mcs2;
    UINT_8 uc5GTxPwrVht160Mcs3;
    UINT_8 uc5GTxPwrVht160Mcs4;
    UINT_8 uc5GTxPwrVht160Mcs5;
    UINT_8 uc5GTxPwrVht160Mcs6;
    UINT_8 uc5GTxPwrVht160Mcs7;
    UINT_8 uc5GTxPwrVht160Mcs8;
    UINT_8 uc5GTxPwrVht160Mcs9;
    UINT_8 uc5GTxPwrHt40Mcs32;
    UINT_8 uc5GTxPwrLGBW40DuplucateMode;
    UINT_8 uc5GTxPwrLGBW80DuplucateMode;
    UINT_8 uc5GTxPwrLGBW1600DuplucateMode;
    UINT_8 uc5GBw5MTxPwrDelta;
    UINT_8 uc5GBw10MTxPwrDelta;
} WIFI_NVRAM_5G_TX_POWER_T , *P_WIFI_NVRAM_5G_TX_POWER_T;

typedef struct _WIFI_NVRAM_FE_LOSS_T
{
    UINT_8 ucTxFeLoss;
    UINT_8 ucRxFeLoss;
} WIFI_NVRAM_FE_LOSS_T, *P_WIFI_NVRAM_FE_LOSS_T;

typedef struct _WIFI_NVRAM_TSSI_T
{
    UINT_8 ucPATssiSlope;
    UINT_8 ucPATssiOffsetLSB;
    UINT_8 ucPATssiOffsetMSB;
    UINT_8 ucTxTargetPower;
} WIFI_NVRAM_TSSI_T, *P_WIFI_NVRAM_TSSI_T;


typedef struct _WIFI_NVRAM_2G4_WF0_PATH_T
{
    WIFI_NVRAM_FE_LOSS_T rFeLoss[14]; /*CH1 ~ CH14*/
    WIFI_NVRAM_TSSI_T rTssi[14]; /*CH1 ~ CH14*/

    UINT_8 uc2G4Tx0TssiOffTxPower;
    UINT_8 aucTx2G4TssiChannelOffsetLowCh[14];  /*Efuse format ,Bit[7]:Vaild bit, Bit[6] sign bit*/
    UINT_8 aucTx2G4DpdG0OffsetrateH_L[6];
    UINT_8 aucG0Delta[14];
    UINT_8 auc2G4Wf0Lna_Gain_CalDiff[2];
    UINT_8 aucTx2G4TssiCVTOffsetLowCh[14];
    UINT_8 aucReserved0[2];
}  WIFI_NVRAM_2G4_WF0_PATH_T, *P_WIFI_NVRAM_2G4_WF0_PATH_T;

typedef struct _WIFI_NVRAM_5G_TX_POWER_OFFSET_T
{
    UINT_8 ucTxPowerOffsetLow;
    UINT_8 ucTxPowerOffsetHigh;
} WIFI_NVRAM_5G_TX_POWER_OFFSET_T, *P_WIFI_NVRAM_5G_TX_POWER_OFFSET_T;

typedef struct _WIFI_NVRAM_5G_TX_DPD_G0_OFFSET_T
{
    UINT_8 ucTxPowerOffsetLow;
    UINT_8 ucTxPowerOffsetHigh;
} WIFI_NVRAM_5G_TX_DPD_G0_OFFSET_T, *P_WIFI_NVRAM_5G_TX_DPD_G0_OFFSET_T;

typedef struct _WIFI_NVRAM_5G_WF0_PATH_T
{
    WIFI_NVRAM_FE_LOSS_T rFeLoss[8];    /*Group 1 ~ Group 8*/
    WIFI_NVRAM_TSSI_T rTssi[8];         /*Group 1 ~ Group 8*/
    UINT_8 uc5GTx0TssiOffTxPower;
    WIFI_NVRAM_5G_TX_POWER_OFFSET_T rTxTssiChannelOffset[8]; /*Group 1 ~ Group 8*/ /*Efuse format ,Bit[7]:Vaild bit, Bit[6] sign bit*/
    UINT_8 aucTx5GDpdG0OffsetrateH_L[48];
    UINT_8 aucG0Delta[8];
    UINT_8 auc5GWf0Lna_Gain_CalDiff[16];
    WIFI_NVRAM_5G_TX_POWER_OFFSET_T rTxTssiCVTOffset[8]; /*Group 1 ~ Group 8*/
    UINT_8 aucReserved0[4];
}  WIFI_NVRAM_5G_WF0_PATH_T, *P_WIFI_NVRAM_5G_WF0_PATH_T;

typedef struct _WIFI_NVRAM_TSSI_PA_CELL_OFFSET_T
{
    UINT_8  ucTssiPACellOffsetHPA;
    UINT_8  ucTssiPACellOffsetMPA;
} WIFI_NVRAM_TSSI_PA_CELL_OFFSET_T, *P_WIFI_NVRAM_TSSI_PA_CELL_OFFSET_T;


typedef struct _WIFI_NVRAM_DYNAMIC_IPA_SWITCH_T
{
    UINT_8  ucPACckThreshold;
    UINT_8  ucPALowRateThreshold;
    UINT_8  ucPAMidRateThreshold;
    UINT_8  ucPAHighRateThreshold;
} WIFI_NVRAM_DYNAMIC_IPA_SWITCH_T, *P_WIFI_NVRAM_DYNAMIC_IPA_SWITCH_T;

typedef struct _WIFI_NVRAM_2G4_ELNA_PARAM_T
{
    UINT_8  ucELNAReceivedModeGain;
    UINT_8  ucELNAReceivedModeP1dB;
    UINT_8  ucELNABypassModeGain;
    UINT_8  ucELNABypassdModeP1dB;
} WIFI_NVRAM_2G4_ELNA_PARAM_T, *P_WIFI_NVRAM_2G4_ELNA_PARAM_T;

typedef struct _WIFI_NVRAM_THERMO_COMP_T
{
    UINT_8 ucTssiOffMinus7StepNumber;
    UINT_8 ucTssiOffMinus6StepNumber;
    UINT_8 ucTssiOffMinus5StepNumber;
    UINT_8 ucTssiOffMinus4StepNumber;
    UINT_8 ucTssiOffMinus3StepNumber;
    UINT_8 ucTssiOffMinus2StepNumber;
    UINT_8 ucTssiOffMinus1StepNumber;
    UINT_8 ucTssiOffMinus0StepNumber;
    UINT_8 ucTssiOffReferenceStep;
    UINT_8 ucTssiOffReferenceTemp;
    UINT_8 ucTssiOffPlus1StepNumber;
    UINT_8 ucTssiOffPlus2StepNumber;
    UINT_8 ucTssiOffPlus3StepNumber;
    UINT_8 ucTssiOffPlus4StepNumber;
    UINT_8 ucTssiOffPlus5StepNumber;
    UINT_8 ucTssiOffPlus6StepNumber;
    UINT_8 ucTssiOffPlus7StepNumber;
} WIFI_NVRAM_THERMO_COMP_T, *P_WIFI_NVRAM_THERMO_COMP_T;

typedef struct _WIFI_NVRAM_EPA_DPD_BOUND_T
{
    UINT_8 ucEpaDpdOnLowrBound;
    UINT_8 ucEpaDpdOnUpperBound;
    UINT_8 ucEpaDpdProtection[2];
} WIFI_NVRAM_EPA_DPD_BOUND_T, *P_WIFI_NVRAM_EPA_DPD_BOUND_T;

typedef struct _WIFI_NVRAM_DESENSE_CONTROL_T
{
    UINT_8 u1DesenseCtl;
} WIFI_NVRAM_DESENSE_CONTROL_T, *P_WIFI_NVRAM_DESENSE_CONTROL_T;

typedef struct _WIFI_NVRAM_2G4_SHAPING_T
{
    UINT_8 u1ShapRight;
    UINT_8 u1ShapLeft;
    UINT_8 u1ShapDc;
} WIFI_NVRAM_2G4_SHAPING_T, *P_WIFI_NVRAM_2G4_SHAPING_T;

typedef struct _WIFI_NVRAM_PA_BIAS_T
{
    UINT_8 u1HPaBiasLSB;
    UINT_8 u1HPaBiasMSB; //[15:0] = H_PA_BIAS
    UINT_8 u1LPaBiasLSB;
    UINT_8 u1LPaBiasMSB; //[31:16] = L_PA_BIAS
} WIFI_NVRAM_PA_BIAS_T, *P_WIFI_NVRAM_PA_BIAS_T;

typedef struct _WIFI_NVRAM_2G4_COMMON_T
{
    WIFI_NVRAM_TSSI_PA_CELL_OFFSET_T rTssiPACellOffset[1];
    WIFI_NVRAM_DYNAMIC_IPA_SWITCH_T rDynamIPASwitch[3];
    WIFI_NVRAM_2G4_ELNA_PARAM_T r2G4ELNAParam[1];
    WIFI_NVRAM_THERMO_COMP_T rThermoComp[1];
    WIFI_NVRAM_EPA_DPD_BOUND_T rEPADpdBound[3];
    INT_8 ic2G4EpaTxFELostGain;
    WIFI_NVRAM_DESENSE_CONTROL_T rDesenseCtrl[2]; //GROUP 1: CH1~CH8, GROUP 2: CH9~CH14
    WIFI_NVRAM_PA_BIAS_T rPaBias[2];
    WIFI_NVRAM_2G4_SHAPING_T rShaping[14];
}  WIFI_NVRAM_2G4_COMMON_T, *P_WIFI_NVRAM_2G4_COMMON_T;

typedef struct _WIFI_NVRAM_5G_ELNA_PARAM_T
{
    UINT_8 ucElnaReceivedModeGain;
    UINT_8 ucElnaReceivedModeNf;
    UINT_8 ucElnaReceivedModeP1Db;
    UINT_8 ucElnaBypassModeGain;
    UINT_8 ucElnaBypassModeNf;
    UINT_8 ucElnaBypassModeP1Db;

} WIFI_NVRAM_5G_ELNA_PARAM_T, *P_WIFI_NVRAM_5G_ELNA_PARAM_T;

typedef struct _WIFI_NVRAM_5G_COMMON_T
{
    WIFI_NVRAM_TSSI_PA_CELL_OFFSET_T rTssiPACellOffset[8];/*Group 1 ~ Group 8*/
    WIFI_NVRAM_DYNAMIC_IPA_SWITCH_T rDynamIPASwitch[3];
    WIFI_NVRAM_5G_ELNA_PARAM_T r5GELNAParam[3];
    WIFI_NVRAM_THERMO_COMP_T rThermoComp[1];
    WIFI_NVRAM_EPA_DPD_BOUND_T rEPADpdBound[3];
    INT_8 ic5GEpaTxFELostGain;
    WIFI_NVRAM_DESENSE_CONTROL_T rDesenseCtrl[1];
    WIFI_NVRAM_PA_BIAS_T rPaBias[1];
    UINT_8 aucReserved5[4];
}  WIFI_NVRAM_5G_COMMON_T, *P_WIFI_NVRAM_5G_COMMON_T;

typedef struct _WIFI_NVRAM_SYSTEM_T
{
    UINT_8    u1TxPath           ; /* Tx Path                                               */
    UINT_8    u1RxPath           ; /* Rx Path                                               */
    UINT_8    u1BandSelectBand0  ; /* Band Select (if DBDC on, for Band0)                   */
    UINT_8    u1CckTxStream      ; /* CCK TxStream                                          */
    UINT_8    u1OfdmTxStream     ; /* OFDM TxStream                                         */
    UINT_8    u1BandSelectBand1  ; /* Band Select (if DBDC on, for Band1)                   */
    UINT_8    u1RxStream         ; /* Rx Stream                                             */
    UINT_8    u1DbdcForceMode    ; /* Dbdc Force-Mode                                         */
    UINT_8    u1TxSW_N_G         ; /* N Ploarity(G_BAND)                                      */
    UINT_8    u1TxSW_P_G         ; /* P Ploarity(G_BAND)                                      */
    UINT_8    u1TxSW_N_A         ; /* N Ploarity(A_BAND)                                      */
    UINT_8    u1TxSW_P_A         ; /* P Ploarity(A_BAND)                                      */
    UINT_8    u1Dbdc0TxPath      ; /* DBDC0 Tx Path                                         */
    UINT_8    u1Dbdc0RxPath      ; /* DBDC0 Rx Path                                         */
    UINT_8    u1Dbdc1TxPath      ; /* DBDC1 Tx Path                                         */
    UINT_8    u1Dbdc1RxPath      ; /* DBDC1 Rx Path                                         */
    UINT_8    u1TxFiler_G        ; /* Tx Filter (G_BAND)                                      */
    UINT_8    u1TxFiler_A        ; /* Tx Filter (A_BAND)                                      */
    UINT_8    u1IpaDPD_G         ; /* iPA DPD (G_BAND)                                        */
    UINT_8    u1EpaDPD_G         ; /* ePA DPD (G_BAND)                                        */
    UINT_8    u1IpaDPD_A         ; /* iPA DPD (A_BAND)                                        */
    UINT_8    u1EpaDPD_A         ; /* ePA DPD (A_BAND)                                        */
    UINT_8    u1DPDTable_G       ; /* DPD Table (G_BAND)                                      */
    UINT_8    u1DPDTable_A       ; /* DPD Table (A_BAND)                                      */
    UINT_8    u1PaEn_G           ; /* PA Enable (G_BAND)                                      */
    UINT_8    u1PaEn_A           ; /* PA Enable (A_BAND)                                      */
    UINT_8    u1FixPa_G          ; /* Fix PA (G_BAND)                                         */
    UINT_8    u1FixPa_A          ; /* Fix PA (A_BAND)                                         */
    UINT_8    u1FixPaCell_G      ; /* Fix PA cell (G_BAND)                                  */
    UINT_8    u1FixPaCell_A      ; /* Fix PA cell (A_BAND)                                  */
    UINT_8    u1LnaEn_G          ; /* LNA Enable (G_BAND)                                     */
    UINT_8    u1LnaEn_A          ; /* LNA Enable (A_BAND)                                     */
    UINT_8    u1LnaPaSelect_G    ; /* LNA/PA select (G_BAND)                                */
    UINT_8    u1LnaPaSelect_A    ; /* LNA/PA select (A_BAND)                                */
    UINT_8    u1RxDCOC_G         ; /* RxDCOC (G_BAND)                                         */
    UINT_8    u1RxDCOC_A         ; /* RxDCOC (A_BAND)                                         */
    UINT_8    u1TssiSet_G        ; /* Tssi Set (if DBDC on, for Band0)                        */
    UINT_8    u1TempCompSel_G    ; /* Temperature compensation Select (if DBDC on, for Band0) */
    UINT_8    u1TssiSet_A        ; /* Tssi Set (if DBDC on, for Band0)                        */
    UINT_8    u1TempCompSel_A    ; /* Temperature compensation Select (if DBDC on, for Band0) */
    UINT_8    u1TssiSet_BAND1    ; /* Tssi Set (if DBDC on, for Band1)                        */
    UINT_8    u1TempCompSel_BAND1; /* Temperature compensation Select (if DBDC on, for Band1) */
    UINT_8    u1CoRx             ; /* CORX */
    UINT_8    u1THADCAnalogPart  ; /* THADC Analog part                                     */
    UINT_8    u1THADCSlopPart    ; /* THADC Slop part                                       */
    UINT_8    u1THADCCalPart     ; /* THADC Calibration part                                */
    UINT_8    u1ThermalCompOffset; /* Thermal Compensation offset                           */
    UINT_8    u1WF0RcalResult    ; /* WF0 Rcal Result                                       */
    UINT_8    u1WF2RcalResult    ; /* WF2 Rcal Result                                       */
    UINT_8    u1RFTxLcLoop       ; /* RF Tx LC Loop                                         */
    UINT_8    u1CoexIsolation    ; /* cOEX isolation                                        */
    UINT_8    u1RxAntDiversity_G ; /* Rx Antenna Diversity (G_BAND)                         */
    UINT_8    u1RxAntDiversity_A ; /* Rx Antenna Diversity (A_BAND)                         */
    UINT_8    aucReserved[5];
}  WIFI_NVRAM_SYSTEM_T, *P_WIFI_NVRAM_SYSTEM_T;

typedef struct _WIFI_SOC1_0_NVRAM_PARAM_STRUCT_T
{
    /* NVRAM offset[0] ~ offset[255] */
    UINT_16 u2Part1OwnVersion;
    UINT_16 u2Part1PeerVersion;
    UINT_8 aucMacAddress[6];
    UINT_8 aucCountryCode[2];
    TX_PWR_PARAM_T rTxPwr;
#if CFG_SUPPORT_NVRAM_5G
    union {
        /* struct NEW_EFUSE_MAPPING2NVRAM u; */
        UINT_8 aucEFUSE[144];
    } EfuseMapping;
#else
    UINT_8 aucEFUSE[144];
#endif
    UINT_8 ucTxPwrValid;
    UINT_8 ucSupport5GBand;
    UINT_8 fg2G4BandEdgePwrUsed;
    INT_8 cBandEdgeMaxPwrCCK;
    INT_8 cBandEdgeMaxPwrOFDM20;
    INT_8 cBandEdgeMaxPwrOFDM40;

    UINT_8 ucRegChannelListMap;
    UINT_8 ucRegChannelListIndex;
    UINT_8 aucRegSubbandInfo[36];
    UINT_8 ucEnable5GBand;			/* move from 256+ offset to here */
    UINT_8 ucNeedCheckLDO;
    UINT_8 ucDefaultTestMode;
    UINT_8 aucReserved0[256 - 243];

    /* NVRAM offset[256] ~ offset[511] */
    UINT_8 ucTypeID0;
    UINT_8 ucTypeLen0;
    UINT_8 ucTxPowerControl; /*0: disable, 1: enable*/
    UINT_8 aucTxPowerTotalSize[2]; /*PowerTotalSize[0]:MSB,ucPowerTotalSize[1]:LSB*/
    UINT_8 ucTypeID1;
    UINT_8 ucTypeLen1;
    WIFI_NVRAM_2G4_TX_POWER_T r2G4TxPower; /*Efuse format ,Bit[7]:Vaild bit, Bit[6] sign bit*/
    UINT_8 ucTypeID2;
    UINT_8 ucTypeLen2;
    WIFI_NVRAM_5G_TX_POWER_T r5GTxpower; /*Efuse format ,Bit[7]:Vaild bit, Bit[6] sign bit*/
    UINT_8 ucTypeID3;
    UINT_8 ucTypeLen3;
    WIFI_NVRAM_2G4_WF0_PATH_T r2G4WF0path;
    UINT_8 ucTypeID4;
    UINT_8 ucTypeLen4;
    WIFI_NVRAM_5G_WF0_PATH_T  r5GWF0path;
    UINT_8 ucTypeID5;
    UINT_8 ucTypeLen5;
    WIFI_NVRAM_2G4_COMMON_T r2G4Cmm;
    UINT_8 ucTypeID6;
    UINT_8 ucTypeLen6;
    WIFI_NVRAM_5G_COMMON_T r5GCmm;
    UINT_8 ucTypeID7;
    UINT_8 ucTypeLen7;
    WIFI_NVRAM_SYSTEM_T rSys;
    /* NVRAM offset[1024] ~ offset[1279] */
    /* NVRAM offset[1280] ~ offset[1535] */
    /* NVRAM offset[1536] ~ offset[1791] */
    /* NVRAM offset[1792] ~ offset[2047] */
    UINT_8 aucReserved1[1116];
} WIFI_SOC1_0_NVRAM_PARAM_STRUCT_T, *P_WIFI_SOC1_0_NVRAM_PARAM_STRUCT_T,
    WIFI_CFG_PARAM_STRUCT, *P_WIFI_CFG_PARAM_STRUCT;
/*----------------------------------------------------------------------------*/
/*!
*  NVRAM data structure defination---------End
*/
/*----------------------------------------------------------------------------*/

typedef struct _WIFI_CUSTOM_PARAM_STRUCT
{
    UINT_32             u4Resv;         /* Reserved */
} WIFI_CUSTOM_PARAM_STRUCT;


/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#define CFG_FILE_WIFI_REC_SIZE           sizeof(WIFI_CFG_PARAM_STRUCT)
#define CFG_FILE_WIFI_REC_TOTAL		       1

#define CFG_FILE_WIFI_CUSTOM_REC_SIZE    sizeof(WIFI_CUSTOM_PARAM_STRUCT)
#define CFG_FILE_WIFI_CUSTOM_REC_TOTAL   1

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/


#endif


