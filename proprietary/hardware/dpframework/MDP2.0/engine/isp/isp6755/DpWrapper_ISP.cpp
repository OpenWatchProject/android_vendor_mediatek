#include "DpWrapper_ISP.h"
#include "mdp_reg_rdma.h"
#include "isp_reg.h"

DP_STATUS_ENUM DpWrapper_ISP::onWaitEvent(DpCommand &command)
{
    //Modify the trigger order to MDP first
    if (TPIPE_PASS2_CMDQ_1 == m_ISPPassType)
    {
        MM_REG_WRITE_MASK(command, ISP_BASE_HW + offsetof(isp_reg_t, CAM_CTL_START), 0x0001, 0x00000001);
    }else if (TPIPE_PASS2_CMDQ_2 == m_ISPPassType)
    {
        MM_REG_WRITE_MASK(command, ISP_BASE_HW + offsetof(isp_reg_t, CAM_CTL_START), 0x0002, 0x00000002);
    }else if (TPIPE_PASS2_CMDQ_3 == m_ISPPassType)
    {
        MM_REG_WRITE_MASK(command, ISP_BASE_HW + offsetof(isp_reg_t, CAM_CTL_START), 0x0010, 0x00000010);
    }

    if (TPIPE_PASS2_CMDQ_1 == m_ISPPassType)
    {
        MM_REG_WAIT(command, DpCommand::ISP_P2_0_DONE);
    }else if (TPIPE_PASS2_CMDQ_2 == m_ISPPassType)
    {
        MM_REG_WAIT(command, DpCommand::ISP_P2_1_DONE);
    }else if (TPIPE_PASS2_CMDQ_3 == m_ISPPassType)
    {
        MM_REG_WAIT(command, DpCommand::ISP_P2_2_DONE);
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_ISP::onFlushBuffer(FLUSH_TYPE_ENUM type)
{
    if (FLUSH_AFTER_HW_WRITE == type)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    DPLOGI("[onFlushBuffer]+,\n");
    //m_pInDataPort->flushBuffer(CLIENT_CONSUMER);

#if CONFIG_FOR_DUMP_COMMAND
    FILE *pFile;
    pFile = fopen("./out/TDRIOutput.bin", "wb");
    fwrite(m_pTDRIBufVA, 1, m_pTileDesc->used_word_no*4, pFile); //Holmes : make the correct size=>uint word=>4 bytes
    fclose(pFile);
#endif // CONFIG_FOR_DUMP_COMMAND

#if CONFIG_FOR_VERIFY_FPGA
    uint32_t i,tmpOffset;
    pFile = fopen("./out/ISPFPGAInfo.bin","wb");
    fwrite(&m_TDRITileID, sizeof(m_TDRITileID), 1, pFile); //Holmes : make the correct size=>uint word=>4 bytes
    for(i=0;i<m_TDRITileID;i++)
    {
        tmpOffset = (m_pTileDesc->tpipe_info[i].dump_offset_no) << 2;
        fwrite(&tmpOffset, sizeof(uint32_t), 1, pFile); //Holmes : make the correct size=>uint word=>4 bytes
    }
    fclose(pFile);
#endif // CONFIG_FOR_VERIFY_FPGA

    DPLOGI("[onFlushBuffer]-,\n");

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_ISP::onConfigFrame(DpCommand &command,
                                           DpConfig  &config)
{
    DP_STATUS_ENUM status;
    int32_t        size;

    DPLOGI("[onConfigFrame]+,\n");

    memset(m_pTileDesc, 0x0, sizeof(ISP_TPIPE_DESCRIPTOR_STRUCT));
    //memset(m_pTileInfo, 0x0, MAX_TILE_TOT_NO * sizeof(ISP_TPIPE_INFORMATION_STRUCT));
    //memset(m_pTdrFlag, 0x0, ((MAX_TILE_TOT_NO + 31) >> 5) * sizeof(unsigned int));
    //memset(m_pIrqFlag, 0x0, ((MAX_TILE_TOT_NO + 31) >> 5) * sizeof(unsigned int));

    status = m_pInDataPort->getParameter(&m_ISPConfig, &size);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpWrapper_ISP: get ISP tile paramter failed\n");
        return DP_STATUS_INVALID_PARAX;
    }

    status = setupFrameInfo(config.scenario,
                            config.inFormat);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

#if CONFIG_FOR_VERIFY_FPGA
    m_TDRIBufPA = m_pTDRIBufMem->mapHWAddress(tIMGI, 0);
#else
    m_TDRIBufPA = m_ISPConfig.drvinfo.tpipeTablePa;
#endif // CONFIG_FOR_VERIFY_FPGA
    assert(0 != m_TDRIBufPA);

#if CONFIG_FOR_VERIFY_FPGA
    m_pTDRIBufVA = (uint32_t*)m_pTDRIBufMem->mapSWAddress();
#else
    m_pTDRIBufVA = m_ISPConfig.drvinfo.tpipeTableVa;
#endif // CONFIG_FOR_VERIFY_FPGA
    assert(0 != m_pTDRIBufVA);

    // Initialize hex dump info
    m_pTileDesc->tpipe_config       = m_pTDRIBufVA;
    m_pTileDesc->tpipe_info         = m_pTileInfo;
    m_pTileDesc->tdr_disable_flag   = m_pTdrFlag;
    m_pTileDesc->last_irq_flag      = m_pIrqFlag;
    m_pTileDesc->total_tpipe_no     = MAX_TILE_TOT_NO;
    m_pTileDesc->total_word_no      = MAX_ISP_TILE_TDR_HEX_NO;

    //m_backup_isp_tilewitdh = m_ISPConfig.sw.tpipe_width;
    //m_backup_isp_tileheight = m_ISPConfig.sw.tpipe_height;

#if !CONFIG_FOR_VERIFY_FPGA
    if (m_inFrameWidth != m_ISPConfig.sw.src_width)
    {
        DPLOGI("Change IMGI input width from %d to %d\n",m_inFrameWidth,m_ISPConfig.sw.src_width);
        m_inFrameWidth = m_ISPConfig.sw.src_width;
    }

    if (m_inFrameHeight != m_ISPConfig.sw.src_height)
    {
        DPLOGI("Change IMGI input height from %d to %d\n",m_inFrameHeight,m_ISPConfig.sw.src_height);
        m_inFrameHeight = m_ISPConfig.sw.src_height;
    }

    if (0 == m_ISPConfig.top.mdp_crop_en)
    {
        DPLOGI("Change IMGI output width/height from ISP config w:%d h:%d\n",m_ISPConfig.sw.src_width, m_ISPConfig.sw.src_height);
        m_outFrameWidth = m_ISPConfig.sw.src_width;
        m_outFrameHeight = m_ISPConfig.sw.src_height;
    }

    if (0 != m_ISPConfig.drvinfo.debugRegDump)
    {
        command.setISPDebugDumpRegs(m_ISPConfig.drvinfo.debugRegDump);
        DPLOGI("setISPDebugDumpRegs: %d\n",m_ISPConfig.drvinfo.debugRegDump);
    }
#endif // CONFIG_FOR_VERIFY_FPGA

    DPLOGI("[onConfigFrame]-,\n");

    command.frameSrcInfo(config.inFormat,
                         config.inWidth,
                         config.inHeight,
                         config.inYPitch,
                         config.inUVPitch,
                         config.memAddr,
                         config.memSize,
                         m_pInDataPort->getSecureStatus());

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_ISP::initTilePath(TILE_PARAM_STRUCT *p_tile_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;

    /* tile core property */
    TILE_REG_MAP_STRUCT *ptr_tile_reg_map = p_tile_param->ptr_tile_reg_map;
    m_pTileFunc = p_tile_param->ptr_tile_func_param;

    assert(NULL == p_tile_param->ptr_isp_tile_descriptor);
    p_tile_param->ptr_isp_tile_descriptor = m_pTileDesc;

    DPLOGI("[initTilePath]+,\n");

#if !CONFIG_FOR_VERIFY_FPGA
    result = tile_copy_config_by_platform(p_tile_param, &m_ISPConfig);
    if (ISP_MESSAGE_OK != result)
    {
        DPLOGE("[Error]onConfigFrame: get ISP tile paramter failed,result(%d)\n",result);
        return DP_STATUS_INVALID_PARAX;
    }

    ptr_tile_reg_map->last_irq_mode = m_ISPConfig.sw.tpipe_irq_mode; // 0 FOR ISP ONLY,2 FOR MDP directtlink
    ptr_tile_reg_map->isp_tile_width  = m_ISPConfig.sw.tpipe_width;
    ptr_tile_reg_map->isp_tile_height = m_ISPConfig.sw.tpipe_height;

    m_ISPPassType = m_ISPConfig.pass2.Pass2CmdqNum;

#else // CONFIG_FOR_VERIFY_FPGA
    result = tile_main_read_isp_reg_file(ptr_tile_reg_map, "tile_reg_map_frame.txt");
    if (ISP_MESSAGE_OK != result)
    {
        DPLOGE("DpWrapper_ISP: read tile_reg_map_frame.txt failed\n");
        return DP_STATUS_INVALID_FILE;
    }

    ptr_tile_reg_map->last_irq_mode = 2; // 0 FOR ISP ONLY,2 FOR MDP directtlink
    ptr_tile_reg_map->isp_tile_width  = 768;
    ptr_tile_reg_map->isp_tile_height = MAX_SIZE;

    ptr_tile_reg_map->tdr_ctrl_en = true;
    ptr_tile_reg_map->skip_tile_mode = true;
    ptr_tile_reg_map->run_c_model_direct_link = 0;

#endif // CONFIG_FOR_VERIFY_FPGA
    ptr_tile_reg_map->max_input_width  = MAX_SIZE;  /* can modify to test */
    ptr_tile_reg_map->max_input_height = MAX_SIZE;  /* can modify to test */

    ptr_tile_reg_map->src_dump_order = m_outDumpOrder; //Normal direction
    ptr_tile_reg_map->src_stream_order = m_inStreamOrder; //Normal direction
    ptr_tile_reg_map->src_cal_order = m_inCalOrder; //Normal direction

    DPLOGI("[initTilePath]-,\n");

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_ISP::onInitTileCalc(struct TILE_PARAM_STRUCT* p_tile_param)
{
    uint32_t index;

    /* tile core property */
    TILE_REG_MAP_STRUCT     *ptr_tile_reg_map = p_tile_param->ptr_tile_reg_map;
    FUNC_DESCRIPTION_STRUCT *ptr_tile_func_param = p_tile_param->ptr_tile_func_param;
    TILE_FUNC_BLOCK_STRUCT  *ptr_func;

    for (index = 0; index < ptr_tile_func_param->used_func_no; index++)
    {
        ptr_func = &ptr_tile_func_param->func_list[index];
        if (TILE_FUNC_IMGI_ID == ptr_func->func_num)
        {
            m_pIMGI = ptr_func;
        }

#if PMQOS_SETTING
        switch(ptr_func->func_num)
        {
            case TILE_FUNC_IMGI_ID:
                m_pISPDMA[TILE_FUNC_DMA_IMGI] = ptr_func;
                break;
            //case TILE_FUNC_IMGBI_ID:
            //    m_pISPDMA[TILE_FUNC_DMA_IMGBI] = ptr_func;
            //    break;
            //case TILE_FUNC_IMGCI_ID:
            //    m_pISPDMA[TILE_FUNC_DMA_IMGCI] = ptr_func;
            //    break;
            //case TILE_FUNC_UFDI_ID:
            //    m_pISPDMA[TILE_FUNC_DMA_UFDI] = ptr_func;
            //    break;
            //case TILE_FUNC_VIPI_ID:
            //    m_pISPDMA[TILE_FUNC_DMA_VIPI] = ptr_func;
            //    break;
            //case TILE_FUNC_VIP2I_ID:
            //    m_pISPDMA[TILE_FUNC_DMA_VIP2I] = ptr_func;
            //    break;
            //case TILE_FUNC_VIP3I_ID:
            //    m_pISPDMA[TILE_FUNC_DMA_VIP3I] = ptr_func;
            //    break;
            //case TILE_FUNC_DMGI_ID:
            //    m_pISPDMA[TILE_FUNC_DMA_DMGI] = ptr_func;
            //    break;
            //case TILE_FUNC_DEPI_ID:
            //    m_pISPDMA[TILE_FUNC_DMA_DEPI] = ptr_func;
            //    break;
            //case TILE_FUNC_MFBO_ID:
            //    m_pISPDMA[TILE_FUNC_DMA_MFBO] = ptr_func;
            //    break;
            //case TILE_FUNC_IMG2O_ID:
            //    m_pISPDMA[TILE_FUNC_DMA_IMG2O] = ptr_func;
            //    break;
            //case TILE_FUNC_IMG2BO_ID:
            //    m_pISPDMA[TILE_FUNC_DMA_IMG2BO] = ptr_func;
            //    break;
            //case TILE_FUNC_FEO_ID:
            //    m_pISPDMA[TILE_FUNC_DMA_FEO] = ptr_func;
            //    break;
            //case TILE_FUNC_IMG3O_ID:
            //    m_pISPDMA[TILE_FUNC_DMA_IMG3O] = ptr_func;
            //    break;
            //case TILE_FUNC_IMG3BO_ID:
            //    m_pISPDMA[TILE_FUNC_DMA_IMG3BO] = ptr_func;
            //    break;
            //case TILE_FUNC_IMG3CO_ID:
            //    m_pISPDMA[TILE_FUNC_DMA_IMG3CO] = ptr_func;
            //    break;
            default:
                break;
        }
#endif // PMQOS_SETTING

        if (TILE_FUNC_MDP_CROP_ID == ptr_func->func_num)
        {
            m_pMDPCrop = ptr_func;
        }
    }

#if !CONFIG_FOR_VERIFY_FPGA
    if (m_onlyISP && m_ISPConfig.top.mdp_crop_en)
    {
        DPLOGE("DpWrapper_ISP: MDP_Crop is enabled in ISP Pass2 only.\n");
        assert(0);
    }
#else // CONFIG_FOR_VERIFY_FPGA
    if (0 != m_pMDPCrop)
    {
        ptr_tile_reg_map->run_c_model_direct_link = 1;
    }
    assert(false == m_tileDisable); // on init frame mode
#endif // CONFIG_FOR_VERIFY_FPGA

    m_TDRITileID = 0;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_ISP::onRetrieveTileParam(struct TILE_PARAM_STRUCT* p_tile_param)
{
#if PMQOS_SETTING
    /* tile core property */
    FUNC_DESCRIPTION_STRUCT *ptr_tile_func_param = p_tile_param->ptr_tile_func_param;
    TILE_REG_MAP_STRUCT *ptr_tile_reg_map = p_tile_param->ptr_tile_reg_map;

    if ((ptr_tile_func_param != NULL) && (ptr_tile_reg_map != NULL))
    {
        m_tileXEndFlag = ptr_tile_func_param->func_list[ptr_tile_reg_map->first_func_en_no].h_end_flag;
        m_tileYEndFlag = ptr_tile_func_param->func_list[ptr_tile_reg_map->first_func_en_no].v_end_flag;
    }
    else //ISP frame mode case
    {
        m_tileXEndFlag = 1;
        m_tileYEndFlag = 1;
        m_inFrameHeight = m_ISPConfig.drvinfo.framemode_h;
        m_inTileAccumulation = m_ISPConfig.drvinfo.framemode_total_in_w;
        m_ispDataSize = m_ISPConfig.drvinfo.total_data_size;
        return DP_STATUS_RETURN_SUCCESS;
    }

#if 0
#endif // PMQOS_SETTING
    if (NULL != m_pMDPCrop)
    {
        m_outTileXLeft   = m_pMDPCrop->out_pos_xs;
        m_outTileXRight  = m_pMDPCrop->out_pos_xe;
        m_outTileYTop    = m_pMDPCrop->out_pos_ys;
        m_outTileYBottom = m_pMDPCrop->out_pos_ye;
        //m_tileXEndFlag   = m_pMDPCrop->h_end_flag;
        //m_tileYEndFlag   = m_pMDPCrop->v_end_flag;
    }
    if (NULL != m_pIMGI)
    {
        m_inTileXLeft   = m_pIMGI->in_pos_xs;
        m_inTileXRight  = m_pIMGI->in_pos_xe;
        m_inTileYTop    = m_pIMGI->in_pos_ys;
        m_inTileYBottom = m_pIMGI->in_pos_ye;
    }
#if PMQOS_SETTING
#endif

    for (int i = 0; i < TILE_FUNC_DMA_NUM; i++)
    {
        if(m_pISPDMA[i]!=NULL)
        {
            switch(i)
            {
                case TILE_FUNC_DMA_IMGI:
                    m_inTileAccumulation += (m_pISPDMA[i]->in_pos_xe - m_pISPDMA[i]->in_pos_xs + 1);
                    break;
                //case TILE_FUNC_DMA_IMGBI:
                //case TILE_FUNC_DMA_IMGCI:
                //case TILE_FUNC_DMA_UFDI:
                //case TILE_FUNC_DMA_VIPI:
                //case TILE_FUNC_DMA_VIP2I:
                //case TILE_FUNC_DMA_VIP3I:
                //case TILE_FUNC_DMA_LCEI:
                //case TILE_FUNC_DMA_DMGI:
                //case TILE_FUNC_DMA_DEPI:
                    //break;
                //case TILE_FUNC_DMA_MFBO:
                //case TILE_FUNC_DMA_IMG2O:
                //case TILE_FUNC_DMA_IMG2BO:
                //case TILE_FUNC_DMA_FEO:
                //case TILE_FUNC_DMA_IMG3O:
                //case TILE_FUNC_DMA_IMG3BO:
                //case TILE_FUNC_DMA_IMG3CO:
                    //break;
                default:
                break;
            }
        }
    }

    // Calculate ISP DMA BW
    if (ptr_tile_reg_map->IMGI_EN)
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->IMGI_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMGI_TILE_YSIZE + 1);
    //if (ptr_tile_reg_map->CTRL_IMGBI_EN)
        //m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->IMGBI_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMGBI_TILE_YSIZE + 1);
    //if (ptr_tile_reg_map->CTRL_IMGCI_EN)
        //m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->IMGCI_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMGCI_TILE_YSIZE + 1);
    if (ptr_tile_reg_map->CTRL_UFDI_EN)
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->UFDI_TILE_XSIZE + 1) * (ptr_tile_reg_map->UFDI_TILE_YSIZE + 1);
    if (ptr_tile_reg_map->CTRL_VIPI_EN)
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->VIPI_TILE_XSIZE + 1) * (ptr_tile_reg_map->VIPI_TILE_YSIZE + 1);
    if (ptr_tile_reg_map->CTRL_VIP2I_EN)
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->VIP2I_TILE_XSIZE + 1) * (ptr_tile_reg_map->VIP2I_TILE_YSIZE + 1);
    if (ptr_tile_reg_map->CTRL_VIP3I_EN)
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->VIP3I_TILE_XSIZE + 1) * (ptr_tile_reg_map->VIP3I_TILE_YSIZE + 1);
    if (ptr_tile_reg_map->CTRL_LCEI_EN)
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->LCEI_TILE_XSIZE + 1) * (ptr_tile_reg_map->LCEI_TILE_YSIZE + 1);
    //if (ptr_tile_reg_map->CTRL_DEPI_EN)
        //m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->DEPI_TILE_XSIZE + 1) * (ptr_tile_reg_map->DEPI_TILE_YSIZE + 1);
    //if (ptr_tile_reg_map->CTRL_DMGI_EN)
        //m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->DMGI_TILE_XSIZE + 1) * (ptr_tile_reg_map->DMGI_TILE_YSIZE + 1);
    if (ptr_tile_reg_map->CTRL_MFBO_EN)
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->MFBO_TILE_XSIZE + 1) * (ptr_tile_reg_map->MFBO_TILE_YSIZE + 1);
    if (ptr_tile_reg_map->CTRL_IMG2O_EN)
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->IMG2O_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMG2O_TILE_YSIZE + 1);
    //if (ptr_tile_reg_map->CTRL_IMG2BO_EN)
        //m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->IMG2BO_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMG2BO_TILE_YSIZE + 1);
    if (ptr_tile_reg_map->CTRL_IMG3O_EN)
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->IMG3O_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMG3O_TILE_YSIZE + 1);
    if (ptr_tile_reg_map->CTRL_IMG3BO_EN)
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->IMG3BO_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMG3BO_TILE_YSIZE + 1);
    if (ptr_tile_reg_map->CTRL_IMG3CO_EN)
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->IMG3CO_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMG3CO_TILE_YSIZE + 1);
    if (ptr_tile_reg_map->CTRL_FEO_EN)
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->FEO_TILE_XSIZE + 1) * (ptr_tile_reg_map->FEO_TILE_YSIZE + 1);

    m_ispDataSize = m_ISPConfig.drvinfo.total_data_size;

    if (m_onlyISP)
    {
        m_inFrameHeight = ptr_tile_reg_map->IMGI_TILE_YSIZE + 1;
        DPLOGD("DpWrapper_ISP: inFrameHeight = %d, total_data = %d\n", m_inFrameHeight, m_ISPConfig.drvinfo.total_data_size);
    }

    DPLOGI("DpWrapper_ISP: m_inTileAccumulation = %d\n", m_inTileAccumulation);
    DPLOGI("DpWrapper_ISP: m_outTileAccumulation = %d\n", m_outTileAccumulation);
#endif // PMQOS_SETTING

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_ISP::onConfigTile(DpCommand &command)
{
    uint32_t tpipePa;

#if CONFIG_FOR_VERIFY_FPGA
    MM_REG_WRITE_MASK(command, MDP_FAKE_ISP_CFG, ((m_TDRITileID&0xFF) << 24), 0xFF000000);
#endif

    tpipePa = m_TDRIBufPA + m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no*sizeof(uint32_t);

    DPLOGI("[onConfigTile]+,m_TDRITileID(%d),offset(0x%x),tpipePa(0x%08x)\n", m_TDRITileID, m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no,tpipePa);

    m_TDRITileID++;
    MM_REG_WRITE_MASK(command, ISP_BASE_HW + offsetof(isp_reg_t, CAM_TDRI_BASE_ADDR), tpipePa, 0xFFFFFFFF);

#if !CONFIG_FOR_VERIFY_FPGA
    DPLOGI("[onConfigTile]p2Cq(%d),VirCqPa(0x%x)\n",m_ISPConfig.drvinfo.p2Cq,m_ISPConfig.drvinfo.DesCqPa);

    switch (m_ISPConfig.drvinfo.p2Cq) {
        case ISP_TPIPE_P2_CQ1:
            MM_REG_WRITE_MASK(command, ISP_BASE_HW + offsetof(isp_reg_t, CAM_CTL_CQ1_BASEADDR), m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF); //ISP CQ1 base address
            break;
        case ISP_TPIPE_P2_CQ2:
            MM_REG_WRITE_MASK(command, ISP_BASE_HW + offsetof(isp_reg_t, CAM_CTL_CQ2_BASEADDR), m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF); //ISP CQ2 base address
            break;
        case ISP_TPIPE_P2_CQ3:
            MM_REG_WRITE_MASK(command, ISP_BASE_HW + offsetof(isp_reg_t, CAM_CTL_CQ3_BASEADDR), m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF); //ISP CQ3 base address
            break;
        default:
            DPLOGE("[Error]Not support this p2Cq(%d)\n",m_ISPConfig.drvinfo.p2Cq);
            break;
    }
#else // CONFIG_FOR_VERIFY_FPGA
    uint32_t width = m_outTileXRight - m_outTileXLeft + 1;
    uint32_t height = m_outTileYBottom - m_outTileYTop + 1;
    MM_REG_WRITE_MASK(command, MDP_FAKE_ISP_CFG, (height << 12) + width, 0x00FFFFFF);
#endif // CONFIG_FOR_VERIFY_FPGA

    return DP_STATUS_RETURN_SUCCESS;
}

#if PMQOS_SETTING
DP_STATUS_ENUM DpWrapper_ISP::onUpdatePMQOS(uint32_t& totalPixel, uint32_t& totalDataSzie, struct timeval& endtime)
{
    uint32_t        pixelCount = 0;

    totalDataSzie += m_ispDataSize;

    pixelCount = m_inTileAccumulation * m_inFrameHeight;

    endtime = m_pInDataPort->getEndTime();

    if (pixelCount > totalPixel)
    {
        totalPixel = pixelCount;
    }

    DPLOGI("DpWrapper_ISP: m_inTileAccumulation = %d\n", m_inTileAccumulation);
    DPLOGI("DpWrapper_ISP: m_outTileAccumulation = %d\n", m_outTileAccumulation);
    DPLOGI("DpWrapper_ISP::updatePMQOS totalDataSzie = %d totalPixel = %d\n", totalDataSzie, totalPixel);

    return DP_STATUS_RETURN_SUCCESS;
}
#endif // PMQOS_SETTING