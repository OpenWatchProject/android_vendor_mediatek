#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1

#include "PQSessionManager.h"
#include <cutils/properties.h>
#include <cutils/log.h>
#include "PQCommon.h"
#include "DpDriver.h"

PQSessionManager* PQSessionManager::s_pInstance = NULL;
PQMutex   PQSessionManager::s_ALMutex;
uint32_t  PQSessionManager::m_VideoIDBufferRing[VIDEO_ID_NUM];
uint32_t  PQSessionManager::m_lastVideoIndex;
uint32_t  PQSessionManager::m_maxVideoIndex;

PQSessionManager* PQSessionManager::getInstance()
{
    AutoMutex lock(s_ALMutex);

    if(NULL == s_pInstance)
    {
        s_pInstance = new PQSessionManager();
        atexit(PQSessionManager::destroyInstance);
    }

    return s_pInstance;
}

void PQSessionManager::destroyInstance()
{
    AutoMutex lock(s_ALMutex);

    if (NULL != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = NULL;
    }
}

PQSessionManager::PQSessionManager()
    :m_pPQSessionListHandle(NULL),
    m_PQSupport(0)
{
    PQ_LOGD("[PQSessionManager] PQSessionManager()... ");

    m_PQSupport = DpDriver::getInstance()->getPQSupport();
    PQ_LOGD("[PQSessionManager] m_PQSupport = %d ", m_PQSupport);
};

PQSessionManager::~PQSessionManager()
{
    PQ_LOGD("[PQSessionManager] ~PQSessionManager()... ");

    PQSessionListHandle* pPQSessionListHandle = m_pPQSessionListHandle;
    while (pPQSessionListHandle)
    {
        PQSessionListHandle* next_p;
        next_p = pPQSessionListHandle->next_p;

        PQ_LOGD("[PQSessionManager] --delete PQSessionListHandle, id[%llx]", pPQSessionListHandle->pPQSessionInstance->getID());
        delete pPQSessionListHandle->pPQSessionInstance;
        delete pPQSessionListHandle;

        pPQSessionListHandle = next_p;
    }

};

PQSession* PQSessionManager::createPQSession(uint64_t id)
{
    AutoMutex lock(s_ALMutex);

    PQSessionListHandle* pPQSessionListHandle;
    PQ_LOGI("[PQSessionManager] createPQSession(%llx)", id);

    if (isIdDuplicated(id))
    {
        pPQSessionListHandle = getPQSessionListHandle(id);

        return pPQSessionListHandle->pPQSessionInstance;
    }

    pPQSessionListHandle = createPQSessionListHandle(id);

    return  pPQSessionListHandle->pPQSessionInstance;
}

void PQSessionManager::destroyPQSession(uint32_t id)
{
    AutoMutex lock(s_ALMutex);

    //!!! need to revise, when destroying PQSession, use PQID to determine PQSession
    // if scenaro is video , and  DSStatus->workTime is too long
    //     call this function to delete PQSession?

    PQ_LOGI("[PQSessionManager] destroyPQSession, PQID = (%x)", id);
    PQSessionListHandle* pPQSessionListHandle = m_pPQSessionListHandle;

    while (pPQSessionListHandle != NULL)
    {
        PQSessionListHandle* pPQSessionListHandle_next = pPQSessionListHandle->next_p;
        uint64_t PQSessionID = pPQSessionListHandle->pPQSessionInstance->getID();
        if ((PQSessionID >> 32) == id)
        {
            PQ_LOGI("[PQSessionManager] getPQSessionListHandle, find id[%llx] ", PQSessionID);
            deletePQSession(pPQSessionListHandle);
        }

        pPQSessionListHandle = pPQSessionListHandle_next;
    }
}

void PQSessionManager::deletePQSession(PQSessionListHandle* pPQSessionListHandle)
{
    if (pPQSessionListHandle == NULL )
    {
        PQ_LOGE("[PQSessionManager] destroyPQSession(), PQSessionListHandle is NULL");
        return;
    }
    else if (pPQSessionListHandle->pPQSessionInstance == NULL)
    {
        PQ_LOGE("[PQSessionManager] destroyPQSession(), PQSessionInstance is NULL");
        return;
    }
    else
    {
        if (pPQSessionListHandle->next_p)
        {
            pPQSessionListHandle->next_p->prev_p = pPQSessionListHandle->prev_p;
        }

        if (pPQSessionListHandle->prev_p)
        {
            pPQSessionListHandle->prev_p->next_p = pPQSessionListHandle->next_p;
        }

        if (pPQSessionListHandle == m_pPQSessionListHandle)
        {
            m_pPQSessionListHandle = pPQSessionListHandle->next_p;
        }

        delete pPQSessionListHandle->pPQSessionInstance;
        delete pPQSessionListHandle;
    }
}

PQSession* PQSessionManager::getPQSession(uint64_t id)
{
    AutoMutex lock(s_ALMutex);

    if (m_PQSupport == 0) {
        return NULL;
    }

    PQ_LOGI("[PQSessionManager] PQSessionManager::getPQSession id[%llx] ", id);

    PQSessionListHandle* pPQSessionListHandle = getPQSessionListHandle(id);

    if (pPQSessionListHandle == NULL)
    {
        return NULL;
    }
    else
    {
        return pPQSessionListHandle->pPQSessionInstance;
    }
}

PQSessionListHandle* PQSessionManager::createPQSessionListHandle(uint64_t id)
{
    PQSessionListHandle* pPQSessionListHandle_new = new PQSessionListHandle;

    PQ_LOGI("[PQSessionManager] create PQSessionListHandle, id[%llx]", id);

    memset(pPQSessionListHandle_new, 0, sizeof(PQSessionListHandle));

    pPQSessionListHandle_new->pPQSessionInstance = new PQSession(id);

    // 1st in queue
    if (m_pPQSessionListHandle == NULL)
    {
        m_pPQSessionListHandle = pPQSessionListHandle_new;
    }
    else
    {
        PQSessionListHandle* pPQSessionListHandle = m_pPQSessionListHandle;

        while(1)
        {
            //check if video id is same, return previous DC handle
            uint64_t videoid = id << 32;
            uint64_t videonum = ((uint64_t)VIDEO_ID_NUM) << 32;
            if ((pPQSessionListHandle->pPQSessionInstance->getScenario() ==  MEDIA_VIDEO) &&
                (videoid > 0) && (videoid <= videonum) && (videoid == (pPQSessionListHandle->pPQSessionInstance->getID() << 32)))
            {
                if (pPQSessionListHandle->pPQSessionInstance->getDCHandle() != NULL)
                {
                    PQ_LOGI("[PQSessionManager] create PQSessionListHandle, same video id[%llx]", pPQSessionListHandle->pPQSessionInstance->getID());
                    pPQSessionListHandle_new->pPQSessionInstance->deleteDCHandle();
                    pPQSessionListHandle_new->pPQSessionInstance->setDCHandle(pPQSessionListHandle->pPQSessionInstance->getDCHandle());
                }
            }

            if (pPQSessionListHandle->next_p != NULL)
            {
                pPQSessionListHandle = pPQSessionListHandle->next_p;
            }
            else
            {
                break;
            }

        }

        pPQSessionListHandle->next_p = pPQSessionListHandle_new;
        pPQSessionListHandle_new->prev_p = pPQSessionListHandle;
    }

    return pPQSessionListHandle_new;
}

PQSessionListHandle* PQSessionManager::getPQSessionListHandle(uint64_t id)
{
    PQSessionListHandle* pPQSessionListHandle = m_pPQSessionListHandle;

    PQ_LOGI("[PQSessionManager] getPQSessionListHandle id[%llx] ", id);

    if ((pPQSessionListHandle != NULL) &&
        (pPQSessionListHandle->pPQSessionInstance->getID() == id))
    {
        return pPQSessionListHandle;
    }

    // try to find handle
    pPQSessionListHandle = m_pPQSessionListHandle;
    while (pPQSessionListHandle != NULL)
    {
        if (pPQSessionListHandle->pPQSessionInstance->getID() == id)
        {
            PQ_LOGI("[PQSessionManager] getPQSessionListHandle, find id[%llx] ", id);
            return pPQSessionListHandle;
        }

        pPQSessionListHandle = pPQSessionListHandle->next_p;
    }

    if (pPQSessionListHandle == NULL)
    {
        PQ_LOGI("[PQSessionManager] pPQSessionListHandle = NULL");

        if (0 == id)
        {
            // create handle for old api
            pPQSessionListHandle = createPQSessionListHandle(id);
        }
        else
        {
            PQ_LOGI("[PQSessionManager] getPQSession id[%llx] fail!! check createPQSession!", id);
            // can't find this id, create another one.
            //pPQSessionListHandle = createPQSessionListHandle(id);

        }
    }
    return pPQSessionListHandle;
}

bool PQSessionManager::isIdDuplicated(uint64_t id)
{
    PQSessionListHandle* pPQSessionListHandle = m_pPQSessionListHandle;

    while (pPQSessionListHandle)
    {
        PQSessionListHandle* next_p;
        next_p = pPQSessionListHandle->next_p;

        if (pPQSessionListHandle->pPQSessionInstance->getID() == id)
        {
            PQ_LOGI("[PQSessionManager] Duplicated ID[%llx] found, please check if double createPQSession!", pPQSessionListHandle->pPQSessionInstance->getID());
            return true;
        }

        pPQSessionListHandle = next_p;
    }

    return false;
}

uint32_t PQSessionManager::findVideoID(uint32_t videoID)
{
    if (videoID == 0)
    {
        return 0;
    }
    else if(m_VideoIDBufferRing[0] == 0)
    {
        m_lastVideoIndex = 0;
        m_VideoIDBufferRing[m_lastVideoIndex] = videoID;
        PQ_LOGI("[PQSessionManager] findVideoID[%llx] buffer is empty. Return new ID %llx", videoID, m_lastVideoIndex + 1);
        return m_lastVideoIndex + 1;
    }

    for (uint32_t i = 0; i < VIDEO_ID_NUM; i++)
    {
        uint32_t VideoIndex = (m_lastVideoIndex + i);
        VideoIndex = (VideoIndex >= VIDEO_ID_NUM) ? (VideoIndex - VIDEO_ID_NUM) : VideoIndex;

        if (videoID == m_VideoIDBufferRing[VideoIndex])
        {
            m_lastVideoIndex = VideoIndex;
            PQ_LOGI("[PQSessionManager] findVideoID[%llx] found on index %llx", videoID, VideoIndex + 1);
            if (VideoIndex > m_maxVideoIndex)
            {
                m_maxVideoIndex = VideoIndex;
            }
            return VideoIndex + 1;
        }
    }
    // videoID is new. Assign next video index for it.
    m_lastVideoIndex = ((m_maxVideoIndex + 1) < VIDEO_ID_NUM) ? (m_maxVideoIndex + 1) : 0;
    m_maxVideoIndex = m_lastVideoIndex;

    m_VideoIDBufferRing[m_lastVideoIndex] = videoID;
    PQ_LOGI("[PQSessionManager] findVideoID[%llx] not found. Return new ID %llx", videoID, m_lastVideoIndex + 1);
    return m_lastVideoIndex + 1;
}

