#ifndef __PQHDRADAPTOR_H__
#define __PQHDRADAPTOR_H__

#include "PQMutex.h"
#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "PQSession.h"
#include "PQSessionManager.h"
#include "PQHDRImpl.h"
#include "PQAlgorithmAdaptor.h"

#define HDR_ENGINE_MAX_NUM (1)
#define GAIN_TABLE_DATA_MAX (256)

class PQHDRAdaptor:  public PQAlgorithmAdaptor
{
public:
    PQHDRAdaptor(uint32_t identifier);
    ~PQHDRAdaptor();

    static PQHDRAdaptor* getInstance(uint32_t identifier);
    static void destroyInstance();
    bool calRegs(PQSession* pPQSession, DpCommand &command, DpConfig &config,
                        int32_t *pFrameConfigLabel, const bool curveOnly);

private:
    void tuningHDRInput(DHDRINPUT *input, int32_t scenario);
    bool tuningHDROutput(DHDROUTPUT *output, int32_t scenario);
    void tuningHDRSWReg(HDRFWReg *SWReg, int32_t scenario);
    bool getHDRInfo(DP_VDEC_DRV_COLORDESC_T *HDRInfo, DpPqParam &PqParam);
    void initHDRFWinput(DHDRINPUT *input, DpConfig &config);
    int32_t onCalculateCore(const int32_t scenario, DHDRINPUT *input, DHDROUTPUT *output, DpConfig &config);
    int32_t onCalculate(PQSession* pPQSession, DHDRINPUT *input, DHDROUTPUT *output, DpConfig &config,
                        DP_VDEC_DRV_COLORDESC_T *HDRInfo);
    void onConfigHDR(DpCommand &command, DHDROUTPUT *outParam, int32_t *pFrameConfigLabel);
    void onConfigCcorr(DpCommand &command, DHDROUTPUT *outParam);
    void onUpdateHDR(DpCommand &command, DHDROUTPUT *outParam, int32_t *pFrameConfigLabel);
    void initCcorrInitParamIn(DHDRINPUT *input, bool ccorr_en, DpPqParam &PqParam);
    void initHDRInitParamIn(DP_VDEC_DRV_COLORDESC_T *HDRInfo, DHDRINPUT *input,
                        bool isHDR2SDR, uint32_t externalPanelNits, DpConfig &config);
    void checkAndResetUnusedHDRFW(HDRHandle* currHDR_p, uint64_t id);
    bool isHDRinfoChanged(DP_VDEC_DRV_COLORDESC_T *HDRInfo);
    bool isSEIInfoExist(DP_VDEC_DRV_COLORDESC_T *HDRInfo);
    bool getHDRTable(void);
    bool getHDROutput(DHDROUTPUT *outParam, int32_t flag);
    bool setHDROutput(DHDROUTPUT *outParam);
    uint64_t getPQID(void);
    bool setPQID(uint64_t id);

private:
    static PQHDRAdaptor *s_pInstance[HDR_ENGINE_MAX_NUM];
    static PQMutex  s_ALMutex;
    uint32_t        m_identifier;
    uint32_t        m_lastWidth;
    uint32_t        m_lastHeight;
    CPQHDRFW*       m_pHDRFW;
    HDR_CONFIG_T    m_pHDRConfig;
    HDRFWReg        m_initHDRFWReg;
    DP_VDEC_DRV_COLORDESC_T m_lastHDRInfo;
    DHDROUTPUT m_lastOutput;
    PANEL_SPEC      m_internalDispPanelSpec;

typedef enum {
  DNposMask = 0x1FFF0000,
  DNposShift = 16,
  UPposMask = 0x00001FFF,
  UPposShift = 0
} LetterBoxOperation;
};
#endif //__PQHDRADAPTOR_H__
