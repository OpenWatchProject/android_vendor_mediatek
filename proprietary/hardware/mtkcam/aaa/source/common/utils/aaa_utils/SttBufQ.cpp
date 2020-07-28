#define LOG_TAG "Hal3A/SttBufQ"

#include <SttBufQ.h>
#include <utils/Mutex.h>
#include <string.h>
#include <isp_tuning/isp_tuning.h>
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>
#include <array>

// makefile can change this setting for different platform
#ifndef STATISTIC_BUF_QUEUE_SIZE_MIN
#define STATISTIC_BUF_QUEUE_SIZE_MIN (9)
#endif

#define SBQ_LOG1(fmt, arg...) do { if(LogLevel>=1) {CAM_LOGD("[%s:%d]" fmt, __FUNCTION__, __LINE__, ##arg );}} while(0)
#define SBQ_LOG2(fmt, arg...) do { if(LogLevel>=2) {CAM_LOGD("[%s:%d]" fmt, __FUNCTION__, __LINE__, ##arg );}} while(0)

//////////////////////////////////////////////////////////////////////
// following code is copied from linux kernel list.h
#define container_of(ptr, type, member) ({                  \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type,member) );})

using namespace android;
using namespace NSIspTuning;
using namespace NS3Av3;


namespace NSCam {

struct list_head {
    struct list_head *next, *prev;
};
static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline int list_empty(const struct list_head *head)
{
    return head->next == head;
}

static inline void list_del(struct list_head *entry)
{
    struct list_head *next = entry->next;
    struct list_head *prev = entry->prev;
    next->prev = prev;
    prev->next = next;
    entry->next = NULL;
    entry->prev = NULL;
}

static inline void list_add(struct list_head *new_, struct list_head *head)
{
    struct list_head *prev = head;
    struct list_head *next = head->next;
    next->prev = new_;
    new_->next = next;
    new_->prev = prev;
    prev->next = new_;
}

static inline void list_add_tail(struct list_head *new_, struct list_head *head)
{
    struct list_head *prev = head->prev;
    struct list_head *next = head;
    next->prev = new_;
    new_->next = next;
    new_->prev = prev;
    prev->next = new_;
}

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

// above code is copied from linux kernel list.h
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Declaration
class SttBufQ : public ISttBufQ
{
public:
    struct DATANODE
    {
        struct list_head node;
        sp<DATA> pData;
    };

public:
    SttBufQ(const char *m_OwnerName);
    virtual ~SttBufQ();

    virtual sp<DATA> deque();
    virtual sp<DATA> deque_last();
    virtual MBOOL enque_first(sp<DATA> pData);
    virtual MBOOL enque(sp<DATA> pData);
    virtual sp<DATA> deque_byMagicNumberRequest(MINT32 MagicNumberRequest);
    virtual sp<DATA> deque_byMagicNumberStt(MINT32 MagicNumberStt);

public:
    Mutex m_lock;
    char m_OwnerName[32];
    list_head m_Queuing;
    list_head m_Dequeued;
    int m_QueueSize;
    DATANODE *DataNode;
};

//////////////////////////////////////////////////////////////////////
// Global Variable
struct MY_INST{
    sp<SttBufQ> instance;
    Mutex lock;
};

static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;
static int LogLevel = 0;

//////////////////////////////////////////////////////////////////////
// Implementation
SttBufQ::SttBufQ(const char *OwnerName)
{
    SBQ_LOG1("m_OwnerName=%s", OwnerName);

    int i;
    INIT_LIST_HEAD(&m_Dequeued);
    INIT_LIST_HEAD(&m_Queuing);

    m_QueueSize = property_get_int32("vendor.debug.camera.SttBufQ.enable", 0);
    if (m_QueueSize < STATISTIC_BUF_QUEUE_SIZE_MIN) m_QueueSize = STATISTIC_BUF_QUEUE_SIZE_MIN;

    DataNode = new DATANODE[m_QueueSize];
    for(i=0;i<m_QueueSize;i++) {
        DataNode[i].pData = new DATA;
        list_add_tail(&DataNode[i].node, &m_Queuing);
    }

    strncpy(m_OwnerName, OwnerName, sizeof(m_OwnerName));
}

SttBufQ::~SttBufQ()
{
    SBQ_LOG1("m_OwnerName=%s", m_OwnerName);

    int i;
    for(i=0;i<m_QueueSize;i++) {
        DataNode[i].pData = NULL;
    }
    delete[] DataNode;
}

MVOID ISttBufQ::createSingleton(MINT32 SensorDev, const char *m_OwnerName)
{
    LogLevel = property_get_int32("vendor.debug.camera.SttBufQ.log", 0);

    CAM_LOGD("[%s:%d] m_OwnerName=%s, dev=%d", __FUNCTION__, __LINE__, m_OwnerName, SensorDev);

    auto i4SensorIdx = mapSensorDevToIdx(SensorDev);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return;
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    Mutex::Autolock _l(rSingleton.lock);
    if(rSingleton.instance == nullptr) {
        rSingleton.instance = new SttBufQ(m_OwnerName);
        CAM_LOGD("[%s:%d] make new instance", __FUNCTION__, __LINE__);
    }
}

MVOID ISttBufQ::destroySingleton(MINT32 SensorDev, const char *m_OwnerName)
{
    CAM_LOGD("[%s:%d] m_OwnerName=%s", __FUNCTION__, __LINE__, m_OwnerName);

    auto i4SensorIdx = mapSensorDevToIdx(SensorDev);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return;
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    Mutex::Autolock _l(rSingleton.lock);
    if(rSingleton.instance.get())
    {
        rSingleton.instance = nullptr;
        CAM_LOGD("[%s:%d] destroy instance", __FUNCTION__, __LINE__);
    }
}

sp<ISttBufQ> ISttBufQ::getInstance(MINT32 SensorDev)
{
    SBQ_LOG2("dev=%d", SensorDev);

    auto i4SensorIdx = mapSensorDevToIdx(SensorDev);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return nullptr;
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    Mutex::Autolock _l(rSingleton.lock);
    if(rSingleton.instance.get()) {
        SBQ_LOG2("m_OwnerName=%s, dev=%d", rSingleton.instance->m_OwnerName, SensorDev);
        return rSingleton.instance;
    } else {
        SBQ_LOG2("SttBufQ not found, dev=%d", SensorDev);
        return nullptr;
    }
}

sp<ISttBufQ::DATA> SttBufQ::deque()
{
    SBQ_LOG2("m_OwnerName=%s", m_OwnerName);

    Mutex::Autolock _l(m_lock);
    if (list_empty(&m_Queuing)) {
        CAM_LOGE("[%s:%d] deque failed", __FUNCTION__, __LINE__);
        return NULL;
    } else {
        struct list_head *pNode = m_Queuing.next;
        DATANODE *pDataNode;
        sp<DATA> pData;

        list_del(pNode);

        pDataNode = container_of(pNode, DATANODE, node);
        pData = pDataNode->pData;
        pDataNode->pData = NULL;

        list_add(pNode, &m_Dequeued);

        SBQ_LOG2("stt=%d, request=%d", pData->MagicNumberStt, pData->MagicNumberRequest);
        return pData;
    }
}

sp<ISttBufQ::DATA> SttBufQ::deque_last()
{
    SBQ_LOG2("m_OwnerName=%s", m_OwnerName);

    Mutex::Autolock _l(m_lock);
    if (list_empty(&m_Queuing)) {
        CAM_LOGE("[%s:%d] deque failed", __FUNCTION__, __LINE__);
        return NULL;
    } else {
        struct list_head *pNode = m_Queuing.prev;
        DATANODE *pDataNode;
        sp<DATA> pData;

        list_del(pNode);

        pDataNode = container_of(pNode, DATANODE, node);
        pData = pDataNode->pData;
        pDataNode->pData = NULL;

        list_add(pNode, &m_Dequeued);

        SBQ_LOG2("stt=%d, request=%d", pData->MagicNumberStt, pData->MagicNumberRequest);
        return pData;
    }
}


MBOOL SttBufQ::enque_first(sp<ISttBufQ::DATA> pData)
{
    SBQ_LOG2("m_OwnerName=%s, stt=%d, request=%d", m_OwnerName, pData->MagicNumberStt, pData->MagicNumberRequest);

    Mutex::Autolock _l(m_lock);
    if (list_empty(&m_Dequeued)) {
        CAM_LOGE("[%s:%d] enque failed", __FUNCTION__, __LINE__);
        return MFALSE;
    } else {
        struct list_head *pNode = m_Dequeued.next;
        DATANODE *pDataNode;

        list_del(pNode);

        pDataNode = container_of(pNode, DATANODE, node);
        pDataNode->pData = pData;

        list_add(pNode, &m_Queuing);
        return MTRUE;
    }
}

MBOOL SttBufQ::enque(sp<ISttBufQ::DATA> pData)
{
    SBQ_LOG2("m_OwnerName=%s, stt=%d, request=%d", m_OwnerName, pData->MagicNumberStt, pData->MagicNumberRequest);

    Mutex::Autolock _l(m_lock);
    if (list_empty(&m_Dequeued)) {
        CAM_LOGE("[%s:%d] enque failed", __FUNCTION__, __LINE__);
        return MFALSE;
    } else {
        struct list_head *pNode = m_Dequeued.next;
        DATANODE *pDataNode;

        list_del(pNode);

        pDataNode = container_of(pNode, DATANODE, node);
        pDataNode->pData = pData;

        list_add_tail(pNode, &m_Queuing);
        return MTRUE;
    }
}

sp<ISttBufQ::DATA> SttBufQ::deque_byMagicNumberRequest(MINT32 MagicNumberRequest)
{
    SBQ_LOG2("m_OwnerName=%s, request=%d", m_OwnerName, MagicNumberRequest);

    struct list_head *pos;
    DATANODE *pDataNode;
    DATANODE *pLastestDataNode;
    sp<DATA> pData;
    struct list_head *pNode;

    Mutex::Autolock _l(m_lock);

    list_for_each(pos, &m_Queuing)
    {
        pDataNode = container_of(pos, DATANODE, node);
        if (pDataNode->pData->MagicNumberRequest == MagicNumberRequest)
        {
            pNode = &pDataNode->node;

            list_del(pNode);

            pData = pDataNode->pData;
            pDataNode->pData = NULL;

            list_add(pNode, &m_Dequeued);

            SBQ_LOG1("stt=%d, request=%d", pData->MagicNumberStt, pData->MagicNumberRequest);
            break;
        }
    }

    // if not found, show all magic# in queue
    if (!pData.get()) {
        char str[64];
        int len = sizeof(str), t;
        char *ptr=str;
        SBQ_LOG1("request=%d not found", MagicNumberRequest);
        list_for_each(pos, &m_Queuing)
        {
            pLastestDataNode = container_of(pos, DATANODE, node);
            t = snprintf(ptr, len, " %d/%d,", pLastestDataNode->pData->MagicNumberStt, pLastestDataNode->pData->MagicNumberRequest);
            len -= t;
            ptr += t;
            if (len<16) {
                SBQ_LOG2("%s", str);
                len = sizeof(str);
                ptr = str;
            }
        }
    }

    pLastestDataNode = NULL;
    list_for_each_prev(pos, &m_Queuing)
    {
        pLastestDataNode = container_of(pos, DATANODE, node);
        if (pLastestDataNode->pData->MagicNumberRequest != 0) break;
    }

    if(pLastestDataNode) {
        int N = pLastestDataNode->pData->MagicNumberRequest - MagicNumberRequest;
        if (N > m_QueueSize) {
            CAM_LOGE("[%s:%d]m_OwnerName=%s: SttBufQ size not enough!! require/size=%d/%d",
                __FUNCTION__, __LINE__, m_OwnerName, N, m_QueueSize);
        } else {
            SBQ_LOG1("m_OwnerName=%s, SttBufQ require/size=%d/%d", m_OwnerName, N, m_QueueSize);
        }
    } else {
        CAM_LOGE("[%s:%d]m_OwnerName=%s: all buffers are dequeued", __FUNCTION__, __LINE__, m_OwnerName);
    }

    return pData;
}

sp<ISttBufQ::DATA> SttBufQ::deque_byMagicNumberStt(MINT32 MagicNumberStt)
{
    SBQ_LOG2("m_OwnerName=%s, stt=%d", m_OwnerName, MagicNumberStt);

    struct list_head *pos;
    DATANODE *pDataNode;
    sp<DATA> pData;
    struct list_head *pNode;

    Mutex::Autolock _l(m_lock);

    list_for_each_prev(pos, &m_Queuing)
    {
        pDataNode = container_of(pos, DATANODE, node);
        if (pDataNode->pData->MagicNumberStt == MagicNumberStt)
        {
            pNode = &pDataNode->node;

            list_del(pNode);

            pData = pDataNode->pData;
            pDataNode->pData = NULL;

            list_add(pNode, &m_Dequeued);

            SBQ_LOG2("stt=%d, request=%d", pData->MagicNumberStt, pData->MagicNumberRequest);
            break;
        }
    }

    return pData;
}

}; //namespace NSCam

