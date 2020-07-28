#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>
#include <mtk_log.h>

#include "atch.h"
#include "atapi.h"

#define TrcMsg(f, a...)   printf("[ATAPI-TRC] %s:%d: " f "\n", &__FILE__[0], __LINE__,  ## a)
//#define DbgMsg(f, a...)   printf("[ATAPI-DBG] %s:%d: " f "\n", &__FILE__[0], __LINE__,  ## a)
//#define ErrMsg(f, a...)   printf("[ATAPI-ERR] %s:%d: " f "\n", &__FILE__[0], __LINE__,  ## a)
//#define SysMsg(f, a...)   printf("[ATAPI-SYS] %s:%d: " f "\n", &__FILE__[0], __LINE__,  ## a)

#define NA_LOG_TAG "Atapi"
#define DbgMsg(...) ((void)mtkLogD(NA_LOG_TAG, __VA_ARGS__))
#define ErrMsg(...) ((void)mtkLogE(NA_LOG_TAG, __VA_ARGS__))

#define MAX_AT_CMD_BUFFER           (1024)
#define MAX_AT_LINE_BUFFER          (1024)
#define MAX_AT_RESPONSE_BUFFER      (4096)

#define TIMEOUT_REALTIME_CMD        (5)
#define TIMEOUT_SHORT_QUERY         (30)
#define TIMEOUT_LONG_QUERY          (300)
#define TIMEOUT_SHORT_EXECUTE       (30)
#define TIMEOUT_NORMAL_EXECUTE     (120)
#define TIMEOUT_LONG_EXECUTE        (300)

typedef struct _atapi_info {
    void                           *channel;
    int                             channel_type;
    void                           *data;
    pthread_mutex_t                 mutex;

    int                             urc_mode;
    ATAPI_urc_notify_cb             urc_cb;
} atapi_info_t;


/* ---------------------------------------------------- */
/*  ATAPI - Internal function                           */
/* ---------------------------------------------------- */

enum AT_AT_FINALCODE_e {
    AT_FINAL_CODE_OK            = 0,
    AT_FINAL_CODE_CONNECT       = 1,
    AT_FINAL_CODE_ERROR         = 2,
    AT_FINAL_CODE_NO_CARRIER    = 3,
    AT_FINAL_CODE_NO_ANSWER     = 4,
    AT_FINAL_CODE_NO_DIALTONE   = 5,
    AT_FINAL_CODE_URC           = 6,
    AT_FINAL_CODE_TIMEOUT       = 7,
    AT_FINAL_CODE_CME           = 8,
    AT_FINAL_CODE_CMS           = 9,
    AT_FINAL_CODE_DROP          = 10,
    AT_FINAL_CODE_BUFFER_FULL   = 11,
    AT_FINAL_CODE_CHANNEL_ERROR = 12,
    AT_FINAL_CODE_UNKNOWN
};


struct _final_code {
    char   *key;
    int     value;
} final_code_list[10] = {
    {"OK\r\n",          AT_FINAL_CODE_OK},
    {"ERROR\r\n",       AT_FINAL_CODE_ERROR},
    {"CONNECT\r\n",     AT_FINAL_CODE_CONNECT},
    {"NO CARRIER\r\n",  AT_FINAL_CODE_NO_CARRIER},
    {"NO ANSWER\r\n",   AT_FINAL_CODE_NO_ANSWER},
    {"NO DIALTONE\r\n", AT_FINAL_CODE_NO_DIALTONE},
    {"+CME ERROR:",      AT_FINAL_CODE_CME},
    {"+CMS ERROR:",      AT_FINAL_CODE_CMS},
    {NULL, 0}
};


static int
_atapi_check_final_code(char *line, int *final_code) {
    int i = 0;

    for (i=0 ; final_code_list[i].key ; i++) {
        char *ptr = line;

        while (*ptr && (*ptr == '\r' || *ptr == ' ')) ptr++;
        if (strncasecmp(ptr, final_code_list[i].key, strlen(final_code_list[i].key)) == 0) {
            *final_code = final_code_list[i].value;
            return 0;
        }
    }

    return -1;
}


#define IGNORE_CHAR(s,c) {while ((*(s)) == (c) && *(s)) (s)++;}
#define FIND_CHAR(s,c) {while ((*(s)) != (c) && *(s)) (s)++;}

#if 0
static int
_atapi_get_argvs(char *line, int argc, char **argv) {
    char *str = line;
    int num = 0;

    {
        int index=  0;
        index = strcspn(str, "\r\n");
        str[index] = 0;
    }

    while (num < argc && *str) {
        IGNORE_CHAR(str, ' ');
        argv[num++] = ((*str == '"') ? (str+1) : str);
        while (*str) {
            if (*str == '"') {
                str++;
                FIND_CHAR(str, '"');
                if (*str == '"') {
                    *str = 0;
                }
                str++;
                FIND_CHAR(str, ',');
                if (*str == ',') {
                    str++;
                }
                break;
            } else {
                FIND_CHAR(str, ',');
                if (*str == ',') {
                    char *ptr = str-1;

                    *str = 0;
                    str++;

                    while (*ptr == ' ') {
                        *ptr = 0;
                        ptr--;
                    }

                    break;
                }
            }
        }
    }

    return num;
}
#endif


static int
_atapi_verify(void *obj, int type) {
    atapi_info_t *atapi_info = (atapi_info_t *)obj;

    if (!atapi_info) {
        ErrMsg("obj is NULL");
        return ATAPI_RET_ERROR;
    }

    if (!(atapi_info->channel)) {
        ErrMsg("channel is NULL");
        return ATAPI_RET_ERROR;
    }

    if (type == 1) {    // send at command
        if (atapi_info->urc_mode == 1) {
            ErrMsg("Current is URC mode, can't execute the at command");
            return ATAPI_RET_ERROR;
        }
    }

    return ATAPI_RET_SUCCESS;
}


static int
_atapi_execute_cmd(void *channel, char *cmd, char *buffer, int buffer_len, char *prefix, int timeout) {
    int fd = atch_getfd(channel);
    int buffer_offset = 0;
    char tmp_buffer[MAX_AT_RESPONSE_BUFFER] = {0};
    int tmp_buffer_offset = 0;
    int is_get_response = (prefix) ? 0 : 1;
    struct sysinfo curr_info = {0};
    long end_time = 0;
    char local_cmd[512] = {0};

    if (fd < 0) {
        ErrMsg("Can't get the fd of channel");
        return AT_FINAL_CODE_CHANNEL_ERROR;
    }

    sysinfo(&curr_info);
    end_time = curr_info.uptime + timeout;

    snprintf(local_cmd, sizeof(local_cmd), "%s\r\n", cmd);
    atch_send(channel, local_cmd, strlen(local_cmd));

    while (1) {
        struct timeval timeval;
        fd_set  iofds;
        int ret = 0;

        sysinfo(&curr_info);
        if (curr_info.uptime >= end_time) {
            break;
        }

        FD_ZERO(&iofds);
        FD_SET(fd, &iofds);

        timeval.tv_sec  = 1;
        timeval.tv_usec = 0;

        ret = select(fd + 1, &iofds, 0, 0, &timeval);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            ErrMsg("select error, errno = %d (%s)", errno, strerror(errno));
            return AT_FINAL_CODE_CHANNEL_ERROR;
        } else if (ret == 0) {
            continue;
        }

        if (FD_ISSET(fd, &iofds)) {
            char *ptr = 0;
            int prefix_len = (prefix) ? strlen(prefix) : 0;

            ret = atch_recv(channel, tmp_buffer+tmp_buffer_offset, sizeof(tmp_buffer)-tmp_buffer_offset-1);
            if (ret < 0) {
                ErrMsg("Can't read the data, errno = %d (%s)", errno, strerror(errno));
                return AT_FINAL_CODE_CHANNEL_ERROR;
            } else if (ret == 0) {
                ErrMsg("Channel is disconnect");
                return AT_FINAL_CODE_CHANNEL_ERROR;
            }

            tmp_buffer_offset += ret;
            tmp_buffer[tmp_buffer_offset] = 0;

            while ((ptr = strchr(tmp_buffer, '\n'))) {
                int update = 1;
                int final_code = -1;
                int last_char = *(ptr+1);

                *(ptr+1) = 0;

                if (prefix) {
                    if (strncasecmp(tmp_buffer, prefix, prefix_len) != 0) {
                        update = 0;
                        if((strncasecmp(tmp_buffer,"+CME ERROR:",11)==0)||(strncasecmp(tmp_buffer,"+CMS ERROR:",11)==0)) {
                            is_get_response = 1;
                        }
                    }
                }

                if (is_get_response) {
                    if (_atapi_check_final_code(tmp_buffer, &final_code) != 0) {
                        final_code = -1;
                    }
                }

                if (update || final_code >= 0) {
                    ret = snprintf(buffer+buffer_offset, buffer_len-buffer_offset, "%s", tmp_buffer);
                    if (ret <= 0) {
                        ErrMsg("Buffer full");
                        return AT_FINAL_CODE_BUFFER_FULL;
                    }
                    buffer_offset += ret;
                    buffer[buffer_offset] = 0;
                    is_get_response = 1;
                }

                *(ptr+1) = last_char;
                tmp_buffer_offset = tmp_buffer_offset - (ptr+1-tmp_buffer);
                memmove(tmp_buffer, ptr+1, tmp_buffer_offset);
                tmp_buffer[tmp_buffer_offset] = 0;

                if (final_code >= 0) {
                    return final_code;
                }
            }
        }
    }

    return AT_FINAL_CODE_TIMEOUT;
}


#if 0
static int
_atapi_execute_cmd_get_values(void *obj, char *at_cmd, char *prefix, int timeout, int num, int *value) {
    atapi_info_t *atapi_info = (atapi_info_t *)obj;
    char at_response[MAX_AT_RESPONSE_BUFFER] = {0};
    char *argv[16] = {0};
    int ret = 0;
    int i = 0;

    if (_atapi_verify(atapi_info, 1) != ATAPI_RET_SUCCESS) {
        return ATAPI_RET_ERROR;
    }

    if (!value) {
        return ATAPI_RET_ERROR;
    }

    if (num <= 0 || num > 15) {
        return ATAPI_RET_ERROR;
    }

    ret = _atapi_execute_cmd(atapi_info->channel, at_cmd, at_response, sizeof(at_response)-1, prefix, TIMEOUT_REALTIME_CMD);
    if (ret != AT_FINAL_CODE_OK) {
        ErrMsg("Can't execute the at command, ret = %d, cmd = (%s). response = (%s)", ret, at_cmd, at_response);
        return (ret == AT_FINAL_CODE_TIMEOUT) ? ATAPI_RET_TIMEOUT : ATAPI_RET_ERROR;
    }

    ret = _atapi_get_argvs(at_response + strlen(prefix), num, argv);
    if (ret != num) {
        ErrMsg("can't get value, cmd = (%s), response = (%s)", at_cmd, at_response);
        return ATAPI_RET_ERROR;
    }

    for (i=0 ; i<num ; ++i) {
        *value = strtol(argv[i], 0, 10);
        value++;
    }

    return ATAPI_RET_SUCCESS;
}

static int
_atapi_execute_cmd_get_strings(void *obj, char *at_cmd, char *prefix, int timeout, int num, char **value) {
    atapi_info_t *atapi_info = (atapi_info_t *)obj;
    char at_response[MAX_AT_RESPONSE_BUFFER] = {0};
    char *argv[16] = {0};
    int ret = 0;
    int i = 0;

    if (_atapi_verify(atapi_info, 1) != ATAPI_RET_SUCCESS) {
        return ATAPI_RET_ERROR;
    }

    if (!value) {
        return ATAPI_RET_ERROR;
    }

    if (num <= 0 || num > 15) {
        return ATAPI_RET_ERROR;
    }

    ret = _atapi_execute_cmd(atapi_info->channel, at_cmd, at_response, sizeof(at_response)-1, prefix, TIMEOUT_REALTIME_CMD);
    if (ret != AT_FINAL_CODE_OK) {
        ErrMsg("Can't execute the at command, ret = %d, cmd = (%s). response = (%s)", ret, at_cmd, at_response);
        return (ret == AT_FINAL_CODE_TIMEOUT) ? ATAPI_RET_TIMEOUT : ATAPI_RET_ERROR;
    }

    ret = _atapi_get_argvs(at_response + strlen(prefix), num, argv);
    if (ret != num) {
        ErrMsg("can't get value, cmd = (%s), response = (%s)", at_cmd, at_response);
        return ATAPI_RET_ERROR;
    }

    for (i=0 ; i<num ; ++i) {
        *value = strdup(argv[i]);
        value++;
    }

    return ATAPI_RET_SUCCESS;
}
#endif

static int
_atapi_urc_filter_enable(void *obj, int mode) {
    atapi_info_t *atapi_info = (atapi_info_t *)obj;

    if (_atapi_verify(atapi_info, 0) != ATAPI_RET_SUCCESS) {
        return ATAPI_RET_ERROR;
    }

    if (atapi_info->channel_type != ATCH_Type_UART) {
        char at_response[MAX_AT_RESPONSE_BUFFER] = {0};
        char at_cmd[32] = {0};
        snprintf(at_cmd, sizeof(at_cmd)-1, "AT@URCFILTER=%d", mode);
        _atapi_execute_cmd(atapi_info->channel, at_cmd, at_response, sizeof(at_response)-1, 0, 3);
        return 1;
    }

    return 0;
}


/* ---------------------------------------------------- */
/*  ATAPI - Core                                        */
/* ---------------------------------------------------- */

void *
atapi_init(int type, char *name) {
    atapi_info_t   *atapi_info = 0;

    atapi_info = malloc(sizeof(atapi_info_t));
    if (!atapi_info) {
        ErrMsg("Can't allocate the memory");
        return 0;
    }

    memset(atapi_info, 0, sizeof(atapi_info_t));

    atapi_info->channel = atch_client(type, name);
    if (!(atapi_info->channel)) {
        ErrMsg("Can't create the channel");
        free(atapi_info);
        return 0;
    }

    pthread_mutex_init(&(atapi_info->mutex), 0);

    atapi_info->channel_type = type;
    atapi_info->urc_mode     = 0;

    //_atapi_urc_filter_enable(atapi_info, 3);

    return atapi_info;
}


int
atapi_deinit(void *obj) {
    atapi_info_t *atapi_info = (atapi_info_t *)obj;

    if (_atapi_verify(atapi_info, 0) != ATAPI_RET_SUCCESS) {
        return ATAPI_RET_ERROR;
    }

    atch_close(atapi_info->channel);

    free(atapi_info);

    return ATAPI_RET_SUCCESS;
}


int
atapi_execute_command(void *obj, char *at_cmd, char *result_buffer, int result_buffer_len, char *result_prefix, int cmd_timeout) {
    atapi_info_t *atapi_info = (atapi_info_t *)obj;
    int ret = 0;


    if (_atapi_verify(atapi_info, 1) != ATAPI_RET_SUCCESS) {
        return ATAPI_RET_ERROR;
    }

    pthread_mutex_lock(&(atapi_info->mutex));
    ret = _atapi_execute_cmd(atapi_info->channel, at_cmd, result_buffer, result_buffer_len, result_prefix, cmd_timeout);
    pthread_mutex_unlock(&(atapi_info->mutex));
    if (ret != AT_FINAL_CODE_OK) {
        ErrMsg("Can't execute the at command, ret = %d, cmd = (%s)", ret, at_cmd);
        return (ret == AT_FINAL_CODE_TIMEOUT) ? ATAPI_RET_TIMEOUT : ATAPI_RET_ERROR;
    }

    return ATAPI_RET_SUCCESS;
}


int
atapi_execute_command_no_response(void *obj, char *at_cmd, int cmd_timeout) {
    atapi_info_t *atapi_info = (atapi_info_t *)obj;
    char at_response[MAX_AT_RESPONSE_BUFFER] = {0};
    int ret = 0;

    if (_atapi_verify(atapi_info, 1) != ATAPI_RET_SUCCESS) {
        return ATAPI_RET_ERROR;
    }

    pthread_mutex_lock(&(atapi_info->mutex));
    ret = _atapi_execute_cmd(atapi_info->channel, at_cmd, at_response, sizeof(at_response)-1, 0, cmd_timeout);
    pthread_mutex_unlock(&(atapi_info->mutex));
    if (ret != AT_FINAL_CODE_OK) {
        ErrMsg("Can't execute the at command, ret = %d, cmd = (%s), response = (%s)", ret, at_cmd, at_response);
        return (ret == AT_FINAL_CODE_TIMEOUT) ? ATAPI_RET_TIMEOUT : ATAPI_RET_ERROR;
    }

    return ATAPI_RET_SUCCESS;
}


/* ---------------------------------------------------- */
/*  ATAPI - Callback                                    */
/* ---------------------------------------------------- */

int
atapi_urc_init(void *obj) {
    atapi_info_t *atapi_info = (atapi_info_t *)obj;

    if (_atapi_verify(atapi_info, 0) != ATAPI_RET_SUCCESS) {
        return ATAPI_RET_ERROR;
    }

    if (atch_set2buffer(atapi_info->channel, 4096) < 0) {
        return ATAPI_RET_ERROR;
    }

    return ATAPI_RET_SUCCESS;
}


char *
atapi_urc_get(void *obj) {
    atapi_info_t *atapi_info = (atapi_info_t *)obj;
    int fd_urc = 0;
    int ret = 0;
    char *line = 0;

    if (_atapi_verify(atapi_info, 0) != ATAPI_RET_SUCCESS) {
        return 0;
    }

    line = atch_getline(atapi_info->channel);
    if (line) {
        return line;
    }

    fd_urc = atch_getfd(atapi_info->channel);
    while (1) {
        //struct timeval timeval;
        fd_set fdr,fdw;

        FD_ZERO(&fdw);
        FD_ZERO(&fdr);
        FD_SET(fd_urc, &fdr);

        //timeval.tv_sec  = 30;
        //timeval.tv_usec = 0;

        //ret = select(fd_urc + 1, &fdr, &fdw, 0, &timeval);
        ret = select(fd_urc + 1, &fdr, &fdw, 0, 0);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            ErrMsg("select error, errno = %d (%s)", errno, strerror(errno));
            break;
        } else if (ret == 0) {
            continue;
        }

        if (FD_ISSET(fd_urc, &fdr)) {
            ret = atch_read2buffer(atapi_info->channel);
            if (ret <= 0) {
                break;
            }

            line = atch_getline(atapi_info->channel);
            if (line) {
                return line;
            }
        }
    }
    return 0;
}


int
atapi_cb_enter_urc_mode(void *obj) {
    atapi_info_t *atapi_info = (atapi_info_t *)obj;
    int fd_urc = 0;
    int ret = 0;

    if (_atapi_verify(atapi_info, 0) != ATAPI_RET_SUCCESS) {
        return ATAPI_RET_ERROR;
    }

    atapi_info->urc_mode = 1;

    _atapi_urc_filter_enable(atapi_info, 2);

    if (atch_set2buffer(atapi_info->channel, 4096) < 0) {
        return ATAPI_RET_ERROR;
    }

    fd_urc = atch_getfd(atapi_info->channel);
    while (1) {
        struct timeval timeval;
        fd_set fdr,fdw;

        FD_ZERO(&fdw);
        FD_ZERO(&fdr);
        FD_SET(fd_urc, &fdr);

        timeval.tv_sec  = 30;
        timeval.tv_usec = 0;

        ret = select(fd_urc + 1, &fdr, &fdw, 0, &timeval);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            ErrMsg("select error, errno = %d (%s)", errno, strerror(errno));
            break;
        } else if (ret == 0) {
            continue;
        }

        if (FD_ISSET(fd_urc, &fdr)) {
            ret = atch_read2buffer(atapi_info->channel);
            if (ret <= 0) {
                break;
            }

            do {
                char *line = 0;

                line = atch_getline(atapi_info->channel);
                if (!line) {
                    break;
                }

                do {
                    {
                        char *ptr = 0;
                        ptr = strchr(line, '\n');
                        if (ptr) *ptr = 0;
                        ptr = strchr(line, '\r');
                        if (ptr) *ptr = 0;
                        if (atapi_info->urc_cb) {
                            atapi_info->urc_cb(atapi_info->data, line);
                        }
                    }
                } while (0);
                free(line);
            } while (1);
        }
    }

    atapi_info->urc_mode = 0;

    return ATAPI_RET_ERROR;
}


int
atapi_cb_set_user_data(void *obj, void *data) {
    atapi_info_t *atapi_info = (atapi_info_t *)obj;

    if (_atapi_verify(atapi_info, 0) != ATAPI_RET_SUCCESS) {
        return ATAPI_RET_ERROR;
    }

    atapi_info->data = data;

    return ATAPI_RET_SUCCESS;
}


int
atapi_cb_get_user_data(void *obj, void **data) {
    atapi_info_t *atapi_info = (atapi_info_t *)obj;

    if (_atapi_verify(atapi_info, 0) != ATAPI_RET_SUCCESS) {
        return ATAPI_RET_ERROR;
    }

    if (!data) {
        return ATAPI_RET_ERROR;
    }

    *data = atapi_info->data;

    return ATAPI_RET_SUCCESS;
}


int
atapi_cb_set_urc_notify(void *obj, ATAPI_urc_notify_cb fn) {
    atapi_info_t *atapi_info = (atapi_info_t *)obj;

    if (_atapi_verify(atapi_info, 0) != ATAPI_RET_SUCCESS) {
        return ATAPI_RET_ERROR;
    }

    atapi_info->urc_cb = fn;

    return ATAPI_RET_SUCCESS;
}


