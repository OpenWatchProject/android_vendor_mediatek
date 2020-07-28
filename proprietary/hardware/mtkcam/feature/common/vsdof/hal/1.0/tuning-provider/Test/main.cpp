#include <limits.h>
#include <gtest/gtest.h>

const char *OUT_PATH = "/sdcard/tuning/";

int main(int argc, char **argv)
{
    struct stat st;
    if(stat(OUT_PATH, &st) == -1) {
        mkdir(OUT_PATH, 0755);
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}