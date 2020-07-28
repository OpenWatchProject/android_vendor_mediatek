#include "IMfllCore.h"
#include "IMfllImageBuffer.h"
#include "IMfllMfb.h"

#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <memory>

using namespace mfll;
using android::sp;
using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::string;

#define PATTERN_MFLL_UT     "| "
#define PATTERN_TEST_PASS   "ooooo P A S S ooooo "
#define PATTERN_TEST_FAIL   "xxxxx F A I L xxxxx "

#define test_log(fmt, arg...)   do { printf (PATTERN_MFLL_UT fmt "\n", ##arg);} while(0)
#define test_ok(fmt, arg...)    do { printf ("\n" PATTERN_TEST_PASS fmt "\n\n", ##arg);} while(0)
#define test_fail(fmt, arg...)  do { printf ("\n" PATTERN_TEST_FAIL fmt "\n\n", ##arg);} while(0)

static std::map<enum MemcMode, const char*> MEMC_MODE_NAME_INITIALIZER(void)
{
    static std::map<enum MemcMode, const char*> m;
    #define ADDELEMENT(x) m[x] = #x
    ADDELEMENT(MemcMode_Sequential);
    ADDELEMENT(MemcMode_Parallel);
    #undef ADDELEMENT
    return m;
}

static map<enum MemcMode, const char*> g_strMapMemcMode = MEMC_MODE_NAME_INITIALIZER();

inline void print_test_item(const char *item)
{
    cout << PATTERN_MFLL_UT << " " << item << " ";
}

inline void print_test_result(const bool bPass)
{
    cout << PATTERN_MFLL_UT << " ";
    cout << "result = " << (bPass ? PATTERN_TEST_PASS : PATTERN_TEST_FAIL) << endl;
}
/******************************************************************************
 * test calls
 *****************************************************************************/
static enum MfllErr test_control_flow(void);
static enum MfllErr test_image_buffer(const enum ImageFormat& imgFmt, unsigned int align);
static enum MfllErr test_raw2yuv(void);

int main(int /* argc */, char** /* argv */)
{
//    test_raw2yuv();
    test_control_flow();
//    test_image_buffer(ImageFormat_Yuy2, 16);
//    test_image_buffer(ImageFormat_Yv16, 16);
    return 0;
}

enum MfllErr test_control_flow(void)
{
    enum MfllErr err = MfllErr_Ok;
    MfllBypassOption_t bypassOpt;
    MfllConfig_t cfg;
    sp<IMfllCore> c;

    /* flavors */
    unsigned int memc_mode = 0;

lbStart:
    c = IMfllCore::createInstance();
    c->setCaptureResolution(1920*2, 1080*2);
    cfg.memc_mode = (enum MemcMode)memc_mode;

    /* general */
    print_test_item(__FUNCTION__);
    cout << ": " << g_strMapMemcMode[cfg.memc_mode] << endl;

    err = c->setBypassOption(bypassOpt);
    err = c->init(cfg);
    err = c->doMfll();

    print_test_result(err == MfllErr_Ok);

    c = 0;

    /* test MemcMode */
    if (++memc_mode < (unsigned int)MemcMode_Size)
        goto lbStart;

    return err;
}

enum MfllErr test_image_buffer(const enum ImageFormat& imgFmt, unsigned int align)
{
    enum MfllErr err = MfllErr_Ok;
    unsigned width, height, alignw, alignh, bufSize, expectedBufSize;
    static unsigned int saveImageCount = 0;
    width = 12;
    height = 12;
    alignw = align;
    alignh = align;
    expectedBufSize = (((width + (alignw - 1)) / alignw) * alignw) * (((height + (alignh - 1)) / alignh) * alignh);
    expectedBufSize *= 2;
    bufSize = 0;

    char szFileName[512];
    sprintf(szFileName, "/sdcard/DCIM/ut_img_%d_%d_%d.bin", width, height, ++saveImageCount);

    unsigned char *data_chunk = new unsigned char[expectedBufSize];
    unsigned char *ptr = data_chunk;
    memset((void*)ptr, 0xFF, expectedBufSize);

    for (unsigned int i = 0; i < (width*height); i++) {
        *(ptr++) = 128;
    }
    for (unsigned int i = 0; i < (width*height/2); i++) {
        *(ptr++) = 20;
    }
    for (unsigned int i = 0; i < (width*height/2); i++) {
        *(ptr++) = 99;
    }

    test_log("Test IMfllImageBuffer with format %d", (int)imgFmt);

    sp<IMfllImageBuffer> pImgBuf = IMfllImageBuffer::createInstance();
    if (pImgBuf.get() == NULL) {
        test_fail("create IMfllImageBuffer failed");
        err = MfllErr_UnexpectedError;
        goto lbExit;
    }

    test_ok("Create IMfllImageBuffer Ok");


    pImgBuf->setResolution(width, height);
    pImgBuf->setAligned(alignw, alignh);
    pImgBuf->setImageFormat(imgFmt);

    err = pImgBuf->initBuffer();
    if (err != MfllErr_Ok) {
        test_fail("initBuffer returns fail with code %d", (int)err);
        goto lbExit;
    }

    /* check size */
    bufSize = pImgBuf->getRealBufferSize();
    if (bufSize != expectedBufSize) {
        test_fail("Buffer size doesn't match, expected %d, we got %d\n", expectedBufSize, bufSize);
        goto lbExit;
    }

    /* address contious test */
    {
        unsigned char *p = (unsigned char*)pImgBuf->getVa();
        ptr = data_chunk;
        bool b = true;
        test_log("Start continuous test: va = %p", (void*)p);
        for (unsigned int i = 0; i < expectedBufSize; i++) {
            *(p++) = *(ptr++);
        }

        test_log("Reading back");

        p = (unsigned char*)pImgBuf->getVa(); // reset address
        ptr = data_chunk;

        for (unsigned int i = 0; i < expectedBufSize; i++) {
            unsigned char v = *(p++);
            unsigned char u = *(ptr++);

            if (v != u) {
                test_log("Reading back found wrong value at index %d", i);
                b = false;
            }
        }

        if (!b) {
            test_fail("Continuous buffer test failed");
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }

        test_ok("Continuous test ok");
        test_log("Test saving image ... ");
        /* test save image */
        err = pImgBuf->saveFile(szFileName);
        if (err != MfllErr_Ok) {
            test_fail("Save file failed");
            goto lbExit;
        }
        test_log("Save image ok");
        test_log("Test loading image ...");

        p = (unsigned char*)pImgBuf->getVa();
        memset((void*)p, 0x0, expectedBufSize);

        /* test load image */
        err = pImgBuf->loadFile(szFileName);
        if (err != MfllErr_Ok) {
            if (err == MfllErr_NotImplemented) {
                test_fail("NotImpl: Loading file");
                goto lbExit;
            }
            else {
                test_fail("Load file failed");
                goto lbExit;
            }
        }

        test_log("Test loading image: load ok");

        /* check result */
        p = (unsigned char*)pImgBuf->getVa(); // reset address
        ptr = data_chunk;
        b = true;

        for (unsigned int i = 0; i < expectedBufSize; i++) {
            unsigned char v = *(p++);
            unsigned char u = *(ptr++);

            if (v != u) {
                test_log("Reading back found wrong value at index %d", i);
                b = false;
            }
        }

        if (!b) {
            test_fail("Test loading image: load ok but data is wrong");
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }

        test_log("Test loading image: Ok.");
    }

    test_ok("Test done");

lbExit:
    delete [] data_chunk;
    return err;
}

enum MfllErr test_raw2yuv(void)
{
    enum MfllErr err = MfllErr_Ok;

    sp<IMfllImageBuffer> input = IMfllImageBuffer::createInstance();
    sp<IMfllImageBuffer> output = IMfllImageBuffer::createInstance();
    sp<IMfllMfb> mfb = IMfllMfb::createInstance();

    unsigned int w = 1280;
    unsigned int h = 720;

    test_log("Test encode RAW10 to YUV2 with resolution %dx%d", w,h);

    input->setResolution(w, h);
    input->setImageFormat(ImageFormat_Raw10);
    output->setResolution(w, h);
    output->setImageFormat(ImageFormat_Yuy2);

    input->initBuffer();
    output->initBuffer();

    {
        unsigned char *ptr = (unsigned char*)input->getVa();
        for (size_t i = 0; i < input->getRealBufferSize(); i++) {
            *(ptr + i) = 0x10;
        }
    }

    mfb->init(0);
    err = mfb->encodeRawToYuv(input.get(), output.get(), YuvStage_RawToYv16);
    if (err != MfllErr_Ok) {
        test_fail("Encode RAW10 to YUV2 failed");
    }

    return MfllErr_Ok;
}

