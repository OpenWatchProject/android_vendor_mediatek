#include "DpTileEngine.h"

#include "tile_driver.h"
#include "tile_param.h"

DP_STATUS_ENUM DpTileEngine::onInitTileCalc(struct TILE_PARAM_STRUCT* p_tile_param)
{
    uint32_t                index;

    /* tile core property */
    FUNC_DESCRIPTION_STRUCT *ptr_tile_func_param = p_tile_param->ptr_tile_func_param;
    TILE_FUNC_BLOCK_STRUCT  *ptr_func;

    m_pFunc = NULL;
    for (index = 0; index < ptr_tile_func_param->used_func_no; index++)
    {
        ptr_func = &ptr_tile_func_param->func_list[index];
        if (TILE_FUNC_MDP_BASE + getEngineType() == ptr_func->func_num)
        {
            m_pFunc = ptr_func;
            break;
        }
    }
    if (NULL == m_pFunc)
    {
        DPLOGE("DpTileEngine: %s initialize tile calculation failed\n", getEngineName());
        assert(0);
        return DP_STATUS_INVALID_ENGINE;
    }

#if CONFIG_FOR_VERIFY_FPGA
    assert(1 == m_pFunc->tot_prev_num);
    assert(getNextCount() == m_pFunc->tot_branch_num);
    assert(TILE_FUNC_MDP_BASE + getPrevType() == m_pFunc->last_func_num[0] ||
           TILE_FUNC_MDP_CROP_ID == m_pFunc->last_func_num[0] ||    // CAMIN
           LAST_MODULE_ID_OF_START == m_pFunc->last_func_num[0]);   // RDMA
    assert(false == m_tileDisable); // on init frame mode
#endif // CONFIG_FOR_VERIFY_FPGA

    m_pFunc->func_data = m_pData;
    /* init tile core TILE_FUNC_BLOCK_STRUCT */
    //m_pFunc->output_disable_flag = m_outputDisable;
    m_pFunc->enable_flag = m_bypassEngine ? false : true;
    m_pFunc->full_size_x_in = m_inFrameWidth;
    m_pFunc->full_size_y_in = m_inFrameHeight;
    m_pFunc->full_size_x_out = m_outFrameWidth;
    m_pFunc->full_size_y_out = m_outFrameHeight;
    m_pFunc->in_cal_order = m_inCalOrder;
    m_pFunc->out_cal_order = m_outCalOrder;
    m_pFunc->in_stream_order = m_inStreamOrder;
    m_pFunc->out_stream_order = m_inStreamOrder;
    m_pFunc->in_dump_order = m_outDumpOrder;
    m_pFunc->out_dump_order = m_outDumpOrder;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpTileEngine::onRetrieveTileParam(struct TILE_PARAM_STRUCT*)
{
    assert(m_pFunc != NULL);
    assert(TILE_FUNC_MDP_BASE + getEngineType() == m_pFunc->func_num);

    m_outputDisable = m_pFunc->output_disable_flag;
    if (true == m_outputDisable)
    {
        DPLOGI("DpTileEngine: %s output disabled\n", getEngineName());
        return DP_STATUS_RETURN_SUCCESS;
    }

    m_tileDisable = (m_pFunc->tdr_h_disable_flag || m_pFunc->tdr_v_disable_flag);
    if (true == m_tileDisable)
    {
        DPLOGI("DpTileEngine: %s tile disabled\n", getEngineName());
        return DP_STATUS_RETURN_SUCCESS;
    }

    /* retrieve tile core TILE_FUNC_BLOCK_STRUCT */
    m_bypassEngine = m_pFunc->enable_flag ? false : true;
    m_inTileXLeft = m_pFunc->in_pos_xs;
    m_inTileXRight = m_pFunc->in_pos_xe;
    m_inTileYTop = m_pFunc->in_pos_ys;
    m_inTileYBottom = m_pFunc->in_pos_ye;
    m_outTileXLeft = m_pFunc->out_pos_xs;
    m_outTileXRight = m_pFunc->out_pos_xe;
    m_outTileYTop = m_pFunc->out_pos_ys;
    m_outTileYBottom = m_pFunc->out_pos_ye;
    /* copy offset */
    m_lumaXOffset = m_pFunc->bias_x;
    m_lumaXSubpixel = m_pFunc->offset_x;
    m_lumaYOffset = m_pFunc->bias_y;
    m_lumaYSubpixel = m_pFunc->offset_y;
    m_chromaXOffset = m_pFunc->bias_x_c;
    m_chromaXSubpixel = m_pFunc->offset_x_c;
    m_chromaYOffset = m_pFunc->bias_y_c;
    m_chromaYSubpixel = m_pFunc->offset_y_c;

    m_inTileAccumulation += (m_inTileXRight - m_inTileXLeft + 1);
    m_outTileAccumulation += (m_outTileXRight - m_outTileXLeft + 1);

#if CONFIG_FOR_VERIFY_FPGA
    /* retrieve tile core TILE_FUNC_BLOCK_STRUCT */
    assert(m_inFrameWidth == m_pFunc->full_size_x_in);
    assert(m_inFrameHeight == m_pFunc->full_size_y_in);
    assert(m_outFrameWidth == m_pFunc->full_size_x_out);
    assert(m_outFrameHeight == m_pFunc->full_size_y_out);
    assert(m_inCalOrder == m_pFunc->in_cal_order);
    assert(m_outCalOrder == m_pFunc->out_cal_order);
    assert(m_inStreamOrder == m_pFunc->in_stream_order);
    assert(m_inStreamOrder == m_pFunc->out_stream_order);
    assert(m_outDumpOrder == m_pFunc->in_dump_order);
    assert(m_outDumpOrder == m_pFunc->out_dump_order);
#endif // CONFIG_FOR_VERIFY_FPGA

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpTileEngine::advanceTile(DpCommand& command, struct TILE_PARAM_STRUCT* p_tile_param)
{
    /* tile core property */
    TILE_REG_MAP_STRUCT *ptr_tile_reg_map = p_tile_param->ptr_tile_reg_map;

    /* retrieve tile core TILE_REG_MAP_STRUCT */
    m_outVertical = ptr_tile_reg_map->curr_vertical_tile_no;
    m_outHorizontal = ptr_tile_reg_map->curr_horizontal_tile_no;

    return onAdvanceTile(command);
}


DP_STATUS_ENUM DpTileEngine::dumpTileAccumulation()
{
    if (queryFeature() & eRMEM)
    {
        DPLOGD("DpTileEngine: Engine %d input width is %d and total tile width is %d\n",
            getEngineType(),
            m_inFrameWidth,
            m_inTileAccumulation);
    }
    else if (queryFeature() & eWMEM)
    {
        DPLOGD("DpTileEngine: Engine %d output width is %d and total tile width is %d\n",
            getEngineType(),
            m_outFrameWidth,
            m_outTileAccumulation);
    }
    else
    {
        DPLOGD("DpTileEngine: other case\n");
    }

    return DP_STATUS_RETURN_SUCCESS;
}

