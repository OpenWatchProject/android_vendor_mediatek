#ifndef __CPU_AE_STREAM_CORE__
#define __CPU_AE_STREAM_CORE__

#include "algorithm/ae_algo_ctrl.h"

typedef enum
{
    CPU_AE_CORE_CAM_ID_MAIN,
    CPU_AE_CORE_CAM_ID_MAIN2,
    CPU_AE_CORE_CAM_ID_MAIN3,
    CPU_AE_CORE_CAM_ID_SUB,
    CPU_AE_CORE_CAM_ID_SUB2,
    CPU_AE_CORE_CAM_ID_NUM
}   CPU_AE_CORE_CAM_ID_ENUM;

#ifdef __cplusplus
extern "C" {
#endif


void CPU_AE_Core_Init(AE_CORE_INIT *Init_in, CPU_AE_CORE_CAM_ID_ENUM cam_id);

void CPU_AE_Core_Ctrl(AE_CTRL_ENUM id, void *ctrl_in, void *ctrl_out, CPU_AE_CORE_CAM_ID_ENUM cam_id);

void CPU_AE_Core_Stat(AE_CORE_MAIN_IN *in, AE_CORE_MAIN_OUT *out, CPU_AE_CORE_CAM_ID_ENUM cam_id);

void CPU_AE_Core_Main(AE_CORE_MAIN_IN *in, AE_CORE_MAIN_OUT *out, CPU_AE_CORE_CAM_ID_ENUM cam_id);


#ifdef __cplusplus
}
#endif



#endif

