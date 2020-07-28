/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <sys/stat.h>
#include <sys/statfs.h>
#include <grp.h>
#include <stddef.h>
#include <cutils/sockets.h>
#include <android/log.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <dirent.h>
#include <cutils/properties.h>
#include <sys/un.h>
#include <dirent.h>
#include <linux/limits.h>
#include <cutils/sockets.h>
#include <cutils/memory.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/poll.h>
#include <sys/param.h>
#include <sys/endian.h>
#include <linux/serial.h> /* struct serial_struct  */
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <termios.h>
#include <sys/poll.h>
#include <linux/serial.h> /* struct serial_struct  */
#include <sched.h>
#include <netdb.h>
#include <pthread.h>
#include "os_linux.h"
#include "stp_dump.h"
#include "eloop.h"
#include <linux/netlink.h>
#include <sys/select.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/mman.h>
#include "wmt_ioctl.h"

#define LOGE printf
#define LOGD printf

#define GENL_ID_CTRL    NLMSG_MIN_TYPE
#define GENL_HDRLEN     NLMSG_ALIGN(sizeof(struct genlmsghdr))
#define MTKLOG_DIR            "/data/vendor/"
#define COMBO_T32_DIR         "/data/vendor/connsyslog/"
#define COMBO_T32_NAME        "combo_t32"
#define COMBO_T32_ENAME       ".cmm"
#define COMBO_T32_NUM_MAX     50
#define COM_STR_MAX 100
#define EMI_ENAME             ".emi"
#define EMI_FILE_NAME         "SYS_WCN_EMI_DUMP"
#define MCIF_EMI_FILE_NAME    "SYS_WCN_MCIF_EMI_DUMP"
#define EMICOREDUMP_CMD       "emicoredump"

enum {
    CTRL_CMD_UNSPEC,
    CTRL_CMD_NEWFAMILY,
    CTRL_CMD_DELFAMILY,
    CTRL_CMD_GETFAMILY,
    CTRL_CMD_NEWOPS,
    CTRL_CMD_DELOPS,
    CTRL_CMD_GETOPS,
    CTRL_CMD_NEWMCAST_GRP,
    CTRL_CMD_DELMCAST_GRP,
    CTRL_CMD_GETMCAST_GRP, /* unused */
    __CTRL_CMD_MAX,
};
#define CTRL_CMD_MAX (__CTRL_CMD_MAX - 1)

enum {
    CTRL_ATTR_UNSPEC,
    CTRL_ATTR_FAMILY_ID,
    CTRL_ATTR_FAMILY_NAME,
    CTRL_ATTR_VERSION,
    CTRL_ATTR_HDRSIZE,
    CTRL_ATTR_MAXATTR,
    CTRL_ATTR_OPS,
    CTRL_ATTR_MCAST_GROUPS,
    __CTRL_ATTR_MAX,
};

#define CTRL_ATTR_MAX (__CTRL_ATTR_MAX - 1)

struct genlmsghdr {
    __u8    cmd;
    __u8    version;
    __u16   reserved;
};

#define GENLMSG_DATA(glh) ((void *)((long)NLMSG_DATA(glh) + GENL_HDRLEN))
#define GENLMSG_PAYLOAD(glh) (NLMSG_PAYLOAD(glh, 0) - GENL_HDRLEN)
#define NLA_DATA(na) ((void *)((char*)(na) + NLA_HDRLEN))

typedef struct _tagGenericNetlinkPacket {
    struct nlmsghdr n;
    struct genlmsghdr g;
    char buf[512*16];
} GENERIC_NETLINK_PACKET, *P_GENERIC_NETLINK_PACKET;



////typedef int socklen_t;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
int stp_debug_level = MSG_MSGDUMP;

struct stp_ctrl_dst {
    struct stp_ctrl_dst *next;
    struct sockaddr_un addr;
    socklen_t addrlen;
    int debug_level;
    int errors;
};

struct ctrl_iface_priv {
    struct stp_dump *stp_d;
    int sock;
    struct stp_ctrl_dst *ctrl_dst;
};

struct trace_iface_priv {
    struct stp_dump *stp_d;
    int sock;
    struct {
        FILE *fp_pkt;
        FILE *fp_drv;
        FILE *fp_fw;
        FILE *fp_t32;
    } log_f;
};

struct stp_dump {
    const char *ctrl_interface;
    const char *ifname;
    char *ctrl_interface_group;
    int wmt_fd;
    int chip_type;
    struct ctrl_iface_priv *ctrl_iface;
    struct trace_iface_priv *trace_iface;
};
struct list_head {
        char file_name[100];
        struct list_head *next, *prev;
};

struct list_head list_head;
int combo_t32_num = 0;
char combo_emi[100];

static inline void INIT_LIST_HEAD(struct list_head *list) {
    list->next = list;
    list->prev = list;
}

static inline void __list_add(struct list_head *new, struct list_head *prev, struct list_head *next) {
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

static inline void list_add_tail(struct list_head *new, struct list_head *head) {
    __list_add(new, head->prev, head);
}

static inline void __list_del(struct list_head * prev, struct list_head * next) {
    next->prev = prev;
    prev->next = next;
}

static inline void list_del(struct list_head *entry) {
    __list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}

void bubble_sort(char *a[], int n)
{
    int i, j;
    char *temp;
    for (j = 0; j < n - 1; j++)
        for (i = 0; i < n - 1 - j; i++) {
            if (strcmp(a[i], a[i+1]) > 0) {
                temp = a[i];
                a[i] = a[i+1];
                a[i+1] = temp;
            }
        }
}

void android_printf(int level, char *format, ...)
{
    int android_log_level;
    if (level >= stp_debug_level) {
        va_list ap;
        if (level == MSG_ERROR) {
            android_log_level = ANDROID_LOG_ERROR;
        } else if (level == MSG_WARNING) {
            android_log_level = ANDROID_LOG_WARN;
        } else if (level == MSG_INFO) {
            android_log_level = ANDROID_LOG_INFO;
        } else {
            android_log_level = ANDROID_LOG_DEBUG;
        }
        va_start(ap, format);
        __android_log_vprint(android_log_level, "stp_dump", format, ap);
        va_end(ap);
    }
}
int check_log_folder_exist()
{
    struct statfs fs_info;

    /* check MTKLOG_DIR folder */
    if (statfs(MTKLOG_DIR,&fs_info) < 0)
    {
        if (mkdir(MTKLOG_DIR, 00777) < 0)
        {
            stp_printf(MSG_ERROR, "remake (%s) failed:(%s)\n", MTKLOG_DIR, strerror(errno));
            return -1;
        } else
            stp_printf(MSG_WARNING, "remake (%s) success\n", MTKLOG_DIR);
    }

    /* check COMBO_T32_DIR folder */
    if (statfs(COMBO_T32_DIR,&fs_info) < 0)
    {
        if (mkdir(COMBO_T32_DIR, 00777) < 0)
        {
            stp_printf(MSG_ERROR, "remake (%s) failed:(%s)\n", COMBO_T32_DIR, strerror(errno));
            return -1;
        } else
            stp_printf(MSG_WARNING, "remake (%s) success\n", COMBO_T32_DIR);
    }
    return 0;
}

void wait_sdcard_ready()
{
    struct statfs fs_status;
    int count = 0;

    while (1)
    {
        if(statfs("/sdcard",&fs_status)==-1)
        {
            count++;
            stp_printf(MSG_WARNING, "fail to get Sdcard stat :%s, count: %d", strerror(errno), count);
        }
        else
        {
            if(fs_status.f_blocks==0)
            {
                stp_printf(MSG_WARNING, "wait sdcard not ready count:%d,:%s", count, strerror(errno));
            }
            else if(fs_status.f_blocks>0)
            {
                break;
            }
        }
        sleep(2);
    }
}

static void _stp_hexdump(int level, const char *title, const u8 *buf,
             size_t len, int show)
{
    size_t i;
    if (level < stp_debug_level)
        return;

    printf("%s - hexdump(len=%lu):", title, (unsigned long) len);
    if (buf == NULL) {
        printf(" [NULL]");
    } else if (show) {
        for (i = 0; i < len; i++)
            printf(" %02x", buf[i]);
    } else {
        printf(" [REMOVED]");
    }
    printf("\n");
}

void stp_hexdump(int level, const char *title, const u8 *buf, size_t len)
{
    _stp_hexdump(level, title, buf, len, 1);
}

static int stp_dump_ctrl_iface_attach(struct ctrl_iface_priv *priv,
                        struct sockaddr_un *from,
                        socklen_t fromlen)
{
    struct stp_ctrl_dst *dst;

    dst = os_zalloc(sizeof(*dst));
    if (dst == NULL)
        return -1;
    os_memcpy(&dst->addr, from, sizeof(struct sockaddr_un));
    dst->addrlen = fromlen;
    dst->debug_level = MSG_INFO;
    dst->next = priv->ctrl_dst;
    priv->ctrl_dst = dst;
    stp_hexdump(MSG_DEBUG, "CTRL_IFACE monitor attached",
            (u8 *) from->sun_path,
            fromlen - offsetof(struct sockaddr_un, sun_path));
    return 0;
}

static int stp_dump_ctrl_iface_detach(struct ctrl_iface_priv *priv,
                        struct sockaddr_un *from,
                        socklen_t fromlen)
{
    struct stp_ctrl_dst *dst, *prev = NULL;

    dst = priv->ctrl_dst;
    while (dst) {
        if (fromlen == dst->addrlen &&
            os_memcmp(from->sun_path, dst->addr.sun_path,
                  fromlen - offsetof(struct sockaddr_un, sun_path))
            == 0) {
            if (prev == NULL)
                priv->ctrl_dst = dst->next;
            else
                prev->next = dst->next;
            os_free(dst);
            stp_hexdump(MSG_DEBUG, "CTRL_IFACE monitor detached",
                    (u8 *) from->sun_path,
                    fromlen -
                    offsetof(struct sockaddr_un, sun_path));
            return 0;
        }
        prev = dst;
        dst = dst->next;
    }
    return -1;
}

static int stp_dump_ctrl_iface_level(struct ctrl_iface_priv *priv,
                       struct sockaddr_un *from,
                       socklen_t fromlen,
                       char *level)
{
    struct stp_ctrl_dst *dst;

    stp_printf(MSG_DEBUG, "CTRL_IFACE LEVEL %s\n", level);

    dst = priv->ctrl_dst;
    while (dst) {
        if (fromlen == dst->addrlen &&
            os_memcmp(from->sun_path, dst->addr.sun_path,
                  fromlen - offsetof(struct sockaddr_un, sun_path))
            == 0) {
            stp_hexdump(MSG_DEBUG, "CTRL_IFACE changed monitor "
                    "level", (u8 *) from->sun_path,
                    fromlen -
                    offsetof(struct sockaddr_un, sun_path));
            dst->debug_level = atoi(level);
            return 0;
        }
        dst = dst->next;
    }

    return -1;
}

static void stp_dump_ctrl_iface_receive(int sock, void *eloop_ctx __attribute__((__unused__)),
                          void *sock_ctx)
{
    struct ctrl_iface_priv *priv = sock_ctx;
    char buf[256];
    int res;
    struct sockaddr_un from;
    socklen_t fromlen = sizeof(from);
    size_t reply_len = 0;
    int new_attached = 0;

    res = recvfrom(sock, buf, sizeof(buf) - 1, 0,
               (struct sockaddr *) &from, &fromlen);
    if (res < 0) {
        perror("recvfrom(ctrl_iface)");
        return;
    }
    buf[res] = '\0';

    if (os_strcmp(buf, "ATTACH") == 0) {
        if (stp_dump_ctrl_iface_attach(priv, &from, fromlen))
            reply_len = 1;
        else {
            new_attached = 1;
            reply_len = 2;
        }
    } else if (os_strcmp(buf, "DETACH") == 0) {
        if (stp_dump_ctrl_iface_detach(priv, &from, fromlen))
            reply_len = 1;
        else
            reply_len = 2;
    } else if (os_strncmp(buf, "LEVEL ", 6) == 0) {
        if (stp_dump_ctrl_iface_level(priv, &from, fromlen,
                            buf + 6))
            reply_len = 1;
        else
            reply_len = 2;
    } else {
        //stp_hexdump(MSG_INFO, "stp_dump_ctrl_iface_process", buf, 4);
        //reply = stp_dump_ctrl_iface_process(stp_d, buf,
        //                      &reply_len);
    }
#if 0
    if (reply) {
       if ( 0 > sendto(sock, reply, reply_len, 0, (struct sockaddr *) &from, fromlen))
           perror("send reply error\n");
        os_free(reply);
    } else if (reply_len == 1) {
#else
    if (reply_len == 1) {
#endif
       if (0 > sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *) &from, fromlen))
           perror("send string 'FAIL' to sock error\n");
    } else if (reply_len == 2) {
        if ( 0 > sendto(sock, "OK\n", 3, 0, (struct sockaddr *) &from, fromlen))
            perror("send string 'OK' to sock error\n");
    }
}


static char * stp_dump_ctrl_iface_path(struct stp_dump *stp_d)
{
    char *buf;
    size_t len;
    char *pbuf, *dir = NULL, *gid_str = NULL;
    int res;

    if (stp_d->ctrl_interface == NULL)
    {
        stp_printf(MSG_ERROR, "stp_d->ctrl_interface = NULL\n");
        return NULL;    
    }
    pbuf = os_strdup(stp_d->ctrl_interface);

    if (pbuf == NULL)
    {
        stp_printf(MSG_ERROR, "copy string failed\n");
        return NULL;
    }
    if (os_strncmp(pbuf, "DIR=", 4) == 0) {
        dir = pbuf + 4;
        gid_str = os_strstr(dir, " GROUP=");
        if (gid_str) {
            *gid_str = '\0';
            gid_str += 7;
        }
    } else
        dir = pbuf;
    
    len = os_strlen(dir) + os_strlen(stp_d->ifname) + 2;
    buf = os_malloc(len);
    if (buf == NULL) 
    {
        stp_printf(MSG_ERROR, "memory allocation failed\n");
        os_free(pbuf);
        return NULL;
    }

    res = os_snprintf(buf, len, "%s/%s", dir, stp_d->ifname);
    if (res < 0 || (size_t) res >= len) 
    {
        stp_printf(MSG_ERROR, "os_snprintf failed\n");
        os_free(pbuf);
        os_free(buf);
        return NULL;
    }
    os_free(pbuf);
    return buf;
}

int stp_dump_ctrl_iface_trace_logger_init(struct trace_iface_priv *tr_priv) {
    char str[100] = {""};
    char *file_name[50] = {NULL};
    int i = 0;
    int t_counter = 0;
    struct dirent *pDirent = NULL;
    DIR *pDir = NULL;
    struct list_head *node;

    while (120 > t_counter) {
        if (mkdir(MTKLOG_DIR, 00777) < 0) {
            if (errno == EEXIST) {
                stp_printf(MSG_INFO, "Using existing control interface directory.\n");
                break;
            } else {
                stp_printf(MSG_ERROR, "mkdir (%s) failed:(%s)\n", MTKLOG_DIR, strerror(errno));
                perror("mkdir[MTKLOG_DIR]");
                os_sleep(0, 500000);
                t_counter++;
            }
        } else {
            stp_printf(MSG_INFO, "mkdir (%s) succeed\n", MTKLOG_DIR);
            break;
        }
    }
    if (120 <= t_counter) {
        stp_printf(MSG_ERROR, "mkdir (%s) failed, retry %d seconds\n", MTKLOG_DIR, t_counter/2);
        return -1;
    }

    t_counter = 0;
    while (120 > t_counter) {
        if (mkdir(COMBO_T32_DIR, 00777) < 0) {
            if (errno == EEXIST) {
                stp_printf(MSG_INFO, "Using existing control interface directory.\n");
                break;
            } else {
                stp_printf(MSG_ERROR, "mkdir (%s) failed:(%s)\n", COMBO_T32_DIR, strerror(errno));
                perror("mkdir[COMBO_T32_DIR]");
                os_sleep(0, 500000);
                t_counter++;
            }
        } else {
            stp_printf(MSG_INFO, "mkdir (%s) succeed\n", COMBO_T32_DIR);
            break;
        }
    }
    if (120 <= t_counter) {
        stp_printf(MSG_ERROR, "mkdir (%s) failed, retry %d seconds\n", COMBO_T32_DIR, t_counter/2);
        return -1;
    }

    sprintf(str, "%s%s%s", COMBO_T32_DIR, COMBO_T32_NAME, COMBO_T32_ENAME);
    tr_priv->log_f.fp_t32 = fopen(str, "a+");
    if (tr_priv->log_f.fp_t32 == NULL) {
        stp_printf(MSG_ERROR, "create combo_fw.log fails, exit errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }
    if (0 != remove(str))
        perror("remove combo_fw fails\n");

    pDir = opendir(COMBO_T32_DIR);
    if (NULL == pDir) {
        stp_printf(MSG_ERROR, "coredump patch cannot be opened\n");
        exit(EXIT_FAILURE);
    }
    /* stp_printf(MSG_ERROR, "coreduzmp patch opened\n"); */

    while (NULL != (pDirent = readdir(pDir))) {
        char *dot = strrchr(pDirent->d_name, '.');
        /* combo_t32_xxxx.cmm */
        if (0 == strncmp(pDirent->d_name, COMBO_T32_NAME, sizeof(COMBO_T32_NAME)-1) && dot &&
            0 == strncmp(dot, COMBO_T32_ENAME, sizeof(COMBO_T32_ENAME)-1)) {
            if (combo_t32_num < COMBO_T32_NUM_MAX) {
                int file_name_len = 0;

                file_name_len = strlen(pDirent->d_name) + 1;
                if (file_name_len > 100) {
                    stp_printf(MSG_ERROR, "combo_t32 length %d over 100\n", file_name_len);
                    break;
                }
                file_name[combo_t32_num] = malloc(100);
                if (file_name[combo_t32_num] == NULL) {
                    stp_printf(MSG_ERROR, "out of memory:%d\n", combo_t32_num);
                    break;
                }
                strncpy(file_name[combo_t32_num], pDirent->d_name, file_name_len);
                stp_printf(MSG_ERROR, "file name:%s\n", file_name[combo_t32_num]);
                combo_t32_num++;
                stp_printf(MSG_ERROR, "combo_t32_num:%d\n", combo_t32_num);
            } else {
                /* stp_printf(MSG_ERROR, "file num more than the max\n"); */
                break;
            }
        }
    }

    stp_printf(MSG_ERROR, "==> combo_t32_num:%d\n", combo_t32_num);

    closedir(pDir);
    bubble_sort(file_name, combo_t32_num);

    INIT_LIST_HEAD(&list_head);
    for (i = 0; i < combo_t32_num; i++) {
        node = malloc(sizeof(struct list_head));
        if (node == NULL) {
            stp_printf(MSG_ERROR, "out of memory:%d\n", combo_t32_num);
        } else {
            sprintf(node->file_name, "%s%s", COMBO_T32_DIR, file_name[i]);
            list_add_tail(node, &list_head);
            stp_printf(MSG_ERROR, "file name:%s\n", node->file_name);
        }
        free(file_name[i]);
    }
    fclose(tr_priv->log_f.fp_t32);
    stp_printf(MSG_INFO, "close file combo_t32\n");
    return 0;
}

void stp_dump_ctrl_iface_trace_logger_decode(int sock, __attribute__((unused))void *eloop_ctx,
                          void *sock_ctx)
{
    struct nlattr *na;
    char *stpmsghdr = NULL;
    int size;
    struct trace_iface_priv *tr_priv = sock_ctx;
    GENERIC_NETLINK_PACKET mBuffer;
    GENERIC_NETLINK_PACKET *prBuffer;
    int i;
    int len;
    char coredump_mode[PROP_VALUE_MAX];
    char date[64];

    property_get("persist.vendor.connsys.coredump.mode", coredump_mode, "0");
    if (atoi(coredump_mode) == 0)
        return;

    if ((size = recv(sock, &mBuffer, sizeof(mBuffer), 0)) < 0)
    {
        LOGE("recv failed (%s)\n", strerror(errno));
        return ;
    }

    prBuffer = &mBuffer;

    /* Validate response message */
    if (!NLMSG_OK(&(prBuffer->n), (unsigned int)size))
    {
        LOGE("invalid reply message\n");
        return ;
    }
    else if (prBuffer->n.nlmsg_type == NLMSG_ERROR)
    { /* error */
        LOGE("received error\n");
        return ;
    }
    else if (!NLMSG_OK((&prBuffer->n), (unsigned int)size))
    {
        LOGE("invalid reply message received via Netlink\n");
        return ;
    }

    size = GENLMSG_PAYLOAD(&prBuffer->n);
    na = (struct nlattr *) GENLMSG_DATA(prBuffer);

    stpmsghdr = (char *)NLA_DATA(na);
    if ((stpmsghdr[0] == '[') &&
        (stpmsghdr[1] == 'M') &&
        (stpmsghdr[2] == ']')) {
        static char start_dump = 1;

        /* TODO: parsing message to know the action to start dump */
        if (start_dump) {
            char combo_t32[100] = {""};
            time_t t = time(0);
            struct list_head *node;
            struct tm *tb;
            int file_name_len = 0; // combo_t32 length

            tb = localtime(&t);
            if (tb == NULL) {
                LOGE("localtime return null. Set as default time.\n");
                snprintf(date, sizeof(date), "%s", "19700101000000");
            } else {
                strftime(date, sizeof(date), "%Y%m%d%H%M%S", tb);
            }
            file_name_len = strlen(COMBO_T32_DIR) + strlen(COMBO_T32_NAME) + strlen(date) +
                    strlen(COMBO_T32_ENAME) + 2;
            if (file_name_len > 100) {
                stp_printf(MSG_ERROR, "combo_t32 length %d over 100\n", file_name_len);
                return ;
            }
            /*fclose(tr_priv->log_f.fp_t32);*/
            /*tr_priv->log_f.fp_t32 = 0;*/
            if (check_log_folder_exist() != 0)
            {
                stp_printf(MSG_ERROR, "####log folder %s not exist####", COMBO_T32_DIR);
                return;
            }
            memset(combo_emi, 0, sizeof(combo_emi));
            sprintf(combo_emi, "%s%s_%s%s", COMBO_T32_DIR, COMBO_T32_NAME, date, EMI_ENAME);
            if (atoi(coredump_mode) == 2) {
                sprintf(combo_t32, "%s%s_%s%s", COMBO_T32_DIR, COMBO_T32_NAME, date, COMBO_T32_ENAME);

                tr_priv->log_f.fp_t32 = fopen(combo_t32, "w");
                if (tr_priv->log_f.fp_t32 == NULL) {
                    stp_printf(MSG_ERROR, "####open %s fail####,%s", combo_t32, strerror(errno));
                    return;
                }
                if (0 != chmod(combo_t32, 00664)) {
                    perror("chmod combo_t32 error\n");
                }
                /* stp_printf(MSG_DEBUG, "create a new file %s\n", combo_t32); */
                combo_t32_num++;

                if (combo_t32_num < COMBO_T32_NUM_MAX) {
                    node = malloc(sizeof(struct list_head));
                    /* stp_printf(MSG_ERROR, "create a new node\n"); */
                } else {
                    char tmp[100] = {0};
                    char *dot;

                    node = list_head.next;
                    if (node != NULL) {
                        dot = strrchr(node->file_name, '.');
                        if (dot) {
                            strncpy(tmp, node->file_name, dot - node->file_name);
                            strncpy(tmp + (dot - node->file_name), EMI_ENAME, sizeof(EMI_ENAME));
                        }
                        list_del(list_head.next);
                        stp_printf(MSG_ERROR, "combo_t32_num more than max, delete %s and %s\n", node->file_name, tmp);
                        if (remove(node->file_name))
                            stp_printf(MSG_ERROR, "combo_t32_num more than max, Unable to delete the %s(%d))\n",
                                   node->file_name, errno);
                        if (remove(tmp))
                            stp_printf(MSG_ERROR, "combo_t32_num more than max, Unable to delete the %s(%d))\n",
                                   tmp, errno);
                    }
                }
                if (node) {
                    strncpy(node->file_name, combo_t32, file_name_len);
                    list_add_tail(node, &list_head);
                }
            }

            ioctl(tr_priv->stp_d->wmt_fd, 10, &stpmsghdr[5]);
            start_dump = 0;
        }

        if (strstr(&stpmsghdr[5], EMICOREDUMP_CMD)) {
            void *map_memory;
            unsigned int emi_size;
            FILE *fp_emi;
            char aee_emi[100] = {""};
            void *map_memory_directpath;
            unsigned int direct_path_emi_size;
            char direct_path_emi[100] = {""};
            FILE *fp_dp_emi;

            /* Get connsys EMI reserve memory size */
            emi_size = ioctl(tr_priv->stp_d->wmt_fd, WMT_IOCTL_GET_EMI_PHY_SIZE, 0);
            if (emi_size < 0) {
                stp_printf(MSG_ERROR, "emi_size = %d error\n", emi_size);
                return;
            }
            stp_printf(MSG_INFO, "emi_size: %d\n", emi_size);
            fp_emi = fopen(combo_emi, "w");
            if (fp_emi == NULL) {
                stp_printf(MSG_ERROR, "####open %s fail####,%s", combo_emi, strerror(errno));
                start_dump = 1;
                return;
            }
            if (0 != chmod(combo_emi, 00664)) {
                perror("chmod combo_emi error\n");
            }

            map_memory = mmap(NULL, emi_size, PROT_READ, MAP_SHARED, tr_priv->stp_d->wmt_fd, 0);
            if (map_memory == MAP_FAILED)
                stp_printf(MSG_INFO, "mmap failed\n");
            else {
                fwrite(map_memory, 1, emi_size, fp_emi);
                munmap(map_memory, emi_size);
            }

            fclose(fp_emi);
            stp_printf(MSG_INFO, "Dump EMI memory Successfully.\n");
            /* For AEE, rename file name to SYS_WCN_EMI_DUMP */
            if (atoi(coredump_mode) == 1) {
                sprintf(aee_emi, "%s%s", COMBO_T32_DIR, EMI_FILE_NAME);
                if (remove(aee_emi) && errno != ENOENT)
                    stp_printf(MSG_ERROR, "Unable to delete the %s(%d))\n", aee_emi, errno);

                if (rename(combo_emi, aee_emi))
                    stp_printf(MSG_ERROR, "Unable to rename the file %s to %s\n", combo_emi, aee_emi);
                else
                    stp_printf(MSG_ERROR, "Rename the file %s to %s\n", combo_emi, aee_emi);
            }

            direct_path_emi_size = ioctl(tr_priv->stp_d->wmt_fd, WMT_IOCTL_GET_DIRECT_PATH_EMI_SIZE, 0);
            stp_printf(MSG_INFO, "direct path emi_size: %d\n", direct_path_emi_size);
            if (direct_path_emi_size != 0) {
                snprintf(direct_path_emi, 100, "%s%s_%s_mcif%s", COMBO_T32_DIR, COMBO_T32_NAME, date, EMI_ENAME);
                stp_printf(MSG_INFO, "Direct path dump file:%s\n", direct_path_emi);
                fp_dp_emi = fopen(direct_path_emi, "w");
                if (fp_dp_emi == NULL) {
                    stp_printf(MSG_ERROR, "####open %s fail####,%s", direct_path_emi, strerror(errno));
                } else {
                    map_memory_directpath = mmap(NULL, direct_path_emi_size, PROT_READ, MAP_SHARED, tr_priv->stp_d->wmt_fd, 1*getpagesize());
                    stp_printf(MSG_INFO, "map_memory_directpath = %p\n", map_memory_directpath);
                    if (map_memory_directpath == MAP_FAILED)
                        stp_printf(MSG_INFO, "mmap direct path emi failed\n");
                    else {
                        fwrite(map_memory_directpath, 1, direct_path_emi_size, fp_dp_emi);
                        munmap(map_memory_directpath, direct_path_emi_size);
                    }
                    fclose(fp_dp_emi);
                    stp_printf(MSG_INFO, "Dump Direct Path EMI memory Successfully.\n");
                    /* For AEE, rename file name to SYS_WCN_MCIF_EMI_DUMP */
                    if (atoi(coredump_mode) == 1) {
                        sprintf(aee_emi, "%s%s", COMBO_T32_DIR, MCIF_EMI_FILE_NAME);
                        if (remove(aee_emi) && errno != ENOENT)
                            stp_printf(MSG_ERROR, "Unable to delete the %s(%d))\n", aee_emi, errno);

                        if (rename(direct_path_emi, aee_emi))
                            stp_printf(MSG_ERROR, "Unable to rename the file %s to %s\n", direct_path_emi, aee_emi);
                        else
                            stp_printf(MSG_ERROR, "Rename the file %s to %s\n", direct_path_emi, aee_emi);
                    }
                }
            }
            ioctl(tr_priv->stp_d->wmt_fd, 11, 0);
            start_dump = 1;
            return;
        }

        len = stpmsghdr[4];
        len <<= 8;
        len += stpmsghdr[3];
        for (i = 0; i < len; i++)
            fprintf(tr_priv->log_f.fp_t32, "%c",  stpmsghdr[5+i]);
        fflush(tr_priv->log_f.fp_t32);
        /* stp_printf(MSG_DEBUG, "rev coredump len:%d\n", len); */
        /* stp_printf(MSG_DEBUG, "rev coredump date:%s\n", &stpmsghdr[5]); */

        if (len < 1024) {
            i = 0;
            do {
                i++;
            }while(('c' != stpmsghdr[len - i]) && (i < 50));
        }
        if (strstr(&stpmsghdr[5], "coredump end") || strstr(&stpmsghdr[len - i], "coredump end")) {
            /* inform user to dump action is done */
            stp_printf(MSG_DEBUG, "coredump end\n");
            fclose(tr_priv->log_f.fp_t32);
            tr_priv->log_f.fp_t32 = 0;
            if (tr_priv->stp_d->chip_type == WMT_CHIP_TYPE_COMBO) {
                ioctl(tr_priv->stp_d->wmt_fd, 11, 0);
                start_dump = 1;
            }
        }
    } else
    {
        LOGE("invalid dump data\n");
    }
    
}

int stp_dump_ctrl_iface_trace_logger_sendto_fd(int s, const char *buf, int bufLen)
{
    struct sockaddr_nl nladdr;
    int r;

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    while ((r = sendto(s, buf, bufLen, 0, (struct sockaddr *) &nladdr,
                    sizeof(nladdr))) < bufLen) 
    {
        if (r > 0) 
        {
            buf += r;
            bufLen -= r;
        } 
        else if (errno != EAGAIN)
        {
            stp_printf(MSG_ERROR, "%s failed\n", __func__); 
            return -1;
        }
    }
	stp_printf(MSG_INFO, "%s succeed\n", __func__); 
    return 0;
}

/*
 * Probe the controller in genetlink to find the family id
 */
int stp_dump_ctrl_iface_trace_logger_get_family_id(int sk, const char *family_name)
{
    struct nlattr *na;
    int rep_len;
    int id = -1;
    GENERIC_NETLINK_PACKET family_req, ans;

    /* Get family name */
    family_req.n.nlmsg_type = GENL_ID_CTRL;
    family_req.n.nlmsg_flags = NLM_F_REQUEST;
    family_req.n.nlmsg_seq = 0;
    family_req.n.nlmsg_pid = getpid();
    family_req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    family_req.g.cmd = CTRL_CMD_GETFAMILY;
    family_req.g.version = 0x1;

    na = (struct nlattr *) GENLMSG_DATA(&family_req);
    na->nla_type = CTRL_ATTR_FAMILY_NAME;
    na->nla_len = strlen(family_name) + 1 + NLA_HDRLEN;
    strncpy((char *)NLA_DATA(na), family_name, (strlen(family_name) + 1));
 
    family_req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    if (stp_dump_ctrl_iface_trace_logger_sendto_fd(sk, (char *) &family_req, family_req.n.nlmsg_len) < 0) 
    {
        stp_printf(MSG_ERROR, "%s failed\n", __func__); 
        return -1;
    }

    rep_len = recv(sk, &ans, sizeof(ans), 0);
    if (rep_len < 0)
    {
        stp_printf(MSG_ERROR, "no response\n");
        return -1;
    }
    /* Validate response message */
    else if (!NLMSG_OK((&ans.n), (unsigned int)rep_len))
    {
        stp_printf(MSG_ERROR,"invalid reply message\n");
        return -1;
    }
    else if (ans.n.nlmsg_type == NLMSG_ERROR) 
    { /* error */
        stp_printf(MSG_ERROR, "received error\n");
        return -1;
    }

    na = (struct nlattr *) GENLMSG_DATA(&ans);
    na = (struct nlattr *) ((char *) na + NLA_ALIGN(na->nla_len));
    if (na->nla_type == CTRL_ATTR_FAMILY_ID) 
    {
        id = *(__u16 *) NLA_DATA(na);
    }

    return id;
}

int stp_dump_ctrl_iface_trace_logger_start(struct trace_iface_priv *tr_priv){

    struct sockaddr_nl nladdr;
    int sz = 64 * 1024;
    GENERIC_NETLINK_PACKET req;
    struct nlattr *na;
    int id;
    int mlength = 14;
    const char *message = "HELLO"; //message
    int rc;
    int count = 100;

    if (stp_dump_ctrl_iface_trace_logger_init(tr_priv))
        return -1;

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;
    nladdr.nl_pid = getpid();

    if ((tr_priv->sock= socket(AF_NETLINK,
                        SOCK_RAW,NETLINK_GENERIC)) < 0)
    {
        stp_printf(MSG_ERROR, "Unable to create uevent socket: %s", strerror(errno));
        return -1;
    }

    if (setsockopt(tr_priv->sock, SOL_SOCKET, SO_RCVBUF, &sz, sizeof(sz)) < 0)
    {
        stp_printf(MSG_ERROR,"Unable to set uevent socket options: %s", strerror(errno));
        return -1;
    }

    if (bind(tr_priv->sock, (struct sockaddr *) &nladdr, sizeof(nladdr)) < 0)
    {
        stp_printf(MSG_ERROR,"Unable to bind uevent socket: %s", strerror(errno));
        return -1;
    }

    while(count--)
    {
        id = stp_dump_ctrl_iface_trace_logger_get_family_id(tr_priv->sock, "STP_DBG");
        if (-1 == id)
        {
            stp_printf(MSG_ERROR,"Unable to get family id, Retry");
            sleep(3);
        }
        else
        {
            stp_printf(MSG_INFO,"[STP_DBG] family id = %d\n", id);
            printf("[STP_DBG] family id = %d\n", id);
            break;
        }
    }

    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_type = id;
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.n.nlmsg_seq = 60;
    req.n.nlmsg_pid = getpid();
    req.g.cmd = 1;

    na = (struct nlattr *) GENLMSG_DATA(&req);
    na->nla_type = 1; //MTK_WIFI_ATTR_MSG
    na->nla_len = mlength + NLA_HDRLEN; //message length
    memcpy(NLA_DATA(na), message, strlen(message) + 1);
    req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    stp_printf(MSG_INFO, "sending dummy command\n");

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    rc = sendto(tr_priv->sock, (char *)&req, req.n.nlmsg_len, 0,
            (struct sockaddr *) &nladdr, sizeof(nladdr));
#if 0
    if (rc > 0)
    {
        stp_printf(MSG_INFO, "sending dummy command okay\n");
    }
    else if (errno != EAGAIN)
    {
        stp_printf(MSG_ERROR, "%s failed\n", __func__);
        return -1;
    }
#endif
    eloop_register_read_sock(tr_priv->sock, stp_dump_ctrl_iface_trace_logger_decode,
                 tr_priv->stp_d, tr_priv);
    return 0;
}

struct ctrl_iface_priv *
stp_dump_ctrl_iface_init(struct stp_dump *stp_d)
{
    struct ctrl_iface_priv *priv;
    struct trace_iface_priv *tr_priv;
    struct sockaddr_un addr;
    char *fname = NULL;
    gid_t gid = 0;
    int gid_set = 0;
    char *buf, *dir = NULL, *gid_str = NULL;
    struct group *grp;
    char *endp;

    priv = os_zalloc(sizeof(*priv));
    if (priv == NULL)
    {   
        stp_printf(MSG_ERROR, "memory allocation for priv failed\n");
        return NULL;
    }

    tr_priv = os_zalloc(sizeof(*tr_priv));
    if(tr_priv == NULL)
    {
        stp_printf(MSG_ERROR, "memory allocation for tr_priv failed\n");
        os_free(priv);
        priv = NULL;
        return NULL;
    }

    priv->stp_d = stp_d;
    priv->sock = -1;

    tr_priv->stp_d = stp_d;
    tr_priv->sock = -1;

    if (stp_d->ctrl_interface == NULL)
    {   
        stp_printf(MSG_ERROR, "stp_d->ctrl_interface = NULL\n");
        os_free(priv);
        priv = NULL;
        os_free(tr_priv);
        tr_priv = NULL;
        return priv;
    }

    buf = os_strdup(stp_d->ctrl_interface);
    if (buf == NULL)
    {
        stp_printf(MSG_ERROR, "os_strdup\n");
        goto fail;
    }

    os_snprintf(addr.sun_path, sizeof(addr.sun_path), "stp_%s",
            stp_d->ctrl_interface);
    stp_printf(MSG_INFO, "addr.sun_path:%s\n", addr.sun_path);
    priv->sock = android_get_control_socket(addr.sun_path);
    if (priv->sock >= 0)
    {
        stp_printf(MSG_INFO, "priv->sock already exist\n");
        goto havesock;
    }
    if (os_strncmp(buf, "DIR=", 4) == 0) {
        dir = buf + 4;
        gid_str = os_strstr(dir, " GROUP=");
        if (gid_str) {
            *gid_str = '\0';
            gid_str += 7;
        }
    } else {
        dir = buf;
        gid_str = stp_d->ctrl_interface_group;
    }

    if (mkdir(dir, 00777) < 0)
    {
        if (errno == EEXIST)
        {
            stp_printf(MSG_INFO, "Using existing control "
                   "interface directory.\n");
        }
        else
        {
            stp_printf(MSG_ERROR, "mkdir (%s) failed:(%s)\n", dir, strerror(errno));
            perror("mkdir[ctrl_interface]");
            goto fail;
        }
    }
    stp_printf(MSG_INFO, "mkdir (%s) succeed\n", dir);
    if (gid_str)
    {
        grp = getgrnam("system");
        if (grp)
        {
            gid = grp->gr_gid;
            gid_set = 1;
            stp_printf(MSG_INFO, "ctrl_interface_group=%d"
                   " (from group name '%s')\n",
                   (int) gid, gid_str);
        }
        else
        {
            /* Group name not found - try to parse this as gid */
            gid = strtol(gid_str, &endp, 10);
            if (*gid_str == '\0' || *endp != '\0') {
                stp_printf(MSG_ERROR, "CTRL: Invalid group "
                       "'%s'\n", gid_str);
                goto fail;
            }
            gid_set = 1;
            stp_printf(MSG_INFO, "ctrl_interface_group=%d\n",
                   (int) gid);
        }
    }

    if (gid_set && chown(dir,  getuid(), gid) < 0)
    {
        perror("chown[ctrl_interface]");
		stp_printf(MSG_ERROR, "chown (%s) failed\n", dir);
        goto fail;
    }
    stp_printf(MSG_INFO, "chown (%s) succeed\n", dir);
	
    /* Make sure the group can enter and read the directory */
    if (gid_set &&
    chmod(dir, 00775) < 0)
    {
        stp_printf(MSG_ERROR, "CTRL: chmod[%s]: %s\n", dir, strerror(errno));
        goto fail;
    }
    stp_printf(MSG_INFO, "CTRL: chmod[%s] succeed\n", dir);
    if (os_strlen(dir) + 1 + os_strlen(stp_d->ifname) >=
        sizeof(addr.sun_path)) 
    {
        stp_printf(MSG_ERROR, "ctrl_iface path limit exceeded\n");
        goto fail;
    }

    if (stp_dump_ctrl_iface_trace_logger_start(tr_priv))
        goto fail;
    priv->sock = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (priv->sock < 0) 
    {
        perror("socket(PF_UNIX)");
		stp_printf(MSG_ERROR, "create socket failed\n");
        goto fail;
    }

    os_memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;

    fname = stp_dump_ctrl_iface_path(stp_d);

    if (fname == NULL)
    {
        stp_printf(MSG_ERROR, "stp_dump_ctrl_iface_path failed\n");
        goto fail;
    }
    os_strlcpy(addr.sun_path, fname, sizeof(addr.sun_path));
    if (bind(priv->sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        stp_printf(MSG_ERROR, "ctrl_iface bind(PF_UNIX) failed: %s\n",
               strerror(errno));
        if (connect(priv->sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            stp_printf(MSG_ERROR, "ctrl_iface exists, but does not"
                   " allow connections - assuming it was left"
                   "over from forced program termination\n");
            if (unlink(fname) < 0) {
                perror("unlink[ctrl_iface]");
                stp_printf(MSG_ERROR, "Could not unlink "
                       "existing ctrl_iface socket '%s'\n",
                       fname);
                goto fail;
            }
            if (bind(priv->sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
                perror("bind(PF_UNIX)");
                goto fail;
            }
            stp_printf(MSG_DEBUG, "Successfully replaced leftover " "ctrl_iface socket '%s'\n", fname);
        } else {
            stp_printf(MSG_INFO, "ctrl_iface exists and seems to " "be in use - cannot override it\n");
            stp_printf(MSG_INFO, "Delete '%s' manually if it is "
                   "not used anymore\n", fname);
            os_free(fname);
            fname = NULL;
            goto fail;
        }
    }
    if (gid_set && chown(fname, -1, gid) < 0) {
        perror("chown[ctrl_interface/ifname]");
		stp_printf(MSG_ERROR, "chown(%s) failed, ", fname);
        goto fail;
    }

    if (chmod(fname, S_IRWXU | S_IRWXG) < 0) {
        perror("chmod[ctrl_interface/ifname]");
		stp_printf(MSG_ERROR, "chmod(%s) failed, ", fname);
        goto fail;
    }
    os_free(fname);
    fname = NULL;
    /* open wmt dev */
    stp_d->wmt_fd = open("/dev/stpwmt", O_RDWR | O_NOCTTY);
    if (stp_d->wmt_fd < 0) {
        stp_printf(MSG_ERROR, "[%s] Can't open stpwmt \n", __FUNCTION__);
        goto fail;
    }
    tr_priv->stp_d->chip_type = ioctl(tr_priv->stp_d->wmt_fd, WMT_IOCTL_GET_CHIP_INFO, 4);

havesock:
    eloop_register_read_sock(priv->sock, stp_dump_ctrl_iface_receive,
                 stp_d, priv);

    os_free(buf);
    buf = NULL;
    return priv;

fail:
    if (priv->sock >= 0)
        close(priv->sock);
    os_free(priv);
    os_free(tr_priv);
    if (fname) {
        unlink(fname);
        os_free(fname);
        fname = NULL;
    }
    os_free(buf);
    buf = NULL;
    return NULL;
}


void stp_dump_ctrl_iface_deinit(struct ctrl_iface_priv *priv)
{
#if 0
    struct wpa_ctrl_dst *dst, *prev;
#endif
    struct stp_ctrl_dst *dst, *prev;
    stp_printf(MSG_INFO, "%s\n", __func__);
    os_sleep(1, 0);
    /* close wmt dev */
    if (priv->stp_d->wmt_fd >= 0) {
        close(priv->stp_d->wmt_fd);
    }

    if (priv->sock > -1) {
        char *fname;
        char *buf, *dir = NULL, *gid_str = NULL;
        eloop_unregister_read_sock(priv->sock);
        if (priv->ctrl_dst) {
            /*
             * Wait a second before closing the control socket if
             * there are any attached monitors in order to allow
             * them to receive any pending messages.
             */
            stp_printf(MSG_DEBUG, "CTRL_IFACE wait for attached "
                   "monitors to receive messages\n");
            os_sleep(1, 0);
        }
        close(priv->sock);
        priv->sock = -1;
#if 0        
        fname = wpa_supplicant_ctrl_iface_path(priv->wpa_s);
#endif
        fname = stp_dump_ctrl_iface_path(priv->stp_d);
        if (fname) {
            unlink(fname);
            os_free(fname);
            fname = NULL;
        }
#if 0
        buf = os_strdup(priv->wpa_s->conf->ctrl_interface);
#endif  
        buf = os_strdup(priv->stp_d->ctrl_interface);
        if (buf == NULL)
            goto free_dst;
        if (os_strncmp(buf, "DIR=", 4) == 0) {
            dir = buf + 4;
            gid_str = os_strstr(dir, " GROUP=");
            if (gid_str) {
                *gid_str = '\0';
                gid_str += 7;
            }
        } else
            dir = buf;

        if (rmdir(dir) < 0) {
            if (errno == ENOTEMPTY) {
                stp_printf(MSG_DEBUG, "Control interface "
                       "directory not empty - leaving it "
                       "behind\n");
            } else {
                perror("rmdir[ctrl_interface]");
            }
        }
        os_free(buf);
    }

free_dst:
    dst = priv->ctrl_dst;
    while (dst) {
        prev = dst;
        dst = dst->next;
        os_free(prev);
    }
    os_free(priv);
}

void stp_dump_ctrl_iface_wait(struct ctrl_iface_priv *priv)
{
    char buf[256];
    int res;
    struct sockaddr_un from;
    socklen_t fromlen = sizeof(from);

    for (;;) {
        stp_printf(MSG_DEBUG, "CTRL_IFACE - %s - wait for monitor to "
               "attach\n", priv->stp_d->ifname);
        eloop_wait_for_read_sock(priv->sock);

        res = recvfrom(priv->sock, buf, sizeof(buf) - 1, 0,
                   (struct sockaddr *) &from, &fromlen);
        if (res < 0) {
            perror("recvfrom(ctrl_iface)");
            continue;
        }
        buf[res] = '\0';

        if (os_strcmp(buf, "ATTACH") == 0) {
            /* handle ATTACH signal of first monitor interface */
            if (!stp_dump_ctrl_iface_attach(priv, &from,
                                  fromlen)) {
               if (0 > sendto(priv->sock, "OK\n", 3, 0, (struct sockaddr *) &from, fromlen))
                   perror("send string 'OK' to sock error\n");
                /* OK to continue */
                return;
            } else {
                if (0 > sendto(priv->sock, "FAIL\n", 5, 0, (struct sockaddr *) &from, fromlen))
                    perror("send string 'FAIL' to sock error\n");
            }
        } else {
            /* return FAIL for all other signals */
           if (0 > sendto(priv->sock, "FAIL\n", 5, 0, (struct sockaddr *) &from, fromlen))
                perror("send string 'FAIL' to sock error\n");
        }
    }
}

static void stp_dump_terminate(__attribute__((unused))int sig, __attribute__((unused))void *eloop_ctx,
                     __attribute__((unused))void *signal_ctx)
{
    eloop_terminate();
}

int stp_dump_run(void)
{
    eloop_register_signal_terminate(stp_dump_terminate, NULL);
    eloop_run();
    return 0;
}

void set_sched_prio(void)
{
    struct sched_param sched, test_sched;
    int policy = 0xff;
    int err=0xff;

    policy = sched_getscheduler(0);
    sched_getparam(0, &test_sched);
    printf("Before %s policy = %d, priority = %d\n", "main" , policy, test_sched.sched_priority);

    sched.sched_priority = sched_get_priority_max(SCHED_FIFO);
    err = sched_setscheduler(0, SCHED_FIFO, &sched);
    if(err == 0){
         printf("pthread_setschedparam SUCCESS \n");
         policy = sched_getscheduler(0);
         sched_getparam(0, &test_sched);
         printf("After %s policy = %d, priority = %d\n", "main" ,policy , test_sched.sched_priority);
    }
    else{
         if(err == EINVAL) printf("policy is not one of SCHED_OTHER, SCHED_RR, SCHED_FIFO\n");
         if(err == EINVAL) printf("the  priority  value  specified by param is not valid for the specified policy\n");
         if(err == EPERM) printf("the calling process does not have superuser permissions\n");
         if(err == ESRCH) printf("the target_thread is invalid or has already terminated\n");
         if(err == EFAULT)  printf("param points outside the process memory space\n");
         printf("pthread_setschedparam FAIL \n");
    }
}

int main(__attribute__((unused))int argc, __attribute__((unused))char *argv[]){

    struct stp_dump *stp_d = NULL;
    /* This folder is created by init_connectivity.rc*/
    const char *conf ="DIR=/data/vendor/stp_dump GROUP=stp";
    const char *ifname = "stpd";
    const char *sname = "data/vendor/stp_dump";
    char value[PROPERTY_VALUE_MAX];

    stp_printf(MSG_INFO, "==>%s \n", __func__ );

    wait_sdcard_ready();

    property_get("ro.crypto.state", value, "");
    if (!strcmp(value, "")) {
            do {
                    stp_printf(MSG_INFO, "ro.crypto.state is NULL, Retry");
                    sleep(1);
                    property_get("ro.crypto.state", value, "");
                    stp_printf(MSG_INFO, "ro.crypto.state: %s!", value);
            }while(!strcmp(value, ""));
    }
    stp_printf(MSG_INFO, "ro.crypto.state: %s!", value);
    if (0 == strcmp(value, "encrypted")) {
        property_get("ro.crypto.type", value, "");
        if (!strcmp(value, "file")) {
            do {
                stp_printf(MSG_INFO, "FBE feature is open, waiting for decrypt done");
                sleep(1);
                if (access(sname, F_OK|R_OK|W_OK) == 0)
                    break;
            } while (1);
        } else {
            property_get("vold.decrypt", value, "");
            if (strcmp(value, "trigger_restart_framework")) {
                do {
                    stp_printf(MSG_INFO, "Waiting for decrypt done");
                    sleep(1);
                    property_get("vold.decrypt", value, "");
                    stp_printf(MSG_INFO, "vold.decrypt: %s!", value);
                }while(strcmp(value, "trigger_restart_framework"));
            }
            stp_printf(MSG_INFO, "vold.decrypt: %s!", value);
        }
            stp_printf(MSG_INFO, "Decrypt done!");
    } else
            stp_printf(MSG_INFO, "Device is unencrypted!");

    stp_d = os_zalloc(sizeof(struct stp_dump));
    if (stp_d == NULL)
    {   
        stp_printf(MSG_ERROR, "memory allocation for stp_dump failed\n");
        return -1;
    }
    stp_d->ctrl_interface = conf;
    stp_d->ifname = ifname;
    stp_d->ctrl_iface = stp_dump_ctrl_iface_init(stp_d);
    if (stp_d->ctrl_iface == NULL) 
    {
        stp_printf(MSG_ERROR,
               "Failed to initialize control interface '%s'.\n"
               "You may have another stp_dump process "
               "already running or the file was\n"
               "left by an unclean termination of stp_dump "
               "in which case you will need\n"
               "to manually remove this file before starting "
               "wpa_supplicant again.\n",
               "used by stp_dump\n");
        return -1;
    } 
    else 
    {
        stp_printf(MSG_INFO, "stp_dump_ctrl_iface_init succeed.\n");
    }
    stp_printf(MSG_INFO, "==>%s222 \n", __func__ );

    //set_sched_prio();
   
    stp_dump_run();

    stp_dump_ctrl_iface_deinit(stp_d->ctrl_iface);
    
    os_free(stp_d);

    return 0;
}


