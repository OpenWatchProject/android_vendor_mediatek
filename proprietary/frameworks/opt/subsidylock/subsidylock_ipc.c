#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <error.h>

#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <sys/socket.h>
#include <android/log.h>
#include "subsidylock_ipc.h"
#include "subsidylock_adaptation.h"

#define SUBSIDYLOCK_SET_BLOB 1
#define SUBSIDYLOCK_GET_STATUS 2
#define IS_RILDFDVALID() ((s_fdRild_command > 0)&&(true == isRilAtciSocketExist)? true:false)

bool checkConnectivityStatus();
bool isSubsidyLockEnabled();

static pthread_mutex_t s_mutex_get_status = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t s_mutex_update_setting = PTHREAD_MUTEX_INITIALIZER;

int s_fd_get_status = 0;
int s_fdRild_command = INVALIDE_SOCKET_FD;
static int s_subsidylock_status = 1;

static void disconnect_socket(int fd) {
    if(fd == INVALIDE_SOCKET_FD) {
        LOGE("[disconnect_socket] Invalid fd: %d", fd);
        return;
    }
    LOGE("[disconnect_socket] %d", fd);
    close(fd);
    s_fdRild_command = INVALIDE_SOCKET_FD;
}

int parse_response(char* result, unsigned char* dest, unsigned int destLength) {
    int ret = ((result == NULL) ? MODEM_STATUS_OTHER_ERROR : (int)strlen(result));
    char *error_string = NULL;
    int error_code = 0;
    bool lock_state = false;
    int* resp = NULL;
    if (NULL == result) {
        return MODEM_STATUS_OTHER_ERROR;
    }

    resp = (int*) result;
    int reqType = resp[0];
    int status = resp[1];
    LOGD("parse_response: reqType = %d, status = %d", reqType, status);

    if (reqType == SUBSIDYLOCK_SET_BLOB) {
        int error_code = status;
        if (error_code == MODEM_STATUS_OK) {
            LOGD("update set blob success");
            return MODEM_STATUS_OK;
        }

        switch(error_code) {
            case 8000:
                ret = MODEM_STATUS_OK;//MODEM_STATUS_VERIFICATION_FAILED; //todo: check this.
                break;

            case 8001:
                ret = MODEM_STATUS_INVALID_ALGO_SIGNATURE;
                break;

            case 8002:
                ret = MODEM_STATUS_INVALID_CONFIG_DATA;//MODEM_STATUS_REBOOT_REQUEST;
                break;

            case 8003:
                ret = MODEM_STATUS_IMEI_MISMATCH;//MODEM_STATUS_REBOOT_REQUEST;
                break;

            case 8004:
                ret = MODEM_STATUS_LENGTH_MISMATCH;//MODEM_STATUS_REBOOT_REQUEST;
                break;

            case 8005:
                ret = MODEM_STATUS_OTHER_ERROR;//MODEM_STATUS_REBOOT_REQUEST; //todo: check this.
                break;

            default:
                ret = MODEM_STATUS_OTHER_ERROR; //todo: check this.
        }
    } else if (reqType == SUBSIDYLOCK_GET_STATUS && status <= 1) {
        if(status == 0) {
            LOGI("SUBSIDYLOCK STATUS: PERMANENT_UNLOCKED");
            dest[0] = false;
        } else {
            LOGI("SUBSIDYLOCK STATUS: SUBSIDYLOCKED");
            dest[0] = true;
        }
        ret = MODEM_STATUS_OK;
    } else {
        ret  = MODEM_STATUS_OTHER_ERROR;
        LOGE("INVALID/ERROR RESPONSE");
    }
    LOGI("[parse_response]: %d", ret);
    return ret;
}

bool connectTarget() {
    LOGI("connectTarget");

    // Set a timeout so that if nothing is read in 10 seconds, we'll stop reading and quit.
    struct timeval tv;
    tv.tv_sec = SOCKET_RECV_TIME;
    tv.tv_usec = 0;

    if (s_fdRild_command == INVALIDE_SOCKET_FD) {
        int fd[2] = {0};
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) >= 0) {
            s_fdRild_command = fd[0];
            if (setsockopt(s_fdRild_command, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1) {
                LOGE("Fail to set SOCKET RECV TIMEOUT for fd: %d, error: %s", fd,  strerror(errno));
            } else {
                LOGE("Sucess SOCKET RECV TIMEOUT for fd: %d", fd);
            }
            setSocketFdForAdaptation(fd[1]);
            LOGI("socketpair success fd[0] = %d, fd[1] = %d", fd[0], fd[1]);
            return true;
        } else {
            LOGE("socketpair failed. errno:%d", errno);
            return false;
        }
    }
    LOGI("socketpair already exist");
    return true;
}

int readDataFromRild(int s_fd_listen, void *data, uint32_t data_len) {
    int recvLen = 0;
    const int MAX_DATA_SIZE = 2 * sizeof(int);
    char buffer[MAX_DATA_SIZE + 1];
    //char* data;
    int response = -1;
    int ret = MODEM_STATUS_OTHER_ERROR;

    LOGI("readDataFromTarget with fd = %d", s_fd_listen);

    memset(buffer, 0, sizeof(buffer));

    do {
        recvLen = recv(s_fd_listen, buffer, MAX_DATA_SIZE, 0);
        if (recvLen == -1) {
            if(errno == EAGAIN || errno == ETIMEDOUT) {
                LOGE("recv timeout");
            }
            LOGE("fail to receive data from target socket. errno = %d", errno);
            ret = MODEM_STATUS_OTHER_ERROR;
            goto done;
        } else if(recvLen == 0) {
            LOGE("The peer has performed an orderly shutdown");
            ret = MODEM_STATUS_OTHER_ERROR;
            goto done;
        }
    } while(recvLen <= 0);

    ret = parse_response((char *)buffer, data, data_len);
done:
    return ret;
}

uint8_t subsidylock_update_simlock_settings(uint8_t sim_id, const uint8_t *simlock_blob,
                                        uint32_t simlock_blob_length) {
    int ret = MODEM_STATUS_OK;
    char *result = (char *)calloc(1, sizeof(char));
    unsigned int i = 0;
    //int fd = 0;
    int lock_ret = 1;

    //unsigned int command_len = strlen("SUBSIDYLOCK_SET") +
                //(COMMAND_SEPERATOR_LENGTH * 2) + COMMAND_SIM_ID_LENGTH ;
    //char *command = NULL;

    unsigned int slb_len = (simlock_blob_length * 2) + 1;
    char *slb = (char *)calloc(slb_len, sizeof(char));

    if (simlock_blob == NULL) {
        LOGE("Received Null blob.");
        ret = MODEM_STATUS_OTHER_ERROR;
        goto done;
    }

    LOGI("subsidylock_update_simlock_settings: %s, %d", simlock_blob, simlock_blob_length);

    for (i = 0; i < simlock_blob_length; i++) {
        sprintf((slb + (i*2)), "%02X", simlock_blob[i]);
    }

    LOGI("subsidylock_update_simlock_settings: slb(char array) %s", slb);
    do {
        lock_ret = pthread_mutex_trylock(&s_mutex_get_status);
        LOGD("try lock returns %d", lock_ret);
        if (lock_ret) {
            LOGE("s_mutex_update_setting lock unsuccessful");
        } else {
            LOGD("s_mutex_update_setting lock Successful");
            break;
        }
    } while (lock_ret);

    if (s_fdRild_command == INVALIDE_SOCKET_FD
            && !connectTarget()) {
        ret = MODEM_STATUS_OTHER_ERROR;
        goto done;
    }

    if (s_fdRild_command != INVALIDE_SOCKET_FD
            && !sendRequest(SUBSIDYLOCK_SET_BLOB, slb, slb_len)) {
        ret = MODEM_STATUS_OTHER_ERROR;
        goto done;
    }
    ret = readDataFromRild(s_fdRild_command, result, 0);
    LOGI("subsidylock_update_simlock_settings: ret = %d", ret);
    if (ret != MODEM_STATUS_OK) {
        goto done;
    }

done:
    if (slb != NULL) {
        free(slb);
    }

    if (result != NULL) {
        free(result);
    }

    disconnect_socket(s_fdRild_command);
    lock_ret = pthread_mutex_unlock(&s_mutex_get_status);
    LOGD("s_mutex_update_setting unlock Successful: %d", lock_ret);
    return ret;
}

//void subsidylock_get_hidl_response(int reqType, )

uint8_t subsidylock_get_modem_status(uint8_t sim_id, uint8_t *status_buffer,
                                uint32_t status_buffer_length) {
    int ret = MODEM_STATUS_OK;
    int lock_ret = 1;


    if (status_buffer == NULL) {
        ret = MODEM_STATUS_OTHER_ERROR;
        goto done;
    }

    LOGD("subsidylock_get_modem_status: %d, %s, %d", sim_id, status_buffer, status_buffer_length);

    do {
        lock_ret = pthread_mutex_trylock(&s_mutex_get_status);
        LOGD("try lock returns %d", lock_ret);
        if (lock_ret) {
            LOGE("s_mutex_get_status lock unsuccessful");
            return MODEM_STATUS_OTHER_ERROR;
        } else {
            LOGI("s_mutex_get_status lock Successful");
            break;
        }
    } while (lock_ret);

    if (s_fdRild_command == INVALIDE_SOCKET_FD
            && !connectTarget()) {
        ret = MODEM_STATUS_OTHER_ERROR;
        goto done;
    }

    if (s_fdRild_command != INVALIDE_SOCKET_FD
            && !sendRequest(SUBSIDYLOCK_GET_STATUS, NULL, 0)) {
        ret = MODEM_STATUS_OTHER_ERROR;
        goto done;
    }
    ret = readDataFromRild(s_fdRild_command, status_buffer, status_buffer_length);
    LOGI("subsidylock_get_modem_status: [status_buffer] %s, ret = %d", status_buffer, ret);
    if (ret != MODEM_STATUS_OK) {
        goto done;
    }

done:
    disconnect_socket(s_fdRild_command);
    lock_ret = pthread_mutex_unlock(&s_mutex_get_status);
    LOGI("s_mutex_get_modem_status unlock Successful: %d", lock_ret);
    return ret;
}

uint8_t Modem_Remote_Simlock(uint8_t *buffer_ptr, uint32_t buffer_len) {
    /* FOR RIL_MODE 93 */
    int ret = MODEM_STATUS_OK;
    if (isSubsidyLockEnabled() == false) {
        LOGI("Modem_Remote_Simlock_Get_Status Subsidy Lock is not supported");
        return MODEM_STATUS_OTHER_ERROR;
    }
    ret = subsidylock_update_simlock_settings(0, buffer_ptr, buffer_len);

    if (ret == MODEM_STATUS_OK) {
        const unsigned int MAX_COUNT = 10;
        const unsigned int SLEEP_INTERVAL = 500;  // 500 ms
        for (int count = 0; count < MAX_COUNT; ++count) {
            if (checkConnectivityStatus() == true) {
                break;
            }
            LOGD("SLEEP_INTERVAL start");
            sleepMsec(SLEEP_INTERVAL);
            LOGD("SLEEP_INTERVAL end");
        }
    }
    LOGI("Modem_Remote_Simlock update blob: ret = %d, status = %c" , ret, ((char*)buffer_ptr)[0]);
    return ret;
}

uint8_t Modem_Remote_Simlock_Get_Status(uint8_t * buffer_ptr,
                                        uint8_t buffer_len) {
    /* FOR RIL_MODE 93 */
    int ret = MODEM_STATUS_OK;
    if (isSubsidyLockEnabled() == false) {
        LOGI("Modem_Remote_Simlock_Get_Status Subsidy Lock is not supported");
        return MODEM_STATUS_OTHER_ERROR;
    }
    ret = subsidylock_get_modem_status(0, buffer_ptr, buffer_len);
    if (ret != MODEM_STATUS_OK) {
        ret = MODEM_STATUS_INVALID_ALGO_SIGNATURE; // 1 = error, 0 = OK
    } else {
        LOGI("buffer_ptr = %d", buffer_ptr[0]);
        if (buffer_ptr[0] == 0) {
            LOGI("PERMANENT_UNLOCKED");
        } else {
            LOGI("SUBSIDYLOCKED");
        }
    }
    LOGI("Modem_Remote_Simlock_Get_Status: ret = %d", ret);
    return ret;
}

bool checkConnectivityStatus() {
    char property_value[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.vendor.subsidylock.connectivity_status", property_value, "0");

    LOGD("[SUBSIDY_LOCK] checkConnectivityStatus = %d ", atoi(property_value));
    if (atoi(property_value) == 1) {
        return true;
    }
    return false;
}

bool isSubsidyLockEnabled() {
    char property_value[PROPERTY_VALUE_MAX] = {0};
    char property_value_standalone[PROPERTY_VALUE_MAX] = {0};

    property_get("vendor.gsm.sim.slot.lock.policy", property_value, "0");
    property_get("ro.vendor.mtk_subsidy_lock_support", property_value_standalone, "0");

    LOGD("[SUBSIDY_LOCK] isSubsidyLockEnabled = %d, %d ", atoi(property_value),
            atoi(property_value_standalone));
    if ((atoi(property_value) == 10) || (atoi(property_value_standalone) == 1)) {
        return true;
    }
    return false;
}
