#ifndef PERFORMANCE_TEST_H
#define PERFORMANCE_TEST_H

const char* NS_GFOCUS = "http://ns.google.com/photos/1.0/focus/";
const char* PRIFIX_GFOCUS = "GFocus";
const char* NS_GIMAGE = "http://ns.google.com/photos/1.0/image/";
const char* PRIFIX_GIMAGE = "GImage";
const char* NS_GDEPTH = "http://ns.google.com/photos/1.0/depthmap/";
const char* PRIFIX_GDEPTH = "GDepth";

typedef struct GoogleStereoInfo {
    double focusBlurAtInfinity;
    double focusFocalDistance;
    double focusFocalPointX;
    double focusFocalPointY;
    double depthNear;
    double depthFar;
    int clearImageSize;
    int depthMapSize;
    char* imageMime;
    char* depthFormat;
    char* depthMime;
    char* clearImage;
    char* depthMap;
} GoogleStereoInfo;

namespace stereo {

class PerformanceTest {

public:
    void testWriteClearImage(const char*, int len);
    int testReadClearImage(char** clearImage);
    void testWriteGDepth(const GoogleStereoInfo* info);
    void testReadGDepth(GoogleStereoInfo* info);
    void dumpBufferToFile(const char* fileName, char* buffer, int size);
};

}

long getCurrentTime();

#endif