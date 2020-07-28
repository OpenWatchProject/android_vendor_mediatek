//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/StereoInfoJsonParser"

#include "StereoInfoJsonParser.h"
#include "StereoLog.h"
#include "BufferManager.h"

using namespace stereo;

StereoInfoJsonParser::StereoInfoJsonParser(const StereoBuffer_t &jsonBuffer) {
    StereoLogD("<StereoInfoJsonParser>");
    if (!jsonBuffer.isValid()) {
        StereoLogE("json buffer is nullptr");
        return;
    }
    pJsonParser = new JsonParser(jsonBuffer);
    StereoLogD("<StereoInfoJsonParser> pJsonParser = %p", pJsonParser);
}

StereoInfoJsonParser::~StereoInfoJsonParser() {
    StereoLogD("<~StereoInfoJsonParser>");
    if (pJsonParser != nullptr) {
        delete pJsonParser;
        pJsonParser = nullptr;
    }
}

int StereoInfoJsonParser::getGeoVerifyLevel() {
    return pJsonParser->getValueIntFromObject(
        &VERIFY_GEO_INFO_TAG, nullptr, &VERIFY_GEO_INFO_LEVEL);
}

int StereoInfoJsonParser::getPhoVerifyLevel() {
    return pJsonParser->getValueIntFromObject(
        &VERIFY_PHO_INFO_TAG, nullptr, &VERIFY_PHO_INFO_LEVEL);
}

int StereoInfoJsonParser::getMtkChaVerifyLevel() {
    return pJsonParser->getValueIntFromObject(&VERIFY_MTK_CHA_INFO_TAG, nullptr,
            &VERIFY_MTK_CHA_INFO_LEVEL);
}

int StereoInfoJsonParser::getJpsWidth() {
    if (mJpsWidth != -1) {
        return mJpsWidth;
    }
    mJpsWidth = pJsonParser->getValueIntFromObject(&JPSINFO_TAG, nullptr, &JPSINFO_WIDTH);
    if (mJpsWidth < 0) {
        mJpsWidth = 0;
    }
    StereoLogD("<getJpsWidth> mJpsWidth: %d", mJpsWidth);
    return mJpsWidth;
}

int StereoInfoJsonParser::getJpsHeight() {
    if (mJpsHeight != -1) {
        return mJpsHeight;
    }
    mJpsHeight = pJsonParser->getValueIntFromObject(&JPSINFO_TAG, nullptr, &JPSINFO_HEIGHT);
    if (mJpsHeight < 0) {
        mJpsHeight = 0;
    }
    StereoLogD("<getJpsHeight> mJpsHeight: %d", mJpsHeight);
    return mJpsHeight;
}

int StereoInfoJsonParser::getGoogleDepthWidth() {
    if (mGDepthWidth != -1) {
        return mGDepthWidth;
    }
    mGDepthWidth = pJsonParser->getValueIntFromObject(
            &DEPTHINFO_TAG, nullptr, &DEPTHINFO_WIDTH);
    StereoLogD("<getGoogleDepthWidth> mGDepthWidth: %d", mGDepthWidth);
    return mGDepthWidth;
}

int StereoInfoJsonParser::getGoogleDepthHeight() {
    if (mGDepthHeight != -1) {
        return mGDepthHeight;
    }
    mGDepthHeight = pJsonParser->getValueIntFromObject(
            &DEPTHINFO_TAG, nullptr, &DEPTHINFO_HEIGHT);
    StereoLogD("<getGoogleDepthHeight> mGDepthHeight: %d", mGDepthHeight);
    return mGDepthHeight;
}

int StereoInfoJsonParser::getMaskWidth() {
    if (mMaskWidth != -1) {
        return mMaskWidth;
    }
    mMaskWidth = pJsonParser->getValueIntFromObject(
            &MASKINFO_TAG, nullptr, &MASKINFO_WIDTH);
    StereoLogD("<getMaskWidth> mMaskWidth: %d", mMaskWidth);
    return mMaskWidth;
}

int StereoInfoJsonParser::getMaskHeight() {
    if (mMaskHeight != -1) {
        return mMaskHeight;
    }
    mMaskHeight = pJsonParser->getValueIntFromObject(
            &MASKINFO_TAG, nullptr, &MASKINFO_HEIGHT);
    StereoLogD("<getMaskHeight> mMaskHeight: %d", mMaskHeight);
    return mMaskHeight;
}

int StereoInfoJsonParser::getMaskSize() {
    if (mMaskSize != -1) {
        return mMaskSize;
    }
    mMaskSize = getMaskWidth() * getMaskHeight();
    StereoLogD("<getMaskSize> mMaskSize: %d", mMaskSize);
    return mMaskSize;
}

void StereoInfoJsonParser::getMaskBuffer(StereoBuffer_t &mask) {
    mMaskSize = getMaskSize();
    StereoVector<StereoVector<int>>* encodedMaskArray
        = pJsonParser->getInt2DArrayFromObject(&MASKINFO_TAG, &MASKINFO_MASK);
    if (encodedMaskArray == nullptr) {
        StereoLogD("<getMaskBuffer> Json mask array is nullptr, return!!");
        return;
    }
    decodeMaskBuffer(encodedMaskArray, mMaskSize, mask);
    delete encodedMaskArray;
}

int StereoInfoJsonParser::getPosX() {
    if (mPosX != -1) {
        return mPosX;
    }
    //mPosX = pJsonParser->getValueIntFromObject(&POSINFO_TAG, nullptr, &POSINFO_X);
    // camera will pass double value, but algo will receive int value.
    // for compatibility reason, convert double to int at here
    mPosX = (int)pJsonParser->getValueDoubleFromObject(&POSINFO_TAG, nullptr, &POSINFO_X);
    StereoLogD("<getPosX> mPosX: %d", mPosX);
    return mPosX;
}

int StereoInfoJsonParser::getPosY() {
    if (mPosY != -1) {
        return mPosY;
    }
    //mPosY = pJsonParser->getValueIntFromObject(&POSINFO_TAG, nullptr, &POSINFO_Y);
    // camera will pass double value, but algo will receive int value.
    // for compatibility reason, convert double to int at here
    mPosY = (int)pJsonParser->getValueDoubleFromObject(&POSINFO_TAG, nullptr, &POSINFO_Y);
    StereoLogD("<getPosY> mPosY: %d", mPosY);
    return mPosY;
}

int StereoInfoJsonParser::getViewWidth() {
    if (mViewWidth != -1) {
        return mViewWidth;
    }
    mViewWidth = pJsonParser->getValueIntFromObject(&VIEWINFO_TAG, nullptr, &VIEWINFO_WIDTH);
    StereoLogD("<getViewWidth> mViewWidth: %d", mViewWidth);
    return mViewWidth;
}

int StereoInfoJsonParser::getViewHeight() {
    if (mViewHeight != -1) {
        return mViewHeight;
    }
    mViewHeight = pJsonParser->getValueIntFromObject(&VIEWINFO_TAG, nullptr, &VIEWINFO_HEIGHT);
    StereoLogD("<getViewHeight> mViewHeight: %d", mViewHeight);
    return mViewHeight;
}

int StereoInfoJsonParser::getOrientation() {
    if (mOrientation != -1) {
        return mOrientation;
    }
    mOrientation = pJsonParser->getValueIntFromObject(&ORIENTATIONINFO_TAG, nullptr,
            &ORIENTATIONINFO_ORIENTATION);
    StereoLogD("<getOrientation> mOrientation: %d", mOrientation);
    return mOrientation;
}

int StereoInfoJsonParser::getDepthRotation() {
    if (mDepthRotation != -1) {
        return mDepthRotation;
    }
    mDepthRotation = pJsonParser->getValueIntFromObject(&DEPTH_ROTATION_INFO_TAG, nullptr,
            &DEPTH_ROTATION_INFO_ORIENTATION);
    StereoLogD("<getDepthRotation> mDepthRotation: %d", mDepthRotation);
    return mDepthRotation;
}

int StereoInfoJsonParser::getMainCamPos() {
    if (mMainCamPostion != -1) {
        return mMainCamPostion;
    }
    mMainCamPostion = pJsonParser->getValueIntFromObject(&MAIN_CAM_POSITION_INFO_TAG, nullptr,
            &MAIN_CAM_POSITION_INFO_POSITION);
    StereoLogD("<getMainCamPos> mMainCamPostion: %d", mMainCamPostion);
    return mMainCamPostion;
}

int StereoInfoJsonParser::getTouchCoordX1st() {
    if (mTouchCoordX1st != -1) {
        return mTouchCoordX1st;
    }
    int left = pJsonParser->getValueIntFromObject(&TOUCH_COORD_INFO_TAG, nullptr,
            &TOUCH_COORD_INFO_LEFT);
    int right = pJsonParser->getValueIntFromObject(&TOUCH_COORD_INFO_TAG, nullptr,
            &TOUCH_COORD_INFO_RIGHT);
    mTouchCoordX1st = (left + right) / 2;
    StereoLogD("<getTouchCoordX1st> mTouchCoordX1st: %d", mTouchCoordX1st);
    return mTouchCoordX1st;
}

int StereoInfoJsonParser::getTouchCoordY1st() {
    if (mTouchCoordY1st != -1) {
        return mTouchCoordY1st;
    }
    int top = pJsonParser->getValueIntFromObject(&TOUCH_COORD_INFO_TAG, nullptr,
            &TOUCH_COORD_INFO_TOP);
    int bottom = pJsonParser->getValueIntFromObject(&TOUCH_COORD_INFO_TAG, nullptr,
            &TOUCH_COORD_INFO_BOTTOM);
    mTouchCoordY1st = (top + bottom) / 2;
    StereoLogD("<getTouchCoordY1st> mTouchCoordY1st: %d", mTouchCoordY1st);
    return mTouchCoordY1st;
}

int StereoInfoJsonParser::getFocusTop() {
    int top = pJsonParser->getValueIntFromObject(
        &TOUCH_COORD_INFO_TAG, nullptr, &TOUCH_COORD_INFO_TOP);
    StereoLogD("<getFocusTop> top: %d", top);
    return top;
}

int StereoInfoJsonParser::getFocusLeft() {
    int left = pJsonParser->getValueIntFromObject(
        &TOUCH_COORD_INFO_TAG, nullptr, &TOUCH_COORD_INFO_LEFT);
    StereoLogD("<getFocusLeft> left: %d", left);
    return left;
}

int StereoInfoJsonParser::getFocusRight() {
    int right = pJsonParser->getValueIntFromObject(
        &TOUCH_COORD_INFO_TAG, nullptr, &TOUCH_COORD_INFO_RIGHT);
    StereoLogD("<getFocusRight> right: %d", right);
    return right;
}

int StereoInfoJsonParser::getFocusBottom() {
    int bottom = pJsonParser->getValueIntFromObject(
        &TOUCH_COORD_INFO_TAG, nullptr, &TOUCH_COORD_INFO_BOTTOM);
    StereoLogD("<getFocusBottom> bottom: %d", bottom);
    return bottom;
}

int StereoInfoJsonParser::getFaceRectCount() {
    if (mFaceRectCount != -1) {
        return mFaceRectCount;
    }
    mFaceRectCount = pJsonParser->getArrayLength(&FACE_DETECTION_INFO_TAG);
    StereoLogD("<getFaceRectCount> mFaceRectCount: %d", mFaceRectCount);
    return mFaceRectCount;
}

Rect* StereoInfoJsonParser::getFaceRect(int index) {
    int left = pJsonParser->getObjectPropertyValueFromArray(&FACE_DETECTION_INFO_TAG, index,
            &FACE_DETECTION_INFO_LEFT);
    int top = pJsonParser->getObjectPropertyValueFromArray(&FACE_DETECTION_INFO_TAG, index,
            &FACE_DETECTION_INFO_TOP);
    int right = pJsonParser->getObjectPropertyValueFromArray(&FACE_DETECTION_INFO_TAG, index,
            &FACE_DETECTION_INFO_RIGHT);
    int bottom = pJsonParser->getObjectPropertyValueFromArray(&FACE_DETECTION_INFO_TAG, index,
            &FACE_DETECTION_INFO_BOTTOM);
    if (left == -1 || top == -1 || right == -1 || bottom == -1) {
        StereoLogW(
                "<getFaceRect> error: left == -1 || top == -1 || right == -1 || bottom == -1");
        return nullptr;
    }
    return new Rect(left, top, right, bottom);
}

int StereoInfoJsonParser::StereoInfoJsonParser::getFaceRip(int index) {
    return pJsonParser->getObjectPropertyValueFromArray(&FACE_DETECTION_INFO_TAG, index,
            &FACE_DETECTION_INFO_RIP);
}

double StereoInfoJsonParser::getGFocusBlurAtInfinity() {
    double blurAtInfinity = pJsonParser->getValueDoubleFromObject(&GFOCUSINFO_TAG, nullptr,
            &GFOCUSINFO_BLUR_AT_INFINITY);
    StereoLogD("<getGFocusBlurAtInfinity> blurAtInfinity: %.15f", blurAtInfinity);
    return blurAtInfinity;
}

double StereoInfoJsonParser::getGFocusFocalDistance() {
    double focalDistance = pJsonParser->getValueDoubleFromObject(&GFOCUSINFO_TAG, nullptr,
            &GFOCUSINFO_FOCAL_DISTANCE);
    StereoLogD("<getGFocusFocalDistance> focalDistance: %.15f", focalDistance);
    return focalDistance;
}

double StereoInfoJsonParser::getGFocusFocalPointX() {
    double focalPointX = pJsonParser->getValueDoubleFromObject(&GFOCUSINFO_TAG, nullptr,
            &GFOCUSINFO_FOCAL_POINT_X);
    StereoLogD("<getGFocusFocalPointX> focalPointX: %.15f", focalPointX);
    return focalPointX;
}

double StereoInfoJsonParser::getGFocusFocalPointY() {
    double focalPointY = pJsonParser->getValueDoubleFromObject(&GFOCUSINFO_TAG, nullptr,
            &GFOCUSINFO_FOCAL_POINT_Y);
    StereoLogD("<getGFocusFocalPointY> focalPointY: %.15f", focalPointY);
    return focalPointY;
}

StereoString StereoInfoJsonParser::getGImageMime() {
    StereoString gImageMime = pJsonParser->getValueStringFromObject(&GIMAGEINFO_TAG, nullptr,
            &GIMAGEINFO_MIME);
    StereoLogD("<getGImageMime> gImageMime: %s", gImageMime.c_str());
    return gImageMime;
}

StereoString StereoInfoJsonParser::getGDepthFormat() {
    StereoString gDepthFormat = pJsonParser->getValueStringFromObject(&GDEPTHINFO_TAG, nullptr,
            &GDEPTHINFO_FORMAT);
    StereoLogD("<getGDepthFormat> gDepthFormat: %s", gDepthFormat.c_str());
    return gDepthFormat;
}

double StereoInfoJsonParser::getGDepthNear() {
    double gDepthNear = pJsonParser->getValueIntFromObject(&GDEPTHINFO_TAG, nullptr,
            &GDEPTHINFO_NEAR);
    StereoLogD("<getGDepthNear> gDepthNear: %.15f", gDepthNear);
    return gDepthNear;
}

double StereoInfoJsonParser::getGDepthFar() {
    double gDepthFar = pJsonParser->getValueIntFromObject(
        &GDEPTHINFO_TAG, nullptr, &GDEPTHINFO_FAR);
    StereoLogD("<getGDepthFar> gDepthFar: %.15f", gDepthFar);
    return gDepthFar;
}

StereoString StereoInfoJsonParser::getGDepthMime() {
    StereoString gDepthMime = pJsonParser->getValueStringFromObject(&GDEPTHINFO_TAG, nullptr,
            &GDEPTHINFO_MIME);
    StereoLogD("<getGDepthMime> gDepthMime: %s", gDepthMime.c_str());
    return gDepthMime;
}

int StereoInfoJsonParser::getDof() {
    int dof = pJsonParser->getValueIntFromObject(nullptr, nullptr, &DOF_LEVEL_TAG);
    StereoLogD("<getDof> dof: %d", dof);
    return dof;
}

float StereoInfoJsonParser::getConvOffset() {
    float offset = (float) pJsonParser->getValueDoubleFromObject(
        nullptr, nullptr, &CONV_OFFSET_TAG);
    StereoLogD("<getConvOffset> offset: %.15f", offset);
    return offset;
}

int StereoInfoJsonParser::getLdcWidth() {
    int ldcWidth = pJsonParser->getValueIntFromObject(&LDCINFO_TAG, nullptr, &LDCINFO_WIDTH);
    StereoLogD("<getLdcWidth> ldcWidth: %d", ldcWidth);
    return ldcWidth;
}

int StereoInfoJsonParser::getLdcHeight() {
    int ldcHeight = pJsonParser->getValueIntFromObject(&LDCINFO_TAG, nullptr, &LDCINFO_HEIGHT);
    StereoLogD("<getLdcHeight> ldcHeight: %d", ldcHeight);
    return ldcHeight;
}

bool StereoInfoJsonParser::getFaceFlag() {
    bool isFace =
            pJsonParser->getValueBoolFromObject(&FOCUS_INFO_TAG, nullptr, &FOCUS_INFO_IS_FACE);
    StereoLogD("<getFaceFlag> FocusInfo.isFace: %d", isFace);
    return isFace;
}

double StereoInfoJsonParser::getFaceRatio() {
    double faceRatio =
            pJsonParser->getValueDoubleFromObject(&FOCUS_INFO_TAG, nullptr, &FOCUS_INFO_FACE_RATIO);
    StereoLogD("<getFaceRatio> FocusInfo.faceRatio: %.15f", faceRatio);
    return faceRatio;
}

int StereoInfoJsonParser::getCurDac() {
    int curDac = pJsonParser->getValueIntFromObject(&FOCUS_INFO_TAG, nullptr, &FOCUS_INFO_DAC_CUR);
    StereoLogD("<getFocusInfo> FocusInfo.curDac: %d", curDac);
    return curDac;
}

int StereoInfoJsonParser::getMinDac() {
    int minDac = pJsonParser->getValueIntFromObject(&FOCUS_INFO_TAG, nullptr, &FOCUS_INFO_DAC_MIN);
    StereoLogD("<getFocusInfo> FocusInfo.minDac: %d", minDac);
    return minDac;
}

int StereoInfoJsonParser::getMaxDac() {
    int maxDac = pJsonParser->getValueIntFromObject(&FOCUS_INFO_TAG, nullptr, &FOCUS_INFO_DAC_MAX);
    StereoLogD("<getFocusInfo> FocusInfo.maxDac: %d", maxDac);
    return maxDac;
}

int StereoInfoJsonParser::getFocusType() {
    int type = pJsonParser->getValueIntFromObject(&FOCUS_INFO_TAG, nullptr, &FOCUS_INFO_FOCUS_TYPE);
    StereoLogD("<getFocusType> FocusInfo.focusType: %d", type);
    return type;
}

int StereoInfoJsonParser::getMetaBufferWidth() {
    int metaWidth = pJsonParser->getValueIntFromObject(&DEPTH_INFO_TAG, nullptr, &DEPTH_INFO_META_WIDTH);
    StereoLogD("<getMetaBufferWidth> metaWidth: %d", metaWidth);
    return metaWidth;
}

int StereoInfoJsonParser::getMetaBufferHeight() {
    int metaHeight = pJsonParser->getValueIntFromObject(&DEPTH_INFO_TAG, nullptr,
            &DEPTH_INFO_META_HEIGHT);
    StereoLogD("<getMetaBufferHeight> metaHeight: %d", metaHeight);
    return metaHeight;
}

int StereoInfoJsonParser::getDepthBufferWidth() {
    int depthWidth = pJsonParser->getValueIntFromObject(&DEPTH_INFO_TAG, nullptr,
            &DEPTH_INFO_DEPTH_WIDTH);
    StereoLogD("<getDepthBufferWidth> depthWidth: %d", depthWidth);
    return depthWidth;
}

int StereoInfoJsonParser::getDepthBufferHeight() {
    int depthHeight = pJsonParser->getValueIntFromObject(&DEPTH_INFO_TAG, nullptr,
            &DEPTH_INFO_DEPTH_HEIGHT);
    StereoLogD("<getDepthBufferHeight> depthHeight: %d", depthHeight);
    return depthHeight;
}

void StereoInfoJsonParser::decodeMaskBuffer(
    StereoVector<StereoVector<int>>* encodedMaskArray, int maskSize, StereoBuffer_t &maskBuffer) {
    int startIndex = 0, endIndex = 0;
    BufferManager::createBuffer(maskSize, maskBuffer);
    memset(maskBuffer.data, 0x00, maskBuffer.size);
    for (unsigned int i = 0; i < encodedMaskArray->size(); i++) {
        startIndex = (*encodedMaskArray)[i][0];
        endIndex = startIndex + (*encodedMaskArray)[i][1];
        if (startIndex > maskSize || startIndex < 0 || endIndex < 0 || endIndex > maskSize) {
            StereoLogE("<decodeMaskBuffer> error, startIndex: %d, , endIndex: %d, maskSize: %d",
                startIndex, endIndex, maskSize);
            return;
        }
        for (int j = startIndex; j < endIndex; j++) {
            (maskBuffer.data)[j] = VALID_MASK;
        }
    }
}