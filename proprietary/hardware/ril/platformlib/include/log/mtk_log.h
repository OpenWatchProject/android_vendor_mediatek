/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
#ifndef __MTK_LOG_H
#define __MTK_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

void mtkLogD(const char *tag, const char *fmt, ...);
void mtkLogI(const char *tag, const char *fmt, ...);
void mtkLogV(const char *tag, const char *fmt, ...);
void mtkLogW(const char *tag, const char *fmt, ...);
void mtkLogE(const char *tag, const char *fmt, ...);
void mtkAssert(char *pErrMsg);

#undef SLOGD
#undef SLOGW
#undef SLOGE
#undef SLOGV
#undef SLOGI

#define SLOGD(...) (mtkLogD(LOG_TAG, __VA_ARGS__))
#define SLOGW(...) (mtkLogW(LOG_TAG, __VA_ARGS__))
#define SLOGE(...) (mtkLogE(LOG_TAG, __VA_ARGS__))
#define SLOGV(...) (mtkLogV(LOG_TAG, __VA_ARGS__))
#define SLOGI(...) (mtkLogI(LOG_TAG, __VA_ARGS__))

#undef ALOGD
#undef ALOGE
#undef ALOGW
#undef ALOGV
#undef ALOGI

#define ALOGD(...) (mtkLogD(LOG_TAG, __VA_ARGS__))
#define ALOGE(...) (mtkLogE(LOG_TAG, __VA_ARGS__))
#define ALOGW(...) (mtkLogW(LOG_TAG, __VA_ARGS__))
#define ALOGV(...) (mtkLogV(LOG_TAG, __VA_ARGS__))
#define ALOGI(...) (mtkLogI(LOG_TAG, __VA_ARGS__))

#undef LOG_ALWAYS_FATAL_IF
#define LOG_ALWAYS_FATAL_IF(cond, ...)          \
    do {                                        \
        if (cond) {                             \
            mtkLogE(LOG_TAG, __VA_ARGS__);      \
            mtkAssert(NULL);                    \
            exit(0);                            \
        }                                       \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif /* __MTK_LOG_H */

