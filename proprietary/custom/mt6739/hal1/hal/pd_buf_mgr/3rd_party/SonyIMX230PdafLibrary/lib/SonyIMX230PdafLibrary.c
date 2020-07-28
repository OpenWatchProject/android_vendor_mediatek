#include "SonyIMX230PdafLibrary.h"
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UNUSED(param) do{(void)(param); }while( 0 )

void SonyPdLibGetVersion( SonyPdLibVersion_t* pfa_SonyPdLibVersion)
{
    memset( pfa_SonyPdLibVersion, 0, sizeof(SonyPdLibVersion_t));
}

void SonyPdLibGetImagerRegDataOnPdaf( unsigned long fa_SetTiming, unsigned long* pfa_DataNum, unsigned long* pfa_Address, unsigned long* pfa_Data)
{
    UNUSED(fa_SetTiming);
    UNUSED(pfa_DataNum);
    UNUSED(pfa_Address);
    UNUSED(pfa_Data);
}

signed long SonyPdLibGetDefocus( SonyPdLibInputData_t* pfa_SonyPdLibInputData, SonyPdLibOutputData_t* pfa_SonyPdLibOutputData)
{
    UNUSED(pfa_SonyPdLibInputData);
    memset( pfa_SonyPdLibOutputData, 0, sizeof(SonyPdLibOutputData_t));
    return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */


