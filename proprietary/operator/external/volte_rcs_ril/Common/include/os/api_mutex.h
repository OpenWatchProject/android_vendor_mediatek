#ifndef MUTEX0901_DEF
#define MUTEX0901_DEF

#define MUTX_MAX_SIZE         40

typedef int cwait_t;

enum {WAIT_NOWAIT = 0, WAIT_FOREVER = -1};

/**
 *	@brief returns the cmutex_t struct size
 */
unsigned MUTX_Size(void);

typedef struct cmutex cmutex_t;

/**
 *    @brief mutex initialization function
 */
int MUTX_Init(cmutex_t* o, char* name);

/**
 *    @brief mutex destroy function
 */
int MUTX_Dest(cmutex_t* o);

/**
 *    @brief mutex acquire function
 */
int MUTX_Get(cmutex_t* o, cwait_t wait);

/**
 *    @brief mutex release function
 */
int MUTX_Put(cmutex_t* o);

/**
 *    @brief mutex control function
 */
int MUTX_Ctrl(cmutex_t* o);
#endif

