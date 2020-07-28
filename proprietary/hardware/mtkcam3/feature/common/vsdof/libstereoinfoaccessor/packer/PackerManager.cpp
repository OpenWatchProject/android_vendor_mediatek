//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/PackerManager"

#include "PackerManager.h"
#include "StereoLog.h"
#include "XmpPacker.h"
#include "CustomizedDataPacker.h"
#include "JpgPacker.h"
#include <utils/Trace.h>

using namespace stereo;

#define ATRACE_TAG ATRACE_TAG_ALWAYS

StereoBuffer_t PackerManager::pack(PackInfo *packInfo) {
    ATRACE_NAME(">>>>PackerManager-pack");
    StereoLogD("<pack>");
    // 1. append xmp format
    XmpPacker xmpPacker(packInfo);
    xmpPacker.pack();
    // 2. append cust format
    CustomizedDataPacker custDataPacker(packInfo);
    custDataPacker.pack();
    // 3. append jpg format
    JpgPacker jpgPacker(packInfo);
    jpgPacker.pack();

    return packInfo->packedJpgBuf;
}

PackInfo* PackerManager::unpack(StereoBuffer_t &src) {
    ATRACE_NAME(">>>>PackerManager-unpack");
    StereoLogD("<unpack>");
    PackInfo *packInfo = new PackInfo();
    packInfo->packedJpgBuf = src;
    // 1. split jpg format
    JpgPacker jpgPacker(packInfo);
    jpgPacker.unpack();
    // 2. split xmp format
    XmpPacker xmpPacker(packInfo);
    xmpPacker.unpack();
    // 3. split cust format
    CustomizedDataPacker custDataPacker(packInfo);
    custDataPacker.unpack();

    return packInfo;
}

