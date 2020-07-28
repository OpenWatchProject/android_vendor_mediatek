#include "sig_handler.h"
// for open
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/system_properties.h>
#include <fcntl.h>
#include "malloc_debug_mtk.h"

static void dump_bt_file(size_t c, intptr_t* addrs, int fd) {
    char buf[32];
    char tmp[512]; // 32*32 is safe
    size_t i;

    tmp[0] = 0; // Need to initialize tmp[0] for the first strcat
    strlcat(tmp, "call stack:\n", sizeof tmp);

    for (i=0; i<c; i++) {
        snprintf(buf, sizeof buf, "%zu: %p\n", i, (void *)addrs[i]);
        strlcat(tmp, buf, sizeof tmp);
    }

    write(fd, tmp, strlen(tmp));
}

static void dump_maps() {
    char cmd[80];
    snprintf(cmd, sizeof(cmd),"cat /proc/%d/maps > /data/maps_%d.txt", getpid(), getpid());
    debug_log("%s\n", cmd);
    system(cmd);
}

static int open_log_file(const char *path_name) {
    assert(path_name != NULL);
    int fd = open(path_name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    return fd;
}

static void dump_info(int fd) {
    uint32_t i;
    //int max_count = 0;
    char write_buf[256];

    info_log("\nstart dumping debug 15\n");

    write(fd, "+++current allocations:\n", strlen("+++current allocations:\n"));
    for (i = 0; i < CHUNK_HASH_TABLE_SIZE; i++) {
        PChunkHashEntry entry = gChunkHashTable.chunk_hash_table[i];
        while (entry != NULL) {
            //sprintf(write_buf, "slot: %d, addr: %p, size: %zu, flag: 0x%x\n", i, entry->chunk_start, entry->bytes, entry->flag);
            sprintf(write_buf, "slot: %d, addr: %p, size: %zu\n", i, entry->chunk_start, entry->bytes);
            write(fd, write_buf, strlen(write_buf));

            if (entry->bt_entry->numEntries != 0) {
                dump_bt_file(entry->bt_entry->numEntries, entry->bt_entry->backtrace, fd);
            }
            else
                error_log("[ERROR] addr without bt\n");

            entry = entry->next;
        }
    }

    memset(write_buf, 0, sizeof(write_buf));
    write(fd, "\n---start dumping historical allocations:\n", strlen("\n---start dumping historical allocations:\n"));
    for(i = 0; i < gDebugConfig.mHistoricalBufferSize; i++) {
        PChunkHashEntry entry = gHistoricalAllocTable.historical_alloc_table[i];
        if (entry != NULL) {
            sprintf(write_buf, "slot: %d, addr: %p, size: %zu\n", i, entry->chunk_start, entry->bytes);
            write(fd, write_buf, strlen(write_buf));
            if (entry->bt_entry->numEntries != 0) {
                write(fd, "malloc ", strlen("malloc "));
                dump_bt_file(entry->bt_entry->numEntries, entry->bt_entry->backtrace, fd);
            }

            if (entry->free_bt != NULL) {
                write(fd, "free ", strlen("free "));
                dump_bt_file(entry->free_bt->numEntries, entry->free_bt->backtrace, fd);
            }
        }
    }

    info_log("end dumping debug 15\n");
}

static void dump_debug15_statistics(int fd) {
    char entry_statistics[80];
    int pid = getpid();
    snprintf(entry_statistics, sizeof(entry_statistics),
        "%-5d %zu\n", pid, gChunkHashTable.count);
    write(fd, entry_statistics, strlen(entry_statistics));
}

/*
 * Catches signal SIGUSR2 for debug 15
 */
void malloc_signal_handler(int n, siginfo_t* info, void* unused) {
    char env[PROP_VALUE_MAX];
    int malloc_debug_statistics = 0;

    int fd;
    char entry_statistics[80];
    (void)n; //avoid build warning
    (void)info; //avoid build warning
    (void)unused;
    debug_log("malloc_signal_handler\n");

    if (__system_property_get("persist.vendor.debug15.statis", env)) {
        malloc_debug_statistics = atoi(env);  // 1-on o-off
    }

    if (gDebugConfig.mSig) {
        if (malloc_debug_statistics) {
            fd = open("/data/debug15_statistics.txt", O_WRONLY|O_APPEND);
            if (fd < 0) {
                fd = open("/data/debug15_statistics.txt", O_WRONLY|O_CREAT|O_APPEND, 0666);
                if (fd < 0) {
                    error_log("couldn't open output file, errno = %d", errno);
                    return;
                }
                fchmod(fd, 0666);
                snprintf(entry_statistics, sizeof(entry_statistics),
                    "pid   chunk_count\n");
                write(fd, entry_statistics, strlen(entry_statistics));
            }

            dump_debug15_statistics(fd);

            close(fd);
        } else {
            fd = open_log_file("/data/debug15.txt");
            if (fd < 0) {
                error_log("couldn't open output file, errno = %d", errno);
                return;
            }

            dump_info(fd);

            dump_maps();

            close(fd);
        }
    }
}

void install_signal() {
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = malloc_signal_handler;
    act.sa_flags = SA_RESTART | SA_SIGINFO;
    sigemptyset(&act.sa_mask);

    sigaction(SIGUSR2, &act, NULL);
    info_log("install sigusr2\n");
}
