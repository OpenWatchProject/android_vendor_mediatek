#ifndef __PQSESSION_H__
#define __PQSESSION_H__

#include <list>

#include "PQTimer.h"

#include "DpDataType.h"
#include "DpCondition.h"
#include "PQMutex.h"
#include "PQDCImpl.h"

#include "PQDCConfig.h"
#include "PQDSConfig.h"
#include "PQColorConfig.h"
#include "PQRSZConfig.h"

#ifdef SUPPORT_HDR
#include "PQHDRImpl.h"
#include "PQHDRConfig.h"
#endif

#if 0
#ifdef SUPPORT_CCORR
#include "PQCcorrConfig.h"
#endif
#endif

#ifdef SUPPORT_DRE
#include "PQDREConfig.h"
#include "mdpAALVPImpl.h"
#endif

#ifdef SUPPORT_HFG
#include "PQHFGConfig.h"
#endif

#define DC_LUMA_HISTOGRAM_NUM (LUMA_HIST_NUM_WEIGHT + 1 + 1) // luma bin + luma sum + color hist

#if DYN_CONTRAST_VERSION == 2
#define DC_CONTOUR_HISTOGRAM_NUM (17)
#else
#define DC_CONTOUR_HISTOGRAM_NUM (0)
#endif

#define TOTAL_HISTOGRAM_NUM (DC_LUMA_HISTOGRAM_NUM + DC_CONTOUR_HISTOGRAM_NUM)

#define HDR_TOTAL_HISTOGRAM_NUM (57)
#define HDR_TOTAL_LETTERBOXPOS_INFO (1)
#define HDR_REGINFO_SIZE (HDR_TOTAL_HISTOGRAM_NUM + HDR_TOTAL_LETTERBOXPOS_INFO)

struct PQDCHist
{
    uint32_t hist[TOTAL_HISTOGRAM_NUM];
    int32_t  ref;
};

typedef std::list<PQDCHist> PQDCHistList;

struct dcHandle;
typedef struct dcHandle {
    PQTimeValue     workTime;
    CPQDCFW*        pADLFW;
    PQDCHistList*   pWaitingHistList;
    PQDCHistList*   pDoneHistList;
    DpCondition*    pHistListCond;
    int8_t          count = 0;
    bool            isHDRContent;
    bool            isAvailable;
}dcHandle;

#ifdef SUPPORT_HDR
struct PQHDRRegInfo
{
    uint32_t hist[HDR_TOTAL_HISTOGRAM_NUM];
    uint32_t LetterBoxPos;
    int32_t  ref;
};

typedef std::list<PQHDRRegInfo> PQHDRRegInfoList;

struct HDRHandle;
typedef struct HDRHandle {
    PQTimeValue         workTime;
    CPQHDRFW*           pHDRFW;
    PQHDRRegInfoList*   pWaitingHistList;
    PQHDRRegInfoList*   pDoneHistList;
    DpCondition*        pHistListCond;
    DHDROUTPUT          initHDROutput;
    bool                isAvailable;
}HDRHandle;
#endif

struct PQSessionHandle;
typedef struct PQSessionHandle {
    uint64_t    id;
    DpPqParam   PQParam;
    dcHandle*   DCHandle;
    bool        m_svp;
#ifdef SUPPORT_HDR
    HDRHandle*  HDRHandle;
#endif
}PQSessionHandle;
class PQSession
{
public:
    PQSession(uint64_t id);
    ~PQSession();

    uint64_t getID(void);
    int32_t  getScenario(void);
    dcHandle* getDCHandle(void);
    void setDCHandle(dcHandle *pDCHandle);
    void deleteDCHandle();
    void setSVP(bool value);
    bool getSVP();
#ifdef SUPPORT_GAME_DRE
    bool getDSReg(const DpPqParam *pParam, uint32_t *DSInfo);
#endif
#ifdef SUPPORT_HDR
    HDRHandle* getHDRHandle(void);
#endif
    bool getDpPqConfig(DpPqConfig **pConfig);
    uint64_t  getPQParam(DpPqParam * param);
    bool setPQparam(const DpPqParam *pParam);
    void setHistogram(uint32_t *pHist, uint32_t size);

#ifdef SUPPORT_HDR
    void setHDRRegInfo(uint32_t *pHDRRegInfo, uint32_t size);
#endif

    bool getDCConfig(DC_CONFIG_T* pDCConfig);
    bool getDSConfig(DS_CONFIG_T** pDSConfig);

#if defined(MDP_COLOR_ENABLE) || defined(CONFIG_FOR_SOURCE_PQ)
    bool getColorConfig(COLOR_CONFIG_T** pColorConfig);
#endif

#ifdef RSZ_2_0
    bool getRSZConfig(RSZ_CONFIG_T * pRSZConfig);
#endif

#ifdef SUPPORT_HDR
    bool getHDRConfig(HDR_CONFIG_T * pHDRConfig);
    bool getHDRInfo(const DpPqParam *PqParam, DP_VDEC_DRV_COLORDESC_T *HDRInfo);
#endif

#if 0
#ifdef SUPPORT_CCORR
    bool getCcorrConfig(CCORR_CONFIG_T **pCcorrConfig);
#endif
#endif

#ifdef SUPPORT_DRE
#ifdef SUPPORT_GAME_DRE
    bool getDREReg(const DpPqParam *pParam, DREReg *DREInfo);
#endif
    bool getDREConfig(DRE_CONFIG_T *pDREConfig);
    void setDrePreviousSize(int32_t prevWidth, int32_t prevHeight);
    void getDrePreviousSize(int32_t *prevWidth, int32_t *prevHeight);
#endif

#ifdef SUPPORT_HFG
    bool getHFGConfig(HFG_CONFIG_T *pHFGConfig);
#endif

private:
    void initDpPqConfig(DpPqConfig *pConfig);
    void setDpPqConfig(DpPqConfig *pConfig, const DpPqParam *pParam);
    void setPictureDpPqConfig(DpPqConfig *pConfig, const DpPqParam *pParam);
    void setVideoDpPqConfig(DpPqConfig *pConfig, const DpPqParam *pParam);
    void setCameraDpPqConfig(DpPqConfig *pConfig, const DpPqParam *pParam);
    void _deleteDCHandle();

public:
    GlobalPQParam globalPQParam;

private:
    PQMutex             s_ALMutex;
    PQSessionHandle*    m_pPQSessionHandle;

    PQDSConfig*         m_pPQDSConfig;
    PQDCConfig*         m_pPQDCConfig;

#if defined(MDP_COLOR_ENABLE) || defined(CONFIG_FOR_SOURCE_PQ)
    PQColorConfig*      m_pPQColorConfig;
#endif

#ifdef RSZ_2_0
    PQRSZConfig*        m_pPQRSZConfig;
#endif

#ifdef SUPPORT_HDR
    PQHDRConfig*        m_pPQHDRConfig;
#endif

    DpPqConfig          m_DpPqConfig;

#if 0
#ifdef SUPPORT_CCORR
    PQCcorrConfig*      m_pPQCcorrConfig;
#endif
#endif

#ifdef SUPPORT_DRE
    int32_t             m_prevWidth;
    int32_t             m_prevHeight;
    PQDREConfig*        m_pPQDREConfig;
#endif

#ifdef SUPPORT_HFG
    PQHFGConfig*        m_pPQHFGConfig;
#endif
};
#endif //__PQSESSION_H__
