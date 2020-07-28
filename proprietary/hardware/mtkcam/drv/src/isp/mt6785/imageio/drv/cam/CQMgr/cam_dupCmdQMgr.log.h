#ifndef _CAM_DUPCMDQMGR_LOG_H_
#define _CAM_DUPCMDQMGR_LOG_H_
#undef LOG_TAG
#define LOG_TAG "CmdQMgr"


#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif


EXTERN_DBG_LOG_VARIABLE(CmdQMgr);

// Clear previous define, use our own define.
#undef CmdQMgr_VRB
#undef CmdQMgr_DBG
#undef CmdQMgr_INF
#undef CmdQMgr_WRN
#undef CmdQMgr_ERR
#undef CmdQMgr_AST

#define CmdQMgr_VRB(fmt, arg...)        do { if (CmdQMgr_DbgLogEnable_VERBOSE) { BASE_LOG_VRB("[0x%x]:" fmt,this->mModule, ##arg); } } while(0)
#define CmdQMgr_DBG(fmt, arg...)        do { if (CmdQMgr_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG("[0x%x]:" fmt,this->mModule, ##arg); } } while(0)
#define CmdQMgr_INF(fmt, arg...)        do { if (CmdQMgr_DbgLogEnable_INFO   ) { BASE_LOG_INF("[0x%x]:" fmt,this->mModule, ##arg); } } while(0)
#define CmdQMgr_WRN(fmt, arg...)        do { if (CmdQMgr_DbgLogEnable_WARN   ) { BASE_LOG_WRN("[0x%x]:" fmt,this->mModule, ##arg); } } while(0)
#define CmdQMgr_ERR(fmt, arg...)        do { if (CmdQMgr_DbgLogEnable_ERROR  ) { BASE_LOG_ERR("[0x%x]:" fmt,this->mModule, ##arg); } } while(0)
#define CmdQMgr_AST(cond, fmt, arg...)  do { if (CmdQMgr_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#endif
