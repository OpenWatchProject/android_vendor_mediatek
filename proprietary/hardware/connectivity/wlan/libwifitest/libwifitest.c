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

/*
 * This is sample code for WLAN test library implementation
 */

#include <stdio.h>
#include <stdint.h>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <cutils/log.h>
#include <cutils/misc.h> /* load_file() */
#include <libnvram.h>
#include <Custom_NvRam_LID.h>
#include <CFG_Wifi_File.h>

#include "libwifitest.h"

#define DBG 1

extern int init_module(void *, unsigned long, const char *);

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

#ifndef BIT
#define BIT(n)          ((uint32_t) 1 << (n))
#define BITS(m,n)       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))
#endif /* BIT */

#define MAC_ADDR_LEN                            6

#define IS_BMCAST_MAC_ADDR(_pucDestAddr)            \
        ((bool) ( ((uint8_t *)(_pucDestAddr))[0] & BIT(0) ))

#define EQUAL_MAC_ADDR(_pucDestAddr, _pucSrcAddr)   \
        (!memcmp(_pucDestAddr, _pucSrcAddr, MAC_ADDR_LEN))

/* Debug print format string for the MAC Address */
#define MACSTR      "%02x:%02x:%02x:%02x:%02x:%02x"

/* Debug print argument for the MAC Address */
#define MAC2STR(a)  ((uint8_t *)a)[0], ((uint8_t *)a)[1], ((uint8_t *)a)[2], \
    ((uint8_t *)a)[3], ((uint8_t *)a)[4], ((uint8_t *)a)[5]

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

#ifndef WIFI_DRIVER_MODULE_PATH
#define WIFI_DRIVER_MODULE_PATH     "/system/lib/modules/wlan.ko"
#endif
#ifndef WIFI_DRIVER_MODULE_NAME
#define WIFI_DRIVER_MODULE_NAME     "wlan"
#endif
#ifndef WIFI_DRIVER_MODULE_ARG
#define WIFI_DRIVER_MODULE_ARG      ""
#endif

#if 0
#ifndef WIFI_IF_NAME
#define WIFI_IF_NAME                "wlan0"
#endif
#endif

char WIFI_IF_NAME[256] = "wlan0";
bool fgDebugMode = true;

//static const char DRIVER_PROP_NAME[]    = "wlan.driver.status";
//static const char DRIVER_MODULE_NAME[]  = WIFI_DRIVER_MODULE_NAME;
//static const char DRIVER_MODULE_TAG[]   = WIFI_DRIVER_MODULE_NAME " ";
//static const char DRIVER_MODULE_TAG[]   = WIFI_DRIVER_MODULE_NAME;
static const char DRIVER_MODULE_PATH[]  = WIFI_DRIVER_MODULE_PATH;
static const char DRIVER_MODULE_ARG[]   = WIFI_DRIVER_MODULE_ARG;
//static const char MODULE_FILE[]         = "/proc/modules";

#define IOCTL_SET_STRUCT                (SIOCIWFIRSTPRIV + 8)
#define IOCTL_GET_STRUCT                (SIOCIWFIRSTPRIV + 9)
#define IOCTL_SET_STRUCT_FOR_EM         (SIOCIWFIRSTPRIV + 11)

#define PRIV_CMD_OID                    15

/* RF Test specific OIDs */
#define OID_CUSTOM_TEST_MODE                            0xFFA0C901
#define OID_CUSTOM_ABORT_TEST_MODE                      0xFFA0C906
#define OID_CUSTOM_MTK_WIFI_TEST_                       0xFFA0C911
#define OID_CUSTOM_MCR_RW                               0xFFA0c801

/* command mask */
#define TEST_FUNC_IDX_MASK              BITS(0,7)
#define TEST_SET_CMD_OFFSET_MASK        BITS(16,31)
#define TEST_SET_CMD_OFFSET             16

/* RF Test Properties */
#define RF_AT_PARAM_RATE_MCS_MASK   BIT(31)
#define RF_AT_PARAM_RATE_MASK       BITS(0,7)
#define RF_AT_PARAM_RATE_1M         0
#define RF_AT_PARAM_RATE_2M         1
#define RF_AT_PARAM_RATE_5_5M       2
#define RF_AT_PARAM_RATE_11M        3
#define RF_AT_PARAM_RATE_6M         4
#define RF_AT_PARAM_RATE_9M         5
#define RF_AT_PARAM_RATE_12M        6
#define RF_AT_PARAM_RATE_18M        7
#define RF_AT_PARAM_RATE_24M        8
#define RF_AT_PARAM_RATE_36M        9
#define RF_AT_PARAM_RATE_48M        10
#define RF_AT_PARAM_RATE_54M        11

#define NULL_MAC_ADDR               {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
typedef struct _NDIS_TRANSPORT_STRUCT {
    uint32_t    ndisOidCmd;
    uint32_t    inNdisOidlength;
    uint32_t    outNdisOidLength;
    uint8_t     ndisOidContent[16];
} NDIS_TRANSPORT_STRUCT, *P_NDIS_TRANSPORT_STRUCT;

typedef struct _PARAM_MTK_WIFI_TEST_STRUC_T {
    uint32_t u4FuncIndex;
    uint32_t u4FuncData;
} PARAM_MTK_WIFI_TEST_STRUC_T, *P_PARAM_MTK_WIFI_TEST_STRUC_T;

typedef struct _PARAM_CUSTOM_MCR_RW_STRUC_T {
    uint32_t    u4McrOffset;
    uint32_t    u4McrData;
} PARAM_CUSTOM_MCR_RW_STRUC_T, *P_PARAM_CUSTOM_MCR_RW_STRUC_T;

typedef enum _ENUM_RF_AT_BW_T {

    RF_AT_BW_20 = 0,
    RF_AT_BW_40,
    RF_AT_BW_U20,
    RF_AT_BW_L20,
    RF_AT_BW_NUM

} ENUM_RF_AT_BW_T, *P_ENUM_RF_AT_BW_T;

typedef enum _ENUM_RF_AT_RXOK_MATCH_RULE_T {

    RF_AT_RXOK_DISABLED = 0,
    RF_AT_RXOK_MATCH_RA_ONLY,
    RF_AT_RXOK_MATCH_TA_ONLY,
    RF_AT_RXOK_MATCH_RA_TA,
    RF_AT_RXOK_NUM

} ENUM_RF_AT_RXOK_MATCH_RULE_T, *P_ENUM_RF_AT_RXOK_MATCH_RULE_T;


/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
//static int wifi_rfkill_id = -1;
//static char *wifi_rfkill_state_path = NULL;
static int skfd = -1;
static bool fgIsTesting = false;
static PARAM_MTK_WIFI_CHANNEL_T g_sChannel;
/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is called to initial rfkill variables
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
#if 0
static int
wifi_init_rfkill(
    void
    )
{
    char path[64];
    char buf[16];
    int fd;
    int sz;
    int id;

    for (id = 0; ; id++) {
        snprintf(path, sizeof(path), "/sys/class/rfkill/rfkill%d/type", id);
        fd = open(path, O_RDONLY);
        if (fd < 0) {
            ALOGW("[%s] open(%s) failed: %s (%d)\n", __func__, path, strerror(errno), errno);
            return -1;
        }
        sz = read(fd, &buf, sizeof(buf));
        close(fd);
        if (sz >= 4 && memcmp(buf, "wlan", 4) == 0) {
            wifi_rfkill_id = id;
            break;
        }
    }

    asprintf(&wifi_rfkill_state_path, "/sys/class/rfkill/rfkill%d/state",
            wifi_rfkill_id);
    return 0;
}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is called to turn on/off Wi-Fi interface
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
#if 0
static int
wifi_set_power(
    int on
    )
{
    int sz;
    int fd = -1;
    int ret = -1;
    const char buffer = (on ? '1' : '0');

    ALOGD("[%s] %d", __func__, on);

    if (wifi_rfkill_id == -1) {
        if (wifi_init_rfkill()) {
            goto out;
        }
    }

    fd = open(wifi_rfkill_state_path, O_WRONLY);
    ALOGD("[%s] %s", __func__, wifi_rfkill_state_path);
    if (fd < 0) {
        ALOGE("open(%s) for write failed: %s (%d)",
                wifi_rfkill_state_path,
                strerror(errno),
                errno);
        goto out;
    }
    sz = write(fd, &buffer, 1);
    if (sz < 0) {
        ALOGE("write(%s) failed: %s (%d)",
                wifi_rfkill_state_path,
                strerror(errno),
                errno);
        goto out;
    }
    ret = 0;

out:
    if (fd >= 0) {
        close(fd);
    }
    return ret;
}
#else
#define WIFI_POWER_PATH                 "/dev/wmtWifi"

int wifi_set_power(int enable) {
    int sz;
    int fd = -1;
    const char buffer = (enable ? '1' : '0');

    fd = open(WIFI_POWER_PATH, O_WRONLY);
    if (fd < 0) {
        ALOGE("Open \"%s\" failed", WIFI_POWER_PATH);
        goto out;
    }
    sz = write(fd, &buffer, 1);
    if (sz < 0) {
        ALOGE("Set \"%s\" [%c] failed", WIFI_POWER_PATH, buffer);
        goto out;
    }

out:
    if (fd >= 0) close(fd);
    return 0;
}

#endif

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to insert module
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
static int
insmod(
    const char *filename,
    const char *args
    )
{
    void *module;
    unsigned int size;
    int ret;

    module = load_file(filename, &size);
    if (!module)
        return -1;

    ret = init_module(module, size, args);

    free(module);

    return ret;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to check if driver is loaded or not
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
static int
check_driver_loaded(
    void
    )
{
#if 1
	return 1;
#else
    FILE *proc;
    char line[sizeof(DRIVER_MODULE_TAG)+10];

    if ((proc = fopen(MODULE_FILE, "r")) == NULL) {
        ALOGW("Could not open %s: %s", MODULE_FILE, strerror(errno));
        return 0;
    }

    while ((fgets(line, sizeof(line), proc)) != NULL) {
        if (strncmp(line, DRIVER_MODULE_TAG, strlen(DRIVER_MODULE_TAG)) == 0) {
            fclose(proc);
            return 1;
        }
    }

    fclose(proc);
    return 0;
#endif
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to extract interface name
 *
 * @param   name    pointer to name buffer
 *          nsize   size of name buffer
 *          buf     current position in buffer
 * @return
 */
/*----------------------------------------------------------------------------*/
static inline char *
iw_get_ifname(
    char *name,
    int	  nsize,
    char *buf
    )
{
    char *end;

    /* Skip leading spaces */
    while(isspace(*buf))
        buf++;

    end = strrchr(buf, ':');

    /* Not found ??? To big ??? */
    if((end == NULL) || (((end - buf) + 1) > nsize))
        return(NULL);

    /* Copy */
    memcpy(name, buf, (end - buf));
    name[end - buf] = '\0';

    /* Return value currently unused, just make sure it's non-NULL */
    return(end);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to check whether wlan0 has been spawn or not
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
static int
find_wifi_device(
    void
    )
{
    FILE *fh;
    char  buff[1024];
    int   ret = -1;

    fh = fopen(PROC_NET_DEV, "r");

    if(fh != NULL) {
        /* Success : use data from /proc/net/wireless */
        /* Eat 2 lines of header */
        fgets(buff, sizeof(buff), fh);
        fgets(buff, sizeof(buff), fh);

        /* Read each device line */
        while(fgets(buff, sizeof(buff), fh)) {
            char  name[IFNAMSIZ + 1];
            char *s;

            /* Skip empty or almost empty lines. It seems that in some
             * *        * cases fgets return a line with only a newline. */
            if ((buff[0] == '\0') || (buff[1] == '\0'))
                continue;
            /* Extract interface name */
            s = iw_get_ifname(name, sizeof(name), buff);

            if(s) {
                ALOGD("[%s] %s", __func__, name);
                if (strcmp(name, WIFI_IF_NAME) == 0 ){
                    ret = 0;
                    break;
                }
            }
        }

        fclose(fh);
    }

    return ret;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief   Wrapper to push some Wireless Parameter in the driver
 *
 * @param   request Wireless extension identifier
 *          pwrq    Pointer to wireless extension request
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
static inline int
ioctl_iw_ext(
    int             request,
    struct iwreq   *pwrq
    )
{
    if(skfd > 0) {
        /* Set device name */
        strncpy(pwrq->ifr_name, WIFI_IF_NAME, (IFNAMSIZ - 1));

        /* Do the request */
        return(ioctl(skfd, request, pwrq));
    }
    else {
        return -1;
    }
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This API is to ask underlying software to enter/leave RF test mode
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
static int
wifi_switch_test_mode(
    int on
    )
{
    int retval;
    struct iwreq wrq;
    NDIS_TRANSPORT_STRUCT rNdisStruct;

    /* zeroize */
    memset(&wrq, 0, sizeof(struct iwreq));

    /* configure NDIS_TRANSPORT_STRUC */
    if(on == 1) {
        rNdisStruct.ndisOidCmd = OID_CUSTOM_TEST_MODE;
    }
    else if(on == 0) {
        rNdisStruct.ndisOidCmd = OID_CUSTOM_ABORT_TEST_MODE;
    }
    else {
        return -1;
    }

    rNdisStruct.inNdisOidlength = 0;
    rNdisStruct.outNdisOidLength = 0;

    /* configure struct iwreq */
    wrq.u.data.pointer = &rNdisStruct;
    wrq.u.data.length = sizeof(NDIS_TRANSPORT_STRUCT);
    wrq.u.data.flags = PRIV_CMD_OID;

    retval = ioctl_iw_ext(IOCTL_SET_STRUCT, &wrq);

    if(retval == 0) {
        if(on == 1) {
            fgIsTesting = true;
        }
        else {
            fgIsTesting = false;
        }
    }

    return retval;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This API provided a generic service for RF test set commands
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
int
WIFI_TEST_set(
    uint32_t  u4FuncIndex,
    uint32_t  u4FuncData,
    uint32_t *pu4FuncIndex,
    uint32_t *pu4FuncData
    )
{
    int retval;
    struct iwreq wrq;
    NDIS_TRANSPORT_STRUCT rNdisStruct;
    P_PARAM_MTK_WIFI_TEST_STRUC_T prTestStruct;

    prTestStruct = (P_PARAM_MTK_WIFI_TEST_STRUC_T)rNdisStruct.ndisOidContent;

    /* zeroize */
    memset(&wrq, 0, sizeof(struct iwreq));

    /* configure TEST_STRUCT */
    prTestStruct->u4FuncIndex = u4FuncIndex;
    prTestStruct->u4FuncData = u4FuncData;

    /* configure NDIS_TRANSPORT_STRUC */
    rNdisStruct.ndisOidCmd = OID_CUSTOM_MTK_WIFI_TEST_;
    rNdisStruct.inNdisOidlength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);
    rNdisStruct.outNdisOidLength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);

    /* configure struct iwreq */
    wrq.u.data.pointer = &rNdisStruct;
    wrq.u.data.length = sizeof(NDIS_TRANSPORT_STRUCT);
    wrq.u.data.flags = PRIV_CMD_OID;

    retval = ioctl_iw_ext(IOCTL_SET_STRUCT, &wrq);

    if(retval == 0) {
        if(pu4FuncIndex) {
            *pu4FuncIndex = prTestStruct->u4FuncIndex;
        }

        if(pu4FuncData) {
            *pu4FuncData = prTestStruct->u4FuncData;
        }

        DBGLOG("<ok> Set CMD[%03u] DATA[0x%08x]\n",
            (u4FuncIndex & TEST_FUNC_IDX_MASK), u4FuncData);
    }
    else {
        DBGLOG("<failed> Set CMD[%03u] DATA[0x%08x]\n",
            (u4FuncIndex & TEST_FUNC_IDX_MASK), u4FuncData);
    }

    return retval;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This API provided a generic service for RF test query commands
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
int
WIFI_TEST_get(
    uint32_t  u4FuncIndex,
    uint32_t  u4FuncData,
    uint32_t *pu4FuncIndex,
    uint32_t *pu4FuncData
    )
{
    int retval;
    struct iwreq wrq;
    NDIS_TRANSPORT_STRUCT rNdisStruct;
    P_PARAM_MTK_WIFI_TEST_STRUC_T prTestStruct;

    prTestStruct = (P_PARAM_MTK_WIFI_TEST_STRUC_T)rNdisStruct.ndisOidContent;

    /* zeroize */
    memset(&wrq, 0, sizeof(struct iwreq));

    /* configure TEST_STRUCT */
    prTestStruct->u4FuncIndex = u4FuncIndex;
    prTestStruct->u4FuncData = u4FuncData;

    /* configure NDIS_TRANSPORT_STRUC */
    rNdisStruct.ndisOidCmd = OID_CUSTOM_MTK_WIFI_TEST_;
    rNdisStruct.inNdisOidlength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);
    rNdisStruct.outNdisOidLength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);

    /* configure struct iwreq */
    wrq.u.data.pointer = &rNdisStruct;
    wrq.u.data.length = sizeof(NDIS_TRANSPORT_STRUCT);
    wrq.u.data.flags = PRIV_CMD_OID;

    retval = ioctl_iw_ext(IOCTL_GET_STRUCT, &wrq);

    if(retval == 0) {
        if(pu4FuncIndex) {
            *pu4FuncIndex = prTestStruct->u4FuncIndex;
        }

        if(pu4FuncData) {
            *pu4FuncData = prTestStruct->u4FuncData;
            DBGLOG("<ok> Get CMD[%03u] DATA[0x%08x] RESULT[0x%08x]\n",
                (u4FuncIndex & TEST_FUNC_IDX_MASK), u4FuncData, *pu4FuncData);
        }
    }
    else {
        DBGLOG("<failed> Get CMD[%03u] DATA[0x%08x]\n",
            (u4FuncIndex & TEST_FUNC_IDX_MASK), u4FuncData);
    }

    return retval;
}


/* API
 * ========================================================================== */
bool WIFI_TEST_OpenDUT(void)
{
    int count = 60;
    bool retval = false;

    ALOGD("[%s] entry\n", __func__);

    wifi_set_power(1);

    if (!check_driver_loaded()) {
        ALOGD("[%s] loading wifi driver ... ...\n", __func__);

        if (insmod(DRIVER_MODULE_PATH, DRIVER_MODULE_ARG) < 0) {
            ALOGD("[%s] failed to load wifi driver!!\n", __func__);
            goto error;
        }
    }

    sched_yield();

    while(count -- > 0) {
        if(find_wifi_device()==0) {
            retval = true;
            ALOGD("[%s] find wifi device\n", __func__);
            break;
        }
        usleep(50000);
    }

    if (retval == false) {
        goto error;
    }
    else {
        /* initialize skfd */
        if ((skfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
            ALOGE("[%s] failed to open net socket\n", __func__);
            goto error;
        }
    }

    /* switch into test mode */
    if(wifi_switch_test_mode(1) != 0) {
        goto error;
    }

    return true;

error:
    ALOGD("[%s] failure", __func__);

    if(WIFI_TEST_CloseDUT() == false) {
        ALOGE("[%s] failed to close DUT\n", __func__);
    }

    return false;
}

bool WIFI_TEST_CloseDUT(void)
{
    /* turn off test mode */
    wifi_switch_test_mode(0);

    /* close socket if necessary */
    if(skfd > 0) {
        close(skfd);
        skfd = -1;
    }

    /* no need to remove module, just turn off host power via rfkill */
    wifi_set_power(0);

    return true;
}

bool WIFI_TEST_TxDataRate(int TxDataRate)
{
    int retval;

    /*
     * CCK 1M:   RF_AT_PREAMBLE_CCK_SHORT(1) WIFI_TEST_RATE_CCK_1MBPS(0)
     * CCK 2M:   RF_AT_PREAMBLE_CCK_SHORT(1) WIFI_TEST_RATE_CCK_2MBPS(1)
     * CCK 5.5M: RF_AT_PREAMBLE_CCK_SHORT(1) WIFI_TEST_RATE_CCK_5_5MBPS(2)
     * CCK 11M:  RF_AT_PREAMBLE_CCK_SHORT(1) WIFI_TEST_RATE_CCK_11MBPS(3)
     * OFDM 6M:  RF_AT_PREAMBLE_NORMAL(0)    WIFI_TEST_RATE_OFDM_6MBPS(4)
     * OFDM 9M:  RF_AT_PREAMBLE_NORMAL(0)    WIFI_TEST_RATE_OFDM_9MBPS(5)
     * OFDM 12M: RF_AT_PREAMBLE_NORMAL(0)    WIFI_TEST_RATE_OFDM_12MBPS(6)
     * OFDM 18M: RF_AT_PREAMBLE_NORMAL(0)    WIFI_TEST_RATE_OFDM_18MBPS(7)
     * OFDM 24M: RF_AT_PREAMBLE_NORMAL(0)    WIFI_TEST_RATE_OFDM_24MBPS(8)
     * OFDM 36M: RF_AT_PREAMBLE_NORMAL(0)    WIFI_TEST_RATE_OFDM_36MBPS(9)
     * OFDM 48M: RF_AT_PREAMBLE_NORMAL(0)    WIFI_TEST_RATE_OFDM_48MBPS(10)
     * OFDM 54M: RF_AT_PREAMBLE_NORMAL(0)    WIFI_TEST_RATE_OFDM_54MBPS(11)
     * HT MCSn:  RF_AT_PREAMBLE_11N_MM(2)    WIFI_TEST_MCS_RATE_n n=0~32
     *           RF_AT_PREAMBLE_11N_GF(3)
     * VHT MCSn: RF_AT_PREAMBLE_11AC(4)      WIFI_TEST_MCS_RATE_n n=0~32
     */

    switch (TxDataRate) {
    case WIFI_TEST_RATE_AUTO:
        return false;   //@FIXME

    default:
        retval = WIFI_TEST_set(RF_AT_FUNCID_RATE,
                TxDataRate,
                NULL,
                NULL);
        break;
    }

    /* return value checking */
    if(retval == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_SetPreamble(ENUM_RF_AT_PREAMBLE_T PreambleType)
{
    int retval;

    switch (PreambleType) {
    case RF_AT_PREAMBLE_NORMAL:
        retval = WIFI_TEST_set(RF_AT_FUNCID_PREAMBLE,
                RF_AT_PREAMBLE_NORMAL,
                NULL,
                NULL);
        break;
    case RF_AT_PREAMBLE_CCK_SHORT:
        retval = WIFI_TEST_set(RF_AT_FUNCID_PREAMBLE,
                RF_AT_PREAMBLE_CCK_SHORT,
                NULL,
                NULL);
        break;
    case RF_AT_PREAMBLE_11N_MM:
        retval = WIFI_TEST_set(RF_AT_FUNCID_PREAMBLE,
                RF_AT_PREAMBLE_11N_MM,
                NULL,
                NULL);
        break;
    case RF_AT_PREAMBLE_11N_GF:
        retval = WIFI_TEST_set(RF_AT_FUNCID_PREAMBLE,
                RF_AT_PREAMBLE_11N_GF,
                NULL,
                NULL);
        break;
    case RF_AT_PREAMBLE_11AC:
        retval = WIFI_TEST_set(RF_AT_FUNCID_PREAMBLE,
                RF_AT_PREAMBLE_11AC,
                NULL,
                NULL);
        break;
    default:
        return false;
    }

    /* return value checking */
    if(retval == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_TxGain(int TxGain)
{
    /* assign TX power gain */
    if(WIFI_TEST_set(RF_AT_FUNCID_POWER,
            TxGain, //bypass to firmware; TxGain=30, 15dBm
            NULL,
            NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_TxBurstInterval(int SIFS)
{
	if (SIFS < 20 || SIFS > 1000)
		return false;

    /* specify packet interval */
    if(WIFI_TEST_set(RF_AT_FUNCID_PKTINTERVAL,
            SIFS,
            NULL,
            NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_TxPayloadLength(int TxPayLength)
{
	if (TxPayLength <= 0)
		return false;

    /* specify packet length */
    if(WIFI_TEST_set(RF_AT_FUNCID_PKTLEN,
            TxPayLength,
            NULL,
            NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_TxBurstFrames(int Frames)
{
	if (Frames < 0)
		return false;

    /* specify packet count */
    if(WIFI_TEST_set(RF_AT_FUNCID_PKTCNT,
                Frames,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_TxDestAddress(unsigned char *addr)
{
    uint8_t aucMacAddr[4];
    uint32_t u4MacAddr;

    /* specify MAC address[0:3] */
    memcpy(aucMacAddr, addr, sizeof(uint8_t) * 4);
    u4MacAddr = *(uint32_t *)(&(aucMacAddr[0]));
    if(WIFI_TEST_set(RF_AT_FUNCID_SET_MAC_DST_ADDRESS,
                u4MacAddr,
                NULL,
                NULL) != 0) {
        return false;
    }

    /* specify MAC address[4:5] */
    memset(aucMacAddr, 0, sizeof(uint8_t) * 4);
    memcpy(aucMacAddr, addr + 4, sizeof(uint8_t) * 2);
    u4MacAddr = *(uint32_t *)(&(aucMacAddr[0]));
    if(WIFI_TEST_set(RF_AT_FUNCID_SET_MAC_DST_ADDRESS | (4 << TEST_SET_CMD_OFFSET),
                u4MacAddr,
                NULL,
                NULL) != 0) {
        return false;
    }

    return true;
}

bool WIFI_TEST_TxStart(void)
{
	/* tx start: without ack, async mode */
    if(WIFI_TEST_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_STARTTX,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_TxStop(void)
{
	/* tx stop */
    if(WIFI_TEST_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_STOPTEST,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_RxStart(void)
{
	/* rx start: without ack, async mode */
    if(WIFI_TEST_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_STARTRX,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_RxStop(void)
{
	/* rx stop */
    if(WIFI_TEST_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_STOPTEST,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_FRError(int *FError)
{
	if (!FError)
		return false;

    if(WIFI_TEST_get(RF_AT_FUNCID_RXERROR_COUNT,
                0,
                NULL,
                (uint32_t *)FError) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_FRGood(int *FRGood)
{
	if (!FRGood)
		return false;

    if(WIFI_TEST_get(RF_AT_FUNCID_RXOK_COUNT,
                0,
                NULL,
                (uint32_t *)FRGood) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_TxCount(uint32_t *TxCount)
{
	if (!TxCount)
		return false;

    if(WIFI_TEST_get(RF_AT_FUNCID_TXED_COUNT,
                0,
                NULL,
                (uint32_t *)TxCount) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_TxGoodCount(uint32_t *TxGoodCount)
{
	if (!TxGoodCount)
		return false;

    if(WIFI_TEST_get(RF_AT_FUNCID_TXOK_COUNT,
                0,
                NULL,
                (uint32_t *)TxGoodCount) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_RSSI(int *RSSI)
{
    uint32_t u4Result;
    int8_t avRssi;

    if (!RSSI)
        return false;

    if(WIFI_TEST_get(RF_AT_FUNCID_RX_RSSI,
                0,
                NULL,
                &u4Result) == 0) {
        /* u4Result[0:7]    Average RSSI (dbM)
         * u4Result[8:15]   Minimum RCPI
         * u4Result[16:23]  Maximum RCPI
         * u4Result[24:31]  Last RCPI
         */
        avRssi = (int8_t)(u4Result & BITS(0,7));
        *RSSI = avRssi;
        return true;
    }
    else {
        return false;
    }
}

#define RCPI_HIGH_BOUND 220
#define NDBM_LOW_BOUND_FOR_RCPI 110
#define RCPI_TO_dBm(_rcpi)            \
        ((float)((_rcpi) > RCPI_HIGH_BOUND ? RCPI_HIGH_BOUND : (_rcpi)) / 2) - (float) NDBM_LOW_BOUND_FOR_RCPI

bool WIFI_TEST_RSSI_05_DBM(float *RSSI)
{
    uint32_t u4Result;
    uint32_t avRcpi;

    if (!RSSI)
        return false;

    if(WIFI_TEST_get(RF_AT_FUNCID_RX_RCPI,
                0,
                NULL,
                &u4Result) == 0) {
        /* u4Result[0:7]    Average RSSI (dbM)
         * u4Result[8:15]   Minimum RCPI
         * u4Result[16:23]  Maximum RCPI
         * u4Result[24:31]  Last RCPI
         */
        avRcpi = (uint32_t)(u4Result & BITS(0,7));
        *RSSI = (float) RCPI_TO_dBm(avRcpi);
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_IsRunning(void)
{
	return fgIsTesting;
}

bool WIFI_TEST_IsUp(void)
{
    struct ifreq ifr;
    int sk, up;

    /* NULL ptr checking */
    sk = socket(PF_INET, SOCK_STREAM, 0);
    if (sk == -1) {
        return false;
    }

    strncpy(ifr.ifr_name, WIFI_IF_NAME, sizeof(ifr.ifr_name)-1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';

    /* retrieve hardware address */
    if (ioctl(sk, SIOCGIFFLAGS, &ifr) == -1) {
        close(sk);
        return false;
    }

    up = ifr.ifr_flags & IFF_UP;

    close(sk);

    if(up) {
        return true;
    }
    else {
        return false;
    }
}

/* for 11n test */
// FrameFormat        : 1 (Mixed Mode), 2 (Green field mode)
// GI (Gard interval) : 1 (Long GI), 2 (Short GI)
bool WIFI_TEST_TxDataRate11n(
    ENUM_WIFI_TEST_MCS_RATE eTxDataRate11n,
    ENUM_WIFI_TEST_PREAMBLE_TYPE ePreambleType,
    ENUM_WIFI_TEST_GI_TYPE eGIType
    )
{
    uint32_t u4Rate, u4Preamble;

    if( eTxDataRate11n > WIFI_TEST_MCS_RATE_7 && eTxDataRate11n != WIFI_TEST_MCS_RATE_32 )
    {
        DBGLOG("<failed> Unsupported MCS rate index[%u]\n", eTxDataRate11n);
        return false;
    }

    if( (ePreambleType != 0) && (ePreambleType != 1) )
    {
        DBGLOG("<failed> Unsupported N mode preamable type[%u]\n", ePreambleType);
        return false;
    }

    if( (eGIType != 0) && (eGIType != 1) )
    {
        return false;
    }

    /* specify 11n rate */
    u4Rate = RF_AT_PARAM_RATE_MCS_MASK | (uint32_t)(eTxDataRate11n);
    if(WIFI_TEST_set(RF_AT_FUNCID_RATE,
                u4Rate,
                NULL,
                NULL) != 0) {
        return false;
    }

    /* specify preamble type */
    switch(ePreambleType) {
    case WIFI_TEST_PREAMBLE_TYPE_MIXED_MODE:
        u4Preamble = RF_AT_PREAMBLE_11N_MM;
        break;

    case WIFI_TEST_PREAMBLE_TYPE_GREENFIELD:
        u4Preamble = RF_AT_PREAMBLE_11N_GF;
        break;

    /** dead code in switch **/
    /** ePreambleType only 0, or 1
    default:
        return false;
    */
    }

    if(WIFI_TEST_set(RF_AT_FUNCID_PREAMBLE,
                u4Preamble,
                NULL,
                NULL) != 0) {
        return false;
    }

    /* specify Guard Interval type */
    if(WIFI_TEST_set(RF_AT_FUNCID_GI,
                eGIType,
                NULL,
                NULL) != 0) {
        return false;
    }

	return true;
}

bool WIFI_TEST_GI(ENUM_WIFI_TEST_GI_TYPE eGIType) {
    if (eGIType != WIFI_TEST_GI_TYPE_NORMAL_GI && eGIType != WIFI_TEST_GI_TYPE_SHORT_GI) {
        return false;
    }

    /* specify Guard Interval type */
    if(WIFI_TEST_set(RF_AT_FUNCID_GI,
                eGIType,
                NULL,
                NULL) != 0) {
        return false;
    }

    return true;
}

/* for 11ac test */
// GI (Gard interval) : 1 (Long GI), 2 (Short GI)
bool WIFI_TEST_TxDataRate11ac(
    ENUM_WIFI_TEST_MCS_RATE eTxMcsRate,
    ENUM_WIFI_TEST_GI_TYPE eGIType
    )
{
    uint32_t u4Rate;

    if(eTxMcsRate > WIFI_TEST_MCS_RATE_9)
    {
        return false;
    }

    if( (eGIType != 0) && (eGIType != 1) )
    {
        return false;
    }

    /* specify MCS rate */
    u4Rate = RF_AT_PARAM_RATE_MCS_MASK | (uint32_t)(eTxMcsRate);
    if(WIFI_TEST_set(RF_AT_FUNCID_RATE,
                u4Rate,
                NULL,
                NULL) != 0) {
        return false;
    }

    if(WIFI_TEST_set(RF_AT_FUNCID_PREAMBLE,
                RF_AT_PREAMBLE_11AC,
                NULL,
                NULL) != 0) {
        return false;
    }

    /* specify Guard Interval type */
    if(WIFI_TEST_set(RF_AT_FUNCID_GI,
                eGIType,
                NULL,
                NULL) != 0) {
        return false;
    }

	return true;
}

/* for 11n test */
bool WIFI_TEST_FrequencyAccuracy(int ChannelNo)
{
    /* set channel */
    if(WIFI_TEST_Channel(ChannelNo) == false) {
        return false;
    }

    /* start carrier tone */
    if(WIFI_TEST_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_LO_LEAKAGE,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

/* for 11n test */
bool WIFI_TEST_FrequencyAccuracy_Stop()
{
    /* checking for testing mode */
    if(WIFI_TEST_IsRunning() == false) {
        if(WIFI_TEST_OpenDUT() == false) {
            return false;
        }
    }

	/* stop */
    if(WIFI_TEST_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_STOPTEST,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}


/*******************************************************************************
 * STYLE #B Control API
 *******************************************************************************
 */
/* static variables for Style #B Control API */
static ENUM_WIFI_TEST_MODE eWifiTestMode = WIFI_TEST_MODE_BY_API_CONTROL;
static ENUM_WIFI_BANDWIDTH eWifiBw = WIFI_TEST_BW_20MHZ;
static uint32_t u4NumGoodFrames = 0;
static uint32_t u4NumBadFrames = 0;

bool WIFI_TEST_SetMode(ENUM_WIFI_TEST_MODE eMode)
{
    if(eMode >= WIFI_TEST_MODE_NUM) {
        return false;
    }
    else {
        eWifiTestMode = eMode;
        return true;
    }
}

bool WIFI_TEST_GetSupportedMode(uint32_t *pu4SupportedMode) {
    uint8_t ucSupportVht = 1;

    if(pu4SupportedMode == NULL) {
        return false;
    }
    else {
        /* read fgEnable5GBand / fgSupport5GBand field from NVRAM */
        uint8_t ucSupport5GBand = 0, ucEnable5GBand = 0;
        F_INFO rFileInfo;
        int fd;
        int sz;

        rFileInfo = NVM_ReadFileVerInfo(AP_CFG_RDEB_FILE_WIFI_LID);
        fd = open(rFileInfo.cFileName, O_RDONLY|O_CREAT, S_IRUSR|S_IRGRP);
        if(fd != -1) {
            sz = lseek(fd, offsetof(WIFI_CFG_PARAM_STRUCT, ucSupport5GBand), SEEK_SET);
            if (sz == -1) {
                DBGLOG("lseek fail, errno=%s(%d)\n", strerror(errno), errno);
                close(fd);
                return false;
            }
            sz = read(fd, &ucSupport5GBand, sizeof(uint8_t));
            if (sz == -1) {
                DBGLOG("read fail, errno=%s(%d)\n", strerror(errno), errno);
                close(fd);
                return false;
            }

            sz = lseek(fd, offsetof(WIFI_CFG_PARAM_STRUCT, ucEnable5GBand), SEEK_SET);
            if (sz == -1) {
                DBGLOG("lseek fail2, errno=%s(%d)\n", strerror(errno), errno);
                close(fd);
                return false;
            }
            sz = read(fd, &ucEnable5GBand, sizeof(uint8_t));
            if (sz == -1) {
                DBGLOG("read fail2, errno=%s(%d)\n", strerror(errno), errno);
                close(fd);
                return false;
            }

            close(fd);
        }

        if(ucSupport5GBand && ucEnable5GBand) {
            *pu4SupportedMode = BIT((int)WIFI_TEST_MODE_80211A_ONLY)
                                    | BIT((int)WIFI_TEST_MODE_80211B_ONLY)
                                    | BIT((int)WIFI_TEST_MODE_80211G_ONLY)
                                    | BIT((int)WIFI_TEST_MODE_80211N_ONLY);
        }
        else {
            *pu4SupportedMode = BIT((int)WIFI_TEST_MODE_80211B_ONLY)
                                    | BIT((int)WIFI_TEST_MODE_80211G_ONLY)
                                    | BIT((int)WIFI_TEST_MODE_80211N_ONLY);
        }

        if(ucSupportVht) {
            *pu4SupportedMode |= BIT((int)WIFI_TEST_MODE_80211AC_ONLY);
        }

        return true;
    }
}

bool WIFI_TEST_SetBandwidth(ENUM_WIFI_BANDWIDTH eBandwidth)
{
    int result = 0;
    ENUM_WIFI_CHANNEL_BANDWIDTH eChBw;

    if(eBandwidth >= WIFI_TEST_BW_NUM) {
        return false;
    }

    /* Legacy/11n BW */
    if(eBandwidth < WIFI_TEST_BW_80MHZ) {
        result = WIFI_TEST_set(RF_AT_FUNCID_BANDWIDTH, (uint32_t)eBandwidth,
                        NULL, NULL);
    } else {
        switch(eBandwidth) {
        case WIFI_TEST_BW_80MHZ:
            eChBw = WIFI_TEST_CH_BW_80MHZ;
            break;

        case WIFI_TEST_BW_160MHZ:
            eChBw = WIFI_TEST_CH_BW_160MHZ;
            break;

        /* for build warning, and exception */
        default:
            return false;
        }

        result = WIFI_TEST_set(RF_AT_FUNCID_SET_CHANNEL_BANDWIDTH,
                (uint32_t)eChBw, NULL, NULL);
    }

    DBGLOG ("<WIFI_TEST_SetBandwidth> result [%x], eBandwidth[%x], g_sChannel.eDefine[%x], g_sChannel.eBW[%x], g_sChannel.u4No[%x]\n", result, eBandwidth, (int)g_sChannel.eDefine, (int)g_sChannel.eBW, (int)g_sChannel.u4No);

    if(result == 0) {
        eWifiBw = eBandwidth;

        /*check if needs re-update center channel */
        if((g_sChannel.eDefine == CHNL_DEFINE_PRIMARY_CHANNEL)&&(g_sChannel.eBW != eWifiBw))
        {
            /*bandwidth has change after WIFI_TEST_PrimaryChannel , it needs re-update center channel */
            WIFI_TEST_PrimaryChannel(g_sChannel.u4No);
        }

        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_SetBandwidthV2(ENUM_WIFI_CHANNEL_BANDWIDTH eBandwidth)
{
    int result = 0;

    if(eBandwidth >= WIFI_TEST_CH_BW_NUM) {
        return false;
    }

    result = WIFI_TEST_set(RF_AT_FUNCID_SET_CHANNEL_BANDWIDTH,
            (uint32_t)eBandwidth, NULL, NULL);

    return (result == 0)? true:false;
}

bool WIFI_TEST_SetPriChannelSetting(uint8_t ucPrimaryChSetting)
{
    int result = 0;

    if(ucPrimaryChSetting >= WIFI_TEST_PRI_CH_SETTING_NUM) {
        return false;
    }

    result = WIFI_TEST_set(RF_AT_FUNCID_SET_PRI_SETTING,
            (uint32_t)ucPrimaryChSetting, NULL, NULL);

    return (result == 0)? true:false;
}

bool WIFI_TEST_SetTxBandwidth(ENUM_WIFI_CHANNEL_BANDWIDTH eBandwidth)
{
    int result = 0;

    if(eBandwidth >= WIFI_TEST_CH_BW_NUM) {
        return false;
    }

    result = WIFI_TEST_set(RF_AT_FUNCID_SET_DATA_BANDWIDTH,
            (uint32_t)eBandwidth, NULL, NULL);

    return (result == 0)? true:false;
}

bool WIFI_TEST_SetFrequency(uint32_t u4FreqInMHz, uint32_t u4OffsetInKHz)
{
    int ChannelNo;
    if(u4OffsetInKHz != 0) {
        /* not supporting offset frequency */
        return false;
    }
    else {
        if(WIFI_TEST_set(RF_AT_FUNCID_CHNL_FREQ,
                u4FreqInMHz * 1000,
                NULL,
                NULL) == 0) {

            if(u4FreqInMHz >=2412 && u4FreqInMHz <=2472) { //Ch1~Ch13
                ChannelNo = ((u4FreqInMHz - 2412) / 5) + 1;
            } else if (u4FreqInMHz == 2484){
                ChannelNo = 14;
            } else if (u4FreqInMHz == 3665){
                ChannelNo = 133;
            } else if (u4FreqInMHz == 3685){
                ChannelNo = 137;
            } else if(u4FreqInMHz >=4915 && u4FreqInMHz <=4980) { //Ch183~Ch196
                ChannelNo = ((u4FreqInMHz - 4915) / 5) + 183;
            } else if(u4FreqInMHz >=5170 && u4FreqInMHz <=5825) { //Ch34~Ch165
                ChannelNo = ((u4FreqInMHz - 5170) / 5) + 34;
            } else {
                ChannelNo = -1;
            }

            /*success , update channel info*/
            g_sChannel.eDefine = CHNL_DEFINE_CENTER_CHANNEL;
            g_sChannel.u4No = ChannelNo;
            g_sChannel.eBW = eWifiBw; /* but BW is not important when center channel mode*/

            DBGLOG ("<WIFI_TEST_SetFrequency> g_sChannel.eDefine[%x], g_sChannel.eBW[%x], g_sChannel.u4No[%x]\n", (int)g_sChannel.eDefine, (int)g_sChannel.eBW, (int)g_sChannel.u4No);
            return true;
        }
        else {
            return false;
        }
    }
}

bool WIFI_TEST_SetRate(uint32_t u4Rate)
{
    ENUM_WIFI_TEST_GI_TYPE eGIType;
    uint32_t u4RateCode;
    ALOGD("[%s] Rate=[%d]\n", __func__, u4Rate);

    /* validate rate by mode/bandwidth settings */
    if(eWifiTestMode == WIFI_TEST_MODE_80211B_ONLY) {
        switch(u4Rate) {
        case 100:
            u4RateCode = RF_AT_PARAM_RATE_1M;
            break;

        case 200:
            u4RateCode = RF_AT_PARAM_RATE_2M;
            break;

        case 550:
            u4RateCode = RF_AT_PARAM_RATE_5_5M;
            break;

        case 1100:
            u4RateCode = RF_AT_PARAM_RATE_11M;
            break;

        default:
            return false;
        }
    }
    else if(eWifiTestMode == WIFI_TEST_MODE_80211G_ONLY || eWifiTestMode == WIFI_TEST_MODE_80211A_ONLY) {
        switch(u4Rate) {
        case 600:
            u4RateCode = RF_AT_PARAM_RATE_6M;
            break;

        case 900:
            u4RateCode = RF_AT_PARAM_RATE_9M;
            break;

        case 1200:
            u4RateCode = RF_AT_PARAM_RATE_12M;
            break;

        case 1800:
            u4RateCode = RF_AT_PARAM_RATE_18M;
            break;

        case 2400:
            u4RateCode = RF_AT_PARAM_RATE_24M;
            break;

        case 3600:
            u4RateCode = RF_AT_PARAM_RATE_36M;
            break;

        case 4800:
            u4RateCode = RF_AT_PARAM_RATE_48M;
            break;

        case 5400:
            u4RateCode = RF_AT_PARAM_RATE_54M;
            break;

        default:
            return false;
        }
    }
    else if(eWifiTestMode == WIFI_TEST_MODE_80211N_ONLY && eWifiBw == WIFI_TEST_BW_20MHZ) {
        switch(u4Rate) {
        case 650:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_0;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 720:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_0;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 1300:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_1;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 1440:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_1;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 1950:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_2;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 2170:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_2;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 2600:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_3;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 2890:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_3;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 3900:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_4;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 4330:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_4;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 5200:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_5;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 5780:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_5;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 5850:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_6;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 6500:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_7;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 7200:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_7;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        default:
            return false;
        }
    }
    else if(eWifiTestMode == WIFI_TEST_MODE_80211N_ONLY && eWifiBw == WIFI_TEST_BW_40MHZ) {
        switch(u4Rate) {
        case 1350:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_0;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 1500:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_0;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 2700:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_1;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 3000:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_1;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 4050:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_2;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 4500:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_2;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 5400:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_3;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 6000:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_3;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 8100:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_4;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 9000:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_4;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 10800:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_5;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 12000:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_5;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 12150:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_6;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 13500:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_7;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 15000:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_7;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        default:
            return false;
        }
    }
    else {
        return false;
    }

    /* set GI type */
    if(eWifiTestMode == WIFI_TEST_MODE_80211N_ONLY &&
            WIFI_TEST_set(RF_AT_FUNCID_GI,
                eGIType,
                NULL,
                NULL) != 0) {
        return false;
    }

    /* set rate code */
    if(WIFI_TEST_set(RF_AT_FUNCID_RATE,
                u4RateCode,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_SetTXPower(uint32_t u4Gain)
{
    if(u4Gain > 6350) {
        return false;
    }
    else if(WIFI_TEST_set(RF_AT_FUNCID_POWER,
            u4Gain / 50, // convert to unit of 0.5dBm
            NULL,
            NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_SetTX(bool fgEnable)
{
    if(fgEnable == true) {
        return WIFI_TEST_TxStart();
    }
    else {
        return WIFI_TEST_TxStop();
    }
}

bool WIFI_TEST_SetRX(bool fgEnable, char *aucSrcAddr, char *aucDstAddr)
{
    uint8_t aucMacAddr[4];
    uint32_t u4MacAddr;
    ENUM_RF_AT_RXOK_MATCH_RULE_T eRxRule;

    if(fgEnable == true) {
        if(aucSrcAddr && aucDstAddr) {
            eRxRule = RF_AT_RXOK_DISABLED;
        }
        else if(aucDstAddr) {
            eRxRule = RF_AT_RXOK_MATCH_RA_ONLY;
        }
        else if(aucSrcAddr) {
            eRxRule = RF_AT_RXOK_MATCH_TA_ONLY;
        }
        else {
            eRxRule = RF_AT_RXOK_DISABLED;
        }

        /* specify policy */
        if(WIFI_TEST_set(RF_AT_FUNCID_SET_RXOK_MATCH_RULE,
                    eRxRule,
                    NULL,
                    NULL) != 0) {
            return false;
        }

        if(aucDstAddr) {
            /* specify MAC address[0:3] */
            memcpy(aucMacAddr, aucDstAddr, sizeof(uint8_t) * 4);
            u4MacAddr = *(uint32_t *)(&(aucMacAddr[0]));
            if(WIFI_TEST_set(RF_AT_FUNCID_SET_MAC_DST_ADDRESS,
                        u4MacAddr,
                        NULL,
                        NULL) != 0) {
                return false;
            }

            /* specify MAC address[4:5] */
            memset(aucMacAddr, 0, sizeof(uint8_t) * 4);
            memcpy(aucMacAddr, &(aucDstAddr[4]), sizeof(uint8_t) * 2);
            u4MacAddr = *(uint32_t *)(&(aucMacAddr[0]));
            if(WIFI_TEST_set(RF_AT_FUNCID_SET_MAC_DST_ADDRESS | (4 << TEST_SET_CMD_OFFSET),
                        u4MacAddr,
                        NULL,
                        NULL) != 0) {
                return false;
            }
        }

        if(aucSrcAddr) {
            /* specify MAC address[0:3] */
            memcpy(aucMacAddr, aucSrcAddr, sizeof(uint8_t) * 4);
            u4MacAddr = *(uint32_t *)(&(aucMacAddr[0]));
            if(WIFI_TEST_set(RF_AT_FUNCID_SET_MAC_SRC_ADDRESS,
                        u4MacAddr,
                        NULL,
                        NULL) != 0) {
                return false;
            }

            /* specify MAC address[4:5] */
            memset(aucMacAddr, 0, sizeof(uint8_t) * 4);
            memcpy(aucMacAddr, &(aucSrcAddr[4]), sizeof(uint8_t) * 2);
            u4MacAddr = *(uint32_t *)(&(aucMacAddr[0]));
            if(WIFI_TEST_set(RF_AT_FUNCID_SET_MAC_SRC_ADDRESS | (4 << TEST_SET_CMD_OFFSET),
                        u4MacAddr,
                        NULL,
                        NULL) != 0) {
                return false;
            }
        }

        /* Start RX */
        //return WIFI_TEST_RxStart();
        return true;
    }
    else {
        if(aucSrcAddr && aucDstAddr) {
            eRxRule = RF_AT_RXOK_DISABLED;
        }
        else if(aucDstAddr) {
            eRxRule = RF_AT_RXOK_MATCH_RA_ONLY;
        }
        else if(aucSrcAddr) {
            eRxRule = RF_AT_RXOK_MATCH_TA_ONLY;
        }
        else {
            eRxRule = RF_AT_RXOK_DISABLED;
        }
        /* specify policy */
        if(WIFI_TEST_set(RF_AT_FUNCID_SET_RXOK_MATCH_RULE,
                eRxRule,
                NULL,
                NULL) != 0) {
            return false;
        }
        //return WIFI_TEST_RxStop();
        return true;
    }
}

bool WIFI_TEST_ClearResult(void)
{
    if(WIFI_TEST_get(RF_AT_FUNCID_RXOK_COUNT,
                0,
                NULL,
                &u4NumGoodFrames) == 0 &&
            WIFI_TEST_get(RF_AT_FUNCID_RXERROR_COUNT,
                0,
                NULL,
                &u4NumBadFrames) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_GetResult(uint32_t *pu4GoodFrameCount, uint32_t *pu4BadFrameCount)
{
    uint32_t u4GoodCnt, u4BadCnt;

    if(WIFI_TEST_get(RF_AT_FUNCID_RXOK_COUNT,
                0,
                NULL,
                &u4GoodCnt) == 0 &&
            WIFI_TEST_get(RF_AT_FUNCID_RXERROR_COUNT,
                0,
                NULL,
                &u4BadCnt) == 0) {

        if(pu4GoodFrameCount) {
            *pu4GoodFrameCount = u4GoodCnt - u4NumGoodFrames;
        }
        if(pu4BadFrameCount) {
            *pu4BadFrameCount = u4BadCnt - u4NumBadFrames;
        }

        return true;
    }
    else {
        return false;
    }
}


/* Set Rx default antenna for Rx diversity */
/* 0: main antenna */
/* 1: AUX antenna */
bool WIFI_TEST_SetRxDefaultAnt(uint8_t ucUseAuxAnt)
{
    if(WIFI_TEST_set(RF_AT_FUNCID_SET_RX_DEF_ANT,
                ucUseAuxAnt,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

/* Set Tx encoding mode */
/* 0: BCC */
/* 1: LDPC */
bool WIFI_TEST_SetTxCodingMode(uint8_t ucLdpc)
{
    if(WIFI_TEST_set(RF_AT_FUNCID_SET_TX_ENCODE_MODE,
                ucLdpc,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

/* Set J mode */
/* 0: disable */
/* 1: 5M single tone */
/* 2: 10M single tone */
bool WIFI_TEST_SetJMode(ENUM_WIFI_J_MODE eJModeSetting)
{
    if(WIFI_TEST_set(RF_AT_FUNCID_SET_J_MODE_SETTING,
                eJModeSetting,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

//for cw mode
bool WIFI_TEST_TxTraining(void)
{
    bool result = false;

    result = WIFI_TEST_TxBurstInterval(20);
    if(!result)
        return result;

    result = WIFI_TEST_TxBurstFrames(50);
    if(!result)
        return result;

    result = WIFI_TEST_TxPayloadLength(500);
    if(!result)
        return result;

    result = WIFI_TEST_TxStart();
    if(!result)
        return result;

    usleep(200000);

    result = WIFI_TEST_TxStop();
    if(!result)
        return result;

    return true;
}

bool WIFI_TEST_CW_MODE(int mode)
{
    if(WIFI_TEST_set(RF_AT_FUNCID_SET_CW_MODE,
                (uint32_t)mode,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_DPD_MODE(int mode)
{
    if(WIFI_TEST_set(RF_AT_FUNCID_DPD_MODE,
                (uint32_t)mode,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_TEMP_COMPEN_MODE(int mode)
{
    if(WIFI_TEST_set(RF_AT_FUNCID_TEMP_COMPEN,
                (uint32_t)mode,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_TX_POWER_MODE(int mode)
{
    if(WIFI_TEST_set(RF_AT_FUNCID_TX_PWR_MODE,
                (uint32_t)mode,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_CW_MODE_START(void)
{
	/* cw mode start */
    if(WIFI_TEST_TxTraining() == false) {
        return false;
    }

    if(WIFI_TEST_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_CW,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }


}

bool WIFI_TEST_init(void) {
	/* initialize skfd */
    if (skfd < 0 && (skfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        ALOGE("[%s] failed to open net socket\n", __func__);
        return false;
    }
	return true;
}

void WIFI_TEST_deinit(void) {
	/* close socket if necessary */
    if(skfd > 0) {
        close(skfd);
        skfd = -1;
    }
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This API provided a service for EFUSE query commands
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/

bool WIFI_TEST_EFUSE_Read(unsigned int offset, unsigned int *val)
{
    if (!val) {
        return false;
    }

    if (WIFI_TEST_get(RF_AT_FUNCID_READ_EFUSE,
                offset,
                NULL,
                (uint32_t *)val) != 0) {
        return false;
    }
    return true;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This API provided a service for EFUSE write commands
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/

bool WIFI_TEST_EFUSE_Write(unsigned int offset, unsigned int val)
{
    if (WIFI_TEST_set(RF_AT_FUNCID_WRITE_EFUSE | (offset << TEST_SET_CMD_OFFSET),
                val,
                NULL,
                NULL) != 0) {
        return false;
    }
    return true;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This API provided a service for MCR query commands
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/

bool WIFI_TEST_MCR_Read(unsigned int addr, unsigned int *val)
{
    int retval = -1;
    struct iwreq wrq;
    NDIS_TRANSPORT_STRUCT rNdisStruct;
    P_PARAM_CUSTOM_MCR_RW_STRUC_T prMCRStruct;

    prMCRStruct = (P_PARAM_CUSTOM_MCR_RW_STRUC_T)rNdisStruct.ndisOidContent;
    /* zeroize */
    memset(&wrq, 0, sizeof(struct iwreq));

    prMCRStruct->u4McrOffset = addr;
    /* zeroize */
    rNdisStruct.ndisOidCmd = OID_CUSTOM_MCR_RW;
    rNdisStruct.inNdisOidlength = sizeof(PARAM_CUSTOM_MCR_RW_STRUC_T);
    rNdisStruct.outNdisOidLength = sizeof(PARAM_CUSTOM_MCR_RW_STRUC_T);

    wrq.u.data.pointer = &rNdisStruct;
    wrq.u.data.length = sizeof(NDIS_TRANSPORT_STRUCT);
    wrq.u.data.flags = PRIV_CMD_OID;

    retval = ioctl_iw_ext(IOCTL_GET_STRUCT, &wrq);

    if (retval == 0) {
        *val = prMCRStruct->u4McrData;
    }

	return !retval;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This API provided a service for MCR set commands
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/

bool WIFI_TEST_MCR_Write(unsigned int addr, unsigned int val)
{
    int retval = -1;
    struct iwreq wrq;
    NDIS_TRANSPORT_STRUCT rNdisStruct;
    P_PARAM_CUSTOM_MCR_RW_STRUC_T prMCRStruct;

    prMCRStruct = (P_PARAM_CUSTOM_MCR_RW_STRUC_T)rNdisStruct.ndisOidContent;
    /* zeroize */
    memset(&wrq, 0, sizeof(struct iwreq));

    prMCRStruct->u4McrOffset = addr;
    prMCRStruct->u4McrData = val;
    /* zeroize */
    rNdisStruct.ndisOidCmd = OID_CUSTOM_MCR_RW;
    rNdisStruct.inNdisOidlength = sizeof(PARAM_CUSTOM_MCR_RW_STRUC_T);
    rNdisStruct.outNdisOidLength = sizeof(PARAM_CUSTOM_MCR_RW_STRUC_T);

    wrq.u.data.pointer = &rNdisStruct;
    wrq.u.data.length = sizeof(NDIS_TRANSPORT_STRUCT);
    wrq.u.data.flags = PRIV_CMD_OID;

    retval = ioctl_iw_ext(IOCTL_SET_STRUCT, &wrq);

    return !retval;
}
/* WIFI_TEST_PrimaryChannel : Set the channel (for primary channel )*/
bool WIFI_TEST_PrimaryChannel(int ChannelNo)
{
    uint8_t u1ChannelOffset = 0;
    bool bRet;
#if 0
    ENUM_CHNL_EXT_T eRfSco = CHNL_EXT_SCA; /*this function default assume CHNL_EXT_SCA*/
#endif

    /*Set center channel , WIFI_TEST_Channel input argument needs center channel*/
    switch(eWifiBw)
    {
    case WIFI_TEST_BW_20MHZ:
        u1ChannelOffset = 0;
        break;
    case WIFI_TEST_BW_40MHZ:
        u1ChannelOffset = 2;
        break;
    case WIFI_TEST_BW_80MHZ:
        u1ChannelOffset = 4;
        break;
    case WIFI_TEST_BW_160MHZ:
        u1ChannelOffset = 8;
        break;
    default:
        u1ChannelOffset = 0;
        break;
    }
#if 0
    switch(eRfSco)
    {
    case CHNL_EXT_SCA:
        bRet = WIFI_TEST_Channel(ChannelNo+u1ChannelOffset);
        break;
    case CHNL_EXT_SCB:
        bRet = WIFI_TEST_Channel(ChannelNo-u1ChannelOffset);
        break;
    case CHNL_EXT_SCN:
    default:
        bRet = WIFI_TEST_Channel(ChannelNo);
        break;
    }
#else
    bRet = WIFI_TEST_Channel(ChannelNo+u1ChannelOffset);
#endif
    if(bRet == true)
    {
        /*success , update channel info , care channel info don't overrides by WIFI_TEST_Channel*/
        g_sChannel.eDefine = CHNL_DEFINE_PRIMARY_CHANNEL;
        g_sChannel.u4No = ChannelNo;
        g_sChannel.eBW = eWifiBw;
    }

    DBGLOG ("<WIFI_TEST_PrimaryChannel> g_sChannel.eDefine[%x], g_sChannel.eBW[%x], g_sChannel.u4No[%x]\n", (int)g_sChannel.eDefine, (int)g_sChannel.eBW, (int)g_sChannel.u4No);

    return bRet;
}


bool WIFI_TEST_Channel(int ChannelNo)
{
    uint32_t u4Freq;

    if(ChannelNo < 0) {
        return false; /* invalid channel number */
    }
    /* 2.4GHz band */
    else if(ChannelNo <= 13) {
        u4Freq = 2412000 + (ChannelNo - 1) * 5000;
    }
    else if(ChannelNo == 14) {
        u4Freq = 2484000;
    }
    /* 5GHz band */
    else if(ChannelNo >= 36) {
        u4Freq = 5180000 + (ChannelNo - 36) * 5000;
    }
    else {
        return false; /* invalid channel number */
    }

    DBGLOG ("<WIFI_TEST_Channel> u4Freq[%x]\n", (int)u4Freq);

    if(WIFI_TEST_set(RF_AT_FUNCID_CHNL_FREQ,
            u4Freq,
            NULL,
            NULL) == 0) {
        /*success , update channel info*/
        g_sChannel.eDefine = CHNL_DEFINE_CENTER_CHANNEL;
        g_sChannel.u4No = ChannelNo;
        g_sChannel.eBW = eWifiBw; /* but BW is not important when center channel mode*/

        DBGLOG ("<WIFI_TEST_Channel> g_sChannel.eDefine[%x], g_sChannel.eBW[%x], g_sChannel.u4No[%x]\n", (int)g_sChannel.eDefine, (int)g_sChannel.eBW, (int)g_sChannel.u4No);

        return true;
    }
    else {
        return false;
    }
}

