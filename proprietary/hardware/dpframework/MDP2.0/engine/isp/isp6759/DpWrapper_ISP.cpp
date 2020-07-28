#include "DpWrapper_ISP.h"
#include "mdp_reg_rdma.h"
#include "isp_reg.h"
#include "mmsys_config.h"

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

    // Direct link
    if (m_ISPConfig.top.mdp_crop_en)
    {
        DPLOGI("DpWrapper_ISP: SW_RST\n");
        // Reset MDP_DL_ASYNC_TX
        // Bit  3: MDP_DL_ASYNC_TX / MDP_RELAY
        MM_REG_WRITE(command, MMSYS_SW0_RST_B,    0x0, 0x00000008);
        MM_REG_WRITE(command, MMSYS_SW0_RST_B, 1 << 3, 0x00000008);
        // Reset MDP_DL_ASYNC_RX
        // Bit  10: MDP_DL_ASYNC_RX
        MM_REG_WRITE(command, MMSYS_SW1_RST_B,     0x0, 0x00000400);
        MM_REG_WRITE(command, MMSYS_SW1_RST_B, 1 << 10, 0x00000400);
    }

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpWrapper_ISP::onWaitEvent(DpCommand &command)
{
    switch (m_ISPPassType) {
        case ISP_DRV_DIP_CQ_THRE0:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0001, 0x00000001);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_0_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE1:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0002, 0x00000002);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_1_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE2:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0004, 0x00000004);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_2_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE3:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0008, 0x00000008);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_3_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE4:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0010, 0x00000010);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_4_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE5:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0020, 0x00000020);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_5_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE6:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0040, 0x00000040);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_6_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE7:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0080, 0x00000080);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_7_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE8:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0100, 0x00000100);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_8_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE9:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0200, 0x00000200);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_9_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE10:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0400, 0x00000400);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_10_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE11:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x0800, 0x00000800);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_11_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE12:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x1000, 0x00001000);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_12_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE13:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x2000, 0x00002000);
            //
            MM_REG_WAIT(command, DpCommand::ISP_P2_13_DONE);
            break;
        case ISP_DRV_DIP_CQ_THRE14:
            MM_REG_WRITE_MASK(command, 0x15022000, 0x4000, 0x00004000);
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

    switch (m_ISPConfig.drvinfo.cqIdx) {
        case ISP_DRV_DIP_CQ_THRE0:
            MM_REG_WRITE_MASK(command, 0x15022108, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE1:
            MM_REG_WRITE_MASK(command, 0x15022114, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE2:
            MM_REG_WRITE_MASK(command, 0x15022120, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE3:
            MM_REG_WRITE_MASK(command, 0x1502212C, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE4:
            MM_REG_WRITE_MASK(command, 0x15022138, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE5:
            MM_REG_WRITE_MASK(command, 0x15022144, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE6:
            MM_REG_WRITE_MASK(command, 0x15022150, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE7:
            MM_REG_WRITE_MASK(command, 0x1502215C, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE8:
            MM_REG_WRITE_MASK(command, 0x15022168, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE9:
            MM_REG_WRITE_MASK(command, 0x15022174, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE10:
            MM_REG_WRITE_MASK(command, 0x15022180, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE11:
            MM_REG_WRITE_MASK(command, 0x1502218C, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE12:
            MM_REG_WRITE_MASK(command, 0x15022198, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE13:
            MM_REG_WRITE_MASK(command, 0x150221A4, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE14:
            MM_REG_WRITE_MASK(command, 0x150221B0, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, &m_frameConfigLabel[index++]);
            break;
		// From CQ15 to CQ18, these do not connect to GCE +++++
		#if 0
        case ISP_DRV_DIP_CQ_THRE15:
            MM_REG_WRITE_MASK(command, 0x150221BC, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF);
            break;
        case ISP_DRV_DIP_CQ_THRE16:
            MM_REG_WRITE_MASK(command, 0x150221C8, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF);
            break;
        case ISP_DRV_DIP_CQ_THRE17:
            MM_REG_WRITE_MASK(command, 0x150221D4, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF);
            break;
        case ISP_DRV_DIP_CQ_THRE18:
            MM_REG_WRITE_MASK(command, 0x150221E0, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF);
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

    switch (m_ISPConfig.drvinfo.cqIdx) {
        case ISP_DRV_DIP_CQ_THRE0:
            MM_REG_WRITE_MASK(command, 0x15022108, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE1:
            MM_REG_WRITE_MASK(command, 0x15022114, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE2:
            MM_REG_WRITE_MASK(command, 0x15022120, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE3:
            MM_REG_WRITE_MASK(command, 0x1502212C, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE4:
            MM_REG_WRITE_MASK(command, 0x15022138, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE5:
            MM_REG_WRITE_MASK(command, 0x15022144, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE6:
            MM_REG_WRITE_MASK(command, 0x15022150, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE7:
            MM_REG_WRITE_MASK(command, 0x1502215C, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE8:
            MM_REG_WRITE_MASK(command, 0x15022168, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE9:
            MM_REG_WRITE_MASK(command, 0x15022174, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE10:
            MM_REG_WRITE_MASK(command, 0x15022180, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE11:
            MM_REG_WRITE_MASK(command, 0x1502218C, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE12:
            MM_REG_WRITE_MASK(command, 0x15022198, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE13:
            MM_REG_WRITE_MASK(command, 0x150221A4, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
        case ISP_DRV_DIP_CQ_THRE14:
            MM_REG_WRITE_MASK(command, 0x150221B0, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
            break;
		// From CQ15 to CQ18, these do not connect to GCE +++++
		#if 0
        case ISP_DRV_DIP_CQ_THRE15:
            MM_REG_WRITE_MASK(command, 0x150221BC, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF);
            break;
        case ISP_DRV_DIP_CQ_THRE16:
            MM_REG_WRITE_MASK(command, 0x150221C8, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF);
            break;
        case ISP_DRV_DIP_CQ_THRE17:
            MM_REG_WRITE_MASK(command, 0x150221D4, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF);
            break;
        case ISP_DRV_DIP_CQ_THRE18:
            MM_REG_WRITE_MASK(command, 0x150221E0, m_ISPConfig.drvinfo.DesCqPa, 0xFFFFFFFF);
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
        if (TILE_FUNC_IMGI_ID == ptr_func->func_num)
        {
            m_pIMGI = ptr_func;
        }

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
    /* tile core property */
    FUNC_DESCRIPTION_STRUCT *ptr_tile_func_param = p_tile_param->ptr_tile_func_param;
    TILE_REG_MAP_STRUCT *ptr_tile_reg_map = p_tile_param->ptr_tile_reg_map;

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

    if ((ptr_tile_func_param != NULL) && (ptr_tile_reg_map != NULL))
    {
        m_tileXEndFlag = ptr_tile_func_param->func_list[ptr_tile_reg_map->first_func_en_no].h_end_flag;
        m_tileYEndFlag = ptr_tile_func_param->func_list[ptr_tile_reg_map->first_func_en_no].v_end_flag;
    }
    else //ISP frame mode case
    {
        m_tileXEndFlag = 1;
        m_tileYEndFlag = 1;
    }
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_ISP::onConfigTile(DpCommand &command)
{
    uint32_t tpipePa;
    uint32_t i;

#if CONFIG_FOR_VERIFY_FPGA
    MM_REG_WRITE_MASK(command, MDP_FAKE_ISP_CFG, ((m_TDRITileID&0xFF) << 24), 0xFF000000);
#endif

    tpipePa = m_TDRIBufPA + m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no*sizeof(uint32_t);

    DPLOGI("[onConfigTile]+,m_TDRITileID(%d),offset(0x%x),tpipePa(0x%08x),used_word_no(%d),P2cqIdx(%d),cqPa(0x%x)\n",
            m_TDRITileID, m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no,tpipePa,m_pTileDesc->used_word_no,
            m_ISPConfig.drvinfo.cqIdx,m_ISPConfig.drvinfo.DesCqPa);

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

    MM_REG_WRITE_MASK(command, 0x15022204, tpipePa, 0xFFFFFFFF, &m_TDRITileLabel[m_TDRITileID]);
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

        MM_REG_WRITE_MASK(command, 0x15022204, tpipePa, 0xFFFFFFFF, NULL, m_TDRITileLabel[i]);
    }

    for (i = 0; i < m_regCount; i++)
    {
        DPLOGI("[reConfigTile]: m_pReadbackReg = 0x%x, PABuffer = 0x%x\n", m_pReadbackReg[i], m_PABuffer[i]);
        MM_REG_READ(command, m_pReadbackReg[i], m_PABuffer[i], NULL, m_regLabel[i]);
    }

    return DP_STATUS_RETURN_SUCCESS;
}
