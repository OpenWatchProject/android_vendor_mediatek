#include "wakelock.h"
#include <stdio.h>

static List_t xWakelocksList;
static wakelock_state_t g_wakelock_state = WAKELOCK_STATE_NORMAL;

void wakelocks_init(void)
{
    vListInitialise(&xWakelocksList);
}

void wake_lock_init(wakelock_t *lock, char *name)
{
    UBaseType_t x;
    configASSERT(lock);                                                         /* Ensure that the lock is valid. */
    for (x = (UBaseType_t)0; x < (UBaseType_t)configMAX_LOCK_NAME_LEN; x++) {   /* Store the task name in the lock. */
        lock->pcLockName[x] = name[x];
        /* Don't copy all configMAX_LOCK_NAME_LEN if the string is shorter than
        configMAX_LOCK_NAME_LEN characters just in case the memory after the
        string is not accessible (extremely unlikely). */
        if (name[x] == 0x00) {
            break;
        }
    }
    vListInitialiseItem(&(lock->xGenericListItem));                             /* Initialize lock structure. Make sure the item is not on a list. */
    listSET_LIST_ITEM_OWNER(&(lock->xGenericListItem), lock);
}

BaseType_t wake_lock(wakelock_t *lock)
{
    configASSERT(lock);                                                         /* Ensure the lock is valid*/
    taskENTER_CRITICAL();                                                       /* Enter critical section to protect xWakelocksList from other task/isr accessing. */
    if (listLIST_ITEM_CONTAINER(&(lock->xGenericListItem)) != NULL) {
        PRINTF_W("[%s] lock re-entry by %s @ %x\n", __func__, lock->pcLockName, (unsigned int)lock);    /* The lock is on the list. Print debug log. */
    } else {
        PRINTF_D("[%s] lock by %s\n", __func__, lock->pcLockName);
        vListInsertEnd(&xWakelocksList, &(lock->xGenericListItem));             /* The lock is not on the list. */
    }
    taskEXIT_CRITICAL();                                                        /* Leave critical section. */
    return pdPASS;
}

BaseType_t wake_lock_FromISR(wakelock_t *lock)
{
    configASSERT(lock);                                                         /* Ensure that the lock is valid. */
    if (listLIST_ITEM_CONTAINER(&(lock->xGenericListItem)) != NULL) {           /* It is in ISR. No need to protect xWakelocksList from other task accessing. */
        PRINTF_W("[%s] lock re-entry by %s @ %x\n", __func__, lock->pcLockName, (unsigned int)lock);    /* The lock is on the list. Print debug log. */
    } else {
        PRINTF_D("[%s] lock by %s\n", __func__, lock->pcLockName);
        vListInsertEnd(&xWakelocksList, &(lock->xGenericListItem));             /* The lock is not on the list. */
    }
    return pdPASS;
}

BaseType_t wake_unlock_FromISR(wakelock_t *lock)
{
    configASSERT(lock);                                                             /* Ensure that the lock is valid. */
    if (listLIST_ITEM_CONTAINER(&(lock->xGenericListItem)) != NULL) {               /* It is in ISR. No need to protect xWakelocksList from other task accessing. */
        PRINTF_D("[%s] unlock by %s\n", __func__, lock->pcLockName);
        (void) uxListRemove(&(lock->xGenericListItem));                             /* The lock is on the list. */
    } else {
        PRINTF_W("[%s] unlock fail by %s @ %x\n", __func__, lock->pcLockName, (unsigned int)lock);  /* The lock is not on the list. Print debug log. */
    }
    return pdPASS;
}

BaseType_t wake_unlock(wakelock_t *lock)
{
    configASSERT(lock);                                                             /* Ensure that the lock is valid. */
    taskENTER_CRITICAL();                                                           /* Enter critical section to protect xWakelocksList from other task/isr accessing. */
    if (listLIST_ITEM_CONTAINER(&(lock->xGenericListItem)) != NULL) {
        PRINTF_D("[%s] unlock by %s\n", __func__, lock->pcLockName);
        (void) uxListRemove(&(lock->xGenericListItem));                             /* The lock is on the list. */
    } else {
        PRINTF_W("[%s] unlock fail by %s @ %x\n", __func__, lock->pcLockName, (unsigned int)lock);  /* The lock is not on the list. Print debug log. */
    }
    taskEXIT_CRITICAL();                                                            /* Leave critical section. */
    return pdPASS;
}

BaseType_t wake_lock_is_empty(void)
{
    wakelock_state_t state = g_wakelock_state;
    if (state == WAKELOCK_STATE_FORCE_LOCK) {
        return pdFALSE;
    } else if (state == WAKELOCK_STATE_FORCE_UNLOCK) {
        return pdTRUE;
    } else {
        if (listLIST_IS_EMPTY(&xWakelocksList) != pdFALSE) {
            return pdTRUE;
        } else {
            return pdFALSE;
        }
    }
}

void wake_lock_list_dump(void)
{
    volatile wakelock_t *pxNextLock;
    volatile wakelock_t *pxFirstLock;
    List_t *pxList = &xWakelocksList;
    PRINTF_D("wakelock state %d\n", g_wakelock_state);
    if (listCURRENT_LIST_LENGTH(pxList) > (UBaseType_t) 0) {
        listGET_OWNER_OF_NEXT_ENTRY(pxFirstLock, pxList);
        do {
            listGET_OWNER_OF_NEXT_ENTRY(pxNextLock, pxList);
            PRINTF_D("wakelock:%s \n", pxNextLock->pcLockName);
        } while (pxFirstLock != pxNextLock);
    }
}

void wake_lock_set_state(wakelock_state_t state)
{
    switch (state) {
        case WAKELOCK_STATE_NORMAL:
        case WAKELOCK_STATE_FORCE_LOCK:
        case WAKELOCK_STATE_FORCE_UNLOCK:
            g_wakelock_state = state;
            PRINTF_D("wakelock set state %d\n", state);
            break;
        default:
            PRINTF_W("wakelock invalid state %d\n", state);
            break;
    }
}

wakelock_state_t wake_lock_get_state(void)
{
    return g_wakelock_state;
}
