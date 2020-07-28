#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "pd_buf_mgr_open.h"

#include <SonyIMX230PdafLibrary.h>


#ifndef _PD_IMX230MIPIRAW_H_
#define _PD_IMX230MIPIRAW_H_

typedef struct _SonyPdLibSensorCoordSetting_t __SonyPdLibSensorCoordSetting_t;

class PD_IMX230MIPIRAW : protected PDBufMgrOpen
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.

	MBOOL m_bDebugEnable;

protected :
	
    MBOOL IsSupport( SPDProfile_t &iPdProfile); 
	MBOOL ExtractPDCL(); 
	MBOOL ExtractCaliData(); 


	unsigned short m_XKnotNum1;
	unsigned short m_YKnotNum1;
	
	unsigned short m_XKnotNum2;
	unsigned short m_YKnotNum2;

	unsigned short m_PointNumForThrLine;

	int m_IsCfgCoordSetting;
	__SonyPdLibSensorCoordSetting_t *m_ParamDataForConvertingAddress; // parameter data for converting address


	SonyPdLibInputData_t  m_SonyPdLibInputData;
	SonyPdLibOutputData_t m_SonyPdLibOutputData;

	unsigned short m_CurrMode;

	MINT32 SetRegData( unsigned short &CurrMode); //use in PDAFTransCoord
	MBOOL TransROICoord(SPDROI_T &srcROI, SPDROI_T &dstROI);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    PD_IMX230MIPIRAW();
    ~PD_IMX230MIPIRAW();

	static PDBufMgrOpen* getInstance();


	//Inherit
	MINT32 GetPDCalSz();

	MBOOL GetPDInfo2HybridAF( MINT32 i4InArySz, MINT32 *i4OutAry);

	MRESULT GetVersionOfPdafLibrary( SPDLibVersion_t &tOutSWVer);

    MBOOL GetDefocus( SPDROIInput_T &iPDInputData, SPDROIResult_T &oPdOutputData); 

};
#endif // _PD_IMX230MIPIRAW_H_

