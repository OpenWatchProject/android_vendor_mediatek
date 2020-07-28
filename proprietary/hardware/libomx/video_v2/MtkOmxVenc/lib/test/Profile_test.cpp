
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <android/log.h>

#include "Profiler.h"

#define LOG_TAG "ProfileTest"
#define ALOG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

void profilerTest(int testCount, int delay)
{
    bool error = false;

    ALOG("+ Starting test %s(%d,%d): ", __func__, testCount, delay);

    MtkVenc::Profiler<int> prof;

    for(int i=0; i<testCount; i++)
    {
        sleep(1);
        int64_t t = prof.profile(i);
        ALOG(" > prof.profile(%d) (%lld) \n", i, t);
    }

    ALOG("verifying: \n");

    for(int i=0; i<testCount; i++)
    {
        if(prof.tagDiff(i)<=0) {
            ALOG("tagDiff %d (%ld) FAILED\n", i, prof.tagDiff(i));
            error = true;
        }

        for(int j=0; j<testCount; j++)
        {
            if(i==j) {
                if(prof.tagDiff(i,j)!=0) {
                    ALOG("tagDiff %d,%d (%ld) FAILED", i, j, prof.tagDiff(i,j));
                    error = true;
                }
            }
            else {
                if(prof.tagDiff(i,j)<=0) {
                    ALOG("tagDiff %d,%d (%ld) FAILED", i, j, prof.tagDiff(i,j));
                    error = true;
                }
            }
        }
    }
    if(!error)
        ALOG("PASSED\n");
}

int main(int argc, char *argv[])
{
    int testCount = 3;
    int delay = 0xffff;

    if(argc > 1) testCount = atoi(argv[1]);
    if(argc > 2) delay = atoi(argv[2]);

    profilerTest(testCount, delay);
}