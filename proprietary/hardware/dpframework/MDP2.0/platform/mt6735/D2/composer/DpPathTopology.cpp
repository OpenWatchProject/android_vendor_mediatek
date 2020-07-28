#include "DpPathTopology.h"

const DpEngineType DpPathTopology::s_topology[tTotal] =
{
    tRDMA0, tIMGI, tCAMIN, tSCL0, tSCL1, tTDSHP0, tWDMA, tWROT0, tVENC, tIMG2O, tIMGO, tTotal
};

const int32_t DpPathTopology::s_engOrder[tTotal] =
{
    1, 10, 9, 2, 0, 3, 4, 5, 7, 6, -1, 8
};

const bool DpPathTopology::s_adjency[tTotal][tTotal] =
{
        /*            tIMGI, tIMGO, tIMG2O, tCAMIN , tRDMA0, tSCL0, tSCL1, tTDSHP0, tWROT0, tWDMA, tJPEGENC, tVENC,  */
    {   /* tIMGI */       0,     1,      1,      1 ,      0,     0,     0,       0,      0,     0,        0,     0,  },
    {   /* tIMGO */       0,     0,      0,      0 ,      0,     0,     0,       0,      0,     0,        0,     0,  },
    {   /* tIMG2O */      0,     0,      0,      0 ,      0,     0,     0,       0,      0,     0,        0,     0,  },
    {   /* tCAMIN */      0,     0,      0,      0 ,      0,     1,     1,       0,      0,     0,        0,     0,  },
    {   /* tRDMA0 */      0,     0,      0,      0 ,      0,     1,     1,       0,      0,     0,        0,     0,  },
    {   /* tSCL0 */       0,     0,      0,      0 ,      0,     0,     1,       1,      1,     1,        0,     0,  },
    {   /* tSCL1 */       0,     0,      0,      0 ,      0,     0,     0,       1,      1,     1,        0,     0,  },
    {   /* tTDSHP0 */     0,     0,      0,      0 ,      0,     0,     0,       0,      1,     1,        0,     0,  },
    {   /* tWROT0 */      0,     0,      0,      0 ,      0,     0,     0,       0,      0,     0,        0,     1,  },
    {   /* tWDMA */       0,     0,      0,      0 ,      0,     0,     0,       0,      0,     0,        0,     1,  },
    {   /* tJPEGENC */    0,     0,      0,      0 ,      0,     0,     0,       0,      0,     0,        0,     0,  },
    {   /* tVENC */       0,     0,      0,      0 ,      0,     0,     0,       0,      0,     0,        0,     0,  },
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
    if( (target < tIMGI) ||
        (target >= tTotal) )
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


DP_STATUS_ENUM DpPathTopology::getPathInfo(STREAM_TYPE_ENUM scenario,
                                           DpPortAdapt      &sourcePort,
                                           DpPortAdapt      &targetPort,
                                           uint64_t         &engFlag,
                                           PathInfo         &pathInfo,
                                           DpEngineType     &sourceEng,
                                           DpEngineType     &targetEng,
                                           uint32_t         numOutputPort,
                                           uint32_t         channelInfo)
{
    int32_t        rotation;
    bool           flipStatus;
    PORT_TYPE_ENUM portType;
    PathInfo       engInfo;
    uint32_t       engCount;
    DP_STATUS_ENUM status;
    DpColorFormat  format;
    int32_t        dummy;

    rotation   = targetPort.getRotation();
    flipStatus = targetPort.getFlipStatus();
    status     = targetPort.getPortInfo(&format,
                                        &dummy,
                                        &dummy,
                                        &dummy,
                                        &dummy,
                                        0);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpPathTopology: query output port format failed\n");
        return status;
    }

    engCount  = 0;

    sourceEng = tNone;
    targetEng = tNone;

    #ifdef CONFIG_FOR_SOURCE_PQ
    if (STREAM_COLOR_BITBLT == scenario)
    {
        engCount = 3;

        sourceEng = tOVL0_EX;
        targetEng = tWDMA_EX;

        pathInfo[0] = tWDMA_EX;
        pathInfo[1] = tCOLOR_EX;
        pathInfo[2] = tOVL0_EX;
        engFlag = (1<<tOVL0_EX) | (1<<tCOLOR_EX) | (1<<tWDMA_EX);

        sourcePort.getPortType(&portType);
        targetPort.getPortType(&portType);

        DPLOGI("DpPathTopology: PQ_BITBLT Fixed path !!!!\n");
        return DP_STATUS_RETURN_SUCCESS;
    }
    #endif

    sourcePort.getPortType(&portType);
    switch (portType)
    {
        case PORT_MEMORY:
            if (//(scenario == STREAM_ISP_IC) ||
                (scenario == STREAM_ISP_VR) ||
                //(scenario == STREAM_ISP_IP) ||
                (scenario == STREAM_ISP_VSS))
            {
                sourceEng = tIMGI;
                DPLOGI("DpPathTopology: select source engine IMGI\n");
            }
            else
            {
                sourceEng = tRDMA0;
                DPLOGI("DpPathTopology: select source engine RDMA0\n");
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
            if ((0     != rotation) ||
                (false != flipStatus) ||
                (channelInfo == VENC_ENABLE_FLAG) ||
                (DP_COLOR_BGR565 == format) ||
                (DP_COLOR_I422 == format) ||
                (DP_COLOR_YV16 == format) ||
                (DP_COLOR_NV61 == format) ||
                (DP_COLOR_NV16 == format) ||
                (DP_COLOR_I444 == format) ||
                (DP_COLOR_YV24 == format))
            {
                // WDMA cannot support these
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
            else if ((tIMGI == sourceEng) ||
                (1 != numOutputPort))
            {
                // output port 0: temp port
                //             2: multi-port
                // prefer WDMA
                if (IS_ENGINE_FREE(engFlag, tWDMA))
                {
                    DPLOGI("DpPathTopology: select target engine WDMA\n");
                    targetEng = tWDMA;
                }
                else if (IS_ENGINE_FREE(engFlag, tWROT0))
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
            else
            {
                // output port 1: blit
                // prefer WROT
                if (IS_ENGINE_FREE(engFlag, tWROT0))
                {
                    DPLOGI("DpPathTopology: select target engine WROT0\n");
                    targetEng = tWROT0;
                }
                else if (IS_ENGINE_FREE(engFlag, tWDMA))
                {
                    DPLOGI("DpPathTopology: select target engine WDMA\n");
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
            if (IS_ENGINE_FREE(engFlag, tVENC))
            {
                DPLOGI("DpPathTopology: select target engine VENC\n");
                targetEng = tVENC;

                // Force add WDMA
                if (IS_ENGINE_FREE(engFlag, tWDMA))
                {
                    engInfo[engCount] = tWDMA;
                    engCount++;
                }
                else
                {
                    DPLOGI("DpPathTopology: can not merge path due to failure in adding WDMA before VENC\n");
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

    if ((tIMGI != sourceEng) || (tIMG2O != targetEng))      // Not ISP-only Pass2
    {
        if (tWDMA == targetEng || tVENC == targetEng)
        {
            // prefer SCL0
            if (IS_ENGINE_FREE(engFlag, tSCL0))
            {
                DPLOGI("DpPathTopology: select resizer engine tSCL0\n");
                engInfo[engCount] = tSCL0;
                engCount++;
            }
            else if (IS_ENGINE_FREE(engFlag, tSCL1))
            {
                DPLOGI("DpPathTopology: select resizer engine tSCL1\n");
                engInfo[engCount] = tSCL1;
                engCount++;
            }
            else
            {
                DPLOGI("DpPathTopology: can not merge path\n");
                return DP_STATUS_CAN_NOT_MERGE;
            }
        }
        else
        {
            // prefer SCL1
            if (IS_ENGINE_FREE(engFlag, tSCL1))
            {
                DPLOGI("DpPathTopology: select resizer engine tSCL1\n");
                engInfo[engCount] = tSCL1;
                engCount++;
            }
            else if (IS_ENGINE_FREE(engFlag, tSCL0))
            {
                DPLOGI("DpPathTopology: select resizer engine tSCL0\n");
                engInfo[engCount] = tSCL0;
                engCount++;
            }
            else
            {
                DPLOGI("DpPathTopology: can not merge path\n");
                return DP_STATUS_CAN_NOT_MERGE;
            }
        }
    }

    // Check TDSHP0 status
    if (targetPort.getTDSHPStatus())
    {
#ifndef BASIC_PACKAGE
        if (0 != DpDriver::getInstance()->getPQSupport()) {
            if (IS_ENGINE_FREE(engFlag, tTDSHP0))
            {
                DPLOGI("DpPathTopology: select sharpness engine tTDSHP0\n");
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
