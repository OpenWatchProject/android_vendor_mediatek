#include "../include/recorder.h"
#include "../include/backtrace.h"

static size_t ubrd_main_thread_stack_start = 0;
static size_t ubrd_main_thread_stack_size = 0;
static pid_t ubrd_pid = 0;

//return 0: success, -1 fail
static int ubrd_get_main_thread_stack(pid_t pid) {
    char c, line[1024];
    int i = 0, j = 0, fd = -1;
    void* lib = NULL;
    int (*close_fptr)(int) = NULL;  // use func ptr to avoid open/close rehook deadlock
    int (*open_fptr)(const char *, int, ...) = NULL;

    lib = dlopen("libc.so", RTLD_NOW|RTLD_GLOBAL);
    if (lib == NULL) {
        ubrd_error_log("Could not open libc.so: %s\n", dlerror());
        return -1;
    }

    close_fptr = (int (*)(int))dlsym(lib, "close");
    open_fptr = (int (*)(const char *, int, ...))dlsym(lib, "open");
    if (!close_fptr || !open_fptr) {
        ubrd_error_log("invalid open/close: %s\n", dlerror());
        dlclose(lib);
        return -1;
    }

    snprintf(line, sizeof(line), "/proc/self/task/%d/maps", pid);

    fd = open_fptr(line, O_RDONLY);
    if (fd < 0) {
        ubrd_error_log("/proc/%d/task/%d/maps open fail\n", pid, pid);
        dlclose(lib);
        return -1;
    }

    while (1) {
        if (!read(fd, &c, 1)) break;
        if (c == '\n') {
            line[i] = '\0';
            if ((i > 21 + 7) && (strncmp((const char *)&line[i-7], "[stack]", 7) == 0)) {
                const char* stack_start;
                ubrd_debug_log("stack:%s\n", line);
            #if defined(__LP64__)
                // 64bit main stack format
                // 7ff1bf1000-7ff1c12000 rw-p 00000000 00:00 0                              [stack]
                line[9] = '0';
                line[10] = 'x';
                line[21] = '\0';  // line[11~20] stack start
                stack_start = (const char*)&line[9];
            #else
                // 32bit main stack format
                // becd7000-becf8000 rw-p 00000000 00:00 0          [stack]
                line[7] = '0';
                line[8] = 'x';
                line[17] = '\0';  // line[9~16] 32bit main thread stack start
                stack_start = (const char*)&line[7];
            #endif
                ubrd_main_thread_stack_start = strtoul(stack_start, (char **)NULL, 16);
                ubrd_main_thread_stack_size = 8 * 1024 * 1024;  // bypass RLIMIT check for simple handling
                ubrd_debug_log("main_thread_stack_start:%p, main_thread_stack_size:%p\n",
                                (void *)ubrd_main_thread_stack_start, (void *)ubrd_main_thread_stack_size);
                close_fptr(fd);
                dlclose(lib);
                return 0;
            }
            else {
                i = 0;
            }
        }
        else {
            line[i++] = c;
        }
    }
    close_fptr(fd);

    // stack is 28th parameter from /proc/self/stat
    fd = open_fptr("/proc/self/stat", O_RDONLY);
    if (fd < 0) {
        ubrd_error_log("/proc/self/stat open fail\n");
        dlclose(lib);
        return -1;
    }

    i = j = 0;
    line[0] = '\0';
    while (1) {
        if (!read(fd, &c, 1)) break;
        if (c == ' ') {
            j++;
            if (j == 28) {
                line[i] = '\0';
                break;
            }
        }
        else if (j == 27) {
            line[i++] = c;
        }
    }

    if (line[0] != '\0') {
        ubrd_main_thread_stack_start = strtoul(line, (char **)NULL, 10);
        ubrd_main_thread_stack_size = 8 * 1024 * 1024;  // 8MB for simple handling
        ubrd_debug_log("main_thread_stack_start from /proc/self/stat:%p, main_thread_stack_size:%p\n",
                      (void *)ubrd_main_thread_stack_start, (void *)ubrd_main_thread_stack_size);
        close_fptr(fd);
        dlclose(lib);
        return 0;
    }

    close_fptr(fd);
    dlclose(lib);
    return -1;
}

//return 0: success, -1 fail
static int ubrd_get_stack(size_t* pthread_stack_start, size_t* pthread_stack_end) {
    pid_t pid = getpid();

    if (gettid() == pid) {
        if (pid != ubrd_pid) {
            ubrd_debug_log("old pid:%d, new pid:%d\n", ubrd_pid, pid);
            ubrd_pid = pid;
            ubrd_main_thread_stack_start = 0;  // reset for new process
            ubrd_main_thread_stack_size = 0;
        }

        if (!ubrd_main_thread_stack_start) {
            if (ubrd_get_main_thread_stack(pid)) {
                ubrd_error_log("get_main_thread_stack fail\n");
                return -1;
            }
        }
        *pthread_stack_start = ubrd_main_thread_stack_start;
        *pthread_stack_end = ubrd_main_thread_stack_start - ubrd_main_thread_stack_size;
    }
    else {
        pthread_attr_t attr;
        pthread_getattr_np(pthread_self(), &attr);
        *pthread_stack_start = (size_t)attr.stack_base + attr.stack_size;
        *pthread_stack_end = (size_t)attr.stack_base;
    }
    return 0;
}

#if defined(__LP64__)
static inline
size_t internal_get_backtrace_fp(uintptr_t* addrs, size_t max_entries,
                                 size_t stack_start, size_t stack_end) {
    size_t k = 0;
    bool have_skipped_self = false;
    size_t next_fp = 0, lr;
    size_t fp_tmp = (size_t)__builtin_frame_address(0);

    while (k < max_entries && fp_tmp > stack_end && fp_tmp < stack_start) {
        if (fp_tmp & 0xf) break;

        if (!have_skipped_self) {
            have_skipped_self = true;
            fp_tmp = *(uintptr_t *)fp_tmp;
            continue;
        }

        lr = *((uintptr_t *)fp_tmp + 1);
        if (lr < 0xfffffffff) {  // filter invalid address
            break;
        }

        // ubrd_debug_log("ubrd_get_backtrace_fp 64bit :%zu, fp:%p, addrs:%p\n",
        //                 k, (void*)fp_tmp, (void*)(*(uintptr_t *)(fp_tmp + 8) - 4));
        /*
        * -4 here because we care about the PC at time of bl,
        * not where the return will go.
        */
        addrs[k++] = lr - 4;
        next_fp = *(uintptr_t *)fp_tmp;
        if (next_fp <= fp_tmp)
            break;
        fp_tmp = next_fp;
    }
    return k;
}

size_t ubrd_get_backtrace_fp(void* fp, uintptr_t* addrs, size_t max_entries) {
    size_t i = 0;
    size_t pthread_stack_start;
    size_t pthread_stack_end;

    (void)fp;
    if (ubrd_get_stack(&pthread_stack_start, &pthread_stack_end))
        return 0;

    i = internal_get_backtrace_fp(addrs, max_entries, pthread_stack_start, pthread_stack_end);
    return i;
}

size_t ubrd_get_backtrace(uintptr_t* addrs, size_t max_entries, size_t skip_count) {
    size_t i = 0;
    size_t pthread_stack_start;
    size_t pthread_stack_end;

    (void)skip_count;
    if (ubrd_get_stack(&pthread_stack_start, &pthread_stack_end))
        return 0;

    i = internal_get_backtrace_fp(addrs, max_entries, pthread_stack_start, pthread_stack_end);
    return i;
}

size_t ubrd_get_backtrace_common(void *fp, uintptr_t* addrs, size_t max_entries, size_t btmethod) {
    size_t i = 0;
    size_t pthread_stack_start;
    size_t pthread_stack_end;

    (void)btmethod;
    (void)fp;
    if (ubrd_get_stack(&pthread_stack_start, &pthread_stack_end))
        return 0;

    i = internal_get_backtrace_fp(addrs, max_entries, pthread_stack_start, pthread_stack_end);
    return i;
}
#else  //32bit
#include "../unwind/libudf-unwind/backtrace.h"

/* depends how the system includes define this */
#ifdef HAVE_UNWIND_CONTEXT_STRUCT
typedef struct _Unwind_Context __unwind_context;
#else
typedef _Unwind_Context __unwind_context;
#endif

struct stack_crawl_state_t {
    uintptr_t* frames;
    size_t frame_count;
    size_t max_depth;
    size_t skip_count;  // LCH add for skip some frames
    bool have_skipped_self;

    stack_crawl_state_t(uintptr_t* frames, size_t max_depth, size_t skip_count)
         : frames(frames), frame_count(0), max_depth(max_depth), skip_count(skip_count), have_skipped_self(false) {
    }
};

static _Unwind_Reason_Code trace_function(__unwind_context* context, void* arg) {
    stack_crawl_state_t* state = static_cast<stack_crawl_state_t*>(arg);

    uintptr_t ip = _Unwind_GetIP(context);

    // The first stack frame is ubrd_get_backtrace itself. Skip it.
    if (ip != 0 && !state->have_skipped_self) {
        state->have_skipped_self = true;
        return _URC_NO_REASON;
    }

    // LCH add for skip some frames
    if (state->skip_count > 0) {
        state->skip_count--;
        return _URC_NO_REASON;
    }

#ifdef __arm__
    /*
    * The instruction pointer is pointing at the instruction after the bl(x), and
    * the _Unwind_Backtrace routine already masks the Thumb mode indicator (LSB
    * in PC). So we need to do a quick check here to find out if the previous
    * instruction is a Thumb-mode BLX(2). If so subtract 2 otherwise 4 from PC.
    */
    if (ip != 0) {
        short* ptr = reinterpret_cast<short*>(ip);
        // Thumb BLX(2)
        if ((*(ptr-1) & 0xff80) == 0x4780) {
            ip -= 2;
        } else {
            ip -= 4;
        }
    }
#endif

    state->frames[state->frame_count++] = ip;
    return (state->frame_count >= state->max_depth) ? _URC_END_OF_STACK : _URC_NO_REASON;
}

size_t ubrd_get_backtrace(uintptr_t* addrs, size_t max_entries, size_t skip_count) {
    stack_crawl_state_t state(addrs, max_entries, skip_count);
    ubrd_debug_log("ubrd_get_backtrace 32bit\n");
    _Unwind_Backtrace(trace_function, &state);
    return state.frame_count;
}

// support gcc/clang fp unwind
size_t ubrd_get_backtrace_fp(void* fp, uintptr_t* addrs, size_t max_entries){
    intptr_t *next_fp = NULL;
    size_t i = 0, lr;

    size_t pthread_stack_start;
    size_t pthread_stack_end;

    if (ubrd_get_stack(&pthread_stack_start, &pthread_stack_end))
        return 0;

    while (i < max_entries && (size_t)fp > pthread_stack_end && (size_t)fp < pthread_stack_start) {
        lr = *(intptr_t *)fp;  // Default to GCC

        if ((lr > pthread_stack_end) && (lr < (pthread_stack_start - 4))) {  // CLANG
            // This is one known situation which will cause NE
            if ((size_t)((intptr_t *)fp + 1) > (pthread_stack_start - 4))
                break;

            lr = *((intptr_t *)fp + 1);
            next_fp = (intptr_t *)(*((intptr_t *)fp));
        } else {  // GCC
            next_fp = (intptr_t *)(*((intptr_t *)fp - 1));
        }

        if (lr < 0x10000000) {  // filter invalid address
            break;
        }

        addrs[i++] = lr - 4;  // get previous frame PC
        if (next_fp <= (intptr_t *)fp)
            break;
        fp = next_fp;
    }

    return i;
}


// TODO: add corkscrew functions
size_t ubrd_get_backtrace_common(void *fp, uintptr_t* addrs, size_t max_entries, size_t btmethod) {
    switch (btmethod) {
    case UBRD_FP_BACKTRACE:
        return ubrd_get_backtrace_fp(fp, addrs, max_entries);
    case UBRD_GCC_UNWIND_BACKTRACE:
    //    return ubrd_get_backtrace(addrs, max_entries, 0);  // mask gcc unwind
    case UBRD_CORKSCREW_UNWIND_BACKTRACE:
        return libudf_unwind_backtrace(addrs, 4, max_entries);
    default:
        return ubrd_get_backtrace(addrs, max_entries, 0);
    }
}
#endif

#ifdef ENABLE_JAVA_BACKTRACE
//
// Following is for java back trace
//
typedef uint32_t u4;
typedef void (*JavaBacktraceFunc)(u4*, u4);
JavaBacktraceFunc dvmGetThreadStack = NULL;

void init_back_trace_func()
{
    char* so_name = "/system/lib/libdvm.so";
    void* so_handler = NULL;
    char* func_symbol = "dvmGetThreadStack";
    char* error;

    so_handler = dlopen(so_name, RTLD_LAZY);
    if (!so_handler) {
        ubrd_error_log("%s [%d]: %s\n", __FILE__, __LINE__, dlerror());
        return;
    }

    dlerror();  // Clear any existing error

    dvmGetThreadStack = dlsym(so_handler, func_symbol);
    if ((error = dlerror()) != NULL) {
        ubrd_error_log("%s [%d]: %s", __FILE__, __LINE__, error);
        dlclose(so_handler);
    } else
        ubrd_error_log("load dvmGetThreadStack: done");

    return;
}

void get_java_backtrace(u4 *backtrace, u4 numEntries)
{
    if (dvmGetThreadStack != NULL)
        dvmGetThreadStack(backtrace, numEntries);
}
#endif // #ifdef ENABLE_JAVA_BACKTRACE
