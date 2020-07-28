#ifndef WAKE_LOCK_H
#define WAKE_LOCK_H

#include "timers.h"
#define configMAX_LOCK_NAME_LEN 5
#define RETRY_TIMEOUT ( ( portTickType) 1000 / portTICK_RATE_MS ) /*timeout 1000 ms*/

typedef struct wakelock {
    char                    pcLockName[ configMAX_TASK_NAME_LEN ];
    ListItem_t              xGenericListItem;
    TickType_t              xExpiredTick;
    TimerHandle_t           xTimer;
} wakelock_t;

void wakelocks_init(void);
void wake_lock_init(wakelock_t *lock, char *name);
void wake_lock_deinit(wakelock_t *lock);
BaseType_t wake_unlock(wakelock_t *lock);
BaseType_t wake_unlock_FromISR(wakelock_t *lock);
BaseType_t wake_lock(wakelock_t *lock);
BaseType_t wake_lock_FromISR(wakelock_t *lock);
BaseType_t wake_lock_timeout(wakelock_t *lock, long timeout);
BaseType_t wake_lock_timeout_FromISR(wakelock_t *lock, long timeout);
void wake_lock_list_dump(void);
BaseType_t wake_lock_is_empty(void);
#endif
