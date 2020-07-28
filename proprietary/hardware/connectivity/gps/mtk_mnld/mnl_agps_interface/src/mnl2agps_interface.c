#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/time.h>
#include <time.h>

#include <stddef.h>  // offsetof
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>

#include <arpa/inet.h>  // inet_addr
#include <sys/un.h>  // struct sockaddr_un

#if defined(__TIZEN_OS__)
#include <dlog/dlog.h>
#endif

#include "mnl2agps_interface.h"
#include "data_coder.h"

#ifndef UNUSED
#define UNUSED(x) (x)=(x)
#endif

static char g_mnl2agps_path[128] = MNL_TO_AGPS;
#if 0
static void tag_log(int type, const char* tag, const char *fmt, ...) {
    char out_buf[1100] = {0};
    char buf[1024] = {0};
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    snprintf(out_buf, sizeof(out_buf), "%s %s", tag, buf);

#if defined(__ANDROID_OS__)
    if (type == 0) {
        __android_log_print(ANDROID_LOG_DEBUG, "agps", "%s", out_buf);
    } else {
        __android_log_print(ANDROID_LOG_ERROR, "agps", "%s", out_buf);
    }
#elif defined(__LINUX_OS__)
    UNUSED(type);
    printf("%s\n", out_buf);
#elif defined(__TIZEN_OS__)
    UNUSED(type);
    dlog_print(DLOG_DEBUG, "%s\n", out_buf);
    printf("%s\n", out_buf);
#else
    UNUSED(type);
#endif
}
#endif
#ifdef LOGD
#undef LOGD
#endif
#ifdef LOGW
#undef LOGW
#endif
#ifdef LOGE
#undef LOGE
#endif
#if 0
#define LOGD(...) tag_log(1, "[MNL2AGPS]", __VA_ARGS__);
#define LOGW(...) tag_log(1, " WARNING: [MNL2AGPS]", __VA_ARGS__);
#define LOGE(...) tag_log(1, " ERR: [MNL2AGPS]", __VA_ARGS__);
#else
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "MNL2AGPS"

#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#ifdef CONFIG_GPS_ENG_LOAD
#define LOGD_ENG(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#else
#define LOGD_ENG(fmt, arg ...) NULL
#endif
#endif
// -1 means failure
static int safe_recvfrom(int sockfd, char* buf, int len) {
    int ret = 0;
    int retry = 10;

    while ((ret = recvfrom(sockfd, buf, len, 0,
         NULL, NULL)) == -1) {
        LOGW("ret=%d len=%d\n", ret, len);
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
        }
        LOGE("sendto reason=[%s]\n", strerror(errno));
        break;
    }
    return ret;
}

// -1 means failure
static int set_socket_blocking(int fd, int blocking) {
    if (fd < 0) {
        LOGE("set_socket_blocking  invalid fd=%d\n", fd);
        return -1;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        LOGE("set_socket_blocking  invalid flags=%d\n", flags);
        return -1;
    }

    flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
    return (fcntl(fd, F_SETFL, flags) == 0) ? 0 : -1;
}

// -1 means failure
static int safe_sendto(int sockfd, const char* dest, const char* buf, int size) {
    int len = 0;
    struct sockaddr_un soc_addr;
    socklen_t addr_len;
    int retry = 10;

    MNLD_STRNCPY(soc_addr.sun_path, dest,sizeof(soc_addr.sun_path));
    soc_addr.sun_family = AF_UNIX;
    addr_len = (offsetof(struct sockaddr_un, sun_path) + strlen(soc_addr.sun_path) + 1);

    while ((len = sendto(sockfd, buf, size, 0,
        (const struct sockaddr *)&soc_addr, (socklen_t)addr_len)) == -1) {
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
        }
        LOGE("sendto dest=[%s] len=%d reason=[%s]\n",
            dest, size, strerror(errno));
        break;
    }
    return len;
}

static int is_path_exist(const char* path) {
    struct stat s;
    int err = stat(path, &s);
    if(-1 == err) {
        if(ENOENT == errno) {
            // does not exist
            return 0;
        } else {
            // perror("stat");
            return 0;
        }
    } else {
        if(S_ISDIR(s.st_mode)) {
            // it's a dir
            return 1;
        } else {
            // it's a file
            return 2;
        }
    }
    return 0;
}

static void utils_strncpy(char* dest, const char* src, size_t n) {
    if (dest && n > 0) {
        // Use strncat for performance because strncpy will always fill n bytes in dest
        dest[0] = '\0';            // Let dest be an empty string
        strncat(dest, src, --n);   // n-1 because strncat may fill n+1 bytes
    }
}

// -1 mean fail or clientfd is returned
static int sock_udp_client_create_local(bool is_abstract, const char* name) {
    int fd;
    int size;
    struct sockaddr_un addr;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    size = strlen(name) + offsetof(struct sockaddr_un, sun_path) + 1;
    if(is_abstract) {
        addr.sun_path[0] = 0;
        memcpy(addr.sun_path + 1, name, strlen(name));
    } else {
        utils_strncpy(addr.sun_path, name, sizeof(addr.sun_path));
    }
    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(fd == -1) {
        LOGE("socket() failed, reason=[%s]%d", strerror(errno), errno);
        return -1;
    }
    if(connect(fd, (struct sockaddr *)&addr, size) == -1) {
        LOGE("connect() failed, abstract=%d name=[%s] reason=[%s]%d",
            is_abstract, name, strerror(errno), errno);
        close(fd);
        return -1;
    }
    return fd;
}

// -1 mean fail or return the number of byte are written
static int fd_write(int fd, const void* buff, uint32_t len) {
    int ret;
    if(buff == NULL) {
        LOGE("buff is NULL");
        return -1;
    }
    if(len == 0) {
        LOGE("fd_write() len=%d is invalid", len);
        return -1;
    }
    ret = write(fd, buff, len);
    if(ret == -1) {
        LOGE("fd_write() write() failed, fd=%d len=%d reason=[%s]%d",
            fd, len, strerror(errno), errno);
        return -1;
    }
    return ret;
}

// -1 mean fail or serverfd is returned
static int sock_udp_server_bind_local(bool is_abstract, const char* name) {
    int fd;
    int size;
    struct sockaddr_un addr;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    size = strlen(name) + offsetof(struct sockaddr_un, sun_path) + 1;
    if(is_abstract) {
        addr.sun_path[0] = 0;
        memcpy(addr.sun_path + 1, name, strlen(name));
    } else {
        utils_strncpy(addr.sun_path, name, sizeof(addr.sun_path));
        if(unlink(addr.sun_path) == -1) {
            LOGE("unlink() failed, reason=[%s]%d", strerror(errno), errno);
        }
    }
    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd == -1) {
        LOGE("socket() failed, reason=[%s]%d", strerror(errno), errno);
        return -1;
    }
    if (bind(fd, (struct sockaddr*)&addr, size) == -1) {
        LOGE("bind() failed, abstract=%d name=[%s] reason=[%s]%d",
            is_abstract, name, strerror(errno), errno);
        close(fd);
        return -1;
    }
    return fd;
}

// -1 means failure
static int send2agps(const char* buff, int len) {
    int ret = 0;
    //Android P0, /data folder cannot be used anymore, use the abstract socket instead
    if(is_path_exist(MNL_TO_AGPS) == 2) {
        int fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
        if(fd == -1) {
            return -1;
        }

        ret = safe_sendto(fd, g_mnl2agps_path, buff, len);
        close(fd);
    } else {
        int fd = sock_udp_client_create_local(true, "mtk_mnl2agps");
        if(fd == -1) {
            return -1;
        }
        ret = fd_write(fd, buff, len);
        close(fd);
    }
    return ret;
}


static int bind_udp_socket(char* path) {
    int fd;
    struct sockaddr_un addr;
    socklen_t addr_len;

    fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (fd < 0) {
        LOGE("socket_bind_udp() socket() failed reason=[%s]%d",
            strerror(errno), errno);
        return -1;
    }
    MNLD_STRNCPY(addr.sun_path, path,sizeof(addr.sun_path));
    addr.sun_family = AF_UNIX;
    addr_len = (offsetof(struct sockaddr_un, sun_path) + strlen(addr.sun_path) + 1);
    unlink(addr.sun_path);
    if (bind(fd, (struct sockaddr *)&addr, addr_len) < 0) {
        LOGE("bind failed path=[%s] reason=[%s]\n", path, strerror(errno));
        close(fd);
        return -1;
    }
    if (chmod(path, 0660) < 0) {
        LOGE("chmod err = [%s]\n", strerror(errno));
    }

    return fd;
}

int mnl2agps_mnl_reboot() {
    LOGD("mnl2agps_mnl_reboot");
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_MNL_REBOOT);

    return send2agps(buff, offset);
}

int mnl2agps_open_gps_done() {
    LOGD("mnl2agps_open_gps_done");
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_AGPS_OPEN_GPS_DONE);

    return send2agps(buff, offset);
}
int mnl2agps_close_gps_done() {
    LOGD("mnl2agps_close_gps_done");
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_AGPS_CLOSE_GPS_DONE);

    return send2agps(buff, offset);
}
int mnl2agps_reset_gps_done() {
    LOGD("mnl2agps_reset_gps_done");
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_AGPS_RESET_GPS_DONE);

    return send2agps(buff, offset);
}

int mnl2agps_gps_init() {
    LOGD("mnl2agps_gps_init");
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_GPS_INIT);

    return send2agps(buff, offset);
}
int mnl2agps_gps_cleanup() {
    LOGD("mnl2agps_gps_cleanup");
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_GPS_CLEANUP);

    return send2agps(buff, offset);
}
// type:AGpsType
int mnl2agps_set_server(int type, const char* hostname, int port) {
    //LOGD("mnl2agps_set_server, hostname = %s, port = %d\n", hostname, port);
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_SET_SERVER);
    put_int(buff, &offset, type);
    put_string(buff, &offset, hostname);
    put_int(buff, &offset, port);

    return send2agps(buff, offset);
}
// flags:GpsAidingData
int mnl2agps_delete_aiding_data(int flags) {
    //LOGD("mnl2agps_delete_aiding_data");
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_DELETE_AIDING_DATA);
    put_int(buff, &offset, flags);

    return send2agps(buff, offset);
}
int mnl2agps_gps_open(int assist_req) {
    LOGD("mnl2agps_gps_open");
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_GPS_OPEN);
    put_int(buff, &offset, assist_req);

    return send2agps(buff, offset);
}
int mnl2agps_gps_close() {
    LOGD("mnl2agps_gps_close");
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_GPS_CLOSE);

    return send2agps(buff, offset);
}
int mnl2agps_data_conn_open(const char* apn) {
    //LOGD("mnl2agps_data_conn_open, apn = %s\n", apn);
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_DATA_CONN_OPEN);
    put_string(buff, &offset, apn);

    return send2agps(buff, offset);
}

int mnl2agps_data_conn_open_ip_type(const char* apn, int ip_type,
        bool network_handle_valid, uint64_t network_handle) {
    LOGD("mnl2agps_data_conn_open  apn=%s ip_type=%d network_handle=[%llu]%d\n",
            apn, ip_type, network_handle, network_handle_valid);

    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_DATA_CONN_OPEN_IP_TYPE);
    put_string(buff, &offset, apn);
    put_int(buff, &offset, ip_type);

    put_byte(buff, &offset, network_handle_valid);
    put_long(buff, &offset, network_handle);

    return send2agps(buff, offset);
}

int mnl2agps_install_certificates(int index, int total, const char* data, int len) {
    LOGD("mnl2agps_install_certificates  (%d/%d) len=%d\n", index, total, len);

    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_INSTALL_CERTIFICATES);
    put_int(buff, &offset, index);
    put_int(buff, &offset, total);
    put_binary(buff, &offset, data, len);

    return send2agps(buff, offset);
}

int mnl2agps_revoke_certificates(const char* data, int len) {
    LOGD("mnl2agps_revoke_certificates  len=%d item=%d\n", len, len/20);

    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_REVOKE_CERTIFICATES);
    put_binary(buff, &offset, data, len);

    return send2agps(buff, offset);
}

int mnl2agps_update_gnss_access_control(bool allowed) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("mnl2agps_update_gnss_access_control allowed=%d\n", allowed);
    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_UPDATE_GNSS_ACCESS_CONTROL);
    put_byte(buff, &offset, allowed);

    return send2agps(buff, offset);
}

int mnl2agps_data_conn_failed() {
    LOGD("mnl2agps_data_conn_failed");
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_DATA_CONN_FAILED);

    return send2agps(buff, offset);
}
int mnl2agps_data_conn_closed() {
    //LOGD("mnl2agps_data_conn_closed");
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_DATA_CONN_CLOSED);

    return send2agps(buff, offset);
}
int mnl2agps_ni_message(const char* msg, int len) {
    //LOGD("mnl2agps_ni_message");
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_NI_MESSAGE);
    put_binary(buff, &offset, msg, len);

    return send2agps(buff, offset);
}

/*
ACCEPT = 1
DENY = 2
NO_RSP = 3
*/
int mnl2agps_ni_respond(int session_id, int user_response) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    //LOGD("session_id: %d, user_response: %d\n", session_id, user_response);
    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_NI_RESPOND);
    put_int(buff, &offset, session_id);
    put_int(buff, &offset, user_response);

    return send2agps(buff, offset);
}
int mnl2agps_set_ref_loc(int type, int mcc, int mnc, int lac, int cid) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    //LOGD("type=%d, mcc=%d, mnc=%d, lac=%d, cid=%d\n", type, mcc, mnc, lac, cid);

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_SET_REF_LOC);
    put_int(buff, &offset, type);
    put_int(buff, &offset, mcc);
    put_int(buff, &offset, mnc);
    put_int(buff, &offset, lac);
    put_int(buff, &offset, cid);

    return send2agps(buff, offset);
}
int mnl2agps_set_set_id(int type, const char* setid) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    //LOGD("type=%d, setid = %s\n", type, setid);
    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_SET_SET_ID);
    put_int(buff, &offset, type);
    put_string(buff, &offset, setid);

    return send2agps(buff, offset);
}
int mnl2agps_update_network_state(int connected, int type, int roaming, const char* extra_info) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    //LOGD("connected=%d, type = %d, roaming=%d, extra_info=%s\n", connected, type, roaming, extra_info);
    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_UPDATE_NETWORK_STATE);
    put_int(buff, &offset, connected);
    put_int(buff, &offset, type);
    put_int(buff, &offset, roaming);
    put_string(buff, &offset, extra_info);

    return send2agps(buff, offset);
}

int mnl2agps_update_network_availability(int avaiable, const char* apn) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    //LOGD("avaiable=%d, apn=%s\n", avaiable, apn);
    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_UPDATE_NETWORK_AVAILABILITY);
    put_int(buff, &offset, avaiable);
    put_string(buff, &offset, apn);

    return send2agps(buff, offset);
}

int mnl2agps_pmtk(const char* pmtk) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_MNL2AGPS_PMTK);
    put_string(buff, &offset, pmtk);

    return send2agps(buff, offset);
}
int mnl2agps_raw_dbg(int enabled) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_RAW_DBG);
    put_int(buff, &offset, enabled);

    return send2agps(buff, offset);
}

int mnl2agps_reaiding_req() {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_REAIDING);

    return send2agps(buff, offset);
}

int mnl2agps_location_sync(double lat, double lng, int acc, bool alt_valid, float alt, bool source_valid, bool source_gnss, bool source_nlp, bool source_sensor) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_LOCATION_SYNC);
    put_double(buff, &offset, lat);
    put_double(buff, &offset, lng);
    put_int(buff, &offset, acc);
    put_byte(buff, &offset, alt_valid);
    put_float(buff, &offset, alt);
    put_byte(buff, &offset, source_valid);
    put_byte(buff, &offset, source_gnss);
    put_byte(buff, &offset, source_nlp);
    put_byte(buff, &offset, source_sensor);

    //LOGW("LPPe debug:mnl2agps_location_sync: lat:%.3f, lng:%.3f, acc:%d, alt_valid:%d, alt:%.2f, source_valid:%d, gnss:%d, nlp:%d, sensor:%d", \
    //    lat, lng, acc, alt_valid, alt, source_valid, source_gnss, source_nlp, source_sensor);
    return send2agps(buff, offset);
}
int mnl2agps_agps_settings_ack(mnl_agps_gnss_settings* settings) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_SETTINGS_ACK);
    put_int(buff, &offset, settings->gps_satellite_support);
    put_int(buff, &offset, settings->glonass_satellite_support);
    put_int(buff, &offset, settings->beidou_satellite_support);
    put_int(buff, &offset, settings->galileo_satellite_support);

    return send2agps(buff, offset);
}

int mnl2agps_vzw_debug_screen_enable(int enabled) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_VZW_DEBUG_SCREEN_ENABLE);
    put_int(buff, &offset, enabled);

    return send2agps(buff, offset);
}
int mnl2agps_lppe_assist_data_req(const char* data, int len) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_LPPE_ASSIST_DATA_REQ);
    put_binary(buff, &offset, data, len);   // refer to gnss_ha_assist_data_req_struct

    return send2agps(buff, offset);
}

int mnl2agps_lppe_assist_data_provide_ack(const char* data, int len) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_LPPE_ASSIST_DATA_PROVIDE_ACK);
    put_binary(buff, &offset, data, len);   // refer to gnss_ha_assist_ack_struct

    return send2agps(buff, offset);
}
void mnl2fake_agpsd_hdlr(int fd) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    int ret;

    ret = safe_recvfrom(fd, buff, sizeof(buff));
    if (ret <= 0) {
        LOGE("mnl2agps_handler() safe_recvfrom() failed ret=%d", ret);
        return;
    }
    int version = get_int(buff, &offset, sizeof(buff));

    if (version != MNL_AGPS_INTERFACE_VERSION) {
        LOGE("agps_ver=%d mnl_ver=%d\n",
            version, MNL_AGPS_INTERFACE_VERSION);
    }
    mnl_agps_type type = get_int(buff, &offset, sizeof(buff));
    LOGE("*#*#type:%d\n",type);
}

int mnl2agps_update_configuration(const char* config_data, int length) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_UPDATE_CONFIGURATION);
    put_string(buff, &offset, config_data);
    put_int(buff, &offset, length);

    return send2agps(buff, offset);
}

int mnl2agps_set_position_mode(int pos_mode) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    LOGD_ENG("mnl2agps_set_position_mode, pos_mode = %d", pos_mode);

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_SET_POSITION_MODE);
    put_int(buff, &offset, pos_mode);

    return send2agps(buff, offset);
}

int mnl2agps_md_time_sync_req(const char* data, int len) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_MD_TIME_SYNC_REQ);
    put_binary(buff, &offset, data, len);   // refer to mnl_md_time_sync_req_struct

    return send2agps(buff, offset);
}

int mnl2agps_md_time_sync_rsp(const char* data, int len) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_MD_TIME_SYNC_RSP);
    put_binary(buff, &offset, data, len);   // refer to mnl_md_time_sync_rsp_struct

    return send2agps(buff, offset);
}

int mnl2agps_md_time_info_rsp(const char* data, int len) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_MD_TIME_INFO_RSP);
    put_binary(buff, &offset, data, len);   // refer to mnl_md_time_info_rsp_struct

    return send2agps(buff, offset);
}

int mnl2agps_update_network_state_with_handle(uint64_t network_handle,
            bool is_connected, unsigned short capabilities, const char* apn) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("update network with handle=%llu connected=%d, capabilities = %d, apn=%s\n",
            network_handle, is_connected, capabilities, apn);
    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_UPDATE_NETWORK_STATE_WITH_HANDLE);
    put_long(buff, &offset, network_handle);
    put_byte(buff, &offset, is_connected);
    put_short(buff, &offset, capabilities);
    put_string(buff, &offset, apn);

    return send2agps(buff, offset);
}

void agps2mnl_hdlr(int fd, agps2mnl_interface* mnl_interface) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    int ret;

    ret = safe_recvfrom(fd, buff, sizeof(buff));
    if (ret <= 0) {
        LOGE("mnl2agps_handler() safe_recvfrom() failed ret=%d", ret);
        return;
    }
    int version = get_int(buff, &offset, sizeof(buff));

    if (version != MNL_AGPS_INTERFACE_VERSION) {
        LOGE("agps_ver=%d mnl_ver=%d\n",
            version, MNL_AGPS_INTERFACE_VERSION);
    }

    mnl_agps_type type = get_int(buff, &offset, sizeof(buff));
    // LOGD("agps2mnl [%s]\n", get_mnl_agps_type_str(type));

    switch (type) {
    case MNL_AGPS_TYPE_AGPS_REBOOT: {
        if (mnl_interface->agps_reboot) {
            mnl_interface->agps_reboot();
        } else {
            LOGE("agps_reboot is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_AGPS_OPEN_GPS_REQ: {
        int show_gps_icon = get_int(buff, &offset, sizeof(buff));
        mnl_agps_open_type open_type = get_int(buff, &offset, sizeof(buff));
        mnl_agps_open_requestor requestor_type = get_int(buff, &offset, sizeof(buff));
        const char* requestor_id = get_string(buff, &offset, sizeof(buff));
        bool emergency_call = get_byte(buff, &offset, sizeof(buff));
        if (mnl_interface->agps_open_gps_req) {
            mnl_interface->agps_open_gps_req(show_gps_icon, open_type,
                    requestor_type, requestor_id, emergency_call);
        } else {
            LOGE("agps_open_gps_req is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_AGPS_CLOSE_GPS_REQ: {
        if (mnl_interface->agps_close_gps_req) {
            mnl_interface->agps_close_gps_req();
        } else {
            LOGE("agps_close_gps_req is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_AGPS_RESET_GPS_REQ: {
        int flags = get_int(buff, &offset, sizeof(buff));
        if (mnl_interface->agps_reset_gps_req) {
            mnl_interface->agps_reset_gps_req(flags);
        } else {
            LOGE("agps_reset_gps_req is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_AGPS_OPEN_GPS_REJECTED: {
        mnl_agps_open_type open_type = get_int(buff, &offset, sizeof(buff));
        mnl_agps_open_requestor requestor_type = get_int(buff, &offset, sizeof(buff));
        const char* requestor_id = get_string(buff, &offset, sizeof(buff));
        bool emergency_call = get_byte(buff, &offset, sizeof(buff));
        if(mnl_interface->agps_open_gps_rejected) {
            mnl_interface->agps_open_gps_rejected(open_type, requestor_type, requestor_id, emergency_call);
        } else {
            LOGE("agps_open_gps_rejected is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_AGPS_SESSION_DONE: {
        if (mnl_interface->agps_session_done) {
            mnl_interface->agps_session_done();
        } else {
            LOGE("agps_session_done is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_NI_NOTIFY: {
        int session_id = get_int(buff, &offset, sizeof(buff));
        mnl_agps_notify_type type = get_int(buff, &offset, sizeof(buff));
        const char* requestor_id = get_string(buff, &offset, sizeof(buff));
        const char* client_name = get_string(buff, &offset, sizeof(buff));
        mnl_agps_ni_type ni_type = get_int(buff, &offset, sizeof(buff));
        if (mnl_interface->ni_notify) {
            mnl_interface->ni_notify(session_id, ni_type, type, requestor_id, client_name);
        } else {
            LOGE("ni_notify is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_NI_NOTIFY_2: {
        int session_id = get_int(buff, &offset, sizeof(buff));
        mnl_agps_notify_type type = get_int(buff, &offset, sizeof(buff));
        const char* requestor_id = get_string(buff, &offset, sizeof(buff));
        const char* client_name = get_string(buff, &offset, sizeof(buff));
        int requestor_id_encoding = get_int(buff, &offset, sizeof(buff));
        int client_name_encoding = get_int(buff, &offset, sizeof(buff));
        mnl_agps_ni_type ni_type = get_int(buff, &offset, sizeof(buff));
        if (mnl_interface->ni_notify2) {
            mnl_interface->ni_notify2(session_id, ni_type, type, requestor_id,
               client_name, requestor_id_encoding, client_name_encoding);
        } else {
            LOGE("ni_notify2 is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_DATA_CONN_REQ: {
        int ipaddr = get_int(buff, &offset, sizeof(buff));
        int is_emergency = get_int(buff, &offset, sizeof(buff));
        if (mnl_interface->data_conn_req) {
            mnl_interface->data_conn_req(ipaddr, is_emergency);
        } else {
            LOGE("data_conn_req is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_DATA_CONN_REQ2: {
        struct sockaddr_storage addr;
        get_binary(buff, &offset, (char*)&addr, sizeof(buff), sizeof(addr));
        int is_emergency = get_int(buff, &offset, sizeof(buff));
        mnl_agps_data_connection_type agps_type = get_int(buff, &offset, sizeof(buff));
        if (mnl_interface->data_conn_req2) {
            mnl_interface->data_conn_req2(&addr, is_emergency, agps_type);
        } else {
            LOGE("data_conn_req2 is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_DATA_CONN_RELEASE: {
        mnl_agps_data_connection_type agps_type = get_int(buff, &offset, sizeof(buff));
        if (mnl_interface->data_conn_release) {
            mnl_interface->data_conn_release(agps_type);
        } else {
            LOGE("data_conn_release is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_SET_ID_REQ: {
        int flags = get_int(buff, &offset, sizeof(buff));
        if (mnl_interface->set_id_req) {
            mnl_interface->set_id_req(flags);
        } else {
            LOGE("set_id_req is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_REF_LOC_REQ: {
        int flags = get_int(buff, &offset, sizeof(buff));
        if (mnl_interface->ref_loc_req) {
            mnl_interface->ref_loc_req(flags);
        } else {
            LOGE("ref_loc_req is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_AGPS2MNL_PMTK: {
        const char* pmtk = get_string(buff, &offset, sizeof(buff));
        if (mnl_interface->rcv_pmtk) {
            mnl_interface->rcv_pmtk(pmtk);
        } else {
            LOGE("rcv_pmtk is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_GPEVT: {
        gpevt_type type = get_int(buff, &offset, sizeof(buff));
        if (mnl_interface->gpevt) {
            mnl_interface->gpevt(type);
        } else {
            LOGE("gpevt is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_AGPS_LOC: {
        mnl_agps_agps_location agps_location;
        get_binary(buff, &offset, (char*)&agps_location, sizeof(buff), sizeof(mnl_agps_agps_location));
        if (mnl_interface->agps_location) {
            mnl_interface->agps_location(&agps_location);
        } else {
            LOGE("agps_location is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_SETTINGS_SYNC: {
        mnl_agps_agps_settings agps_settings;
        agps_settings.sib8_16_enable = get_int(buff, &offset, sizeof(buff));
        agps_settings.gps_satellite_enable = get_int(buff, &offset, sizeof(buff));
        agps_settings.glonass_satellite_enable = get_int(buff, &offset, sizeof(buff));
        agps_settings.beidou_satellite_enable = get_int(buff, &offset, sizeof(buff));
        agps_settings.galileo_satellite_enable = get_int(buff, &offset, sizeof(buff));
        agps_settings.a_glonass_satellite_enable = get_int(buff, &offset, sizeof(buff));
        agps_settings.a_gps_satellite_enable = get_int(buff, &offset, sizeof(buff));
        agps_settings.a_beidou_satellite_enable = get_int(buff, &offset, sizeof(buff));
        agps_settings.a_galileo_satellite_enable = get_int(buff, &offset, sizeof(buff));
        agps_settings.lppe_enable = get_int(buff, &offset, sizeof(buff));
        if (mnl_interface->agps_settings_sync) {
            mnl_interface->agps_settings_sync(&agps_settings);
        } else {
            LOGE("agps_settings_sync is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_VZW_DEBUG_SCREEN_OUTPUT: {
        const char* str = get_string(buff, &offset, sizeof(buff));
        if (mnl_interface->vzw_debug_screen_output) {
            mnl_interface->vzw_debug_screen_output(str);
        } else {
            LOGE("vzw_debug_screen_output is NULL\n");
        }
        break;
    }

    case MNL_AGPS_TYPE_LPPE_ASSIST_COMMON_IONO: {
        if(mnl_interface->lppe_assist_common_iono) {
        int lppe_len=get_int(buff, &offset, sizeof(buff));
        LOGE("mnld rec lppe data ,len=%d,offset=%d\n",lppe_len,offset);
            mnl_interface->lppe_assist_common_iono(buff+offset, lppe_len);
        } else {
            LOGE("lppe_assist_data_provide_common_iono is NULL\n");
        }
        break;
    }
        case MNL_AGPS_TYPE_LPPE_ASSIST_COMMON_TROP: {
        if(mnl_interface->lppe_assist_common_trop) {
        int lppe_len=get_int(buff, &offset, sizeof(buff));
            mnl_interface->lppe_assist_common_trop(buff+offset, lppe_len);
        } else {
            LOGE("lppe_assist_data_provide_common_trop is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_LPPE_ASSIST_COMMON_ALT: {
        if(mnl_interface->lppe_assist_common_alt) {
        int lppe_len=get_int(buff, &offset, sizeof(buff));
            mnl_interface->lppe_assist_common_alt(buff+offset, lppe_len);
        } else {
            LOGE("lppe_assist_data_provide_common_alt is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_LPPE_ASSIST_COMMON_SOLAR: {
        if(mnl_interface->lppe_assist_common_solar) {
        int lppe_len=get_int(buff, &offset, sizeof(buff));
            mnl_interface->lppe_assist_common_solar(buff+offset, lppe_len);
        } else {
            LOGE("lppe_assist_data_provide_common_solar is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_LPPE_ASSIST_COMMON_CCP: {
        if(mnl_interface->lppe_assist_common_ccp) {
        int lppe_len=get_int(buff, &offset, sizeof(buff));
            mnl_interface->lppe_assist_common_ccp(buff+offset, lppe_len);
        } else {
            LOGE("lppe_assist_data_provide_common_ccp is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_LPPE_ASSIST_GENERIC_CCP: {
        if(mnl_interface->lppe_assist_generic_ccp) {
        int lppe_len=get_int(buff, &offset, sizeof(buff));
            mnl_interface->lppe_assist_generic_ccp(buff+offset, lppe_len);
        } else {
            LOGE("lppe_assist_data_provide_generic_ccp is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_LPPE_ASSIST_GENERIC_DM: {
        if(mnl_interface->lppe_assist_generic_dm) {
        int lppe_len=get_int(buff, &offset, sizeof(buff));
            mnl_interface->lppe_assist_generic_dm(buff+offset, lppe_len);
        } else {
            LOGE("lppe_assist_data_provide_generic_dm is NULL\n");
        }
        break;
    }
    default:
        LOGE("agps2mnl unknown type=%d\n", type);
        break;
    }
}

int create_agps2mnl_fd() {
    int fd = bind_udp_socket(AGPS_TO_MNL);
    if(fd == -1) {
        //Android P0, /data folder cannot be used anymore, use the abstract socket instead
        LOGD("it's a Android P version, bind mtk_agps2mnl instead");
        fd = sock_udp_server_bind_local(true, "mtk_agps2mnl");
        if(fd == -1) {
            return -1;
        }
    }
    set_socket_blocking(fd, 0);
    return fd;
}

