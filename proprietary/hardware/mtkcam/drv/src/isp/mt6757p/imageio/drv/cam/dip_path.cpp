#define LOG_TAG "iio/path"
//
//#define _LOG_TAG_LOCAL_DEFINED_
//#include <my_log.h>
//#undef  _LOG_TAG_LOCAL_DEFINED_
//
#include "cam_path_dip.h"
#include <utils/Trace.h> //for systrace

//

#if 0

#include <cutils/properties.h>  // For property_get().


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{Path}"
#include "imageio_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(path);
//EXTERN_DBG_LOG_VARIABLE(path);

// Clear previous define, use our own define.
#undef DIP_PATH_VRB
#undef DIP_PATH_DBG
#undef DIP_PATH_INF
#undef DIP_PATH_WRN
#undef DIP_PATH_ERR
#undef DIP_PATH_AST
#define DIP_PATH_VRB(fmt, arg...)        do { if (path_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define DIP_PATH_DBG(fmt, arg...)        do { if (path_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define DIP_PATH_INF(fmt, arg...)        do { if (path_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define DIP_PATH_WRN(fmt, arg...)        do { if (path_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define DIP_PATH_ERR(fmt, arg...)        do { if (path_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define DIP_PATH_AST(cond, fmt, arg...)  do { if (path_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)
#else
#include <cutils/properties.h>  // For property_get().

#include "imageio_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

DECLARE_DBG_LOG_VARIABLE(p2pathDip);

// Clear previous define, use our own define.
#undef DIP_PATH_VRB
#undef DIP_PATH_DBG
#undef DIP_PATH_INF
#undef DIP_PATH_WRN
#undef DIP_PATH_ERR
#undef DIP_PATH_AST
#define DIP_PATH_VRB(fmt, arg...)        do { if (p2pathDip_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define DIP_PATH_DBG(fmt, arg...)        do { if (p2pathDip_DbgLogEnable_VERBOSE && p2pathDip_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define DIP_PATH_INF(fmt, arg...)        do { if (p2pathDip_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define DIP_PATH_WRN(fmt, arg...)        do { if (p2pathDip_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define DIP_PATH_ERR(fmt, arg...)        do { if (p2pathDip_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define DIP_PATH_AST(cond, fmt, arg...)  do { if (p2pathDip_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

//EXTERN_DBG_LOG_VARIABLE(path);

#endif


/*-----------------------------------------------------------------------------
    MACRO Definition
  -----------------------------------------------------------------------------*/
#if 1
#define PRINT_ELEMENT_INVOLVED( _title_str_, _isplist_, _ispcount_, _mask_, _b_from_head_ )\
{\
    int  _i,_count;    char _temp_str[512];\
    _temp_str[0] = '\0';\
    _count = sizeof(_temp_str) - strlen(_temp_str);\
    strncat( _temp_str, _title_str_, _count-1);\
    if( _b_from_head_ ) {\
        for( _i = 0; _i < _ispcount_; _i++ )    {\
            if( ( _mask_ !=0  )    &&   ( ( _mask_ & _isplist_[_i]->id() ) == 0 )    ){\
                    continue;       }\
            _count = sizeof(_temp_str) - strlen(_temp_str);\
            strncat( _temp_str,"->",_count-1);\
            _count = sizeof(_temp_str) - strlen(_temp_str);\
            strncat( _temp_str,_isplist_[_i]->name_Str(),_count-1);\
        }\
    } else {\
        for( _i = (_ispcount_-1) ; _i >= 0 ; _i-- )    {\
            if( ( _mask_ !=0  )    &&   ( ( _mask_ & _isplist_[_i]->id() ) == 0 )    ){\
                    continue;       }\
            _count = sizeof(_temp_str) - strlen(_temp_str);\
            strncat( _temp_str,"->",_count-1);\
            _count = sizeof(_temp_str) - strlen(_temp_str);\
            strncat( _temp_str,_isplist_[_i]->name_Str(),_count-1);\
        }\
    }\
    DIP_PATH_DBG("%s",_temp_str);\
}
#else
#define PRINT_ELEMENT_INVOLVED
#endif

/*-----------------------------------------------------------------------------
    Functions
  -----------------------------------------------------------------------------*/

DipPath_B::
DipPath_B():dipTh(DIP_ISP_CQ_NONE)
{
    DBG_LOG_CONFIG(imageio, p2pathDip);
    //CQ_D = DIP_ISP_CQ_NONE;
}


int DipPath_B::start( void* pParam )
{
    DIP_PATH_DBG("{%s}::Start E" DEBUG_STR_BEGIN "", this->name_Str() );

    //Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    int ret = 0;

    if( this->_start( pParam ) < 0 ) {
        DIP_PATH_ERR("[ERROR] start(%s) ",this->name_Str());
        ret = -1;
        goto EXIT;
    }
    DIP_PATH_DBG(DEBUG_STR_END"%s::Start""", this->name_Str() );

EXIT:

    DIP_PATH_DBG(":X ");

    return ret;
}

int DipPath_B::stop( void* pParam )
{
    DIP_PATH_INF("{%s}::Stop E" DEBUG_STR_BEGIN "", this->name_Str() );

    //Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    int ret = 0;

    if( this->_stop( pParam ) < 0 ){
        DIP_PATH_ERR("[ERROR] stop(%s) ",this->name_Str());
        //return -1;
        ret = -1;
        goto EXIT;
    }
    DIP_PATH_DBG(DEBUG_STR_END"%s::Stop""", this->name_Str() );

    EXIT:

    DIP_PATH_DBG(":X ");

    return ret;
}
//


int DipPath_B::_config( void* pParam )
{
    ISP_TRACE_CALL();


    int             ret_val = ISP_ERROR_CODE_OK;
    int             i;
    IspFunctionDip_B**  isplist;
    int             ispcount;
    struct CamPathPass2Parameter* pPass2 = (struct CamPathPass2Parameter*)pParam;

    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    //config
    PRINT_ELEMENT_INVOLVED("[_config]:", isplist, ispcount, 0 , 1 );
    for( i = 0; i < ispcount; i++ )
    {
        if (isplist[i]->is_bypass()) {
            DIP_PATH_DBG("[%d]<%s> bypass:",i,isplist[i]->name_Str());
            continue;
        }

        DIP_PATH_DBG("<%s> config:",isplist[i]->name_Str());
        if( ( ret_val = isplist[i]->config() ) < 0 )
        {
            DIP_PATH_ERR("[ERROR] _config(%s) ",isplist[i]->name_Str());
            goto EXIT;
        }

        if( ( ret_val = isplist[i]->write2CQ() ) < 0 )
        {
            DIP_PATH_ERR("[ERROR] _config dipTh(%d) ",this->dipTh);
            goto EXIT;
        }

    }


EXIT:

    DIP_PATH_DBG(":X ");


    return ret_val;
}
//
int DipPath_B::_setZoom( void* pParam )
{
    DIP_PATH_DBG("_setZoom E ");


    int             ret_val = ISP_ERROR_CODE_OK;
    int             i;
    IspFunctionDip_B**  isplist;
    int             ispcount;


    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    //setZoom (From isp to cdp)
    PRINT_ELEMENT_INVOLVED("[_setZoom]:", isplist, ispcount, 0 , 1 );
    for( i = 0; i < ispcount; i++ )
    {
        if (isplist[i]->is_bypass()) {
            DIP_PATH_DBG("<%s> bypass:",isplist[i]->name_Str());
            continue;
        }

        DIP_PATH_DBG("<%s> setZoom:",isplist[i]->name_Str());
        if( ( ret_val = isplist[i]->setZoom() ) < 0 ) {
            DIP_PATH_ERR("[ERROR] _setZoom(%s) ",isplist[i]->name_Str());
            //return ret_val;
            goto EXIT;
        }

    }

    EXIT:

    DIP_PATH_DBG(":X ");

    return ret_val;
}
//
int DipPath_B::_start( void* pParam )
{
    DIP_PATH_DBG("+,_start E ");
    int             ret_val = ISP_ERROR_CODE_OK;
    int             i;

    if( ( ret_val = ispTopCtrl.enable(pParam) ) < 0 ) {
        DIP_PATH_ERR("[ERROR] _start enable ");
        ret_val = ISP_ERROR_CODE_FAIL;
        goto EXIT;
    }

EXIT:
    DIP_PATH_DBG("-");
    return ret_val;
}

int DipPath_B::_stop( void* pParam )
{
    DIP_PATH_DBG("+,_stop");

    int ret = 0;
    int     ret_val = ISP_ERROR_CODE_OK;
    int     i;
    if( ( ret_val = ispTopCtrl.disable(pParam) ) < 0 )
    {
        DIP_PATH_ERR("[ERROR] _stop enable ");
        ret_val = ISP_ERROR_CODE_FAIL;
        goto EXIT;
    }
EXIT:

    DIP_PATH_DBG("-");
    return ret;
}//


int DipPath_B::_registerIrq( NSImageio::NSIspio::Irq_t irq )
{
    return 0;
}
int DipPath_B::_waitIrq( NSImageio::NSIspio::Irq_t irq )
{
    return 0;
}


int DipPath_B::_end( void* pParam )
{
int ret = 0;
    return ret;
}

//
int DipPath_B::enqueueBuf( MUINT32 const dmaChannel, stISP_BUF_INFO bufInfo,MINT32 const enqueCq,MINT32 const dupCqIdx)
{
    //DIP_PATH_DBG("+,dmaChannel(%d)",dmaChannel);

    //Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    int ret = 0;
    //
    if ( 0 != this->ispBufCtrl.enqueueHwBuf(dmaChannel, bufInfo,enqueCq,dupCqIdx) ) {
        DIP_PATH_ERR("ERROR:enqueueHwBuf");
        goto EXIT;
        //return -1;
    }

    EXIT:

    //DIP_PATH_DBG("-,");

    //
    return ret;
}


int DipPath_B::enqueueBuf( MUINT32 const dmaChannel, stISP_BUF_INFO bufInfo, MVOID* private_data, MBOOL bImdMode )
{
    //DIP_PATH_DBG("+,dmaChannel(%d)",dmaChannel);

    //Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    int ret = 0;
    //
    if ( 0 != this->ispBufCtrl.enqueueHwBuf(dmaChannel, bufInfo, private_data,bImdMode) ) {
        DIP_PATH_ERR("ERROR:enqueueHwBuf");
        goto EXIT;
        //return -1;
    }

    EXIT:

    //DIP_PATH_DBG("-,");

    //
    return ret;
}



//
int DipPath_B::freePhyBuf( MUINT32 const mode, stISP_BUF_INFO bufInfo )
{
    DIP_PATH_DBG("freePhyBuf E ");

    //Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    int ret = 0;
    if ( ePathFreeBufMode_SINGLE == mode ) {
        this->ispBufCtrl.freeSinglePhyBuf(bufInfo);
    }
    else if ( ePathFreeBufMode_ALL == mode ) {
        this->ispBufCtrl.freeAllPhyBuf();
    }

    EXIT:

    DIP_PATH_DBG(":X ");

    return ret;
}

//

int DipPath_B::flushCqDescriptor( MUINT32 cq, MUINT32 burstQueIdx, MUINT32 dupCqIdx)
{
    DIP_PATH_INF("cq(%d),burstQueIdx(%d),dupCqIdx(%d)",cq,burstQueIdx,dupCqIdx);

    int ret = 0;
#if 0 //kk test
    if(cq < ISP_DRV_BASIC_CQ_NUM) {
        for(int i=0; i<CAM_DUMMY_; i++)
            this->ispTopCtrl.m_pIspDrvShell->cqDelModule((ISP_DRV_CQ_ENUM)cq, burstQueIdx, dupCqIdx, (CAM_MODULE_ENUM)i);
    } else {
        DIP_PATH_WRN("[warning]not support this cq(%d)",cq);
    }
#endif

    return ret;
}


//




