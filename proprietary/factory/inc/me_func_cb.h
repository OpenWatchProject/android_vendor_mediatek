#ifndef _ME_GLBDEF_CB_H
#define _ME_GLBDEF_CB_H

typedef void (*PARSERFUNC) (void *pfsm, void *pObj);
int ME_URC_Parser(void *pData);

void ME_Set_ModemFunc_CB(void *pData);
void ME_Query_SIMStatus_CB(void *pData);
void ME_Query_IMSI_CB(void *pData);
void ME_Query_FWVersion_CB(void *pData);
void ME_Query_Revision_CB(void *pData);
void ME_Query_Signal_CB(void *pData);
void ME_Query_3G_Status_CB(void *pData);

#endif