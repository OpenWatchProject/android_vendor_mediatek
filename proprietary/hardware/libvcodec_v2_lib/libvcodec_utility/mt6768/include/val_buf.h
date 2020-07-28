#ifndef _VAL_BUF_H_
#define _VAL_BUF_H_


#include "venc_drv_if_public.h"
#ifdef __cplusplus
extern "C" {
#endif


VAL_BOOL_T eVideoQueueOutput(VENC_DRV_DONE_LIST_T *pDoneList, P_VENC_DRV_PARAM_FRM_BUF_T a_prFrameBuf, P_VENC_DRV_PARAM_BS_BUF_T a_prBitstreamBuf,  VENC_DRV_DONE_RESULT_T *a_prResult);
VAL_BOOL_T eVideoDeQueueOutput(VENC_DRV_DONE_LIST_T *pDoneList, VENC_DRV_DONEBUFFER_T *pDoneBuf);
VAL_BOOL_T eVideoInitOutputList(VENC_DRV_DONE_LIST_T *pDoneList);
VAL_BOOL_T eVideoDeInitOutputList(VENC_DRV_DONE_LIST_T *pDoneList);




#ifdef __cplusplus
}
#endif

#endif /* #ifndef _VAL_BUF_H_ */
