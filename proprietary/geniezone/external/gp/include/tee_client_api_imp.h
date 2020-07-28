/*
 * This header file defines the implementation-dependent types,
 * constants and macros for all the Trusted Foundations implementations
 * of the TEE Client API
 */

#ifndef   __TEE_CLIENT_API_IMP_H__
#define   __TEE_CLIENT_API_IMP_H__

#include "uree/system.h"
#include "uree/mem.h"

typedef struct
{
    uint32_t reserved;
} TEEC_Context_IMP;

typedef struct
{
    UREE_SESSION_HANDLE gzSession;
} TEEC_Session_IMP;

typedef struct
{
    UREE_SHAREDMEM_HANDLE gzSharedMem;
} TEEC_SharedMemory_IMP;

typedef struct
{
    uint32_t reserved;
} TEEC_Operation_IMP;


#define TEEC_CONFIG_SHAREDMEM_MAX_SIZE ((size_t)0xFFFFFFFF)

#define TEEC_PARAM_TYPES(param0Type, param1Type, param2Type, param3Type) \
    ((param0Type) | ((param1Type) << 8) | ((param2Type) << 16) | ((param3Type) << 24))


#endif /* __TEE_CLIENT_API_IMP_H__ */
