/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
#define LOG_TAG "Iop/P2PPWp"

#include "HalPipeWrapper.h"
#include <IPostProcPipe.h>
#include <mtkcam/drv/def/ispio_sw_scenario.h>
#include <ispio_stddef.h>
//thread
#include <utils/threads.h>
#include <mtkcam/def/PriorityDefs.h>
//thread priority
#include <system/thread_defs.h>
#include <sys/resource.h>
#include <utils/ThreadDefs.h>
// For property_get().
#include <cutils/properties.h>

#include <vector>
#include <utils/Mutex.h>    // For android::Mutex.

#include <cutils/atomic.h>
/*************************************************************************************
* Log Utility
*************************************************************************************/
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include <imageio_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(P2PipeWrapper);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (P2PipeWrapper_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (P2PipeWrapper_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (P2PipeWrapper_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (P2PipeWrapper_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (P2PipeWrapper_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (P2PipeWrapper_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc;

UserInfo HalPipeWrapper::mUserInfo[MAX_PIPE_USER_NUMBER]=
{
    {ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},
    {ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},
    {ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},
    {ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},
    {ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},
    {ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}}
};

UserInfo HalPipeWrapper::mWPEUserInfo[MAX_PIPE_WPEUSER_NUMBER]=
{
    {ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}},
    {ESatremPipeID_None, {"\0"}},{ESatremPipeID_None, {"\0"}}
};
#if 0
NSImageio::NSIspio::EDrvScenario const gSwScenDrvScenMapping[ENormalStreamTag_total]=
{
    NSImageio::NSIspio::eDrvScenario_P2A,
    NSImageio::NSIspio::eDrvScenario_VSS,
    NSImageio::NSIspio::eDrvScenario_VFB_FB,
    NSImageio::NSIspio::eDrvScenario_MFB_Blending,
    NSImageio::NSIspio::eDrvScenario_MFB_Mixing,
    NSImageio::NSIspio::eDrvScenario_P2B_Bokeh,
    NSImageio::NSIspio::eDrvScenario_FE,
    NSImageio::NSIspio::eDrvScenario_FM,
    NSImageio::NSIspio::eDrvScenario_Color_Effect,
    NSImageio::NSIspio::eDrvScenario_DeNoise,
    NSImageio::NSIspio::eDrvScenario_WUV,
    NSImageio::NSIspio::eDrvScenario_Y16_Dump,
    NSImageio::NSIspio::eDrvScenario_IP_Tpipe,
    NSImageio::NSIspio::eDrvScenario_LPCNR_Pass1,
    NSImageio::NSIspio::eDrvScenario_LPCNR_Pass2
};
#endif

MINT32 getUs()
{
   struct timeval tv;
   ::gettimeofday(&tv, NULL);
   return tv.tv_sec * 1000000 + tv.tv_usec;
}

/******************************************************************************
 *
 ******************************************************************************/
HalPipeWrapper*
HalPipeWrapper::createInstance()
{
    return HalPipeWrapper::getInstance();
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalPipeWrapper::destroyInstance()
{
    //FUNCTION_LOG_START;
    //FUNCTION_LOG_END;
}

/******************************************************************************
 *
 ******************************************************************************/
static HalPipeWrapper Singleton;

HalPipeWrapper*
HalPipeWrapper::
getInstance()
{
    LOG_INF("&Singleton(0x%lx)\n",(long)&Singleton);
    //
    return &Singleton;
}


/******************************************************************************
 *
 ******************************************************************************/
HalPipeWrapper::
HalPipeWrapper()
    : mInitCount (0)
    , mpPostProcPipe(NULL)
    , mFps(30)
    , mDIPEQThreadIdx(0)
    , mWPE_InitCount(0)
    , m_iEnqueFrmNum(0)
    , m_iDequeFrmNum(0)
{
    DBG_LOG_CONFIG(iopipe, P2PipeWrapper);
    char dumppath[256];

    m_iSaveReqToFile = ::property_get_int32("vendor.cam.dipsavefile.enable", 0);
    if (m_iSaveReqToFile > 0)
    {
        snprintf (dumppath, 256, "%s/", DUMP_DIPPREFIX);

        if(!DrvMakePath(dumppath,0660))
        {
            LOG_ERR("DIP makePath [%s] fail",dumppath);
        }
        else
        {
            LOG_DBG("DIP makePath [%s] success",dumppath);
        }
    }

    for(int i =0; i<EThreadProperty_NUM;i++)
    {
        //mHalPWrapperEQThread[i]=NULL;
        mHalPMdpConfigThread[i]=NULL;
        mHalPWrapperDQThread[i]=NULL;
    }

    //
    mMutex_EQFramePackList=PTHREAD_MUTEX_INITIALIZER;
    mMutex_waitDoMdpStartUnitList=PTHREAD_MUTEX_INITIALIZER;
    mEnqMutex=PTHREAD_MUTEX_INITIALIZER;
//	    mHalSensorList=IHalSensorList::get();

    for(int i=0;i<_DIP_NUM_;i++)
    {
        this->mpDipPipe[i] = NULL;
    }

}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::
init(
    char const* szCallerName,
    NSCam::NSIoPipe::EStreamPipeID streamPipeID,
    DipUserParam UserParam,
    MUINT32 secTag
)
{
    //FUNCTION_LOG_START;
    bool ret = true;
    android::Mutex::Autolock lock(mLock);
    if(strlen(szCallerName)<1)
    {
        LOG_ERR("Plz add userName\n");
        return MFALSE;
    }
    //
    if(streamPipeID<0 || streamPipeID>=EStreamPipeID_Total)
    {
        LOG_ERR("wrong pipeID(%d)\n",streamPipeID);
        return MFALSE;
    }

    //Add wpe
     switch(streamPipeID)
    {
        case EStreamPipeID_Normal:
            //[1] increase user count and record user
            LOG_INF("DIP: - E (New) mUCnt(%d), cUser(%s), pipeID(%d)",mInitCount,szCallerName, streamPipeID);
            if(mInitCount < MAX_PIPE_USER_NUMBER)
            {
                bool getspace=false;
                //check first to avoid timing issue
                for(int i=0;i<mInitCount;i++)
                {
                    if(this->mUserInfo[i].pipeID==ESatremPipeID_None)
                    {
                        this->mUserInfo[i].pipeID=streamPipeID;
                        strncpy((char*)(this->mUserInfo[i].name),(char const*)szCallerName, strlen(szCallerName));
                        this->mUserInfo[i].name[strlen(szCallerName)]='\0';
                        getspace=true;
                        break;
                    }
                }

                if(!getspace)
                {
                    this->mUserInfo[mInitCount].pipeID=streamPipeID;
                    strncpy((char*)(this->mUserInfo[mInitCount].name),(char const*)szCallerName, strlen(szCallerName));
                    this->mUserInfo[mInitCount].name[strlen(szCallerName)]='\0';

                }
            }
            else{
                    LOG_ERR("m_userCnt is over upper bound\n");
                    return MFALSE;
                }
            LOG_INF("- flag 3 (%d/%s/%d) ",mInitCount,this->mUserInfo[mInitCount].name,this->mUserInfo[mInitCount].pipeID);

            android_atomic_inc(&mInitCount);
            //first user
            if(mInitCount==1)
            {
#if 0
                mbVssOccupied=MFALSE;
                sem_init(&mSemVencPortCnt, 0, 0);
                sem_init(&mSemVssOccupied, 0, 0);	// For Vss Concurrency Check

                //
                sem_index=0;
                for(int i=0;i<_SEM_EQDONE_NUM_;i++)
                {
                    sem_init(&(mSemEQDone[i]), 0, 0);
                }
#endif
                //NSImageio::NSIspio::IPostProcPipe* .[2];
                char szTmp[30];
                for(int i=0;i<_DIP_NUM_;i++)
                {
                    this->mpDipPipe[i] = NSImageio::NSIspio::IPostProcPipe::createInstance();
                    sprintf(szTmp,"HalPipeWraper_Dip_%d",i);
                    if (NULL ==this->mpDipPipe[i] || !(this->mpDipPipe[i]->init(szTmp, UserParam, secTag)))
                    {
                        LOG_ERR("HalPipeWrapper PostProc init fail");
                        return MFALSE;
                    }
                }
                this->mpPostProcPipe = this->mpDipPipe[_DIP_HW_A_];

                //create thread for enque / deque
                for(int i=0;i<(EThreadProperty_NUM);i++)
                {
                    //this->mHalPWrapperEQThread[i]=HalPipeWrapper_Thread::createInstance(this,EThreadType_ENQUE,static_cast<EThreadProperty>(i));
                    this->mHalPMdpConfigThread[i]=HalPipeWrapper_Thread::createInstance(this,EThreadType_MDPSTART,static_cast<EThreadProperty>(i));
                    this->mHalPWrapperDQThread[i]=HalPipeWrapper_Thread::createInstance(this,EThreadType_DEQUE,static_cast<EThreadProperty>(i));
                }
            }
			else
			{
				if (this->mpPostProcPipe != NULL)
					this->mpPostProcPipe->init(NULL, UserParam, secTag);
			}
            break;
#if 0  //Justin EP porting 6789

        case EStreamPipeID_WarpEG:
             LOG_INF("- E (New) mUCnt(%d), cUser(%s), pipeID(%d)",mWPE_InitCount,szCallerName, streamPipeID);
             if(mWPE_InitCount < MAX_PIPE_WPEUSER_NUMBER)
             {
                 bool getspace=false;
                //check first to avoid timing issue
                for(int i=0;i<mWPE_InitCount;i++)
                {
                    if(this->mWPEUserInfo[i].pipeID==ESatremPipeID_None)
                    {
                        this->mWPEUserInfo[i].pipeID=streamPipeID;
                        strncpy((char*)(this->mWPEUserInfo[i].name),(char const*)szCallerName, strlen(szCallerName));
                        this->mWPEUserInfo[i].name[strlen(szCallerName)] ='\0';
                        getspace=true;
                        break;
                    }
                }

                if(!getspace)
                {
                    this->mWPEUserInfo[mWPE_InitCount].pipeID=streamPipeID;
                    strncpy((char*)(this->mWPEUserInfo[mWPE_InitCount].name),(char const*)szCallerName, strlen(szCallerName));
                    this->mWPEUserInfo[mWPE_InitCount].name[strlen(szCallerName)] ='\0';
                }
            }
            else{
                    LOG_ERR("m_WPEuserCnt is over upper bound\n");
                    return MFALSE;
                }
            LOG_INF("- flag 3 (%d/%s/%d) ",mWPE_InitCount,this->mWPEUserInfo[mWPE_InitCount].name,this->mWPEUserInfo[mWPE_InitCount].pipeID);


            android_atomic_inc(&mWPE_InitCount);
            if(mWPE_InitCount == 1)
            {
                this->mWpeStream = NSCam::NSIoPipe::NSWpe::WpeStream::createInstance();
                if (NULL ==this->mWpeStream || !(this->mWpeStream->init(secTag)))
                {
                    LOG_ERR("HalPipeWrapper Wpe Stream init fail");
                    return MFALSE;
                }
            }
            break;
#endif
        default:
            LOG_ERR("HalPipeWrapperinit fail : Wrong streamPipeID");
            ret = MFALSE;
            break;
    }

    LOG_INF("- X. mUCnt(%d), cUser(%s), pipeID(%d)",mInitCount,szCallerName, streamPipeID);
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::
uninit(
    char const* szCallerName,
    NSCam::NSIoPipe::EStreamPipeID streamPipeID,
    DipUserParam UserParam)
{
    //FUNCTION_LOG_START;
    bool ret = true;
    MBOOL bMatch=MFALSE;
    Mutex::Autolock lock(mLock);
    MINT32 index = 0;

    switch(streamPipeID)
    {
        case EStreamPipeID_Normal:
            //[1] decrease user count and check user information
            LOG_INF("- E. mUCnt(%d),mUser(%s), pipeID(%d)",mInitCount,szCallerName, streamPipeID);
            index = mInitCount;
            for(int i=0;i<MAX_PIPE_USER_NUMBER;i++)
            {
                if(this->mUserInfo[i].pipeID == streamPipeID)
                {
                    if(strcmp((const char*)(this->mUserInfo[i].name),szCallerName) == 0)
                    {
                        index=i;
                        bMatch = MTRUE;
                        break;
                    }
                }
            }
            if(bMatch == MFALSE){
                LOG_ERR("no matching username:%s from pipe_%d\n",szCallerName,streamPipeID);
                for(int i=0;i<MAX_PIPE_USER_NUMBER;i+=4)
                    LOG_ERR("current user: %d_%s, %d_%s, %d_%s, %d_%s\n",this->mUserInfo[i].pipeID, this->mUserInfo[i].name,\
                            this->mUserInfo[i+1].pipeID, this->mUserInfo[i+1].name, this->mUserInfo[i+2].pipeID, this->mUserInfo[i+2].name,
                            this->mUserInfo[i+3].pipeID, this->mUserInfo[i+3].name);
                return MFALSE;
           }
           else{
               this->mUserInfo[index].pipeID=ESatremPipeID_None;
               this->mUserInfo[index].name[0] = '\0';
           }

            android_atomic_dec(&mInitCount);
           //[2]release variable if all users call uninit
           //deq thread related variables are init only once
            if(mInitCount==0)
            {
                //stop thread
                //destroy thread for enque / deque
                for(int i=0;i<(EThreadProperty_NUM);i++)
                {
                    //this->mHalPWrapperEQThread[i]->destroyInstance();
                    this->mHalPMdpConfigThread[i]->destroyInstance();
                    this->mHalPWrapperDQThread[i]->destroyInstance();
                }

                //free vector variable
                //vector<FramePackage>().swap(mLDIPEQFramePackList); //force vector to relinquish its memory (look up "stl swap trick")
                char szTmp[30];
                for(int i=0;i<_DIP_NUM_;i++)
                {
                    sprintf(szTmp,"HalPipeWraper_Dip_%d",i);
                    if(!this->mpDipPipe[i])
                    {
                        LOG_ERR("HalPipeWrapper mpPostProcPipe(%d) is NULL...", i);
                    }
                    else
                    {
                        if (!(this->mpDipPipe[i]->uninit(szTmp, UserParam)))
                        {
                            LOG_ERR("HalPipeWrapper mpPostProcPipe(%d) uninit fail...", i);
                        }
                        this->mpDipPipe[i]->destroyInstance();
                        this->mpDipPipe[i] = NULL;
                    }

                }
                this->mpPostProcPipe = NULL;

#if 0
                ::sem_destroy(&mSemVencPortCnt);
                //::sem_destroy(&mSemVssOccupied);	// For Vss Concurrency Check
                //
                sem_index=0;
                for(int i=0;i<_SEM_EQDONE_NUM_;i++)
                {
                    ::sem_destroy(&(mSemEQDone[i]));
                }
#endif
            }
			else
			{
				if (this->mpPostProcPipe != NULL)
					this->mpPostProcPipe->uninit(NULL, UserParam);

			}

            if(mInitCount <  5){
                LOG_INF("%d_current working user:\n",mInitCount);
                for(int i=0;i<MAX_PIPE_USER_NUMBER;i+=8)
                    LOG_INF("current user: %d_%s, %d_%s, %d_%s, %d_%s, %d_%s, %d_%s, %d_%s, %d_%s\n",    \
                    this->mUserInfo[i].pipeID, this->mUserInfo[i].name,this->mUserInfo[i+1].pipeID, this->mUserInfo[i+1].name, this->mUserInfo[i+2].pipeID, this->mUserInfo[i+2].name,\
                    this->mUserInfo[i+3].pipeID, this->mUserInfo[i+3].name, this->mUserInfo[i+4].pipeID, this->mUserInfo[i+4].name, this->mUserInfo[i+5].pipeID, this->mUserInfo[i+5].name,\
                    this->mUserInfo[i+6].pipeID, this->mUserInfo[i+6].name, this->mUserInfo[i+7].pipeID, this->mUserInfo[i+7].name);
                }
            break;

        case EStreamPipeID_WarpEG:
            LOG_INF("- E. mUCnt(%d),mWPEUser(%s), pipeID(%d)",mWPE_InitCount,szCallerName, streamPipeID);
            index = mWPE_InitCount;
            for(int i=0;i<MAX_PIPE_WPEUSER_NUMBER;i++)
            {
                if(this->mWPEUserInfo[i].pipeID == streamPipeID)
                {
                    if(strcmp((const char*)(this->mWPEUserInfo[i].name),szCallerName) == 0)
                    {
                        index=i;
                        bMatch = MTRUE;
                        break;
                    }
                }
             }
             if(bMatch == MFALSE){
                LOG_ERR("no matching username:%s from pipe_%d\n",szCallerName,streamPipeID);
                for(int i=0;i<MAX_PIPE_WPEUSER_NUMBER;i+=4)
                    LOG_ERR("current user: %d_%s, %d_%s, %d_%s, %d_%s\n",this->mWPEUserInfo[i].pipeID, this->mWPEUserInfo[i].name,\
                        this->mWPEUserInfo[i+1].pipeID, this->mWPEUserInfo[i+1].name, this->mWPEUserInfo[i+2].pipeID, this->mWPEUserInfo[i+2].name,
                        this->mWPEUserInfo[i+3].pipeID, this->mWPEUserInfo[i+3].name);
                return MFALSE;
             }
             else{
                    this->mWPEUserInfo[index].pipeID=ESatremPipeID_None;
                    this->mWPEUserInfo[index].name[0] = '\0';
             }

            android_atomic_dec(&mWPE_InitCount);
#if 0  //Justin EP porting 6789

            if(mWPE_InitCount==0)
            {
                LOG_INF("%d_current working user:\n",mWPE_InitCount);
                if(!this->mWpeStream)
                {
                    LOG_ERR("HalPipeWrapper mpWPE Stream is NULL...");
                }
                else
                {
                    if (!(this->mWpeStream->uninit()))
                    {
                        LOG_ERR("HalPipeWrapper mpWPE Stream uninit fail...");
                    }
                    this->mWpeStream->destroyInstance();
                    this->mWpeStream = NULL;
                }
            }
            break;
#else
            break;
#endif
        default:
            LOG_ERR("HalPipeWrapperinit fail : Wrong streamPipeID");
            ret = MFALSE;
            break;
    }

    //FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
*
******************************************************************************/
NSImageio::NSIspio::IPostProcPipe*
HalPipeWrapper::
getPostProcPipeInstance(unsigned int DipModule)
{
    if (DipModule>=_DIP_NUM_)
    {
        LOG_ERR("DipModule exceed hw support size:%d", DipModule);
        return NULL;
    }
    if (this->mpDipPipe[DipModule]==NULL)
    {
        LOG_ERR("DipPipe:%d is Null!!", DipModule);
        return NULL;
    }
    return this->mpDipPipe[DipModule];
}

/******************************************************************************
*
******************************************************************************/
MVOID
HalPipeWrapper::
getLock(
    ELockEnum lockType)
{
    switch(lockType)
    {
        case ELockEnum_EQFramePackList:
            pthread_mutex_lock(&mMutex_EQFramePackList);
            break;
        case ELockEnum_wDoMdpStartUnitList:
            pthread_mutex_lock(&mMutex_waitDoMdpStartUnitList);
            break;
        case ELockEnum_EnqueLock:
            pthread_mutex_lock(&mEnqMutex);
            break;
        default:
            break;
    }
}

/******************************************************************************
*
******************************************************************************/
MVOID
HalPipeWrapper::
releaseLock(
    ELockEnum lockType)
{
    switch(lockType)
    {
        case ELockEnum_EQFramePackList:
            pthread_mutex_unlock(&mMutex_EQFramePackList);
            break;
        case ELockEnum_wDoMdpStartUnitList:
            pthread_mutex_unlock(&mMutex_waitDoMdpStartUnitList);
            break;
        case ELockEnum_EnqueLock:
            pthread_mutex_unlock(&mEnqMutex);
            break;

        default:
            break;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::
enque(
    NSCam::NSIoPipe::EStreamPipeID streamPipeID,
    MUINT32 sensorDev,
    MINT32 pixID,
    QParams const& rParams,
    vector<QParams>& callerDeQList,
    char* userName)
{

    LOG_DBG("+");
    //Mutex::Autolock lock(mLock); //need? cuz we wait enque done in the end of this function
    (void)sensorDev;
    bool ret = true;
    //get sem index
    //int wait_index=0;

    //distribute the requested enque buffer package
    FramePackage framePack;
    framePack.callerPipe=streamPipeID;
    framePack.pixID=pixID;
    if((userName !=NULL) && (strlen(userName) < MAX_USER_NAME_LEN))
    {
        strncpy((char*)framePack.m_UserName,(char const*)userName, strlen(userName));
        framePack.m_UserName[strlen(userName)]='\0';
    }
#if 0
    //framePack.callerSensorDev=sensorDev;
    switch (sensorDev)
    {
        case SENSOR_DEV_NONE:
            framePack.callerSensorDev=0;
            break;
        case SENSOR_DEV_MAIN:
            framePack.callerSensorDev=1;
            break;
        case SENSOR_DEV_SUB:
            framePack.callerSensorDev=2;
            break;
        case SENSOR_DEV_PIP:
            framePack.callerSensorDev=3;
            break;
        case SENSOR_DEV_MAIN_2:
            framePack.callerSensorDev=4;
            break;
        case SENSOR_DEV_MAIN_3D:
            framePack.callerSensorDev=5;
            break;
        case SENSOR_DEV_SUB_2:
            framePack.callerSensorDev=6;
            break;
        default:
            LOG_ERR("unexpected sensorDev !!(%d)", sensorDev);
            return MFALSE;
    }
#endif
    framePack.callerID=(MUINTPTR)(&callerDeQList);
    framePack.rParams=rParams;

    //error handle
    if(rParams.mvFrameParams.size()<1)
    {
        LOG_ERR("unexpected frameParas size(%lu)", (unsigned long)rParams.mvFrameParams.size());
        return MFALSE;
    }

    //do not support multi-frame containing vss
    if(rParams.mvFrameParams.size() > 1)
    {
        for(MUINT32 i=0;i<rParams.mvFrameParams.size();i++)
        {
            if(rParams.mvFrameParams[i].mStreamTag == ENormalStreamTag_Vss)
            {
                LOG_ERR("do not support multi-frame containing vss (%d of %d)",i,framePack.frameNum);
                return MFALSE;
            }
        }
    }

    //check vss or not
#if 0
    if(rParams.mvFrameParams[0].mStreamTag == ENormalStreamTag_Vss)
    {
        this->checkVssStatus(); // For Vss Concurrency Check
    }
#endif

    switch(streamPipeID)
    {
        case NSCam::NSIoPipe::EStreamPipeID_Normal:
            framePack.frameNum=rParams.mvFrameParams.size();
            if(rParams.mvFrameParams[0].mTuningData == NULL)
            {
                LOG_WRN("tuning buffer empty");
            }
            this->getLock(ELockEnum_EnqueLock);
            ret = this->doEnque(framePack);
            this->releaseLock(ELockEnum_EnqueLock);
            break;
#if 0  //Justin EP porting 6789
        case NSCam::NSIoPipe::EStreamPipeID_WarpEG:
            //error handle
            if(rParams.mvFrameParams.size() < 1)
            {
                LOG_ERR("WPE unexpected frameParas size(%zu)", rParams.mvFrameParams.size());
                ret = MFALSE;
            }
            else
            {
                ret = this->mWpeStream->WPEenque(rParams, (MUINTPTR)(&callerDeQList));
            }
            break;
#endif
        default:
            LOG_ERR("HalPipeWrapperinit fail : Wrong streamPipeID");
            ret = MFALSE;
            break;
    }


    LOG_DBG("-");
    return ret;

}

NSImageio::NSIspio::ERawPxlID
HalPipeWrapper::
PixelIDMapping(MUINT32 pixIdP2)
{
    NSImageio::NSIspio::ERawPxlID p2hwpixelId = NSImageio::NSIspio::ERawPxlID_B;

    switch (pixIdP2)
    {
        case SENSOR_FORMAT_ORDER_RAW_B:
        {
            p2hwpixelId = NSImageio::NSIspio::ERawPxlID_B;
            break;
        }
        case SENSOR_FORMAT_ORDER_RAW_Gb:
        {
            p2hwpixelId = NSImageio::NSIspio::ERawPxlID_Gb;
            break;
        }
        case SENSOR_FORMAT_ORDER_RAW_Gr:
        {
            p2hwpixelId = NSImageio::NSIspio::ERawPxlID_Gr;
            break;
        }
        case SENSOR_FORMAT_ORDER_RAW_R:
        {
            p2hwpixelId = NSImageio::NSIspio::ERawPxlID_R;
            break;
        }
        default:
        {
            break;
        }
    }
    return p2hwpixelId;
}

MBOOL
HalPipeWrapper::
compareTuningWithWorkingBuffer(char const* filepath, MUINT32* workingBuf, MUINT32* pTuningIspReg, size_t size){
    FILE* outFile = NULL;
    LOG_DBG("save to %s", filepath);
    MBOOL fileOpen=MTRUE;
    outFile = ::fopen(filepath, "w+");
    if  ( outFile == NULL )
    {
        fileOpen=MFALSE;
        LOG_ERR("fail to open %s: %s", filepath, ::strerror(errno));
    }
    MBOOL ret=MTRUE;
    int* pTmpWorkingBuf = (int*)workingBuf;
    int* pTmpTuningBuf = (int*)pTuningIspReg;
    int divsize = size / 4;
    int dipbase = DIP_A_BASE_HW;
    int i = 0;
    for (i=0;i<divsize;i++)
    {
        if((*pTmpWorkingBuf) != (*pTmpTuningBuf)){
            ret=MFALSE;
            if(fileOpen==MTRUE){
                ::fprintf(outFile, "Address:0x%08x,  (workingBuffer, tuningBuffer)=(0x%08x,0x%08x) working != tuning buffer\n",dipbase,*pTmpWorkingBuf,*pTmpTuningBuf);
            }
        }
        pTmpWorkingBuf++;
        pTmpTuningBuf++;
        dipbase = dipbase+4;
    }
    if  ( outFile != NULL )
    {
        ::fclose(outFile);
    }

    return ret;
}

void
HalPipeWrapper::
MDPCallback(void* pParam)
{
    LOG_INF("VSS receice MDP_Callback!!");
    //this->mHalPWrapperDQThread[static_cast<int>(EThreadProperty_DIP_1)]->addCmd(ECmd_ENQUE_VSSFRAME_RDY);
    HalPipeWrapper *_this = reinterpret_cast<HalPipeWrapper*>(pParam);
    _this->mHalPWrapperDQThread[static_cast<int>(EThreadProperty_DIP_1)]->addCmd(ECmd_ENQUE_VSSFRAME_RDY);

}
MBOOL
HalPipeWrapper::
doEnque(FramePackage &framePack)
{
    MBOOL ret=MTRUE;
    LOG_DBG("+");
    CAM_TRACE_BEGIN("doEnque");
    char filename[1024];
    MUINT32 a=0,b=0;

    //list<FramePackage>::iterator eniter;
    //eniter = this->mLDIPEQFramePackList.begin();
    //FramePackage &framePack = (*eniter);

    QParams& qParam=framePack.rParams;

    SensorStaticInfo SensorStaticInfo;
    MINT32 sensorDEVIdx=0;
    MUINT32 mOpenedSensor=0x0;
    //[1] parse frame number
    MINT32 frameNum=framePack.frameNum;
    framePack.dequedNum = 0;
    //judge frame number exceeds supported burst queue number or not

    //NSImageio::NSIspio::EDrvScenario drvScen;
    MUINT32 StreamTag=0x0;
    MUINT32 p2CQ=0x0;//, p2dupCQ=0x0;
    MUINT32 RingBufIndex = 0;

    //[2] do buffer related configuration for each frame unit
    std::vector<DipModuleCfg> mvModule;
    NSImageio::NSIspio::PipePackageInfo pipePackageInfo;
    MUINT32 dmaEnPort=0x0;
    EBufferTag bufTag=EBufferTag_Blocking;
    MUINT32 queriedCQ;
    MUINTPTR pVirIspAddr;
    for(int q=0;q<framePack.frameNum;q++)
    {
        LOG_DBG("start to parse information, (%lu_%lu_%lu)",(unsigned long)qParam.mvFrameParams[q].mvIn.size(),(unsigned long)qParam.mvFrameParams[q].mvOut.size(),(unsigned long)qParam.mvFrameParams[q].mvModuleData.size());
        pVirIspAddr=0x0;
        bufTag=EBufferTag_Blocking;

        //(4) specific module setting
        for(MUINT32 i=0;i< qParam.mvFrameParams[q].mvModuleData.size() ;i++)
        {
            DipModuleCfg module;
            module.eDipModule=static_cast<EDipModule>(qParam.mvFrameParams[q].mvModuleData[i].moduleTag);
            module.moduleStruct=qParam.mvFrameParams[q].mvModuleData[i].moduleStruct;
            mvModule.push_back(module);
        }

        //(5) get cq information
        //drvScen=gSwScenDrvScenMapping[(qParam.mvFrameParams[q].mStreamTag)];
        StreamTag = qParam.mvFrameParams[q].mStreamTag;
        //if(drvScen == NSImageio::NSIspio::eDrvScenario_VSS)
        if(StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Vss)
        {
            bufTag=EBufferTag_Vss;
        }
        queriedCQ=this->mpPostProcPipe->queryCQ(StreamTag, framePack.callerSensorDev, q, framePack.frameNum, RingBufIndex, pVirIspAddr);

        //p2CQ = (queriedCQ&0xffff0000) >> 16;
        p2CQ = queriedCQ&0xffff;
        //p2dupCQ = (queriedCQ&0x0000ffff);
        LOG_INF("StreamTag:%d, CQIdx:%d, RingBufIndex:%d\n", StreamTag, p2CQ, RingBufIndex);


        //(6) do driver configuration and start
        pipePackageInfo.burstQIdx=q;
        pipePackageInfo.frameNum=frameNum;
        pipePackageInfo.p2RingBufIdx=RingBufIndex;
        pipePackageInfo.p2cqIdx=p2CQ;
        pipePackageInfo.vModuleParams=mvModule;
        pipePackageInfo.pExtraParam=&(framePack.rParams.mvFrameParams[q].mvExtraParam);
        pipePackageInfo.MDPCallback = MDPCallback;
        pipePackageInfo.MDPparam = (void *)this;
        if(framePack.rParams.mvFrameParams[q].mTuningData == NULL)
        {
            LOG_WRN("Warning tuning Data size is NULL!!");
            pipePackageInfo.pTuningQue = NULL;
        }
        else
        {
            pipePackageInfo.pTuningQue = framePack.rParams.mvFrameParams[q].mTuningData;
        }

        if(qParam.mvFrameParams[q].mSensorIdx == -1)
        {
            pipePackageInfo.pixIdP2 = this->PixelIDMapping(framePack.pixID);
            LOG_DBG("HMyo(0x%x)_fInfo (%d_%d_%d, %d_%d_%d_%d, %d), dEn(0x%08x)", \
                framePack.callerID, p2CQ, RingBufIndex, q, StreamTag, framePack.callerSensorDev,  pipePackageInfo.pixIdP2, bufTag, framePack.idx4semEQDone, dmaEnPort);
        }
        else
        {
            {
//	                sensorDEVIdx = this->mHalSensorList->querySensorDevIdx(qParam.mvFrameParams[q].mSensorIdx);
                mOpenedSensor=static_cast<MUINT32>(sensorDEVIdx);
//	                this->mHalSensorList->querySensorStaticInfo(mOpenedSensor, &SensorStaticInfo);
                pipePackageInfo.pixIdP2  = this->PixelIDMapping(SensorStaticInfo.sensorFormatOrder);
                LOG_DBG("HMyo qqq (%d), userSensorIdx(0x%x), sensorDEVIdx(0x%x),mOpenedSensor(0x%x),order(0x%x)", q, qParam.mvFrameParams[q].mSensorIdx, sensorDEVIdx,mOpenedSensor,pipePackageInfo.pixIdP2);
                LOG_DBG("HMyo(0x%x)_fInfo (%d_%d_%d, %d_%d_%d_%d_%d, %d), dEn(0x%08x)", \
                    framePack.callerID, p2CQ, RingBufIndex, q, StreamTag, framePack.callerSensorDev, qParam.mvFrameParams[q].mSensorIdx,  pipePackageInfo.pixIdP2, bufTag, framePack.idx4semEQDone, dmaEnPort);
            }
        }
        //pipePackageInfo.drvScen=StreamTag;
        pipePackageInfo.StreamTag=StreamTag;

        if(q == (frameNum-1))
        {
            pipePackageInfo.lastframe=MTRUE;
        }
        else
        {
            pipePackageInfo.lastframe=MFALSE;
        }
        if (this->m_iSaveReqToFile)
        {
            for (MUINT32 i = 0 ; i < qParam.mvFrameParams[q].mvIn.size() ; i++ )
            {
                if (qParam.mvFrameParams[q].mvIn[i].mBuffer->getPlaneCount()>=1)
                {
                    snprintf(filename, 1024, "%s/FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_frm(%d)_vInPorts_[%d]_(0x%x)_w(%d)_h(%d)_stride(%zu,0x0,0x0)_idx(%d)_dir(%d)_Size(0x%08zx)_VA(0x%lx)_PA(0x%lx).%s",DUMP_DIPPREFIX,
                                        qParam.mvFrameParams[q].FrameNo,
                                        qParam.mvFrameParams[q].RequestNo,
                                        qParam.mvFrameParams[q].Timestamp,
                                        qParam.mvFrameParams[q].UniqueKey,
                                        this->m_iEnqueFrmNum,
                                        i,
                                        (NSCam::EImageFormat)(qParam.mvFrameParams[q].mvIn[i].mBuffer->getImgFormat()),
                                        qParam.mvFrameParams[q].mvIn[i].mBuffer->getImgSize().w,
                                        qParam.mvFrameParams[q].mvIn[i].mBuffer->getImgSize().h,
                                        qParam.mvFrameParams[q].mvIn[i].mBuffer->getBufStridesInBytes(0),
                                        qParam.mvFrameParams[q].mvIn[i].mPortID.index,
                                        qParam.mvFrameParams[q].mvIn[i].mPortID.inout,
                                        qParam.mvFrameParams[q].mvIn[i].mBuffer->getBufSizeInBytes(0),
                                        (unsigned long)qParam.mvFrameParams[q].mvIn[i].mBuffer->getBufVA(0),
                                        (unsigned long)qParam.mvFrameParams[q].mvIn[i].mBuffer->getBufPA(0),
                                        "dat");
                }
                switch(qParam.mvFrameParams[q].mvIn[i].mPortID.index)
                {
                    case NSImageio::NSIspio::EPortIndex_IMGI:
                        if (this->m_iSaveReqToFile & (DIP_DUMP_IMGI_BUFFER))
                        {
                            qParam.mvFrameParams[q].mvIn[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_IMGBI:
                        if (this->m_iSaveReqToFile & (DIP_DUMP_IMGBI_BUFFER))
                        {
                            qParam.mvFrameParams[q].mvIn[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_IMGCI:
                        if (this->m_iSaveReqToFile & (DIP_DUMP_IMGCI_BUFFER))
                        {
                            qParam.mvFrameParams[q].mvIn[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_VIPI:
                        if (this->m_iSaveReqToFile & (DIP_DUMP_VIPI_BUFFER))
                        {
                            qParam.mvFrameParams[q].mvIn[i].mBuffer->saveToFile(filename);
                        }
                    break;
                    case NSImageio::NSIspio::EPortIndex_LCEI:
                        if (this->m_iSaveReqToFile & (DIP_DUMP_LCEI_BUFFER))
                        {
                            qParam.mvFrameParams[q].mvIn[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_DEPI:
                        if (this->m_iSaveReqToFile & (DIP_DUMP_DEPI_BUFFER))
                        {
                            qParam.mvFrameParams[q].mvIn[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_DMGI:
                        if (this->m_iSaveReqToFile & (DIP_DUMP_DMGI_BUFFER))
                        {
                            qParam.mvFrameParams[q].mvIn[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_UFDI:
                        if (this->m_iSaveReqToFile & DIP_DUMP_UFDI_BUFFER)
                        {
                            qParam.mvFrameParams[q].mvIn[i].mBuffer->saveToFile(filename);
                        }
                        break;
                }
            }

            if(framePack.rParams.mvFrameParams[q].mTuningData != NULL)
            {
                if (this->m_iSaveReqToFile & (DIP_DUMP_TUNING_BUFFER))
                {
                    sprintf(filename, "%s/FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_Tag(%d)_pTuningQue_frm(%d)_size(%zu).%s",DUMP_DIPPREFIX,
                                        qParam.mvFrameParams[q].FrameNo,
                                        qParam.mvFrameParams[q].RequestNo,
                                        qParam.mvFrameParams[q].Timestamp,
                                        qParam.mvFrameParams[q].UniqueKey,
                                        qParam.mvFrameParams[q].mStreamTag,
                                        this->m_iEnqueFrmNum,
                                        sizeof(dip_a_reg_t),
                                        "dat");
                    saveToFile(filename, (unsigned char*)framePack.rParams.mvFrameParams[q].mTuningData, sizeof(dip_a_reg_t));
                }
                if (this->m_iSaveReqToFile & (DIP_DUMP_TUNING_REG))
                {
                    sprintf(filename, "%s/FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_Tag(%d)_pTuningQue_frm(%d)_size(%zu).%s",DUMP_DIPPREFIX,
                                        qParam.mvFrameParams[q].FrameNo,
                                        qParam.mvFrameParams[q].RequestNo,
                                        qParam.mvFrameParams[q].Timestamp,
                                        qParam.mvFrameParams[q].UniqueKey,
                                        qParam.mvFrameParams[q].mStreamTag,
                                        this->m_iEnqueFrmNum,
                                        sizeof(dip_a_reg_t),
                                        "reg");
                    saveToRegFmtFile(filename, (unsigned char*)framePack.rParams.mvFrameParams[q].mTuningData, sizeof(dip_a_reg_t));
                }

            }
            this->m_iEnqueFrmNum++;

        }
        pipePackageInfo.ForceSkipConfig = 0;
        if ((q >= 1) && (framePack.frameNum > 1))//framePack.frameNum
        {
            //Check tuning buffer address is the same as tuning buffer address of previous frame or not
            if ((framePack.rParams.mvFrameParams[q].mTuningData == framePack.rParams.mvFrameParams[q-1].mTuningData)
                && (framePack.rParams.mvFrameParams[q].mvIn.size() == framePack.rParams.mvFrameParams[q-1].mvIn.size())
                && (framePack.rParams.mvFrameParams[q].mvOut.size() == framePack.rParams.mvFrameParams[q-1].mvOut.size()))
            {
                if (this->mpPostProcPipe->checkFrameParamIsTheSame(framePack.rParams.mvFrameParams[q],framePack.rParams.mvFrameParams[q-1])==MTRUE)
                {
                    //this frame is the same as previous frame if the address of tuning data is the same.
                    pipePackageInfo.ForceSkipConfig = 1;
                    LOG_INF("tuning buffer is the same:%p",framePack.rParams.mvFrameParams[q].mTuningData);
                }
            }
        }

        ret=this->mpPostProcPipe->configPipe(qParam.mvFrameParams[q],&pipePackageInfo, framePack.m_UserName);

        if(this->m_iSaveReqToFile & (DIP_COMP_WB_TUNING_BUF)){
            if(pVirIspAddr != 0)
            {
                    if(framePack.rParams.mvFrameParams[q].mTuningData!=NULL) {
                        sprintf(filename, "%s/CompareWBwithTuning_Configured_HWBuf_FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_Tag(%d)_pTuningQue_frm(%d)_size(%zu).%s",DUMP_DIPPREFIX,
                                            qParam.mvFrameParams[q].FrameNo,
                                            qParam.mvFrameParams[q].RequestNo,
                                            qParam.mvFrameParams[q].Timestamp,
                                            qParam.mvFrameParams[q].UniqueKey,
                                            qParam.mvFrameParams[q].mStreamTag,
                                            this->m_iEnqueFrmNum,
                                            sizeof(dip_a_reg_t),
                                            "reg");
                        compareTuningWithWorkingBuffer(filename, (MUINT32*)pVirIspAddr, (MUINT32*)framePack.rParams.mvFrameParams[q].mTuningData, sizeof(dip_a_reg_t));
                    }
            }
       }
        if(!ret)
        {
            LOG_ERR("postprocPipe config fail");
            //ASSERT, and callback...
            a=getUs();
            if (framePack.rParams.mpfnEnQFailCallback != NULL){
                framePack.rParams.mpfnEnQFailCallback(framePack.rParams);
            }
            b=getUs();
            LOG_ERR("=go enque fail callback(%d us)",b-a);
            return ret;
        }

        //(7) collect information for frame package queue
        FParam frmParam;
        frmParam.p2cqIdx = p2CQ;
        frmParam.p2RingBufIdx = RingBufIndex;
        frmParam.p2burstQIdx = q;
        frmParam.bufTag = bufTag;
        frmParam.frameNum = frameNum;
        //(*eniter).m_FParamVec.push_back(frmParam); //another solution?

        this->getLock(ELockEnum_EQFramePackList);

        if(q==0)
        {

            framePack.m_FParamVec.push_back(frmParam);
            if (bufTag==EBufferTag_Vss)
            {
                this->mDIPVSSFramePackList.push_back(framePack);
            }
            else
            {
                this->mLDIPEQFramePackList.push_back(framePack);
            }
            LOG_DBG("framePack.m_FParamVec.size():%zu, q:%d", framePack.m_FParamVec.size(), q);
        }
        else
        {
            list<FramePackage>::iterator itend;
            if (bufTag==EBufferTag_Vss)
            {
                itend = this->mDIPVSSFramePackList.end();
                itend--;
                (*itend).m_FParamVec.push_back(frmParam);
            }
            else
            {
                itend = this->mLDIPEQFramePackList.end();
                itend--;
                (*itend).m_FParamVec.push_back(frmParam);
            }
            LOG_DBG("framePack.m_FParamVec.size():%lu, q:%d", (*itend).m_FParamVec.size(), q);
        }
        this->releaseLock(ELockEnum_EQFramePackList);


        //(8) notify to do mdp start procedure
        this->getLock(ELockEnum_wDoMdpStartUnitList);
        this->mLDIPwaitDoMDPStartUnitList.push_back(frmParam);
        this->releaseLock(ELockEnum_wDoMdpStartUnitList);
        this->mHalPMdpConfigThread[this->mDIPEQThreadIdx]->addCmd(ECmd_MDPSTART_REQ);

        //[9] free local variables
        mvModule.clear();

    }

    //signal for enque done
    //::sem_post(&(this->mpHalPipeWrapper->mSemEQDone[framePack.idx4semEQDone]));
    CAM_TRACE_END();

    LOG_DBG("-");
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::
deque(
    NSCam::NSIoPipe::EStreamPipeID streamPipeID,
    QParams& rParams,
    MUINTPTR callerID,
    MINT64 i8TimeoutNs)
{
    (void)rParams;
    MBOOL ret = MFALSE;

    MINT32 timeout=i8TimeoutNs/1000000000;
    LOG_DBG("timeout(%d),i8TimeoutNs(%lld),callerID(0x%lx)",timeout,(long long)i8TimeoutNs,(unsigned long)callerID);
    switch(streamPipeID)
    {
        case NSCam::NSIoPipe::EStreamPipeID_Normal:
            //[1] goto check if buffer ready
            LOG_ERR("P2 Do Not Support Blocking Deque from Android O version!!!");

            break;
        case NSCam::NSIoPipe::EStreamPipeID_WarpEG:
            LOG_ERR("WPE DO NOT SUPPORT BLOCKING De-Q...");
            break;

        default:
            LOG_ERR("HalPipeWrapperinit fail : Wrong streamPipeID");
            ret = MFALSE;
            break;
    }

    return ret;
}
/******************************************************************************
*
******************************************************************************/
MBOOL
HalPipeWrapper::
sendCommand(
    MINT32 cmd,
    MINTPTR arg1,
    MINTPTR arg2,
    MINTPTR arg3)
{
    MBOOL ret=MTRUE;
    LOG_INF("+,cmd(0x%x),arg1(0x%lx),arg2(0x%lx),arg3(0x%lx)",cmd,(long)arg1,(long)arg2,(long)arg3);
#if 0
    switch (cmd){
        case NSImageio::NSIspio::EPIPECmd_SET_VENC_DRLINK:
            getLock(ELockEnum_VencPortCnt);
            mVencPortCnt=0;
            LOG_DBG("mVencPortCnt(%d)",mVencPortCnt);
            releaseLock(ELockEnum_VencPortCnt);
            if(mpPostProcPipe)
            {
                //ret=mpPostProcPipe->sendCommand(cmd, arg1, arg2, arg3);
                ret=this->mpDipPipe[_DIP_HW_A_]->sendCommand(cmd, arg1, arg2, arg3);
                if(!ret)
                {
                    LOG_ERR("startVideo Record Fail");
                }
            }
            else
            {
                LOG_ERR("plz do init first");
            }
            break;
        case NSImageio::NSIspio::EPIPECmd_RELEASE_VENC_DRLINK:
            //[1] wait all the buffer-batches containing venc port are deque done
            getLock(ELockEnum_VencPortCnt);
            LOG_INF("+ mVencPortCnt(%d)",mVencPortCnt);
            if(mVencPortCnt>0)
            {
                releaseLock(ELockEnum_VencPortCnt);
                while(1)
                {
                    ::sem_wait(&mSemVencPortCnt);
                    getLock(ELockEnum_VencPortCnt);
                    LOG_DBG("* mVencPortCnt(%d)",mVencPortCnt);
                    if(mVencPortCnt==0)
                    {
                        releaseLock(ELockEnum_VencPortCnt);
                        break;
                    }
                    else
                    {
                        releaseLock(ELockEnum_VencPortCnt);
                    }
                }
            }
            else
            {
                releaseLock(ELockEnum_VencPortCnt);
            }
            //[2] do stop video record
            LOG_INF("- mVencPortCnt(%d)",mVencPortCnt);
            if(mpPostProcPipe)
            {
            	//ret=mpPostProcPipe->sendCommand(cmd, 0, 0, 0);
            	ret=this->mpDipPipe[_DIP_HW_A_]->sendCommand(cmd, 0, 0, 0);
            	if(!ret)
            	{
            		LOG_ERR("stopVideo Record Fail");
            	}
            }
            else
            {
            	LOG_ERR("plz do init first");
            }
            break;
        default:
            //this->mpPostProcPipe->sendCommand(cmd, arg1, arg2, arg3);
            this->mpDipPipe[_DIP_HW_A_]->sendCommand(cmd, arg1, arg2, arg3);
            break;

    }
#endif
    return ret;
}

/******************************************************************************
*
******************************************************************************/
MINT32
HalPipeWrapper::
setFps(
    MINT32 fps)
{
    Mutex::Autolock lock(mLock);

    LOG_INF("fps change (%d -> %d)",this->mFps,fps);
    this->mFps=fps;

    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
HalPipeWrapper_Thread*
HalPipeWrapper_Thread::createInstance(HalPipeWrapper* obj, EThreadType threadType, EThreadProperty threadProperty)
{
    return new HalPipeWrapper_Thread(obj, threadType, threadProperty);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalPipeWrapper_Thread::destroyInstance()
{
    //FUNCTION_LOG_START;

    //
    destroyThread();
    //
    delete this;

    //FUNCTION_LOG_END;

}

/******************************************************************************
 *
 ******************************************************************************/
HalPipeWrapper_Thread::
HalPipeWrapper_Thread(HalPipeWrapper* obj, EThreadType threadType, EThreadProperty threadProperty)
    :mCmdListCond()
    ,mpHalPipeWrapper(NULL)
    ,mpPostProcPipe(NULL)
    ,mThreadType(threadType)
    ,mThreadProperty(threadProperty)
{
    //
    LOG_DBG("+ thread type/property (%d_%d)",threadType,threadProperty);
    mThreadName="";

    this->mpHalPipeWrapper=obj;
    sem_init(&mSemThread, 0, 0);

    //postprocpipe object to do operation
    if(threadType==EThreadType_ENQUE)
    {
        switch(threadProperty)
        {
            case EThreadProperty_DIP_1:
                mThreadName.append("HPTD_EQ_DIP1");
                break;
            case EThreadProperty_WarpEG: //EThreadProperty_DIP_2
                mThreadName.append("HPTD_EQ_DIP2");
                break;
            default:
                    LOG_ERR("_Wrong property(%d)...",mThreadProperty);
                break;
        }
    }
    else if(threadType==EThreadType_MDPSTART)
    {
        switch(threadProperty)
        {
            case EThreadProperty_DIP_1:
                mThreadName.append("HPTD_MDPST_DIP1");
                break;
            case EThreadProperty_WarpEG: //EThreadProperty_DIP_2
                mThreadName.append("HPTD_MDPST_DIP2");
                break;
            default:
                    LOG_ERR("_Wrong property(%d)...",mThreadProperty);
                break;
        }
    }
    else if(threadType==EThreadType_DEQUE)
    {
        switch(threadProperty)
        {
            case EThreadProperty_DIP_1:
                mThreadName.append("HPTD_DQ_DIP1");
                break;
            case EThreadProperty_WarpEG: //EThreadProperty_DIP_2
                mThreadName.append("HPTD_DQ_DIP2");
                break;
            default:
                    LOG_ERR("_Wrong property(%d)...",mThreadProperty);
                break;
        }
    }
    else
    {
        LOG_ERR("_Wrong type(%d)...",mThreadType);
    }

    if(this->mpPostProcPipe==NULL)
    {
        this->mpPostProcPipe=this->mpHalPipeWrapper->getPostProcPipeInstance(_DIP_HW_A_);
        if (NULL == this->mpPostProcPipe)

        {
            LOG_ERR("tp(%d_%d)_mpPostProcPipe createInstance/init fail...",mThreadType,mThreadProperty);
        }
    }
    else
    {
        LOG_ERR("tp(%d_%d)_mpPostProcPipe is not NULL...",mThreadType,mThreadProperty);
    }

    //create thread
    createThread();
    LOG_INF("- tp(%d_%d) createThread", mThreadType,mThreadProperty);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalPipeWrapper_Thread::createThread()
{
    //resetDequeVariables();
    pthread_create(&(this->mThread), NULL, onThreadLoop, this);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalPipeWrapper_Thread::destroyThread()
{
    //post exit means all user call uninit, clear all cmds in cmdlist and add uninit
    clearCmds();
    ECmd cmd=ECmd_UNINIT;
    addCmd(cmd);

    //wait thread jump out
    ::sem_wait(&mSemThread);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalPipeWrapper_Thread::
addCmd(
    ECmd const &cmd)
{
    //signal to vss buffer for enque if there is any incoming enque request
    if(cmd == ECmd_ENQUE_REQ)
    {   //the life cycle of mVssDeQCondMtx is in the following region
        Mutex::Autolock autoLock(mVssEnQCondMtx);
        mVssEnQCond.signal();
    }

    //signal to vss buffer for deque if there is any incoming buffer that is enqued done
    if(cmd == ECmd_ENQUE_RDY)
    {   //the life cycle of mVssDeQCondMtx is in the following region
        Mutex::Autolock autoLock(mVssDeQCondMtx);
        mVssDeQCond.signal();
    }

    Mutex::Autolock autoLock(mThreadMtx);
    //[1] add cmd to list
    mLCmdList.push_back(cmd);
    LOG_DBG("tp(%d_%d) mCmdList, size(%lu) + cmd(%d)",mThreadType,mThreadProperty, (unsigned long)mLCmdList.size(), cmd);
    //[2] broadcast signal to user that a command is in cmd list
    mCmdListCond.broadcast();
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalPipeWrapper_Thread::
clearCmds()
{

    Mutex::Autolock autoLock(mThreadMtx);
    //[1] erase all the command in cmd list
    for (list<ECmd>::iterator it = mLCmdList.begin(); it != mLCmdList.end();)
    {
        //LOG_INF("cmd clear(%d)", *it);
        it = mLCmdList.erase(it);
        it++;
    }

}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper_Thread::
getCmd(
    ECmd &cmd)
{
    Mutex::Autolock autoLock(mThreadMtx);
    //[1] check if there is command in cmd list
    if(mLCmdList.size() <= 0)
    {
        LOG_DBG("tp(%d_%d) no more cmd, block wait",mThreadType,mThreadProperty);
        //always send a signal when all the enqueued buffer are dequeued to solve the problem we mention in doDequeue(pre)
        //mpPostProcPipe->endequeFrameCtrl(NSImageio::NSIspio::EPIPE_BUFQUECmd_WAKE_WAITFRAME,0,0,0);
        mCmdListCond.wait(mThreadMtx);   //wait for someone add enqueue cmd to list
    }

    //[2] get out the first command in correponding cmd list (first in first service)
    if(mLCmdList.size()>0)
    {
        LOG_DBG("tp(%d_%d) mCmdList curSize(%lu), todo(%d)",mThreadType,mThreadProperty, (unsigned long)mLCmdList.size(), *mLCmdList.begin());
        cmd = *mLCmdList.begin();
        mLCmdList.erase(mLCmdList.begin());
        //[3] condition that thread loop keep doing dequeue or not
        if (cmd == ECmd_UNINIT)
        {
            return MFALSE;
        }
        else
        {
            return MTRUE;
        }
    }
    else
    {
        cmd=ECmd_UNKNOWN;
        return MTRUE;
    }

return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalPipeWrapper_Thread::
updateThreadProperty(
    EThreadProperty property)
{
    (void)property;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID*
HalPipeWrapper_Thread::
onThreadLoop(
    MVOID *arg)
{
    //
    HalPipeWrapper_Thread *_this = reinterpret_cast<HalPipeWrapper_Thread*>(arg);

    // set policy/priority
    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_SM_PASS2;
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    if(policy == SCHED_OTHER)
    {   //  Note: "priority" is nice-value priority.
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, policy, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, priority);
    }
    else
    {   //  Note: "priority" is real-time priority.
        sched_p.sched_priority = priority;
        ::sched_setscheduler(0, policy, &sched_p);
    }
    // set name
    ::prctl(PR_SET_NAME,_this->mThreadName.c_str(),0,0,0);

    //  detach thread => cannot be join, it means that thread would release resource after exit
    ::pthread_detach(::pthread_self());

    //[2] do dequeue buf if there is enqueue cmd in cmd list
    ECmd cmd;
    bool ret=true;
    EThreadType threadtype =_this->mThreadType;
    EThreadProperty threadproperty = _this->mThreadProperty;
    while(_this->getCmd(cmd))
    {
        LOG_DBG("tp(%d_%d) cmd(%d)",threadtype,threadproperty,cmd);
        switch(cmd)
        {
            case ECmd_ENQUE_REQ:
                //ret = _this->enqueJudgement();
                if(!ret)
                {
                    LOG_ERR("tp(%d_%d) enqueJudgement fail....",threadtype,threadproperty);
                    //AEE???
                }
                break;
            case ECmd_MDPSTART_REQ:
                ret=_this->doMdpStart();
                if(!ret)
                {
                    LOG_ERR("tp(%d_%d) mdp start fail....",threadtype,threadproperty);
                    //AEE???
                }
                break;
            case ECmd_ENQUE_RDY:
                ret = _this->dequeJudgement(_this->mpHalPipeWrapper->mLDIPEQFramePackList);
                if(!ret)
                {
                    LOG_ERR("tp(%d_%d) dequeJudgement fail....",threadtype,threadproperty);
                    //AEE???
                }
                break;
            case ECmd_ENQUE_VSSFRAME_RDY:
                //ret = _this->dequeJudgement();
                ret = _this->dequeJudgement(_this->mpHalPipeWrapper->mDIPVSSFramePackList);
                if(!ret)
                {
                    LOG_ERR("tp(%d_%d) dequeJudgement fail....",threadtype,threadproperty);
                    //AEE???
                }
                break;
            case ECmd_UNINIT:
                goto EXIT;
                break;
            case ECmd_UNKNOWN:
            default:
                break;
        }
        cmd=ECmd_UNKNOWN;//prevent that condition wait in "getfirstCommand" get fake signal
    }
EXIT:
    _this->clearCmds();
    ::sem_post(&(_this->mSemThread));
    LOG_INF("tp(%d_%d) -",threadtype,threadproperty);
    return NULL;
}
