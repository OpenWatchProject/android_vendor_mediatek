#ifndef _SYS_UNISTD_H
#define _SYS_UNISTD_H

#ifdef __cplusplus
extern "C" {
#endif


#define _SC_ARG_MAX                       0
#define _SC_CHILD_MAX                     1
#define _SC_CLK_TCK                       2
#define _SC_NGROUPS_MAX                   3
#define _SC_OPEN_MAX                      4
#define _SC_JOB_CONTROL                   5
#define _SC_SAVED_IDS                     6
#define _SC_VERSION                       7
#define _SC_PAGESIZE                      8
#define _SC_PAGE_SIZE                     _SC_PAGESIZE
/* These are non-POSIX values we accidentally introduced in 2000 without
   guarding them.  Keeping them unguarded for backward compatibility. */
#define _SC_NPROCESSORS_CONF              9
#define _SC_NPROCESSORS_ONLN             10
#define _SC_PHYS_PAGES                   11
#define _SC_AVPHYS_PAGES                 12
/* End of non-POSIX values. */
#define _SC_MQ_OPEN_MAX                  13
#define _SC_MQ_PRIO_MAX                  14
#define _SC_RTSIG_MAX                    15
#define _SC_SEM_NSEMS_MAX                16
#define _SC_SEM_VALUE_MAX                17
#define _SC_SIGQUEUE_MAX                 18
#define _SC_TIMER_MAX                    19
#define _SC_TZNAME_MAX                   20
#define _SC_ASYNCHRONOUS_IO              21
#define _SC_FSYNC                        22
#define _SC_MAPPED_FILES                 23
#define _SC_MEMLOCK                      24
#define _SC_MEMLOCK_RANGE                25
#define _SC_MEMORY_PROTECTION            26
#define _SC_MESSAGE_PASSING              27
#define _SC_PRIORITIZED_IO               28


#ifdef __cplusplus
}
#endif
#endif /* _SYS_UNISTD_H */

