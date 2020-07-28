#ifndef __DP_DATA_PATH__
#define __DP_DATA_PATH__

#include "DpStream.h"
#include "DpPathBase.h"
#include "DpPortAdapt.h"
#include "DpTileEngine.h"
#include "DpPathConnection.h"
#include "DpEngineMutex.h"
#include "DpThread.h"
#include "DpMutex.h"

#include "tile_driver.h"
#include "tile_param.h"
#ifdef WPE_TILE
#include "wpe_cal.h"
#endif //WPE_TILE

#define DP_PATH_MAX_ENGINE_COUNT   32

template <typename Engine, typename Thread>
class DpPath: public DpPathBase, private Thread
{
public:
    enum PATH_STATE_ENUM
    {
        eUnknown,
        eInit,
        eConfig,
        eStart,
        eTile,
        eIdle,
        eStop
    };

    DpPath(DpStream *stream)
        : m_pathState(eUnknown),
          m_lastStatus(DP_STATUS_RETURN_SUCCESS),
          m_pStream(stream),
          m_pCommander(DpCommand::Factory(this)),
          m_pExtCommander(NULL),
          m_connection(this),
          m_engineMutex(this),
          m_engineCount(0),
          m_engineFlag(0),
          m_rootAndLeaf(0),
          m_pRootEngine(NULL),
          m_pendingTile(0),
          m_currentTile(0),
          m_pollDataPort(false),
          m_pollPortIndex(-1),
          m_inCalOrder(0),
          m_outCalOrder(0),
          m_inStreamOrder(0),
          m_outDumpOrder(0),
          m_tileID(0),
          m_pTileInfoFile(NULL),
          m_pipeID(0),
          m_pReadbackValues(NULL),
          m_numReadbackValues(0)
    {
        memset(m_pEngineBlock, 0x0, sizeof(m_pEngineBlock));
        memset(m_pRootAndLeaf, 0x0, sizeof(m_pRootAndLeaf));
    }

    virtual ~DpPath()
    {
        int32_t index;

        DPLOGI("DpPath: destroy engine begin\n");

    #if 1
        // Prevent some body is waitting for me
        m_pathMutexObj.lock();
        m_pathState = eStop;
        m_condition.signal();
        m_pathMutexObj.unlock();
    #endif // 0

        for (index = 0; index < m_engineCount; index++)
        {
            delete m_pEngineBlock[index];
            m_pEngineBlock[index] = NULL;
        }

        delete m_pCommander;
        m_pCommander = NULL;

        DPLOGI("DpPath: destroy engine end\n");
    }

    virtual uint32_t size()
    {
        return m_engineCount;
    }

    virtual iterator begin()
    {
        return iterator((DpEngineBase**)m_pEngineBlock);
    }

    virtual iterator end()
    {
        return iterator((DpEngineBase**)m_pEngineBlock + m_engineCount);
    }

    DP_STATUS_ENUM insert(Engine *pPrev,
                          Engine *pCurr)
    {
        if (NULL == pCurr)
        {
            return DP_STATUS_INVALID_PARAX;
        }

        if (m_engineCount >= DP_PATH_MAX_ENGINE_COUNT)
        {
            return DP_STATUS_OVER_MAX_ENGINE;
        }

        pCurr->setPrevEngine(pPrev);

        if (NULL != pPrev)
        {
            pPrev->addNextEngine(pCurr);
        }
        else
        {
            if ((NULL == m_pRootEngine) ||
                (m_pRootEngine == pCurr))
            {
                m_pRootEngine = pCurr;
            }
            else
            {
                return DP_STATUS_INVALID_STATE;
            }
        }

        m_pEngineBlock[m_engineCount] = pCurr;
        m_engineCount++;

        return DP_STATUS_RETURN_SUCCESS;
    }

    Engine* find(DpEngineType type)
    {
        int32_t index;

        for (index = 0; index < m_engineCount; index++)
        {
            if (type == m_pEngineBlock[index]->getEngineType())
            {
                return m_pEngineBlock[index];
            }
        }

        return NULL;
    }

    DP_STATUS_ENUM config(int32_t count = 65535);

    DP_STATUS_ENUM configFrame(DpConfig &config)
    {
        DP_TRACE_CALL();
        return onConfigFrame(*m_pCommander, config);
    }

    DP_STATUS_ENUM configFrameMode()
    {
        DP_TRACE_CALL();
        return onConfigFrameMode(*m_pCommander);
    }

    DP_STATUS_ENUM configTile()
    {
        DP_TRACE_CALL();
        return onConfigTile(*m_pCommander);
    }

    DP_STATUS_ENUM configCommandEnd()
    {
        DP_TRACE_CALL();
        return onConfigCommandEnd(*m_pCommander);
    }

    DP_STATUS_ENUM dumpDebugInfo()
    {
        return onDumpDebugInfo();
    }

    DP_STATUS_ENUM setTileOrder(uint32_t inCalOrder, uint32_t outCalOrder, uint32_t inStreamOrder, uint32_t outDumpOrder)
    {
        m_inCalOrder = inCalOrder;
        m_outCalOrder = outCalOrder;
        m_inStreamOrder = inStreamOrder;
        m_outDumpOrder = outDumpOrder;
        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM getTileOrder(uint32_t& inCalOrder, uint32_t& outCalOrder, uint32_t& inStreamOrder, uint32_t& outDumpOrder)
    {
        inCalOrder = m_inCalOrder;
        outCalOrder = m_outCalOrder;
        inStreamOrder = m_inStreamOrder;
        outDumpOrder = m_outDumpOrder;
        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM setExtCommander(DpCommand *extCommander)
    {
        m_pExtCommander = extCommander;
        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM setReadbackValues(const uint32_t* pReadbackValues, uint32_t numValues)
    {
        m_pReadbackValues = pReadbackValues;
        m_numReadbackValues = numValues;

        DPLOGI("setReadbackValues in DpDataPath is done\n");
        return DP_STATUS_RETURN_SUCCESS;
    }

    void setPipeID(int pipe)
    {
        m_pipeID = pipe;
    }

    void getPipeID(int *pipe)
    {
        *pipe = m_pipeID;
    }


private:
    PATH_STATE_ENUM  m_pathState;
    DP_STATUS_ENUM   m_lastStatus;
    DpStream         *m_pStream;
    DpCommand        *m_pCommander;
    DpCommand        *m_pExtCommander;
    DpPathConnection m_connection;
    DpEngineMutex    m_engineMutex;
    int32_t          m_engineCount;
    int64_t          m_engineFlag;
    int32_t          m_rootAndLeaf;
    Engine           *m_pRootEngine;
    Engine           *m_pEngineBlock[DP_PATH_MAX_ENGINE_COUNT];
    Engine           *m_pRootAndLeaf[DP_PATH_MAX_ENGINE_COUNT];
    mutable DpMutex  m_pathMutexObj;
    DpCondition      m_condition;
    int32_t          m_pendingTile;
    int32_t          m_currentTile;
    bool             m_pollDataPort;
    int32_t          m_pollPortIndex;

    int32_t          m_inCalOrder;
    int32_t          m_outCalOrder;
    int32_t          m_inStreamOrder;
    int32_t          m_outDumpOrder;

    TILE_PARAM_STRUCT       m_tileParam;
    TILE_REG_MAP_STRUCT     m_tileRegMap;
    FUNC_DESCRIPTION_STRUCT m_tileFunc;
    uint32_t                m_tileID;

#ifdef WPE_TILE
    WPE_TPIPE_CONFIG_STRUCT   m_wpeParam;
#endif //WPE_TILE

    FILE                    *m_pTileInfoFile;

    int32_t         m_pipeID;
    const uint32_t  *m_pReadbackValues;
    uint32_t        m_numReadbackValues;

    inline int32_t lock() const;

    inline void unlock() const;

    inline PATH_STATE_ENUM getState() const
    {
        lock();
        PATH_STATE_ENUM state = m_pathState;
        unlock();
        return state;
    }

    inline void testSetState(PATH_STATE_ENUM test, PATH_STATE_ENUM state)
    {
        lock();
        if (test != m_pathState)
        {
            m_pathState = state;
        }
        unlock();
    }

    virtual DP_STATUS_ENUM onGetScenario(STREAM_TYPE_ENUM *pType)
    {
        if(NULL == m_pStream)
        {
            DPLOGE("onGetScenario: NULL == m_pStream\n");
            return DP_STATUS_INVALID_STATE;
        }

        *pType = m_pStream->getScenario();

        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onGetPQReadback(bool *pReadback)
    {
        if(NULL == m_pStream)
        {
            DPLOGE("onGetPQReadback: NULL == m_pStream\n");
            return DP_STATUS_INVALID_STATE;
        }

        *pReadback = m_pStream->getPQReadback();

        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onGetHDRReadback(bool *pReadback)
    {
        if(NULL == m_pStream)
        {
            DPLOGE("onGetHDRReadback: NULL == m_pStream\n");
            return DP_STATUS_INVALID_STATE;
        }

        *pReadback = m_pStream->getHDRReadback();

        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onGetDREReadback(int32_t *pReadback)
    {
        if(NULL == m_pStream)
        {
            DPLOGE("onGetDREReadback: NULL == m_pStream\n");
            return DP_STATUS_INVALID_STATE;
        }

        *pReadback = m_pStream->getDREReadback();

        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onGetPriority(int32_t *pPriority)
    {
        *pPriority = Thread::priority();

        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onGetPathFlag(int64_t *pFlag)
    {
        int32_t index;

        if (0 == m_engineCount)
        {
            return DP_STATUS_INVALID_STATE;
        }

        if (0 == m_engineFlag)
        {
            for (index = 0; index < m_engineCount; index++)
            {
                m_engineFlag |= m_pEngineBlock[index]->getEngineFlag();
            }
        }

        *pFlag = m_engineFlag;

        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual const uint32_t* onGetReadbackValues(uint32_t& numValues)
    {
        numValues = m_numReadbackValues;
        return m_pReadbackValues;
    }

    virtual DP_STATUS_ENUM onGetCommander(DpCommand **commander)
    {
        *commander = m_pCommander;
        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM pollPort(bool waitPort)
    {
        DP_STATUS_ENUM  status;
        int32_t         index;

        DPLOGI("DpPath: poll port begin\n");

        index = 0;

        lock();
        m_pollDataPort  = true;
        m_pollPortIndex = 0;
        unlock();

        while(1)
        {
            lock();
            if (eStop == m_pathState)
            {
                unlock();
                break;
            }
            unlock();

            DPLOGI("DpPath: poll data port\n");
            status = m_pRootAndLeaf[index]->pollPort(waitPort);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGI("DpPath: abort poll port\n");

                lock();
                m_pollDataPort  = false;
                unlock();

                return status;
            }
            else
            {
                index++;

                lock();
                m_pollPortIndex = index;
                unlock();
            }

            if (index >= m_rootAndLeaf)
            {
                DPLOGI("DpPath: all ports are ready\n");
                break;
            }
        }

        DPLOGI("DpPath: poll port end\n");

        lock();
        m_pollDataPort = false;
        unlock();

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM flushBuffer(DpEngineBase::FLUSH_TYPE_ENUM type)
    {
        DP_STATUS_ENUM status;
        int32_t        index;

        DPLOGI("DpPath: flush buffer begin\n");

        for (index = 0; index < m_rootAndLeaf; index++)
        {
            status = m_pRootAndLeaf[index]->flushBuffer(type);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpPath: flush buffer failed\n");
                return status;
            }
        }

        DPLOGI("DpPath: flush buffer end\n");

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM dumpBuffer()
    {
        DPLOGI("DpPath: dump buffer begin\n");

        DP_STATUS_ENUM status;
        int32_t        index;

        for (index = 0; index < m_rootAndLeaf; index++)
        {
            status = m_pRootAndLeaf[index]->dumpBuffer();
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpPath: dump buffer failed\n");
                return status;
            }
        }

        DPLOGI("DpPath: dump buffer end\n");

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM flipBuffer()
    {
        DP_STATUS_ENUM status;
        int32_t        index;

        DPLOGI("DpPath: flip buffer begin\n");

        for (index = m_rootAndLeaf - 1; index >= 0; index--)
        {
            status = m_pRootAndLeaf[index]->flipBuffer();
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpPath: flip buffer failed\n");
                return status;
            }
        }

        DPLOGI("DpPath: flip buffer end\n");

        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onInitPath()
    {
        int32_t count;
        int32_t index;
        int32_t inner;
        Engine  *pCurr;
        Engine  *pNext;
        int32_t next;

        lock();

        m_pEngineBlock[0] = m_pRootEngine;
        count= 1;

        m_pRootAndLeaf[0] = m_pRootEngine;
        m_rootAndLeaf = 1;

        for (index = 0; index < m_engineCount; index++)
        {
            pCurr = m_pEngineBlock[index];
            if (NULL == pCurr)
            {
                DPLOGE("DpPath: get engine of index(%d) failed\n", index);
                unlock();
                return DP_STATUS_INVALID_STATE;
            }

            if (true == pCurr->isLeafEngine())
            {
                m_pRootAndLeaf[m_rootAndLeaf] = pCurr;
                m_rootAndLeaf++;
            }
            else
            {
                next = pCurr->getNextCount();
                for (inner = 0; inner < next; inner++)
                {
                    pNext = pCurr->getNextEngine(inner);
                    if (NULL == pNext)
                    {
                        break;
                    }

                    m_pEngineBlock[count] = pNext;
                    count++;
                }
            }
        }

        if (count != m_engineCount)
        {
            DPLOGE("DpPath: initialize the path(%p) failed\n", this);
            unlock();
            return DP_STATUS_INVALID_STATE;
        }

        // Already locked
        m_pathState = eInit;

        unlock();
        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onStartPath()
    {
        DP_STATUS_ENUM   status;
        STREAM_TYPE_ENUM scenario;

        DPLOGI("DpPath: start path begin\n");

        lock();
        if ((eIdle == m_pathState) ||
            (eStop == m_pathState))
        {
            m_pathState = eInit;
        }
        unlock();

        DPLOGI("DpPath: start the path 0x%08x with state(%d)\n", this, m_pathState);

        status = getScenario(&scenario);
        if ((DP_STATUS_RETURN_SUCCESS == status) &&
            (//(STREAM_FRAG_JPEGDEC == scenario) ||
             (STREAM_ISP_VSS == scenario) ||
             (STREAM_ISP_ZSD_SLOW == scenario)))
        {
            status = Thread::run(Thread::PRIORITY_LOW);
            DPLOGI("DpPath: got VSS type to pass2B\n");
        }
        else
        {
            status = Thread::run(Thread::PRIORITY_HIGH);
        }

        DPLOGI("DpPath: start path end\n");

        return status;
    }

    virtual DP_STATUS_ENUM onWaitPath()
    {
        DP_STATUS_ENUM status = DP_STATUS_RETURN_SUCCESS;
        //int32_t        retry  = 3;

        DPLOGI("DpPath: wait path begin\n");

        m_pathMutexObj.lock();

        while((eIdle != m_pathState) &&
              (eStop != m_pathState))
        {
        #if 1
            DPLOGI("DpPath: wait condition\n");
            m_condition.wait(m_pathMutexObj);
        #else
            if (m_condition.waitRelative(m_pathMutexObj, 999000000) < 0)
            {
                DPLOGW("DpPath: wait the path timeout, state %d\n", m_pathState);
            }

            if (--retry == 0)
            {
                DPLOGE("DpPath: wait the path timeout, state %d, retried for %d times\n", m_pathState, retry);
                dumpDebugInfo();

                m_pathState  = eStop;
                m_lastStatus = DP_STATUS_INVALID_STATE;
            }
        #endif // 0
        }

        status = m_lastStatus;
        if (status >= 0)
        {
            DPLOGI("DpPath: the last status %d\n", status);
            status = DP_STATUS_RETURN_SUCCESS;
        }

        m_pathMutexObj.unlock();

        DPLOGI("DpPath: wait path end\n");

        return status;
    }

    virtual DP_STATUS_ENUM onStopPath()
    {
        DPLOGI("DpPath: stop path begin\n");

        DPLOGI("DpPath: set state to stop\n");

        testSetState(eStop, eStop);

        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onAbortPath()
    {
        int32_t index;

        DPLOGI("DpPath: abort polling port 0x%08x\n", this);

        for (index = 0; index < m_rootAndLeaf; index++)
        {
            m_pRootAndLeaf[index]->abortPollPort();
        }

        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onDestroyPath()
    {
        DPLOGI("DpPath: join the thread\n");

        Thread::join();

        DPLOGI("DpPath: stop path end\n");

        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onDumpDebugPath()
    {
        dumpDebugInfo();
        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM calcFrameInfo(DpCommand &command);

    DP_STATUS_ENUM dumpTileAccumulation();

    DP_STATUS_ENUM onConfigFrame(DpCommand &command,
                                 DpConfig  &config);

    virtual DP_STATUS_ENUM onGetPMQOS(mdp_pmqos *pPMQOS)
    {
        return configPMQOS(*m_pCommander, pPMQOS);
    }
    DP_STATUS_ENUM configPMQOS(DpCommand &command, mdp_pmqos *pPMQOS)
    {
        Engine  *pCurr;
        int32_t index;
        int32_t mdpCounter = 0;
        int32_t ispCounter = 0;
        int32_t engineType;
        uint32_t totalPixel = 0;
        uint32_t mdpDataSize = 0;
        uint32_t ispDataSize = 0;
        uint32_t tmpPixel = 0;
        uint32_t tmpDataSize = 0;
        DP_STATUS_ENUM rtn;

        struct timeval end_Time;

        pPMQOS->qos2_isp_count = 0;
        pPMQOS->qos2_mdp_count = 0;
        pPMQOS->isp_total_pixel = 0;
        pPMQOS->isp_total_datasize = 0;
        pPMQOS->mdp_total_pixel = 0;
        pPMQOS->mdp_total_datasize = 0;

        for (index = 0; index < m_engineCount; index++)
        {
            pCurr = m_pEngineBlock[index];

            tmpPixel = 0;
            tmpDataSize = 0;
            engineType = 0;

            assert(NULL != pCurr);

            if (0 == (pCurr->queryFeature() & pCurr->eISP))
            {
                pCurr->updatePMQOS(command, tmpPixel, tmpDataSize, engineType, end_Time);
                if(engineType != 0)
                {
                    pPMQOS->qos2_mdp_bandwidth[mdpCounter] = tmpDataSize; //Uint: byte
                    pPMQOS->qos2_mdp_total_pixel[mdpCounter] = tmpPixel; //Uint: pixel
                    pPMQOS->qos2_mdp_port[mdpCounter++] = engineType;
                    pPMQOS->qos2_mdp_count = mdpCounter;
                }
                pPMQOS->mdp_total_datasize += tmpDataSize;
                if (tmpPixel > pPMQOS->mdp_total_pixel)
                {
                    pPMQOS->mdp_total_pixel = tmpPixel;
                }
            }
            else
            {
                do{
                    tmpPixel = 0;
                    tmpDataSize = 0;
                    engineType = 0;
                    //tIMGI or tIMG2O
                    rtn = pCurr->updatePMQOS(command, tmpPixel, tmpDataSize, engineType, end_Time);
                    if(engineType != 0)
                    {
                        pPMQOS->qos2_isp_bandwidth[ispCounter] = tmpDataSize; //Uint: byte
                        pPMQOS->qos2_isp_total_pixel[ispCounter] = tmpPixel; //Uint: pixel
                        pPMQOS->qos2_isp_port[ispCounter++] = engineType;
                        pPMQOS->qos2_isp_count = ispCounter;
                    }
                    if (tmpPixel > pPMQOS->isp_total_pixel)
                    {
                        pPMQOS->isp_total_pixel = tmpPixel;
                    }
                    pPMQOS->isp_total_datasize += tmpDataSize;
                } while(rtn > 0);
            }

        }

        DPLOGI("DpPath: qos2 isp count= %d, mdp count = %d\n", pPMQOS->qos2_isp_count, pPMQOS->qos2_mdp_count);
        DPLOGI("DpPath: ispDataSize = %d, mdpDataSize = %d\n", ispDataSize, mdpDataSize);
        pPMQOS->tv_sec= end_Time.tv_sec;
        pPMQOS->tv_usec = end_Time.tv_usec;
        return DP_STATUS_RETURN_SUCCESS;

    }

    DP_STATUS_ENUM onConfigFrameMode(DpCommand &command)
    {
        DP_STATUS_ENUM  status;
        PATH_STATE_ENUM state;
        int32_t         index;
        Engine          *pCurr;
        int             wpe_result = 0;

        state = getState();

#if !CONFIG_FOR_VERIFY_FPGA
        if (DpStream::CONFIG_FRAME_ONLY & m_pStream->getConfigFlags())
        {
            command.dup(DpCommand::TILE_COMMAND);

            // Config tile information
            for (index = (m_engineCount - 1); index >= 0; index--)
            {
                pCurr = m_pEngineBlock[index];
                assert(NULL != pCurr);

                status = pCurr->reconfigTiles(command);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    DPLOGE("DpTilePath: reconfig tiles error %d\n", status);

                    m_lastStatus = status;
                    return status;
                }
            }

            m_lastStatus = DP_STATUS_ALL_TPIPE_DONE;

            m_pendingTile++;
            testSetState(eStop, eStart);

            return m_lastStatus;
        }
        else if (eConfig == state) // eTile != state
#endif
        {
            DPLOGI("DpTilePath: config frame mode begin\n");

            command.mark(DpCommand::TILE_COMMAND);

            // Retrieve tile information
            for (index = 0; index < m_engineCount; index++)
            {
                pCurr = m_pEngineBlock[index];
                assert(NULL != pCurr);

                status = pCurr->retrieveTileParam(&m_tileParam);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    DPLOGE("DpTilePath: retrieve tile error %d\n", status);
                    m_lastStatus = status;
                    return status;
                }
            }

            // Acquire engine mutex
            if (m_engineMutex.require(command) == false)
            {
                DPLOGE("DpTilePath: require mutex fail\n");
                return DP_STATUS_OPERATION_FAILED;
            }

            // Enable mux settings
            m_connection.enablePath(command);

            // Config tile information
            for (index = (m_engineCount - 1); index >= 0; index--)
            {
                pCurr = m_pEngineBlock[index];
                assert(NULL != pCurr);

                status = pCurr->configTile(command);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    DPLOGE("DpTilePath: config tile error %d\n", status);

                    // Release engine mutex
                    m_engineMutex.release(command);

                    // Disable mux settings
                    m_connection.disablePath(command);

                    m_lastStatus = status;
                    return status;
                }
            }

            m_engineMutex.release(command);

            for (index = 0; index < m_rootAndLeaf; index++)
            {
                pCurr = m_pRootAndLeaf[index];
                assert(NULL != pCurr);

                if (false == pCurr->isOutputDisable())
                {
                    // Wait root engine frame done
                    pCurr->waitEvent(command);
                }
            }

            // Disable mux settings
            m_connection.disablePath(command);

            // Post processing information
            for (index = 0; index < m_engineCount; index++)
            {
                pCurr = m_pEngineBlock[index];
                assert(NULL != pCurr);

                status = pCurr->postProc(command);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    DPLOGE("DpTilePath: post-procedure error %d\n", status);

                    // Disable mux settings
                    m_connection.disablePath(command);

                    m_lastStatus = status;
                    return status;
                }
            }

#ifdef WPE_TILE
            WPE_WORKING_BUFFER_STRUCT wpeWorkingBuffer;

            if (DpStream::CONFIG_WPE_ONLY_TILE_MODE & m_pStream->getConfigFlags())
            {
                //WPE_ONLY
                wpe_result = wpe_cal_main(&m_wpeParam,
                                          0,
                                          (char *)(&wpeWorkingBuffer),
                                          sizeof(wpeWorkingBuffer),
                                          NULL,
                                          0,
                                          0,
                                          (int *)(&(m_tileParam.ptr_isp_tile_descriptor->used_tpipe_no_wpe_s)),
                                          (char *)(m_tileParam.ptr_isp_tile_descriptor->tpipe_config_wpe_s),
                                          query_wpe_tdr_out_buffer_size(32),
                                          (int *)(&(m_tileParam.ptr_isp_tile_descriptor->config_no_per_tpipe_wpe_s)),
                                          NULL,
                                          0,
                                          NULL);
                if (wpe_result)
                {
                    DPLOGE("DpDataPath::onConfigFrameMode WPE Tile error\n");
                }
            }
#endif //WPE_TILE

            m_lastStatus = DP_STATUS_ALL_TPIPE_DONE;

            m_pendingTile++;
            testSetState(eStop, eStart);

            DPLOGI("DpTilePath: config frame mode done %d\n", m_lastStatus);
            return m_lastStatus;
        }

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM onConfigTile(DpCommand &command);

    DP_STATUS_ENUM onConfigCommandEnd(DpCommand &command);

    DP_STATUS_ENUM onDumpDebugInfo()
    {
        int32_t index;
        Engine  *pEngine;

        DPLOGD("DpPath: dump debug info begin\n");

        //lock();

        // Status from the base class
        DPLOGD("DpDataPath: m_pathState:     0x%08x\n", m_pathState);
        DPLOGD("DpDataPath: m_lastStatus:    %d\n", m_lastStatus);

        // Status of the data path
        DPLOGD("DpDataPath: m_pollDataPort:  0x%08x\n", m_pollDataPort);
        DPLOGD("DpDataPath: m_pollPortIndex: 0x%08x\n", m_pollPortIndex);
        //unlock();

        for (index = 0; index < m_rootAndLeaf; index++)
        {
            pEngine = m_pRootAndLeaf[index];
            if (pEngine->isLeafEngine())
            {
                DPLOGD("leaf Engine: %s\n", pEngine->getEngineName());
            }
            else
            {
                DPLOGD("root Engine: %s\n", pEngine->getEngineName());
            }

            pEngine->dumpDebugInfo();
        }

        DPLOGD("DpPath: dump debug info end\n");

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM dumpTileDebugInfo(ISP_MESSAGE_ENUM result);

    DP_STATUS_ENUM execCommand()
    {
        DP_TRACE_CALL();
        DP_STATUS_ENUM  status;
        PATH_STATE_ENUM state;
        bool            bWaitDone = true;
        DpJobID         hJob = 0;
        DpJobID         hExtJob = 0;
        char            *dpFramInfo = NULL;

        // Flush read buffer
        status = flushBuffer(DpEngineBase::FLUSH_BEFORE_HW_READ);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            m_lastStatus = status;
            return status;
        }

        bWaitDone = m_pStream->getSyncMode();

        if (bWaitDone)
        {
            m_pCommander->setReadbackRegs(); // old version pq readback must remove
        }

        // Exec
#ifdef ISP_SMART_TILE_ENABLE
        if (STREAM_ISP_VSS == m_pStream->getScenario())
        {
            m_pCommander->stop(false);
            if (0 != m_currentTile)
            {
                status = m_pCommander->flush(bWaitDone, &hJob, &dpFramInfo);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    m_lastStatus = status;
                    dumpBuffer();  // Dump output buffer
                    return status;
                }
            }
            m_pCommander->swap();
        }
        else
#endif
        {
            m_pCommander->stop();
            m_pCommander->swap();
            status = m_pCommander->flush(bWaitDone, &hJob, &dpFramInfo);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                m_lastStatus = status;
                dumpBuffer();  // Dump output buffer
                return status;
            }
        }

        if ((STREAM_ISP_VR == m_pStream->getScenario() || STREAM_ISP_ZSD == m_pStream->getScenario()) &&
            (NULL != m_pExtCommander))
        {
            m_pExtCommander->setReadbackRegs(); // old version pq readback must remove

            m_pExtCommander->stop();
            m_pExtCommander->swap();
            status = m_pExtCommander->flush(bWaitDone, &hExtJob);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                m_lastStatus = status;
                dumpBuffer();  // Dump output buffer
                return status;
            }

            DPLOGI("Flush ExtCommander job handler: %x\n", hExtJob);
            if (0 == hExtJob)
            {
                DPLOGE("Flush m_pExtCommander job handler failed\n");
                return DP_STATUS_INVALID_OPCODE;
            }
        }

#ifdef ISP_SMART_TILE_ENABLE
        if ((STREAM_ISP_VSS != m_pStream->getScenario()) || (0 != m_currentTile))
#endif
        {
            status = finishCommand(bWaitDone, hJob, hExtJob, dpFramInfo);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                return status;
            }
        }

        state = getState();

        if (eStart == state)
        {
#ifdef ISP_SMART_TILE_ENABLE
            if (STREAM_ISP_VSS == m_pStream->getScenario())
            {
                status = m_pCommander->flush(bWaitDone, &hJob, &dpFramInfo);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    m_lastStatus = status;
                    dumpBuffer();  // Dump output buffer
                    return status;
                }

                status = finishCommand(bWaitDone, hJob, hExtJob, dpFramInfo);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    return status;
                }
            }
#endif
            if ((true == bWaitDone) &&
                (0 == (DpStream::CONFIG_BUFFER_IGNORE & m_pStream->getConfigFlags())))
            {
                status = flipBuffer();
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    m_lastStatus = status;
                    return status;
                }
            }

            // Reset to idle status
            m_pathMutexObj.lock();
            if (eStop != m_pathState)
            {
                m_pathState = eIdle;
            }
            m_condition.signal();
            m_pathMutexObj.unlock();

            m_currentTile = 0;
        }
        else if (eTile == state)
        {
            // Need to config frame info
            testSetState(eStop, eConfig);

            m_currentTile += m_pendingTile;
        }

        // Reset pending tile
        m_pendingTile = 0;
        m_pCommander->reset();

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM finishCommand(bool bWaitDone, DpJobID hJob, DpJobID hExtJob, char *dpFramInfo)
    {
        DP_STATUS_ENUM  status;

        if (false == bWaitDone)
        {
            if (0 != hJob)
            {
#ifdef MDP_VSS_ASYNC_ENABLE
                if (STREAM_ISP_VSS == m_pStream->getScenario()) // vss has multiple jobs
                {
                    m_pStream->addAsyncJob(hJob, dpFramInfo);
                    DPLOGI("VSS job handler: %llx\n",hJob);
                }
                else // normal precedure
#endif
                {
                    m_pStream->setAsyncJob(hJob, 1, dpFramInfo);
                    DPLOGI("Flush job handler: %x\n",hJob);

                    if (0 != hExtJob)
                    {
                        // TODO: using enum to represent this!!!
                        m_pStream->setAsyncJob(hExtJob,2, dpFramInfo); // Means VEnc Commander
                    }
                }
            }
            else
            {
                dumpBuffer(); // Dump output buffer
                DPLOGI("Flush job failure for no return Job Handler\n");
                return DP_STATUS_OPERATION_FAILED;
            }
        }
        else // true == bWaitDone
        {
            // Flush write buffer
            uint32_t numValues;
            uint32_t *pReadbackValues = m_pCommander->getReadbackValues(numValues);

            if (STREAM_DUAL_BITBLT == m_pStream->getScenario())
            {
                setReadbackValues(pReadbackValues, numValues);
            }
            else
            {
                m_pStream->setReadbackValues(pReadbackValues, numValues);
            }

            status = flushBuffer(DpEngineBase::FLUSH_AFTER_HW_WRITE);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                m_lastStatus = status;
                return status;
            }
            dumpBuffer();
        }

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM waitFence()
    {
        DP_STATUS_ENUM status;
        DP_STATUS_ENUM engineStatus;
        int32_t        index;

        DPLOGI("DpPath: waitFence begin\n");

        status = DP_STATUS_RETURN_SUCCESS;

        for (index = 0; index < m_rootAndLeaf; index++)
        {
            engineStatus = m_pRootAndLeaf[index]->waitFence();
            if (DP_STATUS_RETURN_SUCCESS != engineStatus)
            {
                DPLOGE("DpPath: RootAndLeaf[%d] waitFence failed %d \n", index, engineStatus);
                status = engineStatus;
            }
        }

        DPLOGI("DpPath: waitFence end\n");

        return status;
    }

    virtual DP_STATUS_ENUM threadLoop();

    virtual DP_STATUS_ENUM onGetSyncMode(bool *pSyncMode)
    {
        if(NULL == m_pStream)
        {
            DPLOGE("onGetSyncMode: NULL == m_pStream\n");
            return DP_STATUS_INVALID_STATE;
        }

        *pSyncMode = m_pStream->getSyncMode();

        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual uint32_t* onGetReadbackPABuffer(uint32_t& readbackPABufferIndex)
    {
        if(NULL == m_pStream)
        {
            DPLOGE("getReadbackPABuffer: NULL == m_pStream\n");
            return NULL;
        }

        return m_pStream->getReadbackPABuffer(readbackPABufferIndex);
    }

    virtual DP_STATUS_ENUM onSetNumReadbackPABuffer(uint32_t numReadbackPABuffer, uint32_t readbackEngineID)
    {
        if(NULL == m_pStream)
        {
            DPLOGE("setNumReadbackPABuffer: NULL == m_pStream\n");
            return DP_STATUS_INVALID_STATE;
        }

        return m_pStream->setNumReadbackPABuffer(numReadbackPABuffer, readbackEngineID);
    }
};

template <>
inline int32_t DpPath<DpTileEngine, DpSingleThread>::lock() const { return 0; }

template <>
inline void DpPath<DpTileEngine, DpSingleThread>::unlock() const {}

template <>
inline int32_t DpPath<DpTileEngine, DpMultiThread>::lock() const { return m_pathMutexObj.lock(); }

template <>
inline void DpPath<DpTileEngine, DpMultiThread>::unlock() const { m_pathMutexObj.unlock(); }

#endif  // __DP_DATA_PATH__
