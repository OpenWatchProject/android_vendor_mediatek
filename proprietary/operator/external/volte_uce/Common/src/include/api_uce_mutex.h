#ifndef __LIB_UCE_API_MUTEX_H__
#define __LIB_UCE_API_MUTEX_H__

#define CMUTX_MAX_SIZE     40

typedef int cwait_t;

enum
{WAIT_NOWAIT = 0, WAIT_FOREVER = -1};

/**
 * @brief returns the cmutex_t struct size
 */
unsigned CMUTX_Size(void);

typedef struct cmutex cmutex_t;

/**
 * @brief mutex initialization function
 */
int CMUTX_Init(cmutex_t *o, char *name);

/**
 * @brief mutex destroy function
 */
int CMUTX_Dest(cmutex_t *o);

/**
 * @brief mutex acquire function
 */
int CMUTX_Get(cmutex_t *o, cwait_t wait);

/**
 * @brief mutex release function
 */
int CMUTX_Put(cmutex_t *o);

/**
 * @brief mutex control function
 */
int CMUTX_Ctrl(cmutex_t *o);

#endif /* __LIB_UCE_API_MUTEX_H__ */

