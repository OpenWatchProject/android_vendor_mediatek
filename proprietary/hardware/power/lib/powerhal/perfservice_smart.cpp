
#define LOG_TAG "libPowerHal"

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <cutils/properties.h>
#include <utils/Log.h>
#include "perfservice.h"
#include "common.h"
#include <linux/sched.h>
#include <errno.h>

/**************/
/* definition */
/**************/
#define VERBOSE 1

#define PACK_NUM_MAX    128
#define PACK_NAME_MAX   128

#define CUS_BENCHMARK_TABLE         "/data/vendor/powerhal/smart"
#define CUS_GAME_TABLE              "/data/vendor/powerhal/game"

/*************/
/* data type */
/*************/
typedef struct tPackNode {
    char pack_name[PACK_NAME_MAX];
    int  pid;
    int  uid;
} tPackNode;

typedef struct tPackList {
    tPackNode list[PACK_NUM_MAX];
    int num;
} tPackList;

/*******************/
/* global variable */
/*******************/
static tPackList gtBenchmarkList;
static tPackList gtGameList;
static tPackNode gtCurrPack;

static int nTableIsReady = 0;

/******************/
/* local function */
/******************/
static int add_new_pack(int type, tPackNode *curr, int set) // set:1 => write to file
{
    tPackList *pList = NULL;
    int num = 0;
    char file[64] = "\0";
    FILE *ifp;

    if(type < 0 || type > APK_GAME)
        return -1;

    if(type == APK_GAME) {
        pList = &gtGameList;
        set_str_cpy(file, CUS_GAME_TABLE, 64);
    }
    else {
        pList = &gtBenchmarkList;
        set_str_cpy(file, CUS_BENCHMARK_TABLE, 64);
    }

    num = pList->num;

    ALOGI("[add_new_pack] type:%d, num:%d, set:%d, pack:%s", type, num, set, curr->pack_name);

    if(num >= PACK_NUM_MAX)
        return -1;

    set_str_cpy(pList->list[num].pack_name, curr->pack_name, PACK_NAME_MAX);
    pList->num += 1;

    /* write to file */
    if(set) {
        if((ifp = fopen(file,"a")) == NULL) {
            char *err_str = strerror(errno);
            ALOGI("[add_new_pack] open file fail, err:%d, %s", errno, err_str);
            return 0;
        }

        fprintf(ifp, "%s\n", curr->pack_name);
        fclose(ifp);
    }

    return 0;
}

/*******************/
/* global function */
/*******************/
int smart_init(void)
{
    int i = 0;
    /*FILE *ifp;
    tPackNode curr;
    char buf[256];
    */

    gtBenchmarkList.num = 0;
    for(i=0; i<PACK_NUM_MAX; i++) {
        gtBenchmarkList.list[i].pack_name[0] = '\0';
        gtBenchmarkList.list[i].pid = -1;
        gtBenchmarkList.list[i].uid = -1;
    }

    /* mkdir */
    /*if(mkdir("/data/tmp", S_IRWXU | S_IRWXG | S_IROTH)!=0) {
        ALOGD("[smart_init] mkdir fail");
    }*/ /*--mkdir in rc file--*/

    /* benchmark: read from file */
    /* move to smart_table_init */
    return 0;
}

int smart_table_init_flag(void)
{
    return nTableIsReady;
}

int smart_table_init(void)
{
    /*int i;*/
    FILE *ifp;
    tPackNode curr;
    char buf[256];

    if(nTableIsReady)
        return 1;

    memset(&curr, 0, sizeof(tPackNode));

    /* benchmark: read from file */
    do {
        if((ifp = fopen(CUS_BENCHMARK_TABLE,"r")) == NULL) {
            ALOGI("[smart_table_init] open file fail");
            break;
        }

        ALOGI("[smart_table_init] open benchmark file");

        while(fgets(buf, 256, ifp)){
            sscanf(buf, "%127s", curr.pack_name);
            add_new_pack(APK_BENCHMARK, &curr, 0);
        }
        fclose(ifp);
    } while(0);

    /* GAME: read from file */
    do {
        if((ifp = fopen(CUS_GAME_TABLE,"r")) == NULL) {
            ALOGI("[smart_table_init] open file fail");
            break;
        }

        ALOGI("[smart_table_init] open game file");
        while(fgets(buf, 256, ifp)){
            sscanf(buf, "%127s", curr.pack_name);
            add_new_pack(APK_GAME, &curr, 0);
        }
        fclose(ifp);
    } while(0);

    nTableIsReady = 1;

    return 1;
}

int smart_reset(const char *pack, int pid)
{
    //smart_control(1); // don't enable detection in this func
    //smart_reset_flags();

    /* reset current package */
    strncpy(gtCurrPack.pack_name, pack, PACK_NAME_MAX-1);
    gtCurrPack.pack_name[PACK_NAME_MAX-1] = '\0';
    //gtCurrPack.total_score = 0;
    gtCurrPack.pid = pid;
    //memset(gtCurrPack.score, 0, sizeof(int)*INDEX_NUM);
    return 0;
}

int smart_check_pack_existed(int type, const char *pack, int uid, int fromUid)
{
    int i;
    tPackList *pList = NULL;
    int num = 0;

    smart_table_init();

    if(type == APK_GAME) {
        pList = &gtGameList;
    }
    else if(type == APK_BENCHMARK) {
        pList = &gtBenchmarkList;
    }
    else
        return -1;

    num = pList->num;

    for(i=0; i<num; i++) {
        if(!strncmp(pList->list[i].pack_name, pack, strlen(pack))) {
            ALOGD("[smart_check_pack_existed] type:%d, found", type);
            pList->list[i].uid = uid;
            return i;
        }
        else if(type == APK_BENCHMARK && pList->list[i].uid > 0 && pList->list[i].uid == fromUid) {  // launch from a benchmark => also enter benchmark mode
            ALOGI("[smart_check_pack_existed] type:%d, pack:%s, from uid found", type, pack);
            return i;
        }
    }
    ALOGD("[smart_check_pack_existed] pack:%s, type:%d, not found", pack, type);
    return -1;
}

int smart_add_benchmark(void)
{
    ALOGD("smart_add_benchmark");
    add_new_pack(APK_BENCHMARK, &gtCurrPack, 1);  // set:1 => write to file
    return 0;
}

int smart_add_specific_benchmark(const char *pack)
{
    tPackNode tempPack;

    if(!pack)
        return 0;

    ALOGI("smart_add_benchmark");

    strncpy(tempPack.pack_name, pack, sizeof(tempPack.pack_name)-1);
    tempPack.pid = 1; // fake pid
    tempPack.uid = 1; // fake uid

    add_new_pack(APK_BENCHMARK, &tempPack, 1);  // set:1 => write to file
    return 0;
}

int smart_add_game(void)
{
    ALOGD("smart_add_game");
    add_new_pack(APK_GAME, &gtCurrPack, 1);  // set:1 => write to file
    return 0;
}

