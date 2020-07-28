/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#ifndef __COMMON_VOIP_LOGGING_H__
#define __COMMON_VOIP_LOGGING_H__

/**
 * @brief the logging message type
 */

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#ifndef __PRODUCTION_RELEASE__
  #define ENABLED_ASSERT
  #define ENABLED_FULL_LOG
#endif

/* ------------------------ */
/*  log                     */
/* ------------------------ */
#define DEBUG_MSG_TAG       ""

#ifdef __ANDROID__

  #include <android/log.h>
  #define SRC_FILE_OFFSET           (0)
  #ifdef ENABLED_FULL_LOG
    #define TrcMsg(message, ...)       __android_log_print(ANDROID_LOG_VERBOSE, "Rcs lib " DEBUG_MSG_TAG, message " (%s:%d)",  ##__VA_ARGS__, &__FILE__[SRC_FILE_OFFSET], __LINE__)
    #define DbgMsg(message, ...)       __android_log_print(ANDROID_LOG_DEBUG,   "Rcs lib " DEBUG_MSG_TAG, message " (%s:%d)",  ##__VA_ARGS__, &__FILE__[SRC_FILE_OFFSET], __LINE__)
  #else
    #define TrcMsg(message, ...)
    #define DbgMsg(message, ...)
  #endif
  #define ErrMsg(message, ...)       __android_log_print(ANDROID_LOG_ERROR,   "Rcs lib " DEBUG_MSG_TAG, message " (%s:%d)",  ##__VA_ARGS__, &__FILE__[SRC_FILE_OFFSET], __LINE__)
  #define SysMsg(message, ...)       __android_log_print(ANDROID_LOG_INFO,    "Rcs lib " DEBUG_MSG_TAG, message " (%s:%d)",  ##__VA_ARGS__, &__FILE__[SRC_FILE_OFFSET], __LINE__)
  #define SipMsg(message, ...)       __android_log_print(ANDROID_LOG_INFO,    "Rcs lib " DEBUG_MSG_TAG, message "",  ##__VA_ARGS__)

#else

  #include <stdio.h>
  #include <time.h>

  #ifdef ENABLED_FULL_LOG
  #define TrcMsg(f, a...)   printf("(%ld) [TRC] Rcs lib %s: " f " (%s:%d)\n", time(0), DEBUG_MSG_TAG, ## a, &__FILE__[0], __LINE__)
  #define DbgMsg(f, a...)   printf("(%ld) [DBG] Rcs lib %s: " f " (%s:%d)\n", time(0), DEBUG_MSG_TAG, ## a, &__FILE__[0], __LINE__)
  #else
    #define TrcMsg(f, a...)
    #define DbgMsg(f, a...)
  #endif
#define SipMsg(f, a...)\
    printf("(%ld) [SIP] VoLTE %s: " f " (%s:%d)\n", time(0), DEBUG_MSG_TAG, ## a, &__FILE__[0], __LINE__)
#define ErrMsg(f, a...)   printf("(%ld) [ERR] Rcs lib %s: " f " (%s:%d)\n", time(0), DEBUG_MSG_TAG, ## a, &__FILE__[0], __LINE__)
#define SysMsg(f, a...)   printf("(%ld) [SYS] Rcs lib %s: " f " (%s:%d)\n", time(0), DEBUG_MSG_TAG, ## a, &__FILE__[0], __LINE__)

#endif


/* ------------------------ */
/*  assert                  */
/* ------------------------ */

#ifdef ENABLED_ASSERT
  #include <stdio.h>
  #include <stdlib.h>
  #include <assert.h>

#ifdef __ANDROID__
  #include <utils/Log.h>
#else
  #define LOG_ALWAYS_FATAL(x)
#endif

  #define VOIP_ASSERT(expr, M)                      while(!(expr)) { ErrMsg("@@@ ASSERT @@@: %s (%s:%d)\n", M, __FILE__, __LINE__); fflush(stdout); LOG_ALWAYS_FATAL("ASSERT!!!!"); assert(0); exit(-11); }
#else
  #define VOIP_ASSERT(expr, M)                      while(!(expr)) { ErrMsg("@@@ ASSER @@@: %s (%s:%d)", M, __FILE__, __LINE__); fflush(stdout); exit(-11); } 
#endif

#define VOIP_CHECK_AND_RETURN_VAL(_c_, _r_)         if ((_c_)) return (_r_) ///< if (_c_) is true, return (_r_)
#define VOIP_CHECK_AND_RETURN(_c_)                  if ((_c_)) return       ///< if (_c_) is true, return

#define VOIP_ABNORMAL_RETURN_VAL(_c_, _r_)        if ((_c_)) {VOIP_ASSERT(0, "ABNORMAL_RETURN");return (_r_);}
#define VOIP_ABNORMAL_RETURN(_c_)                 if ((_c_)) {VOIP_ASSERT(0, "ABNORMAL_RETURN");return;}
#define VOIP_ABNORMAL_BREAK(_c_)                  if ((_c_)) {VOIP_ASSERT(0, "ABNORMAL_BREAK");break;}
#define VOIP_ABNORMAL_GOTO(_c_, _l_)              if ((_c_)) {VOIP_ASSERT(0, "ABNORMAL_GOTO");goto _l_;}
#define VOIP_ABNORMAL_CHECK(_c_)                  if ((_c_)) {VOIP_ASSERT(0, "ABNORMAL_CHECK");}

#endif /* __COMMON_VOIP_LOGGING_H__ */
