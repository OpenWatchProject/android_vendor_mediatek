/*
 * Header files for UREE memory related functions.
 */

#ifndef __UREE_MEM_H__
#define __UREE_MEM_H__

#include "tz_cross/trustzone.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
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

// ION memory handle define
typedef uint32_t UREE_ION_HANDLE;

/**
 * Shared memory parameter
 *
 * It defines the types for shared memory.
 *
 * @param buffer    A pointer to shared memory buffer
 * @param size    shared memory size in bytes
 */
typedef struct {
	void* buffer;
	void *mapAry;
	uint32_t size;
	uint32_t region_id;
} UREE_SHAREDMEM_PARAM;

typedef struct {
	uint64_t buffer;
	uint32_t size;
	uint32_t region_id;
	//uint32_t *mapAry;
} UREE_SHAREDMEM_PARAM_US;


/**
 * Shared memory
 *
 * A shared memory is normal memory, which can be seen by Normal world
 * and Secure world. It is used to create the comminicattion between
 * two worlds. Currently, zero-copy data transfer is supportted,
 * for simple and efficient design.
 *
 * The shared memory lifetime:
 * 1. CA (Client Application at REE) prepares memory
 * 2. CA registers it to TEE scope.
 * 3. A handle is returned. CA can use it to communicate with TEE.
 * 4. If shared memory is not used, CA unregisters it.
 * 5. CA frees memory.
 *
 * Because it is zero-copy shared memory, the memory characteritics is inherited.
 * If the shared memory will be used for HW, CA must allocate physical
 * continuous memory.
 *
 * Note: Because shared memory can be seen by both Normal and Secure world.
 * It is a possible weakpoint to bed attacked or leak secure data.
 */

/**
 * Register shared memory
 *
 * This API is only for register writable memory to shared memory.
 * Read only memory can not be registed by this API, please use
 * UREE_RegisterSharedmem_ReadOnly.
 *
 * @param session    The session handle.
 * @param shm_handle    [out] A pointer to shared memory handle.
 * @param param  A pointer to shared memory parameters.
 *               param.buffer needs va alignment (i.e., use like
 *               memalign(PAGE_SIZE, shm_size) to create the buffer)
 * @return    return code.
 */
TZ_RESULT UREE_RegisterSharedmem(UREE_SESSION_HANDLE session,
    UREE_SHAREDMEM_HANDLE *shm_handle, UREE_SHAREDMEM_PARAM *param);


/**
 * Register shared memory with tag for debugging
 *
 * This API is same as UREE_RegisterSharedmem. But one additional parameter,
 * tag, for debugging purpose.
 *
 * @param session    The session handle.
 * @param shm_handle    [out] A pointer to shared memory handle.
 * @param param    A pointer to shared memory parameters.
 * @param tag      A string to be marked for debugging.
 * @return    return code.
 */
TZ_RESULT UREE_RegisterSharedmemWithTag (UREE_SESSION_HANDLE session,
	UREE_SHAREDMEM_HANDLE *shm_handle, UREE_SHAREDMEM_PARAM *param,
	const char *tag);

/**
 * Unregister shared memory
 *
 * This API is only for unregister writable memory to shared memory.
 * Read only memory can not be unregisted by this API, please use
 * UREE_UnregisterSharedmem_ReadOnly.
 *
 * @param session    The session handle.
 * @param shm_handle    The shared memory handle.
 * @return    return code.
 */
TZ_RESULT UREE_UnregisterSharedmem(UREE_SESSION_HANDLE session,
	UREE_SHAREDMEM_HANDLE shm_handle);

/**
 * Register shared memory
 *
 * This API is only for register read only memory to shared memory.
 * Writable memory can not be registed by this API, please use
 * UREE_RegisterSharedmem.
 *
 * @param session    The session handle.
 * @param shm_handle    [out] A pointer to shared memory handle.
 * @param param    A pointer to shared memory parameters.
 * @return    return code.
 */
TZ_RESULT UREE_RegisterSharedmem_ReadOnly(UREE_SESSION_HANDLE session,
	UREE_SHAREDMEM_HANDLE *shm_handle, UREE_SHAREDMEM_PARAM *param);


/**
 * Unregister shared memory
 *
 * This API is only for unregister read only memory to shared memory.
 * Writable memory can not be unregisted by this API, please use
 * UREE_UnregisterSharedmem.
 *
 * @param session    The session handle.
 * @param shm_handle    The shared memory handle.
 * @return    return code.
 */
TZ_RESULT UREE_UnregisterSharedmem_ReadOnly (UREE_SESSION_HANDLE session,
	UREE_SHAREDMEM_HANDLE shm_handle);

/**
 * Secure memory
 *
 * A secure memory can be seen only in Secure world.
 * Secure memory, here, is defined as external memory (ex: DRAM) protected
 * by trustzone. It can protect from software attack very well, but can not
 * protect from physical attack, like memory probe.
 * CA (Client Application at REE) can ask TEE for a secure buffer,
 * then control it: to reference, or to free...etc.
 *
 * Secure memory spec.:
 * 1. Protected by trustzone (NS = 0).
 * 2. External memory (ex: external DRAM).
 * 3. With cache.
 */


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
TZ_RESULT UREE_AllocSecuremem(UREE_SESSION_HANDLE session,
	UREE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size);

/**
 * Secure memory allocation with tag for debugging purpose.
 *
 * Same as above function, but additional parameter, tag is provided.
 * @param tag    The string for debugging purpose.
 */
TZ_RESULT UREE_AllocSecurememWithTag(UREE_SESSION_HANDLE session,
	UREE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size,
	const char *tag);

/**
 * Secure memory allocation with tag for debugging purpose.
 *
 * Same as above function, but the buffer is filled with zero.
 */
TZ_RESULT UREE_ZallocSecurememWithTag(UREE_SESSION_HANDLE session,
	UREE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size,
	const char *tag);

/**
 * Secure memory reference
 *
 * Reference memory.
 * Referen count will be increased by 1 after reference.
 *
 * Reference lifetime:
 * 1. Reference the memory before using it, if the memory is allocated by
 *    other process.
 * 2. Unreference it if it is not used.
 *
 * @param session    The session handle.
 * @param mem_handle    The secure memory handle.
 * @param return    return code.
 */
TZ_RESULT UREE_ReferenceSecuremem(UREE_SESSION_HANDLE session,
	UREE_SECUREMEM_HANDLE mem_handle);

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
TZ_RESULT UREE_UnreferenceSecuremem(UREE_SESSION_HANDLE session,
	UREE_SECUREMEM_HANDLE mem_handle);

/**
 * Secure chunk memory
 *
 * A secure chunk memory can be seen only in Secure world.
 * It is a kind of secure memory but with difference characteristic:
 * 1. It is designed and optimized for chunk memory usage.
 * 2. For future work, it can be released as normal memory for more flexible
 *    memory usage.
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
TZ_RESULT UREE_AllocSecurechunkmem(UREE_SESSION_HANDLE session,
	UREE_SECURECM_HANDLE *cm_handle, uint32_t alignment, uint32_t size);

/**
 * Secure chunk memory allocation with tag for debugging purpose.
 *
 * Same as above function, but additional parameter, tag is provided.
 * @param tag    The string for debugging purpose.
 */
TZ_RESULT UREE_AllocSecurechunkmemWithTag(UREE_SESSION_HANDLE session,
	UREE_SECURECM_HANDLE *cm_handle, uint32_t alignment, uint32_t size,
	const char *tag);

/**
 * Secure chunk memory allocation with tag for debugging purpose.
 *
 * Same as above function, but the buffer is filled with zero.
 */
TZ_RESULT UREE_ZallocSecurechunkmemWithTag(UREE_SESSION_HANDLE session,
	UREE_SECUREMEM_HANDLE *cm_handle, uint32_t alignment, uint32_t size,
	const char *tag);

/**
 * Secure chunk memory reference
 *
 * Reference memory.
 * Referen count will be increased by 1 after reference.
 *
 * Reference lifetime:
 * 1. Reference the memory before using it, if the memory is allocated
 *    by other process.
 * 2. Unreference it if it is not used.
 *
 * @param session    The session handle.
 * @param cm_handle    The secure chunk memory handle.
 * @param return    return code.
 */
TZ_RESULT UREE_ReferenceSecurechunkmem(UREE_SESSION_HANDLE session,
	UREE_SECURECM_HANDLE cm_handle);

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
TZ_RESULT UREE_UnreferenceSecurechunkmem(UREE_SESSION_HANDLE session,
	UREE_SECURECM_HANDLE cm_handle);

/**
 * Secure chunk memory release
 *
 * Release secure chunk memory for normal world usage.
 *
 * @param session    The session handle.
 * @param size    [out] The pointer to released size in bytes.
 * @param return    return code.
 */
TZ_RESULT UREE_ReleaseSecurechunkmem(UREE_SESSION_HANDLE session,
	uint32_t *size);

/**
 * Secure chunk memory append
 *
 * Append secure chunk memory back to secure world.
 *
 * @param session    The session handle.
 * @param return    return code.
 */
TZ_RESULT UREE_AppendSecurechunkmem(UREE_SESSION_HANDLE session);

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
TZ_RESULT UREE_ReadSecurechunkmem(UREE_SESSION_HANDLE session,
	uint32_t offset, uint32_t size, void *buffer);

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
TZ_RESULT UREE_WriteSecurechunkmem(UREE_SESSION_HANDLE session,
	uint32_t offset, uint32_t size, void *buffer);


/* Release secure chunk memory for normal world usage.
 *
 * For better utilization, part of secure chunk memory (pre-defined size)
 * can be used by normal world through memory TA.
 * After release, pre-defined secure memory can be read/write by normal world
 * through memroy TA, and can not be used by secure world. After append, it
 * can be used for secure world again. For easy usage at user level, a block
 * device can be registered, and it can access released secure chunk memory
 * by memory TA.
 *
 * How to use secure chunk memory at user level:
 * 1) Create a block device node, ex: /dev/tzmem
 * 2) Release secure chunk memory by UREE_ReleaseSecurechunkmem.
 *    After releasing, the pre-defined chunk memory will be used by
 *    normal world only.
 * 3) Open /dev/tzmem for read/write
 * 4) If finishing to use, close it.
 * 5) Append secure chunk memory back to secure world usage by
 *    UREE_AppendSecurechunkmem.
 *    After appending, the pre-defined chunk memory will not be used by
 *    normal world.
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
TZ_RESULT UREE_ReleaseTzmem(int *fd_p);

/**
 * Secure chunk memory append for block device
 *
 * Append secure chunk memory back to secure world.
 *
 * @param fd     fd.
 * @param return    return code.
 */
TZ_RESULT UREE_AppendTzmem(int fd);
TZ_RESULT UREE_ION_CP_Chm2Shm(UREE_SESSION_HANDLE session,
	UREE_SHAREDMEM_HANDLE shm_handle, UREE_ION_HANDLE ION_Handle, uint32_t size);

TZ_RESULT UREE_ION_TEST_UPT_CHMDATA(UREE_ION_HANDLE ION_Handle, uint32_t size);
TZ_RESULT UREE_ION_TEST_ChmImageRecognition(UREE_ION_HANDLE ION_Handle,
	uint32_t size);


/*only test for secure camera (sc)*/
TZ_RESULT UREE_ION_TO_SHM_HANDLE(UREE_ION_HANDLE ION_Handle,
	UREE_SHAREDMEM_HANDLE *shm_handle);

typedef struct {
	uint32_t size;
	int width;
	int height;
	uint32_t stride;
	int format;
} SecureCamera_IMG_OBJ;


#define TZCMD_to_HA_mtee_kernel_service 0x9002
#define TZCMD_to_HA_scamera_sample 0x9003

TZ_RESULT UREE_SecureCamera_to_MTEE(UREE_SESSION_HANDLE session,
	SecureCamera_IMG_OBJ *scObj, int cmd);


/* for FOD, TEE-MTEE share mem enable*/
TZ_RESULT UREE_FOD_ShareMem_Enable(int on);

#ifdef __cplusplus
}
#endif

#endif
