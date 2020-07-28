#ifndef __DP_COMMAND_QUEUE_H__
#define __DP_COMMAND_QUEUE_H__

#include "DpCommand.h"
#include "DpCommandRecorder.h"

class DpCommandQueue : public DpCommand
{
public:
    DpCommandQueue(DpPathBase *path, uint32_t venc_flag = 0);

    virtual ~DpCommandQueue();

    virtual DP_STATUS_ENUM init();

    virtual DP_STATUS_ENUM reset();

    virtual DP_STATUS_ENUM secure(DpEngineType type,
                                  uint64_t     flag,
                                  uint32_t     secureRegAddr[3],
                                  DpSecure     secMode,
                                  uint32_t     handle,
                                  uint32_t     offset[3],
                                  uint32_t     memSize[3],
                                  uint32_t     planeOffset[3]);

    virtual DP_STATUS_ENUM mark(BLOCK_TYPE_ENUM type);

    virtual DP_STATUS_ENUM mark(BLOCK_TYPE_ENUM type, uint32_t x, uint32_t y);

    virtual DP_STATUS_ENUM dup(BLOCK_TYPE_ENUM type);

    virtual DP_STATUS_ENUM write(uint32_t addr,
                                 uint32_t value,
                                 uint32_t mask = 0xFFFFFFFF);

    virtual DP_STATUS_ENUM write(uint32_t addr,
                                 uint32_t value,
                                 uint32_t mask,
                                 int32_t  *pLabel,
                                 int32_t  label = -1);

    virtual DP_STATUS_ENUM read(uint32_t addr,
                                 uint32_t value,
                                 int32_t  *pLabel,
                                 int32_t  label = -1);

    virtual DP_STATUS_ENUM writeFromReg(uint32_t addr,
                                        uint32_t value,
                                        uint32_t mask = 0xFFFFFFFF);

    virtual DP_STATUS_ENUM read(uint32_t addr,
                                uint32_t value,
                                uint32_t mask = 0xFFFFFFFF);

    virtual DP_STATUS_ENUM poll(uint32_t addr,
                                uint32_t value,
                                uint32_t mask = 0xFFFFFFFF);

    virtual DP_STATUS_ENUM wait(EVENT_TYPE_ENUM event);

    virtual DP_STATUS_ENUM waitNoClear(EVENT_TYPE_ENUM type);

    virtual DP_STATUS_ENUM clear(EVENT_TYPE_ENUM event);

    virtual DP_STATUS_ENUM setEvent(EVENT_TYPE_ENUM event);

    virtual DP_STATUS_ENUM stop(bool reorder = true);

    virtual DP_STATUS_ENUM swap();

    virtual DP_STATUS_ENUM flush(bool waitDone, DpJobID* pRet, char** pFrameInfo = NULL);

    virtual DP_STATUS_ENUM setDumpOrder(uint32_t dumpOrder);

    virtual DP_STATUS_ENUM setReadbackRegs();

    virtual uint32_t* getReadbackValues(uint32_t& numValues);

    virtual DP_STATUS_ENUM setISPDebugDumpRegs(uint32_t DumpRegs);

    virtual void setPath(DpPathBase* pPath);

    virtual void setTDShpPar(uint32_t TDShpPar);

    virtual uint32_t getTDShpPar();

#ifdef MDP_VERSION_2_0
    virtual DP_STATUS_ENUM frameSrcInfo(DpColorFormat format,
                                        int32_t       width,
                                        int32_t       height,
                                        int32_t       YPitch,
                                        int32_t       UVPitch,
                                        uint32_t      memAddr[3],
                                        uint32_t      memSize[3],
                                        DpSecure      secMode);

    virtual DP_STATUS_ENUM frameDstInfo(DpColorFormat format,
                                        int32_t       width,
                                        int32_t       height,
                                        int32_t       YPitch,
                                        int32_t       UVPitch,
                                        int32_t       outXStart,
                                        int32_t       outYStart,
                                        uint32_t      memAddr[3],
                                        uint32_t      memSize[3],
                                        DpSecure      secMode);

    virtual void initRegDstNum();

    virtual void setPQSessionID(uint64_t PqSessionID);

    virtual uint64_t getPQSessionID();
#endif

    virtual void getReadbackStatus(bool& pq_readback, bool& hdr_readback, int32_t& dre_readback, uint64_t& engineFlag, uint32_t& VEncFlag);

    virtual bool getSyncMode();

    virtual uint32_t* getReadbackPABuffer(uint32_t& readbackPABufferIndex);

    virtual DP_STATUS_ENUM setNumReadbackPABuffer(uint32_t numReadbackPABuffer, uint32_t readbackEngineID);

    virtual void setSubtaskId(uint32_t id);

    virtual void addMetLog(const char *name, uint32_t value);

    virtual void addIspMetLog(char *log, uint32_t size);

#if defined(CMDQ_V3) || defined(CMDQ_K414)
    virtual void addIspSecureMeta(cmdqSecIspMeta ispMeta);
#endif

    virtual void addMMpathLog(const char *name, uint32_t value, DP_MMPATH_ENUM type);

private:
    DpCommandRecorder m_recorder;
    const uint32_t    m_VEncFlag;
    uint32_t          m_parTDSHP;
    uint64_t          m_PqSessionID;
};

#endif  // __DP_COMMAND_QUEUE_H__
