#ifndef __PQCOLORCONFIG_H__
#define __PQCOLORCONFIG_H__

#include "PQMutex.h"
#include "PQConfig.h"

typedef enum COLORCONFIG_ARRAY_SIZE
{
    COLORCONFIG_PARTIAL_Y_SIZE = 16,
    COLORCONFIG_PARTIALS_CONTROL = 5,
    COLORCONFIG_PURP_TONE_SIZE = 3,
    COLORCONFIG_SKIN_TONE_SIZE = 8, /* (-6) */
    COLORCONFIG_GRASS_TONE_SIZE = 6,    /* (-2) */
    COLORCONFIG_SKY_TONE_SIZE = 3,
    COLORCONFIG_S_GAIN_BY_Y_CONTROL_CNT = 5,
    COLORCONFIG_S_GAIN_BY_Y_HUE_PHASE_CNT = 20,
    COLORCONFIG_3D_WINDOW_CNT = 3,
    COLORCONFIG_3D_WINDOW_SIZE = 45
} COLORCONFIG_ARRAY_SIZE;

typedef struct {
    unsigned int GLOBAL_SAT  ;
    unsigned int CONTRAST    ;
    unsigned int BRIGHTNESS  ;
    unsigned int PARTIAL_Y    [COLORCONFIG_PARTIAL_Y_SIZE];
    unsigned int PURP_TONE_S  [COLORCONFIG_PARTIALS_CONTROL][COLORCONFIG_PURP_TONE_SIZE];
    unsigned int SKIN_TONE_S  [COLORCONFIG_PARTIALS_CONTROL][COLORCONFIG_SKIN_TONE_SIZE];
    unsigned int GRASS_TONE_S [COLORCONFIG_PARTIALS_CONTROL][COLORCONFIG_GRASS_TONE_SIZE];
    unsigned int SKY_TONE_S   [COLORCONFIG_PARTIALS_CONTROL][COLORCONFIG_SKY_TONE_SIZE];
    unsigned int PURP_TONE_H  [COLORCONFIG_PURP_TONE_SIZE];
    unsigned int SKIN_TONE_H  [COLORCONFIG_SKIN_TONE_SIZE];
    unsigned int GRASS_TONE_H [COLORCONFIG_GRASS_TONE_SIZE];
    unsigned int SKY_TONE_H   [COLORCONFIG_SKY_TONE_SIZE];
    unsigned int S_GAIN_BY_Y  [COLORCONFIG_S_GAIN_BY_Y_CONTROL_CNT][COLORCONFIG_S_GAIN_BY_Y_HUE_PHASE_CNT];
    unsigned int S_GAIN_BY_Y_EN;
    unsigned int LSP_EN;
    unsigned int BYPASS;
    unsigned int LSP_SAT_LIMIT;
    unsigned int DBG_CFG_MAIN;
    unsigned int TWO_D_WINDOW;
    unsigned int Y_LEV_ADJ;
    unsigned int CBOOST_YOFFSET;
    unsigned int NEW_CBOOST_LMT_L;
    unsigned int CBOOST_YCONST;
    unsigned int LSP_1;
    unsigned int LSP_2;
    unsigned int COLOR_3D[COLORCONFIG_3D_WINDOW_CNT][COLORCONFIG_3D_WINDOW_SIZE];
    unsigned int COLOR_CM_CONTROL;
    unsigned int ENABLE;
} COLOR_CONFIG_T;

class PQColorConfig
{
public:
    PQColorConfig();
    ~PQColorConfig();

    bool isEnabled(int32_t scenario);
    bool getColorConfig(COLOR_CONFIG_T** colorConfig, int32_t scenario);
#ifdef CONFIG_FOR_SOURCE_PQ
    void getDisplayStatus(DISP_PQ_STATUS &status, bool bPerFrame);
    void setColorEXEnable(bool enable);
    bool getColorEXEnable(void);
#endif

private:
    bool initColorTable(void);
    bool isMDPColorMode(void);
    bool composeColorParameter(int32_t scenario);
#ifdef CONFIG_FOR_SOURCE_PQ
    void getDisplayStatusEx(DISP_PQ_STATUS &status);
#endif

public:
    static DISPLAY_PQ_T    m_ColorTable;
    static bool            m_ColorTableInit;

    enum COLOR_MODE_ENUM
    {
        COLOR_OFF = 0,
        DISP_COLOR = 1,
        MDP_COLOR = 2,
        DISP_MDP_COLOR = 3
    };

private:
    PQMutex s_ALMutex;

    bool            m_ColorParameterInit;
    DISP_PQ_PARAM   m_ColorTableIndex;
    COLOR_CONFIG_T  m_ColorConfig;
    int32_t         m_ColorMode;
#ifdef CONFIG_FOR_SOURCE_PQ
    DISP_PQ_STATUS  m_DispStatus;
    bool            m_ColorEXEnable;
#endif
    uint32_t        m_PQParameterSN;
};
#endif //__PQCOLORCONFIG_H__
