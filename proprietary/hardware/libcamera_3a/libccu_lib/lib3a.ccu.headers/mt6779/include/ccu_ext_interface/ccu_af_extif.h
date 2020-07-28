#ifndef __CCU_AF_INTERFACE__
#define __CCU_AF_INTERFACE__

#include "ccu_ext_interface/ccu_types.h"
#ifndef __PLATFORM__
#include <mtkcam/def/BuiltinTypes.h>
#endif
#include "ccu_lens_extif.h"
#include "ccu_ext_interface/af_cxu_param.h"

/******************************************************************************
* AF Command ID definition
******************************************************************************/
#define AFO_BUFFER_COUNT 6
#define Half_AFO_BUFFER_COUNT AFO_BUFFER_COUNT/2
#define AF_REG_COUNT 4
#define Half_AF_REG_COUNT AF_REG_COUNT/2

#define CCU_AF_INSTANCE_CAPACITY 2

enum ccu_msg_id_af
{
    /*To identify ccu control msg count*/
    DELIMITER_AF_MSG_MIN = 0,
    /*Receive by CCU*/
    MSG_TO_CCU_AF_INIT = DELIMITER_AF_MSG_MIN,
    MSG_TO_CCU_AF_START,
    MSG_TO_CCU_AF_ALGO_DONE,
    MSG_TO_CCU_AF_ACQUIRE_AFO_BUFFER,
    MSG_TO_CCU_AF_SET_HW_REG,
    MSG_TO_CCU_AF_STOP,
    MSG_TO_CCU_AF_ABORT,
    MSG_TO_CCU_AF_CONTROL_INIT,
    MSG_TO_CCU_AF_HW_INTI,
    MSG_TO_CCU_AF_START_ALGO_INIT,
    MSG_TO_CCU_AF_DOAF,
    MSG_TO_CCU_AF_GET_DOAF_OUTPUT,
    MSG_TO_CCU_AF_GET_CURPOSTION,
    MSG_TO_CCU_AF_GET_PDBLOCK,
    MSG_TO_CCU_AF_GET_DEBUGINFO,
    MSG_TO_CCU_AF_SYNC,
    MSG_TO_CCU_AF_BIND_INSTANCE,
    /*To identify ccu control msg count*/
    DELIMITER_AF_MSG_1,
    /*CCU internal task generated in HW isr*/
    MSG_CCU_INTERNAL_AF_VSYNC = DELIMITER_AF_MSG_1,
    MSG_CCU_INTERNAL_AF_CQ0,
    MSG_CCU_INTERNAL_AF_AFO,
    MSG_CCU_INTERNAL_AF_CQ0B,
    MSG_CCU_INTERNAL_AF_AFOB,
    MSG_CCU_INTERNAL_TIMER1_AF,
    MSG_CCU_INTERNAL_SHUTDOWN_AF,

    /*To identify ccu control msg count*/
    DELIMITER_AF_MSG_MAX
};


/******************************************************************************
* AF related interface
******************************************************************************/
typedef struct AFOINIT_INFO_IN
{
    U32 AFOBufsAddr[Half_AFO_BUFFER_COUNT];
    U32 AFORegAddr[Half_AF_REG_COUNT];

    U32 magic_reg;
    U32 twin_reg;
    LENS_INFO_IN_T lens_info;

} AFOINIT_INFO_IN_T;

typedef struct AFSetHWReg_INFO_IN
{
    U32 AFORegAddr;
} AFSetHWReg_INFO_IN_T;

typedef struct __attribute__ ((aligned (8))) AFAcquireBuf_Reg_INFO_OUT_AP
{
    uint64_t AFOBufAddr;//va
    U32 AFOBufmva;
    U32 AFOBufStride;
    U32 AFOBufConfigNum;
    U32 AFOBufMaxSize;
    U32 AFOBufMagic_num;
    U32 AFORegAddr;
    U32 u4NumBlkX ;
    U32 u4NumBlkY ;
    U32 u4SizeBlkX;
    U32 u4SizeBlkY;
    U32 is_ABonly;
} AFAcquireBuf_Reg_INFO_OUT_AP_T;

typedef struct __attribute__ ((aligned (8))) af_shared_buf_map
{
    MUINT32 lens_curpos;
    MUINT32 move_lens_curpos_time_L;
    MUINT32 move_lens_curpos_time_H;
    MUINT32 lens_lastpos;
    MUINT32 move_lens_lastpos_time_L;
    MUINT32 move_lens_lastpos_time_H;
    AFAcquireBuf_Reg_INFO_OUT_AP_T AFO_info;
    StartAFOutput_T     afstartOutput;
    MUINT32 af_init_done;
    MUINT32 af_hwinit_done;
} af_shared_buf_map_T;

struct ccu_af_bind_instance_output
{
    uint32_t instance_idx;
};
#endif
