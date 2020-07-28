#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include "PQLogger.h"
#include "PQDREHistogramAdaptor.h"

#define MILLI_TO_NANO(v) (static_cast<nsecs_t>(v) * static_cast<nsecs_t>(1000L * 1000L))

PQDREHistogramAdaptor* PQDREHistogramAdaptor::s_pInstance[] = {};
PQMutex   PQDREHistogramAdaptor::s_ALMutex;

PQDREHistogramAdaptor* PQDREHistogramAdaptor::getInstance(uint32_t identifier)
{
    AutoMutex lock(s_ALMutex);

    if (identifier >= DRE_READBACK_MAX_NUM)
    {
        return NULL;
    }

    if (s_pInstance[identifier] == NULL)
    {
        s_pInstance[identifier] = new PQDREHistogramAdaptor(identifier);
        atexit(PQDREHistogramAdaptor::destroyInstance);
    }

    return s_pInstance[identifier];
}

void PQDREHistogramAdaptor::destroyInstance()
{
    AutoMutex lock(s_ALMutex);

    for (int identifier = 0; identifier < DRE_READBACK_MAX_NUM; identifier++)
    {
        if (s_pInstance[identifier] != NULL)
        {
            delete s_pInstance[identifier];
            s_pInstance[identifier] = NULL;
        }
    }
}

PQDREHistogramAdaptor::PQDREHistogramAdaptor(uint32_t identifier)
        : m_identifier(identifier),
        m_isAllUnregister(true)
{
    int i;

    PQ_LOGI("[PQDREHistogramAdaptor] PQDREHistogramAdaptor()... ");

    for (i = 0; i < DRE_BUFFER_MAX_NUM; i++)
    {
        m_buffer[i] = NULL;
    }
}

PQDREHistogramAdaptor::~PQDREHistogramAdaptor()
{
    int i;

    PQ_LOGI("[PQDREHistogramAdaptor] ~PQDREHistogramAdaptor()... ");

    for (i = 0; i < DRE_BUFFER_MAX_NUM; i++)
    {
        if (m_buffer[i] != NULL)
        {
            if (m_buffer[i]->pHistCond != NULL)
            {
                m_buffer[i]->pHistCond->signal();
                delete m_buffer[i]->pHistCond;
            }

            delete m_buffer[i];
            m_buffer[i] = NULL;
        }
    }
}

bool PQDREHistogramAdaptor::registerBuffer(void *_user_buffer, const unsigned long long userId,
        const uint32_t DRE_Cmd, uint32_t *bufferID)
{
    if (_user_buffer != NULL && (DRE_Cmd & DpDREParam::Cmd::Initialize) != 0)
    {
        return initUserBuffer(_user_buffer);
    }

    AutoMutex lock(s_ALMutex);
    bool isSuccess = false;

    do
    {
        if (userId == DRE_DEFAULT_USERID)
        {
            PQ_LOGE("[PQDREHistogramAdaptor][FAIL] userId[0x%llx] is not avaliable", userId);
            break;
        }

        isSuccess = searchBuffer(userId, bufferID);
        if (isSuccess == true)
        {
            break;
        }

        if ((DRE_Cmd & DpDREParam::Cmd::Initialize) == 0)
        {
            PQ_LOGI("[PQDREHistogramAdaptor]registerBuffer, DRE Cmd [0x%x] has no [Initialize]", DRE_Cmd);
            break;
        }

        isSuccess = findEmptyBuffer(userId, bufferID);
        if (isSuccess == true)
        {
            break;
        }

        isSuccess = createBuffer(userId, bufferID);
        if (isSuccess == true)
        {
            break;
        }
    } while (0);

    if (isSuccess == true)
    {
        m_isAllUnregister = false;
        PQ_LOGI("[PQDREHistogramAdaptor] get buffer[%d] for userId[0x%llx]",
            *bufferID, m_buffer[*bufferID]->userId);
    }
    else
    {
        PQ_LOGE("[PQDREHistogramAdaptor][FAIL] userId[0x%llx] is not in the buffer pool", userId);
        dumpBufferPoolImpl();
    }

    return isSuccess;
}

bool PQDREHistogramAdaptor::unregisterBuffer(const unsigned long long userId, const uint32_t DRE_Cmd)
{
    if ((DRE_Cmd & DpDREParam::Cmd::UnInitialize) == 0)
    {
        return true;
    }

    AutoMutex lock(s_ALMutex);
    int i;

    for (i = 0; i < DRE_BUFFER_MAX_NUM; i++)
    {
        if (m_buffer[i] != NULL)
        {
            if (userId == m_buffer[i]->userId)
            {
                PQ_LOGI("[PQDREHistogramAdaptor] unregisterBuffer success, [%d], userId[0x%llx] to [0x%llx]",
                    i, m_buffer[i]->userId, DRE_DEFAULT_USERID);

                m_buffer[i]->userId = DRE_DEFAULT_USERID;
                m_buffer[i]->isAvailable = false;

                return true;
            }
        }
    }

    PQ_LOGE("[PQDREHistogramAdaptor][FAIL] userId[0x%llx] is not in the buffer pool", userId);
    dumpBufferPoolImpl();

    return false;
}

void PQDREHistogramAdaptor::unregisterAllBuffer(void)
{
    AutoMutex lock(s_ALMutex);
    int i;

    if (m_isAllUnregister == true)
    {
        return;
    }

    for (i = 0; i < DRE_BUFFER_MAX_NUM; i++)
    {
        if (m_buffer[i] != NULL)
        {
            PQ_LOGI("[PQDREHistogramAdaptor] unregisterAllBuffer success, [%d], userId[0x%llx] to [0x%llx]",
                i, m_buffer[i]->userId, DRE_DEFAULT_USERID);

            m_buffer[i]->userId = DRE_DEFAULT_USERID;
            m_buffer[i]->isAvailable = false;
        }
    }

    m_isAllUnregister = true;
}

void PQDREHistogramAdaptor::setHistogram(uint32_t *pHist, uint32_t size, void *_user_buffer, const unsigned long long userId,
        int32_t srcWidth, int32_t srcHeight)
{
    AutoMutex lock(s_ALMutex);
    bool isUserBufferMode = false;
    uint32_t bufferID = DRE_ILLEGAL_BUFFER_ID;
    PQDREBuffer *DRE_buffer = NULL;

    if (_user_buffer != NULL)
    {
        DRE_buffer = static_cast<PQDREBuffer *>(_user_buffer);

        isUserBufferMode = true;
    }
    else if (searchBuffer(userId, &bufferID) == true)
    {
        DRE_buffer = m_buffer[bufferID];
    }
    else
    {
        PQ_LOGI("[PQDREHistogramAdaptor][FAIL] setHistogram with wrong data, bypass this frame");
        return;
    }

    if (srcWidth <= 0 || srcHeight <= 0)
    {
        PQ_LOGI("[PQDREHistogramAdaptor][FAIL] setHistogram with wrong previous size, bypass this frame");
        return;
    }

    if (size == DRE30_HIST_REGISTER_NUM)
    {
        //copy histgram data width and height
        DRE_buffer->hist[DRESrcWidth] = srcWidth;
        DRE_buffer->hist[DRESrcHeight] = srcHeight;
        //copy block histgram to user buffer
        memcpy(&DRE_buffer->hist[READBACKDRE_OFFSET], pHist, DRE30_HIST_REGISTER_NUM * sizeof(uint32_t));
        // set histogram buffer is available
        DRE_buffer->isAvailable = true;

        if (isUserBufferMode == false)
        {
            PQ_LOGI("[PQDREHistogramAdaptor] setHistogram, PQ Buffer Mode, userId[0x%llx], isAvailable[%d]",
                DRE_buffer->userId, DRE_buffer->isAvailable);

            DRE_buffer->pHistCond->signal();
        }
        else
        {
            PQ_LOGI("[PQDREHistogramAdaptor] setHistogram, User buffer Mode, buffer[0x%p], isAvailable[%d]",
                _user_buffer, DRE_buffer->isAvailable);
        }
    }
    else
    {
        PQ_LOGE("[PQDREHistogramAdaptor] setDREHistogram().. error, size = %d", size);
    }
}

void PQDREHistogramAdaptor::setBypass(void *_user_buffer, const unsigned long long userId, const DRETopOutput *outParam)
{
    AutoMutex lock(s_ALMutex);
    uint32_t bufferID = DRE_ILLEGAL_BUFFER_ID;
    PQDREBuffer *DRE_buffer = NULL;

    if (_user_buffer != NULL)
    {
        DRE_buffer = static_cast<PQDREBuffer *>(_user_buffer);
    }
    else if (searchBuffer(userId, &bufferID) == true)
    {
        DRE_buffer = m_buffer[bufferID];
    }
    else
    {
        PQ_LOGI("[PQDREHistogramAdaptor] setBypass with wrong data, bypass this frame");
        return;
    }

    memcpy(DRE_buffer->drePrevInfo.PreFloatCurve, outParam->CurFloatCurve, sizeof(DRE_buffer->drePrevInfo.PreFloatCurve));
    DRE_buffer->isAvailable = false;
}

bool PQDREHistogramAdaptor::getBufferData(void *_user_buffer, const uint32_t bufferID, const uint32_t DRE_Cmd, bool *isHistAvailable)
{
    AutoMutex lock(s_ALMutex);
    bool isUserBufferMode = false;
    bool isCmdCorrect = false;
    PQDREBuffer *DRE_buffer = NULL;

    *isHistAvailable = false;
    if (_user_buffer != NULL)
    {
        DRE_buffer = static_cast<PQDREBuffer *>(_user_buffer);

        isUserBufferMode = true;
    }
    else if (bufferID < DRE_BUFFER_MAX_NUM)
    {
        DRE_buffer = m_buffer[bufferID];
    }
    else
    {
        PQ_LOGE("[PQDREHistogramAdaptor] getBufferData, ERROR input");
        return false;
    }

    if ((DRE_Cmd & DpDREParam::Cmd::Default) != 0)
    {
        isCmdCorrect = true;
#if 0
        if (isUserBufferMode == false)
        {
            checkAndResetUnusedDREFW(DRE_buffer);
        }
#endif
        *isHistAvailable = DRE_buffer->isAvailable;
    }
    else
    {
        if ((DRE_Cmd & DpDREParam::Cmd::Apply) != 0)
        {
            isCmdCorrect = true;
            if (isUserBufferMode== false)
            {
                PQTimeValue startTime, currTime;
                uint32_t waitTime = DRE_APPLY_CMD_TIMEOUT_MS;
                int32_t diff;

                PQ_TIMER_GET_CURRENT_TIME(startTime);
                while (DRE_buffer->isAvailable == false)
                {
                    DRE_buffer->pHistCond->waitRelative(s_ALMutex, MILLI_TO_NANO(waitTime));

                    PQ_TIMER_GET_CURRENT_TIME(currTime);
                    PQ_TIMER_GET_DURATION_IN_MS(startTime, currTime, diff);

                    if (diff >= DRE_APPLY_CMD_TIMEOUT_MS)
                    {
                        break;
                    }
                    else
                    {
                        waitTime = DRE_APPLY_CMD_TIMEOUT_MS - diff;
                    }
                }

                if (DRE_buffer->isAvailable == false)
                {
                    PQ_TIMER_GET_CURRENT_TIME(currTime);
                    PQ_TIMER_GET_DURATION_IN_MS(DRE_buffer->workTime, currTime, diff);

                    PQ_LOGE("[PQDREHistogramAdaptor] getBufferData, PQ buffer Mode, wait timeout(ms)[%d] and use bypass setting: userId[0x%llx], Cmd[0x%x], isAvailable[%d], time diff with last worktime(ms)[%d]",
                        DRE_APPLY_CMD_TIMEOUT_MS, DRE_buffer->userId, DRE_Cmd, DRE_buffer->isAvailable, diff);
                }
            }
            else
            {
                DRE_buffer->isAvailable = true;
            }
            *isHistAvailable = DRE_buffer->isAvailable;
        }

        if ((DRE_Cmd & DpDREParam::Cmd::Generate) != 0)
        {
            isCmdCorrect = true;

            DRE_buffer->isAvailable = false;
            if ((DRE_Cmd & DpDREParam::Cmd::Apply) == 0)
            {
                *isHistAvailable = DRE_buffer->isAvailable;
            }

            if ((DRE_Cmd & DpDREParam::Cmd::UnInitialize) != 0)
            {
                PQ_LOGE("[PQDREHistogramAdaptor] getBufferData, DRE Cmd [0x%x] [Generate with UnInitialize] is not allowed", DRE_Cmd);
            }
        }
    }

    if (isCmdCorrect == false)
    {
        PQ_LOGE("[PQDREHistogramAdaptor] getBufferData, ERROR Cmd[0x%x]", DRE_Cmd);
        return false;
    }

    if (isUserBufferMode == false)
    {
        PQ_LOGI("[PQDREHistogramAdaptor] getBufferData, PQ buffer Mode, Cmd[0x%x], userId[0x%llx], isAvailable[%d]",
            DRE_Cmd, DRE_buffer->userId, DRE_buffer->isAvailable);
    }
    else
    {
        PQ_LOGI("[PQDREHistogramAdaptor] getBufferData, User buffer Mode, Cmd[0x%x], buffer[0x%p], isAvailable[%d]",
            DRE_Cmd, _user_buffer, DRE_buffer->isAvailable);
    }

    return true;
}

bool PQDREHistogramAdaptor::getDREInput(void *_user_buffer, const uint32_t bufferID,
        const int dre_blk_x_num, const int dre_blk_y_num, DRETopInput *inParam)
{
    AutoMutex lock(s_ALMutex);
    int blk_x, blk_y;
    uint32_t *pHist = NULL;
    PQDREBuffer *DRE_buffer = NULL;

    if (_user_buffer != NULL)
    {
        DRE_buffer = static_cast<PQDREBuffer *>(_user_buffer);
    }
    else if (bufferID < DRE_BUFFER_MAX_NUM)
    {
        DRE_buffer = m_buffer[bufferID];
    }
    else
    {
        PQ_LOGE("[PQDREHistogramAdaptor] setDREInput, ERROR input");
        return false;
    }

    // Get previous frame data
    memcpy(inParam->PreFloatCurve, DRE_buffer->drePrevInfo.PreFloatCurve, sizeof(inParam->PreFloatCurve));
    //copy histgram data width and height
    inParam->PreWidth = DRE_buffer->hist[DRESrcWidth];
    inParam->PreHeight = DRE_buffer->hist[DRESrcHeight];
    // Read Local histogram for DRE 3
    pHist = &DRE_buffer->hist[READBACKDRE_OFFSET];
    for (blk_y = 0; blk_y < dre_blk_y_num; blk_y++)
    {
        for (blk_x = 0; blk_x < dre_blk_x_num; blk_x++)
        {
            /* read each block histogram and info. */
            if (getDREBlock(pHist, blk_x, blk_y, dre_blk_x_num, dre_blk_y_num, inParam) == false)
            {
                return false;
            }
        }
    }

    return true;
}

bool PQDREHistogramAdaptor::storeDREOutput(void *_user_buffer, const uint32_t bufferID, const DRETopOutput *outParam)
{
    AutoMutex lock(s_ALMutex);
    PQDREBuffer *DRE_buffer = NULL;

    if (_user_buffer != NULL)
    {
        DRE_buffer = static_cast<PQDREBuffer *>(_user_buffer);
    }
    else if (bufferID < DRE_BUFFER_MAX_NUM)
    {
        DRE_buffer = m_buffer[bufferID];
    }
    else
    {
        PQ_LOGE("[PQDREHistogramAdaptor] storeDREOutput, ERROR input");
        return false;
    }

    PQTimeValue currTime;
    int32_t diff;

    memcpy(DRE_buffer->drePrevInfo.PreFloatCurve, outParam->CurFloatCurve, sizeof(DRE_buffer->drePrevInfo.PreFloatCurve));

    PQ_TIMER_GET_CURRENT_TIME(currTime);
    PQ_TIMER_GET_DURATION_IN_MS(DRE_buffer->workTime, currTime, diff);
    PQ_LOGI("[PQDREHistogramAdaptor] setCurrentTime() success, time diff with last worktime(ms)[%d]", diff);

    // update time of curr instance
    PQ_TIMER_GET_CURRENT_TIME(DRE_buffer->workTime);

    return true;
}

void PQDREHistogramAdaptor::dumpBufferPool(void)
{
    AutoMutex lock(s_ALMutex);

    dumpBufferPoolImpl();
}

bool PQDREHistogramAdaptor::searchBuffer(const unsigned long long userId, uint32_t *bufferID)
{
    int i;

    for (i = 0; i < DRE_BUFFER_MAX_NUM; i++)
    {
        if (m_buffer[i] == NULL)
        {
            continue;
        }

        if (userId == m_buffer[i]->userId)
        {
            *bufferID = i;

            return true;
        }
    }

    *bufferID = DRE_ILLEGAL_BUFFER_ID;
    return false;
}

bool PQDREHistogramAdaptor::findEmptyBuffer(const unsigned long long userId, uint32_t *bufferID)
{
    int i;

    for (i = 0; i < DRE_BUFFER_MAX_NUM; i++)
    {
        if (m_buffer[i] == NULL)
        {
            continue;
        }

        if (m_buffer[i]->userId == DRE_DEFAULT_USERID)
        {
            m_buffer[i]->userId = userId;
            m_buffer[i]->isAvailable = false;
            PQ_TIMER_GET_CURRENT_TIME(m_buffer[i]->workTime);

            *bufferID = i;

            return true;
        }
    }

    *bufferID = DRE_ILLEGAL_BUFFER_ID;
    return false;
}

bool PQDREHistogramAdaptor::createBuffer(const unsigned long long userId, uint32_t *bufferID)
{
    int i;

    for (i = 0; i < DRE_BUFFER_MAX_NUM; i++)
    {
        if (m_buffer[i] == NULL)
        {
            m_buffer[i] = new PQDREBuffer;
            m_buffer[i]->pHistCond = new DpCondition();
            m_buffer[i]->userId = DRE_DEFAULT_USERID;

            return findEmptyBuffer(userId, bufferID);
        }
    }

    *bufferID = DRE_ILLEGAL_BUFFER_ID;
    return false;
}

bool PQDREHistogramAdaptor::initUserBuffer(void *_user_buffer)
{
    PQDREBuffer *DRE_buffer = NULL;

    if (_user_buffer != NULL)
    {
        DRE_buffer = static_cast<PQDREBuffer *>(_user_buffer);

        DRE_buffer->isAvailable= false;
        PQ_TIMER_GET_CURRENT_TIME(DRE_buffer->workTime);
    }

    return true;
}

bool PQDREHistogramAdaptor::getDREBlock(uint32_t *pHist, const int block_x, const int block_y,
            const int dre_blk_x_num, const int dre_blk_y_num, DRETopInput *inParam)
{
    bool return_value = false;
    uint32_t read_value;
    uint32_t block_offset = 6 * (block_y * dre_blk_x_num + block_x);

    do {
        if (block_offset >= DRE30_HIST_REGISTER_NUM)
            break;
        read_value = pHist[block_offset++];
        inParam->DREMaxHisSet[block_y][block_x][0] = read_value & 0xff;
        inParam->DREMaxHisSet[block_y][block_x][1] = (read_value>>8) & 0xff;
        inParam->DREMaxHisSet[block_y][block_x][2] = (read_value>>16) & 0xff;
        inParam->DREMaxHisSet[block_y][block_x][3] = (read_value>>24) & 0xff;

        if (block_offset >= DRE30_HIST_REGISTER_NUM)
            break;
        read_value = pHist[block_offset++];
        inParam->DREMaxHisSet[block_y][block_x][4] = read_value & 0xff;
        inParam->DREMaxHisSet[block_y][block_x][5] = (read_value>>8) & 0xff;
        inParam->DREMaxHisSet[block_y][block_x][6] = (read_value>>16) & 0xff;
        inParam->DREMaxHisSet[block_y][block_x][7] = (read_value>>24) & 0xff;

        if (block_offset >= DRE30_HIST_REGISTER_NUM)
            break;
        read_value = pHist[block_offset++];
        inParam->DREMaxHisSet[block_y][block_x][8] = read_value & 0xff;
        inParam->DREMaxHisSet[block_y][block_x][9] = (read_value>>8) & 0xff;
        inParam->DREMaxHisSet[block_y][block_x][10] = (read_value>>16) & 0xff;
        inParam->DREMaxHisSet[block_y][block_x][11] = (read_value>>24) & 0xff;

        if (block_offset >= DRE30_HIST_REGISTER_NUM)
            break;
        read_value = pHist[block_offset++];
        inParam->DREMaxHisSet[block_y][block_x][12] = read_value & 0xff;
        inParam->DREMaxHisSet[block_y][block_x][13] = (read_value>>8) & 0xff;
        inParam->DREMaxHisSet[block_y][block_x][14] = (read_value>>16) & 0xff;
        inParam->DREMaxHisSet[block_y][block_x][15] = (read_value>>24) & 0xff;

        if (block_offset >= DRE30_HIST_REGISTER_NUM)
            break;
        read_value = pHist[block_offset++];
        inParam->DREMaxHisSet[block_y][block_x][16] = read_value & 0xff;
        inParam->DRERGBMaxSum[block_y][block_x] = (read_value>>8) & 0xff;
        inParam->DRELargeDiffCountSet[block_y][block_x] = (read_value>>16) & 0xff;
        inParam->DREMaxDiffSet[block_y][block_x] = (read_value>>24) & 0xff;

        if (block_offset >= DRE30_HIST_REGISTER_NUM)
            break;
        read_value = pHist[block_offset];
#ifdef CALTM_VERSION_6779
        inParam->DREChromaHist2[block_y][block_x] = read_value & 0xff;
#else
        inParam->DREChromaHist[block_y][block_x] = read_value & 0xff;
#endif
        inParam->DREFlatLineCountSet[block_y][block_x] = (read_value>>8) & 0xff;

        return_value = true;
    } while (0);

    return return_value;
}

void PQDREHistogramAdaptor::dumpBufferPoolImpl(void)
{
    int i;
    PQTimeValue currTime;
    int32_t diff;
    PQ_TIMER_GET_CURRENT_TIME(currTime);

    PQ_LOGD("[PQDREHistogramAdaptor] dumpBufferPool, Start:");
    for (i = 0; i < DRE_BUFFER_MAX_NUM; i++)
    {
        if (m_buffer[i] == NULL)
        {
            PQ_LOGD("[PQDREHistogramAdaptor] dumpBufferPool, ID[%d] is NULL", i);
        }
        else
        {
            PQ_TIMER_GET_DURATION_IN_MS(m_buffer[i]->workTime, currTime, diff);
            PQ_LOGD("[PQDREHistogramAdaptor] dumpBufferPool, ID[%d], userId[0x%llx], isAvailable[%d], time diff with last worktime(ms)[%d]",
                i, m_buffer[i]->userId, m_buffer[i]->isAvailable, diff);
        }
    }
}

void PQDREHistogramAdaptor::checkAndResetUnusedDREFW(PQDREBuffer *pDREbuffer)
{
    PQTimeValue currTime;
    PQ_TIMER_GET_CURRENT_TIME(currTime);

    int32_t diff;
    PQ_TIMER_GET_DURATION_IN_MS(pDREbuffer->workTime, currTime, diff);

    /*if DRE object is unused for a while, treat it as different content and reset
        DRE related members, or the continuity of histogram may be wrong. */
    if (diff >= DRE_NEW_CLIP_TIME_INTERVAL_MS)
    {
        PQ_LOGD("[PQDREHistogramAdaptor] checkAndResetUnusedDREFW(), time diff(ms)[%d]", diff);

        /* reset DRE FW object, it will be newed later in first frame condition */
        if (pDREbuffer->pHistCond != NULL)
        {
            pDREbuffer->pHistCond->signal();
            delete pDREbuffer->pHistCond;
            pDREbuffer->pHistCond = new DpCondition();
        }

        pDREbuffer->isAvailable = false;
    }
}

