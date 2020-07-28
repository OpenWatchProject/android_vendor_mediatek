#include <stdio.h>
#include <stdlib.h>

#include <tz_private/sys_ipc.h>
#include <tz_private/log.h>


#if 0
/* Wait queue
*/

typedef uint32_t MTEE_WAITQUEUE;

MTEE_WAITQUEUE *MTEE_CreateWaitQueue (void)
{
#if 0
    unsigned long *ptr;

    ptr = MTEE_GetReeParamAddress ();
    MTEE_ReeServiceCall (REE_SERV_WAITQ_CREATE);

    return (MTEE_WAITQUEUE *) *ptr;
#endif
	print_mtee_orig_msg;
	return; //not support
}

int MTEE_DestroyWaitQueue (MTEE_WAITQUEUE *waitqueue)
{
#if 0
    unsigned long *ptr;

    ptr = MTEE_GetReeParamAddress ();
    *ptr = (unsigned long) waitqueue;
    MTEE_ReeServiceCall (REE_SERV_WAITQ_DESTROY);

    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}

void MTEE_WaitEvent (MTEE_WAITQUEUE *waitqueue, int condition)
{
#if 0
    unsigned long *ptr;

    ptr = MTEE_GetReeParamAddress ();
    *ptr = (unsigned long) waitqueue;
    MTEE_ReeServiceCall (REE_SERV_WAITQ_WAIT);
#endif
	print_mtee_orig_msg;
	return; //not support
}

int MTEE_WaitEvent_Timeout (MTEE_WAITQUEUE *waitqueue, int condition, int timeout)
{
#if 0
    unsigned long *ptr;

    ptr = MTEE_GetReeParamAddress ();
    *ptr = (unsigned long) waitqueue;
    MTEE_ReeServiceCall (REE_SERV_WAITQ_WAITTO);

    return (int) *ptr;
#endif
	print_mtee_orig_msg;
	return; //not support
}

void MTEE_WakeUpWaitQueue (MTEE_WAITQUEUE *waitqueue)
{
#if 0
    unsigned long *ptr;

    ptr = MTEE_GetReeParamAddress ();
    *ptr = (unsigned long) waitqueue;
    MTEE_ReeServiceCall (REE_SERV_WAITQ_WAKEUP);
#endif
	print_mtee_orig_msg;
	return; //not support
}
#endif


#if 0
/* Wait queue
*/    
/*fix mtee sync*/
typedef uint32_t KTEE_WAITQUEUE;

/*fix mtee sync*/
KTEE_WAITQUEUE *KTEE_CreateWaitQueue (void)
{
#if 0
    unsigned long *ptr;

    ptr = MTEE_GetReeParamAddress ();
    MTEE_ReeServiceCall (REE_SERV_WAITQ_CREATE);

    return (MTEE_WAITQUEUE *) *ptr;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int KTEE_DestroyWaitQueue (KTEE_WAITQUEUE *waitqueue)
{
#if 0
	unsigned long *ptr;

    ptr = MTEE_GetReeParamAddress ();
    *ptr = (unsigned long) waitqueue;
    MTEE_ReeServiceCall (REE_SERV_WAITQ_DESTROY);

    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void KTEE_WaitEvent (KTEE_WAITQUEUE *waitqueue, int condition)
{
#if 0
    unsigned long *ptr;

    ptr = MTEE_GetReeParamAddress ();
    *ptr = (unsigned long) waitqueue;
    MTEE_ReeServiceCall (REE_SERV_WAITQ_WAIT);
#endif
	print_mtee_orig_msg;
	return; //not support

}

/*fix mtee sync*/
int KTEE_WaitEvent_Timeout (KTEE_WAITQUEUE *waitqueue, int condition, int timeout)
{
#if 0
    unsigned long *ptr;

    ptr = MTEE_GetReeParamAddress ();
    *ptr = (unsigned long) waitqueue;
    MTEE_ReeServiceCall (REE_SERV_WAITQ_WAITTO);

    return (int) *ptr;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void KTEE_WakeUpWaitQueue (KTEE_WAITQUEUE *waitqueue)
{
#if 0
    unsigned long *ptr;

    ptr = MTEE_GetReeParamAddress ();
    *ptr = (unsigned long) waitqueue;
    MTEE_ReeServiceCall (REE_SERV_WAITQ_WAKEUP);
#endif
	print_mtee_orig_msg;
	return; //not support
}
#endif


