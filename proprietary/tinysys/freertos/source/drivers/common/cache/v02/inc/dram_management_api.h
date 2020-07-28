#ifndef __DRAM_MANAGEMENT_API_H__
#define __DRAM_MANAGEMENT_API_H__

#ifdef CFG_CACHE_SUPPORT
void vDmgrDVFSHookDramOn(void);
void vDmgrDVFSHookDramOff(void);
void vDramManagerIdleHook(void);
BaseType_t xTrySwitchOnDramPower(uint32_t ulStack[]);
BaseType_t xDramManagerInit(void);

#else
/* Empty function */
#define vDmgrDVFSHookDramOn()       do {} while(0)
#define vDmgrDVFSHookDramOff()      do {} while(0)
#define vDramManagerIdleHook()      do {} while(0)
#define xDramManagerInit()          (pdFALSE)
#define xTrySwitchOnDramPower(x)    (pdFALSE)
#endif  /* CFG_CACHE_SUPPORT */

#endif  /* __DRAM_MANAGEMENT_API_H__ */
