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
#define LOG_TAG "CamsvResMgr"

//
#include <mtkcam/utils/std/Log.h>

//
#include <cutils/properties.h>
#include "Camsv_ResMgr.h"
#include "cam_capibility.h"

////////////////////////////////////////////////////////////////////////////////


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT                      // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

DECLARE_DBG_LOG_VARIABLE(camsvResMgr);

#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_ERR
#undef PIPE_WRN

#define PIPE_DBG(fmt, arg...)        do {\
    if (camsvResMgr_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG("[0x%x][%s]:" fmt,this->m_hwModule,this->m_User, ##arg); \
    }\
} while(0)

#define PIPE_INF(fmt, arg...)        do {\
    if (camsvResMgr_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF("[0x%x][%s]:" fmt,this->m_hwModule,this->m_User, ##arg); \
    }\
} while(0)

#define PIPE_WRN(fmt, arg...)        do {\
        if (camsvResMgr_DbgLogEnable_ERROR  ) { \
            BASE_LOG_WRN("[0x%x][%s]:" fmt,this->m_hwModule,this->m_User, ##arg); \
        }\
    } while(0)


#define PIPE_ERR(fmt, arg...)        do {\
    if (camsvResMgr_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[0x%x][%s]:" fmt,this->m_hwModule,this->m_User, ##arg); \
    }\
} while(0)

MBOOL Central_Camsv::get_availNum(vector<ISP_HW_MODULE>& v_available,char Name[32])
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mLock);
    MUINT32 availsize = this->m_available;

    if(*Name == '\0'){
        BASE_LOG_ERR("cannot have NULL userName\n");
        return MFALSE;
    }

    v_available.clear();
    for(MUINT32 i = 0 ; i < CAMSV_MAX - CAMSV_START ; i++){
        if(this->m_module[i].occupied == MFALSE){
            v_available.push_back((ISP_HW_MODULE)i);
        }
    }

    if(v_available.size() != availsize){
        #define str_size (128)
        char str[str_size];
        int cx = 0;
        *str = '\0';
        BASE_LOG_ERR("size mismatch : %zu%d\n", v_available.size(), availsize);
        for(MUINT32 i = 0 ; i < CAMSV_MAX - CAMSV_START ; i++){
            cx = std::snprintf(str + cx, str_size - cx, "camsv:%d_%d,", i, this->m_module[i].occupied);
        }
        BASE_LOG_ERR("resource pool: %s\n", str);
        v_available.clear();
        ret = MFALSE;
    }

    return ret;
}

MBOOL Central_Camsv::Register(ISP_HW_MODULE module, char Name[32], MUINT32 logLevel, MUINT32 mMipiPixrate)
{
    MBOOL ret = MTRUE;
    MUINT32 idx;
    Mutex::Autolock lock(this->mLock);

    //todo, consider input mipi_pixel_rate
    if(module < CAMSV_START || module >= CAMSV_MAX){
        BASE_LOG_ERR("unsupported camsv:%d\n", module);
        ret = MFALSE;
        goto EXIT;
    }

    if(*Name == '\0'){
        BASE_LOG_ERR("cant have NULL userName\n");
        ret = MFALSE;
        goto EXIT;

    }

    //idx shift
    idx = module - CAMSV_START;

    if(this->m_module[idx].occupied == MFALSE){
        this->m_module[idx].occupied = MTRUE;
        memset((void*)this->m_module[idx].User, 0, sizeof(this->m_module[idx].User));

        std::strncpy((char*)this->m_module[idx].User, (char const*)Name, sizeof(this->m_module[idx].User) - 1);

        this->m_available--;
    }
    else{
        if(logLevel){
            BASE_LOG_ERR("camsv:%d already occupied by user:%s,can't register by user:%s\n", module, this->m_module[idx].User, Name);
        }
        ret = MFALSE;
        goto EXIT;
    }

EXIT:
    return ret;
}


MBOOL Central_Camsv::Register(ISP_HW_MODULE module, char Name[32], MUINT32 logLevel, MUINT32 mMipiPixrate,
    vector<E_INPUT>& InOutPut)
{
    MBOOL ret = MTRUE;
    MUINT32 idx;
    Mutex::Autolock lock(this->mLock);
    //todo, consider input mipi_pixel_rate
    BASE_LOG_INF("module %d, mMipiPixrate %d, InOutPut %d\n", module, mMipiPixrate, InOutPut[0]);
    if(module < CAMSV_START || module >= CAMSV_MAX){
        BASE_LOG_ERR("unsupported camsv:%d\n", module);
        ret = MFALSE;
        goto EXIT;
    }

    if(*Name == '\0'){
        BASE_LOG_ERR("cant have NULL userName\n");
        ret = MFALSE;
        goto EXIT;

    }

    //idx shift
    idx = module - CAMSV_START;

    if(this->m_module[idx].occupied == MFALSE){
        this->m_module[idx].occupied = MTRUE;
        memset((void*)this->m_module[idx].User, 0, sizeof(this->m_module[idx].User));
        std::strncpy((char*)this->m_module[idx].User, (char const*)Name, sizeof(this->m_module[idx].User) - 1);
        this->m_available--;
    }
    else{
        if(logLevel){
            BASE_LOG_ERR("camsv:%d already occupied by user:%s,can't register by user:%s\n", module, this->m_module[idx].User, Name);
        }
        ret = MFALSE;
        goto EXIT;
    }

EXIT:
    return ret;
}

MBOOL Central_Camsv::Release(ISP_HW_MODULE module)
{
    MBOOL ret = MTRUE;
    MUINT32 idx;
    Mutex::Autolock lock(this->mLock);

    if(module < CAMSV_START || module >= CAMSV_MAX){
        BASE_LOG_ERR("unsupported camsv:%d\n", module);
        ret = MFALSE;
        goto EXIT;
    }

    //idx shift
    idx = module - CAMSV_START;

    if(this->m_module[idx].occupied == MTRUE){
        this->m_module[idx].occupied = MFALSE;
        *this->m_module[idx].User = '\0';
        this->m_available++;
    }
    else{
        ret = MFALSE;
        BASE_LOG_ERR("camsv:%d logic error: ,should be occupied\n", module);
    }

EXIT:
    return ret;
}

//request SV TG in normal case
ISP_HW_MODULE Central_Camsv::Register_search(ISP_HW_MODULE module, char Name[32])
{
    ISP_HW_MODULE target = CAMSV_MAX;
    MUINT32 _debugConfig = 0;

    _debugConfig = property_get_int32("vendor.debug.fix.camsv.module", 0);
    if(_debugConfig == 1){
        if(this->Register(CAMSV_START, Name, 0, 0) == MTRUE){
            target = CAMSV_START;
            goto EXIT;
        }
    }
    //suppose from DCIF or camsvIOPipe caller,
    //statistic data used TOP_3_1/TOP_3_0~TOP_0_1/TOP_0_0
    //DCIF use TOP_0_0/TOP_0_1
    //let user to determine which camsv to use first, if occupied, find non-occupied one
    //in ISP5.0 HW, all camsv pipeline are symmetric, so just choose non-occupied one
    for(MUINT32 i = CAMSV_7 ; i >= CAMSV_2 ; i--){
        if(this->Register((ISP_HW_MODULE)i, Name, 0, 0) == MTRUE){
            target = (ISP_HW_MODULE)i;
            goto EXIT;
        }
    }

EXIT:
    return target;
}

//support input without TG info, suppose DCIF/UFEO case
ISP_HW_MODULE Central_Camsv::Register_search(MUINT32 mMipiPixrate, char Name[32])
{
    ISP_HW_MODULE target = CAMSV_MAX;

    //in ISP5.0 HW, all camsv pipeline are symmetric, so just choose non-occupied one
    for(MUINT32 i = CAMSV_START ; i < CAMSV_MAX ; i++){
        if(this->Register((ISP_HW_MODULE)i, Name, 0, mMipiPixrate) == MTRUE){
            target = (ISP_HW_MODULE)i;
            goto EXIT;
        }
    }

EXIT:
    return target;
}

ISP_HW_MODULE Central_Camsv::Register_search(MUINT32 mMipiPixrate, char Name[32], vector<E_INPUT>& InOutPut)
{
    ISP_HW_MODULE target = CAMSV_MAX;

    //temp mipi_pixrate = 0  means use TOP_0 only
    if(mMipiPixrate == 0 && (InOutPut[0] == TG_CAMSV_0)) {
        for(MUINT32 i = CAMSV_START ; i < CAMSV_2 ; i++){
            if(this->Register((ISP_HW_MODULE)i, Name, 1, mMipiPixrate, InOutPut) == MTRUE){
                target = (ISP_HW_MODULE)i;
                BASE_LOG_INF("get target %d \n", target);
                goto EXIT;
            }
        }
    }
    else {
        //in ISP5.0 HW, all camsv pipeline are symmetric, so just choose non-occupied one
        for(MUINT32 i = CAMSV_2 ; i < CAMSV_MAX ; i++){
            if(this->Register((ISP_HW_MODULE)i, Name, 0, mMipiPixrate, InOutPut) == MTRUE){
                target = (ISP_HW_MODULE)i;
                BASE_LOG_INF("get target %d \n", target);
                goto EXIT;
            }
        }
    }
EXIT:
    return target;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

Central_Camsv Camsv_ResMgr::m_central;
Mutex         Camsv_ResMgr::mLock;
char          Camsv_ResMgr::m_staticName[16] = "ICamsvResMgr";

/////////////////////////////////////////////////////////////////////////////////////////////

Camsv_ResMgr::Camsv_ResMgr(ISP_HW_MODULE master, char Name[32])
{
    DBG_LOG_CONFIG(imageio, camsvResMgr);
    m_hwModule = master;
    *m_User = '\0';
    memset((void*)m_User, 0, sizeof(m_User));
    std::strncpy((char*)m_User, (char const*)Name, sizeof(m_User) - 1);
    sprintf(m_internalUser, "%s_%d", Camsv_ResMgr::m_staticName, master);
}

Camsv_ResMgr* Camsv_ResMgr::Res_Attach(ISP_HW_MODULE master, char Name[32])
{
    ISP_HW_MODULE target;
    Mutex::Autolock lock(Camsv_ResMgr::mLock);

    if((target = Camsv_ResMgr::m_central.Register_search(master, Name)) == CAMSV_MAX){
        return NULL;
    }
    BASE_LOG_INF("CAMSV Res_Attach:: origin:%d,target:%d,name:%s\n", master, target, Name);

    return new Camsv_ResMgr(target, Name);
}

Camsv_ResMgr* Camsv_ResMgr::Res_Attach(MUINT32 mMipiPixrate,char Name[32])
{
    MBOOL ret = MTRUE;
    ISP_HW_MODULE master;

    BASE_LOG_INF("CAM Res_Attach:: pixrate:%d,name:%s\n",mMipiPixrate,Name);
    Mutex::Autolock lock(Camsv_ResMgr::mLock);
    if( (master = Camsv_ResMgr::m_central.Register_search(mMipiPixrate,Name))  == CAMSV_MAX){
        return NULL;
    }

    return new Camsv_ResMgr(master,Name);
}

Camsv_ResMgr* Camsv_ResMgr::Res_Attach(MUINT32 mMipiPixrate,char Name[32], vector<E_INPUT>& InOutPut)
{
    MBOOL ret = MTRUE;
    ISP_HW_MODULE master;

    BASE_LOG_INF("CAM Res_Attach:: pixrate:%d,name:%s,TG info size:%d\n",mMipiPixrate,Name,InOutPut.size());
    Mutex::Autolock lock(Camsv_ResMgr::mLock);
    if( (master = Camsv_ResMgr::m_central.Register_search(mMipiPixrate,Name,InOutPut))  == CAMSV_MAX){
        return NULL;
    }

    return new Camsv_ResMgr(master,Name);
}

MBOOL Camsv_ResMgr::Res_Detach(void)
{
    MBOOL ret = MTRUE;
    PIPE_INF("CAMSV Res_Detach:\n");

    this->mLock.lock();
    if(this->m_central.Release(this->m_hwModule) == MFALSE){
        ret = MFALSE;
    }
    this->mLock.unlock();

    if(ret == MTRUE)
        delete this;
    else
        PIPE_ERR("release fail, obj won't be destroied \n");
EXIT:
    return ret;
}

ISP_HW_MODULE Camsv_ResMgr::Res_GetCurrentModule(void)
{
    return this->m_hwModule;
}
