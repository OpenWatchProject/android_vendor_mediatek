#ifndef __DIP_AEE_H__
#define __DIP_AEE_H__


#if defined(HAVE_AEE_FEATURE)
//add MOF to match CRDISPATCH_KEY:
#include <aee.h>
#define AEE_ASSERT(String) \
    do { \
        aee_system_exception( \
            "mtkcam/Dip", \
            NULL, \
            DB_OPT_DEFAULT|DB_OPT_NATIVE_BACKTRACE|DB_OPT_PROCESS_COREDUMP|DB_OPT_PROC_MEM|DB_OPT_NE_JBT_TRACES|DB_OPT_PID_SMAPS|DB_OPT_DUMPSYS_PROCSTATS|DB_OPT_DUMP_DISPLAY, \
            String); \
    } while(0)
//raise(SIGKILL);
#else
#define AEE_ASSERT(String)
#endif

#define ABORT() \
    do { \
        abort(); \
    } while(0)


#endif /*__DIP_AEE_H__*/