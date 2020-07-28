/*
 * Header files for UREE memory related functions.
 */

#ifndef __UREE_MEM_H__
#define __UREE_MEM_H__

#include "MobiCoreDriverApi.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define SECMEM_TLC_DEBUG

/*
 * Memory handle define
 *
 * Handle is used to communicate with normal world:
 * 1. Memory information can not expose to normal world. (Major, important!)
 * 2. Too many informations, and thet can be grouped by handle.
 *
 * All kinds of memory use the same handle define.
 * According to their different purpose, they are redefined to specific name.
 * Just for easy programming.
 */

// Shared memory handle define
typedef uint32_t UREE_SHAREDMEM_HANDLE;

// Secure memory handle define
typedef uint32_t UREE_SECUREMEM_HANDLE;

// Secure chunk memory handle define
typedef uint32_t UREE_SECURECM_HANDLE;

typedef uint32_t UREE_RELEASECM_HANDLE;

/*
 * Shared memory parameter
 *
 * It defines the types for shared memory.
 *
 * @param buffer    A pointer to shared memory buffer
 * @param size    shared memory size in bytes
 */
typedef struct { 
    void* buffer; 
    uint32_t size; 
} UREE_SHAREDMEM_PARAM;

typedef struct {
    uint32_t    alignment;  /* IN */
    uint32_t    size;       /* IN */
    uint32_t    refcount;   /* INOUT */
    uint32_t    sec_handle; /* OUT */
    uint32_t    id;
    uint8_t    *owner;
}UREE_SECMEM_PARAM;

/*
 * Shared memory
 *
 * A shared memory is normal memory, which can be seen by Normal world and Secure world.
 * It is used to create the comminicattion between two worlds.
 * Currently, zero-copy data transfer is supportted, for simple and efficient design.
 *
 * The shared memory lifetime:
 * 1. CA (Client Application at REE) prepares memory
 * 2. CA registers it to TEE scope.
 * 3. A handle is returned. CA can use it to communicate with TEE.
 * 4. If shared memory is not used, CA unregisters it.
 * 5. CA frees memory.
 *
 * Because it is zero-copy shared memory, the memory characteritics is inherited.
 * If the shared memory will be used for HW, CA must allocate physical continuous memory.
 *
 * Note: Because shared memory can be seen by both Normal and Secure world.
 * It is a possible weakpoint to bed attacked or leak secure data.
 */
 
/**
 * Register shared memory
 *
 * This API is only for register writable memory to shared memory.
 * Read only memory can not be registed by this API, please use UREE_RegisterSharedmem_ReadOnly.
 *
 * @param session    The session handle. 
 * @param shm_handle    [out] A pointer to shared memory handle.
 * @param param    A pointer to shared memory parameters.
 * @return    return code.
 */
//TZ_RESULT UREE_RegisterSharedmem (UREE_SESSION_HANDLE session, 
//    UREE_SHAREDMEM_HANDLE *shm_handle, UREE_SHAREDMEM_PARAM *param);

    
/**
 * Unregister shared memory
 *
 * This API is only for unregister writable memory to shared memory.
 * Read only memory can not be unregisted by this API, please use UREE_UnregisterSharedmem_ReadOnly.
 *
 * @param session    The session handle.  
 * @param shm_handle    The shared memory handle.
 * @return    return code.
 */    
//TZ_RESULT UREE_UnregisterSharedmem (UREE_SESSION_HANDLE session, UREE_SHAREDMEM_HANDLE shm_handle);

/**
 * Register shared memory
 *
 * This API is only for register read only memory to shared memory.
 * Writable memory can not be registed by this API, please use UREE_RegisterSharedmem.
 *
 * @param session    The session handle. 
 * @param shm_handle    [out] A pointer to shared memory handle.
 * @param param    A pointer to shared memory parameters.
 * @return    return code.
 */
//TZ_RESULT UREE_RegisterSharedmem_ReadOnly (UREE_SESSION_HANDLE session, 
//    UREE_SHAREDMEM_HANDLE *shm_handle, UREE_SHAREDMEM_PARAM *param);

    
/**
 * Unregister shared memory
 *
 * This API is only for unregister read only memory to shared memory.
 * Writable memory can not be unregisted by this API, please use UREE_UnregisterSharedmem.
 *
 * @param session    The session handle.  
 * @param shm_handle    The shared memory handle.
 * @return    return code.
 */    
//TZ_RESULT UREE_UnregisterSharedmem_ReadOnly (UREE_SESSION_HANDLE session, UREE_SHAREDMEM_HANDLE shm_handle);

/**
 * Secure memory
 *
 * A secure memory can be seen only in Secure world.
 * Secure memory, here, is defined as external memory (ex: DRAM) protected by trustzone.
 * It can protect from software attack very well, but can not protect from physical attack, like memory probe.
 * CA (Client Application at REE) can ask TEE for a secure buffer, then control it: 
 * to reference, or to free...etc.
 *
 * Secure memory spec.:
 * 1. Protected by trustzone (NS = 0).
 * 2. External memory (ex: external DRAM).
 * 3. With cache.
 */

#if 0 
mcResult_t UREE_AllocSecuremem_Debug(uint32_t *mem_handle, uint32_t alignment, uint32_t size, uint8_t *owner, uint32_t id);
mcResult_t UREE_AllocSecurememZero_Debug(uint32_t *mem_handle, uint32_t alignment, uint32_t size, uint8_t *owner, uint32_t id);
mcResult_t UREE_ReferenceSecuremem_Debug(uint32_t mem_handle, uint32_t *refcount, uint8_t *owner, uint32_t id);
mcResult_t UREE_UnreferenceSecuremem_Debug(uint32_t mem_handle, uint32_t *refcount, uint8_t *owner, uint32_t id);
mcResult_t UREE_AllocSecurememTBL_Debug(uint32_t *mem_handle, uint32_t alignment, uint32_t size, uint8_t *owner, uint32_t id);
mcResult_t UREE_AllocSecurememTBLZero_Debug(uint32_t *mem_handle, uint32_t alignment, uint32_t size, uint8_t *owner, uint32_t id);
mcResult_t UREE_UnreferenceSecurememTBL_Debug(uint32_t mem_handle, uint32_t *refcount, uint8_t *owner, uint32_t id);

#define UREE_AllocSecuremem(mhdl, align, size)      UREE_AllocSecuremem_Debug(mhdl, align, size, (uint8_t*)__FILE__, __LINE__)
#define UREE_AllocSecurememZero(mhdl, align, size)  UREE_AllocSecurememZero_Debug(mhdl, align, size, (uint8_t*)__FILE__, __LINE__)
#define UREE_ReferenceSecuremem(mhdl, refcnt)       UREE_ReferenceSecuremem_Debug(mhdl, refcnt, (uint8_t*)__FILE__, __LINE__)
#define UREE_UnreferenceSecuremem(mhdl, refcnt)     UREE_UnreferenceSecuremem_Debug(mhdl, refcnt, (uint8_t*)__FILE__, __LINE__)
#define UREE_AllocSecurememTBL(mhdl, align, size)   UREE_AllocSecurememTBL_Debug(mhdl, align, size, (uint8_t*)__FILE__, __LINE__)
#define UREE_AllocSecurememTBLZero(mhdl, align, size)   UREE_AllocSecurememTBLZero_Debug(mhdl, align, size, (uint8_t*)__FILE__, __LINE__)
#define UREE_UnreferenceSecurememTBL(mhdl, refcnt)  UREE_UnreferenceSecurememTBL_Debug(mhdl, refcnt, (uint8_t*)__FILE__, __LINE__)

#else
/**
 * Secure memory allocation
 *
 * Allocate one memory.
 * If memory is allocated successfully, a handle will be provided.
 * 
 * Memory lifetime:
 * 1. Allocate memory, and get the handle.
 * 2. If other process wants to use the same memory, reference it.
 * 3. If they stop to use it, unreference it.
 * 4. Free it (by unreference), if it is not used.
 *
 * Simple rules:
 * 1. start by allocate, end by unreference (for free).
 * 2. start by reference, end by unreference.
 *
 * @param session    The session handle.  
 * @param mem_handle    [out] A pointer to secure memory handle.  
 * @param alignment    Memory alignment in bytes.  
 * @param size    The size of the buffer to be allocated in bytes.
 * @return    return code.
 */ 
mcResult_t UREE_AllocSecuremem(uint32_t *mem_handle, uint32_t alignment, uint32_t size, uint8_t *owner, uint32_t id);
mcResult_t UREE_AllocSecurememZero(uint32_t *mem_handle, uint32_t alignment, uint32_t size, uint8_t *owner, uint32_t id);


/**
 * Secure memory reference
 *
 * Reference memory.
 * Referen count will be increased by 1 after reference.
 * 
 * Reference lifetime:
 * 1. Reference the memory before using it, if the memory is allocated by other process.
 * 2. Unreference it if it is not used.
 *
 * @param session    The session handle.
 * @param mem_handle    The secure memory handle.
 * @param return    return code.
 */ 
mcResult_t UREE_ReferenceSecuremem (uint32_t mem_handle, uint32_t *refcount, uint8_t *owner, uint32_t id);

/**
 * Secure memory unreference
 *
 * Unreference memory.
 * Reference count will be decreased by 1 after unreference.
 * Once reference count is zero, memory will be freed.
 *
 * @param session    The session handle.
 * @param mem_handle    The secure memory handle.
 * @param return    return code.
 */ 
mcResult_t UREE_UnreferenceSecuremem (uint32_t mem_handle, uint32_t *refcount, uint8_t *owner, uint32_t id);

//APIs for t-Play
mcResult_t UREE_AllocSecurememTBL(uint32_t *mem_handle, uint32_t alignment, uint32_t size, uint8_t *owner, uint32_t id);
mcResult_t UREE_AllocSecurememTBLZero(uint32_t *mem_handle, uint32_t alignment, uint32_t size, uint8_t *owner, uint32_t id);


mcResult_t UREE_UnreferenceSecurememTBL (uint32_t mem_handle, uint32_t *refcount, uint8_t *owner, uint32_t id);
#endif

#define UREE_FreeSecuremem(fmt, args...)    UREE_UnreferenceSecuremem(fmt, ##args)
#define UREE_FreeSecurememTBL(fmt, args...) UREE_UnreferenceSecurememTBL(fmt, ##args)

mcResult_t tlcMemOpen(void);

void tlcMemClose(void);

mcResult_t UREE_DumpSecurememInfo(void);


/**
 * Secure chunk memory
 *
 * A secure chunk memory can be seen only in Secure world.
 * It is a kind of secure memory but with difference characteristic:
 * 1. It is designed and optimized for chunk memory usage.
 * 2. For future work, it can be released as normal memory for more flexible memory usage.
 *
 * Secure chunk memory spec.:
 * 1. Protected by trustzone (NS = 0).
 * 2. External memory (ex: external DRAM).
 * 3. With cache.
 * 4. For future, it can be released to normal world.
 */
 
/**
 * Secure chunk memory allocation
 *
 * Allocate one memory.
 * If memory is allocated successfully, a handle will be provided.
 * 
 * Memory lifetime:
 * 1. Allocate memory, and get the handle.
 * 2. If other process wants to use the same memory, reference it.
 * 3. If they stop to use it, unreference it.
 * 4. Free it (by unreference), if it is not used.
 *
 * Simple rules:
 * 1. start by allocate, end by unreference (for free).
 * 2. start by reference, end by unreference.
 *
 * @param session    The session handle.   
 * @param cm_handle    [out] A pointer to secure chunk memory handle. 
 * @param alignment    Memory alignment in bytes.  
 * @param size    The size of the buffer to be allocated in bytes.

 * @return    return code.
 */ 
//TZ_RESULT UREE_AllocSecurechunkmem (UREE_SESSION_HANDLE session, 
//    UREE_SECURECM_HANDLE *cm_handle, uint32_t alignment, uint32_t size);

/**
 * Secure chunk memory reference
 *
 * Reference memory.
 * Referen count will be increased by 1 after reference.
 * 
 * Reference lifetime:
 * 1. Reference the memory before using it, if the memory is allocated by other process.
 * 2. Unreference it if it is not used.
 *
 * @param session    The session handle.
 * @param cm_handle    The secure chunk memory handle.
 * @param return    return code.
 */ 
//TZ_RESULT UREE_ReferenceSecurechunkmem (UREE_SESSION_HANDLE session, UREE_SECURECM_HANDLE cm_handle);

/**
 * Secure chunk memory unreference
 *
 * Unreference memory.
 * Reference count will be decreased by 1 after unreference.
 * Once reference count is zero, memory will be freed.
 *
 * @param session    The session handle.
 * @param cm_handle    The secure chunk memory handle.
 * @param return    return code.
 */ 
//TZ_RESULT UREE_UnreferenceSecurechunkmem (UREE_SESSION_HANDLE session, UREE_SECURECM_HANDLE cm_handle);

/**
 * Secure chunk memory release
 *
 * Release secure chunk memory for normal world usage.
 *
 * @param session    The session handle.
 * @param size    [out] The pointer to released size in bytes.
 * @param return    return code.
 */ 
//TZ_RESULT UREE_ReleaseSecurechunkmem (UREE_SESSION_HANDLE session, uint32_t *size);

/**
 * Secure chunk memory append
 *
 * Append secure chunk memory back to secure world.
 *
 * @param session    The session handle.
 * @param return    return code.
 */ 
//TZ_RESULT UREE_AppendSecurechunkmem (UREE_SESSION_HANDLE session);

/**
 * Released secure chunk memory Read
 *
 * Read release secure chunk memory for normal world usage.
 *
 * @param session    The session handle.
 * @param offset    offset in bytes.
 * @param size    size in bytes.
 * @param buffer    The pointer to read buffer.
 * @param return    return code.
 */
//TZ_RESULT UREE_ReadSecurechunkmem (UREE_SESSION_HANDLE session, uint32_t offset, uint32_t size, void *buffer);

/**
 * Released secure chunk memory Write
 *
 * Write release secure chunk memory for normal world usage.
 *
 * @param session    The session handle.
 * @param offset    offset in bytes.
 * @param size    size in bytes.
 * @param buffer    The pointer to write buffer.
 * @param return    return code.
 */
//TZ_RESULT UREE_WriteSecurechunkmem (UREE_SESSION_HANDLE session, uint32_t offset, uint32_t size, void *buffer);    


/* Release secure chunk memory for normal world usage.
 *
 * For better utilization, part of secure chunk memory (pre-defined size) can be used by
 * normal world through memory TA.
 * After release, pre-defined secure memory can be read/write by normal world through memroy TA, 
 * and can not be used by secure world. After append, it can be used for secure world again.
 * For easy usage at user level, a block device can be registered, and it can access released
 * secure chunk memory by memory TA.
 *  
 * How to use secure chunk memory at user level:
 * 1) Create a block device node, ex: /dev/tzmem
 * 2) Release secure chunk memory by UREE_ReleaseSecurechunkmem.
 *    After releasing, the pre-defined chunk memory will be used by normal world only.
 * 3) Open /dev/tzmem for read/write
 * 4) If finishing to use, close it.
 * 5) Append secure chunk memory back to secure world usage by UREE_AppendSecurechunkmem.
 *    After appending, the pre-defined chunk memory will not be used by normal world.
 *
 * Or simply, using APIs
 * 1) UREE_ReleaseTzmem for release secure chunk memory to normal world usage.
 * 2) UREE_AppendTzmem for append secure chunk memory back to secure world.
 *
 */
 
/**
 * Secure chunk memory release for block device
 *
 * Release secure chunk memory for normal world usage.
 *
 * @param fd_p    The pointer to fd.
 * @param return    return code.
 */ 
//TZ_RESULT UREE_ReleaseTzmem (int *fd_p);

/**
 * Secure chunk memory append for block device
 *
 * Append secure chunk memory back to secure world.
 *
 * @param fd     fd.
 * @param return    return code.
 */ 
//TZ_RESULT UREE_AppendTzmem (int fd);

#ifndef SKIP_DEBUG_ALIAS_MACRO
#define UREE_AllocSecuremem(mhdl, align, size)      UREE_AllocSecuremem(mhdl, align, size, (uint8_t*)__FILE__, __LINE__)
#define UREE_AllocSecurememZero(mhdl, align, size)  UREE_AllocSecurememZero(mhdl, align, size, (uint8_t*)__FILE__, __LINE__)
#define UREE_ReferenceSecuremem(mhdl, refcnt)       UREE_ReferenceSecuremem(mhdl, refcnt, (uint8_t*)__FILE__, __LINE__)
#define UREE_UnreferenceSecuremem(mhdl, refcnt)     UREE_UnreferenceSecuremem(mhdl, refcnt, (uint8_t*)__FILE__, __LINE__)
#define UREE_AllocSecurememTBL(mhdl, align, size)   UREE_AllocSecurememTBL(mhdl, align, size, (uint8_t*)__FILE__, __LINE__)
#define UREE_AllocSecurememTBLZero(mhdl, align, size)   UREE_AllocSecurememTBLZero(mhdl, align, size, (uint8_t*)__FILE__, __LINE__)
#define UREE_UnreferenceSecurememTBL(mhdl, refcnt)  UREE_UnreferenceSecurememTBL(mhdl, refcnt, (uint8_t*)__FILE__, __LINE__)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __UREE_MEM_H__ */
