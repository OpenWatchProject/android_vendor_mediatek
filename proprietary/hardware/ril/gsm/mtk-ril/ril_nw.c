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

#include <telephony/mtk_ril.h>
#include <libmtkrilutils.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include "atchannels.h"
#include "at_tok.h"
#include "misc.h"
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>
#include <ctype.h>

/*ADD-BEGIN-JUNGO-20101008-CTZV support */
#include <time.h>

#include <ril_callbacks.h>
#include <ril_nw.h>
#include <ril_nws.h>
#include <ril_cc.h>
#include <ril_wp.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RIL-NW"
/// M: [Network][C2K] for getting  supported rat for project. @{
#include <ratconfig.h>
/// @}
#include <log/log.h>

#include <cutils/properties.h>
#include "ratconfig.h"

/* Global data Begin */

/* NITZ Operator Name */
static pthread_mutex_t ril_nw_nitzName_mutex[MAX_SIM_COUNT] = RIL_NW_INIT_MUTEX;
static char ril_nw_nitz_oper_code[MAX_SIM_COUNT][MAX_OPER_NAME_LENGTH] = RIL_NW_INIT_STRING;
static char ril_nw_nitz_oper_lname[MAX_SIM_COUNT][MAX_OPER_NAME_LENGTH]= RIL_NW_INIT_STRING;
static char ril_nw_nitz_oper_sname[MAX_SIM_COUNT][MAX_OPER_NAME_LENGTH]= RIL_NW_INIT_STRING;


/* Restrition State */
int ril_nw_cur_state[RIL_NW_NUM] = RIL_NW_INIT_STATE;
int ril_prev_nw_cur_state[RIL_NW_NUM] = RIL_NW_INIT_STATE;

/* GPRS network registration status */
int ril_data_reg_status[RIL_NW_NUM] = RIL_NW_INIT_INT;

int s_signal_response[MAX_SIM_COUNT][MAX_SIGNAL_DATA_SIZE] = {{0}};

/// M: EPDG feature. PS state from MAL @{
int ril_mal_data_reg_status[RIL_NW_NUM] = RIL_NW_INIT_INT;
/// @}

/// M: Local Time VZ_REQ_LTEDATA_6793 @{
static int bSIB16Received = 0;
static int bNitzTzAvailble = 0;
static int bNitzDstAvailble = 0;
static char ril_nw_nitz_tz[MAX_NITZ_TZ_DST_LENGTH] = RIL_NW_INIT_INT;
static char ril_nw_nitz_dst[MAX_NITZ_TZ_DST_LENGTH] = RIL_NW_INIT_INT;
/// @}

int ims_ecc_state[MAX_SIM_COUNT]= {0};


// ALPS00353868 START
static int plmn_list_format[RIL_NW_NUM] = RIL_NW_INIT_INT;
// ALPS00353868 END
//ALPS03269579 START
#define PROPERTY_GSM_WFC_STATUS    "vendor.gsm.wfc.status"
int bEopsSupport = 0;
int plmnListOngoing = 0;
int plmnListAbort = 0;

int sLastReqRatType[RIL_SOCKET_NUM] = {[0 ... RIL_SOCKET_NUM-1] = -1};

/// M:[Network][C2K]Add for band8 desense feature. @{
int bands[4] = RIL_NW_INIT_INT;
int support_bands[4] = RIL_NW_INIT_INT;
int band_mode_request = -1;
int force_switch = 0;
int detachCount = 0;
/// @}
///M: [Network][C2K] Add for CSRA URC. @{
int csraCmdParams[7] = {0};
/// @}

int bPSBEARERSupport = 1;
int s_emergencly_only[RIL_SOCKET_NUM] = {0};

extern int max_pdn_support_23G;
extern int max_pdn_support_4G;

ril_nw_service_state gsm_nw_service_state = {
    0, 0, 0, 0
};

int bCREGType3Support = 1;
int s_reg_state[RIL_SOCKET_NUM] = {0};
int bEmergencyOnly[RIL_SOCKET_NUM] = {0};
/*RIL Network Static functions */

static const RIL_SOCKET_ID s_pollSimId = RIL_SOCKET_1;
static const RIL_SOCKET_ID s_pollSimId2 = RIL_SOCKET_2;
static const RIL_SOCKET_ID s_pollSimId3 = RIL_SOCKET_3;
static const RIL_SOCKET_ID s_pollSimId4 = RIL_SOCKET_4;
static int s_erprat_resend_count = 0;

static const struct timeval TIMEVAL_0 = {0, 0};
static const struct timeval TIMEVAL_5 = {5, 0};
extern const struct timeval TIMEVAL_1;

//Femtocell feature
int csgListAbort = 0;
int csgListOngoing = 0;
pthread_mutex_t ril_femtocell_mutex[MAX_SIM_COUNT] = RIL_NW_INIT_MUTEX;
/* <domain>,<state>,<lognname>,<plmn_id>,<act>,<is_csg_cell/is_femto_cell>,<csg_id>,<csg_icon_type>,<hnb_name>,<cause> */
ril_nw_femto_cell gFemtoCellInfo[MAX_SIM_COUNT];
char *sOp12Plmn[] = {
    "311278", "311483", "310004", "311283", "311488",
    "310890", "311272", "311288", "311277", "311482",
    "311282", "311487", "310590", "311287", "311271",
    "311276", "311481", "311281", "311486", "310013",
    "311286", "311270", "311275", "311480", "311280",
    "311485", "310012", "311285", "311110", "311274",
    "311390", "311279", "311484", "310010", "311284",
    "311489", "310910", "311273", "311289"
};

static const char PROPERTY_NW_MCCMNC[4][40] = {
    "vendor.ril.nw.operator.mccmnc.1",
    "vendor.ril.nw.operator.mccmnc.2",
    "vendor.ril.nw.operator.mccmnc.3",
    "vendor.ril.nw.operator.mccmnc.4",
};

static const char PROPERTY_NW_LTE_SIGNAL[MAX_SIM_COUNT][40] = {
    "vendor.ril.nw.signalstrength.lte.1",
    "vendor.ril.nw.signalstrength.lte.2",
    "vendor.ril.nw.signalstrength.lte.3",
    "vendor.ril.nw.signalstrength.lte.4",
};

extern int inCallNumberPerSim[MAX_SIM_COUNT];
extern int requestSwitchStkUtkMode(int mode, RIL_Token t);

// MTK_TC1_FEATURE for Antenna Testing start
int antennaTestingType = 0;
// MTK_TC1_FEATURE for Antenna Testing end

/* modem ECELL ext3 ext4 support
 * value: 0 not surrpot
 *        1 support
 **/
int ECELLext3ext4Support[MAX_SIM_COUNT] = {-1, -1};

RIL_UnsolicitedResponseFilter last_filter = 0;

// Have to call free() after getting the resp
char* getMask(int i) {
    char* d = NULL;
    asprintf(&d, "%X", i);
    if (d == NULL) {
        LOGE("getMask error");
        return NULL;
    }
    size_t size_ = strlen(d);
    if (size_ == 1 || size_ == 2) {
        d[0] = '*';
        return d;
    }
    for (int i = 0; i < (size_/2); i++) {
        d[i] = '*';
    }
    return d;
}

typedef struct SPNTABLE {
    char mccMnc[8];
    char spn[MAX_OPER_NAME_LENGTH+2];
} SpnTable;

static SpnTable s_mtk_spn_table[] = {
#include "mtk_spn_table.h"
};

// ALPS00353868 START
int setPlmnListFormat(RIL_SOCKET_ID rid, int format){
    if(rid < RIL_NW_NUM){
        /* 0: standard +COPS format , 1: standard +COPS format plus <lac> */
        plmn_list_format[rid] = format;
    }
    return 1;
}
// ALPS00353868 END

int getPLMNNameFromNumeric(const char *numeric, char *longname, char *shortname, int max_length) {
    int i = 0, length = sizeof(s_mtk_spn_table)/sizeof(s_mtk_spn_table[0]);
    // LOGD("ENTER getPLMNNameFromNumeric(), plmn table length = %d", length);
    longname[0] = '\0';
    shortname[0] = '\0';
    for (i=0; i < length; i++) {
        if (0 == strcmp(numeric, s_mtk_spn_table[i].mccMnc)) {
            strncpy(longname, s_mtk_spn_table[i].spn, max_length);
            longname[max_length - 1] = '\0';
            strncpy(shortname, s_mtk_spn_table[i].spn, max_length);
            shortname[max_length - 1] = '\0';
            LOGD("getPLMNNameFromNumeric: s_mtk_spn_table longname = %s, shortname= %s",
                    longname, shortname);
            return 0;
        }
    }
    strncpy(longname, numeric, max_length);
    longname[max_length - 1] = '\0';
    strncpy(shortname, numeric, max_length);
    shortname[max_length - 1] = '\0';
    LOGD("getPLMNNameFromNumeric: s_mtk_spn_table not found %s", numeric);
    return -1;
}

/*
int getSingnalStrength(char *line, int *rssi, int *ber, int *rssi_qdbm, int *rscp_qdbm, int *ecn0_qdbm)
{
    int err;
    err = at_tok_start(&line);
    if (err < 0) return -1;

    err = at_tok_nextint(&line, rssi);
    if (err < 0) return -1;
    else if( (*rssi < 0  || *rssi > 31) && *rssi != 99)   // check if the value is valid or not
    {
        LOGE("Recevice an invalid value from modem for <rssi>");
        return -1;
    }

    err = at_tok_nextint(&line, ber);
    if (err < 0) return -1;
    else if( (*ber < 0  || *ber > 7) && *ber != 99)   // check if the value is valid or not
    {
        LOGE("Recevice an invalid value from modem for <ber>");
        return -1;
    }
    err = at_tok_nextint(&line, rssi_qdbm);
    if (err < 0)
    {
        LOGE("Recevice an invalid value from modem for <rssi_qdbm>");
        return -1;
    }
    err = at_tok_nextint(&line, rscp_qdbm);

    // ALPS00506562
    //if (err < 0) *rscp_qdbm=-1;
    if (err < 0) *rscp_qdbm=0;

    err = at_tok_nextint(&line, ecn0_qdbm);

    // ALPS00506562
    //if (err < 0) *ecn0_qdbm= -1;
    if (err < 0) *ecn0_qdbm= 0;

    return 0;

}
*/

void updateSignalStrengthProperty(int rsrp_in_dbm, int rssnr_in_db, const RIL_SOCKET_ID rid)
{
    //Use int max, as -1 is a valid value in signal strength
    int INVALID = 0x7FFFFFFF;

    int rsrp = INVALID;
    int rssnr = INVALID;
    char *lte_signal = NULL;
    if (rsrp_in_dbm != INVALID) {
        rsrp = rsrp_in_dbm * (-1);
    }
    if (rssnr_in_db != INVALID) {
        rssnr = rssnr_in_db / 10;
    }

    asprintf(&lte_signal, "%d,%d", rsrp, rssnr);

    property_set(PROPERTY_NW_LTE_SIGNAL[rid], lte_signal);

    free(lte_signal);
}

/* Android framework expect spec 27.007  AT+CSQ <rssi> 0~31 format when handling 3G signal strength.
   So we convert 3G signal to <rssi> in RILD */
int convert3GRssiValue(int rscp_in_dbm)
{
    int rssi = 0;
    int INVALID = 0x7FFFFFFF;

    if (rscp_in_dbm == INVALID) {
        return rssi;
    }

    rssi = (rscp_in_dbm + 113) / 2;

    if (rssi > 31) {
        rssi = 31;
    } else if (rssi < 0) {
        rssi = 0;
    }
    return rssi;
}

int getSingnalStrength(char *line, int *response, const RIL_SOCKET_ID rid)
{
    int err;
    int hasMoreThanFiveElements = 0;
    int hasMoreThanThreeElements = 0;

    //Use int max, as -1 is a valid value in signal strength
    int INVALID = 0x7FFFFFFF;

    // R8 modem
    // AT+ECSQ:<rssi>,<bar>,<rssi_qdbm>,<rscp_qdbm>,<ecn0_qdbm>
    // GSM : +ECSQ:<rssi>,<bar>,<rssi_qdbm>
    // UMTS: +ECSQ:<rssi>,<bar>,<rssi_qdbm>,<rscp_qdbm>,<ecn0_qdbm>

    // For LTE (MT6592)
    // AT+ECSQ:<sig1>,<sig2>,<rssi_in_qdbm>,<rscp_in_qdbm>,<ecn0_in_qdbm>,<rsrq_in_qdbm>,<rsrp_in_qdbm>,<Act>
    // GSM : +ECSQ:<rxlev>,<bar>,<rssi_in_qdbm>,<rscp_in_qdbm>,1,1,1,<AcT:0>
    // UMTS: +ECSQ:<rscp>,<ecn0>,1,<rscp_in_qdbm>,<ecn0_in_qdbm>,1,1,<AcT:2>
    // LTE : +ECSQ:<rsrq>,<rsrp>,1,1,1,<rsrq_in_qdbm>,<rsrp_in_qdbm>,<AcT:7>

    int read_sig1 = 0;
    int read_sig2 = 0;
    int read_rssi_in_qdbm = 0;
    int read_rscp_in_qdbm = 0;
    int read_ecn0_in_qdbm = 0;
    int read_rsrq_in_qdb = 0;
    int read_rsrp_in_qdbm = 0;
    int read_act = 0;
    int read_rssnr_in_qdb = 0;

    int *gsm_signal_strength = &response[0];
    int *gsm_bit_error_rate = &response[1];
    int *gsm_timing_advance = &response[2];
    int *cdma_dbm = &response[3];
    int *cdma_ecio = &response[4];
    int *evdo_dbm = &response[5];
    int *evdo_ecio = &response[6];
    int *evdo_snr = &response[7];
    int *lte_signal_strength = &response[8];
    int *lte_rsrp = &response[9];
    int *lte_rsrq = &response[10];
    int *lte_rssnr = &response[11];
    int *lte_cqi = &response[12];
    int *lte_timing_advance = &response[13];
    int *tdscdma_signal_strength = &response[14];
    int *tdscdma_bit_error_rate = &response[15];
    int *tdscdma_rscp = &response[16];
    int *wcdma_signal_strength = &response[17];
    int *wcdma_bit_error_rate = &response[18];
    int *wcdma_scdma_rscp = &response[19];
    int *wcdma_ecno = &response[20];

    *gsm_signal_strength = 99;
    *gsm_bit_error_rate = 99;
    *gsm_timing_advance = -1;
    *cdma_dbm = -1;
    *cdma_ecio = -1;
    *evdo_dbm = -1;
    *evdo_ecio = -1;
    *evdo_snr = -1;
    *lte_signal_strength = 99;
    *lte_rsrp = INVALID;
    *lte_rsrq = INVALID;
    *lte_rssnr = INVALID;
    *lte_cqi = INVALID;
    *lte_timing_advance = INVALID;
    *tdscdma_signal_strength = 99;
    *tdscdma_bit_error_rate = 99;
    *tdscdma_rscp = 255;
    *wcdma_signal_strength = 99;
    *wcdma_bit_error_rate = 99;
    *wcdma_scdma_rscp = 255;
    *wcdma_ecno = 255;

    err = at_tok_start(&line);
    if (err < 0) return -1;

    err = at_tok_nextint(&line, &read_sig1);
    if (err < 0) return -1;

    err = at_tok_nextint(&line, &read_sig2);
    if (err < 0) return -1;

    err = at_tok_nextint(&line, &read_rssi_in_qdbm);
    if (err < 0) {
        return -1;
    }

    // 2G sim has only three parameter,3G sim has five parameters
    err = at_tok_nextint(&line, &read_rscp_in_qdbm);
    if (err < 0) {
        read_rscp_in_qdbm = INVALID;   // for WR8-WCDMA, LR9-WCDMA&LTE
    } else {
        hasMoreThanThreeElements = 1;
    }

    err = at_tok_nextint(&line, &read_ecn0_in_qdbm);
    if (err < 0) {
        read_ecn0_in_qdbm = INVALID;
    }

    if(at_tok_hasmore(&line)){
        // for LTE
        hasMoreThanFiveElements = 1;

        err = at_tok_nextint(&line, &read_rsrq_in_qdb);
        if (err < 0) return -1;
        err = at_tok_nextint(&line, &read_rsrp_in_qdbm);
        if (err < 0) return -1;
        err = at_tok_nextint(&line, &read_act);
        if (err < 0) return -1;
        err = at_tok_nextint(&line, &read_rssnr_in_qdb);
        if (err < 0) return -1;
    }

    if (hasMoreThanFiveElements != 1) {
        // for 2/3G only modem ==> +ECSQ: <rssi>,<ber>
        if (read_sig1 < 0  || read_sig1 > 31) {
            LOGE("Recevice an invalid value from modem for <rssi>");
            return -1;
        }
        if((read_sig2 < 0 || read_sig2 > 7) && read_sig2 != 99){
            LOGE("Recevice an invalid value from modem for <ber>");
            return -1;
        }
        if (hasMoreThanThreeElements != 1) {
            // GSM
            *gsm_signal_strength = read_sig1;
            *gsm_bit_error_rate = read_sig2;
        } else {
            // Wcdma
            *wcdma_signal_strength = read_sig1;
            // fwk expect: -24 > wcdma_scdma_rscp-120 > -120
            *wcdma_scdma_rscp = (read_rscp_in_qdbm/4) + 120;
            *wcdma_ecno = (read_ecn0_in_qdbm/4)*(2) + 49;
        }
    } else {
        if ((read_act == 0) || (read_act == 1) || (read_act == 3)) {  // for GSM: <rxlevel>,<ber>
            if (read_sig1 < 0 || read_sig1 > 63) {
                LOGE("Recevice an invalid value from modem for <rxlevel>");
                return -1;
            }
            if((read_sig2 < 0 || read_sig2 > 7) && read_sig2 != 99){
                LOGE("Recevice an invalid value from modem for <ber>");
                return -1;
            }
            *gsm_signal_strength = ((read_rssi_in_qdbm/4)+113)/2;
            // fwk expects ASU (0-31, 99) as defined in TS 27.007 8.69
            if (*gsm_signal_strength > 31) *gsm_signal_strength = 31;
            if (*gsm_signal_strength < 0) *gsm_signal_strength = 0;
            *gsm_bit_error_rate = read_sig2;
            updateSignalStrengthProperty(*lte_rsrp, *lte_rssnr, rid);
        } else if ((read_act == 2) || ((read_act >= 4) && (read_act <= 6))) { // for UMTS:<rscp>,<ecno>
            if (read_sig1 < 0 || read_sig1 > 96) {
                LOGE("Recevice an invalid value from modem for <rscp>");
                return -1;
            }
            if((read_sig2 < 0 || read_sig2 > 49) && read_sig2 != 255){
                LOGE("Recevice an invalid value from modem for <ecno>");
                return -1;
            }
            int rscp_in_dbm = (read_rscp_in_qdbm/4)*(-1);
            if (rscp_in_dbm > 120) {
                rscp_in_dbm = 120;
            } else if (rscp_in_dbm < 25) {
                rscp_in_dbm = 25;
            }
            int rssi = convert3GRssiValue(rscp_in_dbm*(-1));
            *wcdma_signal_strength = rssi;
            *wcdma_scdma_rscp = read_sig1;
            *wcdma_ecno = read_sig2;
            updateSignalStrengthProperty(*lte_rsrp, *lte_rssnr, rid);
        } else if (read_act == 7) { // for LTE:<rsrq>,<rsrp>
            if((read_sig1 < 0  || read_sig1 > 34) && read_sig1 != 255){
                LOGE("Recevice an invalid value from modem for <rsrq>");
                return -1;
            }
            if (read_sig2 < 0 || read_sig2 > 97) {
                LOGE("Recevice an invalid value from modem for <rsrp>");
                return -1;
            }
            *lte_signal_strength = 99;
            int rsrp_in_dbm = (read_rsrp_in_qdbm/4)*(-1);
            if (rsrp_in_dbm > 140) rsrp_in_dbm = 140;
            else if (rsrp_in_dbm < 44) rsrp_in_dbm = 44;
            *lte_rsrp = rsrp_in_dbm;
            int rsrq_in_dbm = (read_rsrq_in_qdb/4)*(-1);
            if (rsrq_in_dbm > 20) rsrq_in_dbm = 20;
            else if (rsrq_in_dbm < 3) rsrq_in_dbm = 3;
            *lte_rsrq = rsrq_in_dbm;
            int rssnr_in_01dbm = (read_rssnr_in_qdb*10)/4;
            if (rssnr_in_01dbm > 300) {
                rssnr_in_01dbm = 300;
            } else if (rssnr_in_01dbm < -200) {
                rssnr_in_01dbm = -200;
            }
            *lte_rssnr = rssnr_in_01dbm;
            *lte_cqi = 0;
            *lte_timing_advance = 0;
            updateSignalStrengthProperty(rsrp_in_dbm, rssnr_in_01dbm, rid);
        } else {
            LOGE("Recevice an invalid value from modem for <act>");
            return -1;
        }
    }
    return 0;
}

int getOperatorNamesFromNumericCode(
    char *code,
    char *longname,
    char *shortname,
    int max_length,
    RIL_SOCKET_ID rid)
{
    char nitz[PROPERTY_VALUE_MAX];
    char oper_file_path[PROPERTY_VALUE_MAX];
    char oper[128], name[MAX_OPER_NAME_LENGTH];
    char *line, *tmp;
    FILE *list;
    int err;

    char *oper_code, *oper_lname, *oper_sname;

    if (max_length > MAX_OPER_NAME_LENGTH)
    {
        LOGE("The buffer size %d is not valid. We only accept the length less than or equal to %d",
             max_length, MAX_OPER_NAME_LENGTH);
        return -1;
    }

    oper_code = ril_nw_nitz_oper_code[rid];
    oper_lname = ril_nw_nitz_oper_lname[rid];
    oper_sname = ril_nw_nitz_oper_sname[rid];

    longname[0] = '\0';
    shortname[0] = '\0';

    pthread_mutex_lock(&ril_nw_nitzName_mutex[rid]);
    LOGD("Get ril_nw_nitzName_mutex in the getOperatorNamesFromNumericCode");

    /* Check if there is a NITZ name*/
    /* compare if the operator code is the same with <oper>*/
    if(strncmp(code, oper_code, max_length) == 0) {
        /* there is a NITZ Operator Name*/
        /*get operator code and name*/
        /*set short name with long name when short name is null and long name isn't, and vice versa*/
        int nlnamelen = strlen(oper_lname);
        int nsnamelen = strlen(oper_sname);
        if(nlnamelen != 0 && nsnamelen != 0)
        {
            strncpy(longname,oper_lname, max_length);
            strncpy(shortname, oper_sname, max_length);
        }
        else if(strlen(oper_sname) != 0)
        {
            strncpy(longname, oper_sname, max_length);
            strncpy(shortname, oper_sname, max_length);
        }
        else if(strlen(oper_lname) != 0)
        {
            strncpy(longname, oper_lname, max_length);
            strncpy(shortname, oper_lname, max_length);
        }

        LOGD("Return NITZ Operator Name: %s %s %s, lname length: %d, sname length: %d", oper_code,
                                                                                        oper_lname,
                                                                                        oper_sname,
                                                                                        nlnamelen,
                                                                                        nsnamelen);
    }
    else
    {
        //strcpy(longname, code);
        //strcpy(shortname, code);
        getPLMNNameFromNumeric(code, longname, shortname, max_length);
    }

    pthread_mutex_unlock(&ril_nw_nitzName_mutex[rid]);
    return 0;
}

void updateNitzOperInfo(RIL_SOCKET_ID rid) {
    int phoneId = getMappingSIMByCurrentMode(rid);
    getMSimProperty(phoneId, PROPERTY_NITZ_OPER_CODE, ril_nw_nitz_oper_code[rid]);
    getMSimProperty(phoneId, PROPERTY_NITZ_OPER_LNAME, ril_nw_nitz_oper_lname[rid]);
    getMSimProperty(phoneId, PROPERTY_NITZ_OPER_SNAME, ril_nw_nitz_oper_sname[rid]);
    //LOGD("[RIL%d] NITZ Operator Name : %s %s %s", (getMappingSIMByCurrentMode(rid)+1),
    //    ril_nw_nitz_oper_code[rid], ril_nw_nitz_oper_lname[rid], ril_nw_nitz_oper_sname[rid]);
}

/*RIL Network functions */

/** returns 1 if on, 0 if off, 4 for airplane mode. and -1 on error */
int queryRadioState(RILSubSystemId subsystem, RIL_SOCKET_ID rid)
{
    ATResponse *p_response = NULL;
    int err;
    char *line;
    int ret;
    RILChannelCtx *pChannel = getChannelCtxbyProxy();

    err = at_send_command_singleline("AT+CFUN?", "+CFUN:", &p_response, pChannel);

    if (err < 0 || p_response->success == 0) {
        // assume radio is off
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &ret);
    if (err < 0) goto error;

    at_response_free(p_response);

    return ret;

error:
    at_response_free(p_response);
    return -1;
}


/*RIL Network functions */

/** returns 1 if on, 0 if off, and -1 on error */
int isRadioOn(RIL_SOCKET_ID rid)
{
    ATResponse *p_response = NULL;
    int err;
    char *line;
    int ret;
    RIL_NW_UNUSED_PARM(rid);

    err = at_send_command_singleline("AT+CFUN?", "+CFUN:", &p_response, getChannelCtxbyProxy());

    if (err < 0 || p_response->success == 0) {
        // assume radio is off
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &ret);
    if (err < 0) goto error;

    at_response_free(p_response);

    ret = (ret == 4 || ret == 0) ? 0 :    // phone off
          (ret == 1) ? 1 :              // phone on
          -1;                           // invalid value

    return ret;

error:
    at_response_free(p_response);
    return -1;
}

void requestSignalStrength(void * data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;
    int response[MAX_SIGNAL_DATA_SIZE] = {0};
    int default_response[MAX_SIGNAL_DATA_SIZE] = RIL_NW_INIT_SIGNAL;
    char *line;
    const RIL_SOCKET_ID rid = getRILIdByChannelCtx(NW_CHANNEL_CTX);
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    memset(response, 0, sizeof(response));

    err = at_send_command_singleline("AT+ECSQ", "+ECSQ:", &p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0 ||
            p_response->p_intermediates  == NULL) {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = getSingnalStrength(line, response, rid);

    if (err < 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));

    at_response_free(p_response);
    return;

error:
    LOGW("requestSignalStrength return cache value");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, s_signal_response[rid], sizeof(default_response));
    at_response_free(p_response);
}

static unsigned int convertCSNetworkType(unsigned int uiResponse) {
    unsigned int uiRet = 0;

    /* mapping */
    switch (uiResponse) {
        case 0:     // GSM
        case 1:     // GSM compact
        case 3:     // GSM w/EGPRS
            uiRet = 16;      // GSM
            break;
        case 2:     // UTRAN
        case 4:     // UTRAN w/HSDPA
        case 5:     // UTRAN w/HSUPA
        case 6:     // UTRAN w/HSDPA and HSUPA
            uiRet = 3;        // UMTS
            break;
        // for LTE
        case 7:     // E-UTRAN
            uiRet = 14;       // LTE
            break;
        default:
            uiRet = 0;        // Unknown
            break;
    }

    return uiRet;
}

static unsigned int convertNetworkType(unsigned int uiResponse)
{
    unsigned int uiRet = 0;

    /* mapping */
    switch(uiResponse)
    {
    case 0:     //GSM
    case 1:     //GSM compact
        uiRet = 1;        // GPRS only
        break;
    case 2:     //UTRAN
        uiRet = 3;        // UMTS
        break;
    case 3:     //GSM w/EGPRS
        uiRet = 2;        // EDGE
        break;
    case 4:     //UTRAN w/HSDPA
        uiRet = 9;        // HSDPA
        break;
    case 5:     //UTRAN w/HSUPA
        uiRet = 10;        // HSUPA
        break;
    case 6:     //UTRAN w/HSDPA and HSUPA
        uiRet = 11;        // HSPA
        break;
    //for LTE
    case 7:     //E-UTRAN
        uiRet = 14;        // LTE
        break;
    default:
        uiRet = 0;        // Unknown
        break;
    }

    return uiRet;
}

static unsigned int convertCellSppedSupport(unsigned int uiResponse)
{
    // Cell speed support is bitwise value of cell capability:
    // bit7 0x80  bit6 0x40  bit5 0x20  bit4 0x10  bit3 0x08  bit2 0x04  bit1 0x02  bit0 0x01
    // Dual-Cell  HSUPA+     HSDPA+     HSUPA      HSDPA      UMTS       EDGE       GPRS
    unsigned int uiRet = 0;

    if ((uiResponse & 0x2000) != 0) {
        //support carrier aggregation (LTEA)
        uiRet = 19; // ServiceState.RIL_RADIO_TECHNOLOGY_LTE_CA
    } else if ((uiResponse & 0x1000) != 0) {
        uiRet = 14; // ServiceState.RIL_RADIO_TECHNOLOGY_LTE
    } else if ((uiResponse & 0x80) != 0 ||
            (uiResponse & 0x40) != 0 ||
            (uiResponse & 0x20) != 0) {
        uiRet = 15; // ServiceState.RIL_RADIO_TECHNOLOGY_HSPAP
    } else if ((uiResponse & 0x10) != 0 &&
            (uiResponse & 0x08) != 0) {
        uiRet = 11; // ServiceState.RIL_RADIO_TECHNOLOGY_HSPA
    } else if ((uiResponse & 0x10) != 0) {
        uiRet = 10; // ServiceState.RIL_RADIO_TECHNOLOGY_HSUPA
    } else if ((uiResponse & 0x08) != 0) {
        uiRet = 9;  // ServiceState.RIL_RADIO_TECHNOLOGY_HSDPA
    } else if ((uiResponse & 0x04) != 0) {
        uiRet = 3;  // ServiceState.RIL_RADIO_TECHNOLOGY_UMTS
    } else if ((uiResponse & 0x02) != 0) {
        uiRet = 2;  // ServiceState.RIL_RADIO_TECHNOLOGY_EDGE
    } else if ((uiResponse & 0x01) != 0) {
        uiRet = 1;  // ServiceState.RIL_RADIO_TECHNOLOGY_GPRS
    } else {
        uiRet = 0;  // ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN
    }

    return uiRet;
}

static unsigned int convertPSBearerCapability(unsigned int uiResponse)
{
    /*
     *typedef enum
     *{
  *    L4C_NONE_ACTIVATE = 0,
  *    L4C_GPRS_CAPABILITY,
  *    L4C_EDGE_CAPABILITY,
  *    L4C_UMTS_CAPABILITY,
  *    L4C_HSDPA_CAPABILITY, //mac-hs
  *    L4C_HSUPA_CAPABILITY, //mac-e/es
  *    L4C_HSDPA_HSUPA_CAPABILITY, //mac-hs + mac-e/es

  *    L4C_HSDPAP_CAPABILITY, //mac-ehs
  *    L4C_HSDPAP_UPA_CAPABILITY, //mac-ehs + mac-e/es
  *    L4C_HSUPAP_CAPABILITY, //mac-i/is
  *    L4C_HSUPAP_DPA_CAPABILITY, //mac-hs + mac-i/is
  *    L4C_HSPAP_CAPABILITY, //mac-ehs + mac-i/is
  *    L4C_DC_DPA_CAPABILITY, //(DC) mac-hs
  *    L4C_DC_DPA_UPA_CAPABILITY, //(DC) mac-hs + mac-e/es
  *    L4C_DC_HSDPAP_CAPABILITY, //(DC) mac-ehs
  *    L4C_DC_HSDPAP_UPA_CAPABILITY, //(DC) mac-ehs + mac-e/es
  *    L4C_DC_HSUPAP_DPA_CAPABILITY, //(DC) mac-hs + mac-i/is
  *    L4C_DC_HSPAP_CAPABILITY, //(DC) mac-ehs + mac-i/is
     *    } l4c_data_bearer_capablility_enum;
    */

    unsigned int uiRet = 0;

    switch (uiResponse)
    {
    case 1:
        uiRet = 1;  // ServiceState.RIL_RADIO_TECHNOLOGY_GPRS
        break;
    case 2:
        uiRet = 2;  // ServiceState.RIL_RADIO_TECHNOLOGY_EDGE
        break;
    case 3:
        uiRet = 3;  // ServiceState.RIL_RADIO_TECHNOLOGY_UMTS
        break;
    case 4:
        uiRet = 9;  // ServiceState.RIL_RADIO_TECHNOLOGY_HSDPA
        break;
    case 5:
        uiRet = 10; // ServiceState.RIL_RADIO_TECHNOLOGY_HSUPA
        break;
    case 6:
        uiRet = 11; // ServiceState.RIL_RADIO_TECHNOLOGY_HSPA
        break;
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
        uiRet = 15; // ServiceState.RIL_RADIO_TECHNOLOGY_HSPAP
        break;
    case 18:
        uiRet = 14;
        break;
    default:
        uiRet = 0;  // ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN
        break;
    }

    return uiRet;
}


static unsigned int combineMalCgregState(RIL_SOCKET_ID rid)
{
    unsigned int uiRet = 0;

    if (RatConfig_isC2kSupported()) {
        uiRet = ril_data_reg_status[rid];

        LOGD("combineMalCgregState() rid=%d, uiRet=%d", rid, uiRet);
    } else {
        switch (ril_data_reg_status[rid]) {
            case 1:
            case 5:
                uiRet = ril_data_reg_status[rid];
                break;
            default:
                if (ril_mal_data_reg_status[rid] == 99) {
                    uiRet = 1;
                } else {
                    uiRet = ril_data_reg_status[rid];
                }
                break;
        }

        LOGD("combineMalCgregState() data_reg=%d, mal_data_reg=%d, uiRet=%d",
            ril_data_reg_status[rid], ril_mal_data_reg_status[rid], uiRet);
    }

    return uiRet;

}

static unsigned int convertRegState(unsigned int uiRegState)
{
    unsigned int uiRet = 0;

    switch (uiRegState)
    {
    case 6:         // Registered for "SMS only", home network
        uiRet = 1;  // Registered
        break;
    case 7:         // Registered for "SMS only", roaming
        uiRet = 5;  // roaming
        break;
    case 8:         // attached for emergency bearer service only
        uiRet = 0;  // not registered
        break;
    case 9:         // registered for "CSFB not prefereed", home network
        uiRet = 1;  // Registered
        break;
    case 10:       // registered for "CSFB not prefereed", roaming
        uiRet = 5;  // roaming
        break;
    default:
        uiRet = uiRegState;
        break;
    }

    return uiRet;
}

static unsigned int convertToRilRegState(unsigned int uiRegState,RIL_SOCKET_ID rid)
{
    unsigned int uiRet = 0;

    if (s_emergencly_only[rid] == 1 || ims_ecc_state[rid] == 1) {
        switch (uiRegState)
        {
        case 0:
            uiRet = 10;
            break;
        case 2:
            uiRet = 12;
            break;
        case 3:
            uiRet = 13;
            break;
        case 4:
            uiRet = 14;
            break;
        default:
            uiRet = uiRegState;
            break;
        }
    } else {
        uiRet = uiRegState;
    }
    // LOGD("convertToRilRegState() uiRegState=%d , uiRet=%d ",uiRegState,uiRet);
    return uiRet;
}

bool isVolteAttachPsOverwriteCsState() {
    char prop[PROPERTY_VALUE_MAX] = {0};
    bool toOverwrite = false;

    property_get(PROPERTY_VOLTE_ATTACHED_CS, prop, "");

    if (strcmp(prop, "1") == 0) {
        LOGD("prop=%s, toOverwrite=%d", prop, toOverwrite);
        toOverwrite = true;
    }
    return toOverwrite;
}

void requestRegistrationState(void * data, size_t datalen, RIL_Token t)
{
    int err;
    unsigned int response[15];
    int i = 0;
    char * responseStr[15];
    char *emptyStr = "";
    ATResponse *p_response = NULL;
    const char *cmd = NULL;
    const char *prefix;
    char *line, *p;
    int skip;
    int cause_type;
    int reg_info = 0;
    int ext_info = 0;

    unsigned int log[7] = {0};

    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    for (i = 0; i < 15; i++) {
        responseStr[i] = emptyStr;
    }

    cmd = "AT+CREG?";
    prefix = "+CREG:";

    err = at_send_command_singleline(cmd, prefix, &p_response, NW_CHANNEL_CTX);

    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    /* +CREG: <n>, <stat> */
    /* +CREG: <n>, <stat>, <lac>, <cid>, <Act> */
    // LOGD("+CREG: %s", line);

    /* <n> */
    err = at_tok_nextint(&line, &skip);
    if (err < 0) goto error;
    log[0] = (unsigned int)skip;

    /* <stat> */
    err = at_tok_nextint(&line, (int*) &response[0]);

    if (err < 0 || response[0] > 10 )  //for LTE
    {
        LOGE("The value in the field <stat> is not valid: %d", response[0] );
        goto error;
    }
    log[1] = response[0];

    //For Lte
    response[0] = convertRegState(response[0]);
    //[ALPS01763285] -START : fix ECC button display abnormal
    response[0] = convertToRilRegState(response[0], rid);
    asprintf(&responseStr[0], "%d", response[0]);
    //[ALPS01763285] -END

    if (at_tok_hasmore(&line))
    {
        /* <lac> */
        err = at_tok_nexthexint(&line, (int*)&response[1]);
        if ( err < 0 || (response[1] > 0xffff && response[1] != 0xffffffff) )
        {
            LOGE("The value in the field <lac> is not valid.");
            goto error;
        } else {
            log[2] = response[1];
            if (response[1] != 0xffffffff)
                asprintf(&responseStr[1], "%x", response[1]);
            else
                asprintf(&responseStr[1], "-1");
        }

        /* <cid> */
        err = at_tok_nexthexint(&line, (int*)&response[2]);
        if (err < 0 || (response[2] > 0x0fffffff && response[2] != 0xffffffff) )
        {
            LOGE("The value in the field <cid> is not valid: %d", response[2] );
            goto error;
        } else {
            log[3] = response[2];
            if (response[2] != 0xffffffff)
                asprintf(&responseStr[2], "%x", response[2]);
            else
                asprintf(&responseStr[2], "-1");
        }

        /* <Act> */
        err = at_tok_nextint(&line, (int*)&response[3]);
        if (err < 0)
        {
            LOGE("No act in command");
            goto error;
        }

        if (response[3] > 7)  //for LTE
        {
            LOGE("The value in the act is not valid: %d", response[3] );
            goto error;
        }
        log[4] = response[3];

        /* <Act> mapping */
        if (response[0] != 1 && response[0] != 5)
        {
            response[3] = 0;
        }
        else
        {
            response[3] = convertCSNetworkType(response[3]);
        }
        asprintf(&responseStr[3], "%d", response[3]);

        if (at_tok_hasmore(&line))
        {
            /* <cause_type> */
            err = at_tok_nextint(&line, &cause_type);
            if (err < 0 || cause_type != 0)
            {
                LOGE("The value in the field <cause_type> is not valid: %d", cause_type );
                goto error;
            }
            log[5] = (unsigned int) cause_type;

            // set "" to indicate "the field is not present"
            for (i = 4; i < 13; i++)
            {
                asprintf(&responseStr[i], "");
            }

            /* <reject_cause> */
            err = at_tok_nextint(&line, (int*) &response[13]);
            if (err < 0)
            {
                LOGE("The value in the field <reject_cause> is not valid: %d", response[13] );
                goto error;
            }
            log[6] = response[13];
            //LOGD("cause_type: %d, reject_cause: %d", cause_type, response[13]);
            asprintf(&responseStr[14], "");
        }
    }
    else
    {
        /* +CREG: <n>, <stat> */
        response[1] = -1;
        response[2] = -1;
        response[3] = 0;
        asprintf(&responseStr[1], "%d", response[1]);
        asprintf(&responseStr[2], "%d", response[2]);
        asprintf(&responseStr[3], "%d", response[3]);
    }

    char* lac_s = getMask(log[2]);
    char* cid_s = getMask(log[3]);
    if (lac_s != NULL && cid_s != NULL) {
        LOGD("CREG: n=%u, stat=%u, lac=%s, cid=%s, act=%u, cause_type=%u, reject_cause=%u",
                log[0], // n
                log[1], // stat
                lac_s, // lac
                cid_s, // cid
                log[4], // act
                log[5], // cause_type
                log[6]); // reject_cause
    } else {
        LOGD("CREG: n=%u, stat=%u, act=%u, cause_type=%u, reject_cause=%u",
                log[0], // n
                log[1], // stat
                log[4], // act
                log[5], // cause_type
                log[6]); // reject_cause
    }
    if (lac_s) free(lac_s);
    if (cid_s) free(cid_s);
    // LOGE("rid: %d",rid);
    if (response[0] == 1 || response[0] == 5)
    {
        cleanCurrentRestrictionState(RIL_NW_ALL_CS_RESTRICTIONS, rid);
    }
    else
    {
        setCurrentRestrictionState(RIL_RESTRICTED_STATE_CS_NORMAL, rid);
    }
    onRestrictedStateChanged(rid);
    if (isVolteAttachPsOverwriteCsState()) {
        if (response[0] != 1 && response[0] != 5) {
            // CREG out of service, check if VoLTE is attached
            at_response_free(p_response);
            p_response = NULL;
            // +CIREG: <n>,<reg_info>[,<ext_info>]
            err = at_send_command_singleline("AT+CIREG?", "+CIREG:", &p_response, NW_CHANNEL_CTX);

            if (err != 0 || p_response->success == 0 ||
                    p_response->p_intermediates == NULL) goto error;

            line = p_response->p_intermediates->line;

            err = at_tok_start(&line);
            if (err < 0) goto error;

            /* Enables or disables reporting of changes */
            err = at_tok_nextint(&line, &skip);
            if (err < 0) goto error;

            /* Indicates the IMS registration status */
            err = at_tok_nextint(&line, &reg_info);
            if (err < 0) goto error;

            if (reg_info == 1) {
                /* IMS capability of the MT */
                err = at_tok_nexthexint(&line, &ext_info);
                if (err < 0) goto error;
                if ((ext_info & 0x1) == 1) {
                    // VoLTE attach, query CGREG state and return
                    at_response_free(p_response);
                    p_response = NULL;
                    err = at_send_command_singleline("AT+CGREG?", "+CGREG:",
                            &p_response, NW_CHANNEL_CTX);

                    if (err != 0 || p_response->success == 0 ||
                            p_response->p_intermediates == NULL) goto error;

                    line = p_response->p_intermediates->line;

                    err = at_tok_start(&line);
                    if (err < 0) goto error;

                    /* <n> */
                    err = at_tok_nextint(&line, &skip);
                    if (err < 0) goto error;

                    /* <stat> */
                    err = at_tok_nextint(&line, (int*) &response[0]);
                    if (err < 0 || response[0] > 10) {
                        //for Lte
                        LOGE("The value in the field <stat> is not valid: %d", response[0]);
                        goto error;
                    }

                    //For Lte
                    response[0] = convertRegState(response[0]);
                    if (at_tok_hasmore(&line)) {
                        /* <lac> */
                        err = at_tok_nexthexint(&line, (int*)&response[1]);
                        LOGD("The value in the field <lac> :%d", response[1]);
                        if ( err < 0 || (response[1] > 0xffff && response[1] != 0xffffffff))
                        {
                            LOGE("The value in the field <lac> or <stat> is not valid. <stat>:%d, <lac>:%d",
                                 response[0], response[1] );
                            goto error;
                        }

                        /* <cid> */
                        err = at_tok_nexthexint(&line, (int*)&response[2]);
                        LOGD("cid: %d", response[2] );
                        if (err < 0 || (response[2] > 0x0fffffff && response[2] != 0xffffffff)) {
                            LOGE("The value in the field <cid> is not valid: %d", response[2] );
                            goto error;
                        }

                        /* <Act> */
                        err = at_tok_nextint(&line, (int*) &response[3]);
                        LOGE("The value of act: %d", response[3] );
                        if (err < 0) {
                            LOGE("No act in command");
                            goto error;
                        }
                        if (response[3] > 7) {
                            LOGE("The value in the act is not valid: %d", response[3] );
                            goto error;
                        }

                        /* <Act> mapping */
                        if (response[0] != 1 && response[0] != 5) {
                            response[3] = 0;
                            LOGE("The value in the field <act> is set to 0 when <stat>:%d",
                                    response[0]);
                        } else {
                            response[3] = convertNetworkType(response[3]);
                            LOGD("The value in the field <act> is converted to %d", response[3]);
                        }
                    }
                    for (i = 0; i < 4; i++) {
                        if (responseStr[i] != NULL) {
                            free(responseStr[i]);
                        }
                        asprintf(&responseStr[i], "%d", response[i]);
                    }
                }
            }
        }
    }
    gsm_nw_service_state.ril_voice_reg_state = response[0];
    gsm_nw_service_state.ril_voice_nw_type = response[3];
    if (gsm_nw_service_state.ril_voice_reg_state != 1
            && gsm_nw_service_state.ril_voice_reg_state != 5) {
        gsm_nw_service_state.ril_voice_nw_type = 0;
    }

    // Check femtocell info.
    if (isFemtocellSupport()) {
        pthread_mutex_lock(&ril_femtocell_mutex[rid]);
        isFemtoCell(response[0], response[2], response[3], rid);
        pthread_mutex_unlock(&ril_femtocell_mutex[rid]);
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, 15*sizeof(char*));

    at_response_free(p_response);
    for (i = 0; i < 15; i++)
    {
        if (responseStr[i] != NULL && responseStr[i] != emptyStr) free(responseStr[i]);
    }
    return;

error:
    LOGE("requestRegistrationState must never return an error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

ril_nw_service_state getGsmNwServiceState() {
    return gsm_nw_service_state;
}

void requestGprsRegistrationState(void * data, size_t datalen, RIL_Token t)
{
    /*
     * A +CGREG: answer may have a fifth value that corresponds
     * to the network type, as in;
     *
     * +CGREG: n, stat [,lac, cid [,networkType]]
     */

    /***********************************
    * In the mtk solution, we do not need to handle the URC
    * Since the URC will received in the URC channel.
    * So we don't need to follow the above command from google.
    * But we might return <AcT> for +CREG if the stat==2
    * while <lac> is present
    ************************************/

    /* +CGREG: <n>, <stat>, <lac>, <cid>, <Act> */

    int err;
    unsigned int response[11];
    char * responseStr[12];
    char * emptyStr = "";
    ATResponse *p_response = NULL;
    const char *cmd;
    const char *prefix;
    char *line, *p;
    int skip, i;
    int cell_data_speed = 0;
    int max_data_bearer = 0;
    int cause_type;

    //support carrier aggregation (LTEA)
    int ignoreMaxDataBearerCapability = 0;

    unsigned int log[8] = {0};

    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    for (i = 0; i < 12; i++) {
        responseStr[i] = emptyStr;
    }

    cmd = "AT+CGREG?";
    prefix = "+CGREG:";

    err = at_send_command_singleline(cmd, prefix, &p_response, NW_CHANNEL_CTX);

    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    // LOGD("+CGREG:%s", line);

    /* <n> */
    err = at_tok_nextint(&line, &skip);
    if (err < 0) goto error;
    log[0] = (unsigned int) skip;

    /* <stat> */
    err = at_tok_nextint(&line, (int*) &response[0]);

    if (err < 0 || response[0] > 10 )  //for Lte
    {
        LOGE("The value in the field <stat> is not valid: %d", response[0] );
        goto error;
    }
    log[1] = response[0];

    //For Lte
    response[0] = convertRegState(response[0] );
    asprintf(&responseStr[0], "%d", response[0]);

    if( at_tok_hasmore(&line) )
    {
        /* <lac> */
        err = at_tok_nexthexint(&line, (int*)&response[1]);
        if ( err < 0 || (response[1] > 0xffff && response[1] != 0xffffffff) )
        {
            LOGE("The value in the field <lac> is not valid");
            goto error;
        } else {
            asprintf(&responseStr[1], "%x", response[1]);
        }
        log[2] = response[1];

        /* <cid> */
        err = at_tok_nexthexint(&line, (int*)&response[2]);
        if (err < 0 || (response[2] > 0x0fffffff && response[2] != 0xffffffff) )
        {
            LOGE("The value in the field <cid> is not valid: %d", response[2] );
            goto error;
        } else {
            asprintf(&responseStr[2], "%x", response[2]);
        }
        log[3] = response[2];

        /* <Act> */
        err = at_tok_nextint(&line, (int*)&response[3]);
        if (err < 0)
        {
            LOGE("No act in command");
            goto error;
        }
        if (response[3] > 7)
        {
            LOGE("The value in the act is not valid: %d", response[3] );
            goto error;
        }
        log[4] = response[3];

        /* <Act> mapping */
        if (response[0] != 1 && response[0] != 5)
        {
            response[3] = 0;
            LOGE("The value in the field <act> is set to 0 when <stat>:%d", response[0]);
        }
        else
        {
            response[3] = convertNetworkType(response[3]);
            LOGE("The value in the field <act> is converted to %d", response[3]);
        }
        asprintf(&responseStr[3], "%d", response[3]);

        if (at_tok_hasmore(&line))
        {
            /* <rac> */
            err = at_tok_nexthexint(&line, &skip);
            if (err < 0) goto error;
            log[5] = (unsigned int) skip;

            if (at_tok_hasmore(&line))
            {
                /* <cause_type> */
                err = at_tok_nextint(&line, &cause_type);
                if (err < 0 || cause_type != 0)
                {
                    LOGE("The value in the field <cause_type> is not valid: %d", cause_type );
                    goto error;
                }
                log[6] = (unsigned int) cause_type;

                /* <reject_cause> */
                err = at_tok_nextint(&line, (int*) &response[4]);
                // LOGD("reject_cause: %d, cause_type: %d", response[4], cause_type);
                if (err < 0)
                {
                    LOGE("The value in the field <reject_cause> is not valid: %d", response[4] );
                    goto error;
                } else {
                    log[7] = response[4];
                    asprintf(&responseStr[4], "%x", response[4]);
                }
            }
        }

        /* maximum number of simultaneous Data Calls  */
        if (response[3] == 14)
            response[5] = max_pdn_support_4G;
        else
            response[5] = max_pdn_support_23G;
        asprintf(&responseStr[5], "%x", response[5]);
    }
    else
    {
        /* +CGREG: <n>, <stat> */
        response[1] = -1;
        response[2] = -1;
        response[3] = 0;
        asprintf(&responseStr[1], "%d", response[1]);
        asprintf(&responseStr[2], "%d", response[2]);
        asprintf(&responseStr[3], "%d", response[3]);
    }

    char* lac_s = getMask(log[2]);
    char* cid_s = getMask(log[3]);
    if (lac_s != NULL && cid_s != NULL) {
        LOGD("CGREG: n=%u, stat=%u, lac=%s, cid=%s, act=%u, rac=%u, "
                "cause_type=%u, reject_cause=%u",
                log[0], // n
                log[1], // stat
                lac_s, // lac
                cid_s, // cid
                log[4], // act
                log[5], // rac
                log[6], // cause_type
                log[7]); // reject_cause
    } else {
        LOGD("CGREG: n=%u, stat=%u, act=%u, rac=%u, cause_type=%u, reject_cause=%u",
                log[0], // n
                log[1], // stat
                log[4], // act
                log[5], // rac
                log[6], // cause_type
                log[7]); // reject_cause
    }
    if (lac_s) free(lac_s);
    if (cid_s) free(cid_s);

    // Query by +PSBEARER as PS registered and R7R8 support
    if (bPSBEARERSupport == 1 &&
            (response[0] == 1 || response[0] == 5)) {
        at_response_free(p_response);
        p_response = NULL;
        cmd = "AT+PSBEARER?";
        prefix = "+PSBEARER:";

        err = at_send_command_singleline(cmd, prefix, &p_response, NW_CHANNEL_CTX);

        if (err != 0 || p_response->success == 0 ||
                p_response->p_intermediates == NULL) goto skipR7R8;

        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) goto skipR7R8;

        /* <cell_data_speed_support> */
        err = at_tok_nextint(&line, &cell_data_speed);
        if (err < 0) goto skipR7R8;

        // <max_data_bearer_capability> is only support on 3G
        if (cell_data_speed >= 0x1000){
            ignoreMaxDataBearerCapability = 1;
        }

        cell_data_speed = convertCellSppedSupport(cell_data_speed);

        /* <max_data_bearer_capability> */
        err = at_tok_nextint(&line, &max_data_bearer);
        if (err < 0) goto skipR7R8;

        if (!ignoreMaxDataBearerCapability) {
            max_data_bearer = convertPSBearerCapability(max_data_bearer);
        } else {
            max_data_bearer = 0;  // ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN
        }

        if (responseStr[3] != NULL) free(responseStr[3]);
        asprintf(&responseStr[3], "%d",
                (cell_data_speed > max_data_bearer)? cell_data_speed : max_data_bearer);
    }


skipR7R8:
    asprintf(&responseStr[11], "%d", ims_ecc_state[rid]);

    // LOGE("rid: %d",rid);
    if (response[0] == 1 || response[0] == 5) {
        cleanCurrentRestrictionState(RIL_RESTRICTED_STATE_PS_ALL, rid);
    } else {
        setCurrentRestrictionState(RIL_RESTRICTED_STATE_PS_ALL, rid);
    }
    onRestrictedStateChanged(rid);

    gsm_nw_service_state.ril_data_reg_state = response[0];
    gsm_nw_service_state.ril_data_nw_type = response[3];
    if (gsm_nw_service_state.ril_data_reg_state != 1
            && gsm_nw_service_state.ril_data_reg_state != 5) {
        gsm_nw_service_state.ril_data_nw_type = 0;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, 11*sizeof(char*));

    at_response_free(p_response);
    for (i = 0; i < 12; i++) {
        if (responseStr[i] != NULL && responseStr[i] != emptyStr) free(responseStr[i]);
    }

    return;

error:
    LOGE("requestRegistrationState must never return an error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void backupOperatorMccMnc(RIL_SOCKET_ID slotId, char* mccmnc)
{
    /* Update property even if it's not a valid mcc-mnc,
       so the obsolete value will not be used */
    property_set(PROPERTY_NW_MCCMNC[slotId], mccmnc);
}

void requestOperator(void * data, size_t datalen, RIL_Token t)
{
    int i, err;
    int skip;
    char *response[3];
    char *emptyStr = "";
    char *line;
    char longname[MAX_OPER_NAME_LENGTH], shortname[MAX_OPER_NAME_LENGTH];
    const RIL_SOCKET_ID rid = getRILIdByChannelCtx(NW_CHANNEL_CTX);
    ATResponse *p_response = NULL;
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    for (i = 0; i < 3; i++) {
        response[i] = emptyStr;
    }

    /* ALPS00574862 Remove redundant +COPS=3,2;+COPS? multiple cmd. set format in  ril initalization */
    err = at_send_command_singleline(
              "AT+COPS?",
              "+COPS:", &p_response, NW_CHANNEL_CTX);

    /* we expect 1 lines here:
     * +COPS: 0,2,"310170"
     */

    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    // LOGD("+COPS:%s", line);

    /* <mode> */
    err = at_tok_nextint(&line, &skip);
    if (err < 0 || skip < 0 || skip > 4)
    {
        LOGE("The <mode> is an invalid value!!!");
        goto error;
    }
    else if(skip == 2)  // deregister
    {
        LOGW("The <mode> is 2 so we ignore the follwoing fields!!!");
    }
    else if (!at_tok_hasmore(&line))
    {
        // If we're unregistered, we may just get
        // a "+COPS: 0" response

        /* To support get PLMN when not registered to network via AT+EOPS?  START */
        // dont report limited service plmn with android P
        if (bEopsSupport == 1) {
            at_response_free(p_response);
            p_response = NULL;

            err = at_send_command_singleline("AT+EOPS?",
                  "+EOPS:", &p_response, NW_CHANNEL_CTX);

            if (err != 0 || p_response->success == 0 ||
                p_response->p_intermediates == NULL) {
                LOGE("EOPS got error response");
                goto error;
            } else {
                line = p_response->p_intermediates->line;

                err = at_tok_start(&line);
                if (err >= 0){
                    err = at_tok_nextint(&line, &skip);
                    /* <mode> */
                    if ((err >= 0) &&( skip >= 0 && skip <= 4 && skip !=2)){
                        /* <format> */
                        err = at_tok_nextint(&line, &skip);
                        if (err >= 0 && skip == 2)
                        {
                            // a "+EOPS: 0, n" response is also possible
                            if (!at_tok_hasmore(&line)) {
                                onMccMncIndication(rid, "");
                                backupOperatorMccMnc(rid, "");
                            } else{
                                /* <oper> */
                                err = at_tok_nextstr(&line, &(response[2]));
                                //LOGD("EOPS Get operator code %s", response[2]);

                                /* Modem might response invalid PLMN ex: "", "000000" , "??????", all convert to "000000" */
                                if(!((*response[2] >= '0') && (*response[2] <= '9'))){
                                    //LOGE("EOPS got invalid plmn response");
                                    response[2] = emptyStr;
                                }
                                // Don't report 000000 to fwk
                                if ((strlen(response[2]) == strlen("000000")) &&
                                    (strncmp(response[2], "000000", 6) == 0)
                                    ) {
                                    response[2] = emptyStr;
                                }

                                onMccMncIndication(rid, response[2]);
                                backupOperatorMccMnc(rid, response[2]);
                                // clear the limited service plmn
                                response[2] = emptyStr;
                            }
                        }
                    }
                }
            }
        }
        /* To support get PLMN when not registered to network via AT+EOPS?  END */
    }
    else
    {
        /* <format> */
        err = at_tok_nextint(&line, &skip);
        if (err < 0 || skip != 2)
        {
            LOGW("The <format> is incorrect: expect 2, receive %d", skip);
            goto error;
        }

        // a "+COPS: 0, n" response is also possible
        if (!at_tok_hasmore(&line)) {
            onMccMncIndication(rid, "");
            backupOperatorMccMnc(rid, "");
        }
        else
        {
            /* <oper> */
            err = at_tok_nextstr(&line, &(response[2]));
            if (err < 0) goto error;

            /* Modem might response invalid PLMN ex: "", "000000" , "??????", all convert to "000000" */
            if(!((*response[2] >= '0') && (*response[2] <= '9'))) {
                LOGE("+COPS? got invalid PLMN response");
                // [ALPS02719776]
                response[2] = "000000";
            }

            //  keep plmn id for femtocell.
            if (isFemtocellSupport()) {
                pthread_mutex_lock(&ril_femtocell_mutex[rid]);
                strncpy(gFemtoCellInfo[rid].plmn_id, response[2], MAX_OPER_NAME_LENGTH);
                gFemtoCellInfo[rid].plmn_id[MAX_OPER_NAME_LENGTH - 1] = '\0';
                pthread_mutex_unlock(&ril_femtocell_mutex[rid]);
            }

            err = getOperatorNamesFromNumericCode(
                      response[2], longname, shortname, MAX_OPER_NAME_LENGTH, rid);
            onMccMncIndication(rid, response[2]);
            backupOperatorMccMnc(rid, response[2]);

            if(err>=0)
            {
                response[0] = longname;
                response[1] = shortname;
            }

        }
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));
    at_response_free(p_response);

    return;
error:
    LOGE("requestOperator must not return error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestQueryNetworkSelectionMode(void * data, size_t datalen, RIL_Token t)
{
    int err;
    ATResponse *p_response = NULL;
    int response = 0;
    char *line;
    char prop[PROPERTY_VALUE_MAX] = {0};
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    err = at_send_command_singleline("AT+COPS?", "+COPS:", &p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);

    if (err < 0)
    {
        goto error;
    }

    // LOGD("+COPS: %s", line);
    err = at_tok_nextint(&line, &response);

    if ( err < 0 || ( response < 0 || response > 4|| response == 3) )
    {
        goto error;
    }

    if (response == 0) {
        property_get(PROPERTY_SET_AUTO_MODE_ONLY, prop, "0");
        if (strncmp(prop, "1", 1) == 0){
            property_set(PROPERTY_SET_AUTO_MODE_ONLY, "0");
        }
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
    at_response_free(p_response);
    return;
error:
    at_response_free(p_response);
    LOGE("requestQueryNetworkSelectionMode must never return error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestSetNetworkSelectionAutomatic(void * data, size_t datalen, RIL_Token t)
{
    int err;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno;
    char prop[PROPERTY_VALUE_MAX] = {0};
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    property_get(PROPERTY_SET_AUTO_MODE_ONLY, prop, "0");
    if (strncmp(prop, "1", 1) == 0){
        property_set(PROPERTY_SET_AUTO_MODE_ONLY, "0");
        err = at_send_command("AT+EOPS=0", &p_response, NW_CHANNEL_CTX);
    } else {
        err = at_send_command("AT+COPS=0", &p_response, NW_CHANNEL_CTX);
    }

    if (err < 0 || p_response->success == 0)
    {
        ril_errno = RIL_E_REQUEST_NOT_SUPPORTED;
    }
    else
    {
        ril_errno = RIL_E_SUCCESS;
    }
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
}

void requestSetNetworkSelectionManual(void * data, size_t datalen, RIL_Token t)
{
    int err;
    char * cmd = NULL, *numeric_code;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    int len, i;

    numeric_code = (char *)data;

    if (NULL == numeric_code) {
        LOGE(LOG_TAG, "numeric is null!");
        ril_errno = RIL_E_INVALID_ARGUMENTS;
        goto error;
    }

    len = strlen(numeric_code);
    if (len == 6 && (strncmp(numeric_code, "123456", len) == 0)) {
        LOGE(LOG_TAG, "numeric is incorrect %s", numeric_code);
        ril_errno = RIL_E_INVALID_ARGUMENTS;
        goto error;
    }
    if (datalen == sizeof (char*) &&
            (len == 5 || len == 6))
    {
        // check if the numeric code is valid digit or not
        for(i = 0; i < len ; i++)
        {
            if( numeric_code[i] < '0' || numeric_code[i] > '9')
                break;
        }

        if( i == len)
        {

            err = asprintf(&cmd, "AT+COPS=1, 2, \"%s\"", numeric_code);
            if(err >= 0)
            {
                err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);

                free(cmd);

                if ( !(err < 0 || p_response->success == 0) )
                {
                    ril_errno = RIL_E_SUCCESS;
                } else {
                    ril_errno = RIL_E_INVALID_STATE;
                }
            }
        }
        else
        {
            LOGE("the numeric code contains invalid digits");
        }
    }
    else
    {
        LOGE("the data length is invalid for Manual Selection");
    }

error:
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
}

void requestSetNetworkSelectionManualWithAct(void * data, size_t datalen, RIL_Token t)
{
    int err;
    char * cmd = NULL, *numeric_code;
    char * act;
    const char**    args = (const char**)data;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    int len, i;
    bool isSemiAutoMode = false;
    bool noneBlockAutoMode = false;
    char * mode = 0;

    numeric_code = (char*) args[0];
    act = (char*) args[1];

    if (NULL == numeric_code || NULL == act) {
        LOGE(LOG_TAG, "numeric or act is null!");
        ril_errno = RIL_E_INVALID_ARGUMENTS;
        goto error;
    }

    if (datalen / sizeof(char *) == 3) {
        mode = (char*) args[2];
        if (mode != NULL) {
            if (strncmp(mode, "1", 1) == 0) {
                LOGD(LOG_TAG, "Semi auto network selection mode");
                isSemiAutoMode = true;
            } else if (strncmp(mode, "2", 1) == 0) {
                LOGD(LOG_TAG, "none blocking Auto Mode");
                noneBlockAutoMode = true;
            }
        }
    }

    len = strlen(numeric_code);
    if (len == 6 && (strncmp(numeric_code, "123456", len) == 0)) {
        LOGE(LOG_TAG, "numeric is incorrect %s", numeric_code);
        ril_errno = RIL_E_INVALID_ARGUMENTS;
        goto error;
    }
    if (noneBlockAutoMode) {
        err = at_send_command("AT+EOPS=0", &p_response, NW_CHANNEL_CTX);
        if (!(err < 0 || p_response->success == 0)) {
            ril_errno = RIL_E_SUCCESS;
        }
    } else if (len == 5 || len == 6) {
        // check if the numeric code is valid digit or not
        for (i = 0; i < len ; i++) {
            if (!isdigit(numeric_code[i]))
                break;
        }

        if ( i == len) {
            if (strlen(act) == 1 && isdigit(act[0])) {
                if (isSemiAutoMode == true)
                    err = asprintf(&cmd, "AT+EOPS=5, 2, \"%s\", %s", numeric_code, act);
                else
                    err = asprintf(&cmd, "AT+COPS=1, 2, \"%s\", %s", numeric_code, act);

            } else {
                if (isSemiAutoMode == true)
                    err = asprintf(&cmd, "AT+EOPS=5, 2, \"%s\"", numeric_code);
                else
                    err = asprintf(&cmd, "AT+COPS=1, 2, \"%s\"", numeric_code);
            }

            if (err >= 0) {
                err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);
                free(cmd);

                if (!(err < 0 || p_response->success == 0)) {
                    ril_errno = RIL_E_SUCCESS;
                }
            }
        } else {
            LOGE("the numeric code contains invalid digits");
        }
    } else {
        LOGE("the data length is invalid for Manual Selection");
    }

error:
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
}

void requestQueryAvailableNetworks(void * data, size_t datalen, RIL_Token t)
{
    int err, len, i, j, k, num, num_filter;
    ATResponse *p_response = NULL;
    char *line;
    char **response = NULL, **response_filter = NULL;
    char *tmp, *block_p = NULL;
    const RIL_SOCKET_ID rid = getRILIdByChannelCtx(NW_CHANNEL_CTX);
    char *lacStr = NULL;
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    plmnListOngoing = 1;
    LOGD("requestQueryAvailableNetworks AT+COPS=?");
    err = at_send_command_singleline("AT+COPS=?", "+COPS:",&p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0)
    {
        goto error;
    }

    line = p_response->p_intermediates->line;

    // count the number of operator
    len = strlen(line);
    for(i = 0, num = 0, num_filter = 0; i < len ; i++ )
    {
        if (line[i] == ')' && line[i+1] == ',')
        {
            num++;
            num_filter++;
        }
        else if (line[i] == ',' && line[i+1] == ',')
        {
            break;
        }
    }

    // +COPS: (2,"Far EasTone","FET","46601",0),(...),...,,(0, 1, 3),(0-2)

    err = at_tok_start(&line);
    if (err < 0) goto error;
    LOGD("requestQueryAvailableNetworks %s", line);

    response = (char **) calloc(1, sizeof(char*) * num *4); // for string, each one is 20 bytes
    block_p = (char *) calloc(1, num* sizeof(char)*4*MAX_OPER_NAME_LENGTH);
    lacStr = (char *) calloc(1, num* sizeof(char)*4+1);

    if (response == NULL || block_p == NULL || lacStr == NULL) {
        LOGE("requestQueryAvailableNetworks calloc fail");
        goto error;
    }

    for(i = 0, j=0 ; i < num ; i++, j+=4)
    {
        /* get "(<stat>" */
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;

        response[j+0] = &block_p[(j+0)*MAX_OPER_NAME_LENGTH];
        response[j+1] = &block_p[(j+1)*MAX_OPER_NAME_LENGTH];
        response[j+2] = &block_p[(j+2)*MAX_OPER_NAME_LENGTH];
        response[j+3] = &block_p[(j+3)*MAX_OPER_NAME_LENGTH];

        switch(tmp[1])
        {
        case '0':
            sprintf(response[j+3], "unknown");
            break;
        case '1':
            sprintf(response[j+3], "available");
            break;
        case '2':
            sprintf(response[j+3], "current");
            break;
        case '3':
            sprintf(response[j+3], "forbidden");
            break;
        default:
            LOGE("The %d-th <stat> is an invalid value!!!  : %d", i, tmp[1]);
            goto error;
        }

        /* get long name*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;
        sprintf(response[j+0], "%s", tmp);

        /* get short name*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;
        sprintf(response[j+1], "%s", tmp);

        /* get <oper> numeric code*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;
        sprintf(response[j+2], "%s", tmp);

        len = strlen(response[j+2]);
        if (len == 5 || len == 6)
        {
            if (0 == strncmp(response[j+2], ril_nw_nitz_oper_code[rid], MAX_OPER_NAME_LENGTH)) {
                err = getOperatorNamesFromNumericCode(
                        response[j+2], response[j+0], response[j+1], MAX_OPER_NAME_LENGTH, rid);
                if(err < 0) goto error;
            }
        }
        else
        {
            LOGE("The length of the numeric code is incorrect");
            goto error;
        }

        // ALPS00353868 START
        /*plmn_list_format.  0: standard +COPS format , 1: standard +COPS format plus <lac> */
        if(plmn_list_format[rid] == 1) {
            /* get <lac> numeric code*/
            err = at_tok_nextstr(&line, &tmp);
            if (err < 0){
                LOGE("No <lac> in +COPS response");
                goto error;
            }
            memcpy(&(lacStr[i*4]),tmp,4);
        }
        // ALPS00353868 END


        /* get <AcT> 0 is "2G", 2 is "3G", 7 is "4G"*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;

        // check if this plmn is redundant
        for (k=0; k < j; k+=4)
        {
            // compare numeric
            if (0 == strncmp(response[j+2], response[k+2], MAX_OPER_NAME_LENGTH)) {
                response[j+0] = response[j+1] = response[j+2] = response[j+3] = "";
                num_filter--;
                break;
            }
        }
    }

    // filter the response
    response_filter = (char**)calloc(1, sizeof(char*) * num_filter * 4);
    if (NULL == response_filter) {
        LOGE("calloc response_filter failed");
        goto error;
    }
    for (i=0, j=0, k=0; i < num; i++, j+=4)
    {
        if (0 < strlen(response[j+2]))
        {
            response_filter[k+0] = response[j+0];
            response_filter[k+1] = response[j+1];
            response_filter[k+2] = response[j+2];
            response_filter[k+3] = response[j+3];
            k += 4;
        }
    }

    LOGD("requestQueryAvailableNetworks sucess, clear plmnListOngoing and plmnListAbort flag");
    plmnListOngoing = 0;
    plmnListAbort =0; /* always clear here to prevent race condition scenario */

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response_filter, sizeof(char*)*num_filter*4);
    at_response_free(p_response);
    free(response);
    free(response_filter);
    free(block_p);
    free(lacStr);
    return;
error:
    at_response_free(p_response);
    if(response)
    {
        LOGD("FREE!!");
        if (block_p != NULL) free(block_p);
        free(response);
        if (lacStr != NULL) free(lacStr);
    }
    LOGD("requestQueryAvailableNetworks fail, clear plmnListOngoing and plmnListAbort flag");
    plmnListOngoing = 0;
    plmnListAbort =0; /* always clear here to prevent race condition scenario */
    RIL_onRequestComplete(t, RIL_E_MODEM_ERR, NULL, 0);
}

void requestQueryAvailableNetworksWithAct(void * data, size_t datalen, RIL_Token t)
{
    int err, len, i, j, num;
    ATResponse *p_response = NULL;
    char *line;
    char **response = NULL;
    char *tmp, *block_p = NULL;
    const RIL_SOCKET_ID rid = getRILIdByChannelCtx(NW_CHANNEL_CTX);
    char *lacStr = NULL;
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    // LOGD("requestQueryAvailableNetworksWithAct set plmnListOngoing flag");
    plmnListOngoing = 1;
    err = at_send_command_singleline("AT+COPS=?", "+COPS:",&p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    // count the number of operator
    len = strlen(line);
    for (i = 0, num = 0; i < len ; i++) {
        if (line[i] == ')' && line[i+1] == ',') {
            num++;
        } else if (line[i] == ',' && line[i+1] == ',') {
            break;
        }
    }

    // +COPS: (2,"Far EasTone","FET","46601",0),(...),...,,(0, 1, 3),(0-2)

    err = at_tok_start(&line);
    if (err < 0) goto error;

    response = (char **) calloc(1, sizeof(char*) * num *6); // for string, each one is 25 bytes
    block_p = (char *) calloc(1, num* sizeof(char)*6*MAX_OPER_NAME_LENGTH);
    lacStr = (char *) calloc(1, num* sizeof(char)*4+1);

    if (response == NULL || block_p == NULL || lacStr == NULL) {
        LOGE("requestQueryAvailableNetworksWithAct calloc fail");
        goto error;
    }

    for (i = 0, j=0 ; i < num ; i++, j+=6) {
        /* get "(<stat>" */
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;

        response[j+0] = &block_p[(j+0)*MAX_OPER_NAME_LENGTH];
        response[j+1] = &block_p[(j+1)*MAX_OPER_NAME_LENGTH];
        response[j+2] = &block_p[(j+2)*MAX_OPER_NAME_LENGTH];
        response[j+3] = &block_p[(j+3)*MAX_OPER_NAME_LENGTH];
        response[j+4] = &block_p[(j+4)*MAX_OPER_NAME_LENGTH];
        response[j+5] = &block_p[(j+5)*MAX_OPER_NAME_LENGTH];

        switch(tmp[1]) {
        case '0':
            sprintf(response[j+3], "unknown");
            break;
        case '1':
            sprintf(response[j+3], "available");
            break;
        case '2':
            sprintf(response[j+3], "current");
            break;
        case '3':
            sprintf(response[j+3], "forbidden");
            break;
        default:
            LOGE("The %d-th <stat> is an invalid value!!!  : %d", i, tmp[1]);
            goto error;
        }

        /* skip long name*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;

        /* skip short name*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;

        /* get <oper> numeric code*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;
        sprintf(response[j+2], "%s", tmp);

        len = strlen(response[j+2]);
        if (len == 5 || len == 6) {
            err = getOperatorNamesFromNumericCode(
                      response[j+2], response[j+0], response[j+1], MAX_OPER_NAME_LENGTH, rid);
            if(err < 0) goto error;
        } else {
            LOGE("The length of the numeric code is incorrect");
            goto error;
        }

        // ALPS00353868 START
        /*plmn_list_format.  0: standard +COPS format , 1: standard +COPS format plus <lac> */
        if (plmn_list_format[rid] == 1) {
            /* get <lac> numeric code*/
            err = at_tok_nextstr(&line, &tmp);
            if (err < 0){
                LOGE("No <lac> in +COPS response");
                goto error;
            }
            memcpy(&(lacStr[i*4]),tmp,4);
            sprintf(response[j+4], "%s", tmp);
        }
        // ALPS00353868 END

        /* get <AcT> 0 is "2G", 2 is "3G", 7 is "4G"*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;

        switch(tmp[0]) {
        case '0':
            sprintf(response[j+5], "2G");
            break;
        case '2':
            sprintf(response[j+5], "3G");
            break;
        case '7':    //for  LTE
            sprintf(response[j+5], "4G");
            break;
        default:
            LOGE("The %d-th <Act> is an invalid value!!!  : %d", i, tmp[1]);
            goto error;
        }
    }

    LOGD("requestQueryAvailableNetworksWithAct sucess, clear plmnListOngoing and plmnListAbort flag");
    plmnListOngoing = 0;
    plmnListAbort = 0; /* always clear here to prevent race condition scenario */

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(char*)*num*6);
    at_response_free(p_response);
    free(response);
    free(block_p);
    free(lacStr);
    return;

error:
    at_response_free(p_response);
    if (response) {
        LOGD("FREE!!");
        if (block_p != NULL) free(block_p);
        free(response);
        if (lacStr != NULL) free(lacStr);
    }
    LOGE("requestQueryAvailableNetworksWithAct must never return error when radio is on, plmnListAbort=%d",plmnListAbort);
    if (plmnListAbort == 1) {
        RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }

    LOGD("requestQueryAvailableNetworksWithAct fail, clear plmnListOngoing and plmnListAbort flag");
    plmnListOngoing = 0;
    plmnListAbort =0; /* always clear here to prevent race condition scenario */
}

void requestAbortQueryAvailableNetworks(void * data, size_t datalen, RIL_Token t)
{
    int err;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_SUCCESS;
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    LOGD("requestAbortQueryAvailableNetworks execute while plmnListOngoing=%d",plmnListOngoing);
    if (plmnListOngoing == 1) {
        LOGD("requestAbortQueryAvailableNetworks set plmnListAbort flag");
        plmnListAbort =1;

        err = at_send_command("AT+CAPL", &p_response, NW_CHANNEL_CTX);

        if (err < 0 || p_response->success == 0) {
            ril_errno = RIL_E_GENERIC_FAILURE;
            plmnListAbort =0;
            LOGD("requestAbortQueryAvailableNetworks fail,clear plmnListAbort flag");
        } else {
            ril_errno = RIL_E_SUCCESS;
        }
    }

    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
}

void requestSetBandMode(void * data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int req, err, gsm_band, umts_band;
    unsigned int lte_band_1_32, lte_band_33_64;
    char *cmd = NULL;
    RIL_Errno ril_errno = RIL_E_REQUEST_NOT_SUPPORTED;
    RIL_NW_UNUSED_PARM(datalen);
    req = ((int *)data)[0];
    int cme_Error;

    switch(req) {
        case BAND_MODE_UNSPECIFIED: //"unspecified" (selected by baseband automatically)
            gsm_band = 0xff;
            umts_band = 0xffff;
            break;
        case BAND_MODE_EURO: //"EURO band" (GSM-900 / DCS-1800 / WCDMA-IMT-2000)
            gsm_band = GSM_BAND_900 | GSM_BAND_1800;
            umts_band = UMTS_BAND_I;
            break;
        case BAND_MODE_USA: //"US band" (GSM-850 / PCS-1900 / WCDMA-850 / WCDMA-PCS-1900)
            gsm_band = GSM_BAND_850 | GSM_BAND_1900;
            umts_band = UMTS_BAND_II | UMTS_BAND_V;
            break;
        case BAND_MODE_JPN: //"JPN band" (WCDMA-800 / WCDMA-IMT-2000)
            gsm_band = 0;
            umts_band = UMTS_BAND_I | UMTS_BAND_VI;
            break;
        case BAND_MODE_AUS: //"AUS band" (GSM-900 / DCS-1800 / WCDMA-850 / WCDMA-IMT-2000)
            gsm_band = GSM_BAND_900 | GSM_BAND_1800;
            umts_band = UMTS_BAND_I | UMTS_BAND_V;
            break;
        case BAND_MODE_AUS_2: //"AUS band 2" (GSM-900 / DCS-1800 / WCDMA-850)
            gsm_band = GSM_BAND_900 | GSM_BAND_1800;
            umts_band = UMTS_BAND_V;
            break;
        case BAND_MODE_CELL_800: //"Cellular (800-MHz Band)"
        case BAND_MODE_PCS: //"PCS (1900-MHz Band)"
        case BAND_MODE_JTACS: //"Band Class 3 (JTACS Band)"
        case BAND_MODE_KOREA_PCS: //"Band Class 4 (Korean PCS Band)"
        case BAND_MODE_5_450M: //"Band Class 5 (450-MHz Band)"
        case BAND_MODE_IMT2000: // "Band Class 6 (2-GMHz IMT2000 Band)"
        case BAND_MODE_7_700M_2: //"Band Class 7 (Upper 700-MHz Band)"
        case BAND_MODE_8_1800M: //"Band Class 8 (1800-MHz Band)"
        case BAND_MODE_9_900M: //"Band Class 9 (900-MHz Band)"
        case BAND_MODE_10_800M_2: //"Band Class 10 (Secondary 800-MHz Band)"
        case BAND_MODE_EURO_PAMR_400M: //"Band Class 11 (400-MHz European PAMR Band)"
        case BAND_MODE_AWS: //"Band Class 15 (AWS Band)"
        case BAND_MODE_USA_2500M: //"Band Class 16 (US 2.5-GHz Band)"
        default:
            gsm_band = -1;
            umts_band = -1;
            break;
    }

    if (gsm_band != -1 && umts_band != -1)
    {
        /******************************************************
        * If the modem doesn't support certain group of bands, ex. GSM or UMTS
        * It might just ignore the parameter.
        *******************************************************/
        err = asprintf(&cmd, "AT+EPBSE=%d, %d", gsm_band, umts_band);
        if(err >= 0)
        {
            err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);

            free(cmd);

            if(err >= 0 && p_response->success != 0)
            {
                ril_errno = RIL_E_SUCCESS;
            }
        }
        RIL_onRequestComplete(t, ril_errno, NULL, 0);
        at_response_free(p_response);
        return;
    ///M : [Network][C2K]add for band8 desense feature. @{
    } else if (req == BM_FOR_DESENSE_RADIO_ON || req == BM_FOR_DESENSE_RADIO_OFF
            || req == BM_FOR_DESENSE_RADIO_ON_ROAMING || req == BM_FOR_DESENSE_B8_OPEN) {
        band_mode_request = req;
        force_switch = ((int *) data)[1];
        RIL_requestProxyTimedCallback(setBandMode, (void * )t, &TIMEVAL_1,
                getRILChannelId(RIL_SIM, getMainProtocolRid()), "setBandMode");
    }
    /// @}
}

// Running SIM Channel
void setBandMode(void* param) {
    RIL_Token t = *((RIL_Token *) param);
    RIL_Errno ril_errno = RIL_E_REQUEST_NOT_SUPPORTED;
    ATResponse *p_response = NULL;
    int err = -1;
    int cme_error = -1;
    char *cmd = calculateBandMode();
    if (cmd != NULL) {
        err = at_send_command(cmd, &p_response, getChannelCtxbyProxy());
        cme_error = at_get_cme_error(p_response);
        if ((err >= 0 && p_response->success != 0)
                || (err >= 0 && (cme_error == CME_OPERATION_NOT_SUPPORTED))) {
            LOGD("onSetBandModeResult: success or CME_OPERATION_NOT_SUPPORTED no need retry");
            free(cmd);
        } else {
            free(cmd);
            at_response_free(p_response);
            p_response = NULL;
            detachCount = 0;
            onSetBandModeResult(param);
        }
    }
    if (p_response != NULL) {
        at_response_free(p_response);
        p_response = NULL;
    }
    ril_errno = RIL_E_SUCCESS;
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
}

///M : [Network][C2K]add for band8 desense feature. @{
void onSetBandModeResult(void* param) {
    RIL_NW_UNUSED_PARM(param);
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    int err;
    int skipDetach = 0;
    int cme_error;
    cmd = calculateBandMode();
    if (skipDetach == 0 && detachCount < 10 ) {
        if (cmd != NULL) {
            err = at_send_command(cmd, &p_response, getChannelCtxbyProxy());
            cme_error = at_get_cme_error(p_response);
            if ((err >= 0 && p_response->success != 0) ||
                    (err >= 0 && (cme_error == CME_OPERATION_NOT_SUPPORTED))) {
                LOGD("onSetBandModeResult: success or CME_OPERATION_NOT_SUPPORTED no need retry ");
                skipDetach = 1;
                detachCount = 0;
            } else {
                detachCount++;
                LOGE("onSetBandModeResult: fail, count=%d", detachCount);
                RIL_requestProxyTimedCallback((RIL_TimedCallback)onSetBandModeResult, NULL, &TIMEVAL_1,
                        getRILChannelId(RIL_SIM, getMainProtocolRid()), "onSetBandModeResult");
            }
            free(cmd);
            at_response_free(p_response);
            p_response = NULL;
            return;
        }
    }
    return;
}

char* calculateBandMode () {
    requestQueryCurrentBandMode();
    char* bands_value = NULL;
    int c2k_radio_on = -1;
    int err = -1;
    int need_config_umts = 0;
    int need_config_lte_b5 = 0;
    int need_config_lte_b8 = 0;
    int need_config_lte_b26 = 0;
    int gsm_band = bands[0];
    int umts_band = bands[1];
    int lte_band_1_32 = bands[2];
    int lte_band_33_64 = bands[3];
    LOGD("BM FOR DESENCE, gsm_band:%d, umts_band : %d, lte_band_1_32 : %d, lte_band_33_64: %d, req: %d ",
            gsm_band, umts_band, lte_band_1_32, lte_band_33_64, band_mode_request);
    //Check if need disable wcdma band8 and lte band5.
    if (band_mode_request == BM_FOR_DESENSE_RADIO_ON) {
        if(umts_band & 0x00000080) {
            need_config_umts = 1;
            umts_band = umts_band & 0xffffff7f;
        }
        if(lte_band_1_32 & 0x00000010) {
            need_config_lte_b5 = 1;
            lte_band_1_32 = lte_band_1_32 & 0xffffffef;
        }
        if(lte_band_1_32 & 0x00000080) {
            need_config_lte_b8 = 1;
            lte_band_1_32 = lte_band_1_32 & 0xffffff7f;
        }
    } else {
    //Check if need enable wcdma band8 and lte band5.(when support and be disabled)
        if ((support_bands[1] & 0x00000080)
            && ((umts_band & 0x00000080) == 0)) {
                need_config_umts = 1;
                umts_band = umts_band | 0x00000080;
        }
        if ((support_bands[2] & 0x00000010)
                && ((lte_band_1_32 & 0x00000010) == 0)) {
            need_config_lte_b5 = 1;
            lte_band_1_32 = lte_band_1_32 | 0x00000010;
        }
        if ((support_bands[2] & 0x00000080)
            && ((lte_band_1_32 & 0x00000080) == 0)) {
            need_config_lte_b8 = 1;
            lte_band_1_32 = lte_band_1_32 | 0x00000080;
        }
    }
    if (band_mode_request == BM_FOR_DESENSE_RADIO_OFF) {
        c2k_radio_on = 0;
    } else {
        c2k_radio_on = 1;
    }
    LOGD("BM FOR DESENCE, need_config_umts: %d, need_config_lte_b5: %d, "
            " need_config_lte_b8: %d, need_config_lte_b26: %d, force_switch : %d",
            need_config_umts, need_config_lte_b5,
            need_config_lte_b8, need_config_lte_b26, force_switch);
    if (need_config_umts || need_config_lte_b5 || need_config_lte_b8 || need_config_lte_b26
                || force_switch) {
        if (band_mode_request == BM_FOR_DESENSE_B8_OPEN) {
            err = asprintf(&bands_value, "AT+EPBSE=%d,%d,%d,%d", gsm_band, umts_band,
                    lte_band_1_32, lte_band_33_64);
        } else {
            err = asprintf(&bands_value, "AT+EPBSE=%d,%d,%d,%d,%d", gsm_band, umts_band,
                    lte_band_1_32, lte_band_33_64, c2k_radio_on);
        }
    }
    if (err > 0) {
        return bands_value;
    } else {
        return NULL;
    }
}
/// @}

/// M: [Network][C2K]Add for band desense feature. @{
void requestQueryCurrentBandMode(){
    ATResponse *p_response = NULL;
     int err, gsm_band, umts_band;
     char *line;
    err = at_send_command_singleline("AT+EPBSE?", "+EPBSE:", &p_response,
            getChannelCtxbyProxy());

     if (err < 0 || p_response->success == 0) {
         at_response_free(p_response);
         return;
     }

     line = p_response->p_intermediates->line;

     err = at_tok_start(&line);
     if(err < 0) return;

     // get supported GSM bands
     err = at_tok_nextint(&line, &gsm_band);
     if(err < 0) return;

     // get supported UMTS bands
     err = at_tok_nextint(&line, &umts_band);
     if(err < 0) return;

     bands[0] = gsm_band;
     bands[1] = umts_band;
     err = at_tok_nextint(&line, &bands[2]);
     err = at_tok_nextint(&line, &bands[3]);
     LOGD("requestQueryCurrentBandMode, gsm_band:%d, umts_band : %d, lte_band_1_32 : %d, lte_band_33_64: %d",
             bands[0], bands[1], bands[2], bands[3]);
}
/// @}

void requestQueryAvailableBandMode(void * data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err, gsm_band, umts_band;
    char *cmd = NULL, *line = NULL;
    int band_mode[10], index=1;
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    err = at_send_command_singleline("AT+EPBSE=?", "+EPBSE:", &p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0)
        goto error;


    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if(err < 0) goto error;

    // get supported GSM bands
    err = at_tok_nextint(&line, &gsm_band);
    if(err < 0) goto error;

    // get supported UMTS bands
    err = at_tok_nextint(&line, &umts_band);
    if(err < 0) goto error;

    ///M: [Network][C2K] Add for band de-sense feature.@{
    support_bands[0] = gsm_band;
    support_bands[1] = umts_band;
    err = at_tok_nextint(&line, &support_bands[2]);
    err = at_tok_nextint(&line, &support_bands[3]);
    LOGD("requestQueryAvailableBandMode, gsm_band:%d, umts_band : %d, lte_band_1_32 : %d,lte_band_33_64: %d",
            support_bands[0], support_bands[1], support_bands[2], support_bands[3]);
    /// @}

    //0 for "unspecified" (selected by baseband automatically)
    band_mode[index++] = BAND_MODE_UNSPECIFIED;

    if(gsm_band !=0 || umts_band != 0)
    {
        // 1 for "EURO band" (GSM-900 / DCS-1800 / WCDMA-IMT-2000)
        if ((gsm_band == 0 || (gsm_band | GSM_BAND_900 | GSM_BAND_1800)==gsm_band) &&
            (umts_band == 0 || (umts_band | UMTS_BAND_I)==umts_band))
        {
            band_mode[index++] = BAND_MODE_EURO;
        }

        // 2 for "US band" (GSM-850 / PCS-1900 / WCDMA-850 / WCDMA-PCS-1900)
        if ( (gsm_band == 0 ||  (gsm_band | GSM_BAND_850 | GSM_BAND_1900)==gsm_band) &&
             (umts_band == 0 ||  (umts_band | UMTS_BAND_II | UMTS_BAND_V)==umts_band) )
        {
            band_mode[index++] = BAND_MODE_USA;
        }

        // 3 for "JPN band" (WCDMA-800 / WCDMA-IMT-2000)
        if ( (umts_band | UMTS_BAND_I | UMTS_BAND_VI)==umts_band)
        {
            band_mode[index++] = BAND_MODE_JPN;
        }

        // 4 for "AUS band" (GSM-900 / DCS-1800 / WCDMA-850 / WCDMA-IMT-2000)
        if ( (gsm_band == 0 ||  (gsm_band | GSM_BAND_900 | GSM_BAND_1800)==gsm_band) &&
                (umts_band == 0 ||  (umts_band | UMTS_BAND_I | UMTS_BAND_V)==umts_band) )
        {
            band_mode[index++] = BAND_MODE_AUS;
        }

        // 5 for "AUS band 2" (GSM-900 / DCS-1800 / WCDMA-850)
        if ( (gsm_band == 0 ||  (gsm_band | GSM_BAND_900 | GSM_BAND_1800)==gsm_band) &&
                (umts_band == 0 ||  (umts_band | UMTS_BAND_V)==umts_band) )
        {
            band_mode[index++] = BAND_MODE_AUS_2;
        }

    }

    band_mode[0] = index - 1; //ALPS03070712 M->N migration changes

    RIL_onRequestComplete(t, RIL_E_SUCCESS, band_mode, sizeof(int)*index);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

bool isRatPreferred(int req_type, RIL_SOCKET_ID rid){
    char prop[PROPERTY_VALUE_MAX] = {0};
    bool isPreferred = false;
    int baseband=0;
    int isWcdmaPreferred = 0;
    int isTestMode = 0;

    memset(prop,0,sizeof(prop));
    property_get("ro.vendor.mtk_rat_wcdma_preferred",prop , "0");
    isWcdmaPreferred = atoi(prop);

    memset(prop,0,sizeof(prop));
    property_get("vendor.gsm.gcf.testmode", prop, "0");
    isTestMode = atoi(prop);

    if (RatConfig_isLteFddSupported() == 1 || RatConfig_isLteTddSupported() == 1 ||
            isWcdmaPreferred == 1) {
        isPreferred = true;
    }

    if (isWcdmaPreferred == 1 &&
            (isTestMode > 0 || RIL_isTestSim(rid) == true)) {
        LOGD("isWcdmaPreferred=%d, baseband=%d",
                isWcdmaPreferred, baseband);
        isPreferred = false;  // 3G AUTO in test mode/test SIM if WCDMA preferred
    }
    return isPreferred;
}

RIL_Errno handleERATResult(const char* cmd, RIL_Token t) {
    int err = -1;
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    ATResponse *p_response = NULL;

    int skipDetach = 0;
    int detachCount = 0;

    while (skipDetach == 0 && detachCount < 40) {
        // if (inCallNumberPerSim[rid] > 0) {
        //     LOGE("requestSetPreferredNetworkType(): in call, do not retry erat");
        //     break;
        // }
        err = at_send_command(cmd, &p_response, getRILChannelCtxFromToken(t));

        if (CME_ERAT_NOT_ALLOWED_CONFLICT_WITH_CALL == at_get_cme_error(p_response)) {
            LOGE("handleERATResult(): in call, do not retry erat");
            ril_errno = RIL_E_OPERATION_NOT_ALLOWED;
            break;
        }
        // other error response
        if (isATCmdRspErr(err, p_response)) {
            detachCount++;
            LOGE("handleERATResult(): fail, count=%d", detachCount);
            sleep(1);
        } else {
            //LOGD("handleERATResult: success");
            skipDetach = 1;
            ril_errno = RIL_E_SUCCESS;
        }
        at_response_free(p_response);
        p_response = NULL;
    }
    // LOGD("handleERATResult end, ril_errno is %d.", ril_errno);
    return ril_errno;
}

void requestSetPreferredNetworkType(void * data, size_t datalen, RIL_Token t)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int req_type, err, rat, rat1;
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    char *cmd = NULL;
    bool isPreferred = false;
    char optr[PROPERTY_VALUE_MAX];
    property_get("persist.vendor.radio.simswitch", optr, "1");
    int currMajorSim = (atoi(optr)-1);
    RIL_NW_UNUSED_PARM(datalen);

    req_type = ((int *)data)[0];
    rat1= 0;
    isPreferred = isRatPreferred(req_type, rid);

    switch(req_type) {
        case PREF_NET_TYPE_GSM_WCDMA:
            rat = 2;    // 2/3G AUTO
            if(isPreferred){
                rat1= 2;  // 3G preferred
            }
            /// M: [Network][C2K] add for CDMA 4M project start @{
            if (isCdmaLteDcSupport()) {
                // 4M - GSM only
                if (isSvlteLcgSupport()) {
                    rat = 0;
                    rat1 = 0;
                }
            }
            /// M end @}
            break;
        case PREF_NET_TYPE_GSM_ONLY:
            rat = 0;    // 2G only
            break;
        case PREF_NET_TYPE_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_ONLY:
        case PREF_NET_TYPE_TD_SCDMA_WCDMA:
            rat = 1;    // 3G only
            break;
        case PREF_NET_TYPE_GSM_WCDMA_AUTO:
        case PREF_NET_TYPE_TD_SCDMA_GSM:
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:
            rat = 2;    // 2/3G AUTO
            break;
        case PREF_NET_TYPE_CDMA_EVDO_AUTO:
            rat = 7;    // C2K 1x/Evdo
            if (currMajorSim != rid) {
                rat1 = 32;  // C2K 1x only
            }
            break;
        case PREF_NET_TYPE_CDMA_ONLY:
            rat = 7;    // C2K 1x/Evdo
            rat1 = 32;  // C2K 1x only
            break;
        case PREF_NET_TYPE_EVDO_ONLY:
            rat = 7;    // C2K 1x/Evdo
            rat1 = 64;  // C2K Evdo only
            break;
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
            rat = 2;    // GSM/WCDMA mode
            break;
        case PREF_NET_TYPE_LTE_CDMA_EVDO:
            rat = 11;   // LC mode
            break;
        case PREF_NET_TYPE_LTE_GSM_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_GSM_LTE:
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:
            rat = 6;    // 2/3/4G AUTO
            if (isPreferred) {
                rat1= 4;  //4G preferred
            }
            /// M: [Network][C2K] add for India 4M project start@{
            if (isCdmaLteDcSupport()) {
                if (isSvlteLcgSupport()) {
                    rat = 4;
                }
            }
            /// M end@}
            break;
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
            rat = 14;    // LTE CDMA EVDO GSM/WCDMA mode
            break;
        case PREF_NET_TYPE_LTE_ONLY:
            rat = 3;    // LTE only for EM mode
            break;
        case PREF_NET_TYPE_LTE_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_LTE:
        case PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:
            rat = 5;    //  LTE/WCDMA for EM mode
            break;
        case PREF_NET_TYPE_LTE_GSM:
            rat = 4;    // 2/4G
            break;
        default:
            rat = -1;
            break;
    }

    if(rat >= 0) {
        err = asprintf(&cmd,  "AT+ERAT=%d,%d", rat, rat1);
        if (err >= 0) {
            //LOGD("requestSetPreferredNetworkType(), send command %s.", cmd);
            ril_errno = handleERATResult(cmd, t);
            if (ril_errno == RIL_E_SUCCESS) {
                sLastReqRatType[rid] = req_type;
            }
            free(cmd);
        }
    }

    RIL_onRequestComplete(t, ril_errno, NULL, 0);
}

void requestGetPreferredNetworkType(void * data, size_t datalen, RIL_Token t)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    ATResponse *p_response = NULL;
    int err, skip, nt_type , prefer_type, return_type;
    char *cmd = NULL, *line = NULL;
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    if (sLastReqRatType[rid] > -1) {
        return_type = sLastReqRatType[rid];
        goto skipQuery;
    }

    err = at_send_command_singleline("AT+ERAT?", "+ERAT:", &p_response, getRILChannelCtxFromToken(t));

    if (err < 0 || p_response->success == 0)
        goto error;

    line = p_response->p_intermediates->line;

        prefer_type=0;
    err = at_tok_start(&line);
    if(err < 0) goto error;

    //skip <curr_rat>
    err = at_tok_nextint(&line, &skip);
    if(err < 0) goto error;

    //skip <gprs_status>
    err = at_tok_nextint(&line, &skip);
    if(err < 0) goto error;

    //get <rat>
    err = at_tok_nextint(&line, &nt_type);
    if(err < 0) goto error;

    err = at_tok_nextint(&line, &prefer_type);
    if(err < 0) goto error;

    if (nt_type == 0) {
        return_type = PREF_NET_TYPE_GSM_ONLY;
    } else if(nt_type == 1) {
        return_type = PREF_NET_TYPE_WCDMA;
    } else if(nt_type == 2 && prefer_type == 0) {
        return_type = PREF_NET_TYPE_GSM_WCDMA_AUTO;
    } else if(nt_type == 2 && prefer_type == 1) {
        LOGE("Dual mode but GSM prefer, mount to AUTO mode");
        return_type = PREF_NET_TYPE_GSM_WCDMA_AUTO;
    } else if(nt_type == 2 && prefer_type == 2) {
        return_type = PREF_NET_TYPE_GSM_WCDMA;

    //for LTE -- START
    } else if(nt_type == 6 && prefer_type == 4) {
        //4G Preferred (4G, 3G/2G) item
        //Bause we are not defind LTE preferred,
        //so return by NT_LTE_GSM_WCDMA_TYPE temporary
        return_type = PREF_NET_TYPE_LTE_GSM_WCDMA;
    } else if(nt_type == 6 && prefer_type == 0) {
        //4G/3G/2G(Auto) item
        return_type = PREF_NET_TYPE_LTE_GSM_WCDMA;
    } else if (nt_type == 14) {
        // LTE CDMA EVDO GSM/WCDMA mode
        return_type = PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA;
    } else if(nt_type == 3) {
        //4G only
        return_type = PREF_NET_TYPE_LTE_ONLY;
    } else if(nt_type == 5) {
        // 4G/3G
        return_type = PREF_NET_TYPE_LTE_WCDMA;
    } else if(nt_type == 7) {
        // C2K 2/3G
        return_type = PREF_NET_TYPE_CDMA_EVDO_AUTO;
    } else if(nt_type == 11) {
        // LC mode
        return_type = PREF_NET_TYPE_LTE_CDMA_EVDO;
    } else {
        goto error;
    }

skipQuery:
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &return_type, sizeof(int) );
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);

}

void requestGetNeighboringCellIds(void * data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err, skip, nt_type;
    char *cmd = NULL, *line = NULL;
    ATLine *p_line;

    int rat,rssi,ci,lac,psc;
    int i = 0;
    int j = 0;
    RIL_NeighboringCell nbr[6];
    RIL_NeighboringCell *p_nbr[6];
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    err = at_send_command_multiline("AT+ENBR", "+ENBR:", &p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0)
        goto error;

    p_line = p_response->p_intermediates;

    while(p_line != NULL)
    {
        line = p_line->line;

        err = at_tok_start(&line);
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &rat);
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &rssi);
        if(err < 0) goto error;

        if(( (rat == 1) && (rssi < 0  || rssi > 31) && (rssi != 99) )
                || ( (rat == 2) && (rssi < 0 || rssi > 91) ))
        {
            LOGE("The rssi of the %d-th is invalid: %d", i, rssi );
            goto error;
        }

        nbr[i].rssi = rssi;

        if (rat == 1)
        {
            err = at_tok_nextint(&line, &ci);
            if(err < 0) goto error;

            err = at_tok_nextint(&line, &lac);
            if(err < 0) goto error;

            err = asprintf(&nbr[i].cid, "%04X%04X", lac, ci);
            if(err < 0)
            {
                LOGE("Using asprintf and getting ERROR");
                goto error;
            }
            LOGD("NC[%d], rssi:%d, cid:%s", i, nbr[i].rssi, nbr[i].cid);
        }
        else if (rat == 2)
        {
            err = at_tok_nextint(&line, &psc);
            if(err < 0) goto error;

            err = asprintf(&nbr[i].cid, "%08X", psc);
            if(err < 0)
            {
                LOGE("Using asprintf and getting ERROR");
                goto error;
            }
            LOGD("NC[%d], rssi:%d, psc:%d", i, rssi, psc);
        }
        else
            goto error;

        p_nbr[i] = &nbr[i];
        i++;
        p_line = p_line->p_next;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, p_nbr, sizeof(RIL_NeighboringCell*) * i );
    at_response_free(p_response);
    for(j=0;j<i;j++)
        free(nbr[j].cid);
    return;

error:
    LOGE("requestGetNeighboringCellIds has error occur!!");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    for(j=0;j<i;j++)
        free(nbr[j].cid);
}

void requestSetLocationUpdates(void * data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err, enabled;
    RIL_Errno err_no = RIL_E_GENERIC_FAILURE;
    char *cmd = NULL;

    if (datalen == sizeof(int)) {
        enabled = ((int*)data)[0];

        if (enabled == 1 || enabled == 0)
        {
            err = asprintf(&cmd, "AT+CREG=%d", enabled ? 2 : 1);
            if( err >= 0 )
            {
                err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);
                free(cmd);

                if ( err >= 0 && p_response->success > 0)
                {
                    err_no = RIL_E_SUCCESS;
                }
            }
        }
    }
    RIL_onRequestComplete(t, err_no, NULL, 0);
    at_response_free(p_response);

}

static int getCellInfoListV12(char * line, int num, RIL_CellInfo_v12 * response, int slotId)
{
    char currentMccmnc[PROPERTY_VALUE_MAX] = {0};
    int mccMncLength = 0;
    int INVALID = INT_MAX; // 0x7FFFFFFF;
    int err=0, i=0,act=0 ,cid=0,mcc=0,mnc=0,lacTac=0,pscPci=0;
    int sig1=0,sig2=0,rsrp=0,rsrq=0,rssnr=0,cqi=0,timingAdvance=0;
    int bsic=0;
    int arfcn=INVALID;
    char mcc_str[4] = {0};
    char mnc_str[4] = {0};
    int mnc_len = 0;

     /* +ECELL: <num_of_cell>[,<act>,<cid>,<lac_or_tac>,<mcc>,<mnc>,
             <psc_or_pci>, <sig1>,<sig2>,<sig1_in_dbm>,<sig2_in_dbm>,<ta>,
             <ext1>,<ext2>,]
             [<ext3>,<ext4>]
    */
     // ext3 is for gsm bsic
     // ext4 is for arfcn, uarfcn, earfch
    int ext3_ext4_enabled = ECELLext3ext4Support[slotId]; //set to 1 if modem support

    for(i=0 ; i < num ; i++) {
        /* Registered field is used to tell serving cell or neighboring cell.
           The first cell info returned is the serving cell,
           others are neighboring cell */
        if (i == 0)
            response[i].registered = 1;

        err = at_tok_nextint(&line, &act);
        if (err < 0) goto error;

        err = at_tok_nexthexint(&line, &cid);
        if (err < 0) goto error;

        err = at_tok_nexthexint(&line, &lacTac);
        if (err < 0) {
            // LOGE("ERROR occurs when parsing <lacTac> from +ECELL URC");
            lacTac = INVALID;
        }
        err = at_tok_nextint(&line, &mcc);
        /* Check if the registered MCC/MNC is available in system property,
           and use it instead of the one received from +ECELL */
        property_get(PROPERTY_NW_MCCMNC[slotId], currentMccmnc, "");
        mccMncLength = strlen(currentMccmnc);
        if (((mccMncLength == 5) || (mccMncLength == 6)) &&
                (mcc != INVALID)) {
            strncpy(mcc_str, currentMccmnc, 3);
            //LOGD("getCellInfoListV12, currentMccmnc: %s, mcc_str: %s, mcc: %d",
            //        currentMccmnc, mcc_str, mcc);
            mcc = atoi(mcc_str);
        } else if (err < 0) {
            // LOGE("ERROR occurs when parsing <mcc> from +ECELL URC");
            mcc = INVALID;
        }
        err = at_tok_nextint(&line, &mnc);
        if (((mccMncLength == 5) || (mccMncLength == 6)) &&
                (mnc != INVALID)) {
            strncpy(mnc_str, (currentMccmnc+3), (mccMncLength-3));
            //LOGD("getCellInfoListV12, currentMccmnc: %s, mnc_str: %s, mnc: %d",
            //        currentMccmnc, mnc_str, mnc);
            mnc = atoi(mnc_str);
        } else if (err < 0) {
            // LOGE("ERROR occurs when parsing <mnc> from +ECELL URC");
            mnc = INVALID;
        }
        // For reporting "02" for mnc, we have to know the lenght.
        if (mnc >= 100) mnc_len = 3;
        else mnc_len = 2;
        err = at_tok_nextint(&line, &pscPci);
        if (err < 0) {
            // LOGE("ERROR occurs when parsing <pscPci> from +ECELL URC");
            pscPci = INVALID;
        }
        err = at_tok_nextint(&line, &sig1);
        if (err < 0) {
            // LOGE("ERROR occurs when parsing <sig1> from +ECELL URC");
            sig1 = INVALID;
        }
        err = at_tok_nextint(&line, &sig2);
        if (err < 0) {
            // LOGE("ERROR occurs when parsing <sig2> from +ECELL URC");
            sig2 = INVALID;
        }
        err = at_tok_nextint(&line, &rsrp);
        if (err < 0) {
            // LOGE("ERROR occurs when parsing <rsrp> from +ECELL URC");
            rsrp = INVALID;
        }
        err = at_tok_nextint(&line, &rsrq);
        if (err < 0) {
            // LOGE("ERROR occurs when parsing <rsrq> from +ECELL URC");
            rsrq = INVALID;
        }
        err = at_tok_nextint(&line, &timingAdvance);
        if (err < 0) {
            // LOGE("ERROR occurs when parsing <ta> from +ECELL URC");
            timingAdvance = INVALID;
        }
        err = at_tok_nextint(&line, &rssnr);
        if (err < 0) {
            // LOGE("ERROR occurs when parsing <rssnr> from +ECELL URC");
            rssnr = INVALID;
        }
        err = at_tok_nextint(&line, &cqi);
        if (err < 0) {
            // LOGE("ERROR occurs when parsing <cqi> from +ECELL URC");
            cqi = INVALID;
        }
        if (ext3_ext4_enabled == 1) {
            err = at_tok_nextint(&line, &bsic);
            if (err < 0) {
                // LOGE("ERROR occurs when parsing <bsic> from +ECELL URC");
                bsic = 0;
            }
            err = at_tok_nextint(&line, &arfcn);
            if (err < 0) {
                // LOGE("ERROR occurs when parsing <arfcn> from +ECELL URC");
                arfcn = INVALID;
            }
        }
        char mccmnc_string[10];
        char longname[MAX_OPER_NAME_LENGTH], shortname[MAX_OPER_NAME_LENGTH];
        mccmnc_string[0] = '\0';
        longname[0] = '\0';
        shortname[0] = '\0';
        if (mcc != INVALID && mnc != INVALID) {
            int err = 0;
            if (mnc_len == 2) sprintf(mccmnc_string, "%03d%02d", mcc, mnc);
            else sprintf(mccmnc_string, "%03d%03d", mcc, mnc);
            err = getOperatorNamesFromNumericCode(
                    mccmnc_string, longname, shortname, MAX_OPER_NAME_LENGTH, slotId);
            if(err < 0) LOGV("operator name not found");
        }
        char * lac_s = getMask(lacTac);
        char * cid_s = getMask(pscPci);
        if (!lac_s || !cid_s) {
            LOGD("[%s] can not get memeory to print log", __func__);
        } else {
            LOGD("act=%d,cid=%d,mcc=%d,mnc=%d,lacTac=%s,pscPci=%s,sig1=%d,sig2=%d,"
                    "sig1_dbm=%d,sig1_dbm=%d,ta=%d,rssnr=%d,cqi=%d,bsic=%d,arfcn=%d,"
                    "long_name=%s,short_name=%s",
                    act,cid,mcc,mnc,lac_s,cid_s,sig1,sig2,
                    rsrp,rsrq,timingAdvance,rssnr,cqi,bsic,arfcn,
                    longname, shortname);
        }
        if (lac_s) free(lac_s);
        if (cid_s) free(cid_s);
        /* <Act>  0: GSM , 2: UMTS , 7: LTE */
        if(act == 7){
            // LOGD("RIL_CELL_INFO_TYPE_LTE act=%d",act);
            response[i].cellInfoType = RIL_CELL_INFO_TYPE_LTE;
            response[i].CellInfo.lte.cellIdentityLte.ci = cid;
            response[i].CellInfo.lte.cellIdentityLte.mcc = mcc;
            response[i].CellInfo.lte.cellIdentityLte.mnc = mnc;
            response[i].CellInfo.lte.cellIdentityLte.mnc_len = mnc_len;
            response[i].CellInfo.lte.cellIdentityLte.tac = lacTac;
            response[i].CellInfo.lte.cellIdentityLte.pci = pscPci;
            response[i].CellInfo.lte.cellIdentityLte.earfcn = arfcn;
            // bandwidth is unknown for now.
            response[i].CellInfo.lte.cellIdentityLte.bandwidth = 0x7FFFFFFF;
            response[i].CellInfo.lte.signalStrengthLte.signalStrength = sig1;

            int aosp_rsrp = -rsrp / 4;
            if (aosp_rsrp > 140) aosp_rsrp = 140;
            else if (aosp_rsrp < 44) aosp_rsrp = 44;
            response[i].CellInfo.lte.signalStrengthLte.rsrp = aosp_rsrp;

            int aosp_rsrq = rsrq != INVALID ? -rsrq / 4 : INVALID;
            if (aosp_rsrq > 35 && aosp_rsrq != INVALID) aosp_rsrq = 35;
            else if (aosp_rsrq < 3 && aosp_rsrq != INVALID) aosp_rsrq = 3;
            response[i].CellInfo.lte.signalStrengthLte.rsrq = aosp_rsrq;

            int aosp_ta = timingAdvance != INVALID ? timingAdvance : INVALID;
            if (aosp_ta < 0 && aosp_ta != INVALID) aosp_ta = 0;
            else if (aosp_ta > 1282 && aosp_ta != INVALID) aosp_ta = 1282;
            response[i].CellInfo.lte.signalStrengthLte.timingAdvance = aosp_ta;

            if (rssnr < 0 && rssnr != INVALID) rssnr = 0;
            else if (rssnr > 50 && rssnr != INVALID) rssnr = 50;
            response[i].CellInfo.lte.signalStrengthLte.rssnr = rssnr;

            if (cqi < 0 && cqi != INVALID) cqi = 0;
            else if (cqi > 30 && cqi != INVALID) cqi = 30;
            response[i].CellInfo.lte.signalStrengthLte.cqi = cqi;
            // append operator name
            response[i].CellInfo.lte.cellIdentityLte.operName.long_name
                    = strdup(longname);
            response[i].CellInfo.lte.cellIdentityLte.operName.short_name
                    = strdup(shortname);
        } else if(act == 2){
            // LOGD("RIL_CELL_INFO_TYPE_WCDMA act=%d",act);
            response[i].cellInfoType = RIL_CELL_INFO_TYPE_WCDMA;
            response[i].CellInfo.wcdma.cellIdentityWcdma.cid = cid;
            response[i].CellInfo.wcdma.cellIdentityWcdma.mcc = mcc;
            response[i].CellInfo.wcdma.cellIdentityWcdma.mnc = mnc;
            response[i].CellInfo.wcdma.cellIdentityWcdma.mnc_len = mnc_len;
            response[i].CellInfo.wcdma.cellIdentityWcdma.lac = lacTac;
            response[i].CellInfo.wcdma.cellIdentityWcdma.psc = pscPci;
            response[i].CellInfo.wcdma.cellIdentityWcdma.uarfcn = arfcn;
            response[i].CellInfo.wcdma.signalStrengthWcdma.signalStrength =
                    convert3GRssiValue(sig1-120);
            response[i].CellInfo.wcdma.signalStrengthWcdma.bitErrorRate = sig2;
            // append operator name
            response[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name
                    = strdup(longname);
            response[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name
                    = strdup(shortname);
        } else{
            // LOGD("RIL_CELL_INFO_TYPE_GSM act=%d",act);
            response[i].cellInfoType = RIL_CELL_INFO_TYPE_GSM;
            response[i].CellInfo.gsm.cellIdentityGsm.cid = cid;
            response[i].CellInfo.gsm.cellIdentityGsm.mcc = mcc;
            response[i].CellInfo.gsm.cellIdentityGsm.mnc = mnc;
            response[i].CellInfo.gsm.cellIdentityGsm.mnc_len = mnc_len;
            response[i].CellInfo.gsm.cellIdentityGsm.lac = lacTac;
            response[i].CellInfo.gsm.cellIdentityGsm.arfcn = arfcn;
            response[i].CellInfo.gsm.cellIdentityGsm.bsic = bsic;

            int aosp_level = (sig1 + 2) / 2;
            if (aosp_level > 31) aosp_level = 31;
            else if (aosp_level < 0) aosp_level = 0;
            response[i].CellInfo.gsm.signalStrengthGsm.signalStrength = aosp_level;
            response[i].CellInfo.gsm.signalStrengthGsm.bitErrorRate = sig2;

            if (timingAdvance < 0 && timingAdvance != INVALID) timingAdvance = 0;
            else if (timingAdvance > 219 && timingAdvance != INVALID) timingAdvance = 219;
            response[i].CellInfo.gsm.signalStrengthGsm.timingAdvance = timingAdvance;

            // append operator name
            response[i].CellInfo.gsm.cellIdentityGsm.operName.long_name
                    = strdup(longname);
            response[i].CellInfo.gsm.cellIdentityGsm.operName.short_name
                    = strdup(shortname);
        }
    }
    return 0;
error:
    return -1;
}

void requestGetCellInfoList(void * data, size_t datalen __unused, RIL_Token t)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int err = 0;
    int num = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    RIL_CellInfo_v12 *response = NULL;
    RIL_NW_UNUSED_PARM(data);

    /* check if modem support ECELL ext3,ext4 */
    if (ECELLext3ext4Support[rid] == -1) {
        err = at_send_command("AT+ECELL=4", &p_response, NW_CHANNEL_CTX);
        if (err < 0 || p_response->success == 0) {
            LOGE("modem is not support AT+ECELL=4.");
            ECELLext3ext4Support[rid] = 0;
        } else {
            ECELLext3ext4Support[rid] = 1;
        }
        at_response_free(p_response);
        p_response = NULL;
    }

    err = at_send_command_singleline("AT+ECELL", "+ECELL:",
            &p_response, NW_CHANNEL_CTX);

    // +ECELL: <num_of_cell>...
    if (err < 0 || p_response->success == 0) goto error;
    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &num);
    if (err < 0) goto error;
    if (num < 1){
        LOGD("No cell info listed, num=%d",num);
        goto error;
    }

    response = (RIL_CellInfo_v12 *) malloc(num * sizeof(RIL_CellInfo_v12));
    if (response == NULL) {
        LOGE("requestGetCellInfoList malloc fail");
        goto error;
    }
    memset(response, 0, num * sizeof(RIL_CellInfo_v12));

    err = getCellInfoListV12(line, num, response, rid);
    if (err < 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, num * sizeof(RIL_CellInfo_v12));

    at_response_free(p_response);

    if(response != NULL){
        // free all operator name.
        int i = 0;
        for (i = 0; i < num; i++) {
            switch (response[i].cellInfoType) {
                case RIL_CELL_INFO_TYPE_GSM: {
                    if (response[i].CellInfo.gsm.cellIdentityGsm.operName.long_name)
                        free(response[i].CellInfo.gsm.cellIdentityGsm.operName.long_name);
                    if (response[i].CellInfo.gsm.cellIdentityGsm.operName.short_name)
                        free(response[i].CellInfo.gsm.cellIdentityGsm.operName.short_name);
                    break;
                }
                case RIL_CELL_INFO_TYPE_WCDMA: {
                    if (response[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name)
                        free(response[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name);
                    if (response[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name)
                        free(response[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name);
                    break;
                }
                case RIL_CELL_INFO_TYPE_LTE: {
                    if (response[i].CellInfo.lte.cellIdentityLte.operName.long_name)
                        free(response[i].CellInfo.lte.cellIdentityLte.operName.long_name);
                    if (response[i].CellInfo.lte.cellIdentityLte.operName.short_name)
                        free(response[i].CellInfo.lte.cellIdentityLte.operName.short_name);
                    break;
                }
                case RIL_CELL_INFO_TYPE_NR:
                default:
                    break;
            }
        }
        free(response);
    }
    return;

error:
    at_response_free(p_response);
    if(response != NULL){
        free(response);
    }
    LOGE("requestGetCellInfoList must never return error when radio is on");
    RIL_onRequestComplete(t, RIL_E_NO_NETWORK_FOUND, NULL, 0);
    return;
}

void requestSetCellInfoListRate(void * data, size_t datalen __unused, RIL_Token t)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int err=0;
    int time = -1;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    RIL_Errno err_no = RIL_E_SUCCESS;

    time = ((int *)data)[0];

    LOGD("requestSetCellInfoListRate time: %d , rid: %d, ECELLext3ext4Support: %d"
            , time, rid, ECELLext3ext4Support[rid]);

    /* check if modem support ECELL ext3,ext4 */
    if (ECELLext3ext4Support[rid] == -1) {
        err = at_send_command("AT+ECELL=4", &p_response, NW_CHANNEL_CTX);
        if (err < 0 || p_response->success == 0) {
            LOGE("modem is not support AT+ECELL=4.");
            ECELLext3ext4Support[rid] = 0;
        } else {
            ECELLext3ext4Support[rid] = 1;
        }
        at_response_free(p_response);
        p_response = NULL;
    }

    /* According to ril.h
     * Sets the minimum time between when RIL_UNSOL_CELL_INFO_LIST should be invoked.
     * A value of 0, means invoke RIL_UNSOL_CELL_INFO_LIST when any of the reported
     * information changes. Setting the value to INT_MAX(0x7fffffff) means never issue
     * a RIL_UNSOL_CELL_INFO_LIST.
     */

    /* To prevent cell info chang report too much to increase the power consumption
       Modem only support disable/enable +ECELL URC. +ECELL URC will be reported for any of cell info change.
       For other rate , the cell info report is done by FW tol query cell info every time rate */
    if(time == 0){
        asprintf(&cmd, "AT+ECELL=1");
    }else if(time <= 0x7fffffff){
        asprintf(&cmd, "AT+ECELL=0");
    }else{
        goto error;
    }

    err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response->success == 0) {
        LOGE("[RIL%d]AT+ECELL return ERROR", rid+1 );
        goto error;
    }
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}


void requestStartNetworkScan(void * data, size_t datalen __unused, RIL_Token t) {
    // RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    RIL_NetworkScanRequest *p_args = (RIL_NetworkScanRequest*) data;
    LOGE("requestStartNetworkScan:: type: %d", p_args->type);
    RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
    return;
}

void requestStopNetworkScan(void * data, size_t datalen __unused, RIL_Token t) {
    // RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    LOGE("requestStopNetworkScan!");
    RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
    return;
}

void onPseudoCellInfo(char* urc, const RIL_SOCKET_ID rid)
{
    int err;
    int count;
    int i;
    int type = 0, plmn = 0, lac = 0, cid = 0, arfcn = 0, bsic = 0;
    int response[13] = {0};

     /*  URC :
      *     <count><type><plmn><lac><cid><arfcn><bsic>[<type><plmn><lac><cid><arfcn><bsic>]
      *     count: 1 or 2
      */

    err = at_tok_start(&urc);
    if (err < 0) goto error;

    err = at_tok_nextint(&urc, &count);
    if (err < 0) goto error;
    response[0] = count;

    for (i = 0; i < count; i++) {
        err = at_tok_nextint(&urc, &type);
        if (err < 0) goto error;
        response[i*6 + 1] = type;

        if (at_tok_hasmore(&urc)) {
            err = at_tok_nextint(&urc, &plmn);
            if (err < 0) goto error;

            err = at_tok_nexthexint(&urc, &lac);
            if (err < 0) goto error;

            err = at_tok_nexthexint(&urc, &cid);
            if (err < 0) goto error;

            err = at_tok_nextint(&urc, &arfcn);
            if (err < 0) goto error;

            err = at_tok_nextint(&urc, &bsic);
            if (err < 0) goto error;

            response[i*6 + 2] = plmn;
            response[i*6 + 3] = lac;
            response[i*6 + 4] = cid;
            response[i*6 + 5] = arfcn;
            response[i*6 + 6] = bsic;
        }
    }

    RIL_onUnsolicitedResponse(RIL_UNSOL_PSEUDO_CELL_INFO, response, sizeof(response), rid);
    return;

error:
    LOGE("onPseudoCellInfo error");
    return;
}

void requestSetPseudoCellMode(void * data, size_t datalen __unused, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;
    char *cmd = NULL;
    char *property = NULL;

    /*
        *    AT+EAPC=<apc mode>[,<report mode>,<timer>]
        *    <apc mode>: integer
        *         0: disable APC feature
        *         1: enable APC feature mode I
        *         2: enable APC feature mode II
        *     <report mode>
        *         0: disable auto report
        *         1: enable auto report
        *     <timer>
        *         auto report timer interval
        */

    int apc_mode = ((int*)data)[0];
    int report_mode = ((int*)data)[1];
    int timer = ((int*)data)[2];
    RIL_NW_UNUSED_PARM(datalen);

    asprintf(&cmd, "AT+EAPC=%d,%d,%d", apc_mode, report_mode, timer);
    err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    asprintf(&property, "persist.vendor.radio.apc.mode%d", rid);
    property_set(property, cmd);
    LOGD("update property %s = %s", property, cmd);
    free(property);
    free(cmd);
    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestGetPseudoCellInfo(void * data __unused, size_t datalen __unused, RIL_Token t)
{
    ATResponse *p_response = NULL;
    char *line = NULL;
    int mode = 0, report = 0, time = 0;
    int count = 0;
    int err = 0;
    int i = 0;
    int type = 0, plmn = 0, lac = 0, cid = 0, arfcn = 0, bsic = 0;
    int response[16] = {0};
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    err = at_send_command_singleline("AT+EAPC?", "+EAPC:", &p_response, NW_CHANNEL_CTX);
    if (err < 0 || p_response->success == 0 ||
            p_response->p_intermediates  == NULL) {
        goto error;
    }
    /*  response:
     *    +EAPC:<APC_mode>[<report_mode><time><count>
     *        [<type><plmn><lac><cid><arfcn><bsic>[<type><plmn><lac><cid><arfcn><bsic>]]]
     */
    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &mode);
    if (err < 0) goto error;
    response[0] = mode;

    if (at_tok_hasmore(&line)) {
        err = at_tok_nextint(&line, &report);
        if (err < 0) goto error;
        response[1] = report;

        err = at_tok_nextint(&line, &time);
        if (err < 0) goto error;
        response[2] = time;

        err = at_tok_nextint(&line, &count);
        if (err < 0) goto error;
        response[3] = count;

        for (i = 0; i < count; i++) {
            err = at_tok_nextint(&line, &type);
            if (err < 0) goto error;
            response[i*6 + 4] = type;

            if (at_tok_hasmore(&line)) {
                err = at_tok_nextint(&line, &plmn);
                if (err < 0) goto error;

                err = at_tok_nexthexint(&line, &lac);
                if (err < 0) goto error;

                err = at_tok_nexthexint(&line, &cid);
                if (err < 0) goto error;

                err = at_tok_nextint(&line, &arfcn);
                if (err < 0) goto error;

                err = at_tok_nextint(&line, &bsic);
                if (err < 0) goto error;

                response[i*6 + 5] = plmn;
                response[i*6 + 6] = lac;
                response[i*6 + 7] = cid;
                response[i*6 + 8] = arfcn;
                response[i*6 + 9] = bsic;
            }
        }
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestVoiceRadioTech(void * data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err = 0;
    char *line;
    int ret;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    RIL_RadioState state = getRadioState(rid);
    int voiceRadioTech;
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    if (RADIO_STATE_UNAVAILABLE == state) {
        RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        return;
    }

    err = at_send_command_singleline("AT+EPSB?", "+EPSB:", &p_response, getRILChannelCtxFromToken(t));

    if (err < 0 || p_response->success == 0) {
        // assume radio is off
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &ret);
    if (err < 0) goto error;

    /*  +EPSB: <bearer> */

    // GPRS: 0x01
    // EDGE: 0x02
    // WCDMA: 0x04
    // TD-SCDMA: 0x08
    // HSDPA: 0x10
    // HSUPA: 0x20
    // HSPA+: 0x40
    // LTE: 0x80

    if (ret == 0x01) {
        voiceRadioTech = RADIO_TECH_GPRS;
    } else if (ret == 0x02) {
        voiceRadioTech = RADIO_TECH_EDGE;
    } else if (ret == 0x04) {
        voiceRadioTech = RADIO_TECH_UMTS;
    } else if (ret >= 0x10 && ret < 0x20) {
        voiceRadioTech = RADIO_TECH_HSDPA;
    } else if (ret >= 0x20 && ret < 0x40) {
        voiceRadioTech = RADIO_TECH_HSUPA;
    } else if (ret >= 0x40 && ret < 0x80) {
        voiceRadioTech = RADIO_TECH_HSPAP;
    } else if (ret >= 0x80) {
        // To avoid phone switch by carrier config due to RAT is RADIO_TECH_LTE;
        voiceRadioTech = RADIO_TECH_UMTS;
    } else {
        // To avoid phone switch by carrier config due to RAT is RADIO_TECH_LTE;
        voiceRadioTech = RADIO_TECH_GPRS;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &voiceRadioTech, sizeof(int));
    // LOGD("[RIL%d] AT+EPSB return %d", rid+1, ret);
    at_response_free(p_response);
    return;

error:
    LOGE("[RIL%d] AT+EPSB return ERROR", rid+1);
    at_response_free(p_response);
}

void onSignalStrenth(char* urc, const RIL_SOCKET_ID rid) {
    int response[MAX_SIGNAL_DATA_SIZE] ={0};
    int err;

    memset(response, 0, sizeof(response));

    err = getSingnalStrength(urc, response, rid);
    if(err < 0) return;

    // compare the current and previous signal strength
    if (0 == memcmp(s_signal_response[rid], response, sizeof(response))) {
        LOGD("The current signal is the same as previous, ignore");
        return;
    } else if ((1 == s_reg_state[rid] || 5 == s_reg_state[rid]) && 99 == response[0]) {  // in service and rssi 99
        LOGD("Ignore rssi 99(unknown)");
        return;
    }

    // save current signal strength
    memcpy(s_signal_response[rid], response, sizeof(response));

    // 2G sim has only three parameter,3G sim has five parameters
    RIL_UNSOL_RESPONSE(RIL_UNSOL_SIGNAL_STRENGTH, response, sizeof(response), rid);
    return;
}

//ALPS00248788 START
void onInvalidSimInfo(char *urc,RIL_SOCKET_ID rid) {
    int err,id,cs_invalid,ps_invalid,cause;
    char *responseStr[4];
    const char *propTestSim = PROPERTY_RIL_TESTSIM;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    int is_testsim = RIL_isTestSim(rid);

    LOGD("onInvalidSimInfo ,is_testsim=%d", is_testsim);

    if(is_testsim == 1) {
        LOGD("onInvalidSimInfo. ignore due to test sim");
        return;
    }

    err = at_tok_start(&urc);
    if (err < 0) return;

    err = at_tok_nextint(&urc, &id);
    if (err < 0 || id != 16) return;

    if (!at_tok_hasmore(&urc)) {
        LOGE("There is no Invalid SIM info");
        return;
    }

    // get <plmn>
    err = at_tok_nextstr(&urc, &responseStr[0]);
    if (err < 0) {
        LOGE("ERROR occurs when parsing <plmn> of the invalid SIM URC");
        return;
    }

    // get <cs_invalid>
    err = at_tok_nextint(&urc, &cs_invalid);
    if (err <0) {
        LOGE("ERROR occurs when parsing <cs_invalid> of the invalid SIM URC");
        return;
    }

    // get <ps_invalid>
    err = at_tok_nextint(&urc, &ps_invalid);
    if (err <0) {
        LOGE("ERROR occurs when parsing <ps_invalid> of the invalid SIM URC");
        return;
    }

    // get <cause>
    err = at_tok_nextint(&urc, &cause);
    if (err <0) {
        LOGE("ERROR occurs when parsing <cause> of the invalid SIM URC");
        return;
    }

    asprintf(&responseStr[1], "%d", cs_invalid);
    asprintf(&responseStr[2], "%d", ps_invalid);
    asprintf(&responseStr[3], "%d", cause);

    /*  +EIND: 16,<plmn_id>,<cs_invalid>,<ps_invalid>,<cause>   ex:+EIND: 16,"46002",1,0,13 */
    LOGD("Invalid SIM info plmn: %s, cs_invalid %d, ps_invalid %d, cause %d",
            responseStr[0], cs_invalid, ps_invalid, cause);
    RIL_UNSOL_RESPONSE(RIL_UNSOL_INVALID_SIM, responseStr, sizeof(responseStr), rid);

    free(responseStr[1]);
    free(responseStr[2]);
    free(responseStr[3]);
    LOGD("Finish RIL_onUnsolicitedResponse");

}
//ALPS00248788 END
void onNetworkEventReceived(char* urc, const RIL_SOCKET_ID rid) {
    /* +EREGINFO: <Act>,<event_type>
     * <Act>: Access technology (RAT)
     * <event_type>: 0: for RAU event
     *               1: for TAU event
     */

    int ret;
    int response[2] = {0};

    LOGD("onNetworkEventReceived:%s", urc);

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    // get <Act>
    ret = at_tok_nextint(&urc, &response[0]);
    if (ret < 0) goto error;

    // get <event_type>
    ret = at_tok_nextint(&urc, &response[1]);
    if (ret < 0) goto error;

    LOGD("onNetworkEventReceived: <Act>:%d, <event_typ>:%d", response[0], response[1]);

    RIL_UNSOL_RESPONSE (RIL_UNSOL_NETWORK_EVENT, response, sizeof(response), rid);
    return;

error:
    LOGE("There is something wrong with the URC +EREGINFO");
}

void confirmRatBegin(void* param) {
    RIL_SOCKET_ID rild = *((RIL_SOCKET_ID *) param);
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = getChannelCtxbyProxy();
    bool ret = false;
    int err;

    err = at_send_command("AT+ERPRAT", &p_response, p_channel);
    ret = (err < 0 || 0 == p_response->success) ? true:false;
    LOGD("confirmRatBegin, send command AT+ERPRAT, err = %d, ret=%d, rild=%d, count=%d",
        err, ret, rild, s_erprat_resend_count);
    at_response_free(p_response);
    p_response = NULL;
    // If get wrong result, we need check whether go on or not.
    if (ret) {
        s_erprat_resend_count++;
        if (s_erprat_resend_count == 10) {
            LOGD("confirmRatBegin, reach the maximum time, return directly.");
            return;
        }
        RIL_RadioState state = getRadioState(rild);
        if (RADIO_STATE_UNAVAILABLE == state || RADIO_STATE_OFF == state) {
            LOGD("confirmRatBegin, radio unavliable/off, return directly.");
            return;
        }
        RIL_requestProxyTimedCallback(confirmRatBegin, (void*)param, &TIMEVAL_5,
                getRILChannelId(RIL_SIM, rild), "confirmRatBegin");
    }
}

void onConfirmRatBegin(char* urc, const RIL_SOCKET_ID rid) {
    const RIL_SOCKET_ID *p_rilId = &s_pollSimId;
    RIL_NW_UNUSED_PARM(urc);
    if (RIL_SOCKET_2 == rid) {
        p_rilId = &s_pollSimId2;
    } else if (RIL_SOCKET_3 == rid) {
        p_rilId = &s_pollSimId3;
    } else if (RIL_SOCKET_4 == rid) {
        p_rilId = &s_pollSimId4;
    }
    LOGD("onConfirmRatBegin, rilId = %d", *p_rilId);
    s_erprat_resend_count = 0;
    RIL_requestProxyTimedCallback(confirmRatBegin, (void*)p_rilId, &TIMEVAL_0,
            getRILChannelId(RIL_SIM, rid), "confirmRatBegin");
}

/* Please refer to the setRadioState*/
void onRadioState(char* t, const RIL_SOCKET_ID rid)
{
    RIL_NW_UNUSED_PARM(t);
    RIL_NW_UNUSED_PARM(rid);
}

static void releasePsStateInfo(ps_state_info *pInfo) {
    if (pInfo != NULL) {
        free(pInfo);
    }
}

void onPsStateChanged(void* param) {
    // LOGD("Timed callback onPsStateChanged start");
    ps_state_info* pInfo = (ps_state_info*)param;
    int operNumericLength = 0;
    RILSubSystemId subSysId = RIL_NW;
    RILChannelCtx *pChannel = getChannelCtxbyProxy();
    int response[3];

    int err;
    char *eops_response = NULL;
    char *line;
    ATResponse *p_response = NULL;
    err = at_send_command_singleline("AT+EOPS?",
          "+EOPS:", &p_response, pChannel);
    if (err != 0 || p_response->success == 0 ||
        p_response->p_intermediates == NULL) {
        LOGE("EOPS got error response");
    } else {
        int skip;
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err >= 0) {
            err = at_tok_nextint(&line, &skip);
            /* <mode> */
            if ((err >= 0) &&( skip >= 0 && skip <= 4 && skip !=2)){
                /* <format> */
                err = at_tok_nextint(&line, &skip);
                if (err >= 0 && skip == 2)
                {
                    // a "+EOPS: 0, n" response is also possible
                    if (!at_tok_hasmore(&line)) {
                        ;
                    } else{
                        /* <oper> */
                        err = at_tok_nextstr(&line, &eops_response);
                        operNumericLength = strlen(eops_response);
                         /* Modem might response invalid PLMN ex: "", "000000" , "??????", all convert to "000000" */
                        if(!((eops_response[0] >= '0') && (eops_response[0] <= '9'))) {
                            memset(eops_response, 0, operNumericLength);
                        }
                        LOGD("EOPS Get operator code %s", eops_response);
                    }
                }
            }
        }
    }
    response[0] = pInfo->data_reg_state;
    if (eops_response != NULL) {
        pInfo->mcc_mnc = atoi(eops_response);
    }
    response[1] = pInfo->mcc_mnc;
    response[2] = pInfo->rat;
    LOGD("onPsStateChanged, data_reg_state:%d, mcc_mnc:%d, rat:%d", pInfo->data_reg_state,
            pInfo->mcc_mnc, pInfo->rat);
    RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED,
            response, sizeof(response), pInfo->rid);
    RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0, pInfo->rid);
    at_response_free(p_response);
    releasePsStateInfo(pInfo);
}

void onNetworkStateChanged(char *urc, const RIL_SOCKET_ID rid)
{
    int err, is_cs, is_r8 = 0;
    int stat;
    unsigned int response[4] = {-1};
    char* responseStr[5] = {0};
    int i = 0;

    if (strStartsWith(urc,"+CREG:")) {
        is_cs = 1;
        is_r8 = 0;
    } else if (strStartsWith(urc,"+CGREG:")) {
        is_cs = 0;
        is_r8 = 0;
    } else if (strStartsWith(urc,"+CEREG:")) {
        is_cs = 0;
        is_r8 = 0;
    } else if (strStartsWith(urc,"+PSBEARER:")) {
        is_cs = 0;
        is_r8 = 1;
    } else {
        LOGE("The URC is wrong: %s", urc);
        return;
    }

    err = at_tok_start(&urc);
    if (err < 0) goto error;

    err = at_tok_nextint(&urc, &stat);
    if (err < 0) goto error;

    //for Lte
    stat = convertRegState(stat);

    //ALPS00283717
    if (is_r8 == 0)
    {
        /* Note: There might not be full +CREG URC info when screen off
                   Full URC format: +CREG: <n>, <stat>, <lac>, <cid>, <Act>,<cause> */

        // set "-1" to indicate "the field is not present"
        for(i=1; i<5; i++)
            asprintf(&responseStr[i], "-1");

        // fill in <state>
        asprintf(&responseStr[0], "%d", stat);

        //get <lac>
        err = at_tok_nexthexint(&urc,(int*) &response[0]);
        if(err >= 0)
        {
            free(responseStr[1]);
            responseStr[1] = NULL;
            asprintf(&responseStr[1], "%x", response[0]);

            //get <ci>
            err = at_tok_nexthexint(&urc, (int*) &response[1]);
            if(err >= 0)
            {
                free(responseStr[2]);
                responseStr[2] = NULL;
                asprintf(&responseStr[2], "%x", response[1]);

                //get <Act>
                err = at_tok_nextint(&urc, (int*) &response[2]);
                if(err >= 0)
                {
                    free(responseStr[3]);
                    responseStr[3] = NULL;
                    asprintf(&responseStr[3], "%d", response[2]);

                    //if LTE support the nextInt will be <cause_type>
                    if(bCREGType3Support){
                        //get <cause_type>
                        LOGD("bCREGType3Support=1");
                        err = at_tok_nextint(&urc,(int*) &response[3]);
                        if (err >= 0)
                        {
                            //get <cause>
                            err = at_tok_nextint(&urc,(int*) &response[3]);
                            if(err >= 0)
                            {
                                free(responseStr[4]);
                                responseStr[4] = NULL;
                                asprintf(&responseStr[4], "%d", response[3]);
                            }
                        }
                    } else {
                        //get <cause>
                        err = at_tok_nextint(&urc,(int*) &response[3]);
                        if(err >= 0)
                        {
                            free(responseStr[4]);
                            responseStr[4] = NULL;
                            asprintf(&responseStr[4], "%d", response[3]);
                        }
                    }
                }
            }
        }
    }

    if (is_cs == 1)
    {
        int cid = response[1];
        int lac = response[0];
        int act = response[2];
        if((cid == -1) || (stat == 4)){
            /* state(4) is 'unknown'  and cid(-1) means cid was not provided in +creg URC */
            LOGD("No valid info to distinguish limited service and no service");
        }else{
            //[ALPS01451327] need to handle different format of Cid
            // if cid is 0x0fffffff means it is invalid
            // for backward comptable we keep filter of oxffff
            if (((stat == 0) || (stat == 2) || (stat == 3)) &&
                    ((cid != 0x0fffffff) && (lac != 0xffff)) &&
                    // do not set ECC when it is LTE. ECC depends IMS.
                    (act != 7)) {
                if(s_emergencly_only[rid]==0){
                    s_emergencly_only[rid]=1;
                    LOGD("Set s_emergencly_only[%d] to true",rid);
                }
            }
            else{
                if(s_emergencly_only[rid]==1){
                    s_emergencly_only[rid]=0;
                    LOGD("Set s_emergencly_only[%d] to false",rid);
                }
            }
        }

        RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_CS_NETWORK_STATE_CHANGED, responseStr, sizeof(responseStr), rid);
        RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0, rid);
    } else {
        if (is_r8 == 1) {
            RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0, rid);
        } else {
            ril_data_reg_status[rid] = stat;
            //stat = combineMalCgregState(rid);
            //RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED, &stat, sizeof(stat), rid);
            RIL_UNSOL_RESPONSE (RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0, rid);

            ps_state_info* pInfo = calloc(1, sizeof(ps_state_info));
            if (pInfo == NULL) {
                LOGE("onNetworkStateChanged calloc fail");
                goto error;
            }
            pInfo->data_reg_state = stat;
            pInfo->mcc_mnc = 0;
            pInfo->rat = response[2];
            pInfo->rid = rid;
            RIL_requestProxyTimedCallback (onPsStateChanged, (void*)pInfo, &TIMEVAL_0,
                getRILChannelId(RIL_NW, rid), "onPsStateChanged");
        }
    }

   // onRestrictedStateChanged(rid);

    return;

error:
    LOGE("There is something wrong with the URC");
}

void onNitzTimeReceived(char* urc, const RIL_SOCKET_ID rid)
{
    int err;
    int length, i, id;
    char nitz_string[30];
    char *line = NULL;
    char *tz = NULL, *dst = NULL;
    /*+CIEV: 9, <time>, <tz>[, <dt>]
    <time>: "yy/mm/dd, hh:mm:ss"
    <tz>: [+,-]<value>, <value> is quarters of an hour between local time and GMT
    <dt>: day_light_saving value*/

    /* Final format :  "yy/mm/dd,hh:mm:ss(+/-)tz[,dt]" */

    err = at_tok_start(&urc);
    if (err < 0) return;

    err = at_tok_nextint(&urc, &id);
    if (err < 0 || id != 9) return;

    if (!at_tok_hasmore(&urc))
    {
        LOGE("There is no NITZ data");
        return;
    }

    length = strlen(urc);
    if (length >= (int) sizeof(nitz_string))
    {
        LOGE("The length of the NITZ URC is too long: %d", length);
        return;
    }
    nitz_string[0] = '\0';

    // eliminate the white space first
    line = urc;
    for (i = 0; i<length ; i++)
    {
        if(urc[i] != ' ')
        {
            *(line++) = urc[i];
        }
    }
    *line = '\0';

    length = strlen(urc);
    if(length == 0)
    {
        LOGE("Receiving an empty NITZ");
        return;
    }

    // get <time>
    err = at_tok_nextstr(&urc, &line);
    if (err < 0)
    {
        LOGE("ERROR occurs when parsing <time> of the NITZ URC");
        return;
    }
    strncat(nitz_string, line, (30-strlen(nitz_string)-1));

    // concatenate the remaining with <time>
    if (strlen(urc) > 0) {
        strncat(nitz_string, urc, (30-strlen(nitz_string)-1));

        // cache <tz>
        err = at_tok_nextstr(&urc, &tz);
        if (err < 0)
        {
            LOGE("There is no valid <tz>");
            bNitzTzAvailble = 0;
        }
        else
        {
            bNitzTzAvailble = 1;
            strncpy(ril_nw_nitz_tz, tz, MAX_NITZ_TZ_DST_LENGTH-1);
            ril_nw_nitz_tz[MAX_NITZ_TZ_DST_LENGTH-1] = '\0';
        }

        // cache <dst>
        err = at_tok_nextstr(&urc, &dst);
        if (err < 0)
        {
            LOGE("There is no valid <dst>");
            bNitzDstAvailble = 0;
        }
        else
        {
            bNitzDstAvailble = 1;
            strncpy(ril_nw_nitz_dst, dst, MAX_NITZ_TZ_DST_LENGTH-1);
            ril_nw_nitz_dst[MAX_NITZ_TZ_DST_LENGTH-1] = '\0';
        }
    } else {
        LOGE("There is no remaining data, said <tz>[,<dt>]");
        return;
    }

    // ignore local time information in the EMM INFORMATION if SIB16 is broadcast by the network
    if (!bSIB16Received) {
        RIL_UNSOL_RESPONSE (RIL_UNSOL_NITZ_TIME_RECEIVED, nitz_string, sizeof(char *), rid);
    }
}

/* Add-BY-JUNGO-20101008-CTZV SUPPORT*/
void onNitzTzReceived(char *urc, const RIL_SOCKET_ID rid)
{
    int err, i = 0, length = 0;
    int response[2];
    char nitz_string[30];
    time_t calendar_time;
    struct tm *t_info = NULL;
    char *line = NULL;
    char *tz = NULL, *dst = NULL;
    /* Final format :  "yy/mm/dd,hh:mm:ss(+/-)tz[,dt]" */

    err = at_tok_start(&urc);
    if(err < 0) return;

    if(strlen(urc) > 0)
    {

        length = strlen(urc);
        // eliminate the white space first
        line = urc;
        for (i = 0; i<length ; i++)
        {
            if(urc[i] != ' ')
            {
                *(line++) = urc[i];
            }
        }
        *line = '\0';

        //get the system time to fullfit the NITZ string format
        calendar_time = time(NULL);
        if(-1 == calendar_time) return;

        t_info = gmtime(&calendar_time);
        if(NULL == t_info) return;

        memset(nitz_string, 0, sizeof(nitz_string));
        sprintf(nitz_string, "%02d/%02d/%02d,%02d:%02d:%02d%s",  //"yy/mm/dd,hh:mm:ss"
                (t_info->tm_year)%100,
                t_info->tm_mon+1,
                t_info->tm_mday,
                t_info->tm_hour,
                t_info->tm_min,
                t_info->tm_sec,
                urc);

        LOGD("NITZ:%s", nitz_string);

        // cache <tz>
        err = at_tok_nextstr(&urc, &tz);
        if (err < 0)
        {
            LOGE("There is no valid <tz>");
            bNitzTzAvailble = 0;
        }
        else
        {
            bNitzTzAvailble = 1;
            strncpy(ril_nw_nitz_tz, tz, MAX_NITZ_TZ_DST_LENGTH-1);
            ril_nw_nitz_tz[MAX_NITZ_TZ_DST_LENGTH-1] = '\0';
            property_set("vendor.ril.nw.ctzv_tz", ril_nw_nitz_tz);
        }

        // cache <dst>
        err = at_tok_nextstr(&urc, &dst);
        if (err < 0)
        {
            LOGE("There is no valid <dst>");
            bNitzDstAvailble = 0;
        }
        else
        {
            bNitzDstAvailble = 1;
            strncpy(ril_nw_nitz_dst, dst, MAX_NITZ_TZ_DST_LENGTH-1);
            ril_nw_nitz_dst[MAX_NITZ_TZ_DST_LENGTH-1] = '\0';
            property_set("vendor.ril.nw.ctzv_dst", ril_nw_nitz_dst);
        }

        // ignore local time information in the EMM INFORMATION if SIB16 is broadcast by the network
        /* Fwk may set back as wrong time.
        if (!bSIB16Received) {
            RIL_UNSOL_RESPONSE (RIL_UNSOL_NITZ_TIME_RECEIVED, nitz_string, sizeof(char *), rid);
        }
        */

    }
    else
    {
        LOGE("There is no remaining data, said <tz>[,<dt>]");
        return;
    }
}

int convertToModUtf8Encode(int src) {
    int rlt = src;
    int byte1 = 0;
    int byte2 = 0;
    int byte3 = 0;

    if (src > 0x7FF) {
        byte1 = (src >> 12) | 0xE0;
        byte2 = ((src >> 6) & 0x3F) | 0x80;
        byte3 = (src & 0x3F) | 0x80;
        rlt = (byte1 << 16) | (byte2 << 8) | byte3;
    } else if (src > 0x7F) {
        byte1 = (src >> 6) | 0xC0;
        byte2 = (src & 0x3F) | 0x80;
        rlt = (byte1 << 8) | byte2;
    }
    return rlt;
}

void convertToUtf8String(char *src) {
    int i;
    int idx;
    int len;
    int cvtCode;
    char temp_oper_name[MAX_OPER_NAME_LENGTH] = {0};

    idx = 0;
    len = strlen(src);
    memset(temp_oper_name, 0, sizeof(char) * MAX_OPER_NAME_LENGTH);
    for (i = 0; i < len; i++) {
        cvtCode = convertToModUtf8Encode(src[i]);
        LOGD("cvtUTF8 %x", cvtCode);
        if ((cvtCode & 0xFF0000) > 0) {
            if (idx >= MAX_OPER_NAME_LENGTH - 3) {
                break;
            }
            temp_oper_name[idx++] = (cvtCode & 0xFF0000) >> 16;
            temp_oper_name[idx++] = (cvtCode & 0xFF00) >> 8;
            temp_oper_name[idx++] = cvtCode & 0xFF;
        } else if ((cvtCode & 0xFF00) > 0) {
            if (idx >= MAX_OPER_NAME_LENGTH - 2) {
                break;
            }
            temp_oper_name[idx++] = (cvtCode & 0xFF00) >> 8;
            temp_oper_name[idx++] = cvtCode & 0xFF;
        } else {
            temp_oper_name[idx++] = cvtCode;
        }
        if (idx == MAX_OPER_NAME_LENGTH) {
            break;
        }
    }
    temp_oper_name[MAX_OPER_NAME_LENGTH - 1] = '\0';
    strncpy(src, temp_oper_name, MAX_OPER_NAME_LENGTH);
    LOGD("convertToUtf8String %s", src);
}

void onNitzOperNameReceived(char* urc, const RIL_SOCKET_ID rid)
{
    int err;
    int length, i, id;
    char nitz_string[101] = {0};
    char *line = NULL;
    char *oper_code;
    char *oper_lname;
    char *oper_sname;
    int is_lname_hex_str=0;
    int is_sname_hex_str=0;
    char temp_oper_name[MAX_OPER_NAME_LENGTH]={0};

    char t_oper_code[MAX_OPER_NAME_LENGTH]={0};
    char t_oper_lname[MAX_OPER_NAME_LENGTH]={0};
    char t_oper_sname[MAX_OPER_NAME_LENGTH]={0};

    memset(t_oper_code, 0, MAX_OPER_NAME_LENGTH);
    memset(t_oper_lname, 0, MAX_OPER_NAME_LENGTH);
    memset(t_oper_sname, 0, MAX_OPER_NAME_LENGTH);

    /* +CIEV: 10,"PLMN","long_name","short_name" */

    err = at_tok_start(&urc);
    if (err < 0) return;

    err = at_tok_nextint(&urc, &id);
    if (err < 0 || id != 10) return;

    if (!at_tok_hasmore(&urc)) {
        LOGE("There is no NITZ data");
        return;
    }

    oper_code   = t_oper_code;
    oper_lname  = t_oper_lname;
    oper_sname  = t_oper_sname;

    /* FIXME: it is more good to transfer the OPERATOR NAME to the Telephony Framework directly */

    err = at_tok_nextstr(&urc, &line);
    if (err < 0) goto error;
    strncpy(oper_code, line, MAX_OPER_NAME_LENGTH);
    oper_code[MAX_OPER_NAME_LENGTH-1] = '\0';

    err = at_tok_nextstr(&urc, &line);
    if (err < 0) goto error;
    strncpy(oper_lname, line, MAX_OPER_NAME_LENGTH);
    oper_lname[MAX_OPER_NAME_LENGTH-1] = '\0';

    err = at_tok_nextstr(&urc, &line);
    if (err < 0) goto error;
    strncpy(oper_sname, line, MAX_OPER_NAME_LENGTH);
    oper_sname[MAX_OPER_NAME_LENGTH-1] = '\0';

    /* ALPS00459516 start */
    if ((strlen(oper_lname)%8) == 0) {
        LOGD("strlen(oper_lname)=%lu", (unsigned long) strlen(oper_lname));

        length = strlen(oper_lname);
        if (oper_lname[length-1] == '@') {
            oper_lname[length-1] = '\0';
            LOGD("remove @ new oper_lname:%s", oper_lname);
        }
    }

    if ((strlen(oper_sname)%8) == 0) {
        LOGD("strlen(oper_sname)=%lu", (unsigned long) strlen(oper_sname));

        length = strlen(oper_sname);
        if (oper_sname[length-1] == '@') {
            oper_sname[length-1] = '\0';
            LOGD("remove @ new oper_sname:%s", oper_sname);
        }
    }
    /* ALPS00459516 end */

    /* ALPS00262905 start
       +CIEV: 10, <plmn_str>,<full_name_str>,<short_name_str>,<is_full_name_hex_str>,<is_short_name_hex_str> for UCS2 string */
    err = at_tok_nextint(&urc, &is_lname_hex_str);
    if (err >= 0) {
        LOGD("is_lname_hex_str=%d",is_lname_hex_str);

        if (is_lname_hex_str == 1) {
            /* ALPS00273663 Add specific prefix "uCs2" to identify this operator name is UCS2 format.  prefix + hex string ex: "uCs2806F767C79D1"  */
            memset(temp_oper_name, 0, sizeof(temp_oper_name));
            strncpy(temp_oper_name, "uCs2", 4);
            strncpy(&(temp_oper_name[4]), oper_lname, MAX_OPER_NAME_LENGTH-4);
            memset(oper_lname, 0, MAX_OPER_NAME_LENGTH);
            strncpy(oper_lname, temp_oper_name, MAX_OPER_NAME_LENGTH);
            LOGD("lname add prefix uCs2");
        } else {
            convertToUtf8String(oper_lname);
        }

        err = at_tok_nextint(&urc, &is_sname_hex_str);
        LOGD("is_sname_hex_str=%d", is_sname_hex_str);
        if ((err >= 0) && (is_sname_hex_str == 1)) {
            /* ALPS00273663 Add specific prefix "uCs2" to identify this operator name is UCS2 format.  prefix + hex string ex: "uCs2806F767C79D1"  */
            memset(temp_oper_name, 0, sizeof(temp_oper_name));
            strncpy(temp_oper_name, "uCs2", 4);
            strncpy(&(temp_oper_name[4]), oper_sname, MAX_OPER_NAME_LENGTH-4);
            memset(oper_sname, 0, MAX_OPER_NAME_LENGTH);
            strncpy(oper_sname, temp_oper_name, MAX_OPER_NAME_LENGTH);
            LOGD("sname Add prefix uCs2");
        } else {
            convertToUtf8String(oper_sname);
        }
    } else {
        convertToUtf8String(oper_lname);
        convertToUtf8String(oper_sname);
    }
    /* ALPS00262905 end */

    LOGD("Get NITZ Operator Name of RIL %d: %s %s %s", rid+1, oper_code, oper_lname, oper_sname);
    // M: workaround, some network provide unusual name with NITZ.
    if (strncmp(oper_lname, "operator", 8) == 0 ||
            strncmp(oper_sname, "operator", 8) == 0) {
        LOGE("NITZ has unusual name: \"operator\", skip it");
        return;
    }


    pthread_mutex_lock(&ril_nw_nitzName_mutex[rid]);
    LOGD("Get ril_nw_nitzName_mutex in the onNitzOperNameReceived");

    strncpy(ril_nw_nitz_oper_code[rid], oper_code, MAX_OPER_NAME_LENGTH);
    strncpy(ril_nw_nitz_oper_lname[rid], oper_lname, MAX_OPER_NAME_LENGTH);
    strncpy(ril_nw_nitz_oper_sname[rid], oper_sname, MAX_OPER_NAME_LENGTH);

    pthread_mutex_unlock(&ril_nw_nitzName_mutex[rid]);

    if (getMappingSIMByCurrentMode(rid) >= GEMINI_SIM_1) {
        int phoneId = getMappingSIMByCurrentMode(rid);
        setMSimProperty(phoneId, PROPERTY_NITZ_OPER_CODE, oper_code);
        setMSimProperty(phoneId, PROPERTY_NITZ_OPER_LNAME, oper_lname);
        setMSimProperty(phoneId, PROPERTY_NITZ_OPER_SNAME, oper_sname);
    }
    RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0, rid);

error:
    return;
}

void onRestrictedStateChanged(const RIL_SOCKET_ID rid)
{
    int state[1];
    int isSimInsert = isSimInserted(rid);
    int isCdmaSup = RatConfig_isC2kSupported();
    state[0] = ril_nw_cur_state[rid] & RIL_NW_ALL_RESTRICTIONS;
    // LOGD("[%d]RestrictedStateChanged: %d %d %d %d", rid, state[0], isSimInsert, isCdmaSup,
    //         ril_prev_nw_cur_state[rid]);

    /* ALPS02782212 only send RESTRICTED_STATE_CHANGED in non-C2k project */
    if (isSimInserted(rid) && !RatConfig_isC2kSupported()) {
        /* ALPS01438966 prevent sending redundant RESTRICTED_STATE_CHANGED event */
        if(state[0] != ril_prev_nw_cur_state[rid]){
            /* ALPS03475559 not to send RIL_UNSOL_RESTRICTED_STATE_CHANGED anymore */
            //RIL_UNSOL_RESPONSE (RIL_UNSOL_RESTRICTED_STATE_CHANGED, state, sizeof(state), rid);
            ril_prev_nw_cur_state[rid] = state[0];
        }
    }
}

void onModulationInfoReceived(char *urc, const RIL_SOCKET_ID rid)
{
    int ret;
    int response[1];
    int count = 1;

    // LOGD("onModulationInfoReceived:%s",urc);

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    // get <modulation>
    ret = at_tok_nextint(&urc, &response[0]);
    if (ret < 0) goto error;

    if (response[0] > 0xFF) {
        goto error;
    }

    LOGD("onModulationInfoReceived (<modulation>=%d)", response[0]);

    RIL_UNSOL_RESPONSE(RIL_UNSOL_MODULATION_INFO, &response, count * sizeof(int), rid);
    return;

error:
    LOGE("There is something wrong with the +EMODCFG");
}

void onCellInfoList(char *urc, const RIL_SOCKET_ID rid) {
    TimedCallbackParam* mParam = (TimedCallbackParam*)malloc(sizeof(TimedCallbackParam));
    if (mParam == NULL) {
        LOGE("onCellInfoList malloc fail");
        return;
    }
    mParam->rid = rid;
    asprintf(&(mParam->urc), "%s", urc);
    RIL_requestProxyTimedCallback(handleCellInfoList, mParam, &TIMEVAL_0,
            getRILChannelId(RIL_NW, rid), "handleCellInfoList");
}

void handleCellInfoList(void *param)
{
    int err = 0;
    int num = 0;
    RIL_CellInfo_v12 * response = NULL;

    if (param == NULL) goto error;
    TimedCallbackParam* mParam = (TimedCallbackParam*)param;
    char* urc = mParam->urc;
    RIL_SOCKET_ID rid = (RIL_SOCKET_ID)(mParam->rid);

    // +ECELL: <num_of_cell>...
    err = at_tok_start(&urc);
    if (err < 0) goto error;

    err = at_tok_nextint(&urc, &num);
    if (err < 0) goto error;
    if (num < 1){
        LOGD("No cell info listed, num=%d", num);
        goto error;
    }
    LOGD("Cell Info listed, number =%d", num);

    response = (RIL_CellInfo_v12 *) malloc(num * sizeof(RIL_CellInfo_v12));
    if (response == NULL) {
        LOGE("onCellInfoList malloc fail");
        goto error;
    }
    memset(response, 0, num * sizeof(RIL_CellInfo_v12));

    err = getCellInfoListV12(urc, num, response, rid);
    if (err < 0) goto error;

    RIL_UNSOL_RESPONSE(RIL_UNSOL_CELL_INFO_LIST,
            response,  num * sizeof(RIL_CellInfo_v12), rid);

    if(response != NULL){
        // free all operator name.
        int i = 0;
        for (i = 0; i < num; i++) {
            switch (response[i].cellInfoType) {
                case RIL_CELL_INFO_TYPE_GSM: {
                    if (response[i].CellInfo.gsm.cellIdentityGsm.operName.long_name)
                        free(response[i].CellInfo.gsm.cellIdentityGsm.operName.long_name);
                    if (response[i].CellInfo.gsm.cellIdentityGsm.operName.short_name)
                        free(response[i].CellInfo.gsm.cellIdentityGsm.operName.short_name);
                    break;
                }
                case RIL_CELL_INFO_TYPE_WCDMA: {
                    if (response[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name)
                        free(response[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name);
                    if (response[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name)
                        free(response[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name);
                    break;
                }
                case RIL_CELL_INFO_TYPE_LTE: {
                    if (response[i].CellInfo.lte.cellIdentityLte.operName.long_name)
                        free(response[i].CellInfo.lte.cellIdentityLte.operName.long_name);
                    if (response[i].CellInfo.lte.cellIdentityLte.operName.short_name)
                        free(response[i].CellInfo.lte.cellIdentityLte.operName.short_name);
                    break;
                }
                case RIL_CELL_INFO_TYPE_NR:
                default:
                    break;
            }
        }
        free(response);
    }
    if (mParam != NULL) {
        if (mParam->urc != NULL) free(mParam->urc);
        free(mParam);
    }
    return;

error:
    if(response != NULL){
        // free all operator name.
        int i = 0;
        for (i = 0; i < num; i++) {
            switch (response[i].cellInfoType) {
                case RIL_CELL_INFO_TYPE_GSM: {
                    if (response[i].CellInfo.gsm.cellIdentityGsm.operName.long_name)
                        free(response[i].CellInfo.gsm.cellIdentityGsm.operName.long_name);
                    if (response[i].CellInfo.gsm.cellIdentityGsm.operName.short_name)
                        free(response[i].CellInfo.gsm.cellIdentityGsm.operName.short_name);
                    break;
                }
                case RIL_CELL_INFO_TYPE_WCDMA: {
                    if (response[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name)
                        free(response[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name);
                    if (response[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name)
                        free(response[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name);
                    break;
                }
                case RIL_CELL_INFO_TYPE_LTE: {
                    if (response[i].CellInfo.lte.cellIdentityLte.operName.long_name)
                        free(response[i].CellInfo.lte.cellIdentityLte.operName.long_name);
                    if (response[i].CellInfo.lte.cellIdentityLte.operName.short_name)
                        free(response[i].CellInfo.lte.cellIdentityLte.operName.short_name);
                    break;
                }
                default:
                    break;
            }
        }
        free(response);
    }
    if (mParam != NULL) {
        if (mParam->urc != NULL) free(mParam->urc);
        free(mParam);
    }
    LOGE("onCellInfoList parse error");
    return;
}

void updateSignalStrength()
{
    ATResponse *p_response = NULL;
    int err;
    int response[MAX_SIGNAL_DATA_SIZE]={0};
    const RIL_SOCKET_ID rid = getRILIdByChannelCtx(getChannelCtxbyProxy());

    char *line;

    memset(response, 0, sizeof(response));

    err = at_send_command_singleline("AT+ECSQ", "+ECSQ:", &p_response, getChannelCtxbyProxy());

    if (err < 0 || p_response->success == 0 ||
            p_response->p_intermediates  == NULL) {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = getSingnalStrength(line, response, rid);

    if (err < 0) goto error;

    if (99 == response[0])
        LOGD("Ignore rssi 99(unknown)");
    else
        RIL_UNSOL_RESPONSE(RIL_UNSOL_SIGNAL_STRENGTH,
                response,
                sizeof(response),
                getRILIdByChannelCtx(getChannelCtxbyProxy()));

    at_response_free(p_response);
    return;

error:
    at_response_free(p_response);
    LOGE("updateSignalStrength ERROR: %d", err);
}

void onNetworkScanResult(char *urc, const RIL_SOCKET_ID rid) {
    ATResponse *p_response = NULL;
    RIL_NetworkScanResult resp; memset(&resp, 0, sizeof(RIL_NetworkScanResult));
    int err;
    LOGD("onNetworkScanResult");

    err = at_tok_start(&urc);
    if (err < 0) goto error;

    RIL_CellInfo_v12 *response = (RIL_CellInfo_v12 *) malloc(resp.network_infos_length);
    if (response == NULL) {
        LOGE("onNetworkScanResult malloc fail");
        goto error;
    }
    memset(response, 0, resp.network_infos_length);
    resp.network_infos = response;

    err = getCellInfoListV12(urc, resp.network_infos_length, resp.network_infos, rid);
    if (err < 0) {
        LOGE("onNetworkScanResult Cell Info fail");
        goto error;
    }

    RIL_UNSOL_RESPONSE(RIL_UNSOL_NETWORK_SCAN_RESULT,
            &resp,  sizeof(RIL_NetworkScanResult), rid);

    at_response_free(p_response);
    if (response != NULL) {
        free(response);
    }
    return;

error:
    at_response_free(p_response);
    if (response != NULL) {
        free(response);
    }
    LOGE("onNetworkScanResult ERROR: %d", err);
}

//Femtocell(CSG) feature START
int isOp12Plmn(char* plmn) {
    unsigned long i;
    if (plmn != NULL) {
        for (i = 0 ; i < (sizeof(sOp12Plmn)/sizeof(sOp12Plmn[0])) ; i++) {
            if (strcmp(plmn, sOp12Plmn[i]) == 0) {
                //LOGD("[isOp12Plmn] plmn:%s", plmn);
                return true;
            }
        }
    }
    return false;
}

void isFemtoCell(int regState, int cid, int act, int rid) {
    unsigned int i;
    int isFemtocell = 0;
    int eNBid = cid >> 8; // OP12: 0xFA000 - 0xFFFFF

    if (gFemtoCellInfo[rid].is_csg_cell == 1) {
        isFemtocell = 1;
    }

    if (isOp12Plmn(gFemtoCellInfo[rid].plmn_id) == true
            && act == 14  // LTE
            && regState == 1  // in home service
            && eNBid >= 1024000 && eNBid <= 1048575) { // OP12: 0xFA000 - 0xFFFFF
        isFemtocell = 2;
        gFemtoCellInfo[rid].act = 7; //E-UTRAN
        gFemtoCellInfo[rid].hnbName[0] = '\0';
    }

    if (gFemtoCellInfo[rid].is_femtocell != isFemtocell) {
        char *responseStr[10];
        gFemtoCellInfo[rid].is_femtocell = isFemtocell;
        LOGD("update isFemtoCell:%d", gFemtoCellInfo[rid].is_femtocell);
        /* Femtocell info:
        <domain>,<state>,<lognname>,<plmn_id>,<act>,<is_csg_cell/is_femto_cell>,<csg_id>,<csg_icon_type>,<hnb_name>,<cause> */
        asprintf(&responseStr[0], "%d", gFemtoCellInfo[rid].domain);
        asprintf(&responseStr[1], "%d", gFemtoCellInfo[rid].state);
        asprintf(&responseStr[2], "%s", gFemtoCellInfo[rid].optrAlphaLong);
        asprintf(&responseStr[3], "%s", gFemtoCellInfo[rid].plmn_id);
        asprintf(&responseStr[4], "%d", gFemtoCellInfo[rid].act);
        asprintf(&responseStr[5], "%d", gFemtoCellInfo[rid].is_femtocell);
        asprintf(&responseStr[6], "%d", gFemtoCellInfo[rid].csg_id);
        asprintf(&responseStr[7], "%d", gFemtoCellInfo[rid].csg_icon_type);
        asprintf(&responseStr[8], "%s", gFemtoCellInfo[rid].hnbName);
        asprintf(&responseStr[9], "%d", gFemtoCellInfo[rid].cause);
        RIL_UNSOL_RESPONSE(RIL_UNSOL_FEMTOCELL_INFO, responseStr, sizeof(responseStr), rid);
        for (i=0 ; i<(sizeof(responseStr)/sizeof(char*)) ; i++) {
            if (responseStr[i]) {
                // LOGD("free responseStr[%d]=%s", i, responseStr[i]);
                free(responseStr[i]);
                responseStr[i] = NULL;
            }
        }
    }
}

void requestGetFemtoCellList(void * data, size_t datalen, RIL_Token t)
{
    int err, len, i, j, num , act , csgId , csgIconType,numCsg;
    ATResponse *p_response = NULL;
    char *line;
    char **response = NULL;
    char *tmp, *block_p = NULL;
    const RIL_SOCKET_ID rid = getRILIdByChannelCtx(NW_CHANNEL_CTX);
    char shortname[MAX_OPER_NAME_LENGTH];
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    LOGD("requestGetFemtoCellList set csgListOngoing flag");
    csgListOngoing = 1;
    err = at_send_command_singleline("AT+ECSG=0", "+ECSG",&p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) goto error;

    line = p_response->p_intermediates->line;

    /* +ECSG: <num_plmn>,<plmn_id>,<act>,<num_csg>,<csg_id>,<csg_icon_type>,<hnb_name>[,...]
           AT Response Example
           +ECSG: 3,"46000",2,1,<csg_id_A>,<csg_type_A>,<hnb_name_A>,"46002",7,1,<csg_id_B>,<csg_type_B>,<hnb_name_B>,"46002",7,1,<csg_id_C>,<csg_type_C>,<hnb_name_C> */

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &num);
    if (err < 0) goto error;

    response = (char **) calloc(1, sizeof(char*)*num*6); // for string, each one is 25 bytes
    block_p = (char *) calloc(1, num* sizeof(char)*6*MAX_OPER_NAME_LENGTH);

    if (response == NULL || block_p == NULL) {
        LOGE("requestGetFemtoCellList calloc fail");
        goto error;
    }

    for (i = 0, j=0 ; i < num ; i++, j+=6) {
        /* <plmn numeric>,<act>,<plmn long alpha name>,<csgId>,,csgIconType>,<hnbName> */
        response[j+0] = &block_p[(j+0)*MAX_OPER_NAME_LENGTH];
        response[j+1] = &block_p[(j+1)*MAX_OPER_NAME_LENGTH];
        response[j+2] = &block_p[(j+2)*MAX_OPER_NAME_LENGTH];
        response[j+3] = &block_p[(j+3)*MAX_OPER_NAME_LENGTH];
        response[j+4] = &block_p[(j+4)*MAX_OPER_NAME_LENGTH];
        response[j+5] = &block_p[(j+5)*MAX_OPER_NAME_LENGTH];

        /* get <plmn> numeric code*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;
        sprintf(response[j+0], "%s", tmp);

        LOGD("parse plmn numeric= %s",response[j+0]);

        len = strlen(response[j+0]);
        if (len == 5 || len == 6) {
            err = getOperatorNamesFromNumericCode(
                      response[j+0], response[j+1],shortname, MAX_OPER_NAME_LENGTH, rid);
            if(err < 0) goto error;
            LOGD("parse plmn alpha= %s",response[j+1]);
        } else {
            LOGE("The length of the numeric code is incorrect");
            goto error;
        }

        /* get <AcT> 0 is "2G", 2 is "3G", 7 is "4G"*/
        err = at_tok_nextint(&line, &act);
        if (err < 0) goto error;
        sprintf(response[j+2], "%d", act);
        LOGD("parse Act= %s",response[j+2]);

        err = at_tok_nextint(&line, &numCsg);
        if (err < 0) goto error;

        /* get <csgId> */
        err = at_tok_nextint(&line, &csgId);
        if (err < 0) goto error;
        sprintf(response[j+3], "%d", csgId);
        LOGD("parse csgId= %s",response[j+3]);

        /* get <csgIconType> */
        err = at_tok_nextint(&line, &csgIconType);
        if (err < 0) goto error;

        sprintf(response[j+4], "%d", csgIconType);
        LOGD("parse and converted csgIconType= %s",response[j+4]);

        /* get <hnbName> */
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;
        sprintf(response[j+5], "%s", tmp);
        LOGD("parse hnbName= %s",response[j+5]);
    }

    LOGD("requestGetFemtoCellList sucess, clear csgListOngoing and csgListAbort flag");
    csgListOngoing = 0;
    csgListAbort = 0; /* always clear here to prevent race condition scenario */

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(char*)*num*6);
    at_response_free(p_response);
    free(response);
    free(block_p);
    return;
error:
    at_response_free(p_response);
    if (response) {
        LOGD("FREE!!");
        if (block_p != NULL) free(block_p);
        free(response);
    }

    LOGE("requestGetFemtoCellList must never return error when radio is on, csgListAbort=%d", csgListAbort);
    if (csgListAbort == 1) {
        RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }

    LOGD("requestGetFemtoCellList fail, clear csgListOngoing and csgListAbort flag");
    csgListOngoing = 0;
    csgListAbort =0; /* always clear here to prevent race condition scenario */
}


void requestAbortFemtoCellList(void * data, size_t datalen, RIL_Token t)
{
    int err;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_SUCCESS;
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    LOGD("AbortFemtoCellList execute while csgListOngoing=%d",csgListOngoing);
    if(csgListOngoing == 1){
        LOGD("AbortFemtoCellList set csgListAbort flag");
        csgListAbort =1;

        err = at_send_command("AT+ECSG=2", &p_response, NW_CHANNEL_CTX);

        if (err < 0 || p_response->success == 0) {
            ril_errno = RIL_E_GENERIC_FAILURE;
            csgListAbort =0;
            LOGD("AbortFemtoCellList fail,clear csgListAbort flag");
        } else {
            ril_errno = RIL_E_SUCCESS;
        }
    }

    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
    return;
}

void requestSelectFemtoCell(void * data, size_t datalen, RIL_Token t)
{
    int err;
    char * cmd, *numeric_code;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    int plmnLen , csgIdLen, i ,j;
    const char **strings = (const char **)data;
    RIL_NW_UNUSED_PARM(datalen);

    //parameters:  <plmn> , <act> , <csg id>
    plmnLen = strlen(strings[0]); // plmn
    csgIdLen = strlen(strings[2]); // csg id

    LOGD("requestSelectFemtoCell: plmn= %s,act= %s,csgId= %s",strings[0],strings[1],strings[2]);

    // check if the plmn code is valid digit or not
    for(i = 0; i < plmnLen ; i++) {
        if( strings[0][i] < '0' || strings[0][i] > '9')
            break;
    }

    // check if the csg Id code is valid digit or not
    for(j = 0; j < csgIdLen ; j++) {
        if( strings[2][j] < '0' || strings[2][j] > '9')
            break;
    }

    if((i == plmnLen) && (j == csgIdLen)) {
        err = asprintf(&cmd, "AT+ECSG=1,\"%s\",%s,%s", strings[0],strings[2],strings[1]);

        if(err >= 0) {
            err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);

            free(cmd);

            if ( !(err < 0 || p_response->success == 0) ){
                ril_errno = RIL_E_SUCCESS;
            }
        }
    } else {
        LOGE("the plmn or csgId code contains invalid digits");
    }

    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
    return;
}

void requestQueryFemtoCellSystemSelectionMode(void *data, size_t datalen, RIL_Token t)
{
    int err, mode;
    ATResponse *p_response = NULL;
    char *line;
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    LOGD("requestQueryFemtoCellSystemSelectionMode sending AT command");
    err = at_send_command_singleline("AT+EFSS?", "+EFSS:", &p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) goto error;

    line = p_response->p_intermediates->line;

    /* +EFSS: <mode>
       AT Response Example
       +EFSS: 0 */

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &mode);
    if (err < 0) goto error;

    LOGD("requestQueryFemtoCellSystemSelectionMode sucess, free memory");

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &mode, sizeof(int));
    at_response_free(p_response);
    return;

error:
    at_response_free(p_response);

    LOGD("requestGetPOLCapability must never return error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestSetFemtoCellSystemSelectionMode(void *data, size_t datalen, RIL_Token t)
{
    int err;
    char * cmd;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    int mode = ((int *)data)[0];
    RIL_NW_UNUSED_PARM(datalen);

    LOGD("requestSetFemtoCellSystemSelectionMode: mode=%d", mode);

    if ((mode >= 0) && (mode <= 2)) {
        err = asprintf(&cmd, "AT+EFSS=%d", mode);

        if (err >= 0) {
            err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);

            free(cmd);

            if ( !(err < 0 || p_response->success == 0) ){
                ril_errno = RIL_E_SUCCESS;
            }
        }
    } else {
        LOGE("mode is invalid");
    }

    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
    return;
}

void onFemtoCellInfo(char *urc, const RIL_SOCKET_ID rid)
{
    int err = 0;
    int domain = 0;
    int state = 0;
    int isCsgCell = 0;
    int act = 0;
    int csgId = 0;
    int iconType = 0;
    int cause = 0;
    unsigned int i=0;
    char *plmnId = NULL;
    char *shortOperName = NULL;
    char *longOperName = NULL;
    char *hnbName = NULL;
    //Use INT_MAX: 0x7FFFFFFF denotes invalid value
    int INVALID = 0x7FFFFFFF;

    /* +ECSG:  <domain>,<state>,<plmn_id>,<act>,<is_csg_cell>,<csg_id>,<csg_icon_type>,<hnb_name>,<cause> */
    pthread_mutex_lock(&ril_femtocell_mutex[rid]);

    err = at_tok_start(&urc);
    if (err < 0) goto error;

    err = at_tok_nextint(&urc, &domain);
    if (err < 0) goto error;
    gFemtoCellInfo[rid].domain = domain;

    err = at_tok_nextint(&urc, &state);
    if (err < 0) goto error;
    gFemtoCellInfo[rid].state = state;

    shortOperName = (char *) calloc(1, sizeof(char)*MAX_OPER_NAME_LENGTH);
    longOperName = (char *) calloc(1, sizeof(char)*MAX_OPER_NAME_LENGTH);

    if (shortOperName == NULL || longOperName == NULL) {
        LOGE("onFemtoCellInfo calloc fail");
        goto error;
    }

    err = at_tok_nextstr(&urc, &plmnId);
    if (err < 0) goto error;
    strncpy(gFemtoCellInfo[rid].plmn_id, plmnId, MAX_OPER_NAME_LENGTH);
    gFemtoCellInfo[rid].plmn_id[MAX_OPER_NAME_LENGTH - 1] = '\0';

    err = getOperatorNamesFromNumericCode(plmnId, longOperName, shortOperName, MAX_OPER_NAME_LENGTH, rid);
    free(shortOperName);
    if(err < 0) goto error;
    strncpy(gFemtoCellInfo[rid].optrAlphaLong, longOperName, MAX_OPER_NAME_LENGTH);
    gFemtoCellInfo[rid].optrAlphaLong[MAX_OPER_NAME_LENGTH - 1] = '\0';

    err = at_tok_nextint(&urc, &act);
    if (err < 0) goto error;
    gFemtoCellInfo[rid].act = act;

    err = at_tok_nextint(&urc, &isCsgCell);
    if (err < 0) goto error;
    gFemtoCellInfo[rid].is_csg_cell = isCsgCell;

    if(gFemtoCellInfo[rid].is_csg_cell == 1){
        err = at_tok_nextint(&urc, &csgId);
        if (err < 0) goto error;
        gFemtoCellInfo[rid].csg_id = csgId;

        err = at_tok_nextint(&urc, &iconType);
        if (err < 0) goto error;
        gFemtoCellInfo[rid].csg_icon_type = iconType;

        err = at_tok_nextstr(&urc, &hnbName);
        if (err < 0) goto error;
        strncpy(gFemtoCellInfo[rid].hnbName, hnbName, MAX_OPER_NAME_LENGTH);
        gFemtoCellInfo[rid].hnbName[MAX_OPER_NAME_LENGTH - 1] = '\0';
    } else{
        /* <csg_id>,<csg_icon_type>,<hnb_name> are only avaliable when <is_csg_cell> is 1. */
        gFemtoCellInfo[rid].csg_id = INVALID;
        gFemtoCellInfo[rid].csg_icon_type = INVALID;
        strncpy(gFemtoCellInfo[rid].hnbName, "", MAX_OPER_NAME_LENGTH-1);
    }

    /* For backward compatiable for modem +ECSG URC without <cause> */
    err = at_tok_nextint(&urc, &cause);
    if (err < 0) {
        gFemtoCellInfo[rid].cause = INVALID;
    } else {
        gFemtoCellInfo[rid].cause = cause;
    }
    pthread_mutex_unlock(&ril_femtocell_mutex[rid]);
    free(longOperName);

    RIL_UNSOL_RESPONSE (RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL,  0, rid);
    return;

error:
    LOGE("There is something wrong with the onFemtoCellInfo URC, err=%d",err);
    pthread_mutex_unlock(&ril_femtocell_mutex[rid]);
    if (longOperName != NULL) free(longOperName);
    if (shortOperName != NULL) free(shortOperName);
}
//Femtocell feature END

extern int rilNwMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request)
    {
    case RIL_REQUEST_SIGNAL_STRENGTH:
        requestSignalStrength(data, datalen, t);
        break;
    case RIL_REQUEST_VOICE_REGISTRATION_STATE:
        requestRegistrationState(data, datalen, t);
        break;
    case RIL_REQUEST_DATA_REGISTRATION_STATE:
        requestGprsRegistrationState(data, datalen, t);
        break;
    case RIL_REQUEST_OPERATOR:
        requestOperator(data, datalen, t);
        break;
    case RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC:
        requestSetNetworkSelectionAutomatic(data, datalen, t);
        break;
    case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE:
        requestQueryNetworkSelectionMode(data, datalen, t);
        break;
    case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL:
        requestSetNetworkSelectionManual(data, datalen, t);
        break;
    case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT:
        requestSetNetworkSelectionManualWithAct(data, datalen, t);
        break;
    case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS:
        requestQueryAvailableNetworks(data, datalen, t);
        break;
    case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT:
        requestQueryAvailableNetworksWithAct(data, datalen, t);
        break;
    case RIL_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS:
        requestAbortQueryAvailableNetworks(data, datalen, t);
        break;
    case RIL_REQUEST_SET_BAND_MODE:
        requestSetBandMode(data, datalen, t);
        break;
    case RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE:
        requestQueryAvailableBandMode(data, datalen, t);
        break;
    case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
        requestSetPreferredNetworkType(data, datalen, t);
        break;
    case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
        requestGetPreferredNetworkType(data, datalen, t);
        break;
    case RIL_REQUEST_GET_NEIGHBORING_CELL_IDS:
        requestGetNeighboringCellIds(data, datalen, t);
        break;
    case RIL_REQUEST_SET_LOCATION_UPDATES:
        requestSetLocationUpdates(data, datalen, t);
        break;
    case RIL_REQUEST_VOICE_RADIO_TECH:
        requestVoiceRadioTech(data, datalen, t);
        break;
    case RIL_REQUEST_GET_CELL_INFO_LIST:
        requestGetCellInfoList(data, datalen, t);
        break;
    case RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE:
        requestSetCellInfoListRate(data, datalen, t);
        break;
    case RIL_REQUEST_START_NETWORK_SCAN:
        requestStartNetworkScan(data, datalen, t);
        break;
    case RIL_REQUEST_STOP_NETWORK_SCAN:
        requestStopNetworkScan(data, datalen, t);
        break;
    /// M: [C2K][IR] Support SVLTE IR feature. @{
    case RIL_REQUEST_SET_REG_SUSPEND_ENABLED:
        requestSetRegSuspendEnabled(data, datalen, t);
        break;
    case RIL_REQUEST_RESUME_REGISTRATION:
        requestResumeRegistration(data, datalen, t);
        break;
    case RIL_LOCAL_REQUEST_RESET_SUSPEND:
        resetRegSuspend(data, datalen, t);
        break;
    /// M: [C2K][IR] Support SVLTE IR feature. @}
    /// M: [C2K 6M][NW]Set the SVLTE RAT mode. @{
    case RIL_LOCAL_REQUEST_SET_SVLTE_RAT_MODE:
        requestSetSvlteRatMode(data, datalen, t);
        break;
    /// M: [C2K][NW]Set the SVLTE RAT mode. @}
    case RIL_REQUEST_SET_PSEUDO_CELL_MODE:
        requestSetPseudoCellMode(data, datalen, t);
        break;
    case RIL_REQUEST_GET_PSEUDO_CELL_INFO:
        requestGetPseudoCellInfo(data, datalen, t);
        break;
    //Femtocell (CSG) feature START
    case RIL_REQUEST_GET_FEMTOCELL_LIST:
        requestGetFemtoCellList(data, datalen, t);
        break;
    case RIL_REQUEST_ABORT_FEMTOCELL_LIST:
        requestAbortFemtoCellList(data, datalen, t);
        break;
    case RIL_REQUEST_SELECT_FEMTOCELL:
        requestSelectFemtoCell(data, datalen, t);
        break;
    case RIL_REQUEST_QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE:
        requestQueryFemtoCellSystemSelectionMode(data, datalen, t);
        break;
    case RIL_REQUEST_SET_FEMTOCELL_SYSTEM_SELECTION_MODE:
        requestSetFemtoCellSystemSelectionMode(data, datalen, t);
        break;
    //Femtocell (CSG) feature END
    /// M: [Network][C2K] Set the FDD TDD Mode @}
    case RIL_LOCAL_REQUEST_SET_FDD_TDD_MODE:
        setInitFddTddMode (data, datalen, t);
        break;
    /// M @}
    // MTK_TC1_FEATURE for Antenna Testing start
    case RIL_REQUEST_VSS_ANTENNA_CONF:
        requestAntennaConf(data, datalen, t);
        break;
    case RIL_REQUEST_VSS_ANTENNA_INFO:
        requestAntennaInfo(data, datalen, t);
        break;
    // MTK_TC1_FEATURE for Antenna Testing end
    /// M: EPDG feature. Update PS state from MAL @{
    case RIL_LOCAL_REQUEST_MAL_PS_RGEGISTRATION_STATE:
        onMalPsStateChanged(data, datalen, t);
        break;
    /// @}
    case RIL_REQUEST_GET_POL_CAPABILITY:
        requestGetPOLCapability(data, datalen, t);
        break;
    case RIL_REQUEST_GET_POL_LIST:
        requestGetPOLList(data, datalen, t);
        break;
    case RIL_REQUEST_SET_POL_ENTRY:
        requestSetPOLEntry(data, datalen, t);
        break;
    default:
        return 0; /* no matched requests */
        break;
    }

    return 1; /* request find and handled */
}

int rilNwUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel)
{
    RIL_NW_UNUSED_PARM(sms_pdu);

    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    if (strStartsWith(s, "+CREG:") || strStartsWith(s, "+CGREG:") || strStartsWith(s, "+PSBEARER:") || strStartsWith(s, "+CEREG:"))
    {
        onNetworkStateChanged((char*) s,rid);
        return 1;
    }
    else if (strStartsWith(s, "+ECSQ:"))
    {
        onSignalStrenth((char*) s,rid);
        return 1;
    }
    else if (strStartsWith(s, "+EREGINFO:"))
    {
        onNetworkEventReceived((char*) s,rid);
        return 1;
    }
    else if (strStartsWith(s, "+EMODCFG:"))
    {
        onModulationInfoReceived((char*) s, rid);
        return 1;
    }
    else if (strStartsWith(s, "+CIEV: 9,"))
    {
        onNitzTimeReceived((char*) s,rid);
        return 1;
    }
    else if (strStartsWith(s, "+CIEV: 10,"))
    {
        onNitzOperNameReceived((char*) s,rid);
        return 1;
    }
    // Notifies information for LTE SIB 16 time message
    else if (strStartsWith(s, "+CIEV: 11,"))
    {
        onSib16TimeInfoReceived((char*) s,rid);
        return 1;
    }
    else if (strStartsWith(s, "+CTZV:")) //Add by Jugno 20101008
    {
        onNitzTzReceived((char*) s, rid);
        return 1;
    }
    else if (strStartsWith(s, "+EIND: 16")) //ALPS00248788
    {
        onInvalidSimInfo((char*) s, rid);
        return 1;
    }
    /// M:[IR] @{
    else if (strStartsWith(s, "+ECOPS:"))
    {
        onPlmnListChanged((char*) s, rid);
        return 1;
    }
    else if (strStartsWith(s, "+EMSR:"))
    {
        onRegistrationSuspended((char*) s, rid);
        return 1;
    }
    else if (strStartsWith(s, "+EGMSS:"))
    {
        onGmssRat((char*) s, rid);
        return 1;
    }
    /// M: [IR] @}
    else if (strStartsWith(s, "+ECELL:"))
    {
        onCellInfoList((char*) s, rid);
        return 1;
    }
    //Femtocell (CSG) feature
    else if (strStartsWith(s, "+ECSG:"))
    {
        onFemtoCellInfo((char*) s, rid);
        return 1;
    }
    //M: [Modem Feature, add ERPRAT] Start
    else if (strStartsWith(s, "+ERPRAT"))
    {
        onConfirmRatBegin((char*) s, rid);
        return 1;
    }
    //M: [Modem Feature, add ERPRAT] End
    else if (strStartsWith(s, "+EAPC:"))
    {
        onPseudoCellInfo((char*) s,rid);
        return 1;
    }
    /// M:[World Mode] @{
    else if (strStartsWith(s, "+ECSRA:"))
    {
        if (bUbinSupport) {
            onWorldModeChanged((char*) s, rid);
        } else {
            /// M: [Network][C2K] receive ECSRA and set LTE band mode @{
            onLteRadioCapabilityChanged((char*) s, rid);
            /// @}
        }
        return 1;
    }
    else if (strStartsWith((char*) s, "+ESMCI:"))
    {  // switch modem cause information
        LOGD("OEM URC:%s", s);
        onSwitchModemCauseInformation((char*) s, rid);
        return 1;
    }
    /// M: [World Mode] @}
    else if (strStartsWith(s, "+ENWINFO:"))
    {
        onNetworkInfo((char*) s,rid);
        return 1;
    }
    else if (strStartsWith(s, "+EIMSESS:"))
    {
        onImsEmergencySupportR9((char*) s,rid);
        return 1;
    }
    return 0;
}
void requestGetOplmnVersion(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err;
    char *line, *version;
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    err = at_send_command_singleline("AT+EPOL?", "+EPOL:", &p_response,
            getRILChannelCtxFromToken(t));
    if (err < 0 || p_response == NULL) {
        goto error;
    }

    //+EPOL: <version>
    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
        goto error;

    err = at_tok_nextstr(&line, &version);
    if (err < 0)
        goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, version, sizeof(char *));
    at_response_free(p_response);
    return;
    error: RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestSendOplmn(void *data, size_t datalen, RIL_Token t) {
    int ret;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    RIL_NW_UNUSED_PARM(datalen);

    LOGD("Request Send oplmn file: %s", (char*) data);

    asprintf(&cmd, "AT+EPOL=%s", (char*) data);

    ret = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);

    free(cmd);

    if (ret < 0 || p_response->success == 0)
    {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    else
    {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }

    at_response_free(p_response);
}

/// M: EPDG feature. Update PS state from MAL
extern void onMalPsStateChanged(void *data, size_t datalen, RIL_Token t){
        int err;
        int stat;
        char *urc;
        RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
        urc = (char *)data;

        RIL_NW_UNUSED_PARM(datalen);

        LOGD("onMalPsStateChanged: URC is %s", urc);

        err = at_tok_start(&urc);
        if (err < 0) goto error;

        err = at_tok_nextint(&urc, &stat);
        if (err < 0) goto error;

        if (stat == 99 || stat == 98) {
            ril_mal_data_reg_status[rid] = stat; // backup stat
            // write WFC_STATE for fwk.
            int phoneId = getMappingSIMByCurrentMode(rid);
            if (stat == 99) setMSimProperty(phoneId, PROPERTY_WFC_STATE, "1");
            else setMSimProperty(phoneId, PROPERTY_WFC_STATE, "0");

            stat = combineMalCgregState(rid);
        } else {
            goto error;
        }

        if (isEpdgSupport() && RatConfig_isC2kSupported()) {
            RIL_UNSOL_RESPONSE( RIL_LOCAL_GSM_UNSOL_MAL_PS_RGEGISTRATION_STATE,
                &ril_mal_data_reg_status[rid], sizeof(ril_mal_data_reg_status[rid]), rid);
            RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0, rid);
        } else {
            ps_state_info* pInfo = calloc(1, sizeof(ps_state_info));
            if (pInfo == NULL) {
                LOGE("onMalPsStateChanged calloc fail");
                goto error;
            }
            pInfo->data_reg_state = stat;
            pInfo->mcc_mnc = 0;
            pInfo->rat = 18;
            pInfo->rid = rid;
            RIL_requestProxyTimedCallback (onPsStateChanged, (void*)pInfo, &TIMEVAL_0,
                getRILChannelId(RIL_NW, rid), "onPsStateChanged");
        }

        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;

    error:
        LOGE("onMalPsStateChanged:There is something wrong with the URC");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

void clearRilNWMalCache() {
    LOGD("Mal restart, reset ril NW Mal cache");
    int i;
    for (i = 0; i < RIL_NW_NUM; ++i) {
        ril_mal_data_reg_status[i] = 0;
        setMSimProperty(i, PROPERTY_WFC_STATE, "0");
    }
}

void setUnsolResponseFilterSignalStrength(bool enable) {
    int err;
    ATResponse *p_response = NULL;

    if (enable) {
        // enable
        /* Enable get ECSQ URC */
        err = at_send_command("AT+ECSQ=1", &p_response, getChannelCtxbyProxy());
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+ECSQ=1");
        at_response_free(p_response);
        p_response = NULL;
    } else {
        // disable
        /* Disable get ECSQ URC */
        err = at_send_command("AT+ECSQ=0", &p_response, getChannelCtxbyProxy());
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+ECSQ=0");
        at_response_free(p_response);
        p_response = NULL;
    }
}

void setUnsolResponseFilterNetworkState(bool enable) {
    int err;
    ATResponse *p_response = NULL;

    if (enable) {
        // enable
        /* disable mtk optimized +CREG URC report mode, as standard */
        err = at_send_command("AT+ECREG=0", &p_response, getChannelCtxbyProxy());
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+ECREG=0");
        at_response_free(p_response);
        p_response = NULL;

        /* Disable Network registration events of the changes in LAC or CID */
        err = at_send_command("AT+CREG=3", &p_response, getChannelCtxbyProxy());
        if (err != 0 || p_response->success == 0){
            at_response_free(p_response);
            p_response = NULL;
            err = at_send_command("AT+CREG=2", &p_response, getChannelCtxbyProxy());
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CREG=2");
        }
        at_response_free(p_response);
        p_response = NULL;

        /* disable mtk optimized +CGREG URC report mode, as standard */
        err = at_send_command("AT+ECGREG=0", &p_response, getChannelCtxbyProxy());
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+ECGREG=0");
        at_response_free(p_response);
        p_response = NULL;

        err = at_send_command("AT+CGREG=3", &p_response, getChannelCtxbyProxy());
        if (err != 0 || p_response->success == 0) {
            at_response_free(p_response);
            p_response = NULL;
            err = at_send_command("AT+CGREG=2", &p_response, getChannelCtxbyProxy());
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CGREG=2");
        }
        at_response_free(p_response);
        p_response = NULL;

        /* Enable PSBEARER URC */
        err = at_send_command("AT+PSBEARER=1", &p_response, getChannelCtxbyProxy());
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+PSBEARER=1");
        at_response_free(p_response);
        p_response = NULL;

        /* disable mtk optimized +CEREG URC report mode, as standard */
        err = at_send_command("AT+ECEREG=0", &p_response, getChannelCtxbyProxy());
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+ECEREG=0");
        at_response_free(p_response);
        p_response = NULL;

        err = at_send_command("AT+CEREG=3", &p_response, getChannelCtxbyProxy());
        if (err != 0 || p_response->success == 0) {
            at_response_free(p_response);
            p_response = NULL;
            err = at_send_command("AT+CEREG=2", &p_response, getChannelCtxbyProxy());
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CEREG=2");
        }
        at_response_free(p_response);
        p_response = NULL;

        /* Enable EMODCFG URC */
        if (isEnableModulationReport()) {
            err = at_send_command("AT+EMODCFG=1", &p_response, getChannelCtxbyProxy());
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+EMODCFG=1");
            at_response_free(p_response);
            p_response = NULL;
        }

        /* Enable ECSG URC */
        if (isFemtocellSupport()) {
            at_send_command("AT+ECSG=4,1", &p_response, getChannelCtxbyProxy());
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+ECSG=4,1");
            at_response_free(p_response);
            p_response = NULL;
        }

        /* Enable EREGINFO URC */
        err = at_send_command("AT+EREGINFO=1", &p_response, getChannelCtxbyProxy());
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+EREGINFO=1");
        at_response_free(p_response);
        p_response = NULL;
    } else {
        // disable
        /* enable mtk optimized +CREG URC report mode,
           report +CREG when stat or act changed for CREG format 2 and 3 */
        err = at_send_command("AT+ECREG=1", &p_response, getChannelCtxbyProxy());
        if (err != 0 || p_response->success == 0) {
            // NOT support ECREG, set short format.
            at_response_free(p_response);
            p_response = NULL;
            /* Disable Network registration events of the changes in LAC or CID */
            err = at_send_command("AT+CREG=1", &p_response, getChannelCtxbyProxy());
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CREG=1");
        }
        at_response_free(p_response);
        p_response = NULL;

        /* enable mtk optimized +CGREG URC report mode,
           report +CGREG when stat or act changed, for CGREG format 2 and 3 */
        err = at_send_command("AT+ECGREG=1", &p_response, getChannelCtxbyProxy());
        if (err != 0 || p_response->success == 0) {
            at_response_free(p_response);
            p_response = NULL;
            err = at_send_command("AT+CGREG=1", &p_response, getChannelCtxbyProxy());
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CGREG=1");
        }
        at_response_free(p_response);
        p_response = NULL;

        /* Disable PSBEARER URC */
        err = at_send_command("AT+PSBEARER=0", &p_response, getChannelCtxbyProxy());
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+PSBEARER=0");
        at_response_free(p_response);
        p_response = NULL;

        /* enable mtk optimized +CEREG URC report mode,
           report +CGREG when stat or act changed, for CEREG format 2 and 3 */
        err = at_send_command("AT+ECEREG=1", &p_response, getChannelCtxbyProxy());
        if (err != 0 || p_response->success == 0) {
            at_response_free(p_response);
            p_response = NULL;
            err = at_send_command("AT+CEREG=1", &p_response, getChannelCtxbyProxy());
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CEREG=1");
        }
        at_response_free(p_response);
        p_response = NULL;

        /* Disable EMODCFG URC */
        if (isEnableModulationReport()) {
            err = at_send_command("AT+EMODCFG=0", &p_response, getChannelCtxbyProxy());
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+EMODCFG=0");
            at_response_free(p_response);
            p_response = NULL;
        }

        /* Disable ECSG URC */
        if (isFemtocellSupport()) {
            at_send_command("AT+ECSG=4,0", &p_response, getChannelCtxbyProxy());
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+ECSG=4,0");
            at_response_free(p_response);
            p_response = NULL;
        }

        /* Disable EREGINFO URC */
        err = at_send_command("AT+EREGINFO=0", &p_response, getChannelCtxbyProxy());
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+EREGINFO=0");
        at_response_free(p_response);
        p_response = NULL;
    }
}

void onNetworkInfo(char* urc, const RIL_SOCKET_ID rid)
{
    /* +ENWINFO: <type>,<nw_info> */
    int err;
    int type;
    char *responseStr[2] = {NULL};

    err = at_tok_start(&urc);
    if (err < 0) return;

    err = at_tok_nextint(&urc, &type);
    if (err < 0) return;

    /* Remove unecessary check ALPS00416213 */

    asprintf(&responseStr[0], "%d", type);

    // get raw data of structure of NW info
    err = at_tok_nextstr(&urc, &(responseStr[1]));
    if ( err < 0 ) return;

    LOGD("NWInfo: %s, len:%lu ,%s", responseStr[0], (unsigned long)strlen(responseStr[1]),responseStr[1]);

    RIL_UNSOL_RESPONSE (RIL_UNSOL_NETWORK_INFO, responseStr, sizeof(responseStr), rid);
    free(responseStr[0]);
}

void onImsEmergencySupportR9(char* urc, const RIL_SOCKET_ID rid)
{
    /* +EIMSESS: <rat>,<ecc> */
    int err;
    int rat, ecc;

    err = at_tok_start(&urc);
    if (err < 0) return;

    err = at_tok_nextint(&urc, &rat);
    if (err < 0) return;

    err = at_tok_nextint(&urc, &ecc);
    if (err < 0) return;

    if ( (rat == 3 || rat == 4) && (ecc == 1)) {
        ims_ecc_state[rid] = 1;
    } else {
        ims_ecc_state[rid] = 0;
    }
    LOGD("onImsEmergencySupportR9: %d", ims_ecc_state[rid]);

    RIL_UNSOL_RESPONSE (RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0, rid);
}


// MTK_TC1_FEATURE for Antenna Testing start
void requestAntennaConf(void * data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int antennaType, err;
    int response[2] = { 0 };
    char *cmd = NULL;
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    memset(response, 0, sizeof(response));
    RIL_NW_UNUSED_PARM(datalen);

    antennaType = ((int *)data)[0];
    response[0] = antennaType;
    response[1] = 0; // failed
    LOGD("Enter requestAntennaConf(), antennaType = %d ", antennaType);
    // AT command format as below : (for VZ_REQ_LTEB13NAC_6290)
    // AT+ERFTX=8, <type>[,<param1>,<param2>]
    // <param1> is decoded as below:
    //    1 - Normal dual receiver operation(default UE behaviour)
    //    2 - Single receiver operation 'enable primary receiver only'(disable secondary/MIMO receiver)
    //    3 - Single receiver operation 'enable secondary/MIMO receiver only (disable primary receiver)
    switch(antennaType){
        case 0:    // 0: signal information is not available on all Rx chains
            antennaType = 0;
            break;
        case 1:    // 1: Rx diversity bitmask for chain 0
            antennaType = 2;
            break;
        case 2:    // 2: Rx diversity bitmask for chain 1 is available
            antennaType = 3;
            break;
        case 3:    // 3: Signal information on both Rx chains is available.
            antennaType = 1;
            break;
        default:
            LOGE("requestAntennaConf: configuration is an invalid");
            break;
    }
    err = asprintf(&cmd, "AT+ERFTX=8,1,%d", antennaType);
    if (err >= 0 ){
        err = at_send_command(cmd, &p_response, getRILChannelCtxFromToken(t));
        free(cmd);
        if (err < 0 || p_response->success == 0) {
            if (antennaType == 0) {
                // This is special handl for disable all Rx chains
                // <param1>=0 - signal information is not available on all Rx chains
                ril_errno = RIL_E_SUCCESS;
                response[1] = 1;  // success
                antennaTestingType = antennaType;
            }
        } else {
            ril_errno = RIL_E_SUCCESS;
            response[1] = 1; // success
            // Keep this settings for query antenna info.
            antennaTestingType = antennaType;
        }
    }
    RIL_onRequestComplete(t, ril_errno, response, sizeof(response));
    at_response_free(p_response);
}

void requestAntennaInfo(void * data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int param1, param2, err, skip;
    int response[6] = { 0 };
    char *cmd = NULL, *line = NULL ;
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    memset(response, 0, sizeof(response));
    int *primary_antenna_rssi   = &response[0];
    int *relative_phase         = &response[1];
    int *secondary_antenna_rssi = &response[2];
    int *phase1                 = &response[3];
    int *rxState_0              = &response[4];
    int *rxState_1              = &response[5];
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    *primary_antenna_rssi   = 0;  // <primary_antenna_RSSI>
    *relative_phase         = 0;  // <relative_phase>
    *secondary_antenna_rssi = 0;  // <secondary_antenna_RSSI>
    *phase1                 = 0;  // N/A
    *rxState_0              = 0;  // rx0 status(0: not vaild; 1:valid)
    *rxState_1              = 0;  // rx1 status(0: not vaild; 1:valid)
    // AT+ERFTX=8, <type> [,<param1>,<param2>]
    // <type>=0 is used for VZ_REQ_LTEB13NAC_6290
    // <param1> represents the A0 bit in ANTENNA INFORMATION REQUEST message
    // <param2> represents the A1 bit in ANTENNA INFORMATION REQUEST message
    switch(antennaTestingType){
        case 0:    // signal information is not available on all Rx chains
            param1 = 0;
            param2 = 0;
            break;
        case 1:    // Normal dual receiver operation (default UE behaviour)
            param1 = 1;
            param2 = 1;
            break;
        case 2:    // enable primary receiver only
            param1 = 1;
            param2 = 0;
            break;
        case 3:    // enable secondary/MIMO receiver only
            param1 = 0;
            param2 = 1;
            break;
        default:
            LOGE("requestAntennaInfo: configuration is an invalid, antennaTestingType: %d", antennaTestingType);
            goto error;
    }
    LOGD("requestAntennaInfo: antennaType=%d, param1=%d, param2=%d", antennaTestingType, param1, param2);
    if (antennaTestingType == 0){
        err = asprintf(&cmd, "AT+ERFTX=8,0,%d,%d", param1, param2);
        if (err >= 0){
            err = at_send_command(cmd, &p_response, getRILChannelCtxFromToken(t));
            free(cmd);
            if (err >= 0 && p_response->success != 0) {
                ril_errno = RIL_E_SUCCESS;
            }
            RIL_onRequestComplete(t, ril_errno, response, sizeof(response));
            at_response_free(p_response);
        } else {
            LOGE("Using asprintf for set antenna test type and getting ERROR");
            goto error;
        }
        return;
    }
    // set antenna testing type
    err = asprintf(&cmd, "AT+ERFTX=8,1,%d", antennaTestingType);
    if (err >= 0){
        err = at_send_command(cmd, &p_response, getRILChannelCtxFromToken(t));
        free(cmd);
        if (err >= 0 && p_response->success != 0) {
            err = asprintf(&cmd, "AT+ERFTX=8,0,%d,%d", param1, param2);
            if (err >= 0){
                err = at_send_command_singleline(cmd, "+ERFTX:", &p_response, getRILChannelCtxFromToken(t));
                free(cmd);
                if (err >= 0 && p_response->success != 0) {
                    line = p_response->p_intermediates->line;
                    err = at_tok_start(&line);
                    if (err < 0) goto error;
                    // skip <op=8>
                    err = at_tok_nextint(&line, &skip);
                    if (err < 0) goto error;
                    // skip <type=0>
                    err = at_tok_nextint(&line, &skip);
                    if (err < 0) goto error;
                    err = at_tok_nextint(&line, primary_antenna_rssi);
                    if (err < 0) {
                        // response for AT+ERFTX=8,0,0,1
                        // Ex: +ERFTX: 8,0,,100
                    } else {
                        // response for AT+ERFTX=8,0,1,1 or AT+ERFTX=8,0,1,0
                        // Ex: +ERFTX: 8,0,100,200,300 or +ERFTX: 8,0,100
                        *rxState_0 = 1;
                    }
                    if (at_tok_hasmore(&line)) {
                        err = at_tok_nextint(&line, secondary_antenna_rssi);
                        if (err < 0) {
                            LOGE("ERROR occurs <secondary_antenna_rssi> form antenna info request");
                            goto error;
                        } else {
                            // response for AT+ERFTX=8,0,1,0
                            // Ex: +ERFTX: 8,0,100
                            *rxState_1 = 1;
                        }
                        if (at_tok_hasmore(&line)) {
                            // response for AT+ERFTX=8,0,1,1
                            // Ex: +ERFTX: 8,0,100,200,300
                            err = at_tok_nextint(&line, relative_phase);
                            if (err < 0) {
                                LOGE("ERROR occurs <relative_phase> form antenna info request");
                                goto error;
                            }
                        }
                    }
                    ril_errno = RIL_E_SUCCESS;
                }
            } else {
                LOGE("Using asprintf for antenna info and getting ERROR");
                goto error;
            }
        } else {
            LOGE("Set antenna testing type getting ERROR");
            goto error;
        }
    } else {
        LOGE("Using asprintf for set antenna test type and getting ERROR");
        goto error;
    }
    RIL_onRequestComplete(t, ril_errno, response, sizeof(response));
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, ril_errno, response, sizeof(response));
    at_response_free(p_response);
}
// MTK_TC1_FEATURE for Antenna Testing end

void requestGetPOLCapability(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err;
    char *line, *temp;
    int result[4] = {0};
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RIL_NW_UNUSED_PARM(data);
    RIL_NW_UNUSED_PARM(datalen);

    err = at_send_command_singleline("AT+CPOL=?", "+CPOL:", &p_response, NW_CHANNEL_CTX);

    if (err < 0 || NULL == p_response) {
        LOGE("requestGetPOLCapability Fail");
        goto error;
    }

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            LOGD("p_response = %d /n", at_get_cme_error(p_response) );
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
                break;
            case CME_UNKNOWN:
                LOGD("p_response: CME_UNKNOWN");
                break;
            default:
                goto error;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }
    if (ret != RIL_E_SUCCESS) {
        LOGD("requestGetPOLCapability: Error or no SIM inserted!");
        goto error;
    }

    //+CPOL: (<bIndex>-<eIndex>), (<bformatValue>-<eformatValue>)
    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if(err < 0) goto error;

    err = at_tok_char(&line);
    if(err < 0) goto error;
    LOGD("requestGetPOLCapability: found first ( ");
    temp = strchr(line, '-');
    if (temp != NULL && line < temp ) {
        LOGD("requestGetPOLCapability: temp is %s, temp-line is %ld", temp,(long)(temp - line));
    }else {
        goto error;
    }
    err = at_tok_nextint(&line, &result[0]);
    if (err < 0) goto error;

    line = temp;
    LOGD("requestGetPOLCapability: %d , line is %s", result[0],line);

    if (*line == '-') {
        line++;
        LOGD("requestGetPOLCapability: skip -, line is %s", line);
    }
/*
    temp = strchr(line, ')');
    if (temp != NULL && line < temp ) {
        LOGD("requestGetPOLCapability: find ), temp is %s", temp);
    }
    */
    err = at_tok_nextint(&line, &result[1]);
    if (err < 0) goto error;
    LOGD("requestGetPOLCapability: %d , line is %s", result[1], line);
/*  line = temp;
    while (*line != '\0' && *line != ',') {
        LOGD("POL:  *line is %c", *line);
        line++;
    }

    LOGD("requestGetPOLCapability: skip , line is %s", line);
    err = at_tok_char(&line);
    LOGD("requestGetPOLCapability: line is %s", line);

    temp = strchr(line, '-');
    if (temp != NULL && line < temp ) {
        LOGD("requestGetPOLCapability: temp is %s, temp-line is %d", temp,temp - line );
    }else {
        goto error;
    }

    err = at_tok_nextint(&line, &result[2]);
    if(err < 0) goto error;
    LOGD("requestGetPOLCapability: %d ", result[2]);
    line = temp;
    if (*line == '-') {
        line++;
        LOGD("requestGetPOLCapability: skip -, line is %s", line);
    }

    err = at_tok_nextint(&line, &result[3]);
    if(err < 0) goto error;
    */
    result[2] = 0;
    result[3] = 2;

    LOGD("requestGetPOLCapability: %d %d %d %d", result[0],
            result[1], result[2], result[3]);


    RIL_onRequestComplete(t, RIL_E_SUCCESS, &result, 4*sizeof(int));

    at_response_free(p_response);
    return;
error:
    LOGD("requestGetPOLCapability: goto error ");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestGetPOLList(void *data, size_t datalen, RIL_Token t)
{
        int err,i,j,count,len, nAct,tmpAct;
        ATResponse *p_response = NULL;
        char *line;
        ATLine *p_cur;
        char **response = NULL;
        char *tmp, *block_p = NULL;
        RIL_NW_UNUSED_PARM(data);
        RIL_NW_UNUSED_PARM(datalen);

        err = at_send_command_multiline("AT+CPOL?", "+CPOL:",&p_response, NW_CHANNEL_CTX);

        if (err < 0 || p_response->success == 0)
        {
            goto error;
        }

        /* count the entries */
        for (count = 0, p_cur = p_response->p_intermediates
                ; p_cur != NULL
                ; p_cur = p_cur->p_next
        ) {
            count++;
        }
        LOGD("requestGetPOLList!! count is %d", count);

        response = (char **) calloc(1, sizeof(char*) * count *4); // for string, each one is 25 bytes
        /* In order to support E-UTRAN, nAct will be 2 digital,
        changed from 60 to 62 for addition 1 digital and buffer.*/
        block_p = (char *) calloc(1, count* sizeof(char)*62);

        if (response == NULL || block_p == NULL) {
            LOGE("requestGetPOLList calloc fail");
            goto error;
        }

        //+CPOL: <index>, <format>, <oper>, <GSM_Act>, <GSM_Compact_Act>, <UTRAN_Act>, <E-UTRAN Act>
        for (i = 0,j=0, p_cur = p_response->p_intermediates
                ; p_cur != NULL
                ; p_cur = p_cur->p_next, i++,j+=4
        ){
            line = p_cur->line;
            LOGD("requestGetPOLList!! line is %s",line);
            err = at_tok_start(&line);
            if(err < 0) goto error;

            /* get index*/
            err = at_tok_nextstr(&line, &tmp);
            if (err < 0) goto error;

            response[j+0] = &block_p[i*62];
            response[j+1] = &block_p[i*62+8];
            response[j+2] = &block_p[i*62+10];
            response[j+3] = &block_p[i*62+58];

            sprintf(response[j+0], "%s", tmp);

            LOGD("requestGetPOLList!! index is %s",response[j+0]);
            /* get format*/
            err = at_tok_nextstr(&line, &tmp);
            if (err < 0) goto error;
               sprintf(response[j+1], "%s", tmp);
            LOGD("requestGetPOLList!! format is %s",response[j+1]);
            /* get oper*/
            err = at_tok_nextstr(&line, &tmp);
            if (err < 0) goto error;
            sprintf(response[j+2], "%s", tmp);
            LOGD("requestGetPOLList!! oper is %s",response[j+2]);
            nAct = 0;

            if(at_tok_hasmore(&line))
            {
                /* get <GSM AcT> */
                err = at_tok_nextint(&line, &tmpAct);
                if (err < 0) goto error;
                if (tmpAct == 1){
                    nAct = 1;
                }

                /*get <GSM compact AcT> */
                err = at_tok_nextint(&line, &tmpAct);
                if (err < 0) goto error;
                if(tmpAct == 1){
                    nAct |= 0x02;
                }

                /*get <UTRAN AcT> */
                err = at_tok_nextint(&line, &tmpAct);
                if (err < 0) goto error;
                if(tmpAct == 1){
                    nAct |= 0x04;
                }

                /*get <E-UTRAN AcT> */
                if (at_tok_hasmore(&line)) {
                    LOGD("get E-UTRAN AcT");
                    err = at_tok_nextint(&line, &tmpAct);
                    if (err < 0) goto error;
                    if(tmpAct == 1){
                        nAct |= 0x08;
                    }
                }
            }
/* ALPS00368351 To distinguish SIM file without <AcT> support, we set AcT to zero */
////            if(nAct == 0) { nAct = 1;} // No act value for SIM. set to GSM
            LOGD("nAct = %d",nAct);
            sprintf(response[j+3], "%d", nAct);

            LOGD("requestGetPOLList!! act is %s",response[j+3]);

        }
        RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(char*)*count*4);
        at_response_free(p_response);
        free(response);
        free(block_p);
        return;
    error:
        LOGE("requestGetPOLList return error");
        at_response_free(p_response);
        if(response)
        {
            LOGD("FREE!!");
            if (block_p != NULL) free(block_p);
            free(response);
        }
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

}

void requestSetPOLEntry(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err, i;
    int nAct[4] = {0};
    int nActTmp = 0;
    const char**    strings = (const char**)data;
    char* cmd = NULL;
    char *line;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    if (strings[1] == NULL || strcmp("", strings[1]) == 0) {
        // do delete when numeric is null
        asprintf(&cmd, "AT+CPOL=%s", strings[0]);
    } else {
        nActTmp = atoi(strings[2]);
        LOGD("requestSetPOLEntry Act = %d",nActTmp);

        for(i = 0; i < 4; i++) {
            if(((nActTmp >> i) & 1) == 1) {
                LOGD("i = %d",i);
                nAct[i] = 1;
            }
        }

        /* ALPS00368351: To update file without <AcT> support, modem suggest not to set any nAcT parameter */
        if (nActTmp == 0) {
            LOGD("requestSetPOLEntry no Act assigned,strings[2]=%s",strings[2]);
            asprintf(&cmd, "AT+CPOL=%s,2,\"%s\"", strings[0], strings[1]);
        }else{
            LOGD("R8, MOLY and LR9 can supoort 7 arguments");
            asprintf(&cmd, "AT+CPOL=%s,2,\"%s\",%d,%d,%d,%d",
                    strings[0],
                    strings[1],
                    nAct[0],
                    nAct[1],
                    nAct[2],
                    nAct[3]);
        }
    }
    err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || NULL == p_response) {
        LOGE("requestSetPOLEntry Fail");
        goto error;
    }

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            LOGD("p_response = %d /n", at_get_cme_error(p_response) );
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
            break;
            case CME_UNKNOWN:
                LOGD("p_response: CME_UNKNOWN");
            break;
            default:
            break;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }
error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

void onSib16TimeInfoReceived(char* urc, const RIL_SOCKET_ID rid)
{
    int err=0, id=0, dst=0, ls=0, lto=0, dt=0;
    long long raw_utc = 0;

    // difference in milliseconds of epochs used by Android (1970) and the network (1900)
    long long epochDiffInMillis = 2208988800000;

    // abs_time = <raw_utc>*10 - <epochDiffInMillis>
    long long abs_time = 0;

    // currentUtcTimeMillis = raw_utc*10 - epochDiffInMillis + elapsedTimeSinceBroadcast
    time_t  currentUtcTimeMillis; // time_t is measured in seconds.
    struct tm *ts;

    char* responseStr[5] = {NULL};
    char nitz_string[30];
    char sib16_time_string[70];

    int i;

    /* +CIEV: 11, <UTC>, [<daylightSavingTime >], [<leapSeconds >], [<localTimeOffset >], <delayTicks>
      <UTC>: The field counts the number of UTC seconds in 10 ms units since 00:00:00 on Gregorian calendar date 1 January, 1900
      <daylightSavingTime>: It indicates if and how daylight saving time (DST) is applied to obtain the local time.
      <leapSeconds>: GPS time - leapSeconds = UTC time.
      <localTimeOffset>: Offset between UTC and local time in units of 15 minutes.
      <delayTicks>: Time difference from AS receive SIB16 to L4 receive notify

      Final format :  "yy/mm/dd,hh:mm:ss(+/-)tz[,dst]" */

    err = at_tok_start(&urc);
    if (err < 0) return;

    err = at_tok_nextint(&urc, &id);
    if (err < 0 || id != 11) return;

    if (!at_tok_hasmore(&urc))
    {
        LOGE("There is no SIB16 time info");
        return;
    }

    // get <UTC>
    err = at_tok_nextlonglong(&urc, &raw_utc);
    if (err < 0)
    {
        LOGE("ERROR occurs when parsing <UTC> of the SIB16 time info URC");
        return;
    }
    asprintf(&responseStr[0], "%lld", raw_utc);

    // get <daylightSavingTime >
    err = at_tok_nextint(&urc, &dst);
    if (err < 0)
    {
        if (bNitzDstAvailble)
        {
            LOGE("Use <daylightSavingTime> from (E)MM INFORMATION");
            asprintf(&responseStr[1], "%s", ril_nw_nitz_dst);
        }
        else
        {
            LOGE("ERROR occurs when parsing <daylightSavingTime> of the SIB16 time info URC");
            responseStr[1] = "";
            dst = -1;
        }
    }
    else
    {
        asprintf(&responseStr[1], "%d", dst);
    }

    // get <leapSeconds >
    err = at_tok_nextint(&urc, &ls);
    if (err < 0)
    {
        LOGE("ERROR occurs when parsing <leapSeconds> of the SIB16 time info URC");
        responseStr[2] = "";
    }
    else
    {
        asprintf(&responseStr[2], "%d", ls);
    }

    // get <localTimeOffset >
    err = at_tok_nextint(&urc, &lto);
    if (err < 0)
    {
        if (bNitzTzAvailble)
        {
            LOGE("Use <localTimeOffset> from (E)MM INFORMATION");
            asprintf(&responseStr[3], "%s", ril_nw_nitz_tz);
        }
        else
        {
            LOGE("ERROR occurs when parsing <localTimeOffset> of the SIB16 time info URC");
            asprintf(&responseStr[3], "+%02d", 0);
        }
    }
    else
    {
        if (lto >= 0){
            asprintf(&responseStr[3], "+%d", lto);
        } else {
            asprintf(&responseStr[3], "%d", lto);
        }
    }

    // get <delayTicks >
    err = at_tok_nextint(&urc, &dt);
    if (err < 0)
    {
        LOGE("ERROR occurs when parsing <delayTicks> of the SIB16 time info URC");
        return;
    }
    asprintf(&responseStr[4], "%d", dt);

    LOGD("SIB16 time info: UTC %s,daylightSavingTime %s,leapSeconds %s,localTimeOffset %s,delayTicks %s",
            responseStr[0],responseStr[1],responseStr[2],responseStr[3], responseStr[4]);

    abs_time = (raw_utc * 10) - epochDiffInMillis;

    currentUtcTimeMillis = ((raw_utc * 10) - epochDiffInMillis + dt)/1000;
    LOGD("currentUtcTimeMillis: %s", ctime(&currentUtcTimeMillis));

    ts = gmtime(&currentUtcTimeMillis);
    if(NULL == ts) return;

    memset(nitz_string, 0, sizeof(nitz_string));
    memset(sib16_time_string, 0, sizeof(sib16_time_string));
    if (dst != -1) {
        // nitz_string with dst
        sprintf(nitz_string, "%02d/%02d/%02d,%02d:%02d:%02d%s,%s",  //"yy/mm/dd,hh:mm:ss(+/-)tz[,dst]"
                (ts->tm_year)%100,
                ts->tm_mon+1,
                ts->tm_mday,
                ts->tm_hour,
                ts->tm_min,
                ts->tm_sec,
                responseStr[3],
                responseStr[1]);

        // sib16_time_string with dst
        // "yy/mm/dd,hh:mm:ss(+/-)tz,dst,abs_time"
        sprintf(sib16_time_string, "%02d/%02d/%02d,%02d:%02d:%02d%s,%s,%lli",
                (ts->tm_year)%100,
                ts->tm_mon+1,
                ts->tm_mday,
                ts->tm_hour,
                ts->tm_min,
                ts->tm_sec,
                responseStr[3],
                responseStr[1],
                abs_time);
    } else {
        //nitz_srting without dst
        sprintf(nitz_string, "%02d/%02d/%02d,%02d:%02d:%02d%s",  //"yy/mm/dd,hh:mm:ss(+/-)tz"
                (ts->tm_year)%100,
                ts->tm_mon+1,
                ts->tm_mday,
                ts->tm_hour,
                ts->tm_min,
                ts->tm_sec,
                responseStr[3]);
        // sib16_time_string with dst=0
        // "yy/mm/dd,hh:mm:ss(+/-)tz,dst,abs_time"
        sprintf(sib16_time_string, "%02d/%02d/%02d,%02d:%02d:%02d%s,0,%lli",  //"yy/mm/dd,hh:mm:ss(+/-)tz"
                (ts->tm_year)%100,
                ts->tm_mon+1,
                ts->tm_mday,
                ts->tm_hour,
                ts->tm_min,
                ts->tm_sec,
                responseStr[3],
                abs_time);
    }
    LOGD("NITZ:%s", nitz_string);
    LOGD("SIB16_Time:%s", sib16_time_string);
    bSIB16Received = 1;

    RIL_UNSOL_RESPONSE(RIL_UNSOL_NITZ_TIME_RECEIVED,  nitz_string, sizeof(char *), rid);
    //RIL_onUnsolicitedResponseSocket(RIL_UNSOL_SIB16_TIME_RECEIVED,  sib16_time_string, sizeof(char *), rid);

    for (i=0; i<5; i++) {
        if ((responseStr[i] != NULL) && strcmp(responseStr[i], ""))
            free(responseStr[i]);
    }
    LOGD("Finish RIL_onUnsolicitedResponseSocket");
}

/// M: OP01 6M @{
int isOp016m() {
    char optr[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.operator.optr", optr, "");

    return (RatConfig_isC2kSupported() == 1) && ((strcmp("OP01", optr) == 0) ? 1 : 0);
}
/// @}

///M: [Network][C2K] add for 3/4/5M load modem capability switch. @{
// Modem Capability Selection.
static void setModemCapability(const RIL_SOCKET_ID rid) {
    // L/C (type 1)
    if (RatConfig_isLteFddSupported() == 1 && RatConfig_isC2kSupported() == 1
            && RatConfig_isGsmSupported() == 0 && RatConfig_isWcdmaSupported() == 0
            && RatConfig_isTdscdmaSupported() == 0) {
        at_send_command("AT+EGRAT=11", NULL,
                getChannelCtxbyProxy());
        at_send_command("AT+ERAT=11,0", NULL,
                getChannelCtxbyProxy());
    // L/C/W/G (type 2)
    } else if (RatConfig_isLteFddSupported() == 1 && RatConfig_isC2kSupported() == 1
            && RatConfig_isGsmSupported() == 1 && RatConfig_isWcdmaSupported() == 1) {
        if (isSvlteCdmaOnlySetFromEngMode() == 3) {
            at_send_command("AT+ERAT=3,0", NULL, getChannelCtxbyProxy());
        } else if (!isOp016m()){
            at_send_command("AT+EGRAT=14", NULL, getChannelCtxbyProxy());
            at_send_command("AT+ERAT=14,0", NULL, getChannelCtxbyProxy());
        }
    // L/W/G (type 3)
    } else if (RatConfig_isLteFddSupported() == 1 && RatConfig_isC2kSupported() == 0
            && RatConfig_isGsmSupported() == 1 && RatConfig_isWcdmaSupported() == 1) {
        at_send_command("AT+EGRAT=6", NULL,
                getChannelCtxbyProxy());
    // L (type 4)
    } else if (RatConfig_isLteFddSupported() == 1 && RatConfig_isC2kSupported() == 0
            && RatConfig_isGsmSupported() == 0 && RatConfig_isWcdmaSupported() == 0
            && RatConfig_isTdscdmaSupported() == 0) {
        at_send_command("AT+EGRAT=3", NULL,
                getChannelCtxbyProxy());
        at_send_command("AT+ERAT=3,0", NULL,
                getChannelCtxbyProxy());
    }
}
/// @}

///M: [Network][C2K] for network rat mode init. @{
void initNetworkMode(const RIL_SOCKET_ID rid, RILChannelCtx *p_channel) {
    RILChannelCtx *pChannel = p_channel;
    if (RatConfig_isC2kSupported()
            && (RatConfig_isLteFddSupported() || RatConfig_isLteTddSupported())) {
        if (getMainProtocolRid() == rid) {
            int activeModem = getActiveModemType();
            if (activeModem != 4 && activeModem != 6) {
                char prop_tdd_only[PROPERTY_VALUE_MAX] = { 0 };
                property_get("ro.vendor.mtk_tdd_data_only_support", prop_tdd_only, "0");
                LOGD("is tdd data only support: %d", atoi(prop_tdd_only));
                if (atoi(prop_tdd_only) == 1) {
                    char prop[PROPERTY_VALUE_MAX] = { 0 };
                    property_get("persist.vendor.operator.optr", prop, "");
                    if (strcmp(prop, "OP09") == 0) {
                        at_send_command("AT+CSRA=0,0,0,0,0,1,0", NULL, pChannel);
                    } else {
                        at_send_command("AT+CSRA=0,0,0,0,0,1,1", NULL, pChannel);
                    }
                } else {
                    at_send_command("AT+CSRA=0,0,0,0,0,1,1", NULL, pChannel);
                }
            }
            at_send_command("AT+ECSRA=1", NULL, pChannel);
            if (isSvlteCdmaOnlySetFromEngMode() == 1) {
                at_send_command("AT+EIRATMODE=3", NULL, pChannel);
            } else if (isSvlteLcgSupport()) {
                at_send_command("AT+EIRATMODE=2", NULL, pChannel);
            } else {
                at_send_command("AT+EIRATMODE=1", NULL, pChannel);
            }
        }
    }
    if (getMainProtocolRid() == rid) {
        setModemCapability(rid);
    }
}
/// @}

/// M: [Network][C2K] Set the SVLTE RAT mode. @{
static void requestSetSvlteRatMode(void * data, size_t datalen, RIL_Token t) {
    char* cmd = NULL;
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    int err = 0;
    int radioTechMode = ((int *) data)[0];
    int preferNwType = ((int *) data)[1];
    int mode = ((int *) data)[2];
    int preRoaming = ((int *) data)[3];
    int roaming = ((int *) data)[4];
    int is3GDualModeCard =  ((int *) data)[5];
    int volteState =  ((int *) data)[6];
    RIL_NW_UNUSED_PARM(datalen);

    LOGD("requestSetSvlteRatMode(), radioTechMode=%d, preferNwType=%d, mode=%d, "
            "preRoaming =%d, roaming=%d, is3GDualModeCard =%d, volteState =%d.",
            radioTechMode, preferNwType, mode, preRoaming, roaming, is3GDualModeCard, volteState);

    // switch UTK/STK mode.
    if (roaming != preRoaming || roaming == 1) {
        int targetUtkMode = radioTechMode == RADIO_TECH_MODE_SVLTE ? 1: 0;
        requestSwitchStkUtkMode(targetUtkMode, t);
    }

    if (is3GDualModeCard == 1) {
        if (roaming != preRoaming || roaming == 1) {
            grabRadioPowerLock();
            if (err >= 0 && needModeSwitchForCtDualModeCard(getRILIdByChannelCtx(getRILChannelCtxFromToken(t)), roaming)) {
                err = asprintf(&cmd, "AT+CFUN=0");
                LOGD("requestSetSvlteRatMode(), send command %s.", cmd);
                err = at_send_command(cmd, &p_response, p_channel);
                free(cmd);
            }
            if (err >= 0 && p_response != NULL && p_response->success != 0 &&
                    needModeSwitchForCtDualModeCard(getRILIdByChannelCtx(getRILChannelCtxFromToken(t)), roaming)) {
                at_response_free(p_response);
                p_response = NULL;
                resetSimForCt3g(getRILIdByChannelCtx(getRILChannelCtxFromToken(t)));
                setBootUpMode(t);
                resetRadioForRoaming(t);
            }
            releaseRadioPowerLock();
        }
    } else {
        RIL_SOCKET_ID rsId = getRILIdByChannelCtx(p_channel);
        char cardType[PROPERTY_VALUE_MAX] = {0};
        property_get(PROPERTY_RIL_FULL_UICC_TYPE[rsId], cardType, "");
        LOGD("requestSetSvlteRatMode(), %s: %s", PROPERTY_RIL_FULL_UICC_TYPE[rsId], cardType);
        int isCdma4GCard = strstr(cardType, "CSIM") != NULL && strstr(cardType, "USIM") != NULL;
        if (isCdma4GCard == 1 && mode == SVLTE_RAT_MODE_3G && preRoaming != 1 && roaming == 1) {
            resetRadioForRoaming(t);
        }
    }

    if (mode == SVLTE_RAT_MODE_STK_SWITCH ||
        mode == SVLTE_RAT_MODE_IR_SUSPEND) {
        RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
        LOGD("requestSetSvlteRatMode(), RAT mode action:%d ", mode);
        setEctMode(rid, 2);
        if (mode == SVLTE_RAT_MODE_IR_SUSPEND) {
            setRegSuspendFunction(t, false);
        }
    } else if (mode == SVLTE_RAT_MODE_3G) {
        // use APIRAT
        if (err >= 0) {
            // switch LTE radio capability.
            err = switchLteRadioCapability(mode, roaming, t);
            // switch RAT mode.
            err = switchRatMode(preferNwType, mode, roaming, t);
        }
    } else {
        // On MD-IRAT the MD home/roaming switched by GMSS.
        // switch LTE radio capability.
        err = switchDataOnlyMode(mode, t);
        // switch RAT mode.
        err = switchSvlteRatMode(preferNwType, mode, volteState, roaming, t);
    }

    LOGD("requestSetSvlteRatMode(), err=%d.", err);
    if (err < 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else if ((err == RIL_E_OPERATION_NOT_ALLOWED) && isERATRetrySupport()) {
        RIL_onRequestComplete(t, RIL_E_OPERATION_NOT_ALLOWED, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
}
/// @}

///M: [Network][C2K] for rat switch optimization, config rat all by ERAT. @{
static int isEratExtSupport() {
    int isEratExtSupport = 0;
    char eratext[PROPERTY_VALUE_MAX] = { 0 };
    property_get("vendor.ril.nw.erat.ext.support", eratext, "-1");
    if (0 == strcmp(eratext, "1")) {
        isEratExtSupport = 1;
    }
    return isEratExtSupport;
}
/// @}

///M: [Network][C2K] get main capability slot id. @{
static int getMajorSlotId() {
    // get main capability id
    char property_value[PROPERTY_VALUE_MAX];
    memset(property_value, 0, sizeof(property_value));
    property_get("persist.vendor.radio.simswitch", property_value, "1");
    int majorSlotId = atoi(property_value)-1;
    return majorSlotId;
}
/// @}

///M: [Network][C2K] control LTE band mode. @{
static int switchLteRadioCapability(int svlteMode, int roaming, RIL_Token t) {
    int err = -1;
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    LOGD("switchLteRadioCapability(), svlteMode=%d, roaming=%d, rid=%d.", svlteMode, roaming, rid);
    // AT+ECTMODE
    char *cmd = NULL;
    int targetEctMode;

    if (svlteMode == SVLTE_RAT_MODE_4G_DATA_ONLY) {
        targetEctMode = 1;
    } else if (svlteMode == SVLTE_RAT_MODE_3G) {
        // disable 4G
        targetEctMode = 2;
    } else {
        targetEctMode = 0;
    }
    setEctMode(rid, targetEctMode);

    if (svlteMode == SVLTE_RAT_MODE_4G_DATA_ONLY || !isEratExtSupport()) {
        // Only protocol 1 support the ECTMODE command.
        if (rid == getMainProtocolRid()) {
            err = asprintf(&cmd, "AT+ECTMODE=%d", targetEctMode);
            LOGD("switchLteRadioCapability(), send command %s.", cmd);
            err = at_send_command(cmd, &p_response, p_channel);
            free(cmd);
        }
    }

    if (getEctMode(rid) == 2) {
        setRegSuspendFunction(t, false);
    }

    // AT+CSRA
    if (roaming != ROAMING_MODE_JPKR_CDMA) {
        if ((err >= 0 && p_response != NULL && p_response->success != 0)
                || (rid == getMainProtocolRid() && isEratExtSupport())) {
            if (p_response != NULL) {
                at_response_free(p_response);
                p_response = NULL;
            }
            int geran_tdma = 0, utran_fdd = 0, e_utran_fdd = 0, e_utran_tdd = 0;
            if (svlteMode == SVLTE_RAT_MODE_4G || svlteMode == SVLTE_RAT_MODE_LTE_C2K) {
                geran_tdma = (roaming == ROAMING_MODE_HOME) ? 0 : 1;
                utran_fdd = (roaming == ROAMING_MODE_HOME) ? 0 : 1;
                e_utran_fdd = 1;
                e_utran_tdd = (roaming == ROAMING_MODE_HOME) ? 0 : 1;
            } else if (svlteMode == SVLTE_RAT_MODE_4G_DATA_ONLY) {
                geran_tdma = 0;
                utran_fdd = 0;
                e_utran_fdd = (roaming == ROAMING_MODE_HOME) ? 0 : 1;
                e_utran_tdd = 1;
            } else if (svlteMode == SVLTE_RAT_MODE_3G) {
                geran_tdma = (roaming == ROAMING_MODE_HOME) ? 0 : 1;
                utran_fdd = (roaming == ROAMING_MODE_HOME) ? 0 : 1;
                e_utran_fdd = 0;
                e_utran_tdd = 0;
            }
            err = asprintf(&cmd, "AT+CSRA=%d,%d,0,0,0,%d,%d", geran_tdma, utran_fdd, e_utran_fdd, e_utran_tdd);
            LOGD("switchLteRadioCapability(), send command %s.", cmd);
            err = at_send_command(cmd, &p_response, p_channel);
            free(cmd);
            // lock frequency for lab test. @{
            char lockFreqProp[PROPERTY_VALUE_MAX] = { 0 };
            property_get("persist.vendor.sys.lock.freq.lte", lockFreqProp, "0");
            int nLockFrep = atoi(lockFreqProp);
            LOGD("Lock LTE frequency: lockFreqProp = %s, nLockFrep = %d.", lockFreqProp, nLockFrep);
            if (nLockFrep == 1) {
                at_send_command("AT+EPBSE=16,2,64,0", NULL, getChannelCtxbyProxy());
            }
            // lock frequency for lab test. @}
        }
    }
    if (p_response != NULL) {
        at_response_free(p_response);
        p_response = NULL;
    }
    return err;
}
/// @}

///M: [Netwrok][C2K] Switch RAT mode. @{
static int switchRatMode(int preferNwType, int svlteMode, int roaming, RIL_Token t) {
    int err = 0;
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    LOGD("switchRatMode(), svlteMode=%d, roaming=%d.", svlteMode, roaming);

    int svlte_lcg_mode = isSvlteLcgSupport();
    // control the SVLTE 5M/4M. @{
    char lcgModeProp[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.sys.ct.ir.mode", lcgModeProp, "0");
    int nLcgMode = atoi(lcgModeProp);
    if (nLcgMode == 5) {
        svlte_lcg_mode = 0;
    } else if (nLcgMode == 4) {
        svlte_lcg_mode = 1;
    }
    // control the SVLTE 5M/4M. @}

    // AT+ERAT
    char* cmd = NULL;
    int rat;
    int rat1 = -1;
    if (svlteMode == SVLTE_RAT_MODE_3G) {
        rat = (svlte_lcg_mode == 1) ? 0 : 2;
        if (roaming == 1 && preferNwType == PREF_NET_TYPE_GSM_ONLY) {
            rat = 0;
        } else if (isEratExtSupport()) {
            int cdmaSlotId = getActiveSvlteModeSlotId()-1;
            int majorSlotId = getMajorSlotId();

            LOGD("%s: getMajorSlotId=%d, getActiveSvlteModeSlotId=%d.",
                    __FUNCTION__, majorSlotId, cdmaSlotId);

            if (roaming == 0) {
                if (cdmaSlotId == majorSlotId) {
                    rat = 7;
                    rat1 = 0;
                } else {
                    rat = 7;
                    rat1 = 32;
                }
            }

            if (cdmaSlotId == majorSlotId) {
                switch (preferNwType) {
                    case PREF_NET_TYPE_CDMA_EVDO_AUTO:
                        rat = 7;    // C2K 1x/Evdo
                        rat1 = 0;
                        break;
                    case PREF_NET_TYPE_CDMA_ONLY:
                        rat = 7;    // C2K 1x/Evdo
                        rat1 = 32;  // C2K 1x only
                        break;
                    case PREF_NET_TYPE_EVDO_ONLY:
                        rat = 7;    // C2K 1x/Evdo
                        rat1 = 64;  // C2K Evdo only
                        break;
                    default:
                        break;
                }
            }
        }
    } else if (svlteMode == SVLTE_RAT_MODE_4G_DATA_ONLY) {
        rat = 3;
    } else if (svlteMode == SVLTE_RAT_MODE_LTE_C2K) {
        rat = 11;
    } else {
        switch (roaming) {
        case ROAMING_MODE_HOME:
            rat = 3;
            break;
        case ROAMING_MODE_NORMAL_ROAMING:
            rat = (svlte_lcg_mode == 1) ? 4 : 6;
            break;
        case ROAMING_MODE_JPKR_CDMA:
            rat = 0;
            break;
        default:
            break;
        }
    }

    if (isEratExtSupport() && rat1 != -1) {
        err = asprintf(&cmd, "AT+ERAT=%d,%d", rat, rat1);
    } else {
        err = asprintf(&cmd, "AT+ERAT=%d", rat);
    }

    LOGD("switchRatMode(), send command %s.", cmd);
    if (isERATRetrySupport()) {
        err = (int)handleERATResult(cmd, t);
    } else {
        err = at_send_command(cmd, &p_response, p_channel);
    }
    free(cmd);

    if (p_response != NULL) {
        at_response_free(p_response);
        p_response = NULL;
    }
    return err;
}

bool isERATRetrySupport() {
    // only enable in Srlte project or CT card is main SIM in Svlte project.
    bool ret = false;
    if (isSrlteSupport()) {
        ret = true;
    } else if (isSvlteSupport()) {
        int cdmaSlotId = getActiveSvlteModeSlotId();
        char tempstr[PROPERTY_VALUE_MAX];
        memset(tempstr, 0, sizeof(tempstr));
        property_get("persist.vendor.radio.simswitch", tempstr, "1");
        int currMajorSim = atoi(tempstr);
        ret = cdmaSlotId == currMajorSim ? true : false;
    } else {
        ret = false;
    }
    return ret;
}
/// @}

///M: [Network][C2K] switch LTE radio capability. @{
static int switchDataOnlyMode(int svlteMode, RIL_Token t) {
    int err = -1;
    int targetEctMode;
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    if (svlteMode == SVLTE_RAT_MODE_4G_DATA_ONLY) {
        targetEctMode = 1;
    } else if (svlteMode == SVLTE_RAT_MODE_3G) {
        //disable 4G
        targetEctMode = 2;
    } else {
        targetEctMode = 0;
    }
    setEctMode(rid, targetEctMode);
    // AT+ECTMODE
    if (!isEratExtSupport()) {
        char *cmd = NULL;
        err = asprintf(&cmd, "AT+ECTMODE=%d", targetEctMode);
        LOGD("switchDataOnlyMode(), send command %s.", cmd);
        err = at_send_command(cmd, &p_response, p_channel);
        free(cmd);
    }

    if (p_response != NULL) {
        at_response_free(p_response);
        p_response = NULL;
    }
    if (isCdmaLteDcSupport()) {
        if (getEctMode(rid) == 2) {
            setRegSuspendFunction(t, false);
        }
    }
    return err;
}
/// @}

///M: [Network][C2K] Switch RAT mode. @[
static int switchSvlteRatMode(int preferNwType, int svlteMode, int volteState, int roaming, RIL_Token t) {
    int err = 0;
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rild = getRILIdByChannelCtx(p_channel);
    LOGD("switchSvlteRatMode(), svlteMode=%d, projecttype=%d",
         svlteMode, getSvlteProjectType());

    int svlte_lcg_mode = isSvlteLcgSupport();
    RIL_NW_UNUSED_PARM(preferNwType);

    // control the SVLTE 5M/4M. @{
    char lcgModeProp[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.sys.ct.ir.mode", lcgModeProp, "0");
    int nLcgMode = atoi(lcgModeProp);
    if (nLcgMode == 5) {
        svlte_lcg_mode = 0;
    } else if (nLcgMode == 4) {
        svlte_lcg_mode = 1;
    }
    // control the SVLTE 5M/4M. @}

    //add for LWG mode set in engneerMode C2K IR Settings
    char lwgModeProp[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.radio.ct.ir.engmode", lwgModeProp, "0");
    int nLwgModeProp = atoi(lwgModeProp);

    // AT+ERAT
    char* cmd = NULL;
    int rat;
    int rat1 = -1;
    if (svlteMode == SVLTE_RAT_MODE_3G) {
        rat = (svlte_lcg_mode == 1) ? 0 : 2;
    } else if (svlteMode == SVLTE_RAT_MODE_4G_DATA_ONLY
            || svlteMode == SVLTE_RAT_MODE_LTE_ONLY || nLwgModeProp == 3) {
        rat = 3;
    } else if (SVLTE_PROJ_SC_5M == getSvlteProjectType() || nLwgModeProp == 2) {
        rat = 6;
    } else if (svlteMode == SVLTE_RAT_MODE_LTE_C2K) {
        rat = 11;
    } else {
        // M: volte on or volte off but ct is in slave and roaming mode.
        if (volteState == 1 || (rild != getMajorSlotId() && roaming == 1)) {
            rat = 6;    // 2/3/4G AUTO
            rat1 = 4;   // 4G preferred
        } else {
            rat = 14;
        }
    }

    if (rat1 != -1) {
        err = asprintf(&cmd, "AT+ERAT=%d,%d", rat, rat1);
    } else {
        err = asprintf(&cmd, "AT+ERAT=%d", rat);
    }
    LOGD("switchSvlteRatMode(), send command %s.", cmd);

    if (isERATRetrySupport()) {
        err = (int)handleERATResult(cmd, t);
    } else {
        err = at_send_command(cmd, &p_response, p_channel);
    }
    free(cmd);

    if (p_response != NULL) {
        at_response_free(p_response);
        p_response = NULL;
    }
    return err;
}
/// @}

/// M:[Network][C2K] for get switching state. 0 is not switching. 1 is switching @{
int getRatSwitching() {
    char switching[PROPERTY_VALUE_MAX] = { 0 };
    int switchingVal = 0;

    property_get("vendor.ril.rat.switching", switching, "0");
    switchingVal = atoi(switching);
    LOGD("getRatSwitching, switchingVal is %d", switchingVal);
    return switchingVal;
}
/// @}

///M: [Network][C2K] send CSRA to control LTE band mode. @{
static void setLteRadioCapability() {
    int err;
    char* cmd = NULL;
    int* params = &csraCmdParams[0];
    ATResponse *p_response = NULL;
    int i;
    err = asprintf(&cmd, "AT+CSRA=%d,%d,%d,%d,%d,%d,%d",
            params[0], params[1], params[2], params[3], params[4], params[5], params[6]);
    LOGD("setLteRadioCapability(), send command %s.", cmd);
    err = at_send_command(cmd, &p_response, getChannelCtxbyProxy());
    free(cmd);
    at_response_free(p_response);
}
/// @}

///M: [Network][C2K] Handle +CSRA URC @{
static void onLteRadioCapabilityChanged(char* urc, const RIL_SOCKET_ID rid)
{
    int err;
    int* params = &csraCmdParams[0];

    // Parse URC parameters.
    err = at_tok_start(&urc);
    if (err < 0) return;
    err = at_tok_nextint(&urc, &params[0]);
    if (err < 0) return;
    err = at_tok_nextint(&urc, &params[1]);
    if (err < 0) return;
    err = at_tok_nextint(&urc, &params[2]);
    if (err < 0) return;
    err = at_tok_nextint(&urc, &params[3]);
    if (err < 0) return;
    err = at_tok_nextint(&urc, &params[4]);
    if (err < 0) return;
    err = at_tok_nextint(&urc, &params[5]);
    if (err < 0) return;
    err = at_tok_nextint(&urc, &params[6]);
    if (err < 0) return;

    if (rid == getMainProtocolRid()) {
        RIL_requestProxyTimedCallback(setLteRadioCapability, NULL, &TIMEVAL_0,
                getRILChannelId(RIL_NW, getMainProtocolRid()), "setLteRadioCapability");
    }
}
/// @}

///M: [Network][C2K] for TDD data only switch LTE band mode. {
static void setInitFddTddMode(void * data,size_t datalen,RIL_Token t) {
    char* cmd = NULL;
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    int err = 0;
    int bandMode = ((int *) data)[0];
    RIL_NW_UNUSED_PARM(datalen);

    LOGD("setInitFddTddMode(), bandMode =%d.", bandMode);
    err = openBandMode(bandMode, t);
    LOGD("requestSetSvlteRatMode(), set TDD FDD open, err=%d.", err);
    if (err < 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
}

static int openBandMode (int bandMode, RIL_Token t) {
    int err = -1;
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);

    LOGD("openBandMode(), bandMode =%d", bandMode);
    char *cmd = NULL;
     // AT+CSRA
    int geran_tdma = 0, utran_fdd = 0, e_utran_fdd = 0, e_utran_tdd = 0;
    geran_tdma = 0;
    utran_fdd = 0;
    e_utran_fdd = 1;
    if (bandMode == SVLTE_FDD_ONLY_MODE) {
        e_utran_tdd = 0;
    } else if (bandMode == SVLTE_FDD_TDD_MODE) {
        e_utran_tdd = 1;
    }
    err = asprintf(&cmd, "AT+CSRA=%d,%d,0,0,0,%d,%d", geran_tdma, utran_fdd, e_utran_fdd, e_utran_tdd);
    LOGD("switchLteRadioCapability(), send command %s.", cmd);
    err = at_send_command(cmd, &p_response, p_channel);
    free(cmd);
    at_response_free(p_response);
    p_response = NULL;
    return err;
}

void enableLteBand8(RIL_SOCKET_ID rid, RILChannelCtx *p_channel) {
    ATResponse *p_response = NULL;
    int err, skip;
    char *cmd = NULL, *line = NULL;
    char optr[PROPERTY_VALUE_MAX] = {0};
    char seg[PROPERTY_VALUE_MAX] = {0};
    unsigned int lte_band_1_32;

    //only major protocal, c2k support, svlte support, multi-sim and !op09A project need do this
    property_get("persist.vendor.operator.optr", optr, "OM");
    property_get("persist.vendor.operator.seg", seg, "0");
    if (getMainProtocolRid() != rid || !RatConfig_isC2kSupported() || !isSvlteSupport() || !(getSimCount() > 1) ||
        ((strcmp("OP09", optr) == 0 && strcmp("SEGDEFAULT", seg) == 0) ? 1 : 0)) {
        return;
    }

    err = at_send_command_singleline("AT+EPBSE=?", "+EPBSE:", &p_response, p_channel);

    if (err < 0 || p_response->success == 0)
        goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if(err < 0) goto error;

    err = at_tok_nextint(&line, &skip);
    if(err < 0) goto error;

    err = at_tok_nextint(&line, &skip);
    if(err < 0) goto error;

    err = at_tok_nextint(&line, &support_bands[2]);
    if(err < 0) goto error;

    err = at_tok_nextint(&line, &skip);
    if(err < 0) goto error;

    at_response_free(p_response);
    p_response = NULL;

    LOGD("enableLteBand8, querySupportBandMode, lte_band_1_32 : %d", support_bands[2]);

    err = at_send_command_singleline("AT+EPBSE?", "+EPBSE:", &p_response, p_channel);

    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if(err < 0) goto error;

    err = at_tok_nextint(&line, &bands[0]);
    if(err < 0) goto error;

    err = at_tok_nextint(&line, &bands[1]);
    if(err < 0) goto error;

    err = at_tok_nextint(&line, &bands[2]);
    if(err < 0) goto error;

    err = at_tok_nextint(&line, &bands[3]);
    if(err < 0) goto error;

    at_response_free(p_response);
    p_response = NULL;

    LOGD("enableLteBand8, queryCurrentBandMode, gsm_band:%d, umts_band : %d, lte_band_1_32 : %d, lte_band_33_64: %d",
            bands[0], bands[1], bands[2], bands[3]);

    //enable LTE band8
    lte_band_1_32 = bands[2];
    if ((support_bands[2] & 0x00000080)
            && ((lte_band_1_32 & 0x00000080) == 0)) {
        lte_band_1_32 = lte_band_1_32 | 0x00000080;
    } else {
        //not support B8 or enable B8 currently
        LOGD("enableLteBand8, no action");
        return;
    }
    err = asprintf(&cmd, "AT+EPBSE=%d,%d,%d,%d", bands[0], bands[1],
                                    lte_band_1_32, bands[3]);
    err = at_send_command(cmd, &p_response, p_channel);

    if (err < 0 || p_response->success == 0) {
        goto error;
    }
    at_response_free(p_response);
    p_response = NULL;
    return;
error:
    LOGD("enableLteBand8, fail because of error");
    at_response_free(p_response);
    p_response = NULL;
}
/// @}

int onCheckSetUnsolResponseFilter(RIL_UnsolicitedResponseFilter filter, RIL_SOCKET_ID rid) {
    RIL_RadioState radioState = getRadioState(rid);
    if ((radioState == RADIO_STATE_ON) || (radioState == RADIO_STATE_OFF)) {
        LOGD("onNwCheckSetUnsolResponseFilter filter=%x", filter);
        if ((filter & RIL_UR_SIGNAL_STRENGTH) == RIL_UR_SIGNAL_STRENGTH) {
            // enable
            setUnsolResponseFilterSignalStrength(true);
            updateSignalStrength();
        } else {
            // disable
            setUnsolResponseFilterSignalStrength(false);
        }
        if ((filter & RIL_UR_FULL_NETWORK_STATE) == RIL_UR_FULL_NETWORK_STATE) {
            // enable
            setUnsolResponseFilterNetworkState(true);
        } else {
            // disable
            setUnsolResponseFilterNetworkState(false);
        }
        return 1;
    } else {
        LOGD("onCheckSetUnsolResponseFilter pending");
        // calloc memery for the next try every time
        RIL_SOCKET_ID* mParam = (RIL_SOCKET_ID*)calloc(1, sizeof(RIL_SOCKET_ID));
        if (mParam == NULL) {
            LOGE("onCheckSetUnsolResponseFilter calloc fail");
            return 0;
        }
        mParam[0] = rid;
        RIL_requestProxyTimedCallback(onRetrySetUnsolResponseFilter, (void*) mParam, &TIMEVAL_1,
            getRILChannelId(RIL_OEM, rid), "onRetrySetUnsolResponseFilter");
        return 0;
    }
}

int onNwCheckSetUnsolResponseFilter(void *data, size_t datalen, RIL_Token t) {
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    last_filter = ((RIL_UnsolicitedResponseFilter *)data)[0];
    onCheckSetUnsolResponseFilter(last_filter, rid);
    return 0;
}

void onRetrySetUnsolResponseFilter(void* param){
    // param[0] = RIL_SOCKET_ID
    LOGD("onRetrySetUnsolResponseFilter %x", last_filter);
    RIL_SOCKET_ID rid = ((RIL_SOCKET_ID*)param)[0];
    onCheckSetUnsolResponseFilter(last_filter, rid);
    // param is used only for this time
    free(param);
}
