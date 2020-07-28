#ifndef __DP_COMMAND_RECORDER_H__
#define __DP_COMMAND_RECORDER_H__

#include "DpDataType.h"
#include "DpDriver.h"
#include "DpCommand.h"
#include "DpEngineType.h"

#define MAX_MMPATH_INFO (2048)

#define MAX_TILE_NUM (2048)
#define ISP_MAX_OUTPUT_PORT_NUM (4)
#define MAX_MET_INFO (2048)
#define MAX_SECURE_INFO_COUNT (32)
typedef struct DpSecureInfo{
    uint64_t   securePortFlag;
    DpSecure   secureMode;
    uint32_t   secureRegAddr[3];
    uint32_t   secureHandle[3];
    uint32_t   securePortList[3];
    uint32_t   secureSizeList[3];
    uint32_t   secureOffsetList[3];
    uint32_t   secureBlockOffsetList[3];
} DpSecureInfo;

class DpCommandRecorder: public DpCommandBlock
{
public:
    enum BlockType
    {
        NONE_BLOCK  = -1,
        FRAME_BLOCK = DpCommand::FRAME_COMMAND,
        TILE_BLOCK  = DpCommand::TILE_COMMAND,
        EXT_FRAME_BLOCK,

        TOTAL_BLOCK_TYPES
    };

    enum SprReg
    {
        GCE_SPR_TEMP = 0x0,
        GCE_SPR_DATA = 0x1,
    };

    struct cmdq_instruction {
        uint16_t arg_c:16;
        uint16_t arg_b:16;
        uint16_t arg_a:16;
        uint8_t s_op:5;
        uint8_t arg_c_type:1;
        uint8_t arg_b_type:1;
        uint8_t arg_a_type:1;
        uint8_t op:8;
    };

    DpCommandRecorder(DpPathBase *path);

    virtual ~DpCommandRecorder();

    virtual uint32_t getScenario();

    virtual uint32_t getPriority();

    virtual uint64_t getEngineFlag();

    bool getPQReadback();

    bool getHDRReadback();

    int32_t getDREReadback();

    virtual void* getBlockBaseSW();

    virtual uint32_t getBlockSize();

    virtual void* getSecureAddrMD();

    virtual uint32_t getSecureAddrCount();

    virtual uint64_t getSecurePortFlag();

    void setSecureMode(DpEngineType type,
                       uint64_t     flag,
                       uint32_t     secureRegAddr[3],
                       DpSecure     secMode,
                       uint32_t     handle,
                       uint32_t     offset[3],
                       uint32_t     memSize[3],
                       uint32_t     planeOffset[3]);

    bool setSecureMetaData(uint32_t regAddr, uint32_t memAddr);

    void setFrameSrcInfo(DpColorFormat format,
                         int32_t       width,
                         int32_t       height,
                         int32_t       YPitch,
                         int32_t       UVPitch,
                         uint32_t      memAddr[3],
                         uint32_t      memSize[3],
                         DpSecure      secMode);

    void setFrameDstInfo(int32_t       portIndex,
                         DpColorFormat format,
                         int32_t       width,
                         int32_t       height,
                         int32_t       YPitch,
                         int32_t       UVPitch,
                         int32_t       outXStart,
                         int32_t       outYStart,
                         uint32_t      memAddr[3],
                         uint32_t      memSize[3],
                         DpSecure      secMode);

    virtual DpFrameInfo getFrameInfo();

    void setRegDstNum(int32_t regNum);

    virtual int32_t getRegDstNum();

    void markRecord(BlockType type);

    void markRecord(BlockType type, uint32_t x, uint32_t y);

    void reorder();

    void dupRecord(BlockType type);

    void initRecord();

    void stopRecord();

    void swapRecord();

    void resetRecord();

    int16_t getEngine(uint32_t addr);

    void appendMeta(uint16_t op, uint32_t addr, uint32_t value, uint32_t mask);

    void beginLabel();

    int32_t endLabel();

    void beginOverwrite(int32_t label);

    void endOverwrite();

    void flushRecord();

    void dumpRecord();

    void dumpRegister(uint64_t pqSessionId);

    inline void setDumpOrder(uint32_t dumpOrder);

    virtual uint32_t* getReadbackRegs(uint32_t& numReadRegs);

    virtual uint32_t* getReadbackValues(uint32_t& numValues);

    inline void setNumReadbackRegs(uint32_t numReadRegs);

    inline void setISPDebugDumpRegs(uint32_t numDumpRegs);

    virtual uint32_t getISPDebugDumpRegs();

    inline void setPath(DpPathBase* pPath);

    virtual char* getFrameInfoToCMDQ();

    bool getSyncMode();

    uint32_t* getReadbackPABuffer(uint32_t& readbackPABufferIndex);

    DP_STATUS_ENUM setNumReadbackPABuffer(uint32_t numReadbackPABuffer, uint32_t readbackEngineID);

    mdp_pmqos * getMdpPmqos();

    virtual void addMMpathLog(const char *name, uint32_t value, DP_MMPATH_ENUM type);

    virtual void setSubtaskId(uint32_t id);

    virtual void addMetLog(const char *name, uint32_t value);

    virtual void addIspMetLog(char *log, uint32_t size);

#if defined(CMDQ_V3) || defined(CMDQ_K414)
    //secure camera
    virtual bool hasIspSecMeta();

    virtual void addIspSecureMeta(cmdqSecIspMeta ispMeta);

    virtual cmdqSecIspMeta getSecIspMeta();
#endif

    uint32_t translateAddr(uint32_t addr);

private:
    typedef struct {
        int32_t   label;
        BlockType type;
        uint32_t  tileID;
        int32_t   offset;
        uint32_t  length;
    } LabelInfo;

    static inline uint32_t toTileID(uint32_t x, uint32_t y)
    {
        return (x & 0x1F) + ((y & 0x1F) << 5) + 1;
        //return (x & 0x3F) + ((y & 0x3F) << 6) + 1;
    }

    // must be used in reorder()
    op_meta *copyTile(op_meta *dst, uint32_t x, uint32_t y);

    // must be used in endLabel()
    LabelInfo *addLabel();

    LabelInfo *findLabel(int32_t label);

    void dupLabel(uint32_t index);

    DpPathBase *m_pPath;

    // back and front buffer
    op_meta    *m_pBackBuffer;
    op_meta    *m_pFrontBuffer;
    op_meta    *m_pExtBuffer;
    uint32_t   m_backBufferSize;
    uint32_t   m_frontBufferSize;
    uint32_t   m_extBufferSize;
    uint32_t   m_backLength;
    uint32_t   m_frontLength;
    uint32_t   m_extLength;

    // block and tile data
    op_meta    *m_pCurCommand;
    int32_t    m_blockOffset[TOTAL_BLOCK_TYPES];
    uint32_t   m_blockSize[TOTAL_BLOCK_TYPES];
    BlockType  m_blockType;
    int32_t    m_tileOffset[MAX_TILE_NUM];
    uint32_t   m_tileSize[MAX_TILE_NUM];
    uint32_t   m_tileID;

    int32_t    m_frontBlockOffset[TOTAL_BLOCK_TYPES];
    uint32_t   m_frontBlockSize[TOTAL_BLOCK_TYPES];

    // command label
    LabelInfo  *m_pBackLabels;
    LabelInfo  *m_pFrontLabels;
    uint32_t   m_backLabelCount;
    uint32_t   m_frontLabelCount;

    LabelInfo  *m_pCurLabel;
    op_meta   *m_pLabelCommand;
    op_meta   *m_pLastCommand;
    int32_t    m_labelIndex[MAX_TILE_NUM];
    uint32_t   m_nextLabel;

    int32_t    m_frontLabelIndex[MAX_TILE_NUM];

    // tile reordering
    uint32_t   m_dumpOrder;
    uint32_t   m_maxX;
    uint32_t   m_maxY;

    // secure mode
    DpSecure   m_secureMode;
    DpSecureInfo    m_secureInfo[MAX_SECURE_INFO_COUNT];
    uint32_t   m_secureInfoCount;
    cmdqSecAddrMetadataStruct m_secureAddrMD[30];
    uint32_t   m_addrListLength;

    // frame infomation
    DpColorFormat m_srcFormat;
    int32_t       m_srcWidth;
    int32_t       m_srcHeight;
    int32_t       m_srcYPitch;
    int32_t       m_srcUVPitch;
    uint32_t      m_srcMemAddr[3];
    uint32_t      m_srcMemSize[3];
    DpSecure      m_srcSecMode;

    DpColorFormat m_dstFormat[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t       m_dstWidth[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t       m_dstHeight[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t       m_dstYPitch[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t       m_dstUVPitch[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t       m_outXStart[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t       m_outYStart[ISP_MAX_OUTPUT_PORT_NUM];
    uint32_t      m_dstMemAddr[ISP_MAX_OUTPUT_PORT_NUM][3];
    uint32_t      m_dstMemSize[ISP_MAX_OUTPUT_PORT_NUM][3];
    DpSecure      m_dstSecMode[ISP_MAX_OUTPUT_PORT_NUM];

    int32_t       m_regDstNum;

    mdp_pmqos     m_mdp_pmqos;
    // readback registers
    uint32_t   m_readbackRegs[MAX_NUM_READBACK_REGS];
    uint32_t   m_readbackValues[MAX_NUM_READBACK_REGS];
    uint32_t   m_numReadbackRegs;
    uint32_t   m_ISPDebugDumpRegs;
    char       m_frameInfoToCMDQ[MAX_FRAME_INFO_SIZE];

    //MET profiling
    char       m_MET_info[MAX_MET_INFO];
    uint32_t   m_MET_size = 0;
    char       *m_ISP_MET_info = NULL;
    uint32_t   m_ISP_MET_size = 0;

    //MMpath
    char       m_MMpath_info[MAX_MMPATH_INFO];
    uint32_t   m_MMpath_info_size = 0;

    //Secure camera
    bool            m_hasIspSecMeta = false;
#if defined(CMDQ_V3) || defined(CMDQ_K414)
    cmdqSecIspMeta  m_ispMeta;
#endif

    /* base engine search */
    uint32_t m_lastBase;
    uint32_t m_lastEngine;
};


inline void DpCommandRecorder::setDumpOrder(uint32_t dumpOrder)
{
    m_dumpOrder = dumpOrder;
}

inline void DpCommandRecorder::setNumReadbackRegs(uint32_t numReadRegs)
{
    m_numReadbackRegs = numReadRegs;
}

inline void DpCommandRecorder::setISPDebugDumpRegs(uint32_t numDumpRegs)
{
    m_ISPDebugDumpRegs = numDumpRegs;
}

inline void DpCommandRecorder::setPath(DpPathBase* pPath)
{
    m_pPath = pPath;
}

#endif  // __DP_COMMAND_RECORDER_H__
