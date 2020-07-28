#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "SonyPdafTransCoord.h"
#include <pd_imx230mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "pd_buf_mg_imx230mipiraw"

//define calibration block number for imx230. This is depended on sensor setting.
#define _PD_BLK_NUM_W_ 8
#define _PD_BLK_NUM_H_ 6
//define pd block number for imx230. This is depended on calibration data in EEPROM/OTP
#define _CALI_BLK_NUM_W_ 8
#define _CALI_BLK_NUM_H_ 6

//Sensor output data isze is 16*12*5(PD/CL data)+5(Header) Bytes, no matter what kind of sensor setting is.
#define dSize 16*12
//calibration data block size.
#define cSize _CALI_BLK_NUM_W_*_CALI_BLK_NUM_H_

PDBufMgrOpen* 
PD_IMX230MIPIRAW::getInstance()
{
    static PD_IMX230MIPIRAW singleton;
    return &singleton;

}


PD_IMX230MIPIRAW::PD_IMX230MIPIRAW()
{
	m_phase_difference = new MUINT16 [dSize];
	m_confidence_level = new MUINT16 [dSize];
	m_calibration_data = new MUINT16 [cSize];
	m_ParamDataForConvertingAddress = new SonyPdLibSensorCoordSetting_t;
	


	m_XKnotNum1 = _CALI_BLK_NUM_W_;
	m_YKnotNum1 = _CALI_BLK_NUM_H_;
	
	m_XKnotNum2 = _CALI_BLK_NUM_W_;
	m_YKnotNum2 = _CALI_BLK_NUM_H_;
	
	m_PointNumForThrLine = 5;

	m_CurrMode = 0; //default : all pixel mode.

	// Malloc
	m_SonyPdLibInputData.p_SlopeData				= (signed long *)malloc( sizeof(signed long) * m_XKnotNum1 * m_YKnotNum1 );
	m_SonyPdLibInputData.p_OffsetData 				= (signed long *)malloc( sizeof(signed long) * m_XKnotNum1 * m_YKnotNum1 );
	m_SonyPdLibInputData.p_XAddressKnotSlopeOffset	= (unsigned short *)malloc( sizeof(unsigned short) * m_XKnotNum1 );
	m_SonyPdLibInputData.p_YAddressKnotSlopeOffset	= (unsigned short *)malloc( sizeof(unsigned short) * m_YKnotNum1 );
	m_SonyPdLibInputData.p_DefocusOKNGThrLine 		= (DefocusOKNGThrLine_t *)malloc( sizeof(DefocusOKNGThrLine_t) * m_XKnotNum2 * m_YKnotNum2 );
		
	for( unsigned short i = 0; i < m_XKnotNum2*m_YKnotNum2; i++ )
	{
		m_SonyPdLibInputData.p_DefocusOKNGThrLine[i].p_AnalogGain = (unsigned long *)malloc( sizeof(unsigned long) * m_PointNumForThrLine );
		m_SonyPdLibInputData.p_DefocusOKNGThrLine[i].p_Confidence = (unsigned long *)malloc( sizeof(unsigned long) * m_PointNumForThrLine ); 
	}
		
	m_SonyPdLibInputData.p_XAddressKnotDefocusOKNG	= (unsigned short *)malloc( sizeof(unsigned short) * m_XKnotNum2 );
	m_SonyPdLibInputData.p_YAddressKnotDefocusOKNG	= (unsigned short *)malloc( sizeof(unsigned short) * m_YKnotNum2 );


	MY_LOG("[%s]\n", __FUNCTION__);

}

PD_IMX230MIPIRAW::~PD_IMX230MIPIRAW()
{
	if( m_phase_difference)
		delete m_phase_difference;

	if( m_confidence_level)
		delete m_confidence_level;

	if( m_calibration_data)
		delete m_calibration_data;

	if( m_ParamDataForConvertingAddress)
		delete m_ParamDataForConvertingAddress;

	m_phase_difference = NULL;
	m_confidence_level = NULL;
	m_calibration_data = NULL;
	m_ParamDataForConvertingAddress = NULL;


	// Finalize
	// Free
	free ( m_SonyPdLibInputData.p_YAddressKnotDefocusOKNG );
	free ( m_SonyPdLibInputData.p_XAddressKnotDefocusOKNG );
			
	for( unsigned short i = 0; i < m_XKnotNum2*m_YKnotNum2; i++ )
	{
		free ( m_SonyPdLibInputData.p_DefocusOKNGThrLine[i].p_Confidence );
		free ( m_SonyPdLibInputData.p_DefocusOKNGThrLine[i].p_AnalogGain );
	}
			
	free ( m_SonyPdLibInputData.p_DefocusOKNGThrLine );
	free ( m_SonyPdLibInputData.p_YAddressKnotSlopeOffset );
	free ( m_SonyPdLibInputData.p_XAddressKnotSlopeOffset );
	free ( m_SonyPdLibInputData.p_OffsetData );
	free ( m_SonyPdLibInputData.p_SlopeData  );

}



MINT32 PD_IMX230MIPIRAW::SetRegData(unsigned short &CurrMode)
{
	MINT32 ret=D_SONY_PD_LIB_REGDATA_IS_NG;
		
	memset( m_ParamDataForConvertingAddress, 0, sizeof(SonyPdLibSensorCoordSetting_t));

	if( CurrMode==0 || CurrMode==2)
	{
		SonyPdLibSensorCoordRegData_t ImageSensorRegisterData; // image sensor register data

		// HDR Mode
		ImageSensorRegisterData.reg_addr_0x0220 = 0x00;	// HDR mode
		ImageSensorRegisterData.reg_addr_0x0221 = 0x11;	// [7:4]HDR_RESO_REDU_H[3:0]HDR_RESO_REDU_V
		// Analog Crop
		ImageSensorRegisterData.reg_addr_0x0344 = 0x00;	// x_add_sta
		ImageSensorRegisterData.reg_addr_0x0345 = 0x00;	
		ImageSensorRegisterData.reg_addr_0x0346 = 0x00;	// y_add_sta
		ImageSensorRegisterData.reg_addr_0x0347 = 0x00;	
		// Sub-sampling
		ImageSensorRegisterData.reg_addr_0x0381 = 0x01;	// x_evn_inc
		ImageSensorRegisterData.reg_addr_0x0383 = 0x01;	// x_odd_inc
		ImageSensorRegisterData.reg_addr_0x0385 = 0x01;	// y_eve_inc
		ImageSensorRegisterData.reg_addr_0x0387 = 0x01;	// y_odd_inc
		//Binning mode
		ImageSensorRegisterData.reg_addr_0x0900 = 0x00; // BINNING_MODE <= @Tim:should be modified
		ImageSensorRegisterData.reg_addr_0x0901 = 0x11; 	// [7:4]BINNING_TYPE_H [3:0]BINNING_TYPE_V <= @Tim:should be modified

		// Scaling
		ImageSensorRegisterData.reg_addr_0x0401 = 0x00;	// SCALE_MODE
		ImageSensorRegisterData.reg_addr_0x0404 = 0x00;	// SCALE_M
		ImageSensorRegisterData.reg_addr_0x0405 = 0x10;			
		// Digital Crop
		ImageSensorRegisterData.reg_addr_0x0408 = 0x00;	// DIG_CROP_X_OFFSET
		ImageSensorRegisterData.reg_addr_0x0409 = 0x00;	
		ImageSensorRegisterData.reg_addr_0x040A = 0x00;	// DIG_CROP_Y_OFFSET
		ImageSensorRegisterData.reg_addr_0x040B = 0x00;	
		// Mirror / Flip
		ImageSensorRegisterData.reg_addr_0x0101 = 0x00;	// orient H, orient V 
		
		if( CurrMode==2)// bining mode	
		{
			ImageSensorRegisterData.reg_addr_0x0348 = 0x0A; // x_add_end
			ImageSensorRegisterData.reg_addr_0x0349 = 0x6F; 
			ImageSensorRegisterData.reg_addr_0x034A = 0x07; // y_add_end
			ImageSensorRegisterData.reg_addr_0x034B = 0xD7; 					
			// Output Size Setting
			// Output
			ImageSensorRegisterData.reg_addr_0x034C = 0x0A; // x_out_size <= @Tim:should be modified 0x14e0 or 0xa70
			ImageSensorRegisterData.reg_addr_0x034D = 0x6F;
			ImageSensorRegisterData.reg_addr_0x034E = 0x07; // y_out_size <= @Tim:should be modified 0xfb0 or 0x7d8
			ImageSensorRegisterData.reg_addr_0x034F = 0xD7;
			
			// Digital Crop
			ImageSensorRegisterData.reg_addr_0x040C = 0x0A; // DIG_CROP_IMAGE_WIDTH <= @Tim:should be modified 0x14e0 or 0xa70
			ImageSensorRegisterData.reg_addr_0x040D = 0x6F;
			ImageSensorRegisterData.reg_addr_0x040E = 0x07; // DIG_CROP_IMAGE_HEIGHT <= @Tim:should be modified 0xfb0 or 0x7d8
			ImageSensorRegisterData.reg_addr_0x040F = 0xD7;
		}
		else // all pixel mode
		{
			
			ImageSensorRegisterData.reg_addr_0x0348 = 0x14; // x_add_end
			ImageSensorRegisterData.reg_addr_0x0349 = 0xDF; 
			ImageSensorRegisterData.reg_addr_0x034A = 0x0F; // y_add_end
			ImageSensorRegisterData.reg_addr_0x034B = 0xAF; 		
			// Output Size Setting
			// Output
			ImageSensorRegisterData.reg_addr_0x034C = 0x14; // x_out_size <= @Tim:should be modified 0x14e0 or 0xa70
			ImageSensorRegisterData.reg_addr_0x034D = 0xDF;
			ImageSensorRegisterData.reg_addr_0x034E = 0x0F; // y_out_size <= @Tim:should be modified 0xfb0 or 0x7d8
			ImageSensorRegisterData.reg_addr_0x034F = 0xAF;
			
			// Digital Crop
			ImageSensorRegisterData.reg_addr_0x040C = 0x14; // DIG_CROP_IMAGE_WIDTH <= @Tim:should be modified 0x14e0 or 0xa70
			ImageSensorRegisterData.reg_addr_0x040D = 0xDF;
			ImageSensorRegisterData.reg_addr_0x040E = 0x0F; // DIG_CROP_IMAGE_HEIGHT <= @Tim:should be modified 0xfb0 or 0x7d8
			ImageSensorRegisterData.reg_addr_0x040F = 0xAF;

		}

		// extract parameter data for converting address from image sensor register data
		ret = SonyPdLibInterpretRegData(
			//Input
			&ImageSensorRegisterData, 		// image sensor register data
			//Output
			m_ParamDataForConvertingAddress );	// parameter data for converting address
	}

	return ret;



}

MBOOL PD_IMX230MIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
{
	MBOOL ret = MFALSE;
	
	//enable/disable debug log	
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.af_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);


	//binning ( V:1/2, H:1/2) mode and all-pixel mode.
	if( ( iPdProfile.uImgXsz==2672 && iPdProfile.uImgYsz==2008) || ( iPdProfile.uImgXsz==5344 && iPdProfile.uImgYsz==4016) )
	{
		ret = MTRUE;
		m_CurrMode = iPdProfile.u4IsZSD ? 0 : 2;
	}
	else
	{
		m_CurrMode = 1;
	}

	//for transform ROI coordinate.
	m_IsCfgCoordSetting = SetRegData( m_CurrMode);

	MY_LOG("[%s] %d, CurMode=%d, ImgSZ=(%d, %d)\n", __FUNCTION__, ret, m_CurrMode, iPdProfile.uImgXsz, iPdProfile.uImgYsz);
	return ret;

}

MINT32 PD_IMX230MIPIRAW::PD_IMX230MIPIRAW::GetPDCalSz()
{
	return 96;
}


MBOOL PD_IMX230MIPIRAW::GetPDInfo2HybridAF( MINT32 i4InArySz, MINT32 *i4OutAry)
{
	MBOOL ret=MFALSE;
	//rest
	memset( i4OutAry, 0, i4InArySz*sizeof(MINT32));

	//i4InArySz should be 10 which is defined by core PD algorithm.
	if( i4InArySz==10 && i4OutAry)
	{
		// output block size setting for Hybrid AF to crop PD alayzing area.
		switch( m_CurrMode)
		{
			ret=MTRUE;
			case 0 : // all pixel mode
				i4OutAry[0]=5344/_PD_BLK_NUM_W_;
				i4OutAry[1]=4016/_PD_BLK_NUM_H_;
				break;
			case 2 : // bining mode 
				i4OutAry[0]=2672/_PD_BLK_NUM_W_;
				i4OutAry[1]=2008/_PD_BLK_NUM_H_;
				break;
			default : 
				ret=MFALSE;
				MY_LOG("Current Sensor mode(%d) is not support PDAF\n", m_CurrMode);
				break;		
		}
	}
	else
	{
		ret=MFALSE;
		MY_LOG("[%s] Fail, Sz=%d, Addr=0x%x\n", __FUNCTION__, i4InArySz, i4OutAry);		
	}

	return ret;


}



MBOOL PD_IMX230MIPIRAW::ExtractPDCL()
{
	MUINT8 flexibleEn = m_databuf[0];
	MUINT8 modesel = m_databuf[1]>>6;

	
	MUINT8 offset = 5;
	MUINT8 *ptr = &m_databuf[offset];
		
	for( int i=0; i<dSize; i++)
	{
		m_confidence_level[i] = 0xff  & (ptr[i*5]);  
		m_phase_difference[i] = 0x3ff & (ptr[i*5+1]<<2 | ptr[i*5+2]>>6);

	}

	char value[200] = {'\0'};
	property_get("vendor.vc.dump.enable", value, "0");
	MBOOL bEnable = atoi(value);
	if (bEnable)
	{
		char fileName[64];
		sprintf(fileName, "/sdcard/vc/%d_pd.raw", m_frm_num);
		FILE *fp = fopen(fileName, "w");
		if (NULL == fp)
		{
			return MFALSE;
		}	 
		fwrite(reinterpret_cast<void *>(m_phase_difference), 1, 16*12*2, fp);
		fclose(fp); 	   
		
		sprintf(fileName, "/sdcard/vc/%d_cl.raw", m_frm_num);
		fp = fopen(fileName, "w");
		if (NULL == fp)
		{
			return MFALSE;
		}	 
		fwrite(reinterpret_cast<void *>(m_confidence_level), 1, 16*12*2, fp);
		fclose(fp); 
	}

	return MTRUE;

	
}


MBOOL PD_IMX230MIPIRAW::ExtractCaliData()
{
	MY_LOG("[%s] Sz=%d", __FUNCTION__, cSize);

	for( int i=0; i<cSize; i++)
	{
		m_calibration_data[i] = 0xffff & ( m_calidatabuf[i*2]<<8 | m_calidatabuf[i*2+1]);

	}

	// Slope and offset (defocus vs phase difference)
	// Set slope
	for( unsigned short i = 0; i < m_XKnotNum1*m_YKnotNum1; i++ )
	{
		m_SonyPdLibInputData.p_SlopeData[i] = m_calibration_data[i]; 
		MY_LOG("Slope Data[%d] : %6d\n", i,(int)m_SonyPdLibInputData.p_SlopeData[i]);
	}
				
	// Set offset
	for( unsigned short i = 0; i < m_XKnotNum1*m_YKnotNum1; i++ )
	{
		m_SonyPdLibInputData.p_OffsetData[i] = 0;
		MY_LOG("Offset Data[%d] : %6d\n", i,(int)m_SonyPdLibInputData.p_OffsetData[i]);
	}
				

	char value[200] = {'\0'};
	property_get("vendor.vc.dump.enable", value, "0");
	MBOOL bEnable = atoi(value);
	if (bEnable)
	{
		char fileName[64];
		sprintf(fileName, "/sdcard/vc/0_cali.raw");
		FILE *fp = fopen(fileName, "w");
		if (NULL == fp)
		{
			return MFALSE;
		}	 
		fwrite(reinterpret_cast<void *>(m_calibration_data), 1, 8*6*2, fp);
		fclose(fp); 	   
	}

	return MTRUE;
	

}



MRESULT PD_IMX230MIPIRAW::GetVersionOfPdafLibrary( SPDLibVersion_t &tOutSWVer)
{
	// Input
	SonyPdLibVersion_t SonyPdLibVersion;
	
	// Execute
	SonyPdLibGetVersion(&SonyPdLibVersion);

	tOutSWVer.MajorVersion = SonyPdLibVersion.MajorVersion;
	tOutSWVer.MinorVersion = SonyPdLibVersion.MinorVersion;
	
	// Output
	MY_LOG_IF( m_bDebugEnable, "%s = %d.%d\n", __FUNCTION__, (int)tOutSWVer.MajorVersion,(int)tOutSWVer.MinorVersion);

	if( tOutSWVer.MajorVersion==0 && tOutSWVer.MinorVersion==0)
	{
		MY_LOG("[WAR] [%s] Please lincess IMX230 pdaf library from Sony FAE!!!\n", __FUNCTION__);
	}

	return MTRUE;
}



MBOOL PD_IMX230MIPIRAW::TransROICoord( SPDROI_T &srcROI, SPDROI_T &dstROI)
{
	MBOOL ret = MFALSE;

	// declare variables
	SonyPdLibRect_t               InputWindowAddress;				// window address on image sensor output image (input)
	SonyPdLibRect_t               OutputWindowAddress;				// window address for PDAF Library             (output)
	
	// input image sensor register data related to scaling, cropping, mirroring and flipping
	// about image sensor register, please refer to 
	//                                              IMX230_Software_Reference_Manual_X.X.X.pdf
	//                                              IMX230ES_RegisterMap_verX.X_XXXXXX.xlsx	
		
			
	// if parameter data can be extracted successfully
	if( (m_ParamDataForConvertingAddress->img_orientation_h==1 || m_ParamDataForConvertingAddress->img_orientation_v==1) &&
		(m_IsCfgCoordSetting==D_SONY_PD_LIB_REGDATA_IS_OK) )
	{
		// input window address on image sensor output image
		
		// i.e. AF window is set the addresses after output cropping
		InputWindowAddress.sta.x = srcROI.i4XStart;
		InputWindowAddress.sta.y = srcROI.i4YStart;
		InputWindowAddress.end.x = srcROI.i4XEnd;
		InputWindowAddress.end.y = srcROI.i4YEnd;
		
		MY_LOG("[%s] Input : Win Start(%4d, %4d), Win End(%4d, %4d)\n", 
			__FUNCTION__, 
			InputWindowAddress.sta.x, 
			InputWindowAddress.sta.y, 
			InputWindowAddress.end.x, 
			InputWindowAddress.end.y);
		
				
		// convert address for PDAF Library 
		OutputWindowAddress = SonyPdLibTransOutputRectToPdafRect(
			//Input
			InputWindowAddress,					// window address on image sensor output image
			m_ParamDataForConvertingAddress );	// parameter data for converting address

		//output X coordinate
		if( OutputWindowAddress.sta.x<OutputWindowAddress.end.x)
		{
			dstROI.i4XStart = OutputWindowAddress.sta.x;
			dstROI.i4XEnd	= OutputWindowAddress.end.x;
		}
		else
		{
			dstROI.i4XStart = OutputWindowAddress.end.x;
			dstROI.i4XEnd	= OutputWindowAddress.sta.x;
		}

		//output Y coordinate
		if( OutputWindowAddress.sta.y<OutputWindowAddress.end.y)
		{
			dstROI.i4YStart = OutputWindowAddress.sta.y;
			dstROI.i4YEnd	= OutputWindowAddress.end.y;
		}
		else
		{
			dstROI.i4YStart = OutputWindowAddress.end.y;
			dstROI.i4YEnd	= OutputWindowAddress.sta.y;
		}
		
		// the converted address for PDAF Library
		MY_LOG_IF( m_bDebugEnable, "D_SONY_PD_LIB_REGDATA_IS_OK\n");

		ret = MTRUE;
	}
	else
	{
		dstROI.i4XStart = srcROI.i4XStart;
		dstROI.i4YStart = srcROI.i4YStart;
		dstROI.i4XEnd   = srcROI.i4XEnd;
		dstROI.i4YEnd   = srcROI.i4YEnd;
	
		MY_LOG_IF( m_bDebugEnable, "D_SONY_PD_LIB_REGDATA_IS_NG\n");
	}

	MY_LOG("[%s] Output : Win Start(%4d, %4d), Win End(%4d, %4d)\n\n", 
		__FUNCTION__, 
		dstROI.i4XStart, 
		dstROI.i4YStart, 
		dstROI.i4XEnd, 
		dstROI.i4YEnd);
	return ret;


}


MBOOL PD_IMX230MIPIRAW::GetDefocus( SPDROIInput_T &iPDInputData, SPDROIResult_T &oPdOutputData)
{
	
	signed long    Ret = D_SONY_PD_LIB_E_NG;
	unsigned short i   = 0;
	unsigned long  k   = 0;
	
	unsigned short XSize = iPDInputData.XSizeOfImage;	
	unsigned short YSize = iPDInputData.YSizeOfImage;

	// transform ROI coordinate for Mirror / Flip case.
	SPDROI_T convertedROI;
	TransROICoord( iPDInputData.ROI, convertedROI);
	
	// Input
	// Phase difference data and confidence level
	MUINT16 szX = iPDInputData.XSizeOfImage/_PD_BLK_NUM_W_;
	MUINT16 szY = iPDInputData.YSizeOfImage/_PD_BLK_NUM_H_;
	MUINT8 Xidx = (MUINT8)(( (convertedROI.i4XStart+convertedROI.i4XEnd)/2 + szX/2 )/szX);  
	MUINT8 Yidx = (MUINT8)(( (convertedROI.i4YStart+convertedROI.i4YEnd)/2 + szY/2 )/szY);  
	m_SonyPdLibInputData.PhaseDifference = (signed long) (m_phase_difference[Yidx*16+Xidx]>=512 ? m_phase_difference[Yidx*16+Xidx]-1024 : m_phase_difference[Yidx*16+Xidx]);
	m_SonyPdLibInputData.ConfidenceLevel = (signed long) (m_confidence_level[Yidx*16+Xidx]>=512 ? m_confidence_level[Yidx*16+Xidx]-1024 : m_confidence_level[Yidx*16+Xidx]);
			
	MY_LOG("[%s] ImgSz=(%4d,%4d), BlkSZ=(%4d,%4d), BlkIdx=(%2d,%2d)\n", __FUNCTION__, iPDInputData.XSizeOfImage, iPDInputData.YSizeOfImage,szX, szY, Xidx, Yidx);
			
	MY_LOG("[%s] PhaseDifference : %4d\n", __FUNCTION__, (int)m_SonyPdLibInputData.PhaseDifference);
	MY_LOG("[%s] ConfidenceLevel : %4d\n", __FUNCTION__, (int)m_SonyPdLibInputData.ConfidenceLevel);		
	// PDAF window
	// Address is required to be converted into all-pixel mode address 
	// before scaling, cropping, mirroring and flipping
			
	// PDAF window information must be 
	// in synchronization with phase difference data and confidence level
			
	// Set size
	m_SonyPdLibInputData.XSizeOfImage = XSize;
	m_SonyPdLibInputData.YSizeOfImage = YSize;
			
	// Set PDAF window
	m_SonyPdLibInputData.XAddressOfWindowStart	= convertedROI.i4XStart;
	m_SonyPdLibInputData.YAddressOfWindowStart	= convertedROI.i4YStart;
	m_SonyPdLibInputData.XAddressOfWindowEnd	= convertedROI.i4XEnd;
	m_SonyPdLibInputData.YAddressOfWindowEnd	= convertedROI.i4YEnd;
	MY_LOG("[%s] WinAddr (%4d,%4d), (%4d,%4d)\n", 
		__FUNCTION__,
		(int)m_SonyPdLibInputData.XAddressOfWindowStart,
		(int)m_SonyPdLibInputData.YAddressOfWindowStart,
		(int)m_SonyPdLibInputData.XAddressOfWindowEnd,
		(int)m_SonyPdLibInputData.YAddressOfWindowEnd);
			
			
	// Set the number of knots
	m_SonyPdLibInputData.XKnotNumSlopeOffset = m_XKnotNum1;
	m_SonyPdLibInputData.YKnotNumSlopeOffset = m_YKnotNum1;
			
			
	// Set x address of konts
	for( i = 0; i < m_XKnotNum1; i++ )
	{
		m_SonyPdLibInputData.p_XAddressKnotSlopeOffset[i] = 0 + i * XSize / (m_XKnotNum1-1);	// Value is as an example
	}
			
	// Set y address of konts
	for( i = 0; i < m_YKnotNum1; i++ )
	{
		m_SonyPdLibInputData.p_YAddressKnotSlopeOffset[i] = 0 + i * YSize / (m_YKnotNum1-1);	// Value is as an example
	}

	// Set adjustment coefficient of slope according to image sensor mode
	// Set phase detection pixel density aaccording to image sensor mode
	switch( m_CurrMode)
	{
		case 2 : // bining mode	
			m_SonyPdLibInputData.DensityOfPhasePix = D_SONY_PD_LIB_DENSITY_SENS_MODE2;
			m_SonyPdLibInputData.AdjCoeffSlope     = D_SONY_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE2;
			break;
		default : // all pixel mode
			m_SonyPdLibInputData.DensityOfPhasePix = D_SONY_PD_LIB_DENSITY_SENS_MODE0;	
			m_SonyPdLibInputData.AdjCoeffSlope	   = D_SONY_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE0;
			break;

	}
	MY_LOG("[%s] AdjCoeffSlope     : %d\n", __FUNCTION__, (int)m_SonyPdLibInputData.AdjCoeffSlope);
	MY_LOG("[%s] DensityOfPhasePix : %d\n", __FUNCTION__, (int)m_SonyPdLibInputData.DensityOfPhasePix);
		
	// Defocus OK/NG : not using
	// Set image sensor analog gain
	// which must be in synchronization with phase difference data and confidence level
	m_SonyPdLibInputData.ImagerAnalogGain = 10;			// Value is as an example //@Tim
			
	// Set the number of knots
	m_SonyPdLibInputData.XKnotNumDefocusOKNG = m_XKnotNum2;	// Value is as an example
	m_SonyPdLibInputData.YKnotNumDefocusOKNG = m_YKnotNum2;	// Value is as an example
			
	// Set the threshold line
	for( i = 0; i < m_XKnotNum2*m_YKnotNum2; i++ )
	{
		m_SonyPdLibInputData.p_DefocusOKNGThrLine[i].PointNum = m_PointNumForThrLine;
				
		for( k = 0; k < m_PointNumForThrLine; k++ )
		{
			m_SonyPdLibInputData.p_DefocusOKNGThrLine[i].p_AnalogGain[k] = 10 * k;	// Value is as an example
			m_SonyPdLibInputData.p_DefocusOKNGThrLine[i].p_Confidence[k] =  2 * k;	// Value is as an example
		}
	}
			
	// Set x address of konts
	for( i = 0; i < m_XKnotNum2; i++ )
	{
		m_SonyPdLibInputData.p_XAddressKnotDefocusOKNG[i] = 0 + i * XSize / (m_XKnotNum2-1);	// Value is as an example
	}
			
	// Set y address of konts
	for( i = 0; i < m_YKnotNum2; i++ )
	{
		m_SonyPdLibInputData.p_YAddressKnotDefocusOKNG[i] = 0 + i * YSize / (m_YKnotNum2-1);	// Value is as an example
	}
		
	
	// Execute
	// Get defocus data
	Ret = SonyPdLibGetDefocus(&m_SonyPdLibInputData, &m_SonyPdLibOutputData);
	
	// Output
	if( Ret == D_SONY_PD_LIB_E_OK )
	{


		//(1) normalize to fit API spec : output target position.
		oPdOutputData.Defocus                = iPDInputData.curLensPos + ((-1)*m_SonyPdLibOutputData.Defocus/16384);
		//(2) normalize to fit API spec : confidence level is in ragne 0~100.
		oPdOutputData.DefocusConfidence      = (int)(m_SonyPdLibInputData.ConfidenceLevel*100/255);
		//(1) normalize to fit API spec : confidence level is in ragne 0~100.
		oPdOutputData.DefocusConfidenceLevel = (int)(m_SonyPdLibInputData.ConfidenceLevel*100/255);
		//(1) normalize to fit API spec : pixel base.
		oPdOutputData.PhaseDifference        = (-1)*m_SonyPdLibInputData.PhaseDifference*1000/16;
		
		MY_LOG_IF( m_bDebugEnable,"[%s] Output:Reurn Value == D_SONY_PD_LIB_E_OK\n", __FUNCTION__);
		MY_LOG("[%s] ---------------------------------\n", __FUNCTION__);
		// Defocus data
		MY_LOG("[%s] CurrLensPos           : %4d\n", __FUNCTION__, (int)iPDInputData.curLensPos);
		MY_LOG("[%s] Defocus               : %4d\n", __FUNCTION__, (int)m_SonyPdLibOutputData.Defocus);
		MY_LOG("[%s] DefocusConfidence     : %4d\n", __FUNCTION__, (int)m_SonyPdLibOutputData.DefocusConfidence);
		MY_LOG("[%s] DefocusConfidenceLevel: %4d\n", __FUNCTION__, (int)m_SonyPdLibOutputData.DefocusConfidenceLevel);
		MY_LOG("[%s] PhaseDifference       : %4d\n", __FUNCTION__, (int)m_SonyPdLibOutputData.PhaseDifference);
		MY_LOG("[%s][Output] Target %4d DAC, CL %4d, PD %4d\n", 
			__FUNCTION__, 
			oPdOutputData.Defocus, 
			oPdOutputData.DefocusConfidenceLevel, 
			oPdOutputData.PhaseDifference);	
	}
	else
	{
		oPdOutputData.Defocus                = 0;
		oPdOutputData.DefocusConfidence      = 0;
		oPdOutputData.DefocusConfidenceLevel = 0;
		oPdOutputData.PhaseDifference        = 0;
		MY_LOG("[%s] Output:Reurn Value == D_SONY_PD_LIB_E_NG\n", __FUNCTION__);
	}

		

	MY_LOG("[%s] ---------------------------------\n", __FUNCTION__);
	
	return true;		
}
