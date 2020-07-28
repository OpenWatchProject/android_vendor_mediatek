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


/*******************************************************************************
 *
 * Filename:
 * ---------
 *   CFG_PRODUCT_INFO_File.h
 *
 * Project:
 * --------
 *   YuSu
 *
 * Description:
 * ------------
 *    header file of main function
 *
 * Author:
 * -------
 *   Yuchi Xu(MTK81073)
 *
 *------------------------------------------------------------------------------
 *
 *******************************************************************************/



#ifndef _CFG_PRODUCT_INFO_FILE_H
#define _CFG_PRODUCT_INFO_FILE_H


// the record structure define of PRODUCT_INFO nvram file
typedef struct
{
    unsigned char imei[8];
    unsigned char svn;
    unsigned char pad;
} nvram_ef_imei_imeisv_struct;

typedef struct
{
    unsigned char BTAddr[6];
    unsigned char WifiAddr[6];
    unsigned char ADBSeriaNo[20];
    unsigned char revered[32];
} s_target_info_record;

typedef struct _s_ate_gsm_flag
{
    unsigned char        ate_gsm_flag[4];
} s_ate_gsm_flag;         // 4Bytes

typedef struct _s_ate_tdscdma_flag
{
    unsigned char        ate_tdscdma_flag[2];

} s_ate_tdscdma_flag;    // 2Bytes

typedef struct _s_ate_wcdma_flag
{
    unsigned char        ate_wcdma_flag[2];
} s_ate_wcdma_flag;      // 2Bytes

typedef struct _s_ate_lte_flag
{
    unsigned char        ate_lte_flag[2];

} s_ate_lte_flag;    // 2Bytes

typedef struct _s_ate_cdma_flag
{
    unsigned char        ate_cdma_flag[2];

} s_ate_cdma_flag;      // 2Bytes

typedef struct _s_ate_nsft_flag
{
    unsigned char        ate_nsft_flag[2];
} s_ate_nsft_flag;      // 2Bytes

typedef struct _s_ate_wireless_flag
{
    unsigned char        ate_wireless_flag[2];
} s_ate_wireless_flag;  // 2Bytes

typedef struct s_ate_other_flag
{
    unsigned char        ate_other_flag[1];
} s_ate_other_flag;     // 1Bytes

typedef struct _s_ate_flag
{
    unsigned char                    sw_index;
    unsigned char                    md_index;
    s_ate_gsm_flag        	gsm_flag;
    s_ate_tdscdma_flag  	tdscdma_flag;
    s_ate_wcdma_flag     	wcdma_flag;
    s_ate_lte_flag    	    lte_flag;
    s_ate_cdma_flag         cdma_flag;
    s_ate_other_flag      	other_flag;
    s_ate_nsft_flag      	nsft_flag;
    s_ate_wireless_flag 	wireless_flag;
    unsigned char                    reserved[3];
} s_ate_flag;       // 22Bytes


/* 
* ****************************************
*                 WCN_ATE_Tool
* ****************************************
*/
typedef struct _s_wcn_ate_flag
{
    unsigned char        wcn_ate_flag[4];
} s_wcn_ate_flag;   // 4Bytes

/*
* ****************************************
*                 HW_Test_Tool
* ****************************************
*/
typedef struct _s_hw_test_flag
{
    unsigned char        hw_test_flag[20];
	} s_hw_test_flag;               // 20Bytes
/*
* ****************************************
*            ATA_Tool & Factory Mode
* ****************************************
*/
typedef struct _s_factory_mode_flag
{
    unsigned char	    reserved[20];
} s_factory_mode_flag;	// 20Bytes


typedef struct _s_mtk_test_flag
{
    s_ate_flag			ate_flag[5];	// 110Bytes
    s_wcn_ate_flag		wcn_ate_flag;	// 4Bytes
    s_hw_test_flag		hw_test_flag;	// 20Bytes
    s_factory_mode_flag	factory_mode_flag; // 20Bytes
    unsigned char	            reserved[16];
} s_mtk_test_flag;	// 170Bytes
typedef struct
{
    unsigned char  boot_mode;  //0:normal, 1: meta
    unsigned char  com_type;    //0:unknow com,1:uart, 2:usb
	unsigned char  com_id;     //0:single interface device, 1: composite device
} s_mtk_boot_mode_flag;


typedef struct
{
    unsigned char               barcode[64];    // 64B
    nvram_ef_imei_imeisv_struct IMEI[4];        // 40B
    s_target_info_record        target_info;    // 64B
    s_mtk_test_flag             mtk_test_flag;  // 170B
	s_mtk_boot_mode_flag        mtk_boot_mode_flag; //3B
    unsigned char reserved[1024-170-64-40-64-3];
} PRODUCT_INFO;




//the record size and number of PRODUCT_INFO nvram file
#define CFG_FILE_PRODUCT_INFO_SIZE    1024
#define CFG_FILE_PRODUCT_INFO_TOTAL   1
#endif /* _CFG_PRODUCT_INFO_FILE_H */

