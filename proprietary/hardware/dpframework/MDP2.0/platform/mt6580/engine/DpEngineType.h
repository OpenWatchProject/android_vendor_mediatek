#ifndef __DP_ENGINE_TYPE_H__
#define __DP_ENGINE_TYPE_H__

#include "DpConfig.h"
#include "DpDataType.h"

#include "cmdq_def.h"
#include "mdp_def_ex.h"

#define DP_GET_ENGINE_NAME(value, pName)                \
{                                                       \
    switch(value)                                       \
    {                                                   \
        case tIMGI:                                     \
        pName = (char*)"tIMGI";                         \
        break;                                          \
        case tIMGO:                                     \
        pName = (char*)"tIMGO";                         \
        break;                                          \
        case tIMG2O:                                    \
        pName = (char*)"tIMG2O";                        \
        break;                                          \
        case tRDMA0:                                    \
        pName = (char*)"tRDMA0";                        \
        break;                                          \
        case tCAMIN:                                    \
        pName = (char*)"tCAMIN";                        \
        break;                                          \
        case tSCL0:                                     \
        pName = (char*)"tSCL0";                         \
        break;                                          \
        case tSCL1:                                     \
        pName = (char*)"tSCL1";                         \
        break;                                          \
        case tTDSHP0:                                   \
        pName = (char*)"tTDSHP0";                       \
        break;                                          \
        case tWROT0:                                    \
        pName = (char*)"tWROT0";                        \
        break;                                          \
        case tWDMA:                                     \
        pName = (char*)"tWDMA";                         \
        break;                                          \
        case tJPEGENC:                                  \
        pName = (char*)"tJPEGENC";                      \
        break;                                          \
        case tJPEGDEC:                                  \
        pName = (char*)"tJPEGDEC";                      \
        break;                                          \
        case tVENC:                                     \
        pName = (char*)"tVENC";                         \
        break;                                          \
        default:                                        \
        pName = (char*)"tNone";                         \
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
#define tSCL0       CMDQ_ENG_MDP_RSZ0
#define tSCL1       CMDQ_ENG_MDP_RSZ1
#define tTDSHP0     CMDQ_ENG_MDP_TDSHP0
#define tWROT0      CMDQ_ENG_MDP_WROT0
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

// Fake
#define tRDMA1      tNone
#define tWROT1      tNone
#define tSCL2       tNone

#endif  // __DP_ENGINE_TYPE_H__
