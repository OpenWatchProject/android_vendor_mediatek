#ifndef _PD_IMX230MIPIRAW_H_
#define _PD_IMX230MIPIRAW_H_

#include <log/log.h>
#include <fcntl.h>
#include <math.h>

#include "pd_buf_mgr_open.h"

#include <SonyIMX230PdafLibraryWrapper.h>


class PD_IMX230MIPIRAW : protected PDBufMgrOpen
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private :

    SonyIMX230PdafLibraryWrapper *pLibWrapper;

protected :

    MBOOL IsSupport( SPDProfile_t &iPdProfile);
    MBOOL ExtractPDCL();
    MBOOL ExtractCaliData();

    MBOOL TransROICoord(SPDROI_T &srcROI, SPDROI_T &dstROI);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public :
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

