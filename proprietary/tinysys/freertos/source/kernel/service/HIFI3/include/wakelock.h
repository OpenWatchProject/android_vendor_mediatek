#ifndef __WAKELOCK_H__
#define __WAKELOCK_H__

#include <FreeRTOS.h>
#include <task.h>

#define configMAX_LOCK_NAME_LEN 5

typedef struct {
    char pcLockName[configMAX_TASK_NAME_LEN];
    ListItem_t xGenericListItem;
} wakelock_t;

typedef enum {
    WAKELOCK_STATE_NORMAL       = 0,
    WAKELOCK_STATE_FORCE_LOCK   = 1,
    WAKELOCK_STATE_FORCE_UNLOCK = 2
} wakelock_state_t;

void wakelocks_init(void);
void wake_lock_init(wakelock_t *lock, char *name);
BaseType_t wake_unlock(wakelock_t *lock);
BaseType_t wake_unlock_FromISR(wakelock_t *lock);
BaseType_t wake_lock(wakelock_t *lock);
BaseType_t wake_lock_FromISR(wakelock_t *lock);
void wake_lock_list_dump(void);
BaseType_t wake_lock_is_empty(void);
void wake_lock_set_state(wakelock_state_t state);
wakelock_state_t wake_lock_get_state(void);

#endif  /* __WAKELOCK_H__ */
