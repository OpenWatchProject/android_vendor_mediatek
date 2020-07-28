#include "DpWrapper_ISP.h"
#include "mdp_reg_rdma.h"
#include "dip_reg.h"
#include "mmsys_config.h"
#include "smi_master_port.h"
#include "camera_mfb.h"

DP_STATUS_ENUM DpWrapper_ISP::onInitEngine(DpCommand &command)
{
    DP_STATUS_ENUM status;
    int32_t        size;

    status = m_pInDataPort->getParameter(&m_ISPConfig, &size);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpWrapper_ISP: get ISP tile paramter failed\n");
        return DP_STATUS_INVALID_PARAX;
    }

    m_pInDataPort->getIsp2MdpParam(&m_pIsp2Mdp, &size);
    if (m_pIsp2Mdp.MET_String_length > 0){
        //command.addIspMetLog(m_pIsp2Mdp.MET_String, m_pIsp2Mdp.MET_String_length);
    }
	if (m_ISPConfig.drvinfo.dip_secure_tag&0x1) // For secure camera path
    {
        command.addIspSecureMeta(m_pIsp2Mdp.secIspData);
    }


    // Direct link
    if (m_ISPConfig.top.mcrp_en)
    {
        DPLOGI("DpWrapper_ISP: SW_RST ASYNC\n");
        // Reset MDP_DL_ASYNC_TX
        // Bit  3: MDP_DL_ASYNC_TX / MDP_RELAY
        MM_REG_WRITE(command, MMSYS_SW0_RST_B,    0x0, 0x00000008);
        MM_REG_WRITE(command, MMSYS_SW0_RST_B, 1 << 3, 0x00000008);
        // Reset MDP_DL_ASYNC_RX
        // Bit  10: MDP_DL_ASYNC_RX
        MM_REG_WRITE(command, MMSYS_SW1_RST_B,     0x0, 0x00000400);
        MM_REG_WRITE(command, MMSYS_SW1_RST_B, 1 << 10, 0x00000400);

        //enable sof mode
        //MM_REG_WRITE(command, ISP_RELAY_CFG_WD, 0 <<31, 0x80000000);
    }

    if (m_ISPConfig.top.mcrp2_en)
    {
        DPLOGI("DpWrapper_ISP: SW_RST ASYNC2\n");
        // Reset MDP_DL_ASYNC2_TX
        // Bit  4: MDP_DL_ASYNC2_TX / MDP_RELAY2
        MM_REG_WRITE(command, MMSYS_SW0_RST_B,    0x0, 0x00000010);
        MM_REG_WRITE(command, MMSYS_SW0_RST_B, 1 << 4, 0x00000010);
        // Reset MDP_DL_ASYNC2_RX
        // Bit  11: MDP_DL_ASYNC2_RX
        MM_REG_WRITE(command, MMSYS_SW1_RST_B,     0x0, 0x00000800);
        MM_REG_WRITE(command, MMSYS_SW1_RST_B, 1 << 11, 0x00000800);

        //enable sof mode
        //MM_REG_WRITE(command, IPU_RELAY_CFG_WD, 0 <<31, 0x80000000);
    }

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpWrapper_ISP::onWaitEvent(DpCommand &command)
{
    MM_REG_WRITE_MASK(command, 0x150220A8, 0x007fffff, 0xFFFFFFFF);
    MM_REG_WRITE_MASK(command, 0x150220B8, 0x00000fff, 0xFFFFFFFF);
    MM_REG_WRITE_MASK(command, 0x150220C8, 0xFE0000ff, 0xFFFFFFFF);
    MM_REG_WRITE_MASK(command, 0x150220D8, 0xF807ffff, 0xFFFFFFFF);
    MM_REG_WRITE_MASK(command, 0x150220E8, 0x0007ffff, 0xFFFFFFFF);
    MM_REG_WRITE_MASK(command, 0x150220F8, 0x0007ffff, 0xFFFFFFFF);

    MM_REG_WRITE_MASK(command, 0x15020210, 0x00000003, 0x00000003);
    MM_REG_WRITE_MASK(command, 0x15020210, 0x00000000, 0x00000003);

    if (m_ISPConfig.top.mcrp_en)
        MM_REG_WRITE_MASK(command, 0x1502004C, 0x00000000, 0x00000200); // MDP_DL_SEL: select MDP_CROP for 5.0
    if (m_ISPConfig.top.mcrp2_en)
        MM_REG_WRITE_MASK(command, 0x1502004C, 0x00000000, 0x00000C00); // MDP2_DL_SEL: select MDP_CROP2 for 5.0
    if (m_ISPConfig.top.mfb_en)
        MM_REG_WRITE_MASK(command, 0x1502004C, 0x00000000, 0x00000100); // DL P2 SEL = from MFB

    switch (m_ISPPassType) {
        case ISP_DRV_DIP_CQ_THRE0:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0001, 0x00000001);
            if (m_ISPConfig.top.mfb_en) MM_REG_WRITE_MASK(command, 0x15010260, 0x0001, 0x00000001);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_0_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE1:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0002, 0x00000002);
            if (m_ISPConfig.top.mfb_en) MM_REG_WRITE_MASK(command, 0x15010260, 0x0001, 0x00000001);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_1_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE2:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0004, 0x00000004);
            if (m_ISPConfig.top.mfb_en) MM_REG_WRITE_MASK(command, 0x15010260, 0x0001, 0x00000001);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_2_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE3:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0008, 0x00000008);
            if (m_ISPConfig.top.mfb_en) MM_REG_WRITE_MASK(command, 0x15010260, 0x0001, 0x00000001);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_3_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE4:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0010, 0x00000010);
            if (m_ISPConfig.top.mfb_en) MM_REG_WRITE_MASK(command, 0x15010260, 0x0001, 0x00000001);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_4_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE5:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0020, 0x00000020);
            if (m_ISPConfig.top.mfb_en) MM_REG_WRITE_MASK(command, 0x15010260, 0x0001, 0x00000001);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_5_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE6:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0040, 0x00000040);
            if (m_ISPConfig.top.mfb_en) MM_REG_WRITE_MASK(command, 0x15010260, 0x0001, 0x00000001);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_6_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE7:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0080, 0x00000080);
            if (m_ISPConfig.top.mfb_en) MM_REG_WRITE_MASK(command, 0x15010260, 0x0001, 0x00000001);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_7_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE8:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0100, 0x00000100);
            if (m_ISPConfig.top.mfb_en) MM_REG_WRITE_MASK(command, 0x15010260, 0x0001, 0x00000001);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_8_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE9:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0200, 0x00000200);
            if (m_ISPConfig.top.mfb_en) MM_REG_WRITE_MASK(command, 0x15010260, 0x0001, 0x00000001);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_9_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE10:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0400, 0x00000400);
            if (m_ISPConfig.top.mfb_en) MM_REG_WRITE_MASK(command, 0x15010260, 0x0001, 0x00000001);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_10_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE11:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0800, 0x00000800);
            if (m_ISPConfig.top.mfb_en) MM_REG_WRITE_MASK(command, 0x15010260, 0x0001, 0x00000001);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_11_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE12:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x1000, 0x00001000);
            if (m_ISPConfig.top.mfb_en) MM_REG_WRITE_MASK(command, 0x15010260, 0x0001, 0x00000001);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_12_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE13:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x2000, 0x00002000);
            if (m_ISPConfig.top.mfb_en) MM_REG_WRITE_MASK(command, 0x15010260, 0x0001, 0x00000001);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_13_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE14:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x4000, 0x00004000);
            if (m_ISPConfig.top.mfb_en) MM_REG_WRITE_MASK(command, 0x15010260, 0x0001, 0x00000001);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_14_DONE);
            break;
		// From CQ15 to CQ18, these do not connect to GCE +++++
		#if 0
        case ISP_DRV_DIP_CQ_THRE15:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x8000, 0x00008000);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_15_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE16:
			MM_REG_WRITE_MASK(command, 0x15022000, 0x10000, 0x00010000);
			//
			MM_REG_WAIT(command, DpCommand::ISP_P2_16_DONE);
			break;
        case ISP_DRV_DIP_CQ_THRE17:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x20000, 0x00020000);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_17_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE18:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x40000, 0x00040000);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_18_DONE);
            break;
		#endif
		// From CQ15 to CQ18, these do not connect to GCE -----
        default:
            DPLOGE("[Error]not support this m_ISPPassType(%d)\n", m_ISPPassType);
            break;
    }

    if (m_tileXEndFlag && m_tileYEndFlag && m_regCount)
    {
        MM_REG_READ_BEGIN(command);

        for (int i = 0; i < m_regCount; i++)
        {
            DPLOGI("DpWrapper_ISP: m_pReadbackReg = 0x%x, PABuffer = 0x%x\n", m_pReadbackReg[i], m_PABuffer[i]);
            MM_REG_READ(command, m_pReadbackReg[i], m_PABuffer[i], &m_regLabel[i]);
        }

        MM_REG_READ_END(command);
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_ISP::onDumpDebugInfo()
{
    DP_STATUS_ENUM ret = DP_STATUS_RETURN_SUCCESS;
#if 0  // move to ISP function "dequeueHwBuf"
    char* pWorkingBuffer =  NULL;
    uint32_t tpipeWorkingSize = tpipe_main_query_platform_working_buffer_size(8192);
    isp_reg_t *pIspReg;
    IspDrv      *pIspDrv;

    DPLOGI("run onDumpDebugInfo cqIdx(%d),DesCqVa(0x%x),VirCqVa(0x%x)\n",m_ISPConfig.drvinfo.cqIdx,m_ISPConfig.drvinfo.DesCqVa,m_ISPConfig.drvinfo.VirCqVa);

    // for getCurHWRegValues
    pIspDrv = IspDrv::createInstance();
    if (!pIspDrv) {
        DPLOGE("[Error]IspDrv::createInstance() fail \n");
        ret = DP_STATUS_BUFFER_EMPTY;
    }
    //
    if( pIspDrv->init("dpWrapper_isp") == 0) {
        DPLOGE("[Error]IspDrv::init() fail \n");
        ret = DP_STATUS_BUFFER_EMPTY;
    }
    // get hardware register
    pIspReg = (isp_reg_t*)pIspDrv->getCurHWRegValues();
    // dump ISP CQ table
    switch(m_ISPConfig.drvinfo.cqIdx){
        case ISP_TPIPE_P2_CQ1:
            pIspDrv->dumpCQTable(ISP_DRV_CQ01,m_ISPConfig.drvinfo.dupCqIdx,m_ISPConfig.drvinfo.DesCqVa,m_ISPConfig.drvinfo.VirCqVa);
            break;
        case ISP_TPIPE_P2_CQ2:
            pIspDrv->dumpCQTable(ISP_DRV_CQ02,m_ISPConfig.drvinfo.dupCqIdx,m_ISPConfig.drvinfo.DesCqVa,m_ISPConfig.drvinfo.VirCqVa);
            break;
        case ISP_TPIPE_P2_CQ3:
            pIspDrv->dumpCQTable(ISP_DRV_CQ03,m_ISPConfig.drvinfo.dupCqIdx,m_ISPConfig.drvinfo.DesCqVa,m_ISPConfig.drvinfo.VirCqVa);
            break;
        default:
            DPLOGE("[Error]Not support this cqIdx(%d)\n",m_ISPConfig.drvinfo.cqIdx);
            break;
    }
    pIspDrv->uninit("dpWrapper_isp");
    pIspDrv->destroyInstance();
    pIspDrv = NULL;
    //
    pWorkingBuffer = new char[tpipeWorkingSize];
    if((pWorkingBuffer!=NULL)&&(pIspReg!=NULL)) {
        tpipe_main_platform(&m_ISPConfig, m_pTileDesc, pWorkingBuffer, tpipeWorkingSize, (void*)pIspReg);

        delete pWorkingBuffer;
        pWorkingBuffer = NULL;
    } else {
        DPLOGE("pWorkingBuffer(0x%08x),pIspReg(0x%08x)",pWorkingBuffer,pIspReg);
        ret = DP_STATUS_BUFFER_EMPTY;
    }
#endif

    DpTileEngine::onDumpDebugInfo();

    return ret;
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

#if 0 //Holmes
#if CONFIG_FOR_DEBUG_ISP
    int32_t index;

    status = m_pIMGIBuffer->flush(DpMemoryProxy::FLUSH_BEFORE_HW_READ_WHOLE_CACHE);
    assert(true == status);

    status = m_pIMG2OBuffer->flush(DpMemoryProxy::FLUSH_BEFORE_HW_READ_WHOLE_CACHE);
    assert(true == status);

    for (index = 0; index < 10; index++)
    {
        status = m_pMemHandler[index]->flush(DpMemoryProxy::FLUSH_BEFORE_HW_READ_WHOLE_CACHE);
        assert(true == status);
    }
#else
    // Call base class to flush read buffer
    DpTileEngine::onFlushBuffer(type);
#endif  // CONFIG_FOR_DEBUG_ISP

    m_pTDRIBufMem->flush(DpMemoryProxy::FLUSH_BEFORE_HW_READ_WHOLE_CACHE);
#endif

    DPLOGI("[onFlushBuffer]-,\n");

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_ISP::onConfigFrame(DpCommand &command,
                                           DpConfig  &config)
{
    DP_STATUS_ENUM status;
    int32_t        size;
    int32_t        index = 0;

	uint32_t       secureRegAddr[3] = {0};
    uint32_t       offset[3] = {0};
    uint32_t       memSize[3] = {0};
    uint32_t       planeOffset[3] = {0};
    uint64_t       flag = 0;

    DPLOGI("[onConfigFrame]+,\n");

    memset(m_pTileDesc, 0x0, sizeof(ISP_TPIPE_DESCRIPTOR_STRUCT));
    //memset(m_pTileInfo, 0x0, MAX_TILE_TOT_NO * sizeof(ISP_TPIPE_INFORMATION_STRUCT));
    //memset(m_pTdrFlag, 0x0, ((MAX_TILE_TOT_NO + 31) >> 5) * sizeof(unsigned int));
    //memset(m_pIrqFlag, 0x0, ((MAX_TILE_TOT_NO + 31) >> 5) * sizeof(unsigned int));

    status = m_pInDataPort->getParameter(&m_ISPConfig, &size, &m_PABuffer[0]);
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
    m_TDRIBufPA_MFB = m_ISPConfig.drvinfo.tpipeTablePa_mfb;
#endif // CONFIG_FOR_VERIFY_FPGA
    assert(0 != m_TDRIBufPA);

#if CONFIG_FOR_VERIFY_FPGA
    m_pTDRIBufVA = (uint32_t*)m_pTDRIBufMem->mapSWAddress();
#else
    m_pTDRIBufVA = m_ISPConfig.drvinfo.tpipeTableVa;
    m_pTDRIBufVA_MFB = m_ISPConfig.drvinfo.tpipeTableVa_mfb;
#endif // CONFIG_FOR_VERIFY_FPGA
    assert(0 != m_pTDRIBufVA);

    // Initialize hex dump info
    m_pTileDesc->tpipe_config       = m_pTDRIBufVA;
    m_pTileDesc->tpipe_config_mfb   = m_pTDRIBufVA_MFB;
    m_pTileDesc->tpipe_info         = m_pTileInfo;
    m_pTileDesc->tpipe_info_mfb     = m_pTileInfo_MFB;
    m_pTileDesc->tdr_disable_flag   = m_pTdrFlag;
    m_pTileDesc->last_irq_flag      = m_pIrqFlag;
    m_pTileDesc->total_tpipe_no     = MAX_TILE_TOT_NO;
    m_pTileDesc->total_word_no      = MAX_ISP_TILE_TDR_HEX_NO;
    m_pTileDesc->total_word_no_mfb  = MAX_ISP_TILE_TDR_HEX_NO_MFB;

    m_ISPPassType = m_ISPConfig.drvinfo.cqIdx;
    //m_backup_isp_tilewitdh = m_ISPConfig.sw.tpipe_width;
    //m_backup_isp_tileheight = m_ISPConfig.sw.tpipe_height;

    if (m_ISPConfig.top.mfb_en) {
        MM_REG_WRITE_MASK(command, 0x15022004, 0x00000006, 0x00000007, &m_frameConfigLabel[index++]);
        MM_REG_WRITE_MASK(command, 0x15022004, 0x00000002, 0x00000007, &m_frameConfigLabel[index++]);
    }

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

    if (0 == m_ISPConfig.top.mcrp_en)
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

#if 1
	if (m_ISPConfig.drvinfo.dip_secure_tag&0x1) // For secure camera path
	{
		if (m_ISPConfig.drvinfo.dip_img2o_secure_tag != 0) // IMG2O, CRZO
		{
			secureRegAddr[0] = 0x150215B0;
			secureRegAddr[1] = 0x15021620;
			secureRegAddr[2] = 0;
			planeOffset[0] = 0;
			planeOffset[1] = m_ISPConfig.drvinfo.dip_img2bo_base_addr;
			planeOffset[2] = 0;
			flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_IMG2O);
			command.secure(CMDQ_ENG_ISP_IMG2O, flag, secureRegAddr, (DpSecure)m_ISPConfig.drvinfo.dip_img2o_secure_tag, m_ISPConfig.drvinfo.dip_img2o_base_addr, offset, m_ISPConfig.drvinfo.dip_img2o_size, planeOffset);
		}
		if (m_ISPConfig.drvinfo.dip_img3o_secure_tag != 0) // IMG3O
		{
			secureRegAddr[0] = 0x15021790;
			secureRegAddr[1] = 0x15021800;
			secureRegAddr[2] = 0x15021870;
			planeOffset[0] = 0;
			planeOffset[1] = m_ISPConfig.drvinfo.dip_img3bo_base_addr;
			planeOffset[2] = m_ISPConfig.drvinfo.dip_img3co_base_addr;
			flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_IMG3O);
			command.secure(CMDQ_ENG_ISP_IMG3O, flag, secureRegAddr, (DpSecure)m_ISPConfig.drvinfo.dip_img3o_secure_tag, m_ISPConfig.drvinfo.dip_img3o_base_addr, offset, m_ISPConfig.drvinfo.dip_img3o_size, planeOffset);
		}
		if (m_ISPConfig.drvinfo.dip_feo_secure_tag != 0) // FEO
		{
			secureRegAddr[0] = 0x15021690;
			secureRegAddr[1] = 0;
			secureRegAddr[2] = 0;
			planeOffset[0] = 0;
			planeOffset[1] = 0;
			planeOffset[2] = 0;
			flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_IMG2O);
			command.secure(CMDQ_ENG_ISP_IMG2O, flag, secureRegAddr, (DpSecure)m_ISPConfig.drvinfo.dip_feo_secure_tag, m_ISPConfig.drvinfo.dip_feo_base_addr, offset, m_ISPConfig.drvinfo.dip_feo_size, planeOffset);
		}
		if (m_ISPConfig.drvinfo.dip_dceso_secure_tag != 0) // DCESO
		{
			secureRegAddr[0] = 0x15021400;
			secureRegAddr[1] = 0;
			secureRegAddr[2] = 0;
			planeOffset[0] = 0;
			planeOffset[1] = 0;
			planeOffset[2] = 0;
			flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_TIMGO);
			command.secure(CMDQ_ENG_ISP_TIMGO, flag, secureRegAddr, (DpSecure)m_ISPConfig.drvinfo.dip_dceso_secure_tag, m_ISPConfig.drvinfo.dip_dceso_base_addr, offset, m_ISPConfig.drvinfo.dip_dceso_size, planeOffset);
		}
		if (m_ISPConfig.drvinfo.dip_timgo_secure_tag != 0) // TIMGO
		{
			secureRegAddr[0] = 0x15021260;
			secureRegAddr[1] = 0;
			secureRegAddr[2] = 0;
			planeOffset[0] = 0;
			planeOffset[1] = 0;
			planeOffset[2] = 0;
			flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_TIMGO);
			command.secure(CMDQ_ENG_ISP_TIMGO, flag, secureRegAddr, (DpSecure)m_ISPConfig.drvinfo.dip_timgo_secure_tag, m_ISPConfig.drvinfo.dip_timgo_base_addr, offset, m_ISPConfig.drvinfo.dip_timgo_size, planeOffset);
		}
		if (m_ISPConfig.drvinfo.dip_imgi_secure_tag != 0) // IMGI (Separate secure setting for each plane due to BPCI and LSCI)
		{
			secureRegAddr[0] = 0x15021100;
			secureRegAddr[1] = 0;
			secureRegAddr[2] = 0;
			planeOffset[0] = 0;
			planeOffset[1] = 0;
			planeOffset[2] = 0;
			flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_IMGI);
			command.secure(CMDQ_ENG_ISP_IMGI, flag, secureRegAddr, (DpSecure)m_ISPConfig.drvinfo.dip_imgi_secure_tag, m_ISPConfig.drvinfo.dip_imgi_base_addr, offset, m_ISPConfig.drvinfo.dip_imgi_size, planeOffset);
			if(m_ISPConfig.top.bpc_en == 1)
			{
				secureRegAddr[0] = 0x15021200;
				secureRegAddr[1] = 0;
				secureRegAddr[2] = 0;
				planeOffset[0] = 0;
				planeOffset[1] = 0;
				planeOffset[2] = 0;
				flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_IMGCI);
				command.secure(CMDQ_ENG_ISP_IMGCI, flag, secureRegAddr, (DpSecure)m_ISPConfig.drvinfo.dip_imgbi_secure_tag, m_ISPConfig.drvinfo.dip_imgbi_base_addr, offset, m_ISPConfig.drvinfo.dip_imgbi_size, planeOffset);
			}
			else if(m_ISPConfig.drvinfo.dip_imgbi_secure_tag != 0)
			{
				secureRegAddr[0] = 0x15021200;
				secureRegAddr[1] = 0;
				secureRegAddr[2] = 0;
				planeOffset[0] = m_ISPConfig.drvinfo.dip_imgbi_base_addr;
				planeOffset[1] = 0;
				planeOffset[2] = 0;
				flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_IMGCI);
				command.secure(CMDQ_ENG_ISP_IMGCI, flag, secureRegAddr, (DpSecure)m_ISPConfig.drvinfo.dip_imgi_secure_tag, m_ISPConfig.drvinfo.dip_imgi_base_addr, offset, m_ISPConfig.drvinfo.dip_imgbi_size, planeOffset);
			}
			if(m_ISPConfig.top.lsc_en == 1)
			{
				secureRegAddr[0] = 0x15021230;
				secureRegAddr[1] = 0;
				secureRegAddr[2] = 0;
				planeOffset[0] = 0;
				planeOffset[1] = 0;
				planeOffset[2] = 0;
				flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_IMGCI);
				command.secure(CMDQ_ENG_ISP_IMGCI, flag, secureRegAddr, (DpSecure)m_ISPConfig.drvinfo.dip_imgci_secure_tag, m_ISPConfig.drvinfo.dip_imgci_base_addr, offset, m_ISPConfig.drvinfo.dip_imgci_size, planeOffset);
			}
			else if(m_ISPConfig.drvinfo.dip_imgci_secure_tag != 0)
			{
				secureRegAddr[0] = 0x15021230;
				secureRegAddr[1] = 0;
				secureRegAddr[2] = 0;
				planeOffset[0] = m_ISPConfig.drvinfo.dip_imgci_base_addr;
				planeOffset[1] = 0;
				planeOffset[2] = 0;
				flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_IMGCI);
				command.secure(CMDQ_ENG_ISP_IMGCI, flag, secureRegAddr, (DpSecure)m_ISPConfig.drvinfo.dip_imgi_secure_tag, m_ISPConfig.drvinfo.dip_imgi_base_addr, offset, m_ISPConfig.drvinfo.dip_imgci_size, planeOffset);
			}
		}
		if (m_ISPConfig.drvinfo.dip_vipi_secure_tag != 0) // VIPI
		{
			secureRegAddr[0] = 0x15021700;
			secureRegAddr[1] = 0x15021730;
			secureRegAddr[2] = 0x15021760;
			planeOffset[0] = 0;
			planeOffset[1] = m_ISPConfig.drvinfo.dip_vip2i_base_addr;
			planeOffset[2] = m_ISPConfig.drvinfo.dip_vip3i_base_addr;
			flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_VIPI);
			command.secure(CMDQ_ENG_ISP_VIPI, flag, secureRegAddr, (DpSecure)m_ISPConfig.drvinfo.dip_vipi_secure_tag, m_ISPConfig.drvinfo.dip_vipi_base_addr, offset, m_ISPConfig.drvinfo.dip_vipi_size, planeOffset);
		}
		if (m_ISPConfig.drvinfo.dip_dmgi_secure_tag != 0) // DMGI
		{
			secureRegAddr[0] = 0x15021370;
			secureRegAddr[1] = 0;
			secureRegAddr[2] = 0;
			planeOffset[0] = 0;
			planeOffset[1] = 0;
			planeOffset[2] = 0;
			flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_DMGI_DEPI);
			command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, (DpSecure)m_ISPConfig.drvinfo.dip_dmgi_secure_tag, m_ISPConfig.drvinfo.dip_dmgi_base_addr, offset, m_ISPConfig.drvinfo.dip_dmgi_size, planeOffset);
		}
		if (m_ISPConfig.drvinfo.dip_depi_secure_tag != 0) // DEPI
		{
			secureRegAddr[0] = 0x150213A0;
			secureRegAddr[1] = 0;
			secureRegAddr[2] = 0;
			planeOffset[0] = 0;
			planeOffset[1] = 0;
			planeOffset[2] = 0;
			flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_DMGI_DEPI);
			command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, (DpSecure)m_ISPConfig.drvinfo.dip_depi_secure_tag, m_ISPConfig.drvinfo.dip_depi_base_addr, offset, m_ISPConfig.drvinfo.dip_depi_size, planeOffset);
		}
		if (m_ISPConfig.drvinfo.dip_lcei_secure_tag != 0) // LCEI
		{
			secureRegAddr[0] = 0x150213D0;
			secureRegAddr[1] = 0;
			secureRegAddr[2] = 0;
			planeOffset[0] = 0;
			planeOffset[1] = 0;
			planeOffset[2] = 0;
			flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_LCEI);
			command.secure(CMDQ_ENG_ISP_LCEI, flag, secureRegAddr, (DpSecure)m_ISPConfig.drvinfo.dip_lcei_secure_tag, m_ISPConfig.drvinfo.dip_lcei_base_addr, offset, m_ISPConfig.drvinfo.dip_lcei_size, planeOffset);
		}
		if (m_ISPConfig.drvinfo.dip_ufdi_secure_tag != 0) // UFDI
		{
			secureRegAddr[0] = 0x15021130;
			secureRegAddr[1] = 0;
			secureRegAddr[2] = 0;
			planeOffset[0] = 0;
			planeOffset[1] = 0;
			planeOffset[2] = 0;
			flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_DMGI_DEPI);
			command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, (DpSecure)m_ISPConfig.drvinfo.dip_ufdi_secure_tag, m_ISPConfig.drvinfo.dip_ufdi_base_addr, offset, m_ISPConfig.drvinfo.dip_ufdi_size, planeOffset);
		}

		// SMX1
		secureRegAddr[0] = 0x15021160;
		secureRegAddr[1] = 0;
		secureRegAddr[2] = 0;
		planeOffset[0] = m_ISPConfig.drvinfo.smx1iOft;
		planeOffset[1] = 0;
		planeOffset[2] = 0;
		memSize[0] = m_ISPConfig.drvinfo.smxSecSize - m_ISPConfig.drvinfo.smx1iOft;
		flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_SMXIO);
		command.secure(CMDQ_ENG_ISP_SMXIO, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.smxSecHdl, offset, memSize, planeOffset);
		secureRegAddr[0] = 0x15021190;
		command.secure(CMDQ_ENG_ISP_SMXIO, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.smxSecHdl, offset, memSize, planeOffset);
		// SMX2
		secureRegAddr[0] = 0x15021470;
		planeOffset[0] = m_ISPConfig.drvinfo.smx2iOft;
		memSize[0] = m_ISPConfig.drvinfo.smxSecSize - m_ISPConfig.drvinfo.smx2iOft;
		command.secure(CMDQ_ENG_ISP_SMXIO, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.smxSecHdl, offset, memSize, planeOffset);
		secureRegAddr[0] = 0x150214A0;
		command.secure(CMDQ_ENG_ISP_SMXIO, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.smxSecHdl, offset, memSize, planeOffset);
		// SMX3
		secureRegAddr[0] = 0x15021510;
		planeOffset[0] = m_ISPConfig.drvinfo.smx3iOft;
		memSize[0] = m_ISPConfig.drvinfo.smxSecSize - m_ISPConfig.drvinfo.smx3iOft;
		command.secure(CMDQ_ENG_ISP_SMXIO, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.smxSecHdl, offset, memSize, planeOffset);
		secureRegAddr[0] = 0x15021540;
		command.secure(CMDQ_ENG_ISP_SMXIO, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.smxSecHdl, offset, memSize, planeOffset);
		// SMX4
		secureRegAddr[0] = 0x150212D0;
		planeOffset[0] = m_ISPConfig.drvinfo.smx4iOft;
		memSize[0] = m_ISPConfig.drvinfo.smxSecSize - m_ISPConfig.drvinfo.smx4iOft;
		command.secure(CMDQ_ENG_ISP_SMXIO, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.smxSecHdl, offset, memSize, planeOffset);
		secureRegAddr[0] = 0x15021300;
		command.secure(CMDQ_ENG_ISP_SMXIO, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.smxSecHdl, offset, memSize, planeOffset);
		flag = (uint64_t)(1LL<<CMDQ_ENG_ISP_DMGI_DEPI); // For CQ DMA

		switch (m_ISPConfig.drvinfo.cqIdx) {
			case ISP_DRV_DIP_CQ_THRE0:
				secureRegAddr[0] = 0x15022208;
				planeOffset[0] = m_ISPConfig.drvinfo.DesCqOft;
				memSize[0] = m_ISPConfig.drvinfo.cqSecSize - m_ISPConfig.drvinfo.DesCqOft;
				command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.cqSecHdl, offset, memSize, planeOffset);
				break;
			case ISP_DRV_DIP_CQ_THRE1:
				secureRegAddr[0] = 0x15022220;
				planeOffset[0] = m_ISPConfig.drvinfo.DesCqOft;
				memSize[0] = m_ISPConfig.drvinfo.cqSecSize - m_ISPConfig.drvinfo.DesCqOft;
				command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.cqSecHdl, offset, memSize, planeOffset);
				break;
			case ISP_DRV_DIP_CQ_THRE2:
				secureRegAddr[0] = 0x1502222C;
				planeOffset[0] = m_ISPConfig.drvinfo.DesCqOft;
				memSize[0] = m_ISPConfig.drvinfo.cqSecSize - m_ISPConfig.drvinfo.DesCqOft;
				command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.cqSecHdl, offset, memSize, planeOffset);
				break;
			case ISP_DRV_DIP_CQ_THRE3:
				secureRegAddr[0] = 0x15022238;
				planeOffset[0] = m_ISPConfig.drvinfo.DesCqOft;
				memSize[0] = m_ISPConfig.drvinfo.cqSecSize - m_ISPConfig.drvinfo.DesCqOft;
				command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.cqSecHdl, offset, memSize, planeOffset);
				break;
			case ISP_DRV_DIP_CQ_THRE4:
				secureRegAddr[0] = 0x15022244;
				planeOffset[0] = m_ISPConfig.drvinfo.DesCqOft;
				memSize[0] = m_ISPConfig.drvinfo.cqSecSize - m_ISPConfig.drvinfo.DesCqOft;
				command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.cqSecHdl, offset, memSize, planeOffset);
				break;
			case ISP_DRV_DIP_CQ_THRE5:
				secureRegAddr[0] = 0x15022250;
				planeOffset[0] = m_ISPConfig.drvinfo.DesCqOft;
				memSize[0] = m_ISPConfig.drvinfo.cqSecSize - m_ISPConfig.drvinfo.DesCqOft;
				command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.cqSecHdl, offset, memSize, planeOffset);
				break;
			case ISP_DRV_DIP_CQ_THRE6:
				secureRegAddr[0] = 0x1502225C;
				planeOffset[0] = m_ISPConfig.drvinfo.DesCqOft;
				memSize[0] = m_ISPConfig.drvinfo.cqSecSize - m_ISPConfig.drvinfo.DesCqOft;
				command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.cqSecHdl, offset, memSize, planeOffset);
				break;
			case ISP_DRV_DIP_CQ_THRE7:
				secureRegAddr[0] = 0x15022268;
				planeOffset[0] = m_ISPConfig.drvinfo.DesCqOft;
				memSize[0] = m_ISPConfig.drvinfo.cqSecSize - m_ISPConfig.drvinfo.DesCqOft;
				command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.cqSecHdl, offset, memSize, planeOffset);
				break;
			case ISP_DRV_DIP_CQ_THRE8:
				secureRegAddr[0] = 0x15022274;
				planeOffset[0] = m_ISPConfig.drvinfo.DesCqOft;
				memSize[0] = m_ISPConfig.drvinfo.cqSecSize - m_ISPConfig.drvinfo.DesCqOft;
				command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.cqSecHdl, offset, memSize, planeOffset);
				break;
			case ISP_DRV_DIP_CQ_THRE9:
				secureRegAddr[0] = 0x15022280;
				planeOffset[0] = m_ISPConfig.drvinfo.DesCqOft;
				memSize[0] = m_ISPConfig.drvinfo.cqSecSize - m_ISPConfig.drvinfo.DesCqOft;
				command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.cqSecHdl, offset, memSize, planeOffset);
				break;
			case ISP_DRV_DIP_CQ_THRE10:
				secureRegAddr[0] = 0x1502228C;
				planeOffset[0] = m_ISPConfig.drvinfo.DesCqOft;
				memSize[0] = m_ISPConfig.drvinfo.cqSecSize - m_ISPConfig.drvinfo.DesCqOft;
				command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.cqSecHdl, offset, memSize, planeOffset);
				break;
			case ISP_DRV_DIP_CQ_THRE11:
				secureRegAddr[0] = 0x15022298;
				planeOffset[0] = m_ISPConfig.drvinfo.DesCqOft;
				memSize[0] = m_ISPConfig.drvinfo.cqSecSize - m_ISPConfig.drvinfo.DesCqOft;
				command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.cqSecHdl, offset, memSize, planeOffset);
				break;
			case ISP_DRV_DIP_CQ_THRE12:
				secureRegAddr[0] = 0x150222A4;
				planeOffset[0] = m_ISPConfig.drvinfo.DesCqOft;
				memSize[0] = m_ISPConfig.drvinfo.cqSecSize - m_ISPConfig.drvinfo.DesCqOft;
				command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.cqSecHdl, offset, memSize, planeOffset);
				break;
			case ISP_DRV_DIP_CQ_THRE13:
				secureRegAddr[0] = 0x150222B0;
				planeOffset[0] = m_ISPConfig.drvinfo.DesCqOft;
				memSize[0] = m_ISPConfig.drvinfo.cqSecSize - m_ISPConfig.drvinfo.DesCqOft;
				command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.cqSecHdl, offset, memSize, planeOffset);
				break;
			case ISP_DRV_DIP_CQ_THRE14:
				secureRegAddr[0] = 0x150222BC;
				planeOffset[0] = m_ISPConfig.drvinfo.DesCqOft;
				memSize[0] = m_ISPConfig.drvinfo.cqSecSize - m_ISPConfig.drvinfo.DesCqOft;
				command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.cqSecHdl, offset, memSize, planeOffset);
				break;
			default:
				DPLOGE("[Error]not support this cqIdx(%d)\n", m_ISPConfig.drvinfo.cqIdx);
				break;
		}
	}
#endif

	// DIP settings for GCE cmd
	MM_REG_WRITE_MASK(command, 0x15022010, m_ISPConfig.drvinfo.dip_ctl_rgb_en, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022014, m_ISPConfig.drvinfo.dip_ctl_rgb2_en, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022018, m_ISPConfig.drvinfo.dip_ctl_yuv_en, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x1502201C, m_ISPConfig.drvinfo.dip_ctl_yuv2_en, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022020, m_ISPConfig.drvinfo.dip_ctl_dma_en, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022024, m_ISPConfig.drvinfo.dip_ctl_dma2_en, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022040, m_ISPConfig.drvinfo.dip_ctl_mux_sel, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022044, m_ISPConfig.drvinfo.dip_ctl_mux2_sel, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022050, m_ISPConfig.drvinfo.dip_ctl_fmt_sel, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022054, m_ISPConfig.drvinfo.dip_ctl_fmt2_sel, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022058, m_ISPConfig.drvinfo.dip_ctl_misc_sel, 0xFFFFFFFF, &m_frameConfigLabel[index++]);

	MM_REG_WRITE_MASK(command, 0x150215B0, m_ISPConfig.drvinfo.dip_img2o_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021620, m_ISPConfig.drvinfo.dip_img2bo_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021790, m_ISPConfig.drvinfo.dip_img3o_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021800, m_ISPConfig.drvinfo.dip_img3bo_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021870, m_ISPConfig.drvinfo.dip_img3co_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021690, m_ISPConfig.drvinfo.dip_feo_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021400, m_ISPConfig.drvinfo.dip_dceso_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021260, m_ISPConfig.drvinfo.dip_timgo_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);

	MM_REG_WRITE_MASK(command, 0x15021100, m_ISPConfig.drvinfo.dip_imgi_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021200, m_ISPConfig.drvinfo.dip_imgbi_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021230, m_ISPConfig.drvinfo.dip_imgci_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021700, m_ISPConfig.drvinfo.dip_vipi_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021730, m_ISPConfig.drvinfo.dip_vip2i_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021760, m_ISPConfig.drvinfo.dip_vip3i_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021370, m_ISPConfig.drvinfo.dip_dmgi_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x150213A0, m_ISPConfig.drvinfo.dip_depi_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x150213D0, m_ISPConfig.drvinfo.dip_lcei_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021130, m_ISPConfig.drvinfo.dip_ufdi_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    //DIP_X_SMX1I_BASE_ADDR, DIP_X_SMX1O_BASE_ADDR
    MM_REG_WRITE_MASK(command, 0x15021160, m_ISPConfig.drvinfo.smx1iPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    MM_REG_WRITE_MASK(command, 0x15021190, m_ISPConfig.drvinfo.smx1iPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    //DIP_X_SMX2I_BASE_ADDR, DIP_X_SMX2O_BASE_ADDR
    MM_REG_WRITE_MASK(command, 0x15021470, m_ISPConfig.drvinfo.smx2iPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    MM_REG_WRITE_MASK(command, 0x150214A0, m_ISPConfig.drvinfo.smx2iPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    //DIP_X_SMX3I_BASE_ADDR, DIP_X_SMX3O_BASE_ADDR
    MM_REG_WRITE_MASK(command, 0x15021510, m_ISPConfig.drvinfo.smx3iPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    MM_REG_WRITE_MASK(command, 0x15021540, m_ISPConfig.drvinfo.smx3iPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    //DIP_X_SMX4I_BASE_ADDR, DIP_X_SMX4O_BASE_ADDR
    MM_REG_WRITE_MASK(command, 0x150212D0, m_ISPConfig.drvinfo.smx4iPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    MM_REG_WRITE_MASK(command, 0x15021300, m_ISPConfig.drvinfo.smx4iPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    if (m_ISPConfig.top.mfb_en) {

        MFB_Config *pMfbConfig = (MFB_Config *)m_ISPConfig.drvinfo.mfbcommand;

    	MM_REG_WRITE_MASK(command, 0x15010000, pMfbConfig->C02_CON, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010040, pMfbConfig->SRZ_CONTROL, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010044, pMfbConfig->SRZ_IN_IMG, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010048, pMfbConfig->SRZ_OUT_IMG, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x1501004C, pMfbConfig->SRZ_HORI_STEP, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010050, pMfbConfig->SRZ_VERT_STEP, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010054, pMfbConfig->SRZ_HORI_INT_OFST, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010058, pMfbConfig->SRZ_HORI_SUB_OFST, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x1501005C, pMfbConfig->SRZ_VERT_INT_OFST, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010060, pMfbConfig->SRZ_VERT_SUB_OFST, 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    	MM_REG_WRITE_MASK(command, 0x15010080, pMfbConfig->CRSP_CTRL, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010084, pMfbConfig->CRSP_OUT_IMG, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010088, pMfbConfig->CRSP_STEP_OFST, 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    	MM_REG_WRITE_MASK(command, 0x15010100, pMfbConfig->OMC_TOP, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010108, pMfbConfig->OMC_FRAME_SIZE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x1501010C, pMfbConfig->OMC_TILE_EDGE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010110, pMfbConfig->OMC_TILE_OFS, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010114, pMfbConfig->OMC_TILE_SIZE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010120, pMfbConfig->OMC_MV_RDMA_BASE_ADDR, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010124, pMfbConfig->OMC_MV_RDMA_STRIDE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x1501018C, pMfbConfig->OMCC_OMC_C_ADDR_GEN_BASE_ADDR_0, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010194, pMfbConfig->OMCC_OMC_C_ADDR_GEN_STRIDE_0, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010198, pMfbConfig->OMCC_OMC_C_ADDR_GEN_BASE_ADDR_1, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x150101A0, pMfbConfig->OMCC_OMC_C_ADDR_GEN_STRIDE_1, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x150101B4, pMfbConfig->OMCC_OMC_C_CTL_FMT_SEL, 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    	MM_REG_WRITE_MASK(command, 0x15010200, pMfbConfig->MFB_CON, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010204, pMfbConfig->MFB_LL_CON1, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010208, pMfbConfig->MFB_LL_CON2, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010214, pMfbConfig->MFB_EDGE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010218, pMfbConfig->MFB_LL_CON5, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x1501021C, pMfbConfig->MFB_LL_CON6, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010220, pMfbConfig->MFB_LL_CON7, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010224, pMfbConfig->MFB_LL_CON8, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010228, pMfbConfig->MFB_LL_CON9, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x1501022C, pMfbConfig->MFB_LL_CON10, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010230, pMfbConfig->MFB_MBD_CON0, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010234, pMfbConfig->MFB_MBD_CON1, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010238, pMfbConfig->MFB_MBD_CON2, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x1501023C, pMfbConfig->MFB_MBD_CON3, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010240, pMfbConfig->MFB_MBD_CON4, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010244, pMfbConfig->MFB_MBD_CON5, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010248, pMfbConfig->MFB_MBD_CON6, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x1501024C, pMfbConfig->MFB_MBD_CON7, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010250, pMfbConfig->MFB_MBD_CON8, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010254, pMfbConfig->MFB_MBD_CON9, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010258, pMfbConfig->MFB_MBD_CON10, 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    	MM_REG_WRITE_MASK(command, 0x1501025C, pMfbConfig->MFB_MFB_TOP_CFG0, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010264, pMfbConfig->MFB_MFB_TOP_CFG2, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	//MM_REG_WRITE_MASK(command, 0x15010268, 0x1, 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    	MM_REG_WRITE_MASK(command, 0x15010404, pMfbConfig->MFBDMA_TDRI_BASE_ADDR, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x1501040C, pMfbConfig->MFBDMA_TDRI_XSIZE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010420, pMfbConfig->MFBDMA_MFBO_BASE_ADDR, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010428, (pMfbConfig->MFBDMA_MFBO_STRIDE & 0xFFFF)-1, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x1501042C, pMfbConfig->MFBDMA_MFBO_YSIZE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010430, pMfbConfig->MFBDMA_MFBO_STRIDE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    	MM_REG_WRITE_MASK(command, 0x15010444, pMfbConfig->MFBDMA_MFB2O_BASE_ADDR, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x1501044C, (pMfbConfig->MFBDMA_MFB2O_STRIDE & 0xFFFF)-1, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010450, pMfbConfig->MFBDMA_MFB2O_YSIZE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010454, pMfbConfig->MFBDMA_MFB2O_STRIDE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    	MM_REG_WRITE_MASK(command, 0x15010468, pMfbConfig->MFBDMA_MFBI_BASE_ADDR, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010470, (pMfbConfig->MFBDMA_MFBI_STRIDE & 0xFFFF)-1, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010474, pMfbConfig->MFBDMA_MFBI_YSIZE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010478, pMfbConfig->MFBDMA_MFBI_STRIDE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    	MM_REG_WRITE_MASK(command, 0x150104A8, pMfbConfig->MFBDMA_MFB3I_BASE_ADDR, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x150104B0, (pMfbConfig->MFBDMA_MFB3I_STRIDE & 0xFFFF)-1, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x150104B4, pMfbConfig->MFBDMA_MFB3I_YSIZE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x150104B8, pMfbConfig->MFBDMA_MFB3I_STRIDE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    	MM_REG_WRITE_MASK(command, 0x150104C8, pMfbConfig->MFBDMA_MFB4I_BASE_ADDR, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x150104D0, (pMfbConfig->MFBDMA_MFB4I_STRIDE & 0xFFFF)-1, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x150104D4, pMfbConfig->MFBDMA_MFB4I_YSIZE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x150104D8, pMfbConfig->MFBDMA_MFB4I_STRIDE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    	MM_REG_WRITE_MASK(command, 0x1501052C, pMfbConfig->MFBDMA_MFBO_B_BASE_ADDR, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010534, (pMfbConfig->MFBDMA_MFBO_B_STRIDE & 0xFFFF)-1, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010538, pMfbConfig->MFBDMA_MFBO_B_YSIZE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x1501053C, pMfbConfig->MFBDMA_MFBO_B_STRIDE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    	MM_REG_WRITE_MASK(command, 0x15010550, pMfbConfig->MFBDMA_MFBI_B_BASE_ADDR, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010558, (pMfbConfig->MFBDMA_MFBI_B_STRIDE & 0xFFFF)-1, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x1501055C, pMfbConfig->MFBDMA_MFBI_B_YSIZE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010560, pMfbConfig->MFBDMA_MFBI_B_STRIDE, 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    	MM_REG_WRITE_MASK(command, 0x15010800, pMfbConfig->PAK_CONT_Y, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010810, pMfbConfig->PAK_CONT_C, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010820, pMfbConfig->UNP_OFST_Y, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010824, pMfbConfig->UNP_CONT_Y, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010830, pMfbConfig->UNP_OFST_C, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    	MM_REG_WRITE_MASK(command, 0x15010834, pMfbConfig->UNP_CONT_C, 0xFFFFFFFF, &m_frameConfigLabel[index++]);

        //MM_REG_WRITE_MASK(command, 0x1502004C, 0x00000000, 0x00000100, &m_frameConfigLabel[index++]); // DL P2 SEL = from MFB

    	//MM_REG_WRITE_MASK(command, 0x150102AC, 0x00001000, 0x00001000, &m_frameConfigLabel[index++]); // CRC Enable
        //MM_REG_WRITE_MASK(command, 0x150102AC, 0x00010000, 0x00010000, &m_frameConfigLabel[index++]); // CRC Clear
        //MM_REG_WRITE_MASK(command, 0x15010284, 0x00000002, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    }
    //MM_REG_WRITE_MASK(command, 0x15021100, m_ISPConfig.drvinfo.imgi_base_addr, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    switch (m_ISPConfig.drvinfo.cqIdx) {
        case ISP_DRV_DIP_CQ_THRE0:
            MM_REG_WRITE_MASK(command, 0x15022208, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE1:
            MM_REG_WRITE_MASK(command, 0x15022220, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE2:
            MM_REG_WRITE_MASK(command, 0x1502222C, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE3:
            MM_REG_WRITE_MASK(command, 0x15022238, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE4:
            MM_REG_WRITE_MASK(command, 0x15022244, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE5:
            MM_REG_WRITE_MASK(command, 0x15022250, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE6:
            MM_REG_WRITE_MASK(command, 0x1502225C, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE7:
            MM_REG_WRITE_MASK(command, 0x15022268, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE8:
            MM_REG_WRITE_MASK(command, 0x15022274, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE9:
            MM_REG_WRITE_MASK(command, 0x15022280, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE10:
            MM_REG_WRITE_MASK(command, 0x1502228C, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE11:
            MM_REG_WRITE_MASK(command, 0x15022298, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE12:
            MM_REG_WRITE_MASK(command, 0x150222A4, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE13:
            MM_REG_WRITE_MASK(command, 0x150222B0, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE14:
            MM_REG_WRITE_MASK(command, 0x150222BC, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
		// From CQ15 to CQ18, these do not connect to GCE +++++
		#if 0
        case ISP_DRV_DIP_CQ_THRE15:
            MM_REG_WRITE_MASK(command, 0x150221C8, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF);
            break;
        case ISP_DRV_DIP_CQ_THRE16:
            MM_REG_WRITE_MASK(command, 0x150221D4, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF);
            break;
        case ISP_DRV_DIP_CQ_THRE17:
            MM_REG_WRITE_MASK(command, 0x150221E0, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF);
            break;
        case ISP_DRV_DIP_CQ_THRE18:
            MM_REG_WRITE_MASK(command, 0x150221EC, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF);
            break;
		#endif
		// From CQ15 to CQ18, these do not connect to GCE -----
        default:
            DPLOGE("[Error]not support this cqIdx(%d)\n", m_ISPConfig.drvinfo.cqIdx);
            break;
    }
#endif // CONFIG_FOR_VERIFY_FPGA
    m_regCount = m_ISPConfig.drvinfo.regCount;

    if (m_regCount != 0)
    {
        m_pReadbackReg = m_ISPConfig.drvinfo.ReadAddrList;
        DPLOGI("DpWrapper_ISP: m_regCount = %d\n", m_regCount);
        DPLOGI("DpWrapper_ISP: m_pReadbackReg = %x\n", m_pReadbackReg[0]);
        DPLOGI("DpWrapper_ISP: m_PABuffer = 0x%x\n",m_PABuffer[0]);
    }


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


DP_STATUS_ENUM DpWrapper_ISP::onReconfigFrame(DpCommand &command,
                                           DpConfig  &config)
{
    DP_STATUS_ENUM status;
    int32_t        size;
    int32_t        index = 0;

    DPLOGI("[onConfigFrame]+,\n");

    memset(m_pTileDesc, 0x0, sizeof(ISP_TPIPE_DESCRIPTOR_STRUCT));
    //memset(m_pTileInfo, 0x0, MAX_TILE_TOT_NO * sizeof(ISP_TPIPE_INFORMATION_STRUCT));
    //memset(m_pTdrFlag, 0x0, ((MAX_TILE_TOT_NO + 31) >> 5) * sizeof(unsigned int));
    //memset(m_pIrqFlag, 0x0, ((MAX_TILE_TOT_NO + 31) >> 5) * sizeof(unsigned int));

    status = m_pInDataPort->getParameter(&m_ISPConfig, &size, &m_PABuffer[0]);
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
    m_TDRIBufPA_MFB = m_ISPConfig.drvinfo.tpipeTablePa_mfb;
#endif // CONFIG_FOR_VERIFY_FPGA
    assert(0 != m_TDRIBufPA);

#if CONFIG_FOR_VERIFY_FPGA
    m_pTDRIBufVA = (uint32_t*)m_pTDRIBufMem->mapSWAddress();
#else
    m_pTDRIBufVA = m_ISPConfig.drvinfo.tpipeTableVa;
    m_pTDRIBufVA_MFB = m_ISPConfig.drvinfo.tpipeTableVa_mfb;
#endif // CONFIG_FOR_VERIFY_FPGA
    assert(0 != m_pTDRIBufVA);

    // Initialize hex dump info
    m_pTileDesc->tpipe_config       = m_pTDRIBufVA;
    m_pTileDesc->tpipe_config_mfb   = m_pTDRIBufVA_MFB;
    m_pTileDesc->tpipe_info         = m_pTileInfo;
    m_pTileDesc->tpipe_info_mfb     = m_pTileInfo_MFB;
    m_pTileDesc->tdr_disable_flag   = m_pTdrFlag;
    m_pTileDesc->last_irq_flag      = m_pIrqFlag;
    m_pTileDesc->total_tpipe_no     = MAX_TILE_TOT_NO;
    m_pTileDesc->total_word_no      = MAX_ISP_TILE_TDR_HEX_NO;
    m_pTileDesc->total_word_no_mfb  = MAX_ISP_TILE_TDR_HEX_NO_MFB;

    m_ISPPassType = m_ISPConfig.drvinfo.cqIdx;
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

    if (0 == m_ISPConfig.top.mcrp_en)
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



	// DIP settings for GCE cmd
	MM_REG_WRITE_MASK(command, 0x15022010, m_ISPConfig.drvinfo.dip_ctl_rgb_en, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022014, m_ISPConfig.drvinfo.dip_ctl_rgb2_en, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022018, m_ISPConfig.drvinfo.dip_ctl_yuv_en, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x1502201C, m_ISPConfig.drvinfo.dip_ctl_yuv2_en, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022020, m_ISPConfig.drvinfo.dip_ctl_dma_en, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022024, m_ISPConfig.drvinfo.dip_ctl_dma2_en, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022040, m_ISPConfig.drvinfo.dip_ctl_mux_sel, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022044, m_ISPConfig.drvinfo.dip_ctl_mux2_sel, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022050, m_ISPConfig.drvinfo.dip_ctl_fmt_sel, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022054, m_ISPConfig.drvinfo.dip_ctl_fmt2_sel, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15022058, m_ISPConfig.drvinfo.dip_ctl_misc_sel, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);

	MM_REG_WRITE_MASK(command, 0x150215B0, m_ISPConfig.drvinfo.dip_img2o_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021620, m_ISPConfig.drvinfo.dip_img2bo_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021790, m_ISPConfig.drvinfo.dip_img3o_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021800, m_ISPConfig.drvinfo.dip_img3bo_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021870, m_ISPConfig.drvinfo.dip_img3co_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021690, m_ISPConfig.drvinfo.dip_feo_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021400, m_ISPConfig.drvinfo.dip_dceso_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021260, m_ISPConfig.drvinfo.dip_timgo_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);

	MM_REG_WRITE_MASK(command, 0x15021100, m_ISPConfig.drvinfo.dip_imgi_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021200, m_ISPConfig.drvinfo.dip_imgbi_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021230, m_ISPConfig.drvinfo.dip_imgci_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021700, m_ISPConfig.drvinfo.dip_vipi_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021730, m_ISPConfig.drvinfo.dip_vip2i_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021760, m_ISPConfig.drvinfo.dip_vip3i_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021370, m_ISPConfig.drvinfo.dip_dmgi_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x150213A0, m_ISPConfig.drvinfo.dip_depi_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x150213D0, m_ISPConfig.drvinfo.dip_lcei_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
	MM_REG_WRITE_MASK(command, 0x15021130, m_ISPConfig.drvinfo.dip_ufdi_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);

    //DIP_X_SMX1I_BASE_ADDR, DIP_X_SMX1O_BASE_ADDR
    MM_REG_WRITE_MASK(command, 0x15021160, m_ISPConfig.drvinfo.smx1iPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    MM_REG_WRITE_MASK(command, 0x15021190, m_ISPConfig.drvinfo.smx1iPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    //DIP_X_SMX2I_BASE_ADDR, DIP_X_SMX2O_BASE_ADDR
    MM_REG_WRITE_MASK(command, 0x15021470, m_ISPConfig.drvinfo.smx2iPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    MM_REG_WRITE_MASK(command, 0x150214A0, m_ISPConfig.drvinfo.smx2iPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    //DIP_X_SMX3I_BASE_ADDR, DIP_X_SMX3O_BASE_ADDR
    MM_REG_WRITE_MASK(command, 0x15021510, m_ISPConfig.drvinfo.smx3iPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    MM_REG_WRITE_MASK(command, 0x15021540, m_ISPConfig.drvinfo.smx3iPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    //DIP_X_SMX4I_BASE_ADDR, DIP_X_SMX4O_BASE_ADDR
    MM_REG_WRITE_MASK(command, 0x150212D0, m_ISPConfig.drvinfo.smx4iPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    MM_REG_WRITE_MASK(command, 0x15021300, m_ISPConfig.drvinfo.smx4iPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);

    //MM_REG_WRITE_MASK(command, 0x15021100, m_ISPConfig.drvinfo.imgi_base_addr, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    switch (m_ISPConfig.drvinfo.cqIdx) {
        case ISP_DRV_DIP_CQ_THRE0:
            MM_REG_WRITE_MASK(command, 0x15022208, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE1:
            MM_REG_WRITE_MASK(command, 0x15022220, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE2:
            MM_REG_WRITE_MASK(command, 0x1502222C, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE3:
            MM_REG_WRITE_MASK(command, 0x15022238, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE4:
            MM_REG_WRITE_MASK(command, 0x15022244, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE5:
            MM_REG_WRITE_MASK(command, 0x15022250, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE6:
            MM_REG_WRITE_MASK(command, 0x1502225C, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE7:
            MM_REG_WRITE_MASK(command, 0x15022268, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE8:
            MM_REG_WRITE_MASK(command, 0x15022274, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE9:
            MM_REG_WRITE_MASK(command, 0x15022280, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE10:
            MM_REG_WRITE_MASK(command, 0x1502228C, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE11:
            MM_REG_WRITE_MASK(command, 0x15022298, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE12:
            MM_REG_WRITE_MASK(command, 0x150222A4, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE13:
            MM_REG_WRITE_MASK(command, 0x150222B0, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE14:
            MM_REG_WRITE_MASK(command, 0x150221BC, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
		// From CQ15 to CQ18, these do not connect to GCE +++++
		#if 0
        case ISP_DRV_DIP_CQ_THRE15:
            MM_REG_WRITE_MASK(command, 0x150221C8, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF);
            break;
        case ISP_DRV_DIP_CQ_THRE16:
            MM_REG_WRITE_MASK(command, 0x150221D4, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF);
            break;
        case ISP_DRV_DIP_CQ_THRE17:
            MM_REG_WRITE_MASK(command, 0x150221E0, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF);
            break;
        case ISP_DRV_DIP_CQ_THRE18:
            MM_REG_WRITE_MASK(command, 0x150221EC, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF);
            break;
		#endif
		// From CQ15 to CQ18, these do not connect to GCE -----
        default:
            DPLOGE("[Error]not support this cqIdx(%d)\n", m_ISPConfig.drvinfo.cqIdx);
            break;
    }
#endif // CONFIG_FOR_VERIFY_FPGA
    m_regCount = m_ISPConfig.drvinfo.regCount;

    if (m_regCount != 0)
    {
        m_pReadbackReg = m_ISPConfig.drvinfo.ReadAddrList;
        DPLOGI("DpWrapper_ISP: m_regCount = %d\n", m_regCount);
        DPLOGI("DpWrapper_ISP: m_pReadbackReg = %x\n", m_pReadbackReg[0]);
        DPLOGI("DpWrapper_ISP: m_PABuffer = 0x%x\n",m_PABuffer[0]);
    }


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
    ISP_TILE_MESSAGE_ENUM result = ISP_MESSAGE_TILE_OK;

    /* tile core property */
    TILE_REG_MAP_STRUCT *ptr_tile_reg_map = p_tile_param->ptr_tile_reg_map;
    m_pTileFunc = p_tile_param->ptr_tile_func_param;

    assert(NULL == p_tile_param->ptr_isp_tile_descriptor);
    p_tile_param->ptr_isp_tile_descriptor = m_pTileDesc;

    DPLOGI("[initTilePath]+,\n");

#if !CONFIG_FOR_VERIFY_FPGA
    result = tile_copy_config_by_platform(p_tile_param, &m_ISPConfig);
    if (ISP_MESSAGE_TILE_OK != result)
    {
        DPLOGE("[Error]onConfigFrame: get ISP tile paramter failed,result(%d)\n",result);
        return DP_STATUS_INVALID_PARAX;
    }

    ptr_tile_reg_map->last_irq_mode = m_ISPConfig.sw.tpipe_irq_mode; // 0 FOR ISP ONLY,2 FOR MDP directtlink
    ptr_tile_reg_map->isp_tile_width  = m_ISPConfig.sw.tpipe_width;
    ptr_tile_reg_map->isp_tile_height = m_ISPConfig.sw.tpipe_height;

    m_ISPPassType = m_ISPConfig.drvinfo.cqIdx;

#else // CONFIG_FOR_VERIFY_FPGA
    //result = tile_main_read_isp_reg_file(ptr_tile_reg_map, "tile_reg_map_frame.txt", "tile_reg_map_frame_d.txt");
    result = tile_main_read_isp_reg_file(ptr_tile_reg_map, "tile_reg_map_frame.txt", "tile_reg_map_frame_d.txt", "tile_reg_map_frame_wpe.txt", "tile_reg_map_frame_wpe_d.txt", "tile_reg_map_frame_eaf.txt");
    if (ISP_MESSAGE_TILE_OK != result)
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
        if (TILE_FUNC_IMGI_D1_ID == ptr_func->func_num)
        {
            m_pIMGI = ptr_func;
        }

        switch(ptr_func->func_num)
        {
            case TILE_FUNC_IMGI_D1_ID:
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
            //case TILE_FUNC_PAK2O_ID:
            //    m_pISPDMA[TILE_FUNC_DMA_PAK2O] = ptr_func;
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


        if (TILE_FUNC_MCRP_D1_ID == ptr_func->func_num)
        {
            m_pMDPCrop = ptr_func;
        }
    }

#if !CONFIG_FOR_VERIFY_FPGA
    if (m_onlyISP && m_ISPConfig.top.mcrp_en)
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
    m_IspPmqosData.clear(); // clear m_IspPmqosData for ensuring correct PMQOS struct of tilemode

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_ISP::onRetrieveTileParam(struct TILE_PARAM_STRUCT* p_tile_param)
{
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
        m_IspPmqosData.clear(); // clear m_IspPmqosData for ensuring correct PMQOS struct of framemode

        if (m_ISPConfig.drvinfo.dmgi_data_size > 0)
        {
            m_IspPmqosData[SMI_PORT_DMGI_D1] += m_ISPConfig.drvinfo.dmgi_data_size;
        }
        if (m_ISPConfig.drvinfo.depi_data_size > 0)
        {
            m_IspPmqosData[SMI_PORT_UFDI_D1] += m_ISPConfig.drvinfo.depi_data_size;
        }
        if (m_ISPConfig.drvinfo.lcei_data_size > 0)
        {
            m_IspPmqosData[SMI_PORT_LCI_D1] += m_ISPConfig.drvinfo.lcei_data_size;
        }
        if (m_ISPConfig.drvinfo.timgo_data_size > 0)
        {
            m_IspPmqosData[SMI_PORT_UFOC2R] += m_ISPConfig.drvinfo.timgo_data_size;
        }
        return DP_STATUS_RETURN_SUCCESS;
    }

#if 0
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
                //case TILE_FUNC_DMA_PAK2O:
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
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->IMGI_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMGI_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_IMGI_D1] += (ptr_tile_reg_map->IMGI_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMGI_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->CTRL_IMGBI_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->IMGBI_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMGBI_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_IMGCI_D1] += (ptr_tile_reg_map->IMGBI_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMGBI_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->CTRL_IMGCI_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->IMGCI_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMGCI_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_IMGCI_D1] += (ptr_tile_reg_map->IMGCI_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMGCI_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->CTRL_UFDI_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->UFDI_TILE_XSIZE + 1) * (ptr_tile_reg_map->UFDI_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_UFDI_D1] += (ptr_tile_reg_map->UFDI_TILE_XSIZE + 1) * (ptr_tile_reg_map->UFDI_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->CTRL_VIPI_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->VIPI_TILE_XSIZE + 1) * (ptr_tile_reg_map->VIPI_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_UFOCW] += (ptr_tile_reg_map->VIPI_TILE_XSIZE + 1) * (ptr_tile_reg_map->VIPI_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->CTRL_VIPBI_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->VIPBI_TILE_XSIZE + 1) * (ptr_tile_reg_map->VIPBI_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_UFOCW] += (ptr_tile_reg_map->VIPBI_TILE_XSIZE + 1) * (ptr_tile_reg_map->VIPBI_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->CTRL_VIPCI_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->VIPCI_TILE_XSIZE + 1) * (ptr_tile_reg_map->VIPCI_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_UFOCW] += (ptr_tile_reg_map->VIPCI_TILE_XSIZE + 1) * (ptr_tile_reg_map->VIPCI_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->CTRL_LCEI_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->LCEI_TILE_XSIZE + 1) * (ptr_tile_reg_map->LCEI_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_LCI_D1] += (ptr_tile_reg_map->LCEI_TILE_XSIZE + 1) * (ptr_tile_reg_map->LCEI_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->CTRL_DEPI_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->DEPI_TILE_XSIZE + 1) * (ptr_tile_reg_map->DEPI_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_UFDI_D1] += (ptr_tile_reg_map->DEPI_TILE_XSIZE + 1) * (ptr_tile_reg_map->DEPI_TILE_XSIZE + 1);
    }
    if (ptr_tile_reg_map->CTRL_DMGI_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->DMGI_TILE_XSIZE + 1) * (ptr_tile_reg_map->DMGI_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_DMGI_D1] += (ptr_tile_reg_map->DMGI_TILE_XSIZE + 1) * (ptr_tile_reg_map->DMGI_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->CTRL_TIMGO_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->TIMGO_TILE_XSIZE + 1) * (ptr_tile_reg_map->TIMGO_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_UFOC2R] += (ptr_tile_reg_map->TIMGO_TILE_XSIZE + 1) * (ptr_tile_reg_map->TIMGO_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->CTRL_CRZO_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->CRZO_TILE_XSIZE + 1) * (ptr_tile_reg_map->CRZO_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_CRZO_D1] += (ptr_tile_reg_map->CRZO_TILE_XSIZE + 1) * (ptr_tile_reg_map->CRZO_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->CTRL_CRZBO_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->CRZBO_TILE_XSIZE + 1) * (ptr_tile_reg_map->CRZBO_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_CRZO_D1] += (ptr_tile_reg_map->CRZBO_TILE_XSIZE + 1) * (ptr_tile_reg_map->CRZBO_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->CTRL_IMG3O_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->IMG3O_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMG3O_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_UFOYW] += (ptr_tile_reg_map->IMG3O_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMG3O_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->CTRL_IMG3BO_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->IMG3BO_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMG3BO_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_UFOYW] += (ptr_tile_reg_map->IMG3BO_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMG3BO_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->CTRL_IMG3CO_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->IMG3CO_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMG3CO_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_UFOYW] += (ptr_tile_reg_map->IMG3CO_TILE_XSIZE + 1) * (ptr_tile_reg_map->IMG3CO_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->CTRL_FEO_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->FEO_TILE_XSIZE + 1) * (ptr_tile_reg_map->FEO_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_CRZO_D1] += (ptr_tile_reg_map->FEO_TILE_XSIZE + 1) * (ptr_tile_reg_map->FEO_TILE_YSIZE + 1);
    }
    //CHECK Dceso
    if (ptr_tile_reg_map->CTRL_DCESO_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->DCESO_XSIZE + 1) * (ptr_tile_reg_map->DCESO_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_UFOC2R] += (ptr_tile_reg_map->DCESO_XSIZE + 1) * (ptr_tile_reg_map->DCESO_YSIZE + 1);
    }
    //Smart Tile Part
    if (ptr_tile_reg_map->SMT1I_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->SMT1I_TILE_XSIZE + 1) * (ptr_tile_reg_map->SMT1I_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_SMTI_D1] += (ptr_tile_reg_map->SMT1I_TILE_XSIZE + 1) * (ptr_tile_reg_map->SMT1I_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->SMT1O_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->SMT1O_TILE_XSIZE + 1) * (ptr_tile_reg_map->SMT1O_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_SMTO_D1] += (ptr_tile_reg_map->SMT1O_TILE_XSIZE + 1) * (ptr_tile_reg_map->SMT1O_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->SMT2I_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->SMT2I_TILE_XSIZE + 1) * (ptr_tile_reg_map->SMT2I_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_SMTI_D1] += (ptr_tile_reg_map->SMT2I_TILE_XSIZE + 1) * (ptr_tile_reg_map->SMT2I_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->SMT2O_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->SMT2O_TILE_XSIZE + 1) * (ptr_tile_reg_map->SMT2O_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_SMTO_D2] += (ptr_tile_reg_map->SMT2O_TILE_XSIZE + 1) * (ptr_tile_reg_map->SMT2O_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->SMT3I_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->SMT3I_TILE_XSIZE + 1) * (ptr_tile_reg_map->SMT3I_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_SMTI_D1] += (ptr_tile_reg_map->SMT3I_TILE_XSIZE + 1) * (ptr_tile_reg_map->SMT3I_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->SMT3O_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->SMT3O_TILE_XSIZE + 1) * (ptr_tile_reg_map->SMT3O_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_SMTO_D2] += (ptr_tile_reg_map->SMT3O_TILE_XSIZE + 1) * (ptr_tile_reg_map->SMT3O_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->SMT4I_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->SMT4I_TILE_XSIZE + 1) * (ptr_tile_reg_map->SMT4I_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_SMTI_D1] += (ptr_tile_reg_map->SMT4I_TILE_XSIZE + 1) * (ptr_tile_reg_map->SMT4I_TILE_YSIZE + 1);
    }
    if (ptr_tile_reg_map->SMT4O_EN)
    {
        m_ISPConfig.drvinfo.total_data_size += (ptr_tile_reg_map->SMT4O_TILE_XSIZE + 1) * (ptr_tile_reg_map->SMT4O_TILE_YSIZE + 1);
        m_IspPmqosData[SMI_PORT_SMTO_D2] += (ptr_tile_reg_map->SMT4O_TILE_XSIZE + 1) * (ptr_tile_reg_map->SMT4O_TILE_YSIZE + 1);
    }

    m_ispDataSize = m_ISPConfig.drvinfo.total_data_size;

    if (m_onlyISP)
    {
        m_inFrameHeight = ptr_tile_reg_map->IMGI_TILE_YSIZE + 1;
        DPLOGD("DpWrapper_ISP: inFrameHeight = %d, total_data = %d\n", m_inFrameHeight, m_ISPConfig.drvinfo.total_data_size);
    }

    DPLOGI("DpWrapper_ISP: m_inTileAccumulation = %d\n", m_inTileAccumulation);
    DPLOGI("DpWrapper_ISP: m_outTileAccumulation = %d\n", m_outTileAccumulation);

    return DP_STATUS_RETURN_SUCCESS;
}

void MFB_dumpTPipeBuffer(unsigned int *startAddr)
{
    for (int word_num = 0; word_num < 9; word_num++) {
        if (word_num == 8) {
            DPLOGI("[%02d]:0x%08x 0x%08x 0x%08x\n",
                word_num*4,
                *((unsigned int *)(startAddr + 4*word_num)),
                *((unsigned int *)(startAddr + 4*word_num+1)),
                *((unsigned int *)(startAddr + 4*word_num+2)));
        } else {
            DPLOGI("[%02d]:0x%08x 0x%08x 0x%08x 0x%08x\n",
                word_num*4,
                *((unsigned int *)(startAddr + 4*word_num)),
                *((unsigned int *)(startAddr + 4*word_num+1)),
                *((unsigned int *)(startAddr + 4*word_num+2)),
                *((unsigned int *)(startAddr + 4*word_num+3)));
        }
    }
    DPLOGI("\n");
}

void DIP_dumpCQDBuffer(unsigned int *startAddr)
{
    for (int word_num = 0; word_num < 400; word_num++) {
        DPLOGI("RYAN: CQD[%02d]:0x%08x\n",
            word_num, *((unsigned int *)(startAddr + word_num)));
    }
    DPLOGI("\n");
}

DP_STATUS_ENUM DpWrapper_ISP::onConfigTile(DpCommand &command)
{
    uint32_t tpipePa, tpipePa_MFB;
    uint32_t i;

	uint32_t       secureRegAddr[3] = {0};
    uint32_t       offset[3] = {0};
    uint32_t       memSize[3] = {0};
    uint32_t       planeOffset[3] = {0};
    uint64_t       flag = (uint64_t)(1LL<< CMDQ_ENG_ISP_DMGI_DEPI);

#if CONFIG_FOR_VERIFY_FPGA
    MM_REG_WRITE_MASK(command, MDP_FAKE_ISP_CFG, ((m_TDRITileID&0xFF) << 24), 0xFF000000);
#endif

    tpipePa = m_TDRIBufPA + m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no*sizeof(uint32_t);

    DPLOGI("[onConfigTile]+,m_TDRITileID(%d),offset(0x%x),tpipePa(0x%08x),used_word_no(%d),P2cqIdx(%d),cqPa(0x%x)\n",
            m_TDRITileID, m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no,tpipePa,m_pTileDesc->used_word_no,
            m_ISPConfig.drvinfo.cqIdx,m_ISPConfig.drvinfo.DesCqPa);

    if (m_ISPConfig.top.mfb_en) {
        tpipePa_MFB = m_TDRIBufPA_MFB + m_TDRITileID * 160;
        MM_REG_WRITE_MASK(command, 0x15010404, tpipePa_MFB, 0xFFFFFFFF, &m_TDRITileLabel[m_TDRITileID]);
        DPLOGI("[onConfigTile]+,m_TDRITileID(%d),offset(0x%x),tpipePa_MFB(0x%08x),used_word_no(%d),P2cqIdx(%d),cqPa(0x%x)\n",
                m_TDRITileID, m_pTileDesc->tpipe_info_mfb[m_TDRITileID].dump_offset_no,tpipePa_MFB,m_pTileDesc->used_word_no_mfb,
                m_ISPConfig.drvinfo.cqIdx,m_ISPConfig.drvinfo.DesCqPa);
        //DIP_dumpCQDBuffer((unsigned int *)m_ISPConfig.drvinfo.DesCqVa);
        //MFB_dumpTPipeBuffer((unsigned int *)m_pTDRIBufVA_MFB);
        //MFB_dumpTPipeBuffer((unsigned int *)m_pTDRIBufVA_MFB + 40 * 1);
        //MFB_dumpTPipeBuffer((unsigned int *)m_pTDRIBufVA_MFB + 40 * 2);
    }

    #if 0
    for(i=0;i<m_pTileDesc->used_word_no;i+=5){
        DPLOGI("Offset(%d),VA(0x%08x),(0x%08x)(0x%08x)(0x%08x)(0x%08x)(0x%08x)",
            i+m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no,
            (m_pTDRIBufVA+i+m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no),
            *(m_pTDRIBufVA+i+0+m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no),
            *(m_pTDRIBufVA+i+1+m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no),
            *(m_pTDRIBufVA+i+2+m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no),
            *(m_pTDRIBufVA+i+3+m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no),
            *(m_pTDRIBufVA+i+4+m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no));
    }
    #endif

	if (m_ISPConfig.drvinfo.dip_secure_tag&0x1)
    {
        secureRegAddr[0] = 0x15021004;
        planeOffset[0] = m_ISPConfig.drvinfo.tpipeTableOft + m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no*sizeof(uint32_t);
        memSize[0] = m_ISPConfig.drvinfo.tpipeTableSecSize - planeOffset[0];
        command.secure(CMDQ_ENG_ISP_DMGI_DEPI, flag, secureRegAddr, DP_SECURE, m_ISPConfig.drvinfo.tpipeTableSecHdl, offset, memSize, planeOffset);
    }
    MM_REG_WRITE_MASK(command, 0x15021004, tpipePa, 0xFFFFFFFF, &m_TDRITileLabel[m_TDRITileID]);
    m_TDRITileID++;

#if CONFIG_FOR_VERIFY_FPGA
    uint32_t width = m_outTileXRight - m_outTileXLeft + 1;
    uint32_t height = m_outTileYBottom - m_outTileYTop + 1;
    MM_REG_WRITE_MASK(command, MDP_FAKE_ISP_CFG, (height << 12) + width, 0x00FFFFFF);
#endif // CONFIG_FOR_VERIFY_FPGA

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_ISP::onReconfigTiles(DpCommand &command)
{
    uint32_t tpipePa;
    uint32_t i;

    for (i = 0; i < m_TDRITileID; i++)
    {
        tpipePa = m_TDRIBufPA + m_pTileDesc->tpipe_info[i].dump_offset_no*sizeof(uint32_t);

        DPLOGI("[reConfigTile]+,m_TDRITileID(%d),offset(0x%x),tpipePa(0x%08x)\n", i, m_pTileDesc->tpipe_info[i].dump_offset_no,tpipePa);

        MM_REG_WRITE_MASK(command, 0x15021004, tpipePa, 0xFFFFFFFF, NULL, m_TDRITileLabel[i]);
    }

    for (i = 0; i < m_regCount; i++)
    {
        DPLOGI("[reConfigTile]: m_pReadbackReg = 0x%x, PABuffer = 0x%x\n", m_pReadbackReg[i], m_PABuffer[i]);
        MM_REG_READ(command, m_pReadbackReg[i], m_PABuffer[i], NULL, m_regLabel[i]);
    }

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpWrapper_ISP::onTilePostProc(DpCommand &command){
    //command.addMetLog("tileOverhead", m_inTileAccumulation * 100 / m_inFrameWidth -100);
    return DP_STATUS_RETURN_SUCCESS;
}
DP_STATUS_ENUM DpWrapper_ISP::onUpdatePMQOS(DpCommand &command, uint32_t& totalPixel, uint32_t& totalDataSzie, int32_t& engineType, struct timeval& endtime)
{
    DP_UNUSED(command);
    uint32_t        pixelCount = 0;

    if (m_ispPmqosUpdIdx >= m_IspPmqosData.size())
    {
        DPLOGW("DpWrapper_ISP: m_ispPmqosUpdIdx(%d) >= m_IspPmqosData.size()(%d)\n", m_ispPmqosUpdIdx, m_IspPmqosData.size());
        m_ispPmqosUpdIdx = 0;
        return DP_STATUS_RETURN_SUCCESS;
    }
    std::map<uint32_t, uint32_t>::iterator iter = m_IspPmqosData.begin();
    std::advance(iter, m_ispPmqosUpdIdx);

    engineType = iter->first;
    totalDataSzie = iter->second;
    m_ispPmqosUpdIdx++;

    pixelCount = m_inTileAccumulation * m_inFrameHeight;

    endtime = m_pInDataPort->getEndTime();

    if (pixelCount > totalPixel)
    {
        totalPixel = pixelCount;
    }

    DPLOGI("DpWrapper_ISP: m_inTileAccumulation = %d\n", m_inTileAccumulation);
    DPLOGI("DpWrapper_ISP: m_outTileAccumulation = %d\n", m_outTileAccumulation);
    DPLOGI("DpWrapper_ISP::updatePMQOS totalDataSzie = %d totalPixel = %d\n", totalDataSzie, totalPixel);

    if(m_IspPmqosData.size() > m_ispPmqosUpdIdx)
    {
        return (DP_STATUS_ENUM)5;
    }

    m_ispPmqosUpdIdx = 0;
    return DP_STATUS_RETURN_SUCCESS;
}
