#ifndef __ISP_DIPRINGBUFFER_H__
#define __ISP_DIPRINGBUFFER_H__
//
#include <vector>
#include <map>
#include <list>
using namespace std;

#include <utils/threads.h>
#include <cutils/atomic.h>
#include "isp_datatypes.h"
#include "imem_drv.h"
#include "mdp_mgr.h"
#include "isp_drv_dip_platform.h"


typedef enum DipDmaSmxioEnum
{
    DIP_DMA_SMXI_1 = 0,
    DIP_DMA_SMXI_2,
    DIP_DMA_SMXI_3,
    DIP_DMA_SMXI_4,
    DIP_DMA_SMXIO_MAX
}DIP_DMA_SMXIO_ENUM;

typedef enum DipWBStatus
{
    DIP_WB_FREE = 0,
    DIP_WB_USED,
    DIP_WB_TOTAL,
}DIP_WB_STATUS;


struct DipWorkingBuffer
{
    MUINT32* m_pIspVirRegAddr_va;
    MUINT32* m_pIspVirRegAddr_pa;
    ISP_DRV_CQ_CMD_DESC_STRUCT*         m_pIspDescript_vir;
    MUINT32*                            m_pIspDescript_phy;
    unsigned long tpipeTablePa;     //skip tpipe will change the variable when current frame is the same as previous frame.
    unsigned int *tpipeTableVa;    //skip tpipe will change the variable when current frame is the same as previous frame.
    //
    MDPMGR_CFG_STRUCT *m_pMdpCfg;
    DIP_WB_STATUS m_WBStatus;
    unsigned int m_BufIdx;
    char* m_pMetLogBuf;
    MUINT32 m_IspVirRegAddr_oft;
    MUINT32 m_IspDescript_oft;
    MUINT32 tpipeTableOft;
};


class DipWBList
{
public:
    unsigned int wIdx;
    unsigned int rIdx;
    unsigned int iWBSize;
    std::vector<DipWorkingBuffer*> mDipWBList;
    //
    DipWBList()
        : wIdx(0)
        , rIdx(0)
        , iWBSize(0)
        , mDipWBList()
    {
    }
    ~DipWBList()
    {
    }
};



class DipRingBuffer
{
    protected:
        DipRingBuffer();
        virtual ~DipRingBuffer();
    //
    public:
        static DipRingBuffer*  createInstance(ISP_DIP_MODULE_IDX module);
        virtual void    destroyInstance(void) = 0;
        virtual bool   init(MUINT32 secTag) = 0;
        virtual bool   uninit() = 0;

        virtual bool   getTpipeBuf(MUINTPTR& virtAddr, MUINTPTR& phyAddr) = 0;
        virtual bool   getCmdqBuf(MUINTPTR& virtAddr, MUINTPTR& phyAddr) = 0;

        virtual DipWorkingBuffer*   AcquireDipWorkingBuffer(E_ISP_DIP_CQ CmdqId) = 0;
        virtual bool   ReleaseDipWorkingBuffer(DipWorkingBuffer* pDipWBuf, E_ISP_DIP_CQ CmdqId) = 0;

        virtual bool   checkDipWorkingBufferAvaiable(E_ISP_DIP_CQ CmdqId) = 0;
        virtual DipWorkingBuffer*   getDipWorkingBufferbyIdx(E_ISP_DIP_CQ CmdqId, unsigned int BufIdx) =0;
        virtual bool skipTpipeCalculatebyIdx(E_ISP_DIP_CQ CmdqId, unsigned int& frameflag, unsigned int& RefRingBufIdx, DipWorkingBuffer* pCurrFrameDipWBuf)= 0;
        virtual bool   getSecureBufHandle(MUINT32& cqHandle, MUINT32& tpipeHandle) = 0;
        virtual bool   getSecureBufSize(MUINT32& cqSize, MUINT32& tpipeSize) = 0;
        virtual bool   getCqDescBufSize(MUINT32& cqDescSize) = 0;

    protected:
    private:
    //
    public:
        MINT32          mFd;
        MINT32          mInitCount;
        MINT32          mSecureTag;

};

class DipRingBufferImp : public DipRingBuffer
{
    public:
        DipRingBufferImp();
        virtual ~DipRingBufferImp();
    //
    public:
        static DipRingBuffer*  getInstance(ISP_DIP_MODULE_IDX module);
        virtual void    destroyInstance(void);
        virtual bool   init(MUINT32 secTag);
        virtual bool   uninit();

        virtual bool   getTpipeBuf(MUINTPTR& virtAddr, MUINTPTR& phyAddr);
        virtual bool   getCmdqBuf(MUINTPTR& virtAddr, MUINTPTR& phyAddr);

        virtual DipWorkingBuffer*   AcquireDipWorkingBuffer(E_ISP_DIP_CQ CmdqId);
        virtual bool   ReleaseDipWorkingBuffer(DipWorkingBuffer* pDipWBuf, E_ISP_DIP_CQ CmdqId);

        virtual bool   checkDipWorkingBufferAvaiable(E_ISP_DIP_CQ CmdqId);
        virtual DipWorkingBuffer*   getDipWorkingBufferbyIdx(E_ISP_DIP_CQ CmdqId, unsigned int BufIdx);
        virtual bool skipTpipeCalculatebyIdx(E_ISP_DIP_CQ CmdqId, unsigned int& frameflag, unsigned int& RefRingBufIdx, DipWorkingBuffer* pCurrFrameDipWBuf);
        virtual bool   getSecureBufHandle(MUINT32& cqHandle, MUINT32& tpipeHandle);
        virtual bool   getSecureBufSize(MUINT32& cqSize, MUINT32& tpipeSize);
        virtual bool   getCqDescBufSize(MUINT32& cqDescSize);

    protected:


    private:
        virtual bool   BufferMutexLock();
        virtual bool   BufferMutexUnLock();
        bool m_DipRingBufInit;

        IMemDrv* m_pMemDrv;

        IMEM_BUF_INFO  m_CQBufInfo;
        IMEM_BUF_INFO  m_TpipeBufInfo;

        MUINT32     m_MdpCfgsize;
        MUINTPTR    m_pMdpCfgBuf;

        MUINT32     m_DipCmdqWBsize;
        MUINTPTR    m_pDipCmdqWBAddr;

        pthread_mutex_t mDipCmdQWBMutex;

        DipWBList mDipCmdQRBList[DIP_RING_BUFFER_CQ_SIZE];

        IMEM_BUF_INFO  m_CQSecBufInfo;
        IMEM_BUF_INFO  m_TpipeSecBufInfo;
        MUINT32        m_CqDescSize;
};

////////////////////////////////////////////////////////////////////////////////


#endif /*__ISP_DIPRINGBUFFER_H__*/




