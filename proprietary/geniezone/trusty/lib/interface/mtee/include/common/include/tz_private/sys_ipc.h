/*
* Copyright (c) 2015 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __MTEE_SYSTEM_IPC_H__
#define __MTEE_SYSTEM_IPC_H__

#include "tz_cross/trustzone.h"
#include "tz_private/system.h"
#include <stddef.h>
#ifndef __NEBULA_HEE__
#include <kernel/spinlock.h>
#include <kernel/mutex.h>
#include <kernel/event.h>
#include <kernel/semaphore.h>
#endif
#include <tz_private/log.h>

//#define MTEE_LOCK_DEBUG

/**
 * Spinlock define
 *
 * @param lock    locker.
 */
typedef uint32_t MTEE_SPINLOCK;

/**
 * Mutex define
 *
 * @param mutex    mutex id.
 */
typedef uint32_t MTEE_MUTEX;

/**
 * Semaphore define
 *
 * @param semaphore    semaphore id.
 */
typedef uint32_t MTEE_SEMAPHORE;

/**
 * Wait queue define 
 *
 * @param waitqueue    waitqueue id.
 */
typedef uint32_t MTEE_WAITQUEUE;	/*fix mtee sync*/

/**
 * Event define
 */
typedef uint32_t MTEE_EVENT;

#define MTEE_EVENT_AUTOUNSIGNAL 1

/**
 * Init spinlock, used in code
 *
 * @param lock The lock to init
 * @param init_lock Init lock state.
 */
/*fix mtee sync*/
static inline void MTEE_InitSpinlock(MTEE_SPINLOCK *lock, int init_lock)	/*fix mtee sync*/
{
#if 0
    lock->lock = (init_lock) ? 1 : 0;
#ifdef MTEE_LOCK_DEBUG
    lock->owner_stack = lock->owner_pc = 0;
#endif

#endif
	print_mtee_orig_msg;
	return; //not support
}

/**
 * Check if the spin lock is locked.
 *
 * @param lock The lock to check
 */
/*fix mtee sync*/
static inline int MTEE_SpinIsLocked(MTEE_SPINLOCK *lock)	/*fix mtee sync*/
{
#if 0
    return lock->lock != 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}

#ifndef __aarch64__
/*
    __MTEE_SpinLock()/__MTEE_SpinUnlock() are internally used
    TA should not use those function except you know there is no priority inversion cases
    Example:
      A kernel thread with low priority get the spinlock.
      B/C/D kernel threads with real time priority cannot get the spin lock but occupy the CPU forever
*/
/**
 * Grab the spin lock
 *
 * @param lock    The spin lock to take.
 */
/*fix mtee sync*/
static inline void __MTEE_SpinLock(MTEE_SPINLOCK *lock)
{
#if 0
    unsigned int tmp;

    __asm__ volatile(
        "1:    ldrex    %0, [%1]\n"
        "      teq      %0, #0\n"
        "      iteet    ne\n"
        "      wfene    \n"
        "      strexeq  %0, %2, [%1]\n"
        "      teqeq    %0, #0\n"
        "      bne      1b\n"
        "      dmb      \n"
        : "=&r" (tmp)
        : "r" (&lock->lock), "r" (1)
        : "cc", "memory");
#ifdef MTEE_LOCK_DEBUG
{
    register unsigned long sp asm ("sp");
    lock->owner_stack = (void*)sp;
here:
    lock->owner_pc = (void*)&&here;
}
#endif
#endif
	print_mtee_orig_msg;
	return; //not support
}

/**
 * Unlock spin lock
 *
 * @param lock    The spin lock to unlock
 */
/*fix mtee sync*/
static inline void __MTEE_SpinUnlock(MTEE_SPINLOCK *lock)
{
#if 0
#ifdef MTEE_LOCK_DEBUG
    lock->owner_stack = lock->owner_pc = 0;
#endif
    __asm__ volatile(
        "      dmb      \n"
        "      str      %1, [%0]\n"
        "      dsb      \n"
        "      sev      \n"
        :
        : "r" (&lock->lock), "r" (0)
        : "memory");
#endif
	print_mtee_orig_msg;
	return; //not support
}

#else // defined(__aarch64__)

/*
    __MTEE_SpinLock()/__MTEE_SpinUnlock() are internally used
    TA should not use those function except you know there is no priority inversion cases
    Example:
      A kernel thread with low priority get the spinlock.
      B/C/D kernel threads with real time priority cannot get the spin lock but occupy the CPU forever
*/
/**
 * Grab the spin lock
 *
 * @param lock    The spin lock to take.
 */
/*fix mtee sync*/
static inline void __MTEE_SpinLock(MTEE_SPINLOCK *lock)
{
#if 0
    unsigned int tmp;

    __asm__ volatile(
        "	sevl\n"
        "1:	wfe\n"
        "2:	ldaxr   %w0, %1\n"
        "cbnz    %w0, 1b\n"
        "stxr    %w0, %w2, %1\n"
        "cbnz    %w0, 2b\n"

        : "=&r" (tmp), "+Q" (lock->lock)
        : "r" (1)
        : "cc", "memory");
#ifdef MTEE_LOCK_DEBUG
{
    register unsigned long sp asm ("sp");
    lock->owner_stack = (void*)sp;
here:
    lock->owner_pc = (void*)&&here;
}
#endif
#endif
	print_mtee_orig_msg;
	return; //not support
}

/**
 * Unlock spin lock
 *
 * @param lock    The spin lock to unlock
 */
/*fix mtee sync*/
static inline void __MTEE_SpinUnlock(MTEE_SPINLOCK *lock)
{
#if 0
#ifdef MTEE_LOCK_DEBUG
    lock->owner_stack = lock->owner_pc = 0;
#endif

    __asm__ volatile(
	"      stlr    wzr, %0\n"
        : "=Q" (lock->lock) : : "memory");
#endif
	print_mtee_orig_msg;
	return; //not support
}



#endif // defined(__aarch64__)

/**
 * Grab the spin lock and mask IRQ/FIQ
 *
 * @param lock    The spin lock to take.
 * @param state    Previous IRQ/FIQ state to restore to.
 */
static inline uint32_t MTEE_SpinLockMaskIrq(MTEE_SPINLOCK *lock)	/*fix mtee sync*/
{
#if 0
    uint32_t state = MTEE_MaskIrq();
    __MTEE_SpinLock(lock);
    return state;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/**
 * Unlock spin lock and restore IRQ to previous state.
 *
 * @param lock    The spin lock to unlock
 * @param state    Previous IRQ state to restore to.
 */
static inline void MTEE_SpinUnlockRestoreIrq(MTEE_SPINLOCK *lock, uint32_t state)	/*fix mtee sync*/
{
#if 0
    __MTEE_SpinUnlock(lock);
    MTEE_RestoreIrq(state);
#endif
	print_mtee_orig_msg;
	return; //not support
}

/* Mutex
*/
/**
 * Mutex, create
 *
 * @return    A pointer to the the mutex. If fail, return 0.
 */
MTEE_MUTEX *MTEE_CreateMutex (void);

/**
 * Mutex, destory
 *
 * @param mutex    A pointer to the mutex.
 * @return    0 if success. <0 for error.
 */
int MTEE_DestoryMutex (MTEE_MUTEX *mutex);

/**
 * Mutex, lock
 *
 * @param mutex    A pointer to the mutex.
 */
int MTEE_LockMutex (MTEE_MUTEX *mutex);

/**
 * Mutex, unlock
 *
 * @param mutex    A pointer to the mutex.
 */
int MTEE_UnlockMutex (MTEE_MUTEX *mutex);

/**
 * Mutex, try lock
 *
 * @param mutex    A pointer to the mutex.
 * @return    1 if the mutex has been acquired successfully, and 0 on contention.
 */

int MTEE_TryLockMutex (MTEE_MUTEX *mutex);

/**
 * Mutex, querey if it is locked or not
 *
 * @param mutex    A pointer to the mutex.
 * @return    1 if the mutex is locked, 0 if unlocked..
 */
int MTEE_IsLockedMutex (MTEE_MUTEX *mutex);

/* Semaphore
*/
/**
 * Semaphore, create
 *
 * @param val      initial value.
 * @return    A pointer to the the semaphore. If fail, return 0.
 */
MTEE_SEMAPHORE *MTEE_CreateSemaphore (int val);

/**
 * Semaphore, free
 *
 * @param semaphore    A pointer to the semaphore.
 * @return    0 if success. <0 for error.
 */
int MTEE_DestroySemaphore (MTEE_SEMAPHORE *semaphore);

/**
 * Semaphore, down
 *
 * @param semaphore    A pointer to the semaphore.
 */
void MTEE_DownSemaphore (MTEE_SEMAPHORE *semaphore);

/**
 * Semaphore down, and can be interrupted by signal.
 *
 * @param semaphore    A pointer to the semaphore.
 * @return 0 if success. <0 for error.
 */
int MTEE_DownSemaphoreInterruptible (MTEE_SEMAPHORE *semaphore);	/*fix mtee sync*/

/**
 * Semaphore, down with time-out
 *
 * @param semaphore    A pointer to the semaphore.
 * @param timeout    timeout value, in jiffies
 * @return    0 if the @timeout elapsed, and the remaining
 * jiffies if the condition evaluated to true before the timeout elapsed.
 */
int MTEE_DownSemaphore_Timeout (MTEE_SEMAPHORE *semaphore, int timeout);

/**
 * Semaphore, down try lock
 *
 * @param semaphore    A pointer to the semaphore.
 * @return    0 if semaphore is avalibale.
 */
int MTEE_DownTryLockSemaphore (MTEE_SEMAPHORE *semaphore);

/**
 * Semaphore, up
 *
 * @param semaphore    A pointer to the semaphore.
 */
void MTEE_UpSemaphore (MTEE_SEMAPHORE *semaphore);

void MTEE_UpSemaphoreThread(MTEE_SEMAPHORE *semaphore, uint32_t cpu, uint32_t boost_enabled);

#if 0
/* Wait queue
*/
/**
 * Wait queue, create
 *
 * @return    A pointer to the waitqueue. If fail, return 0.
 */
/*fix mtee sync*/
MTEE_WAITQUEUE *MTEE_CreateWaitQueue (void);

/**
 * Wait queue, free
 *
 * @param waitqueue    A pointer to the waitqueue.
 * @return    0 if success. <0 for error.
 */
/*fix mtee sync*/
int MTEE_DestroyWaitQueue (MTEE_WAITQUEUE *waitqueue);

/**
 * Wait queue, wait event
 *
 * @param waitqueue    A pointer to the waitqueue.
 * @condition    The event to wait for
 */
/*fix mtee sync*/
void MTEE_WaitEvent (MTEE_WAITQUEUE *waitqueue, int condition);

/**
 * Wait queue, wait event with timeout
 *
 * @param waitqueue    A pointer to the waitqueue.
 * @condition    The event to wait for
 * @timeout    timeout value, in jiffies
 * @return     0 if the @timeout elapsed, and the remaining
 * jiffies if the condition evaluated to true before the timeout elapsed.
 */
/*fix mtee sync*/
int MTEE_WaitEvent_Timeout (MTEE_WAITQUEUE *waitqueue, int condition, int timeout);

/**
 * Wait queue, wake up
 *
 * @param waitqueue    A pointer to the waitqueue.
 */
/*fix mtee sync*/
void MTEE_WakeUpWaitQueue (MTEE_WAITQUEUE *waitqueue);
#endif

/* Event
*/
/**
 * Event, create
 *
 * @param signaled  Initial value for "signaled", true if != 0
 * @param flags     0 or MTEE_EVENT_AUTOUNSIGNAL
 * @return    A pointer to the event. If fail, return 0.
 */
MTEE_EVENT *MTEE_CreateEvent (int signaled, uint32_t flags);

/**
 * Event, free
 *
 * @param event    A pointer to the event.
 * @return    0 if success. <0 for error.
 */
int MTEE_DestroyEvent (MTEE_EVENT *event);

/**
 * Wait event with timeout
 *
 * @param event    A pointer to the event.
 * @param timeout     timeout value in millisecond
 * @return     0 if success
 */
int MTEE_WaitEvent_Timeout (MTEE_EVENT *event, int timeout);

/**
 * Event, signal
 *
 * @param event    A pointer to the event.
 * @param resched   If != 0, waiting thread(s) are executed immediately.
 *                  Otherwise, waiting threads are placed at the end of the runqueue
 * @return      status, 0 if success.
 */
int MTEE_SignalEvent (MTEE_EVENT *event, int reschedule);


/**
 * Event, unsignal
 *
 * @param event    A pointer to the event.
 */
int MTEE_UnsignalEvent (MTEE_EVENT *event);

#endif /* __MTEE_SYSTEM_IPC_H__ */
