/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "AFMServer"

#define MTK_LOG_ENABLE 1
#include <cutils/log.h>
#include "server.h"
#include "RPCClient.h"

AFMServer::AFMServer() {
    m_bRunning = false;
}
AFMServer::~AFMServer() {
}


void AFMServer::RunAsDaemon() {
    bool r = Create(0, SOCK_STREAM, "EngineerModeServer"); //org "127.0.0.1" 37121
    if (!r) {
        ALOGD("create server socket failure\n");
        return;
    }

    int rc = listen(m_hSocket, 5);
    if (rc) {
        ALOGD("listen() fail; errno:%d, err_msg:%s\n", errno, strerror(errno));
        return;
    }

    m_bRunning = true;
    int failCount = 0;
    while (true) {
        if (!m_bRunning) {
            ALOGD("EM SERVER exit Daemon\n");
            OnClose(failCount);
            break;
        }
        r = AcceptConnection();
        if (!r) {
            failCount++;
            ALOGD("EM SERVER fail to listen connection\n");
        }
        if (failCount >= 5) {
            m_bRunning = false;
            ALOGD("the fail count to listen connection is overflow the max count\n");
        }
    }
}

bool AFMServer::AcceptConnection() {
    int rc = 0;
    fd_set readset;

    FD_ZERO(&readset);
    FD_SET(m_hSocket, &readset);

    rc = select(m_hSocket + 1, &readset, NULL, NULL, NULL);

    if (rc > 0) {
        OnAccept(errno);
    } else {
        ALOGD("Select return abnormal rc:%d, errno:%d, err_msg:%s", rc, errno, strerror(errno));
    }
    return rc > 0 ? true : false;

}

void AFMServer::OnAccept(int) {
    RPCClient* rpc = new RPCClient();
    if (0 == rpc)
        return;

    if (!Accept(*rpc)) {
        delete rpc;
        return;
    }

    ALOGD("OnAccept OK!\n");
    return;
}

