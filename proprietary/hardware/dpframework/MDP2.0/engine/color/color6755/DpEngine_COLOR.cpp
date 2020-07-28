#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "mdp_reg_color.h"
#include "mmsys_config.h"
#if CONFIG_FOR_VERIFY_FPGA
#include "ConfigInfo.h"
#endif

#if CONFIG_FOR_OS_ANDROID
#include <cutils/properties.h>
#include "cust_color.h"
#include "cust_tdshp.h"
#include "ddp_drv.h"

#include "PQSessionManager.h"
#include "PQAlgorithmFactory.h"


#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <utils/threads.h>

#endif
#define  COLOR_CONFIG_FRAME_THRES (2)
/*
#define PURP_TONE    0
#define SKIN_TONE    1
#define GRASS_TONE   2
#define SKY_TONE     3

#define PURP_TONE_START    0
#define PURP_TONE_END      2
#define SKIN_TONE_START    3
#define SKIN_TONE_END     10
#define GRASS_TONE_START  11
#define GRASS_TONE_END    16
#define SKY_TONE_START    17
#define SKY_TONE_END      19

#define SG1 0
#define SG2 1
#define SG3 2
#define SP1 3
#define SP2 4
*/
//--------------------------------------------------------
// COLOR driver engine
//--------------------------------------------------------
class DpEngine_COLOR: public DpTileEngine
{
public:
    DpEngine_COLOR(uint32_t identifier)
        : DpTileEngine(identifier)
    {
        PQColorConfig* pPQColorConfig = new PQColorConfig;

        bool isColorEnable = pPQColorConfig->isEnabled(MEDIA_UNKNOWN);

        m_enableLog = DpDriver::getInstance()->getEnableLog();

        if (isColorEnable == false && m_enableLog != 0){
            DPLOGD("[COLOR] DpEngine_COLOR: MDP COLOR disabled\n");
        }

        m_mdpColor = DpDriver::getInstance()->getMdpColor();

        delete pPQColorConfig;
    }

    ~DpEngine_COLOR()
    {
    }

private:
    DP_STATUS_ENUM onInitEngine(DpCommand&);

    DP_STATUS_ENUM onDeInitEngine(DpCommand&);

    DP_STATUS_ENUM onConfigFrame(DpCommand&,
                                 DpConfig&);

    DP_STATUS_ENUM onConfigTile(DpCommand&);

    void onConfigLumaEngine(DpCommand &command, int32_t scenario);
    void onConfigSatEngine(DpCommand &command);
    void onConfigHueEngine(DpCommand &command);
    void onConfigLocal3DLUT(DpCommand &command);
    bool isColorIndexRangeValid(void);

    int64_t onQueryFeature()
    {
        return eCOLOR;
    }

#if CONFIG_FOR_OS_ANDROID
    int32_t m_enableLog;
    int32_t m_mdpColor;
#endif
};

// Register factory function
static DpEngineBase* COLOR0Factory(DpEngineType type)
{
    if (tCOLOR0 == type)
    {
        return new DpEngine_COLOR(0);
    }
    return NULL;
};

#ifdef tCOLOR1
// Register factory function
static DpEngineBase* COLOR1Factory(DpEngineType type)
{
    if (tCOLOR1 == type)
    {
        return new DpEngine_COLOR(1);
    }
    return NULL;
};
#endif

// Register factory function
EngineReg COLOR0Reg(COLOR0Factory);
#ifdef tCOLOR1
EngineReg COLOR1Reg(COLOR1Factory);
#endif

DP_STATUS_ENUM DpEngine_COLOR::onInitEngine(DpCommand &command)
{
    if (m_mdpColor)
    {
        MM_REG_WRITE(command, DISP_COLOR_START, 0x1, 0x3);
        command.addMetLog("MDP_COLOR__DISP_COLOR_START", 1);
        MM_REG_WRITE(command, DISP_COLOR_WIN_X_MAIN, 0xFFFF0000, 0xFFFFFFFF);
        MM_REG_WRITE(command, DISP_COLOR_WIN_Y_MAIN, 0xFFFF0000, 0xFFFFFFFF);

        // R2Y/Y2R are disabled in MDP
        MM_REG_WRITE(command, DISP_COLOR_CM1_EN, 0x0, 0x1);
        MM_REG_WRITE(command, DISP_COLOR_CM2_EN, 0x0, 0x1);

        //enable interrupt
        MM_REG_WRITE(command, DISP_COLOR_INTEN, 0x00000007, 0x00000007);

        //Set 10bit->8bit Rounding
        MM_REG_WRITE(command, DISP_COLOR_OUT_SEL, 0x333, 0x333);

#ifdef MDP_COLOR_IN_SOUT_SEL
        //Go through color
        MM_REG_WRITE(command, MDP_COLOR_IN_SOUT_SEL, 0x1, 0xF);
        MM_REG_WRITE(command, DISP_COLOR_OUT_SEL_IN, 0x1, 0xF);
#ifdef MDP_COLOR_OUT_SEL_IN_MASK
        MM_REG_WRITE(command, MDP_COLOR_OUT_SEL_IN, 0x1, 0xF);
#endif
    }
    else
    {
        //Pass color
        MM_REG_WRITE(command, MDP_COLOR_IN_SOUT_SEL, 0x0, 0xF);
#endif
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_COLOR::onDeInitEngine(DpCommand&)
{
    // Disable COLOR

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_COLOR::onConfigFrame(DpCommand &command, DpConfig &config)
{
    if (m_mdpColor)
    {
        DpTimeValue    begin;
        DpTimeValue    end;
        int32_t        diff;
        DP_TIMER_GET_CURRENT_TIME(begin);

        PQSession* pPQSession = PQSessionManager::getInstance()->getPQSession(config.pqSessionId);
        if (pPQSession != NULL)
        {
            PQColorAdaptor* pPQColorAdaptor = PQAlgorithmFactory::getInstance()->getColor(m_identifier);
            pPQColorAdaptor->calRegs(pPQSession, command);
        }

        DP_TIMER_GET_CURRENT_TIME(end);
        DP_TIMER_GET_DURATION_IN_MS(begin, end, diff);

        if (diff > COLOR_CONFIG_FRAME_THRES)
        {
            DPLOGD("DpEngine_COLOR: configFrame() time %d ms\n", diff);
        }
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_COLOR::onConfigTile(DpCommand &command)
{
    if (m_mdpColor)
    {
        uint32_t COLOR_in_hsize;
        uint32_t COLOR_in_vsize;

        // Set source size
        COLOR_in_hsize    = m_outTileXRight  - m_outTileXLeft + 1;
        COLOR_in_vsize    = m_outTileYBottom - m_outTileYTop + 1;

        MM_REG_WRITE(command, DISP_COLOR_INTERNAL_IP_WIDTH, COLOR_in_hsize, 0x00003FFF);
        MM_REG_WRITE(command, DISP_COLOR_INTERNAL_IP_HEIGHT, COLOR_in_vsize, 0x00003FFF);
    }

    return DP_STATUS_RETURN_SUCCESS;
}
