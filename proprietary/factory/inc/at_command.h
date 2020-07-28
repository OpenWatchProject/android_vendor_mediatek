#ifndef FACTORY_COMMAND_H
#define FACTORY_COMMAND_H

#include "me_connection.h"

#define _cplusplus
#ifdef _cplusplus
extern "C" {
#endif
bool FileOp_BackupToBinRegion_All();
#ifdef _cplusplus
}
#endif

extern pthread_mutex_t M_EIND;
extern pthread_cond_t  COND_EIND;
 
extern pthread_mutex_t M_VPUP;
extern pthread_cond_t  COND_VPUP;
 
extern pthread_mutex_t M_CREG;
extern pthread_cond_t  COND_CREG;
 
extern pthread_mutex_t M_ESPEECH_ECPI;
extern pthread_cond_t  COND_ESPEECH_ECPI;
 
extern pthread_mutex_t M_CONN;
extern pthread_cond_t  COND_CONN;

extern pthread_mutex_t M_VPON;
extern pthread_cond_t  COND_VPON;

extern pthread_mutex_t M_EUSIM;
extern pthread_cond_t  COND_EUSIM;
extern pthread_mutex_t M_UIMST;
extern pthread_cond_t  COND_UIMST;

extern int g_Flag_CREG  ;
extern int g_Flag_ESPEECH_ECPI ;
extern int g_Flag_CONN  ;
extern int g_Flag_EIND  ;
extern int g_Flag_VPUP  ;
extern int g_Flag_EUSIM  ;
extern int g_Flag_UIMST  ;

extern void (*g_SIGNAL_Callback[4])(void(*pdata));

void SIGNAL1_Callback(void(*pdata));
void SIGNAL2_Callback(void(*pdata));
void SIGNAL3_Callback(void(*pdata));
void SIGNAL4_Callback(void(*pdata));
void SIGNAL_Callback(void(*pdata));
void g_ATE_Callback(const char *pData, int len);
void deal_URC_ESPEECH_ECPI(int s);
void deal_URC_CREG(int s);          
void deal_URC_CONN(int s);
void deal_URC_EIND(int s);
void deal_URC_VPUP(int s);
void deal_URC_EUSIM(int s);
void deal_URC_UIMST(int s);
int wait_Signal_CREG(int time);

int dial112(Connection& modem);
int dial112C2K(Connection& modem);
void init_COND();
void deinit_COND();

int get_ccci_path(int modem_index,char * path);

int is_support_modem(int modem);

int ExitFlightMode(Connection& modem);

int ExitFlightMode_DualTalk(Connection& modem);

int C2Kmodemsignaltest(Connection& modem);

int getBarcode(Connection& modem, char *result);

int wait_URC(int i);

void handleLPlusGMd1(Connection& modem);


#endif
