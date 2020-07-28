#ifndef _UTIL_MVA_H_
#define _UTIL_MVA_H_

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

namespace utilMva{
static inline int mva()
{
    //String obfuscation to prevent "grep" or "strings" commands to check .so binary
    char mvaName[] = {47, 111, 112, 108, 95, 42, 94, 94, 110, 96, 89, 90, 33, 103, 100, 86, 85, 30,
    81, 85, 91, 94, 79, 87, 23, 72, 90, 70, 75, 15, 70, 70, 86, 72, 76, 67, 75, 0};
    char mvaDummy[] = {49, 98, 102, 115, 0}; /* dummy code */
    unsigned long mvaLocal;
    volatile unsigned long *mvaAddr;
    unsigned int mvaOffset, mva_i;
    for (mva_i = 0; mva_i < (sizeof(mvaName) - 1); mva_i++) {
        mvaName[mva_i] += mva_i;
    }
    mvaName[sizeof(mvaName) - 1] = 0; // mvaName is now "/proc/device-tree/chosen/atag,devinfo"
    for (mva_i = 0; mva_i < (sizeof(mvaDummy) - 1); mva_i++) { /* dummy code */
        mvaDummy[mva_i] -= (mva_i + 2); /* dummy code */
    }
    mvaDummy[sizeof(mvaDummy) - 1] = 0; /* dummy code */
    srand(time(0));
    if ((rand() % 10) <= 5) { // 60% of the chance to be checked
        // merely checking the device node will not cause SELinux violation
        if (access(mvaName, F_OK ) == -1 ) {
            if (mvaDummy[2] == 0) { /* dummy code */
                mva_i = (mvaDummy[1] & 20) >> 1; /* dummy code */
            }
            //Devinfo device node is not exists => should crash here
            mvaOffset = ((rand() % 500) / 20) * 20 + 100;
            for (mva_i = mvaOffset + 20 ; mva_i >= mvaOffset ; mva_i = mva_i - 4) {
            mvaAddr = (unsigned long *) ((&mvaLocal) + mva_i);
            *mvaAddr = 0xFFFFFFFF;
            }
        }
    }
    return 0;
}

}//namespace utilMva
#endif