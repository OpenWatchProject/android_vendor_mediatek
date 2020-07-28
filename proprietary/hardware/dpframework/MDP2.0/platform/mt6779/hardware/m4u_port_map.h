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
            return M4U_PORT_IMGI_D1;

        case tIMG2O:
            return M4U_PORT_CRZO_D1;

        case tRDMA0:
            return M4U_PORT_MDP_RDMA0;

        case tRDMA1:
            return M4U_PORT_MDP_RDMA1;

        case tWROT0:
            return M4U_PORT_MDP_WROT0_R;

        case tWROT1:
            return M4U_PORT_MDP_WROT1_R;

        case tWPEI:
            return M4U_PORT_WPE_RDMA0;

        case tWPEO:
            return M4U_PORT_WPE_WDMA;

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
            return M4U_PORT_IMGI_D1;

        case tIMG2O:
            return M4U_PORT_CRZO_D1;

        case tRDMA0:
            return M4U_PORT_MDP_RDMA0;

        case tRDMA1:
            return M4U_PORT_MDP_RDMA1;

        case tWROT0:
            return M4U_PORT_MDP_WROT0_R;

        case tWROT1:
            return M4U_PORT_MDP_WROT1_R;

        case tWPEI:
            return M4U_PORT_WPE_RDMA0;

        case tWPEO:
            return M4U_PORT_WPE_WDMA;
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
            return M4U_PORT_VIPI_D1;
        case CMDQ_ENG_ISP_LCEI:
            return M4U_PORT_LCEI_D1;
        case CMDQ_ENG_ISP_IMG3O:
            return M4U_PORT_IMG3O_D1;
        case CMDQ_ENG_ISP_SMXIO:
            return (M4U_PORT_SMTI_D1 | M4U_PORT_SMTO_D2 | M4U_PORT_SMTO_D1);
	case CMDQ_ENG_ISP_DMGI_DEPI:
            return (M4U_PORT_DMGI_D1 | M4U_PORT_DEPI_D1);
	case CMDQ_ENG_ISP_IMGCI:
            return M4U_PORT_IMGBI_D1;
	case CMDQ_ENG_ISP_TIMGO:
            return M4U_PORT_TIMGO_D1;

        default:
            DPLOGE("DpMmuHandler: unknown engine type: %d\n", type);
            assert(0);
            break;
    }

    return M4U_PORT_UNKNOWN;
}

#endif  // __M4U_PORT_MAP_H__
