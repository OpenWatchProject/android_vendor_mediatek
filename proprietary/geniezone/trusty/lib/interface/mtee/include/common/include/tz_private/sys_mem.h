/*
* Copyright (c) 2015 MediaTek Inc.
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

#ifndef __MTEE_MEM_H__
#define __MTEE_MEM_H__

#include "tz_cross/gz_version.h"
#include "tz_cross/trustzone.h"
#include "tz_private/system.h"
#include <malloc.h>
#include "sys/sys_malloc.h"

// fix me!!!, need a header file
/* GenieZone API version
 * version = Main.Sub
*/

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
typedef uint32_t MTEE_SHAREDMEM_HANDLE;

// Secure memory handle define
typedef uint32_t MTEE_SECUREMEM_HANDLE;

// Chunk memory handle define
typedef uint32_t MTEE_CHUNKMEM_HANDLE;

#if (GZ_API_MAIN_VERSION > 2)
// Secure chunk memory handle define
typedef uint32_t MTEE_SECURECM_HANDLE;

typedef uint32_t MTEE_RELEASECM_HANDLE;
#endif

/**
 * Shared memory parameter
 *
 * It defines the types for shared memory.
 *
 * @param buffer    A pointer to shared memory buffer
 * @param size    shared memory size
 */
typedef struct {
    void* buffer;
    uint32_t size;
	uint64_t pa;
} MTEE_SHAREDMEM_PARAM;


/**
 * compress a PA array by run-length coding
 *
 * It defines the types for shared memory.
 *
 * @param high    high bit of start PA address
 * @param low     low  bit of start PA address
 * @param size    run length size
 */
typedef struct {
	uint32_t high;		//(uint64_t) start PA address = high | low
    uint32_t low;
    uint32_t size;
} MTEE_SHAREDMEM_RUNLENGTH_ENTRY;


/**
 * Memory parameter
 *
 * It defines the parameters for memory.
 *
 * @param buffer    A pointer to shared memory buffer
 * @param size    shared memory size
 */
typedef struct {
    void* buffer;
    int size;
} MTEE_MEM_PARAM;


/**
 * Shared memory information
 *
 * It defines the information structure for shared memory.
 *
 * @param parameter    A pointer to shared memory parameter
 * @param page_num    Page number
 * @param mmuTable     A pointer to mmu table
 */
typedef struct {
	uint64_t pa; // fisrt PA address, for linux kernel memory
	int size; // size in bytes
	uint32_t region_id;
    MTEE_SHAREDMEM_PARAM parameter;
    uint32_t page_num;
    uint64_t *mmuTable;
} MTEE_SHAREDMEM;


#if (GZ_API_MAIN_VERSION > 2)
/**
 * TEE memory map
 *
 * Four levels/kinds of TEE memory:
 * 1. On-chip secure memory
 * 2. External secure memory (secure memory, in brief)
 * 3. External secure chunk memory (secure chunk memory, in brief)
 * 4. Non-secure memory (includes shared memory)
 *
 * Each memory has different secure strength.
 * Here, followed GP defined TEE resource property.
 */

/**
 * Query memory rank
 *
 * To query memory rank, which is memory pool ID.
 * It is used for checking memory if it secure enough or not.
 *
 * @param buffer    a pointer to memory.
 * @return    memory rank, defined as KERNELMEM_ID. If not secure memory, return 0xFFFFFFFF.
 */
uint32_t MTEE_QueryMemoryrank (void *buffer);
#endif

typedef unsigned long phys_addr_t;
/**
 * Get physical address
 *
 * Get physical address according to virtual address.
 * Currently, all memory in TEE is physical continous. Only VA to PA converting is needed.
 *
 * @param buffer    A pointer to memory with virtual address.
 * @return    A pointer to memory with physical address. If fail, it is NULL.
 */
phys_addr_t get_physicaladdr (void *buffer);
#define MTEE_GetPhysicaladdr(buffer) get_physicaladdr(buffer);

#if (GZ_API_MAIN_VERSION > 2)
/**
 * On-chip secure memory
 *
 * An On-chip secure memory can be seen only in Secure world.
 * On-chip secure memory, here, is defined as on-chip memory (ex: On-chip SRAM) protected by trustzone.
 * It can protect from software attack and some physical very well, but can not protect from tamper attack, like decap.
 *
 * Secure memory spec.:
 * 1. Protected by trustzone (NS = 0).
 * 2. On-chip memory (ex: On-chip SRAM).
 * 3. With cache.
 * 4. Physically continuous.
 */

/**
 * Allocate onchip secure memory
 *
 * @param size    The size of the buffer to be allocated in bytes
 * @return    If success, return address. If fail, return NULL.
 */
#define MTEE_AllocOnchipMem(size) onchip_malloc(size)

/**
 * Free onchip secure memory
 *
 * @param buffer    The pointer to the memory block to be referenced
 */
#define MTEE_FreeOnchipMem(buffer) onchip_free(buffer)

/**
 * Query allocated onchip secure memory size
 *
 * @param buffer    The pointer to the memory block to be queried.
 * @return    memoy size in bytes. It may be slightly larger than allcated because of alignment.
 */
#define MTEE_QueryOnchipMemsize(buffer) secure_memsize(buffer)
#endif

/**
 * Secure memory
 *
 * A secure memory can be seen only in Secure world.
 * Secure memory, here, is defined as external memory (ex: external DRAM) protected by trustzone.
 * It can protect from software attack very well, but can not protect from physical attack, like memory probe.
 *
 * Secure memory spec.:
 * 1. Protected by trustzone (NS = 0).
 * 2. External memory (ex: external DRAM).
 * 3. With cache.
 * 4. Physically continuous.
 *
 * All secure memory APIs are defined at <stdlib.h> as well-known memory APIs, ex: malloc.
 * Here, for programming style, some APIs are provided as TEE API style by macro define.
 */

/**
 * Allocate secure memory
 *
 * @param size    The size of the buffer to be allocated in bytes
 * @return    If success, return address. If fail, return NULL.
 */
#define MTEE_AllocMem(size) secure_malloc(size)
//#define MTEE_AllocMem(size) malloc(size)

/*fix mtee sync*/
/**
 * Allocate secure memory
 *
 * @param size    The size of the buffer to be allocated in bytes
 * @param tag     The debugging tag which can be related to module
 * @return    If success, return address. If fail, return NULL.
 */
#define MTEE_AllocMemWithTag(size, tag) secure_malloc_with_tag(size, tag)

/**
 * Free secure memory
 *
 * @param buffer    The pointer to the memory block to be referenced
 */
#define MTEE_FreeMem(buffer) secure_free(buffer)
//#define MTEE_FreeMem(buffer) free(buffer)

/**
 * Reallocation secure memory
 *
 * @param buffer    The pointer to the memory block to be realocated
 * @param size    The size of the buffer to be allocated in bytes
 * @return    If success, return address. If fail, return NULL.
 */
#define MTEE_ReallocMem(old_buffer, size) secure_realloc(old_buffer, size)
//#define MTEE_ReallocMem(old_buffer, size) realloc(old_buffer, size)

/*fix mtee sync*/
/**
 * Reallocation secure memory
 *
 * @param buffer    The pointer to the memory block to be reallocated
 * @param size    The size of the buffer to be allocated in bytes
 * @param tag     The debugging tag which can be related to module
 * @return    If success, return address. If fail, return NULL.
 */
#define MTEE_ReallocMemWithTag(old_buffer, size, tag) \
                secure_realloc_with_tag(old_buffer, size, tag)

/**
 * Calloc secure memory
 *
 * @param element    number of element wanted to allocate
 * @param elem_size    The size of one element in bytes
 * @return    If success, return address. If fail, return NULL.
 */
#define MTEE_CallocMem(element, elem_size) secure_calloc(element, elem_size)
//#define MTEE_CallocMem(element, elem_size) calloc(element, elem_size)

/*fix mtee sync*/
/**
 * Calloc secure memory
 *
 * @param element    number of element wanted to allocate
 * @param elem_size    The size of one element in bytes
 * @param tag     The debugging tag which can be related to module
 * @return    If success, return address. If fail, return NULL.
 */
#define MTEE_CallocMemWithTag(element, elem_size, tag) \
                secure_calloc_with_tag(element, elem_size, tag)

/**
 * Allocate secure memory with alignment
 *
 * @param size    The size of the buffer to be allocated in bytes
 * @param alignment    Memory alignment in bytes.
 * @return    If success, return address. If fail, return NULL.
 */
#define MTEE_AllocMemAlign(alignment, size) secure_memalign(alignment, size)
//#define MTEE_AllocMemAlign(alignment, size) memalign(alignment, size)

/*fix mtee sync*/
/**
 * Allocate secure memory with alignment
 *
 * @param size    The size of the buffer to be allocated in bytes
 * @param alignment    Memory alignment in bytes.
 * @param tag     The debugging tag which can be related to module
 * @return    If success, return address. If fail, return NULL.
 */
#define MTEE_AllocMemAlignWithTag(alignment, size, tag) secure_memalign_with_tag(alignment, size, tag)

/**
 * Query allocated secure memory size
 *
 * @param buffer    The pointer to the memory block to be queried.
 * @return    memoy size in bytes. It may be slightly larger than allcated because of alignment.
 */
#define MTEE_QueryMemsize(buffer) secure_memsize(buffer)

/**
 * Query information of secure memory
 *
 * @param info    The pointer to the memory information defined as MTEE_KERNELMEM.
 */
#define MTEE_QueryMem(info) secure_meminfo(info)

#if (GZ_API_MAIN_VERSION > 2)
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
 * 4. Physically continuous.
 * 5. For future, it can be released to normal world.
 */

/**
 * Allocate secure chunk memory
 *
 * @param size    The size of the buffer to be allocated in bytes
 * @return    If success, return address. If fail, return NULL.
 */
#define MTEE_AllocChunkmem(size) cm_malloc(size)

/*fix mtee sync*/
/**
 * Allocate secure chunk memory
 *
 * @param size    The size of the buffer to be allocated in bytes
 * @param tag     The debugging tag which can be related to module
 * @return    If success, return address. If fail, return NULL.
 */
#define MTEE_AllocChunkmemWithTag(size, tag) cm_malloc_with_tag(size, tag)

/**
 * Free secure chunk memory
 *
 * @param buffer    The pointer to the memory block to be referenced
 */
#define MTEE_FreeChunkmem(buffer) cm_free(buffer)

/**
 * Reallocation secure chunk memory
 *
 * @param buffer    The pointer to the memory block to be realocated
 * @param size    The size of the buffer to be allocated in bytes
 * @return    If success, return address. If fail, return NULL.
 */
#define MTEE_ReallocChunkmem(old_buffer, size) cm_realloc(old_buffer, size)

/*fix mtee sync*/
/**
 * Reallocation secure chunk memory
 *
 * @param buffer    The pointer to the memory block to be realocated
 * @param size    The size of the buffer to be allocated in bytes
 * @param tag     The debugging tag which can be related to module
 * @return    If success, return address. If fail, return NULL.
 */
#define MTEE_ReallocChunkmemWithTag(old_buffer, size, tag) \
                cm_realloc_with_tag(old_buffer, size, tag)

/**
 * Allocate secure chunk memory with alignment
 *
 * @param size    The size of the buffer to be allocated in bytes
 * @param alignment    Memory alignment in bytes.
 * @return    If success, return address. If fail, return NULL.
 */
#define MTEE_AllocChunkmemAlign(alignment, size) cm_memalign(alignment, size)

/*fix mtee sync*/
/**
 * Allocate secure chunk memory with alignment
 *
 * @param size    The size of the buffer to be allocated in bytes
 * @param alignment    Memory alignment in bytes.
 * @param tag     The debugging tag which can be related to module
 * @return    If success, return address. If fail, return NULL.
 */
#define MTEE_AllocChunkmemAlignWithTag(alignment, size, tag) \
                cm_memalign_with_tag(alignment, size, tag)

/**
 * Query allocated secure chunk memory size
 *
 * @param buffer    The pointer to the memory block to be queried.
 * @return    memoy size in bytes. It may be slightly larger than allcated because of alignment.
 */
#define MTEE_QueryChunkmemsize(buffer) secure_memsize(buffer)

/**
 * Query information of secure chunk memory
 *
 * @param info    The pointer to the memory information defined as MTEE_KERNELMEM.
 */
#define MTEE_QueryChunkmem(info) cm_meminfo(info)

/**
 * Get Releasable chunk memory size
 *
 * @return    Releasable chunk memory size.
 */
#define MTEE_GetReleasablesize() cm_get_releasablesize()
#endif


/* Memory TA used only
*/

#define VIRTMAPPED_MEM_BASE (0xffffffffe0000000)
#define VIRTMAPPED_MEM_SIZE (0x08000000) /* 128MB */

/**
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
 * 4. If shared memory is not used, CA deregisters it.
 * 5. CA frees memory.
 *
 * Note: Because shared memory can be seen by both Normal and Secure world.
 * It is a possible weakpoint to bed attacked or leak secure data.
 */

int MTEE_InitSharedmem (void *start, uint32_t size);
void *MTEE_AllocSharedmem (uint32_t size);
int MTEE_FreeSharedmem (void *buffer);
uint32_t *MTEE_CreateMmutbl (MTEE_SHAREDMEM *param);
uint32_t *MTEE_CreateMmutblWithTag(MTEE_SHAREDMEM *param, const char *tag);	/*fix mtee sync*/
void MTEE_DestroyMmutbl (uint32_t *mmutbl);
void MTEE_SetupMmutbl (uint32_t *mmutbl, void *va_base, uint32_t num);
void MTEE_ClearMmutbl (void *virt_addr, int size);
void MTEE_ClearMmutbl_ex (uint32_t *mmutbl, void *va_base, uint32_t num);

void MTEE_ReleaseCmMmu (void *cm_va, void *shared_va, unsigned long cm_pa, uint32_t size);
void MTEE_AppendMmu (void *cm_va, unsigned long cm_pa, void *shared_va, unsigned long shared_pa, uint32_t size);
int cm_memIsReleasable(void); /*fix mtee sync*/

int MTEE_InitNonCachedmem (void);
void *MTEE_AllocNonCachedmem (uint32_t size);
int MTEE_FreeNonCachedmem (void *buffer);

int MTEE_InitVirtMappedmem (void *start, uint32_t size);
void *MTEE_AllocVirtMappedmemAlign (uint32_t alignment, uint32_t size);
int MTEE_FreeVirtMappedmem (void *buffer);

void tz_platform_set_chunk_protection(uint64_t phys_addr, size_t size);	/*fix mtee sync*/

int MTEE_GetChunkMemoryPoolFromREE(unsigned long *chunkmem_pa, size_t *sz);	/*fix mtee sync*/
int MTEE_ReleaseChunkMemoryPoolToREE(void);	/*fix mtee sync*/
int MTEE_GetChunkMemoryPoolInfo(unsigned long *chunkmem_pa, size_t *size);	/*fix mtee sync*/
#endif /* __MTEE_MEM_H__ */

