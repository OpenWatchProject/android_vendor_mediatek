
#ifndef __PTIMER_H__
#define __PTIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*** STANDARD INCLUDES *******************************************************/


/*** PROJECT INCLUDES ********************************************************/


/*** MACROS ******************************************************************/


/*** GLOBAL TYPES DEFINITIONS ************************************************/


/*** PRIVATE TYPES DEFINITIONS ***********************************************/


/*** GLOBAL VARIABLE DECLARATIONS (EXTERN) ***********************************/


/*** PUBLIC FUNCTION PROTOTYPES **********************************************/
int ptimer_mng_create(void ** ppmng);
int ptimer_mng_delete(void * pmng);

int ptimer_mng_getnextduration(void * pmng, unsigned long * mseconds);
int ptimer_mng_getexpired(void * pmng, void ** pptimer, void ** ppdata, unsigned long * pWallSec, unsigned long * pWallNSec);

int ptimer_create(void ** pptimer);
int ptimer_start(void * pmng, void * ptimer, unsigned long mseconds, void * pdata);
int ptimer_stop(void * ptimer);
int ptimer_delete(void * ptimer);

int ptimer_util_walltimepass_ms(unsigned long OldWallSec, unsigned long OldWallNSec, unsigned long * pmseconds);

#ifdef __cplusplus
}
#endif

#endif /* End of #ifndef __PTIMER_H__ */


