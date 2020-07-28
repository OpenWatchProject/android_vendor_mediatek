/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   meta_gps_para.h
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   define the struct for Meta
 *
 * Author:
 * -------
 *  LiChunhui (MTK80143)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * Mar 20 2009 mtk80143
 * [DUMA00111323] [GPS] modify for GPS META
 * Add for GPS META
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#ifndef __META_GAMMA_PARA_H_
#define __META_GAMMA_PARA_H_

#include "MetaPub.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GAMMA_PQ_ONOFF = 0,
    GAMMA_SET_DISP_PATTERN,
    GAMMA_SET_LUT_SIZE,
    GAMMA_WRITE_LUT,
    GAMMA_WRITE_LUT_FINISH,
    GAMMA_READ_CHECKSUM,
    GAMMA_WRITE_GAIN,
    GAMMA_DUMP_LUT,
    GAMMA_READ_NVRAM,
    GAMMA_READ_REG
} GAMMA_CMD_TYPE;

typedef struct {// parameter: R, G, B lut value saved to NvRAM
	unsigned int rgbindex;
} LUT_RGB_VALUE;

typedef struct {// parameter: R, G, B for display ink pattern
	unsigned int r;
	unsigned int g;
	unsigned int b;
} INK_RGB_PATTERN;

typedef struct {// parameter: notify lut size
	unsigned int size;
} LUT_SIZE;

typedef struct {// parameter: PQ driver on/off control
	unsigned char aalink;
	unsigned char aal;
	unsigned char color;
	unsigned char gamma;
	unsigned char ccorr;
} PQ_ONOFF_CONTROL;

typedef struct {// parameter: indicte which NvRAM section used for save current lut
	unsigned int save_index;
} LUT_SAVE_INDEX;

typedef struct {// parameter: gain table
	unsigned int gain_index;
	unsigned int r;
	unsigned int g;
	unsigned int b;
} GAIN_RGB_VALUE;

typedef union {
	PQ_ONOFF_CONTROL pq_onoff_ctl;
	INK_RGB_PATTERN ink_rgb;
	LUT_SIZE lut_size;
	LUT_RGB_VALUE lut_rgb;
	LUT_SAVE_INDEX lut_save_index;
	GAIN_RGB_VALUE gain_rgb;
} GAMMA_CMD_PARAM;

typedef struct {
    FT_H            header;
    GAMMA_CMD_TYPE  type; // commnad type for specified operation and parameter size
    GAMMA_CMD_PARAM param; // request parameter for further operation
} GAMMA_REQ;
/*------------------------------------------------------------------------------------------------
structure for confirm:
------------------------------------------------------------------------------------------------*/
typedef struct {
	unsigned int status;
} GAMMA_STATUS_GENERAL;

typedef struct {
	unsigned int checksum;
} GAMMA_STATUS_RETURN_CHECKSUM;

typedef union {
	GAMMA_STATUS_RETURN_CHECKSUM status_return_checksum;
} GAMMA_STATUS;

typedef struct {
    FT_H            header;
    GAMMA_CMD_TYPE  type;
    GAMMA_STATUS    feedback;
    unsigned char status;
} GAMMA_CNF;

typedef struct
{
    unsigned int reg;
    unsigned int val;
    unsigned int mask;
} DISP_WRITE_REG;

typedef struct
{
    unsigned int reg;
    unsigned int val;
    unsigned int mask;
} DISP_READ_REG;
#if 0
typedef struct {
	unsigned int id;
	unsigned int lut_node_n; /* node mumber of gamma lut */
	unsigned int lut_size;
	unsigned int lut[1024]; /* gamma table 10bit*3*512  */
	unsigned int lut_extension[128]; /* for extension competitable with 12bit */
	unsigned int gain[10];
	unsigned int checksum; /* for check gamma lut correct or not */
} GAMMA_LUT;
#endif
#define DISP_IOCTL_MAGIC        'x'
#define DISP_IOCTL_WRITE_REG       _IOW     (DISP_IOCTL_MAGIC, 1, DISP_WRITE_REG)   // also defined in ddp_drv.h
#define DISP_IOCTL_READ_REG        _IOWR    (DISP_IOCTL_MAGIC, 2, DISP_READ_REG)    // also defined in ddp_drv.h
#define DISP_IOCTL_MUTEX_CONTROL   _IOW     (DISP_IOCTL_MAGIC, 55 , int)            // also defined in ddp_drv.h
#define DISP_IOCTL_WRITE_SW_REG    _IOW    (DISP_IOCTL_MAGIC, 77, DISP_WRITE_REG)   // also defined in ddp_drv.h
#define DISP_IOCTL_READ_SW_REG     _IOWR   (DISP_IOCTL_MAGIC, 78, DISP_READ_REG)    // also defined in ddp_drv.h

void META_GAMMA_OP(GAMMA_REQ *req, char *peer_buff, unsigned short peer_len);

#ifdef __cplusplus
}
#endif

#endif
