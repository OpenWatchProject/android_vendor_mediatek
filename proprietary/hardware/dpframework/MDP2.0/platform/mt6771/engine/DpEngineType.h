#ifndef __DP_ENGINE_TYPE_H__
#define __DP_ENGINE_TYPE_H__

#include "DpConfig.h"
#include "DpDataType.h"

#include "cmdq_v3_def.h"
#include "mdp_def_ex.h"

#define DP_GET_ENGINE_NAME_CASE(_value, _name)          \
    case (_value):                                      \
    _name = #_value;                                    \
    break;
#define DP_GET_ENGINE_NAME(value, name)                 \
{                                                       \
    switch(value)                                       \
    {                                                   \
        DP_GET_ENGINE_NAME_CASE(tWPEI, name)            \
        DP_GET_ENGINE_NAME_CASE(tWPEO, name)            \
        DP_GET_ENGINE_NAME_CASE(tWPEI2, name)           \
        DP_GET_ENGINE_NAME_CASE(tWPEO2, name)           \
        DP_GET_ENGINE_NAME_CASE(tIMGI, name)            \
        DP_GET_ENGINE_NAME_CASE(tIMGO, name)            \
        DP_GET_ENGINE_NAME_CASE(tIMG2O, name)           \
        DP_GET_ENGINE_NAME_CASE(tIPUI, name)            \
        DP_GET_ENGINE_NAME_CASE(tIPUO, name)            \
        DP_GET_ENGINE_NAME_CASE(tCAMIN, name)           \
        DP_GET_ENGINE_NAME_CASE(tCAMIN2, name)          \
        DP_GET_ENGINE_NAME_CASE(tRDMA0, name)           \
        DP_GET_ENGINE_NAME_CASE(tAAL0, name)           \
        DP_GET_ENGINE_NAME_CASE(tCCORR0, name)           \
        DP_GET_ENGINE_NAME_CASE(tSCL0, name)            \
        DP_GET_ENGINE_NAME_CASE(tSCL1, name)            \
        DP_GET_ENGINE_NAME_CASE(tTDSHP0, name)          \
        DP_GET_ENGINE_NAME_CASE(tCOLOR0, name)          \
        DP_GET_ENGINE_NAME_CASE(tPATH0_SOUT, name)      \
        DP_GET_ENGINE_NAME_CASE(tPATH1_SOUT, name)      \
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
#define tWPEI         CMDQ_ENG_WPEI
#define tWPEO         CMDQ_ENG_WPEO
#define tWPEI2        CMDQ_ENG_WPEI2
#define tWPEO2        CMDQ_ENG_WPEO2
#define tIMGI         CMDQ_ENG_ISP_IMGI
#define tIMGO         CMDQ_ENG_ISP_IMGO
#define tIMG2O        CMDQ_ENG_ISP_IMG2O
//IPU
#define tIPUI         CMDQ_ENG_IPUI
#define tIPUO         CMDQ_ENG_IPUO
//MDP
#define tCAMIN        CMDQ_ENG_MDP_CAMIN
#define tCAMIN2       CMDQ_ENG_MDP_CAMIN2
#define tRDMA0        CMDQ_ENG_MDP_RDMA0
#define tRDMA1        tNone
#define tAAL0         CMDQ_ENG_MDP_AAL0
#define tCCORR0       CMDQ_ENG_MDP_CCORR0
#define tSCL0         CMDQ_ENG_MDP_RSZ0
#define tSCL1         CMDQ_ENG_MDP_RSZ1
#define tSCL2         tNone
#define tTDSHP0       CMDQ_ENG_MDP_TDSHP0
#define tCOLOR0       CMDQ_ENG_MDP_COLOR0
#define tWROT0        CMDQ_ENG_MDP_WROT0
#define tWROT1        tNone
#define tWDMA         CMDQ_ENG_MDP_WDMA
#define tPATH0_SOUT   CMDQ_ENG_MDP_PATH0_SOUT
#define tPATH1_SOUT   CMDQ_ENG_MDP_PATH1_SOUT
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
