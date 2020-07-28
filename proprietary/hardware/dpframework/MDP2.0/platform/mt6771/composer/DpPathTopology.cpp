#include "DpPathTopology.h"

const DpEngineType DpPathTopology::s_topology[tTotal] =
{
    tRDMA0, tWPEI, tWPEO,  tWPEI2, tWPEO2, tIPUI, tIPUO, tIMGI, tCAMIN, tCAMIN2, tAAL0, tCCORR0, tSCL0, tSCL1, tTDSHP0, tCOLOR0, tPATH0_SOUT, tPATH1_SOUT, tWROT0, tWDMA, tVENC, tIMG2O, tIMGO, tNone
};

const int32_t DpPathTopology::s_engOrder[tTotal] =
{
    [tRDMA0]    = 0,
    [tWPEI]     = 1,
    [tWPEO]     = 2,
    [tWPEI2]    = 3,
    [tWPEO2]    = 4,
    [tIPUI]     = 5,
    [tIPUO]     = 6,
    [tIMGI]     = 7,
    [tCAMIN]    = 8,
    [tCAMIN2]   = 9,
    [tAAL0]     = 10,
    [tCCORR0]   = 11,
    [tSCL0]     = 12,
    [tSCL1]     = 13,
    [tTDSHP0]   = 14,
    [tCOLOR0]   = 15,
    [tPATH0_SOUT] = 16,
    [tPATH1_SOUT] = 17,
    [tWROT0]    = 18,
    [tWDMA]     = 19,
    [tVENC]     = 20,
    [tIMG2O]    = 21,
    [tIMGO]     = 22,
};

const bool DpPathTopology::s_adjency[tTotal][tTotal] =
{
    [tWPEI]         = {0, [tWPEO] = 1, [tCAMIN2] = 1},
    [tWPEO]         = {0, [tIMGI] = 1 },
    [tWPEI2]        = {0, [tWPEO2] = 1},
    [tWPEO2]        = {0, [tIMGI] = 1},
    [tIMGI]         = {0, [tIMGO] = 1, [tIMG2O] = 1, [tCAMIN] = 1, [tCAMIN2] = 1},
    [tIMGO]         = {0},
    [tIMG2O]        = {0},
    [tIPUI]         = {0, [tIPUO] = 1, [tCAMIN2] = 1},
    [tIPUO]         = {0},
    [tCAMIN]        = {0, [tAAL0] = 1, [tCCORR0] = 1, [tSCL1] = 1},
    [tCAMIN2]       = {0, [tAAL0] = 1, [tCCORR0] = 1, [tSCL1] = 1},
    [tRDMA0]        = {0, [tAAL0] = 1, [tCCORR0] = 1, [tSCL1] = 1, [tPATH0_SOUT] = 1},
    [tAAL0]         = {0, [tCCORR0] = 1, [tSCL0] = 1, [tSCL1] = 1},
    [tCCORR0]       = {0, [tAAL0] = 1, [tSCL0] = 1},
    [tSCL0]         = {0, [tTDSHP0] = 1, [tPATH0_SOUT] = 1},
    [tSCL1]         = {0, [tTDSHP0] = 1, [tCOLOR0] = 1, [tPATH0_SOUT] = 1, [tPATH1_SOUT] = 1},
    [tTDSHP0]       = {0, [tCOLOR0] = 1},
    [tCOLOR0]       = {0, [tPATH0_SOUT] = 1, [tPATH1_SOUT] = 1},
    [tPATH0_SOUT]   = {0, [tWROT0] = 1},
    [tPATH1_SOUT]   = {0, [tWDMA] = 1},
    [tWROT0]        = {0, [tVENC] = 1},
    [tWDMA]         = {0, [tVENC] = 1},
    [tJPEGENC]      = {0},
    [tVENC]         = {0},
};



DpPathTopology::DpPathTopology()
{
}


DpPathTopology::~DpPathTopology()
{
}


bool DpPathTopology::sortPathInfo(DpEngineType source,
                                  DpEngineType target,
                                  PathInfo     &info,
                                  uint32_t     length)
{
    uint32_t     index;
    int32_t      inner;
    DpEngineType temp;
    int32_t      order;

    for (index = 1; index <= (length - 1); index++)
    {
        temp  = info[index];
        order = s_engOrder[temp];
        inner = index - 1;

        while ((inner >= 0) && (order < s_engOrder[info[inner]]))
        {
            info[inner + 1] = info[inner];
            inner = inner - 1;
        }

        info[inner + 1] = temp;
    }

    if ((source != info[0]) ||
        (target != info[length - 1]))
    {
        return false;
    }

    return true;
}


DP_STATUS_ENUM DpPathTopology::getEngUsages()
{
    DP_STATUS_ENUM status;
    char           *module_name;
    int32_t        i;

    status = DpDriver::getInstance()->queryEngUsages(m_engUsages);

    assert(DP_STATUS_RETURN_SUCCESS == status);

    for (i = tRDMA0; i < tTotal; i++)
    {
        DP_GET_ENGINE_NAME(i, module_name);
        DPLOGI("MDP modules( %s) usage: %d\n", module_name, m_engUsages[i]);
    }

    return DP_STATUS_RETURN_SUCCESS;
}

bool DpPathTopology::connectEdge(DpEngineType startPoint,
                                 DpEngineType endPoint,
                                 PathInfo     &dataPath)
{
    int32_t         current;
    DpEngineType    nodeA;
    DpEngineType    nodeB;
    int32_t         source;
    int32_t         target;
    uint32_t        weight;

    target = s_engOrder[endPoint];

    for (source = s_engOrder[startPoint]; source <= target; source++)
    {
        m_distance[topology(source)] = 0;
    }

    for (source = s_engOrder[startPoint]; source <= target; source++)
    {
        for (current = (source + 1); current <= target; ++current)
        {
            nodeA = topology(source),
            nodeB = topology(current);

            if (isConnect(nodeA, nodeB))
            {
                weight = LOAD_BALANCE_BY_ENG_USAGE ? getWeight(nodeA, nodeB) : 1;
                if ((0 == m_distance[nodeB])
                    || (((m_distance[nodeA] + weight) < m_distance[nodeB]) && (dataPath[nodeA] != tNone)))
                {
                    m_distance[nodeB] = m_distance[nodeA] + weight;
                    dataPath[nodeB] = nodeA;
                }
            }
        }
    }

    return true;
}

uint32_t DpPathTopology::getWeight(DpEngineType source,
                                   DpEngineType target)
{
    if ((target < tIMGI) ||
        (target >= tTotal))
    {
        DPLOGE("DpPathTopology: connect engine path failed\n");
        return DP_STATUS_INVALID_PARAX;
    }
    return m_engUsages[target] + 1;
}


bool DpPathTopology::connectPath(PathInfo &engInfo,
                                 int32_t  length,
                                 uint64_t &engFlag,
                                 PathInfo &dataPath)
{
    bool    status;
    int32_t index;

    dataPath[engInfo[0]] = engInfo[0];

    for (index = 0; index < (length - 1); index++)
    {
        // Record the engine is required
        engFlag |= (1 << engInfo[index]);

        status = connectEdge(engInfo[index + 0],
                             engInfo[index + 1],
                             dataPath);
        if (false == status)
        {
            return status;
        }
    }

    engFlag |= (1 << engInfo[length - 1]);

    return true;
}


static bool needResizer(uint32_t    numOutputPort,
                        int32_t     sourceCropWidth,
                        int32_t     sourceCropWidthSubpixel,
                        int32_t     sourceCropHeight,
                        int32_t     sourceCropHeightSubpixel,
                        int32_t     sourceCropSubpixX,
                        int32_t     sourceCropSubpixY,
                        int32_t     targetWidth,
                        int32_t     targetHeight)
{
    bool needResizer = true;

    DPLOGI("DpPathTopology: Width  %d %d %d\n", sourceCropWidth, sourceCropWidthSubpixel, targetWidth);
    DPLOGI("DpPathTopology: Height %d %d %d\n", sourceCropHeight, sourceCropHeightSubpixel, targetHeight);
    DPLOGI("DpPathTopology: CropSubpix %d %d\n", sourceCropSubpixX, sourceCropSubpixY);

    if ((numOutputPort == 1) &&
        (sourceCropWidth == targetWidth) &&
        (sourceCropHeight == targetHeight) &&
        (sourceCropWidthSubpixel == 0) &&
        (sourceCropHeightSubpixel == 0) &&
        (sourceCropSubpixX == 0) &&
        (sourceCropSubpixY == 0))
    {
        needResizer = false;
    }

    DPLOGI("DpPathTopology: needResizer %d\n", needResizer);

    return needResizer;
}

DP_STATUS_ENUM DpPathTopology::getPathInfo(STREAM_TYPE_ENUM scenario,
                                           DpPortAdapt      &sourcePort,
                                           DpPortAdapt      &targetPort,
                                           uint64_t         &engFlag,
                                           PathInfo         &pathInfo,
                                           DpEngineType     &sourceEng,
                                           DpEngineType     &targetEng,
                                           uint32_t         numOutputPort,
                                           uint32_t)
{
    int32_t        rotation;
    bool           flipStatus;
    PORT_TYPE_ENUM portType;
    PathInfo       engInfo;
    uint32_t       engCount;
    DP_STATUS_ENUM status;
    DpColorFormat  sourceFormat;
    int32_t        sourceCropXOffset;
    int32_t        sourceCropYOffset;
    int32_t        sourceCropWidth;
    int32_t        sourceCropWidthSubpixel;
    int32_t        sourceCropHeight;
    int32_t        sourceCropHeightSubpixel;
    int32_t        sourceCropSubpixX;
    int32_t        sourceCropSubpixY;
    bool           sourceDitherStatus;
    DpColorFormat  targetFormat;
    int32_t        targetWidth;
    int32_t        targetHeight;
    int32_t        dummy;
    DpEngineType   curRSZ = tNone;
    DpEngineType   curPATH_SOUT = tNone;

    rotation   = targetPort.getRotation();
    flipStatus = targetPort.getFlipStatus();
    status     = targetPort.getPortInfo(&targetFormat,
                                        &targetWidth,
                                        &targetHeight,
                                        &dummy,
                                        &dummy,
                                        0);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpPathTopology: query output port format failed\n");
        return status;
    }

    if (rotation == 90 || rotation == 270)
    {
        int32_t temp = targetWidth;
        targetWidth = targetHeight;
        targetHeight = temp;
    }

    status     = sourcePort.getPortInfo(&sourceFormat,
                                        &dummy,
                                        &dummy,
                                        &dummy,
                                        &dummy,
                                        0,
                                        &sourceDitherStatus);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpPathTopology: query input port format failed\n");
        return status;
    }

    status = sourcePort.getSourceCrop(&sourceCropXOffset,
                                      &sourceCropSubpixX,
                                      &sourceCropYOffset,
                                      &sourceCropSubpixY,
                                      &sourceCropWidth,
                                      &sourceCropWidthSubpixel,
                                      &sourceCropHeight,
                                      &sourceCropHeightSubpixel);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpPathTopology: query input port crop info failed\n");
        return status;
    }

    /**********************************************************
     * Topology rules
     **********************************************************
     * By Hardware
     *
     * Module select order:
     *  ISP   - PRZ1    RDMA0 - PRZ0
     *          PRZ0
     *  PRZ0  - WROT0   PRZ1  - WDMA
     *         (WDMA)          (WROT0)
     *
     * So that preferred paths:
     *  ISP   == PRZ1 == WDMA
     *        \       \/
     *         \      /\
     *  RDMA0 == PRZ0 == WROT0
     *
     * Constraints:
     * - WDMA: no rotate/flip/venc nor some target formats
     * - WROT0: share rotation sram with DISP_RSZ
     *          (no rotate/flip unless for 2nd_BLT)
     *
     **********************************************************
     * By Scenario and Usage
     *
     * A. [input]
     * ISP_IC/VR/VSS: IMGI
     * else: RDMA0
     *
     * B. [resizer and output]
     * To memory:
     * 1. [resizer]
     *     rotate/flip: PRZ0
     *     IMGI: PRZ1 > PRZ0
     *     else: PRZ0
     * 2. [output]
     *     PRZ0: WROT0
     *     PRZ1: WDMA
     * To ISP: IMG2O
     * To VPU: VPUO
     * To VENC: (IMGI) PRZ1 and WDMA
     **********************************************************/

    engCount  = 0;

    sourceEng = tNone;
    targetEng = tNone;

#ifdef CONFIG_FOR_SOURCE_PQ
    if (scenario == STREAM_COLOR_BITBLT)
    {
        engCount = 3;

        sourceEng = tOVL0_EX;
        targetEng = tWDMA_EX;

        pathInfo[0] = tWDMA_EX;
        pathInfo[1] = tCOLOR_EX;
        pathInfo[2] = tOVL0_EX;
        engFlag = (1 << tOVL0_EX) | (1 << tCOLOR_EX) | (1 << tWDMA_EX);

        sourcePort.getPortType(&portType);
        targetPort.getPortType(&portType);

        DPLOGI("DpPathTopology: PQ_BITBLT Fixed path !!!!\n");
        return DP_STATUS_RETURN_SUCCESS;
    }
#endif

    //ARGB8888 in/out and bypass resizer and disable sharpness
    if ( (DP_COLOR_GET_HW_FORMAT(sourceFormat) == 2 || DP_COLOR_GET_HW_FORMAT(sourceFormat) == 3) &&
        (DP_COLOR_GET_HW_FORMAT(targetFormat) == 2 || DP_COLOR_GET_HW_FORMAT(targetFormat) == 3) &&
        needResizer(numOutputPort, sourceCropWidth, sourceCropWidthSubpixel, sourceCropHeight, sourceCropHeightSubpixel, sourceCropSubpixX, sourceCropSubpixY, targetWidth, targetHeight) == false &&
        (sourceCropXOffset == 0 && sourceCropYOffset == 0) &&
        targetPort.getTDSHPStatus() == 0)
    {
            sourceEng = tRDMA0;
            curPATH_SOUT = tPATH0_SOUT;
            targetEng = tWROT0;

        engInfo[engCount] = sourceEng;
        engCount++;

        engInfo[engCount] = curPATH_SOUT;
        engCount++;

        engInfo[engCount] = targetEng;
        engCount++;

        if (false == sortPathInfo(sourceEng,
                                  targetEng,
                                  engInfo,
                                  engCount))
        {
            DPLOGE("DpPathTopology: invalid engine path info\n");
            assert(0);
            return DP_STATUS_INVALID_PATH;
        }

        if (false == connectPath(engInfo,
                                 engCount,
                                 engFlag,
                                 pathInfo))
        {
            DPLOGE("DpPathTopology: connect engine path failed\n");
            assert(0);
            return DP_STATUS_INVALID_PATH;
        }

        DPLOGI("DpPathTopology: ARGB8888 Fixed path !!!!\n");
        return DP_STATUS_RETURN_SUCCESS;
    }

    sourcePort.getPortType(&portType);
    switch (portType)
    {
        case PORT_MEMORY:
            if ((scenario == STREAM_ISP_IC) ||
                (scenario == STREAM_ISP_VR) ||
                //(scenario == STREAM_ISP_IP) ||
                (scenario == STREAM_ISP_VSS))
            {
                sourceEng = tIMGI;
                DPLOGI("DpPathTopology: select source engine IMGI\n");
            }
            else if (scenario == STREAM_WPE)
            {
                sourceEng = tWPEI;
                DPLOGI("DpPathTopology: select source engine tWPEI\n");
            }
            else if (scenario == STREAM_WPE2)
            {
                sourceEng = tWPEI2;
                DPLOGI("DpPathTopology: select source engine tWPEI2\n");
            }
            else
            {
                    sourceEng = tRDMA0;

                char *module_name;
                DP_GET_ENGINE_NAME(sourceEng, module_name);
                DPLOGI("DpPathTopology: select source engine %s\n", module_name);
            }
            break;
        default:
            DPLOGE("DpPathTopology: invalid source port\n");
            assert(0);
            return DP_STATUS_INVALID_PORT;
    }

    //if rotate/flip: PRZ0
    //else if IMGI: PRZ1 > PRZ0
    //else: PRZ0
    targetPort.getPortType(&portType);

    if (tIMGI == sourceEng)
    {
        if (PORT_MDP == portType)
        {

            DPLOGI("DpPathTopology: select direct-link engine CAMIN\n");
            engInfo[engCount] = tCAMIN;
            engCount++;
        }
        else if (PORT_MDP2 == portType)
        {
            DPLOGI("DpPathTopology: select direct-link engine CAMIN2\n");
            engInfo[engCount] = tCAMIN2;
            engCount++;
        }
    }

    switch (portType)
    {
        case PORT_MDP:
        case PORT_MDP2:
            if ((0 != rotation) || (flipStatus == true))
            {
                if (IS_ENGINE_FREE(engFlag, tCCORR0) && IS_ENGINE_FREE(engFlag, tSCL0))
                {
                    DPLOGI("DpPathTopology: select CCORR0 and resizer engine SCL0\n");
                    engInfo[engCount] = tCCORR0;
                    engCount++;
                    engInfo[engCount] = tSCL0;
                    engCount++;
                    curRSZ = tSCL0;
                }
                else
                {
                    DPLOGI("DpPathTopology: can not merge path\n");
                    return DP_STATUS_CAN_NOT_MERGE;
                }
            }
            else if (tIMGI == sourceEng || tWPEI == sourceEng)
            {
                //prefer SCL1
                if (IS_ENGINE_FREE(engFlag, tSCL1))
                {
                    DPLOGI("DpPathTopology: select resizer engine SCL1\n");
                    engInfo[engCount] = tSCL1;
                    engCount++;
                    curRSZ = tSCL1;
                }
                else if (IS_ENGINE_FREE(engFlag, tCCORR0) && IS_ENGINE_FREE(engFlag, tSCL0))
                {
                    DPLOGI("DpPathTopology: select CCORR0 and resizer engine SCL0\n");
                    engInfo[engCount] = tCCORR0;
                    engCount++;
                    engInfo[engCount] = tSCL0;
                    engCount++;
                    curRSZ = tSCL0;
                }
                else
                {
                    DPLOGI("DpPathTopology: can not merge path\n");
                    return DP_STATUS_CAN_NOT_MERGE;
                }
            }
            else if (tWPEI2 == sourceEng)
            {
                //prefer SCL0
                if (IS_ENGINE_FREE(engFlag, tCCORR0) && IS_ENGINE_FREE(engFlag, tSCL0))
                {
                    DPLOGI("DpPathTopology: select CCORR0 and resizer engine SCL0\n");
                    engInfo[engCount] = tCCORR0;
                    engCount++;
                    engInfo[engCount] = tSCL0;
                    engCount++;
                    curRSZ = tSCL0;
                }
                else if (IS_ENGINE_FREE(engFlag, tSCL1))
                {
                    DPLOGI("DpPathTopology: select resizer engine SCL1\n");
                    engInfo[engCount] = tSCL1;
                    engCount++;
                    curRSZ = tSCL1;
                }
                else
                {
                    DPLOGI("DpPathTopology: can not merge path\n");
                    return DP_STATUS_CAN_NOT_MERGE;
                }
            }
            else // tRDMA0
            {
                // prefer SCL0
                if (IS_ENGINE_FREE(engFlag, tCCORR0) && IS_ENGINE_FREE(engFlag, tSCL0))
                {
                    DPLOGI("DpPathTopology: select CCORR0 and resizer engine SCL0\n");
                    engInfo[engCount] = tCCORR0;
                    engCount++;
                    engInfo[engCount] = tSCL0;
                    engCount++;
                    curRSZ = tSCL0;
                }
                else
                {
                    DPLOGI("DpPathTopology: can not merge path\n");
                    return DP_STATUS_CAN_NOT_MERGE;
                }
            }

            if (tSCL0 == curRSZ)
            {
                if (IS_ENGINE_FREE(engFlag, tPATH0_SOUT) && IS_ENGINE_FREE(engFlag, tWROT0))
                {
                    DPLOGI("DpPathTopology: select target engine WROT0\n");
                    curPATH_SOUT = tPATH0_SOUT;
                    engInfo[engCount] = tPATH0_SOUT;
                    engCount++;
                    targetEng = tWROT0;
                }
                else
                {
                    DPLOGI("DpPathTopology: can not merge path\n");
                    return DP_STATUS_CAN_NOT_MERGE;
                }
            }
            else if (tSCL1 == curRSZ)
            {
                // output port 1: blit
                // prefer WDMA
                if (IS_ENGINE_FREE(engFlag, tPATH1_SOUT) && IS_ENGINE_FREE(engFlag, tWDMA))
                {
                    DPLOGI("DpPathTopology: select target engine WDMA\n");
                    curPATH_SOUT = tPATH1_SOUT;
                    engInfo[engCount] = tPATH1_SOUT;
                    engCount++;
                    targetEng = tWDMA;
                }
                else
                {
                    DPLOGI("DpPathTopology: can not merge path\n");
                    return DP_STATUS_CAN_NOT_MERGE;
                }
            }
            break;
        case PORT_ISP:
            if (IS_ENGINE_FREE(engFlag, tIMG2O))
            {
                DPLOGI("DpPathTopology: select target engine IMG2O\n");
                targetEng = tIMG2O;
            }
            else
            {
                DPLOGI("DpPathTopology: can not merge path\n");
                return DP_STATUS_CAN_NOT_MERGE;
            }
            break;
        case PORT_WPE:
            if (IS_ENGINE_FREE(engFlag, tWPEO)  && (tWPEI == sourceEng))
            {
                DPLOGI("DpPathTopology: select target engine tWPEO\n");
                targetEng = tWPEO;
            }
            else if (IS_ENGINE_FREE(engFlag, tWPEO2)  && (tWPEI2 == sourceEng))
            {
                DPLOGI("DpPathTopology: select target engine tWPEO2\n");
                targetEng = tWPEO2;
            }
            else
            {
                DPLOGI("DpPathTopology: can not merge path\n");
                return DP_STATUS_CAN_NOT_MERGE;
            }
            break;
#if 0
        case PORT_JPEG:
            if (IS_ENGINE_FREE(engFlag, tJPEGENC))
            {
                DPLOGI("DpPathTopology: select target engine JPEGENC\n");
                targetEng = tJPEGENC;
            }
            break;
#endif
        case PORT_VENC:
            // select SCL1
            if (IS_ENGINE_FREE(engFlag, tSCL1))
            {
                DPLOGI("DpPathTopology: select resizer engine SCL1\n");
                engInfo[engCount] = tSCL1;
                engCount++;
            }
            else
            {
                DPLOGI("DpPathTopology: can not merge path\n");
                return DP_STATUS_CAN_NOT_MERGE;
            }

            if (IS_ENGINE_FREE(engFlag, tVENC))
            {
                DPLOGI("DpPathTopology: select target engine VENC\n");
                targetEng = tVENC;

                // Force add WDMA
                if (IS_ENGINE_FREE(engFlag, tPATH1_SOUT) && IS_ENGINE_FREE(engFlag, tWDMA))
                {
                    engInfo[engCount] = tPATH1_SOUT;
                    engCount++;
                    engInfo[engCount] = tWDMA;
                    engCount++;
                }
                else
                {
                    DPLOGI("DpPathTopology: can not merge path due to failure in adding tWROT0 before VENC\n");
                    return DP_STATUS_CAN_NOT_MERGE;
                }
            }
            break;
        default:
            DPLOGE("DpPathTopology: invalid target port\n");
            assert(0);
            return DP_STATUS_INVALID_PORT;
    }

    if ((tNone == sourceEng) ||
        (tNone == targetEng))
    {
        DPLOGE("DpPathTopology: unknown source or target engine\n");
        return DP_STATUS_INVALID_ENGINE;
    }

    // Check TDSHP0 status
    if (targetPort.getTDSHPStatus())
    {
#ifndef BASIC_PACKAGE
        if (0 != DpDriver::getInstance()->getPQSupport())
        {
            if (IS_ENGINE_FREE(engFlag, tTDSHP0))
            {
                DPLOGI("DpPathTopology: select sharpness engine TDSHP0\n");
                engInfo[engCount] = tTDSHP0;
                engCount++;
            }
            else
            {
                DPLOGI("DpPathTopology: can not merge path\n");
                return DP_STATUS_CAN_NOT_MERGE;
            }
        }
#endif // BASIC_PACKAGE
    }

    // Check AAL0 status
    if (sourcePort.getAALStatus())
    {
#ifndef BASIC_PACKAGE
        if (0 != DpDriver::getInstance()->getPQSupport())
        {
            DPLOGI("DpPathTopology: select DRE engine AAL0\n");
            engInfo[engCount] = tAAL0;
            engCount++;
        }
#endif // BASIC_PACKAGE
    }

    engInfo[engCount] = sourceEng;
    engCount++;

    engInfo[engCount] = targetEng;
    engCount++;

    if (false == sortPathInfo(sourceEng,
                              targetEng,
                              engInfo,
                              engCount))
    {
        DPLOGE("DpPathTopology: invalid engine path info\n");
        assert(0);
        return DP_STATUS_INVALID_PATH;
    }

    if (false == connectPath(engInfo,
                             engCount,
                             engFlag,
                             pathInfo))
    {
        DPLOGE("DpPathTopology: connect engine path failed\n");
        assert(0);
        return DP_STATUS_INVALID_PATH;
    }

    return DP_STATUS_RETURN_SUCCESS;
}
