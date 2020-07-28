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
#include <cutils/properties.h>

#if defined(__TIZEN_OS__)
#include <dlog/dlog.h>
#endif

#include "agps2mnl_interface.h"
#include "data_coder.h"

#ifndef UNUSED
#define UNUSED(x) (x)=(x)
#endif
#define MNL_TO_AGPS "/data/agps_supl/mnl_to_agps"

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
#define LOGD(...) tag_log(1, "[agps][n][AGPS] [MNL]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[agps] WARNING: [MNL]", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[agps] ERR: [MNL]", __VA_ARGS__);
#else
#define LOG_TAG "agps"
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#endif

#define HIDE_LOG_PROP "ro.vendor.mtk_log_hide_gps"
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

static char g_agps2mnl_path[128] = AGPS_TO_MNL;

// -1 means failure
static int send2mnl(const char* buff, int len) {
    int ret = 0;
    //Android P0, /data folder cannot be used anymore, use the abstract socket instead
    if(is_path_exist(AGPS_TO_MNL) == 2) {
        int fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
        if(fd == -1) {
            return -1;
        }
        ret = safe_sendto(fd, g_agps2mnl_path, buff, len);
        close(fd);
    } else {
        int fd = sock_udp_client_create_local(true, "mtk_agps2mnl");
        if(fd == -1) {
            return -1;
        }
        ret = fd_write(fd, buff, len);
        close(fd);
    }
    return ret;
}

static int bind_udp_socket(char* path) {
    int sockfd;
    struct sockaddr_un soc_addr;
    socklen_t addr_len;

    sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        LOGE("socket failed reason=[%s]\n", strerror(errno));
        return -1;
    }

    MNLD_STRNCPY(soc_addr.sun_path, path,sizeof(soc_addr.sun_path));
    soc_addr.sun_family = AF_UNIX;
    addr_len = (offsetof(struct sockaddr_un, sun_path) + strlen(soc_addr.sun_path) + 1);

    unlink(soc_addr.sun_path);
    if (bind(sockfd, (struct sockaddr *)&soc_addr, addr_len) < 0) {
        LOGE("bind failed path=[%s] reason=[%s]\n", path, strerror(errno));
        close(sockfd);
        return -1;
    }

    if (chmod(path, 0660) < 0) {
        LOGE("chmod err = [%s]\n", strerror(errno));
    }
    return sockfd;
}

int agps2mnl_agps_reboot() {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("write  agps_reboot\n");

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_AGPS_REBOOT);

    return send2mnl(buff, offset);
}

int agps2mnl_agps_open_gps_req(int show_gps_icon, mnl_agps_open_type open_type,
        mnl_agps_open_requestor requestor_type, const char* requestor_id,
        bool emergency_call) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("write  agps_open_gps_req  show_gps_icon=%d open_type=%d requestor_type=%d requestor_id=%s emergency_call=%d\n",
            show_gps_icon, open_type, requestor_type, requestor_id, emergency_call);

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_AGPS_OPEN_GPS_REQ);
    put_int(buff, &offset, show_gps_icon);
    put_int(buff, &offset, open_type);
    put_int(buff, &offset, requestor_type);
    put_string(buff, &offset, requestor_id);
    put_byte(buff, &offset, emergency_call);

    return send2mnl(buff, offset);
}
int agps2mnl_agps_close_gps_req() {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("write  agps_close_gps_req\n");

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_AGPS_CLOSE_GPS_REQ);

    return send2mnl(buff, offset);
}
int agps2mnl_agps_reset_gps_req(int flags) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("write  agps_reset_gps_req  flags=0x%x\n", flags);

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_AGPS_RESET_GPS_REQ);
    put_int(buff, &offset, flags);

    return send2mnl(buff, offset);
}
int agps2mnl_agps_open_gps_rejected(mnl_agps_open_type open_type,
        mnl_agps_open_requestor requestor_type, const char* requestor_id,
        bool emergency_call) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("write  agps_open_gps_rejected  open_type=%d requestor_type=%d requestor_id=%s emergency_call=%d\n",
            open_type, requestor_type, requestor_id, emergency_call);

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_AGPS_OPEN_GPS_REJECTED);
    put_int(buff, &offset, open_type);
    put_int(buff, &offset, requestor_type);
    put_string(buff, &offset, requestor_id);
    put_byte(buff, &offset, emergency_call);

    return send2mnl(buff, offset);
}

int agps2mnl_agps_session_done() {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("write  agps_session_done\n");

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_AGPS_SESSION_DONE);

    return send2mnl(buff, offset);
}

int agps2mnl_ni_notify(int session_id, mnl_agps_ni_type ni_type, mnl_agps_notify_type type,
        const char* requestor_id, const char* client_name) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("write  ni_notify  session_id=%d ni_type=%d type=%d requestor_id=[%s] client_name=[%s]\n",
        session_id, ni_type, type, requestor_id, client_name);
    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_NI_NOTIFY);
    put_int(buff, &offset, session_id);
    put_int(buff, &offset, type);
    put_string(buff, &offset, requestor_id);
    put_string(buff, &offset, client_name);
    put_int(buff, &offset, ni_type);

    return send2mnl(buff, offset);
}

int agps2mnl_ni_notify2(int session_id, mnl_agps_ni_type ni_type, mnl_agps_notify_type type,
        const char* requestor_id, const char* client_name,
        mnl_agps_ni_encoding_type requestor_id_encoding,
        mnl_agps_ni_encoding_type client_name_encoding) {

    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("write  ni_notify2  session_id=%d ni_type=%d type=%d requestor_id=[%s] type=%d client_name=[%s] type=%d\n",
        session_id, ni_type, type, requestor_id, requestor_id_encoding, client_name, client_name_encoding);

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_NI_NOTIFY_2);
    put_int(buff, &offset, session_id);
    put_int(buff, &offset, type);
    put_string(buff, &offset, requestor_id);
    put_string(buff, &offset, client_name);
    put_int(buff, &offset, requestor_id_encoding);
    put_int(buff, &offset, client_name_encoding);
    put_int(buff, &offset, ni_type);

    return send2mnl(buff, offset);
}

int agps2mnl_data_conn_req(int ipaddr, int is_emergency) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("write  data_conn_req  ipaddr=0x%x is_emergency=%d\n", ipaddr, is_emergency);

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_DATA_CONN_REQ);
    put_int(buff, &offset, ipaddr);
    put_int(buff, &offset, is_emergency);

    return send2mnl(buff, offset);
}

int agps2mnl_data_conn_req2(struct sockaddr_storage* addr, int is_emergency,
        mnl_agps_data_connection_type agps_type) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("write  data_conn_req2  is_emergency=%d\n", is_emergency);

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_DATA_CONN_REQ2);
    put_binary(buff, &offset, (const char*)addr, sizeof(*addr));
    put_int(buff, &offset, is_emergency);
    put_int(buff, &offset, agps_type);

    return send2mnl(buff, offset);
}

int agps2mnl_data_conn_release(mnl_agps_data_connection_type agps_type) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("write  data_conn_release\n");

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_DATA_CONN_RELEASE);
    put_int(buff, &offset, agps_type);

    return send2mnl(buff, offset);
}

int agps2mnl_set_id_req(int flags) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("write  set_id_req  flags=0x%x\n", flags);

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_SET_ID_REQ);
    put_int(buff, &offset, flags);

    return send2mnl(buff, offset);
}

int agps2mnl_ref_loc_req(int flags) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("write  ref_loc_req  flags=0x%x\n", flags);

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_REF_LOC_REQ);
    put_int(buff, &offset, flags);

    return send2mnl(buff, offset);
}

int agps2mnl_pmtk(const char* pmtk) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("write  pmtk  [%s]\n", pmtk);

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_AGPS2MNL_PMTK);
    put_string(buff, &offset, pmtk);

    return send2mnl(buff, offset);
}
int agps2mnl_gpevt(gpevt_type type) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    LOGD("write  gpevt  type=%d\n", type);

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_GPEVT);
    put_int(buff, &offset, type);

    return send2mnl(buff, offset);
}

int agps2mnl_agps_location(mnl_agps_agps_location* location) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    char result[PROPERTY_VALUE_MAX] = {0};

    property_get(HIDE_LOG_PROP, result, NULL);
    if (result[0] != '1') {
        LOGD("write  agps_location  lat=%f lng=%f\n", location->latitude, location->longitude);
    }

    put_int(buff, &offset, MNL_AGPS_INTERFACE_VERSION);
    put_int(buff, &offset, MNL_AGPS_TYPE_AGPS_LOC);
    put_binary(buff, &offset, (const char*)location, sizeof(mnl_agps_agps_location));

    return send2mnl(buff, offset);
}

void mnl2agps_hdlr(int fd, mnl2agps_interface* agps_interface) {
    char buff[MNL_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    int ret;

    ret = safe_recvfrom(fd, buff, sizeof(buff));
    if (ret <= 0) {
        LOGE("agps2mnl_handler() safe_recvfrom() failed ret=%d", ret);
        return;
    }

    int version = get_int(buff, &offset, sizeof(buff));

    if (version != MNL_AGPS_INTERFACE_VERSION) {
        LOGE("mnl_ver=%d agps_ver=%d\n",
            version, MNL_AGPS_INTERFACE_VERSION);
    }

    mnl_agps_type type = get_int(buff, &offset, sizeof(buff));

    switch (type) {
    case MNL_AGPS_TYPE_MNL_REBOOT: {
        if (agps_interface->mnl_reboot) {
           agps_interface->mnl_reboot();
        } else {
            LOGE("mnl_reboot is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_AGPS_OPEN_GPS_DONE: {
        if (agps_interface->open_gps_done) {
           agps_interface->open_gps_done();
        } else {
            LOGE("open_gps_done is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_AGPS_CLOSE_GPS_DONE: {
        if (agps_interface->close_gps_done) {
           agps_interface->close_gps_done();
        } else {
            LOGE("close_gps_done is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_AGPS_RESET_GPS_DONE: {
        if (agps_interface->reset_gps_done) {
           agps_interface->reset_gps_done();
        } else {
            LOGE("reset_gps_done is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_GPS_INIT: {
        if (agps_interface->gps_init) {
           agps_interface->gps_init();
        } else {
            LOGE("gps_init is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_GPS_CLEANUP: {
        if (agps_interface->gps_cleanup) {
           agps_interface->gps_cleanup();
        } else {
            LOGE("gps_cleanup is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_SET_SERVER: {
        int type = get_int(buff, &offset, sizeof(buff));
        const char* hostname = get_string(buff, &offset, sizeof(buff));
        int port = get_int(buff, &offset, sizeof(buff));
        if (agps_interface->set_server) {
           agps_interface->set_server(type, hostname, port);
        } else {
            LOGE("set_server is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_DELETE_AIDING_DATA: {
        int flags = get_int(buff, &offset, sizeof(buff));
        if (agps_interface->delete_aiding_data) {
           agps_interface->delete_aiding_data(flags);
        } else {
            LOGE("delete_aiding_data is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_GPS_OPEN: {
        int assist_req = get_int(buff, &offset, sizeof(buff));
        if (agps_interface->gps_open) {
           agps_interface->gps_open(assist_req);
        } else {
            LOGE("gps_open is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_GPS_CLOSE: {
        if (agps_interface->gps_close) {
           agps_interface->gps_close();
        } else {
            LOGE("gps_close is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_DATA_CONN_OPEN: {
        const char* apn = get_string(buff, &offset, sizeof(buff));
        if (agps_interface->data_conn_open) {
           agps_interface->data_conn_open(apn);
        } else {
            LOGE("data_conn_open is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_DATA_CONN_OPEN_IP_TYPE: {
        const char* apn = get_string(buff, &offset, sizeof(buff));
        int ip_type = get_int(buff, &offset, sizeof(buff));
        bool network_handle_valid = get_byte(buff, &offset, sizeof(buff));
        uint64_t network_handle = get_long(buff, &offset, sizeof(buff));
        if (agps_interface->data_conn_open_ip_type) {
           agps_interface->data_conn_open_ip_type(apn, ip_type,
                    network_handle_valid, network_handle);
        } else {
            LOGE("data_conn_open_ip_type is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_INSTALL_CERTIFICATES: {
        int index = get_int(buff, &offset, sizeof(buff));
        int total = get_int(buff, &offset, sizeof(buff));
        char data[MNL_AGPS_MAX_BUFF_SIZE] = {0};
        int len = get_binary(buff, &offset, data, sizeof(buff), sizeof(data));
        if (agps_interface->install_certificates) {
           agps_interface->install_certificates(index, total, data, len);
        } else {
            LOGE("install_certificates is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_REVOKE_CERTIFICATES: {
        char data[MNL_AGPS_MAX_BUFF_SIZE] = {0};
        int len = get_binary(buff, &offset, data, sizeof(buff), sizeof(data));
        if (agps_interface->revoke_certificates) {
           agps_interface->revoke_certificates(data, len);
        } else {
            LOGE("revoke_certificates is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_UPDATE_GNSS_ACCESS_CONTROL: {
        bool allowed = get_byte(buff, &offset, sizeof(buff));
        if(agps_interface->update_gnss_access_control) {
           agps_interface->update_gnss_access_control(allowed);
        } else {
            LOGE("update_gnss_access_control is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_DATA_CONN_FAILED: {
        if (agps_interface->data_conn_failed) {
           agps_interface->data_conn_failed();
        } else {
            LOGE("data_conn_failed is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_DATA_CONN_CLOSED: {
        if (agps_interface->data_conn_closed) {
           agps_interface->data_conn_closed();
        } else {
            LOGE("data_conn_closed is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_NI_MESSAGE: {
        char msg[1024] = {0};
        int len;
        len = get_binary(buff, &offset, msg, sizeof(buff), sizeof(msg));
        if (agps_interface->ni_message) {
           agps_interface->ni_message(msg, len);
        } else {
            LOGE("ni_message is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_NI_RESPOND: {
        int session_id = get_int(buff, &offset, sizeof(buff));
        int user_response = get_int(buff, &offset, sizeof(buff));
        if (agps_interface->ni_respond) {
           agps_interface->ni_respond(session_id, user_response);
        } else {
            LOGE("ni_respond is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_SET_REF_LOC: {
        int type = get_int(buff, &offset, sizeof(buff));
        int mcc = get_int(buff, &offset, sizeof(buff));
        int mnc = get_int(buff, &offset, sizeof(buff));
        int lac = get_int(buff, &offset, sizeof(buff));
        int cid = get_int(buff, &offset, sizeof(buff));
        if (agps_interface->set_ref_loc) {
           agps_interface->set_ref_loc(type, mcc, mnc, lac, cid);
        } else {
            LOGE("set_ref_loc is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_SET_SET_ID: {
        int type = get_int(buff, &offset, sizeof(buff));
        const char* setid = get_string(buff, &offset, sizeof(buff));
        if (agps_interface->set_set_id) {
           agps_interface->set_set_id(type, setid);
        } else {
            LOGE("set_set_id is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_UPDATE_NETWORK_STATE: {
        int connected = get_int(buff, &offset, sizeof(buff));
        int type = get_int(buff, &offset, sizeof(buff));
        int roaming = get_int(buff, &offset, sizeof(buff));
        const char* extra_info = get_string(buff, &offset, sizeof(buff));
        if (agps_interface->update_network_state) {
           agps_interface->update_network_state(connected, type, roaming, extra_info);
        } else {
            LOGE("update_network_state is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_UPDATE_NETWORK_AVAILABILITY: {
        int avaiable = get_int(buff, &offset, sizeof(buff));
        const char* apn = get_string(buff, &offset, sizeof(buff));
        if (agps_interface->update_network_availability) {
           agps_interface->update_network_availability(avaiable, apn);
        } else {
            LOGE("update_network_availability is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_MNL2AGPS_PMTK: {
        const char* pmtk = get_string(buff, &offset, sizeof(buff));
        if (agps_interface->rcv_pmtk) {
           agps_interface->rcv_pmtk(pmtk);
        } else {
            LOGE("rcv_pmtk is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_RAW_DBG: {
        int enabled = get_int(buff, &offset, sizeof(buff));
        if (agps_interface->raw_dbg) {
           agps_interface->raw_dbg(enabled);
        } else {
            LOGE("raw_dbg is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_REAIDING: {
        if (agps_interface->reaiding_req) {
            agps_interface->reaiding_req();
        } else {
            LOGE("reaiding_req is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_LOCATION_SYNC: {
        if (agps_interface->location_sync) {
            double lat = get_double(buff, &offset, sizeof(buff));
            double lng = get_double(buff, &offset, sizeof(buff));
            int acc = get_int(buff, &offset, sizeof(buff));
            bool alt_valid = get_byte(buff, &offset, sizeof(buff));
            float alt = get_float(buff, &offset, sizeof(buff));
            bool source_valid = get_byte(buff, &offset, sizeof(buff));
            bool source_gnss = get_byte(buff, &offset, sizeof(buff));
            bool source_nlp = get_byte(buff, &offset, sizeof(buff));
            bool source_sensor = get_byte(buff, &offset, sizeof(buff));
            agps_interface->location_sync(lat, lng, acc, alt_valid, alt, source_valid, source_gnss, source_nlp, source_sensor);
        } else {
            LOGE("location_sync is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_SET_POSITION_MODE: {
        if(agps_interface->set_position_mode) {
            int pos_mode = get_int(buff, &offset, sizeof(buff));
            agps_interface->set_position_mode(pos_mode);
        } else {
            LOGE("set_position_mode is NULL\n");
        }
        break;
    }
    case MNL_AGPS_TYPE_UPDATE_NETWORK_STATE_WITH_HANDLE: {
        uint64_t network_handle = get_long(buff, &offset, sizeof(buff));
        char connected = get_byte(buff, &offset, sizeof(buff));
        unsigned short capabilities = get_short(buff, &offset, sizeof(buff));
        const char* apn = get_string(buff, &offset, sizeof(buff));
        if(agps_interface->update_network_state_with_handle) {
            agps_interface->update_network_state_with_handle(network_handle, connected, capabilities, apn);
        } else {
            LOGE("update_network_state_with_handle is NULL\n");
        }
        break;
    }
    default:
        LOGE("mnl2agps unknown type=%d\n", type);
        break;
    }
}

int create_mnl2agps_fd(const char* agps2mnl_path, const char* mnl2agps_path) {
    int fd = bind_udp_socket((char *)mnl2agps_path);
    if(fd == -1) {
        //Android P0, /data folder cannot be used anymore, use the abstract socket instead
        LOGD("it's a Android P version, bind mtk_mnl2agps instead");
        fd = sock_udp_server_bind_local(true, "mtk_mnl2agps");
        if(fd == -1) {
            return -1;
        }
    }
    set_socket_blocking(fd, 0);
    MNLD_STRNCPY(g_agps2mnl_path, agps2mnl_path, sizeof(g_agps2mnl_path));
    return fd;
}

