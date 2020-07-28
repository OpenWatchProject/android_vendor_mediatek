#include <sys/mman.h> //mmap
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <features.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/sysconf.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/ioctl.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <cutils/sockets.h>
#include <android/fdsan.h>
#include <android-base/unique_fd.h>
#include <log/log.h>
#include <poll.h>
#include <vector>
#include <unordered_map>
#include <utility>

// sync HAVE_DEPRECATED_MALLOC_FUNCS definition
#include <private/bionic_config.h>  // bionic/libc

/* Entry in malloc dispatch table. need sync with malloc_debug_common.h*/
#ifndef STRUCT_MALLINFO_DECLARED
#define STRUCT_MALLINFO_DECLARED 1
struct mallinfo {
  size_t arena;    /* Total number of non-mmapped bytes currently allocated from OS. */
  size_t ordblks;  /* Number of free chunks. */
  size_t smblks;   /* (Unused.) */
  size_t hblks;    /* (Unused.) */
  size_t hblkhd;   /* Total number of bytes in mmapped regions. */
  size_t usmblks;  /* Maximum total allocated space; greater than total if trimming has occurred. */
  size_t fsmblks;  /* (Unused.) */
  size_t uordblks; /* Total allocated space (normal or mmapped.) */
  size_t fordblks; /* Total free space. */
  size_t keepcost; /* Upper bound on number of bytes releasable by malloc_trim. */
};
#endif  /* STRUCT_MALLINFO_DECLARED */

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "UBRD_TEST"
#include <log/log.h>

static void mmap_test(void);
static void gpudebug_test(void);
static void fdrehook_test(void);
static void fdleak_test(void);
static void fdleak_unexpect_close_test(void);
static void debug15_test(void);
static void debug15_double_free(void);
static void debug15_buffer_overflow(void);
static void debug15_mass_malloc_free(void);

static void help_info() {
  fputs("Usage:\n"
        "  -mmap mmap debug test\n"
        "  -ubrd ubrd api test\n"
        "  -d15 debug15 rehook api test\n"
        "  -df debug15 memory double free test\n"
        "  -mo debug15 memory buffer overflow test\n"
        "  -mmf debug15 mass malloc/free test\n"
        "  -fdrehook fd alloc/free api rehook test\n"
        "  -fdleak fd exhaust test\n"
        "  -fdec fd unexpect close(errno=9) test\n"
        "Example:\n"
        "  ubrd_utest -d15\n"
        "  ubrd_utest -fdleak\n", stderr);
}

int main (int argc, char **argv) {
    ALOGD("argv[0]: %s\n", argv[0]);
    if (argc == 2) {
        ALOGD("argv[1]: %s\n", argv[1]);
        if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "-H")) {
            help_info();
        } else if (!strcmp(argv[1], "-mmap")) {
            mmap_test();
        } else if (!strcmp(argv[1], "-ubrd")) {
            gpudebug_test();
        } else if (!strcmp(argv[1], "-d15")) {
            debug15_test();
        } else if (!strcmp(argv[1], "-df")) {
            debug15_double_free();
        } else if (!strcmp(argv[1], "-mo")) {
            debug15_buffer_overflow();
        } else if (!strcmp(argv[1], "-mmf")) {
            debug15_mass_malloc_free();
        } else if (!strcmp(argv[1], "-fdrehook")) {
            fdrehook_test();
        } else if (!strcmp(argv[1], "-fdleak")) {
            fdleak_test();
        } else if (!strcmp(argv[1], "-fdec")) {
            fdleak_unexpect_close_test();
        } else {
            help_info();
        }
    } else {
       help_info();
    }

    return 0;
}


// ============================================================
//  fdleak debug test
// ============================================================

#ifndef TEMP_FAILURE_RETRY
/* Used to retry syscalls that can return EINTR. */
#define TEMP_FAILURE_RETRY(exp) ({         \
    typeof (exp) _rc;                      \
    do {                                   \
        _rc = (exp);                       \
    } while (_rc == -1 && errno == EINTR); \
    _rc; })
#endif

static void fd_common_op() {
    int fd;
    int fd1, fd2, fd3, fd4, targetfd; // declare for dup
    DIR *dir = NULL;
    int dfd = -1;
    int pipefds[2];
    int ret = 0;
    int sockfd, sockpair[2];
    int efd;
    int epollfd;

    /* open test */
    fd = open("/data/tmp1.txt", O_WRONLY|O_CREAT, 0666);
    if (fd < 0) {
        ALOGE("[FDLEAK_TEST]open /data/tmp1.txt fail\n");
    } else
        ALOGI("[FDLEAK_TEST]/data/tmp1.txt open success, fd: %d\n", fd);
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
    ALOGI("\n");

    fd1 = open("/data/tmp1.txt", O_WRONLY);
    if (fd1 < 0) {
        ALOGE("[FDLEAK_TEST]open /data/tmp1.txt again fail\n");
    } else
        ALOGI("[FDLEAK_TEST]/data/tmp1.txt open again success, fd1: %d\n", fd1);
    if (fd1 >= 0) {
        close(fd1);
        fd1 = -1;
    }
    ALOGI("\n");

    /* open64 test */
    fd = open64("/data/tmp2.txt", O_WRONLY|O_CREAT, 0666);
    if (fd < 0) {
        ALOGE("[FDLEAK_TEST]open64 /data/tmp2.txt fail\n");
    } else {
        ALOGI("[FDLEAK_TEST]/data/tmp2.txt open64 success, fd: %d\n", fd);
        close(fd);
    }
    ALOGI("\n");

    fd = open64("/data/tmp2.txt", O_WRONLY|O_CREAT);
    if (fd < 0) {
        ALOGE("[FDLEAK_TEST]open64 /data/tmp2.txt fail\n");
    } else {
        ALOGI("[FDLEAK_TEST]/data/tmp2.txt open64 again success, fd: %d\n", fd);
        close(fd);
    }
    ALOGI("\n");

    /* create and create64 test */
    fd = creat("/data/tmp2.txt", 0666);
    if (fd < 0) {
        ALOGE("[FDLEAK_TEST]creat /data/tmp2.txt fail\n");
    } else
        ALOGI("[FDLEAK_TEST]/data/tmp2.txt creat success, fd: %d\n", fd);
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
    fd = creat64("/data/tmp2.txt", 0666);
    if (fd < 0) {
        ALOGE("[FDLEAK_TEST]creat64 /data/tmp2.txt fail\n");
    } else
        ALOGI("[FDLEAK_TEST]/data/tmp2.txt creat64 success, fd: %d\n", fd);
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
    ALOGI("\n");

    /* openat test */
    dir = opendir("/data");
    if (dir != NULL) {
        dfd = dirfd(dir);
        fd = openat(dfd, "tmp3.txt", O_WRONLY|O_CREAT, 0666);
        if (fd < 0) {
            ALOGE("[FDLEAK_TEST]openat /data/tmp3.txt fail\n");
        } else {
            ALOGI("[FDLEAK_TEST]/data/tmp3.txt openat success, fd: %d\n", fd);
            close(fd);
            fd = -1;
        }
    }
    fd1 = openat(dfd, "tmp3.txt", O_WRONLY);
    if (fd1 < 0) {
        ALOGE("[FDLEAK_TEST]openat /data/tmp3.txt fail\n");
    }
    if (fd1 >= 0) {
        ALOGI("[FDLEAK_TEST]/data/tmp3.txt openat again success, fd1: %d\n", fd1);
        close(fd1);
        fd1 = -1;
    }
    closedir(dir);
    ALOGI("\n");

    /* openat64 test */
    dir = opendir("/data");
    if (dir != NULL) {
        dfd = dirfd(dir);
        fd = openat64(dfd, "tmp3.txt", O_WRONLY|O_CREAT, 0666);
        if (fd < 0) {
            ALOGE("[FDLEAK_TEST]openat64 /data/tmp3.txt fail\n");
        } else {
            ALOGI("[FDLEAK_TEST]/data/tmp3.txt openat64 success, fd: %d\n", fd);
            close(fd);
            fd = -1;
        }
    }
    fd1 = openat64(dfd, "tmp3.txt", O_WRONLY);
    if (fd1 < 0) {
        ALOGE("[FDLEAK_TEST]openat64 /data/tmp3.txt fail\n");
    }
    if (fd1 >= 0) {
        ALOGI("[FDLEAK_TEST]/data/tmp3.txt openat64 again success, fd1: %d\n", fd1);
        close(fd1);
        fd1 = -1;
    }
    closedir(dir);
    ALOGI("\n");

    /* eventfd test */
    efd = eventfd(0, EFD_NONBLOCK);
    if (efd == -1) {
       ALOGE("[FDLEAK_TEST]eventfd fail\n");
    } else
        ALOGI("[FDLEAK_TEST]eventfd efd: %d\n", efd);
    if (efd != -1) {
        close(efd);
        efd = -1;
    }
    ALOGI("\n");

    /* pipe and pipe2 test */
    ret = pipe(pipefds);
    if (ret == 0) {
        ALOGI("[FDLEAK_TEST]pipe fds: %d, %d\n", pipefds[0], pipefds[1]);
        close(pipefds[0]);
        close(pipefds[1]);
    } else {
        ALOGE("[FDLEAK_TEST]pipe open fail\n");
    }
    ret = pipe2(pipefds, O_NONBLOCK);
    if (ret == 0) {
        ALOGI("[FDLEAK_TEST]pipe2 fds: %d, %d\n", pipefds[0], pipefds[1]);
        close(pipefds[0]);
        close(pipefds[1]);
    } else {
        ALOGE("[FDLEAK_TEST]pipe2 open fail\n");
    }
    ALOGI("\n");

    /* socket test */
    sockfd = socket(PF_UNIX, SOCK_RAW, 0);
    if (sockfd < 0) {
        ALOGE("[FDLEAK_TEST]socket create fail\n");
    } else
        ALOGI("[FDLEAK_TEST]socket fd: %d\n", sockfd);
    if (sockfd != -1) {
        close(sockfd);
        sockfd = -1;
    }
    ALOGI("\n");

    /*  socketpair test  */
    ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, sockpair);
    if (ret == 0) {
        ALOGI("[FDLEAK_TEST]socketpair fds: %d, %d\n", sockpair[0], sockpair[1]);
        close(sockpair[0]);
        close(sockpair[1]);
    } else {
        ALOGE("[FDLEAK_TEST]socketpair create fail\n");
    }
    ALOGI("\n");

    /* dup, dup2, dup3, fcntl test */
    fd = open("/data/tmp4.txt", O_WRONLY|O_CREAT, 0666);
    if (fd < 0) {
        ALOGE("[FDLEAK_TEST]open /data/tmp4.txt fail\n");
    } else {
        ALOGI("[FDLEAK_TEST]/data/tmp4.txt open success, fd: %d\n", fd);

        fd1 = dup(fd);
        if (fd1 >= 0) {
            ALOGI("[FDLEAK_TEST]dup fd: %d, fd1: %d\n", fd, fd1);
            //close(fd1);
        } else {
            ALOGE("[FDLEAK_TEST] dup fail");
        }

        targetfd = 667;
        fd2 = dup2(fd, targetfd);
        if (fd2 == targetfd) {
            ALOGI("[FDLEAK_TEST]dup2 fd: %d, targetfd: %d, fd2: %d\n", fd, targetfd, fd2);
            //close(fd2);
        } else if (fd2 < 0) {
            ALOGE("[FDLEAK_TEST] dup2 fail");
        }

        targetfd = 668;
        fd3 = dup3(fd, targetfd, O_CLOEXEC);
        if (fd3 == targetfd) {
            ALOGI("[FDLEAK_TEST]dup3 fd: %d, targetfd: %d, fd3: %d\n", fd, targetfd, fd3);
            //close(fd3);
        } else if (fd3 < 0) {
            ALOGE("[FDLEAK_TEST] dup3 fail");
        }

        targetfd = 669;
        fd4 = fcntl(fd, F_DUPFD, targetfd);
        if (fd4 == targetfd) {
            ALOGI("[FDLEAK_TEST]fcntl-dup fd: %d, targetfd: %d, fd4: %d\n", fd, targetfd, fd4);
            //close(fd4);
        } else if (fd4 < 0) {
            ALOGE("fcntl DUPFD fail");
        }

        //close(fd);
    }
    ALOGI("\n");

    /* epoll_create and epoll_create1 test */
    epollfd = epoll_create(100);
    if (epollfd == -1) {
        ALOGE("[FDLEAK_TEST]epoll_create fail\n");
    } else {
        ALOGI("[FDLEAK_TEST]epoll_create efd: %d\n", epollfd);
        close(epollfd);
        epollfd = -1;
    }
    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        ALOGE("[FDLEAK_TEST]epoll_create1 fail\n");
    } else {
        ALOGI("[FDLEAK_TEST]epoll_create1 efd: %d\n", epollfd);
        close(epollfd);
        epollfd = -1;
    }
    ALOGI("\n");

    /* ------ unique_fd test ----------- */
    android::base::unique_fd uniq_fd(open("/dev/null", O_RDONLY));
    fd = uniq_fd.get();
    ALOGI("alloc uniq_fd addr: %p, fd: %d", &uniq_fd, fd);

    uint64_t tag = android_fdsan_get_owner_tag(fd);
    uint64_t tag_value = android_fdsan_get_tag_value(tag);
    const char* tag_type = android_fdsan_get_tag_type(tag);
    ALOGI("uniq_fd info, fd: %d, tag = 0x%" PRIx64 ", tag_value: 0x%" PRIx64 ", tag_type: %s",
        fd, tag, tag_value, tag_type);
    ALOGI("\n");
}

// clients buffer
static std::unordered_map<int, bool> gClients;

static int onDataAvailable(int sock) {
    char buf[128] = {};
    ssize_t rc = 0;

    rc = TEMP_FAILURE_RETRY(read(sock, buf, sizeof(buf) - 1));
    if (rc < 0 ) {
        ALOGE("read fail, (%s)", strerror(errno));
    } else if ( rc == 0) { // if peer closed, pollHUP and read return 0
        ALOGE("read empty, the peer may be closed");
    } else
        ALOGI("read success, msg: %s, size: %zd", buf, rc);
    return rc;
}

static void* wait_clients(void* socket) {
    int serverFd = *(int *)socket;
    ALOGI("serverFd: %d", serverFd);

    while (true) {
        std::vector<pollfd> fds;

        struct pollfd tmpPfd = {
            .fd = serverFd,
            .events = POLLIN
        };
        fds.push_back(tmpPfd);

        for (auto pair : gClients) {
            if (pair.second) {
                fds.push_back({.fd = pair.first, .events = POLLIN});
            }
        }
        // show fds polling on
        int i = 0;
        for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); ++it) {
            ALOGI("polling-%d on fd: %d", i++, (*it).fd);
        }

        #if 0
        i = 0;
        for (auto pfd : fds) {
            ALOGI("polling-%d on fd: %d", i++, pfd.fd);
        }
        #endif

        int rc = TEMP_FAILURE_RETRY(poll(fds.data(), fds.size(), -1));
        const struct pollfd& p0 = fds[0];
        ALOGI("serverFd: %d, revents: %#hx", p0.fd, p0.revents);

        if (rc < 0) {
            ALOGE("poll failed (%s)", strerror(errno));
            sleep(1);
            continue;
        }
        if (fds[0].revents & (POLLIN | POLLERR | POLLHUP)) {
            int sock = TEMP_FAILURE_RETRY(accept(serverFd, nullptr, nullptr));
            if (sock < 0) {
                ALOGE("Accept failed (%s)", strerror(errno));
                sleep(1);
                continue;
            } else {
                gClients[sock] = true; // insert this client to map
                ALOGI("client sock: %d", sock);
            }
        }

        std::vector<int> pending;
        const int size = fds.size();
        for (int i = 1; i < size; i++) {
            const struct pollfd& p = fds[i];
            ALOGI("clinet-%d, fd: %d, p.revents: 0x%04hx", i, p.fd, p.revents);
            if (p.revents & (POLLIN | POLLERR | POLLHUP)) {
                std::unordered_map<int, bool>::iterator it = gClients.find(p.fd);
                if (it == gClients.end()) {
                    ALOGE("The fd vanished: %d", p.fd);
                    continue;
                } else if (it->second == false) {
                    ALOGE("The fd not enabled to poll: %d", p.fd);
                    continue;
                }
                pending.push_back(p.fd);
            }
        }
        for (int fd : pending) {
            if (onDataAvailable(fd) <= 0) {
                close(fd);
                gClients[fd] = false;
                ALOGE("Next time not poll the fd: %d", fd);
            }
        }
    }
    return nullptr;
}

static int setup_fdserver() {
    int ret;
    static int socket_id = 0;
    pthread_t thread_id;
    pthread_attr_t attr;

    socket_id = socket_local_server("fdleak_test_server", ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_SEQPACKET);
    if (socket_id < 0) {
        ALOGE("create server fail(%s).",strerror(errno));
        return 0;
    }

    if ((ret = listen(socket_id, 8)) < 0) {
        ALOGE("listen socket fail(%s).",strerror(errno));
        close(socket_id);
        return 0;
    }

    if (!pthread_attr_init(&attr)) {
        if (!pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) {
            if (!pthread_create(&thread_id, &attr, wait_clients,(void *)&socket_id))
                pthread_attr_destroy(&attr);
            else {
                ALOGE("create thread fail(%s)",strerror(errno));
                close(socket_id);
            }
        }
    }
    return 1;
}

// client connect And Send msg
static void* connectAndSend(void* name) {
    // wait for server setup, because of  client connect before server setup in the test case
    usleep(100 * 1000);
    char buf[128] = {};

    int sock = socket_local_client("fdleak_test_server", ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_SEQPACKET);
    if (sock < 0) {
        ALOGE("setup client for fdleak_test_server fail (%s)", strerror(errno));
        return nullptr;
    }

    // send msg
    snprintf(buf, sizeof(buf), "%s_JUST TEST!", name);
    if (TEMP_FAILURE_RETRY(write(sock, buf, strlen(buf) + 1)) <= 0) {
        ALOGE("send '%s' fail (%s)", buf, strerror(errno));
    } else
        ALOGI("send '%s' done", buf);
    //TEMP_FAILURE_RETRY(pause());
    sleep(1);
    return nullptr;
}

static int creat_client(const char* name) {
    pthread_t thread_id;
    pthread_attr_t attr;
    if (!pthread_attr_init(&attr)) {
        if (!pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) {
            if (!pthread_create(&thread_id, &attr, connectAndSend,(void *)name))
                pthread_attr_destroy(&attr);
            else {
                ALOGE("create thread fail(%s)",strerror(errno));
                return -1;
            }
        }
    }
    return 0;
}

void fdrehook_test() {
    ALOGI("--------------------- [FD_Rehook_TEST] ---------------------");
    fd_common_op();

    // socket api test
    pid_t pid = fork();
    if (pid == 0) { // child process,
        creat_client("client_A");
        creat_client("client_B");
        creat_client("client_C");

        //TEMP_FAILURE_RETRY(pause());
        ALOGI("Child process wait 2s for testcase related to socket");
        fprintf(stderr, LOG_TAG " Child process(pid: %d) wait 2s for testcase related to socket\n", getpid());
        sleep(2);
        _exit(0);
    }
    setup_fdserver();

    //TEMP_FAILURE_RETRY(pause());
    ALOGI("main thread wait 3s for testcase related to socket");
    fprintf(stderr, LOG_TAG " Father process(pid: %d) wait 3s for testcase related to socket\n", getpid());
    sleep(3);
    exit(0);
}

void fdleak_test() {
    int fd;

    void (*fd_bt_rd)(int) = NULL;
    if (!fd_bt_rd) {
        fd_bt_rd = reinterpret_cast<void (*)(int)>(dlsym(RTLD_DEFAULT,
                                          "fdleak_record_backtrace_safe"));
        if (!fd_bt_rd) {
            ALOGE("[FDLEAK_TEST]dlsym RTLD_DEFAULT fdleak_record_backtrace_safe faild, dlerror:%s\n", dlerror());
        }
    }
    if (fd_bt_rd) {
       ALOGD("[FDLEAK_TEST]dlsym RTLD_DEFAULT fdleak_record_backtrace_safe:%p\n", fd_bt_rd);
       fd_bt_rd(600);
    }

    // test fd leakage
    ALOGI("[FDLEAK_TEST]trigger fd exhaust\n");
    for (int i = 0; i < 1024; i++) {
        fd = open("/data/tmp4.txt", O_WRONLY|O_CREAT, 0666);
        if (fd < 0) {
            ALOGE("[FDLEAK_TEST]open fail errno:%d\n", errno);
            break;
        }
    }
}

void fdleak_unexpect_close_test(void) {
    int fd = -1;
    ssize_t ret = -1;
    int fd_dup = -1;

    fd = open("/data/tmp4.txt", O_WRONLY|O_CREAT, 0666);
    if (fd < 0) {
        ALOGE("[FDLEAK_TEST]open /data/tmp2.txt fail, errno:%d\n", errno);
        return;
    }
    ALOGD("[FDLEAK_TEST]open /data/tmp2.txt fd:%d\n", fd);

    // dup2 fd over fdleak bt2log thd
    fd_dup = dup2(fd, 512);
    ALOGD("[FDLEAK_TEST]dup fd to 512 fd_dup:%d\n", fd_dup);
    close(fd);
    if (fd_dup >= 0) {
        ALOGD("[FDLEAK_TEST]close fd_dup:%d\n", fd_dup);
        close(fd_dup);
    } else {
        ALOGE("[FDLEAK_TEST]dup fd to 512 fail, errno:%d\n", errno);
        return;
    }

    ALOGD("[FDLEAK_TEST]write data to fd:%d after close\n", fd_dup);
    ret = write(fd_dup, "hello", strlen("hello"));
    if (ret < 0) {
        ALOGE("[FDLEAK_TEST]write data to fd:%d fail, errno:%d\n", fd_dup, errno);
        abort();
    } else {
        ALOGD("[FDLEAK_TEST]write data to fd:%d size:%zd\n", fd_dup, ret);
    }
}


// ============================================================
//  mmap test
// ============================================================
#define MMAP_SIZE (5*1024)

void mmap_test() {
    int i =0;
    void* mmap_ptr[10];
    void* mmap_ptr2 = NULL;
    void* mmap_ptr1 = mmap(NULL, MMAP_SIZE, PROT_READ |PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, /*fd*/-1, 0);
    ALOGD("[MMAP_BRTEST] mmap_ptr:%p", mmap_ptr1);
    if(mmap_ptr1 != MAP_FAILED) {
        ALOGD("[MMAP_BRTEST] munmap_ptr:%p", mmap_ptr1);
        memset(mmap_ptr1, 0x0, MMAP_SIZE);
        munmap(mmap_ptr1, MMAP_SIZE);
        //*((unsigned int *)0) = 0x01;
        mmap_ptr1 = NULL;
        mmap_ptr1 = mmap(NULL, MMAP_SIZE*5, PROT_READ |PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, /*fd*/-1, 0);
        ALOGD("[MMAP_BRTEST] mmap_ptr 1:%p", mmap_ptr1);
        mmap_ptr2 = mmap(NULL, MMAP_SIZE*10, PROT_READ |PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, /*fd*/-1, 0);
        ALOGD("[MMAP_BRTEST] mmap_ptr 2:%p", mmap_ptr2);
        //while(1);
    }

    //for same backtrace driver bt entry test
    for (i=0; i<10; i++) {
        mmap_ptr[i] = mmap(NULL, MMAP_SIZE, PROT_READ |PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, /*fd*/-1, 0);
        ALOGD("[MMAP_BRTEST] mmap ptr %d:%p", i, mmap_ptr[i]);
    }

    for (i=0; i<10; i++) {
       if (mmap_ptr[i] != MAP_FAILED) {
           ALOGD("[MMAP_BRTEST] munmap ptr %d:%p", i, mmap_ptr[i]);
           munmap(mmap_ptr[i], MMAP_SIZE);
       }
    }
}

// ============================================================
//  gpu debug test
//  Notes: 
//  suggest only use ubrd API  in MTK internal eng load for debug
//  suggest not use ubrd API on customer load
// ============================================================

#include <ubrd_config.h>

PUBRD g_GPUDebug_PUBRD = NULL;
UBRD_INIT_FUNCPTR GPUDebug_ubrd_init = NULL;
UBRD_BTRACE_RECORD_FUNCPTR GPUDebug_btrace_record = NULL;
UBRD_BTRACE_REMOVE_FUNCPTR GPUDebug_btrace_remove = NULL;

#if 0
// if need specifc compare for find entry to delete, need reimplement compareFunc, 
// default only check PEntryInfo->mAddr
//
//return 1: equal
//         0: not equal
static int GPUDebug_compareFunc(PEntryInfo data1, PEntryInfo data2) {
    if (data1->mAddr == data2->mAddr
        && data1->mBytes == data2->mBytes
        && data1->mExtraInfoLen == data2->mExtraInfoLen) {
        if (!data1->mExtraInfoLen) return 1;
        else if (!memcmp(data1->mExtraInfo, data2->mExtraInfo, data1->mExtraInfoLen)) {
            ALOGE("[GPU_BRTEST]PEntryInfo 1:%p, PEntryInfo 2:%p equal\n", data1, data2);
            return 1;
        }
        else {
            return 0;
        }
    }
    else
        return 0;
}
#endif

static inline void btrace_record_init(const char *module_name, uint64_t config) {
    GPUDebug_ubrd_init = reinterpret_cast<UBRD_INIT_FUNCPTR>(dlsym(RTLD_DEFAULT, UBRD_INIT));
    if (GPUDebug_ubrd_init && (g_GPUDebug_PUBRD = GPUDebug_ubrd_init(module_name, config, NULL))) {
        GPUDebug_btrace_record =
            reinterpret_cast<UBRD_BTRACE_RECORD_FUNCPTR>(dlsym(RTLD_DEFAULT, UBRD_BTRACE_RECORD));
        GPUDebug_btrace_remove =
            reinterpret_cast<UBRD_BTRACE_REMOVE_FUNCPTR>(dlsym(RTLD_DEFAULT, UBRD_BTRACE_REMOVE));
        if (!GPUDebug_btrace_record || !GPUDebug_btrace_remove) {
            ALOGE("[GPU_TEST]dlsym RTLD_DEFAULT fail btrace_record: %p, btrace_remove:%p, error:%s\n",
            GPUDebug_btrace_record, GPUDebug_btrace_remove, dlerror());
        }
        else
            ALOGE("[GPU_TEST]btrace_record: %p, btrace_remove:%p\n",
                   GPUDebug_btrace_record, GPUDebug_btrace_remove);
    }
    else
        ALOGE("[GPU_TEST]dlsym RTLD_DEFAULT fail GPUDebug_ubrd_init: %p, error:%s\n",
               GPUDebug_ubrd_init, dlerror());
}

void gpudebug_test() 
{
    void *buf = NULL;
    char client[] = "texture";
    int i = 0;
    void *buf_array[10];

    //if need property control, you can set by yourself, naming rule as below, detail reference BrCustomMmap.c
    //persist.debug.xxx                - for debug enable or not
    //persist.debug.xxx.program   -  for control specific process enable
    //persist.debug.xxx.config      -  for config dynamic setting

    /*
    * config item need reference ubrd_config.h
    */
    uint64_t debugConfig = 0x22002010;//0x62000010;

    btrace_record_init("GPUDebug", debugConfig);

    buf = malloc(256);
    if (GPUDebug_btrace_record) { //call after memory allocation
        GPUDebug_btrace_record(g_GPUDebug_PUBRD, buf, 256, client, strlen(client)+1);
    }
    ALOGD("[GPU_BRTEST]malloc buf:%p, size:%d\n", buf, 256);
    memset(buf, 0x0, 256);
#if 0
    if (GPUDebug_btrace_remove) { //call before free
        ALOGD("[GPU_BRTEST]free buf:%p\n", buf);
        GPUDebug_btrace_remove(g_GPUDebug_PUBRD, buf, 0, NULL, 0);
    }
    free(buf);
#endif
    for (i=0; i<10; i++) {
       buf_array[i] = malloc(258+i*1);
       if (GPUDebug_btrace_record) { //call after memory allocation
           GPUDebug_btrace_record(g_GPUDebug_PUBRD, buf_array[i], 258+i*1, client, strlen(client)+1);
       }
       ALOGD("[GPU_BRTEST]malloc buf %d:%p, size:%d\n", i, buf_array[i], 258+i*1);
    }

    for (i=0; i<10; i++) {
        ALOGD("[GPU_BRTEST]free buf %d:%p\n", i, buf_array[i]);
        if (GPUDebug_btrace_remove) {
            GPUDebug_btrace_remove(g_GPUDebug_PUBRD, buf_array[i], 0, NULL, 0);
        }
        free(buf_array[i]);
    }
}


#if defined(HAVE_DEPRECATED_MALLOC_FUNCS)
extern "C" void* pvalloc(size_t);
extern "C" void* valloc(size_t);
#endif

#ifndef WORKAROUND_MALLOC_ITERATE
// malloc_iterate test
extern "C" void malloc_disable();
extern "C" void malloc_enable();
extern "C" int malloc_iterate(uintptr_t base, size_t size,
    void (*callback)(uintptr_t base, size_t size), void* arg);

static uintptr_t alloc_find;
static size_t alloc_find_size;

static void callback(uintptr_t ptr, size_t size) {
    if (ptr <= alloc_find && alloc_find < ptr + size) {
        if (alloc_find + alloc_find_size <= ptr + size) {
            ALOGI("[DEBUG15_TEST]malloc_iterate callback "
                  "alloc_find: %p, ptr: %p, size:%zu\n",
                  (void *)alloc_find, (void *)ptr, size);
        }
    }
}

static void malloc_iterate_ut()
{
    void *ptr;
    size_t size = 100;

    ptr = malloc(size);
    if (!ptr)
        return;

    alloc_find = (uintptr_t)ptr;
    alloc_find_size = size;

    if (malloc_iterate((uintptr_t)ptr, size, callback, NULL) == 0)
        ALOGI("[DEBUG15_TEST]malloc_iterate find ptr: %p, size:%zu success\n", ptr, size);
    else
        ALOGE("[DEBUG15_TEST]malloc_iterate can't find ptr: %p, size:%zu success\n", ptr, size);

    free(ptr);
}
#endif

// ============================================================
//  debug15 test
// ============================================================
void debug15_test() {
    void *ptr;
    void *new_ptr;
    size_t size;
    int ret;
    struct mallinfo test_mallinfo;

    ptr = malloc(256);
    ALOGW("[DEBUG15_TEST]malloc ptr:%p, size:%d\n", ptr, 256);
    ALOGW("[DEBUG15_TEST]free ptr:%p\n", ptr);
    free(ptr);
    ALOGW("\n");

    ptr = calloc(10, 256);
    ALOGW("[DEBUG15_TEST]calloc ptr:%p, size:%d\n", ptr, 10*256);
    ALOGW("[DEBUG15_TEST]calloc free ptr:%p\n", ptr);
    free(ptr);
    ALOGW("\n");

    ptr = malloc(288);
    ALOGW("[DEBUG15_TEST]malloc ptr:%p, size:%d for realloc\n", ptr, 288);
    new_ptr = realloc(ptr, 512);
    ALOGW("[DEBUG15_TEST]realloc new_ptr:%p, size:%d\n", new_ptr, 512);
    ALOGW("[DEBUG15_TEST]realloc free new_ptr:%p\n", new_ptr);
    free(new_ptr);
    ALOGW("\n");

    ptr = memalign(256, 324);;
    ALOGW("[DEBUG15_TEST]memalign ptr:%p, size:%d for realloc\n", ptr, 324);
    new_ptr = realloc(ptr, 1024);
    ALOGW("[DEBUG15_TEST]realloc new_ptr:%p, size:%d\n", new_ptr, 512);
    ALOGW("[DEBUG15_TEST]realloc free new_ptr:%p\n", new_ptr);
    free(new_ptr);
    ALOGW("\n");

    ptr = memalign(256, 324);
    ALOGW("[DEBUG15_TEST]memalign ptr:%p, size:%d\n", ptr, 324);
    size = malloc_usable_size(ptr);
    ALOGW("[DEBUG15_TEST]memalign ptr:%p, malloc_usable_size:%zu\n", ptr, size);
    ALOGW("[DEBUG15_TEST]memalign free ptr:%p\n", ptr);
    free(ptr);
    ptr = NULL;
    ALOGW("\n");

    ret = posix_memalign(&ptr, 256, 356);
    if (!ret) {
        ALOGW("[DEBUG15_TEST]posix_memalign ptr:%p, size:%d\n", ptr, 356);
        ALOGW("[DEBUG15_TEST]posix_memalign free ptr:%p\n", ptr);
        free(ptr);
        ALOGW("\n");
    }

#if defined(HAVE_DEPRECATED_MALLOC_FUNCS)
    size_t pagesize = sysconf(_SC_PAGESIZE);
    ALOGW("[DEBUG15_TEST]pvalloc pagesize:%zu\n", pagesize);

    ptr = pvalloc(377);
    ALOGW("[DEBUG15_TEST]pvalloc ptr:%p, size:%d\n", ptr, 377);
    ALOGW("[DEBUG15_TEST]pvalloc free ptr:%p\n", ptr);
    free(ptr);
    ALOGW("\n");

    ptr = valloc(388);
    ALOGW("[DEBUG15_TEST]valloc ptr:%p, size:%d\n", ptr, 388);
    ALOGW("[DEBUG15_TEST]valloc free ptr:%p\n", ptr);
    free(ptr);
    ALOGW("\n");
#endif

    char *s = strdup("debug15_test_string");
    ALOGW("[DEBUG15_TEST]malloc_function strdup %p:%s\n", s, s);
    free(s);

    test_mallinfo =  mallinfo();
    ALOGW("[DEBUG15_TEST]mallinfo arena:%zu, ordblks:%zu, hblkhd:%zu, usmblks:%zu, uordblks:%zu, fordblks:%zu, keepcost:%zu",
    test_mallinfo.arena, test_mallinfo.ordblks,
    test_mallinfo.hblkhd, test_mallinfo.usmblks,
    test_mallinfo.uordblks, test_mallinfo.fordblks,
    test_mallinfo.keepcost);

    ptr = malloc(512);
    ALOGW("[DEBUG15_TEST]malloc ptr:%p, size:%d\n", ptr, 512);
    free(ptr);
    ptr = malloc(1024);
    ALOGW("[DEBUG15_TEST]malloc ptr:%p, size:%d\n", ptr, 1024);
    free(ptr);
    ptr = malloc(2018);
    ALOGW("[DEBUG15_TEST]malloc ptr:%p, size:%d\n", ptr, 2018);
    free(ptr);
    ptr = malloc(324);
    ALOGW("[DEBUG15_TEST]malloc ptr:%p, size:%d\n", ptr, 324);
    free(ptr);

#ifndef WORKAROUND_MALLOC_ITERATE
    malloc_iterate_ut();
#endif
}

static void debug15_double_free() {
    void *ptr = malloc(256);
    ALOGW("[DEBUG15_TEST]malloc ptr:%p, size:%d\n", ptr, 256);
    ALOGW("[DEBUG15_TEST]free ptr:%p\n", ptr);
    free(ptr);
    ALOGW("[DEBUG15_TEST]free ptr:%p again\n", ptr);
    free(ptr);
}

static void debug15_buffer_overflow() {
    void *ptr = malloc(256);
    ALOGW("[DEBUG15_TEST]malloc ptr:%p, size:%d\n", ptr, 256);
    ALOGW("[DEBUG15_TEST]rear guard ptr[%d]: 0x%x\n", 258, *((char *)ptr+258));
    *((char *)ptr+258) = 0x0;
    ALOGW("[DEBUG15_TEST]overwrite rear guard ptr[%d], new val:%d\n", 258, *((char *)ptr+258));
    ALOGW("[DEBUG15_TEST]free ptr:%p\n", ptr);
    free(ptr);
}

static void debug15_mass_malloc_free(void) {
    int count = 0;
    void *ptr = NULL;

    for (count = 0; count < 4096; count++) {  // should over debug15 historical table
        ptr = malloc(256);
        ALOGW("[DEBUG15_TEST]malloc ptr:%p, size:%d\n", ptr, 256);
        ALOGW("[DEBUG15_TEST]free ptr:%p\n", ptr);
        free(ptr);
    }
}
