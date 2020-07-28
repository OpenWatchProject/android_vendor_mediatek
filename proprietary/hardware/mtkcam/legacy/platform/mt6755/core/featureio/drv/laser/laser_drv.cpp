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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "LaserDrv"
#include <utils/Errors.h>
#include <fcntl.h>
#include <stdlib.h>  //memset 
#include <stdio.h> //sprintf
#include <cutils/log.h>

#include "MediaTypes.h"
#include "laser_drv.h"

#include <isp_tuning.h>
#include <camera_custom_nvram.h>
#include "nvbuf_util.h"


#define DEBUG_LASER_DRV
#ifdef DEBUG_LASER_DRV
#define DRV_DBG(fmt, arg...) ALOGD(fmt, ##arg)
#define DRV_ERR(fmt, arg...) ALOGD(fmt, ##arg)
#else
#define DRV_DBG(a,...)
#define DRV_ERR(a,...)
#endif

#define LASER_THREAD
#define LASER_CALIB_TO_MAIN2_NVRAM

/*******************************************************************************
*
********************************************************************************/
LaserDrv* 
LaserDrv::getInstance()
{
	static LaserDrv singleton;
	return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
LaserDrv::destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
LaserDrv::LaserDrv()
{
    m_fd_Laser = -1;
	
    m_Lens10cmPosDAC = 0;
    m_Lens50cmPosDAC = 0;
    memset(m_LensDistDac, 0, sizeof(m_LensDistDac)); 
    memset(m_LaserDistRV, 0, sizeof(m_LaserDistRV)); 	
    m_bLaserThreadLoop = 0;
    m_LaserCurPos_DAC = 0;
    m_LaserCurPos_RV = 0;

    DRV_DBG("LaserDrv()\n");
}

/*******************************************************************************
*
********************************************************************************/
LaserDrv::~LaserDrv()
{
}

/*******************************************************************************
*
********************************************************************************/
int
LaserDrv::init(void)
{
	 Mutex::Autolock lock(mLock);
	
	if( m_fd_Laser == -1 )
	{
		m_fd_Laser = open("/dev/stmvl6180",O_RDWR);

		if (m_fd_Laser < 0)
		{
			DRV_DBG("Device error opening : %s", strerror(errno));
			
			return 0;
		}
		
		#ifdef LASER_THREAD
		EnableLaserThread(1);
		#endif
	}
	
	return 1;
}

/*******************************************************************************
*
********************************************************************************/
int
LaserDrv::uninit(void)
{
	Mutex::Autolock lock(mLock);

	if( m_fd_Laser > 0 )  
	{	
		#ifdef LASER_THREAD
		EnableLaserThread(0);
		#endif
	
		close(m_fd_Laser);
		m_fd_Laser = -1;
	}

    	return 1;
}

/*******************************************************************************
*
********************************************************************************/
int
LaserDrv::checkHwSetting(void)
{
	if (ioctl(m_fd_Laser, LASER_IOCTL_INIT , NULL) < 0) 
	{
		DRV_DBG("Laser Error: Could not perform VL6180_IOCTL_INIT");

		return 0;
	}

    	return 1;
}

int 
LaserDrv::EnableLaserThread(int a_bEnable)
{
	#ifdef LASER_THREAD
	if( a_bEnable )
	{
		if( m_bLaserThreadLoop == 0 )		
		{
			DRV_DBG("[LaserThread] Create");

			m_bLaserThreadLoop = 1;

			pthread_create(&m_LaserThread, NULL, LaserThreadFunc, this);
		}
	}  
	else
	{
		if( m_bLaserThreadLoop == 1 )
		{
			m_bLaserThreadLoop = 0;
			
			pthread_join(m_LaserThread, NULL);
			
			DRV_DBG("[LaserThread] Delete");
		}
	}
	#endif

	return 0;
}

void*
LaserDrv::LaserThreadFunc(void* arg)
{
	#ifdef LASER_THREAD
	LaserDrv* _this = reinterpret_cast<LaserDrv*>(arg);

	int RangeValue;
	int DacValue;
	u8 LaserDistIdx;

	while( _this->m_bLaserThreadLoop )  
	{
		_this->getLaserRangeValue(&RangeValue);

		if( ( RangeValue > 70 ) && ( RangeValue < 501 ) )
		{			   
			for( LaserDistIdx = 0; LaserDistIdx < LASER_GOLDEN_TABLE_LEVEL; LaserDistIdx++ )
			{
				if( RangeValue < _this->m_LaserDistRV[LaserDistIdx]  )
				{
					break;
				}
			}
			
			// Transform Laser Range Value into Lens DAC
			if( LaserDistIdx > 0 )
			{
				if( LaserDistIdx == LASER_GOLDEN_TABLE_LEVEL )
				{
					if( ( _this->m_LaserDistRV[LASER_GOLDEN_TABLE_LEVEL - 1] - _this->m_LaserDistRV[LASER_GOLDEN_TABLE_LEVEL - 2] ) != 0 )
					{
						DacValue = ( _this->m_LensDistDac[LASER_GOLDEN_TABLE_LEVEL - 2] * ( _this->m_LaserDistRV[LASER_GOLDEN_TABLE_LEVEL - 1] - RangeValue ) + 
							            _this->m_LensDistDac[LASER_GOLDEN_TABLE_LEVEL - 1] * ( RangeValue - _this->m_LaserDistRV[LASER_GOLDEN_TABLE_LEVEL - 2] ) ) /
							          ( _this->m_LaserDistRV[LASER_GOLDEN_TABLE_LEVEL - 1] - _this->m_LaserDistRV[LASER_GOLDEN_TABLE_LEVEL - 2] );
					}
					else
					{
						DacValue = 0;
					}	
				}
				else
				{
					if( ( _this->m_LaserDistRV[LaserDistIdx] - _this->m_LaserDistRV[LaserDistIdx - 1] ) != 0 )
					{
						DacValue = ( _this->m_LensDistDac[LaserDistIdx - 1] * ( _this->m_LaserDistRV[LaserDistIdx] - RangeValue ) + 
						                    _this->m_LensDistDac[LaserDistIdx] * ( RangeValue - _this->m_LaserDistRV[LaserDistIdx - 1] ) ) /
						                  ( _this->m_LaserDistRV[LaserDistIdx] - _this->m_LaserDistRV[LaserDistIdx - 1] );
					}
					else
					{
						DacValue = 0;
					}
				}
			}
			else
			{
				DacValue = _this->m_LensDistDac[0];
			}
			
			// Remapping Golden DAC Table to Current Lens DAC Table
			if( DacValue > 0 )
			{
				int Dac10cmGT = _this->m_LensDistDac[0];   
				int Dac50cmGT = _this->m_LensDistDac[LASER_GOLDEN_TABLE_LEVEL - 1];

				if( _this->m_Lens10cmPosDAC > 0 && ( Dac50cmGT - Dac10cmGT ) != 0 )
				{
					DacValue = ( _this->m_Lens10cmPosDAC * ( Dac50cmGT - DacValue ) + 
						            _this->m_Lens50cmPosDAC * ( DacValue - Dac10cmGT ) ) /
						          ( Dac50cmGT - Dac10cmGT );
				}
			}
		}
		else
		{
			RangeValue = 0;
			DacValue = 0;
		}

		{
			Mutex::Autolock autoLock(_this->m_LaserMtx); 
			_this->m_LaserCurPos_DAC	= DacValue;
			_this->m_LaserCurPos_RV	= RangeValue;
		}
		
	}
	#endif
	return NULL;
}


/*******************************************************************************
*
********************************************************************************/
void
LaserDrv::setLaserGoldenTable(unsigned int *TableValue)
{
	int i;
	//LC898212AF
	//unsigned int DefaultTable[LASER_GOLDEN_TABLE_LEVEL] = { 9430100, 8730110, 8120120, 7250140, 6640160, 6130180, 5560210, 5180240, 4830270, 4610300, 4330340, 4130380, 3990420, 3830460, 3750500 }; // [71]~[85] for LaserAF GoldenTable 15 Level : Laser Dist & Lens DAC , Value = DAC * 10000 + Dist
	unsigned int Data;

	unsigned int *Value;

	if( TableValue[0] > 50 && TableValue[LASER_GOLDEN_TABLE_LEVEL-1] > 50 )
	{
		Value = TableValue;
	}
	else
	{
		//Value = DefaultTable;
		return;
	}

	for( i = 0; i < LASER_GOLDEN_TABLE_LEVEL; i++ )
	{
		//Value = DAC * 10000 + RV
		Data = (unsigned int)Value[i];
		m_LensDistDac[i] = Value[i] / 10000;
		m_LaserDistRV[i] = Value[i] - m_LensDistDac[i] * 10000;
		DRV_DBG("Laser GT[%d] : %d - %d\n", i, m_LaserDistRV[i], m_LensDistDac[i] );
	}
}


/*******************************************************************************
*
********************************************************************************/
void
LaserDrv::setLensCalibrationData(int LensDAC_10cm, int LensDAC_50cm)
{
	m_Lens10cmPosDAC = LensDAC_10cm;
	m_Lens50cmPosDAC = LensDAC_50cm;
	DRV_DBG("Lens Calib Data : %d - %d\n", m_Lens10cmPosDAC, m_Lens50cmPosDAC );
}


/*******************************************************************************
*
********************************************************************************/
int
LaserDrv:: getLaserRangeValue(int *RangeValue)
{
	int err; 
	int DMAX = 0;
	*RangeValue = 0;
	
	if( m_fd_Laser > 0 )  
	{
		err = ioctl(m_fd_Laser, LASER_IOCTL_GETDATA, RangeValue);

		if (err < 0) 
		{
			DRV_DBG("[getLaserRangingData] ioctl - LASER_IOCTL_GETDATA");

			*RangeValue = 0;

			return 0;
		}
		//DRV_DBG("Laser Range Data : %d\n", *RangeValue);
	}
	
 	return 1;
}

/*******************************************************************************
*
********************************************************************************/
int
LaserDrv:: getLaserCurPos(int *pDacValue, int *pDistValue)
{
	//static int	 LaserDistRV[15] 	= { 100, 110, 120, 130, 140, 150, 170, 190, 210, 250, 290, 340, 390, 440, 500 }; //RV 100 = 10cm , Limit : LaserDistRV[Idx] < LaserDistRV[Idx + 1]
	//static int	 LensDistDac[15] 	= { 414, 399, 381, 366, 351, 340, 326, 306, 292, 289, 267, 259, 256, 233, 216 }; // DW9714AF DAC ,  Limit : LensDistDac[Idx] > LensDistDac[Idx + 1]
	#ifdef LASER_THREAD
	{
		Mutex::Autolock autoLock(m_LaserMtx); 
		*pDacValue	= m_LaserCurPos_DAC;
		*pDistValue 	= m_LaserCurPos_RV;
	}
	#else
	int RangeValue;
	int DacValue;
       u8 LaserDistIdx;
	int Dac10cmGT = m_LensDistDac[0];   
	int Dac50cmGT = m_LensDistDac[LASER_GOLDEN_TABLE_LEVEL - 1];
	
	getLaserRangeValue(&RangeValue);

	if( ( RangeValue > 70 ) && ( RangeValue < 501 ) )
	{			   
		for( LaserDistIdx = 0; LaserDistIdx < LASER_GOLDEN_TABLE_LEVEL; LaserDistIdx++ )
		{
			if( RangeValue < m_LaserDistRV[LaserDistIdx]  )
			{
				break;
			}
		}
		
		// Transform Laser Range Value into Lens DAC
		if( LaserDistIdx > 0 )
		{
			if( LaserDistIdx == LASER_GOLDEN_TABLE_LEVEL )
			{
				if( ( m_LaserDistRV[LASER_GOLDEN_TABLE_LEVEL - 1] - m_LaserDistRV[LASER_GOLDEN_TABLE_LEVEL - 2] ) != 0 )
				{
					DacValue = ( m_LensDistDac[LASER_GOLDEN_TABLE_LEVEL - 2] * ( m_LaserDistRV[LASER_GOLDEN_TABLE_LEVEL - 1] - RangeValue ) + 
						            m_LensDistDac[LASER_GOLDEN_TABLE_LEVEL - 1] * ( RangeValue - m_LaserDistRV[LASER_GOLDEN_TABLE_LEVEL - 2] ) ) /
						          ( m_LaserDistRV[LASER_GOLDEN_TABLE_LEVEL - 1] - m_LaserDistRV[LASER_GOLDEN_TABLE_LEVEL - 2] );
				}
				else
				{
					DacValue = 0;
				}	
			}
			else
			{
				if( ( m_LaserDistRV[LaserDistIdx] - m_LaserDistRV[LaserDistIdx - 1] ) != 0 )
				{
					DacValue = ( m_LensDistDac[LaserDistIdx - 1] * ( m_LaserDistRV[LaserDistIdx] - RangeValue ) + 
					                    m_LensDistDac[LaserDistIdx] * ( RangeValue - m_LaserDistRV[LaserDistIdx - 1] ) ) /
					                  ( m_LaserDistRV[LaserDistIdx] - m_LaserDistRV[LaserDistIdx - 1] );
				}
				else
				{
					DacValue = 0;
				}
			}
		}
		else
		{
			DacValue = m_LensDistDac[0];
		}
		
		// Remapping Golden DAC Table to Current Lens DAC Table
		if( DacValue > 0 )
		{
			if( m_Lens10cmPosDAC > 0 && ( Dac50cmGT - Dac10cmGT ) != 0 )
			{
				DacValue = ( m_Lens10cmPosDAC * ( Dac50cmGT - DacValue ) + 
					            m_Lens50cmPosDAC * ( DacValue - Dac10cmGT ) ) /
					          ( Dac50cmGT - Dac10cmGT );
			}
		}
	}
	else
	{
		RangeValue = 0;
		DacValue = 0;
	}
	
	*pDacValue = DacValue;
	*pDistValue = RangeValue;
	
	#endif

	//DRV_DBG("LaserOut Data : %d - %d\n", *DistValue,  *DacValue);
	
 	return 1;
}

/*******************************************************************************
*
********************************************************************************/
void 
LaserDrv::setLaserCalibrationData(unsigned int OffsetData, unsigned int XTalkData)
{
	int ReadOffset;
	int ReadXTalk;
	int Enable;
	int CalibData;

	ReadOffset = 0;
	ReadXTalk = 0;
	
	#ifdef LASER_CALIB_TO_MAIN2_NVRAM
	NVRAM_LENS_PARA_STRUCT* pNVRAM_LASER;

	int err = NvBufUtil::getInstance().getBufAndReadNoDefault(CAMERA_NVRAM_DATA_LENS, ESensorDev_MainSecond, (void*&)pNVRAM_LASER);
	int val; 

	if( err == 0 )
	{
		if( pNVRAM_LASER->rAFNVRAM.i4Coefs[71] == 0x52 )
		{
			val = pNVRAM_LASER->rAFNVRAM.i4Coefs[72];
			DRV_DBG("Laser Offset Calib Data : %d", val);
			setLaserOffsetCalib(val);
			ReadOffset = 1;
		}
		if( pNVRAM_LASER->rAFNVRAM.i4Coefs[73] == 0x52 )
		{
			val = pNVRAM_LASER->rAFNVRAM.i4Coefs[74];
			DRV_DBG("Laser XTalk Calib Data : %d", val);
			setLaserXTalkCalib(val);
			ReadXTalk = 1;
		}
	}
	#endif

	Enable = 0;
	CalibData = 0;
	
	if( ReadOffset == 0 )
	{
		Enable = OffsetData / 10000;
		CalibData = OffsetData - Enable * 10000;
		if( Enable == 1 )
		{
			DRV_DBG("NVRAM Laser Offset Calib Data : %d", CalibData);
			setLaserOffsetCalib(CalibData);
		}
	}

	Enable = 0;
	CalibData = 0;
	
	if( ReadXTalk == 0 )
	{
		Enable = XTalkData / 10000;
		CalibData = XTalkData - Enable * 10000;
		if( Enable == 1 )
		{
			DRV_DBG("NVRAM Laser XTalk Calib Data : %d", CalibData);
			setLaserXTalkCalib(CalibData);
		}
	}	
}

/*******************************************************************************
*
********************************************************************************/
int 
LaserDrv::getLaserOffsetCalib(int *Value)
{
	int err;  
	*Value = 0;
	
	if( m_fd_Laser > 0 )  
	{
		err = ioctl(m_fd_Laser, LASER_IOCTL_GETOFFCALB, Value);

		if (err < 0) 
		{
			DRV_DBG("[getLaserOffsetCalib] ioctl - LASER_IOCTL_GETOFFCALB");

			*Value = 0;

			return 0;
		} 
	}

	#ifdef LASER_CALIB_TO_MAIN2_NVRAM
	NVRAM_LENS_PARA_STRUCT* pNVRAM_LASER;		 
	
	err = NvBufUtil::getInstance().getBufAndReadNoDefault(CAMERA_NVRAM_DATA_LENS, ESensorDev_MainSecond, (void*&)pNVRAM_LASER);

	DRV_DBG("[getLaserOffsetCalib] LaserCali : %d(%d)", err, *Value);
	
	if( err == 0 )
	{
		pNVRAM_LASER->rAFNVRAM.i4Coefs[71] = 0x52;
		pNVRAM_LASER->rAFNVRAM.i4Coefs[72] = *Value;
		NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_LENS, ESensorDev_MainSecond);
	}
	#endif
	         		
 	return 1;	
}

int 
LaserDrv::setLaserOffsetCalib(int Value)
{
	int err;   
	
	if( m_fd_Laser > 0 )  
	{
		DRV_DBG("Laser Offset Calib Data : %d \n", Value);
		
		err = ioctl(m_fd_Laser, LASER_IOCTL_SETOFFCALB, Value);

		if (err < 0) 
		{
			DRV_DBG("[setLaserOffsetCalib] ioctl - LASER_IOCTL_SETOFFCALB"); 

			return 0;
		} 
	}
	
 	return 1;		
}

int 
LaserDrv::getLaserXTalkCalib(int *Value)
{
	int err;  
	*Value = 0;
	
	if( m_fd_Laser > 0 )  
	{
		err = ioctl(m_fd_Laser, LASER_IOCTL_GETXTALKCALB, Value);

		if (err < 0) 
		{
			DRV_DBG("[getLaserXTalkCalib] ioctl - LASER_IOCTL_GETXTALKCALB");

			*Value = 0;

			return 0;
		} 
	}

	#ifdef LASER_CALIB_TO_MAIN2_NVRAM
	NVRAM_LENS_PARA_STRUCT* pNVRAM_LASER;
	
	err = NvBufUtil::getInstance().getBufAndReadNoDefault(CAMERA_NVRAM_DATA_LENS, ESensorDev_MainSecond, (void*&)pNVRAM_LASER);

	DRV_DBG("[getLaserXTalkCalib] LaserCali : %d(%d)", err, *Value);
	
	if( err == 0 )
	{
		pNVRAM_LASER->rAFNVRAM.i4Coefs[73] = 0x52;
		pNVRAM_LASER->rAFNVRAM.i4Coefs[74] = *Value;
		NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_LENS, ESensorDev_MainSecond);
	}
	#endif
	
	return 1;
}

int 
LaserDrv::setLaserXTalkCalib(int Value)
{
	int err;   
	
	if( m_fd_Laser > 0 )  
	{
		DRV_DBG("Laser XTalk Calib Data : %d \n", Value);
		
		err = ioctl(m_fd_Laser, LASER_IOCTL_SETXTALKCALB, Value);

		if (err < 0) 
		{
			DRV_DBG("[setLaserXTalkCalib] ioctl - LASER_IOCTL_SETXTALKCALB"); 

			return 0;
		} 
	}
	
	return 1;
}


