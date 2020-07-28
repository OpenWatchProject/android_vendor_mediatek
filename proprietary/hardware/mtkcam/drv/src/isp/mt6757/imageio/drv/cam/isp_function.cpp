#define LOG_TAG "iio/ifunc"

#include "isp_function.h"
#include <mtkcam/def/PriorityDefs.h>
//open syscall
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//close syscall
#include <unistd.h>
//mmap syscall
#include <sys/mman.h>

#include <cutils/properties.h>  // For property_get().
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

DECLARE_DBG_LOG_VARIABLE(function);
//EXTERN_DBG_LOG_VARIABLE(function);

// Clear previous define, use our own define.
#undef ISP_FUNC_VRB
#undef ISP_FUNC_DBG
#undef ISP_FUNC_INF
#undef ISP_FUNC_WRN
#undef ISP_FUNC_ERR
#undef ISP_FUNC_AST
#define ISP_FUNC_VRB(fmt, arg...)        do { if (function_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define ISP_FUNC_DBG(fmt, arg...)        do { if (function_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define ISP_FUNC_INF(fmt, arg...)        do { if (function_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define ISP_FUNC_WRN(fmt, arg...)        do { if (function_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define ISP_FUNC_ERR(fmt, arg...)        do { if (function_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define ISP_FUNC_AST(cond, fmt, arg...)  do { if (function_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

MUINT32 IspFunction_B::m_TimeClk = 1;
Mutex   IspFunction_B::fbcPhyLock_IMGO[CAM_MAX];
Mutex   IspFunction_B::fbcPhyLock_RRZO[CAM_MAX];
Mutex   IspFunction_B::fbcPhyLock_UFEO[CAM_MAX];
Mutex   IspFunction_B::fbcPhyLock_AFO[CAM_MAX];
Mutex   IspFunction_B::fbcPhyLock_AAO[CAM_MAX];
Mutex   IspFunction_B::fbcPhyLock_LCSO[CAM_MAX];
Mutex   IspFunction_B::fbcPhyLock_PDO[CAM_MAX];
Mutex   IspFunction_B::fbcPhyLock_EISO[CAM_MAX];
Mutex   IspFunction_B::fbcPhyLock_FLKO[CAM_MAX];
Mutex   IspFunction_B::fbcPhyLock_RSSO[CAM_MAX];
IspFunction_B::IspFunction_B()
{
    m_bypass=MFALSE;
    m_pIspDrv=NULL;
    m_hwModule = MAX_ISP_HW_MODULE;
    DBG_LOG_CONFIG(imageio, function);
}


MINT32 IspFunction_B::config( void )
{
    MINT32 retval = 0;
    if(this->m_bypass == MTRUE){
        ISP_FUNC_INF("%s:bypass this setting\n",this->name_Str());
        return retval;
    }
    retval = this->_config();       //virtual
    if( retval != 0 ){
        ISP_FUNC_ERR("%s:config fail!\n",this->name_Str());
    }
    return retval;
}

MINT32 IspFunction_B::enable( void* pParam )
{
    MINT32 retval;
    retval = this->_enable(pParam);       //virtual
    if( retval != 0 ){
        ISP_FUNC_ERR("%s:_enable fail!\n",this->name_Str());
    }
    return retval;
}

MINT32 IspFunction_B::disable( void* pParam )
{
    MINT32 retval;
    retval = this->_disable(pParam);       //virtual
    if( retval != 0 ){
        ISP_FUNC_ERR("%s:_disable fail!\n",this->name_Str());
    }
    return retval;
}

MINT32 IspFunction_B::write2CQ(void)
{
    MINT32 retval;
    retval = this->_write2CQ();       //virtual
    if( retval != 0 ){
        ISP_FUNC_ERR("%s:_write2CQ fail!\n",this->name_Str());
    }
    return retval;
}








