/****************************************************************/
/*                        Dummy library                         */
/****************************************************************/

#include "SonyIMX519PdafLibrary.h"

/****************************************************************/
/*                          version                             */
/****************************************************************/

#define D_MAJOR_VERSION (00)
#define D_MINOR_VERSION (00)

/****************************************************************/
/*                      external function                       */
/****************************************************************/
/* API : Get version information of PDAF Library. */
extern void PdLibGetVersion
(
    PdLibVersion_t  *pfa_PdLibVersion
)
{
    (*pfa_PdLibVersion).MajorVersion = D_MAJOR_VERSION;
    (*pfa_PdLibVersion).MinorVersion = D_MINOR_VERSION;

    return ;
}

/* API : Get defocus data according to a PDAF window. */
extern signed long PdLibGetDefocus
(
    PdLibInputData_t    *pfa_PdLibInputData,
    PdLibOutputData_t   *pfa_PdLibOutputData
)
{
    pfa_PdLibOutputData->Defocus = 0;
    pfa_PdLibOutputData->DefocusConfidence = 0;
    pfa_PdLibOutputData->DefocusConfidenceLevel = 0;
    pfa_PdLibOutputData->PhaseDifference = 0;
    return -1;
}

