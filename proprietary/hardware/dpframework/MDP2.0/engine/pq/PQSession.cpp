#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1

#include "PQSession.h"
#include <cutils/log.h>
#include <cutils/properties.h>

#include <dlfcn.h>
#include <PQCommon.h>
#ifdef SUPPORT_PQ_WHITE_LIST
#include "PQWhiteList.h"
#endif
#include <sys/mman.h>
#include "ui/gralloc_extra.h"
#include "graphics_mtk_defs.h"
#include "gralloc_mtk_defs.h"

#include <cutils/properties.h>
#include "PQDSImpl.h"
#include "PQRszImpl.h"
#include "DpDriver.h"
#include <vendor/mediatek/hardware/pq/2.0/IPictureQuality.h>

#include "PQReadBackFactory.h"

using vendor::mediatek::hardware::pq::V2_0::IPictureQuality;
using vendor::mediatek::hardware::pq::V2_0::Result;

#define UNUSED(expr) do { (void)(expr); } while (0)

PQSession::PQSession(uint64_t id)
    : m_pPQSessionHandle(NULL)
{
    AutoMutex lock(s_ALMutex);

    PQ_LOGI("[PQSession] PQSession() %llx... ",id);

#ifdef SUPPORT_PQ_WHITE_LIST
    PQWhiteList &whiteList = PQWhiteList::getInstance();
    whiteList.queryWhiteListConfig();
#endif

    m_pPQSessionHandle = new PQSessionHandle;
    m_pPQSessionHandle->id = id;
    m_pPQSessionHandle->m_svp = false;

    m_pPQSessionHandle->DCHandle = new dcHandle;
    memset(m_pPQSessionHandle->DCHandle, 0, sizeof(dcHandle));
    m_pPQSessionHandle->DCHandle->count++;

#ifdef SUPPORT_HDR
    m_pPQSessionHandle->HDRHandle = new HDRHandle;
    memset(m_pPQSessionHandle->HDRHandle, 0, sizeof(HDRHandle));
#endif

    m_pPQDCConfig = new PQDCConfig;
    m_pPQDSConfig = new PQDSConfig;

#ifdef MDP_COLOR_ENABLE
    m_pPQColorConfig = new PQColorConfig;
#endif

#ifdef RSZ_2_0
    m_pPQRSZConfig = new PQRSZConfig;
#endif

#if defined(SUPPORT_HDR) || defined(SUPPORT_CCORR)
    m_pPQHDRConfig = new PQHDRConfig;
#endif

#if 0
#ifdef SUPPORT_CCORR
    m_pPQCcorrConfig = new PQCcorrConfig;
#endif
#endif

#ifdef SUPPORT_DRE
    m_pPQDREConfig = new PQDREConfig;
    m_prevWidth = -1;
    m_prevHeight = -1;
#endif

    PQ_TIMER_GET_CURRENT_TIME(m_pPQSessionHandle->DCHandle->workTime);
    m_pPQSessionHandle->DCHandle->pWaitingHistList = new PQDCHistList;
    m_pPQSessionHandle->DCHandle->pDoneHistList = new PQDCHistList;
    m_pPQSessionHandle->DCHandle->pHistListCond = new DpCondition();

    globalPQParam.globalPQSupport = 0;
    globalPQParam.globalPQSwitch = 0;
    globalPQParam.globalPQStrength = 0;
    globalPQParam.globalPQType = 0;
    globalPQParam.globalPQindexInit = 0;
    memset(&(globalPQParam.globalPQindex), 0, sizeof(GLOBAL_PQ_INDEX_T));

#ifdef SUPPORT_HDR
    PQ_TIMER_GET_CURRENT_TIME(m_pPQSessionHandle->HDRHandle->workTime);
    m_pPQSessionHandle->HDRHandle->pWaitingHistList = new PQHDRRegInfoList;
    m_pPQSessionHandle->HDRHandle->pDoneHistList = new PQHDRRegInfoList;
    m_pPQSessionHandle->HDRHandle->pHistListCond = new DpCondition();
#endif

    memset(&m_DpPqConfig, 0, sizeof(m_DpPqConfig));
};

PQSession::~PQSession()
{
    AutoMutex lock(s_ALMutex);

    PQ_LOGI("[PQSession] ~PQSession()... ");

    if (m_pPQSessionHandle)
    {
        PQ_LOGI("[PQSession] --delete PQSession handle, id[%llx]", m_pPQSessionHandle->id);

        _deleteDCHandle();

#ifdef SUPPORT_HDR
        m_pPQSessionHandle->HDRHandle->pHistListCond->signal();

        delete m_pPQSessionHandle->HDRHandle->pWaitingHistList;
        delete m_pPQSessionHandle->HDRHandle->pDoneHistList;
        delete m_pPQSessionHandle->HDRHandle->pHistListCond;
        delete m_pPQSessionHandle->HDRHandle->pHDRFW;
        delete m_pPQSessionHandle->HDRHandle;
#endif
        delete m_pPQSessionHandle;

        delete m_pPQDCConfig;
        delete m_pPQDSConfig;

#ifdef MDP_COLOR_ENABLE
        delete m_pPQColorConfig;
#endif

#ifdef RSZ_2_0
        delete m_pPQRSZConfig;
#endif

#ifdef SUPPORT_HDR
        delete m_pPQHDRConfig;
#endif

#if 0
#ifdef SUPPORT_CCORR
        delete m_pPQCcorrConfig;
#endif
#endif

#ifdef SUPPORT_DRE
        delete m_pPQDREConfig;
#endif
    }
};

void PQSession::deleteDCHandle()
{
    AutoMutex lock(s_ALMutex);
    _deleteDCHandle();

}

void PQSession::_deleteDCHandle()
{
    PQ_LOGI("[PQSession] deleteDCHandle() count[%d]",m_pPQSessionHandle->DCHandle->count);
    m_pPQSessionHandle->DCHandle->count--;
    if (m_pPQSessionHandle->DCHandle->count <= 0)
    {
        m_pPQSessionHandle->DCHandle->pHistListCond->signal();
        delete m_pPQSessionHandle->DCHandle->pHistListCond;
        delete m_pPQSessionHandle->DCHandle->pWaitingHistList;
        delete m_pPQSessionHandle->DCHandle->pDoneHistList;
        delete m_pPQSessionHandle->DCHandle->pADLFW;
        delete m_pPQSessionHandle->DCHandle;
    }
}
uint64_t PQSession::getPQParam(DpPqParam * param)
{
    AutoMutex lock(s_ALMutex);

    memcpy(param, &(m_pPQSessionHandle->PQParam),sizeof(DpPqParam));

    return 0;
}

uint64_t PQSession::getID(void)
{
    AutoMutex lock(s_ALMutex);

    return m_pPQSessionHandle->id;
}

int32_t PQSession::getScenario(void)
{
    AutoMutex lock(s_ALMutex);

    return m_pPQSessionHandle->PQParam.scenario;
}

dcHandle* PQSession::getDCHandle(void)
{
    AutoMutex lock(s_ALMutex);

    return m_pPQSessionHandle->DCHandle;
}

void PQSession::setDCHandle(dcHandle *pDCHandle)
{
    AutoMutex lock(s_ALMutex);

    m_pPQSessionHandle->DCHandle = pDCHandle;
    m_pPQSessionHandle->DCHandle->count++;
}

void PQSession::setSVP(bool value)
{
    AutoMutex lock(s_ALMutex);

    m_pPQSessionHandle->m_svp = value;
}
bool PQSession::getSVP()
{
    AutoMutex lock(s_ALMutex);

    return m_pPQSessionHandle->m_svp;
}

#ifdef SUPPORT_HDR
HDRHandle* PQSession::getHDRHandle(void)
{
    AutoMutex lock(s_ALMutex);

    return m_pPQSessionHandle->HDRHandle;
}
#endif

bool PQSession::getDpPqConfig(DpPqConfig** pDpPqConfig)
{
    AutoMutex lock(s_ALMutex);

    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        PQ_LOGI("[PQSession] getDpPqConfig() Bypassed ...\n");
        *pDpPqConfig = &m_DpPqConfig;
        return false;
    }

    DpPqParam *pParam = &(m_pPQSessionHandle->PQParam);

    *pDpPqConfig = &m_DpPqConfig;

    PQ_LOGI("[PQSession] getDpPqConfig() id[%llx] enable[%d], scenario[%d]\n", m_pPQSessionHandle->id, (int32_t)pParam->enable, (int32_t)pParam->scenario);
    PQ_LOGI("[PQSession] getDpPqConfig() enSharp[%d] enDC[%d] enColor[%d] enHDR[%d] enUR[%d] enReFocus[%d] enCcorr[%d] enDRE[%d] enHFG[%d]\n"
        , m_DpPqConfig.enSharp, m_DpPqConfig.enDC, m_DpPqConfig.enColor, m_DpPqConfig.enHDR
        , m_DpPqConfig.enUR, m_DpPqConfig.enReFocus, m_DpPqConfig.enCcorr, m_DpPqConfig.enDRE, m_DpPqConfig.enHFG);

    return true;
}

void PQSession::initDpPqConfig(DpPqConfig *pConfig)
{
    /* 0: diable, 1: enable, 2: default */
    pConfig->enSharp = 0;
    pConfig->enDC = 0;
    pConfig->enColor = 0;
    pConfig->enHDR = 0;
    pConfig->enUR = 2;
    pConfig->enReFocus = 0;
    pConfig->enCcorr = 0;
    pConfig->enDRE = 0;
    pConfig->enHFG = 0;
}

void PQSession::setDpPqConfig(DpPqConfig *pConfig, const DpPqParam *pParam)
{
    initDpPqConfig(pConfig);

    bool isPQEnabledOnActive;

#ifdef SUPPORT_PQ_WHITE_LIST
    //check if WhiteListServer is ready on N0
    isPQEnabledOnActive = PQWhiteList::getInstance().isPQEnabledOnActive();
#else
    isPQEnabledOnActive = true;
#endif

    if (isPQEnabledOnActive)
    {
        // add MEDIA_VIDEO_CODEC case later
        if ((pParam->enable & PQ_DEFAULT_EN) != 0)
        {
            if (pParam->scenario == MEDIA_PICTURE)
            {
                setPictureDpPqConfig(pConfig, pParam);
            }
            else if (pParam->scenario == MEDIA_VIDEO || pParam->scenario == MEDIA_VIDEO_CODEC)
            {
                setVideoDpPqConfig(pConfig, pParam);
            }
            else if (pParam->scenario == MEDIA_ISP_PREVIEW || pParam->scenario == MEDIA_ISP_CAPTURE)
            {
                setCameraDpPqConfig(pConfig, pParam);
            }
            else
            {
                PQ_LOGE("[PQSession] setDpPqConfig() id[%llx], unknown scenario[%d]\n", m_pPQSessionHandle->id, (int32_t)pParam->scenario);
            }
        }
        else
        {
            pConfig->enSharp = (pParam->enable & PQ_SHP_EN) != 0;
            pConfig->enUR = (pParam->enable & PQ_ULTRARES_EN) != 0;
            pConfig->enReFocus= (pParam->enable & PQ_REFOCUS_EN) != 0;
            pConfig->enDC = (pParam->enable & PQ_DYN_CONTRAST_EN) != 0;
            if (DpDriver::getInstance()->getMdpColor() == true)
                pConfig->enColor = (pParam->enable & PQ_COLOR_EN) != 0;
#ifdef SUPPORT_CCORR
            pConfig->enCcorr = (pParam->enable & PQ_CCORR_EN) != 0;
            if (pConfig->enCcorr == 1 && pParam->srcGamut == pParam->dstGamut)
            {
                /* disable ccorr when srcGamut equal to dstGamut */
                pConfig->enCcorr = 0;
                PQ_LOGI("[PQSession] srcGamut = %d, dstGamut = %d, PQConfig.enCcorr = %d\n",
                    pParam->srcGamut, pParam->dstGamut, pConfig->enCcorr);
            }
#endif
#if defined(SUPPORT_DRE) || defined(SUPPORT_VIDEO_DRE)
            pConfig->enDRE = (pParam->enable & PQ_DRE_EN) != 0;
#endif
#ifdef SUPPORT_HFG
            pConfig->enHFG = (pParam->enable & PQ_HFG_EN) != 0;
#endif
            /* enable sharpness when DC or UR or HFG is enable */
            if (pConfig->enDC | pConfig->enUR | pConfig->enHFG)
                pConfig->enSharp = 1;

#ifdef SUPPORT_HDR
            DP_VDEC_DRV_COLORDESC_T HDRInfo;

            memset(&HDRInfo, 0, sizeof(DP_VDEC_DRV_COLORDESC_T));
            if ((pParam->scenario == MEDIA_VIDEO_CODEC || pParam->scenario == MEDIA_VIDEO) &&
                getHDRInfo(pParam, &HDRInfo))
            {
                /* in this case, enHDR should always be true, we need to tell vcodec new usage of setPQParamter() */
                pConfig->enHDR = 1;
                PQ_LOGI("[PQSession] videoThumbnail, PQConfig.enHDR = %d\n", pConfig->enHDR);
            }
            else
            {
                pConfig->enHDR = 0;
            }
#endif
#ifdef SUPPORT_VIDEO_DRE
            /* enable DRE when HDR is enable for video scenario */
            if (pConfig->enHDR && pParam->scenario == MEDIA_VIDEO)
                pConfig->enDRE = 1;
#endif
        }

        /*set pq off if secure video playback*/
        if (m_pPQSessionHandle->m_svp)
            initDpPqConfig(pConfig);

        PQ_LOGI("[PQSession] setDpPqConfig m_svp = %d, enDRE = %d\n", m_pPQSessionHandle->m_svp, pConfig->enDRE);

    }
}

void PQSession::setPictureDpPqConfig(DpPqConfig *pConfig, const DpPqParam *pParam)
{
    pConfig->enSharp = 1;

    if (pParam->u.image.withHist)
    {
        pConfig->enDC = 1;
        PQ_LOGI("[PQSession] image DC on\n");
    }
    //!! need to revise
#ifdef MDP_COLOR_ENABLE
    if (m_pPQColorConfig->isEnabled(pParam->scenario))
    {
        pConfig->enColor = 1;
    }
#endif
}

void PQSession::setVideoDpPqConfig(DpPqConfig *pConfig, const DpPqParam *pParam)
{
    if (globalPQParam.globalPQSupport != 0)
    {
        pConfig->enSharp = globalPQParam.globalPQSwitch & 0x1;
        pConfig->enDC = (globalPQParam.globalPQSwitch & 0x2) >> 1;
    }
    else
    {
        pConfig->enSharp = 1;
        pConfig->enDC = 1;
    }
#ifdef CONFIG_FOR_SOURCE_PQ
        /* To check if user request this feature? */
        char value[PROPERTY_VALUE_MAX];
        uint32_t prop = 0;
        DISP_PQ_STATUS st;

        memset(&st, 0, sizeof(st));
        property_get(PQ_COLOREX_PROPERTY_STR, value, PQ_COLOREX_INDEX_DEFAULT);
        prop = atoi(value);
        pConfig->enColor = prop ? 1 : 0;
        m_pPQColorConfig->setColorEXEnable(prop);

        if (prop == 1)
        {
            getDisplayStatus(st, true);
        }

        pConfig->enColor =
            (prop==1) &&      // Feature is enabled by end user
            (st.suspend==0) &&    // Not sleeping
            (st.decouple==1) &&   // Decouple mode
            (st.session_num<2);   // Not connected to TV

        PQ_LOGI("[PQSession] en=%d:prop=%d,sus=%d,dc=%d,ce=%d,sn=%d,mutex=%d\n",
            pConfig->enColor, prop, st.suspend, st.decouple,
            st.cascade, st.session_num, st.mutex);
#else
#ifdef MDP_COLOR_ENABLE
    if (m_pPQColorConfig->isEnabled(pParam->scenario))
    {
        pConfig->enColor = 1;
        PQ_LOGI("[PQSession] PQConfig.enColor = %d\n", pConfig->enColor);
    }
#endif

#ifdef SUPPORT_HDR
    DP_VDEC_DRV_COLORDESC_T HDRInfo;

    memset(&HDRInfo, 0, sizeof(DP_VDEC_DRV_COLORDESC_T));
    if (getHDRInfo(pParam, &HDRInfo))
    {
        pConfig->enHDR = 1;
        PQ_LOGI("[PQSession] PQConfig.enHDR = %d\n", pConfig->enHDR);
    }
#endif

#ifdef SUPPORT_SCLTM
#ifdef SUPPORT_VIDEO_DRE
    /* enable DRE for SC-LTM in VIDEO sceanrio */
    if (pParam->scenario == MEDIA_VIDEO)
    {
        pConfig->enDRE = 1;
    }
#endif
#else
    /* enable DRE for HDR in VIDEO sceanrio */
    if (pConfig->enHDR && pParam->scenario == MEDIA_VIDEO)
    {
        pConfig->enDRE = 1;
    }
#endif /* SUPPORT_SCLTM */

#ifdef SUPPORT_CCORR
    /* enable CCORR for SC-LTM */
    if (pConfig->enHDR == 0)
    {
        pConfig->enCcorr = 1;
    }
#endif

#endif /* CONFIG_FOR_SOURCE_PQ */

    PQ_LOGI("[PQSession] video TS [%u]\n", pParam->u.video.timeStamp);
}

void PQSession::setCameraDpPqConfig(DpPqConfig *pConfig, const DpPqParam *pParam)
{
    if (globalPQParam.globalPQSupport != 0)
    {
        pConfig->enSharp = globalPQParam.globalPQSwitch & 0x1;
        pConfig->enDC = (globalPQParam.globalPQSwitch & 0x2) >> 1;
    }
    else
    {
        pConfig->enSharp = 1;
    }
#ifdef MDP_COLOR_ENABLE
    if (m_pPQColorConfig->isEnabled(pParam->scenario))
    {
        pConfig->enColor = 1;
    }
#else
    UNUSED(pParam);
#endif
}

bool PQSession::setPQparam(const DpPqParam *pParam)
{
    AutoMutex lock(s_ALMutex);
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        PQ_LOGI("[PQSession] setPQParam() Bypassed ...\n");
        return false;
    }
    PQ_LOGI("[PQSession] setPQParam() id[%llx] enable[%d], scenario[%d]\n", m_pPQSessionHandle->id, (int32_t)pParam->enable, (int32_t)pParam->scenario);

    setDpPqConfig(&m_DpPqConfig, pParam);

    memcpy(&(m_pPQSessionHandle->PQParam), pParam, sizeof(DpPqParam));

    if (pParam->scenario != MEDIA_VIDEO)
    {
        globalPQParam.globalPQSupport = 0;
    }
    else
    {
        globalPQParam.globalPQSupport = DpDriver::getInstance()->getGlobalPQSupport();

        if (globalPQParam.globalPQSupport != 0 && pParam->scenario == MEDIA_VIDEO)
        {
            sp<IPictureQuality> service = IPictureQuality::getService();
            if (service == nullptr) {
                PQ_LOGD("[setPQparam] failed to get HW service");
            }
            else
            {
                android::hardware::Return<void> ret = service->getGlobalPQSwitch(
                    [&] (Result retval, int32_t switch_value) {
                    if (retval == Result::OK) {
                        globalPQParam.globalPQSwitch = switch_value;
                        m_pPQSessionHandle->PQParam.enable = globalPQParam.globalPQSwitch;
                    }
                });
                if (!ret.isOk()){
                    PQ_LOGE("Transaction error in IPictureQuality::getGlobalPQSwitch");
                }

                ret = service->getGlobalPQStrength(
                    [&] (Result retval, int32_t strength) {
                    if (retval == Result::OK) {
                        globalPQParam.globalPQStrength = strength;
                    }
                });
                if (!ret.isOk()){
                    PQ_LOGE("Transaction error in IPictureQuality::getGlobalPQStrength");
                }
            }

            if (pParam->u.video.id == 0xFF00)
                globalPQParam.globalPQType = GLOBAL_PQ_OTHER;
            else
                globalPQParam.globalPQType = GLOBAL_PQ_VIDEO;
            globalPQParam.globalPQindexInit = 0;
        }
        PQ_LOGI(" setPQParam globalPQSupport = [%d] globalPQSwitch = [%d] globalPQStrength = [%d] globalPQType = [%d]",
            globalPQParam.globalPQSupport, globalPQParam.globalPQSwitch, globalPQParam.globalPQStrength, globalPQParam.globalPQType);
    }

    uint32_t ispScenarioIndex = 0;
    uint32_t lensId = 0;

    if (m_pPQSessionHandle->PQParam.scenario == MEDIA_ISP_PREVIEW || m_pPQSessionHandle->PQParam.scenario == MEDIA_ISP_CAPTURE)
    {
        PQ_LOGI("[PQSession] Timestamp = [%d], FrameNo = [%d], RequestNo = [%d], enableDump = [%d]",
        m_pPQSessionHandle->PQParam.u.isp.timestamp,
        m_pPQSessionHandle->PQParam.u.isp.frameNo,
        m_pPQSessionHandle->PQParam.u.isp.requestNo,
        m_pPQSessionHandle->PQParam.u.isp.enableDump);

        PQ_LOGI("[PQSession] PQParam.u.isp.lensId = [%d]",
        m_pPQSessionHandle->PQParam.u.isp.lensId);

        m_pPQSessionHandle->PQParam.u.isp.isIspScenario = 0;

        if (m_DpPqConfig.enUR == 1)
        {
            m_pPQSessionHandle->PQParam.u.isp.isIspScenario = 1;
            lensId = m_pPQSessionHandle->PQParam.u.isp.lensId;
            if (m_pPQSessionHandle->PQParam.scenario == MEDIA_ISP_CAPTURE && m_pPQSessionHandle->PQParam.u.isp.clearZoomParam.captureShot == CAPTURE_SINGLE)
            {
                ispScenarioIndex = 1;
            }
            else if (m_pPQSessionHandle->PQParam.scenario == MEDIA_ISP_CAPTURE && m_pPQSessionHandle->PQParam.u.isp.clearZoomParam.captureShot == CAPTURE_MULTI)
            {
                ispScenarioIndex = 2;
            }

            PQ_LOGI("[PQSession] PQParam.u.isp.clearZoomParam.captureShot = [%d]",
                m_pPQSessionHandle->PQParam.u.isp.clearZoomParam.captureShot);
        }
        else if (m_DpPqConfig.enReFocus == 1)
        {
            m_pPQSessionHandle->PQParam.u.isp.isIspScenario = 1;
            /* will modify to if (m_DpPqConfig.enRefocus == 1) */
            if (m_pPQSessionHandle->PQParam.u.isp.vsdofParam.isRefocus)
            {
                ispScenarioIndex = 3; // only one setting for VSDoF
            }

            PQ_LOGI("[PQSession] m_pPQSessionHandle->PQParam.u.isp.vsdofParam.isRefocus = [%d], m_pPQSessionHandle->PQParam.u.isp.vsdofParam.defaultUpTable = [%d], m_pPQSessionHandle->PQParam.u.isp.vsdofParam.defaultDownTable = [%d], m_pPQSessionHandle->PQParam.u.isp.vsdofParam.IBSEGain = [%d]",
                m_pPQSessionHandle->PQParam.u.isp.vsdofParam.isRefocus,
                m_pPQSessionHandle->PQParam.u.isp.vsdofParam.defaultUpTable,
                m_pPQSessionHandle->PQParam.u.isp.vsdofParam.defaultDownTable,
                m_pPQSessionHandle->PQParam.u.isp.vsdofParam.IBSEGain);
        }

#ifdef SUPPORT_DRE
        if (m_DpPqConfig.enDRE == 1)
        {
            if (m_pPQSessionHandle->PQParam.u.isp.dpDREParam.buffer == NULL)
            {
                PQ_LOGI("[PQSession] PQ Buffer Mode, m_pPQSessionHandle->PQParam.u.isp.dpDREParam.cmd = [0x%x], m_pPQSessionHandle->PQParam.u.isp.dpDREParam.userId = [0x%llx]",
                    m_pPQSessionHandle->PQParam.u.isp.dpDREParam.cmd,
                    m_pPQSessionHandle->PQParam.u.isp.dpDREParam.userId);
            }
            else
            {
                PQ_LOGI("[PQSession] User Buffer Mode, m_pPQSessionHandle->PQParam.u.isp.dpDREParam.cmd = [0x%x], m_pPQSessionHandle->PQParam.u.isp.dpDREParam.buffer = [0x%p]",
                    m_pPQSessionHandle->PQParam.u.isp.dpDREParam.cmd, m_pPQSessionHandle->PQParam.u.isp.dpDREParam.buffer);
            }
        }
#endif
#ifdef SUPPORT_HFG
        if (m_DpPqConfig.enHFG == 1)
        {
            PQ_LOGI("[PQSession] HFG enable");
        }
#endif
        m_pPQSessionHandle->PQParam.u.isp.ispScenario = (ispScenarioIndex * LENS_TYPE_CNT) + lensId;

        PQ_LOGI("[PQSession] ispScenarioIndex = [%d]", ispScenarioIndex);
        PQ_LOGI("[PQSession] m_pPQSessionHandle->PQParam.u.isp.isIspScenario = [%d], m_pPQSessionHandle->PQParam.u.isp.ispScenario = [%d]", m_pPQSessionHandle->PQParam.u.isp.isIspScenario, m_pPQSessionHandle->PQParam.u.isp.ispScenario);
    }

#ifdef SUPPORT_VIDEO_DRE
    if (m_pPQSessionHandle->PQParam.scenario == MEDIA_VIDEO && m_DpPqConfig.enDRE == 1)
    {
        m_pPQSessionHandle->PQParam.u.video.dpDREParam.SRAMId = DpDREParam::DRESRAM::SRAM00;
        m_pPQSessionHandle->PQParam.u.video.dpDREParam.cmd = DpDREParam::Cmd::Default | DpDREParam::Cmd::Initialize;
        m_pPQSessionHandle->PQParam.u.video.dpDREParam.userId = (((unsigned long long)MEDIA_VIDEO) << 32);
    }
#endif
    return true;
}

bool PQSession::getDCConfig(DC_CONFIG_T* pDCConfig)
{
    AutoMutex lock(s_ALMutex);

    bool status;
    //PQ_LOGD("[PQSession] getDCConfig(), id[%llx] waiting[%d] done[%d]", m_pPQSessionHandle->id, m_pPQSessionHandle->DCHandle->pWaitingHistList->size(), m_pPQSessionHandle->DCHandle->pDoneHistList->size());
    status = m_pPQDCConfig->getDCConfig(pDCConfig, globalPQParam);

    return status;
}

bool PQSession::getDSConfig(DS_CONFIG_T** pDSConfig)
{
    AutoMutex lock(s_ALMutex);

    bool status;
    status = m_pPQDSConfig->getDSConfig(pDSConfig, m_pPQSessionHandle->PQParam.scenario, globalPQParam);

    return status;
}

#ifdef MDP_COLOR_ENABLE
bool PQSession::getColorConfig(COLOR_CONFIG_T ** pColorConfig)
{
    AutoMutex lock(s_ALMutex);

    bool status;
    int32_t scenario = m_pPQSessionHandle->PQParam.scenario;
    status = m_pPQColorConfig->getColorConfig(pColorConfig, scenario);

    return status;
}
#endif

#ifdef RSZ_2_0
bool PQSession::getRSZConfig(RSZ_CONFIG_T * pRSZConfig)
{
    AutoMutex lock(s_ALMutex);

    bool status;
    status = m_pPQRSZConfig->getRSZConfig(pRSZConfig);

    return status;
}
#endif

#ifdef SUPPORT_HDR
bool PQSession::getHDRConfig(HDR_CONFIG_T * pHDRConfig)
{
    AutoMutex lock(s_ALMutex);

    bool status;
    status = m_pPQHDRConfig->getHDRConfig(pHDRConfig);

    return status;
}
#endif

#if 0
#ifdef SUPPORT_CCORR
bool PQSession::getCcorrConfig(CCORR_CONFIG_T **pCcorrConfig)
{
    AutoMutex lock(s_ALMutex);

    bool status;
    int32_t scenario = m_pPQSessionHandle->PQParam.scenario;

    status = m_pPQCcorrConfig->getCcorrConfig(pCcorrConfig, scenario);

    return status;
}
#endif
#endif

#ifdef SUPPORT_DRE
bool PQSession::getDREConfig(DRE_CONFIG_T *pDREConfig)
{
    AutoMutex lock(s_ALMutex);

    bool status;
    int32_t scenario = m_pPQSessionHandle->PQParam.scenario;

    status = m_pPQDREConfig->getDREConfig(pDREConfig);

    return status;
}

void PQSession::setDrePreviousSize(int32_t prevWidth, int32_t prevHeight)
{
    AutoMutex lock(s_ALMutex);

    m_prevWidth = prevWidth;
    m_prevHeight = prevHeight;
}

void PQSession::getDrePreviousSize(int32_t *prevWidth, int32_t *prevHeight)
{
    AutoMutex lock(s_ALMutex);

    *prevWidth = m_prevWidth;
    *prevHeight = m_prevHeight;
}
#endif

#ifdef SUPPORT_HFG
bool PQSession::getHFGConfig(HFG_CONFIG_T *pHFGConfig)
{
    AutoMutex lock(s_ALMutex);

    bool status;

    status = m_pPQHFGConfig->getHFGConfig(pHFGConfig);

    return status;
}
#endif

void PQSession::setHistogram(uint32_t *pHist, uint32_t size)
{
    AutoMutex lock(s_ALMutex);

    if (m_pPQSessionHandle && m_pPQSessionHandle->DCHandle)
    {
        uint64_t id = m_pPQSessionHandle->id;

        PQ_LOGI("[PQSession] setHistogram(), id[%llx] waiting[%d] done[%d]", id, m_pPQSessionHandle->DCHandle->pWaitingHistList->size(), m_pPQSessionHandle->DCHandle->pDoneHistList->size());

        size = TOTAL_HISTOGRAM_NUM;
        if (size <= TOTAL_HISTOGRAM_NUM)
        {
            if (m_pPQSessionHandle->DCHandle->pWaitingHistList->empty())
            {
                PQ_LOGI("[PQSession] setHistogram(), iterator is empty");
                return;
            }

            PQDCHistList::iterator iter = m_pPQSessionHandle->DCHandle->pWaitingHistList->begin();

            memcpy(iter->hist, pHist, size*sizeof(uint32_t));

            #ifdef CONFIG_FOR_SOURCE_PQ
                if (m_pPQColorConfig->getColorEXEnable() == false)
                {
                    iter->hist[18] = 0; //set color histogram info to zero, avoid luma curve to fallback
                }
            #endif

            for (uint32_t i = 0; i < DC_LUMA_HISTOGRAM_NUM; i++)
            {
                PQ_LOGI("[PQSession] setHistogram().. luma hist[%d] = %d", i, iter->hist[i]);
            }
#if DYN_CONTRAST_VERSION == 2
            for (uint32_t i = DC_LUMA_HISTOGRAM_NUM; i < TOTAL_HISTOGRAM_NUM; i++)
            {
                PQ_LOGI("[PQSession] setHistogram().. contour hist[%d] = %d", i - DC_LUMA_HISTOGRAM_NUM, iter->hist[i]);
            }
#endif
            m_pPQSessionHandle->DCHandle->pDoneHistList->push_back(*iter);
            m_pPQSessionHandle->DCHandle->pWaitingHistList->erase(iter);
            m_pPQSessionHandle->DCHandle->isAvailable = true;
            m_pPQSessionHandle->DCHandle->pHistListCond->signal();
        }
        else
        {
            PQ_LOGE("[PQSession] setHistogram().. error, size = %d", size);
        }
    }
    else
    {
        PQ_LOGD("[PQSession] setHistogram().. handle is NULL, bypass this frame");
    }
}
#ifdef SUPPORT_GAME_DRE
bool PQSession::getDSReg(const DpPqParam *pParam, uint32_t *DSInfo)
{
    PQ_LOGI("getDSReg(), scenario = %d, isGAME = %d", pParam->scenario, pParam->u.video.videoScenario);//isGAME = 1 => Game Scenario

    if (pParam->scenario != MEDIA_VIDEO) //enable = true => VR, enable = false => thumbnail
    {
        return false;
    }

    if (pParam->u.video.grallocExtraHandle == NULL)
    {
        PQ_LOGE("getDSReg(), grallocExtraHandle == NULL");
        return false;
    }

    ge_pq_mira_vision_info_t ge_apkreg_info;
    int32_t err = gralloc_extra_query(pParam->u.video.grallocExtraHandle, GRALLOC_EXTRA_GET_PQ_MIRA_VISION_INFO, &ge_apkreg_info);

    if (err != 0 || ge_apkreg_info.inc_count == 0 || ((ge_apkreg_info.value_size*sizeof(int)) < sizeof(DREReg)))
    {
        PQ_LOGE("getDSeg(), gralloc_extra_query failed, err = %d, int_count = %d", err, ge_apkreg_info.inc_count);
        return false;
    }

    DSInfo[0] = ge_apkreg_info.reg_values[GAME_TDSHP_GAIN_MID];
    DSInfo[1] = ge_apkreg_info.reg_values[GAME_TDSHP_GAIN_HIGH];
    DSInfo[2] = ge_apkreg_info.reg_values[GAME_TDSHP_GAIN];
    PQ_LOGI("getDSReg(), GRALLOC_EXTRA_GET_SHP_INFO success");

    PQ_LOGI("tdshp_gain_mid = %d, tdshp_gain_high = %d, tdshp_gain = %d", DSInfo[0], DSInfo[1], DSInfo[2]);

    return true;
}

bool PQSession::getDREReg(const DpPqParam *pParam, DREReg *DREInfo)
{
    PQ_LOGI("getDREReg(), scenario = %d, isGAME = %d", pParam->scenario, pParam->u.video.videoScenario); //isGAME = 1 => Game Scenario

    if (pParam->scenario != MEDIA_VIDEO) //enable = true => VR, enable = false => thumbnail
    {
        return false;
    }


    if (pParam->u.video.grallocExtraHandle == NULL)
    {
        PQ_LOGE("getDREReg(), grallocExtraHandle == NULL");
        return false;
    }

    ge_pq_mira_vision_info_t ge_apkreg_info;
    int32_t err = gralloc_extra_query(pParam->u.video.grallocExtraHandle, GRALLOC_EXTRA_GET_PQ_MIRA_VISION_INFO, &ge_apkreg_info);

    if (err != 0 || ((ge_apkreg_info.value_size)*sizeof(int) < sizeof(DREReg)) || ge_apkreg_info.inc_count == 0)
    {
        PQ_LOGE("getDREReg(), gralloc_extra_query failed, err = %d, int_count = %d", err, ge_apkreg_info.inc_count);
        return false;
    }

    memcpy(DREInfo, ge_apkreg_info.reg_values, sizeof(DREReg));
    PQ_LOGI("getDREReg(), GRALLOC_EXTRA_GET_DRE_INFO success");

    PQ_LOGI("dre_fw_en = %d, dre_curve_en = %d, dre_gain_flt_en = %d, bADLWeight1 = %d, bADLWeight2 = %d", DREInfo->dre_fw_en, DREInfo->dre_curve_en, DREInfo->dre_gain_flt_en, DREInfo->bADLWeight1, DREInfo->bADLWeight2);
    PQ_LOGI("bADLWeight3 = %d, bBSDCGain = %d, bBSACGain = %d, bBSLevel = %d, bMIDDCGain = %d", DREInfo->bADLWeight3, DREInfo->bBSDCGain, DREInfo->bBSACGain, DREInfo->bBSLevel, DREInfo->bMIDDCGain);
    PQ_LOGI("bMIDACGain = %d, bWSDCGain = %d, bWSACGain = %d, bWSLevel = %d, dre_dync_spike_wgt_min = %d", DREInfo->bMIDACGain, DREInfo->bWSDCGain, DREInfo->bWSACGain, DREInfo->bWSLevel, DREInfo->dre_dync_spike_wgt_min);
    PQ_LOGI("dre_dync_spike_wgt_max = %d, dre_dync_spike_th = %d, dre_dync_spike_slope = %d, bSpikeBlendmethod = %d, bSkinWgtSlope = %d", DREInfo->dre_dync_spike_wgt_max, DREInfo->dre_dync_spike_th, DREInfo->dre_dync_spike_slope, DREInfo->bSpikeBlendmethod, DREInfo->bSkinWgtSlope);
    PQ_LOGI("bSkinBlendmethod = %d, SkinDummy1 = %d, Layer0_Ratio = %d, Layer1_Ratio = %d, Layer2_Ratio = %d", DREInfo->bSkinBlendmethod, DREInfo->SkinDummy1, DREInfo->Layer0_Ratio, DREInfo->Layer1_Ratio, DREInfo->Layer2_Ratio);
    PQ_LOGI("Dark_Ratio = %d, Bright_Ratio = %d, dre_dync_flt_coef_min = %d, dre_dync_flt_coef_max = %d, dre_dync_flt_ovr_pxl_th = %d", DREInfo->Dark_Ratio, DREInfo->Bright_Ratio, DREInfo->dre_dync_flt_coef_min, DREInfo->dre_dync_flt_coef_max, DREInfo->dre_dync_flt_ovr_pxl_th);
    PQ_LOGI("dre_dync_flt_ovr_pxl_slope = %d, dre_iir_force_range = %d, LLPValue = %d, LLPRatio = %d, APLCompRatioLow = %d", DREInfo->dre_dync_flt_ovr_pxl_slope, DREInfo->dre_iir_force_range, DREInfo->LLPValue, DREInfo->LLPRatio, DREInfo->APLCompRatioLow);
    PQ_LOGI("APLCompRatioHigh = %d, FltConfSlope = %d, FltConfTh = %d, BlkHistCountRatio = %d, BinIdxDiffSlope = %d", DREInfo->APLCompRatioHigh, DREInfo->FltConfSlope, DREInfo->dre_dync_flt_coef_min, DREInfo->dre_dync_flt_coef_max, DREInfo->dre_dync_flt_ovr_pxl_th);
    PQ_LOGI("BinIdxDiffTh = %d, BinIdxDiffWgtOft = %d, APLTh = %d, APLSlope = %d, APLWgtOft = %d", DREInfo->BinIdxDiffTh, DREInfo->BinIdxDiffWgtOft, DREInfo->APLTh, DREInfo->APLSlope, DREInfo->APLWgtOft);
    PQ_LOGI("APL2Th = %d, APL2Slope = %d, APL2WgtOft = %d, APL2WgtMax = %d, BlkSpaFltEn = %d", DREInfo->APL2Th, DREInfo->APL2Slope, DREInfo->APL2WgtOft, DREInfo->APL2WgtMax, DREInfo->BlkSpaFltEn);
    PQ_LOGI("BlkSpaFltType = %d, LoadBlkCurveEn = %d, SaveBlkCurveEn = %d, Flat_Length_Th_Base = %d, Flat_Length_Slope_Base = %d", DREInfo->BlkSpaFltType, DREInfo->LoadBlkCurveEn, DREInfo->SaveBlkCurveEn, DREInfo->Flat_Length_Th_Base, DREInfo->Flat_Length_Slope_Base);
    PQ_LOGI("DZ_Fallback_En = %d, DZ_Size_Th = %d, DZ_Size_Slope = %d", DREInfo->DZ_Fallback_En, DREInfo->DZ_Size_Th, DREInfo->DZ_Size_Slope);

#ifdef CALTM_VERSION_6779
    PQ_LOGI("bGreenWgtSlope = %d, dre_dync_compact_wgt_min = %d, dre_dync_compact_wgt_max = %d, dre_dync_compact_th = %d, dre_dync_compact_slope = %d", DREInfo->bGreenWgtSlope, DREInfo->dre_dync_compact_wgt_min, DREInfo->dre_dync_compact_wgt_max, DREInfo->dre_dync_compact_th, DREInfo->dre_dync_compact_slope);
    PQ_LOGI("bCompactBlendmethod = %d, dre_dync_compact_LowBoundBin = %d, dre_dync_compact_HighBoundBin = %d, dre_dync_compact_BinMinWeight = %d, dre_dync_compact_BinMaxWeight = %d", DREInfo->bCompactBlendmethod, DREInfo->dre_dync_compact_LowBoundBin, DREInfo->dre_dync_compact_HighBoundBin, DREInfo->dre_dync_compact_BinMinWeight, DREInfo->dre_dync_compact_BinMaxWeight);
#endif

    return true;
}
#endif

#ifdef SUPPORT_HDR
void PQSession::setHDRRegInfo(uint32_t *pHDRRegInfo, uint32_t size)
{
    AutoMutex lock(s_ALMutex);

    if (m_pPQSessionHandle && m_pPQSessionHandle->HDRHandle)
    {
        uint64_t id = m_pPQSessionHandle->id;

        PQ_LOGI("[PQSession] setHDRRegInfo(), id[%llx] waiting[%lu] done[%lu]", id, m_pPQSessionHandle->HDRHandle->pWaitingHistList->size(), m_pPQSessionHandle->HDRHandle->pDoneHistList->size());

        if (size <= HDR_REGINFO_SIZE)
        {

            if (m_pPQSessionHandle->HDRHandle->pWaitingHistList->empty())
            {
                PQ_LOGI("[PQSession] setHDRRegInfo(), iterator is empty");
                return;
            }

            PQHDRRegInfoList::iterator iter = m_pPQSessionHandle->HDRHandle->pWaitingHistList->begin();

            memcpy(iter->hist, pHDRRegInfo, HDR_TOTAL_HISTOGRAM_NUM * sizeof(uint32_t)); //copy hist & letterBoxPos
            iter->LetterBoxPos = pHDRRegInfo[HDR_REGINFO_SIZE - 1];
            //PQ_LOGD("[PQSession] setHDRRegInfo()3, id[%llx] waiting[%d] done[%d]", id, m_pPQSessionHandle->DCHandle->pWaitingHistList->size(), m_pPQSessionHandle->DCHandle->pDoneHistList->size());
            for (uint32_t i = 0; i < HDR_TOTAL_HISTOGRAM_NUM; i++)
            {
                PQ_LOGI("[PQSession] setHDRRegInfo().. hist[%d] = %d", i, iter->hist[i]);
            }

            PQ_LOGI("[PQSession] setHDRRegInfo(), LetterBoxPos = [0x%x]", iter->LetterBoxPos);

            //PQ_LOGD("[PQSession] setHDRRegInfo()4, id[%llx] waiting[%d] done[%d]", id, m_pPQSessionHandle->DCHandle->pWaitingHistList->size(), m_pPQSessionHandle->DCHandle->pDoneHistList->size());

            //move hist node from waiting to done
            m_pPQSessionHandle->HDRHandle->pDoneHistList->push_back(*iter);
            m_pPQSessionHandle->HDRHandle->pWaitingHistList->erase(iter);
            m_pPQSessionHandle->HDRHandle->isAvailable = true;
            m_pPQSessionHandle->HDRHandle->pHistListCond->signal();
        }
        else
        {
            PQ_LOGE("[PQSession] setHDRRegInfo().. error, size = %d", size);
        }
    }
    else
    {
        PQ_LOGD("[PQSession] setHDRRegInfo().. handle is NULL, bypass this frame");
    }
}

bool PQSession::getHDRInfo(const DpPqParam *pParam, DP_VDEC_DRV_COLORDESC_T *HDRInfo)
{
    PQ_LOGI("getHDRInfo(), scenario = %d, isHDR2SDR = %d", pParam->scenario, pParam->u.video.isHDR2SDR);

    if (pParam->scenario == MEDIA_VIDEO_CODEC) //enable = true => VR, enable = false => thumbnail
    {
        PQ_LOGI("getHDRInfo(), MEDIA_VIDEO_CODEC");
        memcpy(HDRInfo, &(pParam->u.video.HDRinfo), sizeof(DP_VDEC_DRV_COLORDESC_T));
    }
    else if (pParam->scenario == MEDIA_VIDEO)
    {
        if (pParam->u.video.grallocExtraHandle == NULL)
        {
            PQ_LOGE("getHDRInfo(), grallocExtraHandle == NULL");
            return false;
        }

        ge_hdr_info_t ge_hdr_info;
        int32_t err = gralloc_extra_query(pParam->u.video.grallocExtraHandle, GRALLOC_EXTRA_GET_HDR_INFO, &ge_hdr_info);

        if (err != 0)
        {
            PQ_LOGE("getHDRInfo(), gralloc_extra_query failed, err = %d", err);
            return false;
        }
        else
        {
            memcpy(HDRInfo, &ge_hdr_info, sizeof(DP_VDEC_DRV_COLORDESC_T));
            PQ_LOGI("getHDRInfo(), GRALLOC_EXTRA_GET_HDR_INFO success");
        }
    }
    else
    {
        PQ_LOGI("getHDRInfo(), unknown scenario [%d]", pParam->scenario);
        return false;
    }

    PQ_LOGI("[PQSession] u4ColorPrimaries = %d", HDRInfo->u4ColorPrimaries);
    PQ_LOGI("[PQSession] u4TransformCharacter = %d", HDRInfo->u4TransformCharacter);
    PQ_LOGI("[PQSession] u4MatrixCoeffs = %d", HDRInfo->u4MatrixCoeffs);

    for (int index = 0; index < 3; index++)
    {
        PQ_LOGI("[PQSession] u4DisplayPrimariesX[%d] = %d", index, HDRInfo->u4DisplayPrimariesX[index]);
        PQ_LOGI("[PQSession] u4DisplayPrimariesY[%d] = %d", index, HDRInfo->u4DisplayPrimariesY[index]);
    }

    PQ_LOGI("[PQSession] u4WhitePointX = %d", HDRInfo->u4WhitePointX);
    PQ_LOGI("[PQSession] u4WhitePointY = %d", HDRInfo->u4WhitePointY);
    PQ_LOGI("[PQSession] u4MaxDisplayMasteringLuminance = %d", HDRInfo->u4MaxDisplayMasteringLuminance);
    PQ_LOGI("[PQSession] u4MinDisplayMasteringLuminance = %d", HDRInfo->u4MinDisplayMasteringLuminance);
    PQ_LOGI("[PQSession] u4MaxContentLightLevel = %d", HDRInfo->u4MaxContentLightLevel);
    PQ_LOGI("[PQSession] u4MaxPicAverageLightLevel = %d", HDRInfo->u4MaxPicAverageLightLevel);

    if (HDRInfo->u4TransformCharacter == TRANSFER_CHARACTOR_SMPTEST2084 ||
        HDRInfo->u4TransformCharacter == TRANSFER_CHARACTOR_ITURBT2020_10 ||
        HDRInfo->u4TransformCharacter == TRANSFER_CHARACTOR_ARIBSTD_B67)  //valid struct for HDR
    {
        return true;
    }
    else
    {
        return false;
    }
}
#endif

