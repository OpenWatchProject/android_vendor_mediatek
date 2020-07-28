#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "mdp_reg_tdshp.h"
#include "tile_mdp_reg.h"
#include "DpPlatform.h"
#if CONFIG_FOR_VERIFY_FPGA
#include "ConfigInfo.h"
#endif

#if CONFIG_FOR_OS_ANDROID
#include <cutils/properties.h>
#include "cust_color.h"
#include "cust_tdshp.h"

#include "PQSessionManager.h"
#include <PQAlgorithmFactory.h>

#define _PQ_TDSHP_DEBUG_
//#define _PQ_ADL_DEBUG_
#define _PQ_DSHP_DEBUG_
//#define _PQ_TIME_DEBUG_

// define to print log for performance issue.
#define TDSHP_CONFIG_FRAME_THRES    ( 2)  // ms
//#define TDSHP_CALC_ADL_THRES        (  1)  // ms
//#define TDSHP_IOCTL_THRES           (  2)  // ms
//#define TDSHP_GET_PROP_THRES        (  1)  // ms

#endif // CONFIG_FOR_OS_ANDROID

//--------------------------------------------------------
// TDSHP driver engine
//--------------------------------------------------------
class DpEngine_TDSHP: public DpTileEngine
{
public:
    DpEngine_TDSHP(uint32_t identifier)
        : DpTileEngine(identifier),
          m_outHistXLeft(0),
          m_outHistYTop(0),
          m_lastOutVertical(0),
          m_lastOutHorizontal(0),
          m_prevPABufferIndex(0),
          m_pqSessionId(0),
          m_prevReadbackCount(0),
          m_HFGEnable(m_data.m_HFGEnable)
    {
        m_enableLog = DpDriver::getInstance()->getEnableLog();
        memset(m_regLabel, -1, sizeof(m_regLabel));
        m_pData = &m_data;
    }

    ~DpEngine_TDSHP()
    {
    }

private:
    MDP_TDSHP_DATA m_data;
    int32_t        m_outHistXLeft;
    int32_t        m_outHistYTop;
    int32_t        m_lastOutVertical;
    int32_t        m_lastOutHorizontal;

    int32_t        m_regLabel[MAX_NUM_READBACK_PA_BUFFER];

    uint32_t       m_prevPABufferIndex;
    uint32_t       m_prevReadbackCount;

    uint64_t       m_pqSessionId;

    bool           &m_HFGEnable;

    DP_STATUS_ENUM onInitEngine(DpCommand&);

    DP_STATUS_ENUM onDeInitEngine(DpCommand&);

    DP_STATUS_ENUM onConfigFrame(DpCommand&,
                                 DpConfig&);

    DP_STATUS_ENUM onReconfigFrame(DpCommand&,
                                   DpConfig&);

    DP_STATUS_ENUM onConfigTile(DpCommand&);

    DP_STATUS_ENUM onPostProc(DpCommand &command);

    DP_STATUS_ENUM onAdvanceTile(DpCommand&);

    int64_t onQueryFeature()
    {
        return eTDSHP;
    }

#if CONFIG_FOR_OS_ANDROID
    // TDSHP
    void resetSWReg(void);

    int32_t         m_enableLog;
#else
#if CONFIG_FOR_VERIFY_FPGA
    void onFPGATestConfigTDSHP(DpCommand &command, DpConfig &config);
#endif // CONFIG_FOR_VERIFY_FPGA


#endif // CONFIG_FOR_OS_ANDROID

    // ADL
    void ADL_resetLumaHist(DpCommand &command);

    DP_STATUS_ENUM onReconfigTiles(DpCommand &command);
};

// Register factory function
static DpEngineBase* TDSHP0Factory(DpEngineType type)
{
    if (tTDSHP0 == type)
    {
        return new DpEngine_TDSHP(0);
    }
    return NULL;
};

#ifdef tTDSHP1
// Register factory function
static DpEngineBase* TDSHP1Factory(DpEngineType type)
{
    if (tTDSHP1 == type)
    {
        return new DpEngine_TDSHP(1);
    }
    return NULL;
};
#endif

#ifdef tTDSHP2
// Register factory function
static DpEngineBase* TDSHP2Factory(DpEngineType type)
{
    if (tTDSHP2 == type)
    {
        return new DpEngine_TDSHP(2);
    }
    return NULL;
};
#endif

#ifdef tTDSHP3
// Register factory function
static DpEngineBase* TDSHP3Factory(DpEngineType type)
{
    if (tTDSHP3 == type)
    {
        return new DpEngine_TDSHP(3);
    }
    return NULL;
};
#endif

// Register factory function
EngineReg TDSHP0Reg(TDSHP0Factory);
#ifdef tTDSHP1
EngineReg TDSHP1Reg(TDSHP1Factory);
#endif
#ifdef tTDSHP2
EngineReg TDSHP2Reg(TDSHP2Factory);
#endif
#ifdef tTDSHP3
EngineReg TDSHP3Reg(TDSHP3Factory);
#endif

#ifdef MDP_VERSION_8173
static DpEngineBase* TDSHP1Factory(DpEngineType type)
{
    if (tTDSHP1 == type)
    {
        return new DpEngine_TDSHP(1);
    }
    return NULL;
};

EngineReg TDSHP1Reg(TDSHP1Factory);
#endif

DP_STATUS_ENUM DpEngine_TDSHP::onInitEngine(DpCommand &command)
{
    // Enable TDSHP
    //MM_REG_WRITE(command, MDP_TDSHP_00, 0x80000000, 0x80000000); // TDS_EN will be enabled by following steps.
    MM_REG_WRITE(command, MDP_TDSHP_CTRL, 0x00000001, 0x00000001);
    command.addMetLog("MDP_TDSHP__MDP_TDSHP_CTRL", 0x00000001);

    // Enable fifo
    MM_REG_WRITE(command, MDP_TDSHP_CFG, 0x00000002, 0x00000002);

    // reset LumaHist
    ADL_resetLumaHist(command);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_TDSHP::onDeInitEngine(DpCommand &command)
{
    // Disable fifo
    MM_REG_WRITE(command, MDP_TDSHP_CFG, 0x00000002, 0x00000002);

    // Disable TDSHP
    //MM_REG_WRITE(command, MDP_TDSHP_00, 0x00000000, 0x80000000);
    MM_REG_WRITE(command, MDP_TDSHP_CTRL, 0x00000000, 0x00000001);

    return DP_STATUS_RETURN_SUCCESS;
}

#if CONFIG_FOR_OS_ANDROID
#else
#if CONFIG_FOR_VERIFY_FPGA
void DpEngine_TDSHP::onFPGATestConfigTDSHP(DpCommand &command, DpConfig &config)
{
    tdshp_config &tdshp = *(tdshp_config*)config.pEngine_cfg;

    MM_REG_WRITE(command, MDP_TDSHP_00     , tdshp.tds_cor_gain<<0
                                             |tdshp.tds_gain_high<<8
                                             |tdshp.tds_gain_mid<<16
                                             |tdshp.tds_ink_sel<<24
                                             |tdshp.tds_adap_luma_bp<<28    // ADAP_LUMA_BP
                                             |tdshp.tds_bypass_high<<29
                                             |tdshp.tds_bypass_mid<<30
                                             |1<<31                         // TDS_EN
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_01     , tdshp.tds_limit_ratio<<0
                                             |tdshp.tds_gain<<4
                                             |tdshp.tds_cor_zero<<16
                                             |tdshp.tds_cor_thr<<24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_02     , tdshp.tds_cor_value<<8
                                             |tdshp.tds_bound<<16
                                             |tdshp.tds_limit<<24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_03     , tdshp.tds_sat_proc<<0
                                             |tdshp.tds_ac_lpf_coe<<8
                                             |tdshp.tds_clip_thr<<16
                                             |tdshp.tds_clip_ratio<<24
                                             |tdshp.tds_clip_en<<31
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_05     , tdshp.tds_ylev_p048<<0
                                             |tdshp.tds_ylev_p032<<8
                                             |tdshp.tds_ylev_p016<<16
                                             |tdshp.tds_ylev_p000<<24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_06     , tdshp.tds_ylev_p112<<0
                                             |tdshp.tds_ylev_p096<<8
                                             |tdshp.tds_ylev_p080<<16
                                             |tdshp.tds_ylev_p064<<24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_07     , tdshp.tds_ylev_p176<<0
                                             |tdshp.tds_ylev_p160<<8
                                             |tdshp.tds_ylev_p144<<16
                                             |tdshp.tds_ylev_p128<<24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_08     , tdshp.tds_ylev_p240<<0
                                             |tdshp.tds_ylev_p224<<8
                                             |tdshp.tds_ylev_p208<<16
                                             |tdshp.tds_ylev_p192<<24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_09     , tdshp.tds_ylev_en<<14
                                             |tdshp.tds_ylev_alpha<<16
                                             |tdshp.tds_ylev_p256<<24
                                            , 0xFFFFFFFF);

    // PBC1
    MM_REG_WRITE(command, MDP_PBC_00       , tdshp.pbc1_radius_r<<0
                                             |tdshp.pbc1_theta_r<<6
                                             |tdshp.pbc1_rslope_1<<12
                                             |tdshp.pbc1_gain<<22
                                             |tdshp.pbc1_lpf_en<<30
                                             |tdshp.pbc1_en<<31
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_PBC_01       , tdshp.pbc1_lpf_gain<<0
                                             |tdshp.pbc1_tslope<<6
                                             |tdshp.pbc1_radius_c<<16
                                             |tdshp.pbc1_theta_c<<24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_PBC_02       , tdshp.pbc1_edge_slope<<0
                                             |tdshp.pbc1_edge_thr<<8
                                             |tdshp.pbc1_edge_en<<14
                                             |tdshp.pbc1_conf_gain<<16
                                             |tdshp.pbc1_rslope<<22
                                            , 0xFFFFFFFF);
    // PBC2
    MM_REG_WRITE(command, MDP_PBC_03       , tdshp.pbc2_radius_r<<0
                                             |tdshp.pbc2_theta_r<<6
                                             |tdshp.pbc2_rslope_1<<12
                                             |tdshp.pbc2_gain<<22
                                             |tdshp.pbc2_lpf_en<<30
                                             |tdshp.pbc2_en<<31
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_PBC_04       , tdshp.pbc2_lpf_gain<<0
                                             |tdshp.pbc2_tslope<<6
                                             |tdshp.pbc2_radius_c<<16
                                             |tdshp.pbc2_theta_c<<24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_PBC_05       , tdshp.pbc2_edge_slope<<0
                                             |tdshp.pbc2_edge_thr<<8
                                             |tdshp.pbc2_edge_en<<14
                                             |tdshp.pbc2_conf_gain<<16
                                             |tdshp.pbc2_rslope<<22
                                            , 0xFFFFFFFF);
    // PBC3
    MM_REG_WRITE(command, MDP_PBC_06       , tdshp.pbc3_radius_r<<0
                                             |tdshp.pbc3_theta_r<<6
                                             |tdshp.pbc3_rslope_1<<12
                                             |tdshp.pbc3_gain<<22
                                             |tdshp.pbc3_lpf_en<<30
                                             |tdshp.pbc3_en<<31
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_PBC_07       , tdshp.pbc3_lpf_gain<<0
                                             |tdshp.pbc3_tslope<<6
                                             |tdshp.pbc3_radius_c<<16
                                             |tdshp.pbc3_theta_c<<24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_PBC_08       , tdshp.pbc3_edge_slope<<0
                                             |tdshp.pbc3_edge_thr<<8
                                             |tdshp.pbc3_edge_en<<14
                                             |tdshp.pbc3_conf_gain<<16
                                             |tdshp.pbc3_rslope<<22
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_Y_FTN_1_0_MAIN   , tdshp.y_ftn_0<<0
                                                 |tdshp.y_ftn_1<<15
                                                , 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_Y_FTN_3_2_MAIN   , tdshp.y_ftn_2<<0
                                                 |tdshp.y_ftn_3<<15
                                                , 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_Y_FTN_5_4_MAIN   , tdshp.y_ftn_4<<0
                                                 |tdshp.y_ftn_5<<15
                                                , 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_Y_FTN_7_6_MAIN   , tdshp.y_ftn_6<<0
                                                 |tdshp.y_ftn_7<<15
                                                , 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_Y_FTN_9_8_MAIN   , tdshp.y_ftn_8<<0
                                                 |tdshp.y_ftn_9<<15
                                                , 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_Y_FTN_11_10_MAIN , tdshp.y_ftn_10<<0
                                                 |tdshp.y_ftn_11<<15
                                                , 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_Y_FTN_13_12_MAIN , tdshp.y_ftn_12<<0
                                                 |tdshp.y_ftn_13<<15
                                                , 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_Y_FTN_15_14_MAIN , tdshp.y_ftn_14<<0
                                                 |tdshp.y_ftn_15<<15
                                                , 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_Y_FTN_17_16_MAIN , tdshp.y_ftn_16<<0
                                                , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_C_BOOST_MAIN     , tdshp.cboost_gain<<0
                                                 |tdshp.new_cboost_en<<12
                                                 |tdshp.new_cboost_lmt_l<<15
                                                 |tdshp.new_cboost_lmt_u<<23
                                                , 0x0000007F);
    MM_REG_WRITE(command, MDP_C_BOOST_MAIN_2   , tdshp.cboost_yoffset<<0
                                                 |tdshp.cboost_yoffset_sel<<15
                                                , 0x0000007F);

#ifdef TDSHP_1_1
    MM_REG_WRITE(command, MDP_DC_TWO_D_W1, 0x40106051, 0xFFFFFFFF); //color hist
    MM_REG_WRITE(command, MDP_TDSHP_CFG, 0x00000080, 0x00000080);  // enable color_hist_en
#endif

    MM_REG_WRITE(command, MDP_TDSHP_CFG, 0x00000000, 0x00000001);  // disable relay mode
}
#endif // CONFIG_FOR_VERIFY_FPGA
#endif // CONFIG_FOR_OS_ANDROID

DP_STATUS_ENUM DpEngine_TDSHP::onConfigFrame(DpCommand &command, DpConfig &config)
{
#if CONFIG_FOR_OS_ANDROID
    bool shp_enable = false;
    bool adl_enable = false;
    bool hfg_enable = false;
    DpTimeValue    begin;
    DpTimeValue    end;
    int32_t        diff;

    DP_TIMER_GET_CURRENT_TIME(begin);

    DPLOGI("DpEngine_TDSHP: pqSessionId = %llx \n", config.pqSessionId);

    PQSession* pPQSession = PQSessionManager::getInstance()->getPQSession(config.pqSessionId);

    if (pPQSession != NULL)
    {
        PQAlgorithmFactory* pPQAlgorithmFactory = PQAlgorithmFactory::getInstance();

        PQDCAdaptor* pPQDCAdaptor = pPQAlgorithmFactory->getDynamicContrast(m_identifier);
        adl_enable = pPQDCAdaptor->calRegs(pPQSession, command, config, m_frameConfigLabel);
        DPLOGI("DpEngine_TDSHP: configFrame() adl_enable = %d\n",adl_enable);
#ifdef SUPPORT_HFG
        PQHFGAdaptor* pPQHFGAdaptor = pPQAlgorithmFactory->getHFG(m_identifier);
        hfg_enable = pPQHFGAdaptor->calRegs(pPQSession, command, config);
        m_HFGEnable = hfg_enable;
        DPLOGI("DpEngine_TDSHP: configFrame() hfg_enable = %d\n",hfg_enable);
#endif
        PQDSAdaptor* pPQDSAdaptor = pPQAlgorithmFactory->getDynamicSharpness(m_identifier);
        shp_enable = pPQDSAdaptor->calRegs(pPQSession, command, config);
        DPLOGI("DpEngine_TDSHP: configFrame() shp_enable = %d\n",shp_enable);

        if (shp_enable == false &&
            hfg_enable == false &&
            adl_enable == false)
        {
            m_bypassEngine = true;
            MM_REG_WRITE(command, MDP_TDSHP_CFG, 0x00000001, 0x00000001); // set relay mode
        }
        else
        {
            m_bypassEngine = false;
            MM_REG_WRITE(command, MDP_TDSHP_CFG, 0x00000000, 0x00000001); // disable relay mode
        }
    }

    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin, end, diff);

    if (diff > TDSHP_CONFIG_FRAME_THRES)
    {
        DPLOGD("DpEngine_TDSHP: configFrame() time %d ms\n", diff);
    }

#ifdef HW_SUPPORT_10BIT_PATH
    // 10 bit format
    if (DP_COLOR_GET_10BIT_PACKED(config.outFormat) ||
        DP_COLOR_GET_10BIT_LOOSE(config.outFormat) ||
        DP_COLOR_GET_10BIT_PACKED(config.inFormat) ||
        DP_COLOR_GET_10BIT_LOOSE(config.inFormat) ||
        config.scenario == STREAM_ISP_IC ||
        config.scenario == STREAM_ISP_VR ||
        config.scenario == STREAM_ISP_ZSD ||
        config.scenario == STREAM_ISP_IP ||
        config.scenario == STREAM_ISP_VSS ||
        config.scenario == STREAM_ISP_ZSD_SLOW ||
        config.scenario == STREAM_WPE ||
        config.scenario == STREAM_WPE2)
    {
        MM_REG_WRITE(command, MDP_TDSHP_CTRL, 0 << 2, 0x4);
    }
    else
    {
        MM_REG_WRITE(command, MDP_TDSHP_CTRL, 1 << 2, 0x4);
    }
#endif // HW_SUPPORT_10BIT_PATH

#else // CONFIG_FOR_OS_ANDROID
#if CONFIG_FOR_VERIFY_FPGA
    onFPGATestConfigTDSHP(command, config);
#endif // CONFIG_FOR_VERIFY_FPGA
#endif // CONFIG_FOR_OS_ANDROID

    m_outHistXLeft      = 0;
    m_outHistYTop       = 0;
    m_lastOutVertical   = 0;
    m_lastOutHorizontal = 0;
    m_pqSessionId = config.pqSessionId;
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_TDSHP::onReconfigFrame(DpCommand &command, DpConfig &config)
{
    bool adl_enable = false;

    DPLOGI("DpEngine_TDSHP: onReconfigFrame");
    DPLOGI("DpEngine_TDSHP: pqSessionId = %llx \n", config.pqSessionId);

    PQSession* pPQSession = PQSessionManager::getInstance()->getPQSession(config.pqSessionId);

    if (pPQSession != NULL)
    {
        PQAlgorithmFactory* pPQAlgorithmFactory = PQAlgorithmFactory::getInstance();

        PQDCAdaptor* pPQDCAdaptor = pPQAlgorithmFactory->getDynamicContrast(m_identifier);
        adl_enable = pPQDCAdaptor->calRegs(pPQSession, command, config, m_frameConfigLabel, true);
        DPLOGI("DpEngine_TDSHP: configFrame() adl_enable = %d\n",adl_enable);
    }

    m_outHistXLeft      = 0;
    m_outHistYTop       = 0;
    m_lastOutVertical   = 0;
    m_lastOutHorizontal = 0;
    m_pqSessionId = config.pqSessionId;
    return DP_STATUS_RETURN_SUCCESS;
}

void DpEngine_TDSHP::ADL_resetLumaHist(DpCommand &command)
{
    // reset LUMA HIST
    //DPLOGI("DpEngine_TDSHP: ADL_resetLumaHist()\n");
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_00, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_01, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_02, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_03, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_04, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_05, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_06, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_07, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_08, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_09, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_10, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_11, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_12, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_13, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_14, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_15, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_HIST_INIT_16, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_LUMA_SUM_INIT, 0, 0xFFFFFFFF);
#ifdef TDSHP_1_1
    MM_REG_WRITE(command, MDP_DC_TWO_D_W1_RESULT_INIT, 0, 0xFFFFFFFF);
#endif

#if DYN_CONTRAST_VERSION == 2
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_00, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_01, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_02, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_03, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_04, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_05, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_06, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_07, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_08, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_09, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_10, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_11, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_12, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_13, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_14, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_15, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_CONTOUR_HIST_INIT_16, 0, 0xFFFFFFFF);
#endif
}

#if CONFIG_FOR_OS_ANDROID

#endif // CONFIG_FOR_OS_ANDROID

DP_STATUS_ENUM DpEngine_TDSHP::onConfigTile(DpCommand &command)
{
    uint32_t TDS_in_hsize;
    uint32_t TDS_in_vsize;
    uint32_t TDS_out_hoffset;
    uint32_t TDS_out_voffset;
    uint32_t TDS_out_hsize;
    uint32_t TDS_out_vsize;
    uint32_t TDS_hist_left;
    uint32_t TDS_hist_top;

    // Set source size
    TDS_in_hsize    = m_inTileXRight  - m_inTileXLeft + 1;
    TDS_in_vsize    = m_inTileYBottom - m_inTileYTop + 1;
    MM_REG_WRITE(command, MDP_TDSHP_INPUT_SIZE , (TDS_in_hsize << 16) +
                                          (TDS_in_vsize <<  0), 0x1FFF1FFF);

    // Set crop offset
    TDS_out_hoffset = m_outTileXLeft - m_inTileXLeft;
    TDS_out_voffset = m_outTileYTop  - m_inTileYTop;
    MM_REG_WRITE(command, MDP_TDSHP_OUTPUT_OFFSET, (TDS_out_hoffset << 16) +
                                            (TDS_out_voffset <<  0), 0x00FF00FF);

    // Set target size
    TDS_out_hsize   = m_outTileXRight - m_outTileXLeft + 1;
    TDS_out_vsize   = m_outTileYBottom - m_outTileYTop + 1;
    MM_REG_WRITE(command, MDP_TDSHP_OUTPUT_SIZE, (TDS_out_hsize << 16) +
                                            (TDS_out_vsize <<  0), 0x1FFF1FFF);

    // Set histogram window
    TDS_hist_left = (m_outTileXLeft > m_outHistXLeft) ? m_outTileXLeft : m_outHistXLeft;
    TDS_hist_top  = (m_outTileYTop  > m_outHistYTop)  ? m_outTileYTop  : m_outHistYTop;
    MM_REG_WRITE(command, MDP_HIST_CFG_00, ((m_outTileXRight - m_inTileXLeft) << 16) +
                                            ((TDS_hist_left   - m_inTileXLeft) <<  0), 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_HIST_CFG_01, ((m_outTileYBottom - m_inTileYTop) << 16) +
                                            ((TDS_hist_top     - m_inTileYTop) <<  0), 0xFFFFFFFF);
#ifdef SUPPORT_HFG
    PQSession* pPQSession = PQSessionManager::getInstance()->getPQSession(m_pqSessionId);
    if (pPQSession != NULL)
    {
        PQHFGAdaptor* pPQHFGAdaptor = PQAlgorithmFactory::getInstance()->getHFG(m_identifier);
        pPQHFGAdaptor->calTileRegs(pPQSession, command, TDS_out_hsize, TDS_out_vsize, m_outTileXLeft, m_outTileYTop);
    }
#endif
#if 0
    {
        uint32_t* pTmp;
        pTmp = (uint32_t*)(0x83700060);
        MM_REG_WRITE_FROM_MEM_BEGIN(command);
        for(i = 0x200; i<= 0x244; i += 4)
        {
            MM_REG_WRITE_FROM_MEM(command,MDP_TDSHP_00+i,pTmp,0xFFFFFFFF);
            pTmp++;
        }
        MM_REG_WRITE_FROM_MEM_END(command);
    }
#endif

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_TDSHP::onPostProc(DpCommand &command)
{
    if (command.getSyncMode()) //old version pq readback must remove
    {
        DPLOGI("DpEngine_TDSHP::onPostProc : SyncMode do nothing\n");
        return DP_STATUS_RETURN_SUCCESS;
    }
    uint32_t index;
    bool pq_readback;
    bool hdr_readback;
    int32_t dre_readback;
    uint64_t engineFlag;
    uint32_t VEncFlag;
    uint32_t counter = 0;
    uint32_t* readbackPABuffer = NULL;
    uint32_t readbackPABufferIndex = 0;

    command.getReadbackStatus(pq_readback, hdr_readback, dre_readback, engineFlag, VEncFlag);

    if ((((engineFlag >> tVENC) & 0x1) && VEncFlag) || !(pq_readback))
    {
        DPLOGI("DpEngine_TDSHP::onPostProc : VENC and no readback do nothing\n");
        return DP_STATUS_RETURN_SUCCESS;
    }
    readbackPABuffer = command.getReadbackPABuffer(readbackPABufferIndex);

    if (readbackPABuffer == NULL)
    {
        DPLOGW("DpEngine_TDSHP::onPostProc : readbackPABuffer has been destroyed readbackPABuffer = %p, readbackPABufferIndex = %d\n", readbackPABuffer, readbackPABufferIndex);
        return DP_STATUS_RETURN_SUCCESS;
    }
    MM_REG_READ_BEGIN(command);

    if (pq_readback)
    {
        uint32_t iTDSHPBase = /*((engineFlag >> tTDSHP0) & 0x1) ?*/ MDP_TDSHP0_BASE /*: MDP_TDSHP1_BASE*/;

        for (index = 0x6C; index <= 0xB4; index += 4)
        {
            if (index == 0x88)
                continue;

            MM_REG_READ(command, iTDSHPBase + index, readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], &m_regLabel[counter]);
            counter++;
        }

        //For shit usage to read color info by Algo's requests
        #ifdef CONFIG_FOR_SOURCE_PQ
                MM_REG_READ(command, COLOREX_COLOR_TWO_D_W1_RESULT, readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], &m_regLabel[counter]);
                counter++;
        #else
            #ifdef CMD_GPR_R32
                MM_REG_READ(command, CMD_GPR_R32(CMDQ_DATA_REG_PQ_COLOR), readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], &m_regLabel[counter]);
                counter++;
            #else
                MM_REG_READ(command, iTDSHPBase + 0x264, readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], &m_regLabel[counter]);
                counter++;
            #endif
        #endif

#if DYN_CONTRAST_VERSION == 2
        for (index = 0x3DC; index <= 0x41C; index += 4)
        {
            MM_REG_READ(command, iTDSHPBase + index, readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], &m_regLabel[counter]);
            counter++;
        }
#endif
    }

    MM_REG_READ_END(command);

    command.setNumReadbackPABuffer(counter, tTDSHP0);

    m_prevPABufferIndex = readbackPABufferIndex;
    m_prevReadbackCount = counter;

    DPLOGI("DpEngine_TDSHP::onPostProc: counter:%d\n", counter);

#if 0
    uint32_t    i;
    uint32_t*   pTmp;
    pTmp = (uint32_t*)(0x83700060);

    MM_REG_READ_BEGIN(command);
    for(i = 0x6C; i < 0xB8; i += 4)
    {
        if(0x88 == i)
            continue;

        MM_REG_READ(command,MDP_TDSHP_00+i,pTmp);
        pTmp++;
    }
    MM_REG_READ_END(command);
#endif
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_TDSHP::onAdvanceTile(DpCommand &command)
{
    // FIXME: check tile order
    if (m_lastOutHorizontal != m_outHorizontal)
    {
        m_outHistXLeft      = m_outHorizontal ? (m_outTileXRight + 1) : 0;
        m_lastOutHorizontal = m_outHorizontal;
    }
    if (m_lastOutVertical != m_outVertical)
    {
        m_outHistYTop       = m_outVertical ? (m_outTileYBottom + 1) : 0;
        m_lastOutVertical   = m_outVertical;
    }

#ifdef MDP_HW_TILE_SW_RESET
    MM_REG_WRITE_FROM_REG_BEGIN(command);

    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_00, MDP_LUMA_HIST_00, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_01, MDP_LUMA_HIST_01, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_02, MDP_LUMA_HIST_02, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_03, MDP_LUMA_HIST_03, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_04, MDP_LUMA_HIST_04, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_05, MDP_LUMA_HIST_05, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_06, MDP_LUMA_HIST_06, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_07, MDP_LUMA_HIST_07, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_08, MDP_LUMA_HIST_08, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_09, MDP_LUMA_HIST_09, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_10, MDP_LUMA_HIST_10, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_11, MDP_LUMA_HIST_11, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_12, MDP_LUMA_HIST_12, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_13, MDP_LUMA_HIST_13, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_14, MDP_LUMA_HIST_14, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_15, MDP_LUMA_HIST_15, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_HIST_INIT_16, MDP_LUMA_HIST_16, 0xFFFFFFFF);
    MM_REG_WRITE_FROM_REG(command, MDP_LUMA_SUM_INIT, MDP_LUMA_SUM, 0xFFFFFFFF);

    MM_REG_WRITE_FROM_REG(command, MDP_DC_TWO_D_W1_RESULT_INIT, MDP_DC_TWO_D_W1_RESULT, 0xFFFFFFFF); // for color

    MM_REG_WRITE_FROM_REG_END(command);
#else
    DP_UNUSED(command);
#endif

    return DP_STATUS_RETURN_SUCCESS;
}

#ifdef CONFIG_FOR_SOURCE_PQ

#include "mdp_reg_colorex.h"
#include "dispsys_reg.h"
#include "mmsys_config.h"
#include "mmsys_mutex.h"
//#define _DISPSYS_DEBUG_

//--------------------------------------------------------
// COLOR_EX driver engine
//--------------------------------------------------------
class DpEngine_COLOR_EX: public DpTileEngine
{
public:
    DpEngine_COLOR_EX(uint32_t identifier)
        : DpTileEngine(identifier)
    {
        DPLOGI("DpEngine_COLOR_EX\n");
    }

    ~DpEngine_COLOR_EX()
    {
    }

private:

    DP_STATUS_ENUM onInitEngine(DpCommand&);

    DP_STATUS_ENUM onDeInitEngine(DpCommand&);

    DP_STATUS_ENUM onConfigFrame(DpCommand&,
                                 DpConfig&);

    int64_t onQueryFeature()
    {
        return eCOLOR_EX;
    }
};

DP_STATUS_ENUM DpEngine_COLOR_EX::onInitEngine(DpCommand &command)
{
    DPLOGI("%s\n", __FUNCTION__);

    /***************** init COLOR ********************/
    //DISP_REG_SET(cmdq, COLOREX_COLOR_CFG_MAIN + offset,(1<<29)); //color enable
    //DISP_REG_SET(cmdq, COLOREX_COLOR_START + offset, 0x00000001); //color start
    MM_REG_WRITE(command, COLOREX_COLOR_CFG_MAIN, 0x00000000, 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_START, 0x00000001, 0xFFFFFFFF);

    //DPLOGD("DpEngine_COLORonInit(), en[%d],  x[0x%x], y[0x%x] \n", g_split_en, g_split_window_x, g_split_window_y);

    //DISP_REG_SET(cmdq, COLOREX_COLOR_DBG_CFG_MAIN + offset, g_split_en << 3);
    //DISP_REG_SET(cmdq, COLOREX_COLOR_WIN_X_MAIN + offset, g_split_window_x);
    //DISP_REG_SET(cmdq, COLOREX_COLOR_WIN_Y_MAIN + offset, g_split_window_y);
    MM_REG_WRITE(command, COLOREX_COLOR_DBG_CFG_MAIN, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_WIN_X_MAIN, 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_WIN_Y_MAIN, 0, 0xFFFFFFFF);

    //enable interrupt
    //DISP_REG_SET(cmdq, COLOREX_COLOR_INTEN + offset, 0x00000007);
    MM_REG_WRITE(command, COLOREX_COLOR_INTEN, 0x00000007, 0xFFFFFFFF);

    //Set 10bit->8bit Rounding
    //DISP_REG_SET(cmdq, COLOREX_COLOR_OUT_SEL + offset, 0x333);
    MM_REG_WRITE(command, COLOREX_COLOR_OUT_SEL, 0x333, 0xFFFFFFFF);

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_COLOR_EX::onDeInitEngine(DpCommand &command)
{
    DPLOGI("%s\n", __FUNCTION__);
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_COLOR_EX::onConfigFrame(DpCommand &command, DpConfig &config)
{
    int index = 0;
    unsigned int u4Temp = 0;
    unsigned char h_series[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    DPLOGI("%s\n", __FUNCTION__);

    COLOR_CONFIG_T colorConfig;
    bool getColorConfig;
    PQSession* pPQSession = PQSessionManager::getInstance()->getPQSession(config.pqSessionId);

    if (pPQSession != NULL)
    {
        getColorConfig = pPQSession->getColorConfig(&colorConfig);
    }
    else
    {
        DPLOGE("[TDSHP][COLOR_EX] pPQSession is NULL !\n");
        return DP_STATUS_UNKNOWN_ERROR;
    }

    if(getColorConfig == false)
    {
        DPLOGE("[TDSHP][COLOR_EX] Tuning index range error !\n");
        return DP_STATUS_UNKNOWN_ERROR;
    }

    /****************** bypass CCORR ********************/
    MM_REG_WRITE(command, COLOREX_CCORR_EN, 0x1, 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_CCORR_CFG, 0x1, 0x1);
    MM_REG_WRITE(command, COLOREX_CCORR_SIZE, (config.outWidth << 16) | config.outHeight, 0xFFFFFFFF);

    /****************** bypass AAL ********************/
    MM_REG_WRITE(command, COLOREX_AAL_EN, 0x1, 0x1);
    MM_REG_WRITE(command, COLOREX_AAL_CFG, 0x1, 0x1);
    MM_REG_WRITE(command, COLOREX_AAL_SIZE, (config.outWidth << 16) | config.outHeight, 0xFFFFFFFF);

    /****************** bypass GAMMA ********************/
    MM_REG_WRITE(command, COLOREX_GAMMA_EN, 0x1, 0x1);
    MM_REG_WRITE(command, COLOREX_GAMMA_CFG, 0x1, 0x1);
    MM_REG_WRITE(command, COLOREX_GAMMA_SIZE, (config.outWidth << 16) | config.outHeight, 0xFFFFFFFF);

    /****************** bypass DITHER ********************/
    MM_REG_WRITE(command, COLOREX_DITHER_EN, 0x1, 0x1);
    MM_REG_WRITE(command, COLOREX_DITHER_CFG, 0x1, 0x1);
    MM_REG_WRITE(command, COLOREX_DITHER_SIZE, (config.outWidth << 16) | config.outHeight, 0xFFFFFFFF);


    //DISP_REG_SET(cmdq, COLOREX_COLOR_INTERNAL_IP_WIDTH + offset, srcWidth);  //wrapper width
    //DISP_REG_SET(cmdq, COLOREX_COLOR_INTERNAL_IP_HEIGHT + offset, srcHeight); //wrapper height
    MM_REG_WRITE(command, COLOREX_COLOR_INTERNAL_IP_WIDTH, config.outWidth, 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_INTERNAL_IP_HEIGHT, config.outHeight, 0xFFFFFFFF);

    // enable R2Y/Y2R in Color Wrapper
    //DISP_REG_SET(cmdq, COLOREX_COLOR_CM1_EN + offset, 0x01);
    //DISP_REG_SET(cmdq, COLOREX_COLOR_CM2_EN + offset, 0x11);   // also set no rounding on Y2R
    MM_REG_WRITE(command, COLOREX_COLOR_CM1_EN, 0x01, 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_CM2_EN, 0x11, 0xFFFFFFFF);

    // config parameter from customer color_index.h
    //DISP_REG_SET(cmdq, COLOREX_COLOR_G_PIC_ADJ_MAIN_1, (colorConfig.BRIGHTNESS[m_PQParam.u4Brightness]<<16) |colorConfig.CONTRAST[m_PQParam.u4Contrast]);
    //DISP_REG_SET(cmdq, COLOREX_COLOR_G_PIC_ADJ_MAIN_2, (0x200<<16) |colorConfig.GLOBAL_SAT[m_PQParam.u4SatGain]);
    MM_REG_WRITE(command, COLOREX_COLOR_G_PIC_ADJ_MAIN_1, (colorConfig.BRIGHTNESS << 16) |colorConfig.CONTRAST, 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_G_PIC_ADJ_MAIN_2, (0x200<<16) |colorConfig.GLOBAL_SAT, 0xFFFFFFFF);


    // Partial Y Function
    for (index = 0; index < 8; index++)
    {
        //DISP_REG_SET(cmdq, COLOREX_COLOR_Y_SLOPE_1_0_MAIN + 4 * index, (colorConfig.PARTIAL_Y[2 * index ] | colorConfig.PARTIAL_Y[2 * index + 1]<<16));
        MM_REG_WRITE(command, COLOREX_COLOR_Y_SLOPE_1_0_MAIN + 4 * index, (colorConfig.PARTIAL_Y[2 * index ] | colorConfig.PARTIAL_Y[2 * index + 1] << 16), 0xFFFFFFFF);
    }


    // Partial Saturation Function

    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_GAIN1_0 + offset, ( colorConfig.PURP_TONE_S[SG1][0] | colorConfig.PURP_TONE_S[SG1][1] << 8 | colorConfig.PURP_TONE_S[SG1][2] << 16 | colorConfig.SKIN_TONE_S[SG1][0] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_GAIN1_1 + offset, ( colorConfig.SKIN_TONE_S[SG1][1] | colorConfig.SKIN_TONE_S[SG1][2] << 8 | colorConfig.SKIN_TONE_S[SG1][3] << 16 | colorConfig.SKIN_TONE_S[SG1][4] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_GAIN1_2 + offset, ( colorConfig.SKIN_TONE_S[SG1][5] | colorConfig.SKIN_TONE_S[SG1][6] << 8 | colorConfig.SKIN_TONE_S[SG1][7] << 16 | colorConfig.GRASS_TONE_S[SG1][0] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_GAIN1_3 + offset, ( colorConfig.GRASS_TONE_S[SG1][1] | colorConfig.GRASS_TONE_S[SG1][2] << 8 | colorConfig.GRASS_TONE_S[SG1][3] << 16 | colorConfig.GRASS_TONE_S[SG1][4] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_GAIN1_4 + offset, ( colorConfig.GRASS_TONE_S[SG1][5] | colorConfig.SKY_TONE_S[SG1][0] << 8 | colorConfig.SKY_TONE_S[SG1][1] << 16 | colorConfig.SKY_TONE_S[SG1][2] << 24 ));
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_GAIN1_0, ( colorConfig.PURP_TONE_S[SG1][0] | colorConfig.PURP_TONE_S[SG1][1] << 8 | colorConfig.PURP_TONE_S[SG1][2] << 16 | colorConfig.SKIN_TONE_S[SG1][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_GAIN1_1, ( colorConfig.SKIN_TONE_S[SG1][1] | colorConfig.SKIN_TONE_S[SG1][2] << 8 | colorConfig.SKIN_TONE_S[SG1][3] << 16 | colorConfig.SKIN_TONE_S[SG1][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_GAIN1_2, ( colorConfig.SKIN_TONE_S[SG1][5] | colorConfig.SKIN_TONE_S[SG1][6] << 8 | colorConfig.SKIN_TONE_S[SG1][7] << 16 | colorConfig.GRASS_TONE_S[SG1][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_GAIN1_3, ( colorConfig.GRASS_TONE_S[SG1][1] | colorConfig.GRASS_TONE_S[SG1][2] << 8 | colorConfig.GRASS_TONE_S[SG1][3] << 16 | colorConfig.GRASS_TONE_S[SG1][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_GAIN1_4, ( colorConfig.GRASS_TONE_S[SG1][5] | colorConfig.SKY_TONE_S[SG1][0] << 8 | colorConfig.SKY_TONE_S[SG1][1] << 16 | colorConfig.SKY_TONE_S[SG1][2] << 24 ), 0xFFFFFFFF);

    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_GAIN2_0 + offset, ( colorConfig.PURP_TONE_S[SG2][0] | colorConfig.PURP_TONE_S[SG2][1] << 8 | colorConfig.PURP_TONE_S[SG2][2] << 16 | colorConfig.SKIN_TONE_S[SG2][0] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_GAIN2_1 + offset, ( colorConfig.SKIN_TONE_S[SG2][1] | colorConfig.SKIN_TONE_S[SG2][2] << 8 | colorConfig.SKIN_TONE_S[SG2][3] << 16 | colorConfig.SKIN_TONE_S[SG2][4] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_GAIN2_2 + offset, ( colorConfig.SKIN_TONE_S[SG2][5] | colorConfig.SKIN_TONE_S[SG2][6] << 8 | colorConfig.SKIN_TONE_S[SG2][7] << 16 | colorConfig.GRASS_TONE_S[SG2][0] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_GAIN2_3 + offset, ( colorConfig.GRASS_TONE_S[SG2][1] | colorConfig.GRASS_TONE_S[SG2][2] << 8 | colorConfig.GRASS_TONE_S[SG2][3] << 16 | colorConfig.GRASS_TONE_S[SG2][4] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_GAIN2_4 + offset, ( colorConfig.GRASS_TONE_S[SG2][5] | colorConfig.SKY_TONE_S[SG2][0] << 8 | colorConfig.SKY_TONE_S[SG2][1] << 16 | colorConfig.SKY_TONE_S[SG2][2] << 24 ));
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_GAIN2_0, ( colorConfig.PURP_TONE_S[SG2][0] | colorConfig.PURP_TONE_S[SG2][1] << 8 | colorConfig.PURP_TONE_S[SG2][2] << 16 | colorConfig.SKIN_TONE_S[SG2][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_GAIN2_1, ( colorConfig.SKIN_TONE_S[SG2][1] | colorConfig.SKIN_TONE_S[SG2][2] << 8 | colorConfig.SKIN_TONE_S[SG2][3] << 16 | colorConfig.SKIN_TONE_S[SG2][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_GAIN2_2, ( colorConfig.SKIN_TONE_S[SG2][5] | colorConfig.SKIN_TONE_S[SG2][6] << 8 | colorConfig.SKIN_TONE_S[SG2][7] << 16 | colorConfig.GRASS_TONE_S[SG2][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_GAIN2_3, ( colorConfig.GRASS_TONE_S[SG2][1] | colorConfig.GRASS_TONE_S[SG2][2] << 8 | colorConfig.GRASS_TONE_S[SG2][3] << 16 | colorConfig.GRASS_TONE_S[SG2][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_GAIN2_4, ( colorConfig.GRASS_TONE_S[SG2][5] | colorConfig.SKY_TONE_S[SG2][0] << 8 | colorConfig.SKY_TONE_S[SG2][1] << 16 | colorConfig.SKY_TONE_S[SG2][2] << 24 ), 0xFFFFFFFF);

    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_GAIN3_0 + offset, ( colorConfig.PURP_TONE_S[SG3][0] | colorConfig.PURP_TONE_S[SG3][1] << 8 | colorConfig.PURP_TONE_S[SG3][2] << 16 | colorConfig.SKIN_TONE_S[SG3][0] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_GAIN3_1 + offset, ( colorConfig.SKIN_TONE_S[SG3][1] | colorConfig.SKIN_TONE_S[SG3][2] << 8 | colorConfig.SKIN_TONE_S[SG3][3] << 16 | colorConfig.SKIN_TONE_S[SG3][4] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_GAIN3_2 + offset, ( colorConfig.SKIN_TONE_S[SG3][5] | colorConfig.SKIN_TONE_S[SG3][6] << 8 | colorConfig.SKIN_TONE_S[SG3][7] << 16 | colorConfig.GRASS_TONE_S[SG3][0] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_GAIN3_3 + offset, ( colorConfig.GRASS_TONE_S[SG3][1] | colorConfig.GRASS_TONE_S[SG3][2] << 8 | colorConfig.GRASS_TONE_S[SG3][3] << 16 | colorConfig.GRASS_TONE_S[SG3][4] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_GAIN3_4 + offset, ( colorConfig.GRASS_TONE_S[SG3][5] | colorConfig.SKY_TONE_S[SG3][0] << 8 | colorConfig.SKY_TONE_S[SG3][1] << 16 | colorConfig.SKY_TONE_S[SG3][2] << 24 ));
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_GAIN3_0, ( colorConfig.PURP_TONE_S[SG3][0] | colorConfig.PURP_TONE_S[SG3][1] << 8 | colorConfig.PURP_TONE_S[SG3][2] << 16 | colorConfig.SKIN_TONE_S[SG3][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_GAIN3_1, ( colorConfig.SKIN_TONE_S[SG3][1] | colorConfig.SKIN_TONE_S[SG3][2] << 8 | colorConfig.SKIN_TONE_S[SG3][3] << 16 | colorConfig.SKIN_TONE_S[SG3][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_GAIN3_2, ( colorConfig.SKIN_TONE_S[SG3][5] | colorConfig.SKIN_TONE_S[SG3][6] << 8 | colorConfig.SKIN_TONE_S[SG3][7] << 16 | colorConfig.GRASS_TONE_S[SG3][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_GAIN3_3, ( colorConfig.GRASS_TONE_S[SG3][1] | colorConfig.GRASS_TONE_S[SG3][2] << 8 | colorConfig.GRASS_TONE_S[SG3][3] << 16 | colorConfig.GRASS_TONE_S[SG3][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_GAIN3_4, ( colorConfig.GRASS_TONE_S[SG3][5] | colorConfig.SKY_TONE_S[SG3][0] << 8 | colorConfig.SKY_TONE_S[SG3][1] << 16 | colorConfig.SKY_TONE_S[SG3][2] << 24 ), 0xFFFFFFFF);

    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_POINT1_0 + offset, ( colorConfig.PURP_TONE_S[SP1][0] | colorConfig.PURP_TONE_S[SP1][1] << 8 | colorConfig.PURP_TONE_S[SP1][2] << 16 | colorConfig.SKIN_TONE_S[SP1][0] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_POINT1_1 + offset, ( colorConfig.SKIN_TONE_S[SP1][1] | colorConfig.SKIN_TONE_S[SP1][2] << 8 | colorConfig.SKIN_TONE_S[SP1][3] << 16 | colorConfig.SKIN_TONE_S[SP1][4] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_POINT1_2 + offset, ( colorConfig.SKIN_TONE_S[SP1][5] | colorConfig.SKIN_TONE_S[SP1][6] << 8 | colorConfig.SKIN_TONE_S[SP1][7] << 16 | colorConfig.GRASS_TONE_S[SP1][0] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_POINT1_3 + offset, ( colorConfig.GRASS_TONE_S[SP1][1] | colorConfig.GRASS_TONE_S[SP1][2] << 8 | colorConfig.GRASS_TONE_S[SP1][3] << 16 | colorConfig.GRASS_TONE_S[SP1][4] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_POINT1_4 + offset, ( colorConfig.GRASS_TONE_S[SP1][5] | colorConfig.SKY_TONE_S[SP1][0] << 8 | colorConfig.SKY_TONE_S[SP1][1] << 16 | colorConfig.SKY_TONE_S[SP1][2] << 24 ));
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_POINT1_0, ( colorConfig.PURP_TONE_S[SP1][0] | colorConfig.PURP_TONE_S[SP1][1] << 8 | colorConfig.PURP_TONE_S[SP1][2] << 16 | colorConfig.SKIN_TONE_S[SP1][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_POINT1_1, ( colorConfig.SKIN_TONE_S[SP1][1] | colorConfig.SKIN_TONE_S[SP1][2] << 8 | colorConfig.SKIN_TONE_S[SP1][3] << 16 | colorConfig.SKIN_TONE_S[SP1][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_POINT1_2, ( colorConfig.SKIN_TONE_S[SP1][5] | colorConfig.SKIN_TONE_S[SP1][6] << 8 | colorConfig.SKIN_TONE_S[SP1][7] << 16 | colorConfig.GRASS_TONE_S[SP1][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_POINT1_3, ( colorConfig.GRASS_TONE_S[SP1][1] | colorConfig.GRASS_TONE_S[SP1][2] << 8 | colorConfig.GRASS_TONE_S[SP1][3] << 16 | colorConfig.GRASS_TONE_S[SP1][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_POINT1_4, ( colorConfig.GRASS_TONE_S[SP1][5] | colorConfig.SKY_TONE_S[SP1][0] << 8 | colorConfig.SKY_TONE_S[SP1][1] << 16 | colorConfig.SKY_TONE_S[SP1][2] << 24 ), 0xFFFFFFFF);

    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_POINT2_0 + offset, ( colorConfig.PURP_TONE_S[SP2][0] | colorConfig.PURP_TONE_S[SP2][1] << 8 | colorConfig.PURP_TONE_S[SP2][2] << 16 | colorConfig.SKIN_TONE_S[SP2][0] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_POINT2_1 + offset, ( colorConfig.SKIN_TONE_S[SP2][1] | colorConfig.SKIN_TONE_S[SP2][2] << 8 | colorConfig.SKIN_TONE_S[SP2][3] << 16 | colorConfig.SKIN_TONE_S[SP2][4] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_POINT2_2 + offset, ( colorConfig.SKIN_TONE_S[SP2][5] | colorConfig.SKIN_TONE_S[SP2][6] << 8 | colorConfig.SKIN_TONE_S[SP2][7] << 16 | colorConfig.GRASS_TONE_S[SP2][0] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_POINT2_3 + offset, ( colorConfig.GRASS_TONE_S[SP2][1] | colorConfig.GRASS_TONE_S[SP2][2] << 8 | colorConfig.GRASS_TONE_S[SP2][3] << 16 | colorConfig.GRASS_TONE_S[SP2][4] << 24 ));
    //DISP_REG_SET(cmdq, COLOREX_COLOR_PART_SAT_POINT2_4 + offset, ( colorConfig.GRASS_TONE_S[SP2][5] | colorConfig.SKY_TONE_S[SP2][0] << 8 | colorConfig.SKY_TONE_S[SP2][1] << 16 | colorConfig.SKY_TONE_S[SP2][2] << 24 ));
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_POINT2_0, ( colorConfig.PURP_TONE_S[SP2][0] | colorConfig.PURP_TONE_S[SP2][1] << 8 | colorConfig.PURP_TONE_S[SP2][2] << 16 | colorConfig.SKIN_TONE_S[SP2][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_POINT2_1, ( colorConfig.SKIN_TONE_S[SP2][1] | colorConfig.SKIN_TONE_S[SP2][2] << 8 | colorConfig.SKIN_TONE_S[SP2][3] << 16 | colorConfig.SKIN_TONE_S[SP2][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_POINT2_2, ( colorConfig.SKIN_TONE_S[SP2][5] | colorConfig.SKIN_TONE_S[SP2][6] << 8 | colorConfig.SKIN_TONE_S[SP2][7] << 16 | colorConfig.GRASS_TONE_S[SP2][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_POINT2_3, ( colorConfig.GRASS_TONE_S[SP2][1] | colorConfig.GRASS_TONE_S[SP2][2] << 8 | colorConfig.GRASS_TONE_S[SP2][3] << 16 | colorConfig.GRASS_TONE_S[SP2][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, COLOREX_COLOR_PART_SAT_POINT2_4, ( colorConfig.GRASS_TONE_S[SP2][5] | colorConfig.SKY_TONE_S[SP2][0] << 8 | colorConfig.SKY_TONE_S[SP2][1] << 16 | colorConfig.SKY_TONE_S[SP2][2] << 24 ), 0xFFFFFFFF);

    for (index = 0; index < 3; index++)
    {
        h_series[index + PURP_TONE_START] = colorConfig.PURP_TONE_H[index];
    }

    for (index = 0; index < 8; index++)
    {
        h_series[index + SKIN_TONE_START] = colorConfig.SKIN_TONE_H[index];
    }

    for (index = 0; index < 6; index++)
    {
        h_series[index + GRASS_TONE_START] = colorConfig.GRASS_TONE_H[index];
    }

    for (index = 0; index < 3; index++)
    {
        h_series[index + SKY_TONE_START] = colorConfig.SKY_TONE_H[index];
    }

    for (index = 0; index < 5; index++)
    {
        u4Temp = (h_series[4 * index]) +
                 (h_series[4 * index + 1] << 8) +
                 (h_series[4 * index + 2] << 16) +
                 (h_series[4 * index + 3] << 24);
        //DISP_REG_SET(cmdq, COLOREX_COLOR_LOCAL_HUE_CD_0 + offset + 4 * index, u4Temp);
        MM_REG_WRITE(command, COLOREX_COLOR_LOCAL_HUE_CD_0 + 4 * index, u4Temp, 0xFFFFFFFF);
    }

    // color window

    //DISP_REG_SET(cmdq, COLOREX_COLOR_TWO_D_WINDOW_1 + offset, g_color_window);
    MM_REG_WRITE(command, COLOREX_COLOR_TWO_D_WINDOW_1, 0x40106051, 0xFFFFFFFF);    //g_color_window = 0x40106051;

    return DP_STATUS_RETURN_SUCCESS;
}

//--------------------------------------------------------
// OVL0_EX driver engine
//--------------------------------------------------------
class DpEngine_OVL0_EX: public DpTileEngine
{
public:
    DpEngine_OVL0_EX(uint32_t identifier)
        : DpTileEngine(identifier),
        m_CascadeEnabled(true),
        m_PQSupported(true),
        m_Offset(0)
    {
        m_pPQColorConfig = new PQColorConfig;
        DPLOGI("DpEngine_OVL0_EX\n");
    }

    ~DpEngine_OVL0_EX()
    {
        delete m_pPQColorConfig;
    }

private:

    DP_STATUS_ENUM onInitEngine(DpCommand&);

    DP_STATUS_ENUM onDeInitEngine(DpCommand&);

    DP_STATUS_ENUM onConfigFrame(DpCommand&, DpConfig&);

    int64_t onQueryFeature()
    {
        return eRMEM;
    }

    DP_STATUS_ENUM queryDisplayStatus (DpCommand&);
    bool isOVLCascade (DpCommand&);
    bool isPQSupported (DpCommand&);

    PQColorConfig* m_pPQColorConfig;

    // Display Status
    bool m_CascadeEnabled;
    bool m_PQSupported;
    uint32_t m_Offset;
};

bool DpEngine_OVL0_EX::isOVLCascade (DpCommand &command)
{
    return m_CascadeEnabled;
}

bool DpEngine_OVL0_EX::isPQSupported (DpCommand &command)
{
    return m_PQSupported;
}

DP_STATUS_ENUM DpEngine_OVL0_EX::queryDisplayStatus (DpCommand &command)
{
    DPLOGI("%s\n", __FUNCTION__);

    DISP_PQ_STATUS status;

    m_pPQColorConfig->getDisplayStatus(status, false);

    if (status.decouple) {
        m_PQSupported = true;
    } else {
        m_PQSupported = false;
    }
    if (status.cascade) {
        m_CascadeEnabled = true;
    } else {
        m_CascadeEnabled = false;
    }

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_OVL0_EX::onInitEngine(DpCommand &command)
{
    DPLOGI("%s\n", __FUNCTION__);
    // 1. Power On
    // Do nothing
    // 2. To check some status
    queryDisplayStatus(command);

    // 3. Start hw, ready to work now
    // Firstly, clear Frame Done flag of last frame if needed
    MM_REG_WAIT(command, DpCommand::DISP_WDMA0_FRAME_DONE);
    // Then, using OVL0 and config OVL1 if cascaded
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_EN, 0x1, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_INTEN, 0x1e2, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_SRC_CON, 0x1, 0xFFFFFFFF);
    if (m_CascadeEnabled) {
        MM_REG_WRITE(command, (DISP_OVL_INDEX_OFFSET - m_Offset) + DISP_REG_OVL_EN, 0x1, 0xFFFFFFFF);
    }

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_OVL0_EX::onDeInitEngine(DpCommand &command)
{
    DPLOGI("%s\n", __FUNCTION__);
    // 1. Power Off
    // Do nothing
    // 2. Stop hw
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_EN, 0x0, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_SRC_CON, 0x0, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_INTEN, 0x0, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_INTSTA, 0x0, 0xFFFFFFFF);
    if (m_CascadeEnabled) {
        MM_REG_WRITE(command, (DISP_OVL_INDEX_OFFSET - m_Offset) + DISP_REG_OVL_EN, 0x0, 0xFFFFFFFF);
        MM_REG_WRITE(command, (DISP_OVL_INDEX_OFFSET - m_Offset) + DISP_REG_OVL_SRC_CON, 0x0, 0xFFFFFFFF);
    }

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_OVL0_EX::onConfigFrame(DpCommand &command, DpConfig &config)
{
    uint32_t fmt_matrix = 0x6;  // 0101 MTX_BT601_TO_RGB
    uint32_t value = 0x0;

    DPLOGI("%s\n", __FUNCTION__);
    DPLOGI("OVL0 config: x=%d,y=%d,w=%d,h=%d,fmt=0x%x,ypitch=%d,uvpitch=%d,addr0=0x%x,addr1=0x%x,addr2=0x%x \n",
        config.inXOffset, config.inYOffset, config.inWidth, config.inHeight,
        config.inFormat, config.inYPitch, config.inUVPitch, config.memAddr[0], config.memAddr[1], config.memAddr[2]);
    if (DP_COLOR_GET_COLOR_GROUP(config.inFormat))
    {
        if (config.inFormat != eYUYV && config.inFormat != eYVYU && config.inFormat != eUYVY && config.inFormat != eVYUY)
        {
          DPLOGE("OVL unsupported input color format: 0x%x \n", config.inFormat);
          return DP_STATUS_INVALID_BUFFER;
        }
    }
    // enable OVL_RANDOM_BGCLR_EN, should set layer_en[i]=0, to pass through
    // bg color
#if defined(_DISPSYS_DEBUG_)
    MM_REG_WRITE(command, offset + DISP_REG_OVL_DATAPATH_CON, 0x1, 0xFFFFFFFF);
#endif
    // 1. Config ROI
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_ROI_SIZE, (config.inHeight<<16) | config.inWidth, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_ROI_BGCLR, 0xFF000000, 0xFFFFFFFF);

    // 2. Enable layer-0 only, layer-1 ~ layer-3 is disabled
#if defined(_DISPSYS_DEBUG_)
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_SRC_CON, 0x0, 0xFFFFFFFF);
#else
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_SRC_CON, 0x1, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_RDMA0_CTRL, 0x1, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_RDMA1_CTRL, 0x0, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_RDMA2_CTRL, 0x0, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_RDMA3_CTRL, 0x0, 0xFFFFFFFF);
#endif
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_RDMA0_MEM_GMC_SETTING, 0x50FF, 0xFFFFFFFF);

    if (m_CascadeEnabled) {
        MM_REG_WRITE(command, (DISP_OVL_INDEX_OFFSET - m_Offset) + DISP_REG_OVL_ROI_SIZE, (config.inHeight<<16) | config.inWidth, 0xFFFFFFFF);
        MM_REG_WRITE(command, (DISP_OVL_INDEX_OFFSET - m_Offset) + DISP_REG_OVL_ROI_BGCLR, 0xFF000000, 0xFFFFFFFF);
        MM_REG_WRITE(command, (DISP_OVL_INDEX_OFFSET - m_Offset) + DISP_REG_OVL_SRC_CON, 0x0, 0xFFFFFFFF);
        MM_REG_WRITE(command, (DISP_OVL_INDEX_OFFSET - m_Offset) + DISP_REG_OVL_RDMA0_CTRL, 0x0, 0xFFFFFFFF);
        MM_REG_WRITE(command, (DISP_OVL_INDEX_OFFSET - m_Offset) + DISP_REG_OVL_RDMA1_CTRL, 0x0, 0xFFFFFFFF);
        MM_REG_WRITE(command, (DISP_OVL_INDEX_OFFSET - m_Offset) + DISP_REG_OVL_RDMA2_CTRL, 0x0, 0xFFFFFFFF);
        MM_REG_WRITE(command, (DISP_OVL_INDEX_OFFSET - m_Offset) + DISP_REG_OVL_RDMA3_CTRL, 0x0, 0xFFFFFFFF);
    }

    if (DP_COLOR_GET_COLOR_GROUP(config.inFormat) == 0) {
        fmt_matrix = 0x0;
    }
#if 1
    value = 0xff | (1<<8) | (DP_COLOR_GET_HW_FORMAT(config.inFormat)<<12) | (fmt_matrix<<16) | (DP_COLOR_GET_SWAP_ENABLE(config.outFormat)<<24);
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_L0_CON, value, 0xFFFFFFFF);
#else
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_L0_CON, 0xFF, 0xFF);  // bit0~7: constant alpha
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_L0_CON, 0x1, 0x100);  // bit8: alpha_en
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_L0_CON, 0x0, 0x800);  // bit11: ext_mtx_en
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_L0_CON, DP_COLOR_GET_HW_FORMAT(config.inFormat), 0xF000); // bit12~15: color fmt
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_L0_CON, fmt_matrix, 0xF0000); // bit16~19: int_mtx_sel
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_L0_CON, DP_COLOR_GET_SWAP_ENABLE(config.outFormat), 0x1000000); // bit24: byte_swap
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_L0_CON, 0x0, 0xF0000000); // bit28~29: source; bit30:srckey_en; bit31: dstkey_en
#endif
    value = config.memAddr[0] + config.inYOffset*config.inYPitch + config.inXOffset*DP_COLOR_BITS_PER_PIXEL(config.inFormat)/8;
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_L0_ADDR, config.memAddr[0], 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_L0_PITCH, config.inYPitch, 0xFFFFFFFF);

    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_L0_SRC_SIZE, (config.inHeight<<16)|config.inWidth, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_OVL_L0_OFFSET, (config.inYOffset<<16)|config.inXOffset, 0xFFFFFFFF);

    // 4. misc

    return DP_STATUS_RETURN_SUCCESS;
}

//--------------------------------------------------------
// WDMA_EX driver engine
//--------------------------------------------------------
class DpEngine_WDMA_EX: public DpTileEngine
{
public:
    DpEngine_WDMA_EX(uint32_t identifier)
        : DpTileEngine(identifier),
        m_Offset(0),
        m_mutexID(0)
    {
        m_pPQColorConfig = new PQColorConfig;
        DPLOGI("DpEngine_WDMA_EX\n");
    }

    ~DpEngine_WDMA_EX()
    {
        delete m_pPQColorConfig;
    }

private:

    DP_STATUS_ENUM onInitEngine(DpCommand&);

    DP_STATUS_ENUM onDeInitEngine(DpCommand&);

    DP_STATUS_ENUM onConfigFrame(DpCommand&, DpConfig&);

    DP_STATUS_ENUM queryDisplayStatus (DpCommand&);

    int64_t onQueryFeature()
    {
        return eWMEM;
    }

    PQColorConfig* m_pPQColorConfig;

    // Display Status
    uint32_t m_Offset;
    uint32_t m_mutexID;
};

DP_STATUS_ENUM DpEngine_WDMA_EX::queryDisplayStatus (DpCommand &command)
{
    DPLOGI("%s\n", __FUNCTION__);

    DISP_PQ_STATUS status;

    m_pPQColorConfig->getDisplayStatus(status, false);

    m_mutexID = status.mutex;

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_WDMA_EX::onInitEngine(DpCommand &command)
{
    DPLOGI("%s\n", __FUNCTION__);
    // 1. Power On
    // Do nothing
    queryDisplayStatus(command);

    // 2. Start hw, ready to work now
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_EN, 0x1, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_INTEN, 0x3, 0xFFFFFFFF);

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_WDMA_EX::onDeInitEngine(DpCommand &command)
{
    DPLOGI("%s\n", __FUNCTION__);
    // 1. Power Off
    // Do nothing

    // 2. Stop hw
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_EN, 0x0, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_INTEN, 0x0, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_INTSTA, 0x0, 0xFFFFFFFF);

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_WDMA_EX::onConfigFrame(DpCommand &command, DpConfig &config)
{
    uint32_t fmt_matrix = 0x2;  // 0010 RGB_TO_BT601
    uint32_t value = 0;

    DPLOGI("%s\n", __FUNCTION__);
    DPLOGI("WDMA0 config: src(%d,%d),crop(%d,%d %d,%d),dst(%d,%d %d,%d),Ypitch=%d,UVpitch=%d,fmt=0x%x,addr0=0x%x,addr1=0x%x,addr2=0x%x \n",
        config.inWidth, config.inHeight, config.inXOffset, config.inYOffset, config.inCropWidth, config.inCropHeight,
        config.outXStart, config.outYStart, config.outWidth, config.outHeight, config.outYPitch, config.outUVPitch,
        config.outFormat, config.memAddr[0], config.memAddr[1], config.memAddr[2]);

    // 1. Config WDMA0
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_SRC_SIZE, (config.inHeight<<16)|config.inWidth, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_CLIP_COORD, (config.inYOffset<<16)|config.inXOffset, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_CLIP_SIZE, (config.inCropHeight<<16)|config.inCropWidth, 0xFFFFFFFF);

#if 1
    value = (DP_COLOR_GET_HW_FORMAT(config.outFormat)<<4) | ((DP_COLOR_GET_COLOR_GROUP(config.outFormat)?1:0)<<11) | (DP_COLOR_GET_SWAP_ENABLE(config.outFormat)<<16) | (1<<17) | (fmt_matrix<<24);
    MM_REG_WRITE(command, DISP_REG_WDMA_CFG, value, 0xFFFFFFFF);
#else
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_CFG, DP_COLOR_GET_HW_FORMAT(config.outFormat), 0xF0);  // bit4~7: out color fmt
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_CFG, DP_COLOR_GET_COLOR_GROUP(config.outFormat)?1:0, 0x800); // bit11: ct_en
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_CFG, 0x0, 0x2000); // bit13: ext_mtx_en
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_CFG, DP_COLOR_GET_SWAP_ENABLE(config.outFormat), 0x10000); // bit16: byte_swap
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_CFG, fmt_matrix, 0xF000000); // bit24~27: int_mtx_sel
#endif

    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_DST_ADDR0, config.memAddr[0], 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_DST_ADDR_OFFSET0, 0x0, 0xFFFFFFFF);

    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_DST_ADDR_OFFSET1, 0x0, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_DST_ADDR_OFFSET2, 0x0, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_DST_W_IN_BYTE, config.outYPitch, 0xFFFFFFFF);
    MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_ALPHA, 0, 0xFFFFFFFF);

    if (DP_COLOR_GET_PLANE_COUNT(config.outFormat)>1)
    {
        MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_DST_UV_PITCH, config.outUVPitch, 0x0FFFF);
        MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_DST_ADDR1, config.memAddr[1], 0xFFFFFFFF);
        MM_REG_WRITE(command, m_Offset + DISP_REG_WDMA_DST_ADDR2, config.memAddr[2], 0xFFFFFFFF);
    }
    //FIXME: put this to a better place
    // Trigger & Wait frame done
    MM_REG_WRITE(command, MM_MUTEX_EN, 0x1, 0xFFFFFFFF);
#if 1
    MM_REG_WAIT(command, DpCommand::DISP_WDMA0_FRAME_DONE);
    //MM_REG_WAIT will clear the flag, so need to reset it
    //TODO: add another API MM_REG_WAIT_NO_CLEAR() ?
    MM_REG_SET_EVENT(command, DpCommand::DISP_WDMA0_FRAME_DONE);
    // fix issue: screen flash when exit VP, PUT HERE because onDeInitEngine() is not called every frame
    MM_REG_WRITE(command, DISP_REG_OVL_SRC_CON, 0x0, 0xFFFFFFFF);
#else
    MM_REG_WAIT_NO_CLEAR(command, DpCommand::DISP_WDMA0_FRAME_DONE);
#endif
    return DP_STATUS_RETURN_SUCCESS;
}

#ifdef CONFIG_FOR_OS_ANDROID
DpEngineBase* COLOREXFactory(DpEngineType type)
{
    if(tCOLOR_EX == type)
    {
        return new DpEngine_COLOR_EX(0);
    }

    return 0;
};

DpEngineBase* OVL0EXFactory(DpEngineType type)
{
    if(tOVL0_EX == type)
    {
        return new DpEngine_OVL0_EX(0);
    }

    return 0;
};

DpEngineBase* WDMAEXFactory(DpEngineType type)
{
    if(tWDMA_EX == type)
    {
        return new DpEngine_WDMA_EX(0);
    }

    return 0;
};

#else
static DpEngineBase* COLOREXFactory(DpEngineType type)
{
    if(tCOLOR_EX == type)
    {
        return new DpEngine_COLOR_EX(0);
    }

    return 0;
};


static DpEngineBase* OVL0EXFactory(DpEngineType type)
{
    if(tOVL0_EX == type)
    {
        return new DpEngine_OVL0_EX(0);
    }

    return 0;
};


static DpEngineBase* WDMAEXFactory(DpEngineType type)
{
    if(tWDMA_EX == type)
    {
        return new DpEngine_WDMA_EX(0);
    }

    return 0;
};
#endif  // CONFIG_FOR_OS_ANDROID

EngineReg COLOREXReg(COLOREXFactory);
EngineReg OVL0EXReg(OVL0EXFactory);
EngineReg WDMAEXReg(WDMAEXFactory);

#endif // CONFIG_FOR_SOURCE_PQ

DP_STATUS_ENUM DpEngine_TDSHP::onReconfigTiles(DpCommand &command)
{
    if (command.getSyncMode()) //old version pq readback must remove
    {
        DPLOGI("DpEngine_TDSHP::onReconfigTiles : SyncMode do nothing\n");
        return DP_STATUS_RETURN_SUCCESS;
    }

    uint32_t index;
    bool pq_readback;
    bool hdr_readback;
    int32_t dre_readback;
    uint64_t engineFlag;
    uint32_t VEncFlag;
    uint32_t counter = 0;
    uint32_t* readbackPABuffer = NULL;
    uint32_t readbackPABufferIndex = 0;

    command.getReadbackStatus(pq_readback, hdr_readback, dre_readback, engineFlag, VEncFlag);

    if ((((engineFlag >> tVENC) & 0x1) && VEncFlag) || !(pq_readback))
    {
        DPLOGI("DpEngine_TDSHP::onReconfigTiles : VENC and no readback do nothing\n");
        return DP_STATUS_RETURN_SUCCESS;
    }

    readbackPABuffer = command.getReadbackPABuffer(readbackPABufferIndex);
    command.setNumReadbackPABuffer(m_prevReadbackCount, tTDSHP0);
    if (readbackPABufferIndex == m_prevPABufferIndex)
    {
        DPLOGI("DpEngine_TDSHP::onReconfigTiles : PABufferIndex no change do nothing\n");
        return DP_STATUS_RETURN_SUCCESS;
    }

    if (readbackPABuffer == NULL)
    {
        DPLOGW("DpEngine_TDSHP::onReconfigTiles : readbackPABuffer has been destroyed readbackPABuffer = %p, readbackPABufferIndex = %d\n", readbackPABuffer, readbackPABufferIndex);
        return DP_STATUS_RETURN_SUCCESS;
    }

    if (pq_readback)
    {
        uint32_t iTDSHPBase = /*((engineFlag >> tTDSHP0) & 0x1) ?*/ MDP_TDSHP0_BASE /*: MDP_TDSHP1_BASE*/;

        for (index = 0x6C; index <= 0xB4; index += 4)
        {
            if (index == 0x88)
                continue;

            MM_REG_READ(command, iTDSHPBase + index, readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], NULL, m_regLabel[counter]);
            counter++;
        }

        //For shit usage to read color info by Algo's requests
        #ifdef CONFIG_FOR_SOURCE_PQ
                MM_REG_READ(command, COLOREX_COLOR_TWO_D_W1_RESULT, readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], NULL, m_regLabel[counter]);
                counter++;
        #else
            #ifdef CMD_GPR_R32
                MM_REG_READ(command, CMD_GPR_R32(CMDQ_DATA_REG_PQ_COLOR), readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], NULL, m_regLabel[counter]);
                counter++;
            #else
                MM_REG_READ(command, iTDSHPBase + 0x264, readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], NULL, m_regLabel[counter]);
                counter++;
            #endif
        #endif

#if DYN_CONTRAST_VERSION == 2
        for (index = 0x3DC; index <= 0x41C; index += 4)
        {
            MM_REG_READ(command, iTDSHPBase + index, readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], NULL, m_regLabel[counter]);
            counter++;
        }
#endif
    }

    m_prevPABufferIndex = readbackPABufferIndex;

    return DP_STATUS_RETURN_SUCCESS;
}
