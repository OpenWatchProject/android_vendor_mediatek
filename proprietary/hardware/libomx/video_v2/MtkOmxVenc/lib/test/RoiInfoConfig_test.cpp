
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>

#include <android/log.h>

//#define DEBUG_ROIINFO
#include "RoiInfoConfig.h"

#define LOG_TAG "RoiInfoConfigTest"
#define PRINTF(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

//#define DEBUG_ROIINFOCONFIGTEST
#ifdef DEBUG_ROIINFOCONFIGTEST
#define ALOG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define IN_FUNC() ALOG("+ %s", __func__)
#define OUT_FUNC() ALOG("- %s", __func__)
#define PROP() ALOG(" --> %s: %d", __func__, __LINE__)
#else
#define ALOG(...)
#define IN_FUNC()
#define OUT_FUNC()
#define PROP()
#endif

#define ASSERT(COND, VA...) \
    if(!(COND)) {\
        ALOG("[UT_FAIL] " VA); \
        return __LINE__; \
    }
#define ASSERT_NORET(COND, VA...) \
    if(!(COND)) {\
        ALOG("[UT_FAIL] " VA); \
    }

#define timespecsub(tsp, usp, vsp)                                      \
    do {                                                            \
            (vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;          \
            (vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;       \
            if ((vsp)->tv_nsec < 0) {                               \
                    (vsp)->tv_sec--;                                \
                    (vsp)->tv_nsec += 1000000000L;                  \
            }                                                       \
    } while (/* CONSTCOND */ 0)

inline void getTime(struct timespec* tv)
{
    tv->tv_sec = 0;
    tv->tv_nsec = 0;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, tv);
}

inline void diffNowTime(struct timespec* dest, struct timespec* t1)
{
    struct timespec now;
    getTime(&now);
    timespecsub(&now, t1, dest);
}


typedef int (*TestFunc)();
struct testCase {
    const char* name;
    TestFunc func;
};

int falseAlarmTest();
int basicSingleInfoKeyMap();
int basicSingleInfoKeyMap_Base64();
int basicSingleInfoMultiKeyMap();
int basicSingleInfoMultiKeyMap_Base64();
int basicMultiInfoKeyMap_Base64();
int workerMultiInfoKeyMap();
int workerMultiInfoKeyMap_Base64();

struct testCase testCases[] = {
    {"falseAlarmTest", falseAlarmTest}
    ,{"basicSingleInfoKeyMap", basicSingleInfoKeyMap}
    ,{"basicSingleInfoKeyMap_Base64", basicSingleInfoKeyMap_Base64}
    ,{"basicSingleInfoMultiKeyMap", basicSingleInfoMultiKeyMap}
    ,{"basicSingleInfoMultiKeyMap_Base64", basicSingleInfoMultiKeyMap_Base64}
    ,{"basicMultiInfoKeyMap_Base64", basicMultiInfoKeyMap_Base64}
    ,{"workerMultiInfoKeyMap", workerMultiInfoKeyMap}
    ,{"workerMultiInfoKeyMap_Base64", workerMultiInfoKeyMap_Base64}
};

int main(int argc, char *argv[])
{
    int testCount = argc;
    int testCaseSize = sizeof(testCases)/sizeof(struct testCase);
    PRINTF("Testing %d cases... (max %d)", argc, testCaseSize);

    for(int i=1; i<testCount; i++)
    {
        int testCaseIndex = atoi(argv[i]);
        if(testCaseIndex < 0 || testCaseIndex >= testCaseSize) continue;

        PRINTF("Test: %d %s %p", testCaseIndex, testCases[testCaseIndex].name, testCases[testCaseIndex].func);

        struct timespec t1, diff;
        getTime(&t1);
        int err = testCases[testCaseIndex].func();
        diffNowTime(&diff, &t1);

        if(err == 0)
            PRINTF("Test case %s - PASSED (used time: %lld.%.9ld)", testCases[testCaseIndex].name, (long long)diff.tv_sec, diff.tv_nsec);
        else
            PRINTF("Test case %s - FAILED at line: %d (used time: %lld.%.9ld)", testCases[testCaseIndex].name, err, (long long)diff.tv_sec, diff.tv_nsec);
    }
}

using namespace MtkVenc;

int falseAlarmTest()
{
    unsigned short roiInfo[] = {0,0,79,44,50, 1,1,78,43,49, 2,2,77,42,48};
    // char* roiInfo = "AAAAAE8ALAAyAA==";
    int key = 1;

    unsigned char map1[80*45];
    unsigned char* pMap1 = NULL;

    // roi map 1280x720 = 80x45
    MtkRoiConfig<int, MtkRoiConfigEntry>* config1 = new MtkRoiConfig<int, MtkRoiConfigEntry>(80,45,false);

    config1->resetInfoSession();
    config1->giveRoiInfo(3, roiInfo);
    config1->giveRoiMap(map1);

    config1->giveBuffer(key);
    pMap1 = config1->getBufferRoiMap(key);
    config1->ungiveBuffer(key);

    delete config1;

    ASSERT_NORET(pMap1[0] != 50, "map1 value[0][0] is %d, should be 50 (false alarm)", pMap1[0]);
    ASSERT_NORET(pMap1[46] == 50, "map1 value[1][1] is %d, should not be 50 (false alarm)", pMap1[46]);
    ASSERT_NORET(pMap1[92] == 50, "map1 value[2][2] is %d, should not be 50 (false alarm)", pMap1[92]);

    ASSERT_NORET(pMap1[0] == 49, "map1 value[0][0] is %d, should be not 49 (false alarm)", pMap1[0]);
    ASSERT_NORET(pMap1[46] != 49, "map1 value[1][1] is %d, should be 49 (false alarm)", pMap1[46]);
    ASSERT_NORET(pMap1[92] == 49, "map1 value[2][2] is %d, should not be 49 (false alarm)", pMap1[92]);

    ASSERT_NORET(pMap1[0] == 48, "map1 value[0][0] is %d, should not be 48 (false alarm)", pMap1[0]);
    ASSERT_NORET(pMap1[46] == 48, "map1 value[1][1] is %d, should not be 48 (false alarm)", pMap1[46]);
    ASSERT_NORET(pMap1[92] != 48, "map1 value[2][2] is %d, should be 48 (false alarm)", pMap1[92]);

    return 0;
}

int basicSingleInfoKeyMap()
{
    IN_FUNC();
    // tlbrv
    unsigned short roiInfo[] = {0,0,79,44,50};
    // char* roiInfo = "AAAAAE8ALAAyAA==";
    int key = 1;

    unsigned char map1[80*45];
    unsigned char* pMap1 = NULL;

    // roi map 1280x720 = 80x45
    MtkRoiConfig<int, MtkRoiConfigEntry>* config1 = new MtkRoiConfig<int, MtkRoiConfigEntry>(80,45,false);

    config1->resetInfoSession();
    config1->giveRoiInfo(1, roiInfo);
    config1->giveRoiMap(map1);

    config1->giveBuffer(key);
    pMap1 = config1->getBufferRoiMap(key);
    config1->ungiveBuffer(key);

    delete config1;

    ASSERT(pMap1 != NULL, "map1 address NULL");
    ASSERT(pMap1[0] == 50, "map1 value[0][0] is %d, should be 50", pMap1[0]);
    ASSERT(pMap1[44] == 50, "map1 value[0][44] is %d, should be 50", pMap1[44]);
    ASSERT(pMap1[3555] == 50, "map1 value[79][0] is %d, should be 50", pMap1[3555]);
    ASSERT(pMap1[3599] == 50, "map1 value[79][44] is %d, should be 50", pMap1[3599]);

    PROP();

    unsigned char map2[80*45];
    unsigned char* pMap2 = NULL;

    MtkRoiConfig<int, MtkRoiConfigEntryJump>* config2 = new MtkRoiConfig<int, MtkRoiConfigEntryJump>(80,45,false);

    config2->resetInfoSession();
    config2->giveRoiInfo(1, roiInfo);
    config2->giveRoiMap(map2);

    config2->giveBuffer(key);
    pMap2 = config2->getBufferRoiMap(key);
    config2->ungiveBuffer(key);

    delete config2;

    ASSERT(pMap2 != NULL, "map2 address NULL");
    ASSERT(pMap2[0] == 50, "map2 value[0][0] is %d, should be 50", pMap2[0]);
    ASSERT(pMap2[44] == 50, "map2 value[0][44] is %d, should be 50", pMap2[44]);
    ASSERT(pMap2[3555] == 50, "map2 value[79][0] is %d, should be 50", pMap2[3555]);
    ASSERT(pMap2[3599] == 50, "map2 value[79][44] is %d, should be 50", pMap2[3599]);

    OUT_FUNC();
    return 0;
}

int basicSingleInfoKeyMap_Base64()
{
    IN_FUNC();
    // tlbrv
    //unsigned short roiInfo[] = {0,0,79,44,50};
    char* roiInfo = "AAAAAE8ALAAyAA==";
    int key = 1;

    unsigned char map1[80*45];
    unsigned char* pMap1 = NULL;

    // roi map 1280x720 = 80x45
    MtkRoiConfig<int, MtkRoiConfigEntry>* config1 = new MtkRoiConfig<int, MtkRoiConfigEntry>(80,45,false);

    config1->resetInfoSession();
    config1->giveRoiInfo(1, roiInfo);
    config1->giveRoiMap(map1);

    config1->giveBuffer(key);
    pMap1 = config1->getBufferRoiMap(key);
    config1->ungiveBuffer(key);

    delete config1;

    ASSERT(pMap1 != NULL, "map1 address NULL");
    ASSERT(pMap1[0] == 50, "map1 value[0][0] is %d, should be 50", pMap1[0]);
    ASSERT(pMap1[44] == 50, "map1 value[0][44] is %d, should be 50", pMap1[44]);
    ASSERT(pMap1[3555] == 50, "map1 value[79][0] is %d, should be 50", pMap1[3555]);
    ASSERT(pMap1[3599] == 50, "map1 value[79][44] is %d, should be 50", pMap1[3599]);

    PROP();

    unsigned char map2[80*45];
    unsigned char* pMap2 = NULL;

    MtkRoiConfig<int, MtkRoiConfigEntryJump>* config2 = new MtkRoiConfig<int, MtkRoiConfigEntryJump>(80,45,false);

    config2->resetInfoSession();
    config2->giveRoiInfo(1, roiInfo);
    config2->giveRoiMap(map2);

    config2->giveBuffer(key);
    pMap2 = config2->getBufferRoiMap(key);
    config2->ungiveBuffer(key);

    delete config2;

    ASSERT(pMap2 != NULL, "map2 address NULL");
    ASSERT(pMap2[0] == 50, "map2 value[0][0] is %d, should be 50", pMap2[0]);
    ASSERT(pMap2[44] == 50, "map2 value[0][44] is %d, should be 50", pMap2[44]);
    ASSERT(pMap2[3555] == 50, "map2 value[79][0] is %d, should be 50", pMap2[3555]);
    ASSERT(pMap2[3599] == 50, "map2 value[79][44] is %d, should be 50", pMap2[3599]);

    OUT_FUNC();
    return 0;
}

int basicSingleInfoMultiKeyMap()
{
    // tlbrv
    unsigned short roiInfo1[] = {2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,50};
    //char* roiInfo1 = "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsADIA";

    unsigned char map1[10][80*45];
    unsigned char* pMap1[10] = {0};

    // roi map 1280x720 = 80x45
    MtkRoiConfig<int, MtkRoiConfigEntry>* config1 = new MtkRoiConfig<int, MtkRoiConfigEntry>(80,45,false);

    for(int key=0; key<10; key++) {
        config1->resetInfoSession();
        config1->giveRoiInfo(3, roiInfo1);
        config1->giveRoiMap(map1[key]);

        config1->giveBuffer(key);
    }

    for(int key=0; key<10; key++) {
        pMap1[key] = config1->getBufferRoiMap(key);
        config1->ungiveBuffer(key);
    }

    delete config1;

    for(int key=0; key<10; key++) {
        ASSERT(pMap1[key] != NULL, "key %d map1 address NULL", key);
        ASSERT(pMap1[key][0] == 50, "key %d map1 value[0][0] is %d, should be 50", key, pMap1[key][0]);
        ASSERT(pMap1[key][46] == 50, "key %d map1 value[1][1] is %d, should be 50", key, pMap1[key][46]);
        ASSERT(pMap1[key][92] == 50, "key %d map1 value[2][2] is %d, should be 50", key, pMap1[key][92]);
    }

    PROP();

    unsigned char map2[10][80*45];
    unsigned char* pMap2[10] = {0};

    MtkRoiConfig<int, MtkRoiConfigEntryJump>* config2 = new MtkRoiConfig<int, MtkRoiConfigEntryJump>(80,45,false);

    for(int key=0; key<10; key++) {
        config2->resetInfoSession();
        config2->giveRoiInfo(3, roiInfo1);
        config2->giveRoiMap(map2[key]);

        config2->giveBuffer(key);
    }

    for(int key=0; key<10; key++) {
        pMap2[key] = config2->getBufferRoiMap(key);
        config2->ungiveBuffer(key);
    }

    delete config2;

    for(int key=0; key<10; key++) {
        ASSERT(pMap2[key] != NULL, "key %d map2 address NULL", key);
        ASSERT(pMap2[key][0] == 50, "key %d map2 value[0][0] is %d, should be 50", key, pMap2[key][0]);
        ASSERT(pMap2[key][46] == 50, "key %d map2 value[1][1] is %d, should be 50", key, pMap2[key][46]);
        ASSERT(pMap2[key][92] == 50, "key %d map2 value[2][2] is %d, should be 50", key, pMap2[key][92]);
    }

    return 0;
}

int basicSingleInfoMultiKeyMap_Base64()
{
    // tlbrv
    //unsigned short roiInfo1[] = {2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,50};
    char* roiInfo1 = "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsADIA";

    unsigned char map1[10][80*45];
    unsigned char* pMap1[10] = {0};

    // roi map 1280x720 = 80x45
    MtkRoiConfig<int, MtkRoiConfigEntry>* config1 = new MtkRoiConfig<int, MtkRoiConfigEntry>(80,45,false);

    for(int key=0; key<10; key++) {
        config1->resetInfoSession();
        config1->giveRoiInfo(3, roiInfo1);
        config1->giveRoiMap(map1[key]);

        config1->giveBuffer(key);
    }

    for(int key=0; key<10; key++) {
        pMap1[key] = config1->getBufferRoiMap(key);
        config1->ungiveBuffer(key);
    }

    delete config1;

    for(int key=0; key<10; key++) {
        ASSERT(pMap1[key] != NULL, "key %d map1 address NULL", key);
        ASSERT(pMap1[key][0] == 50, "key %d map1 value[0][0] is %d, should be 50", key, pMap1[key][0]);
        ASSERT(pMap1[key][46] == 50, "key %d map1 value[1][1] is %d, should be 50", key, pMap1[key][46]);
        ASSERT(pMap1[key][92] == 50, "key %d map1 value[2][2] is %d, should be 50", key, pMap1[key][92]);
    }

    PROP();

    unsigned char map2[10][80*45];
    unsigned char* pMap2[10] = {0};

    MtkRoiConfig<int, MtkRoiConfigEntryJump>* config2 = new MtkRoiConfig<int, MtkRoiConfigEntryJump>(80,45,false);

    for(int key=0; key<10; key++) {
        config2->resetInfoSession();
        config2->giveRoiInfo(3, roiInfo1);
        config2->giveRoiMap(map2[key]);

        config2->giveBuffer(key);
    }

    for(int key=0; key<10; key++) {
        pMap2[key] = config2->getBufferRoiMap(key);
        config2->ungiveBuffer(key);
    }

    delete config2;

    for(int key=0; key<10; key++) {
        ASSERT(pMap2[key] != NULL, "key %d map1 address NULL", key);
        ASSERT(pMap2[key][0] == 50, "key %d map1 value[0][0] is %d, should be 50", key, pMap2[key][0]);
        ASSERT(pMap2[key][46] == 50, "key %d map1 value[1][1] is %d, should be 50", key, pMap2[key][46]);
        ASSERT(pMap2[key][92] == 50, "key %d map1 value[2][2] is %d, should be 50", key, pMap2[key][92]);
    }

    return 0;
}

int basicMultiInfoKeyMap_Base64()
{
    // tlbrv
    /*unsigned short roiInfo1[10][15] = {
        {2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,50}
        ,{0,0,79,44,50, 1,1,78,43,49, 2,2,77,42,48}
        ,{0,0,79,44,47, 1,1,78,43,46, 2,2,77,42,45}
        ,{2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,11}
        ,{2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,12}
        ,{2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,50}
        ,{0,0,79,44,50, 1,1,78,43,49, 2,2,77,42,48}
        ,{0,0,79,44,47, 1,1,78,43,46, 2,2,77,42,45}
        ,{2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,11}
        ,{2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,12}
    };*/

    char* roiInfo1[10] = {
        "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsADIA",
        "AAAAAE8ALAAyAAEAAQBOACsAMQACAAIATQAqADAA",
        "AAAAAE8ALAAvAAEAAQBOACsALgACAAIATQAqAC0A",
        "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsAAsA",
        "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsAAwA",
        "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsADIA",
        "AAAAAE8ALAAyAAEAAQBOACsAMQACAAIATQAqADAA",
        "AAAAAE8ALAAvAAEAAQBOACsALgACAAIATQAqAC0A",
        "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsAAsA",
        "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsAAwA"
    };

    unsigned char map1[10][80*45];
    unsigned char* pMap1[10] = {0};

    unsigned short result[10][3] = {
        {50,50,50}
        ,{50,49,48}
        ,{47,46,45}
        ,{11,11,11}
        ,{12,12,12}
        ,{50,50,50}
        ,{50,49,48}
        ,{47,46,45}
        ,{11,11,11}
        ,{12,12,12}
    };

    // roi map 1280x720 = 80x45
    MtkRoiConfig<int, MtkRoiConfigEntry>* config1 = new MtkRoiConfig<int, MtkRoiConfigEntry>(80,45,false);

    for(int key=0; key<10; key++) {
        config1->resetInfoSession();
        config1->giveRoiInfo(3, roiInfo1[key]);
        config1->giveRoiMap(map1[key]);

        config1->giveBuffer(key);
    }

    for(int key=0; key<10; key++) {
        pMap1[key] = config1->getBufferRoiMap(key);
        config1->ungiveBuffer(key);
    }

    delete config1;

    for(int key=0; key<10; key++) {
        ASSERT(pMap1[key] != NULL, "key %d map1 address NULL", key);
        ASSERT(pMap1[key][0] == result[key][0], "key %d map1 value[0][0] is %d, should be %d", key, pMap1[0][key], result[key][0]);
        ASSERT(pMap1[key][46] == result[key][1], "key %d map1 value[1][1] is %d, should be %d", key, pMap1[46][key], result[key][1]);
        ASSERT(pMap1[key][92] == result[key][2], "key %d map1 value[2][2] is %d, should be %d", key, pMap1[92][key], result[key][2]);
    }

    PROP();

    unsigned char map2[10][80*45];
    unsigned char* pMap2[10] = {0};

    MtkRoiConfig<int, MtkRoiConfigEntryJump>* config2 = new MtkRoiConfig<int, MtkRoiConfigEntryJump>(80,45,false);

    for(int key=0; key<10; key++) {
        config2->resetInfoSession();
        config2->giveRoiInfo(3, roiInfo1[key]);
        config2->giveRoiMap(map2[key]);

        config2->giveBuffer(key);
    }

    for(int key=0; key<10; key++) {
        pMap2[key] = config2->getBufferRoiMap(key);
        config2->ungiveBuffer(key);
    }

    delete config2;

    for(int key=0; key<10; key++) {
        ASSERT(pMap2[key] != NULL, "key %d map2 address NULL", key);
        ASSERT(pMap2[key][0] == result[key][0], "key %d map2 value[0][0] is %d, should be %d", key, pMap2[0][key], result[key][0]);
        ASSERT(pMap2[key][46] == result[key][1], "key %d map2 value[1][1] is %d, should be %d", key, pMap2[46][key], result[key][1]);
        ASSERT(pMap2[key][92] == result[key][2], "key %d map2 value[2][2] is %d, should be %d", key, pMap2[92][key], result[key][2]);
    }

    return 0;
}

int workerMultiInfoKeyMap()
{
    // tlbrv
    unsigned short roiInfo1[10][15] = {
        {2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,50}
        ,{0,0,79,44,50, 1,1,78,43,49, 2,2,77,42,48}
        ,{0,0,79,44,47, 1,1,78,43,46, 2,2,77,42,45}
        ,{2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,11}
        ,{2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,12}
        ,{2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,50}
        ,{0,0,79,44,50, 1,1,78,43,49, 2,2,77,42,48}
        ,{0,0,79,44,47, 1,1,78,43,46, 2,2,77,42,45}
        ,{2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,11}
        ,{2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,12}
    };
    /*
    char* roiInfo1[10] = {
        "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsADIA",
        "AAAAAE8ALAAyAAEAAQBOACsAMQACAAIATQAqADAA",
        "AAAAAE8ALAAvAAEAAQBOACsALgACAAIATQAqAC0A",
        "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsAAsA",
        "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsAAwA",
    };
    */

    unsigned char map1[10][80*45];
    unsigned char* pMap1[10] = {0};

    unsigned short result[10][3] = {
        {50,50,50}
        ,{50,49,48}
        ,{47,46,45}
        ,{11,11,11}
        ,{12,12,12}
        ,{50,50,50}
        ,{50,49,48}
        ,{47,46,45}
        ,{11,11,11}
        ,{12,12,12}
    };

    // roi map 1280x720 = 80x45
    MtkRoiConfig<int, MtkRoiConfigEntry>* config1 = new MtkRoiConfig<int, MtkRoiConfigEntry>(80,45,true);

    for(int key=0; key<10; key++) {
        config1->resetInfoSession();
        config1->giveRoiInfo(3, roiInfo1[key]);
        config1->giveRoiMap(map1[key]);

        config1->giveBuffer(key);
    }

    for(int key=0; key<10; key++) {
        pMap1[key] = config1->getBufferRoiMap(key);
        config1->ungiveBuffer(key);
    }

    delete config1;

    for(int key=0; key<10; key++) {
        ASSERT(pMap1[key] != NULL, "key %d map1 address NULL", key);
        ASSERT(pMap1[key][0] == result[key][0], "key %d map1 value[0][0] is %d, should be %d", key, pMap1[0][key], result[key][0]);
        ASSERT(pMap1[key][46] == result[key][1], "key %d map1 value[1][1] is %d, should be %d", key, pMap1[46][key], result[key][1]);
        ASSERT(pMap1[key][92] == result[key][2], "key %d map1 value[2][2] is %d, should be %d", key, pMap1[92][key], result[key][2]);
    }

    PROP();

    unsigned char map2[10][80*45];
    unsigned char* pMap2[10] = {0};

    MtkRoiConfig<int, MtkRoiConfigEntryJump>* config2 = new MtkRoiConfig<int, MtkRoiConfigEntryJump>(80,45,true);

    for(int key=0; key<10; key++) {
        config2->resetInfoSession();
        config2->giveRoiInfo(3, roiInfo1[key]);
        config2->giveRoiMap(map2[key]);

        config2->giveBuffer(key);
    }

    for(int key=0; key<10; key++) {
        pMap2[key] = config2->getBufferRoiMap(key);
        config2->ungiveBuffer(key);
    }

    delete config2;

    for(int key=0; key<10; key++) {
        ASSERT(pMap2[key] != NULL, "key %d map2 address NULL", key);
        ASSERT(pMap2[key][0] == result[key][0], "key %d map2 value[0][0] is %d, should be %d", key, pMap2[0][key], result[key][0]);
        ASSERT(pMap2[key][46] == result[key][1], "key %d map2 value[1][1] is %d, should be %d", key, pMap2[46][key], result[key][1]);
        ASSERT(pMap2[key][92] == result[key][2], "key %d map2 value[2][2] is %d, should be %d", key, pMap2[92][key], result[key][2]);
    }

    return 0;
}

int workerMultiInfoKeyMap_Base64()
{
    // tlbrv
    /*unsigned short roiInfo1[10][15] = {
        {2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,50}
        ,{0,0,79,44,50, 1,1,78,43,49, 2,2,77,42,48}
        ,{0,0,79,44,47, 1,1,78,43,46, 2,2,77,42,45}
        ,{2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,11}
        ,{2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,12}
        ,{2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,50}
        ,{0,0,79,44,50, 1,1,78,43,49, 2,2,77,42,48}
        ,{0,0,79,44,47, 1,1,78,43,46, 2,2,77,42,45}
        ,{2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,11}
        ,{2,2,77,42,48, 1,1,78,43,49, 0,0,79,44,12}
    };*/

    char* roiInfo1[10] = {
        "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsADIA",
        "AAAAAE8ALAAyAAEAAQBOACsAMQACAAIATQAqADAA",
        "AAAAAE8ALAAvAAEAAQBOACsALgACAAIATQAqAC0A",
        "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsAAsA",
        "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsAAwA",
        "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsADIA",
        "AAAAAE8ALAAyAAEAAQBOACsAMQACAAIATQAqADAA",
        "AAAAAE8ALAAvAAEAAQBOACsALgACAAIATQAqAC0A",
        "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsAAsA",
        "AgACAE0AKgAwAAEAAQBOACsAMQAAAAAATwAsAAwA"
    };

    unsigned char map1[10][80*45];
    unsigned char* pMap1[10] = {0};

    unsigned short result[10][3] = {
        {50,50,50}
        ,{50,49,48}
        ,{47,46,45}
        ,{11,11,11}
        ,{12,12,12}
        ,{50,50,50}
        ,{50,49,48}
        ,{47,46,45}
        ,{11,11,11}
        ,{12,12,12}
    };

    // roi map 1280x720 = 80x45
    MtkRoiConfig<int, MtkRoiConfigEntry>* config1 = new MtkRoiConfig<int, MtkRoiConfigEntry>(80,45,true);

    for(int key=0; key<10; key++) {
        config1->resetInfoSession();
        config1->giveRoiInfo(3, roiInfo1[key]);
        config1->giveRoiMap(map1[key]);

        config1->giveBuffer(key);
    }

    for(int key=0; key<10; key++) {
        pMap1[key] = config1->getBufferRoiMap(key);
        config1->ungiveBuffer(key);
    }

    delete config1;

    for(int key=0; key<10; key++) {
        ASSERT(pMap1[key] != NULL, "key %d map1 address NULL", key);
        ASSERT(pMap1[key][0] == result[key][0], "key %d map1 value[0][0] is %d, should be %d", key, pMap1[key][0], result[key][0]);
        ASSERT(pMap1[key][46] == result[key][1], "key %d map1 value[1][1] is %d, should be %d", key, pMap1[key][46], result[key][1]);
        ASSERT(pMap1[key][92] == result[key][2], "key %d map1 value[2][2] is %d, should be %d", key, pMap1[key][92], result[key][2]);
    }

    PROP();

    unsigned char map2[10][80*45];
    unsigned char* pMap2[10] = {0};

    MtkRoiConfig<int, MtkRoiConfigEntryJump>* config2 = new MtkRoiConfig<int, MtkRoiConfigEntryJump>(80,45,true);

    for(int key=0; key<10; key++) {
        config2->resetInfoSession();
        config2->giveRoiInfo(3, roiInfo1[key]);
        config2->giveRoiMap(map2[key]);

        config2->giveBuffer(key);
    }

    for(int key=0; key<10; key++) {
        pMap2[key] = config2->getBufferRoiMap(key);
        config2->ungiveBuffer(key);
    }

    delete config2;

    for(int key=0; key<10; key++) {
        ASSERT(pMap2[key] != NULL, "key %d map2 address NULL", key);
        ASSERT(pMap2[key][0] == result[key][0], "key %d map2 value[0][0] is %d, should be %d", key, pMap2[key][0], result[key][0]);
        ASSERT(pMap2[key][46] == result[key][1], "key %d map2 value[1][1] is %d, should be %d", key, pMap2[key][46], result[key][1]);
        ASSERT(pMap2[key][92] == result[key][2], "key %d map2 value[2][2] is %d, should be %d", key, pMap2[key][92], result[key][2]);
    }

    return 0;
}

