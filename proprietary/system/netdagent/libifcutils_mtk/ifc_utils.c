/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software/firmware and related documentation ("MediaTek Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to MediaTek Inc. and/or its licensors. Without
*  the prior written permission of MediaTek inc. and/or its licensors, any
*  reproduction, modification, use or disclosure of MediaTek Software, and
*  information contained herein, in whole or in part, shall be strictly
*  prohibited.
*  MediaTek Inc. (C) [2017]. All rights reserved.
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
*  ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
*  WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
*  NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
*  RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
*  INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
*  TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
*  RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
*  OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
*  SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
*  RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
*  STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
*  ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
*  RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
*  MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
*  CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*****************************************************************************/

#define LOG_TAG "mtk_ifc"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <log/log.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <ifcutils/ifc.h>


#define SIOCSTXQSTATE (SIOCDEVPRIVATE + 0)  //start/stop ccmni tx queue
#define SIOCSCCMNICFG (SIOCDEVPRIVATE + 1)  //configure ccmni/md remapping
#define SIOCACKPRIO  (SIOCDEVPRIVATE + 3)  //disable ack first mechanism

int ifc_set_txq_state(const char *ifname, int state)
{
    struct ifreq ifr;
    int ret, ctl_sock;

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;
    ifr.ifr_ifru.ifru_ivalue = state;

    ctl_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(ctl_sock < 0){
	   ALOGE("create ctl socket failed\n");
       return -1;
    }
    ret = ioctl(ctl_sock, SIOCSTXQSTATE, &ifr);
    if(ret < 0)
       ALOGE("ifc_set_txq_state failed, err:%d(%s)\n", errno, strerror(errno));
    else
       ALOGI("ifc_set_txq_state as %d, ret: %d\n", state, ret);

    close(ctl_sock);

    return ret;
}

int ifc_ccmni_md_cfg(const char *ifname, int md_id)
{
    struct ifreq ifr;
    int ret = 0;
    int ctl_sock = 0;

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;
    ifr.ifr_ifru.ifru_ivalue = md_id;

    ctl_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(ctl_sock < 0){
        ALOGE("ifc_ccmni_md_cfg: create ctl socket failed\n");
        return -1;
    }

    if(ioctl(ctl_sock, SIOCSCCMNICFG, &ifr) < 0) {
        ALOGE("ifc_ccmni_md_configure(ifname=%s, md_id=%d) error:%d(%s)", \
             ifname, md_id, errno, strerror(errno));
       ret = -1;
    } else {
        ALOGI("ifc_ccmni_md_configure(ifname=%s, md_id=%d) OK", ifname, md_id);
    }

    close(ctl_sock);
    return ret;
}


int ifc_enable_ack_prio(int enable)
{
    struct ifreq ifr;
    int ret, ctl_sock;

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, "ccmni0", IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;
    ifr.ifr_ifru.ifru_ivalue = enable;

    ctl_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(ctl_sock < 0){
        ALOGE("create ctl socket failed\n");
       return -1;
    }
    ret = ioctl(ctl_sock, SIOCACKPRIO, &ifr);
    if(ret < 0)
       ALOGE("ifc_set_ack_prio failed, err:%d(%s)\n", errno, strerror(errno));
    else
       ALOGI("ifc_set_ack_prio as %d, ret: %d\n", enable, ret);

    close(ctl_sock);

    return ret;
}

