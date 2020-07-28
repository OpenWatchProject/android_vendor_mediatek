#ifndef __PQSESSIONMANAGER_H__
#define __PQSESSIONMANAGER_H__

#include "PQMutex.h"
#include "PQSession.h"

#define PQDC_NEW_CLIP_TIME_INTERVAL  (1000)
#define HDR_NEW_CLIP_TIME_INTERVAL  (1000)
#define PQ_REFER_STEP (1)

#define VIDEO_ID_NUM (10)
#define ISP_VIDEO_ID (0)
#define FRAG_VIDEO_ID (0)

struct PQSessionListHandle;
typedef struct PQSessionListHandle {
    PQSessionListHandle*    prev_p;
    PQSessionListHandle*    next_p;
    PQSession*  pPQSessionInstance;
}PQSessionListHandle;

class PQSessionManager
{
public:
    PQSessionManager();
    ~PQSessionManager();

    static PQSessionManager* getInstance();
    static void destroyInstance();
    PQSession*  createPQSession(uint64_t id);
    PQSession*  getPQSession(uint64_t id);
    void        destroyPQSession(uint32_t id);
    uint32_t    findVideoID(uint32_t VideoID);

private:
    bool isIdDuplicated(uint64_t id);
    PQSessionListHandle* createPQSessionListHandle(uint64_t id);
    PQSessionListHandle* getPQSessionListHandle(uint64_t id);
    void deletePQSession(PQSessionListHandle* pPQSessionListHandle);

private:
    PQSessionListHandle*        m_pPQSessionListHandle;
    static PQSessionManager*    s_pInstance;
    static PQMutex              s_ALMutex;
    static uint32_t             m_VideoIDBufferRing[VIDEO_ID_NUM];
    static uint32_t             m_lastVideoIndex;
    static uint32_t             m_maxVideoIndex;
    uint8_t                     m_PQSupport;
};
#endif // __PQSESSIONMANAGER_H__