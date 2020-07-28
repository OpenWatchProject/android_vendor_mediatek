
#ifndef __PERFSERVD_CMD_H__
#define __PERFSERVD_CMD_H__

#ifdef __cplusplus
extern "C" {
#endif


/*** STANDARD INCLUDES *******************************************************/


/*** PROJECT INCLUDES ********************************************************/


/*** MACROS ******************************************************************/
#define PS_SCN_TYPE_BASE        0x0000
#define PS_SCN_TYPE_MEMORY      (PS_SCN_TYPE_BASE + 1)


#if 0
#define PS_SCN_TYPE_DURATION        (PS_SCN_TYPE_BASE + 2)
#define PS_SCN_TYPE_CORE_MIN        (PS_SCN_TYPE_BASE + 3)
#define PS_SCN_TYPE_CORE_MAX        (PS_SCN_TYPE_BASE + 4)
#define PS_SCN_TYPE_VCORE           (PS_SCN_TYPE_BASE + 5)
#define PS_SCN_TYPE_FREQ_MIN        (PS_SCN_TYPE_BASE + 6)
#define PS_SCN_TYPE_FREQ_MAX        (PS_SCN_TYPE_BASE + 7)
#define PS_SCN_TYPE_GPU_FREQ_MIN    (PS_SCN_TYPE_BASE + 8)
#define PS_SCN_TYPE_GPU_FREQ_MAX    PS_SCN_TYPE_BASE + 9)
#define PS_SCN_TYPE_END             (PS_SCN_TYPE_BASE + 10)
#endif


/*** GLOBAL TYPES DEFINITIONS ************************************************/


/*** PRIVATE TYPES DEFINITIONS ***********************************************/


/*** GLOBAL VARIABLE DECLARATIONS (EXTERN) ***********************************/


/*** PUBLIC FUNCTION PROTOTYPES **********************************************/
int powerd_cmd_create(void ** ppcmd);
int powerd_cmd_destory(void * pcmd);


#ifdef __cplusplus
}
#endif

#endif /* End of #ifndef __PERFSERVD_CMD_H__ */

