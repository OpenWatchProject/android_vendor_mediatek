#ifndef __DP_ENGINE_TYPE_H__
#define __DP_ENGINE_TYPE_H__

#include "DpConfig.h"
#include "DpDataType.h"

#include "cmdq_def.h"
#include "mdp_def_ex.h"

#define DP_GET_ENGINE_NAME_CASE(_value, _name)          \
    case (_value):                                      \
    _name = #_value;                                    \
    break;
#define DP_GET_ENGINE_NAME(value, name)                 \
{                                                       \
    switch(value)                                       \
    {                                                   \
        DP_GET_ENGINE_NAME_CASE(tIMGI, name)            \
        DP_GET_ENGINE_NAME_CASE(tIMGO, name)            \
        DP_GET_ENGINE_NAME_CASE(tIMG2O, name)           \
        DP_GET_ENGINE_NAME_CASE(tCAMIN, name)           \
        DP_GET_ENGINE_NAME_CASE(tRDMA0, name)           \
        DP_GET_ENGINE_NAME_CASE(tRDMA1, name)           \
        DP_GET_ENGINE_NAME_CASE(tSCL0, name)            \
        DP_GET_ENGINE_NAME_CASE(tSCL1, name)            \
        DP_GET_ENGINE_NAME_CASE(tSCL2, name)            \
        DP_GET_ENGINE_NAME_CASE(tTDSHP0, name)          \
        DP_GET_ENGINE_NAME_CASE(tCOLOR0, name)          \
        DP_GET_ENGINE_NAME_CASE(tWROT0, name)           \
        DP_GET_ENGINE_NAME_CASE(tWROT1, name)           \
        DP_GET_ENGINE_NAME_CASE(tWDMA, name)            \
        DP_GET_ENGINE_NAME_CASE(tJPEGENC, name)         \
        DP_GET_ENGINE_NAME_CASE(tVENC, name)            \
        DP_GET_ENGINE_NAME_CASE(tJPEGDEC, name)         \
        default:                                        \
        name = (char*)"tNone";                          \
        assert(0);                                      \
        break;                                          \
    }                                                   \
}

// Invalid engine type
#define tNone       -1
//CAM
#define tIMGI       CMDQ_ENG_ISP_IMGI
#define tIMGO       CMDQ_ENG_ISP_IMGO
#define tIMG2O      CMDQ_ENG_ISP_IMG2O
//MDP
#define tCAMIN      CMDQ_ENG_MDP_CAMIN
#define tRDMA0      CMDQ_ENG_MDP_RDMA0
#define tRDMA1      CMDQ_ENG_MDP_RDMA1
#define tSCL0       CMDQ_ENG_MDP_RSZ0
#define tSCL1       CMDQ_ENG_MDP_RSZ1
#define tSCL2       CMDQ_ENG_MDP_RSZ2
#define tTDSHP0     CMDQ_ENG_MDP_TDSHP0
#define tCOLOR0     CMDQ_ENG_MDP_COLOR0
#define tWROT0      CMDQ_ENG_MDP_WROT0
#define tWROT1      CMDQ_ENG_MDP_WROT1
#define tWDMA       CMDQ_ENG_MDP_WDMA
//JPEG
#define tJPEGENC    CMDQ_ENG_JPEG_ENC
#define tVENC       CMDQ_ENG_VIDEO_ENC
#define tTotal      (tVENC+1)
#define tJPEGDEC    CMDQ_ENG_JPEG_DEC
#define tJPEGREMDC  CMDQ_ENG_JPEG_REMDC
//PQ
#define tCOLOR_EX   CMDQ_ENG_DISP_COLOR0
#define tOVL0_EX    CMDQ_ENG_DISP_OVL0
#define tWDMA_EX    CMDQ_ENG_DISP_WDMA0

#endif  // __DP_ENGINE_TYPE_H__
