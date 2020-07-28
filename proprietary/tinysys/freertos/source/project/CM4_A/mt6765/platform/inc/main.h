/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
/* Includes ------------------------------------------------------------------*/
#include "stdio.h"


/******************************************************************************
* 1. New entries must be appended to the end of the structure.
* 2. Do NOT use conditional option such as #ifdef inside the structure.
******************************************************************************/
struct scp_region_info_st {
    unsigned int ap_loader_start;
    unsigned int ap_loader_size;
    unsigned int ap_firmware_start;
    unsigned int ap_firmware_size;
    unsigned int ap_cached_start;
    unsigned int ap_cached_size;
    unsigned int ap_cached_backup_start;
    unsigned int struct_size;
                    // This is the size of the structure. It can act as a
                    // version number if entries can only be added to (not
                    // deleted from) the structure. It should be the first
                    // entry of the structure, but for compatibility reason it
                    // is appended here.
    unsigned int scp_log_thru_ap_uart;
    unsigned int TaskContext_ptr;
                    // TaskContext_ptr is the address of taskCtxDump which
                    // save the information of CPU stack.
                    // Bit 31 in TaskContext_ptr (i.e., 0x80000000) is set when
                    // the AP WDT event is triggered. The bit is used in LK
                    // to identify this case.
    unsigned int Il1c_con;
    unsigned int Dl1c_con;
    unsigned int scpctl;
    unsigned int ap_params_start;
};

#define REGION_INFO_BASE 0x400


/* Exported macro ------------------------------------------------------------*/
#ifdef CFG_MTK_DYNAMIC_AP_UART_SWITCH
#define DYNAMIC_AP_UART_SWITCH      1
#else
#define DYNAMIC_AP_UART_SWITCH      0
#endif  // CFG_MTK_DYNAMIC_AP_UART_SWITCH


#define INFO_BASE_POINTER           ((struct scp_region_info_st *)REGION_INFO_BASE)
#define INFO_AP_LOADER_START        (INFO_BASE_POINTER->ap_loader_start)
#define INFO_AP_LOADER_SIZE         (INFO_BASE_POINTER->ap_loader_size)
#define INFO_AP_FIRMWARE_START      (INFO_BASE_POINTER->ap_firmware_start)
#define INFO_AP_FIRMWARE_SIZE       (INFO_BASE_POINTER->ap_firmware_size)
#define INFO_AP_CACHED_START        (INFO_BASE_POINTER->ap_cached_start)
#define INFO_AP_CACHED_SIZE         (INFO_BASE_POINTER->ap_cached_size)
#define INFO_AP_CACHED_BACKUP_START (INFO_BASE_POINTER->ap_cached_backup_start)
#define INFO_STRUCT_SIZE            (INFO_BASE_POINTER->struct_size)
#define INFO_SCP_LOG_THRU_AP_UART   (INFO_BASE_POINTER->scp_log_thru_ap_uart)

#define IS_INFO_SIZE_OKAY()         (INFO_STRUCT_SIZE >= sizeof(struct scp_region_info_st))


/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */

