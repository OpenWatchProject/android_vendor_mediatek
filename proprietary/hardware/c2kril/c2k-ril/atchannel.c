/* //device/system/reference-ril/atchannel.c
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include "atchannel.h"
#include "at_tok.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>

#include <utils/Log.h>
#include <cutils/properties.h>

#include "misc.h"
#include <termios.h>
#include <ril_callbacks.h>

#ifdef HAVE_AEE_FEATURE
#include "aee.h"
#endif

#include "hardware/ccci_intf.h"
#include "atchannel_config.h"
#include <libmtkrilutils.h>

#define LOG_TAG "C2K_AT"

static const char *s_atcmd_without_log[] = {
    "+VGMUID",
    "+GSN",
    "^MEID",
    "+CCID",
    "+VICCID",
    "+VCIMI",
    "+CRSM",
    "AT+CPIN=",
    "AT+CPUK=",
    "AT+CLCK=",
    "AT+CPIN2=",
    "AT+CPUK2=",
    "+CMT",
    "+CMGR",
    "+ECARDESNME",
    "AT+CMGW",
    "AT+CMGS",
    "+CPBR",
    "+CPBW",
    "+CGPADDR",
    "+CGPRCO",
    "+UTKNOTIFY:",
    "+UTKCALL:",
    /// M: CC: sensitive info hidden settings -start
    "AT+CDV",
    "AT+VTC",
    "AT+VTS",
    "+CDV",
    "+CCWA",
    "+CLIP",
    "+CLCC",
    "+REDIRNUM",
    "+CFNM",
    "+CEXTD",
    /// M: CC: sensitive info hidden settings -end
    /// M: NW [Start]
    "+VLOCINFO",    // sid,nid,bs_id,bs_lat
    "+CSNID",       // System ID and Network ID
    "+CREG",        // lac, cid
    "+ENWINFO",     // NW info
    /// M: NW [END]
};

extern int s_isUserLoad;

// M: Fix bug: Request/Timedcallback should queue until AT init done.
extern pthread_mutex_t s_at_init_Mutex;
extern pthread_cond_t s_at_init_Cond;
extern int s_at_init_finish;
extern pthread_t mainloop_thread_id;

static pthread_t s_tid_reader;
static int s_fd = -1;    /* fd of the AT channel */
static ATUnsolHandler s_unsolHandler;

/* for input buffering */

static char s_ATBuffer[MAX_AT_RESPONSE+1];
static char *s_ATBufferCur = s_ATBuffer;

// For multi channel support
static RILChannelCtx s_RILChannel[RIL_SUPPORT_CHANNELS];

extern int s_md3_off;
extern int s_md3_vpon;
// M: Revise condattr to fix system time jump cause timer incorrect issue in android N. @{
pthread_condattr_t s_cond_attr;
#ifdef MTK_ECCCI_C2K
extern pthread_cond_t s_md3_init_Cond;
extern pthread_mutex_t s_md3_init_Mutex;
#endif
/// @}


#define DEBUG 0

#if DEBUG
#define DLogd(x...) RLOGD( x )
#define DLogi(x...) RLOGI( x )
#else
#define DLogd(x...) do {} while(0)
#define DLogi(x...) do {} while(0)
#endif

#if AT_DEBUG
void  AT_DUMP(const char*  prefix, const char*  buff, int  len)
{
    UNUSED(prefix);
    if (len < 0)
        len = strlen(buff);
    LOGD("%.*s", len, buff);
}
#endif

/*
 * for current pending command
 * these are protected by s_commandmutex
 */

static pthread_mutex_t s_commandmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_commandcond = PTHREAD_COND_INITIALIZER;

static const char *s_responsePrefix = NULL;
static const char *s_smsPDU = NULL;
static const char *md5 = NULL;
static ATResponse *sp_response = NULL;

static void (*s_onTimeout)(RILChannelId channelId) = NULL;
static void (*s_onReaderClosed)(RILChannelId channelId) = NULL;

static RILAtSendHookCb s_atHookCallback = NULL;

static void onReaderClosed();
static int writeCtrlZ(const char *s, RILChannelCtx *p_channel);
static int writeline(const char *s, RILChannelCtx *p_channel);
static int findStrInResponse(ATResponse * p_response, const char* strSought);
void waitAtInitDoneForProxyThread();
int isModemPoweredOff();

extern void RIL_onRequestAck(RIL_Token t);

/**
 * returns 1 if line is a request acknowledgment from MD
 */
const char *s_reqAck[] = {
        "ACK"
};

int isRequestAcked(const char *line) {
    size_t i;
    if ((RIL_VERSION < 13) || (line == NULL)) {
        return 0;
    }
    for (i = 0; i < NUM_ELEMS(s_reqAck); i++) {
        if (strStartsWith(line, s_reqAck[i])) {
            return 1;
        }
    }
    return 0;
}

// For multi channel support
const char *channelIdToString(RILChannelId id) {
    switch (id) {
    case URC_CHANNEL:
        return "URC_CHANNEL";
    case SIM_CHANNEL:
        return "SIM_CHANNEL";
    case CC_CHANNEL:
        return "CC_CHANNEL";
    case NW_CHANNEL:
        return "NW_CHANNEL";
    case DATA_CHANNEL:
        return "DATA_CHANNEL";
    case SMS_CHANNEL:
        return "SMS_CHANNEL";
    case ATCI_CHANNEL:
        return "ATCI_CHANNEL";
    case DEFAULT_CHANNEL:
        return "DEFAULT_CHANNEL";
    default:
        return "Unknown channel";
    }
}

// For multi channel support
void initRILChannels(void) {
    RILChannelCtx *channel;

    int i;

    // M: Revise condattr to fix system time jump cause timer incorrect issue in android N. @{
    pthread_condattr_init(&s_cond_attr);
    pthread_condattr_setclock(&s_cond_attr, CLOCK_MONOTONIC);
#ifdef MTK_ECCCI_C2K
    pthread_cond_init(&s_md3_init_Cond, &s_cond_attr);
    pthread_mutex_init(&s_md3_init_Mutex, NULL);
#endif
    /// @}
    for (i = 0; i < RIL_SUPPORT_CHANNELS; i++) {
        channel = &s_RILChannel[i];
        memset(channel, 0, sizeof(RILChannelCtx));

        channel->fd = -1; /* fd of the AT channel */
        channel->ATBufferCur = channel->ATBuffer;
        channel->myName = channelIdToString(i);
        channel->id = i;
        // init mutex to exclusive
        pthread_mutex_init(&channel->commandmutex, NULL);
        pthread_cond_init(&channel->commandcond, &s_cond_attr);
    }
}

#define NS_PER_S 1000000000
static void setTimespecRelative(struct timespec *p_ts, long long msec) {
    /// M: Revise gettime to fix system time jump cause timer incorrect issue in android N. @{
    clock_gettime(CLOCK_MONOTONIC, p_ts);
    p_ts->tv_sec += (msec / 1000);
    p_ts->tv_nsec += (msec % 1000) * 1000L * 1000L;
    /// @
}

static void sleepMsec(long long msec)
{
    struct timespec ts;
    int err;

    ts.tv_sec = (msec / 1000);
    ts.tv_nsec = (msec % 1000) * 1000 * 1000;

    do {
        err = nanosleep (&ts, &ts);
    } while (err < 0 && errno == EINTR);
}

/** add an intermediate response to sp_response*/
static void addIntermediate(const char *line, RILChannelCtx *p_channel)
{
    ATResponse *p_response = p_channel->p_response;

    ATLine *p_new;

    p_new = (ATLine  *) malloc(sizeof(ATLine));
    assert(p_new);

    p_new->line = strdup(line);

    /* note: this adds to the head of the list, so the list
       will be in reverse order of lines received. the order is flipped
       again before passing on to the command issuer */
    p_new->p_next = p_response->p_intermediates;
    p_response->p_intermediates = p_new;
}


/**
 * returns 1 if line is a final response indicating error
 * See 27.007 annex B
 * WARNING: NO CARRIER and others are sometimes unsolicited
 */
static const char * s_finalResponsesError[] = {
    "ERROR",
    "+CMS ERROR:",
    "+CME ERROR:",
    "NO CARRIER", /* sometimes! */
    "NO ANSWER",
    "NO DIALTONE",
    "BUSY" };
static int isFinalResponseError(const char *line, RILChannelCtx *p_channel)
{
    size_t i;

    for (i = 0 ; i < NUM_ELEMS(s_finalResponsesError) ; i++) {
        if (strStartsWith(p_channel->p_response->current_cmd, "ATD") == 0
                && strStartsWith(p_channel->p_response->current_cmd, "AT+CDV=") == 0
                && strStartsWith(line, "NO CARRIER"))
            continue;

        if (strStartsWith(line, s_finalResponsesError[i])) {
            return 1;
        }
    }

    return 0;
}

/**
 * returns 1 if line is a final response indicating success
 * See 27.007 annex B
 * WARNING: NO CARRIER and others are sometimes unsolicited
 */
static const char * s_finalResponsesSuccess[] = {
    "OK"
#if !(USE_DATA_CHANNEL)
    , "CONNECT"       /* some stacks start up data on another channel */
#endif
};
static int isFinalResponseSuccess(const char *line, RILChannelCtx *p_channel)
{
    size_t i;

    if (strStartsWith(p_channel->p_response->current_cmd, "AT+CMGS")
            || strStartsWith(p_channel->p_response->current_cmd, "ATD#777")
            || strStartsWith(p_channel->p_response->current_cmd, "AT+CDV=")) {
        return 0;
    /// M: VPON is treat as response of CPON/CPOF/EPOF, not "OK". but if VPON is return
    /// before "OK", it is not real response. if treat it as response of CPON/CPOF/EPOF
    /// in this case, NE will be occured. This modification is for check available of VPON
    /// as response. @{
    } else if (strStartsWith(p_channel->p_response->current_cmd, "AT+CPON")
            || strStartsWith(p_channel->p_response->current_cmd, "AT+CPOF")
            || strStartsWith(p_channel->p_response->current_cmd, "AT+EPOF")) {
        if (strStartsWith(line, "OK")) {
            addIntermediate(line, p_channel);
        }
        return 0;
    }
    /// @}
    for (i = 0 ; i < NUM_ELEMS(s_finalResponsesSuccess) ; i++) {
        if (strStartsWith(line, s_finalResponsesSuccess[i])) {
            return 1;
        }
    }

    return 0;
}

/**
 * returns 1 if line is the first line in (what will be) a two-line
 * SMS unsolicited response
 */
static const char * s_smsUnsoliciteds[] = {
        "+CDS:",
        "+CBM:",
        "+CMGR:"
};

static const char *s_IntermediatePattern[] = {
    "> ",
};

static int isIntermediatePattern(const char *line)
{
    size_t i;
    for (i = 0; i < NUM_ELEMS(s_IntermediatePattern); i++) {
        if (!strcmp(line, s_IntermediatePattern[i])) {
            return 1;
        }
    }
    return 0;
}

static int isSMSUnsolicited(const char *line)
{
    size_t i;

    for (i = 0 ; i < NUM_ELEMS(s_smsUnsoliciteds) ; i++) {
        if (strStartsWith(line, s_smsUnsoliciteds[i])) {
            return 1;
        }
    }

    return 0;
}

/** assumes s_commandmutex is held */
static void handleFinalResponse(const char *line, RILChannelCtx *p_channel)
{
    ATResponse *p_response = p_channel->p_response;
    p_response->finalResponse = strdup(line);

    pthread_cond_signal(&p_channel->commandcond);
}

void handleRequestAck(RILChannelCtx *p_channel) {
    pthread_cond_signal(&p_channel->commandcond);
}

static void handleUnsolicited(const char *line, RILChannelCtx *p_channel)
{
    if (p_channel->unsolHandler != NULL) {
        p_channel->unsolHandler(line, NULL);
    }
}

static void processLine(const char *line, RILChannelCtx *p_channel)
{
    ATResponse *p_response = p_channel->p_response;
    const char *smsPDU = p_channel->smsPDU;

    if (p_response == NULL) {
        /* no command pending */
        handleUnsolicited(line, p_channel);
        return;
    } else if (isRequestAcked(line)) {
        RLOGD("receive ACK (%s)\n", line);
        p_response->ack = 1;
        handleRequestAck(p_channel);
    }
    //add this just for sending sms,because we should block AT+CMGS till ^HCMGSS unsolicited
    else if (strStartsWith(p_response->current_cmd, "AT+CMGS")
            && strStartsWith(line, "^HCMGSS:")) {
        p_response->success = 1;
        handleFinalResponse(line, p_channel);
    } else if (strStartsWith(p_response->current_cmd, "AT+CMGS")
            && strStartsWith(line, "^HCMGSF:")) {
        p_response->success = 1;
        handleFinalResponse(line, p_channel);
    } else if (strStartsWith(p_response->current_cmd, "ATD#777")
            && strStartsWith(line, "CONNECT")) {
        p_response->success = 1;
        handleFinalResponse(line, p_channel);
    } else if (strStartsWith(p_response->current_cmd, "AT+CDV=")
            && strStartsWith(line, "^ORIG")) {
        p_response->success = 1;
        handleFinalResponse(line, p_channel);
    }
    /*two case would cause cp return +CEND:8 when make a MO call, one is there is a MT call currenly, the other is CDV dial string is NULL*/
    else if (strStartsWith(p_response->current_cmd, "AT+CDV=")
            && strStartsWith(line, "+CEND:8") && (7 == strlen(line))) {
        p_response->success = 0;
        handleFinalResponse(line, p_channel);
    } else if (strStartsWith(p_response->current_cmd, "AT+CPON")
            && strStartsWith(line, "+VPON:1")
            && findStrInResponse(p_response, "OK")) {
        p_response->success = 1;
        s_md3_vpon = 1;
        handleFinalResponse(line, p_channel);
    } else if (strStartsWith(p_response->current_cmd, "AT+CPOF")
            && strStartsWith(line, "+VPON:0")
            && findStrInResponse(p_response, "OK")) {
        p_response->success = 1;
        s_md3_vpon = 0;
        handleFinalResponse(line, p_channel);
    }  else if (strStartsWith(p_response->current_cmd, "AT+EPOF")
            && strStartsWith(line, "+VPON:0")
            && findStrInResponse(p_response, "OK")) {
        p_response->success = 1;
        handleFinalResponse(line, p_channel);
    }  else if (isFinalResponseSuccess(line, p_channel)) {
        p_response->success = 1;
        handleFinalResponse(line, p_channel);
    } else if (isFinalResponseError(line, p_channel)) {
        p_response->success = 0;
        handleFinalResponse(line, p_channel);
    } else if (smsPDU != NULL && 0 == strcmp(line, "> ")) {
        // See eg. TS 27.005 4.3
        // Commands like AT+CMGS have a "> " prompt
        writeCtrlZ(smsPDU, p_channel);
        smsPDU = NULL;
    } else if (/*md5 != NULL &&*/0 == strcmp(line, "> ")) {
//c+w
        writeCtrlZ(md5, p_channel);
        md5 = NULL;
    } else switch (p_channel->type) {
        case NO_RESULT:
            handleUnsolicited(line, p_channel);
            break;
        case NUMERIC:
            if (p_response->p_intermediates == NULL
                && isdigit(line[0])
            ) {
                addIntermediate(line, p_channel);
            } else {
                /* either we already have an intermediate response or
                   the line doesn't begin with a digit */
                handleUnsolicited(line, p_channel);
            }
            break;
        case SINGLELINE:
            if (p_response->p_intermediates == NULL
                && strStartsWith (line, p_channel->responsePrefix)
            ) {
                addIntermediate(line, p_channel);
            } else {
                /* we already have an intermediate response */
                handleUnsolicited(line, p_channel);
            }
            break;
        case MULTILINE:
            if (strStartsWith (line, p_channel->responsePrefix)) {
                addIntermediate(line, p_channel);
            } else {
                handleUnsolicited(line, p_channel);
            }
        break;
        /* atci start */
        case RAW:
            if (isIntermediatePattern(line)) {
                p_response->success = 1;
                handleFinalResponse(line, p_channel);
            } else if (strStartsWith(line, "+EI3GPPIRAT:")) {
                handleUnsolicited(line, p_channel);
            } else {
                addIntermediate(line, p_channel);
            }
            break;
        /* atci end */
        default: /* this should never be reached */
            LOGE("Unsupported AT command type %d\n", p_channel->type);
            handleUnsolicited(line, p_channel);
        break;
    }

    //pthread_mutex_unlock(&s_commandmutex);
}


/**
 * Returns a pointer to the end of the next line
 * special-cases the "> " SMS prompt
 *
 * returns NULL if there is no complete line
 */
static char * findNextEOL(char *cur)
{
    if (cur[0] == '>' && cur[1] == ' ' && cur[2] == '\0') {
        /* SMS prompt character...not \r terminated */
        return cur+2;
    }

    // Find next newline
    while (*cur != '\0' && *cur != '\r' && *cur != '\n') cur++;

    return *cur == '\0' ? NULL : cur;
}

/**
 * Reads a line from the AT channel, returns NULL on timeout.
 * Assumes it has exclusive read access to the FD
 *
 * This line is valid only until the next call to readline
 *
 * This function exists because as of writing, android libc does not
 * have buffered stdio.
 */
static const char *readline(RILChannelCtx *p_channel)
{
    ssize_t count;
    int erroCount = 0;
    char *p_read = NULL;
    char *p_eol = NULL;
    char *ret;

    char *ATBufferCur = p_channel->ATBufferCur;

    /* this is a little odd. I use *s_ATBufferCur == 0 to
     * mean "buffer consumed completely". If it points to a character, than
     * the buffer continues until a \0
     */
    if (*p_channel->ATBufferCur == '\0') {
        /* empty buffer */
        p_channel->ATBufferCur = p_channel->ATBuffer;
        *p_channel->ATBufferCur = '\0';
        p_read = p_channel->ATBuffer;
    } else {   /* *s_ATBufferCur != '\0' */
        /* there's data in the buffer from the last read */

        // skip over leading newlines
        while (*p_channel->ATBufferCur == '\r' || *p_channel->ATBufferCur == '\n')
            p_channel->ATBufferCur++;

        p_eol = findNextEOL(p_channel->ATBufferCur);

        if (p_eol == NULL) {
            /* a partial line. move it up and prepare to read more */
            size_t len;

            len = strlen(p_channel->ATBufferCur);

            memmove(p_channel->ATBuffer, p_channel->ATBufferCur, len + 1);
            p_read = p_channel->ATBuffer + len;
            p_channel->ATBufferCur = p_channel->ATBuffer;
        }
        /* Otherwise, (p_eol !- NULL) there is a complete line  */
        /* that will be returned the while () loop below        */
    }

    while (p_eol == NULL) {
        if (0 == MAX_AT_RESPONSE - (p_read - p_channel->ATBuffer)) {
            RLOGE("ERROR: Input line exceeded buffer\n");
            /* ditch buffer and start over again */
            p_channel->ATBufferCur = p_channel->ATBuffer;
            *p_channel->ATBufferCur = '\0';
            p_read = p_channel->ATBuffer;
        }

        int local_errorno = 0;
        do {
            DLogi("AT read start, p_channel->myName=%s\n",  p_channel->myName);
            count = read(p_channel->fd, p_read,
                            MAX_AT_RESPONSE - (p_read - p_channel->ATBuffer));
            local_errorno = errno;
            if (count < 0 && local_errorno == EINTR) {
                LOGE("%s: read error! line is %d", __FUNCTION__, __LINE__);
            }
            DLogi("AT read end: %d:%s\n", count, strerror(local_errorno));
        } while (count < 0 && local_errorno == EINTR);

        if (count > 0) {
            int i = 0;
            // ReaderLoop maybe receive error fromat data from pty server as pty client when TRM
            // occur frequently in Denali, and the error content always be a 16 byte string:
            // 0 0 0 12 4 0 0 0 232 3 0 0 0 0 0 0(ASIC code), the \0 in the content caused current
            // while loop always think have not recevie a whole line from MD, so it continue to
            // read data, then enter dead loop. Add protect code here to let the error data can be
            // handle as an unsupport URC not block rild normal flow.
            int local_err = 0;
            for (i = 0; i < count; i++) {
                if (p_read[i] == 0) {
                    p_read[i] = '\r';
                    local_err = 1;
                }
            }
            if (local_err == 1) {
                LOGE("%s: read error data!", __FUNCTION__);
            }
            p_channel->readCount += count;

            p_read[count] = '\0';

            // skip over leading newlines
            while (*p_channel->ATBufferCur == '\r' || *p_channel->ATBufferCur == '\n')
                p_channel->ATBufferCur++;

            p_eol = findNextEOL(p_channel->ATBufferCur);
            p_read += count;
        } else if (count <= 0) {
            erroCount++;
            /* read error encountered or EOF reached */
            if(count == 0) {
                LOGI("<fd %d> atchannel: EOF reached, errorCount = %d", p_channel->fd,
                        erroCount);
                if (erroCount < 5) {
                    continue;
                }
            } else {
                LOGI("<fd %d> atchannel: read error %s, errorCount = %d", p_channel->fd,
                        strerror(errno), erroCount);
                if (errno == EAGAIN && erroCount < 5) {
                    sleepMsec(10);
                    continue;
                }
            }
            /* The Buffer may moved */
            p_channel->ATBufferCur = ATBufferCur;
            return NULL;
        }
    }

    /* a full line in the buffer. Place a \0 over the \r and return */

    ret = p_channel->ATBufferCur;
    *p_eol = '\0';
    p_channel->ATBufferCur = p_eol + 1; /* this will always be <= p_read,    */
    /* and there will be a \0 at *p_read */

    unsigned int i = 0;
    for (i = 0; i < NUM_ELEMS(s_atcmd_without_log); i++) {
        if (strStartsWith(ret, s_atcmd_without_log[i])) {
            LOGI("<fd %d> AT < %s=*** (%s, tid:%lu)\n", p_channel->fd, s_atcmd_without_log[i],
                    p_channel->myName, p_channel->tid_myProxy);
            break;
        }
    }

    if (i == NUM_ELEMS(s_atcmd_without_log)) {
        int index = 0;
        if ((index = needToHidenLog(ret)) >= 0) {
            LOGI("<fd %d> AT < %s=*** (%s, tid:%lu)\n", p_channel->fd, getHidenLogPreFix(index),
                    p_channel->myName, p_channel->tid_myProxy);
        } else {
            LOGI("<fd %d> AT < %s (%s, tid:%lu)\n", p_channel->fd, ret, p_channel->myName,
                    p_channel->tid_myProxy);
        }
    }

    return ret;
}

static void onReaderClosed(RILChannelId channel_id)
{
    RILChannelCtx *p_channel = &s_RILChannel[channel_id];
    if (s_onReaderClosed != NULL && p_channel->readerClosed == 0) {
        pthread_mutex_lock(&p_channel->commandmutex);

        p_channel->readerClosed = 1;

        pthread_cond_signal(&p_channel->commandcond);

        pthread_mutex_unlock(&p_channel->commandmutex);

        s_onReaderClosed(p_channel->id);
    }
}

static void *readerLoop(void *arg __unused)
{
    RILChannelCtx *p_channel = (RILChannelCtx *)arg;
    const char *readerName = p_channel->myName;

    LOGI("%s is up", readerName);

    for (;;) {
        const char * line;

        line = readline(p_channel);

        if (line == NULL) {
            char tempstr[PROPERTY_VALUE_MAX] = { 0 };
            int temp = getCdmaModemSlot();

            LOGD("slot:%d", temp);
            if (1 == temp) {
                property_get("vendor.ril.ipo.radiooff", tempstr, "");
            } else if (2 == temp) {
                property_get("vendor.ril.ipo.radiooff.2", tempstr, "");
            } else {
                LOGD("can not get right value from getCdmaModemSlot");
            }
            if (!strcmp(tempstr, "1")) {
                LOGD("In IPO Mode, discard EOF");
                continue;
            } else {
                sleep(10);
                break;
            }
        }

        // !!!NOTE!!! Via sms unsolicited has ONLY ONE line
        // !!!NOTE!!! so don't treat it as multiline or it will block at channel
        // !!!NOTE!!! we SHOULD REMOVE ignor CMT mulitline processing about sms unsolicited

        if(isSMSUnsolicited(line)) {
            char *line1;
            const char *line2;

            // The scope of string returned by 'readline()' is valid only
            // till next call to 'readline()' hence making a copy of line
            // before calling readline again.
            line1 = strdup(line);
            line2 = readline(p_channel);

            if (line2 == NULL) {
                free(line1);
                break;
            }

            if (p_channel->unsolHandler != NULL) {
                LOGD("%s: line1:%s,line2:%s", readerName, line1, line2);
                p_channel->unsolHandler (line1, line2);
            }
            free(line1);
        } else {
            pthread_mutex_lock(&p_channel->commandmutex);
            DLogd("%s Enter processLine", readerName);
            processLine(line, p_channel);
            pthread_mutex_unlock(&p_channel->commandmutex);
        }
    }

    LOGE("%s Closed", readerName);
    onReaderClosed(p_channel->id);

    return NULL;
}

/**
 * Sends string s to the radio with a \r appended.
 * Returns AT_ERROR_* on error, 0 on success
 *
 * This function exists because as of writing, android libc does not
 * have buffered stdio.
 */
static int writeline (const char *s, RILChannelCtx *p_channel)
{
    size_t cur = 0;
    size_t len = strlen(s);
    ssize_t written;
    unsigned int i = 0;

    if (p_channel->fd < 0 || p_channel->readerClosed > 0) {
        return AT_ERROR_CHANNEL_CLOSED;
    }

    for (i = 0; i < NUM_ELEMS(s_atcmd_without_log); i++) {
        if (strStartsWith(s, s_atcmd_without_log[i])) {
            LOGI("<fd %d> AT > %s=*** (%s, tid:%lu)\n", p_channel->fd, s_atcmd_without_log[i],
                    p_channel->myName, p_channel->tid_myProxy);
            break;
        }
    }

    if (i == NUM_ELEMS(s_atcmd_without_log)) {
        int index = 0;
        if ((index = needToHidenLog(s)) >= 0) {
            LOGI("<fd %d> AT > %s=*** (%s, tid:%lu)\n", p_channel->fd, getHidenLogPreFix(index), p_channel->myName,
                    p_channel->tid_myProxy);
        } else {
            LOGI("<fd %d> AT > %s (%s, tid:%lu)\n", p_channel->fd, s, p_channel->myName,
                p_channel->tid_myProxy);
        }
    }

    /* the main string */
    while (cur < len) {
        do {
            DLogd("AT write start, p_channel->fd=%d \n", p_channel->fd);
            written = write (p_channel->fd, s + cur, len - cur);
        } while (written < 0 && errno == EINTR);

        if (written < 0) {
            LOGI("writeline error1");
            return AT_ERROR_GENERIC;
        }

        cur += written;
    }

    /* the \r  */

    do {
        written = write (p_channel->fd, "\r" , 1);
    } while ((written < 0 && errno == EINTR) || (written == 0));

    if (written < 0) {
        LOGI("writeline error2");
        return AT_ERROR_GENERIC;
    }
    DLogd("writeline exit");
    return 0;
}
static int writeCtrlZ (const char *s, RILChannelCtx *p_channel)
{
    size_t cur = 0;
    size_t len = strlen(s);
    ssize_t written;

    if (p_channel->fd < 0 || p_channel->readerClosed > 0) {
        return AT_ERROR_CHANNEL_CLOSED;
    }

    LOGI("AT> %s^Z\n", s);

    AT_DUMP( ">* ", s, strlen(s) );

    /* the main string */
    while (cur < len) {
        do {
            written = write (p_channel->fd, s + cur, len - cur);
        } while (written < 0 && errno == EINTR);

        if (written < 0) {
            return AT_ERROR_GENERIC;
        }

        cur += written;
    }

    /* the ^Z  */

    do {
        written = write (p_channel->fd, "\032" , 1);
    } while ((written < 0 && errno == EINTR) || (written == 0));

    if (written < 0) {
        return AT_ERROR_GENERIC;
    }

    return 0;
}

static void clearPendingCommand(RILChannelCtx *p_channel)
{
    if (p_channel->p_response != NULL) {
        at_response_free(p_channel->p_response);
    }

    p_channel->p_response = NULL;
    p_channel->responsePrefix = NULL;
    p_channel->smsPDU = NULL;
}

void at_set_hook(RILAtSendHookCb hookCb) {
    s_atHookCallback = hookCb;
}


/**
 * Starts AT handler on stream "fd'
 * returns 0 on success, -1 on error
 */
int at_open(int fd, ATUnsolHandler h, RILChannelCtx *p_channel)
{
    UNUSED(fd);
    int ret;
    pthread_attr_t attr;

    assert(p_channel->fd == fd);
    p_channel->unsolHandler = h;

    pthread_mutex_lock(&p_channel->commandmutex);
    p_channel->readerClosed = 0;
    pthread_mutex_unlock(&p_channel->commandmutex);

    p_channel->responsePrefix = NULL;
    p_channel->smsPDU = NULL;
    p_channel->p_response = NULL;

    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&p_channel->tid_reader, &attr, readerLoop, (void *)p_channel);

    if (ret < 0) {
        perror ("pthread_create");
        return -1;
    }

    // Only init for at channel
    if (p_channel->id == FIRST_AT_CHANNEL) {
        // Do AT handshake.
        at_handshake(p_channel);
#ifdef ANDROID_KK
        probeForModemMode(sMdmInfo);
#endif /* ANDROID_KK */
    }
    /*  New channel atchannel is tolerant of echo but it must */
    /*  have verbose result codes */
    at_send_command("ATE0Q0V1", NULL, p_channel);

    return 0;
}

/* FIXME is it ok to call this from the reader and the command thread? */
void at_close(RILChannelId channelId)
{
    assert(channelId < RIL_SUPPORT_CHANNELS);
    RILChannelCtx *p_channel = &s_RILChannel[channelId];
    if (p_channel->fd >= 0)
    {
        close(p_channel->fd);
        p_channel->fd = -1;
        pthread_mutex_lock(&p_channel->commandmutex);
        p_channel->readerClosed = 1;

        pthread_cond_signal(&p_channel->commandcond);
        pthread_mutex_unlock(&p_channel->commandmutex);
        /* the reader thread should eventually die */
    }
}

static ATResponse * at_response_new()
{
    ATResponse *response = (ATResponse *) calloc(1, sizeof(ATResponse));
    assert(response);
    return response;
}

void at_response_free(ATResponse *p_response)
{
    ATLine *p_line;

    if (p_response == NULL) return;

    p_line = p_response->p_intermediates;

    while (p_line != NULL) {
        ATLine *p_toFree;

        p_toFree = p_line;
        p_line = p_line->p_next;

        free(p_toFree->line);
        free(p_toFree);
    }
    if (p_response->finalResponse != NULL)
        free(p_response->finalResponse);
    free (p_response);
}

/**
 * The line reader places the intermediate responses in reverse order
 * here we flip them back
 */
static void reverseIntermediates(ATResponse *p_response)
{
    ATLine *pcur,*pnext;

    pcur = p_response->p_intermediates;
    p_response->p_intermediates = NULL;

    while (pcur != NULL) {
        pnext = pcur->p_next;
        pcur->p_next = p_response->p_intermediates;
        p_response->p_intermediates = pcur;
        pcur = pnext;
    }
}

/**
 * Internal send_command implementation
 * Doesn't lock or call the timeout callback
 *
 * timeoutMsec == 0 means infinite timeout
 */

static int at_send_command_full_nolock_ack (const char *command, ATCommandType type,
                    const char *responsePrefix, const char *smspdu,
                    long long timeoutMsec, ATResponse **pp_outResponse, RILChannelCtx *p_channel,
                    RIL_Token ackToken)
{
    int err = 0;
    struct timespec ts;

    if(p_channel->p_response != NULL) {
        err = AT_ERROR_COMMAND_PENDING;
        LOGE("AT_ERROR_COMMAND_PENDING: %s", command);
        goto error;
    }

    err = writeline (command, p_channel);
    if (err < 0) {
        goto error;
    }

    p_channel->type = type;
    p_channel->responsePrefix = responsePrefix;
    p_channel->smsPDU = smspdu;
    p_channel->p_response = at_response_new();
    p_channel->p_response->current_cmd = command;

    if (timeoutMsec != 0) {
        setTimespecRelative(&ts, timeoutMsec);
    }


    while (p_channel->p_response->finalResponse == NULL && p_channel->readerClosed == 0) {
        if (timeoutMsec != 0) {
            err = pthread_cond_timedwait(&p_channel->commandcond, &p_channel->commandmutex, &ts);
        } else {
            err = pthread_cond_wait(&p_channel->commandcond, &p_channel->commandmutex);
        }

        if (err == ETIMEDOUT) {
            LOGI("at_send_command_full_nolock pthread cond wait error");
            err = AT_ERROR_TIMEOUT;
            goto error;
        }
        if ((p_channel->p_response->finalResponse == NULL && p_channel->readerClosed == 0)
                && (p_channel->p_response->ack == 1)) {
            if (ackToken != NULL) {
                RIL_onRequestAck(ackToken);
            }
            p_channel->p_response->ack = 0;
        }
    }

    if (p_channel->readerClosed > 0) {
        LOGI("at_send_command_full_nolock pthread cond wait reader closed");
        err = AT_ERROR_CHANNEL_CLOSED;
        goto error;
    }

    if (pp_outResponse == NULL) {
        at_response_free(p_channel->p_response);
    } else {
        /* line reader stores intermediate responses in reverse order */
        reverseIntermediates(p_channel->p_response);
        *pp_outResponse = p_channel->p_response;
    }

    p_channel->p_response = NULL;

    err = 0;
error:
    clearPendingCommand(p_channel);

    return err;
}

static int at_send_command_full_nolock(const char *command, ATCommandType type,
        const char *responsePrefix, const char *smspdu,
        long long timeoutMsec, ATResponse **pp_outResponse, RILChannelCtx *p_channel) {
    return at_send_command_full_nolock_ack(command, type, responsePrefix, smspdu, timeoutMsec,
            pp_outResponse, p_channel, NULL);
}

int isHaveAeeFeatureSupport() {
    int isHaveAeeFeature = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.vendor.have_aee_feature", property_value, "0");
    isHaveAeeFeature = atoi(property_value);
    RLOGI("isHaveAeeFeatureSupport: %d", isHaveAeeFeature);
    return isHaveAeeFeature;
}

/**
 * Internal send_command implementation
 *
 * timeoutMsec == 0 means infinite timeout
 */
static int at_send_command_full_ack (const char *command, ATCommandType type,
                    const char *responsePrefix, const char *smspdu,
                    long long timeoutMsec, ATResponse **pp_outResponse, RILChannelCtx *p_channel,
                    RIL_Token ackToken)
{
    int err;
    if (s_atHookCallback != NULL) {
        return s_atHookCallback(command, type, responsePrefix, smspdu, timeoutMsec,
                pp_outResponse, p_channel, ackToken);
    }

    if (s_md3_off == 1 && !strstr(command, "EPOF")) {
        LOGD("s_md3_off ==1 and command %s, return", command);
        /// M: When md3 is off, rild create new *pp_outResponse, then each module do not need to care about this.
        if (pp_outResponse != NULL) {
            LOGD("s_md3_off ==1 at_response_new and command %s", command);
            *pp_outResponse = at_response_new();
            (*pp_outResponse)->success = 0;
        }
        return AT_ERROR_GENERIC;
    }

    /// M: Save main loop thread id. Used to lock and wait until init done before sending AT to MD
    waitAtInitDoneForProxyThread();

    if (0 != pthread_equal(p_channel->tid_reader, pthread_self())) {
        /* cannot be called from reader thread */
        LOGI("Invalid Thread: send on %s, reader:%lu, self: %lu",
                p_channel->myName, p_channel->tid_reader, pthread_self());
        //return AT_ERROR_INVALID_THREAD;
    }

    if (0 != p_channel->tid_myProxy) {
        /* This channel is occupied by some proxy */
        LOGI("Occupied Thread: %s send on %s, pthread_self(): %d, tid: %lu", command,
                p_channel->myName, gettid(), p_channel->tid_myProxy);
        //assert(0);
        // return AT_ERROR_INVALID_THREAD;
    }

    pthread_mutex_lock(p_channel->p_channelMutex);
    pthread_mutex_lock(&p_channel->commandmutex);
    /* Assign owner proxy */
    p_channel->tid_myProxy = pthread_self();


    // Follow VIA experience to expand timeout 30s.
    if (timeoutMsec != 0) {
        timeoutMsec += DEFAULT_RETRY_TIMES;
    }
    err = at_send_command_full_nolock_ack(command, type, responsePrefix, smspdu, timeoutMsec,
            pp_outResponse, p_channel, ackToken);

    /* Release the proxy */
    p_channel->tid_myProxy = 0;

    pthread_mutex_unlock(&p_channel->commandmutex);
    pthread_mutex_unlock(p_channel->p_channelMutex);

    if (err == AT_ERROR_TIMEOUT && s_onTimeout != NULL) {
        char modemException[PROPERTY_VALUE_MAX] = { 0 };
        property_get(PROPERTY_MODEM_EE, modemException, "");
        if ((isInternalLoad() || !isUserLoad()) && (!isModemPoweredOff() || strstr(command, "EPOF"))) {
            // parse the AT CMD
            char key[20] = {0};
            char *pErrMsg = NULL;
            pErrMsg = calloc(1, 201);
            if (pErrMsg != NULL) {
                int cmdLen = strlen(command);
                int i = 0, start = 0, end = cmdLen - 1;
                for (i = 0; i < cmdLen; i++) {
                    if (command[i] == '+') {
                        start = i + 1;
                    }
                    if (command[i] == '=') {
                        end = i - 1;
                        break;
                    }
                }
                strncpy(key, (command + start), (end - start + 1));
                if (strcmp(modemException, "exception") != 0) {
                    snprintf(pErrMsg, 200, "AT command pending too long, assert!!!"
                            " AT cmd: %s, modemException: %s\nCRDISPATCH_KEY:ATTO_MD3=%s",
                            key, modemException, key);
                    RLOGI("AT command pending too long, assert!!!"
                            " on %s, tid:%lu, AT cmd: %s, modemException: %s",
                            p_channel->myName, p_channel->tid_myProxy, command,
                            modemException);
                    if (isHaveAeeFeatureSupport() == 1) {
                        #ifdef HAVE_AEE_FEATURE
                        aee_system_exception("C2KRILD", NULL, DB_OPT_DEFAULT, pErrMsg);
                        _exit(0);
                        #endif
                    } else {
                        LOG_ALWAYS_FATAL("%s", pErrMsg);
                    }
                } else {
                    RLOGI("Modem already exception, ignore!!!"
                            " on %s, tid:%lu, last AT cmd: %s, modemException: %s",
                            p_channel->myName, p_channel->tid_myProxy, command,
                            modemException);
                    _exit(0);
                }
                free(pErrMsg);
            } else {
                if (strcmp(modemException, "exception") != 0) {
                    LOG_ALWAYS_FATAL(
                            "AT command pending too long, assert!!! on %s, tid:%lu, AT cmd: %s, modemException: %s",
                            p_channel->myName, p_channel->tid_myProxy, command,
                            modemException);
                } else {
                    RLOGI(
                            "Modem already exception, ignore!!! on %s, tid:%lu, last AT cmd: %s, modemException: %s",
                            p_channel->myName, p_channel->tid_myProxy, command,
                            modemException);
                }
            }
        } else {
            if (!isModemPoweredOff() || strstr(command, "EPOF")) {
                // Exit directly, Statusd will trigger TRM after C2K RILD killed
                LOGI("Exit directly. AT command pending too long, assert!!!"
                        " on %s, tid:%lu, AT cmd: %s,"
                        " modemException: %s, s_md3_off: %d",
                        p_channel->myName, p_channel->tid_myProxy, command,
                        modemException, s_md3_off);
                property_set("vendor.ril.cdma.report.case", "1");
                property_set("vendor.ril.mux.report.case", "2");
                triggerIoctl(CCCI_IOC_MD_RESET);
            } else {
                // If modem power on, rild process will restart
                LOGI("Modem already powered off, ignore all at timeout");
            }
        }
    }
    return err;
}

RILChannelCtx *getDefaultChannelCtx() {
    RILChannelCtx *channel = &s_RILChannel[RIL_SUPPORT_CHANNELS - 1];
    return channel;
}

RILChannelCtx *getRILChannelCtxFromToken(RIL_Token t) {
    assert(RIL_queryMyChannelId(t) < RIL_SUPPORT_CHANNELS);
    return &(s_RILChannel[RIL_queryMyChannelId(t)]);
}

void lockAtChannel(RIL_Token t) {
    RILChannelId current_channel_id = getRILChannelCtxFromToken(t)->id;
    for (int i = 0; i < RIL_SUPPORT_CHANNELS; i++) {
        LOGD("Sim switch lock channel[%d]", i);
        if (i != current_channel_id) {
            RILChannelCtx* p_channel = &s_RILChannel[i];
            pthread_mutex_lock(p_channel->p_channelMutex);
        }
    }
    LOGI("Sim switch lock all channel done");
}

void unlockAtChannel(RIL_Token t) {
    RILChannelId current_channel_id = getRILChannelCtxFromToken(t)->id;
    for (int i = 0; i < RIL_SUPPORT_CHANNELS; i++) {
        if (i != current_channel_id) {
            RILChannelCtx* p_channel = &s_RILChannel[i];
            pthread_mutex_unlock(p_channel->p_channelMutex);
        }
    }
    LOGD("Sim switch unlock all channel done");
}

RILChannelCtx *getChannelCtxbyProxy()
{
    int proxyId = RIL_queryMyProxyIdByThread();
    assert(proxyId < RIL_SUPPORT_CHANNELS);

    if (proxyId > -1 && proxyId < RIL_SUPPORT_CHANNELS)
        return &s_RILChannel[proxyId];
    else
        return getDefaultChannelCtx();
}

/**
 * Issue a single normal AT command with no intermediate response expected
 *
 * "command" should not include \r
 * pp_outResponse can be NULL
 *
 * if non-NULL, the resulting ATResponse * must be eventually freed with
 * at_response_free
 */
int at_send_command (const char *command, ATResponse **pp_outResponse, RILChannelCtx *p_channel)
{
    return at_send_command_ack(command, pp_outResponse, p_channel, NULL);
}

int at_send_command_ack(const char *command, ATResponse **pp_outResponse, RILChannelCtx *p_channel,
        RIL_Token ackToken) {
    int err;

    err = at_send_command_full_ack (command, NO_RESULT, NULL,
                                    NULL, ATCOMMAND_TIMEOUT_MSEC, pp_outResponse, p_channel, ackToken);

    return err;
}


int at_send_command_singleline (const char *command,
                                const char *responsePrefix,
                                 ATResponse **pp_outResponse,
                                 RILChannelCtx * p_channel)
{
    return at_send_command_singleline_ack(command, responsePrefix, pp_outResponse, p_channel, NULL);
}

int at_send_command_singleline_ack(const char * command,
        const char * responsePrefix,
        ATResponse ** pp_outResponse,
        RILChannelCtx * p_channel,
        RIL_Token ackToken) {
    int err;
    err = at_send_command_full_ack(command, SINGLELINE, responsePrefix, NULL,
            ATCOMMAND_TIMEOUT_MSEC, pp_outResponse, p_channel, ackToken);

    if (err == 0 && pp_outResponse != NULL && *pp_outResponse != NULL
            && (*pp_outResponse)->success > 0
            && (*pp_outResponse)->p_intermediates == NULL) {
        /* successful command must have an intermediate response */
        LOGI("at_send_command_singleline invalid response");
        at_response_free(*pp_outResponse);
        *pp_outResponse = NULL;
        return AT_ERROR_INVALID_RESPONSE;
    }
    return err;
}

/**
 * Send AT commands without timeout checking
 *
 * "command" should not include \r
 * pp_outResponse can be NULL
 *
 * if non-NULL, the resulting ATResponse * must be eventually freed with
 * at_response_free
 */
int at_send_command_notimeout(const char *command, ATResponse **pp_outResponse, RILChannelCtx *p_channel) {
    return at_send_command_notimeout_ack(command, pp_outResponse, p_channel, NULL);
}

int at_send_command_notimeout_ack(const char *command, ATResponse **pp_outResponse, RILChannelCtx *p_channel,
        RIL_Token ackToken) {
    int err;

    err = at_send_command_full_ack(command, NO_RESULT, NULL, NULL, 0,
            pp_outResponse, p_channel, ackToken);

    return err;
}

/**
 * Send AT commands with a specified timeout checking
 *
 * "command" should not include \r
 * pp_outResponse can be NULL
 *
 * if non-NULL, the resulting ATResponse * must be eventually freed with
 * at_response_free
 */
int at_send_command_with_specified_timeout(const char *command,
        long long timeoutMsec, ATResponse **pp_outResponse, RILChannelCtx *p_channel) {
    return at_send_command_with_specified_timeout_ack(command, timeoutMsec,
            pp_outResponse, p_channel, NULL);
}

int at_send_command_with_specified_timeout_ack(const char *command,
        long long timeoutMsec, ATResponse **pp_outResponse, RILChannelCtx *p_channel,
        RIL_Token ackToken) {
    int err;

    /* timeoutMsec will be added to a value read from property in at_send_command_full. ex. +30s*/
    err = at_send_command_full_ack(command, NO_RESULT, NULL, NULL, timeoutMsec,
            pp_outResponse, p_channel, ackToken);

    return err;
}

int at_send_command_numeric(const char * command,
        ATResponse ** pp_outResponse,
        RILChannelCtx * p_channel)
{
    return at_send_command_numeric_ack(command, pp_outResponse, p_channel, NULL);
}

int at_send_command_numeric_ack(const char * command,
        ATResponse ** pp_outResponse,
        RILChannelCtx * p_channel,
        RIL_Token ackToken) {
    int err;

    err = at_send_command_full_ack (command, NUMERIC, NULL,
                                    NULL, ATCOMMAND_TIMEOUT_MSEC, pp_outResponse, p_channel, ackToken);

    if (err == 0 && pp_outResponse != NULL && (*pp_outResponse)->success > 0
            && (*pp_outResponse)->p_intermediates == NULL) {
        /* successful command must have an intermediate response */
        at_response_free(*pp_outResponse);
        *pp_outResponse = NULL;
        return AT_ERROR_INVALID_RESPONSE;
    }

    return err;
}

int at_send_command_sms(const char *command, const char *pdu,
        const char *responsePrefix, ATResponse **pp_outResponse, RILChannelCtx * p_channel) {
    return at_send_command_sms_ack(command, pdu, responsePrefix, pp_outResponse, p_channel, NULL);
}

int at_send_command_sms_ack(const char *command, const char *pdu,
        const char *responsePrefix, ATResponse **pp_outResponse, RILChannelCtx * p_channel,
        RIL_Token ackToken) {
    int err;

    err = at_send_command_full_ack(command, SINGLELINE, responsePrefix, pdu,
            ATCOMMAND_TIMEOUT_MSEC, pp_outResponse, p_channel, ackToken);

    if (err == 0 && pp_outResponse != NULL && (*pp_outResponse)->success > 0
            && (*pp_outResponse)->p_intermediates == NULL) {
        /* successful command must have an intermediate response */
        at_response_free(*pp_outResponse);
        *pp_outResponse = NULL;
        return AT_ERROR_INVALID_RESPONSE;
    }

    return err;
}

int at_send_command_multiline(const char *command, const char *responsePrefix,
        ATResponse **pp_outResponse, RILChannelCtx * p_channel) {
    return at_send_command_multiline_ack(command, responsePrefix, pp_outResponse, p_channel, NULL);
}

int at_send_command_multiline_ack(const char *command, const char *responsePrefix,
        ATResponse **pp_outResponse, RILChannelCtx * p_channel,
        RIL_Token ackToken) {
    int err;

    err = at_send_command_full_ack(command, MULTILINE, responsePrefix, NULL,
            ATCOMMAND_TIMEOUT_MSEC, pp_outResponse, p_channel, ackToken);

    return err;
}

/** This callback is invoked on the command thread */
void at_set_on_timeout(void (*onTimeout)(RILChannelId channelId))
{
    s_onTimeout = onTimeout;
}

/**
 *  This callback is invoked on the reader thread (like ATUnsolHandler)
 *  when the input stream closes before you call at_close
 *  (not when you call at_close())
 *  You should still call at_close()
 */

void at_set_on_reader_closed(void (*onClose)(RILChannelId channel_id))
{
    s_onReaderClosed = onClose;
}


/**
 * Periodically issue an AT command and wait for a response.
 * Used to ensure channel has start up and is active
 */
int at_handshake(RILChannelCtx *p_channel)
{
    int i;
    int err = 0;

    if (0 != pthread_equal(p_channel->tid_reader, pthread_self())) {
        /* cannot be called from reader thread */
        return AT_ERROR_INVALID_THREAD;
    }

    // Wait for MD3 +VPUP URC before send ATZ.
    waitMd3InitDone();

    pthread_mutex_lock(&p_channel->commandmutex);

    for (i = 0 ; i < HANDSHAKE_RETRY_COUNT ; i++) {
        /* some stacks start with verbose off */
        err = at_send_command_full_nolock ("ATZ", NO_RESULT,
                    NULL, NULL, ATCOMMAND_TIMEOUT_MSEC, NULL, p_channel);

        if (err == 0) {
            break;
        }
    }

    if (err == 0) {
        /* pause for a bit to let the input buffer drain any unmatched OK's
           (they will appear as extraneous unsolicited responses) */

        sleepMsec(HANDSHAKE_TIMEOUT_MSEC);
    } else {
        char *pErrMsg = calloc(1, 201);
        assert(pErrMsg);
        char modemException[PROPERTY_VALUE_MAX] = { 0 };
        property_get(PROPERTY_MODEM_EE, modemException, "");

        snprintf(pErrMsg, 200, "AT command pending too long, assert!!!"
                " AT cmd: ATZ, modemException:%s\nCRDISPATCH_KEY:ATTO_MD3= ATZ",
                modemException);
        if (isHaveAeeFeatureSupport() == 1) {
#ifdef HAVE_AEE_FEATURE
            aee_system_exception("C2KRILD", NULL, DB_OPT_DEFAULT, pErrMsg);
            _exit(0);
#endif
        } else {
            LOG_ALWAYS_FATAL("%s", pErrMsg);
        }
    }

    pthread_mutex_unlock(&p_channel->commandmutex);

    return err;
}

/**
 * Returns error code from response
 * Assumes AT+CMEE=1 (numeric) mode
 */
AT_CME_Error at_get_cme_error(const ATResponse *p_response)
{
    int ret;
    int err;
    char *p_cur;

    if (p_response == NULL) {
        return CME_ERROR_NON_CME;
    }

    if (p_response->success > 0) {
        return CME_SUCCESS;
    }

    if (p_response->finalResponse == NULL
        || !strStartsWith(p_response->finalResponse, "+CME ERROR:")
    ) {
        return CME_ERROR_NON_CME;
    }

    p_cur = p_response->finalResponse;
    err = at_tok_start(&p_cur);

    if (err < 0) {
        return CME_ERROR_NON_CME;
    }

    err = at_tok_nextint(&p_cur, &ret);

    if (err < 0) {
        return CME_ERROR_NON_CME;
    }

    return (AT_CME_Error) ret;
}

//c+w
int at_send_command_singleline_cw(const char *command,
        const char *responsePrefix, const char * string,
        ATResponse **pp_outResponse, RILChannelCtx* p_channel) {
    return at_send_command_singleline_cw_ack(command, responsePrefix, string, pp_outResponse,
            p_channel, NULL);

}

int at_send_command_singleline_cw_ack(const char *command,
        const char *responsePrefix, const char * string,
        ATResponse **pp_outResponse, RILChannelCtx* p_channel,
        RIL_Token ackToken) {
    int err;
    md5 = string;
    LOGW("at_send_command_singleline_cw");
    LOGW("md5 = %s", md5);

    err = at_send_command_full_ack(command, SINGLELINE, responsePrefix, NULL,
            ATCOMMAND_TIMEOUT_MSEC, pp_outResponse, p_channel, ackToken);

    if (err == 0 && pp_outResponse != NULL && (*pp_outResponse)->success > 0
            && (*pp_outResponse)->p_intermediates == NULL) {
        /* successful command must have an intermediate response */
        at_response_free(*pp_outResponse);
        *pp_outResponse = NULL;
        return AT_ERROR_INVALID_RESPONSE;
    }

    return err;
}

/**
 * Starts AT handler on stream "fd'
 * returns 0 on success, -1 on error
 */
int at_open_old(int fd, ATUnsolHandler h) {
    int ret;
    pthread_attr_t attr;

    s_fd = fd;
    s_unsolHandler = h;
    s_readerClosed = 0;

    s_responsePrefix = NULL;
    s_smsPDU = NULL;
    sp_response = NULL;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&s_tid_reader, &attr, readerLoop, &attr);

    if (ret < 0) {
        perror("pthread_create");
        return -1;
    }

    return 0;
}

/* FIXME is it ok to call this from the reader and the command thread? */
void at_close_old() {
    if (s_fd >= 0) {
        close(s_fd);
    }
    s_fd = -1;

    pthread_mutex_lock(&s_commandmutex);
    memset(s_ATBuffer, 0, sizeof(s_ATBuffer));
    s_ATBufferCur = s_ATBuffer;
    s_readerClosed = 1;

    pthread_cond_signal(&s_commandcond);

    pthread_mutex_unlock(&s_commandmutex);

    /* the reader thread should eventually die */
}

int at_send_command_multiline_notimeout(const char *command, const char *responsePrefix,
        ATResponse **pp_outResponse, RILChannelCtx * p_channel) {
    return at_send_command_multiline_notimeout_ack(command, responsePrefix,
                pp_outResponse, p_channel, NULL);
}

int at_send_command_multiline_notimeout_ack(const char *command,
        const char *responsePrefix, ATResponse **pp_outResponse,
        RILChannelCtx * p_channel, RIL_Token ackToken) {
    int err;

    err = at_send_command_full_ack(command, MULTILINE, responsePrefix, NULL, 0,
        pp_outResponse, p_channel, ackToken);

    return err;
}

bool isChipTestMode() {
    char prop_val[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.chiptest.enable", prop_val, "0");
    int chipMode = atoi(prop_val);
    RLOGD("chipmode: %d", chipMode);
    if (chipMode == 1) {
        return true;
    } else {
        return false;
    }
}

int at_send_command_raw(const char *command, ATResponse **pp_outResponse, RILChannelCtx *p_channel)
{
    return at_send_command_raw_ack(command, pp_outResponse, p_channel, NULL);
}

int at_send_command_raw_ack(const char *command, ATResponse **pp_outResponse, RILChannelCtx *p_channel,
        RIL_Token ackToken) {
    int err;
    err = at_send_command_full_ack(command, RAW, NULL, NULL, 0, pp_outResponse, p_channel, ackToken);

    if (isChipTestMode() && (strStartsWith(command, "AT+CPOF") || strStartsWith(command, "AT+EPOF"))){
        //This part is for Chiptest, radio reqeust from ATCI.
        RLOGD("send raw: Received AT+CPOF of AT+EPOF, set radio state to off");
        setRadioState(RADIO_STATE_OFF);
    }
    return err;
}


void waitAtInitDoneForProxyThread() {
    if (s_at_init_finish == 0) {
        int ret = pthread_mutex_lock(&s_at_init_Mutex);
        assert(ret == 0);
        LOGD("C2K at_send_command_full get lock");
        // If this AT command is not sending from mainloop,
        // should wait mainloop initialization done.
        if (0 == pthread_equal(mainloop_thread_id, pthread_self())) {
            LOGD("C2K mainloop update thread_id and unlock");
            if (s_at_init_finish == 0) {
                LOGD("C2K at_send_command_full s_at_init_finish still 0 and wait for ready");
                ret = pthread_cond_wait(&s_at_init_Cond, &s_at_init_Mutex);
                assert(ret == 0);
                LOGD("C2K at_send_command_full finish wait");
            }
        }
        LOGD("C2K at_send_command_full finish unlock and ready to send");
        ret = pthread_mutex_unlock(&s_at_init_Mutex);
        assert(ret == 0);
    }
}

int isModemPoweredOff() {
    int slotId = getCdmaModemSlot();
    char tempstr[PROPERTY_VALUE_MAX] = { 0 };
    if (slotId == 1) {
        property_get("vendor.ril.ipo.radiooff", tempstr, "");
    } else if (slotId == 2) {
        property_get("vendor.ril.ipo.radiooff.2", tempstr, "");
    } else {
        LOGD("can not get right value from getCdmaModemSlot");
    }
    LOGD("isModemPoweredOff radio off system property=%s", tempstr);
    if (!strcmp(tempstr, "1") || (s_md3_off == 1)) {
        return 1;
    } else {
        return 0;
    }
}

static int findStrInResponse(ATResponse * p_response, const char* strSought)
{
    ATLine *p_line;
    int result = 0;

    if (p_response == NULL)
    {
       goto error;
    }

    p_line = p_response->p_intermediates;
    while (p_line != NULL) {
        if (!strncmp(p_line->line, strSought, strlen(strSought))) {
            result = 1;
            break;
        }
        p_line = p_line->p_next;
    }
error:
    LOGD("findStrInResponse, result=%d", result);
    return result;
}

void openRILChannels(char mux_path[][32], ATUnsolHandler h) {
    RILChannelCtx *p_channel;
    for (int i = 0; i < RIL_SUPPORT_CHANNELS; i++) {
        p_channel = &s_RILChannel[i];
        while (p_channel->fd < 0) {
            do {
                LOGD("muxpath[%d] = %s", i, mux_path[i]);
                p_channel->fd = open(mux_path[i], O_RDWR);
                LOGD("open psuedo errno = %d", errno);
            } while (p_channel->fd < 0 && errno == EINTR);

            p_channel->p_channelMutex = getPseudoChannelMutex(i);

            LOGD("Channel %d path=%s, fd=%d, mutex = %x", i, mux_path[i], p_channel->fd,
                 p_channel->p_channelMutex);
            if (p_channel->fd < 0) {
                perror("opening AT interface. retrying...");
                RLOGE("could not connect to %s: %s", mux_path[i],
                     strerror(errno));
                sleep(10);
                /* never returns */
            } else {
                /* disable echo on serial ports */
                struct termios ios;
                tcgetattr(p_channel->fd, &ios);
                ios.c_lflag = 0; /* disable ECHO, ICANON, etc... */
                ios.c_iflag = 0;
                tcsetattr(p_channel->fd, TCSANOW, &ios);
            }

            int ret = at_open(p_channel->fd, h, p_channel);
            if (ret < 0) {
                LOGE("AT error %d on at_open\n", ret);
                break;
            }
        }
    }
}

