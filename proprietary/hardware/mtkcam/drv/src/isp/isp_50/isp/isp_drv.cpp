/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "Isp_Drv"

#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>

#include "camera_isp.h"
#include <mtkcam/def/common.h>
#include <isp_drv.h>


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(isp_drv);


// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (isp_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (isp_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (isp_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (isp_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (isp_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (isp_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

class IspDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    IspDbgTimer(char const*const pszTitle)
        : mpszName(pszTitle)
        , mIdx(0)
        , mi4StartUs(getUs())
        , mi4LastUs(getUs())
    {
    }

    inline MINT32 getUs() const
    {
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    inline MBOOL ProfilingPrint(char const*const pszInfo = "") const
    {
        MINT32 const i4EndUs = getUs();
        if  (0==mIdx)
        {
            LOG_INF("[%s] %s:(%d-th) ===> [start-->now: %.06f ms]", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000);
        }
        else
        {
            LOG_INF("[%s] %s:(%d-th) ===> [start-->now: %.06f ms] [last-->now: %.06f ms]", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000, (float)(i4EndUs-mi4LastUs)/1000);
        }
        mi4LastUs = i4EndUs;

        //sleep(4); //wait 1 sec for AE stable

        return  MTRUE;
    }
};

MINT32              IspDrv::m_Fd = -1;
void*               IspDrv::m_pIspRegMap = NULL;
android::Mutex      IspDrv::RegRWMacro;
volatile MINT32     IspDrvImp::m_UserCnt = 0;
ISP_DRV_RW_MODE     IspDrvImp::m_regRWMode = ISP_DRV_R_ONLY;
android::Mutex      IspDrvImp::m_IspInitMutex;
vector<string>             IspDrvImp::m_User;

vector<MUINT32>     IspDrvImp::m_DapcReg;
MBOOL               IspDrvImp::m_SecOn = MFALSE;

static IspDrvImp    gIspDrvObj[MAX_ISP_HW_MODULE];

IspDrvImp::IspDrvImp()
{
    DBG_LOG_CONFIG(drv, isp_drv);
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    m_pIspHwRegAddr = NULL;
    m_HWmodule = CAM_A;
}

IspDrv* IspDrvImp::createInstance(ISP_HW_MODULE module)
{
    LOG_DBG("+,module(%d)",module);
    gIspDrvObj[module].m_HWmodule = module;
    return (IspDrv*)&gIspDrvObj[module];
}

MBOOL IspDrvImp::init(const char* userName)
{
    MBOOL Result = MTRUE, wakelock = MTRUE, camsys_pmqos = MTRUE;
    MUINT32 resetModule = 0;
    MINT32 tmp=0;
    //
    android::Mutex::Autolock lock(m_IspInitMutex);
    //
    LOG_INF("+,m_UserCnt(%d), curUser(%s).", this->m_UserCnt,userName);

    if (strlen(userName) < 1) {
            LOG_ERR("[Error]Plz add userName if you want to use isp driver\n");
        return MFALSE;
    }

    if (this->m_UserCnt > 0) {
            LOG_INF(" - X. m_UserCnt: %d,UserName:%s", this->m_UserCnt,userName);
        // Save userName and increase UserCnt
        m_User.push_back(userName);
        tmp = android_atomic_inc(&m_UserCnt);
            return Result;
        }

    // Open isp device
    if ((this->m_Fd = open(ISP_DRV_DEV_NAME, O_RDWR)) < 0)    // open failed.
        {
        LOG_ERR("ISP device node open fail, errno(%d):%s.", errno, strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }
    else    // open success.   // Sometimes GDMA will go this path, too. e.g. File Manager -> Phone Storage -> Photo.
    {
            // fd opened only init once
            switch(this->m_HWmodule){
                case CAM_A:
                case CAM_B:
                case CAM_C:
                case CAMSV_0:
                case CAMSV_1:
                case CAMSV_2:
                case CAMSV_3:
                case CAMSV_4:
                case CAMSV_5:
                    gIspDrvObj[CAM_A].m_pIspHwRegAddr = (MUINT32 *) mmap(0, CAM_BASE_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CAM_A_BASE_HW);
                    gIspDrvObj[CAM_B].m_pIspHwRegAddr = (MUINT32 *) mmap(0, CAM_BASE_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CAM_B_BASE_HW);
                    gIspDrvObj[CAM_C].m_pIspHwRegAddr = (MUINT32 *) mmap(0, CAM_BASE_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CAM_C_BASE_HW);
                    gIspDrvObj[CAMSV_0].m_pIspHwRegAddr = (MUINT32 *) mmap(0, CAMSV_BASE_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CAMSV_0_BASE_HW);
                    gIspDrvObj[CAMSV_1].m_pIspHwRegAddr = (MUINT32 *) mmap(0, CAMSV_BASE_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CAMSV_1_BASE_HW);
                    gIspDrvObj[CAMSV_2].m_pIspHwRegAddr = (MUINT32 *) mmap(0, CAMSV_BASE_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CAMSV_2_BASE_HW);
                    gIspDrvObj[CAMSV_3].m_pIspHwRegAddr = (MUINT32 *) mmap(0, CAMSV_BASE_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CAMSV_3_BASE_HW);
                    gIspDrvObj[CAMSV_4].m_pIspHwRegAddr = (MUINT32 *) mmap(0, CAMSV_BASE_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CAMSV_4_BASE_HW);
                    gIspDrvObj[CAMSV_5].m_pIspHwRegAddr = (MUINT32 *) mmap(0, CAMSV_BASE_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CAMSV_5_BASE_HW);
                    break;
                default:
                    LOG_ERR("#############\n");
                    LOG_ERR("this hw module(%d) is unsupported\n",this->m_HWmodule);
                    LOG_ERR("#############\n");
                    goto EXIT;
                    break;
            }
            for(MUINT32 i=0;i<MAX_ISP_HW_MODULE;i++){
                if(gIspDrvObj[i].m_pIspHwRegAddr == MAP_FAILED)
                {
                    LOG_ERR("ISP mmap fail (module:0x%x), errno(%d):%s",i, errno, strerror(errno));
                    Result = MFALSE;
                    goto EXIT;
                }
            }

            this->m_regRWMode=ISP_DRV_RW_MMAP;
    }

    if (ioctl(this->m_Fd, ISP_WAKELOCK_CTRL, (MUINT8*)& wakelock) < 0) {
        LOG_ERR("ISP_WAKELOCK_CTRL add error\n");
        Result = MFALSE;
        goto EXIT;
    }

    if (ioctl(this->m_Fd, ISP_DFS_CTRL, (MUINT8*)& camsys_pmqos) < 0) {
        LOG_ERR("ISP_DFS_CTRL turn on error\n");
        Result = MFALSE;
        goto EXIT;
    }

    while ((ISP_HW_MODULE)resetModule < PHY_CAM) {
        MUINT32 reset = 0;

        if ((ISP_HW_MODULE)resetModule == CAM_A) {
            reset = ISP_CAM_A_IDX;
        } else if ((ISP_HW_MODULE)resetModule == CAM_B) {
            reset = ISP_CAM_B_IDX;
        } else if ((ISP_HW_MODULE)resetModule == CAM_C) {
            reset = ISP_CAM_C_IDX;
        }

        if (ioctl(this->m_Fd, ISP_RESET_BY_HWMODULE, &reset) < 0) {
            LOG_ERR("HW reset fail. resetModule:%d\n", resetModule);
            Result = MFALSE;
            goto EXIT;
        }
        resetModule++;
    }

    //map reg map for R/W Macro, in order to calculate addr-offset by reg name
    if(this->m_pIspRegMap == NULL){
        this->m_pIspRegMap = (void*)malloc(MAX_MODULE_SIZE());
    }
    else{
        LOG_ERR("re-allocate RegMap\n");
    }

    // Save userName and increase UserCnt
    m_User.push_back(userName);
    tmp = android_atomic_inc(&m_UserCnt);

EXIT:

    //
    if (!Result)    // If some init step goes wrong.
    {
        if(this->m_Fd >= 0)
        {
            // unmap to avoid memory leakage
            for(MUINT32 i=0;i<MAX_ISP_HW_MODULE;i++){
                if(gIspDrvObj[i].m_pIspHwRegAddr != MAP_FAILED){
                    int ret = 0;
                    switch(i){
                        case CAM_A:
                        case CAM_B:
                        case CAM_C:
                            ret = munmap(gIspDrvObj[i].m_pIspHwRegAddr, CAM_BASE_RANGE);
                            if (ret < 0) {
                                LOG_ERR("munmap fail: %p\n", gIspDrvObj[i].m_pIspHwRegAddr);
                                break;
                            }
                            gIspDrvObj[i].m_pIspHwRegAddr = NULL;
                            break;
                        case CAMSV_0:
                        case CAMSV_1:
                        case CAMSV_2:
                        case CAMSV_3:
                        case CAMSV_4:
                        case CAMSV_5:
                            munmap(gIspDrvObj[i].m_pIspHwRegAddr, CAMSV_BASE_RANGE);
                            gIspDrvObj[i].m_pIspHwRegAddr = NULL;
                            break;
                        default:
                            LOG_ERR("#############\n");
                            LOG_ERR("this hw module(%d) is unsupported\n",i);
                            LOG_ERR("#############\n");
                            break;
                    }
                }
                else
                    gIspDrvObj[i].m_pIspHwRegAddr = NULL;
            }

            wakelock = MFALSE;
            if(ioctl(this->m_Fd, ISP_WAKELOCK_CTRL, (MUINT8*)&wakelock) < 0){
                LOG_ERR("ISPWakeLockCtrl disable fail!!");
            }

            camsys_pmqos = MFALSE;

            if(ioctl(this->m_Fd, ISP_DFS_CTRL, (MUINT8*)& camsys_pmqos) < 0) {
                LOG_ERR("ISP_DFS_CTRL turn off error\n");
            }

            close(this->m_Fd);
            this->m_Fd = -1;
            LOG_INF("close isp device Fd");
        }
    }

    LOG_DBG("-,ret: %d. mInitCount:(%d)", Result, this->m_UserCnt);
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::uninit(const char* userName)
{
    MBOOL Result = MTRUE, wakelock = MFALSE, camsys_pmqos = MFALSE;
    MINT32 tmp=0;

    //
    android::Mutex::Autolock lock(m_IspInitMutex);
    //
    LOG_INF("-,m_UserCnt(%d),curUser(%s)", this->m_UserCnt,userName);
    //
    if (strlen(userName)<1) {
        LOG_ERR("Plz add userName if you want to uninit isp driver\n");
        return MFALSE;
    }

    //
    if (this->m_UserCnt <= 0) {
        LOG_ERR("no more user in IspDrv , curUser(%s)",userName);
        return MFALSE;
    }

    auto itr = std::find(m_User.begin(), m_User.end(), userName);
    if (itr != m_User.end()) {
        m_User.erase(itr);
    } else {
        LOG_ERR("No matching userName:%s\n",userName);

        char str[200] = {0,};
        char tmp[50] = {0,};
        MUINT32 count = 0;
        for (MUINT32 i = 0; i < m_User.size(); i++) {
            sprintf(tmp, "CurUser(%d):%s, ", i, m_User.at(i).c_str());
            count = sizeof(str) - strlen(str);
            strncat(str, tmp, count-1);
            if (((i+1) % 6) == 0) {
                LOG_INF("%s", str);
                memset(str, 0, sizeof(str));
        }
    }
        if(str[0] != 0)
            LOG_INF("%s", str);

        return MFALSE;
    }


    // More than one user
    tmp = android_atomic_dec(&m_UserCnt);
    if(this->m_UserCnt > 0)    // If there are still users, exit.
        goto EXIT;

    for(MUINT32 i=0;i<MAX_ISP_HW_MODULE;i++){
        if(gIspDrvObj[i].m_pIspHwRegAddr != MAP_FAILED){
            int ret = 0;
            switch(i){
                case CAM_A:
                case CAM_B:
                case CAM_C:
                    ret = munmap(gIspDrvObj[i].m_pIspHwRegAddr, CAM_BASE_RANGE);
                    if (ret < 0) {
                        LOG_ERR("munmap fail: %p\n", gIspDrvObj[i].m_pIspHwRegAddr);
                        break;
                    }
                    gIspDrvObj[i].m_pIspHwRegAddr = NULL;
                    break;
                case CAMSV_0:
                case CAMSV_1:
                case CAMSV_2:
                case CAMSV_3:
                case CAMSV_4:
                case CAMSV_5:
                    munmap(gIspDrvObj[i].m_pIspHwRegAddr, CAMSV_BASE_RANGE);
                    gIspDrvObj[i].m_pIspHwRegAddr = NULL;
                    break;
                case CAM_A_TWIN_B://virtual hw , do nothing
                case CAM_A_TWIN_C:
                case CAM_B_TWIN_C:
                case CAM_A_TRIPLE_B:
                case CAM_A_TRIPLE_C:
                    break;
                default:
                    LOG_ERR("#############\n");
                    LOG_ERR("this hw module(%d) is unsupported\n",i);
                    LOG_ERR("#############\n");
                    break;
            }
        }
    }

    //
    if(this->m_pIspRegMap){
        free((MUINT32*)this->m_pIspRegMap);
        this->m_pIspRegMap = NULL;
    }

    if(this->m_SecOn){
        m_DapcReg.clear();
        this->m_SecOn = 0;
    }

    //
    if (this->m_Fd >= 0) {
        if (ioctl(this->m_Fd, ISP_WAKELOCK_CTRL, (MUINT8*)& wakelock) < 0) {
            LOG_ERR("ISP_WAKELOCK_CTRL remove error\n");
            Result = MFALSE;
            goto EXIT;
        }

        if(ioctl(this->m_Fd, ISP_DFS_CTRL, (MUINT8*)& camsys_pmqos) < 0) {
            LOG_ERR("ISP_DFS_CTRL turn off error\n");
            Result = MFALSE;
            goto EXIT;
        }

        close(this->m_Fd);
        this->m_Fd = -1;
        this->m_regRWMode=ISP_DRV_R_ONLY;
    }


EXIT:

    LOG_INF(" - X. ret: %d. m_UserCnt: %d.", Result, this->m_UserCnt);

    if(this->m_UserCnt!= 0){
        char str[200] = {0,};
        char tmp[50] = {0,};
        MUINT32 count = 0;
        for (MUINT32 i = 0; i < m_User.size(); i++) {
            sprintf(tmp, "CurUser(%d):%s, ", i, m_User.at(i).c_str());
            count = sizeof(str) - strlen(str);
            strncat(str, tmp, count-1);
            if (((i+1) % 6) == 0) {
                LOG_INF("%s", str);
                memset(str, 0, sizeof(str));
            }
        }
        if(str[0] != 0)
            LOG_INF("%s", str);
    }

    return Result;
}

#define FD_CHK()({\
    MINT32 Ret=0;\
    if(this->m_Fd < 0){\
        LOG_ERR("no isp device\n");\
        Ret = -1;\
    }\
    Ret;\
})

MBOOL IspDrvImp::waitIrq(ISP_WAIT_IRQ_ST* pWaitIrq)
{
    MINT32 Ret = 0;
    MUINT32 OrgTimeOut;
    ISP_IRQ_CLEAR_ENUM OrgClr;
    IspDbgTimer dbgTmr("waitIrq");
    MUINT32 Ta=0,Tb=0;
    ISP_WAIT_IRQ_STRUCT wait;
    LOG_DBG(" - E. hwmodule:0x%x,Status(0x%08x),Timeout(%d).\n",this->m_HWmodule, pWaitIrq->Status, pWaitIrq->Timeout);
    if(FD_CHK() == -1){
        return MFALSE;
    }

    switch(this->m_HWmodule){
        case CAM_A:     wait.Type = ISP_IRQ_TYPE_INT_CAM_A_ST;
            break;
        case CAM_B:     wait.Type = ISP_IRQ_TYPE_INT_CAM_B_ST;
            break;
        case CAM_C:     wait.Type = ISP_IRQ_TYPE_INT_CAM_C_ST;
            break;
        case CAMSV_0:   wait.Type = ISP_IRQ_TYPE_INT_CAMSV_0_ST;
            break;
        case CAMSV_1:   wait.Type = ISP_IRQ_TYPE_INT_CAMSV_1_ST;
            break;
        case CAMSV_2:   wait.Type = ISP_IRQ_TYPE_INT_CAMSV_2_ST;
            break;
        case CAMSV_3:   wait.Type = ISP_IRQ_TYPE_INT_CAMSV_3_ST;
            break;
        case CAMSV_4:   wait.Type = ISP_IRQ_TYPE_INT_CAMSV_4_ST;
            break;
        case CAMSV_5:   wait.Type = ISP_IRQ_TYPE_INT_CAMSV_5_ST;
            break;
        default:
            LOG_ERR("unsupported hw module:0x%x\n",this->m_HWmodule);
            return MFALSE;
            break;
    }

    OrgTimeOut = pWaitIrq->Timeout;
    OrgClr = pWaitIrq->Clear;
    memcpy(&wait.EventInfo,pWaitIrq,sizeof(ISP_WAIT_IRQ_ST));


    do{
        Ta=dbgTmr.getUs();
        Ret = ioctl(this->m_Fd,ISP_WAIT_IRQ,&wait);
        Tb=dbgTmr.getUs();

        //receive restart system call signal
        if ((errno == SIG_ERESTARTSYS) && (wait.EventInfo.Timeout > 0)) {
            if ((Tb-Ta) > 1000 && (((Tb-Ta)/1000) < wait.EventInfo.Timeout)) { //at least 1ms
            wait.EventInfo.Timeout=wait.EventInfo.Timeout - ((Tb-Ta)/1000);
            wait.EventInfo.Clear = ISP_IRQ_CLEAR_NONE;
                LOG_INF("Interrupted by other wait again. Type(%d),Status(0x%08x),Timeout(%dms)\n", wait.Type, pWaitIrq->Status, wait.EventInfo.Timeout);
            } else {
                break;
            }
        }
        else
            break;
    }while(1);


    pWaitIrq->Timeout = OrgTimeOut;
    pWaitIrq->Clear = OrgClr;


    if(Ret < 0) {
        LOG_ERR("ISP(0x%x)_WAIT_IRQ fail(%d). Wait Status(0x%08x), Timeout(%d).\n",this->m_HWmodule, errno, pWaitIrq->Status, pWaitIrq->Timeout);
        return MFALSE;
    }


    return MTRUE;
}

MBOOL IspDrvImp::clearIrq(ISP_CLEAR_IRQ_ST* pClearIrq)
{
    MINT32 Ret;
    ISP_CLEAR_IRQ_STRUCT clear;
    //
    LOG_DBG(" - E. hw module:0x%x,user(%d), Status(%d)\n",this->m_HWmodule,pClearIrq->UserKey, pClearIrq->Status);
    if(FD_CHK() == -1){
        return MFALSE;
    }
    switch(this->m_HWmodule){
        case CAM_A:     clear.Type = ISP_IRQ_TYPE_INT_CAM_A_ST;
            break;
        case CAM_B:     clear.Type = ISP_IRQ_TYPE_INT_CAM_B_ST;
            break;
        case CAM_C:     clear.Type = ISP_IRQ_TYPE_INT_CAM_C_ST;
            break;
        case CAMSV_0:   clear.Type = ISP_IRQ_TYPE_INT_CAMSV_0_ST;
            break;
        case CAMSV_1:   clear.Type = ISP_IRQ_TYPE_INT_CAMSV_1_ST;
            break;
        case CAMSV_2:   clear.Type = ISP_IRQ_TYPE_INT_CAMSV_2_ST;
            break;
        case CAMSV_3:   clear.Type = ISP_IRQ_TYPE_INT_CAMSV_3_ST;
            break;
        case CAMSV_4:   clear.Type = ISP_IRQ_TYPE_INT_CAMSV_4_ST;
            break;
        case CAMSV_5:   clear.Type = ISP_IRQ_TYPE_INT_CAMSV_5_ST;
            break;
        default:
            LOG_ERR("unsupported hw module:0x%x\n",this->m_HWmodule);
            return MFALSE;
            break;
    }
    //
    memcpy(&clear.EventInfo,pClearIrq,sizeof(ISP_CLEAR_IRQ_ST));
    Ret = ioctl(this->m_Fd,ISP_CLEAR_IRQ,&clear);
    if(Ret < 0)
    {
        LOG_ERR("ISP(0x%x)_CLEAR_IRQ fail(%d)\n",this->m_HWmodule,Ret);
        return MFALSE;
    }
    return MTRUE;
}

MBOOL IspDrvImp::registerIrq(ISP_REGISTER_USERKEY_STRUCT* pRegIrq)
{
    MINT32 Ret;
    LOG_DBG(" - E. hw module:0x%x,userkey(%d), name(%s)\n",this->m_HWmodule,pRegIrq->userKey, pRegIrq->userName);
    //
    Ret = ioctl(this->m_Fd,ISP_REGISTER_IRQ_USER_KEY,pRegIrq);
    if(Ret < 0) {
        LOG_ERR("ISP_REGISTER_IRQ fail(%d). hw module:0x%x, userkey(%d), name(%s\n", Ret, this->m_HWmodule, pRegIrq->userKey, pRegIrq->userName);
        return MFALSE;
    }
    return MTRUE;
}


MUINT32 IspDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    MINT32 Ret=0;
    MUINT32 value=0x0;
    MUINT32 legal_range = REG_SIZE;
    LOG_DBG("+,Isp_read:m_HWmodule(0x%x),Addr(0x%x)\n",this->m_HWmodule,Addr);
    android::Mutex::Autolock lock(this->IspRegMutex);
    (void)caller;
    if(FD_CHK() == -1){
        return 1;
    }

    //For secure, DAPC register check
    if(this->m_SecOn){
        for(MUINT32 i=0;i<m_DapcReg.size();i++){
            if(Addr == m_DapcReg.at(i)){
                LOG_DBG("[SecCam ON]Reg Addr:0x%x is not allowed to read\n",Addr);
                return 1;
            }
        }
    }


    if(this->m_regRWMode==ISP_DRV_RW_MMAP){
        if(Addr >= legal_range){
            LOG_ERR("over range(0x%x)\n",Addr);
            return 0;
        }
        value = this->m_pIspHwRegAddr[(Addr>>2)];
    }
    else{
        ISP_REG_IO_STRUCT IspRegIo;
        ISP_DRV_REG_IO_STRUCT RegIo;

        switch(this->m_HWmodule){
            case CAM_A: RegIo.module = (MUINT32)ISP_CAM_A_IDX;
                break;
            case CAM_B: RegIo.module = (MUINT32)ISP_CAM_B_IDX;
                break;
            case CAM_C: RegIo.module = (MUINT32)ISP_CAM_C_IDX;
                break;
            default:
                LOG_ERR("unsupported module:%d\n",this->m_HWmodule);
                return 1;
                break;
        }

        RegIo.Addr = Addr;
        IspRegIo.pData = (ISP_REG_STRUCT*)&RegIo;
        IspRegIo.Count = 1;

        Ret = ioctl(this->m_Fd, ISP_READ_REGISTER, &IspRegIo);
        if(Ret < 0)
        {
            LOG_ERR("ISP(0x%x)_READ via IO fail(%d)",this->m_HWmodule,Ret);
            return value;
        }
        value=RegIo.Data;
    }
    LOG_DBG("-,Isp_read:(0x%x,0x%x)",Addr,value);
    return value;
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    MUINT32 legal_range = REG_SIZE;
    android::Mutex::Autolock lock(this->IspRegMutex);
    (void)caller;
    if(FD_CHK() == -1){
        return MFALSE;
    }

    if(this->m_regRWMode == ISP_DRV_RW_MMAP){
        unsigned int i=0;
        do{
            if(pRegIo[i].Addr >= legal_range){
                LOG_ERR("over range,bypass_0x%x\n",pRegIo[i].Addr);
                i = Count;
            }
            else
                pRegIo[i].Data = this->m_pIspHwRegAddr[(pRegIo[i].Addr>>2)];
        }while(++i<Count);
    }
    else{
        ISP_REG_IO_STRUCT IspRegIo;

        switch(this->m_HWmodule){
            case CAM_A: pRegIo->module = ISP_CAM_A_IDX;
                break;
            case CAM_B: pRegIo->module = ISP_CAM_B_IDX;
                break;
            case CAM_C: pRegIo->module = ISP_CAM_C_IDX;
                break;
            default:
                LOG_ERR("unsupported module:%d\n",this->m_HWmodule);
                return MFALSE;
                break;
        }

        IspRegIo.pData = (ISP_REG_STRUCT*)pRegIo;
        IspRegIo.Count = Count;


        Ret = ioctl(this->m_Fd, ISP_READ_REGISTER, &IspRegIo);
        if(Ret < 0)
        {
            LOG_ERR("ISP(0x%x)_READ via IO fail(%d)",this->m_HWmodule,Ret);
            return MFALSE;
        }
    }

    LOG_DBG("Isp_reads_0x%x(%d): 0x%x_0x%x",this->m_HWmodule,Count,pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;
}


MBOOL IspDrvImp::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    MINT32 ret=0;
    MUINT32 legal_range = REG_SIZE;
    LOG_DBG("Isp_write:m_HWmodule(0x%x),m_regRWMode(0x%x),(0x%x,0x%lx)",this->m_HWmodule,this->m_regRWMode,Addr,Data);
    android::Mutex::Autolock lock(this->IspRegMutex);
    (void)caller;
    if(FD_CHK() == -1){
        return MFALSE;
    }


    //For secure, DAPC register check
    if(this->m_SecOn){
        for(MUINT32 i=0;i<m_DapcReg.size();i++){
            if(Addr == m_DapcReg.at(i)){
                LOG_DBG("[SecCam ON]Reg Addr:0x%x is not allowed to write\n",Addr);
                return 1;
            }
        }
    }


    switch(this->m_regRWMode){
        case ISP_DRV_RW_MMAP:
            if(Addr >= legal_range){
                LOG_ERR("over range(0x%x)\n",Addr);
                return MFALSE;
            }
            this->m_pIspHwRegAddr[(Addr>>2)] = Data;
            break;
        case ISP_DRV_RW_IOCTL:
            ISP_REG_IO_STRUCT IspRegIo;
            ISP_DRV_REG_IO_STRUCT RegIo;
            switch(this->m_HWmodule){
                case CAM_A: RegIo.module = ISP_CAM_A_IDX;
                    break;
                case CAM_B: RegIo.module = ISP_CAM_B_IDX;
                    break;
                case CAM_C: RegIo.module = ISP_CAM_C_IDX;
                    break;
                default:
                    LOG_ERR("unsupported module:%d\n",this->m_HWmodule);
                    return MFALSE;
                    break;
            }

            RegIo.Addr = Addr;
            RegIo.Data = Data;
            IspRegIo.pData = (ISP_REG_STRUCT*)&RegIo;
            IspRegIo.Count = 1;
            ret = ioctl(this->m_Fd, ISP_WRITE_REGISTER, &IspRegIo);
            if(ret < 0){
                LOG_ERR("ISP(0x%x)_WRITE via IO fail(%d)",this->m_HWmodule,ret);
                return MFALSE;
            }
            break;
        case ISP_DRV_R_ONLY:
            LOG_ERR("ISP Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }


    //
    return MTRUE;
}

MBOOL IspDrvImp::writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    unsigned int i=0;
    MUINT32 legal_range = REG_SIZE;
    android::Mutex::Autolock lock(this->IspRegMutex);
    (void)caller;
    if(FD_CHK() == -1){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case ISP_DRV_RW_IOCTL:
            ISP_REG_IO_STRUCT IspRegIo;
            switch(this->m_HWmodule){
                case CAM_A: pRegIo->module = ISP_CAM_A_IDX;
                    break;
                case CAM_B: pRegIo->module = ISP_CAM_B_IDX;
                    break;
                case CAM_C: pRegIo->module = ISP_CAM_C_IDX;
                    break;
                default:
                    LOG_ERR("unsupported module:%d\n",this->m_HWmodule);
                    return MFALSE;
                    break;
            }

            IspRegIo.pData = (ISP_REG_STRUCT*)pRegIo;
            IspRegIo.Count = Count;

            Ret = ioctl(this->m_Fd, ISP_WRITE_REGISTER, &IspRegIo);
            if(Ret < 0){
                LOG_ERR("ISP(0x%x)_WRITE via IO fail(%d)",this->m_HWmodule,Ret);
                return MFALSE;
            }
            break;
        case ISP_DRV_RW_MMAP:
            do{
                if(pRegIo[i].Addr >= legal_range){
                    LOG_ERR("over range,bypass_0x%x\n",pRegIo[i].Addr);
                    i = Count;
                }
                else
                    this->m_pIspHwRegAddr[(pRegIo[i].Addr>>2)] = pRegIo[i].Data;
            }while(++i<Count);
            break;
        case ISP_DRV_R_ONLY:
            LOG_ERR("ISP Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    LOG_DBG("Isp_writes_0x%x(%d):0x%x_0x%x\n",this->m_HWmodule,Count,pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;

}

MUINT32 IspDrvImp::getRWMode(void)
{
    return this->m_regRWMode;
}

//for early porting use only,  not regular path.
MBOOL IspDrvImp::setRWMode(ISP_DRV_RW_MODE rwMode)
{
    if(rwMode > ISP_DRV_RW_CQ)
    {
        LOG_ERR("no reg operation mode(0x%x)",this->m_regRWMode);
        return MFALSE;
    }

    this->m_regRWMode = rwMode;

    return MTRUE;
}

MBOOL IspDrvImp::getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData)
{
    MBOOL rst = MTRUE;

    switch(eCmd){
        case _GET_VSYNC_CNT:
            switch(this->m_HWmodule){
                case CAM_A:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAM_A_ST;
                    break;
                case CAM_B:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAM_B_ST;
                    break;
                case CAM_C:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAM_C_ST;
                    break;
                default:
                    LOG_ERR("unsupported module:0x%x\n",this->m_HWmodule);
                    return MFALSE;
                    break;
            }
            if(ioctl(this->m_Fd,ISP_GET_VSYNC_CNT,(unsigned char*)pData) < 0){
                LOG_ERR("_GET_VSYNC_CNT fail\n");
                rst = MFALSE;
            }
            break;
        default:
            LOG_ERR("unsupported cmd:0x%x\n",eCmd);
            return MFALSE;
        break;
    }
    return rst;
}

MBOOL IspDrvImp::setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData)
{
    switch(eCmd){
        case _RESET_VSYNC_CNT:
            if(ioctl(this->m_Fd,ISP_RESET_VSYNC_CNT,NULL) < 0){
                LOG_ERR("_RESET_VSYNC_CNT error,ctrl:0x%x\n",((ISP_BUFFER_CTRL_STRUCT*)pData)->ctrl);
                return MFALSE;
            }
            break;
        case _SET_WAKE_LOCK:
            if(ioctl(this->m_Fd,ISP_WAKELOCK_CTRL,(unsigned char*)pData) < 0){
                LOG_ERR("_SET_WAKE_LOCK error\n");
                return MFALSE;
            }
            break;
        default:
            LOG_ERR("unsupported cmd:0x%x",eCmd);
            return MFALSE;
        break;
    }
    return MTRUE;
}

MBOOL IspDrvImp::setDapcReg(vector<MUINT32> reg_list)
{

    m_DapcReg = reg_list;
    this->m_SecOn = MTRUE;

    LOG_INF("m_SecOn:0x%x",this->m_SecOn);
    for(MUINT32 i=0;i<m_DapcReg.size();i++)
        LOG_DBG("Addr:0x%x",m_DapcReg.at(i));

    return MTRUE;
}
