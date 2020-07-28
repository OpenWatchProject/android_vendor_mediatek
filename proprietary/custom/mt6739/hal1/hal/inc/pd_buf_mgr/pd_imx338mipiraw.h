#ifndef _PD_IMX338MIPIRAW_H_
#define _PD_IMX338MIPIRAW_H_

#include <log/log.h>
#include <fcntl.h>
#include <math.h>

#include "pd_buf_mgr_open.h"
#include "SonyIMX338PdafLibraryWrapper.h"

//#define USE_FLEXIBLE_WIN_CFG

class PD_IMX338MIPIRAW : protected PDBufMgrOpen
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:

    SonyIMX338PdafLibraryWrapper *pLibWrapper;

    MBOOL m_bDebugEnable;

    MUINT8 m_CurrMode;

    SPDProfile_t m_tPDProfile;

    MINT32  m_WinMode;

    MUINT32 m_WinEn;

    static MUINT32 m_u4Reg_AdDaPair_Sz;

    static MUINT16 m_u2Reg_AdDaPair[82];

    static MUINT32 m_u4Reg_AdDaPair_PDWin_Flexible_Sz;

    static MUINT16 m_u2Reg_AdDaPair_PDWin_Flexible[146];

    static MUINT32 m_u4Reg_AdDaPair_PDWin_Fix_Sz;

    static MUINT16 m_u2Reg_AdDaPair_PDWin_Fix[34];

protected :

    MBOOL IsSupport( SPDProfile_t &iPdProfile);

    MBOOL ExtractPDCL();

    MBOOL ExtractCaliData();

    MBOOL GetPDWinSetting( MVOID *arg1, MVOID *arg2, MVOID *arg3);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    PD_IMX338MIPIRAW();
    ~PD_IMX338MIPIRAW();

    static PDBufMgrOpen* getInstance();

    //Inherit
    MINT32 GetPDCalSz();

    MBOOL GetPDInfo2HybridAF( MINT32 i4InArySz, MINT32 *i4OutAry);

    MRESULT GetVersionOfPdafLibrary( SPDLibVersion_t &tOutSWVer);

    MBOOL GetDefocus( SPDROIInput_T &iPDInputData, SPDROIResult_T &oPdOutputData);

    MBOOL sendCommand( MUINT32  i4Cmd, MVOID* arg1=NULL, MVOID* arg2=NULL, MVOID* arg3=NULL);

};
#endif // _PD_IMX338MIPIRAW_H_

