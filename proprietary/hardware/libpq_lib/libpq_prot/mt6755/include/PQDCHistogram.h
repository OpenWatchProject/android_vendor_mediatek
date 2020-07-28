#ifndef DYNCHIST_H_
#define DYNCHIST_H_

#include <stdio.h>

#define ADL_ANDROID_PLATFORM

#define DCHIST_INFO_NUM 20

// Input structure of Dynamic Contrast for still image
struct DynCInput {
  unsigned char * pSrcFB; // Input frame (1D array)
  int iWidth; // Input frame width
  int iHeight; // Input frame height
};

// Output structure of Dynamic Contrast for still image
struct DynCOutput {
  unsigned int Info[DCHIST_INFO_NUM];
};
// Dynamic Contrast FW registers
struct DynCReg {
  int w1_param1;
  int w1_param2;
  int w1_param3;
  int w1_param4;
};

class CPQDCHistogram
{
private:
    void Initialize();

    bool DebugEnable;

public:
    CPQDCHistogram();
    ~CPQDCHistogram();
    void Main(const DynCInput &input, DynCOutput * output);

    DynCReg * pDynCReg;

#ifndef ADL_ANDROID_PLATFORM
    FILE * ifp1;
#endif

};
#endif
