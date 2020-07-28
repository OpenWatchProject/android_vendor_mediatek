
#ifndef __PERFSERVD_CORE_H__
#define __PERFSERVD_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*** STANDARD INCLUDES *******************************************************/


/*** PROJECT INCLUDES ********************************************************/


/*** MACROS ******************************************************************/
#define PS_CLUSTER_MAX 8


/*** GLOBAL TYPES DEFINITIONS ************************************************/
#if 0
struct tPowerData
{
   int scn_core_min[PS_CLUSTER_MAX];
   int scn_core_max[PS_CLUSTER_MAX];

   int scn_vcore;

   int scn_freq_min[PS_CLUSTER_MAX];
   int scn_freq_max[PS_CLUSTER_MAX];

   int scn_gpu_freq_min[PS_CLUSTER_MAX];
   int scn_gpu_freq_max[PS_CLUSTER_MAX];

   void * pOtherCmd;
};
#endif


/*** PRIVATE TYPES DEFINITIONS ***********************************************/


/*** GLOBAL VARIABLE DECLARATIONS (EXTERN) ***********************************/


/*** PUBLIC FUNCTION PROTOTYPES **********************************************/
int powerd_core_init(void * pTimerMng);
int powerd_core_timer_handle(void * pTimer, void * pData);
int powerd_core_init(void * pTimerMng);

//long powerd_lock_aquire(unsigned long handle, int duration, struct tPowerData * pScnData);
//long powerd_lock_rel(unsigned long handle);

int powerd_req(void * pMsg, void ** pRspMsg);


#ifdef __cplusplus
}
#endif

#endif /* End of #ifndef __PERFSERVD_CORE_H__ */

