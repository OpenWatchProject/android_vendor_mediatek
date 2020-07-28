#ifndef _ME_AT_PARSER_H
#define _ME_AT_PARSER_H

//AT parser
void ME_Query_FWVersion_Parser(void *pfsm, void *pObj);
void ME_Query_SIMStatus_Parser(void *pfsm, void *pObj);
void ME_Query_IMEI_Parser(void *pfsm, void *pObj);
void ME_Query_IMSI_Parser(void *pfsm, void *pObj);
void ME_Query_Revision_Parser(void *pfsm, void *pObj);
void ME_Query_Signal_Parser(void *pfsm, void *pObj);
void ME_Query_3G_Status_Parser(void *pfsm, void *pObj);

//URC parser
int ME_URC_ESIMS_Parser(void *pData);
int ME_URC_CREG_Parser(void *pData);
int ME_URC_ECSQ_Parser(void *pData);
int ME_URC_CGREG_Parser(void *pData);
int ME_URC_EIND_Parser(void *pData);
int ME_URC_RING_Parser(void *pData);
int ME_URC_PSBEARER_Parser(void* pData);
int ME_URC_ECPI_Parser(void *pData);
int ME_URC_ESPEECH_Parser(void* pData);
int ME_URC_VPUP_Parser(void* pData);
int ME_URC_CONN_Parser(void* pData);
int ME_URC_VPON_Parser(void* pData);
int ME_URC_EUSIM_Parser(void *pData);
int ME_URC_UIMST_Parser(void *pData);
#endif