#ifndef MTK_FLP_LOWPOWER_H
#define MTK_FLP_LOWPOWER_H

#include "mtk_flp_main.h"

#ifdef __cplusplus
extern "C" {
#endif


#define FLP_VERSION_HEAD 'F','L','P','_','V','E','R','_'
#define FLP_MAJOR_VERSION '1','7','0','6','1','9','0','1'  // y,y,m,m,d,d,rev,rev
#define FLP_BRANCH_INFO '_','0','.','5','8','_'
#define FLP_MINER_VERSION 'N','1'
#define FLP_VER_INFO FLP_VERSION_HEAD,FLP_MAJOR_VERSION,FLP_BRANCH_INFO,FLP_MINER_VERSION

#define MTK_FLP_ZPG_NORMAL_SEC 90
#define MTK_FLP_ZPG_MAX_TIMEOUT_SEC 600

int64_t mtk_flp_get_timestamp(struct timeval *ptv);
int mtk_flp_controller_report_loc(MTK_FLP_LOCATION_T outloc);
int mtk_flp_controller_process(MTK_FLP_MSG_T *prmsg) ;
int mtk_flp_controller_query_mode ();
int mtk_flp_controller_reset_status(void);

#ifdef __cplusplus
   }  /* extern "C" */
#endif

#endif
