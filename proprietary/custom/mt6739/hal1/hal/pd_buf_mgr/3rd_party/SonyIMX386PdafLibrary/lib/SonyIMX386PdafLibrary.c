#include "SonyIMX386PdafLibrary.h"
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void SonyPdLibGetVersion( SonyPdLibVersion_t    *pfa_SonyPdLibVersion)
{
    memset( pfa_SonyPdLibVersion, 0, sizeof(SonyPdLibVersion_t));
}

void SonyPdLibGetImagerRegDataOnPdaf( unsigned long fa_SetTiming, unsigned long *pfa_DataNum, unsigned long *pfa_Address, unsigned long *pfa_Data)
{

}

signed long SonyPdLibGetDefocus( SonyPdLibInputData_t    *pfa_SonyPdLibInputData, SonyPdLibOutputData_t    *pfa_SonyPdLibOutputData)
{
    memset( pfa_SonyPdLibOutputData, 0, sizeof(SonyPdLibOutputData_t));
    return -1;
}

#ifdef __cplusplus
}
#endif            /* __cplusplus */


