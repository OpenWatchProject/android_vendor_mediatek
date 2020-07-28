
#include <ctype.h>
#include "at_tok.h"
#include "misc.h"
#include "ril_embms.h"
#include "telephony/mtk_ril.h"
#include "netutils/ifc.h"
#include "ril_callbacks.h"
#include "ril_nw.h"

#include <linux/if.h>
#include <stdint.h>
#include <time.h>
#include <log/log.h>
#include <errno.h>
#include <unistd.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RIL-EMBMS"

extern char* getNetworkInterfaceName(int interfaceId);
extern void setNwIntfDown(const char * pzIfName);
extern void ril_data_setflags(int s, struct ifreq *ifr, int set, int clr);

/* Refer to system/core/libnetutils/ifc_utils.c */
extern int ifc_get_ifindex(const char *name, int *if_indexp);

#ifndef UNUSED
#define UNUSED(x) (x)   // Eliminate "warning: unused parameter"
#endif

#define EMBMS_CHANNEL_CTX getRILChannelCtxFromToken(t)
#define VDBG 1

static const struct timeval TIMEVAL_0 = {0, 0};

RIL_EMBMS_LocalOosNotify oos_tmgi = {0, 0, {{0}}};
int oos_tmgi_count = 0;

int embms_sock_fd = 0;
int embms_sock6_fd = 0;

// Prevent report duplicated session list
static RIL_EMBMS_LocalSessionNotify g_active_session;
static RIL_EMBMS_LocalSessionNotify g_available_session;

/**
 * returns 1 if line is eMBMS unsolicited response
 */
static const char *s_eMbmsUnsoliciteds[] = {
    "+EMSRV:",
    "+EMSLU",
    "+EMSAIL",
    "+EMSESS:"
};

/**
 * returns 1 if line is eMBMS unsolicited response
 */
static const char *s_eMbmsCommonUnsoliciteds[] = {
    "+CEREG:",
    "+EHVOLTE:"
};

bool isHvolteDisable() {
    bool ret = false;
    char prop[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.vendor.embms.hvolte.disable", prop, "0");

    if (!strcmp(prop, "1")) {
        LOGD("Skip hVolte event");
        ret = true;
    }

    return ret;
}

// for RJIL old middleware version with spec v1.8
bool isRjilSupport() {
    // On android N all use latest Middleware, for RJIL also support requirement >= 1.9
    return false;
}

bool isAtCmdEnableSupport() {
    bool ret = false;
    char prop[PROPERTY_VALUE_MAX] = {0};

    // for RJIL old middleware version
    if (isRjilSupport()) {
        return false;
    }

    property_get("persist.vendor.radio.embms.atenable", prop, "1");

    if (!strcmp(prop, "1")) {
        return true;
    } else if (!strcmp(prop, "0")) {
        return false;
    }

    return ret;
}

int isEmbmsUnsolicited(const char *line) {
    size_t i;

    for (i = 0 ; i < NUM_ELEMS(s_eMbmsUnsoliciteds) ; i++) {
        if (strStartsWith(line, s_eMbmsUnsoliciteds[i])) {
            return 1;
        }
    }

    return 0;
}

int isEmbmsCommonUnsolicited(const char *line) {
    size_t i;

    for (i = 0 ; i < NUM_ELEMS(s_eMbmsCommonUnsoliciteds) ; i++) {
        if (strStartsWith(line, s_eMbmsCommonUnsoliciteds[i])) {
            return 1;
        }
    }

    return 0;
}

void ril_embms_ioctl_init() {
    if (embms_sock_fd > 0)
        close(embms_sock_fd);

    embms_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (embms_sock_fd < 0)
        LOGD("Couldn't create IP socket: errno=%d", errno);
    else
        LOGD("Allocate embms_sock_fd=%d", embms_sock_fd);

    if (embms_sock6_fd > 0)
        close(embms_sock6_fd);

    embms_sock6_fd = socket(AF_INET6, SOCK_DGRAM, 0);

    if (embms_sock6_fd < 0) {
        embms_sock6_fd = -errno;    /* save errno for later */
        LOGD("Couldn't create IPv6 socket: errno=%d", errno);
    } else {
        LOGD("Allocate embms_sock6_fd=%d", embms_sock6_fd);
    }
}

void configureEmbmsNetworkInterface(int interfaceId, int isUp) {
    struct ifreq ifr;

    LOGD("configureNetworkInterface interface %d to %s", interfaceId, isUp ? "UP" : "DOWN");
    memset(&ifr, 0, sizeof(struct ifreq));
    sprintf(ifr.ifr_name, "%s", getNetworkInterfaceName(interfaceId));

    if (isUp) {
        LOGD("set network interface down before up");
        setNwIntfDown(ifr.ifr_name);
    }

    ril_embms_ioctl_init();

    if (isUp) {
        ril_data_setflags(embms_sock_fd, &ifr, IFF_UP, 0);
        ril_data_setflags(embms_sock6_fd, &ifr, IFF_UP, 0);
    } else {
        setNwIntfDown(ifr.ifr_name);
    }

    close(embms_sock_fd);
    close(embms_sock6_fd);
    embms_sock_fd = 0;
    embms_sock6_fd = 0;
}

int isTmgiEmpty(char * tmgi) {
    return(strlen(tmgi) == 0);
}

// Return list of available services
void requestAtAvailService(char * data, size_t datalen, RIL_Token t) {
    // AT%MBMSCMD?
    char *cmd;
    ATResponse *p_response = NULL;
    ATLine *p_cur = NULL;
    int skip;
    int num_sessions = -1;
    char *line;
    char current_session_tmgis[EMBMS_MAX_NUM_SESSIONINFO][EMBMS_MAX_LEN_TMGI + 1];
    char current_session_ids[EMBMS_MAX_NUM_SESSIONINFO][EMBMS_MAX_SESSIONID_LEN + 1];
    int index = 0;
    char cmdline[1024];
    char* response;
    char* tmpBuffer;
    int err = 0;

    UNUSED(datalen);
    LOGD("[requestAtAvailService]%s", data);
    memset(cmdline, 0, sizeof(cmdline));
    memset(current_session_tmgis, 0, sizeof(current_session_tmgis));
    memset(current_session_ids, 0, sizeof(current_session_ids));

    err = at_send_command_multiline("AT+EMSLU?", "+EMSLU", &p_response, EMBMS_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0 || p_response->p_intermediates == NULL) {
        LOGE("AT+EMSLU? Fail");
        goto error;
    }

    // Skip +EMSLU: <enable>
    p_cur = p_response->p_intermediates;
    p_cur = p_cur->p_next;

    // [+EMSLUI: <num_sessions>,<x>,<tmgix>,[<session_idx>],<statusx>][...repeat num_sessions times]
    for (; p_cur != NULL; p_cur = p_cur->p_next) {
        line = p_cur->line;
        err = at_tok_start(&line);

        if (err < 0) goto error;

        // <num_sessions>
        if (num_sessions == -1) {
            int i;
            err = at_tok_nextint(&line, &num_sessions);

            if (err < 0) goto error;

            LOGD("num_sessions:%d", num_sessions);

            if (num_sessions > EMBMS_MAX_NUM_SESSIONINFO) {
                LOGE("Over session size limitation");
                num_sessions = EMBMS_MAX_NUM_SESSIONINFO;
            }

            index = 0;
        } else {
            err = at_tok_nextint(&line, &skip);

            if (err < 0) goto error;
        }

        // handle from <x>
        if (index < num_sessions && index < EMBMS_MAX_NUM_SESSIONINFO) {
            //  <x>
            err = at_tok_nextint(&line, &skip);

            if (err < 0) goto error;

            // x should be index+1
            if (skip != (index + 1)) {
                LOGE("x(%d)!=index(%d)+1", skip, index);
            }

            // <tmgix>
            err = at_tok_nextstr(&line, &tmpBuffer);

            if (err < 0) goto error;

            if (VDBG) LOGD("tmgi = %s", tmpBuffer);
            strncpy(current_session_tmgis[index], tmpBuffer, EMBMS_MAX_LEN_TMGI);

            // <sessionId>
            err = at_tok_nextstr(&line, &tmpBuffer);
            if (err < 0) goto error;
            if (VDBG) LOGD("sessionId = %s", tmpBuffer);
            strncpy(current_session_ids[index], tmpBuffer, EMBMS_MAX_SESSIONID_LEN);

            // skip status, both active and available are counted as available

            index++;
          }
    }

    at_response_free(p_response);


    // %MBMSCMD: <TMGI>[,<SessionID>]
    // OK/ERROR
    char* tmgi;
    int i;

    for (i = 0; i < num_sessions; i++) {
        tmgi = current_session_tmgis[i];

        if (!isTmgiEmpty(tmgi)) {
            if (strlen(current_session_ids[i]) == 0) {
                asprintf(&tmpBuffer, "%%MBMSCMD: %s\n", tmgi);
            } else {
                char *endptr = NULL;
                int sessionId = (int) strtol(current_session_ids[i], &endptr, 16);
                asprintf(&tmpBuffer, "%%MBMSCMD: %s,%d\n", tmgi, sessionId);
            }
            strncat(cmdline, tmpBuffer, strlen(tmpBuffer));
            free(tmpBuffer);
        }
    }

    asprintf(&response, "%sOK\n", cmdline);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, strlen(response));
    free(response);
    return;
error:
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    asprintf(&response, "ERROR\n");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, strlen(response));
    free(response);
}

// Debug usage
void forceEnableMdEmbms(RIL_Token t) {
    ATResponse *p_response = NULL;
    char *cmd = NULL;

    asprintf(&cmd, "AT+ESBP=5,\"SBP_LTE_MBMS\",1");
    at_send_command(cmd, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd);
    at_response_free(p_response);

    asprintf(&cmd, "AT+ESBP=5,\"SBP_LTE_MBMS_COUNTING\",1");
    at_send_command(cmd, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd);
    at_response_free(p_response);

    asprintf(&cmd, "AT+ESBP=5,\"SBP_LTE_MBMS_SERVICE_CONTINUITY\",1");
    at_send_command(cmd, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd);
    at_response_free(p_response);

    asprintf(&cmd, "AT+ESBP=5,\"SBP_LTE_MBMS_SCELL\",1");
    at_send_command(cmd, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd);
    at_response_free(p_response);
}

// Parse SAIs list
bool parseSailist(ATResponse *p_response, RIL_EMBMS_LocalSaiNotify* p_embms_sailist) {
    char *line;
    ATLine *p_cur = NULL;
    int err = 0, count = 0, value = 0;
    int nfreq_index = 0, emsailcf_done = 0;
    int i;

    LOGD("[parseSailist]");
    p_embms_sailist->nsai_total = 0;

    // Skip +EMSAIL: <enable>.
    p_cur = p_response->p_intermediates;
    p_cur = p_cur->p_next;

    for (; p_cur != NULL; p_cur = p_cur->p_next) {
        line = p_cur->line;
        err = at_tok_start(&line);

        if (err < 0) return false;

        if (!emsailcf_done) {
            int cfreq_index = 0;
            // +EMSAILCF: <num_intra_freq>[,<freq_1>[,<freq_2>[...]],<num_sais>,<sai_1>[,<sai_2>[...]]]
            err = at_tok_nextint(&line, &count);

            if (err < 0) return false;

            // read current freq first
            emsailcf_done = 1;
            if (VDBG) LOGD("num_cf = %d", count);

            for (i = 0; i < count; i++) {
                if (at_tok_nextint(&line, &value) < 0) {
                    return false;
                }
                if (cfreq_index < EMBMS_MAX_NUM_NEIGHBOR_FREQ) {
                    if (VDBG) LOGD("cfx = %d, idx = %d", value, cfreq_index);
                    p_embms_sailist->curFreqData[cfreq_index] = value;
                    cfreq_index++;
                    p_embms_sailist->cf_total = cfreq_index;
                } else {
                    LOGE("Warning! curFreqData is full");
                }
            }

            if (p_embms_sailist->cf_total > 0) {
                err = at_tok_nextint(&line, &count);
            } else {
                // If no current freq which means no sai. No more AT data to read
                count = 0;
            }

            if (err < 0) return false;

            p_embms_sailist->csai_total = count;

            if (VDBG) LOGD("csai_total = %d", p_embms_sailist->csai_total);

            for (i = 0; i < count; i++) {
                if (at_tok_nextint(&line, &value) < 0) {
                    return false;
                }

                p_embms_sailist->curSaiData[i] = value;
                LOGD("s sai[%d] = %d" , i, value);
            }
        } else {
            // +EMSAILNF: <num_nf>,[<x>,<nfx>[,<num_bands_nfx>[,<band1_nfx>[,<band2_nfx>[,<...>]]]]
            //    [,<num_sais_nfx>[,<sai_1_nfx>[,<sai_2_nfx>[,<...>]]]][...]]
            //  repeat several lines.

            err = at_tok_nextint(&line, &count);  // num_nf

            if (err < 0) return false;

            if (VDBG) LOGD("num_nf = %d", count);
            if (count > 0) {
                at_tok_nextint(&line, &count);  // x
                // x should be nfreq_index + 1;
                err = at_tok_nextint(&line, &value);  // nfx
                if (err < 0) return false;
                if (VDBG) LOGD("nfx = %d, idx = %d", value, nfreq_index);
                if (nfreq_index < EMBMS_MAX_NUM_NEIGHBOR_FREQ) {
                    p_embms_sailist->neiFreqData[nfreq_index] = value;
                } else {
                    LOGE("Warning! neiFreqData is full");
                }

                err = at_tok_nextint(&line, &count);  // num_bands_nfx
                if (err < 0) return false;
                if (VDBG) LOGD("num_bands_nfx = %d", count);
                for (i = 0; i < count; i++) {
                    if (at_tok_nextint(&line, &value) < 0) {  // bandy_nfx
                        return false;
                    }
                }

                err = at_tok_nextint(&line, &count);  // num_sais_nfx
                if (err < 0) return false;
                if (VDBG) LOGD("num_sais_nfx = %d", count);

                if (nfreq_index < EMBMS_MAX_NUM_NEIGHBOR_FREQ) {
                    p_embms_sailist->nsai_count_per_group[nfreq_index] = count;
                    // The last one use nfreq_index, increase counter;
                    nfreq_index++;
                    p_embms_sailist->nf_total = nfreq_index;
                } else {
                    LOGE("Warning! nsai_count_per_group is full");
                }


                for (i = 0; i < count; i++) {
                    if (at_tok_nextint(&line, &value) < 0) {
                        return false;
                    }

                    if (p_embms_sailist->nsai_total + i >= EMBMS_MAX_NUM_SAI) {
                        LOGE("Error! neigData[] is full!");
                        continue;
                    }

                    p_embms_sailist->neiSaiData[p_embms_sailist->nsai_total + i] = value;
                    LOGD("n sai[%d] = %d" , p_embms_sailist->nsai_total + i, value);
                }
            } else {
                //Force break for loop to prevent p_cur not end due to unknown reason
                break;
            }
            p_embms_sailist->nsai_total += count;
            if (VDBG) LOGD("nsai_total = %d", p_embms_sailist->nsai_total);
        }
    }

    return true;
}

// Return list of current and neighbor SAIs, for old middleware version only
void requestAtGetSaiList_old(char * data, size_t datalen, RIL_Token t) {
    // AT%MBMSCMD="GET_SAIS_LIST" , for old middleware version only
    ATResponse *p_response = NULL;
    int err = 0, count = 0, value = 0;
    int i;
    ATLine *p_cur = NULL;
    char cmdline[1024];
    char* response;
    char* tmpBuffer;
    RIL_EMBMS_LocalSaiNotify embms_sailit;
    memset(&embms_sailit, 0, sizeof(embms_sailit));

    memset(cmdline, 0, sizeof(cmdline));
    UNUSED(datalen);
    LOGD("[requestAtGetSaiList]%s", data);

    err = at_send_command_multiline("AT+EMSAIL?", "+EMSAIL", &p_response, EMBMS_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0 ||
            p_response->p_intermediates  == NULL) {
        goto error;
    }

    if (!parseSailist(p_response, &embms_sailit)) {
        goto error;
    }
    at_response_free(p_response);

    // cmdline = <SAI_COUNT>,[<csai1>,<csai2>,K],<SAI_COUNT>,[<nsai1>,<nsai2>,K]
    asprintf(&tmpBuffer, "%d", embms_sailit.csai_total);
    strncat(cmdline, tmpBuffer, strlen(tmpBuffer));
    free(tmpBuffer);

    // csai1
    for (i = 0; i < (int) embms_sailit.csai_total; i++) {
        strncat(cmdline, ",", 1);
        asprintf(&tmpBuffer, "%d", embms_sailit.curSaiData[i]);
        strncat(cmdline, tmpBuffer, strlen(tmpBuffer));
        free(tmpBuffer);
    }

    // n-SAI_COUNT
    strncat(cmdline, ",", 1);
    asprintf(&tmpBuffer, "%d", embms_sailit.nsai_total);
    strncat(cmdline, tmpBuffer, strlen(tmpBuffer));
    free(tmpBuffer);

    for (i = 0; i < (int) embms_sailit.nsai_total; i++) {
        strncat(cmdline, ",", 1);
        asprintf(&tmpBuffer, "%d", embms_sailit.neiSaiData[i]);
        strncat(cmdline, tmpBuffer, strlen(tmpBuffer));
        free(tmpBuffer);
    }

    // %MBMSCMD:<SAI_COUNT>,[<csai1>,<csai2>,K],<SAI_COUNT>,[<nsai1>,<nsai2>,K]
    // OK/ERROR
    asprintf(&response, "%%MBMSCMD:%s\nOK\n", cmdline);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, strlen(response));
    free(response);
    return;
error:
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    asprintf(&response, "ERROR\n");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, strlen(response));
    free(response);
}

// Return list of current and neighbor SAIs
void requestAtGetSaiList(char * data, size_t datalen, RIL_Token t) {
    // AT%MBMSCMD="CURRENT_SAI_LIST"
    ATResponse *p_response = NULL;
    int err = 0, count = 0, value = 0;
    int i;
    ATLine *p_cur = NULL;
    char cmdline[1024];
    char* response;
    char* tmpBuffer;
    RIL_EMBMS_LocalSaiNotify embms_sailit;
    memset(&embms_sailit, 0, sizeof(embms_sailit));

    memset(cmdline, 0, sizeof(cmdline));
    UNUSED(datalen);
    LOGD("[requestAtGetSaiList]%s", data);

    err = at_send_command_multiline("AT+EMSAIL?", "+EMSAIL", &p_response, EMBMS_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0 ||
            p_response->p_intermediates  == NULL) {
        goto error;
    }

    if (!parseSailist(p_response, &embms_sailit)) {
        goto error;
    }
    at_response_free(p_response);

    // %MBMSCMD:<SAI_COUNT>,[<csai1>,<csai2>, <csai3>,...], <INTER_FREQUENCY_COUNT>,
    // [<Frequency_1>,<SAI_COUNT_1>,[<nsai11>, <nsai12>, <nsai13>,...]],
    // [<Frequency_n>,<SAI_COUNT_n>,[<nsain1>, <nsain2>, <nsain3>,...]]
    // OK/ERROR

    // SAI_COUNT
    asprintf(&tmpBuffer, "%d", embms_sailit.csai_total);
    strncat(cmdline, tmpBuffer, strlen(tmpBuffer));
    free(tmpBuffer);

    // csai1
    for (i = 0; i < (int) embms_sailit.csai_total; i++) {
        strncat(cmdline, ",", 1);
        asprintf(&tmpBuffer, "%d", embms_sailit.curSaiData[i]);
        strncat(cmdline, tmpBuffer, strlen(tmpBuffer));
        free(tmpBuffer);
    }

    // INTER_FREQUENCY_COUNT
    strncat(cmdline, ",", 1);
    asprintf(&tmpBuffer, "%d", embms_sailit.nf_total);
    strncat(cmdline, tmpBuffer, strlen(tmpBuffer));
    free(tmpBuffer);

    int tail_index = 0, j;
    for (i = 0; i < (int) embms_sailit.nf_total; i++) {
        // <Frequency_1>, <SAI_COUNT_1>
        strncat(cmdline, ",", 1);
        asprintf(&tmpBuffer, "%d,%d", embms_sailit.neiFreqData[i],
            embms_sailit.nsai_count_per_group[i]);
        strncat(cmdline, tmpBuffer, strlen(tmpBuffer));
        free(tmpBuffer);

        // <nsai11>
        for (j = 0; j < (int) embms_sailit.nsai_count_per_group[i]; j++) {
            strncat(cmdline, ",", 1);
            asprintf(&tmpBuffer, "%d", embms_sailit.neiSaiData[tail_index+j]);
            strncat(cmdline, tmpBuffer, strlen(tmpBuffer));
            free(tmpBuffer);
        }
        tail_index += embms_sailit.nsai_count_per_group[i];
    }

    // OK/ERROR
    asprintf(&response, "%%MBMSCMD:%s\nOK\n", cmdline);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, strlen(response));
    free(response);
    return;
error:
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    asprintf(&response, "ERROR\n");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, strlen(response));
    free(response);
}

// Requesting the modem for the BSSI signal level
void requestAtBssiSignalLevel(char * data, size_t datalen, RIL_Token t) {
    // AT%MBMSCMD="BSSI_SIGNAL_LEVEL"
    // reference requestSignalStrength() in ril_nw.c
    ATResponse *p_response = NULL;
    int err;
    int response[32] = {0};
    char *line;
    char cmdResponse[128];
    int rssi = 0;
    const RIL_SOCKET_ID rid = getRILIdByChannelCtx(EMBMS_CHANNEL_CTX);
    UNUSED(datalen);
    LOGD("[requestAtBssiSignalLevel]%s", data);

    err = at_send_command_singleline("AT+ECSQ", "+ECSQ:", &p_response, EMBMS_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0 ||
            p_response->p_intermediates  == NULL) {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = getSingnalStrength(line, response, rid);

    if (err < 0) goto error;

    if (99 == response[0]) {
        LOGD("Ignore rssi 99(unknown)");
        goto error;
    }

    rssi = response[9];
    at_response_free(p_response);

    // %MBMSCMD:<BSSI signal level>
    // OK/ERROR
    sprintf(cmdResponse, "%%MBMSCMD:%d\nOK\n", rssi);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, cmdResponse, strlen(cmdResponse));
    return;

error:
    sprintf(cmdResponse, "ERROR\n");
    if (p_response) {
        at_response_free(p_response);
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, cmdResponse, strlen(cmdResponse));
}

bool parseCellId(char* p_response, int* status, unsigned int* cell_id) {
    char *line;
    int skip;
    int err;
    line = p_response;

    if (cell_id == NULL || status == NULL) {
        LOGE("ptr is NULL!!");
        return false;
    }
    *cell_id = 0;
    err = at_tok_start(&line);

    if (err < 0) return false;

    /* <n> */
    err = at_tok_nextint(&line, &skip);

    if (err < 0) return false;

    /* <stat> */
    err = at_tok_nextint(&line, (int*) status);

    if (err < 0) {  // for LTE
        return false;
    } else if (skip > 10) {
        LOGE("The value in the field <stat> is not valid: %d", skip);
        return false;
    }

    if (at_tok_hasmore(&line)) {
        /* <lac/tac> */
        err = at_tok_nexthexint(&line, (int*)cell_id);

        if (err < 0) return false;

        /* <cid> */
        err = at_tok_nexthexint(&line, (int*)cell_id);
        LOGD("cid: %04X", *cell_id);

        if (err < 0 || (*cell_id > 0x0fffffff && *cell_id != 0xffffffff)) {
            LOGE("The value in the field <cid> is not valid: %d", *cell_id);
            return false;
        }
    } else {
        *cell_id = 0;
    }
    return true;
}

// Requesting for the network side information
void requestAtNetworkInfo(char * data, size_t datalen, RIL_Token t) {
    // AT%MBMSCMD="NETWORK INFORMATION"
    int err = 0;
    char *cmd_query;
    ATResponse *p_response = NULL;
    char *line;
    char *answer_line;
    unsigned int cell_id = (unsigned int) - 1;
    int skip;
    char* plmn = NULL;

    UNUSED(datalen);
    LOGD("[requestAtNetworkInfo]%s", data);

    err = at_send_command_singleline("AT+CEREG?", "+CEREG:", &p_response, EMBMS_CHANNEL_CTX);

    // +CREG: <n>,<stat>[,[<lac>],[<ci>],[<AcT>][,<cause_type>,<reject_cause>]]
    // when <n>=0, 1, 2 or 3 and command successful:

    //  +CGREG: <n>,<stat>[,[<lac>],[<ci>],[<AcT>],[<rac>][,<cause_type>,<reject_cause>]]
    //    when <n>=4 or 5 and command successful:
    //  +CGREG: <n>,<stat>[,[<lac>],[<ci>],[<AcT>],[<rac>][,[<cause_type>],[<reject_cause>]
    //    [,[<Active-Time>],[<Periodic-RAU>],[<GPRS-READY-timer>]]]]

    //    when <n>=0, 1, 2 or 3 and command successful:
    //  +CEREG: <n>,<stat>[,[<tac>],[<ci>],[<AcT>[,<cause_type>,<reject_cause>]]]
    //    when <n>=4 or 5 and command successful:
    //   +CEREG: <n>,<stat>[,[<lac>],[<ci>],[<AcT>],[<rac>][,[<cause_type>],[<reject_cause>]
    //    [,[<Active-Time>],[<Periodic-TAU>]]]]

    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) goto error;

    line = p_response->p_intermediates->line;
    if (!parseCellId(line, &skip, &cell_id)) {
        cell_id = 0;
    }
    at_response_free(p_response);
    p_response = NULL;

    // %MBMSCMD:<cell_id>
    // OK/ERROR

    err = at_send_command_singleline("AT+EOPS?", "+EOPS:", &p_response, EMBMS_CHANNEL_CTX);
    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;

    /* <mode> */
    err = at_tok_nextint(&line, &skip);
    if (err < 0 || skip < 0 || skip > 4) {
        LOGE("The <mode> is an invalid value!!!");
        goto error;
    } else if (skip == 2) {  // deregister
        LOGW("The <mode> is 2 so we ignore the follwoing fields!!!");
    } else if (at_tok_hasmore(&line)) {
        /* <format> */
        err = at_tok_nextint(&line, &skip);
        if (err < 0 || skip != 2) {
            LOGW("The <format> is incorrect: expect 2, receive %d", skip);
            goto error;
        }
        // a "+COPS: 0, n" response is also possible
        if (!at_tok_hasmore(&line)) {
        } else {
            /* <oper> */
            err = at_tok_nextstr(&line, &plmn);
            if (err < 0) goto error;

            LOGD("Get operator code %s", plmn);
        }
    }

    if (cell_id != (unsigned int) - 1 && plmn != NULL) {
        // On android N all use latest Middleware, for RJIL also requirement >= 1.9
        asprintf(&answer_line, "%%MBMSCMD:%s%07X\nOK\n", plmn, cell_id);
    } else {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, answer_line, strlen(answer_line));
    free(answer_line);
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    return;
error:
    asprintf(&answer_line, "ERROR\n");
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, answer_line, strlen(answer_line));
    free(answer_line);
}

// Enable/Disable unsolicited MBMS events
void requestAtEnableUrcEvents(char * data, size_t datalen, RIL_Token t) {
    // AT%MBMSEV=[0,1]
    int err = 0;
    int input_err = 0;
    int enable;
    char *cmd_sru;
    char *cmd_slu;
    char *cmd_sail;
    ATResponse *p_response = NULL;
    char *line;
    RILChannelCtx* pChannel = (RILChannelCtx*) getRILChannelCtxFromToken(t);

    UNUSED(datalen);
    LOGD("[requestAtEnableUrcEvents]%s", data);
    // skip to next '='
    at_tok_equal(&data);

    input_err = at_tok_nextint(&data, &enable);

    if (input_err < 0) goto input_error;

    // cmd_sru
    asprintf(&cmd_sru, "AT+EMSRV=%d", enable);
    err = at_send_command(cmd_sru, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd_sru);

    if (err != 0 || p_response->success == 0) {
        LOGE("AT+EMSRV=%d Fail", enable);
        goto error;
    }

    at_response_free(p_response);


    // cmd_slu
    asprintf(&cmd_slu, "AT+EMSLU=%d", enable);
    err = at_send_command(cmd_slu, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd_slu);

    if (err != 0 || p_response->success == 0) {
        LOGE("AT+EMSLU=%d Fail", enable);
        goto error;
    }

    at_response_free(p_response);

    // cmd_sail
    asprintf(&cmd_sail, "AT+EMSAIL=%d", enable);
    err = at_send_command(cmd_sail, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd_sail);

    if (err != 0 || p_response->success == 0) {
        LOGE("AT+EMSAIL=%d Fail", enable);
        goto error;
    }

    at_response_free(p_response);

    // OK/ERROR
    asprintf(&line, "OK\n");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
    free(line);
    return;

input_error:
    LOGE("Input parameter error!!");
error:
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    asprintf(&line, "ERROR\n");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
    free(line);
}

// Display extended verbose result codes.
void requestAtDisplayVerbose(char * data, size_t datalen, RIL_Token t) {
    // AT+CMEE=[0,1,2]
    int input_err = 0;
    int level;
    char *cmd_set;
    ATResponse *p_response = NULL;
    int err = 0;
    char *line;
    UNUSED(datalen);
    LOGD("[requestAtDisplayVerbose]%s", data);
    // skip to next '='
    at_tok_equal(&data);

    input_err = at_tok_nextint(&data, &level);

    if (input_err < 0) goto input_error;

    // cmd_sru
    asprintf(&cmd_set, "AT+CMEE=%d", level);
    err = at_send_command(cmd_set, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd_set);

    if (err != 0 || p_response->success == 0) {
        LOGE("AT+CMEE=%d Fail", level);
        goto error;
    }

    at_response_free(p_response);

    // OK / ERROR
    asprintf(&line, "OK\n");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
    free(line);
    return;

input_error:
    LOGE("Input parameter error!!");
error:
    asprintf(&line, "ERROR\n");
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
    free(line);
}

// requesting the modem status
void requestAtModemStatus(char * data, size_t datalen, RIL_Token t) {
    // AT+CFUN?
    ATResponse *p_response = NULL;
    int err = 0;
    char *line;
    UNUSED(datalen);
    LOGD("[requestAtModemStatus]%s", data);
    err = at_send_command_singleline("AT+CFUN?", "+CFUN:", &p_response, EMBMS_CHANNEL_CTX);

    if (err != 0 || p_response->success == 0) {
        LOGE("AT+CFUN? Fail");
        goto error;
    }

    // AT+CFUN?
    // OK/ERROR
    asprintf(&line, "%s\nOK\n", p_response->p_intermediates->line);
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
    free(line);
    return;

error:
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    asprintf(&line, "ERROR\n");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
    free(line);
}

// AT+CEREG=?
// +CEREG: (3) / +CEREG: (0-3) / +CEREG: (1,3)
// OK
void requestAtNetworkRegSupport(char * data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err = 0;
    char *line;
    UNUSED(datalen);
    LOGI("[requestAtNetworkRegSupport]%s", data);
    err = at_send_command_singleline("AT+CEREG=?", "+CEREG:", &p_response, EMBMS_CHANNEL_CTX);

    if (err != 0 || p_response->success == 0) {
        LOGE("AT+CEREG=? Fail");
        goto error;
    }

    // AT+CEREG=?
    // +CEREG: (3) / +CEREG: (0-3) / +CEREG: (1,3)
    // OK
    asprintf(&line, "%s\nOK\n", p_response->p_intermediates->line);
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
    free(line);
    return;

error:
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    asprintf(&line, "ERROR\n");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
    free(line);
}

// AT+CEREG?
// +CEREG: <n>,<stat>[,[<tac>],[<ci>],[<AcT>[,<cause_type>,<reject_cause>]]]
void requestAtNetworkRegStatus(char * data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err = 0;
    char *line;
    UNUSED(datalen);
    LOGI("[requestAtNetworkRegStatus]%s", data);
    err = at_send_command_singleline("AT+CEREG?", "+CEREG:", &p_response, EMBMS_CHANNEL_CTX);

    if (err != 0 || p_response->success == 0) {
        LOGE("AT+CEREG? Fail");
        goto error;
    }

    asprintf(&line, "%s\nOK\n", p_response->p_intermediates->line);
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
    free(line);
    return;

error:
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    asprintf(&line, "ERROR\n");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
    free(line);
}

void onEmbmsSrvStatus(char *urc, RIL_SOCKET_ID rid) {
    int err;
    int status;
    // +EMSRV: <srv>[,<num_area_ids>,<area_id1>[,<area_id2>[K]]]
    // 0: No service, 1:only unicast available, 2:in eMBMS supporting area 3:e911 4:hVolte 5:flight mode
    // 6: Gemini suspend 7: Virtual suspend
    LOGD("[onEmbmsSrvStatus]%s", urc);

    err = at_tok_start(&urc);

    if (err < 0) return;

    err = at_tok_nextint(&urc, &status);

    if (err < 0) return;

    if (VDBG) LOGD("status = %d", status);

    int response[1];  // embms_local_coverage_state
    response[0] = status;
    if (status < 7) {
        RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_EMBMS_COVERAGE_STATE, response, sizeof(response), rid);
    }

    // 0: Unicast OOS
    // 1: Multicast OOS
    // 2: Exit OOS
    if (status == EMBMS_LOCAL_SRV_EMBMS) {
        RIL_EMBMS_LocalOosNotify oos_response;
        memset(&oos_response, 0, sizeof(oos_response));
        oos_response.reason = EMBMS_EXIT_OOS;
        oos_response.tmgi_info_count = 0;
        RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_EMBMS_OOS_NOTIFICATION,
            &oos_response, sizeof(oos_response), rid);
    }
}

void postSessionListUpdate(void* param) {
    int index = 0;
    int available_count = 0;
    int active_count = 0;
    char *line;
    ATLine *p_cur = NULL;
    int current_enabled;
    int num_sessions = -1;  // -1 means not set yet
    int skip;
    int err;
    char* tmp_tmgi;
    char* tmp_session_id;
    int tmp_status;

    RIL_SOCKET_ID rid = ((embmsTimeCallbackParam*)param)->rid;
    char* urc = ((embmsTimeCallbackParam*)param)->urc;
    RILChannelCtx* pDataChannel = getChannelCtxbyProxy();
    LOGD("postSessionListUpdate rid=%d, dataChannelId=%d", rid, pDataChannel->id);

    RIL_EMBMS_LocalSessionNotify active_session;
    RIL_EMBMS_LocalSessionNotify available_session;
    ATResponse *p_response = NULL;
    memset(&active_session, 0, sizeof(active_session));
    memset(&available_session, 0, sizeof(available_session));
    free(urc);
    free(param);
    err = at_send_command_multiline("AT+EMSLU?", "+EMSLU",
                                        &p_response, pDataChannel);
    if (err < 0 || p_response->success == 0 || p_response->p_intermediates == NULL) {
        LOGE("AT+EMSLU? Fail");
        at_response_free(p_response);
        return;
    }

    // +EMSLU: <enable>
    p_cur = p_response->p_intermediates;
    line = p_cur->line;
    err = at_tok_start(&line);
    if (at_tok_nextint(&line, &current_enabled) < 0) {
        goto error;
    }

    // [+EMSLUI: <num_sessions>,<x>,<tmgix>,[<session_idx>],<statusx>][...repeat num_sessions times]
    p_cur = p_cur->p_next;
    for (; p_cur != NULL; p_cur = p_cur->p_next) {
        line = p_cur->line;
        err = at_tok_start(&line);
        if (err < 0) goto error;
        // <num_sessions>
        if ( num_sessions == -1 ) {
            err = at_tok_nextint(&line, &num_sessions);
            if (err < 0) goto error;
            LOGD("num_sessions:%d", num_sessions);
            index = 0;
        } else {
            err = at_tok_nextint(&line, &skip);
            if (err < 0) goto error;
        }

        if ( index < num_sessions ) {
            // <x>
            err = at_tok_nextint(&line, &skip);
            if (err < 0) goto error;
            // x should be index+1
            if (skip != (index+1)) {
                LOGE("x(%d)!=index(%d)+1", skip, index);
            }
            // <tmgix>

            if (index >= EMBMS_MAX_NUM_SESSIONINFO) {
                break;
            }
            err = at_tok_nextstr(&line, &tmp_tmgi);
            if (err < 0) goto error;
            err = at_tok_nextstr(&line, &tmp_session_id);
            if (err < 0) goto error;
            err = at_tok_nextint(&line, &tmp_status);
            if (err < 0) goto error;

            if (tmp_status == 0 || tmp_status == 1) {
                strncpy(available_session.tmgix[available_count], tmp_tmgi, EMBMS_MAX_LEN_TMGI);
                strncpy(available_session.session_idx[available_count], tmp_session_id, EMBMS_LEN_SESSION_IDX);
                available_count++;
                available_session.tmgi_info_count = available_count;
            }
            if (tmp_status == 1) {
                strncpy(active_session.tmgix[active_count], tmp_tmgi, EMBMS_MAX_LEN_TMGI);
                strncpy(active_session.session_idx[active_count], tmp_session_id, EMBMS_LEN_SESSION_IDX);
                active_count++;
                active_session.tmgi_info_count = active_count;
            }
            index++;
        }
    }
    at_response_free(p_response);

    LOGD("active_session_count=%d, available_session_count=%d",
                        active_session.tmgi_info_count, available_session.tmgi_info_count);

    if (memcmp(&g_active_session, &active_session, sizeof(RIL_EMBMS_LocalSessionNotify)) != 0) {
        RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_EMBMS_ACTIVE_SESSION, &active_session,
                                        sizeof(active_session), rid);
        memcpy(&g_active_session, &active_session, sizeof(RIL_EMBMS_LocalSessionNotify));
    }

    if (memcmp(&g_available_session, &available_session, sizeof(RIL_EMBMS_LocalSessionNotify)) != 0) {
        RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_EMBMS_AVAILABLE_SESSION, &available_session,
                                        sizeof(available_session), rid);
        memcpy(&g_available_session, &available_session, sizeof(RIL_EMBMS_LocalSessionNotify));
    }

    return;
error:
    at_response_free(p_response);
    LOGE("[embms]Error during parse AT command");
}

void onEmbmsSessionListUpdate(char *urc, RIL_SOCKET_ID rid) {
    /*
    %MBMSEV:<event>
        List of <event>:
        0 : Service change event
    */
    int num;
    int index;
    int err;
    LOGD("[onEmbmsSessionListUpdate]%s", urc);

    // +EMSLUI: <num_sessions>,<x>,<tmgix>,[<session_idx>],<statusx>
    err = at_tok_start(&urc);
    if (err < 0) return;

    err = at_tok_nextint(&urc, &num);
    if (err < 0) return;
    if ( num == 0 ) {
        index = 0;
    } else {
        err = at_tok_nextint(&urc, &index);
    }
    if (err < 0) return;

    if (num == index) {
        RILChannelId dataChannelId = getRILChannelId(RIL_SUBSYS_ATCI, rid);
        LOGD("trigger postSessionListUpdate rid=%d, dataChannelId=%d", rid, dataChannelId);
        embmsTimeCallbackParam *param = (embmsTimeCallbackParam*)malloc(sizeof(embmsTimeCallbackParam));
        if (param == NULL) {
            LOGE("malloc fail");
            return;
        }
        param->rid = rid;
        asprintf(&(param->urc), "%s", urc);
        RIL_requestProxyTimedCallback(postSessionListUpdate, param, &TIMEVAL_0,
            dataChannelId, "postSessionListUpdate");
    }
    return;
}

void postSaiListUpdate(void* param) {
    int err = 0;
    ATResponse *p_response = NULL;
    RIL_SOCKET_ID rid = ((embmsTimeCallbackParam*)param)->rid;
    char* urc = ((embmsTimeCallbackParam*)param)->urc;
    RILChannelCtx* pDataChannel = getChannelCtxbyProxy();
    LOGD("postSaiListUpdate rid=%d, dataChannelId=%d", rid, pDataChannel->id);

    RIL_EMBMS_LocalSaiNotify embms_sailit;
    memset(&embms_sailit, 0, sizeof(embms_sailit));
    free(urc);
    free(param);

    err = at_send_command_multiline("AT+EMSAIL?", "+EMSAIL", &p_response, getChannelCtxbyProxy());

    if (err < 0 || p_response->success == 0 ||
            p_response->p_intermediates  == NULL) {
        at_response_free(p_response);
        return;
    }

    if (!parseSailist(p_response, &embms_sailit)) {
        at_response_free(p_response);
        return;
    }
    at_response_free(p_response);

    RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_EMBMS_SAI_LIST_NOTIFICATION, &embms_sailit,
                                        sizeof(embms_sailit), rid);
    return;
}

void onEmbmsSaiListUpdate(char *urc, RIL_SOCKET_ID rid) {
    UNUSED(urc);
    UNUSED(rid);
    int num;
    int index = 0;;
    int err = 0;
    LOGD("[onEmbmsSaiListUpdate]%s", urc);

    // wait for num_nf = x in +EMSAILNF: <num_nf>,<x>,<nfx>...,
    err = at_tok_start(&urc);
    if (err < 0) return;

    err = at_tok_nextint(&urc, &num);
    if (err < 0) return;

    if ( num > 0 ) {
        err = at_tok_nextint(&urc, &index);
    } else {
        index = 0;
    }
    if (err < 0) return;

    if (num == index) {
        RILChannelId dataChannelId = getRILChannelId(RIL_SUBSYS_ATCI, rid);
        LOGD("trigger postSaiListUpdate rid=%d, dataChannelId=%d", rid, dataChannelId);
        embmsTimeCallbackParam *param = (embmsTimeCallbackParam*)malloc(sizeof(embmsTimeCallbackParam));
        if (param == NULL) {
            LOGE("malloc fail");
            return;
        }
        param->rid = rid;
        asprintf(&(param->urc), "%s", urc);
        RIL_requestProxyTimedCallback(postSaiListUpdate, param, &TIMEVAL_0,
            dataChannelId, "postSaiListUpdate");
    }
}

void postEpsNetworkUpdate(void* param) {
    RIL_SOCKET_ID rid = ((embmsTimeCallbackParam*)param)->rid;
    char* urc = ((embmsTimeCallbackParam*)param)->urc;
    char* urc_to_free = urc;
    RILChannelCtx* pDataChannel = getChannelCtxbyProxy();
    LOGD("postEpsNetworkUpdate rid=%d, dataChannelId=%d, urc=%s", rid, pDataChannel->id, urc);
    ATResponse *p_response = NULL;
    int err;
    int status;
    int result_status;
    /*
    +CEREG: <stat>[,[<tac>],[<ci>],[<AcT>][,<cause_type>,<reject_cause>]]
    +CEREG:<status>
        List of <status>:
        0 : not registered
        1 : registered, home network
        2 : not registered, trying to attach or searching an operator to register
        3 : registration denied
        4 : [3GPP]unknown (e.g. out of GERAN/UTRAN coverage or Radio Off),
                may use +CFUN to query and to see if the resule is (1:functional but out of coverage) or (4:disabled)
            [Expway]Flight mode
        5 : registered, roaming
    */
    err = at_tok_start(&urc);

    if (err < 0) goto error;

    err = at_tok_nextint(&urc, &status);

    if (err < 0) goto error;

    if (VDBG) LOGD("status = %d", status);

    // Need cell info
    unsigned int cell_id = -1;
    if (at_tok_hasmore(&urc)) {
        // <lac/tac>
        err = at_tok_nexthexint(&urc, (int*)&cell_id);
        if (err < 0) goto error;
        LOGD("tac: %04X", cell_id);

        // <cid>
        err = at_tok_nexthexint(&urc, (int*)&cell_id);
        LOGD("cid: %04X", cell_id);

        if (err < 0 || (cell_id > 0x0fffffff && cell_id != 0xffffffff)) {
            LOGE("The value in the field <cid> is not valid: %d", cell_id);
        }
    } else {
        LOGE("No cell_id available in CEREG!");
    }
    free(urc_to_free);
    urc_to_free = NULL;
    free(param);
    param = NULL;

    char* plmn = NULL;
    char *line;
    int skip;

    err = at_send_command_singleline("AT+EOPS?", "+EOPS:", &p_response, getChannelCtxbyProxy());
    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;

    /* <mode> */
    err = at_tok_nextint(&line, &skip);
    if (err < 0 || skip < 0 || skip > 4) {
        LOGE("The <mode> is an invalid value!!!");
        goto error;
    } else if (skip == 2) {  // deregister
        LOGW("The <mode> is 2 so we ignore the follwoing fields!!!");
    } else if (at_tok_hasmore(&line)) {
        /* <format> */
        err = at_tok_nextint(&line, &skip);
        if (err < 0 || skip != 2) {
            LOGW("The <format> is incorrect: expect 2, receive %d", skip);
            goto error;
        }
        // a "+COPS: 0, n" response is also possible
        if (!at_tok_hasmore(&line)) {
        } else {
            /* <oper> */
            err = at_tok_nextstr(&line, &plmn);
            if (err < 0) goto error;

            LOGD("Get operator code %s", plmn);
        }
    }



    if (cell_id != (unsigned int) - 1) {
        int response[3];
        response[0] = cell_id;
        response[1] = status;
        response[2] = (plmn == NULL)? 0:atoi(plmn);
        RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_EMBMS_CELL_INFO_NOTIFICATION, response,
            sizeof(response), rid);
    } else {
        LOGE("Error due to cell_id = %d", cell_id);
    }

    at_response_free(p_response);
    p_response = NULL;
    return;

error:
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    if (urc_to_free != NULL) {
        free(urc_to_free);
    }
    if (param != NULL) {
        free(param);
    }
}

void onEpsNetworkUpdate(char *urc, RIL_SOCKET_ID rid) {
    // Note: In order not to effect other module, skip +CREG=1 in requestAtEnableUrcEvents now

    int err;
    int status;
    int result_status;
    LOGD("[onEpsNetworkUpdate]%s", urc);

    RILChannelId dataChannelId = getRILChannelId(RIL_SUBSYS_ATCI, rid);
    LOGD("trigger postEpsNetworkUpdate rid=%d, dataChannelId=%d", rid, dataChannelId);
    embmsTimeCallbackParam *param = (embmsTimeCallbackParam*)malloc(sizeof(embmsTimeCallbackParam));
    if (param == NULL) {
        LOGE("malloc fail");
        return;
    }
    param->rid = rid;
    asprintf(&(param->urc), "%s", urc);
    RIL_requestProxyTimedCallback(postEpsNetworkUpdate, param, &TIMEVAL_0,
        dataChannelId, "postEpsNetworkUpdate");
}

void postHvolteUpdate(void* param) {
    RIL_SOCKET_ID rid = ((embmsTimeCallbackParam*)param)->rid;
    char* urc = ((embmsTimeCallbackParam*)param)->urc;
    char* urc_to_free = urc;
    RILChannelCtx* pDataChannel = getChannelCtxbyProxy();
    LOGD("postHvolteUpdate rid=%d, dataChannelId=%d, urc=%s", rid, pDataChannel->id, urc);

    int err;
    int mode;
    int srlte_enable;
    char *cmd;
    ATResponse *p_response = NULL;

    err = at_tok_start(&urc);
    if (err < 0) goto error;

    err = at_tok_nextint(&urc, &mode);
    if (err < 0) goto error;
    if (VDBG) LOGD("mode = %d", mode);

    if (mode == 0) {
        srlte_enable = 1;
    } else {
        srlte_enable = 0;
    }
    free(urc_to_free);
    urc_to_free = NULL;
    free(param);
    param = NULL;


    asprintf(&cmd, "AT+EMEVT=2,%d", srlte_enable);
    err = at_send_command(cmd, &p_response, getChannelCtxbyProxy());
    free(cmd);
    if (err != 0 || p_response->success == 0) {
        LOGW("Failed to set eMBMS coverage status");
        goto error;
    }
    at_response_free(p_response);
    return;

error:
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    if (urc_to_free != NULL) {
        free(urc_to_free);
    }
    if (param != NULL) {
        free(param);
    }
}

void onEmbmsHvolteUpdate(char *urc, RIL_SOCKET_ID rid) {
    // +EHVOLTE: <mode>
    // <mode>: integer.
    // 0 SRLTE mode
    // 1 LTE-only mode (received in SIB-13 MBSFN-AreaInfoList)
    LOGD("[onEmbmsHvolteUpdate]%s", urc);

    RILChannelId dataChannelId = getRILChannelId(RIL_SUBSYS_ATCI, rid);
    LOGD("trigger postHvolteUpdate rid=%d, dataChannelId=%d", rid, dataChannelId);
    embmsTimeCallbackParam *param = (embmsTimeCallbackParam*)malloc(sizeof(embmsTimeCallbackParam));
    if (param == NULL) {
        LOGE("malloc fail");
        return;
    }
    param->rid = rid;
    asprintf(&(param->urc), "%s", urc);
    RIL_requestProxyTimedCallback(postHvolteUpdate, param, &TIMEVAL_0,
        dataChannelId, "postHvolteUpdate");
}

void onEmbmsSessionActiveUpdate(char *urc, RIL_SOCKET_ID rid) {
    int err;
    int status;
    char* session_id;
    char* raw_tmgi;
    int cause = -1;
    int sub_cause;
    int num_sessions;
    int x;
    // +EMSESS: <num_sessions>,<x>,<tmgi>,[<session_id>],<status>[,<cause>[,<sub_cause>]]
    // Cause for session deactivation:
    // 0:    Normal deactivation (requested by
    //          AT+EMSESS=0)
    // 1:    Session activation failed
    // 2:    Moved out of MBMS service area that provides
    //          the session
    // 3:    Session lost due to loss of eMBMS coverage
    // 4:    Session lost due to loss of LTE coverage/LTE
    //          deactivation (e.g. CSFB triggered, or
    //          reception lost)

    // Sub-cause for activating session failed (cause=1):
    // 1:   Unknown error
    // 2:   The requested session is already active
    // 3:   MBMS service is disabled
    // 4:   Missing control info. Some or none of the
    //       MCCHs can be acquired
    // 5:   Missing TMGI. All MCCHs were read and TMGI
    //       of interest cannot be found
    // 6:   Out of MBMS coverage
    // 7:   Out of service
    // 8:   The frequency of requested activated session    conflicts to the frequency of the current   cell
    // 9:   Maximum number of sessions is activated.
    //       Now modem supports 8 concurrent sessions

    LOGD("[onEmbmsSessionActiveUpdate]%s", urc);

    err = at_tok_start(&urc);
    if (err < 0) return;

    // <num_sessions>
    err = at_tok_nextint(&urc, &num_sessions);
    if (err < 0) return;

    // <x>
    err = at_tok_nextint(&urc, &x);
    if (err < 0) return;

    // <tmgi>
    err = at_tok_nextstr(&urc, &raw_tmgi);
    if (err < 0) return;

    // <session_id>, might be empty
    err = at_tok_nextstr(&urc, &session_id);

    // <status>
    err = at_tok_nextint(&urc, &status);
    if (err < 0) return;
    if (VDBG) LOGD("status = %d", status);

    // <cause>
    if (!status) {
        err = at_tok_nextint(&urc, &cause);
        if (err < 0) return;
        if (VDBG) LOGD("cause = %d", cause);
    }
    // sub cause of session activation failed
    if (cause == 1) {
        err = at_tok_nextint(&urc, &sub_cause);
        if (err < 0) return;
        if (VDBG) LOGD("sub_cause = %d", sub_cause);

        if (sub_cause == 2) {
            // Update session status to succeed.
            status = 1;
        }
    }

    // RIL_LOCAL_GSM_UNSOL_EMBMS_START_SESSION_RESPONSE
    RIL_EMBMS_LocalStartSessionResp start_ss_response;
    memset(&start_ss_response, 0, sizeof(start_ss_response));
    start_ss_response.trans_id = -1;
    start_ss_response.response = -1;
    LOGD("status:%d, cause:%d", status, cause);

    if (status == 1) {
        start_ss_response.response = EMBMS_SESSION_SUCCESS;
    } else if (cause == 1) {
        int ss_response_reason;
        switch (sub_cause) {
            case 1:
                ss_response_reason = EMBMS_SESSION_UNKNOWN_ERROR;
                break;
            case 2:
                ss_response_reason = EMBMS_SESSION_ALREADY_ACTIVATED;
                break;
            case 3:
                ss_response_reason = EMBMS_SESSION_NOT_ALLOWED_DISABLED;
                break;
            case 4:
                ss_response_reason = EMBMS_SESSION_MISSING_CONTROL_INFO;
                break;
            case 5:
                ss_response_reason = EMBMS_SESSION_MISSING_TMGI;
                break;
            case 6:
                ss_response_reason = EMBMS_SESSION_OUT_OF_COVERAGE;
                break;
            case 7:
                ss_response_reason = EMBMS_SESSION_OUT_OF_SERVICE;
                break;
            case 8:
                ss_response_reason = EMBMS_SESSION_FREQ_CONFLICT;
                break;
            case 9:
                ss_response_reason = EMBMS_SESSION_MAX_NUM;
                break;
            default:
                ss_response_reason = EMBMS_SESSION_UNKNOWN_ERROR;
                break;
        }
        LOGD("ss_response_reason:%d", ss_response_reason);
        start_ss_response.response = ss_response_reason;
    }

    if (start_ss_response.response != -1) {
        start_ss_response.tmgi_info_valid = 1;
        strncpy(start_ss_response.tmgi, raw_tmgi, EMBMS_MAX_LEN_TMGI);
        RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_EMBMS_START_SESSION_RESPONSE,
            &start_ss_response, sizeof(start_ss_response), rid);
    }

    // RIL_LOCAL_GSM_UNSOL_EMBMS_OOS_NOTIFICATION
    // OOS Response reason:
    // UNICAST_OOS      0: Unicast is out of service
    // MULTICAST_OOS    1: Broadcast is out of service
    // EXIT_OOS         2: Both Unicast and Multicast out of service is cleared
    int response_reason = -1;
    switch (cause) {
        case 4:
            response_reason = EMBMS_UNICAST_OOS;
            break;
        case 3:
        case 2:
            response_reason = EMBMS_MULTICAST_OOS;
            break;
    }

    LOGD("oos response_reason %d", response_reason);

    if ( x < 1 || x > EMBMS_MAX_NUM_SESSIONINFO ) {
        LOGE("invalid x = %d !!", x);
    } else if (x == 1) {
        LOGD("initial data base for x = %d", x);
        memset(&oos_tmgi, 0, sizeof(oos_tmgi));
        oos_tmgi_count = 0;
    }

    if (response_reason != -1) {
        oos_tmgi.reason = response_reason;
        strncpy(oos_tmgi.tmgix[oos_tmgi_count], raw_tmgi, EMBMS_MAX_LEN_TMGI);
        LOGD("Save oos tmgi[%d] as: %s",
            x, oos_tmgi.tmgix[oos_tmgi_count]);

        oos_tmgi_count++;
        oos_tmgi.tmgi_info_count = oos_tmgi_count;
    }

    // Fire OOS URC only when index == num
    if (x == num_sessions && oos_tmgi_count > 0) {
        RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_EMBMS_OOS_NOTIFICATION, &oos_tmgi, sizeof(oos_tmgi), rid);
        memset(&oos_tmgi, 0, sizeof(oos_tmgi));
        oos_tmgi_count = 0;
    }
}

void requestAtDeviceInfo(char * data, size_t datalen, RIL_Token t) {
    int fd;
    struct ifreq ifr;
    unsigned char *mac;
    char line[32];

    UNUSED(data);
    UNUSED(datalen);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd >= 0) {
        ifr.ifr_addr.sa_family = AF_INET;
        sprintf(ifr.ifr_name, "%s", getNetworkInterfaceName(EMBMS_IFACE_ID - 1));
        ioctl(fd, SIOCGIFHWADDR, &ifr);
        close(fd);
        mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
        sprintf(line, "%%MBMSCMD:%.2X:%.2X:%.2X:%.2X:%.2X:%.2X\nOK\n" , mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        LOGD("mac address:%s", line);
        RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
    } else {
        sprintf(line, "ERROR\n");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
    }
}

//  user preference of multicast over unicast
void requestAtSetPreference(char * data, size_t datalen, RIL_Token t) {
    //  AT%MBMSCMD="MBMS_PREFERENCE",[0,1]
    int err = 0;
    int input_err = 0;
    int enable;
    char *cmd;
    char *line;
    ATResponse *p_response = NULL;

    UNUSED(datalen);
    LOGD("[requestAtSetPreference]%s", data);

    //  skip to next ','
    input_err = at_tok_nextstr(&data, &line);
    if (input_err < 0) goto input_error;

    input_err = at_tok_nextint(&data, &enable);
    if (input_err < 0) goto input_error;

    //  priority
    asprintf(&cmd, "AT+EMPRI=%d", enable);
    err = at_send_command(cmd, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd);

    if (err != 0 || p_response->success == 0) {
        LOGE("AT+EMPRI=%d Fail", enable);
        goto error;
    }

    at_response_free(p_response);

    //  OK/ERROR
    asprintf(&line, "OK\n");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
    free(line);

    return;

input_error:
    LOGE("Input parameter error!!");
error:
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    asprintf(&line, "ERROR\n");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
    free(line);
}

// Inform modem the E911 status
void requestEmbmsSetCoverageStatus(char * data, size_t datalen, RIL_Token t) {
    char *cmd;
    ATResponse *p_response = NULL;
    int err = 0;
    int scenario = ((int *)data)[0];
    int status = ((int *)data)[1];

    UNUSED(datalen);

    LOGD("requestEmbmsSetCoverageStatus:<%d,%d>", scenario, status);
    // AT+EMEVT=<scenario>, <status>
    // <scenario> 1: E911,2: hVoLTE
    // <status> 1: The beginning of the scenario, 0: The end of the scenario
    asprintf(&cmd, "AT+EMEVT=%d,%d", scenario, status);
    err = at_send_command(cmd, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd);
    if (err != 0 || p_response->success == 0) {
        LOGW("Failed to set eMBMS coverage status");
        goto error;
    }
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestEmbmsGetCoverageStatus(void * data, size_t datalen, RIL_Token t) {
    int err = 0;
    char *cmd;
    ATResponse *p_response = NULL;
    RIL_EMBMS_GetCoverageResp coverage_response;
    char *line;
    int is_enabled, srv_status;

    UNUSED(datalen);
    LOGD("[requestEmbmsGetCoverageStatus]:");

    // initial
    coverage_response.trans_id = ((int32_t *)data)[0];
    LOGD("r_trans_id:%d", coverage_response.trans_id);
    coverage_response.response = EMBMS_GENERAL_FAIL;  // default fail
    coverage_response.coverage_state_valid = 0;

    UNUSED(datalen);

    err = at_send_command_singleline("AT+EMSRV?", "+EMSRV:", &p_response, EMBMS_CHANNEL_CTX);

    if (err != 0 || p_response->success == 0) {
        LOGE("AT+EMSRV? Fail");
        goto error;
    }

    line = p_response->p_intermediates->line;
    // +EMSRV: <enable>,<srv>,<num_area_ids>,<area_id1>[,area_id2>[...]]
    // OK
    err = at_tok_start(&line);
    if (err < 0) goto error;

    // <enable>
    err = at_tok_nextint(&line, &is_enabled);
    if (err < 0) goto error;

    // <srv>
    // 0: No service, 1:only unicast available,
    // 2: in eMBMS supporting area 3:e911 4:hVolte 5:flight mode
    err = at_tok_nextint(&line, &srv_status);
    if (err < 0) goto error;

    // send OK response
    coverage_response.response = EMBMS_GENERAL_SUCCESS;
    coverage_response.coverage_state_valid = 1;
    // Let ril-proxy do the value transform
    coverage_response.coverage_state = srv_status;
    // 0: No service, 1:only unicast available, 2:in eMBMS supporting area 3:e911 4:hVolte 5:flight mode

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &coverage_response, sizeof(coverage_response));
    at_response_free(p_response);
    return;
error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &coverage_response, sizeof(coverage_response));
}

//  Parse SAIs list
bool parseSIB16Time(ATResponse *p_response, RIL_EMBMS_LocalGetTimeResp* time_response) {
    char *line;
    uint64_t utc_time;
    int err = 0;
    int tmp = 0;

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;

    // <dst>
    time_response->day_light_saving = -1;
    err = at_tok_nextint(&line, (int*)&tmp);
    time_response->day_light_saving = tmp;
    if (err < 0) goto error;
    if (time_response->day_light_saving >= 0) {
        time_response->day_light_saving_valid = 1;
    }

    // <ls>
    err = at_tok_nextint(&line, &tmp);
    time_response->leap_seconds = tmp;
    if (err < 0) goto error;
    time_response->leap_seconds_valid = 1;

    // <lto>
    err = at_tok_nextint(&line, &tmp);
    time_response->local_time_offset = tmp;
    time_response->local_time_offset_valid = 1;
    if (err < 0) goto error;

    // <ut>
    err = at_tok_nextlonglong(&line, (long long*)&utc_time);
    if (err < 0) goto error;

    // Integer value represented as number 10ms units
    // since 00:00:00 on Gregorian calendar date 1 January 1900
    LOGD("utc time before modify:%llu", (unsigned long long)utc_time);
    utc_time = utc_time * 10;  // Convert to milliseconds
    time_response->milli_sec = utc_time;
    LOGD("parseSIB16Time success.");
    return true;
error:
    LOGD("parseSIB16Time error!!");
    return false;
}

//  Parse NITZ list
bool parseNitzTime(ATResponse *p_response, RIL_EMBMS_LocalGetTimeResp* time_response) {
    // +EMTSI: 2[,[<dst>],<lto>,<year>,<month>,<day>,<hour>,<min>,<sec>]
    char *line;
    int err = 0;
    int type = 0;
    int tmp = 0;
    struct tm m_time;
    memset(&m_time, 0, sizeof(m_time));

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;

    // type
    err = at_tok_nextint(&line, &type);
    if (err < 0) goto error;
    if (type != 2) goto error;

    // <dst>
    time_response->day_light_saving = -1;
    err = at_tok_nextint(&line, &tmp);
    time_response->day_light_saving = tmp;
    if (err < 0) goto error;
    if (time_response->day_light_saving >= 0) {
        time_response->day_light_saving_valid = 1;
    }

    // <lto>
    err = at_tok_nextint(&line, &tmp);
    time_response->local_time_offset = tmp;
    if (err < 0) goto error;
    time_response->local_time_offset = time_response->local_time_offset / 4;
    time_response->local_time_offset_valid = 1;

    // <year> 2000-2999
    err = at_tok_nextint(&line, &tmp);
    if (err < 0) goto error;
    // Since 1900
    m_time.tm_year = tmp - 1900;

    // <month> 1-12
    err = at_tok_nextint(&line, &tmp);
    if (err < 0) goto error;
    // 0-11
    m_time.tm_mon = tmp - 1;

    // <day> 1-31
    err = at_tok_nextint(&line, &tmp);
    if (err < 0) goto error;
    // 1-31
    m_time.tm_mday = tmp;

    // <hour> 0-23
    err = at_tok_nextint(&line, &tmp);
    if (err < 0) goto error;
    // 0-23
    m_time.tm_hour = tmp;

    // <min> 0-59
    err = at_tok_nextint(&line, &tmp);
    if (err < 0) goto error;
    // 0-59
    m_time.tm_min = tmp;

    // <sec> 0-59
    err = at_tok_nextint(&line, &tmp);
    if (err < 0) goto error;
    // 0-59
    m_time.tm_sec = tmp;

    // negative if the information is not available
    m_time.tm_isdst = -1;

    // Convert to UTC second since Epoch, then convert to milli second
    time_response->milli_sec = timegm(&m_time) * 1000LL;
    LOGD("Nitz time:%lld", (long long)time_response->milli_sec);
    LOGD("parseNitzTime success.");
    return true;
error:
    LOGD("parseNitzTime error!!");
    return false;
}

void requestLocalEmbmsGetNetworkTime(char * data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err = 0;
    UNUSED(datalen);
    LOGD("[requestLocalEmbmsGetNetworkTime]");
    RIL_EMBMS_LocalGetTimeResp time_response;

    // initial
    memset(&time_response, 0, sizeof(time_response));
    time_response.trans_id = ((int32_t *)data)[0];
    LOGD("trans_id:%d", time_response.trans_id);
    time_response.response = EMBMS_GET_TIME_ERROR;
    time_response.day_light_saving = -1;

    err = at_send_command_singleline("AT+EMTSI", "+EMTSI:", &p_response, EMBMS_CHANNEL_CTX);

    // +EMTSI: <dst>,<ls>,<lto>,<ut>
    if (err < 0 || p_response->success == 0 ||
            p_response->p_intermediates  == NULL) {
        time_response.response = EMBMS_GET_TIME_ERROR;
    } else if (parseSIB16Time(p_response, &time_response)) {
        time_response.response = EMBMS_GET_TIME_SIB16;
    } else {
        time_response.response = EMBMS_GET_TIME_ERROR;
    }
    at_response_free(p_response);
    p_response = NULL;

    // If Get SIB16 Fail then try to get Nitz
    if (time_response.response == EMBMS_GET_TIME_ERROR) {
        memset(&time_response, 0, sizeof(time_response));
        // Try to get NITZ time
        err = at_send_command_singleline("AT+EMTSI=2", "+EMTSI:", &p_response, EMBMS_CHANNEL_CTX);
        if (err < 0 || p_response->success == 0 ||
                p_response->p_intermediates  == NULL) {
            time_response.response = EMBMS_GET_TIME_ERROR;
        } else if (parseNitzTime(p_response, &time_response)) {
            time_response.response = EMBMS_GET_TIME_NITZ;
        } else {
            time_response.response = EMBMS_GET_TIME_ERROR;
        }
        at_response_free(p_response);
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &time_response, sizeof(time_response));
    return;
}

void dummyOkResponse(char * data, size_t datalen, RIL_Token t) {
    char line[32];

    UNUSED(data);
    UNUSED(datalen);

    sprintf(line, "OK\n");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
}


int requestEmbmsAt(void *data, size_t datalen, RIL_Token t) {
    int result = 1;
    LOGD("requestEmbmsAt:%s", (char*)data);

    if (data == NULL) {
        LOGE("Error!! requestEmbmsAt: get null!!");
        return 0;
    // for RJIL old middleware version, and this only occur from v1.8
    } else if (strStartsWith(data, "AT%MBMSCMD=\"CURRENT_SAI_LIST\"")) {
        if (isRjilSupport()) {
            requestAtGetSaiList_old(data, datalen, t);
        } else {
            requestAtGetSaiList(data, datalen, t);
        }
    } else if (strStartsWith(data, "AT%MBMSCMD=\"NETWORK INFORMATION\"")) {
        requestAtNetworkInfo(data, datalen, t);
    } else if (strStartsWith(data, "AT%MBMSCMD=\"BSSI_SIGNAL_LEVEL\"")) {
        requestAtBssiSignalLevel(data, datalen, t);
    } else if (strStartsWith(data, "AT+CMEE=")) {
        dummyOkResponse(data, datalen, t);
    } else if (strStartsWith(data, "AT+CFUN?")) {
        requestAtModemStatus(data, datalen, t);
    } else if (strStartsWith(data, "AT%MBMSEV=")) {
        requestAtEnableUrcEvents(data, datalen, t);
    } else if (strStartsWith(data, "AT%MBMSCMD?")) {
        requestAtAvailService((char*)data, datalen, t);
    } else if (strStartsWith(data, "AT%MBMSCMD=?")) {
        dummyOkResponse(data, datalen, t);
    } else if (strStartsWith(data, "AT%MBMSCMD=\"DEVICE INFO\"")) {
        requestAtDeviceInfo(data, datalen, t);
    } else if (strStartsWith(data, "ATE1")) {
        dummyOkResponse(data, datalen, t);
    } else if (strStartsWith(data, "AT%MBMSCMD=\"MBMS_PREFERENCE\"")) {
        requestAtSetPreference((char*)data, datalen, t);
    } else if (strStartsWith(data, "AT+CEREG=?")) {
        requestAtNetworkRegSupport(data, datalen, t);
    } else if (strStartsWith(data, "AT+CEREG?")) {
        requestAtNetworkRegStatus(data, datalen, t);
    } else if (strStartsWith(data, "AT+CEREG=")) {
        dummyOkResponse(data, datalen, t);
    } else if (strStartsWith(data, "AT") && strlen(data) == 2) {
        dummyOkResponse(data, datalen, t);
    } else {
        LOGE("Unknown command.");
        result = 0;
    }

    return result;
}

// Deactivation of the modem in eMBMS mode
void requestLocalEmbmsDisable(char * data, size_t datalen, RIL_Token t) {
    const int enable = 0;
    int err = 0;
    char *cmd;
    ATResponse *p_response = NULL;
    RIL_EMBMS_LocalDisableResp enable_response;
    int response_length;
    int type;

    UNUSED(datalen);
    // initial
    enable_response.trans_id = ((int32_t *)data)[0];
    enable_response.response = EMBMS_GENERAL_FAIL;  // default fail
    type = ((int32_t *)data)[1];
    LOGD("[requestLocalEmbmsDisable]:%d, r_trans_id:%d", type, enable_response.trans_id);

    memset(&oos_tmgi, 0, sizeof(RIL_EMBMS_LocalOosNotify));
    oos_tmgi_count = 0;

    memset(&g_active_session, 0xff, sizeof(RIL_EMBMS_LocalSessionNotify));
    memset(&g_available_session, 0xff, sizeof(RIL_EMBMS_LocalSessionNotify));

    // setNetworkInterface to ccmni7
    configureEmbmsNetworkInterface(EMBMS_IFACE_ID -1 , enable);

    response_length = sizeof(RIL_EMBMS_LocalDisableResp);

    asprintf(&cmd, "AT+EMBIND=%d,\"M-CCMNI\",%d", enable, EMBMS_IFACE_ID);
    err = at_send_command(cmd, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd);
    if (err != 0 || p_response->success == 0) {
        LOGE("ignore AT+EMBIND=%d Fail to finish other command", enable);
    }
    at_response_free(p_response);

    // enable eMbms Service
    asprintf(&cmd, "AT+EMSEN=%d", enable);
    err = at_send_command(cmd, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd);
    if (err != 0 || p_response->success == 0) {
        LOGE("AT+EMSEN=%d Fail", enable);
        goto error;
    }
    at_response_free(p_response);

    // priority
    asprintf(&cmd, "AT+EMPRI=%d", enable);
    err = at_send_command(cmd, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd);

    if (err != 0 || p_response->success == 0) {
        LOGE("AT+EMPRI=%d Fail", enable);
        goto error;
    }

    at_response_free(p_response);

    // Ignore all URC since service already disabled

    // OK / ERROR
    enable_response.response = EMBMS_GENERAL_SUCCESS;
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &enable_response, response_length);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &enable_response, response_length);
}

// Activation of the modem in eMBMS mode
void requestLocalEmbmsEnable(char * data, size_t datalen, RIL_Token t) {
    const int enable = 1;
    int err = 0;
    char *cmd;
    ATResponse *p_response = NULL;
    RIL_EMBMS_LocalEnableResp enable_response;
    int response_length;
    int type;

    UNUSED(datalen);
    // initial
    enable_response.trans_id = ((int32_t *)data)[0];
    enable_response.response = EMBMS_GENERAL_FAIL;  // default fail
    type = ((int32_t *)data)[1];
    LOGD("[requestLocalEmbmsEnable]:%d, r_trans_id:%d", type, enable_response.trans_id);

    memset(&oos_tmgi, 0, sizeof(RIL_EMBMS_LocalOosNotify));
    oos_tmgi_count = 0;

    memset(&g_active_session, 0xff, sizeof(RIL_EMBMS_LocalSessionNotify));
    memset(&g_available_session, 0xff, sizeof(RIL_EMBMS_LocalSessionNotify));

    // setNetworkInterface to ccmni7
    configureEmbmsNetworkInterface(EMBMS_IFACE_ID - 1, enable);

    response_length = sizeof(RIL_EMBMS_LocalEnableResp);
    enable_response.interface_index_valid = 0;  // interface_index_valid

    // get interface index
    sprintf(enable_response.interface_name, "ccmni%d", EMBMS_IFACE_ID - 1);
    LOGD("response.interface_name:%s", enable_response.interface_name);
    int if_index;
    ifc_init();
    if (ifc_get_ifindex(enable_response.interface_name, &if_index)) {
        LOGE("cannot obtain interface index");
        if_index = -1;
    }
    ifc_close();

    if (if_index != -1) {
        enable_response.interface_index_valid = 1;
        enable_response.interface_index = if_index;
        LOGD("if_index:%d", if_index);
    } else {
        goto error;
    }

    // Prevent duplicated bind, force unbind if already binded
    asprintf(&cmd, "AT+EMBIND=0,\"M-CCMNI\",%d", EMBMS_IFACE_ID);
    err = at_send_command(cmd, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd);
    at_response_free(p_response);

    // Real bind
    asprintf(&cmd, "AT+EMBIND=%d,\"M-CCMNI\",%d", enable, EMBMS_IFACE_ID);
    err = at_send_command(cmd, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd);
    if (err != 0 || p_response->success == 0) {
        LOGE("AT+EMBIND=%d Fail", enable);
        goto error;
    }
    at_response_free(p_response);

    // enable eMbms Service
    asprintf(&cmd, "AT+EMSEN=%d", enable);
    err = at_send_command(cmd, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd);
    if (err != 0 || p_response->success == 0) {
        LOGE("AT+EMSEN=%d Fail", enable);
        goto error;
    }
    at_response_free(p_response);

    // priority
    if(type == EMBMS_COMMAND_RIL || !isAtCmdEnableSupport()) {
        err = at_send_command("AT+EMPRI=1", &p_response, EMBMS_CHANNEL_CTX);

        if (err != 0 || p_response->success == 0) {
            LOGE("AT+EMPRI=%d Fail", enable);
            goto error;
        }
        at_response_free(p_response);
    }

    // Set tmgi serch timer to 8s
    // +EMSESSCFG=<index>,<para_1>
    // index: 1: TMGI search timer
    // <para_1>: TMGI search time (second)
    at_send_command("AT+EMSESSCFG=1,8", &p_response, EMBMS_CHANNEL_CTX);
    at_response_free(p_response);

    // Enable unsolicited indication.
    if (type == EMBMS_COMMAND_RIL) {
        at_send_command("AT+EMSRV=1", &p_response, EMBMS_CHANNEL_CTX);
        at_response_free(p_response);
        // Enable session list update unsolicited event
        at_send_command("AT+EMSLU=1", &p_response, EMBMS_CHANNEL_CTX);
        at_response_free(p_response);
        // Enable SAI list unsolicited event
        at_send_command("AT+EMSAIL=1", &p_response, EMBMS_CHANNEL_CTX);
        at_response_free(p_response);
    }

    unsigned int cell_id = (unsigned int) - 1;
    char *line;
    int skip;
    int status;
    err = at_send_command_singleline("AT+CEREG?", "+CEREG:", &p_response, EMBMS_CHANNEL_CTX);

    // +CREG: <n>,<stat>[,[<lac>],[<ci>],[<AcT>][,<cause_type>,<reject_cause>]]
    // when <n>=0, 1, 2 or 3 and command successful:

    //  +CGREG: <n>,<stat>[,[<lac>],[<ci>],[<AcT>],[<rac>][,<cause_type>,<reject_cause>]]
    // when <n>=4 or 5 and command successful:
    //  +CGREG: <n>,<stat>[,[<lac>],[<ci>],[<AcT>],[<rac>][,[<cause_type>],[<reject_cause>]
    //    [,[<Active-Time>],[<Periodic-RAU>],[<GPRS-READY-timer>]]]]

    // when <n>=0, 1, 2 or 3 and command successful:
    //  +CEREG: <n>,<stat>[,[<tac>],[<ci>],[<AcT>[,<cause_type>,<reject_cause>]]]
    // when <n>=4 or 5 and command successful:
    // +CEREG: <n>,<stat>[,[<lac>],[<ci>],[<AcT>],[<rac>][,[<cause_type>],[<reject_cause>]
    //    [,[<Active-Time>],[<Periodic-TAU>]]]]

    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) goto error;

    line = p_response->p_intermediates->line;
    if (!parseCellId(line, &status, &cell_id)) {
        cell_id = 0;
    }
    at_response_free(p_response);
    p_response = NULL;

    char* plmn = NULL;

    err = at_send_command_singleline("AT+EOPS?", "+EOPS:", &p_response, EMBMS_CHANNEL_CTX);
    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;

    /* <mode> */
    err = at_tok_nextint(&line, &skip);
    if (err < 0 || skip < 0 || skip > 4) {
        LOGE("The <mode> is an invalid value!!!");
        goto error;
    } else if (skip == 2) {  // deregister
        LOGW("The <mode> is 2 so we ignore the follwoing fields!!!");
    } else if (at_tok_hasmore(&line)) {
        /* <format> */
        err = at_tok_nextint(&line, &skip);
        if (err < 0 || skip != 2) {
            LOGW("The <format> is incorrect: expect 2, receive %d", skip);
            goto error;
        }
        // a "+COPS: 0, n" response is also possible
        if (!at_tok_hasmore(&line)) {
        } else {
            /* <oper> */
            err = at_tok_nextstr(&line, &plmn);
            if (err < 0) goto error;

            LOGD("Get operator code %s", plmn);
        }
    }

    int response[3];
    response[0] = cell_id;
    response[1] = status;
    response[2] = (plmn == NULL)? 0:atoi(plmn);
    RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_EMBMS_CELL_INFO_NOTIFICATION, response,
        sizeof(response), getRILIdByChannelCtx( getRILChannelCtxFromToken(t)));

    // OK / ERROR
    enable_response.response = EMBMS_GENERAL_SUCCESS;
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &enable_response, response_length);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &enable_response, response_length);
}

void requestLocalEmbmsStopSession(char * data, size_t datalen, RIL_Token t) {
    char *cmd;
    ATResponse *p_response = NULL;
    int err = 0;
    char cmdline[1024];
    char* tmpBuffer;
    memset(cmdline, 0, sizeof(cmdline));

    UNUSED(datalen);
    LOGD("[requestLocalEmbmsStopSession]");
    RIL_EMBMS_LocalStopSessionReq* request = (RIL_EMBMS_LocalStopSessionReq*)data;
    RIL_EMBMS_LocalStartSessionResp response;

    // initial
    memset(&response, 0, sizeof(response));
    response.trans_id = request->trans_id;
    LOGD("trans_id:%d", response.trans_id);
    response.response = EMBMS_SESSION_UNKNOWN_ERROR;

    // AT+EMSESS=<act>,<tmgi>[,[<session_id>][,[<area_id>][,<num_sais>,[sai_1>[,<sai_2>[K]]]
    // [,<num_freq>,<freq_1>[,<freq_2>[K]]]]]]
    if (request->stop_type == EMBMS_DEACTIVE_ALL_SESSION) {
        int current_enabled = 0;
        int num_sessions = -1;
        int active_num_sessions = 0;
        int skip, index = 0, i = 0;
        char *line;
        char* tmgi;
        char* session_id;
        int status;
        ATLine *p_cur = NULL;
        char current_session_tmgis[EMBMS_MAX_NUM_EMSLUI][EMBMS_MAX_LEN_TMGI + 1];
        memset(current_session_tmgis, 0, sizeof(current_session_tmgis));

        err = at_send_command_multiline("AT+EMSLU?", "+EMSLU", &p_response, EMBMS_CHANNEL_CTX);
        if (err < 0 || p_response->success == 0 || p_response->p_intermediates == NULL) {
            LOGE("AT+EMSLU? Fail");
            goto error;
        }

        //  +EMSLU: <enable>
        p_cur = p_response->p_intermediates;
        line = p_cur->line;
        err = at_tok_start(&line);

        if (at_tok_nextint(&line, &current_enabled) < 0) {
            goto error;
        }

        //  [+EMSLUI: <num_sessions>,<x>,<tmgix>,[<session_idx>],<statusx>][...repeat num_sessions times]
        p_cur = p_cur->p_next;

        for (; p_cur != NULL; p_cur = p_cur->p_next) {
            line = p_cur->line;
            err = at_tok_start(&line);

            if (err < 0) goto error;

            // <num_sessions>
            if (num_sessions == -1) {
                err = at_tok_nextint(&line, &num_sessions);

                if (err < 0) goto error;

                LOGD("num_sessions:%d", num_sessions);

                index = 0;
            } else {
                err = at_tok_nextint(&line, &skip);

                if (err < 0) goto error;
            }

            if (index < num_sessions) {
                //  <x>
                err = at_tok_nextint(&line, &skip);

                if (err < 0) goto error;

                //  x should be index+1
                if (skip != (index + 1)) {
                    LOGE("Error! x(%d)!=index(%d)+1.", skip, index);
                    goto error;
                }

                //  <tmgix>
                err = at_tok_nextstr(&line, &tmpBuffer);
                if (err < 0) goto error;

                //  session_id
                err = at_tok_nextstr(&line, &session_id);
                if (err < 0) goto error;

                //  status
                err = at_tok_nextint(&line, &status);
                if (err < 0) goto error;

                if (VDBG) LOGD("tmpBuffer = %s, status =%d", tmpBuffer, status);
                if (status) {
                    strncpy(current_session_tmgis[active_num_sessions], tmpBuffer, EMBMS_MAX_LEN_TMGI);
                    active_num_sessions++;
                }
                index++;
            }
        }
        at_response_free(p_response);

        for (i = 0; i < active_num_sessions; i++) {
            tmgi = current_session_tmgis[i];

            if (!isTmgiEmpty(tmgi)) {
                asprintf(&cmd, "AT+EMSESS=0,\"%s\"", tmgi);
                err = at_send_command(cmd, &p_response, EMBMS_CHANNEL_CTX);
                free(cmd);

                if (err != 0 || p_response->success == 0) {
                    LOGE("AT+EMSESS=0 Fail");
                    goto error;
                }

                at_response_free(p_response);
            }
        }

        response.response = EMBMS_SESSION_SUCCESS;
        RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
        return;

    } else {  // EMBMS_DEACTIVE_SESSION
        if (request->session_id > 0) {  // !=INVALID_EMBMS_SESSION_ID
            strncat(cmdline, ",", 1);
            asprintf(&tmpBuffer, "\"%02X\"", request->session_id);
            strncat(cmdline, tmpBuffer, strlen(tmpBuffer));
            free(tmpBuffer);
        }
        asprintf(&cmd, "AT+EMSESS=0,\"%s\"%s", request->tmgi, cmdline);
        err = at_send_command(cmd, &p_response, EMBMS_CHANNEL_CTX);
        free(cmd);

        if (err < 0 || p_response->success == 0) {
            goto error;
        }

        at_response_free(p_response);
        response.response = EMBMS_SESSION_SUCCESS;
        response.tmgi_info_valid = 1;
        strncpy(response.tmgi, request->tmgi, EMBMS_MAX_LEN_TMGI);
        RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
        return;
    }

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
}

void requestLocalEmbmsStartSession(char * data, size_t datalen, RIL_Token t) {
    char *cmd;
    ATResponse *p_response = NULL;
    int err = 0;
    char *line;
    char cmdline[1024] = {0};
    char* tmpBuffer;
    uint32_t i;

    UNUSED(datalen);
    LOGD("[requestLocalEmbmsStartSession]");
    RIL_EMBMS_LocalStartSessionReq* request = (RIL_EMBMS_LocalStartSessionReq*)data;
    RIL_EMBMS_LocalStartSessionResp response;

    // initial
    memset(&response, 0, sizeof(response));
    response.trans_id = request->trans_id;
    LOGD("trans_id:%d", response.trans_id);
    response.response = EMBMS_SESSION_UNKNOWN_ERROR;

    // AT+EMSESS=<act>,<tmgi>[,[<session_id>][,[<area_id>][,<num_sais>,[sai_1>[,<sai_2>[K]]]
    //  [,<num_freq>,<freq_1>[,<freq_2>[K]]]]]]

    //  session_id
    if (request->saiList_count > 0 || request->earfcnlist_count > 0 || request->session_id > 0) {
        strncat(cmdline, ",", 1);
    }
    if (request->session_id > 0) {  // !=INVALID_EMBMS_SESSION_ID
        asprintf(&tmpBuffer, "\"%02X\"", request->session_id);
        strncat(cmdline, tmpBuffer, strlen(tmpBuffer));
        free(tmpBuffer);
    }

    if (request->saiList_count > 0 || request->earfcnlist_count > 0) {
        // area_id, not support
        strncat(cmdline, ",", 1);

        // num_sais
        if (request->saiList_count > EMBMS_MAX_NUM_SAI) {
            request->saiList_count = EMBMS_MAX_NUM_SAI;
        }
        strncat(cmdline, ",", 1);
        asprintf(&tmpBuffer, "%d", request->saiList_count);
        strncat(cmdline, tmpBuffer, strlen(tmpBuffer));
        free(tmpBuffer);

        // sai_y
        for (i = 0; i < request->saiList_count; i++) {
            strncat(cmdline, ",", 1);
            asprintf(&tmpBuffer, "%d", request->saiList[i]);
            strncat(cmdline, tmpBuffer, strlen(tmpBuffer));
            free(tmpBuffer);
        }

        // num_freq
        if (request->earfcnlist_count > EMBMS_MAX_NUM_FREQ) {
            request->earfcnlist_count = EMBMS_MAX_NUM_FREQ;
        }
        strncat(cmdline, ",", 1);
        asprintf(&tmpBuffer, "%d", request->earfcnlist_count);
        strncat(cmdline, tmpBuffer, strlen(tmpBuffer));
        free(tmpBuffer);

        for (i = 0; i < request->earfcnlist_count; i++) {
            strncat(cmdline, ",", 1);
            asprintf(&tmpBuffer, "%d", request->earfcnlist[i]);
            strncat(cmdline, tmpBuffer, strlen(tmpBuffer));
            free(tmpBuffer);
        }
    }

    asprintf(&cmd, "AT+EMSESS=1,\"%s\"%s", request->tmgi, cmdline);
    err = at_send_command(cmd, &p_response, EMBMS_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    at_response_free(p_response);
    response.response = EMBMS_SESSION_SUCCESS;
    response.tmgi_info_valid = 1;
    strncpy(response.tmgi, request->tmgi, EMBMS_MAX_LEN_TMGI);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
}

void requestEmbmsTriggerCellInfoNotify(void * data, size_t datalen, RIL_Token t) {
    int err = 0;
    ATResponse *p_response = NULL;
    char *line;
    char *urc_line;
    unsigned int cell_id = (unsigned int) - 1;
    int status, skip;

    UNUSED(datalen);
    LOGD("[requestEmbmsTriggerCellInfoNotify]");

    err = at_send_command_singleline("AT+CEREG?", "+CEREG:", &p_response, EMBMS_CHANNEL_CTX);

    // +CREG: <n>,<stat>[,[<lac>],[<ci>],[<AcT>][,<cause_type>,<reject_cause>]]
    // when <n>=0, 1, 2 or 3 and command successful:

    //  +CGREG: <n>,<stat>[,[<lac>],[<ci>],[<AcT>],[<rac>][,<cause_type>,<reject_cause>]]
    // when <n>=4 or 5 and command successful:
    //  +CGREG: <n>,<stat>[,[<lac>],[<ci>],[<AcT>],[<rac>][,[<cause_type>],[<reject_cause>]
    //    [,[<Active-Time>],[<Periodic-RAU>],[<GPRS-READY-timer>]]]]

    // when <n>=0, 1, 2 or 3 and command successful:
    //  +CEREG: <n>,<stat>[,[<tac>],[<ci>],[<AcT>[,<cause_type>,<reject_cause>]]]
    // when <n>=4 or 5 and command successful:
    // +CEREG: <n>,<stat>[,[<lac>],[<ci>],[<AcT>],[<rac>][,[<cause_type>],[<reject_cause>]
    //    [,[<Active-Time>],[<Periodic-TAU>]]]]

    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) goto error;

    line = p_response->p_intermediates->line;
    if (!parseCellId(line, &status, &cell_id)) {
        cell_id = 0;
    }
    at_response_free(p_response);
    p_response = NULL;

    char* plmn = NULL;

    err = at_send_command_singleline("AT+EOPS?", "+EOPS:", &p_response, EMBMS_CHANNEL_CTX);
    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;

    /* <mode> */
    err = at_tok_nextint(&line, &skip);
    if (err < 0 || skip < 0 || skip > 4) {
        LOGE("The <mode> is an invalid value!!!");
        goto error;
    } else if (skip == 2) {  // deregister
        LOGW("The <mode> is 2 so we ignore the follwoing fields!!!");
    } else if (at_tok_hasmore(&line)) {
        /* <format> */
        err = at_tok_nextint(&line, &skip);
        if (err < 0 || skip != 2) {
            LOGW("The <format> is incorrect: expect 2, receive %d", skip);
            goto error;
        }
        // a "+COPS: 0, n" response is also possible
        if (!at_tok_hasmore(&line)) {
        } else {
            /* <oper> */
            err = at_tok_nextstr(&line, &plmn);
            if (err < 0) goto error;

            LOGD("Get operator code %s", plmn);
        }
    }

    int response[3];
    response[0] = cell_id;
    response[1] = status;
    response[2] = (plmn == NULL)? 0:atoi(plmn);
    RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_EMBMS_CELL_INFO_NOTIFICATION, response,
        sizeof(response), getRILIdByChannelCtx( getRILChannelCtxFromToken(t)));

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    return;
error:
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

int rilEmbmsMain(int request, void *data, size_t datalen, RIL_Token t) {
    int result = 1;

    switch (request) {
    case RIL_LOCAL_REQUEST_EMBMS_AT_CMD:
        result = requestEmbmsAt(data, datalen, t);
        break;
    case RIL_LOCAL_REQUEST_EMBMS_ENABLE:
        requestLocalEmbmsEnable(data, datalen, t);
        break;
    case RIL_LOCAL_REQUEST_EMBMS_DISABLE:
        requestLocalEmbmsDisable(data, datalen, t);
        break;
    case RIL_LOCAL_REQUEST_EMBMS_START_SESSION:
        requestLocalEmbmsStartSession(data, datalen, t);
        break;
    case RIL_LOCAL_REQUEST_EMBMS_STOP_SESSION:
        requestLocalEmbmsStopSession(data, datalen, t);
        break;
    case RIL_LOCAL_REQUEST_EMBMS_GET_TIME:
        requestLocalEmbmsGetNetworkTime(data, datalen, t);
        break;
    case RIL_LOCAL_REQUEST_EMBMS_SET_E911:
        requestEmbmsSetCoverageStatus(data, datalen, t);
        break;
    case RIL_LOCAL_REQUEST_EMBMS_GET_COVERAGE_STATE:
        requestEmbmsGetCoverageStatus(data, datalen, t);
        break;
    case RIL_LOCAL_REQUEST_EMBMS_TRIGGER_CELL_INFO_NOTIFY:
        requestEmbmsTriggerCellInfoNotify(data, datalen, t);
        break;
    default:
        result = 0;
    }

    if (!result) {
        return result;  /* no matched request */
    }

    return result;  /* request found and handled */
}

int rilEmbmsUnsolicited(const char *s, RILChannelCtx* p_channel) {
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    if (strStartsWith(s, "+EMSRV:")) {
        onEmbmsSrvStatus((char *)s, rid);
    } else if (strStartsWith(s, "+EMSLU")) {
        onEmbmsSessionListUpdate((char *)s, rid);
    } else if (strStartsWith(s, "+EMSAILNF")) {
        onEmbmsSaiListUpdate((char *)s, rid);
    } else if (strStartsWith(s, "+EMSESS:")) {
        onEmbmsSessionActiveUpdate((char *)s, rid);
    }

    return 1;
}

int rilEmbmsCommonUnsolicited(const char *src_s, RILChannelCtx* p_channel) {
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    char* s = strndup(src_s, strlen(src_s));
    if (strStartsWith(s, "+CEREG:")) {
        onEpsNetworkUpdate((char *)s, rid);
    } else if (strStartsWith(s, "+EHVOLTE")) {
        if (!isHvolteDisable()) {
            onEmbmsHvolteUpdate((char *)s, rid);
        }
    }
    free(s);
    return 1;
}
