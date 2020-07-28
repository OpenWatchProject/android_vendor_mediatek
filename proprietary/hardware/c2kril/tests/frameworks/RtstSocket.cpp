/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <cutils/log.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include "RtstSocket.h"
#include "RtstUtils.h"
#include "RtstLog.h"
/*****************************************************************************
 * Defines
 *****************************************************************************/
#define TAG "RTF"

/*****************************************************************************
 * class RfxTestSocket
 *****************************************************************************/
RtstFd::RtstFd(int fd) : mFd(fd){
}


RtstFd::~RtstFd() {
    if (mFd != -1) {
        RTST_LOG_V(TAG, "~RtstFd() [%d]", mFd);
        close(mFd);
    }
}


int RtstFd::write(void* buf, int len)  const {
    int n, retry = 10;

    if(mFd < 0 || buf == NULL || len < 0) {
        RTST_LOG_E(TAG, "write fd=%d buf=%p len=%d\n",
            mFd, buf, len);
        return -1;
    }

    while((n = ::write(mFd, buf, len)) != len) {
        if(errno == EINTR) continue;
        if(errno == EAGAIN) {
            if(retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
            goto exit;
        }
        goto exit;
    }
    RtstUtils::dumpToHexFormatLog("", (const char *)buf, len);
    RTST_LOG_V(TAG, "RtstSocket::write [fd = %d][len = %d]", mFd, len);
    return n;
exit:
    RTST_LOG_E(TAG, "write reason=[%s]%d\n", strerror(errno), errno);
    return -1;
}


int RtstFd::read(void* buf, int len) const {
    int n, retry = 10;

    if(mFd < 0 || buf == NULL || len < 0) {
        RTST_LOG_E(TAG, "read fd=%d buf=%p len=%d\n",
            mFd, buf, len);
        return -1;
    }

    if(len == 0) {
        return 0;
    }

    while((n = ::read(mFd, buf, len)) < 0) {
        if(errno == EINTR) continue;
        if(errno == EAGAIN) {
            if(retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
            goto exit;
        }
        goto exit;
    }
    RtstUtils::dumpToHexFormatLog("", (const char *)buf, n);
    RTST_LOG_V(TAG, "RtstSocket::read [fd = %d][len = %d]", mFd, n);
    return n;

exit:
    if(errno != EAGAIN) {
        RTST_LOG_E(TAG, "read reason=[%s] fd=%d len=%d buf=%p\n",
            strerror(errno), mFd, len, buf);
    }
    return -1;
}


/*****************************************************************************
 * Class RtstSocketPair
 *****************************************************************************/
 RtstSocketPair::RtstSocketPair() : m_s1(-1), m_s2(-1){
    int fd[2];
    int err = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    if (err < 0) {
        RTST_LOG_E(TAG, "RtstSocketPair Error[%s]",
                strerror(errno));
        return;
    }
    RTST_LOG_V(TAG, "RtstSocketPair [%d][%d]", fd[0], fd[1]);
    m_s1.setFd(fd[0]);
    m_s2.setFd(fd[1]);
}