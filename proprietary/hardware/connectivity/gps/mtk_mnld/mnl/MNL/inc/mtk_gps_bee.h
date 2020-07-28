/***********************************************************************
*   This software/firmware and related documentation ("MediaTek Software")
*   are protected under relevant copyright laws. The information contained
*   herein is confidential and proprietary to MediaTek Inc. and/or its licensors.
*
*   Without the prior written permission of MediaTek Inc. and/or its licensors,
*   any reproduction, modification, use or disclosure of MediaTek Software, and
*   information contained herein, in whole or in part, shall be strictly prohibited.
*
*   MediaTek Inc. (C) [2009]. All rights reserved.
*
*************************************************************************/
#ifndef MTK_GPS_BEE_H
#define MTK_GPS_BEE_H


#ifdef __cplusplus
   extern "C" {
#endif

#include "mtk_gps_type.h"


/*****************************************************************************
 * FUNCTION
 *  mtk_gps_bee_init
 * DESCRIPTION
 *  Initialize BEE module
 * PARAMETERS
 *  path             [IN]  single-byte null-terminated string of BEET000A folder path
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_bee_init(UINT8 *path);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_bee_uninit
 * DESCRIPTION
 *  Un-initialize BEE module
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_bee_uninit(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_bee_gen
 * DESCRIPTION
 *  Generate BEE data
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_bee_gen(void);


#ifdef __cplusplus
   }  /* extern "C" */
#endif

#endif

