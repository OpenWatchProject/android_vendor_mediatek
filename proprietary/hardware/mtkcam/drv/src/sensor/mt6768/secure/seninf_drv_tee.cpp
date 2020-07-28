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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "SeninfDrvTee"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <utils/threads.h>
#include <utils/Errors.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <mtkcam/def/common.h>
#include "kd_seninf.h"
#include "kd_imgsensor_CA_TA_cmd.h"
#include "seninf_type.h"
#include "seninf_tee_reg.h"
#include "seninf_drv_tee.h"
#ifndef USING_MTK_LDVT
#define LOG_MSG(fmt, arg...)    ALOGD("[%s]" fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    ALOGD("[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    ALOGE("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#else
#include "uvvf.h"
#if 1
#define LOG_MSG(fmt, arg...)    VV_MSG("[%s]" fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    VV_MSG("[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    VV_MSG("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#else
#define LOG_MSG(fmt, arg...)
#define LOG_WRN(fmt, arg...)
#define LOG_ERR(fmt, arg...)
#endif
#endif

using namespace NSCam;

/*******************************************************************************
*
********************************************************************************/
SeninfDrv*
SeninfDrv::
createInstance (MBOOL isSecure)
{
    return (isSecure) ? SeninfDrvTeeImp::getInstance() : SeninfDrvImp::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrvTeeImp::SeninfDrvTeeImp() : SeninfDrvImp()
{
    memset(&mTeeReg, 0, sizeof(SENINF_TEE_REG));
    mUser = 0;
    mpCa = nullptr;
    seninf_ca_open_session = nullptr;
    seninf_ca_close_session = nullptr;
    seninf_ca_sync_to_pa = nullptr;
    seninf_ca_sync_to_va = nullptr;
    seninf_write_flag = 0;
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrvTeeImp::~SeninfDrvTeeImp()
{

}

/*******************************************************************************
*
********************************************************************************/
SeninfDrv*
SeninfDrvTeeImp::
getInstance()
{
    static SeninfDrvTeeImp drv;
    return (SeninfDrv*)&drv;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvTeeImp::init()
{
    int ret = 0;

    Mutex::Autolock lock(mLock);

    LOG_MSG("SeninfDrvTee init %d \n", mUser);
    if (mUser) {
        android_atomic_inc(&mUser);
        return 0;
    }

    ret = SeninfDrvImp::init();

    void *pDynamicLink = dlopen(SENINF_CA_LIB, RTLD_NOW);

    if (pDynamicLink == nullptr) {
        LOG_ERR("seninf_ca lib open failed (%s)", dlerror());
        return MFALSE;
    }

    if (seninf_ca_open_session == nullptr) {
        seninf_ca_open_session = (seninf_ca_open_session_t*) dlsym(pDynamicLink, SENINF_CA_OPEN_SESSION);
        if (seninf_ca_open_session == nullptr) {
            LOG_ERR("dlsym seninf_ca_open_session failed!");
	    delete pDynamicLink;
            pDynamicLink = nullptr;
            return MFALSE;
        }
    }

    if (seninf_ca_close_session == nullptr) {
        seninf_ca_close_session = (seninf_ca_close_session_t*) dlsym(pDynamicLink, SENINF_CA_CLOSE_SESSION);
        if (seninf_ca_close_session == nullptr) {
            LOG_ERR("dlsym seninf_ca_close_session failed!");
	    delete pDynamicLink;
            pDynamicLink = nullptr;
            return MFALSE;
        }
    }
#if 0
    if (seninf_ca_sync_to_pa == nullptr) {
        seninf_ca_sync_to_pa = (seninf_ca_sync_to_pa_t*) dlsym(pDynamicLink, SENINF_CA_SYNC_TO_PA);
        if (seninf_ca_sync_to_pa == nullptr) {
            LOG_ERR("dlsym seninf_ca_sync_to_pa failed!");
            return MFALSE;
        }
    }

    if (seninf_ca_sync_to_va == nullptr) {
        seninf_ca_sync_to_va = (seninf_ca_sync_to_va_t*) dlsym(pDynamicLink, SENINF_CA_SYNC_TO_VA);
        if (seninf_ca_sync_to_va == nullptr) {
            LOG_ERR("dlsym seninf_ca_write_reg failed!");
            return MFALSE;
        }
    }
#endif
    if (seninf_ca_checkpipe == nullptr) {
        seninf_ca_checkpipe = (seninf_ca_checkpipe_t*) dlsym(pDynamicLink, SENINF_CA_CHECKPIPE);
        if (seninf_ca_checkpipe == nullptr) {
            LOG_ERR("dlsym seninf_ca_checkpipe init failed!");
	    delete pDynamicLink;
            pDynamicLink = nullptr;
            return MFALSE;
        }
    }

    if (seninf_ca_free == nullptr) {
        seninf_ca_free = (seninf_ca_free_t*) dlsym(pDynamicLink, SENINF_CA_FREE);
        if (seninf_ca_free == nullptr) {
            LOG_ERR("dlsym seninf_ca_free init failed!");
	    delete pDynamicLink;
            pDynamicLink = nullptr;
            return MFALSE;
        }
    }

    mpCa = seninf_ca_open_session();
    if (mpCa == nullptr) {
        LOG_ERR("seninf_ca_open_session failed!");
	delete pDynamicLink;
        pDynamicLink = nullptr;
        return MFALSE;
    }
#if 0
    memset(&mTeeReg, 0, sizeof(SENINF_TEE_REG));
    seninf_ca_sync_to_va(mpCa, (void *)&mTeeReg);
#endif

    android_atomic_inc(&mUser);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvTeeImp::uninit()
{
    int ret = 0;

    Mutex::Autolock lock(mLock);

    LOG_MSG("SeninfDrvTee uninit %d \n", mUser);
    android_atomic_dec(&mUser);

    if (mUser) {
        return 0;
    }

    seninf_ca_close_session(mpCa);
#if 0
    memset(&mTeeReg, 0, sizeof(SENINF_TEE_REG));
#endif
    seninf_write_flag = 0;
    mpCa = nullptr;
    seninf_ca_open_session = nullptr;
    seninf_ca_close_session = nullptr;
    seninf_ca_sync_to_pa = nullptr;
    seninf_ca_sync_to_va = nullptr;
    seninf_ca_checkpipe = nullptr;
    seninf_ca_free = nullptr;

    ret = SeninfDrvImp::uninit();

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvTeeImp::sendCommand(int cmd, unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
    int ret = 0;

    Mutex::Autolock lock(mLock);//for uninit, some pointer will be set to NULL

    LOG_MSG("seninfdrvtee sendcommand %d", cmd);

    switch (cmd) {
    case CMD_SENINF_CHECK_PIPE:
        LOG_MSG("Start Check seninf before streaming!");
        seninf_ca_checkpipe(mpCa);
        seninf_write_flag = 1;
        break;

    case CMD_SENINF_RESUME_FREE:
        LOG_MSG("Free streaming lock, close cam_mux!");
        seninf_ca_free(mpCa);
        seninf_write_flag = 0;
        break;

    default:
        if (seninf_write_flag == 0) {
            ret = SeninfDrvImp::sendCommand(cmd, arg1, arg2, arg3);
        } else {
            LOG_WRN("write seninf after dapc lock!, %d", cmd);
        }
        break;
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvTeeImp::setSeninfTopMuxCtrl(unsigned int seninfMuXIdx, SENINF_ENUM seninfSrc)
{
    return SeninfDrvImp::setSeninfTopMuxCtrl(seninfMuXIdx, seninfSrc);
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvTeeImp::setSeninfMuxCtrl(
    unsigned long Muxsel, unsigned long hsPol, unsigned long vsPol,
    SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType,
    unsigned int PixelMode)
{
    int ret =0;
    ret = SeninfDrvImp::setSeninfMuxCtrl(Muxsel, hsPol, vsPol,
               inSrcTypeSel, inDataType, PixelMode);
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvTeeImp::debug()
{
    /*close cam_mux & devapc lock to prevent violation*/
    unsigned long arg1 = 0, arg2 = 0, arg3 = 0;
    sendCommand(CMD_SENINF_RESUME_FREE, arg1, arg2, arg3);

    return SeninfDrvImp::debug();
}

