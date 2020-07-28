#include "back_trace.h"
#include "malloc_debug_mtk.h"

static size_t malloc_main_thread_stack_start = 0;
static size_t malloc_main_thread_stack_size = 0;
static pid_t malloc_pid = 0;

//return 0: success, -1 fail
static int malloc_get_main_thread_stack(pid_t pid) {
    char c, line[1024];
    int i = 0, j = 0, fd = -1;
    int (*close_fptr)(int) = NULL; //use func ptr to avoid open/close rehook deadlock
    int (*open_fptr)(const char *, int, ...) = NULL;

    void *lib = dlopen("libc.so", RTLD_NOW|RTLD_GLOBAL);
    if (lib == NULL) {
        error_log("%s could not open libc.so: %s\n", __FUNCTION__, dlerror());
        return -1;
    }

    close_fptr = (int (*)(int))dlsym(lib, "close");
    open_fptr = (int (*)(const char *, int, ...))dlsym(lib, "open");
    if (!close_fptr || !open_fptr) {
        error_log("invalid open/close: %s\n", dlerror());
        dlclose(lib);
        return -1;
    }

    snprintf(line, sizeof(line), "/proc/self/task/%d/maps", pid);

    fd = open_fptr(line, O_RDONLY);
    if (fd < 0) {
        error_log("/proc/%d/task/%d/maps open fail\n", pid, pid);
        dlclose(lib);
        return -1;
    }

    while (1) {
        if (!read(fd, &c, 1)) break;
        if (c == '\n') {
            line[i] = '\0';
            if ((i > 21 + 7) && (strncmp((const char *)&line[i-7], "[stack]", 7) == 0)) {
                const char* stack_start;
                debug_log("stack:%s\n", line);
                #if defined(__LP64__)
                //64bit main stack format
                //7ff1bf1000-7ff1c12000 rw-p 00000000 00:00 0                              [stack]
                line[9] = '0';
                line[10] = 'x';
                line[21] = '\0'; //line[11~20] stack start
                stack_start = (const char*)&line[9];
            #else
                //32bit main stack format
                //becd7000-becf8000 rw-p 00000000 00:00 0          [stack]
                line[7] = '0';
                line[8] = 'x';
                line[17] = '\0'; //line[9~16] 32bit main thread stack start
                stack_start = (const char*)&line[7];
            #endif
                malloc_main_thread_stack_start = strtoul(stack_start, (char **)NULL, 16);
                malloc_main_thread_stack_size = 8 * 1024 * 1024; //8MB for simple handling
                debug_log("main_thread_stack_start:0x%zx, main_thread_stack_size:0x%zx\n",
                           malloc_main_thread_stack_start, malloc_main_thread_stack_size);
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
        error_log("/proc/self/stat open fail\n");
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
        malloc_main_thread_stack_start = strtoul(line, (char **)NULL, 10);
        malloc_main_thread_stack_size = 8 * 1024 * 1024;  // 8MB for simple handling
        debug_log("main_thread_stack_start from /proc/stat %d:0x%zx, main_thread_stack_size:0x%zx\n",
                            j, malloc_main_thread_stack_start, malloc_main_thread_stack_size);
        close_fptr(fd);
        dlclose(lib);
        return 0;
    }

    close_fptr(fd);
    dlclose(lib);
    return -1;
}

//return 0: success, -1 fail
static int malloc_get_stack(size_t* pthread_stack_start, size_t* pthread_stack_end) {
    pid_t pid = getpid();

    if (gettid() == pid) {
        if (pid != malloc_pid) {
            debug_log("old_pid:%d, new pid:%d\n", malloc_pid, pid);
            malloc_pid = pid;
            malloc_main_thread_stack_start = 0; // reset for new process
            malloc_main_thread_stack_size = 0;
        }

        if (!malloc_main_thread_stack_start) {
            if (malloc_get_main_thread_stack(pid)) {
                error_log("malloc_get_main_thread_stack fail\n");
                return -1;
            }
        }
        *pthread_stack_start = malloc_main_thread_stack_start;
        *pthread_stack_end = malloc_main_thread_stack_start - malloc_main_thread_stack_size;
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
size_t get_backtrace_fp(void* fp, intptr_t* addrs, size_t max_entries) {
    size_t i = 0;
    size_t pthread_stack_start;
    size_t pthread_stack_end;
    int have_skipped_self = 0;
    size_t next_fp = 0, lr;
    size_t fp_tmp = (size_t)__builtin_frame_address(0);

    (void)fp;
    if (malloc_get_stack(&pthread_stack_start, &pthread_stack_end))
        return 0;

    while(i < max_entries && fp_tmp > pthread_stack_end && fp_tmp < pthread_stack_start) {
        if (fp_tmp & 0xf) break;

        if (!have_skipped_self) {
            have_skipped_self = 1;
            fp_tmp = *(intptr_t *)fp_tmp;
            continue;
        }

        lr = *((intptr_t *)fp_tmp + 1);
        if (lr < 0xfffffffff) {  // filter invalid address
            break;
        }

        /*
        * -4 here because we care about the PC at time of bl,
        * not where the return will go.
        */
        addrs[i++] = lr - 4;
        next_fp = *(intptr_t *)fp_tmp;
        if(next_fp <= fp_tmp)
            break;
        fp_tmp = next_fp;
    }

    return i;
}

size_t get_backtrace(intptr_t* addrs, size_t max_entries) {
    size_t i = 0;
    size_t pthread_stack_start;
    size_t pthread_stack_end;
    int have_skipped_self = 0;
    size_t next_fp = 0, lr;
    size_t fp_tmp = (size_t)__builtin_frame_address(0);

    if (malloc_get_stack(&pthread_stack_start, &pthread_stack_end))
        return 0;

    while(i < max_entries && fp_tmp > pthread_stack_end && fp_tmp < pthread_stack_start) {
        if (fp_tmp & 0xf) break;

        if (!have_skipped_self) {
            have_skipped_self = 1;
            fp_tmp = *(intptr_t *)fp_tmp;
            continue;
        }

        lr = *((intptr_t *)fp_tmp + 1);
        if (lr < 0xfffffffff) {  // filter invalid address
            break;
        }

        /*
         * -4 here because we care about the PC at time of bl,
         * not where the return will go.
         */
        addrs[i++] = lr - 4;
        next_fp = *(intptr_t *)fp_tmp;
        if (next_fp <= fp_tmp)
            break;
        fp_tmp = next_fp;
    }

    return i;
}
#else //32bit
typedef struct
{
    size_t count;
    intptr_t* addrs;
} stack_crawl_state_t;


typedef struct _Unwind_Context __unwind_context;

static _Unwind_Reason_Code trace_function(__unwind_context *context, void *arg)
{
    stack_crawl_state_t* state = (stack_crawl_state_t*)arg;
    if (state->count) {
        intptr_t ip = (intptr_t)_Unwind_GetIP(context);
        if (ip) {
            state->addrs[0] = ip;
            state->addrs++;
            state->count--;
            return _URC_NO_REASON;
        }
    }
    /*
     * If we run out of space to record the address or 0 has been seen, stop
     * unwinding the stack.
     */
    return _URC_END_OF_STACK;
}

size_t get_backtrace_fp(void* fp, intptr_t* addrs, size_t max_entries) {
    size_t lr, i = 0;
    intptr_t *next_fp = NULL;
    size_t pthread_stack_start;
    size_t pthread_stack_end;

    if (malloc_get_stack(&pthread_stack_start, &pthread_stack_end))
        return 0;

    /*
     * If some binaries are not compiled with -fno-omit-frame-pointer, we can't make sure
     * the parameter "fp" will be frame pointer.
     * If content of the "fp" locates between pthread_stack_start - 3 ~ pthread_stack_start -1,
     * access *fp will cause NE (try to access exceed the stack boundry)
     */
    while (i < max_entries && (size_t)fp >= pthread_stack_end && (size_t)fp <= (pthread_stack_start - 4)) {
        /*
        * For Clang case, FP will point to FP of previous frame
        * For GCC case, FP will point to LR of previous frame (LR will be "outside" the stack)
        * So, in Clang case, the content of current FP should be located inside the stack.
        */

        lr = *(intptr_t *)fp;  // Default to GCC

        if ((lr >= pthread_stack_end) && (lr <= (pthread_stack_start - 4))) {  // CLANG
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

        addrs[i++] = lr - 4;
        if (next_fp <= (intptr_t *)fp)
            break;
        fp = next_fp;
    }

    return i;
}

size_t get_backtrace(intptr_t* addrs, size_t max_entries)
{
    stack_crawl_state_t state;
    state.count = max_entries;
    state.addrs = (intptr_t*)addrs;
    _Unwind_Backtrace(trace_function, (void*)&state);
    return max_entries - state.count;
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
        error_log("%s [%d]: %s\n", __FILE__, __LINE__, dlerror());
        return;
    }

    dlerror(); // Clear any existing error

    dvmGetThreadStack = dlsym(so_handler, func_symbol);
    if ((error = dlerror()) != NULL) {
        error_log("%s [%d]: %s", __FILE__, __LINE__, error);
        dlclose(so_handler);
    } else
        error_log("load dvmGetThreadStack: done");

    return;
}

void get_java_backtrace(u4 *backtrace, u4 numEntries)
{
    if (dvmGetThreadStack != NULL)
        dvmGetThreadStack(backtrace, numEntries);
}
#endif // #ifdef ENABLE_JAVA_BACKTRACE
