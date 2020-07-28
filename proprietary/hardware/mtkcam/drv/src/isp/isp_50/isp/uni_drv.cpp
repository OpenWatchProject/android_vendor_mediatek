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
#define LOG_TAG "UniDrv"

#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>

#include "camera_isp.h"
#include <mtkcam/def/common.h>
#include <uni_drv.h>


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(uni_drv);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (uni_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (uni_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (uni_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (uni_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (uni_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (uni_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define UNUSED(expr) do { (void)(expr); } while (0)

UniDrvImp::UniDrvImp()
{
    DBG_LOG_CONFIG(drv, uni_drv);
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    m_UserCnt = 0;

    m_pIspHwRegAddr = NULL;
    pIspdrvimp = NULL;
    m_HWmodule = MAX_UNI_HW_MODULE;
}


static UniDrvImp    gUniDrvObj[MAX_UNI_HW_MODULE];


IspDrv* UniDrvImp::createInstance(UNI_HW_MODULE module)
{
    gUniDrvObj[module].m_HWmodule = module;

    return (IspDrv*)&gUniDrvObj[module];
}

MBOOL UniDrvImp::init(const char* userName)
{
    UNUSED(userName);
    LOG_ERR("Please pass correct IspDrv obj, pConnectedObj, for uni link\n");
    return MFALSE;
}

MBOOL UniDrvImp::init(const char* userName, IspDrvImp* pConnectedObj)
{
    MBOOL Result = MTRUE;
    MINT32 tmp;
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    LOG_INF(" - E. m_UserCnt(%d), curUser(%s).", this->m_UserCnt,userName);
    //
    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to use isp driver\n");
        return MFALSE;
    }
    //
    if(this->m_UserCnt > 0)
    {
        tmp = android_atomic_inc(&this->m_UserCnt);
        LOG_INF(" - X. m_UserCnt: %d.", this->m_UserCnt);
        return Result;
    }

    this->pIspdrvimp = pConnectedObj;

    if(this->pIspdrvimp->init("UNI DRV") == MFALSE){
        Result = MFALSE;
        goto EXIT;
    }

    //check wheather kernel is existed or not
    if (this->m_Fd < 0)    // 1st time open failed.
    {
        LOG_ERR("ISP kernel is not existed\n");
        Result = MFALSE;
        goto EXIT;
    }
    else    // 1st time open success.   // Sometimes GDMA will go this path, too. e.g. File Manager -> Phone Storage -> Photo.
    {
        // mmap isp reg
        switch(this->m_HWmodule){
            case UNI_A:
                this->m_pIspHwRegAddr = (MUINT32 *) mmap(0, UNI_BASE_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, UNI_A_BASE_HW);
                break;
            default:
                LOG_ERR("#############\n");
                LOG_ERR("this hw module(%d) is unsupported\n",this->m_HWmodule);
                LOG_ERR("#############\n");
                goto EXIT;
                break;
        }
        if(this->m_pIspHwRegAddr == MAP_FAILED)
        {
            LOG_ERR("UNI mmap fail (module:0x%x), errno(%d):%s",this->m_HWmodule, errno, strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }
    }

    //
    tmp = android_atomic_inc(&this->m_UserCnt);

EXIT:

    LOG_INF(" - X. ret: %d. mInitCount: %d.", Result, this->m_UserCnt);
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL UniDrvImp::uninit(const char* userName)
{
    MBOOL Result = MTRUE;
    MINT32 tmp=0;
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    LOG_INF(" - E. m_UserCnt(%d), curUser(%s)", this->m_UserCnt,userName);
    //
    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to uninit isp driver\n");
        return MFALSE;
    }

    //
    if(this->m_UserCnt <= 0)
    {
        LOG_ERR("no more user in UniDrv , curUser(%s)",userName);
        goto EXIT;
    }
    // More than one user
    tmp = android_atomic_dec(&this->m_UserCnt);

    if(this->m_UserCnt > 0)    // If there are still users, exit.
        goto EXIT;

    //
    if(this->m_pIspHwRegAddr != MAP_FAILED){
        int ret = 0;
        switch(this->m_HWmodule){
            case UNI_A:
                ret = munmap(this->m_pIspHwRegAddr, UNI_BASE_RANGE);
                if (ret < 0) {
                    LOG_ERR("munmap fail: %p\n", this->m_pIspHwRegAddr);
                    break;
                }
                this->m_pIspHwRegAddr = NULL;
                break;
            default:
                LOG_ERR("#############\n");
                LOG_ERR("this hw module(%d) is unsupported\n",this->m_HWmodule);
                LOG_ERR("#############\n");
                break;
        }
    }


    //
    if(this->pIspdrvimp->uninit("UNI DRV") == MFALSE){
        Result = MFALSE;
        goto EXIT;
    }
    this->pIspdrvimp = NULL;
EXIT:

    LOG_INF(" - X. ret: %d. m_UserCnt: %d.", Result, this->m_UserCnt);
    return Result;
}

#define FD_CHK()({\
    MINT32 Ret=0;\
    if(this->m_Fd < 0){\
        LOG_ERR("no isp device");\
        Ret = -1;\
    }\
    Ret;\
})

void UniDrvImp::destroyInstance(void)
{
}

MUINT32 UniDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    MINT32 Ret=0;
    MUINT32 value=0x0;
    LOG_DBG("Isp_read:0x%x_0x%x",this->m_HWmodule,Addr);
    android::Mutex::Autolock lock(this->IspRegMutex);
    (void)caller;
    if(FD_CHK() == -1){
        return value;
    }

    if(this->pIspdrvimp->getRWMode() ==ISP_DRV_RW_MMAP){
        value = this->m_pIspHwRegAddr[(Addr>>2)];
    }
    else{
        ISP_REG_IO_STRUCT IspRegIo;
        ISP_DRV_REG_IO_STRUCT RegIo;
        RegIo.module = this->m_HWmodule;
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
    LOG_DBG("Isp_read:0x%x_0x%x",Addr,value);
    return value;
}

//-----------------------------------------------------------------------------
MBOOL UniDrvImp::readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    android::Mutex::Autolock lock(this->IspRegMutex);
    (void)caller;
    if(FD_CHK() == -1){
        return MFALSE;
    }

    if(this->pIspdrvimp->getRWMode() == ISP_DRV_RW_MMAP){
        unsigned int i=0;
        do{
            pRegIo[i].Data = this->m_pIspHwRegAddr[(pRegIo[i].Addr>>2)];
        }while(++i<Count);
    }
    else{
        ISP_REG_IO_STRUCT IspRegIo;
        pRegIo->module = this->m_HWmodule;
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

MBOOL UniDrvImp::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    MINT32 ret=0;
    LOG_DBG("Isp_write:0x%x_0x%x_0x%lx",this->m_HWmodule,Addr,Data);
    android::Mutex::Autolock lock(this->IspRegMutex);
    (void)caller;
    if(FD_CHK() == -1){
        return MFALSE;
    }

    switch(this->pIspdrvimp->getRWMode()){
        case ISP_DRV_RW_MMAP:
            this->m_pIspHwRegAddr[(Addr>>2)] = Data;
            break;
        case ISP_DRV_RW_IOCTL:
            ISP_REG_IO_STRUCT IspRegIo;
            ISP_DRV_REG_IO_STRUCT RegIo;
            RegIo.module = this->m_HWmodule;
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
            LOG_ERR("no reg operation mode(0x%x)",this->pIspdrvimp->getRWMode());
            return MFALSE;
            break;
    }
    //
    //release mutex in order to read back for DBG log
    this->IspRegMutex.unlock();
    LOG_DBG("Isp_write_0x%x:0x%x_0x%lx(0x%x)",this->m_HWmodule,Addr,Data,this->readReg(Addr));
    return MTRUE;
}

MBOOL UniDrvImp::writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    unsigned int i=0;
    android::Mutex::Autolock lock(this->IspRegMutex);
    (void)caller;
    if(FD_CHK() == -1){
        return MFALSE;
    }

    switch(this->pIspdrvimp->getRWMode()){
        case ISP_DRV_RW_IOCTL:
            ISP_REG_IO_STRUCT IspRegIo;
            pRegIo->module = this->m_HWmodule;
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
                this->m_pIspHwRegAddr[(pRegIo[i].Addr>>2)] = pRegIo[i].Data;
            }while(++i<Count);
            break;
        case ISP_DRV_R_ONLY:
            LOG_ERR("ISP Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no reg operation mode(0x%x)",this->pIspdrvimp->getRWMode());
            return MFALSE;
            break;
    }
    LOG_DBG("Isp_writes_0x%x(%d):0x%x_0x%x\n",this->m_HWmodule,Count,pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;

}

UNI_HW_MODULE UniDrvImp::getCurObjInfo(void)
{
    return this->m_HWmodule;
}

MBOOL UniDrvImp::DumpReg(void)
{
    char _tmpchr[16] = "\0";
    char _chr[256] = "\0";
    MUINT32 shift;
    MUINT32 count = 0;

    switch(this->m_HWmodule){
        case UNI_A:
            shift = 0x3000;
            break;
        default:
            LOG_ERR("unsupported module:0x%x\n",this->m_HWmodule);
            return MFALSE;
            break;
    }

    LOG_INF("###################\n");
    LOG_INF("start dump phy reg\n");
    for(MUINT32 i=0x0;i<UNI_BASE_RANGE;i+=0x20){
        _chr[0] = '\0';
        sprintf(_chr,"0x%x: ",shift + i);
        for(MUINT32 j=i;j<(i+0x20);j+=0x4){
            if(j >= UNI_BASE_RANGE)
                break;
            _tmpchr[0] = '\0';
            sprintf(_tmpchr,"0x%8x - ",this->readReg(j));
            count = sizeof(_chr) - strlen(_chr);
            strncat(_chr,_tmpchr,count-1);
        }
        LOG_INF("%s\n",_chr);
    }
    LOG_INF("###################\n");
    return MTRUE;
}



