/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
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

/*
 * Declarations for internal use only.
 * Don't include this file outside PAL.
 */

#ifndef PAL_INTERNAL_H_
#define PAL_INTERNAL_H_

#include <math.h>
#include <pal.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/cdefs.h>
#include <utils/Log.h>

#define UNUSED(__X__) (void)(__X__)

#define _SHORT_FILE_ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define PRINT_DBG(__FMT__, ...)        \
    ALOGD("%15s:%04d:%15s: " __FMT__, \
        _SHORT_FILE_, __LINE__,     \
        __FUNCTION__, ##__VA_ARGS__); \

#define PRINT_WRN(__FMT__, ...)        \
    ALOGW("%15s:%04d:%15s: " __FMT__, \
        _SHORT_FILE_, __LINE__,     \
        __FUNCTION__, ##__VA_ARGS__); \

#define BZEROTYPE(__VALUE__) bzero((void*)(&(__VALUE__)), sizeof(__VALUE__))

#define OMADM_SRV_READY()               \
    if (!OmadmServiceAPI::isPalInitialized()) { \
        PRINT_WRN("RESULT_ERROR_PAL_NO_INIT");     \
        return RESULT_ERROR_PAL_NO_INIT;          \
    }                                             \

__BEGIN_DECLS

static inline int log10i(int arg) {
    int cntr = 0;
    while (arg >= 10) {
        arg /= 10;
        cntr ++;
    }
    return cntr;
}

static inline int print_size_10(int arg) {
  if (arg < 0) {
    arg *= 10;
  }
  arg = abs(arg);
  arg ++;
  arg = log10i(arg) + 1;
  return arg;
}

char* pal_strcat_path(const char* arg0, const char* arg1, char** out);

int pal_is_path_dir(char* path);


__END_DECLS

#define CHECK_BUFFER(__BUFF__) {              \
    if ((__BUFF__) == NULL) {                 \
        PRINT_WRN("&data_buffer_t is null");   \
        return RESULT_ERROR_INVALID_ARGS;     \
    }                                         \
    if ((__BUFF__)->data == NULL) {           \
      PRINT_WRN("data_buffer_t.data is null"); \
        return RESULT_BUFFER_NOT_DEFINED;     \
    }                                         \
    PRINT_DBG("size = %d", (__BUFF__)->size);  \
}                                             \

#define CHECK_PVALUE(__PVALUE__) {            \
    if ((__PVALUE__) == NULL) {               \
        PRINT_WRN(#__PVALUE__" is null");      \
        return RESULT_ERROR_INVALID_ARGS;     \
    }                                         \
}                                             \

#define CHECK_AND_SET_BUFFER_INT2STR(__BUFF__, __ARG_INT__) {        \
    CHECK_BUFFER(__BUFF__);                                          \
    uint_t required_size = print_size_10(__ARG_INT__) + 1;           \
    if ((__BUFF__)->size < required_size) {                          \
        (__BUFF__)->size = required_size;                            \
        return RESULT_BUFFER_OVERFLOW;                               \
    }                                                                \
    snprintf((__BUFF__)->data, (__BUFF__)->size, "%d", __ARG_INT__); \
}                                                                    \

#define CHECK_AND_SET_BUFFER_STR2STR(__BUFF__, __ARG_STR__) { \
    CHECK_BUFFER(__BUFF__);                                   \
    uint_t required_size = strlen(__ARG_STR__) + 1;           \
    if ((__BUFF__)->size < required_size) {                   \
        (__BUFF__)->size = required_size;                     \
        return RESULT_BUFFER_OVERFLOW;                        \
    }                                                         \
    strncpy((__BUFF__)->data, __ARG_STR__, (__BUFF__)->size); \
}                                                             \

#define FREE_IF(__POINTER__)     \
    if (NULL != (__POINTER__)) { \
      free(__POINTER__);         \
      (__POINTER__) = NULL;      \
    }

#endif /* PAL_INTERNAL_H_ */
