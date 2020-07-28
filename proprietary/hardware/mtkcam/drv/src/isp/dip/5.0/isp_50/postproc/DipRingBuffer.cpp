#define LOG_TAG "iio/ifunc_dip"

#include "DipRingBuffer.h"
//open syscall
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//mutex
#include <pthread.h>
#include <cutils/properties.h>  // For property_get().
// TODO: hungwen
#include <stdlib.h>

using namespace std;

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#define  DEBUG_ISP_ONLY     MFALSE

DECLARE_DBG_LOG_VARIABLE(DipRingBuf);

// Clear previous define, use our own define.
#undef DIP_RINGBUF_VRB
#undef DIP_RINGBUF_DBG
#undef DIP_RINGBUF_INF
#undef DIP_RINGBUF_WRN
#undef DIP_RINGBUF_ERR
#undef DIP_RINGBUF_AST
#define DIP_RINGBUF_VRB(fmt, arg...)        do { if (DipRingBuf_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define DIP_RINGBUF_DBG(fmt, arg...)        do { if (DipRingBuf_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define DIP_RINGBUF_INF(fmt, arg...)        do { if (DipRingBuf_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define DIP_RINGBUF_WRN(fmt, arg...)        do { if (DipRingBuf_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define DIP_RINGBUF_ERR(fmt, arg...)        do { if (DipRingBuf_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define DIP_RINGBUF_AST(cond, fmt, arg...)  do { if (DipRingBuf_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

//-----------------------------------------------------------------------------
DipRingBuffer* DipRingBuffer::createInstance(DIP_HW_MODULE module)
{
    return DipRingBufferImp::getInstance(module);
}

static DipRingBufferImp singleton[DIP_HW_MAX];

DipRingBuffer* DipRingBufferImp::getInstance(DIP_HW_MODULE module)
{
    DIP_RINGBUF_DBG("singleton[0x%08x].", &singleton[module]);

    return &singleton[module];
}

void DipRingBufferImp::destroyInstance(void)
{

}

DipRingBuffer::DipRingBuffer()
    :mFd(0), mInitCount(0),mSecureTag(0)
{

}

DipRingBuffer::~DipRingBuffer()
{

}


DipRingBufferImp::DipRingBufferImp()
    : mDipCmdQWBMutex(PTHREAD_MUTEX_INITIALIZER)
{
    m_MdpCfgsize = 0;
    m_pMdpCfgBuf = 0;
    m_DipCmdqWBsize = 0;
    m_pDipCmdqWBAddr = 0;
    m_bInitAllocSmxBuf = 0;
    m_DipRingBufInit = false;
    m_MetLogBufsize = 0;
    m_pMegLogBuf = 0;
    for (int i=0;i<DIP_RING_BUFFER_CQ_SIZE;i++)
    {
        mDipCmdQRBList[i].mDipWBList.clear();
        mDipCmdQRBList[i].wIdx = 0;
        mDipCmdQRBList[i].rIdx = 0;
        mDipCmdQRBList[i].iWBSize = 0;
    }
    m_DipRingBufInit = false;
    for (int i=0;i<DIP_RING_BUFFER_CQ_SIZE;i++)
    {
        for (int j=0;j<DIP_DMA_SMXIO_MAX;j++)
        {
            m_DmaSMXioVAddr[i][j] = 0;
            m_DmzSMXioPAddr[i][j] = 0;
            m_DmaSMXioOft[i][j] = 0;
        }    
    }
    m_pMemDrv = NULL;
    m_CqDescSize = 0;
}

DipRingBufferImp::~DipRingBufferImp()
{

}

bool DipRingBufferImp::init(bool AllocSmxBuf, MUINT32 secTag)
{
    MUINT32 cqDesAllocSize;
    MUINTPTR _tmpVirAddr,_tmpPhyAddr;
    MUINT32 bufIndex;
    MUINT32 bufOffset;
    MUINTPTR   tempMdrCfgBuf;
    MUINTPTR   tempMetLogBuf;
    MUINTPTR   tempDipCmdqWBBuf;
    MUINT32 offset = 0;
    MUINT32 workingbufidx;
    MUINT32 MdpCfgoffset = 0;
    MUINT32 DipCmdqWBoffset = 0;
    MUINT32 smxiobufOffset;
    MUINT32 Metlogoffset = 0;
    
    DBG_LOG_CONFIG(imageio, DipRingBuf);

    // create & init imem
    if(this->mInitCount > 0)
    {
        android_atomic_inc(&this->mInitCount);
        DIP_RINGBUF_INF("dip ring buffer mInitCount(%d)", this->mInitCount);
        return true;
    }

    this->mSecureTag = secTag;
    DIP_RINGBUF_INF("secure Tag(%d, %d)", this->mSecureTag, secTag);

    this->m_pMemDrv = DipIMemDrv::createInstance();
    if(this->m_pMemDrv->init() == MFALSE){
        DIP_RINGBUF_ERR("dip ring buf imem init fail\n");
        return false;
    }

    
    /*============================================
     alloc cq buffer
    =============================================*/
    cqDesAllocSize = (GET_MAX_CQ_DESCRIPTOR_SIZE()/ISP_DIP_CQ_DUMMY_BUFFER+1)*ISP_DIP_CQ_DUMMY_BUFFER;
    m_CqDescSize = cqDesAllocSize;
    m_CQBufInfo.size = (((cqDesAllocSize+DIP_REG_RANGE+0x3) & (~0x3)))*(DIP_RING_BUFFER_CQ_SIZE*MAX_CMDQ_RING_BUFFER_SIZE_NUM); //4byte alignment,round up
    m_CQBufInfo.useNoncache = 1;
    if ( this->m_pMemDrv->allocVirtBuf(&m_CQBufInfo) ){
        DIP_RINGBUF_ERR("[Error] cq buf alloc fail!!");
        return false;
    }
    
    if ( this->m_pMemDrv->mapPhyAddr(&m_CQBufInfo) ) {
        DIP_RINGBUF_ERR("[ERROR] cq buf mapaddr fail!!");
        return false;
    }
    DIP_RINGBUF_DBG("[CQBufInfo]size(0x%x),Va(0x%lx),Pa(0x%lx)",m_CQBufInfo.size,(unsigned long)m_CQBufInfo.virtAddr,(unsigned long)m_CQBufInfo.phyAddr);

     //va/pa 0 init
     memset((MUINT8*)m_CQBufInfo.virtAddr,0x0,m_CQBufInfo.size);
    
    /*============================================
     alloc cq secure buffer
    =============================================*/
    #if 0
    if (this->mSecureTag == 1)
    {
        m_CQSecBufInfo.size = m_CQBufInfo.size;
        if ( this->m_pMemDrv->allocSecureBuf(&m_CQSecBufInfo) ){
            DIP_RINGBUF_ERR("[Error] cq secure buf alloc fail!!");
            return false;
        }
        m_CQSecBufInfo.secHandle = (MUINT32)m_CQSecBufInfo.phyAddr;
        DIP_RINGBUF_INF("[CQSecBufInfo]size(0x%x),Handle(0x%lx)",m_CQSecBufInfo.size,(unsigned long)m_CQSecBufInfo.secHandle);
    }
    #endif
    /*============================================
     alloc tpipe buffer
    =============================================*/
    m_TpipeBufInfo.size = DIP_RING_BUFFER_CQ_SIZE*MAX_CMDQ_RING_BUFFER_SIZE_NUM*MAX_ISP_TILE_TDR_HEX_NO+(15+1);
    m_TpipeBufInfo.useNoncache = 1; //alloc non-cacheable mem for tpipe.
    if (m_pMemDrv->allocVirtBuf(&m_TpipeBufInfo) ) {
        DIP_RINGBUF_ERR("[ERROR] tpipe buf alloc fail!!,size(0x%x)",m_TpipeBufInfo.size);
        return false;
    }
    if ( m_pMemDrv->mapPhyAddr(&m_TpipeBufInfo) ) {
        DIP_RINGBUF_ERR("[ERROR]tpipe buf mapaddr fail!!");
        return false;
    }
    DIP_RINGBUF_DBG("[TPIPE]size(0x%x),Va(0x%lx),Pa(0x%lx)",m_TpipeBufInfo.size,(unsigned long)m_TpipeBufInfo.virtAddr,(unsigned long)m_TpipeBufInfo.phyAddr);

    /*============================================
     alloc tpipe secure buffer
    =============================================*/
    #if 0
    if (this->mSecureTag == 1)
    {
        m_TpipeSecBufInfo.size = m_TpipeBufInfo.size;
        if ( this->m_pMemDrv->allocSecureBuf(&m_TpipeSecBufInfo) ){
            DIP_RINGBUF_ERR("[Error] tpipe secure buf alloc fail!!");
            return false;
        }
        m_TpipeSecBufInfo.secHandle = (MUINT32)m_TpipeSecBufInfo.phyAddr;
        DIP_RINGBUF_INF("[TpipeSecBufInfo]size(0x%x),Handle(0x%lx)",m_TpipeSecBufInfo.size,(unsigned long)m_TpipeSecBufInfo.secHandle);
    }
    #endif
    /*============================================
     alloc smx buffer
    =============================================*/
    m_bInitAllocSmxBuf = AllocSmxBuf;
    if (m_bInitAllocSmxBuf == true){
        m_SMXBufInfo.size = (MAX_SMX1_BUF_SIZE+MAX_SMX2_BUF_SIZE+MAX_SMX3_BUF_SIZE+MAX_SMX4_BUF_SIZE)*DIP_RING_BUFFER_CQ_SIZE; //Alignment 
        m_SMXBufInfo.useNoncache = 1;
        if ( this->m_pMemDrv->allocVirtBuf(&m_SMXBufInfo) ){
            DIP_RINGBUF_ERR("[Error] smx buf alloc fail!!");
            return false;
        }
        
        if ( this->m_pMemDrv->mapPhyAddr(&m_SMXBufInfo) ) {
            DIP_RINGBUF_ERR("[ERROR] smx buf mapaddr fail!!");
            return false;
        }
        DIP_RINGBUF_DBG("[SMXBufInfo]size(0x%x),Va(0x%lx),Pa(0x%lx)",m_SMXBufInfo.size,(unsigned long)m_SMXBufInfo.virtAddr,(unsigned long)m_SMXBufInfo.phyAddr);
        smxiobufOffset = 0;
        for (int i=0;i<DIP_RING_BUFFER_CQ_SIZE;i++)
        {
            for (int j=0;j<DIP_DMA_SMXIO_MAX;j++)
            {
                //alignment chk
                if(( (_tmpVirAddr=m_SMXBufInfo.virtAddr+smxiobufOffset) % 0x4) != 0 ){
                    DIP_RINGBUF_WRN("[Warning]iMem virRegAddr not 4 bytes alignment(0x%lx), round up",(unsigned long)(m_SMXBufInfo.virtAddr+smxiobufOffset));
                    _tmpVirAddr = (m_SMXBufInfo.virtAddr+smxiobufOffset + 0x3) & (~0x3);
                }
                if(( (_tmpPhyAddr=m_SMXBufInfo.phyAddr+smxiobufOffset) % 0x4) != 0 ){
                    DIP_RINGBUF_WRN("[Warning]iMem phyRegAddr not 4 bytes alignment(0x%lx), round up",(unsigned long)(m_SMXBufInfo.phyAddr+smxiobufOffset));
                    _tmpPhyAddr = (m_SMXBufInfo.phyAddr+smxiobufOffset + 0x3) & (~0x3);
                }  
                m_DmaSMXioVAddr[i][j] = _tmpVirAddr;
                m_DmzSMXioPAddr[i][j] = _tmpPhyAddr;
                m_DmaSMXioOft[i][j] = smxiobufOffset;	// For secure path
                DIP_RINGBUF_DBG("DmaSmxioId:%d, smxiobufOffset:0x%x, SmxVaddr:0x%x, SmxPaddr:0x%x!!", j, smxiobufOffset, _tmpVirAddr, _tmpPhyAddr);
                switch (j)
                {
                    case DIP_DMA_SMXI_1:
                        smxiobufOffset = smxiobufOffset + MAX_SMX1_BUF_SIZE;
                        break;
                    case DIP_DMA_SMXI_2:
                        smxiobufOffset = smxiobufOffset + MAX_SMX2_BUF_SIZE;
                        break;
                    case DIP_DMA_SMXI_3:
                        smxiobufOffset = smxiobufOffset + MAX_SMX3_BUF_SIZE;
                        break;
                    case DIP_DMA_SMXI_4:
                        smxiobufOffset = smxiobufOffset + MAX_SMX4_BUF_SIZE;
                        break;
                    default:
                        DIP_RINGBUF_ERR("no such smxio index:%d", j);
                        break;
                }
            }
        }
    }

    /*============================================
     alloc smx/tpipe/cq secure buffer
    =============================================*/
    if (this->mSecureTag == 1)
    {
        m_SMXSecBufInfo.size = m_SMXBufInfo.size;
        if ( this->m_pMemDrv->allocSecureBuf(&m_SMXSecBufInfo) ){
            DIP_RINGBUF_ERR("[Error] SMX secure buf alloc fail!!");
            return false;
        }
        m_SMXSecBufInfo.secHandle = (MUINT32)m_SMXSecBufInfo.phyAddr;
		if ( this->m_pMemDrv->setBufferdbgName("DIP-SMXSecureBuffer", &m_SMXSecBufInfo) ){
            DIP_RINGBUF_WRN("[Warn] SMX secure buf set debug name fail!!");
            //return false;
        }

        m_TpipeSecBufInfo.size = m_TpipeBufInfo.size;
        if ( this->m_pMemDrv->allocSecureBuf(&m_TpipeSecBufInfo) ){
            DIP_RINGBUF_ERR("[Error] tpipe secure buf alloc fail!!");
            return false;
        }
        m_TpipeSecBufInfo.secHandle = (MUINT32)m_TpipeSecBufInfo.phyAddr;
		if ( this->m_pMemDrv->setBufferdbgName("DIP-TpipeSecureBuffer", &m_TpipeSecBufInfo) ){
            DIP_RINGBUF_WRN("[Warn] tpipe secure buf set debug name fail!!");
            //return false;
        }

        m_CQSecBufInfo.size = m_CQBufInfo.size;
        if ( this->m_pMemDrv->allocSecureBuf(&m_CQSecBufInfo) ){
            DIP_RINGBUF_ERR("[Error] cq secure buf alloc fail!!");
            return false;
        }
        m_CQSecBufInfo.secHandle = (MUINT32)m_CQSecBufInfo.phyAddr;
		if ( this->m_pMemDrv->setBufferdbgName("DIP-CQSecureBuffer", &m_CQSecBufInfo) ){
            DIP_RINGBUF_WRN("[Warn] cq secure buf set debug name fail!!");
            //return false;
        }

        DIP_RINGBUF_INF("[DipSecBufInfo] CQ: size(0x%x),Handle(0x%lx), Tpipe: size(0x%x),Handle(0x%lx), SMX: size(0x%x),Handle(0x%lx)",m_CQSecBufInfo.size,(unsigned long)m_CQSecBufInfo.secHandle,m_TpipeSecBufInfo.size,(unsigned long)m_TpipeSecBufInfo.secHandle,m_SMXSecBufInfo.size,(unsigned long)m_SMXSecBufInfo.secHandle);
    }

    //MET log Buffer
    m_MetLogBufsize = DIP_RING_BUFFER_CQ_SIZE*MAX_CMDQ_RING_BUFFER_SIZE_NUM*MAX_METLOG_SIZE;
    m_pMegLogBuf = (MUINTPTR)malloc(m_MetLogBufsize);
    if ( m_pMegLogBuf == 0) {
        DIP_RINGBUF_ERR("[ERROR]malloc m_pMegLogBuf fail!!");
        return false;
    }
    memset((MUINT8*)m_pMegLogBuf,'\0',m_MetLogBufsize);

    tempMetLogBuf = ((m_pMegLogBuf+15)&(~15));

    m_MdpCfgsize = DIP_RING_BUFFER_CQ_SIZE*MAX_CMDQ_RING_BUFFER_SIZE_NUM*sizeof(MDPMGR_CFG_STRUCT)+(15+1);
    m_pMdpCfgBuf = (MUINTPTR)malloc(m_MdpCfgsize);
    if ( m_pMdpCfgBuf == 0) {
        DIP_RINGBUF_ERR("[ERROR]malloc MdpCfgBuf fail!!");
        return false;
    }
    memset((MUINT8*)m_pMdpCfgBuf,0x0,m_MdpCfgsize);

    tempMdrCfgBuf = ((m_pMdpCfgBuf+15)&(~15));
    
    m_DipCmdqWBsize = DIP_RING_BUFFER_CQ_SIZE*MAX_CMDQ_RING_BUFFER_SIZE_NUM*sizeof(DipWorkingBuffer)+(15+1);
    m_pDipCmdqWBAddr = (MUINTPTR)malloc(m_DipCmdqWBsize);
    if ( m_pDipCmdqWBAddr == 0) {
        DIP_RINGBUF_ERR("[ERROR]malloc DipCmdqWBAddr fail!!");
        return false;
    }
    memset((MUINT8*)m_pDipCmdqWBAddr,0x0,m_DipCmdqWBsize);

    tempDipCmdqWBBuf = ((m_pDipCmdqWBAddr+15)&(~15));
    
    // set tpipe address and virtual isp address to working buffer
    DipWorkingBuffer* dip_workingbuf = (DipWorkingBuffer*)tempDipCmdqWBBuf;
    for (int i=0;i<DIP_RING_BUFFER_CQ_SIZE;i++)
    {
        workingbufidx = 0;
        mDipCmdQRBList[i].mDipWBList.clear();
        mDipCmdQRBList[i].wIdx = 0;
        mDipCmdQRBList[i].rIdx = 0;
        mDipCmdQRBList[i].iWBSize = 0;
        for (int j=0;j<MAX_CMDQ_RING_BUFFER_SIZE_NUM;j++)
        {
            bufIndex = (i * MAX_CMDQ_RING_BUFFER_SIZE_NUM) + j;
            bufOffset = ((cqDesAllocSize+DIP_REG_RANGE+0x3) & (~0x3))*bufIndex;
            //
            //alignment chk
            if(( (_tmpVirAddr=m_CQBufInfo.virtAddr+bufOffset) % 0x4) != 0 ){
                DIP_RINGBUF_WRN("[Warning]iMem virRegAddr not 4 bytes alignment(0x%lx), round up",(unsigned long)(m_CQBufInfo.virtAddr+bufOffset));
                _tmpVirAddr = (m_CQBufInfo.virtAddr+bufOffset + 0x3) & (~0x3);
            }
            if(( (_tmpPhyAddr=m_CQBufInfo.phyAddr+bufOffset) % 0x4) != 0 ){
                DIP_RINGBUF_WRN("[Warning]iMem phyRegAddr not 4 bytes alignment(0x%lx), round up",(unsigned long)(m_CQBufInfo.phyAddr+bufOffset));
                _tmpPhyAddr = (m_CQBufInfo.phyAddr+bufOffset + 0x3) & (~0x3);
            }    
            //Cmdq Des/Vir ISP Buffer
            dip_workingbuf->m_pIspDescript_vir = (ISP_DRV_CQ_CMD_DESC_STRUCT*)_tmpVirAddr;
            dip_workingbuf->m_pIspDescript_phy = (MUINT32*)_tmpPhyAddr;
            dip_workingbuf->m_IspDescript_oft = bufOffset;
            
            dip_workingbuf->m_pIspVirRegAddr_va = (MUINT32*)(dip_workingbuf->m_pIspDescript_vir+(cqDesAllocSize/sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)));
            dip_workingbuf->m_pIspVirRegAddr_pa = (MUINT32*)(dip_workingbuf->m_pIspDescript_phy+(cqDesAllocSize/sizeof(MUINT32)));
            dip_workingbuf->m_IspVirRegAddr_oft = bufOffset + cqDesAllocSize;
    
            //Tpipe Buffer
            dip_workingbuf->tpipeTableVa = (unsigned int*)(m_TpipeBufInfo.virtAddr + offset);
            dip_workingbuf->tpipeTablePa = m_TpipeBufInfo.phyAddr + offset;
            dip_workingbuf->tpipeTableOft = offset;
            offset += MAX_ISP_TILE_TDR_HEX_NO;
    
            //MdpCfg
            dip_workingbuf->m_pMdpCfg = (MDPMGR_CFG_STRUCT*)((MUINTPTR)tempMdrCfgBuf + MdpCfgoffset);
            dip_workingbuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa = dip_workingbuf->tpipeTableVa;
            dip_workingbuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTablePa = dip_workingbuf->tpipeTablePa;
            MdpCfgoffset += sizeof(MDPMGR_CFG_STRUCT);

            //LogBuf
            dip_workingbuf->m_pMetLogBuf = (char*)((MUINTPTR)tempMetLogBuf + Metlogoffset);
            Metlogoffset += MAX_METLOG_SIZE;
    
            //initiate cq descriptor buffer
            for(int m=0;m<(DIP_A_MODULE_MAX - 1);m++){
                dip_workingbuf->m_pIspDescript_vir[m].u.cmd = ISP_DRV_CQ_DUMMY_TOKEN;
                dip_workingbuf->m_pIspDescript_vir[m].v_reg_addr = 0;
            }
            dip_workingbuf->m_pIspDescript_vir[DIP_A_END_].u.cmd = ISP_DRV_CQ_END_TOKEN;
            dip_workingbuf->m_pIspDescript_vir[DIP_A_END_].v_reg_addr = 0;
            dip_workingbuf->m_WBStatus = DIP_WB_FREE;
            dip_workingbuf->m_BufIdx = workingbufidx;

            DIP_RINGBUF_DBG("workingbufidx:%d, dip_workingbuf(0x%lx), tpipePA(0x%lx),tpipeVA(0x%lx),tpipeOFT(0x%lx), desPA(0x%lx),desVA(0x%lx),desOFT(0x%lx), virISPPA(0x%lx),virISPVA(0x%lx),virISPOFT(0x%lx), MdpCfgAddr(0x%lx)",workingbufidx, (unsigned long)dip_workingbuf, (unsigned long)dip_workingbuf->tpipeTablePa, (unsigned long)dip_workingbuf->tpipeTableVa, (unsigned long)dip_workingbuf->tpipeTableOft
                ,dip_workingbuf->m_pIspDescript_phy, dip_workingbuf->m_pIspDescript_vir, (unsigned long)dip_workingbuf->m_IspDescript_oft, dip_workingbuf->m_pIspVirRegAddr_pa, dip_workingbuf->m_pIspVirRegAddr_va, (unsigned long)dip_workingbuf->m_IspVirRegAddr_oft, (unsigned long)dip_workingbuf->m_pMdpCfg);

            workingbufidx++;
    
            mDipCmdQRBList[i].mDipWBList.push_back(dip_workingbuf);  //list for frame package enqueued from user using DIP
    
            //Dip Command Working Buffer
            DipCmdqWBoffset += sizeof(DipWorkingBuffer);
            dip_workingbuf = (DipWorkingBuffer*)((MUINTPTR)tempDipCmdqWBBuf + DipCmdqWBoffset);
    
        }
        mDipCmdQRBList[i].iWBSize = mDipCmdQRBList[i].mDipWBList.size();
        DIP_RINGBUF_DBG("idx(%d),size(%d)", i, mDipCmdQRBList[i].iWBSize);
    }

    android_atomic_inc(&this->mInitCount);
    DIP_RINGBUF_INF("dip ring buffer init done!!mInitCount(%d)", this->mInitCount);

    return true;
}


bool DipRingBufferImp::uninit()
{
    //
    if(mInitCount > 0)
    {
        android_atomic_dec(&mInitCount);
        if(mInitCount != 0)
        {
            goto DipRingBufferImp_UNINITEXIT;
        }
    }
    else
    {
        DIP_RINGBUF_ERR("DipRingBufferImp init and uninit is not paired!!");
        goto DipRingBufferImp_UNINITEXIT;
    }

    for (int i=0;i<DIP_RING_BUFFER_CQ_SIZE;i++)
    {
        mDipCmdQRBList[i].mDipWBList.clear();
        mDipCmdQRBList[i].wIdx = 0;
        mDipCmdQRBList[i].rIdx = 0;
        mDipCmdQRBList[i].iWBSize = 0;
    }

    if (m_pMegLogBuf != 0)
        free((void*)m_pMegLogBuf);

    if (m_pMdpCfgBuf != 0)
        free((void*)m_pMdpCfgBuf);
    
    if (m_pDipCmdqWBAddr != 0)
        free((void*)m_pDipCmdqWBAddr);
    
    /*============================================
     free smx buffer
    =============================================*/
    if (m_bInitAllocSmxBuf == true){
        if ( m_pMemDrv->unmapPhyAddr(&m_SMXBufInfo) ) {
            DIP_RINGBUF_ERR("[ERROR]m_SMXBufInfo unmap addr fail!!");
            return false;
        }
        
        if ( m_pMemDrv->freeVirtBuf(&m_SMXBufInfo) ) {
            DIP_RINGBUF_ERR("[ERROR]m_SMXBufInfo unmap addr fail!!,size(0x%x)",m_SMXBufInfo.size);
            return false;
        }
        m_SMXBufInfo.size = 0;
        m_bInitAllocSmxBuf = false;
        for (int i=0;i<DIP_RING_BUFFER_CQ_SIZE;i++)
        {
            for (int j=0;j<DIP_DMA_SMXIO_MAX;j++)
            {
                m_DmaSMXioVAddr[i][j] = 0;
                m_DmzSMXioPAddr[i][j] = 0;
            }    
        }

    }
    else
    {
        for (int i=0;i<DIP_RING_BUFFER_CQ_SIZE;i++)
        {
            for (int j=0;j<DIP_DMA_SMXIO_MAX;j++)
            {
                if ((m_DmaSMXioVAddr[i][j]!=0) && (m_DmzSMXioPAddr[i][j]!=0))
                {
                    if ( m_pMemDrv->unmapPhyAddr(&m_DmaSMXioBufInfo[i][j]) ) {
                        DIP_RINGBUF_ERR("[ERROR]m_DmaSMXioBufInfo[%d][%d] unmap addr fail!!", i, j);
                        return false;
                    }
                    
                    if ( m_pMemDrv->freeVirtBuf(&m_DmaSMXioBufInfo[i][j]) ) {
                        DIP_RINGBUF_ERR("[ERROR]m_DmaSMXioBufInfo[%d][%d] unmap addr fail!!,size(0x%x)", i, j, m_SMXBufInfo.size);
                        return false;
                    }
                    m_DmaSMXioBufInfo[i][j].size = 0;
                    m_DmaSMXioVAddr[i][j] = 0;
                    m_DmzSMXioPAddr[i][j] = 0;
                }
            }    
        }
    }

    /*============================================
     free smx secure buffer
    =============================================*/
    if (this->mSecureTag == 1)
    {
        if ( m_pMemDrv->freeSecureBuf(&m_SMXSecBufInfo) ) {
            DIP_RINGBUF_ERR("[ERROR]m_SMXSecBufInfo free fail!!,size(0x%x)",m_SMXSecBufInfo.size);
            return false;
        }
        m_SMXSecBufInfo.size = 0;
    }

    /*============================================
     free tpipe buffer
    =============================================*/
    if ( m_pMemDrv->unmapPhyAddr(&m_TpipeBufInfo) ) {
        DIP_RINGBUF_ERR("[ERROR]m_TpipeBufInfo unmap addr fail!!");
        return false;
    }
    
    if ( m_pMemDrv->freeVirtBuf(&m_TpipeBufInfo) ) {
        DIP_RINGBUF_ERR("[ERROR]m_TpipeBufInfo unmap addr fail!!,size(0x%x)",m_TpipeBufInfo.size);
        return false;
    }
    m_TpipeBufInfo.size = 0;
    
    /*============================================
     free tpipe secure buffer
    =============================================*/
    if (this->mSecureTag == 1)
    {
        if ( m_pMemDrv->freeSecureBuf(&m_TpipeSecBufInfo) ) {
            DIP_RINGBUF_ERR("[ERROR]m_TpipeSecBufInfo free fail!!,size(0x%x)",m_TpipeSecBufInfo.size);
            return false;
        }
        m_TpipeSecBufInfo.size = 0;
    }

    /*============================================
     free cq buffer
    =============================================*/
    if ( m_pMemDrv->unmapPhyAddr(&m_CQBufInfo) ) {
        DIP_RINGBUF_ERR("[ERROR]m_CQBufInfo unmap addr fail!!");
        return false;
    }
    
    if ( m_pMemDrv->freeVirtBuf(&m_CQBufInfo) ) {
        DIP_RINGBUF_ERR("[ERROR]m_CQBufInfo unmap addr fail!!,size(0x%x)",m_CQBufInfo.size);
        return false;
    }
    m_CQBufInfo.size = 0;
    m_TpipeBufInfo.size = 0;
    m_MdpCfgsize = 0;
    m_pMdpCfgBuf = 0;
    m_DipCmdqWBsize = 0;
    m_pDipCmdqWBAddr = 0;
    
    /*============================================
     free cq secure buffer
    =============================================*/
    if (this->mSecureTag == 1)
    {
        if ( m_pMemDrv->freeSecureBuf(&m_CQSecBufInfo) ) {
            DIP_RINGBUF_ERR("[ERROR]m_CQSecBufInfo free fail!!,size(0x%x)",m_CQSecBufInfo.size);
            return false;
        }
        m_CQSecBufInfo.size = 0;
    }

    /*============================================
        imem driver
    =============================================*/
    m_pMemDrv->uninit();
    m_pMemDrv->destroyInstance();
    m_pMemDrv = NULL;

    DIP_RINGBUF_INF("secure Tag(%d)", this->mSecureTag);
    this->mSecureTag = 0;
    DIP_RINGBUF_INF("dip ring buffer un-init done!!");

DipRingBufferImp_UNINITEXIT:

    return true;
}

bool DipRingBufferImp::getTpipeBuf(MUINTPTR& virtAddr, MUINTPTR& phyAddr)
{
    if(this->mInitCount == 0)
    {
        DIP_RINGBUF_INF("Please init dipringbuffer before use getTpipeBuf!!");
        return false;
    }
    virtAddr = m_TpipeBufInfo.virtAddr;
    phyAddr = m_TpipeBufInfo.phyAddr;
    return true;
}

bool DipRingBufferImp::getCmdqBuf(MUINTPTR& virtAddr, MUINTPTR& phyAddr)
{
    if(this->mInitCount == 0)
    {
        DIP_RINGBUF_INF("Please init dipringbuffer before use getCmdqBuf!!");
        return false;
    }
    virtAddr = m_CQBufInfo.virtAddr;
    phyAddr = m_CQBufInfo.phyAddr;
    return true;
}

bool DipRingBufferImp::getSMXIOBuffer(E_ISP_DIP_CQ CmdqId, DIP_DMA_SMXIO_ENUM DmaSmxioId, MUINTPTR& SmxVaddr, MUINTPTR& SmxPaddr, MUINT32& SmxOft)
{
    MUINTPTR _tmpVirAddr,_tmpPhyAddr;
    if (CmdqId >= DIP_RING_BUFFER_CQ_SIZE){
        DIP_RINGBUF_ERR("[ERROR] getSMXIOBuffer:Use the un-supported command queue(0x%x)",CmdqId);        
        return false;
    }

    if (m_bInitAllocSmxBuf == false)
    {
        //Dynamic Allocated Buffer
        switch (DmaSmxioId)
        {
            case DIP_DMA_SMXI_1:
                m_DmaSMXioBufInfo[CmdqId][DmaSmxioId].size = MAX_SMX1_BUF_SIZE;
                break;
            case DIP_DMA_SMXI_2:
                m_DmaSMXioBufInfo[CmdqId][DmaSmxioId].size = MAX_SMX2_BUF_SIZE;
                break;
            case DIP_DMA_SMXI_3:
                m_DmaSMXioBufInfo[CmdqId][DmaSmxioId].size = MAX_SMX3_BUF_SIZE;
                break;
            case DIP_DMA_SMXI_4:
                m_DmaSMXioBufInfo[CmdqId][DmaSmxioId].size = MAX_SMX4_BUF_SIZE;
                break;
            default:
                DIP_RINGBUF_ERR("cmdq:%d, no such smxio index:%d", CmdqId, DmaSmxioId);
                SmxVaddr = 0;
                SmxPaddr = 0;
                return false;
        }
        if (((m_DmaSMXioVAddr[CmdqId][DmaSmxioId]==0) && (m_DmzSMXioPAddr[CmdqId][DmaSmxioId]==0)))
        {
            m_DmaSMXioBufInfo[CmdqId][DmaSmxioId].useNoncache = 1;
            if ( this->m_pMemDrv->allocVirtBuf(&m_DmaSMXioBufInfo[CmdqId][DmaSmxioId]) ){
                DIP_RINGBUF_ERR("[Error] cmdqId(%d), dynamic smx(%d) buf alloc fail!!", CmdqId, DmaSmxioId);
                return false;
            }
            
            if ( this->m_pMemDrv->mapPhyAddr(&m_DmaSMXioBufInfo[CmdqId][DmaSmxioId]) ) {
                DIP_RINGBUF_ERR("[ERROR] cmdqId(%d), dynamic smx(%d) buf mapaddr fail!!", CmdqId, DmaSmxioId);
                return false;
            }
            DIP_RINGBUF_DBG("[SMXBufInfo] cmdqId(%d), smxidx(0x%x),size(0x%x),Va(0x%lx),Pa(0x%lx)",CmdqId, DmaSmxioId, m_DmaSMXioBufInfo[CmdqId][DmaSmxioId].size,(unsigned long)m_DmaSMXioBufInfo[CmdqId][DmaSmxioId].virtAddr,(unsigned long)m_DmaSMXioBufInfo[CmdqId][DmaSmxioId].phyAddr);
            
            //alignment chk
            if(( (_tmpVirAddr=m_DmaSMXioBufInfo[CmdqId][DmaSmxioId].virtAddr) % 0x4) != 0 ){
                DIP_RINGBUF_WRN("[Warning]Dynamic SMXio virRegAddr not 4 bytes alignment(0x%lx), round up",(unsigned long)(m_DmaSMXioBufInfo[CmdqId][DmaSmxioId].virtAddr));
                _tmpVirAddr = (m_DmaSMXioBufInfo[CmdqId][DmaSmxioId].virtAddr + 0x3) & (~0x3);
            }
            if(( (_tmpPhyAddr=m_DmaSMXioBufInfo[CmdqId][DmaSmxioId].phyAddr) % 0x4) != 0 ){
                DIP_RINGBUF_WRN("[Warning]Dynamic SMXio phyRegAddr not 4 bytes alignment(0x%lx), round up",(unsigned long)(m_DmaSMXioBufInfo[CmdqId][DmaSmxioId].phyAddr));
                _tmpPhyAddr = (m_DmaSMXioBufInfo[CmdqId][DmaSmxioId].phyAddr + 0x3) & (~0x3);
            }  
            m_DmaSMXioVAddr[CmdqId][DmaSmxioId] = _tmpVirAddr;
            m_DmzSMXioPAddr[CmdqId][DmaSmxioId] = _tmpPhyAddr;
        }
    }
    SmxVaddr = m_DmaSMXioVAddr[CmdqId][DmaSmxioId];
    SmxPaddr = m_DmzSMXioPAddr[CmdqId][DmaSmxioId];
    SmxOft = m_DmaSMXioOft[CmdqId][DmaSmxioId];
    DIP_RINGBUF_DBG("cmdqId(%d), DmaSmxioId:%d, SmxVaddr:0x%x, SmxPaddr:0x%x, SmxOft:0x%x !!", CmdqId, DmaSmxioId, SmxVaddr, SmxPaddr, SmxOft);
    return true;
}

bool   DipRingBufferImp::getSecureBufHandle(MUINT32& cqHandle, MUINT32& tpipeHandle, MUINT32& smxHandle)
{
    if(this->mInitCount == 0)
    {
        DIP_RINGBUF_INF("Please init dipringbuffer before use getSecureBufHandle!!");
        return false;
    }
    cqHandle = m_CQSecBufInfo.secHandle;
    tpipeHandle = m_TpipeSecBufInfo.secHandle;
    smxHandle = m_SMXSecBufInfo.secHandle;
    return true;
}

bool   DipRingBufferImp::getSecureBufSize(MUINT32& cqSize, MUINT32& tpipeSize, MUINT32& smxSize)
{
    if(this->mInitCount == 0)
    {
        DIP_RINGBUF_INF("Please init dipringbuffer before use getSecureBufSize!!");
        return false;
    }
    cqSize = m_CQSecBufInfo.size;
    tpipeSize = m_TpipeSecBufInfo.size;
    smxSize = m_SMXSecBufInfo.size;
    return true;
}

bool   DipRingBufferImp::getCqDescBufSize(MUINT32& cqDescSize)
{
    if(this->mInitCount == 0)
    {
        DIP_RINGBUF_INF("Please init dipringbuffer before use getCqDescBufSize!!");
        return false;
    }
    cqDescSize = m_CqDescSize;
    return true;
}

bool DipRingBufferImp::BufferMutexLock()
{
    pthread_mutex_lock(&mDipCmdQWBMutex);
    return true;
}
bool DipRingBufferImp::BufferMutexUnLock()
{
    pthread_mutex_unlock(&mDipCmdQWBMutex);
    return true;
}

bool DipRingBufferImp::checkDipWorkingBufferAvaiable(E_ISP_DIP_CQ CmdqId)
{
    this->BufferMutexLock();
    if ( mDipCmdQRBList[CmdqId].iWBSize > 0)
    {
        this->BufferMutexUnLock();
        return true;
    }
    else
    {
        this->BufferMutexUnLock();
        return false;
    }
}

DipWorkingBuffer* DipRingBufferImp::getDipWorkingBufferbyIdx(E_ISP_DIP_CQ CmdqId, unsigned int BufIdx, MBOOL CheckWBStatus)
{
    DipWorkingBuffer* pDipWBuf = NULL;
    if (CmdqId >= DIP_RING_BUFFER_CQ_SIZE){
        DIP_RINGBUF_ERR("[ERROR] getDipWorkingBufferbyIdx:Use the un-supported command queue(0x%x)",CmdqId);        
        return NULL;
    }
    if (mDipCmdQRBList[CmdqId].mDipWBList.size() == 0){
        DIP_RINGBUF_ERR("getDipWorkingBufferbyIdx:the size of command queue(0x%x) is zero!!",CmdqId);        
        return NULL;
    }

    this->BufferMutexLock();
    if ((mDipCmdQRBList[CmdqId].mDipWBList.size()>0) &&
        (BufIdx < mDipCmdQRBList[CmdqId].mDipWBList.size()))
    {
        if (CheckWBStatus == MTRUE)
        {
            if (mDipCmdQRBList[CmdqId].mDipWBList.at(BufIdx)->m_WBStatus == DIP_WB_USED)
            {
                pDipWBuf = (DipWorkingBuffer*)mDipCmdQRBList[CmdqId].mDipWBList.at(BufIdx);
                this->BufferMutexUnLock();
            }
            else    //State Machine Error!!
            {
                pDipWBuf = NULL;
                this->BufferMutexUnLock();
                DIP_RINGBUF_ERR("m_WBStatus of Working Buffer is not DIP_WB_USED!! cmd queue(0x%x), BufIdx(0x%x)!!",CmdqId, BufIdx);		
            }
        }
        else
        {
            pDipWBuf = (DipWorkingBuffer*)mDipCmdQRBList[CmdqId].mDipWBList.at(BufIdx);
            this->BufferMutexUnLock();
        }

    }
    else
    {
        pDipWBuf = NULL;
        this->BufferMutexUnLock();
        DIP_RINGBUF_INF("wrong cmd queue(0x%x), BufIdx(0x%x)",CmdqId, BufIdx);        
    }
    return pDipWBuf;    
}


DipWorkingBuffer* DipRingBufferImp::AcquireDipWorkingBuffer(E_ISP_DIP_CQ CmdqId)
{
    unsigned int DipWBufWRIdx;
    DipWorkingBuffer* pDipWBuf = NULL;
    if (CmdqId >= DIP_RING_BUFFER_CQ_SIZE){
        DIP_RINGBUF_ERR("[ERROR] AcquireDipWorkingBuffer:Use the un-supported command queue(0x%x)",CmdqId);        
        return NULL;
    }
    if (mDipCmdQRBList[CmdqId].mDipWBList.size() == 0){
        DIP_RINGBUF_ERR("AcquireDipWorkingBuffer:the size of command queue(0x%x) is zero!!",CmdqId);        
        return NULL;
    }

    //TODO Handle mutex protection and how to handle no free buffer ?
    this->BufferMutexLock();
    DipWBufWRIdx = mDipCmdQRBList[CmdqId].wIdx;
    DIP_RINGBUF_DBG("CmdqId(%d),DipWBufWRIdx(%d)", CmdqId, DipWBufWRIdx);
    if ((mDipCmdQRBList[CmdqId].mDipWBList.size()>0) && (mDipCmdQRBList[CmdqId].iWBSize > 0) &&
        (DipWBufWRIdx < mDipCmdQRBList[CmdqId].mDipWBList.size()))
    {
        if (mDipCmdQRBList[CmdqId].mDipWBList.at(DipWBufWRIdx)->m_WBStatus == DIP_WB_FREE)
        {
            mDipCmdQRBList[CmdqId].mDipWBList.at(DipWBufWRIdx)->m_WBStatus = DIP_WB_USED;
            pDipWBuf = (DipWorkingBuffer*)mDipCmdQRBList[CmdqId].mDipWBList.at(DipWBufWRIdx);
            mDipCmdQRBList[CmdqId].wIdx++;
            if (mDipCmdQRBList[CmdqId].wIdx == MAX_CMDQ_RING_BUFFER_SIZE_NUM){
                mDipCmdQRBList[CmdqId].wIdx = 0;
            }
            mDipCmdQRBList[CmdqId].iWBSize--;
            this->BufferMutexUnLock();
            DIP_RINGBUF_DBG("CmdqId(%d), mDipCmdQRBList[CmdqId].wIdx(%d), mDipCmdQRBList[CmdqId].iWBSize(%d), pDipWBuf(0x%x)", CmdqId, mDipCmdQRBList[CmdqId].wIdx, mDipCmdQRBList[CmdqId].iWBSize, pDipWBuf);
        }
        else    //State Machine Error!!
        {
            pDipWBuf = NULL;
            this->BufferMutexUnLock();
            DIP_RINGBUF_ERR("State Machine of command queue(0x%x) have error, DipWBufWRIdx(0x%x)!!",CmdqId, DipWBufWRIdx);        
        }
    }
    else
    {
        pDipWBuf = NULL;
        this->BufferMutexUnLock();
        DIP_RINGBUF_INF("no any free working buffer of command queue(0x%x), please wait!!",CmdqId);        
    }
    return pDipWBuf;
}
bool DipRingBufferImp::ReleaseDipWorkingBuffer(DipWorkingBuffer* pDipWBuf, E_ISP_DIP_CQ CmdqId)
{
    unsigned int DipWBufRdIdx;

    if (CmdqId >= DIP_RING_BUFFER_CQ_SIZE){
        DIP_RINGBUF_ERR("[ERROR] ReleaseDipWorkingBuffer:Use the un-supported command queue(0x%x)",CmdqId);        
        return false;
    }
    if (mDipCmdQRBList[CmdqId].mDipWBList.size() == 0){
        DIP_RINGBUF_ERR("ReleaseDipWorkingBuffer:the size of command queue(0x%x) is zero!!",CmdqId);        
        return false;
    }

    this->BufferMutexLock();
    DipWBufRdIdx = mDipCmdQRBList[CmdqId].rIdx;
    if ( (mDipCmdQRBList[CmdqId].mDipWBList.size()>0) && (pDipWBuf != NULL) &&
        (DipWBufRdIdx < mDipCmdQRBList[CmdqId].mDipWBList.size()))
    {
        if ((mDipCmdQRBList[CmdqId].mDipWBList.at(DipWBufRdIdx) ==  pDipWBuf) && 
           (mDipCmdQRBList[CmdqId].mDipWBList.at(DipWBufRdIdx)->m_WBStatus == DIP_WB_USED))
        {
            mDipCmdQRBList[CmdqId].mDipWBList.at(DipWBufRdIdx)->m_WBStatus = DIP_WB_FREE;
            mDipCmdQRBList[CmdqId].rIdx++;
            if (mDipCmdQRBList[CmdqId].rIdx == MAX_CMDQ_RING_BUFFER_SIZE_NUM){
                mDipCmdQRBList[CmdqId].rIdx = 0;
            }
            mDipCmdQRBList[CmdqId].iWBSize++;
            this->BufferMutexUnLock();
        }
        else
        {
            this->BufferMutexUnLock();
            DIP_RINGBUF_ERR("State Machine of command queue(0x%x) have error, DipWBufRdIdx(0x%x), pDipWBuf(0x%x)!!",CmdqId, DipWBufRdIdx, pDipWBuf);        
            return false;
        }
    }
    else //Find Specific DipWBuf Address
    {
        this->BufferMutexUnLock();
        DIP_RINGBUF_ERR("State Machine of command queue(0x%x) have error, DipWBufRdIdx(0x%x), pDipWBuf(0x%x)!!",CmdqId, DipWBufRdIdx, pDipWBuf);        
        return false;
    }
    return true;
}

bool DipRingBufferImp::skipTpipeCalculatebyIdx(E_ISP_DIP_CQ CmdqId, unsigned int& frameflag, unsigned int& RefRingBufIdx, DipWorkingBuffer* pCurrFrameDipWBuf)
{
    DipWorkingBuffer* pPrevDipWBuf = NULL;
    unsigned int BufIdx;
    unsigned int TmpRingBufIdx;
    unsigned long TmptpipeTablePa = 0;
    unsigned int *TmptpipeTableVa = NULL;
    unsigned int LOAD_SIZE = 0, TileNum = 0, cpysize = 0;

    frameflag = 0;

    if (CmdqId >= DIP_RING_BUFFER_CQ_SIZE){
        DIP_RINGBUF_ERR("[ERROR] skipTpipeCalculatebyIdx:Use the un-supported command queue(0x%x)",CmdqId);        
        return false;
    }
    if (mDipCmdQRBList[CmdqId].mDipWBList.size() == 0){
        DIP_RINGBUF_ERR("skipTpipeCalculatebyIdx:the size of command queue(0x%x) is zero!!",CmdqId);        
        return false;
    }
    if (pCurrFrameDipWBuf == NULL){
        DIP_RINGBUF_ERR("skipTpipeCalculatebyIdx:pCurrFrameDipWBuf) of command queue(0x%x) is NULL!!",CmdqId);        
        return false;
    }
    if(CmdqId == ISP_DRV_DIP_CQ_THRE1)
    {
        //Skip VSS Process, MDP's VSS Stream don't support skip tpipe mechansim.
        return true;
    }

    //Reset tpipe table value to original tpipe value.
    //soluton:1 and soluton:4
    //pCurrFrameDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa = pCurrFrameDipWBuf->tpipeTableVa;
    //pCurrFrameDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTablePa = pCurrFrameDipWBuf->tpipeTablePa;    
    if(pCurrFrameDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.frameflag == 0x2)
    {
        frameflag = 2;
        //Skip FM Process
        return true;
    }

    BufIdx = pCurrFrameDipWBuf->m_BufIdx;
    if (BufIdx > 0)
    {
        TmpRingBufIdx = BufIdx-1;
    }
    else
    {
        TmpRingBufIdx = MAX_CMDQ_RING_BUFFER_SIZE_NUM-1;
    }
    //this->BufferMutexLock();
    //Skip Tpipe Rule 1: check the setting is the same as previous frame.
    if ((TmpRingBufIdx >= 0) && (TmpRingBufIdx < mDipCmdQRBList[CmdqId].mDipWBList.size()))
    {
        pPrevDipWBuf = mDipCmdQRBList[CmdqId].mDipWBList.at(TmpRingBufIdx);
        //Compare whether skip tpipe calculation.
        if(!(::memcmp(&(pCurrFrameDipWBuf->m_pMdpCfg->ispTpipeCfgInfo),
                                &(pPrevDipWBuf->m_pMdpCfg->ispTpipeCfgInfo), 
                                sizeof(ISP_TPIPE_CONFIG_STRUCT) - sizeof(ISP_DRIVER_CONFIG_STRUCT))))
        {   //adopt the setting in previous frame and copy previus frame tpipe value to current frame tpipe value
            frameflag=0x1;
//Solutin4:
            if (pPrevDipWBuf->m_WBStatus == DIP_WB_FREE)
            {
                this->BufferMutexLock();
                TmptpipeTablePa = pCurrFrameDipWBuf->tpipeTablePa;
                TmptpipeTableVa = pCurrFrameDipWBuf->tpipeTableVa;
                pCurrFrameDipWBuf->tpipeTablePa = pPrevDipWBuf->tpipeTablePa;
                pCurrFrameDipWBuf->tpipeTableVa = pPrevDipWBuf->tpipeTableVa;
                pCurrFrameDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTablePa = pPrevDipWBuf->tpipeTablePa;
                pCurrFrameDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa = pPrevDipWBuf->tpipeTableVa;
                pPrevDipWBuf->tpipeTablePa = TmptpipeTablePa;
                pPrevDipWBuf->tpipeTableVa = TmptpipeTableVa;
                pPrevDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTablePa = TmptpipeTablePa;
                pPrevDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa = TmptpipeTableVa;
                this->BufferMutexUnLock();
            }
            else
            {
                // get one tile have how many reigster setting. +1:register num, <<2:one rigster have 4 bytes, +4: 4 byte for loadsize.
                //LOAD_SIZE = ((((pPrevDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa[0] >> 8) & (0xff)) + 1) << 2) + 4;
                LOAD_SIZE = ((((pPrevDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa[0] >> 16) & (0xff)) + 1) << 2) + 4;
                TileNum = (pPrevDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.sw.src_width / TPIPE_WIDTH);
                if (pPrevDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.sw.src_width % TPIPE_WIDTH > 0)
                {
                    TileNum = TileNum + 1;
                }
                cpysize = TileNum*LOAD_SIZE;
                if ((cpysize > MAX_ISP_TILE_TDR_HEX_NO) || (cpysize == 0))
                {
                    cpysize = MAX_ISP_TILE_TDR_HEX_NO;
                }
                int tmpCpySize = cpysize;
                cpysize = MAX_ISP_TILE_TDR_HEX_NO;

                //cpysize = MAX_ISP_TILE_TDR_HEX_NO;
                ::memcpy(pCurrFrameDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa, pPrevDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa, cpysize);
                DIP_RINGBUF_INF("mem cpy from  previous tpipe buf(0x%x), current tpipe buf(0x%x), cpysize(%d) debug_cpysize(%d!!",pPrevDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa, pCurrFrameDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa, cpysize, tmpCpySize);
            }

//Solution3:
            //copy will wast 2~3ms.
            //::memcpy(pCurrFrameDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa, pPrevDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa, MAX_ISP_TILE_TDR_HEX_NO);
            //Change the tpipe buffer: tpipeTablePa and tpipeTableVa

//soluton:1 you will the isp is the same, but the mdp is not the same. you must re-caluclate the tile. so you can't change the tpipe buffer. unless you check the previous is done or not.
//            pCurrFrameDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa = pPrevDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa;
//            pCurrFrameDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTablePa = pPrevDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTablePa;


//Soluton:2 <==will fail.
            //TmptpipeTableVa = pCurrFrameDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa;
            //TmptpipeTablePa = pCurrFrameDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTablePa;
            //pCurrFrameDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa = pPrevDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa;
            //pCurrFrameDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTablePa = pPrevDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTablePa;
            //pPrevDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa = TmptpipeTableVa;
            //pPrevDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTablePa = TmptpipeTablePa;

            RefRingBufIdx = TmpRingBufIdx;
        }
        else
        {
            //a specical case: if 1st frame is frame 17, and frame17's frameflag is zero. Frame 18 to frame 39 and frame 0 to frame 16 are skip tpipe. so frame 16
            //will use frame 17's tdr buffer. new frame 17 enque and frame 16 is still running and frame 16 will fail.
            //check currect tpipe is the same as previous several frames.
        }
    }
    else
    {
        DIP_RINGBUF_ERR("skipTpipeCalculatebyIdx:TmpRingBufIdx(0x%x) is out of range in command queue(0x%x) is NULL!!",TmpRingBufIdx, CmdqId);        
        return false;        
    }
    //this->BufferMutexUnLock();

    return true;
}


