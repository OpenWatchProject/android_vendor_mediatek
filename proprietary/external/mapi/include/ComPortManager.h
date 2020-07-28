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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef __COM_PORT_MANAGER_H__
#define __COM_PORT_MANAGER_H__

#include "IoChannel.h"
#include "mdi_util.h"

class ComPortManager: public IoChannel {
private:
    int m_usbPortFd;
    const char *m_vcomDevName;

    bool SetupProperty()
    {
        int count = 100;
        char tmp[256] = {0};

        property_get(PROP_SYS_USB_CONFIG, tmp, "");
        MDI_LOGD(TAG, "%s=%s", PROP_SYS_USB_CONFIG, tmp);

        property_get(PROP_SYS_USB_IDX, tmp, "");
        MDI_LOGD(TAG, "%s=%s", PROP_SYS_USB_IDX, tmp);

        if (NULL != strchr(tmp, '4')) { // 4 in sys.usb.acm_idx means that acm port is mapped to ttyGS3
            return true;
        }

        property_set(PROP_SYS_USB_CONFIG, "gs3");

        property_get(PROP_SYS_USB_CONFIG, tmp, "");
        MDI_LOGD(TAG, "new %s=%s", PROP_SYS_USB_CONFIG, tmp);

        property_get(PROP_SYS_USB_IDX, tmp, "");
        while((tmp[0] == 0 || strchr(tmp, '4') == NULL) && count-- > 0) {
            MDI_LOGE(TAG, "wait for sys.usb.config to take effect");
            sleep(1);
            property_get(PROP_SYS_USB_IDX, tmp, "");
        }
        MDI_LOGD(TAG, "new %s=%s", PROP_SYS_USB_IDX, tmp);
        if (count == 0) {
            return false;
        }
        return true;
    }
    bool OpenComDev()
    {
        struct termios termOptions;

        MDI_LOGD(TAG, "ComPortManager::Open");

        Close();

        if (0 > (m_usbPortFd = open(m_vcomDevName, O_RDWR | O_NOCTTY))) {
            MDI_LOGE(TAG, "Failed to open dev [%s]. errno = [%d](%s)", m_vcomDevName, errno, strerror(errno));
            return false;
        }

        MDI_LOGD(TAG, "Open dev [%s] successfully.", m_vcomDevName);

        bzero(&termOptions, sizeof(termOptions));
        fcntl(m_usbPortFd, F_SETFL, 0);

        // Get the current options:
        tcgetattr(m_usbPortFd, &termOptions);

        // Raw mode
        termOptions.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF | IXANY);
        termOptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /*raw input*/
        termOptions.c_oflag &= ~OPOST; /*raw output*/

        tcflush(m_usbPortFd, TCIFLUSH); //clear input buffer
        termOptions.c_cc[VTIME] = 1; /* inter-character timer unused */
        termOptions.c_cc[VMIN] = 0; /* blocking read until 0 character arrives */

        tcsetattr(m_usbPortFd, TCSANOW, &termOptions);
        return true;
    }
    void Close()
    {
        MDI_LOGD(TAG, "ComPortManager::Close");
        if (m_usbPortFd != -1) {
            if (0 != close(m_usbPortFd)) {
                MDI_LOGE(TAG, "Failed to close port [%d], errno = [%d](%s)", m_usbPortFd, errno, strerror(errno));
            } else {
                MDI_LOGD(TAG, "Port [%d] was successfully closed.", m_usbPortFd);
            }
            m_usbPortFd = -1;
        }
    }
    bool Open()
    {
        if (!SetupProperty()) {
            MDI_LOGE(TAG, "Failed to setup property.");
            return false;
        }
        if (!OpenComDev()) {
            MDI_LOGE(TAG, "Failed to open COM dev [%s]", m_vcomDevName);
            return false;
        }
        return true;
    }

public:
    ComPortManager():m_usbPortFd(-1), m_vcomDevName("/dev/ttyGS3")
    {
        // Stop logging
        system("am broadcast -a com.mediatek.mtklogger.ADB_CMD -e cmd_name stop --ei cmd_target 2");

        // Switch to USB mode
        system("am broadcast -a com.mediatek.mtklogger.ADB_CMD -e cmd_name switch_modem_log_mode --ei cmd_target 2");

        // Start logging
        system("am broadcast -a com.mediatek.mtklogger.ADB_CMD -e cmd_name start --ei cmd_target 2");

        Open();
    }
    virtual ~ComPortManager()
    {
        Close();
    }

    bool Write(const uint8_t *pData, size_t len)
    {
        size_t bytes_left = len;
        const uint8_t *pCurrData = pData;

        MDI_LOGD(TAG, "ComPortManager::Write");

        if (0 == len) {
            return true;
        }

        if (NULL == pData) {
            MDI_LOGE(TAG, "pData is NULL");
            return false;
        }

        while (bytes_left > 0 && !gbTerminate) {
            ssize_t bytes_written = write(m_usbPortFd, pCurrData, bytes_left);

            if (bytes_written == -1) {
                if (errno == EINTR) {
                    MDI_LOGE(TAG, "write failed: EINTR. Retry.");
                    continue;
                }
                if (errno == EAGAIN) {
                    MDI_LOGE(TAG, "write failed: EAGAIN. Retry.");
                    continue;
                }
                if (errno == EBADF || errno == EPIPE) {
                    MDI_LOGE(TAG, "write failed: EBADF or EPIPE. Retry.");
                    Open();
                    continue;
                }
                MDI_LOGE(TAG, "write failed: errno = %d (%s), total len = [%zu], left = [%zu]", errno, strerror(errno), len, bytes_left);
                return false;
            }
            pCurrData += bytes_written;
            bytes_left -= bytes_written;
        }

        MDI_LOGD(TAG, "Write [%zu] bytes to PC", len);

        if (0 == bytes_left) {
            return true;
        }
        return false;
    }
};

#endif