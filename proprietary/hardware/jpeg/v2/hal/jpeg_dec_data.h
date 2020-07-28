/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2007
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
#include "jpeg_dec_hal.h"

#ifndef __JPEG_DEC_DATA_H__
#define __JPEG_DEC_DATA_H__

//=======================================================================
// mt6589_FPGA
//=======================================================================

#include "jpeg_drv.h"


typedef enum
{
   JPG_COLOR_420    = 0x00221111 
  ,JPG_COLOR_422    = 0x00211111
  ,JPG_COLOR_444    = 0x00111111
  ,JPG_COLOR_422V   = 0x00121111
  ,JPG_COLOR_422x2  = 0x00412121
  ,JPG_COLOR_422Vx2 = 0x00222121
  ,JPG_COLOR_400    = 0x00110000 
  ,JPG_UNSUPPORT    = 0x00FFFFFF 
   
} JPEG_COLOR_FORMAT ;


 

typedef enum _JpegDecMarker	/* JPEG marker codes */
{
	M_SOF0  = 0xffc0,
	M_SOF1  = 0xffc1,
	M_SOF2  = 0xffc2,
	M_SOF3  = 0xffc3,

	M_SOF5  = 0xffc5,
	M_SOF6  = 0xffc6,
	M_SOF7  = 0xffc7,

	M_JPG   = 0xffc8,
	M_SOF9  = 0xffc9,
	M_SOF10 = 0xffca,
	M_SOF11 = 0xffcb,

	M_SOF13 = 0xffcd,
	M_SOF14 = 0xffce,
	M_SOF15 = 0xffcf,

	M_DHT   = 0xffc4,

	M_DAC   = 0xffcc,

	M_RST0  = 0xffd0,
	M_RST1  = 0xffd1,
	M_RST2  = 0xffd2,
	M_RST3  = 0xffd3,
	M_RST4  = 0xffd4,
	M_RST5  = 0xffd5,
	M_RST6  = 0xffd6,
	M_RST7  = 0xffd7,

	M_SOI   = 0xffd8,
	M_EOI   = 0xffd9,
	M_SOS   = 0xffda,
	M_DQT   = 0xffdb,
	M_DNL   = 0xffdc,
	M_DRI   = 0xffdd,
	M_DHP   = 0xffde,
	M_EXP   = 0xffdf,

	M_APP0  = 0xffe0,
	M_APP1  = 0xffe1,
	M_APP2  = 0xffe2,
	M_APP3  = 0xffe3,
	M_APP4  = 0xffe4,
	M_APP5  = 0xffe5,
	M_APP6  = 0xffe6,
	M_APP7  = 0xffe7,
	M_APP8  = 0xffe8,
	M_APP9  = 0xffe9,
	M_APP10 = 0xffea,
	M_APP11 = 0xffeb,
	M_APP12 = 0xffec,
	M_APP13 = 0xffed,
	M_APP14 = 0xffee,
	M_APP15 = 0xffef,

	M_JPG0  = 0xfff0,
	M_JPG13 = 0xfffd,
	M_COM   = 0xfffe,

	M_TEM   = 0xff01,

	M_ERROR = 0x100
} JpegDecMarker;


typedef struct
{
	/* memory address of jpeg image */
	unsigned char		*pu1JpegImage;
    unsigned int        u4JpegPhyAddr;
    int                 i4JpegFD;
	/* file size of this jpeg image */
	unsigned int		u4ImageByteSize;
	/* flags for data status */
	int			    	fgInited;			/* jpeg image set */
	int		    		fgParsed;			/* jpeg image parsed successfully */
	int	    			fgFatalError;		/* fatal error found and can not decode */
	int 				fgThumbnail;		/* thumbnail image available (app0 or app1 marker) */
	/* current read pointer */
	unsigned char*		pu1CurrRead;
	/* SOI - start of image */
	int 				fgSOI;				/* indicate that already have an SOI marker */
  int         fgSOI_skip;	
	/* SOF - start of frame */
	JpegDecSOF  		rSOF;
	/* DHT - de-huffman table */
	JpegDecDHT		    rDHT;
	/* DQT - de-quantization table */
	JpegDecDQT		    rDQT;
	/* DRI - restart interval */
	unsigned short		u2RestartInterval;
	/* FMT - format flag */
	unsigned int		fgFormat;			/* JFIF or Exif or ... */
	/* APP0 */
	unsigned int    	u4NumApp0;
//	JPEG_JFIF_APP0_T	arAPP0[D_MAX_NUM_APP0];
	/* APP1 */
//	JPEG_JFIF_APP1_T	rAPP1;
	/* SOS - start of scan */
	unsigned int		u4NumScan;
	unsigned int		u4CurrScan;
	JpegDecSOS  		arSOS[MAX_NUM_SCAN];
	JpegDecSOS          rSOS;
	/* EOI */
	int				    fgEOI;					/* EOI reached */
	unsigned char*		pu1EOI;				/* point to EOI marker */
	/* LAST SCAN */
	unsigned int		au4CompLastScan[MAX_JPEG_COMPONENT];
	int 				fgEOF;
} JpegDecParser;

int jpeg_dec_parse_scan(JpegDecParser* parser, unsigned int u4Scan);

//JpegDecParser* jpeg_dec_parse_prog(unsigned char* pu1ImageStartAddr, unsigned int size);
JpegDecParser* jpeg_dec_parse(unsigned char* file, unsigned int size);


//=============================================================================================










typedef struct JPEG_FILE_STRUCTURE
{
    UINT32 srcStreamAddrBase;
    UINT32 srcStreamSize;   
    UINT32 srcStreamAddrWritePtr ;

    UINT32 outputBuffer0[3];
    UINT32 outputBuffer1[3];                         /* nouse in full frame mode, only use in PauseResume/DirectCouple mode */    
    //UINT32 dstStreamAddr;
    //UINT32 dstStreamSize;

    //UINT32 tempBufferAddr;
    //UINT32 tempBufferSize;
    
   // JPEG file information
    UINT32 thumbnail_offset;
    UINT32 jpg_precision;
    
    UINT32 width;
    UINT32 height;
   
    UINT32 paddedWidth[3];             ///< the padded width after JPEG block padding
    UINT32 paddedHeight[3];            ///< the padded height after JPEG block padding

    UINT32 mcuPerRow;                  ///< number of MCU row in the JPEG file
    UINT32 totalMcuRows;               ///< number of MCU column in the JPEG file

    UINT32 compDU[3];              ///< (required by HW decoder) number of DU for each component
    UINT32 duPerMCURow[3];          ///< (required by HW decoder) DU per MCU row for each component
    UINT32 dummyDU[3];              ///< (required by HW decoder) number of dummy DU for each component

    UINT32 samplingFormat;
    UINT8 jpg_progressive;          // 0: baseline, 1:progressive
    
    // JPEG component information
    UINT32  componentNum;
    UINT32  uvFlag;
    UINT32  componentID[3];          ///< Ci
    UINT32  hSamplingFactor[3];      ///< Hi
    UINT32  vSamplingFactor[3];      ///< Vi
    UINT32  qTableSelector[3];       ///< Tqi


    UINT32  hSamplingFactorMax;
    UINT32  vSamplingFactorMax;    

	  UINT32   totalMCU ;
	  UINT32	 blkNumInMCU;          //total drv no use
	  UINT32	 blkInCompMCU[3];    


    /* for BRZ */
    UINT32  membershipList ;
    UINT32  srcColorFormat;
    UINT32  dstColorFormat;
    UINT32  u4isColorConv;
    UINT32  u4ds_width[3] ;
    UINT32  u4ds_height[3] ;
    
    UINT32  decodeMode;
    UINT32  reg_OpMode  ;
    UINT32  gdmaBypassEn;
    UINT32  regDecDumpEn;

    UINT32 dma_McuInGroup ;
    UINT32 dma_GroupNum    ;
    UINT32 dma_LastMcuNum ;    

	  UINT32 compImgStride[D_MAX_JPEG_HW_COMP];		      // hSamplingFactor[n] * 8 * mcuPerRow (byte pitch of a component)
	  UINT32 compMemStride[D_MAX_JPEG_HW_COMP];
	  UINT32 compMemSize[D_MAX_JPEG_HW_COMP];
	  UINT32 compTileBufStride[D_MAX_JPEG_HW_COMP];		      // hSamplingFactor[n] * 8 * mcuPerRow (byte pitch of a component)



    UINT32  pauseRow_en;
    UINT32  pauseRowCnt;
    UINT32  tileBufRowNum ;
    UINT32  buffer_Y_PA ;
    UINT32  buffer_Cb_PA;
    UINT32  buffer_Cr_PA;
    UINT32  buffer_Y_row_size ;
    UINT32  buffer_C_row_size ;
    UINT8   lumaHorDecimate;      //(0): 1, (1): 1/2, (2): 1/4, (3): 1/8
    UINT8   lumaVerDecimate;
    UINT8   cbcrHorDecimate;
    UINT8   cbcrVerDecimate;
    UINT8   cbcrHorConvEn ;






} JPEG_FILE_INFO;

#define JPEG_MAX_OFFSET         0xFFFFFFFF

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int jpeg_drv_parse_file(UINT8 *f_ptr, UINT32 f_size, JPEG_FILE_INFO *info);
int jpeg_drv_calculate_info(JPEG_FILE_INFO *info);


//-------------------------------------------------------------------
// JPEG MARKER
//-------------------------------------------------------------------

#define JPEG_MARKER_START_CODE   0XFF

#define JPEG_MARKER_SOF(I)       (0xC0 + I)

/* Start of Frame markers, non-differential, Huffman coding */
#define JPEG_MARKER_SOF0         0XC0
#define JPEG_MARKER_SOF1         0XC1
#define JPEG_MARKER_SOF2         0XC2
#define JPEG_MARKER_SOF3         0XC3

/* Start of Frame markers, differential, Huffman coding */
#define JPEG_MARKER_SOF5         0XC5
#define JPEG_MARKER_SOF6         0XC6
#define JPEG_MARKER_SOF7         0XC7

/* Start of Frame markers, non-differential, arithmatic coding */
#define JPEG_MARKER_JPG0         0XC8
#define JPEG_MARKER_SOF9         0XC9
#define JPEG_MARKER_SOF10        0XCA
#define JPEG_MARKER_SOF11        0XCB

/* Start of Frame markers, differential, arithmatic coding */
#define JPEG_MARKER_SOF13        0xCD
#define JPEG_MARKER_SOF14        0xCE
#define JPEG_MARKER_SOF15        0xCF

/* Huffman table specification */
#define JPEG_MARKER_DHT          0xC4  /* Define Huffman table(s) */

/* Arithmatic coding conditioning specification */
#define JPEG_MARKER_DAC          0xCC  /* Define arithmatic coding conditioning(s) */

/* Restart interval termination */
#define JPEG_MARKER_RST(I)       (0xD0 + I)
#define JPEG_MARKER_RST0         0xD0
#define JPEG_MARKER_RST1         0xD1
#define JPEG_MARKER_RST2         0xD2
#define JPEG_MARKER_RST3         0xD3
#define JPEG_MARKER_RST4         0xD4
#define JPEG_MARKER_RST5         0xD5
#define JPEG_MARKER_RST6         0xD6
#define JPEG_MARKER_RST7         0xD7

#define JPEG_MARKER_SOI          0xD8
#define JPEG_MARKER_EOI          0xD9
#define JPEG_MARKER_SOS          0xDA
#define JPEG_MARKER_DQT          0xDB
#define JPEG_MARKER_DNL          0xDC
#define JPEG_MARKER_DRI          0xDD
#define JPEG_MARKER_DHP          0xDE
#define JPEG_MARKER_EXP          0xDF

#define JPEG_MARKER_APP(I)       (0xE0 + I)

#define JPEG_MARKER_JPG(I)       (0xF0 + I)

#define JPEG_MARKER_TEM          0x01

#define JPEG_MARKER_ZERO         0x00

#define JPEG_MARKER_COM          0xFE


/* Definition for mandatory marker to decode a JPEG file */
#define JPEG_MANDATORY_MARKER_SOF_BIT  0x00000001
#define JPEG_MANDATORY_MARKER_DQT_BIT  0x00000002
#define JPEG_MANDATORY_MARKER_DHT_BIT  0x00000004

#define JPEG_MANDATORY_MARKER_CHECK    0x00000007


























#endif   /// __JPEG_DEC_DATA_H__
