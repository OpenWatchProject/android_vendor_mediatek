#ifndef __MTK_OMX_CMD_QUEUE_H__
#define __MTK_OMX_CMD_QUEUE_H__

#include <log/log.h>
#include <cutils/properties.h>
#include <queue>

#include "MtkOmxBase.h"
#include "osal_utils.h"

#define MAX_CMD_ITEM 512

#define MTK_OMX_CMDITEM_LOGD(thisclass, fmt, arg...) \
do { \
    if (MtkOmxCmdQueue::mEnableCmdItemLog) { \
        ALOGD("[0x%08x] " fmt, thisclass, ##arg); \
    } \
} while(0)

typedef struct _MTK_OMX_CMD_QUEUE_ITEM
{
    OMX_BOOL                    Used;
    MTK_OMX_COMMAND_CATEGORY    CmdCat;
    OMX_COMMANDTYPE             Cmd;
    OMX_U32                     CmdParam;
    OMX_U32                     buffer_type;
    OMX_BUFFERHEADERTYPE*       pBuffHead;
    OMX_PTR                     pCmdData;
    int                         nItemIndex;
}MTK_OMX_CMD_QUEUE_ITEM;

class MtkOmxCmdQueue
{
public:
    MtkOmxCmdQueue();
    ~MtkOmxCmdQueue();

    void NewCmdItem(MTK_OMX_CMD_QUEUE_ITEM** ppItem);
    void PutCmd(MTK_OMX_CMD_QUEUE_ITEM* pItem);
    void GetCmd(MTK_OMX_CMD_QUEUE_ITEM** ppItem);
    void FreeCmdItem(MTK_OMX_CMD_QUEUE_ITEM* pItem);
    int GetCmdSize();

    OMX_BOOL mEnableCmdItemLog;

private:
    MTK_OMX_CMD_QUEUE_ITEM mCmdQueueItems[MAX_CMD_ITEM];
    std::queue<MTK_OMX_CMD_QUEUE_ITEM*>   mCmdQueue;
    pthread_mutex_t mCmdMutex;
    sem_t           mCmdSem;
};

#endif