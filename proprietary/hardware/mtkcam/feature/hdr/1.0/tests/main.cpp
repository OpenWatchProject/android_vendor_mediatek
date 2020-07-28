#define LOG_TAG "EffectHal_test"

#include <gtest/gtest.h>
#include <unistd.h>

int testVar1;
int testVar2;

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    int opt;
    testVar1 = 5;
    testVar2 = 5;
    while((opt = getopt(argc, argv, "nt:")) != -1) {
        switch(opt) {
        case 'n':
            testVar1 = 1;
            break;
        case 't':
            testVar2 = atoi(optarg);
            break;
        }
    }

    return RUN_ALL_TESTS();
}

