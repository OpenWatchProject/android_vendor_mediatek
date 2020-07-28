#define LOG_TAG "DipCmdQMgr"

#include "dip_CmdQMgr.h"
#include "isp_function_dip.h"

#include <unistd.h> //for usleep
#include <vector>
#include <semaphore.h>

#include <cutils/properties.h>  // For property_get().
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif


using namespace std;


DECLARE_DBG_LOG_VARIABLE(DipCQMgr);

// Clear previous define, use our own define.
#undef DipCQMgr_VRB
#undef DipCQMgr_DBG
#undef DipCQMgr_INF
#undef DipCQMgr_WRN
#undef DipCQMgr_ERR
#undef DipCQMgr_AST
#define DipCQMgr_VRB(fmt, arg...)        do { if (DipCQMgr_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define DipCQMgr_DBG(fmt, arg...)        do { if (DipCQMgr_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define DipCQMgr_INF(fmt, arg...)        do { if (DipCQMgr_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define DipCQMgr_WRN(fmt, arg...)        do { if (DipCQMgr_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define DipCQMgr_ERR(fmt, arg...)        do { if (DipCQMgr_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define DipCQMgr_AST(cond, fmt, arg...)  do { if (DipCQMgr_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


//
class DipCmdQMgrImp : public DipCmdQMgr
{
public:
    DipCmdQMgrImp();
    virtual ~DipCmdQMgrImp();
    virtual MBOOL init () ;
    virtual MBOOL uninit () ;
    virtual MBOOL checkCQUserNum(MINT32 p2CQ);
    virtual MBOOL releaseCQUser(MINT32 p2CQ);
    virtual MUINT32 getDupCQIdx(MINT32 p2CQ, MINT32 p2BurstQIndx, MINT32 frameNum, _debug_info_ debug);

private:
    volatile MINT32     mInitCount;
    mutable Mutex mLock;
    pthread_mutex_t mMutex_CQUserNum;                                 //mutex to protect array which cq user number is recorded in
    volatile MINT32          mp2CQDupIdx[ISP_DRV_DIP_CQ_NUM];
    volatile MINT32          mp2CQUserNum[ISP_DRV_DIP_CQ_NUM];  //current enqueued user num for each CQ
    vector<MINT32> enquedCQList; //list for recording enquedCQ in the same package
    sem_t               mSemCQ[ISP_DRV_DIP_CQ_NUM];
};


static DipCmdQMgrImp Singleton;

/**************************************************************************
*
**************************************************************************/
DipCmdQMgr *DipCmdQMgr::createInstance( void )
{
    DipCQMgr_DBG("+");

    DBG_LOG_CONFIG(imageio, DipCQMgr);

    DipCQMgr_DBG("-");
    return &Singleton;
}

/**************************************************************************
*
**************************************************************************/
void DipCmdQMgr::destroyInstance( void )
{

}

/**************************************************************************
*
**************************************************************************/
DipCmdQMgrImp::DipCmdQMgrImp()
{
    DipCQMgr_DBG("+");
    mMutex_CQUserNum=PTHREAD_MUTEX_INITIALIZER;
    for(int i=0;i<ISP_DRV_DIP_CQ_NUM;i++)
    {
        mp2CQDupIdx[i]=0;
        mp2CQUserNum[i]=0;
    }
    DipCQMgr_DBG("-");
}

/**************************************************************************
*
**************************************************************************/
DipCmdQMgrImp::~DipCmdQMgrImp()
{
    DipCQMgr_DBG("+");
    DipCQMgr_DBG("-");
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DipCmdQMgrImp::
init()
{
    MBOOL ret=MTRUE;
    Mutex::Autolock lock(mLock);
    android_atomic_inc(&mInitCount);

    if(mInitCount == 1)
    {
        //initialize variables
        for(int i=0;i<ISP_DRV_DIP_CQ_NUM;i++)
        {
            mp2CQDupIdx[i]=0;
            mp2CQUserNum[i]=0;
            sem_init(&(mSemCQ[i]), 0, 0);
        }
        enquedCQList.resize(0);
    }
    
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DipCmdQMgrImp::
uninit()
{
    MBOOL ret=MTRUE;
    Mutex::Autolock lock(mLock);
    android_atomic_dec(&mInitCount);
    
    return ret;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
DipCmdQMgrImp::
checkCQUserNum(
    MINT32 p2CQ)
{
    pthread_mutex_lock(&mMutex_CQUserNum);
    if(mp2CQUserNum[p2CQ]==MAX_DUP_CQ_NUM)
    {
        pthread_mutex_unlock(&mMutex_CQUserNum);
        return MTRUE;
    }
    else
    {
        pthread_mutex_unlock(&mMutex_CQUserNum);
        return MFALSE;
    }
}

/******************************************************************************
*
******************************************************************************/
MBOOL
DipCmdQMgrImp::
releaseCQUser(
    MINT32 p2CQ)
{

    pthread_mutex_lock(&mMutex_CQUserNum);
    mp2CQUserNum[p2CQ]--;  //update current user number
    DipCQMgr_DBG("Release EnqueueBufList Resource CQ(%d)",p2CQ);
    ::sem_post(& (mSemCQ[p2CQ]));
    pthread_mutex_unlock(&mMutex_CQUserNum);

    return MTRUE;
}


/******************************************************************************
*
******************************************************************************/
MUINT32
DipCmdQMgrImp::
getDupCQIdx(
    MINT32 p2CQ, 
    MINT32 p2BurstQIndx, 
    MINT32 frameNum,
    _debug_info_ debug)
{
    MINT32 dupCQ=0;
    bool enquedCQ=false;

    DipCQMgr_DBG("CQdBG (%d_%d_%d_%d_0x%x), (%ld)",p2CQ, p2BurstQIndx, frameNum, debug.eDrvScen, debug.senDev, (unsigned long)enquedCQList.size());
    //judge cq user (dupCQ-based, only judge the CQ never enqued before in the same package, cuz same CQs would be set in the same dupCq but different burstCQ)
    for(vector<MINT32>::iterator t = enquedCQList.begin(); t != enquedCQList.end(); t++)
    {
        DipCQMgr_DBG("CQdBG temp (%d_%d)",(*t), p2CQ );
        if((*t) == p2CQ)
        {
            enquedCQ=true;
            break;
        }
    }

    DipCQMgr_DBG("CQdBG temp2 (%d)",enquedCQ );
    if(!enquedCQ)
    {
        while(checkCQUserNum(p2CQ))
        {
            DipCQMgr_DBG("Block Enqueue Here, P2CQ (%d), eDrvScen/SenDev(0x%x/0x%x)", p2CQ, debug.eDrvScen, debug.senDev);
            ::sem_wait(& (mSemCQ[p2CQ]));
        }

        pthread_mutex_lock(&mMutex_CQUserNum);
        mp2CQUserNum[p2CQ]++;  //update current user number
        pthread_mutex_unlock(&mMutex_CQUserNum);   
        enquedCQList.push_back(p2CQ);
    }
    //
    dupCQ=mp2CQDupIdx[p2CQ]; //for frames in the current package
    DipCQMgr_DBG("CQdBG temp3 (%d_%d)",dupCQ,mp2CQDupIdx[p2CQ] );

    //reset the list if all frames in package are enqued done
    if(p2BurstQIndx == (frameNum-1))
    {
        enquedCQList.resize(0);
        mp2CQDupIdx[p2CQ]= (mp2CQDupIdx[p2CQ]+1)  % (MAX_DUP_CQ_NUM); //update at last frame in the same package
    }
        
    return dupCQ;
}

