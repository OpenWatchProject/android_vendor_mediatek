#ifndef SUBSIDYLOCK_IPC_H
#define SUBSIDYLOCK_IPC_H 1

#include <android/log.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>

#define COMMAND_SEPERATOR_LENGTH    1
#define COMMAND_SIM_ID_LENGTH       1
#define COMMAND_RESULT_CODE_LENGTH  15
#define SOCKET_RECV_TIME 10
#define RIL_MODE_90 0
#define RIL_MODE_93 1

#define PERMANENT_UNLOCKED 0
#define SUBSIDYLOCKED 1

#define INVALIDE_SOCKET_FD -1
#define RILD_SOCKET_NAME "rild-oem"

#define PROP_SUBSIDYLOCK_STATUS_RIL = "persist.subsidylock.lockstatus_ril"

#define LOGD(format, ...)  \
    __android_log_print(ANDROID_LOG_DEBUG, "SUBSIDYLOCK_IPC", (format), ##__VA_ARGS__ )

#define LOGW(format, ...)  \
    __android_log_print(ANDROID_LOG_WARN, "SUBSIDYLOCK_IPC", (format), ##__VA_ARGS__ )

#define LOGE(format, ...)  \
    __android_log_print(ANDROID_LOG_ERROR, "SUBSIDYLOCK_IPC", (format), ##__VA_ARGS__ )

#define LOGI(format, ...)  \
    __android_log_print(ANDROID_LOG_INFO, "SUBSIDYLOCK_IPC", (format), ##__VA_ARGS__ )

#define RILD_SOCKET_NAME "rild-oem"


enum ModemStatus
{
    MODEM_STATUS_OK = 0,
    MODEM_STATUS_INVALID_ALGO_SIGNATURE = 1,
    MODEM_STATUS_INVALID_CONFIG_DATA = 2,
    MODEM_STATUS_IMEI_MISMATCH = 3,
    MODEM_STATUS_LENGTH_MISMATCH = 4,
    MODEM_STATUS_OTHER_ERROR = 5
};

extern uint8_t Modem_Remote_Simlock_ex(uint8_t *buffer_ptr, uint32_t buffer_len);
extern uint8_t Modem_Remote_Simlock_Get_Status_ex(uint8_t * buffer_ptr,
                                        uint8_t buffer_len);

static int toByte(char c){
    if (c >= '0' && c <= '9') return (c - '0');
    if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f') return (c - 'a' + 10);

    LOGE("toByte Error: %c",c);
    return 0;
}

static int checkByte(unsigned char* hexString){
    unsigned int i = 0;

    for (i = 0; i < strlen((const char*)hexString); i++) {
        if (!((hexString[i] >= '0' && hexString[i] <= '9')
            ||(hexString[i] >= 'A' && hexString[i] <= 'F')
            ||(hexString[i] >= 'a' && hexString[i] <= 'f'))) {
            LOGE("checkByte Error: %c", hexString[i] );
            return -1;
        }
    }

    return 0;
}

static void sleepMsec(long long msec) {
    struct timespec ts;
    int err;

    ts.tv_sec = (msec / 1000);
    ts.tv_nsec = (msec % 1000) * 1000 * 1000;

    do {
        err = nanosleep(&ts, &ts);
    } while (err < 0 && errno == EINTR);
}

#endif /*SUBSIDYLOCK_IPC_H*/