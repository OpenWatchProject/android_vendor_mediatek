#define LOG_TAG "iio/path"
//
//#define _LOG_TAG_LOCAL_DEFINED_
//#include <my_log.h>
//#undef  _LOG_TAG_LOCAL_DEFINED_
//
#include "cam_path.h"
#include <utils/Trace.h> //for systrace

using android::Mutex;

//

#if 0

#include <cutils/properties.h>  // For property_get().


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{Path}"
#include "imageio_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(path);
//EXTERN_DBG_LOG_VARIABLE(path);

// Clear previous define, use our own define.
#undef ISP_PATH_VRB
#undef ISP_PATH_DBG
#undef ISP_PATH_INF
#undef ISP_PATH_WRN
#undef ISP_PATH_ERR
#undef ISP_PATH_AST
#define ISP_PATH_VRB(fmt, arg...)        do { if (path_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define ISP_PATH_DBG(fmt, arg...)        do { if (path_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define ISP_PATH_INF(fmt, arg...)        do { if (path_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define ISP_PATH_WRN(fmt, arg...)        do { if (path_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define ISP_PATH_ERR(fmt, arg...)        do { if (path_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define ISP_PATH_AST(cond, fmt, arg...)  do { if (path_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)
#else
#include <cutils/properties.h>  // For property_get().

#include "imageio_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

DECLARE_DBG_LOG_VARIABLE(path);

#endif


/*-----------------------------------------------------------------------------
    MACRO Definition
  -----------------------------------------------------------------------------*/
#if 0
#define PRINT_ELEMENT_INVOLVED( _title_str_, _isplist_, _ispcount_, _mask_, _b_from_head_ )\
{\
    int  _i;    char _temp_str[512];\
    _temp_str[0] = '\0';\
    strcat( _temp_str, _title_str_ );\
    if( _b_from_head_ ) {\
        for( _i = 0; _i < _ispcount_; _i++ )    {\
            if( ( _mask_ !=0  )    &&   ( ( _mask_ & _isplist_[_i]->id() ) == 0 )    ){\
                    continue;       }\
            strcat( _temp_str,"->");\
            strcat( _temp_str,_isplist_[_i]->name_Str());\
        }\
    } else {\
        for( _i = (_ispcount_-1) ; _i >= 0 ; _i-- )    {\
            if( ( _mask_ !=0  )    &&   ( ( _mask_ & _isplist_[_i]->id() ) == 0 )    ){\
                    continue;       }\
            strcat( _temp_str,"->");\
            strcat( _temp_str,_isplist_[_i]->name_Str());\
        }\
    }\
    ISP_PATH_DBG("%s",_temp_str);\
}
#else
#define PRINT_ELEMENT_INVOLVED
#endif


/*-----------------------------------------------------------------------------
    Functions
  -----------------------------------------------------------------------------*/

CamPath_B::
CamPath_B()
{
    DBG_LOG_CONFIG(imageio, path);
    m_hwModule = MAX_ISP_HW_MODULE;
    m_FSM = op_unknown;
}


MINT32 CamPath_B::start( void* pParam )
{
    ISP_PATH_DBG("{%s}::Start:0x%x E" DEBUG_STR_BEGIN "", this->name_Str(),this->m_hwModule);


    int ret = 0;

    if( this->_start( pParam ) < 0 ) {
        ISP_PATH_ERR("[ERROR] start(%s) ",this->name_Str());
        ret = -1;
        goto EXIT;
    }
    ISP_PATH_DBG(DEBUG_STR_END"%s::Start""", this->name_Str() );

    this->m_FSM = op_start;
EXIT:

    ISP_PATH_DBG(":X ");

    return ret;
}

MINT32 CamPath_B::stop( void* pParam )
{
    ISP_PATH_INF("{%s}::Stop:0x%x E" DEBUG_STR_BEGIN "", this->name_Str(),this->m_hwModule);


    int ret = 0;

    if( this->_stop( pParam ) < 0 ){
        ISP_PATH_ERR("[ERROR] stop(%s) ",this->name_Str());
        //return -1;
        ret = -1;
        goto EXIT;
    }
    ISP_PATH_DBG(DEBUG_STR_END"%s::Stop""", this->name_Str() );

    this->m_FSM = op_stop;
    EXIT:

    ISP_PATH_DBG(":X ");

    return ret;
}



MINT32 CamPath_B::_config( void* pParam )
{
    ISP_TRACE_CALL();

    ISP_PATH_DBG("[_config:0x%x]: E ",this->m_hwModule);


    MINT32      ret_val = ISP_ERROR_CODE_OK;
    IspFunction_B**  isplist;
    MINT32      ispcount;
    pParam;
    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();


    //config
//    PRINT_ELEMENT_INVOLVED("[_config]:", isplist, ispcount, 0 , 1 );
    for(int i = 0; i < ispcount; i++ )
    {
        if (isplist[i]->is_bypass()) {
            ISP_PATH_DBG("[%d]<%s> bypass:",i,isplist[i]->name_Str());
            continue;
        }

        ISP_PATH_DBG("<%s> config:",isplist[i]->name_Str());
        if( ( ret_val = isplist[i]->config() ) < 0 )
        {
            ISP_PATH_ERR("[ERROR] _config(%s) ",isplist[i]->name_Str());
            goto EXIT;
        }

        if( ( ret_val = isplist[i]->write2CQ() ) < 0 )
        {
            ISP_PATH_ERR("[ERROR] _config  ");
            goto EXIT;
        }
    }

EXIT:

    ISP_PATH_DBG(":X ");


    return ret_val;
}
//
MINT32 CamPath_B::_start( void* pParam )
{
    ISP_PATH_DBG("_start:0x%x E ",this->m_hwModule);

    MINT32  ret_val = ISP_ERROR_CODE_OK;
    IspFunction_B**  isplist;
    MINT32  ispcount;
    MINT32  isp_start = 0;

    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();


    if( ret_val < 0 ) {
        ISP_PATH_ERR("[ERROR] _start ");
        goto EXIT;
        //return ret_val;
    }

    for(int i = (ispcount-1) ; i >= 0 ; i-- )
    {
        //ISP_PATH_DBG("<%s> enable.",isplist[i]->name_Str() );
        if( ( ret_val = isplist[i]->enable((void*)pParam ) ) < 0 ) {
            ISP_PATH_ERR("[ERROR] _start enable ");
            goto EXIT;
            //return ret_val;
        }
    }

    EXIT:

    ISP_PATH_DBG(":X ");

    return ret_val;
}

MINT32 CamPath_B::_stop( void* pParam )
{
    ISP_PATH_DBG("_stop:0x%x E ",this->m_hwModule);

    MINT32  ret = 0;
    IspFunction_B**  isplist;
    int             ispcount;

    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    //3.disable (From head to tail)
 //   PRINT_ELEMENT_INVOLVED("[disable]:", isplist, ispcount, 0, 1 );
    for( int i = 0; i < ispcount; i++ )
    {
        //ISP_PATH_DBG("<%s> disable.",isplist[i]->name_Str() );
        if( ( ret = isplist[i]->disable() ) < 0 ){
            ISP_PATH_ERR("_stop fail\n");
            goto EXIT;
        }
    }

    EXIT:

    ISP_PATH_DBG(":X ");

    return ret;
}//


