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

#ifndef __MTK_TEE_DRSTD_H__
#define __MTK_TEE_DRSTD_H__


#if defined(__TRUSTONIC_TEE__)
//==============================================================================
// __TRUSTONIC_TEE__
//==============================================================================

#pragma message ( "TRUSTONIC_TEE" )
#include "drStd.h"

#elif defined(__MICROTRUST_TEE__)
//==============================================================================
// __MICROTRUST_TEE__
//==============================================================================

#pragma message ( "MICROTRUST_TEE" )
#include <stdint.h>
#include <stdbool.h>
#if defined(__cplusplus)

    #define _EXTERN_C                extern "C"
    #define _BEGIN_EXTERN_C          extern "C" {
    #define _END_EXTERN_C            }

#else

    #define _EXTERN_C
    #define _BEGIN_EXTERN_C
    #define _END_EXTERN_C

#endif // defined(__cplusplus)

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

#define SHIFT_4KB               (12U) /**<  SIZE_4KB is 1 << SHIFT_4KB aka. 2^SHIFT_4KB. */
#ifndef SIZE_4KB
    #define SIZE_4KB                (1 << SHIFT_4KB) /**< Size of 1 KiB. */
#endif

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
#include <errno.h>

#ifndef __cplusplus

#define bool    _Bool
#define true    1
#define false   0

#else /* __cplusplus */

/* Supporting <stdbool.h> in C++ is a GCC extension.  */
#define _Bool   bool
#define bool    bool
#define false   false
#define true    true

#endif /* __cplusplus */

typedef void        *void_ptr;  /**< a pointer to anything. */
typedef void_ptr    addr_t;     /**< an address, can be physical or virtual */
typedef uint32_t    bool_t;     /**< boolean data type. */

#define SHIFT_4KB               (12U) /**<  SIZE_4KB is 1 << SHIFT_4KB aka. 2^SHIFT_4KB. */
#ifndef SIZE_4KB
    #define SIZE_4KB                (1 << SHIFT_4KB) /**< Size of 1 KiB. */
#endif

#define MAP_READABLE            (1U << 0) /**< mapping gives the ability to do read access. */
#define MAP_WRITABLE            (1U << 1) /**< mapping gives the ability to do write access. */
#define MAP_EXECUTABLE          (1U << 2) /**< mapping gives the ability to do program execution. */
#define MAP_UNCACHED            (1U << 3) /**< mapping gives uncached memory access. */
#define MAP_IO                  (1U << 4) /**< mapping gives memory mapped I/O access. Will ignore MAP_UNCACHED, as this would be implied anyway. */
/** Memory with hardware attributes. */
#define MAP_HARDWARE            (MAP_READABLE | MAP_WRITABLE | MAP_IO)
#define MAP_NOT_SECURE          (1U << 7)
#define MAP_ALLOW_NONSECURE     (1U << 7)
#define MAP_STRONGLY_ORDERED    (1U << 8)


//Common result type
typedef uint32_t drApiResult_t;

//MTK types
typedef uint32_t  taskid_t,   *taskid_ptr;     /**< task id data type. */
typedef uint32_t  threadno_t, *threadno_ptr;   /**< thread no. data type. */
typedef uint32_t  threadid_t, *threadid_ptr;   /**< thread id data type. */
typedef uint32_t  intrNo_t, *intrNo_ptr;      /**< interrupt number. */

#if defined(__cplusplus)

    #define _EXTERN_C                extern "C"
    #define _BEGIN_EXTERN_C          extern "C" {
    #define _END_EXTERN_C            }

#else

    #define _EXTERN_C
    #define _BEGIN_EXTERN_C
    #define _END_EXTERN_C

#endif // defined(__cplusplus)


#define _UNUSED             __attribute__((unused))
#define _USED               __attribute__((used))
#define _NORETURN           __attribute__((noreturn))

#define DRIVER_MAX_PRIORITY         (9U)       /**< maximum priority available for a driver. */
#define MAX_PRIORITY                (DRIVER_MAX_PRIORITY) /** Legacy define */
#define _DRAPI_EXTERN_C     _EXTERN_C
#define _DRAPI_NORETURN     _NORETURN
#define _DRAPI_ENTRY    _DRAPI_EXTERN_C _DRAPI_NORETURN
#define _THREAD_ENTRY
#define E_OK                        0
#define E_INVALID                   1
#define NILTASK                     0
#define NILTHREAD                   0
#define DRAPI_OK                        0x0 
#define DRAPI_CACHE_ALL        (0xff)

#define E_DRAPI_KERNEL_ERROR            0xF01    /**< Kernel returned error. */
#define E_DRAPI_INVALID_PARAMETER       0xF02    /**< Invalid parameter. */
#define E_DRAPI_NOT_PERMITTED           0xF03    /**< Permission error */
#define E_DRAPI_IPC_ERROR               0xF04    /**< Error in IPC. */
#define E_DRAPI_TASK_NOT_ACCEPTABLE     0xF05    /**< Task not acceptable for operation. */
#define E_DRAPI_CANNOT_MAP              0xF06    /**< Cannot create mapping. */
#define E_DRAPI_DRV_NO_SUCH_CLIENT      0xF07    /**< Client does not exist. */
#define E_DRAPI_CANNOT_INIT             0xF08    /**< Cannot be initialized. */
#define E_DRAPI_NOT_IMPLEMENTED         0xF09    /**< Function not yet implemented. */

#define TBASE_API_LEVEL 0

#define PTR2VAL(p)              ((uintptr_t)(p))
#define VAL2PTR(v)              ((addr_t)(v))
#define FUNC_PTR(func)          VAL2PTR( PTR2VAL( func ) )

#define TASK_THREAD_QUOTA_SHL       24
#define TASK_TIME_QUOTA_MASK        ((1 << TASK_THREAD_QUOTA_SHL) - 1)  /**< mask to get/set time quota of a task. */
#define TIME_INFINITE               (TASK_TIME_QUOTA_MASK)      /**< returns infinite. */

typedef uint32_t        stackEntry_t, *stackEntry_ptr;

// calculate the size in byte for the given number of stack entries
#define STACK_ENTRY_COUNT_TO_BYTE_SIZE(e)    (e * sizeof(stackEntry_t))
// calculate the number of stack entries require to hold given number of bytes
#define BYTE_SIZE_TO_STACK_ENTRY_COUNT(b)    ((b + sizeof(stackEntry_t) - 1) / sizeof(stackEntry_t))

// internal macros to get variable names from given name. Don't try using the
// variable names directly.
#define _STACK_ARRAY_VAR(_name_)    _stack_##_name_##_array
#define _STACK_ENTRIES_VAR(_name_)  _stack_##_name_##_entries
#define _SECTION_STACK              _SECTION("stack")

// declare a stack. Basically, this is an array in RAM. However, no assumptions
// must be make about how a stack is defined internally. Access functions and
// macros must be used. The actual implementation may change anytime
// Note about the "extern const" declaration, that in C++ variables declared
// "const" have internal linkage. To enforce external linkage, "extern" must be
// given, too. Actually, it is given by _EXTERN_C, but we keep the additional
// "extern" here to remember why we need it.
// _STACK_ARRAY_VAR() has "_USED", because compiler/liker shall never remove
// it. If a stack is declared, it must exist, even if no code references it. It
// may still be needed by a wrapper that set up our binary externally.
// _STACK_ENTRIES_VAR() has "_UNSED" because it my not be referenced at all. It
// is just an internal helper variable.
#if defined(__ARMCC__)
    #define DECLARE_STACK(_name_,_size_) \
        _BEGIN_EXTERN_C \
            _USED stackEntry_t          __attribute__((aligned(8))) _STACK_ARRAY_VAR(_name_)[BYTE_SIZE_TO_STACK_ENTRY_COUNT(_size_)] _SECTION_STACK _ZEROINIT; \
            _USED extern const uint32_t _STACK_ENTRIES_VAR(_name_) = BYTE_SIZE_TO_STACK_ENTRY_COUNT(_size_); \
        _END_EXTERN_C
#elif defined(__GNUC__)
    #define DECLARE_STACK(_name_,_size_) \
        _BEGIN_EXTERN_C \
            _USED stackEntry_t      __attribute__((aligned(8))) _STACK_ARRAY_VAR(_name_)[BYTE_SIZE_TO_STACK_ENTRY_COUNT(_size_)]; \
            _USED const uint32_t    _STACK_ENTRIES_VAR(_name_) = BYTE_SIZE_TO_STACK_ENTRY_COUNT(_size_); \
        _END_EXTERN_C
#else
    #error "unknown compiler"
#endif


// import the references from a stack declares somewhere else
#define EXTERNAL_STACK(_name_) \
    _BEGIN_EXTERN_C \
        extern stackEntry_t     _STACK_ARRAY_VAR(_name_)[]; \
        extern const uint32_t   _STACK_ENTRIES_VAR(_name_); \
    _END_EXTERN_C

#define DECLARE_DRIVER_MAIN_STACK(_size_) DECLARE_STACK(drMain, _size_)


// get a pointer to the top of a given stack.
#define GET_STACK_TOP(_name_) \
            ( &_STACK_ARRAY_VAR(_name_)[_STACK_ENTRIES_VAR(_name_)] )
#define getStackTop(_name_) GET_STACK_TOP(_name_)

// get a pointer to the bottom of a given stack.
#define GET_STACK_BOTTOM(_name_) \
            ( &_STACK_ARRAY_VAR(_name_)[0] )
#define getStackBottom(_name_) GET_STACK_BOTTOM(_name_)

// get the size of a given stack. cannot use sizeof() here, as only the symbol
// name are know for external stacks.
#define GET_STACK_SIZE(_name_) \
            ( _STACK_ENTRIES_VAR(_name_) * sizeof(stackEntry_t) )
#define getStackSize(_name_) GET_STACK_SIZE(_name_)


typedef enum {

    MSG_NULL = 0,  // Used for initializing state machines
    MSG_RQ                          = 1,
        // Client Request, blocks until MSG_RS is received
        // Client -> Server
    MSG_RS                          = 2,
        // Driver Response, answer to MSG_RQ
        // Server -> Client
    MSG_RD                          = 3,
        // Driver becomes ready
        // Server -> IPCH
    MSG_NOT                         = 4,
        // Notification to NWd for a session, send-only message with no
        // response
        // client/server -> IPCH;
    MSG_CLOSE_TRUSTLET              = 5,
        // Close Trustlet, must be answered by MSG_CLOSE_TRUSTLET_ACK
        // MSH -> IPCH, IPCH -> Server
    MSG_CLOSE_TRUSTLET_ACK          = 6,
        // Close Trustlet Ack, in response to MSG_CLOSE_TRUSTLET
        // Server -> IPCH
    MSG_MAP                         = 7,
        // Map Client into Server, send-only message with no reponse
        //Server -> IPCH;
    MSG_ERR_NOT                     = 8,
        // Error Notification
        // EXCH/SIQH -> IPCH
    MSG_CLOSE_DRIVER                = 9,
        // Close Driver, must be answered with MSG_CLOSE_DRIVER_ACK
        // MSH -> IPCH, IPCH -> Driver/Server
    MSG_CLOSE_DRIVER_ACK            = 10,
        // Close Driver Ack, response to MSG_CLOSE_DRIVER
        // Driver/Server -> IPCH, IPCH -> MSH
    MSG_GET_DRIVER_VERSION          = 11,
        // Get driver version, used for response also
        // Client <-> IPCH
    MSG_GET_DRAPI_VERSION           = 12,
        // Get DrApi version, used for response also
        // Driver <-> IPCH */
    MSG_SET_NOTIFICATION_HANDLER    = 13,
        // Set (change) the SIQ handler thread, used for response also
        // Driver <-> IPCH
    MSG_GET_REGISTRY_ENTRY          = 14,
        // Get registry entry, available only if MC_FEATURE_DEBUG_SUPPORT is
        //   set, used for response also
        // Driver <-> IPCH
    MSG_DRV_NOT                     = 15,
        // Notification to a Trustlet, looks like a notification from NWd for
        //   the Trustlet, send-only message with no response
        // Driver -> Trustlet
    MSG_SET_FASTCALL_HANDLER        = 16,
        // install a FastCall handler, used for response also
        // Driver <-> IPCH
    MSG_GET_CLIENT_ROOT_AND_SP_ID   = 17,
        // get Root DI and SP ID, used for response also
        // Driver <-> IPCH
    MSG_SUSPEND                     = 18,
        // Suspend, requires MSG_SUSPEND_ACK as response
        // EXCH -> IPCH, IPCH -> driver
    MSG_SUSPEND_ACK                 = 19,
        // Suspend Ack, response to MSG_SUSPEND
        // driver -> IPCH, IPCH -> EXCH
    MSG_RESUME                      = 20,
    MSG_RESUME_ACK                  = 21,
        // resume, , response to MSG_RESUME
        // driver ->  IPCH, IPCH -> EXCH
    MSG_GET_ENDORSEMENT_SO          = 22,
        // get SO from RTM for the Endorsement functionality
        // Driver <-> IPCH
    MSG_GET_SERVICE_VERSION         = 23,
        // get version of service (TA)
        // Driver <-> IPCH
    MSG_ERROR                       = 24,
        // IPCH returns error to Driver
        // IPCH <-> DRIVER
    MSG_CALL_FASTCALL               = 25,
        // Call fastcall from driver
        // DRIVER -> IPCH -> MTK -> FASTCALL -> return
    MSG_GET_PROPERTY                = 26,
        // get properties of a TA/driver
        // Driver <-> IPCH
    MSG_RQ_EX                       = 27,
        // Client Request, blocks until MSG_RS or MSG_RQ_ERROR is received
        // Client -> Server
    MSG_RQ_ERROR                    = 28,
        // Sent to Client to raise an error in IPC communication level
        // Server -> Client
    MSG_INIT_HEAP                   = 29,
        // init heap of a TA/driver
        // client/server -> IPCH;
    MSG_EXTEND_HEAP                 = 30,
        // extend heap of a TA/driver
        // client/server -> IPCH;
    MSG_MAP_TASK_BUFFER             = 31,
        // Map client task buffer into Server,
        // Server -> IPCH;
    MSG_MAP_PHYSICAL_BUFFER         = 32,
        // Map physical buffer into Server
        // Server -> IPCH;
    MSG_UNMAP_BUFFER                = 33,
        // Unmap buffer from Server
        // Server -> IPCH;
    MSG_GET_AREA_TYPE               = 34,
        // get area attributes
        // Server -> IPCH;
    MSG_UNMAP_TASK_BUFFERS          = 35,
        // Unmap entire client (all its buffers) from Server
        // Server -> IPCH;
    MSG_RESERVED_1                  = 36,
    MSG_RESERVED_2                  = 37,
    MSG_RESERVED_3                  = 38,
    MSG_RESERVED_4                  = 39,
        // Reserved values, do not use!
    MSG_ERR_TEARDOWN                = 40,
        // Error Notification
        // EXCH/SIQH -> IPCH
    MSG_REGISTER_FOR_POWER_EVENTS   = 41,
        // Register for suspend/resume messages
        // IPCH <-> driver
    MSG_GET_REE_TIME                = 42,

    MSG_RESERVED_5                  = 43,
    //Get system time
    MSG_GET_TEE_TIME             = 44

} message_t;




#else
#error "NO TEE SUPPORT defined!!!"

#endif



#endif /* __MTK_TEE_DR_COMMON_H__ */
