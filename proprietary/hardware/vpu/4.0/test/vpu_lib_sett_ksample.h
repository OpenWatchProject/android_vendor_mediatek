/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _VPU_LIB_SETT_KSAMPLE_H_
#define _VPU_LIB_SETT_KSAMPLE_H_

/****************************************************************************************************************************************************
 * Provide library attribute
 *  1. port_prop_descs: given a descriptor o fhow many input/output port(buffer) this library needed with buffer name string.
 *  2. info_prop_descs: given a descriptor of what properties this library provide. ex. "version", "support_format", etc...
 *  3. sett_prop_descs: given a descriptor of setting structure this library needed for algo. processing. ex. "tileWidth", "tileHeight", etc...
 *  4. vpu_st_sett_ksample: given a setting parameter structure.
****************************************************************************************************************************************************/

#if defined( _VPU_LIB_ )
/***************************************************************************************
*******************************   USER Modify Area       *******************************
***************************************************************************************/
#ifdef SM_SRAM
#undef SM_SRAM
#define  SM_SRAM(align)
#endif

#define INS_PROP(id, type, access, count, name ) \
		{ id, VPU_PROP_TYPE_ ## type, VPU_PROP_ACCESS_ ## access, 0, count, name }

#define PORT_DESC(id, usage, dir, name) \
		{ id, usage, dir, name }

#define PORT_NAME(name) name"\0"
/****************************************************************************************************************************************************
*  1. port_prop_descs: give a descriptor o fhow many input/output port(buffer) this library needed with buffer name string.
****************************************************************************************************************************************************/
#define TM_NUM_PORT 2
static vpu_port_t port_prop_descs[TM_NUM_PORT] SM_SRAM(1) = {
	PORT_DESC(0, VPU_PORT_USAGE_IMAGE, VPU_PORT_DIR_IN , PORT_NAME("i_U16_ImgG") 	),
	PORT_DESC(1, VPU_PORT_USAGE_IMAGE, VPU_PORT_DIR_OUT, PORT_NAME("o_S16_ImgOut") 	),
};

/****************************************************************************************************************************************************
*  2. info_prop_descs: given a descriptor of what properties this library provide. ex. "version", "support_format", etc...
****************************************************************************************************************************************************/
#define TM_NUM_INFO 4
static vpu_prop_desc_t info_prop_descs[TM_NUM_INFO] SM_SRAM(1) = {
	INS_PROP(0, INT32, RDONLY, 1,  "version"       ),
	INS_PROP(1, INT32, RDONLY, 1,  "wrk_buf_size"  ),
	INS_PROP(2, INT32, RDONLY, 10, "support_format"),
	INS_PROP(3, INT32, RDONLY, 64, "reserved"      ),
};

/****************************************************************************************************************************************************
*  3. sett_prop_descs: given a descriptor of setting structure this library needed for algo. processing. ex. "tileWidth", "tileHeight", etc...
****************************************************************************************************************************************************/
#define TM_NUM_SETT 3
static vpu_prop_desc_t sett_prop_descs[TM_NUM_SETT] SM_SRAM(1)= {
	INS_PROP(2, INT32, RDWR  , 1, "S32_tileWidth" ),
	INS_PROP(3, INT32, RDWR  , 1, "S32_tileHeight"),
	INS_PROP(4, INT32, RDWR  , 1, "S32_Stride"	 ),
};

#endif //#if defined( _VPU_LIB_ )

/****************************************************************************************************************************************************
*  4. ST_SETT_KSAMPLE: given a setting parameter structure.
****************************************************************************************************************************************************/
typedef unsigned long long	UINT64;


struct vpu_st_sett_ksample {
	int S32_Wight;
	int S32_Height;
	int S32_Op;
	int S32_PAD;
	UINT64 U64_TotalLength;

};

#endif
