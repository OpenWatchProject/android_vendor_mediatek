#ifndef __ION_COPY_H__
#define __ION_COPY_H__
#include <linux/ion.h>

//copy paste :/device/mediatek/common/kernel-headers/linux/mtk_ion.h
 enum mtk_ion_heap_type {                                                                   
	 ION_HEAP_TYPE_MULTIMEDIA = 10,                                                             
	 ION_HEAP_TYPE_FB = 11,                                                                     
	 ION_HEAP_TYPE_MULTIMEDIA_FOR_CAMERA = 12,                                                  
	 ION_HEAP_TYPE_MULTIMEDIA_SEC = 13,                                                         
	 ION_HEAP_TYPE_MULTIMEDIA_MAP_MVA = 14,                                                     
	 ION_HEAP_TYPE_MULTIMEDIA_PROT = 16,                                                        
	 ION_HEAP_TYPE_MULTIMEDIA_2D_FR = 17,                                                       
	 ION_HEAP_TYPE_MULTIMEDIA_WFD = 18,                                                         
};                                                                                          
#define ION_HEAP_MULTIMEDIA_MASK (1 << ION_HEAP_TYPE_MULTIMEDIA)                            
#define ION_HEAP_FB_MASK (1 << ION_HEAP_TYPE_FB)                                            
#define ION_HEAP_CAMERA_MASK (1 << ION_HEAP_TYPE_MULTIMEDIA_FOR_CAMERA)                     
#define ION_HEAP_MULTIMEDIA_SEC_MASK (1 << ION_HEAP_TYPE_MULTIMEDIA_SEC)                    
#define ION_HEAP_MULTIMEDIA_MAP_MVA_MASK (1 << ION_HEAP_TYPE_MULTIMEDIA_MAP_MVA)            
#define ION_HEAP_MULTIMEDIA_TYPE_PROT_MASK (1 << ION_HEAP_TYPE_MULTIMEDIA_PROT)             
#define ION_HEAP_MULTIMEDIA_TYPE_2D_FR_MASK (1 << ION_HEAP_TYPE_MULTIMEDIA_2D_FR)           
#define ION_HEAP_WFD_MASK	(1 << ION_HEAP_TYPE_MULTIMEDIA_WFD)                               
                                                                                            
//#define ION_NUM_HEAP_IDS sizeof(unsigned int) * 8                                           


//copy paste : /device/mediatek/common/kernel-headers/linux/ion_drv.h 
#define BACKTRACE_SIZE 10

/* Structure definitions */

typedef enum {
 ION_CMD_SYSTEM,
 ION_CMD_MULTIMEDIA,
 ION_CMD_MULTIMEDIA_SEC
} ION_CMDS;

typedef enum {
 ION_MM_CONFIG_BUFFER,
 ION_MM_SET_DEBUG_INFO,
 ION_MM_GET_DEBUG_INFO,
 ION_MM_SET_SF_BUF_INFO,
 ION_MM_GET_SF_BUF_INFO,
 ION_MM_CONFIG_BUFFER_EXT,
 ION_MM_ACQ_CACHE_POOL,
 ION_MM_QRY_CACHE_POOL,
 ION_MM_GET_IOVA,
 ION_MM_GET_IOVA_EXT,
} ION_MM_CMDS;

typedef enum {
 ION_SYS_CACHE_SYNC,
 ION_SYS_GET_PHYS,
 ION_SYS_GET_CLIENT,
 ION_SYS_SET_HANDLE_BACKTRACE,
 ION_SYS_SET_CLIENT_NAME,
 ION_SYS_DMA_OP,
} ION_SYS_CMDS;

typedef enum {
 ION_CACHE_CLEAN_BY_RANGE,
 ION_CACHE_INVALID_BY_RANGE,
 ION_CACHE_FLUSH_BY_RANGE,
 ION_CACHE_CLEAN_BY_RANGE_USE_VA,
 ION_CACHE_INVALID_BY_RANGE_USE_VA,
 ION_CACHE_FLUSH_BY_RANGE_USE_VA,
 ION_CACHE_CLEAN_ALL,
 ION_CACHE_INVALID_ALL,
 ION_CACHE_FLUSH_ALL
} ION_CACHE_SYNC_TYPE;

typedef enum {
 ION_ERROR_CONFIG_LOCKED = 0x10000
} ION_ERROR_E;

/* mm or mm_sec heap flag which is do not conflist with ION_HEAP_FLAG_DEFER_FREE */
#define ION_FLAG_MM_HEAP_INIT_ZERO (1 << 16)
#define ION_FLAG_MM_HEAP_SEC_PA (1 << 18)

#define ION_FLAG_GET_FIXED_PHYS 0x103

typedef struct ion_sys_cache_sync_param {
 union {
   ion_user_handle_t handle;
   void * kernel_handle;
 };
 void * va;
 unsigned int size;
 ION_CACHE_SYNC_TYPE sync_type;
} ion_sys_cache_sync_param_t;

typedef enum {
 ION_DMA_MAP_AREA,
 ION_DMA_UNMAP_AREA,
 ION_DMA_MAP_AREA_VA,
 ION_DMA_UNMAP_AREA_VA,
 ION_DMA_FLUSH_BY_RANGE,
 ION_DMA_FLUSH_BY_RANGE_USE_VA,
 ION_DMA_CACHE_FLUSH_ALL
} ION_DMA_TYPE;

typedef enum {
 ION_DMA_FROM_DEVICE,
 ION_DMA_TO_DEVICE,
 ION_DMA_BIDIRECTIONAL,
} ION_DMA_DIR;

typedef struct ion_dma_param {
 union {
   ion_user_handle_t handle;
   void * kernel_handle;
 };
 void *va;
 unsigned int size;
 ION_DMA_TYPE dma_type;
 ION_DMA_DIR dma_dir;
} ion_sys_dma_param_t;

typedef struct ion_sys_get_phys_param {
 union {
   ion_user_handle_t handle;
   void * kernel_handle;
 };
 unsigned int phy_addr;
 unsigned long len;
} ion_sys_get_phys_param_t;

#define ION_MM_DBG_NAME_LEN 48
#define ION_MM_SF_BUF_INFO_LEN 16

typedef struct __ion_sys_client_name {
 char name[ION_MM_DBG_NAME_LEN];
} ion_sys_client_name_t;

typedef struct ion_sys_get_client_param {
 unsigned int client;
} ion_sys_get_client_param_t;

typedef struct ion_sys_record_param {
 pid_t group_id;
 pid_t pid;
 unsigned int action;
 unsigned int address_type;
 unsigned int address;
 unsigned int length;
 unsigned int backtrace[BACKTRACE_SIZE];
 unsigned int backtrace_num;
 struct ion_handle * handle;
 struct ion_client * client;
 struct ion_buffer * buffer;
 struct file * file;
 int fd;
} ion_sys_record_t;

typedef struct ion_sys_data {
 ION_SYS_CMDS sys_cmd;
 union {
   ion_sys_cache_sync_param_t cache_sync_param;
   ion_sys_get_phys_param_t get_phys_param;
   ion_sys_get_client_param_t get_client_param;
   ion_sys_client_name_t client_name_param;
   ion_sys_record_t record_param;
   ion_sys_dma_param_t dma_param;
 };
} ion_sys_data_t;

typedef struct ion_mm_config_buffer_param {
 union {
    ion_user_handle_t handle;
    void * kernel_handle;
 };
 int eModuleID;
 unsigned int security;
 unsigned int coherent;
 unsigned int reserve_iova_start;
 unsigned int reserve_iova_end;
} ion_mm_config_buffer_param_t;

typedef struct __ion_mm_buf_debug_info {
 union {
   ion_user_handle_t handle;
   void * kernel_handle;
 };
 char dbg_name[ION_MM_DBG_NAME_LEN];
 unsigned int value1;
 unsigned int value2;
 unsigned int value3;
 unsigned int value4;
} ion_mm_buf_debug_info_t;

typedef struct __ion_mm_sf_buf_info {
 union {
   ion_user_handle_t handle;
   void * kernel_handle;
 };
 unsigned int info[ION_MM_SF_BUF_INFO_LEN];
} ion_mm_sf_buf_info_t;

typedef struct __ion_mm_cache_pool_info {
 size_t len;
 size_t align;
 unsigned int heap_id_mask;
 unsigned int flags;
 unsigned int ret;
} ion_mm_cache_pool_info_t;

typedef struct ion_mm_get_iova_param {
 union {
   ion_user_handle_t handle;
   struct ion_handle *kernel_handle;
 };
 int module_id;
 unsigned int security;
 unsigned int coherent;
 unsigned int reserve_iova_start;
 unsigned int reserve_iova_end;
 unsigned long long phy_addr;
 unsigned long len;
} ion_mm_get_iova_param_t;

typedef struct ion_mm_data {
 ION_MM_CMDS mm_cmd;
 union {
   ion_mm_config_buffer_param_t config_buffer_param;
   ion_mm_buf_debug_info_t buf_debug_info_param;
   ion_mm_sf_buf_info_t sf_buf_info_param;
   ion_mm_cache_pool_info_t cache_pool_info_param;
   ion_mm_get_iova_param_t get_phys_param;
 };
} ion_mm_data_t;
 


#endif  // __ION_COPY_H__
