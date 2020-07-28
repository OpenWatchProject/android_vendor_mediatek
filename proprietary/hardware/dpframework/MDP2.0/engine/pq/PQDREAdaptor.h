#ifndef __PQDREADAPTOR_H__
#define __PQDREADAPTOR_H__

#include "PQMutex.h"
#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "PQSession.h"
#include "PQAlgorithmAdaptor.h"

#include "AdaptiveCALTM.h"
#include "mdpAALVPImpl.h"

#define DRE_ENGINE_MAX_NUM (1)

#define DRE30_HIST_START         (1024)
#define DRE30_HIST_END           (4092)
#define DRE30_GAIN_START         (4096)
#define DRE30_GAIN_POINT16_START (6144)
#define DRE30_GAIN_END           (6268)
#define PQ_TRANS_BY_SMEN (1)

#if PQ_TRANS_BY_SMEN
//Game_PQ
struct DRETransInfo
{
    USER_INFO_SCENARIO_ENUM user;
    uint32_t dre_blk_x_num;
    uint32_t dre_blk_y_num;
};
#endif

class PQDREAdaptor: public PQAlgorithmAdaptor
{
public:
    PQDREAdaptor(uint32_t identifier);
    ~PQDREAdaptor();

    static PQDREAdaptor* getInstance(uint32_t identifier);
    static void destroyInstance();
    int32_t getPQScenario(PQSession *pPQSession);
    void calRegs(PQSession* pPQSession, DpCommand &command, int32_t imWidth, int32_t imHeight,
        int32_t frameWidth, int32_t frameHeight, int32_t *pFrameConfigLabel, const bool curveOnly);
    void calTileRegs(DpCommand &command,
        const uint32_t hSize, const uint32_t vSize,
        const uint32_t inTileStart, const uint32_t inTileEnd,
        const uint32_t outTileStart, const uint32_t outTileEnd,
        const uint32_t winXStart, const uint32_t winXEnd,
        const uint32_t roiHSize, const uint32_t roiVSize, int32_t scenario);

private:
    void tuningDREInput(void *input, int32_t scenario);
    bool tuningDREOutput(void *output, int32_t scenario);
    void tuningDRESWReg(CDRETopFW *pDREFW, int32_t scenario);
    void tuningDREVPSWReg(CDREVPTopFW *pDREFW, int32_t scenario);
    void tuningToolReadDREReg(CDREVPTopFW *pDREFW, int32_t scenario);
    bool getDRETable();
#if PQ_TRANS_BY_SMEN
//Game_PQ
    bool getDRETransInfo();
    bool getDRETransPrefloatBuffer();
    bool getDRETransFinalCoefBuffer();
    bool getDRETransCurtgtBuffer();
    bool getDRESkinWgtSetBuffer();
#endif
    bool getDREOutput();
#if PQ_TRANS_BY_SMEN
//Game_PQ
    bool setDRETransPrefloatBuffer(DREsharedata *transParam);
    bool setDRETransFinalCoefBuffer(DREsharedata *transParam);
    bool setDRETransCurtgtBuffer(DREsharedata *transParam);
    bool setDRETransInfo(DRETransInfo *transInfo);
    bool setDRESkinWgtSetBuffer(DREsharedata *transParam);
#endif
    bool setDREOutput(DRETopOutput *outParam);

    void initDREFWparam(const int32_t scenario, int32_t imWidth, int32_t imHeight, int32_t frameWidth, int32_t frameHeight);
    void onCalculateIspImpl(const bool isBypass, DpPqParam &PQParam, DRETopOutput *outParam, CDRETopFW *pDREFW);
    void onCalculateVideoImpl(PQSession *pPQSession, DpPqParam &PQParam, const bool isBypass, DRETopOutput *outParam, CDREVPTopFW *pDREFW,
        const bool isHDR, const uint32_t selTable);
    void onCalculate(PQSession *pPQSession, DpPqParam &PQParam, const bool isBypass, const uint32_t modifyDREBlock,
        DRETopOutput *outParam, bool *isApplyFw, int32_t frameWidth, int32_t frameHeight);
    void initDREOutput(DRETopOutput *outParam);
    void getDefaultDREOutput(uint32_t modifyDREBlk, DRETopOutput *outParam);
    void getDefaultDREVPOutput(uint32_t modifyDREBlk, DRETopOutput *outParam);

    void sramWrite(DpCommand &command, unsigned int addr, unsigned int value,
        int32_t *pFrameConfigLabel, int32_t &index, const bool curveOnly);
    void writeBlock(DpCommand &command, const DRETopOutput *output,
        const uint32_t block_x, const uint32_t block_y,
        const uint32_t dre_blk_x_num, const uint32_t dre_blk_y_num,
        int32_t *pFrameConfigLabel, int32_t &index, const bool curveOnly);
    void writeCurve16(DpCommand &command, const DRETopOutput *output,
        const uint32_t dre_blk_x_num, const uint32_t dre_blk_y_num,
        int32_t *pFrameConfigLabel, int32_t &index, const bool curveOnly);
    void writeDRE30Curve(DpCommand &command, const DRETopOutput *output,
        const uint32_t dre_blk_x_num, const uint32_t dre_blk_y_num,
        int32_t *pFrameConfigLabel, int32_t &index, const bool curveOnly);
    void debugDumpFWInput(const DRETopInput *inParam);
    void debugDumpFrameReg(const DRETopOutput *outParam, const DREInitParam *initDREParam, const DREInitReg *initDREReg);
    void debugDumpTileReg(const DREInitParam *initDREParam, const DREInitReg *initDREReg,
        const uint32_t outTileStart, const uint32_t outTileEnd);
    template <class regType>
    void writeStringWithTag(const char *pTag, const regType *pSrc, char *pDst, int &cnt);
    void dumpFwReg(const bool isApplyFw, const DpPqParam *PQParam,
        const DRETopInput *inReg, const DRETopOutput *outReg, const DREInitReg *initReg, const DREReg *swReg,
        const TAdaptiveCALTMReg *adaptiveCALTMReg, const TAdaptiveCALTMFace_Exif *adaptiveCALTMFace_exif, const TAdaptiveCALTMFace_Dump *adaptiveCALTMFace_dump);
    void dumpFwRegInFw(const DpPqParam *PQParam);
#ifdef PQ_TRANS_BY_SMEN
    void debugDumpTransData(const DRETransInfo *initDRETransInfo,
            const DREsharedata *initDREsharedata);
#endif

private:
    static PQDREAdaptor *s_pInstance[DRE_ENGINE_MAX_NUM];
    static PQMutex  s_ALMutex;

    uint32_t       m_identifier;
    DRE_CONFIG_T   m_pDREConfig;
    CDRETopFW      *m_pDREFW;
    CDREVPTopFW    *m_pDREVPFW;
    TAdaptiveCALTM *m_pAdaptiveCALTMFW;
    DRETopInput    m_pDREInput;
    DREInitParam   m_initDREParam;
    DRETopOutput   m_defaultDREOutput;
    DRETopOutput   m_defaultDREVPOutput;
    uint32_t       m_enableDumpRegister;
    uint64_t       m_prePQID;
    int32_t        m_preFrameWidth;
    int32_t        m_preFrameHeight;
    DREReg         m_lastestDREInfo;
#ifdef PQ_TRANS_BY_SMEN
    DREsharedata   m_pDREsharedata;
    DRETransInfo   m_pDRETransInfo;
#endif

};

#endif //__PQDREADAPTOR_H__
