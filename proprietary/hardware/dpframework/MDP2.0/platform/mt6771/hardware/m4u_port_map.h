#ifndef __M4U_PORT_MAP_H__
#define __M4U_PORT_MAP_H__

#include "DpLogger.h"
#if defined(MTK_M4U_SUPPORT)
#include <m4u_lib.h>
#else
#include "mt_iommu_port.h"
#endif //MTK_M4U_SUPPORT

static M4U_MODULE_ID_ENUM convertType(DpEngineType type,
                                      int32_t)
{
    switch (type)
    {
        case tIMGI:
            return M4U_PORT_CAM_IMGI;

        case tIMGO:
            return M4U_PORT_CAM_IMGO;

        case tIMG2O:
            return M4U_PORT_CAM_IMG2O;

        case tRDMA0:
            return M4U_PORT_MDP_RDMA0;

        case tWROT0:
            return M4U_PORT_MDP_WROT0;

        case tWDMA:
            return M4U_PORT_MDP_WDMA0;

        case tWPEI:
            return M4U_PORT_CAM_WPE0_RDMA0;

        case tWPEO:
            return M4U_PORT_CAM_WPE0_WDMA;

        /*
        case tJPEGENC:
            if(plane == 0)  //for top to map bitstream output mva
                return M4U_PORT_REMDC_WDMA;
            else if(plane == 1)
                return M4U_PORT_JPGENC_BSDMA;
            else
                return M4U_PORT_JPGENC_SDMA;

        case tJPEGREMDC:
             if(plane == 0)
                return M4U_PORT_REMDC_WDMA;
             else if(plane == 1)
                return M4U_PORT_REMDC_BSDMA;
             else
                return M4U_PORT_REMDC_SDMA;
        */

        case tOVL0_EX:
            return M4U_PORT_DISP_OVL0;

        case tWDMA_EX:
            return M4U_PORT_DISP_WDMA0;

        default:
            DPLOGE("DpMmuHandler: unknown engine type: %d\n", type);
            assert(0);
            break;
    }

    return M4U_PORT_UNKNOWN;
}

static M4U_PORT_ID_ENUM convertPort(DpEngineType type,
                                    int32_t)
{
    switch (type)
    {
        case tIMGI:
            return M4U_PORT_CAM_IMGI;

        case tIMGO:
            return M4U_PORT_CAM_IMGO;

        case tIMG2O:
            return M4U_PORT_CAM_IMG2O;

        case tRDMA0:
            return M4U_PORT_MDP_RDMA0;


        case tWROT0:
            return M4U_PORT_MDP_WROT0;

        case tWDMA:
            return M4U_PORT_MDP_WDMA0;
        /*
        case tJPEGENC:
            if(plane == 0)  //for top to map bitstream output mva
                return M4U_PORT_REMDC_WDMA;
            else if(plane == 1)
              return M4U_PORT_JPGENC_BSDMA;
            else
              return M4U_PORT_JPGENC_SDMA;

        case tJPEGREMDC:
             if(plane == 0)
                return M4U_PORT_REMDC_WDMA;
             else if(plane == 1)
                return M4U_PORT_REMDC_BSDMA;
             else
                return M4U_PORT_REMDC_SDMA;
        */

        case tOVL0_EX:
            return M4U_PORT_DISP_OVL0;

        case tWDMA_EX:
            return M4U_PORT_DISP_WDMA0;

        case CMDQ_ENG_ISP_VIPI:
            return M4U_PORT_CAM_VIPI;
        case CMDQ_ENG_ISP_LCEI:
            return M4U_PORT_CAM_LCEI;
        case CMDQ_ENG_ISP_IMG3O:
            return M4U_PORT_CAM_IMG3O;
        case CMDQ_ENG_ISP_SMXIO:
            return (M4U_PORT_CAM_SMXI | M4U_PORT_CAM_SMXO);

        case CMDQ_ENG_WPEI:
            return M4U_PORT_CAM_WPE0_RDMA1;
        case CMDQ_ENG_WPEO:
            return M4U_PORT_CAM_WPE0_WDMA;
        case CMDQ_ENG_WPEI2:
            return M4U_PORT_CAM_WPE1_RDMA1;
        case CMDQ_ENG_WPEO2:
            return M4U_PORT_CAM_WPE1_WDMA;

        default:
            DPLOGE("DpMmuHandler: unknown engine type: %d\n", type);
            assert(0);
            break;
    }

    return M4U_PORT_UNKNOWN;
}

#endif  // __M4U_PORT_MAP_H__
