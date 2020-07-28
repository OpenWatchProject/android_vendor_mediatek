#ifndef __DP_COMMAND_H__
#define __DP_COMMAND_H__

#include "DpDataType.h"
#include "DpDriver.h"

class DpPathBase;

typedef enum DP_MMPATH_ENUM
{
    DP_MMPATH_ADDR,
    DP_MMPATH_FORMAT,
    DP_MMPATH_ENGINE,
    DP_MMPATH_TILEOVERHEAD,
    DP_MMPATH_OTHER
} DP_MMPATH_ENUM;

class DpCommand: public DpCommandBase
{
public:
    enum BLOCK_TYPE_ENUM
    {
        FRAME_COMMAND = 0,
        TILE_COMMAND  = 1
    };

    static DpCommand* Factory(DpPathBase *path, uint32_t venc_flag = 0);

    DpCommand()
    {
    }

    virtual ~DpCommand()
    {
    }

    virtual DP_STATUS_ENUM init() = 0;

    virtual DP_STATUS_ENUM reset() = 0;

    virtual DP_STATUS_ENUM secure(DpEngineType type,
                                  uint64_t     flag,
                                  uint32_t     secureRegAddr[3],
                                  DpSecure     secMode,
                                  uint32_t     handle,
                                  uint32_t     offset[3],//roi offset
                                  uint32_t     memSize[3],
                                  uint32_t     planeOffset[3]) = 0;//plane buffer offset

    virtual DP_STATUS_ENUM mark(BLOCK_TYPE_ENUM type) = 0;

    virtual DP_STATUS_ENUM mark(BLOCK_TYPE_ENUM type, uint32_t x, uint32_t y) = 0;

    virtual DP_STATUS_ENUM dup(BLOCK_TYPE_ENUM type) = 0;

    virtual DP_STATUS_ENUM write(uint32_t addr,
                                 uint32_t value,
                                 uint32_t mask = 0xFFFFFFFF) = 0;

    virtual DP_STATUS_ENUM write(uint32_t addr,
                                 uint32_t value,
                                 uint32_t mask,
                                 int32_t  *pLabel,
                                 int32_t  label = -1) = 0;

    virtual DP_STATUS_ENUM read(uint32_t addr,
                                 uint32_t value,
                                 int32_t  *pLabel,
                                 int32_t  label = -1) = 0;

    virtual DP_STATUS_ENUM writeFromReg(uint32_t addr,
                                        uint32_t value,
                                        uint32_t mask = 0xFFFFFFFF) = 0;

    virtual DP_STATUS_ENUM read(uint32_t addr,
                                uint32_t value,
                                uint32_t mask = 0xFFFFFFFF) = 0;

    virtual DP_STATUS_ENUM poll(uint32_t addr,
                                uint32_t value,
                                uint32_t mask = 0xFFFFFFFF) = 0;

    virtual DP_STATUS_ENUM wait(EVENT_TYPE_ENUM type) = 0;

    virtual DP_STATUS_ENUM waitNoClear(EVENT_TYPE_ENUM type) = 0;

    virtual DP_STATUS_ENUM setEvent(EVENT_TYPE_ENUM type) = 0;

    virtual DP_STATUS_ENUM clear(EVENT_TYPE_ENUM type) = 0;

    virtual DP_STATUS_ENUM stop(bool reorder = true) = 0;

    virtual DP_STATUS_ENUM swap() = 0;

    virtual DP_STATUS_ENUM flush(bool waitDone, DpJobID* pRet, char** pFrameInfo = NULL) = 0;

    virtual DP_STATUS_ENUM setDumpOrder(uint32_t dumpOrder) = 0;

    virtual DP_STATUS_ENUM setReadbackRegs() = 0;

    virtual DP_STATUS_ENUM setISPDebugDumpRegs(uint32_t DumpRegs) = 0;

    virtual uint32_t* getReadbackValues(uint32_t& numValues) = 0;

    virtual void setPath(DpPathBase* pPath) = 0;

    virtual void setTDShpPar(uint32_t TDShpPar) = 0;

    virtual uint32_t getTDShpPar() = 0;

#ifdef MDP_VERSION_2_0
    virtual DP_STATUS_ENUM frameSrcInfo(DpColorFormat format,
                                        int32_t       width,
                                        int32_t       height,
                                        int32_t       YPitch,
                                        int32_t       UVPitch,
                                        uint32_t      memAddr[3],
                                        uint32_t      memSize[3],
                                        DpSecure      secMode) = 0;

    virtual DP_STATUS_ENUM frameDstInfo(DpColorFormat format,
                                        int32_t       width,
                                        int32_t       height,
                                        int32_t       YPitch,
                                        int32_t       UVPitch,
                                        int32_t       outXStart,
                                        int32_t       outYStart,
                                        uint32_t      memAddr[3],
                                        uint32_t      memSize[3],
                                        DpSecure      secMode) = 0;

    virtual void initRegDstNum();

    virtual void setPQSessionID(uint64_t PqSessionID) = 0;

    virtual uint64_t getPQSessionID() = 0;
#endif

    virtual void getReadbackStatus(bool& pq_readback, bool& hdr_readback, int32_t& dre_readback, uint64_t& engineFlag, uint32_t& VEncFlag) = 0;

    virtual bool getSyncMode() = 0;

    virtual uint32_t* getReadbackPABuffer(uint32_t& readbackPABufferIndex) = 0;

    virtual DP_STATUS_ENUM setNumReadbackPABuffer(uint32_t numReadbackPABuffer, uint32_t readbackEngineID) = 0;

    virtual void addMMpathLog(const char *name, uint32_t value, DP_MMPATH_ENUM type) = 0;

    virtual void setSubtaskId(uint32_t id) = 0;

    virtual void addMetLog(const char *name, uint32_t value) = 0;

    virtual void addIspMetLog(char *log, uint32_t size) = 0;

#if defined(CMDQ_V3) || defined(CMDQ_K414)
    virtual void addIspSecureMeta(cmdqSecIspMeta ispMeta) = 0;
#endif
};

#endif  // __DP_COMMAND_H__
