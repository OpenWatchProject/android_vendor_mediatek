/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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
#ifndef MLOG_H
#define MLOG_H

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

#define TO_DATA     (0)
#define TO_SDCARD   (1)
#define TO_BOTH     (2)

#define MLOG(type, ...) __mb_log(type, __VA_ARGS__)
#define MLOGE(type, ...)    \
    ({                      \
        MLOG(type, __VA_ARGS__);    \
        ALOGE(__VA_ARGS__); \
     })

#define MLOGI(type, ...)    \
    ({                      \
        MLOG(type, __VA_ARGS__);    \
        ALOGI(__VA_ARGS__); \
     })

#ifndef MBLOG_DEBUG
#define MLOGD(...)          ((void)0)
#else
#define MLOGD(...)          MLOGE(TO_DATA, __VA_ARGS__)
#endif
#define MLOGE_DATA(...)     MLOGE(TO_DATA, __VA_ARGS__)
#define MLOGI_DATA(...)     MLOGI(TO_DATA, __VA_ARGS__)
#define MLOGE_SDCARD(...)   MLOGE(TO_SDCARD, __VA_ARGS__)
#define MLOGI_SDCARD(...)   MLOGI(TO_SDCARD, __VA_ARGS__)
#define MLOGE_BOTH(...)     MLOGE(TO_BOTH, __VA_ARGS__)
#define MLOGI_BOTH(...)     MLOGI(TO_BOTH, __VA_ARGS__)

#define WARN_CONFIG_FAIL    0
#define WARN_SOCKET_FAIL    1
#define WARN_LISTEN_FAIL    2
#define WARN_PTHREAD_FAIL   3
#define WARN_COPY_FAIL      4
#define WARN_OUTPUT_FAIL    5
#define WARN_READLOG_FAIL   6
#define WARN_SHM_FAIL       7
#define WARN_FORMAT_FAIL    8
#define WARN_ROTATE_FAIL    9
#define WARN_DEVICE_FAIL    10
#define WARN_SDFULL_FAIL    11
#define WARN_TIMEOUT_FAIL   12

void __mb_log(int type, const char *fmt, ...);

#endif
