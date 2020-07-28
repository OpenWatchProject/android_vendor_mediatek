#include "DpPathComposer.h"
#include "DpChannel.h"
#include "DpDataPath.h"
#include "DpTilePath.h"
#include "DpBufferPool.h"
#include "DpEngineType.h"


DpPathComposer::DpPathComposer(DpStream *stream)
    : m_pStream(stream),
      m_tempCount(0)
{
    memset(m_pTempPort, 0, sizeof(m_pTempPort));
    memset(m_pTempPool, 0, sizeof(m_pTempPool));
}


DpPathComposer::~DpPathComposer()
{
    reset();
}

DP_STATUS_ENUM DpPathComposer::reset()
{
    PathList::iterator      iterator;
    uint32_t                index;

    DPLOGI("DpPathComposer: delete composer object begin\n");

    DPLOGI("DpPathComposer: start deleting path object\n");

    if (!m_pathList.empty())
    {
        for (iterator = m_pathList.begin(); iterator != m_pathList.end(); iterator++)
        {
            DPLOGI("DpPathComposer: deleting path object %p\n", &(*iterator));
            delete &(*iterator);
        }
        m_pathList.clear();
    }

    DPLOGI("DpPathComposer: start deleting temp port\n");

    for (index = 0; index < m_tempCount; index++)
    {
        DPLOGI("DpPathComposer: deleting TempPort %p, TempPool\n", m_pTempPort[index], m_pTempPool[index]);
        delete m_pTempPort[index];
        m_pTempPort[index] = NULL;

        delete m_pTempPool[index];
        m_pTempPool[index] = NULL;
    }
    m_tempCount = 0;

    DPLOGI("DpPathComposer: delete composer object end\n");
    return DP_STATUS_RETURN_SUCCESS;
}


template <class PathType, class NodeType>
DP_STATUS_ENUM DpPathComposer::createPath(DpPortAdapt  *pInPort,
                                          DpPortAdapt  *pOutPort,
                                          PathInfo     &pathInfo,
                                          DpEngineType source,
                                          DpEngineType target,
                                          PathType     *pDataPath)
{
    DP_STATUS_ENUM  status;
    NodeType        *pPrev;
    NodeType        *pCurr;
    DpEngineType    current;
    int32_t         index;
#ifdef CONFIG_FOR_SOURCE_PQ
    STREAM_TYPE_ENUM   scenario = m_pStream->getScenario();
#endif

    DPLOGI("DpPathComposer: create path begin\n");

    DPLOGI("DpPathComposer: create target with type %d\n", target);

    current = target;
    pCurr   = (NodeType*)DpEngineBase::Factory(target);
    if (NULL == pCurr)
    {
        DPLOGE("DpPathComposer: can not create target %d\n", target);
        return DP_STATUS_INVALID_ENGINE;
    }

    // Bind source port
    status = pCurr->bindInPort(pInPort);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpPathComposer: bind input port error %d\n", status);
        return status;
    }

    // Bind target port
    status = pCurr->bindOutPort(pOutPort);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpPathComposer: bind output port error %d\n", status);
        return status;
    }

    for (index = 0; index < tTotal; index++)
    {
        #ifdef CONFIG_FOR_SOURCE_PQ
        if (STREAM_COLOR_BITBLT == scenario)
        {
            DPLOGI("DpPathComposer: create source with type %d\n", pathInfo[index+1]);
            pPrev = (NodeType*)DpEngineBase::Factory(pathInfo[index+1]);
        }
        else
        #endif
        {
            DPLOGI("DpPathComposer: create source with type %d\n", pathInfo[current]);
            pPrev = (NodeType*)DpEngineBase::Factory(pathInfo[current]);
        }

        if (NULL == pPrev)
        {
            DPLOGE("DpPathComposer: can not create target %d\n", pathInfo[current]);
            return DP_STATUS_INVALID_ENGINE;
        }

        status = pPrev->bindInPort(pInPort);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpPathComposer: bind input port error %d\n", status);
            return status;
        }

        status = pPrev->bindOutPort(pOutPort);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpPathComposer: bind output port error %d\n", status);
            return status;
        }

        status = pDataPath->insert(pPrev, pCurr);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpPathComposer: insert engine to path error %d\n", status);
            return status;
        }

        if (source == pathInfo[current])
        {
            pDataPath->insert(NULL, pPrev);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpPathComposer: insert root engine error %d\n", status);
                return status;
            }

            DPLOGI("DpPathComposer: find the source engine %d\n", source);
            break;
        }

        #ifdef CONFIG_FOR_SOURCE_PQ
        if (STREAM_COLOR_BITBLT == scenario)
        {
            if (source == pathInfo[index+1])
            {
                pDataPath->insert(NULL, pPrev);

                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    DPLOGE("DpPathComposer: insert root engine error %d\n", status);
                    return status;
                }

                DPLOGI("DpPathComposer: find the source engine %d\n", source);
                break;
            }

            current = pathInfo[index+1];
        }
        else
        #endif
        {
            current = pathInfo[current];
        }

        pCurr   = pPrev;
    }

    DPLOGI("DpPathComposer: create path end\n");

    return DP_STATUS_RETURN_SUCCESS;
}


template <class PathType, class NodeType>
DP_STATUS_ENUM DpPathComposer::mergePath(DpPortAdapt  *pInPort,
                                         DpPortAdapt  *pOutPort,
                                         PathInfo     &pathInfo,
                                         DpEngineType,
                                         DpEngineType target,
                                         PathType     *pDataPath)
{
    DP_STATUS_ENUM  status;
    NodeType        *pPrev;
    NodeType        *pCurr;
    DpEngineType    current;
    DpEngineType    previous;
    bool            stop;

    DPLOGI("DpPathComposer: merge path begin\n");

    DPLOGI("DpPathComposer: create target engine %d\n", target);

    current = target;
    pCurr   = (NodeType*)DpEngineBase::Factory(target);
    if (NULL == pCurr)
    {
        DPLOGE("DpPathComposer: can not create target %d\n", target);
        return DP_STATUS_INVALID_ENGINE;
    }

    // Bind source port
    status = pCurr->bindInPort(pInPort);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpPathComposer: bind input port error %d\n", status);
        return status;
    }

    // Bind target port
    status = pCurr->bindOutPort(pOutPort);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpPathComposer: bind output port error %d\n", status);
        return status;
    }

    stop = false;
    do
    {
        previous = (pathInfo[current]);

        pPrev = pDataPath->find(previous);
        if (NULL != pPrev)
        {
            // We have found the merge point
            stop = true;

            status = pPrev->bindOutPort(pOutPort);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpPathComposer: bind output port error %d\n", status);
                return status;
            }
        }
        else
        {
            DPLOGI("DpPathComposer: create engine with type %d\n", previous);

            pPrev = (NodeType*)DpEngineBase::Factory(previous);
            if (NULL == pPrev)
            {
                DPLOGE("DpPathComposer: can not create engine %d\n", previous);
                return DP_STATUS_INVALID_ENGINE;
            }

            status = pPrev->bindInPort(pInPort);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpPathComposer: bind input port error %d\n", status);
                return status;
            }

            status = pPrev->bindOutPort(pOutPort);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpPathComposer: bind output port error %d\n", status);
                return status;
            }
        }

        status = pDataPath->insert(pPrev, pCurr);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpPathComposer: insert engine to path error %d\n", status);
            return status;
        }

        pCurr   = pPrev;
        current = previous;
    } while (false == stop);

    DPLOGI("DpPathComposer: merge path end\n");

    return DP_STATUS_RETURN_SUCCESS;
}


template <class PathType, class NodeType>
DP_STATUS_ENUM DpPathComposer::createPathAndNew(DpPortAdapt  *pInPort,
                                                DpPortAdapt  *pOutPort,
                                                PathInfo     &pathInfo,
                                                DpEngineType source,
                                                DpEngineType target,
                                                PathType     *&pDataPath)
{
    DP_STATUS_ENUM status;

    status = createPath<PathType, NodeType>(pInPort,
                                            pOutPort,
                                            pathInfo,
                                            source,
                                            target,
                                            pDataPath);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpComposer: create path error %d\n", status);
        return status;
    }

    pInPort->bindClient(CLIENT_CONSUMER);

    pOutPort->bindClient(CLIENT_PRODUCER);

    pDataPath->setPipeID(0);

    m_pathList.push_back(pDataPath);

    pDataPath = new PathType(m_pStream);
    if (NULL == pDataPath)
    {
        DPLOGE("DpComposer: out of memory error %d\n", status);
        return DP_STATUS_OUT_OF_MEMORY;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


template <class PathType, class NodeType>
DP_STATUS_ENUM DpPathComposer::setupPath(STREAM_TYPE_ENUM scenario)
{
    DP_STATUS_ENUM          status;
    DpStream::ChanIterator  chanItor;
    uint32_t                JPGDLRotateEn;

    DP_TRACE_CALL();
    DPLOGI("DpComposer: compose path begin\n");

#if LOAD_BALANCE_BY_ENG_USAGE
    m_topology.getEngUsages();
#endif

    JPGDLRotateEn = 0;

    for (chanItor = m_pStream->chanBegin(); chanItor != m_pStream->chanEnd(); chanItor++)
    {
        STREAM_TYPE_ENUM        curType;
        DpChannel               *pChannel;
        DpPortAdapt             *pInPort;
        uint32_t                numOutputPort;
        DpChannel::PortIterator portItor;
        DpColorFormat           inFormat;
        int32_t                 inWidth;
        int32_t                 inHeight;
        int32_t                 inCropWidth;
        int32_t                 inCropWidthSubpixel;
        int32_t                 inCropHeight;
        int32_t                 inCropHeightSubpixel;
        int32_t                 dummy;

        PathType                *pDataPath;
        uint64_t                engFlag;
        PathInfo                pathInfo;
        DpEngineType            source = tNone;
        DpEngineType            target = tNone;

        int32_t                 index;
        for (index = 0; index < tTotal; index++)
        {
            pathInfo[index] = tNone;
        }

        curType = scenario;

        pChannel = chanItor->second();
        if (NULL == pChannel)
        {
            return DP_STATUS_INVALID_CHANNEL;
        }

        pInPort  = pChannel->getSourcePort();

        numOutputPort = pChannel->getOutPortNumber();
        portItor = pChannel->beginOutPort();

        if (numOutputPort == 1)
        {
            status = pInPort->getPortInfo(&inFormat,
                                          &inWidth,
                                          &inHeight,
                                          &dummy,
                                          &dummy);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                return status;
            }

            status = portItor->getSourceCrop(&dummy,
                                             &dummy,
                                             &dummy,
                                             &dummy,
                                             &inCropWidth,
                                             &inCropWidthSubpixel,
                                             &inCropHeight,
                                             &inCropHeightSubpixel);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                return status;
            }

            //Using the CROPPED SIZE as the final input size
            if ((inWidth > inCropWidth) && (0 < inCropWidth))
            {
                inWidth = inCropWidth;
            }

            if ((inHeight > inCropHeight) && (0 < inCropHeight))
            {
                inHeight = inCropHeight;
            }
        }

        pDataPath = new PathType(m_pStream);
        if (NULL == pDataPath)
        {
            DPLOGE("DpComposer: out of memory error %d\n", status);
            return DP_STATUS_OUT_OF_MEMORY;
        }

        // Default value
        engFlag = 0;

        do
        {
            DpPortAdapt     *pOutPort;
            DpColorFormat   outFormat;
            int32_t         outWidth;
            int32_t         outHeight;
            PORT_TYPE_ENUM  portType;

            pOutPort = &(*portItor);

            if ((90  == pOutPort->getRotation()) ||
                (270 == pOutPort->getRotation()))
            {
                status = pOutPort->getPortInfo(&outFormat,
                                               &outHeight,
                                               &outWidth,
                                               &dummy,
                                               &dummy);
            }
            else
            {
                status = pOutPort->getPortInfo(&outFormat,
                                               &outWidth,
                                               &outHeight,
                                               &dummy,
                                               &dummy);
            }
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                return status;
            }

            status = pOutPort->getPortType(&portType);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                return status;
            }

            if (PORT_JPEG == portType)
            {
                JPGDLRotateEn = pOutPort->getRotation();
            }

            if (PORT_VENC == portType)
            {
                DpVEnc_Config* pVEnc_cfg = NULL;

                status = pOutPort->getVENCConfig(&pVEnc_cfg);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    return status;
                }

                if (NULL == pVEnc_cfg->pVEncCommander)
                {
                    pVEnc_cfg->pVEncCommander = DpCommand::Factory(pDataPath, VENC_ENABLE_FLAG);
                }
                else
                {
                    pVEnc_cfg->pVEncCommander->init();
                }
                //
                pVEnc_cfg->pVEncCommander->setPath(pDataPath);
                //
                pDataPath->setExtCommander(pVEnc_cfg->pVEncCommander);
            }

            if ((numOutputPort == 1) &&
                #ifdef CONFIG_FOR_SOURCE_PQ
                (STREAM_COLOR_BITBLT != scenario) &&
                #endif
                (((inWidth  / outWidth)  > 64) ||
                 ((inHeight / outHeight) > 255) ||
                 ((outWidth  / inWidth)  > 32) ||
                 ((outHeight / inHeight) > 32)))
            {
                DpAutoBufferPool    *pPool;
                DpPortAdapt         *pMiddle;
                int32_t             tmpWidth;
                int32_t             tmpHeight;

                pPool = new DpAutoBufferPool();
                if (NULL == pPool)
                {
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                if ((inWidth / outWidth) > 64)
                {
                    tmpWidth = inWidth / 64;

                    // meet alignment limitation for UYVY format
                    tmpWidth += (tmpWidth & 0x1);
                }
                else if ((outWidth  / inWidth) > 32)
                {
                    tmpWidth = inWidth * 32;

                    // meet alignment limitation for UYVY format
                    tmpWidth += (tmpWidth & 0x1);
                }
                else
                {
                    tmpWidth = inWidth;
                }

                if ((inHeight / outHeight) > 255)
                {
                    tmpHeight = inHeight / 255;
                }
                else if ((outHeight / inHeight) > 32)
                {
                    tmpHeight = inHeight * 32;
                }
                else
                {
                    tmpHeight = inHeight;
                }

                DPLOGD("DpComposer: width/height for intermediate port = %d, %d", tmpWidth, tmpHeight);

                pPool->createBuffer(DP_COLOR_UYVY,
                                    tmpWidth,
                                    tmpHeight,
                                    DP_COLOR_GET_MIN_Y_PITCH(DP_COLOR_UYVY, tmpWidth),
                                    1);

                pMiddle = new DpPortAdapt(0,
                                          NULL,
                                          PORT_MEMORY,
                                          DP_COLOR_UYVY,
                                          tmpWidth,
                                          tmpHeight,
                                          DP_COLOR_GET_MIN_Y_PITCH(DP_COLOR_UYVY, tmpWidth),
                                          0,
                                          0,      // No Rotate
                                          false,  // No flip
                                          0,      // No id
                                          0,      // No sharpness
                                          false,  // No HDR
                                          false,  // No DRE
                                          false,  // No dither
                                          pPool);
                if (NULL == pMiddle)
                {
                    DPLOGE("DpComposer: out of memory error %d\n", status);
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                // Record the middle ports and buffers
                m_pTempPort[m_tempCount] = pMiddle;
                m_pTempPool[m_tempCount] = pPool;
                m_tempCount++;
                assert(m_tempCount <= DP_COMPOSER_MAX_TEMP_PORT);

                status = m_topology.getPathInfo(curType,
                                                *pInPort,
                                                *pMiddle,
                                                engFlag,
                                                pathInfo,
                                                source,
                                                target,
                                                0,  // Must be ZERO output for temp port
                                                0);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    DPLOGE("DpComposer: get path info %d\n", status);
                    return status;
                }

                status = createPathAndNew<PathType, NodeType>(pInPort,
                                                              pMiddle,
                                                              pathInfo,
                                                              source,
                                                              target,
                                                              pDataPath);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    return status;
                }

                DPLOGW("DpComposer: create another path for large scaling ratio\n");

                engFlag = 0;
                curType = STREAM_BITBLT;
                pInPort = pMiddle;
            }
            else if (STREAM_DUAL_BITBLT == scenario) //Create another path
            {
                status = m_topology.getPathInfo(curType,
                                                *pInPort,
                                                *pOutPort,
                                                engFlag,
                                                pathInfo,
                                                source,
                                                target,
                                                numOutputPort,
                                                pChannel->getVENCInserted());
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    DPLOGE("DpComposer: get path info %d\n", status);
                    return status;
                }

                status = createPathAndNew<PathType, NodeType>(pInPort,
                                                              pOutPort,
                                                              pathInfo,
                                                              source,
                                                              target,
                                                              pDataPath);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    return status;
                }

                DPLOGD("DpComposer: create another path for dual pipe\n");

                //engFlag = 0;
            }

            status = m_topology.getPathInfo(curType,
                                            *pInPort,
                                            *pOutPort,
                                            engFlag,
                                            pathInfo,
                                            source,
                                            target,
                                            numOutputPort,
                                            pChannel->getVENCInserted());
            if (DP_STATUS_CAN_NOT_MERGE == status)
            {
                if (PORT_JPEG != portType)
                {
                    DPLOGE("DpComposer: cannot merge path due to engine conflict\n");
                    return status;
                }

                switch(JPGDLRotateEn)
                {
                    case 0:
                        break;
                    case 90:
                        pDataPath->setTileOrder(0, 0, TILE_ORDER_BOTTOM_TO_TOP, (TILE_ORDER_Y_FIRST | TILE_ORDER_BOTTOM_TO_TOP));
                        break;
                    case 180:
                        pDataPath->setTileOrder(0,0, TILE_ORDER_BOTTOM_TO_TOP, (TILE_ORDER_BOTTOM_TO_TOP | TILE_ORDER_RIGHT_TO_LEFT));
                        break;
                    case 270:
                        pDataPath->setTileOrder(0, 0, 0, (TILE_ORDER_Y_FIRST | TILE_ORDER_RIGHT_TO_LEFT));
                        break;
                }

                pDataPath->setPipeID(0);

                m_pathList.push_back(pDataPath);

                pDataPath = new PathType(m_pStream);
                if (NULL == pDataPath)
                {
                    DPLOGE("DpComposer: out of memory error %d\n", status);
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                DPLOGW("DpComposer: create another path due to unable to merge path\n");

                JPGDLRotateEn = 0;
                engFlag = 0;

                continue;
            }

            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                return status;
            }

            if (0 == pDataPath->size())
            {
                status = createPath<PathType, NodeType>(pInPort,
                                                        pOutPort,
                                                        pathInfo,
                                                        source,
                                                        target,
                                                        pDataPath);

                pInPort->bindClient(CLIENT_CONSUMER);

                pOutPort->bindClient(CLIENT_PRODUCER);
            }
            else
            {
                status = mergePath<PathType, NodeType>(pInPort,
                                                       pOutPort,
                                                       pathInfo,
                                                       source,
                                                       target,
                                                       pDataPath);

                //merge path we need only to bind output port
                pOutPort->bindClient(CLIENT_PRODUCER);
            }

            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpComposer: compose path error %d\n", status);
                return status;
            }

            portItor++;
        } while(portItor != pChannel->endOutPort());

        switch(JPGDLRotateEn)
        {
            case 0:
                break;
            case 90:
                pDataPath->setTileOrder(0, 0, TILE_ORDER_BOTTOM_TO_TOP, (TILE_ORDER_Y_FIRST | TILE_ORDER_BOTTOM_TO_TOP));
                break;
            case 180:
                pDataPath->setTileOrder(0,0, TILE_ORDER_BOTTOM_TO_TOP, (TILE_ORDER_BOTTOM_TO_TOP | TILE_ORDER_RIGHT_TO_LEFT));
                break;
            case 270:
                pDataPath->setTileOrder(0, 0, 0, (TILE_ORDER_Y_FIRST | TILE_ORDER_RIGHT_TO_LEFT));
                break;
        }

        pDataPath->setPipeID(1);

        m_pathList.push_back(pDataPath);
    }

    DPLOGI("DpComposer: compose path end\n");

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpPathComposer::onCompose()
{
    DP_STATUS_ENUM     status;
    PathList::iterator iterator;
    STREAM_TYPE_ENUM   scenario;

    // Destroy the created path objects
    reset();

    scenario = m_pStream->getScenario();
    if ((STREAM_BITBLT == scenario)
        #ifdef CONFIG_FOR_SOURCE_PQ
        || (STREAM_COLOR_BITBLT == scenario)
        #endif
        || (STREAM_GPU_BITBLT == scenario)
        || (STREAM_2ND_BITBLT == scenario))
    {
        status = setupPath<DpTilePathST, DpTileEngine>(scenario);
    }
    else
    {
        status = setupPath<DpTilePathMT, DpTileEngine>(scenario);
    }

    return status;
}
