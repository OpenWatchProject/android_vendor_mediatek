#include "DpPathTopology.h"

const DpEngineType DpPathTopology::s_topology[tTotal] =
{
    tRDMA0, tRDMA1, tIMGI, tCAMIN, tSCL0, tSCL1, tSCL2, tTDSHP0, tCOLOR0, tWDMA, tWROT0, tWROT1, tVENC, tIMG2O, tIMGO, tNone
};

const int32_t DpPathTopology::s_engOrder[tTotal] =
{
    2, 14, 13, 3, 0, 1, 4, 5, 6, 7, 8, 10, 11, 9, -1, 12
};

const bool DpPathTopology::s_adjency[tTotal][tTotal] =
{
        /*            tIMGI, tIMGO, tIMG2O, tCAMIN , tRDMA0, tRDMA1, tSCL0, tSCL1, tSCL2, tTDSHP0, tCOLOR0, tWROT0, tWROT1, tWDMA, tJPEGENC, tVENC,  */
    {   /* tIMGI */       0,     1,      1,      1 ,      0,      0,     0,     0,     0,       0,       0,      0,      0,     0,        0,     0,  },
    {   /* tIMGO */       0,     0,      0,      0 ,      0,      0,     0,     0,     0,       0,       0,      0,      0,     0,        0,     0,  },
    {   /* tIMG2O */      0,     0,      0,      0 ,      0,      0,     0,     0,     0,       0,       0,      0,      0,     0,        0,     0,  },
    {   /* tCAMIN */      0,     0,      0,      0 ,      0,      0,     1,     1,     1,       0,       0,      0,      0,     0,        0,     0,  },
    {   /* tRDMA0 */      0,     0,      0,      0 ,      0,      0,     1,     1,     1,       0,       0,      1,      0,     0,        0,     0,  },
    {   /* tRDMA1 */      0,     0,      0,      0 ,      0,      0,     0,     0,     1,       0,       0,      0,      0,     0,        0,     0,  },
    {   /* tSCL0 */       0,     0,      0,      0 ,      0,      0,     0,     1,     0,       1,       0,      1,      0,     1,        0,     0,  },
    {   /* tSCL1 */       0,     0,      0,      0 ,      0,      0,     0,     0,     0,       1,       0,      1,      1,     1,        0,     0,  },
    {   /* tSCL2 */       0,     0,      0,      0 ,      0,      0,     0,     0,     0,       1,       0,      0,      1,     1,        0,     0,  },
    {   /* tTDSHP0 */     0,     0,      0,      0 ,      0,      0,     0,     0,     0,       0,       1,      0,      0,     0,        0,     0,  },
    {   /* tCOLOR0 */     0,     0,      0,      0 ,      0,      0,     0,     0,     0,       0,       0,      1,      1,     1,        0,     0,  },
    {   /* tWROT0 */      0,     0,      0,      0 ,      0,      0,     0,     0,     0,       0,       0,      0,      0,     0,        0,     1,  },
    {   /* tWROT1 */      0,     0,      0,      0 ,      0,      0,     0,     0,     0,       0,       0,      0,      0,     0,        0,     1,  },
    {   /* tWDMA */       0,     0,      0,      0 ,      0,      0,     0,     0,     0,       0,       0,      0,      0,     0,        0,     1,  },
    {   /* tJPEGENC */    0,     0,      0,      0 ,      0,      0,     0,     0,     0,       0,       0,      0,      0,     0,        0,     0,  },
    {   /* tVENC */       0,     0,      0,      0 ,      0,      0,     0,     0,     0,       0,       0,      0,      0,     0,        0,     0,  },
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
                if ((0 == m_distance[nodeB]) || ((m_distance[nodeA] + weight) < m_distance[nodeB]))
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

static bool needResizer(uint32_t     numOutputPort,
                        int32_t      sourceCropWidth,
                        int32_t      sourceCropWidthSubpixel,
                        int32_t      sourceCropHeight,
                        int32_t      sourceCropHeightSubpixel,
                        int32_t      sourceCropSubpixX,
                        int32_t      sourceCropSubpixY,
                        int32_t      targetWidth,
                        int32_t      targetHeight)
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
    DpColorFormat  targetFormat;
    int32_t        targetWidth;
    int32_t        targetHeight;
    int32_t        dummy;
    DpEngineType   curRSZ = tNone;

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
                                        0);
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
     *  ISP/2-out/2-pass/FRAG - PRZ2
     *                         (PRZ1)
     *  RDMA0 - PRZ1    RDMA1 - PRZ2
     *          PRZ2
     *  PRZ1  - WROT0   PRZ2  - WROT1
     *
     * So that preferred paths:
     *
     *           PRZ0    WDMA
     *
     *
     *  ISP   == PRZ1 == WROT0
     *        \/
     *        /\
     *  RDMA0 == PRZ2 == WROT1
     *         /
     *        /
     *  RDMA1
     *
     * Constraints:
     * - RDMA1: no block mode nor UFO source format
     *
     **********************************************************
     * By Scenario and Usage
     *
     * ISP_IC/VR/VSS: IMGI
     * else:
     * 1. [scenario preference]
     *     ZSD/GPU/FRAG: RDMA0
     *     2nd_BLT: RDMA1
     *     Usage RDMA0>RDMA1: RDMA1
     *     else: RDMA0
     * 2. [constraints and balance]
     *     Block/UFO: RDMA0
     *     2-out/2-pass: RDMA0
     *     BLT/GPU: by addRefCnt
     * 3. [fixed scenario]
     *     none
     *
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
        targetEng = tWROT0;

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
            else
            {
                // 1. scenario preference
                if ((scenario == STREAM_GPU_BITBLT) ||
                    (scenario == STREAM_FRAG) ||
                    (scenario == STREAM_FRAG_JPEGDEC) ||
                    (scenario == STREAM_ISP_ZSD) ||
                    (scenario == STREAM_ISP_ZSD_SLOW))
                {
                    sourceEng = tRDMA0;
                }
                else if ((scenario == STREAM_2ND_BITBLT))
                {
                    sourceEng = tRDMA1;
                }
#if LOAD_BALANCE_BY_ENG_USAGE
                else if (m_engUsages[tRDMA0] > m_engUsages[tRDMA1])
                {
                    sourceEng = tRDMA1;
                }
#endif
                else
                {
                    sourceEng = tRDMA0;
                }

                // 2. constraints and balance
                if (//DP_COLOR_GET_UFP_ENABLE(sourceFormat) ||
                    DP_COLOR_GET_BLOCK_MODE(sourceFormat))
                {
                    sourceEng = tRDMA0;
                }
                else if (1 != numOutputPort)
                {
                    // output port 0: temp port
                    //             2: multi-port
                    sourceEng = tRDMA0;
                }
                else if ((STREAM_BITBLT == scenario) || (STREAM_GPU_BITBLT == scenario))
                {
                    // for the same process /w multiple blit stream
                    // change source engine for better hw usage
                    DpDriver::getInstance()->addRefCnt(sourceEng);
                }

                // 3. fixed scenario

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

    targetPort.getPortType(&portType);
    switch (portType)
    {
        case PORT_MEMORY:
            if (tRDMA1 == sourceEng)
            {
                // fixed to SCL2
                if (IS_ENGINE_FREE(engFlag, tSCL2))
                {
                    DPLOGI("DpPathTopology: select resizer engine SCL2\n");
                    engInfo[engCount] = tSCL2;
                    engCount++;
                    curRSZ = tSCL2;
                }
                else
                {
                    DPLOGI("DpPathTopology: can not merge path\n");
                    return DP_STATUS_CAN_NOT_MERGE;
                }
            }
            else if ((tIMGI == sourceEng) ||
                (1 != numOutputPort) ||
                (scenario == STREAM_FRAG) ||
                (scenario == STREAM_FRAG_JPEGDEC))
            {
                // output port 0: temp port
                //             2: multi-port
                // prefer SCL2
                if (IS_ENGINE_FREE(engFlag, tSCL2))
                {
                    DPLOGI("DpPathTopology: select resizer engine SCL2\n");
                    engInfo[engCount] = tSCL2;
                    engCount++;
                    curRSZ = tSCL2;
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
                // prefer SCL1
                if (IS_ENGINE_FREE(engFlag, tSCL1))
                {
                    DPLOGI("DpPathTopology: select resizer engine SCL1\n");
                    engInfo[engCount] = tSCL1;
                    engCount++;
                    curRSZ = tSCL1;
                }
                else if (IS_ENGINE_FREE(engFlag, tSCL2))
                {
                    DPLOGI("DpPathTopology: select resizer engine SCL2\n");
                    engInfo[engCount] = tSCL2;
                    engCount++;
                    curRSZ = tSCL2;
                }
                else
                {
                    DPLOGI("DpPathTopology: can not merge path\n");
                    return DP_STATUS_CAN_NOT_MERGE;
                }
            }

            if (tSCL2 == curRSZ)
            {
                if (IS_ENGINE_FREE(engFlag, tWROT1))
                {
                    DPLOGI("DpPathTopology: select target engine WROT1\n");
                    targetEng = tWROT1;
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
                // prefer WROT0
                if (IS_ENGINE_FREE(engFlag, tWROT0))
                {
                    DPLOGI("DpPathTopology: select target engine WROT0\n");
                    targetEng = tWROT0;
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
            //Change to prefer SCL2 and WROT1, as input port must be IMGI (ISP)
            if (IS_ENGINE_FREE(engFlag, tSCL2))
            {
                DPLOGI("DpPathTopology: select resizer engine SCL2\n");
                engInfo[engCount] = tSCL2;
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

                // Force add WROT1
                if (IS_ENGINE_FREE(engFlag, tWROT1))
                {
                    engInfo[engCount] = tWROT1;
                    engCount++;
                }
                else
                {
                    DPLOGI("DpPathTopology: can not merge path due to failure in adding WROT1 before VENC\n");
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
        if (0 != DpDriver::getInstance()->getPQSupport()) {
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
