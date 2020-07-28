#include <sys/types.h>
#include "mtee_srv.h"
//#include <tz_private/ktee_sys_mem.h>
#include <tz_private/ta_sys_mem.h>
#include <tz_private/mtee_sys_fd.h>

/* Generic Handle Manager */
typedef void *MTEE_HANDLE_TYPE_ID;


/* Command for Secure Memory Management */
enum MTEE_MEM_USER_CMD {
	MTEE_MEM_SECUREMEM_ALLOC = 0x1,
	MTEE_MEM_SECUREMEM_REF,
	MTEE_MEM_SECUREMEM_UNREF,
	MTEE_MEM_SECUREMEM_QUERY,
	MTEE_MEM_SHAREDMEM_REG,
	MTEE_MEM_SHAREDMEM_UNREG,
	MTEE_MEM_SHAREDMEM_QUERY,
	MTEE_MEM_SET_EXTRA,
	MTEE_MEM_GET_EXTRA,
	MTEE_MEM_COPY_EXTRA,
	MTEE_MEM_SECUREMEM_ZALLOC,
	MTEE_MEM_SECUREMEM_NUM,
	MTEE_MEM_APPEND_MULTI_CHUNKMEM,
	MTEE_MEM_APPEND_MULTI_CHUNKMEM_ION,
	MTEE_MEM_RELEASE_MULTI_CHUNKMEM,
	MTEE_MEM_RELEASE_MULTI_CHUNKMEM_ION,
	MTEE_MEM_SECUREMULTICHUNKMEM_ALLOC,
	MTEE_MEM_SECUREMULTICHUNKMEM_ZALLOC,
	MTEE_MEM_SECUREMULTICHUNKMEM_REF,
	MTEE_MEM_SECUREMULTICHUNKMEM_UNREF,
	MTEE_MEM_SECUREMULTICHUNKMEM_QUERY,
	MTEE_MEM_SECUREMULTICHUNKMEM_INFO_QUERY,
	MTEE_MEM_SECUREMULTICHUNKMEM_QUERY_DEBUG,
	MTEE_MEM_SECUREMULTICHUNKMEM_QUERY_PA,
	MTEE_MEM_QUERY_IONHandle,
	MTEE_MEM_SECUREMEM_ALLOC_WITH_TAG,
	MTEE_MEM_SECUREMEM_ZALLOC_WITH_TAG,
	PLAT_INIT_SHMEM_FOR_PARAM,
	MTEE_MEM_SECUREMULTICHUNKMEM_QUERY_GZ_PA,
	MTEE_MEM_QUERY_PROJECT_BUILD_TYPE,
	MTEE_MEM_CONFIG_CHUNKMEM_INFO_ION
};

typedef struct secure_mem {
	//user_session_t session;	// the user session requesting secure memory
	uint32_t region_id;
	uint32_t region_id2;
	int size;				// input: requested secure memory size
	int alignment;			// input: memalign boundary if required, 0 if no alignment
	MTEE_SECUREMEM_HANDLE mem_handle;			// output: memory handle id
	MTEE_SECUREMEM_HANDLE chm_mem_handle;	//add on 2018/2/1 for multi-chunk memory
	uint64_t key;			//add on 2018/2/1 for multi-chunk memory auth.
	uint64_t addr;
	int ref_count;			// output: reference count
	uint64_t paddr;
	uint64_t tag;			//add on 2018/02/12
	uint64_t *mmuTable; 	//add on 2017/01/19
} secure_mem_t;

typedef struct _shared_mem {
	//user_session_t session;	// the user session requesting secure memory
	int size;				// input: requested secure memory size
	int alignment;			// input: memalign boundary if required, 0 if no alignment
	MTEE_SHAREDMEM_HANDLE mem_handle;			// output: memory handle id
	uint64_t addr;
	int ref_count;			// output: reference count
	uint64_t paddr;
	uint64_t *mmuTable; 	//add on 2017/01/19
} shared_mem_t;


