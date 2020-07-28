/*
 * Copyright (c) 2019 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __MTK_TEE_TLSTD_H__
#define __MTK_TEE_TLSTD_H__


#if defined(__TRUSTONIC_TEE__)
//==============================================================================
// __TRUSTONIC_TEE__
//==============================================================================

#pragma message ( "TRUSTONIC_TEE" )
#include "tlStd.h"
#include "tee_error.h"

#elif defined(__MICROTRUST_TEE__)
//==============================================================================
// __MICROTRUST_TEE__
//==============================================================================

#pragma message ( "MICROTRUST_TEE" )

#define DECLARE_TRUSTLET_MAIN_STACK(_size_)
#define DECLARE_TRUSTED_APPLICATION_MAIN_STACK  DECLARE_TRUSTLET_MAIN_STACK

#ifndef EOL
    #define EOL "\n"
#endif

#if !defined(TRUE)
    #define TRUE    (1==1)
#endif // !defined(TRUE)

#if !defined(FALSE)
    #define FALSE   (1!=1)
#endif // !defined(TRUE)

#ifndef NULL
#  ifdef __cplusplus
#     define NULL  0
#  else
#     define NULL  ((void *)0)
#  endif
#endif

#define IN
#define OUT

//#define TLAPI_OK                            0x00000000
#define TEE_ALLOCATION_HINT_ZEROED           0x00000000
//#define TEE_MALLOC_FILL_ZERO                 TEE_ALLOCATION_HINT_ZEROED


#define TEE_DbgPrintLnf(...)      do{TEE_DbgPrintf(__VA_ARGS__);TEE_DbgPrintf(EOL);}while(FALSE)

#elif defined(__TRUSTY_TEE__)
//==============================================================================
// __TRUSTY_TEE__
//==============================================================================

#pragma message ( "TRUSTY_TEE" )

#elif defined(__BLOWFISH_TEE__)
//==============================================================================
// __BLOWFISH_TEE__
//==============================================================================

#pragma message ( "BLOWFISH_TEE" )

#include <stdint.h>
#include <stdio.h>

//==============================================================================
// C/C++ compatibility
//==============================================================================

#if defined(__cplusplus)

    #define _EXTERN_C                extern "C"
    #define _BEGIN_EXTERN_C          extern "C" {
    #define _END_EXTERN_C            }

#else

    #define _EXTERN_C
    #define _BEGIN_EXTERN_C
    #define _END_EXTERN_C

#endif // defined(__cplusplus)

#define _NORETURN           __attribute__((noreturn))
#define _TLAPI_EXTERN_C     _EXTERN_C
#define _TLAPI_NORETURN     _NORETURN

#if !defined(TRUE)
    #define TRUE    (1==1)
#endif // !defined(TRUE)

#if !defined(FALSE)
    #define FALSE   (1!=1)
#endif // !defined(TRUE)

#ifndef NULL
#  ifdef __cplusplus
#     define NULL  0
#  else
#     define NULL  ((void *)0)
#  endif
#endif

typedef void        *void_ptr;  /**< a pointer to anything. */
typedef void_ptr    addr_t;     /**< an address, can be physical or virtual */
typedef uint32_t    bool_t;     /**< boolean data type. */

#define DECLARE_TRUSTLET_MAIN_STACK(_size_)
#define DECLARE_TRUSTED_APPLICATION_MAIN_STACK  DECLARE_TRUSTLET_MAIN_STACK

#ifndef EOL
    #define EOL "\n"
#endif

#define IN
#define OUT

#define TEE_ALLOCATION_HINT_ZEROED           0x00000000
//#define TEE_MALLOC_FILL_ZERO                 TEE_ALLOCATION_HINT_ZEROED

//#define TEE_LogPrintf printf
//#define TEE_DbgPrintf msee_ta_printf
//#define TEE_DbgPrintLnf(...)      do{TEE_DbgPrintf(__VA_ARGS__);TEE_DbgPrintf(EOL);}while(FALSE)

#else
#error "NO TEE SUPPORT defined!!!"

#endif


#endif /* __MTK_TEE_TLSTD_H__ */
