#include <utils/Thread.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include "PicDetect.h"
#include "LogUtils.h"
#include "aw_portrait_bokeh.h"
#include "aw_portrait_bokeh_opengl.h"
#include "aw_portrait_common.h"
#include "aw_portrait_mask.h"
#include "BufferUtils.h"

#define LOG_TAG "Portrait/PicDetect"

using namespace NSCam::NSPipelinePlugin;
PicDetect::PicDetect() {
}

PicDetect::~PicDetect() {
}

void PicDetect::init() {
    FUNCTION_IN;
    this -> maskResultForPreview = (unsigned char *)malloc(MASK_WIDTH*MASK_HEIGHT*1.5);
    MSize inSize = in->getImgSize();
    imageWidth = FACE_DETECT_WIDTH;
    imageHeight = (FACE_DETECT_WIDTH*inSize.h)/inSize.w;
    int resize_buffer_size = imageWidth*imageHeight*1.5;
    MSize size_face_detect(imageWidth, imageHeight);
    if (this -> inBuffer == nullptr) {
        this -> inBuffer = (unsigned char *)malloc(resize_buffer_size);
    }
    if (mInputWorkingBuffer == nullptr) {
        mInputWorkingBuffer = BufferUtils::acquireWorkingBuffer(size_face_detect, eImgFmt_YV12);
    }
    if (mInputWorkingBuffer != nullptr) {
        BufferUtils::mdpResizeAndConvert(in, mInputWorkingBuffer);
    }
    if (this -> inBuffer != nullptr && mInputWorkingBuffer != nullptr) {
        memcpy((void *)(this->inBuffer), (void *)(mInputWorkingBuffer->getBufVA(0)), resize_buffer_size);
    }
    FUNCTION_OUT;
}

void PicDetect::uninit() {
    FUNCTION_IN;
    free(this -> inBuffer);
    free(this -> maskResultForPreview);
    if (mMasks != nullptr) {
        mMasks->release();
    }
    FUNCTION_OUT;
}

bool PicDetect::processing() {
    if (mMasks == nullptr) {
        mMasks = new AwPortraitMask();
        char *dir = modelDir.data();
        mMasks->init(dir, 128, 320);
        mMasks->setLogable(true);
    }
    MY_LOGD("PicDetect send request imageWidth = %d,imageHeight = %d,orientation = %d",
        this -> imageWidth, this -> imageHeight, this -> orientation);
    mMasks->detectBuffer(this -> inBuffer,
            this -> imageWidth,
            this -> imageHeight,
            PIX_FMT_YUVI420,
            this -> orientation,
            this -> maskResultForPreview,
            MASK_WIDTH,
            MASK_HEIGHT);
    return true;
}