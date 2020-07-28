#ifndef __PQDREHISTOGRAMADAPTOR_H__
#define __PQDREHISTOGRAMADAPTOR_H__

#include "PQMutex.h"
#include "PQTimer.h"
#include "DpDataType.h"
#include "DpCondition.h"

#include "mdpAALImpl.h"

#define DRE_READBACK_MAX_NUM           (1)
#define DRE_BUFFER_MAX_NUM             (8)
#define DRE_ILLEGAL_BUFFER_ID          (DRE_BUFFER_MAX_NUM+1)
#define DRE_NEW_CLIP_TIME_INTERVAL_MS  (1000)
#define DRE_APPLY_CMD_TIMEOUT_MS       (1000)
#define DRE30_HIST_REGISTER_NUM        (768)

enum READBACKDRE
{
    DRESrcWidth = 0,
    DRESrcHeight,
    READBACKDRE_OFFSET
};

#define DRE_TOTAL_REGISTER_NUM        (DRE30_HIST_REGISTER_NUM + READBACKDRE_OFFSET)

struct DREPrevInfo
{
    // Previous Float Curve
    int PreFloatCurve[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X][mdpDRE_LUMA_CURVE_NUM]; // 15-bit [0, 255*128]
};

struct PQDREBuffer
{
    unsigned long long  userId;
    uint32_t            hist[DRE_TOTAL_REGISTER_NUM];
    DREPrevInfo         drePrevInfo;
    bool                isAvailable;
    PQTimeValue         workTime;
    DpCondition         *pHistCond;
};

class PQDREHistogramAdaptor
{
public:
    PQDREHistogramAdaptor(uint32_t identifier);
    ~PQDREHistogramAdaptor();

    static PQDREHistogramAdaptor* getInstance(uint32_t identifier);
    static void destroyInstance();

    bool registerBuffer(void *_user_buffer, const unsigned long long userId,
        const uint32_t DRE_Cmd, uint32_t *bufferID);
    bool unregisterBuffer(const unsigned long long userId, const uint32_t DRE_Cmd);
    void unregisterAllBuffer(void);
    void setHistogram(uint32_t *pHist, uint32_t size, void *_user_buffer, const unsigned long long userId,
        int32_t srcWidth, int32_t srcHeight);
    void setBypass(void *_user_buffer, const unsigned long long userId, const DRETopOutput *outParam);
    bool getBufferData(void *_user_buffer, const uint32_t bufferID, const uint32_t DRE_Cmd, bool *isHistAvailable);
    bool getDREInput(void *_user_buffer, const uint32_t bufferID,
        const int dre_blk_x_num, const int dre_blk_y_num, DRETopInput *inParam);
    bool storeDREOutput(void *_user_buffer, const uint32_t bufferID, const DRETopOutput *outParam);
    void dumpBufferPool(void);

private:
    bool searchBuffer(const unsigned long long userId, uint32_t *bufferID);
    bool findEmptyBuffer(const unsigned long long userId, uint32_t *bufferID);
    bool createBuffer(const unsigned long long userId, uint32_t *bufferID);
    bool initUserBuffer(void *_user_buffer);
    bool getDREBlock(uint32_t *pHist, const int block_x, const int block_y,
        const int dre_blk_x_num, const int dre_blk_y_num, DRETopInput *inParam);
    void dumpBufferPoolImpl(void);
    void checkAndResetUnusedDREFW(PQDREBuffer *pDREbuffer);

private:
    static PQDREHistogramAdaptor *s_pInstance[DRE_READBACK_MAX_NUM];
    static PQMutex  s_ALMutex;

    uint32_t m_identifier;
    bool m_isAllUnregister;
    PQDREBuffer *m_buffer[DRE_BUFFER_MAX_NUM];
};
#endif //__PQDREHISTOGRAMADAPTOR_H__