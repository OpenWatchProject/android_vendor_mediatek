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

#ifndef RIL_NW_H
#define RIL_NW_H 1

extern void requestSignalStrength(void * data, size_t datalen, RIL_Token t);
extern void requestRegistrationState(void * data, size_t datalen, RIL_Token t);
extern void requestGprsRegistrationState(void * data, size_t datalen, RIL_Token t);
extern void requestOperator(void * data, size_t datalen, RIL_Token t);
extern void requestRadioPower(void * data, size_t datalen, RIL_Token t);
extern void requestQueryNetworkSelectionMode(void * data, size_t datalen, RIL_Token t);
extern void requestSetNetworkSelectionAutomatic(void * data, size_t datalen, RIL_Token t);
extern void requestSetNetworkSelectionManual(void * data, size_t datalen, RIL_Token t);
extern void requestQueryAvailableNetworks(void * data, size_t datalen, RIL_Token t);
extern void requestSetBandMode(void * data, size_t datalen, RIL_Token t);
extern void requestQueryAvailableBandMode(void * data, size_t datalen, RIL_Token t);
extern void requestSetPreferredNetworkType(void * data, size_t datalen, RIL_Token t);
extern void requestGetPreferredNetworkType(void * data, size_t datalen, RIL_Token t);
extern void requestGetNeighboringCellIds(void * data, size_t datalen, RIL_Token t);
extern void requestSetLocationUpdates(void * data, size_t datalen, RIL_Token t);
extern void requestVoiceRadioTech(void * data, size_t datalen, RIL_Token t);
extern void requestGetCellInfoList(void * data, size_t datalen, RIL_Token t);
extern void requestSetCellInfoListRate(void * data, size_t datalen, RIL_Token t);
extern void requestStartNetworkScan(void * data, size_t datalen, RIL_Token t);
extern void requestStopNetworkScan(void * data, size_t datalen, RIL_Token t);
extern void onRadioState(char* urc, RIL_SOCKET_ID rid);
extern void onNetworkStateChanged(char *urc, RIL_SOCKET_ID rid);
extern void onNitzTimeReceived(char *urc, RIL_SOCKET_ID rid);
extern void onSignalStrenth(char* urc, const RIL_SOCKET_ID rid);
extern void onRestrictedStateChanged(RIL_SOCKET_ID rid);
extern void onCellInfoList(char *urc, const RIL_SOCKET_ID rid);
extern void onNetworkScanResult(char *urc, const RIL_SOCKET_ID rid);

/* Add-BY-JUNGO-20101008-CTZV SUPPORT */
extern void onNitzTzReceived(char *urc, RIL_SOCKET_ID rild);
extern void updateNitzOperInfo(RIL_SOCKET_ID rid);
extern void updateSignalStrength();

extern int isRadioOn(RIL_SOCKET_ID rid);
extern int queryRadioState(RILSubSystemId subsystem, RIL_SOCKET_ID rid);
extern int rilNwMain(int request, void *data, size_t datalen, RIL_Token t);
extern int rilNwUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);

//extern int getSingnalStrength(char *line, int *rssi, int *ber, int *rssi_qdbm, int *rscp_qdbm, int *ecn0_qdbm);
extern int getSingnalStrength(char *line, int *response, const RIL_SOCKET_ID rid);
extern void updateSignalStrengthProperty(int rsrp_in_dbm, int rssnr_in_db, const RIL_SOCKET_ID rid);

/* MTK proprietary start */

extern void onSimInsertChanged(const char *s,RIL_SOCKET_ID rid);
extern void onInvalidSimInfo(char *urc,RIL_SOCKET_ID rid);
extern void onNetworkEventReceived(char* urc, const RIL_SOCKET_ID rid);
extern void onModulationInfoReceived(char *urc, const RIL_SOCKET_ID rid);
extern void requestSimReset(RIL_SOCKET_ID rid);
extern void requestSimInsertStatus(RIL_SOCKET_ID rid);
extern void requestSendOplmn(void *data, size_t datalen, RIL_Token t);
extern void requestGetOplmnVersion(void *data, size_t datalen, RIL_Token t);
extern int getPLMNNameFromNumeric(const char *numeric, char *longname, char *shortname, int max_length);
extern int getOperatorNamesFromNumericCode(char *code,char *longname,char *shortname,int max_length,RIL_SOCKET_ID rid);

extern void onConfirmRatBegin(char* urc, const RIL_SOCKET_ID rid);

/// M: EPDG feature. Update PS state from MAL @{
extern void requestMalGprsRegistrationState(void * data, size_t datalen, RIL_Token t);
extern void onMalPsStateChanged(void *data, size_t datalen, RIL_Token t);
extern void enableLteBand8(RIL_SOCKET_ID rid, RILChannelCtx *p_channel);
/// @}

// M: mal restart callback
extern void clearRilNWMalCache();
/// @}
/// M: [Network][C2K]Add for band de-sense feature. @{
extern void requestQueryCurrentBandMode();
extern void onSetBandModeResult();
extern char* calculateBandMode ();
extern void setBandMode(void* param);
/// @}
extern int getRatSwitching();
extern void requestGetPOLCapability(void *data, size_t datalen, RIL_Token t);
extern void requestGetPOLList(void *data, size_t datalen, RIL_Token t);
extern void requestSetPOLEntry(void *data, size_t datalen, RIL_Token t);

// Femtocell feature
extern void isFemtoCell(int regState, int cid, int act, int rid);
extern void requestGetFemtoCellList(void * data, size_t datalen, RIL_Token t);
extern void requestAbortFemtoCellList(void * data, size_t datalen, RIL_Token t);
extern void requestSelectFemtoCell(void * data, size_t datalen, RIL_Token t);
extern void requestQueryFemtoCellSystemSelectionMode(void * data, size_t datalen, RIL_Token t);
extern void requestSetFemtoCellSystemSelectionMode(void * data, size_t datalen, RIL_Token t);
extern void onFemtoCellInfo(char* urc, const RIL_SOCKET_ID rid);
extern void requestAntennaConf(void * data, size_t datalen, RIL_Token t);
extern void requestAntennaInfo(void * data, size_t datalen, RIL_Token t);

extern void onNetworkInfo(char* urc, const RIL_SOCKET_ID rid);
extern void onImsEmergencySupportR9(char* urc, const RIL_SOCKET_ID rid);
void handleCellInfoList(void *param);

///M: [Network][C2K] @{
static void requestSetSvlteRatMode(void * data, size_t datalen, RIL_Token t);
static void onLteRadioCapabilityChanged(char* urc, const RIL_SOCKET_ID rid);
static int switchLteRadioCapability(int svlteMode, int roaming, RIL_Token t);
static int switchRatMode(int preferNwType, int svlteMode, int roaming, RIL_Token t);
static int switchDataOnlyMode(int svlteMode, RIL_Token t);
static int switchSvlteRatMode(int preferNwType, int svlteMode, int volteState, int roaming, RIL_Token t);
static void setLteRadioCapability();
static int isEratExtSupport();
static int getMajorSlotId();
static int switchUtkMode(int radioTechMode, RIL_Token t);
static void setInitFddTddMode(void * data, size_t datalen, RIL_Token t);
static int openBandMode (int bandMode, RIL_Token t);
/// @}
void onSib16TimeInfoReceived(char* urc, const RIL_SOCKET_ID rid);
RIL_Errno handleERATResult(const char* cmd, RIL_Token t);
bool isERATRetrySupport();
int onNwCheckSetUnsolResponseFilter(void *data, size_t datalen, RIL_Token t);
void onRetrySetUnsolResponseFilter(void* param);

static const struct timeval TIMEVAL_1 = {1, 0};

/* MTK proprietary end */

/* RIL Network Structure */
typedef struct
{
int ril_voice_reg_state;
int ril_voice_nw_type;
int ril_data_reg_state;
int ril_data_nw_type;
} ril_nw_service_state;
extern ril_nw_service_state getGsmNwServiceState();

typedef struct
{
    unsigned short arfcn;
    unsigned char bsic;
    unsigned char rxlev;
} gas_nbr_cell_meas_struct;

typedef struct
{
    unsigned short mcc;
    unsigned short mnc;
    unsigned short lac;
    unsigned short ci;
} global_cell_id_struct;

typedef struct
{
    unsigned char nbr_meas_num;
    gas_nbr_cell_meas_struct nbr_cells[15];
} gas_nbr_meas_struct;

typedef struct
{
    global_cell_id_struct gci;
    unsigned char nbr_meas_rslt_index;
} gas_cell_info_struct;

typedef struct
{
    gas_cell_info_struct serv_info;
    unsigned char ta;
    unsigned char ordered_tx_pwr_lev;
    unsigned char nbr_cell_num;
    gas_cell_info_struct nbr_cell_info[6];
    gas_nbr_meas_struct nbr_meas_rslt;
} gas_nbr_cell_info_struct;

typedef struct
{
    unsigned char need_revise; // Lexel: this fake, can remove if have other element
} uas_nbr_cell_info_struct; //Lexel: Not define uas_nbr_cell_info_struct yet

typedef union
{
    gas_nbr_cell_info_struct gas_nbr_cell_info;
    uas_nbr_cell_info_struct uas_nbr_cell_info;
} ps_nbr_cell_info_union_type;

typedef struct {
    int data_reg_state;
    int mcc_mnc;
    int rat;
    int rid;
} ps_state_info;

/* RIL Network Enumeration */

typedef enum
{
    GSM_BAND_900    = 0x02,
    GSM_BAND_1800   = 0x08,
    GSM_BAND_1900   = 0x10,
    GSM_BAND_850    = 0x80
} GSM_BAND_ENUM;

typedef enum
{
    UMTS_BAND_I     = 0x0001,
    UMTS_BAND_II    = 0x0002,
    UMTS_BAND_III   = 0x0004,
    UMTS_BAND_IV    = 0x0008,
    UMTS_BAND_V     = 0x0010,
    UMTS_BAND_VI    = 0x0020,
    UMTS_BAND_VII   = 0x0040,
    UMTS_BAND_VIII  = 0x0080,
    UMTS_BAND_IX    = 0x0100,
    UMTS_BAND_X     = 0x0200
} UMTS_BAND_ENUM;

///M: [Network][C2K] add for and8 desense feature. @{
typedef enum
{
    BM_FOR_DESENSE_RADIO_ON = 200,
    BM_FOR_DESENSE_RADIO_OFF = 201,
    BM_FOR_DESENSE_RADIO_ON_ROAMING = 202,
    BM_FOR_DESENSE_B8_OPEN = 203
} BAND_MODE;
/// @}

typedef enum RadioTechMode {
    RADIO_TECH_MODE_UNKNOWN  = 1,
    RADIO_TECH_MODE_CSFB  = 2,
    RADIO_TECH_MODE_SVLTE = 3
} radio_tech_mode;


/* RIL Network Constant */

#define NW_CHANNEL_CTX getRILChannelCtxFromToken(t)

#define RIL_NW_ALL_RESTRICTIONS (\
    RIL_RESTRICTED_STATE_CS_ALL | \
    RIL_RESTRICTED_STATE_CS_NORMAL | \
    RIL_RESTRICTED_STATE_CS_EMERGENCY | \
    RIL_RESTRICTED_STATE_PS_ALL)

#define RIL_NW_ALL_CS_RESTRICTIONS (\
    RIL_RESTRICTED_STATE_CS_ALL | \
    RIL_RESTRICTED_STATE_CS_NORMAL | \
    RIL_RESTRICTED_STATE_CS_EMERGENCY)

#define RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS (\
    RIL_RESTRICTED_STATE_CS_ALL | \
    RIL_RESTRICTED_STATE_PS_ALL)

#define MAX_OPER_NAME_LENGTH    50

#define PROPERTY_VOLTE_ATTACHED_CS    "persist.vendor.radio.volte_attached_cs"

#define PROPERTY_SERIAL_NUMBER "vendor.gsm.serial"

#define PROPERTY_RIL_TESTSIM "vendor.gsm.sim.ril.testsim"
#define PROPERTY_RIL_TESTSIM_2 "vendor.gsm.sim.ril.testsim.2"


#define RIL_NW_NUM              MAX_SIM_COUNT
#define RIL_NW_INIT_MUTEX       {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,\
                                 PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER}
#define RIL_NW_INIT_INT         {0, 0, 0, 0}
#define RIL_NW_INIT_STRING      {{0},{0},{0},{0}}
#define RIL_NW_INIT_STRUCT      {NULL, NULL, NULL, NULL}
#define RIL_NW_INIT_STATE   {RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS, RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS,\
                             RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS, RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS}
#define RIL_NW_INIT_SIGNAL      {99, -1,\
                                 -1, -1,\
                                 -1, -1, -1,\
                                 99, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF,\
                                 0x7FFFFFFF}

#define MAX_SIGNAL_DATA_SIZE        21

#define MAX_NITZ_TZ_DST_LENGTH      10
#define PROPERTY_NITZ_OPER_CODE     "persist.vendor.radio.nitz_oper_code"
#define PROPERTY_NITZ_OPER_LNAME    "persist.vendor.radio.nitz_oper_lname"
#define PROPERTY_NITZ_OPER_SNAME    "persist.vendor.radio.nitz_oper_sname"

#define PROPERTY_WFC_STATE          "persist.vendor.radio.wfc_state"
#define PROPERTY_MTK_DATA_TYPE      "persist.vendor.radio.mtk_data_type"

// Set auto mode only without network selection
#define PROPERTY_SET_AUTO_MODE_ONLY "vendor.ril.nw.auto_selection.mode"

/*RIL Network MACRO */
#define cleanCurrentRestrictionState(flags,x) (ril_nw_cur_state[x] &= ~flags)
#define setCurrentRestrictionState(flags,x) (ril_nw_cur_state[x] |= flags)

static void *noopRemoveNetworkWarning( void *a ) { return a; }
#define RIL_NW_UNUSED_PARM(a) noopRemoveNetworkWarning((void *)&(a));

typedef enum
{
    NO_SIM_INSERTED     = 0x00,
    SIM1_INSERTED       = 0x01,
    SIM2_INSERTED       = 0x02,
    SIM3_INSERTED       = 0x04,
    SIM4_INSERTED       = 0x08,
    DUAL_SIM_INSERTED   = (SIM1_INSERTED | SIM2_INSERTED),
    TRIPLE_SIM_INSERTED   = (SIM1_INSERTED | SIM2_INSERTED | SIM3_INSERTED),
    QUAD_SIM_INSERTED   = (SIM1_INSERTED | SIM2_INSERTED | SIM3_INSERTED| SIM4_INSERTED)
} sim_inserted_status_enum;

/// M: [Network][C2K]  @{
// add SVLTE rat mode
typedef enum
{
    SVLTE_RAT_MODE_4G = 0,
    SVLTE_RAT_MODE_3G = 1,
    SVLTE_RAT_MODE_4G_DATA_ONLY = 2,
    SVLTE_RAT_MODE_LTE_C2K = 3,
    SVLTE_RAT_MODE_STK_SWITCH = 4,
    SVLTE_RAT_MODE_IR_SUSPEND = 5,
    SVLTE_RAT_MODE_LTE_ONLY = 6
} svlte_rat_mode;

// add svlte roaming mode
typedef enum RoamingMode {
    ROAMING_MODE_HOME = 0,
    ROAMING_MODE_NORMAL_ROAMING = 1,
    ROAMING_MODE_JPKR_CDMA = 2,    // only for 4M version.
    ROAMING_MODE_UNKNOWN = 3,
} roaming_mode;

typedef enum {
    SVLTE_FDD_ONLY_MODE = 0,
    SVLTE_FDD_TDD_MODE = 1
} band_mode;
/// @}

// Femtocell feature
typedef struct {
    int domain;
    int state;
    char plmn_id[MAX_OPER_NAME_LENGTH];
    char optrAlphaLong[MAX_OPER_NAME_LENGTH];
    int act;
    int is_femtocell;
    int is_csg_cell;
    int csg_id;
    int csg_icon_type;
    char hnbName[MAX_OPER_NAME_LENGTH];
    int cause;
} ril_nw_femto_cell;

#endif /* RIL_NW_H */

