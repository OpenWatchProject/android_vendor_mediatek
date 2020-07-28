#ifndef __DP_ENGINE_TYPE_H__
#define __DP_ENGINE_TYPE_H__

#include "DpConfig.h"
#include "DpDataType.h"

#if defined(CMDQ_V3)
#include "cmdq_v3_driver.h"
#elif defined(CMDQ_K414)
#include "cmdq_k414_driver.h"
#endif
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
        DP_GET_ENGINE_NAME_CASE(tCCORR0, name)          \
        DP_GET_ENGINE_NAME_CASE(tSCL0, name)            \
        DP_GET_ENGINE_NAME_CASE(tSCL1, name)            \
        DP_GET_ENGINE_NAME_CASE(tTDSHP0, name)          \
        DP_GET_ENGINE_NAME_CASE(tCOLOR0, name)          \
        DP_GET_ENGINE_NAME_CASE(tWROT0, name)           \
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
#define tWPEI         tNone
#define tWPEO         tNone
#define tWPEI2        tNone
#define tWPEO2        tNone
#define tIMGI         CMDQ_ENG_ISP_IMGI
#define tIMGO         CMDQ_ENG_ISP_IMGO
#define tIMG2O        CMDQ_ENG_ISP_IMG2O
#define tIMG3O        CMDQ_ENG_ISP_IMG3O
#define tVIPI         CMDQ_ENG_ISP_VIPI
#define tLCEI         CMDQ_ENG_ISP_LCEI

//IPU
#define tIPUI         tNone
#define tIPUO         tNone
//MDP
#define tCAMIN        CMDQ_ENG_MDP_CAMIN
#define tCAMIN2       tNone
#define tRDMA0        CMDQ_ENG_MDP_RDMA0
#define tRDMA1        tNone
#define tAAL0         tNone
#define tCCORR0       CMDQ_ENG_MDP_CCORR0
#define tSCL0         CMDQ_ENG_MDP_RSZ0
#define tSCL1         CMDQ_ENG_MDP_RSZ1
#define tSCL2         tNone
#define tTDSHP0       CMDQ_ENG_MDP_TDSHP0
#define tCOLOR0       CMDQ_ENG_MDP_COLOR0
#define tWROT0        CMDQ_ENG_MDP_WROT0
#define tWROT1        tNone
#define tWDMA         CMDQ_ENG_MDP_WDMA
#define tPATH0_SOUT   tNone
#define tPATH1_SOUT   tNone
//DISP
#define tTO_WROT      tNone
#define tTO_WROT_SOUT tNone
#define tTO_DISP0     tNone
#define tTO_DISP1     tNone
#define tTO_WDMA      tNone
#define tTO_RSZ       tNone
//JPEG
#define tJPEGENC      CMDQ_ENG_JPEG_ENC
#define tVENC         CMDQ_ENG_VIDEO_ENC
#define tTotal        (tVENC+1)
#define tJPEGDEC      CMDQ_ENG_JPEG_DEC
#define tJPEGREMDC    CMDQ_ENG_JPEG_REMDC
//PQ
#define tCOLOR_EX     CMDQ_ENG_DISP_COLOR0
#define tOVL0_EX      CMDQ_ENG_DISP_OVL0
#define tWDMA_EX      CMDQ_ENG_DISP_WDMA0

#endif  // __DP_ENGINE_TYPE_H__
