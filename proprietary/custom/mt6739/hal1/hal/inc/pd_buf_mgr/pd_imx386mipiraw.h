#ifndef _PD_IMX386MIPIRAW_H_
#define _PD_IMX386MIPIRAW_H_

#include <log/log.h>
#include <fcntl.h>
#include <math.h>

#include "pd_buf_mgr_open.h"
#include "SonyIMX386PdafLibraryWrapper.h"




class PD_IMX386MIPIRAW : protected PDBufMgrOpen
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:

    SonyIMX386PdafLibraryWrapper *pLibWrapper;

    MBOOL m_bDebugEnable;

    MUINT8 m_CurrMode;

    static MUINT32 m_u4Reg_AdDaPair_Sz;

    static MUINT16 m_u2Reg_AdDaPair[82];

protected :
    MBOOL IsSupport( SPDProfile_t &iPdProfile);
    MBOOL ExtractPDCL();
    MBOOL ExtractCaliData();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    PD_IMX386MIPIRAW();
    ~PD_IMX386MIPIRAW();

    static PDBufMgrOpen* getInstance();

    //Inherit
    MINT32 GetPDCalSz();

    MBOOL GetPDInfo2HybridAF( MINT32 i4InArySz, MINT32 *i4OutAry);

    MRESULT GetVersionOfPdafLibrary( SPDLibVersion_t &tOutSWVer);

    MBOOL GetDefocus( SPDROIInput_T &iPDInputData, SPDROIResult_T &oPdOutputData);

    MBOOL sendCommand( MUINT32  i4Cmd, MVOID* arg1=NULL, MVOID* arg2=NULL, MVOID* arg3=NULL);

};
#endif // _PD_IMX386MIPIRAW_H_

