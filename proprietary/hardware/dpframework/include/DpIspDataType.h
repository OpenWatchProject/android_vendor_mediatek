#ifndef __DP_ISP_DATA_TYPE_H__
#define __DP_ISP_DATA_TYPE_H__
#include "cmdq_k414_def.h"

typedef struct ISP2MDP_STRUCT {
    char *MET_String;
    uint32_t MET_String_length;
    cmdqSecIspMeta secIspData;
}ISP2MDP_STRUCT;

#endif  // __DP_ISP_DATA_TYPE_H__
