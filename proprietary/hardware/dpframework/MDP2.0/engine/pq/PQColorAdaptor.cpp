#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/properties.h>
#include <cutils/log.h>
#include <fcntl.h>

#include <PQCommon.h>
#include <PQColorAdaptor.h>
#include "PQTuningBuffer.h"

#include "mdp_reg_color.h"

enum TONE_RANGE{
    PURP_TONE_START   = 0,
    PURP_TONE_END     = 2,
    SKIN_TONE_START   = 3,
    SKIN_TONE_END    = 10,
    GRASS_TONE_START = 11,
    GRASS_TONE_END   = 16,
    SKY_TONE_START   = 17,
    SKY_TONE_END     = 19
};
enum SAT_CP{
    SG1 = 0,
    SG2 = 1,
    SG3 = 2,
    SP1 = 3,
    SP2 = 4
};

enum WINDOW_SETTING{
    WIN1 = 0,
    WIN2,
    WIN3,
    WIN_TOTAL
};

enum LUT_YHS {
    LUT_H = 0,
    LUT_Y,
    LUT_S,
    LUT_TOTAL
};

enum LUT_REG {
    REG_SLOPE0 = 0,
    REG_SLOPE1,
    REG_SLOPE2,
    REG_SLOPE3,
    REG_SLOPE4,
    REG_SLOPE5,
    REG_WGT_LSLOPE,
    REG_WGT_USLOPE,
    REG_L,
    REG_POINT0,
    REG_POINT1,
    REG_POINT2,
    REG_POINT3,
    REG_POINT4,
    REG_U,
    LUT_REG_TOTAL
};

PQColorAdaptor* PQColorAdaptor::s_pInstance[] = {};
PQMutex   PQColorAdaptor::s_ALMutex;

PQColorAdaptor* PQColorAdaptor::getInstance(uint32_t identifier)
{
    AutoMutex lock(s_ALMutex);

    if(NULL == s_pInstance[identifier])
    {
        s_pInstance[identifier] = new PQColorAdaptor(identifier);
        atexit(PQColorAdaptor::destroyInstance);
    }

    return s_pInstance[identifier];
}

void PQColorAdaptor::destroyInstance()
{
    AutoMutex lock(s_ALMutex);

    for (int identifier = 0; identifier < COLOR_ENGINE_MAX_NUM; identifier++){
        if (NULL != s_pInstance[identifier])
        {
            delete s_pInstance[identifier];
            s_pInstance[identifier] = NULL;
        }
    }
}

PQColorAdaptor::PQColorAdaptor(uint32_t identifier)
        : m_identifier(identifier),
          PQAlgorithmAdaptor(PROXY_COLOR_SWREG,
                             PROXY_COLOR_INPUT,
                             PROXY_COLOR_OUTPUT)
{
    PQ_LOGD("[PQColorAdaptor] PQColorAdaptor()... ");
    m_colorConfig = NULL;
};

PQColorAdaptor::~PQColorAdaptor()
{
    PQ_LOGD("[PQColorAdaptor] ~PQColorAdaptor()... ");
};

bool PQColorAdaptor::tuningColorOutput(COLOR_CONFIG_T *output, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_outputBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(COLOR_CONFIG_T);

    if (p_buffer->isValid() == false) {
        return false;
    }

    if (copy_size > p_buffer->getModuleSize()) {
        copy_size = p_buffer->getModuleSize();
    }

    if (p_buffer->isOverwritten()) {
        if (p_buffer->isSync()) {
            p_buffer->pull();
        }
        memcpy(output, overwritten_buffer, copy_size);
    } else if (scenario == MEDIA_PICTURE) {
        p_buffer->resetReady();
        memcpy(reading_buffer, output, copy_size);
        p_buffer->push();
    } else if (p_buffer->toBePrepared()) {
        memcpy(reading_buffer, output, copy_size);
        p_buffer->push();
    }

    if (p_buffer->isBypassHWAccess()) {
        return true;
    }

    return false;
}

void PQColorAdaptor::calRegs(PQSession* pPQSession, DpCommand &command)
{
    AutoMutex lock(s_ALMutex);

    DpPqConfig *DpPqConfig;

    pPQSession->getDpPqConfig(&DpPqConfig);

    pPQSession->getColorConfig(&m_colorConfig);

    int32_t scenario = pPQSession->getScenario();

    PQ_LOGI("[PQColorAdaptor] onConfigFrame: PQParam.scenario =  %d\n", scenario);

    if (m_colorConfig->BYPASS == 0)
    {
        m_colorConfig->BYPASS = !DpPqConfig->enColor;
    }

    bool bypassHWAccess = tuningColorOutput(m_colorConfig, scenario);

    if (bypassHWAccess)
    {
        return;
    }

    MM_REG_WRITE(command, DISP_COLOR_CFG_MAIN, (m_colorConfig->LSP_SAT_LIMIT << 21) | (m_colorConfig->LSP_EN << 20)
                                               | (m_colorConfig->S_GAIN_BY_Y_EN << 15)
                                               | m_colorConfig->BYPASS << 7, 0x0030808F);

    onConfigLumaEngine(m_colorConfig, command, scenario);
    onConfigSatEngine(m_colorConfig, command);
    onConfigHueEngine(m_colorConfig, command);
    onConfigLocal3DLUT(m_colorConfig, command);

    // color window
    MM_REG_WRITE(command, DISP_COLOR_TWO_D_WINDOW_1, m_colorConfig->TWO_D_WINDOW, 0xFFFFFFFF);

    return ;
}

void PQColorAdaptor::onConfigLumaEngine(COLOR_CONFIG_T* colorConfig, DpCommand &command, int32_t scenario)
{
    int index = 0;

    // for partial Y contour issue
    MM_REG_WRITE(command, DISP_COLOR_LUMA_ADJ, colorConfig->Y_LEV_ADJ, 0x0000007F);
    // Chroma boost setting by scenario
    MM_REG_WRITE(command, DISP_COLOR_C_BOOST_MAIN_2, colorConfig->CBOOST_YOFFSET, 0x000000FF);

    MM_REG_WRITE(command, DISP_COLOR_C_BOOST_MAIN, colorConfig->NEW_CBOOST_LMT_L << 16, 0x00FF0000);
#ifdef COLOR_2_1
    MM_REG_WRITE(command, DISP_COLOR_C_BOOST_MAIN_2, colorConfig->CBOOST_YCONST << 24, 0xFF000000);
#endif

    // config parameter from customer color_index.h

    MM_REG_WRITE(command, DISP_COLOR_G_PIC_ADJ_MAIN_1, ( colorConfig->BRIGHTNESS << 16) | colorConfig->CONTRAST, 0x07FF01FF);
    MM_REG_WRITE(command, DISP_COLOR_G_PIC_ADJ_MAIN_2, colorConfig->GLOBAL_SAT, 0x000001FF);
    PQ_LOGI("[PQColorAdaptor] onConfigFrame: colorConfig->BRIGHTNESS[m_ColorParam.u4Brightness] =  %d, colorConfig->CONTRAST[m_ColorParam.u4Contrast] = %d, colorConfig->GLOBAL_SAT[m_ColorParam.u4SatGain]  = %d\n",colorConfig->BRIGHTNESS,colorConfig->CONTRAST,colorConfig->GLOBAL_SAT);

    for (index = 0; index < 8; index++)
    {
        MM_REG_WRITE_MASK(command, DISP_COLOR_Y_SLOPE_1_0_MAIN + 4 * index, ( colorConfig->PARTIAL_Y[2 * index ] |  colorConfig->PARTIAL_Y[2 * index + 1]<<16), 0x00FF00FF);
    }
}

void PQColorAdaptor::onConfigSatEngine(COLOR_CONFIG_T* colorConfig, DpCommand &command)
{
    // Partial Saturation
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_GAIN1_0, (  colorConfig->PURP_TONE_S[SG1][0] |  colorConfig->PURP_TONE_S[SG1][1] << 8 |  colorConfig->PURP_TONE_S[SG1][2] << 16 |  colorConfig->SKIN_TONE_S[SG1][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_GAIN1_1, (  colorConfig->SKIN_TONE_S[SG1][1] |  colorConfig->SKIN_TONE_S[SG1][2] << 8 |  colorConfig->SKIN_TONE_S[SG1][3] << 16 |  colorConfig->SKIN_TONE_S[SG1][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_GAIN1_2, (  colorConfig->SKIN_TONE_S[SG1][5] |  colorConfig->SKIN_TONE_S[SG1][6] << 8 |  colorConfig->SKIN_TONE_S[SG1][7] << 16 |  colorConfig->GRASS_TONE_S[SG1][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_GAIN1_3, (  colorConfig->GRASS_TONE_S[SG1][1] |  colorConfig->GRASS_TONE_S[SG1][2] << 8 |  colorConfig->GRASS_TONE_S[SG1][3] << 16 |  colorConfig->GRASS_TONE_S[SG1][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_GAIN1_4, (  colorConfig->GRASS_TONE_S[SG1][5] |  colorConfig->SKY_TONE_S[SG1][0] << 8 |  colorConfig->SKY_TONE_S[SG1][1] << 16 |  colorConfig->SKY_TONE_S[SG1][2] << 24 ), 0xFFFFFFFF);

    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_GAIN2_0, (  colorConfig->PURP_TONE_S[SG2][0] |  colorConfig->PURP_TONE_S[SG2][1] << 8 |  colorConfig->PURP_TONE_S[SG2][2] << 16 |  colorConfig->SKIN_TONE_S[SG2][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_GAIN2_1, (  colorConfig->SKIN_TONE_S[SG2][1] |  colorConfig->SKIN_TONE_S[SG2][2] << 8 |  colorConfig->SKIN_TONE_S[SG2][3] << 16 |  colorConfig->SKIN_TONE_S[SG2][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_GAIN2_2, (  colorConfig->SKIN_TONE_S[SG2][5] |  colorConfig->SKIN_TONE_S[SG2][6] << 8 |  colorConfig->SKIN_TONE_S[SG2][7] << 16 |  colorConfig->GRASS_TONE_S[SG2][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_GAIN2_3, (  colorConfig->GRASS_TONE_S[SG2][1] |  colorConfig->GRASS_TONE_S[SG2][2] << 8 |  colorConfig->GRASS_TONE_S[SG2][3] << 16 |  colorConfig->GRASS_TONE_S[SG2][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_GAIN2_4, (  colorConfig->GRASS_TONE_S[SG2][5] |  colorConfig->SKY_TONE_S[SG2][0] << 8 |  colorConfig->SKY_TONE_S[SG2][1] << 16 |  colorConfig->SKY_TONE_S[SG2][2] << 24 ), 0xFFFFFFFF);

    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_GAIN3_0, (  colorConfig->PURP_TONE_S[SG3][0] |  colorConfig->PURP_TONE_S[SG3][1] << 8 |  colorConfig->PURP_TONE_S[SG3][2] << 16 |  colorConfig->SKIN_TONE_S[SG3][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_GAIN3_1, (  colorConfig->SKIN_TONE_S[SG3][1] |  colorConfig->SKIN_TONE_S[SG3][2] << 8 |  colorConfig->SKIN_TONE_S[SG3][3] << 16 |  colorConfig->SKIN_TONE_S[SG3][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_GAIN3_2, (  colorConfig->SKIN_TONE_S[SG3][5] |  colorConfig->SKIN_TONE_S[SG3][6] << 8 |  colorConfig->SKIN_TONE_S[SG3][7] << 16 |  colorConfig->GRASS_TONE_S[SG3][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_GAIN3_3, (  colorConfig->GRASS_TONE_S[SG3][1] |  colorConfig->GRASS_TONE_S[SG3][2] << 8 |  colorConfig->GRASS_TONE_S[SG3][3] << 16 |  colorConfig->GRASS_TONE_S[SG3][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_GAIN3_4, (  colorConfig->GRASS_TONE_S[SG3][5] |  colorConfig->SKY_TONE_S[SG3][0] << 8 |  colorConfig->SKY_TONE_S[SG3][1] << 16 |  colorConfig->SKY_TONE_S[SG3][2] << 24 ), 0xFFFFFFFF);

    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_POINT1_0, (  colorConfig->PURP_TONE_S[SP1][0] |  colorConfig->PURP_TONE_S[SP1][1] << 8 |  colorConfig->PURP_TONE_S[SP1][2] << 16 |  colorConfig->SKIN_TONE_S[SP1][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_POINT1_1, (  colorConfig->SKIN_TONE_S[SP1][1] |  colorConfig->SKIN_TONE_S[SP1][2] << 8 |  colorConfig->SKIN_TONE_S[SP1][3] << 16 |  colorConfig->SKIN_TONE_S[SP1][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_POINT1_2, (  colorConfig->SKIN_TONE_S[SP1][5] |  colorConfig->SKIN_TONE_S[SP1][6] << 8 |  colorConfig->SKIN_TONE_S[SP1][7] << 16 |  colorConfig->GRASS_TONE_S[SP1][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_POINT1_3, (  colorConfig->GRASS_TONE_S[SP1][1] |  colorConfig->GRASS_TONE_S[SP1][2] << 8 |  colorConfig->GRASS_TONE_S[SP1][3] << 16 |  colorConfig->GRASS_TONE_S[SP1][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_POINT1_4, (  colorConfig->GRASS_TONE_S[SP1][5] |  colorConfig->SKY_TONE_S[SP1][0] << 8 |  colorConfig->SKY_TONE_S[SP1][1] << 16 |  colorConfig->SKY_TONE_S[SP1][2] << 24 ), 0xFFFFFFFF);

    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_POINT2_0, (  colorConfig->PURP_TONE_S[SP2][0] |  colorConfig->PURP_TONE_S[SP2][1] << 8 |  colorConfig->PURP_TONE_S[SP2][2] << 16 |  colorConfig->SKIN_TONE_S[SP2][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_POINT2_1, (  colorConfig->SKIN_TONE_S[SP2][1] |  colorConfig->SKIN_TONE_S[SP2][2] << 8 |  colorConfig->SKIN_TONE_S[SP2][3] << 16 |  colorConfig->SKIN_TONE_S[SP2][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_POINT2_2, (  colorConfig->SKIN_TONE_S[SP2][5] |  colorConfig->SKIN_TONE_S[SP2][6] << 8 |  colorConfig->SKIN_TONE_S[SP2][7] << 16 |  colorConfig->GRASS_TONE_S[SP2][0] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_POINT2_3, (  colorConfig->GRASS_TONE_S[SP2][1] |  colorConfig->GRASS_TONE_S[SP2][2] << 8 |  colorConfig->GRASS_TONE_S[SP2][3] << 16 |  colorConfig->GRASS_TONE_S[SP2][4] << 24 ), 0xFFFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_PARTIAL_SAT_POINT2_4, (  colorConfig->GRASS_TONE_S[SP2][5] |  colorConfig->SKY_TONE_S[SP2][0] << 8 |  colorConfig->SKY_TONE_S[SP2][1] << 16 |  colorConfig->SKY_TONE_S[SP2][2] << 24 ), 0xFFFFFFFF);

#ifdef COLOR_2_1
    /* S Gain By Y */
    unsigned int u4Temp = 0;
    int i, j, reg_index;

    reg_index = 0;

    for (i = 0; i < S_GAIN_BY_Y_CONTROL_CNT; i++) {
        for (j = 0; j < S_GAIN_BY_Y_HUE_PHASE_CNT; j += 4) {
            u4Temp = (colorConfig->S_GAIN_BY_Y[i][j]) +
                     (colorConfig->S_GAIN_BY_Y[i][j + 1] << 8) +
                     (colorConfig->S_GAIN_BY_Y[i][j + 2] << 16) +
                     (colorConfig->S_GAIN_BY_Y[i][j + 3] << 24);
            MM_REG_WRITE_MASK(command, DISP_COLOR_S_GAIN_BY_Y0_0 + reg_index, u4Temp, 0xFFFFFFFF);
            reg_index += 4;
        }
    }
    /* LSP */
    MM_REG_WRITE(command, DISP_COLOR_LSP_1, colorConfig->LSP_1,0x1FFFFFFF);
    MM_REG_WRITE(command, DISP_COLOR_LSP_2, colorConfig->LSP_2,0x3FFF7F7F);
#endif
}

void PQColorAdaptor::onConfigHueEngine(COLOR_CONFIG_T* colorConfig, DpCommand &command)
{
    int index = 0;
    unsigned int u4Temp = 0;
    unsigned char h_series[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    for (index = 0; index < PURP_TONE_SIZE; index++)
    {
        h_series[index+PURP_TONE_START] =  colorConfig->PURP_TONE_H[index];
    }

    for (index = 0; index < SKIN_TONE_SIZE; index++)
    {
        h_series[index+SKIN_TONE_START] =  colorConfig->SKIN_TONE_H[index];
    }

    for (index = 0; index < GRASS_TONE_SIZE; index++)
    {
        h_series[index+GRASS_TONE_START] =  colorConfig->GRASS_TONE_H[index];
    }

    for (index = 0; index < SKY_TONE_SIZE; index++)
    {
        h_series[index+SKY_TONE_START] =  colorConfig->SKY_TONE_H[index];
    }

    for (index = 0; index < 5; index++)
    {
        u4Temp = (h_series[4 * index]) +
                 (h_series[4 * index + 1] << 8) +
                 (h_series[4 * index + 2] << 16) +
                 (h_series[4 * index + 3] << 24);

        MM_REG_WRITE_MASK(command, DISP_COLOR_LOCAL_HUE_CD_0 + 4 * index, u4Temp, 0xFFFFFFFF);
    }
}

void PQColorAdaptor::onConfigLocal3DLUT(COLOR_CONFIG_T* colorConfig, DpCommand &command)
{
#ifdef COLOR_3_0
    int i, j, reg_index;

    MM_REG_WRITE_MASK(command, DISP_COLOR_CM_CONTROL, colorConfig->COLOR_CM_CONTROL, 0x1B7);

    for (i = 0; i < WIN_TOTAL; i++) {
        reg_index = i * 4 * (LUT_TOTAL * 5);
        for (j = 0; j < LUT_TOTAL; j++) {
            MM_REG_WRITE_MASK(command, DISP_COLOR_CM_W1_HUE_0 + reg_index,
                (colorConfig->COLOR_3D[i][j*LUT_REG_TOTAL+REG_L]) |
                (colorConfig->COLOR_3D[i][j*LUT_REG_TOTAL+REG_U] << 10) |
                (colorConfig->COLOR_3D[i][j*LUT_REG_TOTAL+REG_POINT0] << 20), 0xFFFFFFFF);

            MM_REG_WRITE_MASK(command, DISP_COLOR_CM_W1_HUE_1 + reg_index,
                (colorConfig->COLOR_3D[i][j*LUT_REG_TOTAL+REG_POINT1]) |
                (colorConfig->COLOR_3D[i][j*LUT_REG_TOTAL+REG_POINT2] << 10) |
                (colorConfig->COLOR_3D[i][j*LUT_REG_TOTAL+REG_POINT3] << 20), 0xFFFFFFFF);

            MM_REG_WRITE_MASK(command, DISP_COLOR_CM_W1_HUE_2 + reg_index,
                (colorConfig->COLOR_3D[i][j*LUT_REG_TOTAL+REG_POINT4]) |
                (colorConfig->COLOR_3D[i][j*LUT_REG_TOTAL+REG_SLOPE0] << 10) |
                (colorConfig->COLOR_3D[i][j*LUT_REG_TOTAL+REG_SLOPE1] << 20), 0xFFFFFFFF);

            MM_REG_WRITE_MASK(command, DISP_COLOR_CM_W1_HUE_3 + reg_index,
                (colorConfig->COLOR_3D[i][j*LUT_REG_TOTAL+REG_SLOPE2]) |
                (colorConfig->COLOR_3D[i][j*LUT_REG_TOTAL+REG_SLOPE3] << 8) |
                (colorConfig->COLOR_3D[i][j*LUT_REG_TOTAL+REG_SLOPE4] << 16) |
                (colorConfig->COLOR_3D[i][j*LUT_REG_TOTAL+REG_SLOPE5] << 24), 0xFFFFFFFF);

            MM_REG_WRITE_MASK(command, DISP_COLOR_CM_W1_HUE_4 + reg_index,
                (colorConfig->COLOR_3D[i][j*LUT_REG_TOTAL+REG_WGT_LSLOPE]) |
                (colorConfig->COLOR_3D[i][j*LUT_REG_TOTAL+REG_WGT_USLOPE] << 16), 0xFFFFFFFF);

            reg_index += (4 * 5);
        }
    }
#endif
}

