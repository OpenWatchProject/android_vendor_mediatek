#include <cutils/properties.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>
#include <sys/limits.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_PATH 20
#define MAXSIZE 50
#define PROP_MAPPING_MGR "vendor.debug.mapping_mgr.enable"
#define PROP_IDX_CACHE_LOG "vendor.debug.idxcache.log"
#define PROP_ATMS_DUMP "vendor.debug.atms.dump"
#define START_LOG_VALUE "2"
#define IDX_CACHE_LOG_START "1"
#define END_LOG_VALUE "0"
#define MAX_DEVICES_NUM 8

static void usage(char *name)
{
    fprintf(stderr, "Usage: %s [-f] \n", name);
    fprintf(stderr, "    -f: replay file name\n");
    fprintf(stderr, "    -c: Open IdxCache Log\n");
    fprintf(stderr, "    -x: Dump log\n");
}

void main(int argc, char *argv[]){
    unsigned int target[3];
    int64_t past_sec = 0, past_usec = 0, sleeptime = 0;
    unsigned int sec, usec, device_num, type, code, value;
    int fds[16];
    int c, i;
    struct input_event event;
    char device[MAX_PATH];
    char device_name[3];
    const char *device_path = "/dev/input/event";
    char event_log_path[MAXSIZE] = "/sdcard/";
    struct timeval start;
    struct timeval end;
    static int logging = 0;
    int cacheLog = 0, dumpLog = 0;

    opterr = 0;
    do {
        c = getopt(argc, argv, "cxf:h");
        if (c == EOF)
            break;
        switch (c) {
        case 'c':
            cacheLog = 1;
            break;
        case 'x':
            dumpLog = 1;
            break;
        case 'f':
            strncat(event_log_path, optarg, MAXSIZE - strlen(event_log_path));
            break;
        case '?':
            fprintf(stderr, "%s: invalid option -%c\n",
                argv[0], optopt);
            break;
        case 'h':
            usage(argv[0]);
            exit(1);
        }
    } while (1);

    if(event_log_path[strlen(event_log_path) - 1] == '/')
        strncat(event_log_path, "default.bin", MAXSIZE - strlen(event_log_path));

    FILE *fp = fopen(event_log_path, "rb");

    if(fp == NULL){
        printf("Fail to open file: \'%s\'\n", event_log_path);
        return;
    }

    if(fread(target, sizeof(int), 3, fp) == 0){
        printf("End of file\n");
        fclose(fp);
        fp = NULL;
        return;
    }

    for(i = 0; i < MAX_DEVICES_NUM; i++){
        strncpy(device, device_path, sizeof(device));
        snprintf(device_name, sizeof(device_name), "%d", i);
        strncat(device, device_name, MAX_PATH- strlen(device));
        fds[i] = open(device, O_RDWR);
    }

    gettimeofday(&start, NULL);

    do{
        sec = target[0] >> 23;
        usec = ((target[0] << 9) >> 12);
        device_num = target[0] & 0x00000007;
        type = target[1] >> 16;
        code = target[1] & 0x0000FFFF;
        value = target[2];
        sleeptime = (sec * 1000000LL + usec) - (past_sec * 1000000LL + past_usec);
        while(sleeptime >= 1000000){
            sleeptime -= 1000000;
            sleep(1);
        }
        usleep(sleeptime);

        past_sec = sec;
        past_usec = usec;

        // printf("[%04d.%05d] %s: %04x %04x %08x\n", sec, usec, device, type, code, value);

        // Start/End Log Event
        if(device_num == 0 && type == 0xFFFF && code == 0xFFFF && value == 0xFFFFFFFF){
            if(logging == 0){
                logging = 1;
                printf("Log Starts at [%03d.%06d]\n", sec, usec);
                int ret = (cacheLog) ? property_set(PROP_IDX_CACHE_LOG, IDX_CACHE_LOG_START) : property_set(PROP_MAPPING_MGR, START_LOG_VALUE);
                if (dumpLog) {
                    ret = property_set(PROP_ATMS_DUMP, "1");
                }
                char *set_result = (ret == 0) ? "Succeed in setting property on." : "Fail to set property.";
                // printf("%s\n", set_result);
            }
            else if(logging == 1){
                logging = 0;
                printf("Log Ends at [%03d.%06d]\n", sec, usec);
                int ret = (cacheLog) ? property_set(PROP_IDX_CACHE_LOG, END_LOG_VALUE) : property_set(PROP_MAPPING_MGR, END_LOG_VALUE);
                if (dumpLog) {
                    ret = property_set(PROP_ATMS_DUMP, "0");
                }
                char *set_result = (ret == 0) ? "Succeed in setting property off." : "Fail to set property.";
                // printf("%s\n", set_result);
            }
            continue;
        }

        // Create Event
        memset(&event, 0, sizeof(event));
        event.type = type;
        event.code = code;
        event.value = value;

        int ret = write(fds[device_num], &event, sizeof(event));
        if(ret < (ssize_t) sizeof(event)) {
            printf("Write event failed, %s\n", strerror(errno));
            fclose(fp);
            fp = NULL;
            return;
        }

    }while( fread(target, sizeof(int), 3, fp) );

    gettimeofday(&end, NULL);

    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }


    printf("Record Time: %.6f\n", (double)(past_sec * 1000000LL + past_usec) / 1000000.0);
    printf("Replay Time: %.6f\n", (double)((end.tv_sec * 1000000LL + end.tv_usec) - (start.tv_sec * 1000000LL + start.tv_usec)) / 1000000.0);

}
