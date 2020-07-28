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
    , mVencPortCnt(0)
    , mHalSensorList(NULL)
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
        mHalPWrapperEQThread[i]=NULL;
        mHalPMdpConfigThread[i]=NULL;
        mHalPWrapperDQThread[i]=NULL;
    }

    //
    mMutex_VssOccupied=PTHREAD_MUTEX_INITIALIZER;
    mMutex_EQFramePackList=PTHREAD_MUTEX_INITIALIZER;
    mMutex_waitDQFramePackList=PTHREAD_MUTEX_INITIALIZER;
    mMutex_waitDoMdpStartUnitList=PTHREAD_MUTEX_INITIALIZER;
    mMutex_waitDQFrameUnitList=PTHREAD_MUTEX_INITIALIZER;
    mMutex_DQFramePackList=PTHREAD_MUTEX_INITIALIZER;
    mMutex_VencPortCnt=PTHREAD_MUTEX_INITIALIZER;
    mMutex_SemEQDone=PTHREAD_MUTEX_INITIALIZER;
    mHalSensorList=IHalSensorList::get();

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
        		//mP2PixId= p2PixelID;
        		mbVssOccupied=MFALSE;
        		sem_init(&mSemVencPortCnt, 0, 0);
        		sem_init(&mSemVssOccupied, 0, 0);	// For Vss Concurrency Check

        		//
        		sem_index=0;
        		for(int i=0;i<_SEM_EQDONE_NUM_;i++)
        		{
            		sem_init(&(mSemEQDone[i]), 0, 0);
        		}

    		    for (list<FramePackage>::iterator it = mLDIPDQFramePackList.begin(); it != mLDIPDQFramePackList.end();)
        		{
            		it = mLDIPDQFramePackList.erase(it);
            		it++;
        		}

        		//
                //NSImageio::NSIspio::IPostProcPipe* .[2];
                char szTmp[30];
        		for(int i=0;i<_DIP_NUM_;i++)
        		{
            		this->mpDipPipe[i] = NSImageio::NSIspio::IPostProcPipe::createInstance();
                    sprintf(szTmp,"HalPipeWraper_Dip_%d",i);
                    if (NULL ==this->mpDipPipe[i] || !(this->mpDipPipe[i]->init(szTmp, secTag)))
                    {
                        LOG_ERR("HalPipeWrapper PostProc init fail");
                        return MFALSE;
                    }
        		}
                this->mpPostProcPipe = this->mpDipPipe[_DIP_HW_A_];

#if 0
        		this->mpPostProcPipe=NSImageio::NSIspio::IPostProcPipe::createInstance();
        		if (NULL ==this->mpPostProcPipe || !(this->mpPostProcPipe->init("HalPipeWrapper")))
        		{
            		LOG_ERR("HalPipeWrapper PostProc init fail");
            		return MFALSE;
        		}
#endif
        		//create thread for enque / deque
        		for(int i=0;i<(EThreadProperty_NUM);i++)
        		{
            		this->mHalPWrapperEQThread[i]=HalPipeWrapper_Thread::createInstance(this,EThreadType_ENQUE,static_cast<EThreadProperty>(i));
            		this->mHalPMdpConfigThread[i]=HalPipeWrapper_Thread::createInstance(this,EThreadType_MDPSTART,static_cast<EThreadProperty>(i));
            		this->mHalPWrapperDQThread[i]=HalPipeWrapper_Thread::createInstance(this,EThreadType_DEQUE,static_cast<EThreadProperty>(i));
        		}
    		}
			break;
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
    NSCam::NSIoPipe::EStreamPipeID streamPipeID)
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
         		   this->mHalPWrapperEQThread[i]->destroyInstance();
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
                        if (!(this->mpDipPipe[i]->uninit(szTmp)))
                        {
                            LOG_ERR("HalPipeWrapper mpPostProcPipe(%d) uninit fail...", i);
                        }
                        this->mpDipPipe[i]->destroyInstance();
                        this->mpDipPipe[i] = NULL;
                    }

        		}
                this->mpPostProcPipe = NULL;

#if 0

        		if(!this->mpPostProcPipe)
        		{
            		LOG_ERR("HalPipeWrapper mpPostProcPipe is NULL...");
       		    }
        		else
        		{
            		if (!(this->mpPostProcPipe->uninit("HalPipeWrapper")))
            		{
                		LOG_ERR("HalPipeWrapper mpPostProcPipe uninit fail...");
            		}
            		this->mpPostProcPipe->destroyInstance();
            		this->mpPostProcPipe = NULL;
        		}
#endif
        		::sem_destroy(&mSemVencPortCnt);
        		::sem_destroy(&mSemVssOccupied);	// For Vss Concurrency Check
        		//
        		sem_index=0;
        		for(int i=0;i<_SEM_EQDONE_NUM_;i++)
        		{
            		::sem_destroy(&(mSemEQDone[i]));
        		}

#if 0

               // free p2 tuning Queue
               LOG_DBG("Free Tuning Queue");
               misV3=false;
#endif
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
        case ELockEnum_VssOccupied:
            pthread_mutex_lock(&mMutex_VssOccupied);
            break;
        case ELockEnum_EQFramePackList:
            pthread_mutex_lock(&mMutex_EQFramePackList);
            break;
        case ELockEnum_wDQFramePackList:
            pthread_mutex_lock(&mMutex_waitDQFramePackList);
            break;
        case ELockEnum_wDoMdpStartUnitList:
            pthread_mutex_lock(&mMutex_waitDoMdpStartUnitList);
            break;
        case ELockEnum_wDQFrameUnitList:
            pthread_mutex_lock(&mMutex_waitDQFrameUnitList);
            break;
        case ELockEnum_DQFramePackList:
            pthread_mutex_lock(&mMutex_DQFramePackList);
            break;
        case ELockEnum_VencPortCnt:
            pthread_mutex_lock(&mMutex_VencPortCnt);
            break;
        case ELockEnum_SemEQDone:
            pthread_mutex_lock(&mMutex_SemEQDone);
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
        case ELockEnum_VssOccupied:
            pthread_mutex_unlock(&mMutex_VssOccupied);
            break;
        case ELockEnum_EQFramePackList:
            pthread_mutex_unlock(&mMutex_EQFramePackList);
            break;
        case ELockEnum_wDQFramePackList:
            pthread_mutex_unlock(&mMutex_waitDQFramePackList);
            break;
        case ELockEnum_wDoMdpStartUnitList:
            pthread_mutex_unlock(&mMutex_waitDoMdpStartUnitList);
            break;
        case ELockEnum_wDQFrameUnitList:
            pthread_mutex_unlock(&mMutex_waitDQFrameUnitList);
            break;
        case ELockEnum_DQFramePackList:
            pthread_mutex_unlock(&mMutex_DQFramePackList);
            break;
        case ELockEnum_VencPortCnt:
            pthread_mutex_unlock(&mMutex_VencPortCnt);
            break;
        case ELockEnum_SemEQDone:
            pthread_mutex_unlock(&mMutex_SemEQDone);
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
    bool ret = true;
	//get sem index
	int wait_index=0;

    //distribute the requested enque buffer package
    FramePackage framePack;
    framePack.callerPipe=streamPipeID;
    framePack.pixID=pixID;
    if((userName !=NULL) && (strlen(userName) < MAX_USER_NAME_LEN))
    {
        strncpy((char*)framePack.m_UserName,(char const*)userName, strlen(userName));
        framePack.m_UserName[strlen(userName)]='\0';
    }
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
    if(rParams.mvFrameParams[0].mStreamTag == ENormalStreamTag_Vss)
    {
        this->checkVssStatus(); // For Vss Concurrency Check
    }

    switch(streamPipeID)
    {
        case NSCam::NSIoPipe::EStreamPipeID_Normal:
	    	framePack.frameNum=rParams.mvFrameParams.size();
			if(rParams.mvFrameParams[0].mTuningData == NULL)
    		{
        		LOG_WRN("tuning buffer empty");
    		}

			this->getLock(ELockEnum_SemEQDone);
    		wait_index = this->sem_index;
    		framePack.idx4semEQDone = wait_index;
    		this->sem_index = (this->sem_index + 1)%(_SEM_EQDONE_NUM_) ; //shift to another
    		this->releaseLock(ELockEnum_SemEQDone);
            //push to buffer list
            this->getLock(ELockEnum_EQFramePackList);
            this->mLDIPEQFramePackList.push_back(framePack);
            this->releaseLock(ELockEnum_EQFramePackList);
             //send signal to corresponding enque thread
            this->mHalPWrapperEQThread[this->mDIPEQThreadIdx]->addCmd(ECmd_ENQUE_REQ); //should add high frame rate control, pin-pong
            //wait for enque done
			//update enque thread index when handling DIP related buffer
            #if 0
            if(this->mFps >= HIGH_FPS_THRESHOLD)
            {
            	this->mDIPEQThreadIdx=1-this->mDIPEQThreadIdx;
            }
            else
            {
        		//do nothing, usually DIP_1 enque thread is adopted in normal case
        		//mDIPEQThreadIdx=0;
    		}
    		#endif
    		//to avoid middleware enque too fast
    		//usleep(10000);

    		::sem_wait(&(mSemEQDone[wait_index]));
            break;

		case NSCam::NSIoPipe::EStreamPipeID_WarpEG:
			 //error handle
	 		if(rParams.mvFrameParams.size() < 1)
    		{
        		LOG_ERR("WPE unexpected frameParas size(%d)", rParams.mvFrameParams.size());
        		ret = MFALSE;
   			}
	        else
	        {
				this->mWpeStream->WPEenque(rParams, (MUINTPTR)(&callerDeQList));
	        }
			break;

		default:
			LOG_ERR("HalPipeWrapperinit fail : Wrong streamPipeID");
			ret = MFALSE;
			break;
    }


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
    MBOOL ret = MFALSE;

    MINT32 timeout=i8TimeoutNs/1000000000;
    LOG_DBG("timeout(%d),i8TimeoutNs(%lld),callerID(0x%lx)",timeout,(long long)i8TimeoutNs,(unsigned long)callerID);
    switch(streamPipeID)
    {
        case NSCam::NSIoPipe::EStreamPipeID_Normal:
			//[1] goto check if buffer ready
    		MUINT32 a,b;
    		a=getUs();
    		//assume that first enqued p2 dupCQ first out, always pass 0 as p2dupCQidx when deque
    		//ret=this->mpPostProcPipe->bufferQueCtrl(NSImageio::NSIspio::EPIPE_P2BUFQUECmd_WAIT_FRAME, p2Eng, callerID,\
    	    //_no_matter_, _no_matter_ , _no_matter_, _no_matter_, timeout);
            if(this->mpDipPipe[_DIP_HW_A_])
            {
                ret=this->mpDipPipe[_DIP_HW_A_]->bufferQueCtrl(NSImageio::NSIspio::EPIPE_P2BUFQUECmd_WAIT_FRAME, NSImageio::NSIspio::EPIPE_P2engine_DIP, callerID,\
                _no_matter_, _no_matter_ , _no_matter_, _no_matter_, timeout);
            }
            else
            {
                LOG_ERR("deque: plz do init first");
                return MFALSE;
            }
    		b=getUs();
    		LOG_INF("===== ret/period(%d/%d us) =====",ret,b-a);
    		//[2] get the first matched element in dequeue buffer list
    		if(ret)
    		{
        		MBOOL rt=queryFrame(streamPipeID, rParams, callerID);
        		if(!rt)
        		{
            		LOG_ERR("queryFrame fail");
        		}
        		ret = rt;
    		}
    		else
    		{
        		LOG_ERR("bufferQueCtrl fail");
        		ret = MFALSE;
    		}
            break;
        case NSCam::NSIoPipe::EStreamPipeID_WarpEG:
            LOG_ERR("WPE DO NOT SUPPORT BLOCKING De-Q...");
            /*
            p2Eng=NSImageio::NSIspio::EPIPE_P2engine_Warp;
			ret = this->mWpeStream->WPEdeque();
			*/
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
queryFrame(
    NSCam::NSIoPipe::EStreamPipeID streamPipeID,
    QParams& params,
    MINT32 callerID)
{
    bool ret=false;
    MUINT32 cnt=0x0;
    //FUNCTION_LOG_START;

    switch(streamPipeID)
    {
        case NSCam::NSIoPipe::EStreamPipeID_Normal:
        default:
        {
            this->getLock(ELockEnum_DQFramePackList);
            LOG_INF("callerID(0x%x) dBufSize(%d)",callerID,(int)(this->mLDIPDQFramePackList.size()));
            for (list<FramePackage>::iterator it = this->mLDIPDQFramePackList.begin(); it != this->mLDIPDQFramePackList.end();)
            {
                LOG_INF("ggDBG (0x%x_0x%x)",(*it).callerID, callerID);
                if((MINT32)((*it).callerID) == callerID)
                {   //get matched buffer
                    params=(*it).rParams;
                    this->mLDIPDQFramePackList.erase(it);
                    LOG_INF("dequeuedBufListSize(%d)",(int)(this->mLDIPDQFramePackList.size()));
                    ret=true;
                    break;
                }
                it++;
                cnt++;
                LOG_INF("QQ count(%d)",cnt);
            }
            this->releaseLock(ELockEnum_DQFramePackList);
        }
        case NSCam::NSIoPipe::EStreamPipeID_WarpEG:
            break;
    }
    //FUNCTION_LOG_END;
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
* For Vss Concurrency Check
******************************************************************************/
MVOID
HalPipeWrapper::
checkVssStatus()
{
#if 0 //remove only support 1 vss at a time
    getLock(ELockEnum_VssOccupied);
    LOG_DBG("mbVssOccupied(%d)",mbVssOccupied);
    if (mbVssOccupied == true)
    {
        releaseLock(ELockEnum_VssOccupied);
        LOG_INF("sem_wait vss");
        ::sem_wait(&mSemVssOccupied);
        getLock(ELockEnum_VssOccupied);
        mbVssOccupied = true;
        releaseLock(ELockEnum_VssOccupied);
    }
    else
    {
        mbVssOccupied = true;
        releaseLock(ELockEnum_VssOccupied);
        //LOG_INF("sem_trywait");
        ::sem_trywait(&mSemVssOccupied);
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

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
#if 0
    if(!this->mpPostProcPipe)
    {
        LOG_ERR("(%d/%d)_mpPostProcPipe is NULL...",mThreadType,mThreadProperty);
    }
    else
    {
        if (!(this->mpPostProcPipe->uninit(mThreadName.c_str())))
        {
            LOG_ERR("(%d/%d)_mpPostProcPipe uninit fail...",mThreadType,mThreadProperty);
        }
        this->mpPostProcPipe->destroyInstance();
        this->mpPostProcPipe = NULL;
    }
#endif
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
        //this->mpPostProcPipe=NSImageio::NSIspio::IPostProcPipe::createInstance();
        //if (NULL == this->mpPostProcPipe || !(this->mpPostProcPipe->init(mThreadName.c_str())))
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
    property;
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
    {	//  Note: "priority" is nice-value priority.
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, policy, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, priority);
    }
    else
    {	//  Note: "priority" is real-time priority.
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
                ret = _this->enqueJudgement();
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
                ret = _this->dequeJudgement();
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

