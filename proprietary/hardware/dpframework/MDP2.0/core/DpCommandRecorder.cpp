#include "DpCommandRecorder.h"
#include "DpEngineType.h"
#include "DpPathBase.h"

#include "m4u_port_map.h"
#include "mdp_reg_rdma.h"
#include "mdp_reg_wdma.h"
#include "mdp_reg_wrot.h"
#include "DpPlatform.h"
#if CONFIG_FOR_OS_ANDROID
#ifndef BASIC_PACKAGE
#include "PQSessionManager.h"
#endif //BASIC_PACKAGE
#endif // CONFIG_FOR_OS_ANDROID
#include <string.h>

#define UNIT_COMMAND_BLOCK_SIZE (sizeof(op_meta) * 4096)
#define BASE_COMMAND_BLOCK_SIZE (UNIT_COMMAND_BLOCK_SIZE * 5)

#define CMDQ_SUBSYS_INVALID     (-1)
#define CMDQ_SUBSYS_UNDEFINED   (99)
#define CMDQ_ADDR_MASK          0xFFFFFFFC
#define CMDQ_ADDR_MASK_16       0xFFFC


static inline int32_t get_cmdq_subsys(uint32_t argA)
{
    int32_t subsys = (argA & 0xffff0000) >> 16;

    switch (subsys) {
#define DECLARE_CMDQ_SUBSYS(msb, id, grp, base) case msb: return id;
#include "cmdq_subsys.h"
#undef DECLARE_CMDQ_SUBSYS
        default:
            // White list
            if (0x1502 == subsys || 0x1501 == subsys || 0x1500 == subsys)// ISP DIP WPE CQ
            {
                return CMDQ_SUBSYS_UNDEFINED;
            }
            else if (0x1f00 == subsys || 0x1f01 == subsys || 0x1f02 == subsys)              //mt6789 mdpsubsys
            {
                return CMDQ_SUBSYS_UNDEFINED;
            }
            else
            {
                DPLOGE("DpCommandRecorder: unknown cmdq subsys %#06x", subsys);
            }
    }
    return CMDQ_SUBSYS_INVALID;
}

struct DpEngineAddrBase
{
    uint16_t engine;
    uint32_t base;
};

DpCommandRecorder::DpCommandRecorder(DpPathBase *path)
    : m_pPath(path)
    , m_pBackBuffer(NULL)
    , m_pFrontBuffer(NULL)
    , m_pExtBuffer(NULL)
    , m_backBufferSize(0)
    , m_frontBufferSize(0)
    , m_extBufferSize(0)
    , m_backLength(0)
    , m_frontLength(0)
    , m_extLength(0)
    , m_pCurCommand(NULL)
    , m_blockType(NONE_BLOCK)
    , m_tileID(0)
    , m_pBackLabels(NULL)
    , m_pFrontLabels(NULL)
    , m_backLabelCount(0)
    , m_frontLabelCount(0)
    , m_pCurLabel(NULL)
    , m_pLabelCommand(NULL)
    , m_pLastCommand(NULL)
    , m_nextLabel(0)
    , m_dumpOrder(0)
    , m_maxX(0)
    , m_maxY(0)
    , m_secureMode(DP_SECURE_NONE)
    , m_secureInfoCount(0)
    , m_addrListLength(0)
    , m_srcFormat(DP_COLOR_UNKNOWN)
    , m_srcWidth(0)
    , m_srcHeight(0)
    , m_srcYPitch(0)
    , m_srcUVPitch(0)
    , m_srcSecMode(DP_SECURE_NONE)
    , m_regDstNum(0)
    , m_numReadbackRegs(0)
    , m_ISPDebugDumpRegs(0)
    , m_hasIspSecMeta(false)
{
    DPLOGI("DpCommandRecorder: create DpCommandRecorder\n");

    memset(m_blockOffset, -1, sizeof(m_blockOffset));
    memset(m_blockSize, 0, sizeof(m_blockSize));
    memset(m_tileOffset, -1, sizeof(m_tileOffset));
    memset(m_tileSize, 0, sizeof(m_tileSize));

    memset(m_frontBlockOffset, -1, sizeof(m_frontBlockOffset));
    memset(m_frontBlockSize, 0, sizeof(m_frontBlockSize));

    memset(m_labelIndex, -1, sizeof(m_labelIndex));
    memset(m_frontLabelIndex, -1, sizeof(m_frontLabelIndex));

    memset(m_secureInfo, 0x0, sizeof(m_secureInfo));
    memset(m_secureAddrMD, 0x0, sizeof(m_secureAddrMD));
#if defined(CMDQ_V3) || defined(CMDQ_K414)
    memset(&m_ispMeta, 0x0, sizeof(m_ispMeta));
#endif

    memset(m_srcMemAddr, 0x0, sizeof(m_srcMemAddr));
    memset(m_srcMemSize, 0x0, sizeof(m_srcMemSize));
    memset(m_dstFormat, 0x0, sizeof(m_dstFormat));
    memset(m_dstWidth, 0x0, sizeof(m_dstWidth));
    memset(m_dstHeight, 0x0, sizeof(m_dstHeight));
    memset(m_dstYPitch, 0x0, sizeof(m_dstYPitch));
    memset(m_dstUVPitch, 0x0, sizeof(m_dstUVPitch));
    memset(m_dstMemAddr, 0x0, sizeof(m_dstMemAddr));
    memset(m_dstMemSize, 0x0, sizeof(m_dstMemSize));
    memset(m_dstSecMode, 0x0, sizeof(m_dstSecMode));

    memset(m_readbackRegs, 0, sizeof(m_readbackRegs));
    memset(m_readbackValues, 0, sizeof(m_readbackValues));
    memset(m_frameInfoToCMDQ, 0, sizeof(m_frameInfoToCMDQ));
    memset(&m_mdp_pmqos, 0, sizeof(m_mdp_pmqos));
}


DpCommandRecorder::~DpCommandRecorder()
{
    DPLOGI("DpCommandRecorder: destroy DpCommandRecorder\n");

    free(m_pBackBuffer);
    m_pBackBuffer = NULL;

    free(m_pFrontBuffer);
    m_pFrontBuffer = NULL;

    free(m_pExtBuffer);
    m_pExtBuffer = NULL;

    free(m_pBackLabels);
    m_pBackLabels = NULL;

    free(m_pFrontLabels);
    m_pFrontLabels = NULL;
}


uint32_t DpCommandRecorder::getScenario()
{
    DP_STATUS_ENUM   status;
    STREAM_TYPE_ENUM type;

    status = m_pPath->getScenario(&type);
#if !CONFIG_FOR_VERIFY_FPGA
    assert(DP_STATUS_RETURN_SUCCESS == status);
#endif

    return (uint32_t)type;
}


bool DpCommandRecorder::getPQReadback()
{
    DP_STATUS_ENUM status;
    bool           readback;

    status = m_pPath->getPQReadback(&readback);
#if !CONFIG_FOR_VERIFY_FPGA
    assert(DP_STATUS_RETURN_SUCCESS == status);
#endif

    return readback;
}


bool DpCommandRecorder::getHDRReadback()
{
    DP_STATUS_ENUM status;
    bool           readback;

    status = m_pPath->getHDRReadback(&readback);
#if !CONFIG_FOR_VERIFY_FPGA
    assert(DP_STATUS_RETURN_SUCCESS == status);
#endif

    return readback;
}

int32_t DpCommandRecorder::getDREReadback()
{
    DP_STATUS_ENUM status;
    int32_t        readback;

    status = m_pPath->getDREReadback(&readback);
#if !CONFIG_FOR_VERIFY_FPGA
    assert(DP_STATUS_RETURN_SUCCESS == status);
#endif

    return readback;
}

uint32_t DpCommandRecorder::getPriority()
{
    DP_STATUS_ENUM status;
    int32_t        priority;

    status = m_pPath->getPriority(&priority);
#if !CONFIG_FOR_VERIFY_FPGA
    assert(DP_STATUS_RETURN_SUCCESS == status);
#endif

    return priority;
}


uint64_t DpCommandRecorder::getEngineFlag()
{
    DP_STATUS_ENUM   status;
    STREAM_TYPE_ENUM type;
    int64_t          flag = 0;

    status = m_pPath->getPathFlag(&flag);
#if !CONFIG_FOR_VERIFY_FPGA
    assert(DP_STATUS_RETURN_SUCCESS == status);
#endif

    status = m_pPath->getScenario(&type);
#if !CONFIG_FOR_VERIFY_FPGA
    assert(DP_STATUS_RETURN_SUCCESS == status);
#endif

#if 0
    if (STREAM_FRAG_JPEGDEC == type) // embedded HW JPEG DEC mode for FragStream
    {
        flag |= (1LL << tJPEGDEC);
    }
#endif

    return flag;
}


void* DpCommandRecorder::getBlockBaseSW()
{
    return m_pFrontBuffer;
}


uint32_t DpCommandRecorder::getBlockSize()
{
    return m_frontLength;
}


void* DpCommandRecorder::getSecureAddrMD()
{
    DPLOGI("m_secureAddrMD addr: %p\n", m_secureAddrMD);
    return m_secureAddrMD;
}


uint32_t DpCommandRecorder::getSecureAddrCount()
{
    DPLOGI("m_addrListLength: %d\n", m_addrListLength);
    return m_addrListLength;
}

uint64_t DpCommandRecorder::getSecurePortFlag()
{
    uint64_t rtn = 0;
    for (int i = 0; i < MAX_SECURE_INFO_COUNT; i++)
    {
        if (m_secureInfo[i].securePortFlag == 0)
            break;
        rtn |= m_secureInfo[i].securePortFlag;
    }
    DPLOGI("DpCommandRecorder::getSecurePortFlag%x", rtn);
    return rtn;
}

void DpCommandRecorder::setSecureMode(DpEngineType type,
                                      uint64_t     flag,
                                      uint32_t     secureRegAddr[3],
                                      DpSecure     secMode,
                                      uint32_t     handle,
                                      uint32_t     offset[3],
                                      uint32_t     memSize[3],
                                      uint32_t     planeOffset[3])
{
    int i;
    if(m_secureInfoCount >= MAX_SECURE_INFO_COUNT)
    {
        DPLOGE("Secure Info overflow, current count: %d", m_secureInfoCount);
        return;
    }

    m_secureMode = secMode;

    DPLOGI("Secure mode %d Engine %d Flag 0x%08x\n", secMode, type, flag);
    DPLOGI("Hand 0x%08x\n", handle);
    DPLOGI("Addr 0x%08x 0x%08x 0x%08x\n", secureRegAddr[0], secureRegAddr[1], secureRegAddr[2]);
    DPLOGI("Offs 0x%08x 0x%08x 0x%08x\n", offset[0], offset[1], offset[2]);
    DPLOGI("Size 0x%08x 0x%08x 0x%08x\n", memSize[0], memSize[1], memSize[2]);
    DPLOGI("POff 0x%08x 0x%08x 0x%08x\n", planeOffset[0], planeOffset[1], planeOffset[2]);

    if (DP_SECURE_NONE != secMode)
    {
        for (i = 0; i < m_secureInfoCount; i++)
        {
             if(m_secureInfo[i].secureRegAddr[0] == secureRegAddr[0])
                 break;
        }

        m_secureInfo[i].securePortFlag = flag;
        m_secureInfo[i].secureMode = secMode;

        m_secureInfo[i].secureRegAddr[0] = secureRegAddr[0];
        m_secureInfo[i].secureRegAddr[1] = secureRegAddr[1];
        m_secureInfo[i].secureRegAddr[2] = secureRegAddr[2];

        m_secureInfo[i].secureHandle[0] = handle;
        m_secureInfo[i].secureHandle[1] = handle;
        m_secureInfo[i].secureHandle[2] = handle;

        m_secureInfo[i].secureOffsetList[0] = offset[0];
        m_secureInfo[i].secureOffsetList[1] = offset[1];
        m_secureInfo[i].secureOffsetList[2] = offset[2];

        m_secureInfo[i].secureBlockOffsetList[0] = planeOffset[0];
        m_secureInfo[i].secureBlockOffsetList[1] = planeOffset[1];
        m_secureInfo[i].secureBlockOffsetList[2] = planeOffset[2];

        m_secureInfo[i].secureSizeList[0] = memSize[0];
        m_secureInfo[i].secureSizeList[1] = memSize[1];
        m_secureInfo[i].secureSizeList[2] = memSize[2];

        m_secureInfo[i].securePortList[0] = convertPort(type, 0);
        m_secureInfo[i].securePortList[1] = convertPort(type, 1);
        m_secureInfo[i].securePortList[2] = convertPort(type, 2);

        if (i >= m_secureInfoCount)
        m_secureInfoCount++;
    }
}


bool DpCommandRecorder::setSecureMetaData(uint32_t regAddr, uint32_t memAddr)
{
    int32_t index, sub_index;

    if (m_secureMode == DP_SECURE_NONE)
        return false;

    if (!memAddr)
        return false;

    for (index = 0; index < m_secureInfoCount; index++)
    {
        if (regAddr == m_secureInfo[index].secureRegAddr[0])
        {
            sub_index = 0;
            break;
        }
        else if (regAddr == m_secureInfo[index].secureRegAddr[1])
        {
            sub_index = 1;
            break;
        }
        else if(regAddr == m_secureInfo[index].secureRegAddr[2])
        {
            sub_index = 2;
            break;
        }
    }

    if (index >= m_secureInfoCount)
        return false;
    if(m_addrListLength >= 30)
    {
        DPLOGE("secureAddr list overflow! addr %08x, index %d", m_secureInfo[index].secureRegAddr[sub_index],  ((unsigned long)m_pCurCommand - (unsigned long)m_pBackBuffer) / sizeof(op_meta));
        return false;
    }

    m_secureAddrMD[m_addrListLength].baseHandle = m_secureInfo[index].secureHandle[sub_index];
    m_secureAddrMD[m_addrListLength].instrIndex = ((unsigned long)m_pCurCommand - (unsigned long)m_pBackBuffer) / sizeof(op_meta);
    m_secureAddrMD[m_addrListLength].offset = m_secureInfo[index].secureOffsetList[sub_index];
    m_secureAddrMD[m_addrListLength].blockOffset = m_secureInfo[index].secureBlockOffsetList[sub_index];
#if defined(CMDQ_V3) || defined(CMDQ_K414)
    if(m_secureInfo[index].secureMode == DP_SECURE_PROTECTED)
        m_secureAddrMD[m_addrListLength].type = CMDQ_SAM_PH_2_MVA;
    else
#endif
        m_secureAddrMD[m_addrListLength].type = CMDQ_SAM_H_2_MVA;
    m_secureAddrMD[m_addrListLength].size = m_secureInfo[index].secureSizeList[sub_index];
    m_secureAddrMD[m_addrListLength].port = m_secureInfo[index].securePortList[sub_index];

    DPLOGI("DpCommandRecorder: secure index %d m_secureAddrMD[%d] addr %08x, index %d handle %016llx offset %08x %08x size %08x port %d type %d\n",
        index, m_addrListLength,
        m_secureInfo[index].secureRegAddr[sub_index],
        m_secureAddrMD[m_addrListLength].instrIndex,
        m_secureAddrMD[m_addrListLength].baseHandle,
        m_secureAddrMD[m_addrListLength].blockOffset,
        m_secureAddrMD[m_addrListLength].offset,
        m_secureAddrMD[m_addrListLength].size,
        m_secureAddrMD[m_addrListLength].port,
        m_secureAddrMD[m_addrListLength].type);

    m_addrListLength++;

    return true;
}

void DpCommandRecorder::setFrameSrcInfo(DpColorFormat format,
                                        int32_t       width,
                                        int32_t       height,
                                        int32_t       YPitch,
                                        int32_t       UVPitch,
                                        uint32_t      memAddr[3],
                                        uint32_t      memSize[3],
                                        DpSecure      secMode)
{
    m_srcFormat = format;
    m_srcWidth = width;
    m_srcHeight = height;
    m_srcYPitch = YPitch;
    m_srcUVPitch = UVPitch;
    m_srcMemAddr[0] = memAddr[0];
    m_srcMemAddr[1] = memAddr[1];
    m_srcMemAddr[2] = memAddr[2];
    m_srcMemSize[0] = memSize[0];
    m_srcMemSize[1] = memSize[1];
    m_srcMemSize[2] = memSize[2];
    m_srcSecMode = secMode;
}

void DpCommandRecorder::setFrameDstInfo(int32_t       portIndex,
                                        DpColorFormat format,
                                        int32_t       width,
                                        int32_t       height,
                                        int32_t       YPitch,
                                        int32_t       UVPitch,
                                        int32_t       outXStart,
                                        int32_t       outYStart,
                                        uint32_t      memAddr[3],
                                        uint32_t      memSize[3],
                                        DpSecure      secMode)
{
    m_dstFormat[portIndex] = format;
    m_dstWidth[portIndex] = width;
    m_dstHeight[portIndex] = height;
    m_dstYPitch[portIndex] = YPitch;
    m_dstUVPitch[portIndex] = UVPitch;
    m_outXStart[portIndex] = outXStart;
    m_outYStart[portIndex] = outYStart;
    m_dstMemAddr[portIndex][0] = memAddr[0];
    m_dstMemAddr[portIndex][1] = memAddr[1];
    m_dstMemAddr[portIndex][2] = memAddr[2];
    m_dstMemSize[portIndex][0] = memSize[0];
    m_dstMemSize[portIndex][1] = memSize[1];
    m_dstMemSize[portIndex][2] = memSize[2];
    m_dstSecMode[portIndex] = secMode;
}

DpFrameInfo DpCommandRecorder::getFrameInfo()
{
    DpFrameInfo frameInfo;

    frameInfo.m_srcFormat = m_srcFormat;
    frameInfo.m_srcWidth = m_srcWidth;
    frameInfo.m_srcHeight = m_srcHeight;
    frameInfo.m_srcYPitch = m_srcYPitch;
    frameInfo.m_srcUVPitch = m_srcUVPitch;
    frameInfo.m_srcMemAddr[0] = m_srcMemAddr[0];
    frameInfo.m_srcMemAddr[1] = m_srcMemAddr[1];
    frameInfo.m_srcMemAddr[2] = m_srcMemAddr[2];
    frameInfo.m_srcMemSize[0] = m_srcMemSize[0];
    frameInfo.m_srcMemSize[1] = m_srcMemSize[1];
    frameInfo.m_srcMemSize[2] = m_srcMemSize[2];
    frameInfo.m_srcSecMode = m_srcSecMode;

    for (int index = 0 ; index < ISP_MAX_OUTPUT_PORT_NUM ; index++)
    {
        frameInfo.m_dstFormat[index] = m_dstFormat[index];
        frameInfo.m_dstWidth[index] = m_dstWidth[index];
        frameInfo.m_dstHeight[index] = m_dstHeight[index];
        frameInfo.m_dstYPitch[index] = m_dstYPitch[index];
        frameInfo.m_dstUVPitch[index] = m_dstUVPitch[index];
        frameInfo.m_outXStart[index] = m_outXStart[index];
        frameInfo.m_outYStart[index] = m_outYStart[index];
        frameInfo.m_dstMemAddr[index][0] = m_dstMemAddr[index][0];
        frameInfo.m_dstMemAddr[index][1] = m_dstMemAddr[index][1];
        frameInfo.m_dstMemAddr[index][2] = m_dstMemAddr[index][2];
        frameInfo.m_dstMemSize[index][0] = m_dstMemSize[index][0];
        frameInfo.m_dstMemSize[index][1] = m_dstMemSize[index][1];
        frameInfo.m_dstMemSize[index][2] = m_dstMemSize[index][2];
        frameInfo.m_dstSecMode[index] = m_dstSecMode[index];
    }

    return frameInfo;
}

void DpCommandRecorder::setRegDstNum(int32_t regNum)
{
    m_regDstNum = regNum;
}

int32_t DpCommandRecorder::getRegDstNum()
{
    return m_regDstNum;
}


void DpCommandRecorder::markRecord(BlockType type)
{
    switch (m_blockType)
    {
        case FRAME_BLOCK:
        case TILE_BLOCK:
            m_tileSize[m_tileID]     = m_backLength - m_tileOffset[m_tileID];
            m_blockSize[m_blockType] = m_backLength - m_blockOffset[m_blockType];
            break;
        case EXT_FRAME_BLOCK:
            m_blockSize[m_blockType] = m_extLength - m_blockOffset[m_blockType];
            break;
        default:
            break;
    }

    if (type != m_blockType)
    {
        switch (type)
        {
            case FRAME_BLOCK:
                //m_tileID = 0;
                m_tileOffset[0] = m_backLength;
                m_tileSize[0]   = 0;
                /* fall through */
            case TILE_BLOCK:
                m_blockOffset[type] = m_backLength; // Store current offset
                m_blockSize[type]   = 0;
                break;
            case EXT_FRAME_BLOCK:
                // For additional frame setting at last
                if (NULL == m_pExtBuffer)
                {
                    m_extBufferSize = BASE_COMMAND_BLOCK_SIZE;
                    m_pExtBuffer    = (op_meta *)malloc(m_extBufferSize);
                }

                m_pCurCommand = m_pExtBuffer;
                m_extLength   = 0;

                m_blockOffset[type] = 0;
                m_blockSize[type]   = 0;
                assert(0);
                break;
            default:
                break;
        }

        m_blockType = type;
    }
}


void DpCommandRecorder::markRecord(BlockType type, uint32_t x, uint32_t y)
{
    if (TILE_BLOCK == type)
    {
        uint32_t preTileID = m_tileID;
        //DPLOGD("DpCommandRecorder: %d, %#x\n", preTileID, m_tileOffset[preTileID]);

        m_tileID = toTileID(x, y);
        assert(m_tileID < MAX_TILE_NUM);

        m_tileSize[preTileID]  = m_backLength - m_tileOffset[preTileID];
        m_tileOffset[m_tileID] = m_backLength;
        m_tileSize[m_tileID]   = 0;

        m_maxX = MAX(m_maxX, x);
        m_maxY = MAX(m_maxY, y);
    }
}


void DpCommandRecorder::reorder()
{
    uint32_t i, j;
    op_meta *tempPtr = NULL;

    if (m_dumpOrder)
    {
        // use front buffer as temp buffer
        m_frontBufferSize = m_backBufferSize;
        m_pFrontBuffer    = (op_meta *)realloc(m_pFrontBuffer, m_backBufferSize);

        m_frontLength = 0;
        tempPtr       = m_pFrontBuffer;

        // copy frame setting
        //DPLOGD("frame: %#x, %#x\n", m_tileOffset[0], m_tileSize[0]);
        if (tempPtr)
        {
            memcpy(tempPtr, (uint8_t *)m_pBackBuffer + m_tileOffset[0], m_tileSize[0]);
            m_frontLength += m_tileSize[0];
            tempPtr = (op_meta *)((uint8_t *)m_pFrontBuffer + m_frontLength);
        }
        DPLOGD("Only apply in JPEG direclink mode m_dumpOrder: %#x\n", m_dumpOrder);
    }

    if ((m_dumpOrder & (TILE_ORDER_Y_FIRST | TILE_ORDER_RIGHT_TO_LEFT)) == (TILE_ORDER_Y_FIRST | TILE_ORDER_RIGHT_TO_LEFT)) // 270
    {
        for (i = m_maxX; i >= 0; i--)
        {
            for (j = 0; j <= m_maxY; j++)
            {
                tempPtr = copyTile(tempPtr, i, j);
            }
        }
    }
    else if ((m_dumpOrder & (TILE_ORDER_BOTTOM_TO_TOP | TILE_ORDER_RIGHT_TO_LEFT)) == (TILE_ORDER_BOTTOM_TO_TOP | TILE_ORDER_RIGHT_TO_LEFT)) // 180
    {
        for (j = 0; j <= m_maxY; j++)
        {
            for (i = m_maxX; i >= 0 ; i--)
            {
                tempPtr = copyTile(tempPtr, i, j);
            }
        }
    }
    else if ((m_dumpOrder & (TILE_ORDER_Y_FIRST | TILE_ORDER_BOTTOM_TO_TOP)) == (TILE_ORDER_Y_FIRST | TILE_ORDER_BOTTOM_TO_TOP)) // 90
    {
        for (i = 0; i <= m_maxX; i++)
        {
            for (j = 0; j <= m_maxY; j++)
            {
                tempPtr = copyTile(tempPtr, i, j);
            }
        }
    }
    else if ((m_dumpOrder & (TILE_ORDER_RIGHT_TO_LEFT)) == (TILE_ORDER_RIGHT_TO_LEFT)) // Left to Right
    {
        for (j = 0; j <= m_maxY; j++)
        {
            for (i = m_maxX; i >= 0; i--)
            {
                tempPtr = copyTile(tempPtr, i, j);
            }
        }
    }
    else
    {
        // do nothing
    }

    if (m_dumpOrder)
    {
        m_pCurCommand = tempPtr;

        // swap buffer
        tempPtr        = m_pBackBuffer;
        m_pBackBuffer  = m_pFrontBuffer;
        m_pFrontBuffer = tempPtr;

        m_backLength  = m_frontLength;
        m_frontLength = 0;
    }
}


op_meta *DpCommandRecorder::copyTile(op_meta *dst, uint32_t x, uint32_t y)
{
    uint32_t i;
    uint32_t indexTile;

    indexTile = toTileID(x, y);
    //DPLOGD("x:%d y:%d, %x, %d, %d\n", x, y, indexTile, m_tileOffset[indexTile], m_tileSize[indexTile]);

    for (i = 0; i < m_backLabelCount; i++)
    {
        if (m_pBackLabels[i].type == TILE_BLOCK && m_pBackLabels[i].tileID == indexTile)
        {
            m_pBackLabels[i].offset += m_frontLength - m_tileOffset[indexTile];
        }
    }

    memcpy((void *)dst, (uint8_t *)m_pBackBuffer + m_tileOffset[indexTile], m_tileSize[indexTile]);
    m_frontLength += m_tileSize[indexTile];
    return (op_meta *)((uint8_t *)m_pFrontBuffer + m_frontLength);
}


void DpCommandRecorder::dupRecord(BlockType type)
{
    // Ring buffer mode would have this to duplicate the frame setting
    if (m_frontBlockOffset[type] < 0)
    {
        assert(0);
        return;
    }

    switch (m_blockType)
    {
        case FRAME_BLOCK:
        case TILE_BLOCK:
            m_tileSize[m_tileID]     = m_backLength - m_tileOffset[m_tileID];
            m_blockSize[m_blockType] = m_backLength - m_blockOffset[m_blockType];
            break;
        case EXT_FRAME_BLOCK:
            m_blockSize[m_blockType] = m_extLength - m_blockOffset[m_blockType];
            break;
        default:
            break;
    }

    if ((m_backLength + m_frontBlockSize[type]) >= m_backBufferSize)
    {
        m_backBufferSize += ((m_frontBlockSize[type] / UNIT_COMMAND_BLOCK_SIZE) + 1) * UNIT_COMMAND_BLOCK_SIZE;
        m_pBackBuffer = (op_meta *)realloc(m_pBackBuffer, m_backBufferSize);

        m_pCurCommand = (op_meta *)((uint8_t *)m_pBackBuffer + m_backLength);
    }

    switch (type)
    {
        case FRAME_BLOCK:
            //m_tileID = 0;
            m_tileOffset[0] = m_backLength;
            m_tileSize[0]   = m_frontBlockSize[type];
            /* fall through */
        case TILE_BLOCK:
            m_blockOffset[type] = m_backLength;
            m_blockSize[type]   = m_frontBlockSize[type];

            // copy from front
            memcpy(m_pCurCommand, (uint8_t*)m_pFrontBuffer + m_frontBlockOffset[type], m_frontBlockSize[type]);
            m_backLength += m_frontBlockSize[type];
            m_pCurCommand = (op_meta *)((uint8_t *)m_pBackBuffer + m_backLength);

            for (uint32_t index = 0; index < m_frontLabelCount; index++)
            {
                if (m_pFrontLabels[index].type == type)
                {
                    dupLabel(index);
                }
            }
            break;
        default:
            assert(0);
            break;
    }

    m_blockType = type;
}


void DpCommandRecorder::initRecord()
{
    resetRecord();

    memset(m_blockOffset, -1, sizeof(m_blockOffset));
    memset(m_blockSize, 0, sizeof(m_blockSize));
    memset(m_tileOffset, -1, sizeof(m_tileOffset));
    memset(m_tileSize, 0, sizeof(m_tileSize));

    memset(m_secureInfo, 0x0, sizeof(m_secureInfo));
    memset(m_secureAddrMD, 0x0, sizeof(m_secureAddrMD));
#if defined(CMDQ_V3) || defined(CMDQ_K414)
    memset(&m_ispMeta, 0x0, sizeof(m_ispMeta));
#endif
}


void DpCommandRecorder::stopRecord()
{
    markRecord(NONE_BLOCK);
}


void DpCommandRecorder::swapRecord()
{
    // swap buffer
    op_meta *tempPtr = m_pFrontBuffer;
    uint32_t tempSize = m_frontBufferSize;

    m_pFrontBuffer    = m_pBackBuffer;
    m_frontBufferSize = m_backBufferSize;

    m_pBackBuffer     = tempPtr;
    m_backBufferSize  = tempSize;

    m_frontLength = m_backLength;
    m_backLength  = 0;

    memcpy(m_frontBlockOffset, m_blockOffset, sizeof(m_frontBlockOffset));
    memcpy(m_frontBlockSize, m_blockSize, sizeof(m_frontBlockSize));

    // swap label
    LabelInfo *temp = m_pFrontLabels;
    m_pFrontLabels  = m_pBackLabels;
    m_pBackLabels   = temp;

    m_frontLabelCount = m_backLabelCount;
    m_backLabelCount  = 0;

    if (m_frontLabelCount > 0) // copy on used
    {
        memcpy(m_frontLabelIndex, m_labelIndex, sizeof(m_frontLabelIndex));
    }

    //flushRecord();
}


void DpCommandRecorder::resetRecord()
{
    m_blockType = NONE_BLOCK;
    m_tileID = 0;

    m_addrListLength = 0;

    if (NULL == m_pBackBuffer)
    {
        m_backBufferSize = BASE_COMMAND_BLOCK_SIZE;
        m_pBackBuffer    = (op_meta *)malloc(m_backBufferSize);
    }

    m_nextLabel = 0;

    m_maxX = 0;
    m_maxY = 0;

    m_pCurCommand = m_pBackBuffer;
    m_backLength  = 0;

    m_secureInfoCount = 0;
    m_hasIspSecMeta = false;

#ifdef CONFIG_FOR_SOURCE_PQ
    if (STREAM_COLOR_BITBLT == getScenario())
    {
        DPLOGI("DpCommandRecorder: COLOR_BITBLT reset Record !!!!\n");
        memset(m_blockSize, 0, sizeof(m_blockSize));
    }
#endif
    m_MMpath_info_size = 0;
}

int16_t DpCommandRecorder::getEngine(uint32_t addr)
{
    static DpEngineAddrBase DpEngineAddrs[] =
    {
            {ENGBASE_MMSYS_CONFIG, MMSYS_CONFIG_BASE},
            {ENGBASE_MMSYS_MUTEX, MMSYS_MUTEX_BASE},
            {ENGBASE_MDP_RDMA0, MDP_RDMA0_BASE},
#ifdef USE_ENGBASE_MDP_RDMA1
            {ENGBASE_MDP_RDMA1, MDP_RDMA1_BASE},
#endif
#ifdef USE_ENGBASE_MDP_RDMA2
            {ENGBASE_MDP_RDMA2, MDP_RDMA2_BASE},
#endif
#ifdef USE_ENGBASE_MDP_RDMA3
            {ENGBASE_MDP_RDMA3, MDP_RDMA3_BASE},
#endif
#ifdef USE_ENGBASE_MDP_WROT0
            {ENGBASE_MDP_WROT0, MDP_WROT0_BASE},
#endif
#ifdef USE_ENGBASE_MDP_WROT1
            {ENGBASE_MDP_WROT1, MDP_WROT1_BASE},
#endif
#ifdef USE_ENGBASE_MDP_WROT2
            {ENGBASE_MDP_WROT2, MDP_WROT2_BASE},
#endif
#ifdef USE_ENGBASE_MDP_WROT3
            {ENGBASE_MDP_WROT3, MDP_WROT3_BASE},
#endif
#ifdef USE_ENGBASE_MDP_AAL0
            {ENGBASE_MDP_AAL0, MDP_AAL0_BASE},
#endif
#ifdef USE_ENGBASE_MDP_AAL1
            {ENGBASE_MDP_AAL1, MDP_AAL1_BASE},
#endif
#ifdef USE_ENGBASE_MDP_AAL2
            {ENGBASE_MDP_AAL2, MDP_AAL2_BASE},
#endif
#ifdef USE_ENGBASE_MDP_AAL3
            {ENGBASE_MDP_AAL3, MDP_AAL3_BASE},
#endif
#ifdef USE_ENGBASE_MDP_RSZ0
            {ENGBASE_MDP_RSZ0, MDP_RSZ0_BASE},
#endif
#ifdef USE_ENGBASE_MDP_RSZ1
            {ENGBASE_MDP_RSZ1, MDP_RSZ1_BASE},
#endif
#ifdef USE_ENGBASE_MDP_RSZ2
            {ENGBASE_MDP_RSZ2, MDP_RSZ2_BASE},
#endif
#ifdef USE_ENGBASE_MDP_RSZ3
            {ENGBASE_MDP_RSZ3, MDP_RSZ3_BASE},
#endif
#ifdef USE_ENGBASE_MDP_HDR0
            {ENGBASE_MDP_HDR0, MDP_HDR0_BASE},
#endif
#ifdef USE_ENGBASE_MDP_HDR1
            {ENGBASE_MDP_HDR1, MDP_HDR1_BASE},
#endif
#ifdef USE_ENGBASE_MDP_TDSHP0
            {ENGBASE_MDP_TDSHP0, MDP_TDSHP0_BASE},
#endif
#ifdef USE_ENGBASE_MDP_TDSHP1
            {ENGBASE_MDP_TDSHP1, MDP_TDSHP1_BASE},
#endif
#ifdef USE_ENGBASE_MDP_TDSHP2
            {ENGBASE_MDP_TDSHP2, MDP_TDSHP2_BASE},
#endif
#ifdef USE_ENGBASE_MDP_TDSHP3
            {ENGBASE_MDP_TDSHP3, MDP_TDSHP3_BASE},
#endif
#ifdef USE_ENGBASE_MDP_FG0
            {ENGBASE_MDP_FG0, MDP_FG0_BASE},
#endif
#ifdef USE_ENGBASE_MDP_FG1
            {ENGBASE_MDP_FG1, MDP_FG1_BASE},
#endif
#ifdef USE_ENGBASE_MDP_TCC0
            {ENGBASE_MDP_TCC0, MDP_TCC0_BASE},
#endif
#ifdef USE_ENGBASE_MDP_TCC1
            {ENGBASE_MDP_TCC1, MDP_TCC1_BASE},
#endif
#ifdef USE_ENGBASE_MDP_TCC2
            {ENGBASE_MDP_TCC2, MDP_TCC2_BASE},
#endif
#ifdef USE_ENGBASE_MDP_TCC3
            {ENGBASE_MDP_TCC3, MDP_TCC3_BASE},
#endif
#ifdef USE_ENGBASE_MDP_COLOR0
            {ENGBASE_MDP_COLOR0, MDP_COLOR0_BASE},
#endif
#ifdef USE_ENGBASE_MDP_COLOR1
            {ENGBASE_MDP_COLOR1, MDP_COLOR1_BASE},
#endif
#ifdef USE_ENGBASE_MDP_CCORR0
            {ENGBASE_MDP_CCORR0, MDP_CCORR0_BASE},
#endif
#ifdef USE_ENGBASE_ISP_MSFDL
            {ENGBASE_ISP_MSFDL, ISP_MSFDL_BASE},
#endif
#ifdef USE_ENGBASE_ISP_MSS
            {ENGBASE_ISP_MSS, ISP_MSS_BASE},
#endif
#ifdef USE_ENGBASE_ISP_MSS_B
            {ENGBASE_ISP_MSS_B, ISP_MSS_B_BASE},
#endif
#ifdef USE_ENGBASE_ISP_MFB
            {ENGBASE_ISP_MFB, ISP_MFB_BASE},
#endif
#ifdef USE_ENGBASE_ISP_MFB_B
            {ENGBASE_ISP_MFB_B, ISP_MFB_B_BASE},
#endif
#ifdef USE_ENGBASE_ISP_DIP1
            {ENGBASE_ISP_DIP1, ISP_DIP1_BASE},
#endif
#ifdef USE_ENGBASE_ISP_DIP2
            {ENGBASE_ISP_DIP2, ISP_DIP2_BASE},
#endif
#ifdef USE_ENGBASE_ISP_DIP_A
            {ENGBASE_ISP_DIP_A, ISP_DIP_A_BASE},
#endif
#ifdef USE_ENGBASE_ISP_DIP_A0
            {ENGBASE_ISP_DIP_A0, ISP_DIP_A0_BASE},
#endif
#ifdef USE_ENGBASE_ISP_DIP_A1
            {ENGBASE_ISP_DIP_A1, ISP_DIP_A1_BASE},
#endif
#ifdef USE_ENGBASE_ISP_DIP_A7
            {ENGBASE_ISP_DIP_A7, ISP_DIP_A7_BASE},
#endif
#ifdef USE_ENGBASE_IMGSYS
            {ENGBASE_IMGSYS, IMGSYS_BASE},
#endif
#ifdef USE_ENGBASE_IMGSYS2_CONFIG
            {ENGBASE_IMGSYS2_CONFIG, IMGSYS2_CONFIG_BASE},
#endif
#ifdef USE_ENGBASE_ISPSYS
            {ENGBASE_ISPSYS, ISPSYS_BASE},
#endif
#ifdef USE_ENGBASE_ISP_DIP_B0
            {ENGBASE_ISP_DIP_B0, ISP_DIP_B0_BASE},
#endif
#ifdef USE_ENGBASE_ISP_DIP_B1
            {ENGBASE_ISP_DIP_B1, ISP_DIP_B1_BASE},
#endif
#ifdef USE_ENGBASE_ISP_DIP_B7
            {ENGBASE_ISP_DIP_B7, ISP_DIP_B7_BASE},
#endif
#ifdef USE_ENGBASE_ISP_CAM_0
            {ENGBASE_ISP_CAM_0, ISP_CAM_0_BASE},
#endif
#ifdef USE_ENGBASE_ISP_CAM_3
            {ENGBASE_ISP_CAM_3, ISP_CAM_3_BASE},
#endif
#ifdef USE_ENGBASE_ISP_CAM_A
            {ENGBASE_ISP_CAM_A, ISP_CAM_A_BASE},
#endif
#ifdef USE_ENGBASE_ISP_CAM_D
            {ENGBASE_ISP_CAM_D, ISP_CAM_D_BASE},
#endif
#ifdef USE_ENGBASE_ISP_CAMSYS
            {ENGBASE_ISP_CAMSYS, ISP_CAMSYS_BASE},
#endif
#ifdef USE_ENGBASE_ISP_CAM
            {ENGBASE_ISP_CAM, ISP_CAM_BASE},
#endif
#ifdef USE_ENGBASE_ISP_CAM_DMA
            {ENGBASE_ISP_CAM_DMA, ISP_CAM_DMA_BASE},
#endif
#ifdef USE_ENGBASE_ISP_WPE_A
            {ENGBASE_ISP_WPE_A, ISP_WPE_A_BASE},
#endif
#ifdef USE_ENGBASE_ISP_WPE_B
            {ENGBASE_ISP_WPE_B, ISP_WPE_B_BASE},
#endif
#ifdef USE_ENGBASE_JPGENC,
            {ENGBASE_JPGENC, JPGENC_BASE},
#endif
#ifdef USE_ENGBASE_JPGDEC,
            {ENGBASE_JPGDEC, JPGDEC_BASE},
#endif
#ifdef USE_ENGBASE_MDP_WDMA
            {ENGBASE_MDP_WDMA, MDP_WDMA_BASE},
#endif
#ifdef USE_ENGBASE_MMSYS_CMDQ
            {ENGBASE_MMSYS_CMDQ, MMSYS_CMDQ_BASE},
#endif
    };

    uint32_t base = addr & 0xfffff000;
    uint32_t i;

    if (m_lastBase == base)
        return m_lastEngine;

    for (i = 0; i < sizeof(DpEngineAddrs) / sizeof(DpEngineAddrs[0]); i++)
    {
        if (base == DpEngineAddrs[i].base) {
            m_lastEngine = DpEngineAddrs[i].engine;
            m_lastBase = base;
            return m_lastEngine;
        }
    }

    m_lastBase = 0;
    DPLOGE("DpCommandRecorder::getEngine fail to match addr:%#x\n", addr);
    return ~0;
}

uint32_t DpCommandRecorder::translateAddr(uint32_t addr)
{
    return (getEngine(addr) << 16) | (addr & 0xfff);
}

void DpCommandRecorder::appendMeta(uint16_t op, uint32_t addr, uint32_t value, uint32_t mask)
{
    if (!m_pLastCommand && m_backLength + UNIT_COMMAND_BLOCK_SIZE >= m_backBufferSize)
    {
        int32_t offset = (unsigned long)m_pLabelCommand - (unsigned long)m_pBackBuffer;

        m_backBufferSize += UNIT_COMMAND_BLOCK_SIZE;
        m_pBackBuffer = (op_meta *)realloc(m_pBackBuffer, m_backBufferSize);

        if (m_pLabelCommand)
        {
            m_pLabelCommand = (op_meta *)((uint8_t*)m_pBackBuffer + offset);
        }
        m_pCurCommand = (op_meta *)((uint8_t*)m_pBackBuffer + m_backLength);
    }

    m_pCurCommand->op = op;
    if (op == CMDQ_MOP_WRITE)
    {
        m_pCurCommand->engine = getEngine(addr);
        m_pCurCommand->offset = addr & 0xfff;
        m_pCurCommand->value = value;
        m_pCurCommand->mask = mask;

        if (m_secureMode && setSecureMetaData(addr, value))
        {
            m_pCurCommand->op = CMDQ_MOP_WRITE_SEC;
            m_pCurCommand->mask = m_addrListLength - 1;
        }
    }
    else if (op == CMDQ_MOP_READ || op == CMDQ_MOP_POLL)
    {
        m_pCurCommand->engine = getEngine(addr);
        m_pCurCommand->offset = addr & 0xfff;
        m_pCurCommand->value = value;
        m_pCurCommand->mask = mask;

        if (m_secureMode)
            DPLOGD("DpCommandRecorder: read in secure addr:%#x\n", addr);
    }
    else if (op == CMDQ_MOP_WRITE_FROM_REG)
    {
        m_pCurCommand->engine = getEngine(addr);
        m_pCurCommand->offset = addr & 0xfff;
        m_pCurCommand->from_engine = getEngine(value);
        m_pCurCommand->from_offset = value & 0xfff;
        m_pCurCommand->mask = mask;
    }
    else if (op == CMDQ_MOP_WAIT || op == CMDQ_MOP_WAIT_NO_CLEAR || op == CMDQ_MOP_CLEAR ||
        op == CMDQ_MOP_SET || op == CMDQ_MOP_ACQUIRE)
    {
        m_pCurCommand->engine = 0;
        m_pCurCommand->event = addr;
        m_pCurCommand->value = 0;
        m_pCurCommand->mask = 0;
    }
    else
    {
        m_pCurCommand->engine = 0;
        m_pCurCommand->offset = 0;
        m_pCurCommand->op = CMDQ_MOP_NOP;
        m_pCurCommand->mask = 0;
    }
    m_pCurCommand++;

    if (!m_pLastCommand)
        m_backLength = (unsigned long)m_pCurCommand - (unsigned long)m_pBackBuffer;
}

void DpCommandRecorder::beginLabel()
{
    if (m_pCurLabel != NULL)
    {
        return;
    }

    if (m_pLabelCommand != NULL)
    {
        DPLOGE("DpCommandRecorder: unexpected label begin\n");
        return;
    }

    m_pLabelCommand = m_pCurCommand;
}


int32_t DpCommandRecorder::endLabel()
{
    if (m_pCurLabel != NULL)
    {
        return m_pCurLabel->label;
    }

    if (m_pLabelCommand == NULL)
    {
        DPLOGE("DpCommandRecorder: possible double or dummy label end\n");
        return -1;
    }

    LabelInfo *pLabel = addLabel();

    m_pLabelCommand = NULL;

    if (pLabel == NULL)
    {
        DPLOGE("DpCommandRecorder: out of label\n");
        return -1;
    }
    return pLabel->label;
}


DpCommandRecorder::LabelInfo *DpCommandRecorder::addLabel()
{
    int32_t label;
    LabelInfo *pLabel = NULL;

    if (NULL == m_pBackLabels)
    {
        m_pBackLabels = (LabelInfo*)malloc(MAX_TILE_NUM * sizeof(LabelInfo));
        m_pFrontLabels = (LabelInfo*)malloc(MAX_TILE_NUM * sizeof(LabelInfo));
    }
    if (m_backLabelCount == 0) // reset on first use
    {
        memset(m_labelIndex, -1, sizeof(m_labelIndex));
        if (m_frontLabelCount == 0)
        {
            memset(m_frontLabelIndex, -1, sizeof(m_frontLabelIndex));
        }
    }

    for (label = m_nextLabel; label < MAX_TILE_NUM; label++)
    {
        if (m_frontLabelIndex[label] == -1)
        {
            break;
        }
    }
    if (label == MAX_TILE_NUM) // out of label
    {
        m_nextLabel = MAX_TILE_NUM;
        return NULL;
    }

    m_nextLabel = label + 1;

    if (m_pBackLabels != NULL)
    {
        pLabel = &m_pBackLabels[m_backLabelCount];
        pLabel->label  = label;
        pLabel->type   = m_blockType;
        pLabel->tileID = m_tileID;
        pLabel->offset = (unsigned long)m_pLabelCommand - (unsigned long)m_pBackBuffer - m_blockOffset[m_blockType];
        pLabel->length = (unsigned long)m_pCurCommand - (unsigned long)m_pLabelCommand;
    }
    else
    {
        return NULL;
    }
    m_labelIndex[label] = m_backLabelCount;
    m_backLabelCount ++;

    return pLabel;
}


DpCommandRecorder::LabelInfo *DpCommandRecorder::findLabel(int32_t label)
{
    if (label < 0 || label >= MAX_TILE_NUM)
    {
        return NULL;
    }

    int32_t index = m_labelIndex[label];
    if (index < 0 || (uint32_t)index >= m_backLabelCount || m_pBackLabels[index].label != label)
    {
        return NULL;
    }

    return &m_pBackLabels[index];
}


void DpCommandRecorder::dupLabel(uint32_t index)
{
    int32_t label = m_pFrontLabels[index].label;

    if (m_backLabelCount == MAX_TILE_NUM) // out of label
    {
        DPLOGE("DpCommandRecorder: out of label\n");
        return;
    }
    if (m_backLabelCount == 0) // reset on first use
    {
        memset(m_labelIndex, -1, sizeof(m_labelIndex));
    }

    m_pBackLabels[m_backLabelCount] = m_pFrontLabels[index];
    m_labelIndex[label] = m_backLabelCount;
    m_backLabelCount ++;
}


void DpCommandRecorder::beginOverwrite(int32_t label)
{
    if (m_pCurLabel != NULL)
    {
        DPLOGE("DpCommandRecorder: unexpected overwrite begin\n");
        return;
    }

    m_pCurLabel = findLabel(label);
    if (m_pCurLabel == NULL)
    {
        DPLOGE("DpCommandRecorder: unexpected overwrite label %d\n", label);
        return;
    }

    m_pLabelCommand = (op_meta *)((uint8_t *)m_pBackBuffer + m_blockOffset[m_pCurLabel->type] + m_pCurLabel->offset);
    m_pLastCommand = m_pCurCommand;
    m_pCurCommand = m_pLabelCommand;
}


void DpCommandRecorder::endOverwrite()
{
    if (m_pCurLabel == NULL)
    {
        DPLOGE("DpCommandRecorder: possible double or dummy overwrite end\n");
        return;
    }

    if ((unsigned long)m_pCurCommand - (unsigned long)m_pLabelCommand != m_pCurLabel->length)
    {
        DPLOGE("DpCommandRecorder: overwrite command length mismatch\n");
    }

    m_pCurCommand = m_pLastCommand;
    m_pLastCommand = NULL;
    m_pLabelCommand = NULL;

    m_pCurLabel = NULL;
}


void DpCommandRecorder::flushRecord()
{
    //m_blockOffset[TILE_BLOCK] = -1;
}


void DpCommandRecorder::dumpRecord()
{
    FILE *pFile;
#if CONFIG_FOR_OS_ANDROID
    pFile = fopen("/data/command.bin", "wb");
#else
    pFile = fopen("./out/command.bin", "wb");
#endif
    if (NULL != pFile)
    {
        fwrite(m_pFrontBuffer, m_frontLength, 1, pFile);
        fclose(pFile);
    }

    DPLOGD("command block: start %p, size %#x, engine %#010x\n", getBlockBaseSW(), getBlockSize(), getEngineFlag());
    DPLOGD("command block: back %p size %#x, frame %#x %#x, tile %#x %#x\n", m_pBackBuffer, m_backLength,
        m_blockOffset[FRAME_BLOCK], m_blockSize[FRAME_BLOCK], m_blockOffset[TILE_BLOCK], m_blockSize[TILE_BLOCK]);
}


void DpCommandRecorder::dumpRegister(uint64_t pqSessionId)
{
#ifndef BASIC_PACKAGE
#ifdef DEBUG_DUMP_REG
    PQSession* pPQSession = PQSessionManager::getInstance()->getPQSession(pqSessionId);
    DpPqParam param;
    FILE *pFile;
    char name[256] = {0};
    int32_t DumpReg = DpDriver::getInstance()->getEnableDumpRegister();

    if (pPQSession != NULL)
    {
        pPQSession->getPQParam(&param);

        if (param.scenario == MEDIA_ISP_PREVIEW || param.scenario == MEDIA_ISP_CAPTURE)
        {
            if (param.u.isp.enableDump ||
                (((DumpReg == DUMP_ISP_PRV && param.scenario == MEDIA_ISP_PREVIEW) ||
                (DumpReg == DUMP_ISP_CAP && param.scenario == MEDIA_ISP_CAPTURE) ||
                (DumpReg == DUMP_ISP_PRV_CAP)) &&
                param.u.isp.timestamp != 0xFFFFFFFF))
                sprintf(name, "/data/vendor/camera_dump/%09d-%04d-%04d-MDP-%s-%d-%s.mdp",
                    param.u.isp.timestamp,
                    param.u.isp.requestNo,
                    param.u.isp.frameNo,
                    (param.scenario == MEDIA_ISP_PREVIEW) ? "Prv" : "CAP",
                    param.u.isp.lensId,
                    param.u.isp.userString);
            else
                return;
        }
        else
        {
            return;
        }

#if CONFIG_FOR_OS_ANDROID
        pFile = fopen(name, "ab");

        if (NULL != pFile)
        {
            fwrite(m_pFrontBuffer, m_frontLength, 1, pFile);
            fclose(pFile);
            DPLOGD("Dump register to %s\n", name);
        }
        else
        {
            DPLOGD("Open %s failed\n", name);
        }
#endif
    }
#endif
#endif
}


uint32_t* DpCommandRecorder::getReadbackRegs(uint32_t& numReadRegs)
{
    numReadRegs = m_numReadbackRegs;
    return m_readbackRegs;
}


uint32_t* DpCommandRecorder::getReadbackValues(uint32_t& numValues)
{
    numValues = m_numReadbackRegs;
    return m_readbackValues;
}


uint32_t DpCommandRecorder::getISPDebugDumpRegs()
{
    return m_ISPDebugDumpRegs;
}


char* DpCommandRecorder::getFrameInfoToCMDQ()
{
    return m_frameInfoToCMDQ;
}

bool DpCommandRecorder::getSyncMode()
{
    DP_STATUS_ENUM status;
    bool           syncMode;

    status = m_pPath->getSyncMode(&syncMode);
#if !CONFIG_FOR_VERIFY_FPGA
    assert(DP_STATUS_RETURN_SUCCESS == status);
#endif

    return syncMode;
}

uint32_t* DpCommandRecorder::getReadbackPABuffer(uint32_t& readbackPABufferIndex)
{
    return m_pPath->getReadbackPABuffer(readbackPABufferIndex);
}

DP_STATUS_ENUM DpCommandRecorder::setNumReadbackPABuffer(uint32_t numReadbackPABuffer, uint32_t readbackEngineID)
{
    return m_pPath->setNumReadbackPABuffer(numReadbackPABuffer, readbackEngineID);
}

mdp_pmqos* DpCommandRecorder::getMdpPmqos() {
    int32_t enableMMpath = DpDriver::getInstance()->getEnableMMpath();
    int32_t enableMet = DpDriver::getInstance()->getEnableMet();

    m_pPath->getPMQOS(&m_mdp_pmqos);

    DPLOGI("m_mdp_pmqos.mdp_total_datasize %d\n", m_mdp_pmqos.mdp_total_datasize);
    DPLOGI("m_mdp_pmqos.mdp_total_pixel %d\n", m_mdp_pmqos.mdp_total_pixel);

    DPLOGI("m_mdp_pmqos.isp_total_datasize %d\n", m_mdp_pmqos.isp_total_datasize);
    DPLOGI("m_mdp_pmqos.isp_total_pixel %d\n", m_mdp_pmqos.isp_total_pixel);

    DPLOGI("m_mdp_pmqos.tv_sec %lld ms\n", m_mdp_pmqos.tv_sec);
    DPLOGI("m_mdp_pmqos.tv_usec %lld ms\n", m_mdp_pmqos.tv_usec);

    m_mdp_pmqos.ispMetString = 0;
    m_mdp_pmqos.ispMetStringSize = 0;
    m_mdp_pmqos.mdpMetString = 0;
    m_mdp_pmqos.mdpMetStringSize = 0;
    DPLOGI("m_mdp_pmqos size %d\n", sizeof(m_mdp_pmqos));
    if (m_ISP_MET_size> 0 && enableMet) {
        m_mdp_pmqos.ispMetString = (unsigned long)m_ISP_MET_info;
        m_mdp_pmqos.ispMetStringSize = m_ISP_MET_size;
        DPLOGI("DpCommandRecorder::isp met log (%d) %s", m_ISP_MET_size, m_ISP_MET_info);
    }
    if (m_MET_size > 0 && enableMet) {
        m_mdp_pmqos.mdpMetString = (unsigned long)m_MET_info;
        m_mdp_pmqos.mdpMetStringSize = m_MET_size+1;
        DPLOGI("DpCommandRecorder::mdp met log (%d) %s", m_MET_size, m_MET_info);
    }

    m_mdp_pmqos.mdpMMpathString = 0;
    m_mdp_pmqos.mdpMMpathStringSize = 0;
    DPLOGI("m_mdp_pmqos size %d\n", sizeof(m_mdp_pmqos));
    if (m_MMpath_info_size > 0 && enableMMpath) {
        m_mdp_pmqos.mdpMMpathString = (unsigned long)m_MMpath_info;
        m_mdp_pmqos.mdpMMpathStringSize = m_MMpath_info_size+1;
        DPLOGI("DpCommandRecorder::mdp MMpath log (%d) %s", m_MMpath_info_size, m_MMpath_info);
    }

    return &m_mdp_pmqos;
}

const char *format2string (uint32_t value)
{
    const char *name;

    switch (value)
    {
        case DP_COLOR_FULLG8:
            name = "DP_COLOR_FULLG8";
            break;
        case DP_COLOR_FULLG10:
            name = "DP_COLOR_FULLG10";
            break;
        case DP_COLOR_FULLG12:
            name = "DP_COLOR_FULLG12";
            break;
        case DP_COLOR_FULLG14:
            name = "DP_COLOR_FULLG14";
            break;
        case DP_COLOR_BAYER8:
            name = "DP_COLOR_BAYER8";
            break;
        case DP_COLOR_BAYER10:
            name = "DP_COLOR_BAYER10";
            break;
        case DP_COLOR_BAYER12:
            name = "DP_COLOR_BAYER12";
            break;
        case DP_COLOR_RGB565:
            name = "DP_COLOR_RGB565";
            break;
        case DP_COLOR_BGR565:
            name = "DP_COLOR_BGR565";
            break;
        case DP_COLOR_RGB888:
            name = "DP_COLOR_RGB888";
            break;
        case DP_COLOR_RGBA8888:
            name = "DP_COLOR_RGBA8888";
            break;
        case DP_COLOR_BGRA8888:
            name = "DP_COLOR_BGRA8888";
            break;
        case DP_COLOR_ARGB8888:
            name = "DP_COLOR_ARGB8888";
            break;
        case DP_COLOR_ABGR8888:
            name = "DP_COLOR_ABGR8888";
            break;
        case DP_COLOR_UYVY:
            name = "DP_COLOR_UYVY";
            break;
        case DP_COLOR_VYUY:
            name = "DP_COLOR_VYUY";
            break;
        case DP_COLOR_YUYV:
            name = "DP_COLOR_YUYV";
            break;
        case DP_COLOR_YVYU:
            name = "DP_COLOR_YVYU";
            break;
        case DP_COLOR_I420:
            name = "DP_COLOR_I420";
            break;
        case DP_COLOR_YV12:
            name = "DP_COLOR_YV12";
            break;
        case DP_COLOR_I422:
            name = "DP_COLOR_I422";
            break;
        case DP_COLOR_YV16:
            name = "DP_COLOR_YV16";
            break;
        case DP_COLOR_I444:
            name = "DP_COLOR_I444";
            break;
        case DP_COLOR_YV24:
            name = "DP_COLOR_YV24";
            break;
        case DP_COLOR_NV12:
            name = "DP_COLOR_NV12";
            break;
        case DP_COLOR_NV21:
            name = "DP_COLOR_NV21";
            break;
        case DP_COLOR_NV16:
            name = "DP_COLOR_NV16";
            break;
        case DP_COLOR_NV61:
            name = "DP_COLOR_NV61";
            break;
        case DP_COLOR_NV24:
            name = "DP_COLOR_NV24";
            break;
        case DP_COLOR_NV42:
            name = "DP_COLOR_NV42";
            break;
        case DP_COLOR_420_BLKP:
            name = "DP_COLOR_420_BLKP";
            break;
        case DP_COLOR_420_BLKI:
            name = "DP_COLOR_420_BLKI";
            break;
        case DP_COLOR_422_BLKP:
            name = "DP_COLOR_422_BLKP";
            break;
        case DP_COLOR_IYU2:
            name = "DP_COLOR_IYU2";
            break;
        case DP_COLOR_YUV444:
            name = "DP_COLOR_YUV444";
            break;
        case DP_COLOR_GREY:
            name = "DP_COLOR_GREY";
            break;
        default:
            name = "Unknown or Others";
            break;
    }

    return name;
}
void DpCommandRecorder::addMMpathLog(const char *name, uint32_t value, DP_MMPATH_ENUM type)
{
    char *start = NULL;
    int32_t enableMMpath = DpDriver::getInstance()->getEnableMMpath();

    if (!enableMMpath)
        return;
    if (name == NULL)
        return;
    if (m_MMpath_info_size + 30 > MAX_MMPATH_INFO)
    {
        DPLOGE("MM path buffer size too small");
        return;
    }

    switch (type)
    {
        case DP_MMPATH_OTHER:
        {
            m_MMpath_info_size += sprintf(m_MMpath_info + m_MMpath_info_size, m_MMpath_info_size ? ",%s=%d" : "%s=%d", name, value);
            break;
        }
        case DP_MMPATH_TILEOVERHEAD:
        {
            float temp = 0;
            temp = 1 + ((float)value) / 1000;
            m_MMpath_info_size += sprintf(m_MMpath_info + m_MMpath_info_size, m_MMpath_info_size ? ",%s=%f" : "%s=%f", name, temp);
            break;
        }
        case DP_MMPATH_ADDR:
        {
            m_MMpath_info_size += sprintf(m_MMpath_info + m_MMpath_info_size, m_MMpath_info_size ? ",%s=0x%x" : "%s=0x%x", name, value);
            break;
        }
        case DP_MMPATH_FORMAT:
        {
            const char *format = format2string(value);
            int32_t en_10b = 0;
            int32_t bpp = 0;
            char temp[512];
            sprintf(temp, "%s_fmt", name);
            m_MMpath_info_size += sprintf(m_MMpath_info + m_MMpath_info_size, m_MMpath_info_size ? ",%s=%s" : "%s=%s", temp, format);

            /*bpp*/
            bpp = DP_COLOR_BITS_PER_PIXEL(value);

            if (DP_COLOR_GET_BLOCK_MODE(value))
                bpp = bpp >> 5;

            bpp = bpp + (bpp >> DP_COLOR_GET_H_SUBSAMPLE(value) >> DP_COLOR_GET_V_SUBSAMPLE(value)) * (DP_COLOR_GET_PLANE_COUNT(value) - 1 + DP_COLOR_IS_UV_COPLANE(value));
            sprintf(temp, "%s_bpp", name);
            m_MMpath_info_size += sprintf(m_MMpath_info + m_MMpath_info_size, ",%s=%f", temp, bpp/8.0);


            /*10b_en & 8b_en*/
            en_10b = value & 0xC0000000 ? 1 : 0;
            sprintf(temp, "%s_10b_en", name);
            m_MMpath_info_size += sprintf(m_MMpath_info + m_MMpath_info_size, ",%s=%d", temp, en_10b);
            sprintf(temp, "%s_8b_en", name);
            m_MMpath_info_size += sprintf(m_MMpath_info + m_MMpath_info_size, ",%s=%d", temp, en_10b ? 0 : 1);
            break;
        }
        default:
            DPLOGE("DpCommandRecorder: MM path unknown log type\n");
            break;
    }
}

void DpCommandRecorder::setSubtaskId(uint32_t id){
    const char *subtask = "subtask";
    int32_t enableMet = DpDriver::getInstance()->getEnableMet();

    if (!enableMet)
        return;

    if (strstr(m_MET_info, subtask)!= NULL){
        sprintf(strstr(m_MET_info, subtask), "subtask=%d", id%10);
        if (m_MET_size > strlen(m_MET_info)){
            m_MET_info[strlen(m_MET_info)] = ',';
        }
    }
    else if (m_MET_size + 20 > MAX_MET_INFO)
    {
        DPLOGE("MET buffer size too small");
        return;
    }
    else{
       m_MET_size += sprintf(m_MET_info + m_MET_size, ",subtask=%d", id);
    }
}

void DpCommandRecorder::addMetLog(const char *name, uint32_t value)
{
    int32_t enableMet = DpDriver::getInstance()->getEnableMet();

    if (!enableMet)
        return;
    if (name == NULL)
        return;
    if (m_MET_size + 30 > MAX_MET_INFO)
    {
        DPLOGE("MET buffer size too small");
        return;
    }
    if (m_MET_size > 0)
       m_MET_size += sprintf(m_MET_info + m_MET_size, ",%s=%d", name, value);
    else
       m_MET_size += sprintf(m_MET_info + m_MET_size, "%s=%d", name, value);

}

void DpCommandRecorder::addIspMetLog(char *log, uint32_t size)
{
    DPLOGI("DpCommandRecorder::addIspMetLog (%p) (%d)", log, size);
    if (log == NULL || size == 0)
        return ;
    m_ISP_MET_info = log;
    m_ISP_MET_size = size;
}

#if defined(CMDQ_V3) || defined(CMDQ_K414)
bool DpCommandRecorder::hasIspSecMeta()
{
    return m_hasIspSecMeta;
}

void DpCommandRecorder::addIspSecureMeta(cmdqSecIspMeta ispMeta)
{
    m_hasIspSecMeta = true;
    m_ispMeta = ispMeta;
}

cmdqSecIspMeta DpCommandRecorder::getSecIspMeta()
{
    return m_ispMeta;
}
#endif
