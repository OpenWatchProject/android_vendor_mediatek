#define LOG_TAG "RlbMgr"

#include <unistd.h> //for usleep
#include <cutils/properties.h>  // For property_get().

#include "rlb_mgr.h"
#include "isp_function_cam.h"
#include "cam_capibility.h"
#include <mtkcam/drv/def/ispio_port_index.h>

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

using namespace std;


DECLARE_DBG_LOG_VARIABLE(RlbMgr);

// Clear previous define, use our own define.
#undef RlbMgr_VRB
#undef RlbMgr_DBG
#undef RlbMgr_INF
#undef RlbMgr_WRN
#undef RlbMgr_ERR
#undef RlbMgr_AST
#define RlbMgr_VRB(fmt, arg...)        do { if (RlbMgr_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define RlbMgr_DBG(fmt, arg...)        do { if (RlbMgr_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define RlbMgr_INF(fmt, arg...)        do { if (RlbMgr_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define RlbMgr_WRN(fmt, arg...)        do { if (RlbMgr_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define RlbMgr_ERR(fmt, arg...)        do { if (RlbMgr_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define RlbMgr_AST(cond, fmt, arg...)  do { if (RlbMgr_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define _CAL_OFFSET(_pDrv, _module) ({\
    MUINT32 _ofst;\
    switch (_module) {\
    case CAM_A: _ofst = 0; break;\
    case CAM_B: _ofst = _pDrv->m_capacity - _pDrv->m_allocatedSize[_module]; break;\
    default:    _ofst = -1;\
        RlbMgr_ERR("Wrong module id: 0x%x\n", (MUINT32)_module);\
        break;\
    }\
    _ofst;\
})

class RlbMgrImp : public RlbMgr
{
public:
    RlbMgrImp();
    virtual ~RlbMgrImp(){}

    virtual E_RlbRetStatus  acquire(ISP_HW_MODULE module, MUINT32 size, MUINT32 *pOffset);
    virtual E_RlbRetStatus  release(ISP_HW_MODULE module);
    virtual E_RlbRetStatus  sendCommand(ISP_HW_MODULE module, E_RlbCmd eQuery, MUINTPTR arg1, MUINTPTR arg2 = 0);
private :
    enum E_FSM{
        op_empty = 0x0,
        op_acquire,
        op_release,
        op_queryAvailable,
        op_queryAcquired,
        op_queryOffset
    };
    E_RlbRetStatus          FSM_CHK(ISP_HW_MODULE module, MUINT32 op);

private:
    MUINT32         mFSM[PHY_CAM];

    MUINT32         m_capacity;
    MUINT32         m_freeSize;
    MUINT32         m_allocatedSize[PHY_CAM];

    mutable Mutex   m_lock;
};


RlbMgr* RlbMgr::getInstance(ISP_HW_MODULE module)
{
    static RlbMgrImp m_RlbMgrSingleton;
    (void)module;

    return &m_RlbMgrSingleton;
}

RlbMgrImp::RlbMgrImp()
{
    MUINT32     i;
    tCAM_rst    camrst;
    capibility  cap;

    if (cap.GetCapibility(NSImageio::NSIspio::EPortIndex_RRZO,
                            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,
                            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),
                            camrst, E_CAM_BS_Max_size) == MFALSE) {
        RlbMgr_ERR("Query RLB max size capibility failed, force rlb size 1\n");
        camrst.bs_info.bs_max_size = 1;
    }

    this->m_capacity = camrst.bs_info.bs_max_size;
    this->m_freeSize = camrst.bs_info.bs_max_size;

    memset(this->m_allocatedSize, 0, sizeof(this->m_allocatedSize));
    for (i = 0; i < sizeof(this->mFSM)/sizeof(this->mFSM[0]); i++) {
        this->mFSM[i] = op_empty;
    }

    DBG_LOG_CONFIG(imageio, RlbMgr);
}

E_RlbRetStatus RlbMgrImp::FSM_CHK(ISP_HW_MODULE module, MUINT32 op)
{
    E_RlbRetStatus ret = E_RlbRet_OK;

    switch (module) {
    case CAM_A:
    case CAM_B:
    case CAM_C:
        break;
    default:
        return E_RlbRet_ErrArg;
    }

    switch(op){
    case op_acquire:
    case op_queryAvailable:
        switch(this->mFSM[module]) {
        case op_empty:
            break;
        default:
            ret = E_RlbRet_ErrFSM;
            break;
        }
        break;
    case op_release:
    case op_queryOffset:
        switch(this->mFSM[module]) {
        case op_acquire:
            break;
        default:
            ret = E_RlbRet_ErrFSM;
            break;
        }
        break;
    case op_queryAcquired:
        break;
    default:
        ret = E_RlbRet_ErrArg;
        break;
    }

    if (ret != E_RlbRet_OK) {
        RlbMgr_ERR("[0x%x]FSM error ret:%d cur:0x%x tar:0x%x\n",
            module, (MINT32)ret, this->mFSM[module], op);
    }

    return ret;
}

E_RlbRetStatus RlbMgrImp::acquire(ISP_HW_MODULE module, MUINT32 size, MUINT32 *pOffset)
{
    Mutex::Autolock lock(this->m_lock);
    E_RlbRetStatus ret;
    tCAM_rst    camrst;
    capibility  cap;

    ret = FSM_CHK(module, op_acquire);
    if (ret != E_RlbRet_OK) {
        return ret;
    }

    if (cap.GetCapibility(NSImageio::NSIspio::EPortIndex_RRZO,
                            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,
                            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),
                            camrst, E_CAM_BS_Alignment) == MFALSE) {
        RlbMgr_ERR("Query RLB BS alignment capibility failed, force 8-alignment \n");
        camrst.bs_info.bs_alignment = 8;
    }
    size = size + (8 * 2); // reserve space for slave cams' rlb offset alignment, at most two slave cams
    size = ((size + (camrst.bs_info.bs_alignment - 1)) >> 3) << 3;

    if (size <= this->m_freeSize) {
        this->m_freeSize -= size;
        this->m_allocatedSize[module] = size;

        this->mFSM[module] = op_acquire;

        *pOffset = _CAL_OFFSET(this, module);

        RlbMgr_INF("[0x%x]RLB acquired at %d size %d left:%d\n",
            module, *pOffset, this->m_allocatedSize[module], this->m_freeSize);
    }
    else {
        RlbMgr_ERR("[0x%x]Acquire fail: need:%d left:%d\n", module, size, this->m_freeSize);
        ret = E_RlbRet_NoMem;
    }

    return ret;
}

E_RlbRetStatus RlbMgrImp::release(ISP_HW_MODULE module)
{
    Mutex::Autolock lock(this->m_lock);
    E_RlbRetStatus ret;

    ret = FSM_CHK(module, op_release);
    if (ret != E_RlbRet_OK) {
        return ret;
    }

    this->m_freeSize += this->m_allocatedSize[module];
    this->m_allocatedSize[module] = 0;

    this->mFSM[module] = op_empty;

    return ret;
}

E_RlbRetStatus RlbMgrImp::sendCommand(ISP_HW_MODULE module, E_RlbCmd eQuery, MUINTPTR arg1, MUINTPTR arg2)
{
    Mutex::Autolock lock(this->m_lock);
    E_RlbRetStatus ret;
    arg2;
    switch (eQuery) {
    case eRlbCmd_QueryAvailable:
        ret = FSM_CHK(module, op_queryAvailable);
        if (ret != E_RlbRet_OK) {
            return ret;
        }

        *(MUINT32 *)arg1 = this->m_freeSize;
        break;
    case eRlbCmd_QueryAcquired:
        ret = FSM_CHK(module, op_queryAcquired);
        if (ret != E_RlbRet_OK) {
            break;
        }

        *(MUINT32 *)arg1 = this->m_allocatedSize[module];
        break;
    case eRlbCmd_GetOffset:
        ret = FSM_CHK(module, op_queryOffset);
        if (ret != E_RlbRet_OK) {
            break;
        }

        *(MUINT32 *)arg1 = _CAL_OFFSET(this, module);
        break;
    default:
        ret = E_RlbRet_ErrArg;
        RlbMgr_ERR("[0x%x]unsupported cmd: 0x%x\n", (MUINT32)module, (MUINT32)eQuery);
        break;
    }

    return ret;
}




