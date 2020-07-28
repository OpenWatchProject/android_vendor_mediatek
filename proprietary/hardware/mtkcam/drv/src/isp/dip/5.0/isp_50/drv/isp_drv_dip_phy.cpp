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
#define LOG_TAG "IspDrvDipPhy"

#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>
#include <mtkcam/def/common.h>

#include "camera_dip.h"
#include "isp_drv_dip_phy.h"

#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(isp_drv_dip_phy);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (isp_drv_dip_phy_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (isp_drv_dip_phy_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (isp_drv_dip_phy_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (isp_drv_dip_phy_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (isp_drv_dip_phy_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (isp_drv_dip_phy_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#if 0
volatile MINT32     PhyDipDrvImp::m_UserCnt = 0;
ISP_DRV_RW_MODE     PhyDipDrvImp::m_regRWMode = ISP_DRV_R_ONLY;
android::Mutex      PhyDipDrvImp::m_IspInitMutex;
char                PhyDipDrvImp::m_UserName[MAX_USER_NUMBER][MAX_USER_NAME_SIZE] =
{
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
};
#endif


#define FD_CHK()({\
    MINT32 Ret=0;\
    if(this->m_Fd < 0){\
        LOG_ERR("no isp device\n");\
        Ret = -1;\
    }\
    Ret;\
})
    
static PhyDipDrvImp    gIspDrvObj[DIP_HW_MAX];
  
DipDrv* PhyDipDrvImp::createInstance(DIP_HW_MODULE module)
{
    LOG_INF("DIP: PhyDipDrvImp CreateInstance");
    LOG_DBG("+,module(%d)",module);
    gIspDrvObj[module].m_HWmodule = module;
    return (DipDrv*)&gIspDrvObj[module];
}

PhyDipDrvImp::PhyDipDrvImp()
{
    DBG_LOG_CONFIG(drv, isp_drv_dip_phy);
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    mpIspDipHwRegAddr = NULL;
	m_HWmodule = DIP_HW_A;
    m_IsEverDumpBuffer = MTRUE;
    mpTempIspDipHWRegValues = NULL;
    m_UserCnt = 0;
    m_Fd = -1;
    this->m_regRWMode=ISP_DRV_R_ONLY;
}

MBOOL PhyDipDrvImp::init(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 resetModule;
    MUINT32 strLen;
    MINT32 tmp=0;
    //
    android::Mutex::Autolock lock(m_IspInitMutex);
    //
    LOG_INF("DIP +,m_UserCnt(%d), curUser(%s).", this->m_UserCnt,userName);
    //
    strLen = strlen(userName);
    if((strLen<1)||(strLen>=MAX_USER_NAME_SIZE))
    {
        if(strLen<1)
            LOG_ERR("[Error]Plz add userName if you want to use isp driver\n");
        else if(strLen>=MAX_USER_NAME_SIZE)
            LOG_ERR("[Error]user's userName length(%d) exceed the default length(%d)\n",strLen,MAX_USER_NAME_SIZE);
        else
            LOG_ERR("[Error]coding error, please check judgement condition\n");
        return MFALSE;
    }
    //
    if(this->m_UserCnt > 0)
    {
        if(this->m_UserCnt < MAX_USER_NUMBER){
            strncpy((char*)this->m_UserName[this->m_UserCnt],userName,strlen(userName));
            this->m_UserName[this->m_UserCnt][strlen(userName)]='\0';
            android_atomic_inc(&m_UserCnt);
            LOG_INF(" - X. m_UserCnt: %d,UserName:%s", this->m_UserCnt,userName);
            return Result;
        }
        else{
            LOG_ERR("m_userCnt is over upper bound\n");
            return MFALSE;
        }
    }

    // Open isp device
    LOG_INF("open dip device\n");
    this->m_Fd = open(ISP_DRV_DEV_NAME, O_RDWR);
    if (this->m_Fd < 0)    // 1st time open failed.
    {
        LOG_ERR("DIP kernel 1st open fail, errno(%d):%s.", errno, strerror(errno));
        // Try again, using "Read Only".
        this->m_Fd = open(ISP_DRV_DEV_NAME, O_RDONLY);
        if (this->m_Fd < 0) // 2nd time open failed.
        {
            LOG_ERR("DIP kernel 2nd open fail, errno(%d):%s.", errno, strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }
        else
            this->m_regRWMode=ISP_DRV_R_ONLY;
    }
    else    // 1st time open success.   // Sometimes GDMA will go this path, too. e.g. File Manager -> Phone Storage -> Photo.
    {
            // fd opened only once at the very 1st init
            switch(this->m_HWmodule){
                case DIP_HW_A:
                    gIspDrvObj[this->m_HWmodule].mpIspDipHwRegAddr = (MUINT32 *) mmap(0, DIP_REG_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, DIP_A_BASE_HW);
                    break;
                default:
                    LOG_ERR("#############\n");
                    LOG_ERR("this hw module(%d) is unsupported\n",this->m_HWmodule);
                    LOG_ERR("#############\n");
                    goto EXIT;
                    break;
            }
            //for(MUINT32 i=0;i<MAX_DIP_HW_MODULE;i++){
                if(gIspDrvObj[this->m_HWmodule].mpIspDipHwRegAddr == MAP_FAILED)
                {
                    LOG_ERR("DIP mmap fail (module:0x%x), errno(%d):%s",this->m_HWmodule, errno, strerror(errno));
                    Result = MFALSE;
                    goto EXIT;
                }
            //}

            this->m_regRWMode=ISP_DRV_RW_MMAP;
    }

    switch(this->m_HWmodule){
        case DIP_HW_A:
            resetModule = DIP_DIP_A_IDX;
            break;
        default:
            LOG_ERR("unsupported module:0x%x\n",this->m_HWmodule);
            Result = MFALSE;
            goto EXIT;
            break;
    }
    LOG_DBG("m_HWmodule(%d),resetModule(%d)", this->m_HWmodule, resetModule);

    if(ioctl(this->m_Fd, DIP_RESET_BY_HWMODULE, &resetModule) < 0){
        LOG_ERR("Hw reset fail!, reset hw module: %d, corresponding resetModule: %d\n", this->m_HWmodule, resetModule);
        Result = MFALSE;
        goto EXIT;
    }

    //map reg map for R/W Macro, in order to calculate addr-offset by reg name
    if(this->m_pIspRegMap == NULL){
        this->m_pIspRegMap = (void*)malloc(DIP_REG_RANGE);
    }
    else{
        LOG_ERR("re-allocate RegMap\n");
    }
#if 1
    if(this->loadInitSetting() == MFALSE){
        Result = MFALSE;
        LOG_ERR("load dip hw init setting fail\n");
        goto EXIT;
    }
#endif
    mpTempIspDipHWRegValues = (dip_x_reg_t*)malloc(sizeof(dip_x_reg_t));    //always allocate this(be used to get register offset in macro)
    //
    strncpy((char*)this->m_UserName[this->m_UserCnt],userName,strlen(userName));
    this->m_UserName[this->m_UserCnt][strlen(userName)]='\0';
    tmp = android_atomic_inc(&this->m_UserCnt);

EXIT:

    //
    if (!Result)    // If some init step goes wrong.
    {
        if(this->m_Fd >= 0)
        {
            // unmap to avoid memory leakage
            //for(MUINT32 i=0;i<MAX_DIP_HW_MODULE;i++){
                if(gIspDrvObj[this->m_HWmodule].mpIspDipHwRegAddr != MAP_FAILED){
                    int ret = 0;
                    switch(this->m_HWmodule){
                        case DIP_HW_A:
                            ret = munmap(gIspDrvObj[this->m_HWmodule].mpIspDipHwRegAddr, DIP_REG_RANGE);
                            if (ret < 0) {
                                LOG_ERR("munmap fail: %p\n", gIspDrvObj[this->m_HWmodule].mpIspDipHwRegAddr);
                                break;
                            }
                            gIspDrvObj[this->m_HWmodule].mpIspDipHwRegAddr = NULL;
                            //mpIspDipHwRegAddr = NULL;
                            break;
                        default:
                            LOG_ERR("#############\n");
                            LOG_ERR("this hw module(%d) is unsupported\n",this->m_HWmodule);
                            LOG_ERR("#############\n");
                            break;
                    }
                }
                else
                    gIspDrvObj[this->m_HWmodule].mpIspDipHwRegAddr = NULL;
            //}

            close(this->m_Fd);
            this->m_Fd = -1;
            LOG_INF("close isp device Fd");
        }
    }

    LOG_DBG("-,ret: %d. mInitCount:(%d),m_pIspHwRegAddr_DipA(0x%p)", Result, this->m_UserCnt,gIspDrvObj[this->m_HWmodule].mpIspDipHwRegAddr);
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL PhyDipDrvImp::uninit(const char* userName)
{
    MBOOL Result = MTRUE;
    MINT32 bMatch = -1;
    MINT32 tmp=0;
    //
    android::Mutex::Autolock lock(m_IspInitMutex);
    //
    LOG_INF("-,m_UserCnt(%d),curUser(%s)", this->m_UserCnt,userName);
    //
    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to uninit isp driver\n");
        return MFALSE;
    }

    //
    if(this->m_UserCnt <= 0)
    {
        LOG_ERR("no more user in DipDrv , curUser(%s)",userName);
        goto EXIT;
    }

    for(MUINT32 i=0;i<MAX_USER_NUMBER;i++){
        if(strcmp((const char*)this->m_UserName[i],userName) == 0){
            bMatch = i;
            break;
        }
    }

    if(bMatch==-1){
        LOG_ERR("no matching username:%s\n",userName);
        for(MUINT32 i=0;i<MAX_USER_NUMBER;i+=8)
           LOG_ERR("cur-user: %s,%s,%s,%s,%s,%s,%s,%s\n",\
            this->m_UserName[i],this->m_UserName[i+1],this->m_UserName[i+2],this->m_UserName[i+3],\
            this->m_UserName[i+4],this->m_UserName[i+5],this->m_UserName[i+6],this->m_UserName[i+7]);
        return MFALSE;
    }
    else
        this->m_UserName[bMatch][0] = '\0';

    // More than one user
    tmp = android_atomic_dec(&m_UserCnt);


	/* move last user string content to the one that be matched with currently */
	if(this->m_UserCnt != bMatch) {
        strncpy((char*)m_UserName[bMatch],m_UserName[this->m_UserCnt],strlen(m_UserName[this->m_UserCnt]));
        this->m_UserName[bMatch][strlen(m_UserName[this->m_UserCnt])]='\0';
		m_UserName[this->m_UserCnt][0] = '\0';
	}


    if(this->m_UserCnt > 0)    // If there are still users, exit.
        goto EXIT;
#if 0
    //
    if(this->m_pIspHwRegAddr != MAP_FAILED){
        switch(this->m_HWmodule){
            case CAM_A:
            case CAM_B:
                munmap(this->m_pIspHwRegAddr, CAM_BASE_RANGE);
                this->m_pIspHwRegAddr = NULL;
                break;
            case DIP_A:
                munmap(this->m_pIspHwRegAddr, DIP_BASE_RANGE);
                this->m_pIspHwRegAddr = NULL;
                break;
            default:
                LOG_ERR("#############\n");
                LOG_ERR("this hw module(%d) is unsupported\n",this->m_HWmodule);
                LOG_ERR("#############\n");
                break;
        }
    }
#else   //unmap all at 1 time because of map is binding with fd-open

    //for(MUINT32 i=0;i<MAX_DIP_HW_MODULE;i++){
        if(gIspDrvObj[this->m_HWmodule].mpIspDipHwRegAddr != MAP_FAILED){
            int ret = 0;
            switch(this->m_HWmodule){
                case DIP_HW_A:
                    ret = munmap(gIspDrvObj[this->m_HWmodule].mpIspDipHwRegAddr, DIP_REG_RANGE);
                    if (ret < 0) {
                        LOG_ERR("munmap fail: %p\n", gIspDrvObj[this->m_HWmodule].mpIspDipHwRegAddr);
                        break;
                    }
                    gIspDrvObj[this->m_HWmodule].mpIspDipHwRegAddr = NULL;
                    //mpIspDipHwRegAddr = NULL;
                    break;
                default:
                    LOG_ERR("#############\n");
                    LOG_ERR("this hw module(%d) is unsupported\n",this->m_HWmodule);
                    LOG_ERR("#############\n");
                    break;
            }
        }
        else
            gIspDrvObj[this->m_HWmodule].mpIspDipHwRegAddr = NULL;

    //}
#endif


    if(mpTempIspDipHWRegValues != NULL)
    {
        free((MUINT32*)mpTempIspDipHWRegValues);
        mpTempIspDipHWRegValues = NULL;
    }


    //
    if(this->m_pIspRegMap){
        free((MUINT32*)this->m_pIspRegMap);
        this->m_pIspRegMap = NULL;
    }


    //
    if(this->m_Fd >= 0)
    {
        close(this->m_Fd);
        this->m_Fd = -1;
        this->m_regRWMode=ISP_DRV_R_ONLY;
    }

    //
EXIT:

    LOG_INF(" - X. ret: %d. m_UserCnt: %d.", Result, this->m_UserCnt);

    if(this->m_UserCnt!= 0){
        for(MINT32 i=0;i<MAX_USER_NUMBER;i+=8) {
            if(this->m_UserCnt > i) {
                LOG_INF("current user[%d]: %s, %s, %s, %s, %s, %s, %s, %s\n"    \
                ,i,this->m_UserName[i],this->m_UserName[i+1],this->m_UserName[i+2],this->m_UserName[i+3]  \
                ,this->m_UserName[i+4],this->m_UserName[i+5],this->m_UserName[i+6],this->m_UserName[i+7]);
            }
        }
    }
    return Result;
}

MBOOL PhyDipDrvImp::waitIrq(DIP_WAIT_IRQ_ST* pWaitIrq)
{
    MINT32 Ret = 0;
    MUINT32 OrgTimeOut;
    DIP_IRQ_CLEAR_ENUM OrgClr;
    IspDbgTimer dbgTmr("waitIrq");
    MUINT32 Ta=0,Tb=0;
    DIP_WAIT_IRQ_STRUCT wait;
    LOG_DBG(" - E. hwmodule:0x%x,Status(0x%08x),Timeout(%d).\n",this->m_HWmodule, pWaitIrq->Status, pWaitIrq->Timeout);
    if(FD_CHK() == -1){
        return MFALSE;
    }

    OrgTimeOut = pWaitIrq->Timeout;
    OrgClr = pWaitIrq->Clear;
    switch(this->m_HWmodule){
        case DIP_HW_A:     wait.Type = DIP_IRQ_TYPE_INT_DIP_A_ST;
            break;
        default:
            LOG_ERR("unsupported hw module:0x%x\n",this->m_HWmodule);
            return MFALSE;
            break;
    }

    memcpy(&wait.EventInfo,pWaitIrq,sizeof(DIP_WAIT_IRQ_ST));


    do{
        Ta=dbgTmr.getUs();
        Ret = ioctl(this->m_Fd,DIP_WAIT_IRQ,&wait);
        Tb=dbgTmr.getUs();

        //receive restart system call signal
        if( (Ret== (-SIG_ERESTARTSYS)) && (wait.EventInfo.Timeout > 0)){
            wait.EventInfo.Timeout=wait.EventInfo.Timeout - ((Tb-Ta)/1000);
            wait.EventInfo.Clear = DIP_IRQ_CLEAR_NONE;
            LOG_INF("ERESTARTSYS,Type(%d),Status(0x%08x),Timeout(%d us)\n",wait.Type, pWaitIrq->Status, wait.EventInfo.Timeout);
        }
        else
            break;
    }while(1);


    //memcpy(&pWaitIrq->TimeInfo,&wait.EventInfo.TimeInfo,sizeof(DIP_IRQ_TIME_STRUCT));
    pWaitIrq->Timeout = OrgTimeOut;
    pWaitIrq->Clear = OrgClr;


    if(Ret < 0) {
        LOG_ERR("ISP(0x%x)_WAIT_IRQ fail(%d). Wait Status(0x%08x), Timeout(%d).\n",this->m_HWmodule, Ret,  pWaitIrq->Status, pWaitIrq->Timeout);
        return MFALSE;
    }


    return MTRUE;
}

MBOOL PhyDipDrvImp::clearIrq(DIP_CLEAR_IRQ_ST* pClearIrq)
{
    MINT32 Ret;
    DIP_CLEAR_IRQ_STRUCT clear;
    //
    LOG_DBG(" - E. hw module:0x%x,user(%d), Status(%d)\n",this->m_HWmodule,pClearIrq->UserKey, pClearIrq->Status);
    if(FD_CHK() == -1){
        return MFALSE;
    }
    switch(this->m_HWmodule){
        case DIP_HW_A:     clear.Type = DIP_IRQ_TYPE_INT_DIP_A_ST;
            break;
        default:
            LOG_ERR("unsupported hw module:0x%x\n",this->m_HWmodule);
            return MFALSE;
            break;
    }
    //
    memcpy(&clear.EventInfo,pClearIrq,sizeof(DIP_CLEAR_IRQ_ST));
    Ret = ioctl(this->m_Fd,DIP_CLEAR_IRQ,&clear);
    if(Ret < 0)
    {
        LOG_ERR("ISP(0x%x)_CLEAR_IRQ fail(%d)\n",this->m_HWmodule,Ret);
        return MFALSE;
    }
    return MTRUE;
}

MBOOL PhyDipDrvImp::signalIrq(DIP_WAIT_IRQ_ST* pWaitIrq)
{
    LOG_INF("IRQ SIGNAL:hw module:0x%x, userKey:0x%x, status:0x%x",this->m_HWmodule,pWaitIrq->UserKey,pWaitIrq->Status);
    DIP_WAIT_IRQ_STRUCT wait;

    memcpy(&wait.EventInfo,pWaitIrq,sizeof(DIP_WAIT_IRQ_ST));
    switch(this->m_HWmodule){
        case DIP_HW_A:
            wait.Type = DIP_IRQ_TYPE_INT_DIP_A_ST;
            break;
        default:
            LOG_ERR("unsupported hw hwModule:0x%x\n",this->m_HWmodule);
            return MFALSE;
            break;
    }

    if(ioctl(this->m_Fd,DIP_FLUSH_IRQ_REQUEST,&wait) < 0){
        LOG_ERR("signal IRQ fail(irq:0x%x,status:0x%x)",wait.Type,wait.EventInfo.Status);
        return MFALSE;
    }
    return MTRUE;
}


MUINT32 PhyDipDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    MINT32 Ret=0;
    MUINT32 value=0x0;
    MUINT32 legal_range = DIP_REG_RANGE;
    LOG_DBG("+,Isp_read:m_HWmodule(0x%x),Addr(0x%x)\n",this->m_HWmodule,Addr);
    android::Mutex::Autolock lock(this->IspRegMutex);
    (void)caller;
    if(FD_CHK() == -1){
        return 1;
    }


    if(this->m_regRWMode==ISP_DRV_RW_MMAP){
        legal_range = DIP_REG_RANGE;
        if(Addr >= legal_range){
            LOG_ERR("over range(0x%x)\n",Addr);
            return 0;
        }
        value = this->mpIspDipHwRegAddr[(Addr>>2)];
    }
    else{
        DIP_REG_IO_STRUCT IspRegIo;
        ISP_DRV_REG_IO_STRUCT RegIo;
        RegIo.module = this->m_HWmodule;
        RegIo.Addr = Addr;
        IspRegIo.pData = (DIP_REG_STRUCT*)&RegIo;
        IspRegIo.Count = 1;

        Ret = ioctl(this->m_Fd, DIP_READ_REGISTER, &IspRegIo);
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
MBOOL PhyDipDrvImp::readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    MUINT32 legal_range = DIP_REG_RANGE;
    android::Mutex::Autolock lock(this->IspRegMutex);
    (void)caller;
    if(FD_CHK() == -1){
        return MFALSE;
    }

    if(this->m_regRWMode == ISP_DRV_RW_MMAP){
        unsigned int i=0;
        legal_range = DIP_REG_RANGE;
        do{
            if(pRegIo[i].Addr >= legal_range){
                LOG_ERR("over range,bypass_0x%x\n",pRegIo[i].Addr);
                i = Count;
            }
            else
                pRegIo[i].Data = this->mpIspDipHwRegAddr[(pRegIo[i].Addr>>2)];
        }while(++i<Count);
    }
    else{
        DIP_REG_IO_STRUCT IspRegIo;
        pRegIo->module = this->m_HWmodule;
        IspRegIo.pData = (DIP_REG_STRUCT*)pRegIo;
        IspRegIo.Count = Count;


        Ret = ioctl(this->m_Fd, DIP_READ_REGISTER, &IspRegIo);
        if(Ret < 0)
        {
            LOG_ERR("ISP(0x%x)_READ via IO fail(%d)",this->m_HWmodule,Ret);
            return MFALSE;
        }
    }

    LOG_DBG("Isp_reads_0x%x(%d): 0x%x_0x%x",this->m_HWmodule,Count,pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;
}


MBOOL PhyDipDrvImp::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    MINT32 ret=0;
    MUINT32 legal_range = DIP_REG_RANGE;
    LOG_DBG("Isp_write:m_HWmodule(0x%x),m_regRWMode(0x%x),(0x%x,0x%lx)",this->m_HWmodule,this->m_regRWMode,Addr,Data);
    android::Mutex::Autolock lock(this->IspRegMutex);
    (void)caller;
    if(FD_CHK() == -1){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case ISP_DRV_RW_MMAP:
            legal_range = DIP_REG_RANGE;
            if(Addr >= legal_range){
                LOG_ERR("over range(0x%x)\n",Addr);
                return MFALSE;
            }
            this->mpIspDipHwRegAddr[(Addr>>2)] = Data;
            break;
        case ISP_DRV_RW_IOCTL:
            DIP_REG_IO_STRUCT IspRegIo;
            ISP_DRV_REG_IO_STRUCT RegIo;
            RegIo.Addr = Addr;
            RegIo.Data = Data;
            IspRegIo.pData = (DIP_REG_STRUCT*)&RegIo;
            IspRegIo.Count = 1;
            ret = ioctl(this->m_Fd, DIP_WRITE_REGISTER, &IspRegIo);
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

MBOOL PhyDipDrvImp::writeRegs(MUINT32 DstOffsetAddr,MUINT32 Count,MUINT32* SrcAddr, MINT32 caller)
{

    return MTRUE;
}
MBOOL PhyDipDrvImp::writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    unsigned int i=0;
    MUINT32 legal_range = DIP_REG_RANGE;
    android::Mutex::Autolock lock(this->IspRegMutex);
    (void)caller;
    if(FD_CHK() == -1){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case ISP_DRV_RW_IOCTL:
            DIP_REG_IO_STRUCT IspRegIo;
            IspRegIo.pData = (DIP_REG_STRUCT*)pRegIo;
            IspRegIo.Count = Count;

            Ret = ioctl(this->m_Fd, DIP_WRITE_REGISTER, &IspRegIo);
            if(Ret < 0){
                LOG_ERR("ISP(0x%x)_WRITE via IO fail(%d)",this->m_HWmodule,Ret);
                return MFALSE;
            }
            break;
        case ISP_DRV_RW_MMAP:
            legal_range = DIP_REG_RANGE;
            do{
                if(pRegIo[i].Addr >= legal_range){
                    LOG_ERR("over range,bypass_0x%x\n",pRegIo[i].Addr);
                    i = Count;
                }
                else
                    this->mpIspDipHwRegAddr[(pRegIo[i].Addr>>2)] = pRegIo[i].Data;
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

MUINT32 PhyDipDrvImp::getRWMode(void)
{
    return this->m_regRWMode;
}

//for early porting use only,  not regular path.
MBOOL PhyDipDrvImp::setRWMode(ISP_DRV_RW_MODE rwMode)
{
    if(rwMode > ISP_DRV_RW_CQ)
    {
        LOG_ERR("no reg operation mode(0x%x)",this->m_regRWMode);
        return MFALSE;
    }

    this->m_regRWMode = rwMode;

    return MTRUE;
}

MBOOL PhyDipDrvImp::setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData)
{
    DIP_P2_BUFQUE_STRUCT p2bufQ;

    switch(eCmd){
        case _SET_DBG_INT:
            if(ioctl(this->m_Fd,DIP_DEBUG_FLAG,(unsigned char*)pData) < 0){
                LOG_ERR("kernel log enable error\n");
                return MFALSE;
            }
            break;
        case _SET_DIP_BUF_INFO:
            p2bufQ.ctrl = static_cast<DIP_P2_BUFQUE_CTRL_ENUM>(pData[0]);
            p2bufQ.property = static_cast<DIP_P2_BUFQUE_PROPERTY>(pData[1]);
            p2bufQ.processID = 0;//static_cast<MUINT32>(pData[2]);
            p2bufQ.callerID = static_cast<MUINT32>(pData[3]) | (static_cast<MUINT32>(pData[4])<<8) | (static_cast<MUINT32>(pData[5])<<16) | (static_cast<MUINT32>(pData[6])<<24);
            p2bufQ.frameNum = static_cast<MINT32>(pData[7]);
            p2bufQ.cQIdx = static_cast<MINT32>(pData[8]);
            p2bufQ.dupCQIdx = static_cast<MINT32>(pData[9]);
            p2bufQ.burstQIdx = static_cast<MINT32>(pData[10]);
            p2bufQ.timeoutIns = static_cast<MUINT32>(pData[11]);
            LOG_DBG("p2bufQ(%d_%d_%d_0x%x_%d_%d_%d_%d_%d)",p2bufQ.ctrl,p2bufQ.property,p2bufQ.processID,p2bufQ.callerID,\
                p2bufQ.frameNum,p2bufQ.cQIdx ,p2bufQ.dupCQIdx,p2bufQ.burstQIdx,p2bufQ.timeoutIns);
            if(ioctl(this->m_Fd,DIP_P2_BUFQUE_CTRL,&p2bufQ) < 0){
                LOG_ERR("ISP_P2_BUFQUE_CTRL(%d) error\n",p2bufQ.ctrl);
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

MBOOL PhyDipDrvImp::loadInitSetting(void)
{

    LOG_INF("loadInitSetting size(%d)",ISP_DIP_INIT_SETTING_COUNT);
    this->writeRegs(mIspDipInitReg, ISP_DIP_INIT_SETTING_COUNT, DIP_HW_A);

    return MTRUE;
}

dip_x_reg_t* PhyDipDrvImp::getCurHWRegValues()
{

    MUINT32 size=sizeof(dip_x_reg_t);
    MUINT32* startAddr= this->mpIspDipHwRegAddr;
    //startAddr+=0x4000;
    //LOG_INF("isp_reg_t size(0x%x),starting Addr(0x%x),size/sizeof(MUINT32) (0x%x),0x4000Addr(0x%x)\n",size,getMMapRegAddr(),size/sizeof(MUINT32),startAddr);

    for(MUINT32 i=0;i<size/sizeof(MUINT32);i++)
    {
         //LOG_INF("addr(0x%x) value (0x%x)\n",((MUINT32*)startAddr+i),*((MUINT32*)startAddr+i));
         *((MUINT32*)mpTempIspDipHWRegValues+i)=*((MUINT32*)startAddr+i);
    }
    return (mpTempIspDipHWRegValues);

}
MBOOL PhyDipDrvImp::getDipDumpInfo(MUINT32& tdriaddr, MUINT32& cmdqaddr, MUINT32& imgiaddr)
{
    DIP_GET_DUMP_INFO_STRUCT get_dumpinfo;

    if(ioctl(this->m_Fd,DIP_GET_DUMP_INFO,&get_dumpinfo) < 0){
        LOG_ERR("get dump info fail \n");
    }
    tdriaddr = (get_dumpinfo.tdri_baseaddr & 0xFFFF000);
    cmdqaddr = get_dumpinfo.cmdq_baseaddr;
    imgiaddr = get_dumpinfo.imgi_baseaddr;
    return MTRUE;

}

MBOOL PhyDipDrvImp::dumpDipHwReg(IspDumpDbgLogDipPackage* pP2Package)
{
    dip_x_reg_t dipReg;
    MUINT32 i;
    MUINT32 dumpTpipeLine;
    MUINT32 regTpipePA;
    DIP_DUMP_BUFFER_STRUCT dumpBufStruct;
    MUINT32 size=sizeof(dip_x_reg_t);
    DIP_GET_DUMP_INFO_STRUCT get_dumpinfo;

    //dump top regs
    LOG_INF("DIP_X_CTL_START=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_START-(MUINT8*)&dipReg)), DIP_HW_A));
	LOG_INF("DIP_X_CTL_YUV_EN=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_YUV_EN-(MUINT8*)&dipReg)), DIP_HW_A));
	LOG_INF("DIP_X_CTL_YUV2_EN=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_YUV2_EN-(MUINT8*)&dipReg)), DIP_HW_A));
	LOG_INF("DIP_X_CTL_RGB_EN=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_RGB_EN-(MUINT8*)&dipReg)), DIP_HW_A));
	LOG_INF("DIP_X_CTL_DMA_EN=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_DMA_EN-(MUINT8*)&dipReg)), DIP_HW_A));
	LOG_INF("DIP_X_CTL_FMT_SEL=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_FMT_SEL-(MUINT8*)&dipReg)), DIP_HW_A));
	LOG_INF("DIP_X_CTL_PATH_SEL=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_PATH_SEL-(MUINT8*)&dipReg)), DIP_HW_A));
	LOG_INF("DIP_X_CTL_MISC_SEL=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_MISC_SEL-(MUINT8*)&dipReg)), DIP_HW_A));
	LOG_INF("DIP_X_CTL_TDR_CTL=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_TDR_CTL-(MUINT8*)&dipReg)), DIP_HW_A));
	LOG_INF("DIP_X_CTL_TDR_TILE=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_TDR_TILE-(MUINT8*)&dipReg)), DIP_HW_A));
	LOG_INF("DIP_X_CTL_TDR_TCM_EN=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_TDR_TCM_EN-(MUINT8*)&dipReg)), DIP_HW_A));
	LOG_INF("DIP_X_CTL_TDR_TCM2_EN=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_TDR_TCM2_EN-(MUINT8*)&dipReg)), DIP_HW_A));
	LOG_INF("DIP_X_CTL_TDR_TCM3_EN=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_TDR_TCM3_EN-(MUINT8*)&dipReg)), DIP_HW_A));

	//dump tpipe
    regTpipePA = readReg((MUINT32)((MUINT8*)&dipReg.DIP_X_TDRI_BASE_ADDR-(MUINT8*)&dipReg));
    LOG_INF("[Tpipe]va(0x%lx),pa(0x%08x),regPa(0x%08x)",(unsigned long)pP2Package->tpipeTableVa,pP2Package->tpipeTablePa,regTpipePA);
    dumpTpipeLine = DUMP_TPIPE_SIZE / DUMP_TPIPE_NUM_PER_LINE;
    for(i=0;i<dumpTpipeLine;i++){
        LOG_INF("[Tpipe](%02d)-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x",i,
            pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+1],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+2],
            pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+3],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+4],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+5],
            pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+6],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+7],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+8],
            pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+9]);
    }


    //dump all dip registers
    for(i=0x0;i<=size;i+=16){
        LOG_INF("(0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)",
            0x15022000+i,readReg(i, DIP_HW_A),0x15022000+i+4,readReg(i+4, DIP_HW_A),0x15022000+i+8,readReg(i+8, DIP_HW_A),0x15022000+i+12,readReg(i+12, DIP_HW_A));
    }

    if(ioctl(this->m_Fd,DIP_GET_DUMP_INFO,&get_dumpinfo) < 0){
        LOG_ERR("get dump info fail \n");
    }
    //0xffff0000 chip dependent, sizeof = 256x256 = 0x10000
    if ((get_dumpinfo.tdri_baseaddr & DIP_TDRI_ADDR_MASK) == (pP2Package->tpipeTablePa & DIP_TDRI_ADDR_MASK))
    {
        if ((get_dumpinfo.imgi_baseaddr != DIP_DUMP_ADDR_MASK) && (get_dumpinfo.imgi_baseaddr == pP2Package->IspVirRegAddrVa[DIP_IMBI_BASEADDR_OFFSET]))
        {
            m_IsEverDumpBuffer = MFALSE;
            LOG_INF("imgi baseaddr is the same as kernel imgi base address, get_dumpinfo.imgi_baseaddr:0x%x, pP2Package->IspVirRegAddrVa[256]:0x%x",get_dumpinfo.imgi_baseaddr ,pP2Package->IspVirRegAddrVa[256]);
        }
        else
        {
            LOG_INF("imgi baseaddr is not the same as kernel imgi base address, get_dumpinfo.imgi_baseaddr:0x%x, pP2Package->IspVirRegAddrVa[256]:0x%x",get_dumpinfo.imgi_baseaddr ,pP2Package->IspVirRegAddrVa[256]);
        }
        m_IsEverDumpBuffer = MFALSE;
    }

    LOG_INF("QQ Tunning buffer address : 0x%lx, tpipeTablePa: 0x%x, tpipeTableVa: 0x%lx, m_IsEverDumpBuffer:%d, tdri(0x%x), imgi(0x%x), dmgi(0x%x)",(unsigned long)pP2Package->pTuningQue, pP2Package->tpipeTablePa, (unsigned long)pP2Package->tpipeTableVa, m_IsEverDumpBuffer, get_dumpinfo.tdri_baseaddr, get_dumpinfo.imgi_baseaddr, get_dumpinfo.dmgi_baseaddr);

    if (MFALSE == m_IsEverDumpBuffer)
    {
        dumpBufStruct.DumpCmd = DIP_DUMP_TPIPEBUF_CMD;
        dumpBufStruct.BytesofBufferSize = MAX_ISP_TILE_TDR_HEX_NO;
        dumpBufStruct.pBuffer = pP2Package->tpipeTableVa;
        if(ioctl(this->m_Fd,DIP_DUMP_BUFFER,&dumpBufStruct) < 0){
            LOG_ERR("dump tpipe buffer fail, size:0x%x\n", dumpBufStruct.BytesofBufferSize);
        }

        dumpBufStruct.DumpCmd = DIP_DUMP_DIPVIRBUF_CMD;
        dumpBufStruct.BytesofBufferSize = size;
        dumpBufStruct.pBuffer = pP2Package->IspVirRegAddrVa;
        if(ioctl(this->m_Fd,DIP_DUMP_BUFFER,&dumpBufStruct) < 0){
            LOG_ERR("dump vir isp buffer fail, size:0x%x\n", dumpBufStruct.BytesofBufferSize);
        }

        dumpBufStruct.DumpCmd = DIP_DUMP_CMDQVIRBUF_CMD;
        dumpBufStruct.BytesofBufferSize = (GET_MAX_CQ_DESCRIPTOR_SIZE()/ISP_DIP_CQ_DUMMY_BUFFER+1)*ISP_DIP_CQ_DUMMY_BUFFER;
        dumpBufStruct.pBuffer = pP2Package->IspDescriptVa;
        if(ioctl(this->m_Fd,DIP_DUMP_BUFFER,&dumpBufStruct) < 0){
            LOG_ERR("dump cmdq buffer fail, size:0x%x\n", dumpBufStruct.BytesofBufferSize);
        }

    }

    if (pP2Package->pTuningQue != NULL)
    {
        LOG_INF("QQ count : %u", (MUINT32)(size/sizeof(MUINT32)));
        unsigned int *pTuningBuf = (unsigned int *)pP2Package->pTuningQue;
        if (MFALSE == m_IsEverDumpBuffer)
        {
            dumpBufStruct.DumpCmd = DIP_DUMP_TUNINGBUF_CMD;
            dumpBufStruct.BytesofBufferSize = size;
            dumpBufStruct.pBuffer = (unsigned int *)pP2Package->pTuningQue;
            if(ioctl(this->m_Fd,DIP_DUMP_BUFFER,&dumpBufStruct) < 0){
                LOG_ERR("dump tuning buffer fail, size:0x%x\n", dumpBufStruct.BytesofBufferSize);
            }
        }
        MUINT32 dip_size=size;
        if ((dip_size & 0x7) > 0)
        {
            size = ((size>>3)<<3);
        }

        for(i=0x0;i<size;i+=32){
            LOG_INF("QQ (0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)",
                0x15022000+i,pTuningBuf[(i>>2)],0x15022000+i+4,pTuningBuf[(i>>2)+1],0x15022000+i+8,pTuningBuf[(i>>2)+2],0x15022000+i+12,pTuningBuf[(i>>2)+3],0x15022000+i+16,pTuningBuf[(i>>2)+4],0x15022000+i+20,pTuningBuf[(i>>2)+5],0x15022000+i+24,pTuningBuf[(i>>2)+6],0x15022000+i+28,pTuningBuf[(i>>2)+7]);
        }
        if ((dip_size-size)>0)
        {
            for(i=size;i<dip_size;i=i+4){
                LOG_INF("res QQ (0x%08x,0x%08x)",
                    0x15022000+i,pTuningBuf[(i>>2)]);
            }
        }


        for(i=0;i<size/sizeof(MUINT32);i++)
            pP2Package->pTuningQue[i] = readReg(i*4,DIP_HW_A);

    }
    m_IsEverDumpBuffer = MTRUE;

    return MTRUE;
}

MBOOL PhyDipDrvImp::setMemInfo(unsigned int meminfocmd, unsigned long tpipeTablePa, unsigned int *tpipeTableVa, unsigned int MemSizeDiff)
{
    DIP_MEM_INFO_STRUCT TpipeMemInfoStruct;

    TpipeMemInfoStruct.MemInfoCmd = meminfocmd;
    TpipeMemInfoStruct.MemPa = tpipeTablePa;
    TpipeMemInfoStruct.MemVa = tpipeTableVa;
    TpipeMemInfoStruct.MemSizeDiff = MemSizeDiff;

    if(ioctl(this->m_Fd,DIP_SET_MEM_INFO,&TpipeMemInfoStruct) < 0){
        LOG_ERR("set tpipe mem info fail, cmd:0x%x, memPa:0x%lx, memVa:0x%lx, memSizeDiff:0x%x\n",
            TpipeMemInfoStruct.MemInfoCmd, (unsigned long)TpipeMemInfoStruct.MemPa, (unsigned long)TpipeMemInfoStruct.MemVa, TpipeMemInfoStruct.MemSizeDiff);
    }

    return MTRUE;
}



