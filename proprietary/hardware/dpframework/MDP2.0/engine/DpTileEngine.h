#ifndef __DP_TILE_ENGINE_H__
#define __DP_TILE_ENGINE_H__

#include "DpEngineBase.h"

#define TILE_MAX_BRANCH_COUNT           (5)
#define TILE_MAX_BACKUP_COUNT           (32)

#define TILE_WITHOUT_CONSTRAINT         (0)

struct TILE_PARAM_STRUCT;
struct TILE_FUNC_BLOCK_STRUCT;
struct TILE_FUNC_DATA_STRUCT;

class DpTileEngine: public DpEngineBase, public DpEngineNode<DpTileEngine>
{
public:
    DpTileEngine(uint32_t identifier)
        : DpEngineBase(identifier),
          m_bypassEngine(false),
          m_tileDisable(false),
          m_inCalOrder(0),
          m_outCalOrder(0),
          m_inStreamOrder(0),
          m_outDumpOrder(0),
          m_inTileXLeft(0),
          m_inTileXRight(0),
          m_inTileYTop(0),
          m_inTileYBottom(0),
          m_outTileXLeft(0),
          m_outTileXRight(0),
          m_outTileYTop(0),
          m_outTileYBottom(0),
          m_lumaXOffset(0),
          m_lumaXSubpixel(0),
          m_lumaYOffset(0),
          m_lumaYSubpixel(0),
          m_chromaXOffset(0),
          m_chromaXSubpixel(0),
          m_chromaYOffset(0),
          m_chromaYSubpixel(0),
          m_outVertical(0),
          m_outHorizontal(0),
          m_inTileAccumulation(0),
          m_outTileAccumulation(0),
          m_pFunc(NULL),
          m_pData(NULL)
    {
    }

    virtual ~DpTileEngine()
    {
    }

    DP_STATUS_ENUM initTileCalc(struct TILE_PARAM_STRUCT* p_tile_param)
    {
        DP_STATUS_ENUM status;

        status = onInitTileCalc(p_tile_param);
        //m_engineState = eConfig;

        return status;
    }

    DP_STATUS_ENUM retrieveFrameParam(struct TILE_PARAM_STRUCT* p_tile_param, DpCommand &command)
    {
        return onRetrieveFrameParam(p_tile_param, command);
    }

    DP_STATUS_ENUM retrieveTileParam(struct TILE_PARAM_STRUCT* p_tile_param)
    {
        return onRetrieveTileParam(p_tile_param);
    }

    DP_STATUS_ENUM dumpTileAccumulation();

    DP_STATUS_ENUM updatePMQOS(DpCommand &command, uint32_t& totalPixel, uint32_t& totalDataSzie, int32_t& engineType, struct timeval& time)
    {
        DP_TRACE_CALL();
        DP_STATUS_ENUM status;

        status = DP_STATUS_RETURN_SUCCESS;

        if (false == m_outputDisable)
        {
            status = onUpdatePMQOS(command, totalPixel, totalDataSzie, engineType, time);
        }

        return status;
    }

    DP_STATUS_ENUM configTile(DpCommand &command)
    {
        DP_TRACE_CALL();
        DP_STATUS_ENUM status;

        status = DP_STATUS_RETURN_SUCCESS;

        if (false == m_outputDisable)
        {
            if (false == m_tileDisable)
            {
                status = onConfigTile(command);
            }
            //m_engineState = eTile;
        }

        return status;
    }

    DP_STATUS_ENUM reconfigTiles(DpCommand &command)
    {
        DP_TRACE_CALL();
        DP_STATUS_ENUM status;

        status = DP_STATUS_RETURN_SUCCESS;

        if (false == m_outputDisable)
        {
            if (false == m_tileDisable)
            {
                status = onReconfigTiles(command);
            }
            //m_engineState = eTile;
        }

        return status;
    }

    DP_STATUS_ENUM postProc(DpCommand& command)
    {
        return onPostProc(command);
    }

    DP_STATUS_ENUM advanceTile(DpCommand& command, struct TILE_PARAM_STRUCT* p_tile_param);

    DP_STATUS_ENUM commandEnd(DpCommand& command)
    {
        return onCommandEnd(command);
    }

    virtual bool isOutputDisable()
    {
        return m_outputDisable || m_tileDisable;
    }

    DP_STATUS_ENUM getTileNumber(uint32_t& nHorizontal, uint32_t& nVertical)
    {
        nHorizontal = m_outHorizontal;
        nVertical = m_outVertical;
        return DP_STATUS_RETURN_SUCCESS;
    }


protected:

    bool                m_bypassEngine;
    bool                m_tileDisable;

    int32_t             m_inCalOrder;
    int32_t             m_outCalOrder;

    int32_t             m_inStreamOrder;
    int32_t             m_outDumpOrder;

    int32_t             m_inTileXLeft;
    int32_t             m_inTileXRight;
    int32_t             m_inTileYTop;
    int32_t             m_inTileYBottom;

    int32_t             m_outTileXLeft;
    int32_t             m_outTileXRight;
    int32_t             m_outTileYTop;
    int32_t             m_outTileYBottom;

    // Luma&Chromal crop information
    int32_t             m_lumaXOffset;
    int32_t             m_lumaXSubpixel;
    int32_t             m_lumaYOffset;
    int32_t             m_lumaYSubpixel;
    int32_t             m_chromaXOffset;
    int32_t             m_chromaXSubpixel;
    int32_t             m_chromaYOffset;
    int32_t             m_chromaYSubpixel;

    int32_t             m_outVertical;      // Output vertical tile index
    int32_t             m_outHorizontal;    // Output horizontal tile index
    int32_t             m_inTileAccumulation;
    int32_t             m_outTileAccumulation;

    struct TILE_FUNC_BLOCK_STRUCT   *m_pFunc;
    struct TILE_FUNC_DATA_STRUCT    *m_pData;

    virtual DpEngineType onGetParentType()
    {
        return getPrevType();
    }

    /**
     * Function for ISP direct link
     */
    virtual DP_STATUS_ENUM onInitTileCalc(struct TILE_PARAM_STRUCT* p_tile_param);

    virtual DP_STATUS_ENUM onRetrieveFrameParam(struct TILE_PARAM_STRUCT*, DpCommand&)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onRetrieveTileParam(struct TILE_PARAM_STRUCT* p_tile_param);

    virtual DP_STATUS_ENUM onConfigTile(DpCommand&)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onReconfigTiles(DpCommand&)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onUpdatePMQOS(DpCommand&, uint32_t& totalPixel, uint32_t& totalDataSzie, int32_t& engineType, struct timeval& time)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM addPmqosRequest(uint32_t& bandwidth, uint32_t& port, struct timeval& time)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onPostProc(DpCommand&)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onAdvanceTile(DpCommand&)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onCommandEnd(DpCommand&)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM onSetTileOrder(uint32_t inCalOrder,
                                  uint32_t outCalOrder,
                                  uint32_t inStreamOrder,
                                  uint32_t outDumpOrder)
    {
        m_inCalOrder = inCalOrder;
        m_outCalOrder = outCalOrder;
        m_inStreamOrder = inStreamOrder;
        m_outDumpOrder = outDumpOrder;
        return DP_STATUS_RETURN_SUCCESS;
    }
};

#endif  // __DP_TILE_ENGINE_H__
