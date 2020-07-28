#ifndef __SONY_IMX230_PDAF_LIBRARY_WRAPPER_H__
#define __SONY_IMX230_PDAF_LIBRARY_WRAPPER_H__

#include <log/log.h>
#include <fcntl.h>
#include <math.h>
#include <string>

#include "pd_buf_common.h"

using namespace std;

struct _SonyPdLibSensorCoordSetting_t;
struct _SonyPdLibInputData_t;
struct _SonyPdLibOutputData_t;


class SonyIMX230PdafLibraryWrapper
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.

    MBOOL m_bDebugEnable;

    // setting for 3rd party library.
    unsigned short m_CurrMode;
    unsigned short m_XKnotNum1;
    unsigned short m_YKnotNum1;
    unsigned short m_XKnotNum2;
    unsigned short m_YKnotNum2;
    unsigned short m_PointNumForThrLine;

    // pdcl data and clibration data buffer
    MUINT16  *m_phase_difference;
    MUINT16  *m_confidence_level;
    MUINT16  *m_calibration_data;

protected :

    /* to dynamic loading 3rd party shared library */
    MBOOL m_bLdDn;
    void *m_Libhandle;      // handle of shared library.
    void *m_pfunGetVersion; // function pointer
    void *m_pfunGetImagerRegDataOnPdaf; // function pointer
    void *m_pfunGetDefocus; // function pointer

    int m_IsCfgCoordSetting;

    _SonyPdLibSensorCoordSetting_t *m_ParamDataForConvertingAddress; // parameter data for converting address

    _SonyPdLibInputData_t  *m_SonyPdLibInputData;

    _SonyPdLibOutputData_t *m_SonyPdLibOutputData;

    MINT32 SetRegData( unsigned short &CurrMode); //use in PDAFTransCoord
    MBOOL TransROICoord(SPDROI_T &srcROI, SPDROI_T &dstROI);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    SonyIMX230PdafLibraryWrapper();

    ~SonyIMX230PdafLibraryWrapper();

    MBOOL IsSupport( SPDProfile_t &iPdProfile);

    MBOOL ExtractCaliData( MUINT32 u4CaliDaSz, MUINT8 *pCaliDaBuf);

    MBOOL ExtractPDCL( MUINT32 u4DaSz, MUINT8 *pInDaBuf);

    MINT32 GetPDCalSz();

    MBOOL GetPDInfo2HybridAF( MINT32 i4InArySz, MINT32 *i4OutAry);

    MRESULT GetVersionOfPdafLibrary( SPDLibVersion_t &tOutSWVer);

    MBOOL GetDefocus( SPDROIInput_T &iPDInputData, SPDROIResult_T &oPdOutputData);

};
#endif // __SONY_IMX230_PDAF_LIBRARY_WRAPPER_H__

