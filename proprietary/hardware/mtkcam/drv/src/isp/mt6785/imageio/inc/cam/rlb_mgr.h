#ifndef _RLB_MGR_H_
#define _RLB_MGR_H_

#include <isp_drv.h>
#include "isp_datatypes.h"
#include "cam_dupCmdQMgr.h"

//support streaming only. rawi is via cam_path_cam

typedef enum {
    E_RlbRet_OK = 0,
    E_RlbRet_ErrFSM = -1,
    E_RlbRet_ErrArg = -2,
    E_RlbRet_NoMem = -3
} E_RlbRetStatus;

class RlbMgr
{
public:
    RlbMgr(){}
    virtual ~RlbMgr(){}

    static RlbMgr*          getInstance(ISP_HW_MODULE module);

    virtual E_RlbRetStatus  acquire(ISP_HW_MODULE module, MUINT32 size, MUINT32 *pOffset);
    virtual E_RlbRetStatus  release(ISP_HW_MODULE module);

    typedef enum {
        eRlbCmd_QueryAvailable = 0,
        eRlbCmd_QueryAcquired,
        eRlbCmd_GetOffset
    } E_RlbCmd;
    virtual E_RlbRetStatus  sendCommand(ISP_HW_MODULE module, E_RlbCmd eQuery, MUINTPTR arg1, MUINTPTR arg2 = 0);
};
#endif

