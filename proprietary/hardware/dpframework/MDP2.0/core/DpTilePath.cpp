#include "DpTilePath.h"
#include "DpWrapper_TILE.h"
#ifdef WPE_TILE
#include "wpe_cal.h"
#endif //WPE_TILE

#define DUMP_TILE_ACC 0

static DP_STATUS_ENUM tile_message_to_dp_status(ISP_MESSAGE_ENUM result)
{
    switch (result)
    {
    case ISP_MESSAGE_OVER_MAX_TILE_FUNC_NO_ERROR:
    case ISP_MESSAGE_OVER_MAX_TILE_FUNC_EN_NO_ERROR:
    case ISP_MESSAGE_OVER_MAX_TILE_FUNC_PREV_NO_ERROR:
        return DP_STATUS_OVER_MAX_ENGINE;
    case ISP_MESSAGE_OVER_MAX_BRANCH_NO_ERROR:
        return DP_STATUS_OVER_MAX_BRANCH;
    case ISP_MESSAGE_TILE_FUNC_CANNOT_FIND_LAST_FUNC_ERROR:
        return DP_STATUS_INVALID_PATH;
    case ISP_MESSAGE_SCHEDULING_BACKWARD_ERROR:
    case ISP_MESSAGE_SCHEDULING_FORWARD_ERROR:
        return DP_STATUS_SCHEDULE_ERROR;
    case ISP_MESSAGE_IN_CONST_X_ERROR:
    case ISP_MESSAGE_OUT_CONST_X_ERROR:
        return DP_STATUS_INVALID_X_ALIGN;
    case ISP_MESSAGE_IN_CONST_Y_ERROR:
    case ISP_MESSAGE_OUT_CONST_Y_ERROR:
        return DP_STATUS_INVALID_Y_ALIGN;
    case ISP_MESSAGE_INIT_INCORRECT_X_INPUT_SIZE_POS_ERROR:
        return DP_STATUS_INVALID_X_INPUT;
    case ISP_MESSAGE_INIT_INCORRECT_Y_INPUT_SIZE_POS_ERROR:
        return DP_STATUS_INVALID_Y_INPUT;
    case ISP_MESSAGE_INIT_INCORRECT_X_OUTPUT_SIZE_POS_ERROR:
        return DP_STATUS_INVALID_X_OUTPUT;
    case ISP_MESSAGE_INIT_INCORRECT_Y_OUTPUT_SIZE_POS_ERROR:
        return DP_STATUS_INVALID_Y_OUTPUT;
    case ISP_MESSAGE_TILE_LOSS_OVER_TILE_HEIGHT_ERROR:
        return DP_STATUS_LOSS_OVER_HEIGHT;
    case ISP_MESSAGE_TILE_LOSS_OVER_TILE_WIDTH_ERROR:
        return DP_STATUS_LOSS_OVER_WIDTH;
    case ISP_MESSAGE_TILE_OUTPUT_HORIZONTAL_OVERLAP_ERROR:
        return DP_STATUS_X_OUT_OVERLAP;
    case ISP_MESSAGE_TILE_OUTPUT_VERTICAL_OVERLAP_ERROR:
        return DP_STATUS_Y_OUT_OVERLAP;
    //case ISP_MESSAGE_TP8_FOR_INVALID_OUT_XYS_XYE_ERROR:
    //case ISP_MESSAGE_TP6_FOR_INVALID_OUT_XYS_XYE_ERROR:
    //case ISP_MESSAGE_TP4_FOR_INVALID_OUT_XYS_XYE_ERROR:
    //case ISP_MESSAGE_TP2_FOR_INVALID_OUT_XYS_XYE_ERROR:
    //case ISP_MESSAGE_SRC_ACC_FOR_INVALID_OUT_XYS_XYE_ERROR:
    //case ISP_MESSAGE_CUB_ACC_FOR_INVALID_OUT_XYS_XYE_ERROR:
    //case ISP_MESSAGE_RESIZER_SRC_ACC_SCALING_UP_ERROR:
    //case ISP_MESSAGE_RESIZER_CUBIC_ACC_SCALING_UP_ERROR:
    //    return DP_STATUS_RESIZER_ERROR;
    case ISP_MESSAGE_VERIFY_BACKWARD_XS_LESS_THAN_FORWARD_ERROR:
    case ISP_MESSAGE_VERIFY_FORWARD_XE_LESS_THAN_BACKWARD_ERROR:
        return DP_STATUS_X_LESS_THAN_LAST;
    case ISP_MESSAGE_VERIFY_BACKWARD_YS_LESS_THAN_FORWARD_ERROR:
    case ISP_MESSAGE_VERIFY_FORWARD_YE_LESS_THAN_BACKWARD_ERROR:
        return DP_STATUS_Y_LESS_THAN_LAST;
    case ISP_MESSAGE_CHECK_IN_CONFIG_ALIGN_XS_POS_ERROR:
    case ISP_MESSAGE_CHECK_IN_CONFIG_ALIGN_XE_POS_ERROR:
    case ISP_MESSAGE_CHECK_OUT_CONFIG_ALIGN_XS_POS_ERROR:
    case ISP_MESSAGE_CHECK_OUT_CONFIG_ALIGN_XE_POS_ERROR:
    case ISP_MESSAGE_XSIZE_NOT_DIV_BY_IN_CONST_X_ERROR:
    case ISP_MESSAGE_XSIZE_NOT_DIV_BY_OUT_CONST_X_ERROR:
        return DP_STATUS_X_ALIGN_ERROR;
    case ISP_MESSAGE_CHECK_IN_CONFIG_ALIGN_YS_POS_ERROR:
    case ISP_MESSAGE_CHECK_IN_CONFIG_ALIGN_YE_POS_ERROR:
    case ISP_MESSAGE_CHECK_OUT_CONFIG_ALIGN_YS_POS_ERROR:
    case ISP_MESSAGE_CHECK_OUT_CONFIG_ALIGN_YE_POS_ERROR:
    case ISP_MESSAGE_YSIZE_NOT_DIV_BY_IN_CONST_Y_ERROR:
    case ISP_MESSAGE_YSIZE_NOT_DIV_BY_OUT_CONST_Y_ERROR:
        return DP_STATUS_Y_ALIGN_ERROR;
    case ISP_MESSAGE_TILE_FORWARD_OUT_OVER_TILE_WIDTH_ERROR:
    case ISP_MESSAGE_TILE_BACKWARD_IN_OVER_TILE_WIDTH_ERROR:
        return DP_STATUS_OVER_MAX_WIDTH;
    case ISP_MESSAGE_TILE_FORWARD_OUT_OVER_TILE_HEIGHT_ERROR:
    case ISP_MESSAGE_TILE_BACKWARD_IN_OVER_TILE_HEIGHT_ERROR:
        return DP_STATUS_OVER_MAX_HEIGHT;
    case ISP_MESSAGE_FORWARD_CHECK_TOP_EDGE_ERROR:
    case ISP_MESSAGE_BACKWARD_CHECK_TOP_EDGE_ERROR:
        return DP_STATUS_TOP_EDGE_ERROR;
    case ISP_MESSAGE_FORWARD_CHECK_BOTTOM_EDGE_ERROR:
    case ISP_MESSAGE_BACKWARD_CHECK_BOTTOM_EDGE_ERROR:
        return DP_STATUS_BOTTOM_EDGE_ERROR;
    case ISP_MESSAGE_FORWARD_CHECK_LEFT_EDGE_ERROR:
    case ISP_MESSAGE_BACKWARD_CHECK_LEFT_EDGE_ERROR:
        return DP_STATUS_LEFT_EDGE_ERROR;
    case ISP_MESSAGE_FORWARD_CHECK_RIGHT_EDGE_ERROR:
    case ISP_MESSAGE_BACKWARD_CHECK_RIGHT_EDGE_ERROR:
        return DP_STATUS_RIGHT_EDGE_ERROR;
    case ISP_MESSAGE_DISABLE_FUNC_X_SIZE_CHECK_ERROR:
        return DP_STATUS_INVALID_WIDTH;
    case ISP_MESSAGE_DISABLE_FUNC_Y_SIZE_CHECK_ERROR:
        return DP_STATUS_INVALID_HEIGHT;
    case ISP_MESSAGE_DEBUG_PRINT_FILE_OPEN_ERROR:
        return DP_STATUS_INVALID_FILE;

    case MDP_MESSAGE_BUFFER_EMPTY:
        return DP_STATUS_BUFFER_EMPTY;
    case MDP_MESSAGE_RDMA_NULL_DATA:
    //case MDP_MESSAGE_AAL_NULL_DATA:
    case MDP_MESSAGE_PRZ_NULL_DATA:
    case MDP_MESSAGE_WROT_NULL_DATA:
    case MDP_MESSAGE_WDMA_NULL_DATA:
    case MDP_MESSAGE_JPEGENC_NULL_DATA:
        return DP_STATUS_INVALID_ENGINE;
    //case MDP_MESSAGE_RESIZER_SCALING_ERROR:
    //    return DP_STATUS_RESIZER_ERROR;
    case MDP_MESSAGE_TDSHP_BACK_LT_FORWARD:
        return DP_STATUS_BACK_LE_FORWARD;
    case MDP_MESSAGE_WROT_INVALID_FORMAT:
        return DP_STATUS_INVALID_FORMAT;
    case MDP_MESSAGE_INVALID_STATE:
        return DP_STATUS_INVALID_STATE;

    case ISP_MESSAGE_OK:
        return DP_STATUS_RETURN_SUCCESS;
    //case MDP_MESSAGE_UNKNOWN_ERROR:
    default:
        return DP_STATUS_UNKNOWN_ERROR;
    }
}


template <typename Engine, typename Thread>
DP_STATUS_ENUM DpPath<Engine, Thread>::dumpTileDebugInfo(ISP_MESSAGE_ENUM result)
{
    int32_t index;
    FUNC_DESCRIPTION_STRUCT *ptr_tile_func_param = m_tileParam.ptr_tile_func_param;

    if (ptr_tile_func_param &&
        MDP_MESSAGE_BUFFER_EMPTY != result)
    {
        DPLOGE("DpTilePath: dump tile func blocks\n");
        DPLOGE("func_num: full_in     , full_out    "
            ", in_pos  (xs, xe, ys, ye), out_pos (xs, xe, ys, ye)"
            ", back_in (xs, xe, ys, ye), back_out(xs, xe, ys, ye)"
            ", edge, disable\n");
        for (index = 0; index < ptr_tile_func_param->used_func_no; index++)
        {
            TILE_FUNC_BLOCK_STRUCT *ptr_func = &ptr_tile_func_param->func_list[index];

            DPLOGE(" [%5d]: (%4d, %4d), (%4d, %4d)"
                ", (%4d, %4d, %4d, %4d), (%4d, %4d, %4d, %4d)"
                ", (%4d, %4d, %4d, %4d), (%4d, %4d, %4d, %4d)"
                ", 0x%02X,  (%1d, %1d)\n",
                ptr_func->func_num,
                ptr_func->full_size_x_in, ptr_func->full_size_y_in,
                ptr_func->full_size_x_out, ptr_func->full_size_y_out,
                ptr_func->in_pos_xs, ptr_func->in_pos_xe,
                ptr_func->in_pos_ys, ptr_func->in_pos_ye,
                ptr_func->out_pos_xs, ptr_func->out_pos_xe,
                ptr_func->out_pos_ys, ptr_func->out_pos_ye,
                ptr_func->backward_input_xs_pos, ptr_func->backward_input_xe_pos,
                ptr_func->backward_input_ys_pos, ptr_func->backward_input_ye_pos,
                ptr_func->backward_output_xs_pos, ptr_func->backward_output_xe_pos,
                ptr_func->backward_output_ys_pos, ptr_func->backward_output_ye_pos,
                ptr_func->tdr_edge,
                ptr_func->tdr_h_disable_flag, ptr_func->tdr_v_disable_flag);
        }
    }

    return tile_message_to_dp_status(result);
}


template <typename Engine, typename Thread>
DP_STATUS_ENUM DpPath<Engine, Thread>::calcFrameInfo(DpCommand &command)
{
    DP_STATUS_ENUM status;
    int32_t        index;
    Engine         *pCurr;

    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    bool             stop = false;

#if !CONFIG_FOR_VERIFY_FPGA
    if ((DpStream::CONFIG_FRAME_ONLY | DpStream::CONFIG_ISP_FRAME_MODE) & m_pStream->getConfigFlags())
    {
        m_tileParam.ptr_tile_reg_map = NULL;
        m_tileParam.ptr_tile_func_param = NULL;

        DPLOGI("DpTilePath: bypass calcFrameInfo\n");
        return DP_STATUS_RETURN_SUCCESS;
    }
#endif //!CONFIG_FOR_VERIFY_FPGA

    memset(&m_tileParam, 0, sizeof(m_tileParam));
    memset(&m_tileRegMap, 0, sizeof(m_tileRegMap));
    memset(&m_tileFunc, 0, sizeof(m_tileFunc));

    m_tileParam.ptr_tile_reg_map = &m_tileRegMap;
    m_tileParam.ptr_tile_func_param = &m_tileFunc;

    m_tileRegMap.LAST_IRQ = 1;

#if !CONFIG_FOR_VERIFY_FPGA
#ifdef WPE_TILE
    if (DpStream::CONFIG_WPE_ONLY_TILE_MODE & m_pStream->getConfigFlags())
    {
        memset(&m_wpeParam, 0, sizeof(m_wpeParam));
        status = m_connection.initTilePath(&m_tileParam, &m_wpeParam);
        DPLOGI("DpTilePath: bypass calcFrameInfo when wpe only tile mode with wpe_cal_main\n");
        return DP_STATUS_RETURN_SUCCESS;
    }
#endif //WPE_TILE
#endif //!CONFIG_FOR_VERIFY_FPGA

#ifdef WPE_TILE
    status = m_connection.initTilePath(&m_tileParam, &m_wpeParam);
#else
    status = m_connection.initTilePath(&m_tileParam);
#endif //WPE_TILE
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

#if CONFIG_FOR_VERIFY_FPGA
    if (m_tileParam.ptr_isp_tile_descriptor)
    {
        m_pTileInfoFile = fopen("tile_cal_dump_frame.txt", "wb");
        fprintf(m_pTileInfoFile, "/* n#,   fn#, run_mode, tdr_group, tile_edge,");
        fprintf(m_pTileInfoFile, "in_xstr, in_xend, out_xstr, out_xend, ");
        fprintf(m_pTileInfoFile, "in_ystr, in_yend, out_ystr, out_yend, ");
        fprintf(m_pTileInfoFile, "max_h_end, h_end, max_v_end, v_end, ");
        fprintf(m_pTileInfoFile, "offx, offy, subpix_x, subpix_y, ");
        fprintf(m_pTileInfoFile, "in_str, in_cal, in_tdr, out_str, out_cal, out_tdr, ");
        fprintf(m_pTileInfoFile, "tdr_h_disable, tdr_v_disable, ");
        fprintf(m_pTileInfoFile, "valid_h_no, valid_v_no, ");
        fprintf(m_pTileInfoFile, "min_in_xstr, min_in_xend, min_out_xstr, min_out_xend, ");
        fprintf(m_pTileInfoFile, "min_in_ystr, min_in_yend, min_out_ystr, min_out_yend, ");
        fprintf(m_pTileInfoFile, "min_tdr_h_disable, min_h_end, min_max_h_end, ");
        fprintf(m_pTileInfoFile, "min_tdr_v_disable, min_v_end, min_max_v_end, ");
        fprintf(m_pTileInfoFile, "min_in_xstr, max_in_xend, min_out_xstr, max_out_xend, ");
        fprintf(m_pTileInfoFile, "min_in_ystr, max_in_yend, min_out_ystr, max_out_yend, ");
        fprintf(m_pTileInfoFile, "back_tdr_h_disable, back_h_end, back_tdr_v_disable_flag, back_v_end, ");
        fprintf(m_pTileInfoFile, "max_h_edge_xs, max_h_edge_xe, max_v_edge_ye */\n");
    }
#endif

    result = wrapper_tile_cal_main_dp(TILE_CAL_MAIN_DP_INIT_FUNC_PROP, &m_tileParam);
    if (ISP_MESSAGE_OK != result)
    {
        return dumpTileDebugInfo(result);
    }

    // Setup tile calculation
    for (index = (m_engineCount - 1); index >= 0; index--)
    {
        pCurr = m_pEngineBlock[index];
        assert(NULL != pCurr);

        status = pCurr->initTileCalc(&m_tileParam);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpTilePath: init tile calcuation error %d\n", status);
            return status;
        }
    }

    result = wrapper_tile_cal_main_dp(TILE_CAL_MAIN_DP_INIT_FRAME_MODE, &m_tileParam);
    if (ISP_MESSAGE_OK != result)
    {
        return dumpTileDebugInfo(result);
    }

    result = wrapper_tile_proc_main_single(&m_tileParam, 0, &stop, m_pTileInfoFile);
    assert(true == stop);
    if (ISP_MESSAGE_OK != result)
    {
        return dumpTileDebugInfo(result);
    }

    // Retrieve frame information
    for (index = 0; index < m_engineCount; index++)
    {
        pCurr = m_pEngineBlock[index];
        assert(NULL != pCurr);

        status = pCurr->retrieveFrameParam(&m_tileParam, command);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpTilePath: retrieve frame error %d\n", status);
            m_lastStatus = status;
            return status;
        }
    }

    result = wrapper_tile_cal_main_dp(TILE_CAL_MAIN_DP_CLOSE_FRAME_MODE, &m_tileParam);
    if (ISP_MESSAGE_OK != result)
    {
        return dumpTileDebugInfo(result);
    }

#if CONFIG_FOR_VERIFY_FPGA
    if (m_tileParam.ptr_isp_tile_descriptor)
    {
        result = wrapper_tile_fprint_reg_map(&m_tileRegMap);
        fclose(m_pTileInfoFile);
        if (ISP_MESSAGE_OK != result)
        {
            return dumpTileDebugInfo(result);
        }

        result = wrapper_tile_main_read_isp_reg_file(&m_tileRegMap);
        if (ISP_MESSAGE_OK != result)
        {
            return dumpTileDebugInfo(result);
        }

        m_tileRegMap.last_irq_mode = 2; // 0 FOR ISP ONLY,2 FOR MDP directtlink
        m_tileRegMap.isp_tile_width   = 768;
        m_tileRegMap.isp_tile_height  = MAX_SIZE;

        m_tileRegMap.tdr_ctrl_en = true;
        m_tileRegMap.skip_tile_mode = false;
        m_tileRegMap.run_c_model_direct_link = 1;

        m_tileRegMap.max_input_width  = MAX_SIZE;  /* can modify to test */
        m_tileRegMap.max_input_height = MAX_SIZE;  /* can modify to test */

        //m_tileRegMap.src_dump_order = m_outDumpOrder; //Normal direction
        //m_tileRegMap.src_stream_order = m_inStreamOrder; //Normal direction
        //m_tileRegMap.src_cal_order = m_inCalOrder; //Normal direction

        m_pTileInfoFile = fopen("tile_cal_dump.txt", "wb");
        fprintf(m_pTileInfoFile, "/* n#,   fn#, run_mode, tdr_group, tile_edge,");
        fprintf(m_pTileInfoFile, "in_xstr, in_xend, out_xstr, out_xend, ");
        fprintf(m_pTileInfoFile, "in_ystr, in_yend, out_ystr, out_yend, ");
        fprintf(m_pTileInfoFile, "max_h_end, h_end, max_v_end, v_end, ");
        fprintf(m_pTileInfoFile, "offx, offy, subpix_x, subpix_y, ");
        fprintf(m_pTileInfoFile, "in_str, in_cal, in_tdr, out_str, out_cal, out_tdr, ");
        fprintf(m_pTileInfoFile, "tdr_h_disable, tdr_v_disable, ");
        fprintf(m_pTileInfoFile, "valid_h_no, valid_v_no, ");
        fprintf(m_pTileInfoFile, "min_in_xstr, min_in_xend, min_out_xstr, min_out_xend, ");
        fprintf(m_pTileInfoFile, "min_in_ystr, min_in_yend, min_out_ystr, min_out_yend, ");
        fprintf(m_pTileInfoFile, "min_tdr_h_disable, min_h_end, min_max_h_end, ");
        fprintf(m_pTileInfoFile, "min_tdr_v_disable, min_v_end, min_max_v_end, ");
        fprintf(m_pTileInfoFile, "min_in_xstr, max_in_xend, min_out_xstr, max_out_xend, ");
        fprintf(m_pTileInfoFile, "min_in_ystr, max_in_yend, min_out_ystr, max_out_yend, ");
        fprintf(m_pTileInfoFile, "back_tdr_h_disable, back_h_end, back_tdr_v_disable_flag, back_v_end, ");
        fprintf(m_pTileInfoFile, "max_h_edge_xs, max_h_edge_xe, max_v_edge_ye */\n");
    }
#endif

    result = wrapper_tile_cal_main_dp(TILE_CAL_MAIN_DP_INIT_TILE_MODE, &m_tileParam);
    if (ISP_MESSAGE_OK != result)
    {
        return dumpTileDebugInfo(result);
    }

    if (m_tileParam.ptr_isp_tile_descriptor &&
        m_tileParam.ptr_isp_tile_descriptor->tpipe_config)
    {
        memset(m_tileParam.ptr_isp_tile_descriptor->tpipe_config,
            0x0, sizeof(uint8_t) * MAX_ISP_TILE_TDR_HEX_NO);
    }

    m_tileID = 0;
    m_pendingTile = 0;

#if CONFIG_FOR_TPIPE_FINFO
    dumpTileDebugInfo(result);
#endif // CONFIG_FOR_TPIPE_FINFO
    return DP_STATUS_RETURN_SUCCESS;
}

template <typename Engine, typename Thread>
DP_STATUS_ENUM DpPath<Engine, Thread>::dumpTileAccumulation()
{
    Engine    *pEngine;
    int32_t index;

    for (index = 0; index < m_rootAndLeaf; index++)
    {
        pEngine = m_pRootAndLeaf[index];
        assert(NULL != pEngine);

        if (false == pEngine->isOutputDisable())
        {
            pEngine->dumpTileAccumulation();
        }
    }

    return DP_STATUS_RETURN_SUCCESS;
}


/*
 * Single thread version tile path
 */
template <>
DP_STATUS_ENUM DpTilePathST::config(int32_t count)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM  status, endStatus;
    PATH_STATE_ENUM state;
    DpConfig        config;

    memset(&config, 0x0, sizeof(DpConfig));
    config.rootAndLeaf = m_rootAndLeaf;

    status = getScenario(&config.scenario);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    state = getState();

    if ((eConfig != state) &&
        (eTile   != state))
    {
        DP_PROFILER_CONFIG_FRAME_BEGIN();
        status = configFrame(config);
        DP_PROFILER_CONFIG_FRAME_END();

        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    #ifdef CONFIG_FOR_SOURCE_PQ
    if (STREAM_COLOR_BITBLT == config.scenario)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }
    #endif

    DP_PROFILER_CONFIG_TILE_BEGIN();

    if ((DpStream::CONFIG_ISP_FRAME_MODE & m_pStream->getConfigFlags()) ||
        (DpStream::CONFIG_WPE_ONLY_TILE_MODE & m_pStream->getConfigFlags()))
    {
        status = configFrameMode();
    }
    else
    {
        while ((DP_STATUS_RETURN_SUCCESS == status) && (--count >= 0))
        {
            status = configTile();
        }
    }

    endStatus = configCommandEnd();
    if (DP_STATUS_RETURN_SUCCESS != endStatus)
    {
        return endStatus;
    }

    DP_PROFILER_CONFIG_TILE_END();

    if ((DP_STATUS_RETURN_SUCCESS != status) &&
        (DP_STATUS_ALL_TPIPE_DONE != status))
    {
        return status;
    }

#if DUMP_TILE_ACC
    if (DP_STATUS_ALL_TPIPE_DONE == status)
    {
        dumpTileAccumulation();
    }
#endif

    return DP_STATUS_RETURN_SUCCESS;
}


template <>
DP_STATUS_ENUM DpTilePathST::onConfigFrame(DpCommand &command,
                                           DpConfig  &config)
{
    DP_STATUS_ENUM  status;
    PATH_STATE_ENUM state;
    int32_t         index;
    DpTileEngine    *pCurr;

    state = getState();

#if !CONFIG_FOR_VERIFY_FPGA
    if ((DpStream::CONFIG_FRAME_ONLY & m_pStream->getConfigFlags()) && !DpDriver::getInstance()->getDisableReduceConfig())
    {
        DPLOGI("DpTilePathST: onConfigFrame dup(FRAME)");
        command.dup(DpCommand::FRAME_COMMAND);

        for (index = 0; index < m_engineCount; index++)
        {
            pCurr = m_pEngineBlock[index];
            assert(NULL != pCurr);

            status = pCurr->reconfigFrame(command, config);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpTilePath: reconfig frame error %d\n", status);
                m_lastStatus = status;
                return status;
            }
        }

        testSetState(eStop, eConfig);
    }
    else if ((eInit == state) ||
             (eIdle == state))
#endif
    {
        DPLOGI("DpTilePath: config frame information begin\n");

        command.mark(DpCommand::FRAME_COMMAND);

        // Reset engine
        command.initRegDstNum();
        for (index = 0; index < m_engineCount; index++)
        {
            pCurr = m_pEngineBlock[index];
            assert(NULL != pCurr);

            status = pCurr->initEngine(command);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpTilePath: init engine error %d\n", status);
                m_lastStatus = status;
                return status;
            }
        }

        // Config frame mode
        for (index = 0; index < m_engineCount; index++)
        {
            pCurr = m_pEngineBlock[index];
            assert(NULL != pCurr);

            status = pCurr->configFrame(command, config);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpTilePath: config frame error %d\n", status);
                m_lastStatus = status;
                return status;
            }
        }

        #ifdef CONFIG_FOR_SOURCE_PQ
        if (STREAM_COLOR_BITBLT == config.scenario)
        {
            testSetState(eStop, eStart);
            return DP_STATUS_RETURN_SUCCESS;
        }
        #endif

        status = calcFrameInfo(command);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            m_lastStatus = status;
            return status;
        }

        testSetState(eStop, eConfig);

        DPLOGI("DpTilePath: config frame information end\n");
    }

    return DP_STATUS_RETURN_SUCCESS;
}


template <>
DP_STATUS_ENUM DpTilePathST::onConfigTile(DpCommand &command)
{
    DP_STATUS_ENUM  status;
    PATH_STATE_ENUM state;
    int32_t         index;
    DpTileEngine    *pCurr;

    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    int              wpe_result = 0;
    bool             stop = false;

    state = getState();

    if ((0 != m_currentTile) && (eConfig == state))
    {
        command.dup(DpCommand::FRAME_COMMAND);
    }

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

        m_pendingTile ++;
        testSetState(eStop, eStart);

        return m_lastStatus;
    }
    else if ((eConfig == state) ||
             (eTile   == state))
#endif
    {
        DPLOGI("DpTilePath: config tile information begin\n");

        command.mark(DpCommand::TILE_COMMAND);

        result = wrapper_tile_proc_main_single(&m_tileParam, m_tileID, &stop, m_pTileInfoFile);
        if (ISP_MESSAGE_OK != result)
        {
            status = dumpTileDebugInfo(result);
            m_lastStatus = status;
            return status;
        }

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
            DPLOGE("DpTilePathST: require mutex fail\n");
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

#if CONFIG_FOR_TPIPE_FINFO
        dumpTileDebugInfo(result);
#endif // CONFIG_FOR_TPIPE_FINFO

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

        // Advance to the next tile
        for (index = 0; index < m_engineCount; index++)
        {
            pCurr = m_pEngineBlock[index];
            assert(NULL != pCurr);

            status = pCurr->advanceTile(command, &m_tileParam);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpTilePath: advance tile error %d\n", status);

                // Disable mux settings
                m_connection.disablePath(command);

                m_lastStatus = status;
                return status;
            }
        }

        // Disable mux settings
        m_connection.disablePath(command);

        m_tileID ++;
        if (stop)
        {
            result = wrapper_tile_cal_main_dp(TILE_CAL_MAIN_DP_CLOSE_TILE_MODE, &m_tileParam);
            if (ISP_MESSAGE_OK != result)
            {
                status = dumpTileDebugInfo(result);
                m_lastStatus = status;
                return status;
            }
#if CONFIG_FOR_VERIFY_FPGA
            if (m_tileParam.ptr_isp_tile_descriptor)
            {
                result = wrapper_tile_fprint_reg_map(&m_tileRegMap);
                fclose(m_pTileInfoFile);
                if (ISP_MESSAGE_OK != result)
                {
                    return dumpTileDebugInfo(result);
                }
            }
#endif

            // Post processing information
            for (index = 0; index < m_engineCount; index++)
            {
                pCurr = m_pEngineBlock[index];
                assert(NULL != pCurr);

                status = pCurr->postProc(command);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    DPLOGE("DpTilePath: post-procedure error %d\n", status);
                    m_lastStatus = status;
                    return status;
                }
            }

#ifdef WPE_TILE
            STREAM_TYPE_ENUM curStreamType;
            getScenario(&curStreamType);
            WPE_WORKING_BUFFER_STRUCT wpeWorkingBuffer;

            if ((STREAM_WPE == curStreamType || STREAM_WPE2 == curStreamType))
            {
                //WPE DL
                wpe_result = wpe_cal_main(&m_wpeParam,
                                          m_tileParam.ptr_isp_tile_descriptor->used_tpipe_no,
                                          (char *)(&wpeWorkingBuffer),
                                          sizeof(wpeWorkingBuffer),
                                          (char *)(m_tileParam.ptr_isp_tile_descriptor->tpipe_config_wpe),
                                          m_tileParam.ptr_isp_tile_descriptor->used_word_no_wpe * 4,
                                          m_tileParam.ptr_isp_tile_descriptor->config_no_per_tpipe_wpe,
                                          (int *)(&(m_tileParam.ptr_isp_tile_descriptor->used_tpipe_no_wpe_s)),
                                          (char *)(m_tileParam.ptr_isp_tile_descriptor->tpipe_config_wpe_s),
                                          query_wpe_tdr_out_buffer_size(32),
                                          (int *)(&(m_tileParam.ptr_isp_tile_descriptor->config_no_per_tpipe_wpe_s)),
                                          NULL,
                                          0,
                                          NULL);
            }
            if (wpe_result)
            {
                DPLOGE("DpTilePathST::onConfigTile WPE Tile error\n");
            }
#endif //WPE_TILE

            m_lastStatus = DP_STATUS_ALL_TPIPE_DONE;

            m_pendingTile ++;
            testSetState(eStop, eStart);

            DPLOGI("DpTilePath: config tile information done %d\n", m_lastStatus);
            return m_lastStatus;
        }

        m_pendingTile ++;
        testSetState(eStop, eTile);

        DPLOGI("DpTilePath: config tile information end\n");
    }

    return DP_STATUS_RETURN_SUCCESS;
}


template <>
DP_STATUS_ENUM DpTilePathST::threadLoop()
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM  status;
    PATH_STATE_ENUM state;

    do
    {
        // Reset engineState and lastStatus
        m_lastStatus = DP_STATUS_RETURN_SUCCESS;

        if (0 == (DpStream::CONFIG_BUFFER_IGNORE & m_pStream->getConfigFlags()))
        {
            status = pollPort(false);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                m_lastStatus = status;
                break;
            }
        }

        status = config();
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            m_lastStatus = status;
            break;
        }

        state = getState();

        if (eStart == state)
        {
            if (0 == (DpStream::CONFIG_BUFFER_IGNORE & m_pStream->getConfigFlags()))
            {
                status = waitFence();
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    // When fence timeout, keep job process instead of system hang.
                    // ex: Use GPU to draw game and work load too heavy
                    // Todo: implement job error handle flow
                }
            }

            status = execCommand();
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                m_lastStatus = status;
                break;
            }
        }
        else
        {
            m_lastStatus = DP_STATUS_INVALID_STATE;
            break;
        }
    } while (0);

    if (m_lastStatus < 0)
    {
        testSetState(eStop, eStop);

        dumpDebugInfo();
        flipBuffer();
        return m_lastStatus;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


/*
 * Multi thread version tile path
 */
template <>
DP_STATUS_ENUM DpTilePathMT::config(int32_t count)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM  status, endStatus;
    PATH_STATE_ENUM state;
    DpConfig        config;

    memset(&config, 0x0, sizeof(DpConfig));
    config.rootAndLeaf = m_rootAndLeaf;

    status = getScenario(&config.scenario);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    state = getState();

    if ((eConfig != state) &&
        (eTile   != state))
    {
        if (STREAM_DUAL_BITBLT == config.scenario)
        {
            getPipeID(&config.pipeID);
            DPLOGI("DpPathConfig: getpipeID: %d\n", config.pipeID);
        }

        DP_PROFILER_CONFIG_FRAME_BEGIN();
        getTileOrder(config.inCalOrder, config.outCalOrder, config.inStreamOrder, config.outDumpOrder);
        status = configFrame(config);
        DP_PROFILER_CONFIG_FRAME_END();

        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if ((STREAM_ISP_VSS == config.scenario) || (STREAM_ISP_ZSD_SLOW == config.scenario))
    {
        count = 3;
        //if (STREAM_ISP_ZSD_ONE == config.scenario)
        //{
        //    count = 1;
        //}
        DPLOGI("DpTilePathMT: got VSS type to pass2B\n");
    }

    DP_PROFILER_CONFIG_TILE_BEGIN();

    if ((DpStream::CONFIG_ISP_FRAME_MODE & m_pStream->getConfigFlags()) ||
        (DpStream::CONFIG_WPE_ONLY_TILE_MODE & m_pStream->getConfigFlags()))
    {
        status = configFrameMode();
    }
    else
    {
        while ((DP_STATUS_RETURN_SUCCESS == status) && (--count >= 0))
        {
            status = configTile();
        }
    }

    endStatus = configCommandEnd();
    if (DP_STATUS_RETURN_SUCCESS != endStatus)
    {
        return endStatus;
    }

    DP_PROFILER_CONFIG_TILE_END();

    if ((DP_STATUS_RETURN_SUCCESS != status) &&
        (DP_STATUS_BUFFER_FULL    != status) &&
        (DP_STATUS_BUFFER_EMPTY   != status) &&
        (DP_STATUS_ALL_TPIPE_DONE != status))
    {
        // If the buffer is not ready, we can wait in thread loop.
        return status;
    }

#if DUMP_TILE_ACC
    if (DP_STATUS_ALL_TPIPE_DONE == status)
    {
        dumpTileAccumulation();
    }
#endif

    return DP_STATUS_RETURN_SUCCESS;
}


template <>
DP_STATUS_ENUM DpTilePathMT::onConfigFrame(DpCommand &command,
                                           DpConfig  &config)
{
    DP_STATUS_ENUM  status;
    PATH_STATE_ENUM state;
    int32_t         index;
    DpTileEngine    *pCurr;

    state = getState();

    command.setDumpOrder(config.outDumpOrder);

    if (eStop == state)
    {
        // Don't need to config frame and tile
        m_lastStatus = DP_STATUS_INVALID_STATE;
        return DP_STATUS_INVALID_STATE;
    }

#if !CONFIG_FOR_VERIFY_FPGA
    if ((DpStream::CONFIG_FRAME_ONLY & m_pStream->getConfigFlags()) && !DpDriver::getInstance()->getDisableReduceConfig())
    {
        DPLOGI("DpTilePathMT: onConfigFrame dup(FRAME)");
        command.dup(DpCommand::FRAME_COMMAND);

        for (index = 0; index < m_engineCount; index++)
        {
            pCurr = m_pEngineBlock[index];
            assert(NULL != pCurr);

            status = pCurr->reconfigFrame(command, config);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpTilePath: reconfig frame error %d\n", status);
                m_lastStatus = status;
                return status;
            }
        }

        testSetState(eStop, eConfig);
    }
    else if ((eInit == state) ||
             (eIdle == state))
#endif
    {
        DPLOGI("DpTilePath: config frame information begin\n");

        command.mark(DpCommand::FRAME_COMMAND);

        // Reset engine
        command.initRegDstNum();
        for (index = 0; index < m_engineCount; index++)
        {
            pCurr = m_pEngineBlock[index];
            assert(NULL != pCurr);

            status = pCurr->initEngine(command);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpTilePath: init engine error %d\n", status);
                m_lastStatus = status;
                return status;
            }
        }

        // Config frame mode
        for (index = 0; index < m_engineCount; index++)
        {
            pCurr = m_pEngineBlock[index];
            assert(NULL != pCurr);

            status = pCurr->configFrame(command, config);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpTilePath: config frame error %d\n", status);
                m_lastStatus = status;
                return status;
            }
        }

        status = calcFrameInfo(command);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            m_lastStatus = status;
            return status;
        }

        testSetState(eStop, eConfig);

        DPLOGI("DpTilePath: config frame information end\n");
    }

    return DP_STATUS_RETURN_SUCCESS;
}


template <>
DP_STATUS_ENUM DpTilePathMT::onConfigTile(DpCommand &command)
{
    DP_STATUS_ENUM  status;
    PATH_STATE_ENUM state;
    int32_t         index;
    DpTileEngine    *pCurr;

    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    int              wpe_result = 0;
    bool             stop = false;

    STREAM_TYPE_ENUM curStreamType;
    getScenario(&curStreamType);

    state = getState();

    if (eStop == state)
    {
        // Don't need to config frame and tile
        m_lastStatus = DP_STATUS_INVALID_STATE;
        return DP_STATUS_INVALID_STATE;
    }

    if ((0 != m_currentTile) && (eConfig == state))
    {
        command.dup(DpCommand::FRAME_COMMAND);
    }

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
    else if ((eConfig == state) ||
             (eTile   == state))
#endif
    {
        DPLOGI("DpTilePath: config tile information begin\n");

        command.mark(DpCommand::TILE_COMMAND);

        result = wrapper_tile_proc_main_single(&m_tileParam, m_tileID, &stop, m_pTileInfoFile);
        if (ISP_MESSAGE_OK != result)
        {
            status = dumpTileDebugInfo(result);
            m_lastStatus = status;
            return status;
        }
#ifdef WPE_TILE
            if ((STREAM_WPE == curStreamType || STREAM_WPE2 == curStreamType))
            {
                //WPE DL
                wpe_result = wpe_cal_main(&m_wpeParam,
                                          0,
                                          NULL,
                                          0,
                                          NULL,
                                          0,
                                          0,
                                          NULL,
                                          NULL,
                                          0,
                                          (int *)(&(m_tileParam.ptr_isp_tile_descriptor->config_no_per_tpipe_wpe_s)),
                                          NULL,
                                          0,
                                          NULL);
            }
            if (wpe_result)
            {
                DPLOGE("DpTilePathMT::onConfigTile WPE Tile error\n");
            }
#endif //WPE_TILE

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

        if (m_pEngineBlock[0])
        {
            uint32_t x, y;
            m_pEngineBlock[0]->getTileNumber(x, y);
            command.mark(DpCommand::TILE_COMMAND, x, y);
        }

        // Acquire engine mutex
        if (m_engineMutex.require(command) == false)
        {
            DPLOGE("DpTilePathMT: require mutex fail\n");
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

#if CONFIG_FOR_TPIPE_FINFO
        dumpTileDebugInfo(result);
#endif // CONFIG_FOR_TPIPE_FINFO

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

        // Advance to the next tile
        for (index = 0; index < m_engineCount; index++)
        {
            pCurr = m_pEngineBlock[index];
            assert(NULL != pCurr);

            status = pCurr->advanceTile(command, &m_tileParam);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpTilePath: advance tile error %d\n", status);

                // Disable mux settings
                m_connection.disablePath(command);

                m_lastStatus = status;
                return status;
            }
        }

        // Disable mux settings
        m_connection.disablePath(command);

        m_tileID ++;
        if (stop)
        {
            result = wrapper_tile_cal_main_dp(TILE_CAL_MAIN_DP_CLOSE_TILE_MODE, &m_tileParam);
            if (ISP_MESSAGE_OK != result)
            {
                status = dumpTileDebugInfo(result);
                m_lastStatus = status;
                return status;
            }
#if CONFIG_FOR_VERIFY_FPGA
            if (m_tileParam.ptr_isp_tile_descriptor)
            {
                result = wrapper_tile_fprint_reg_map(&m_tileRegMap);
                fclose(m_pTileInfoFile);
                if (ISP_MESSAGE_OK != result)
                {
                    return dumpTileDebugInfo(result);
                }
            }
#endif

            // Post processing information
            for (index = 0; index < m_engineCount; index++)
            {
                pCurr = m_pEngineBlock[index];
                assert(NULL != pCurr);

                status = pCurr->postProc(command);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    DPLOGE("DpTilePath: post-procedure error %d\n", status);
                    m_lastStatus = status;
                    return status;
                }
            }

#ifdef WPE_TILE
            WPE_WORKING_BUFFER_STRUCT wpeWorkingBuffer;

            if ((STREAM_WPE == curStreamType || STREAM_WPE2 == curStreamType))
            {
                //WPE DL
                wpe_result = wpe_cal_main(&m_wpeParam,
                                          m_tileParam.ptr_isp_tile_descriptor->used_tpipe_no,
                                          (char *)(&wpeWorkingBuffer),
                                          sizeof(wpeWorkingBuffer),
                                          (char *)(m_tileParam.ptr_isp_tile_descriptor->tpipe_config_wpe),
                                          m_tileParam.ptr_isp_tile_descriptor->used_word_no_wpe * 4,
                                          m_tileParam.ptr_isp_tile_descriptor->config_no_per_tpipe_wpe,
                                          (int *)(&(m_tileParam.ptr_isp_tile_descriptor->used_tpipe_no_wpe_s)),
                                          (char *)(m_tileParam.ptr_isp_tile_descriptor->tpipe_config_wpe_s),
                                          query_wpe_tdr_out_buffer_size(32),
                                          (int *)(&(m_tileParam.ptr_isp_tile_descriptor->config_no_per_tpipe_wpe_s)),
                                          NULL,
                                          0,
                                          NULL);
            }
            if (wpe_result)
            {
                DPLOGE("DpTilePathMT::onConfigTile WPE Tile error\n");
            }
#endif //WPE_TILE

            m_lastStatus = DP_STATUS_ALL_TPIPE_DONE;

            m_pendingTile++;
            testSetState(eStop, eStart);

            DPLOGI("DpTilePath: config tile information done %d\n", m_lastStatus);
            return m_lastStatus;
        }

        STREAM_TYPE_ENUM curStreamType;
        getScenario(&curStreamType);
        if (STREAM_FRAG_JPEGDEC == curStreamType)
        {
            // Do nothing
        }
        else
        {
            m_pendingTile++;
        }

        testSetState(eStop, eTile);

        DPLOGI("DpTilePath: config tile information end\n");
    }

    return DP_STATUS_RETURN_SUCCESS;
}


template <typename Engine, typename Thread>
DP_STATUS_ENUM DpPath<Engine, Thread>::onConfigCommandEnd(DpCommand &command)
{
    DP_STATUS_ENUM  status;
    PATH_STATE_ENUM state;
    int32_t         index;
    Engine          *pCurr;

    state = getState();

    if (eStop == state)
    {
        // Don't need to config frame and tile
        m_lastStatus = DP_STATUS_INVALID_STATE;
        return DP_STATUS_INVALID_STATE;
    }

    if (!(DpStream::CONFIG_FRAME_ONLY & m_pStream->getConfigFlags()) &&
        ((eStart == state) ||
         (eTile  == state)))
    {
        // Config ending information
        for (index = 0; index < m_engineCount; index++)
        {
            pCurr = m_pEngineBlock[index];
            assert(NULL != pCurr);

            status = pCurr->commandEnd(command);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpTilePath: config command end error %d\n", status);
                m_lastStatus = status;
                return status;
            }
        }
    }
    return DP_STATUS_RETURN_SUCCESS;
}


template <>
DP_STATUS_ENUM DpTilePathMT::threadLoop()
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM  status;
    PATH_STATE_ENUM state;
    int taskCounter = 0;
    DPLOGI("DpTilePathMT: thread loop begin\n");

    state = getState();

#ifdef MDP_VSS_ASYNC_ENABLE
    // Reset engineState and lastStatus
    m_lastStatus = DP_STATUS_RETURN_SUCCESS;
#endif
    while (eStop != state)
    {
#ifndef MDP_VSS_ASYNC_ENABLE
    // Reset engineState and lastStatus
    m_lastStatus = DP_STATUS_RETURN_SUCCESS;
#endif

        if ((eStart == state) ||
            (0 != m_pendingTile))
        {
            if (0 == (DpStream::CONFIG_BUFFER_IGNORE & m_pStream->getConfigFlags()))
            {
                status = waitFence();
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    // When fence timeout, keep job process instead of system hang.
                    // ex: Use GPU to draw game and work load too heavy
                    // Todo: implement job error handle flow
                }
            }
            if (STREAM_ISP_VSS == m_pStream->getScenario()){
                m_pCommander->setSubtaskId(taskCounter++);
            }
            status = execCommand();
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                m_lastStatus = status;
                break;
            }
            else
            {
                if (((STREAM_FRAG != m_pStream->getScenario()) &&
                    (STREAM_FRAG_JPEGDEC != m_pStream->getScenario()) &&
                    (STREAM_ISP_VSS != m_pStream->getScenario()) &&
                    (STREAM_ISP_ZSD_SLOW != m_pStream->getScenario()))
#ifdef MDP_VSS_ASYNC_ENABLE
                    || (STREAM_ISP_VSS == m_pStream->getScenario() && m_lastStatus == DP_STATUS_ALL_TPIPE_DONE)
#endif
                    )
                {
                    testSetState(eStop, eStop);
                }
            }
        }
        else
        {
            if (0 == (DpStream::CONFIG_BUFFER_IGNORE & m_pStream->getConfigFlags()))
            {
                // Wait for the next frame
                status = pollPort(true);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    m_lastStatus = status;
                    break;
                }
            }

            status = config();
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                m_lastStatus = status;
                break;
            }
        }

        state = getState();
    }

    lock();
    if (eStop == m_pathState)
    {
        m_lastStatus = DP_STATUS_RETURN_SUCCESS;
    }
    m_pathState = eStop;
    unlock();

    if (m_lastStatus < 0)
    {
        dumpDebugInfo();

        abortPath();
        flipBuffer();

        m_pathMutexObj.lock();
        m_condition.signal();
        m_pathMutexObj.unlock();

        return m_lastStatus;
    }

    DPLOGI("DpTilePathMT: thread loop end\n");

    return DP_STATUS_RETURN_SUCCESS;
}

