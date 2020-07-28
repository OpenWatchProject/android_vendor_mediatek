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
            DB_OPT_DEFAULT|DB_OPT_FTRACE|DB_OPT_NATIVE_BACKTRACE|DB_OPT_PROCESS_COREDUMP|DB_OPT_PROC_MEM|DB_OPT_NE_JBT_TRACES|DB_OPT_PID_SMAPS|DB_OPT_DUMPSYS_PROCSTATS|DB_OPT_DUMP_DISPLAY, \
            String); \
        raise(SIGKILL); \
    } while(0)
    //exit(0); 

#define AEE_ASSERT_DEFAULT(String) \
    do { \
        aee_system_exception( \
            "mtkcam/Dip", \
            NULL, \
            DB_OPT_DEFAULT|DB_OPT_FTRACE, \
            String); \
        raise(SIGKILL); \
    } while(0)

#define AEE_WARNING_ASSERT(String) \
        do { \
            aee_system_warning( \
                "mtkcam/Dip", \
                NULL, \
                DB_OPT_DEFAULT|DB_OPT_FTRACE|DB_OPT_NATIVE_BACKTRACE|DB_OPT_PROCESS_COREDUMP|DB_OPT_PROC_MEM|DB_OPT_NE_JBT_TRACES|DB_OPT_PID_SMAPS|DB_OPT_DUMPSYS_PROCSTATS|DB_OPT_DUMP_DISPLAY, \
                String); \
            raise(SIGKILL); \
        } while(0)


#define AEE_WARNING_ASSERT_DEFAULT(String) \
        do { \
            aee_system_warning( \
                "mtkcam/Dip", \
                NULL, \
                DB_OPT_DEFAULT|DB_OPT_FTRACE, \
                String); \
            raise(SIGKILL); \
        } while(0)

#else
#define AEE_ASSERT(String)
#endif

#define ABORT() \
    do { \
        abort(); \
    } while(0)


#if defined(DIP_ENG_LOAD)
    #define DIP_ENG_LOAD_SUPPORT
    //#define DIP_USERDBG_LOAD_ONLY
    //#define DIP_USER_LOAD_ONLY
    #undef   DIP_USERDBG_AND_USER_LOAD_SUPPORT  
#else
    #define DIP_USERDBG_AND_USER_LOAD_SUPPORT
#endif


#endif /*__DIP_AEE_H__*/
