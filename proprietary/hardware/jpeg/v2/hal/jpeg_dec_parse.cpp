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

#define MTK_LOG_ENABLE 1
#include "jpeg_dec_data.h"
#include <cutils/log.h>



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "jpeg_dec_data.h"

#define JPEG_FPGA_DEBUG

#define JPEG_READ_BYTE(PINSTANCE, NTH_BYTE) (PINSTANCE->pu1CurrRead[NTH_BYTE])
#define JPEG_READ_2BYTES(PINSTANCE, NTH_BYTE) ((JPEG_READ_BYTE(PINSTANCE, NTH_BYTE) << 8) | JPEG_READ_BYTE(PINSTANCE, (NTH_BYTE) + 1))

#ifdef FPGA_VERSION 

int peek_marker(JpegDecParser* prInstance, unsigned short *pu2Marker)
{
	int i4Ret;
	unsigned short u2Marker;

	while ((i4Ret = *(prInstance->pu1CurrRead)) != 0xff)
	{
		prInstance->pu1CurrRead++;
        
        if (((unsigned int)prInstance->pu1CurrRead) >= ((unsigned int)&prInstance->pu1JpegImage[prInstance->u4ImageByteSize]))
            return -(int)E_JPG_ERR_PASS_END;
	}

	u2Marker = (prInstance->pu1CurrRead[0] << 8) + prInstance->pu1CurrRead[1];
	*pu2Marker = u2Marker;
	return E_JPG_OK;
}

void skip_marker(JpegDecParser* prInstance)
{
	unsigned short u2Length;

	// this is a marker, advance 2 bytes first
	prInstance->pu1CurrRead += 2;
	u2Length = JPEG_READ_2BYTES(prInstance, 0);
    prInstance->pu1CurrRead += u2Length;

    printf("Skip marker: %d bytes\n", u2Length);
}

int jpeg_proc_SOI(JpegDecParser* prInstance)
{
	int i4Ret; /* for ADVANCE_BYTES() error return */

	if (prInstance->fgSOI)
	{
		return -(int)E_JPG_ERR_MULTI_SOI;
	}

	prInstance->fgSOI = 1;
	prInstance->pu1CurrRead += 2;

	return E_JPG_OK;
}

/* process start of frame */
int jpeg_proc_SOF(JpegDecParser* prInstance)
{
	int i4Ret;
	unsigned short u2Marker;
	unsigned short u2Length;
	unsigned char u1CompIndex;


	if (prInstance->rSOF.fgSOF)
	{
	    printf("Already set SOF.\n");
		return 0;
	}


	u2Marker = JPEG_READ_2BYTES(prInstance, 0);	// 0,1
	u2Length = JPEG_READ_2BYTES(prInstance, 2); // 2,3
	prInstance->pu1CurrRead += 2;

	/*
	 * do baseline or progressive check here, but due to we implement baseline decoder only,
	 * hence these code was marked out
	 */

    printf("JPEG marker: %08x\n", u2Marker);

	switch (u2Marker)
	{
	case M_SOF0		:
		prInstance->rSOF.eJpegFormat = E_JPG_BASELINE;
		break;
	case M_SOF1		:
		prInstance->rSOF.eJpegFormat = E_JPG_EX_SEQ_HUFFMAN;
		break;
	case M_SOF2		:
		prInstance->rSOF.eJpegFormat = E_JPG_PROGRESSIVE_HUFFMAN;
		break;
		//return -(INT32)E_JPG_ERR_UNSUPPORT_SOF;
	case M_SOF3		:
		prInstance->rSOF.eJpegFormat = E_JPG_EX_SEQ_ARITHMETIC;
		return -(int)E_JPG_ERR_UNSUPPORT_SOF;
	case M_SOF5		:
		prInstance->rSOF.eJpegFormat = E_JPG_PROGRESSIVE_ARITHMETIC;
		return -(int)E_JPG_ERR_UNSUPPORT_SOF;
	case M_SOF6		:
		prInstance->rSOF.eJpegFormat = E_JPG_LOSSLESS_HUFFMAN;
		return -(int)E_JPG_ERR_UNSUPPORT_SOF;
	case M_SOF7		:
		prInstance->rSOF.eJpegFormat = E_JPG_DIFFERENTIAL_SEQ_HUFFMAN;
		return -(int)E_JPG_ERR_UNSUPPORT_SOF;
	case M_SOF9		:
		prInstance->rSOF.eJpegFormat = E_JPG_DIFF_PROG_HUFFMAN;
		return -(int)E_JPG_ERR_UNSUPPORT_SOF;
	case M_SOF10	:
		prInstance->rSOF.eJpegFormat = E_JPG_DIFF_LLESS_HUFFMAN;
		return -(int)E_JPG_ERR_UNSUPPORT_SOF;
	case M_SOF11	:
		prInstance->rSOF.eJpegFormat = E_JPG_RESERVED_FOR_EXTENSIONS;
		return -(int)E_JPG_ERR_UNSUPPORT_SOF;
	case M_SOF13	:
		prInstance->rSOF.eJpegFormat = E_JPG_LOSSLESS_ARITHMETIC;
		return -(int)E_JPG_ERR_UNSUPPORT_SOF;
	case M_SOF14	:
		prInstance->rSOF.eJpegFormat = E_JPG_DIFF_SEQ_ARITHMETIC;
		return -(int)E_JPG_ERR_UNSUPPORT_SOF;
	case M_SOF15	:
		prInstance->rSOF.eJpegFormat = E_JPG_DIFF_PROG_ARITHMETIC;
		return -(int)E_JPG_ERR_UNSUPPORT_SOF;
	default			:
		prInstance->rSOF.eJpegFormat = E_JPG_UNSUPPORT_FORMAT;
		return -(int)E_JPG_ERR_UNSUPPORT_SOF;
	}

	// SOF0 & SOF1 & SOF2 parsing
	if (u2Length < 8)
	{
		return -(int)E_JPG_ERR_INVALID_SOF;
	}

	prInstance->rSOF.u1DataPrecision	= JPEG_READ_BYTE	(prInstance, 2);	// 2
	prInstance->rSOF.u2ImageHeight		= JPEG_READ_2BYTES	(prInstance, 3);	// 3,4
	prInstance->rSOF.u2ImageWidth		= JPEG_READ_2BYTES	(prInstance, 5);	// 5,6
	prInstance->rSOF.u1NumComponents	= JPEG_READ_BYTE	(prInstance, 7);	// 7


	prInstance->pu1CurrRead += 8;
	u2Length -= 8;

	//code sync
	if ((prInstance->rSOF.u1NumComponents	<  1)||(prInstance->rSOF.u1NumComponents	>  3))
	{
		return (int)E_JPG_ERR_INVALID_COMPONENT;
	}
		
	// start of frame validation test
	if ((prInstance->rSOF.u2ImageHeight		== 0)	||
		(prInstance->rSOF.u2ImageWidth		== 0)	||
		(u2Length != ((unsigned short)(prInstance->rSOF.u1NumComponents * 3))))
	{
		return -(int)E_JPG_ERR_INVALID_SOF;
	}

	for (u1CompIndex = 0; u1CompIndex < prInstance->rSOF.u1NumComponents; u1CompIndex++)
	{
		unsigned char u1CompId, u1Factor, u1QuantTblNo;
		u1CompId		= JPEG_READ_BYTE(prInstance, 0);
		u1Factor		= JPEG_READ_BYTE(prInstance, 1);
		u1QuantTblNo	= JPEG_READ_BYTE(prInstance, 2);

		prInstance->rSOF.arSofComp[u1CompIndex].u1CompId	= u1CompId;
		prInstance->rSOF.au1MapId2Index[u1CompId]               = u1CompIndex;
		prInstance->rSOF.arSofComp[u1CompIndex].u1HSampFactor	= u1Factor >> 4;
		prInstance->rSOF.arSofComp[u1CompIndex].u1VSampFactor	= u1Factor & 0xf;
		prInstance->rSOF.arSofComp[u1CompIndex].u1QuantTblNo	= u1QuantTblNo;
        
        printf("H/V sample factor: %d/%d Q: %d\n", prInstance->rSOF.arSofComp[u1CompIndex].u1HSampFactor, 
            prInstance->rSOF.arSofComp[u1CompIndex].u1VSampFactor, prInstance->rSOF.arSofComp[u1CompIndex].u1QuantTblNo); 

		prInstance->pu1CurrRead += 3;
	}

	prInstance->rSOF.fgSOF = 1;

    printf("Num comp: %d, w/h: %d/%d\n", prInstance->rSOF.u1NumComponents, prInstance->rSOF.u2ImageWidth, prInstance->rSOF.u2ImageHeight);
       
	return (int)E_JPG_OK;
}

/* process start of scan */
int jpeg_proc_SOS(JpegDecParser* prInstance)
{
	int i4Ret;
	unsigned short u2Length;
	unsigned char u1CompInScan;
	unsigned char u1CompIndex;


	if (!prInstance->rSOF.fgSOF)
	{
	    printf("SOS before SOF\n");
		return -(int)E_JPG_ERR_SOS_BEFORE_SOF;
	}

	u2Length = JPEG_READ_2BYTES(prInstance, 2); // 2,3
	u1CompInScan = JPEG_READ_BYTE(prInstance, 4);	// 4
	prInstance->pu1CurrRead += 5;

	// sos validation
	if ((u2Length < (unsigned short)((u1CompInScan * 2) + 6)) ||
		(u1CompInScan == 0) || (u1CompInScan > MAX_JPEG_COMPONENT))
	{
	    printf("Invaild SOS\n");
		return -(int)E_JPG_ERR_INVALID_SOS;
	}

	// store components in scan
	prInstance->arSOS[prInstance->u4CurrScan].u1CompInScan = u1CompInScan;

    printf("Curr scan: %d, Comp in Scan: %d\n", prInstance->u4CurrScan, prInstance->arSOS[prInstance->u4CurrScan].u1CompInScan);

	u2Length -= 3;	// skip length(2 bytes), comp in scan(1 byte)

	/*
	sos structure :
		2 bytes - SOS marker
		2 bytes - length
		1 byte  - num comp in scan
		n * 3 bytes - comp data
		1 byte  - ss
		1 byte  - se
		1 byte  - ah al
	*/
	if (prInstance->u4CurrScan != 0)
	{
		memcpy(
			&prInstance->arSOS[prInstance->u4CurrScan + 1],
			&prInstance->arSOS[prInstance->u4CurrScan],
			sizeof(JpegDecSOS));
	}

	prInstance->arSOS[prInstance->u4CurrScan].u4ScanPass = prInstance->u4CurrScan;

	// read component id and table no
	for (u1CompIndex = 0; u1CompIndex < u1CompInScan; u1CompIndex++)
	{
		unsigned char u1CompId;
    unsigned char u1TableId;
    unsigned char u1MapedIndex;
		u1CompId = JPEG_READ_BYTE(prInstance, 0);
		u1TableId = JPEG_READ_BYTE(prInstance, 1);
		prInstance->pu1CurrRead += 2;
		prInstance->arSOS[prInstance->u4CurrScan].au1CompNoList[u1CompIndex] = prInstance->rSOF.au1MapId2Index[u1CompId];

		u1MapedIndex = prInstance->rSOF.au1MapId2Index[u1CompId];
		if (u1MapedIndex >= (unsigned char)4)
		{
		    printf("MapIndex > 4,Invaild SOS\n");
		    return -(int)E_JPG_ERR_INVALID_SOS;
		}
		/* following data use maped index */
		prInstance->arSOS[prInstance->u4CurrScan].au1DcId[u1MapedIndex] = u1TableId >> 4;
		prInstance->arSOS[prInstance->u4CurrScan].au1AcId[u1MapedIndex] = u1TableId & 0xf;        
		prInstance->au4CompLastScan[u1MapedIndex] = prInstance->u4CurrScan;

    printf("Comp: %d MapIdx: %d DcTblId: %d AcTblId: %d\n", u1CompId, u1MapedIndex, prInstance->arSOS[prInstance->u4CurrScan].au1DcId[u1MapedIndex],
                                                            prInstance->arSOS[prInstance->u4CurrScan].au1AcId[u1MapedIndex]);
	}

	prInstance->arSOS[prInstance->u4CurrScan].u1Ss		= JPEG_READ_BYTE(prInstance, 0);
	prInstance->arSOS[prInstance->u4CurrScan].u1Se		= JPEG_READ_BYTE(prInstance, 1);
  

  unsigned char u1AhAl = JPEG_READ_BYTE(prInstance, 2);
  
  prInstance->arSOS[prInstance->u4CurrScan].u1AhAl	= u1AhAl;
  prInstance->rSOS.u1Ah  = u1AhAl >> 4;
  prInstance->rSOS.u1Al  = u1AhAl & 0xF;

	prInstance->pu1CurrRead += 3;

	prInstance->arSOS[prInstance->u4CurrScan].pu1ScanDataStart = prInstance->pu1CurrRead;
    
	prInstance->u4CurrScan++;
	prInstance->u4NumScan++;    
    
	return (int)E_JPG_OK;
}

/* process standard APP0 "JFIF" marker (and "JFXX" extension) */
int jpeg_proc_APP0(JpegDecParser* prInstance)
{
	int i4Ret;
	int i4Index;
	unsigned short u2Length;


	u2Length = JPEG_READ_2BYTES(prInstance, 2);	//2, 3
    prInstance->pu1CurrRead += 4;

	if (u2Length < 8)
	{
		return -(int)E_JPG_ERR_INVALID_APP0;
	}
	u2Length -= 2;

    prInstance->pu1CurrRead += u2Length;
    return (int)E_JPG_OK;
}

int jpeg_proc_DHT(JpegDecParser* prInstance)
{
	int i4Ret, i;
	unsigned short u2Length;
	unsigned char au1Bits[17];
	unsigned char au1HuffVal[256];


	for(i=0;i<sizeof(au1HuffVal); i++)
	{
		au1HuffVal[i]=0;
	}
	u2Length = JPEG_READ_2BYTES(prInstance, 2); // 2,3
	prInstance->pu1CurrRead += 4;

	if (u2Length < (2 + 17))
	{
		return -(int)E_JPG_ERR_INVALID_DHT;
	}

	u2Length -= 2;

	while (u2Length > 16)
	{
		int u4Bits;
		int u4Count;
		int u4HuffIndex;
		int fgIsAc;
		unsigned char u1TblIndex;

		memset(au1Bits, 0, 17);
		memset(au1HuffVal, 0, 256);

		u1TblIndex = JPEG_READ_BYTE(prInstance, 0);
		fgIsAc = (int)(u1TblIndex >> 4);
		//u1TblIndex &= 0xf;
		u1TblIndex &= 0x3;	// at most 4 table each, for dc and ac
		u4Count = 0;

		for (u4Bits = 1; u4Bits < 17; u4Bits++)
		{
			au1Bits[u4Bits] = JPEG_READ_BYTE(prInstance, u4Bits);
			u4Count += au1Bits[u4Bits];
		}
        prInstance->pu1CurrRead += 17;
		u2Length -= 17;

		/*
		if ((INT32)E_JPG_OK != i4Ret)
		{
			return i4Ret;
		}
		*/

		if (u4Count > 256)
		{
			//bogus dht
			u4Count = 256;
		}

		if ((int)u2Length < u4Count)
		{
			//bogus again
			u4Count = u2Length;
		}

		// fill huff value
		for (u4HuffIndex = 0; u4HuffIndex < u4Count; u4HuffIndex++)
		{
			au1HuffVal[u4HuffIndex] = JPEG_READ_BYTE(prInstance, u4HuffIndex);
            printf("%02x ", au1HuffVal[u4HuffIndex]);
		}

		u2Length -= u4Count;
        prInstance->pu1CurrRead += u4Count;

		printf("DHT : %s table index %d Count: %d\n", fgIsAc ? "AC" : "DC", u1TblIndex, u4Count);

		if (fgIsAc)
		{
		    if((prInstance->rDHT.fgAcTblLoaded & (1 << u1TblIndex)) == 0)
		    {
			    prInstance->rDHT.u4NumAcTbl++;
			    prInstance->rDHT.fgAcTblLoaded |= 1 << u1TblIndex;
	    	}
			memcpy(prInstance->rDHT.arAcTbl[u1TblIndex].au1HuffVal, au1HuffVal, u4Count);
			memcpy(prInstance->rDHT.arAcTbl[u1TblIndex].au1Bits, au1Bits, 17);
		}
		else
		{
		    if((prInstance->rDHT.fgDcTblLoaded & (1 << u1TblIndex)) == 0)
		    {
			    prInstance->rDHT.u4NumDcTbl++;
			    prInstance->rDHT.fgDcTblLoaded |= 1 << u1TblIndex;
	    	}
			memcpy(prInstance->rDHT.arDcTbl[u1TblIndex].au1HuffVal, au1HuffVal, u4Count);
			memcpy(prInstance->rDHT.arDcTbl[u1TblIndex].au1Bits, au1Bits, 17);
		}
	}

	return (int)E_JPG_OK;
}


/* process de-quantization table */
int jpeg_proc_DQT(JpegDecParser* prInstance)
{
	int i4Ret;
	unsigned short u2Length;
	int i4Ntbl = 0;

	u2Length = JPEG_READ_2BYTES(prInstance, 2); // 2,3
	prInstance->pu1CurrRead += 4;

	if (u2Length < (2 + 1 + 64))
	{
		return -(int)E_JPG_ERR_INVALID_DHT;
	}

	u2Length -= 2;

	while (u2Length > 64)
	{
		unsigned char u1Prec;
		unsigned char u1Qtbl;

		u1Prec = JPEG_READ_BYTE(prInstance, 0);
		u1Qtbl = u1Prec & 0x3;	// table index 0 .. 3
		u1Prec >>= 4;

		if (u1Prec && (u2Length < (128 + 1)))
		{
			return -(int)E_JPG_ERR_INVALID_DHT;
		}

		printf("DQT %d table index %d\n", u1Prec ? 12 : 8, u1Qtbl);

		memcpy(prInstance->rDQT.aau1Qtbl[u1Qtbl],
			prInstance->pu1CurrRead + 1, u1Prec ? (size_t)128: (size_t)64);
			
		prInstance->rDQT.afgPrec[u1Qtbl] = (unsigned int)u1Prec;


		u2Length -= u1Prec ? (128 + 1) : (64 + 1);
        if (u1Prec)
            prInstance->pu1CurrRead += 129;
        else
            prInstance->pu1CurrRead += 65;
		i4Ntbl++;
	}
	prInstance->rDQT.u1NumQ += i4Ntbl;
	prInstance->rDQT.fgDQT = 1;
	return (int)E_JPG_OK;
}

/* process restart interval marker */
int jpeg_proc_DRI(JpegDecParser* prInstance)
{
	int i4Ret;
	unsigned short u2Length;

	u2Length = JPEG_READ_2BYTES(prInstance, 2); // 2,3
	prInstance->pu1CurrRead += 4;

	if (u2Length < 4)
	{
		return -(int)E_JPG_ERR_INVALID_DRI;
	}

	prInstance->u2RestartInterval = JPEG_READ_2BYTES(prInstance, 0);

	prInstance->pu1CurrRead += 2;

	return (int)E_JPG_OK;

}


static unsigned int dispatch_marker(JpegDecParser* prInstance, unsigned short u2Marker)
{
	unsigned int i4Ret = 1;


	switch (u2Marker)
	{
	case M_SOI		:		// start of image
		i4Ret = jpeg_proc_SOI(prInstance);
		break;
	case M_SOF0		:		// baseline
	case M_SOF1		:		// extended sequential huffman
	case M_SOF2		:		// progressive huffman
	case M_SOF3		:		// lossless huffman
	case M_SOF5		:		// differential sequential huffman
	case M_SOF6		:		// differential progressive huffman
	case M_SOF7		:		// differential lossless huffman
	case M_JPG		:		// reserved jpeg extension
	case M_SOF9		:		// extended sequential arithmetic
	case M_SOF10	:		// progressive arithmetic
	case M_SOF11	:		// lossless arithmetic
	case M_SOF13	:		// differential sequential arithmetic
	case M_SOF14	:		// differential progressive arithmetic
	case M_SOF15	:		// differential lossless arithmetic
		i4Ret = jpeg_proc_SOF(prInstance);
		if ((int)E_JPG_OK != i4Ret)
		{
			prInstance->fgFatalError		= 1;
		}
		break;
	case M_SOS		:		// start of scan
		i4Ret = jpeg_proc_SOS(prInstance);
		break;
	case M_EOI		:		// end of image
		//i4Ret = _jfifProcEOI(prInstance);
		break;
	case M_DHT		:		// de-huffman table
		i4Ret = jpeg_proc_DHT(prInstance);
		break;
	case M_DQT		:		// de-quantization table
		i4Ret = jpeg_proc_DQT(prInstance);
		break;
	case M_DRI		:		// restart interval marker
		i4Ret = jpeg_proc_DRI(prInstance);
		break;
	case M_APP0		:
		i4Ret = jpeg_proc_APP0(prInstance);
		break;
	case M_APP1		:
		i4Ret = jpeg_proc_APP0(prInstance);
		break;
	case M_APP14	:
		i4Ret = jpeg_proc_APP0(prInstance);
		break;
	case M_APP2		:		// NIKON ICC_PROFILE, EXIF FPXR (FlashPix)
	case M_APP3		:
	case M_APP4		:
	case M_APP5		:
	case M_APP6		:
	case M_APP7		:
	case M_APP8		:
	case M_APP9		:
	case M_APP10	:
	case M_APP11	:
	case M_APP12	:
	case M_APP13	:
	case M_APP15	:
	case M_COM	    :
		// unsupport marker, just skip it
		printf("Unsupport marker: %08x\n", u2Marker);
		skip_marker(prInstance);
        i4Ret = E_JPG_OK;
		break;
        
	default			:
		// unknown marker, just skip it
		// due to unknown, we can not trust the length
		// hence advance through it
		if (u2Marker < M_RST0 || u2Marker > M_RST7)
        {
//            printf("Unknown marker: %08x\n", u2Marker);
        }
		prInstance->pu1CurrRead += 2;
		//prInstance->pu1CurrRead++;
		//skip_marker(prInstance);
        i4Ret = E_JPG_OK;
		break;		
	}

	return i4Ret;
}


JpegDecParser* jpeg_dec_parse(unsigned char* pu1ImageStartAddr, unsigned int size)
{
    int i4Ret;
	  unsigned short u2JfifMarker;
    unsigned char fgFirstScanMarker = 0;
    unsigned int u4Scan;

    JpegDecParser* parser = (JpegDecParser*)malloc(sizeof(JpegDecParser));
    memset(parser, 0, sizeof(JpegDecParser));
    
    parser->pu1JpegImage = pu1ImageStartAddr;
    parser->pu1CurrRead = parser->pu1JpegImage;
    parser->u4ImageByteSize = size;
    parser->fgInited = 1;

#ifdef JPEG_FPGA_DEBUG    
    parser->fgSOI_skip = 1;
    printf("### set SKIP SOI FLAG ###");
#endif
    

	// parse until EOI
	for(;;)
	{
		i4Ret = peek_marker(parser, &u2JfifMarker);
		if (i4Ret == E_JPG_OK)
		{
			// marker found
			i4Ret = dispatch_marker(parser, u2JfifMarker);
			if (((unsigned short)M_EOI == u2JfifMarker) ||
				  ((unsigned short)M_SOS == u2JfifMarker) )
			{
                // stop condition reached - EOI
                if ((parser->rSOF.fgSOF) &&
                    (parser->fgSOI || parser->fgSOI_skip) &&
                    (parser->u4NumScan > 0) &&
                    (!parser->fgFatalError))
                {
                    parser->fgParsed = 1;
                }
                else
                {
                    parser->fgParsed = 0;
                }             
                break;
       }

			 if (i4Ret != E_JPG_OK)
			 {			    
			   printf("JPEG file parsing failed\n");
			 	 break;
			 }
		}
		else
		{
    		printf("JPEG file parsing failed\n");
		    free(parser);
            parser = NULL;
			break;
		}
	}

  printf("JPEG DEC Parsed OK.\n");

  return parser;
}



#endif 









//============================================================================================


/// Refer to CCITT Rec. T.81 (1992 E) Page.48 Figure B.16 - Flow of compressed data syntax
int jpeg_drv_parse_file(UINT8 *f_ptr, UINT32 f_size, JPEG_FILE_INFO *info)
{
    UINT8 byte, byte1, byte2;
    UINT32 index, index_offset, i, count, temp;
    UINT16 marker_length;   
    bool bJFIF = false;
    //info->srcStreamAddrBase = f_ptr;
    info->srcStreamSize = f_size;
    
    info->thumbnail_offset = JPEG_MAX_OFFSET;
    //info->file_size = f_size;
    index = 0;
    
    byte1 = f_ptr[index++];
    byte2 = f_ptr[index++];

    if ((byte1 != JPEG_MARKER_START_CODE) || (byte2 != JPEG_MARKER_SOI))
    { 
        JPG_ERR("HW parse can't find Start Code (%x %x) from %p !!!",(unsigned int)byte1, (unsigned int)byte2, f_ptr); 
        return -1;
    }
    
    while(index < f_size)
    {       
        // search 0xFF
        do{
            byte1 = f_ptr[index++];
        }while ((byte1 != JPEG_MARKER_START_CODE) && (index < f_size));

        if(index >= f_size)
            break;
            
        byte2 = f_ptr[index++];

        switch (byte2)
        {
            case JPEG_MARKER_SOF0:
            case JPEG_MARKER_SOF2:
            
                if (byte2 == JPEG_MARKER_SOF0)
                {       
                    // baseline mode
                    info->jpg_progressive = 0;
                }
                else
                {
                    // progressive mode
                    info->jpg_progressive = 1;
                }
                
                marker_length = (f_ptr[index++] << 8);
                marker_length |= f_ptr[index++] - 2;
                
                info->jpg_precision = f_ptr[index++];
                info->height = (f_ptr[index++] << 8);
                info->height |= f_ptr[index++];           
                info->width = (f_ptr[index++] << 8);
                info->width |= f_ptr[index++];
                
                info->componentNum = f_ptr[index++];
                
                if(info->componentNum != 1 && info->componentNum != 3){ 
                  JPG_ERR("HW decoder unsupport componentNum (%d) !!!", info->componentNum); 
                  return -1;
                }

                for(i = 0 ; i < info->componentNum ; i++)
                {
                    info->componentID[i] = f_ptr[index++];
                    if(info->componentNum == 1){
                      info->hSamplingFactor[i] = 1;
                      info->vSamplingFactor[i] = 1;
                      index++ ;
                    }else{
                      info->hSamplingFactor[i] = (f_ptr[index] & 0xF0) >> 4;
                      info->vSamplingFactor[i] = f_ptr[index++] & 0x0F;
                    }
                    info->qTableSelector[i] = f_ptr[index++];
                }
        
                //byte2 = JPEG_MARKER_EOI;
                // directly return since we've already got the image info we need
                return 0;
                
            case JPEG_MARKER_SOS:
            case JPEG_MARKER_JPG0:
            case JPEG_MARKER_ZERO:
                break;
                
            case JPEG_MARKER_DQT:
            case JPEG_MARKER_DHT:
                if(info->thumbnail_offset == JPEG_MAX_OFFSET)
                {
                    info->thumbnail_offset = index - 2;
                }
                
                index_offset=(f_ptr[index++] << 8);
                index_offset += (int) (f_ptr[index++]-2);
                byte = f_ptr[index];
                

                if(index_offset > 2048)
                {
                    JPG_ERR("Marker:%x length:%d to larger", byte2, index_offset);
                    return -1;
                }

                if(byte2 == JPEG_MARKER_DHT)
                {
                    if((byte != 0x00) && (byte != 0x01) && (byte != 0x10) && (byte != 0x11))
                    {
                        JPG_ERR("Marker:%x table number error : 0x%x", byte2, byte);
                        return -1;
                    }
                    count = 1;
                    while(count < index_offset)
                    {
                        temp = 0;
                        for(i = 0 ; i < 16 ; i++)
                        { 
                            temp += f_ptr[index + count];
                            count++;
                        }
                        count += temp;
                        if(count >= index_offset)
                            break;
                        byte = f_ptr[index + count];
                        if((byte != 0x00) && (byte != 0x01) && (byte != 0x10) && (byte != 0x11))
                        {
                            JPG_ERR("Marker:%x table number error : 0x%x", byte2, byte);
                            return -1;
                        }
                        count++;
                    }
                }
                else
                {
                    if((byte >> 4) != 0){
                        JPG_ERR("Marker DQT:invalid number error : 0x%x", byte);
                        return -1;
                    }
                    if(index_offset > 65)
                    {
                        byte = f_ptr[index + 65];
                        if((byte >> 4) != 0)
                        {
                            JPG_ERR("Marker:%x Pq number error : 0x%x", byte2, byte);
                            return -1;                        
                        }
                    }
                    if(index_offset > 65*2)
                    {
                        byte = f_ptr[index + 65*2];
                        if((byte >> 4) != 0)
                        {
                            JPG_ERR("Marker:%x Pq number error : 0x%x", byte2, byte);
                            return -1; 
                        }
                    }
                }

                index +=index_offset;
                break;
            //case JPEG_MARKER_DRI :
            //     JPG_ERR("Marker:%x unsupported : 0x%x", byte2, byte);
            //     return -1;                
            //   break;    
            default:
                if ((byte2==JPEG_MARKER_SOF1) ||
                   ((byte2>=JPEG_MARKER_SOF3) && (byte2<=JPEG_MARKER_SOF15))||
                   (byte2==JPEG_MARKER_DQT) || (byte2==JPEG_MARKER_DNL) ||
                   (byte2==JPEG_MARKER_DHP) ||
                   (byte2==JPEG_MARKER_EXP) || (byte2==JPEG_MARKER_COM))
                   {
                        index_offset = (f_ptr[index++] << 8);
                        index_offset += (int) (f_ptr[index++]-2);
                        index += index_offset;
                   }
                else if ((byte2>=JPEG_MARKER_APP(0)) && (byte2<=JPEG_MARKER_APP(15)))
                {
                    if(byte2 == JPEG_MARKER_APP(14) && !bJFIF)
                    {
                        JPG_ERR("Unsupport Adobe Marker!!!");
                        return -1;
                    }
                    else
                    {
                        bJFIF = true;
                        index_offset = (f_ptr[index++] << 8);
                        index_offset += (int) (f_ptr[index++]-2);
                        index += index_offset;                        
                    }
                }
                break;
        }

        if((byte1 == JPEG_MARKER_START_CODE) && (byte2 == JPEG_MARKER_EOI))
        {
            break;
        }
    }


   return 0;
}

int jpeg_drv_calculate_info(JPEG_FILE_INFO *info)
{
    INT8 h[3], v[3];
    UINT32 i;
    UINT32 width, height;
    UINT32 y_h_factor, y_v_factor;
    UINT32 comp_h_count, comp_v_count;
    
    switch (info->componentNum)
    {
        case 1:
            info->samplingFormat = 400;
            info->hSamplingFactorMax = info->hSamplingFactor[0];    
            info->vSamplingFactorMax = info->vSamplingFactor[0]; 
            break;

        case 3:
            /// using the sum of Hy & Vy as the color format index
            /// "00" : YUV 4:4:4 (Hy=1, Vy=1), (sum=2) - 2 = 0
            /// "01" : YUV 4:2:2 (Hy=2, Vy=1), (sum=3) - 2 = 1
            /// "10" : YUV 4:2:0 (Hy=2, Vy=2), (sum=4) - 2 = 2
            /// "11" : YUV 4:1:1 (Hy=4, Vy=1), (sum=5) - 2 = 3


            h[0] = info->hSamplingFactor[0];
            h[1] = info->hSamplingFactor[1];
            h[2] = info->hSamplingFactor[2];
            v[0] = info->vSamplingFactor[0];
            v[1] = info->vSamplingFactor[1];
            v[2] = info->vSamplingFactor[2];

            if (1 != (v[1] * v[2] * h[1] * h[2]))
            {
                info->samplingFormat = 0;
                return -1;
            }
            else
            {
                switch (h[0] + v[0] - 2)
                {
                    case 0:
                        info->samplingFormat = 444;
                        info->hSamplingFactorMax = h[0];
                        info->vSamplingFactorMax = v[0];
                        break;
                    
                    case 1:
                        info->samplingFormat = 422;
                        info->hSamplingFactorMax = h[0];
                        info->vSamplingFactorMax = v[0];
                        break;

                    case 2:
                        info->samplingFormat = 420;
                        info->hSamplingFactorMax = h[0];
                        info->vSamplingFactorMax = v[0]; 
                        break;

                    case 3:
                        info->samplingFormat = 411;
                        info->hSamplingFactorMax = h[0];
                        info->vSamplingFactorMax = v[0];
                        break;

                    default:   
                        JPG_ERR("Unsupport color format!!!");
                        info->samplingFormat = 0;
                        info->hSamplingFactorMax = h[0];
                        info->vSamplingFactorMax = v[0];
                        return -1;
                }
            }
            break;

        default:
            JPG_ERR("Unsupport color component number %d!!!", info->componentNum);
            info->samplingFormat = 0;
            return -1;
    }
   


    /// Calculate the paddedWidth & paddedHeight
    width = info->width;
    height = info->height;
    y_h_factor = 2 + info->hSamplingFactor[0];
    y_v_factor = 2 + info->vSamplingFactor[0];
    
    info->mcuPerRow = (width + (1 << y_h_factor) - 1) >> y_h_factor;
    info->totalMcuRows = (height + (1 << y_v_factor) - 1) >> y_v_factor;

    for(i=0; i<info->componentNum ;i++){
      //y_h_factor = 2 + info->hSamplingFactor[i];
      //y_v_factor = 2 + info->vSamplingFactor[i];      
      info->paddedWidth[i]  = info->mcuPerRow << (2 + info->hSamplingFactor[i]);
      info->paddedHeight[i] = info->totalMcuRows << (2 + info->vSamplingFactor[i]);
    }

    if(info->hSamplingFactorMax == 0 || info->vSamplingFactorMax == 0)
    {
        JPG_ERR("SamplingFactorMax = 0 (%d %d) ", info->hSamplingFactorMax, info->vSamplingFactorMax);
        return -1;
    }
    
    // Calculate DU number
    for(i = 0 ; i < info->componentNum ; i++)
    {
        if(info->hSamplingFactor[i] == 0 || info->vSamplingFactor[i] == 0)
        {
            JPG_ERR("SamplingFactor[%d] = 0 (%d %d) ", i, info->hSamplingFactor[i], info->vSamplingFactor[i]);
            return -1;
        }
        comp_h_count = (info->hSamplingFactorMax / info->hSamplingFactor[i]) * 8;
        comp_v_count = (info->vSamplingFactorMax / info->vSamplingFactor[i]) * 8;
        
        info->duPerMCURow[i] = info->mcuPerRow * info->hSamplingFactor[i];  
        //info->dummyDU[i] = (info->paddedWidth - info->width) * info->hSamplingFactor[i] / 
        //                    info->hSamplingFactorMax / 8;
        
        info->compDU[i] = ((info->width + comp_h_count - 1) / comp_h_count) *
                           ((info->height + comp_v_count - 1) / comp_v_count);
    }

    return 0;
} /* parse_jpeg_file() */

