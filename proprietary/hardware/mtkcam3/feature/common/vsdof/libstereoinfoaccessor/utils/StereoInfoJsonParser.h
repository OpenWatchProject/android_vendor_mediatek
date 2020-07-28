#ifndef STEREO_INFO_JSON_PARSER_H
#define STEREO_INFO_JSON_PARSER_H

#include "types.h"
#include "StereoInfo.h"
#include "JsonParser.h"

namespace stereo {

class StereoInfoJsonParser {

public:

    /**
     * constructor, create JsonParser.
     * @param jsonBuffer
     */
    explicit StereoInfoJsonParser(const StereoBuffer_t &jsonBuffer);

    virtual ~StereoInfoJsonParser();

    /**
     * get GEO verify level.
     * @return GEO verify level
     */
    int getGeoVerifyLevel();

    /**
     * get photo verify level.
     *
     * @return photo verify level
     */
    int getPhoVerifyLevel();

    /**
     * get MTKCHA verify level.
     * @return MTKCHA verify level
     */
    int getMtkChaVerifyLevel();

    /**
     * get jps width.
     * @return jps width
     */
    int getJpsWidth();

    /**
     * get jps height.
     * @return jps height
     */
    int getJpsHeight();

    /**
     * Get google depth buffer width returned by camera.
     * @return google depth buffer width
     */
     int getGoogleDepthWidth();

    /**
     * Get google depth buffer height returned by camera.
     * @return google depth buffer height
     */
     int getGoogleDepthHeight();

    /**
     * get mask width.
     * @return mask width
     */
     int getMaskWidth();

    /**
     * get mask height.
     * @return mask height
     */
     int getMaskHeight();

    /**
     * get mask size.
     * @return mask size
     */
     int getMaskSize();

    /**
     * get mask buffer.
     * @param [out]mask buffer
     */
     void getMaskBuffer(StereoBuffer_t &mask);

    /**
     * get posX.
     * @return posX
     */
     int getPosX();

    /**
     * get posY.
     * @return posY
     */
     int getPosY();

    /**
     * get view width.
     * @return view width
     */
     int getViewWidth();

    /**
     * get view height.
     * @return view height
     */
     int getViewHeight();

    /**
     * get orientation.
     * @return orientation
     */
     int getOrientation();

    /**
     * get depth rotation.
     * @return depth rotation
     */
     int getDepthRotation();

    /**
     * get main camera position.
     * @return main camera position
     */
     int getMainCamPos() ;

    /**
     * get first touch x coordinates.
     * @return first touch x coordinates
     */
     int getTouchCoordX1st();

    /**
     * get first touch y coordinates.
     * @return first touch y coordinates
     */
     int getTouchCoordY1st();

    /**
     * get focus top.
     * @return focus top
     */
     int getFocusTop();

    /**
     * get focus left.
     * @return focus left
     */
     int getFocusLeft();

    /**
     * get focus right.
     * @return focus right
     */
     int getFocusRight();

    /**
     * get focus bottom.
     * @return focus bottom
     */
     int getFocusBottom();

    /**
     * get face count.
     * @return face count
     */
     int getFaceRectCount();

    /**
     * get face region.
     * @param index
     *            index
     * @return face region
     */
     Rect* getFaceRect(int index);

    /**
     * get face rip.
     * @param index
     *            face rip
     * @return face rip
     */
     int getFaceRip(int index);

    /**
     * Get BlurAtInfinity.
     * @return BlurAtInfinity
     */
     double getGFocusBlurAtInfinity();

    /**
     * Get FocalDistance.
     * @return FocalDistance
     */
     double getGFocusFocalDistance();

    /**
     * Get FocalPointX.
     * @return FocalPointX
     */
     double getGFocusFocalPointX();

    /**
     * Get FocalPointY.
     * @return FocalPointY
     */
     double getGFocusFocalPointY();

    /**
     * Get ImageMime.
     * @return ImageMime
     */
     StereoString getGImageMime();

    /**
     * Get DepthFormat.
     * @return DepthFormat
     */
     StereoString getGDepthFormat();

    /**
     * Get DepthNear.
     * @return DepthNear
     */
     double getGDepthNear();

    /**
     * Get DepthFar.
     * @return DepthFar
     */
     double getGDepthFar();

    /**
     * Get DepthMime.
     * @return DepthMime
     */
     StereoString getGDepthMime();

    /**
     * Get DOF value.
     * @return dof value
     */
     int getDof();

    /**
     * Get offset value.
     * @return offset value
     */
     float getConvOffset();

    /**
     * Get ldc width.
     * @return ldc width
     */
     int getLdcWidth();

    /**
     * get ldc height.
     * @return ldc height
     */
     int getLdcHeight();

    /**
     * Get isFace flag.
     * @return isFace flag
     */
     bool getFaceFlag();

    /**
     * Get faceRatio.
     * @return faceRatio
     */
     double getFaceRatio();

    /**
     * Get curDac.
     * @return curDac
     */
     int getCurDac();

    /**
     * Get minDac.
     * @return minDac
     */
     int getMinDac();

    /**
     * Get maxDac.
     * @return maxDac
     */
     int getMaxDac();

    /**
     * Get focus type.
     * @return focus type
     */
     int getFocusType();

    /**
     * Get meta width.
     * @return meta width
     */
     int getMetaBufferWidth();

    /**
     * Get meta height.
     * @return meta height
     */
     int getMetaBufferHeight();

    /**
     * Get depth width.
     * @return depth width
     */
     int getDepthBufferWidth();

    /**
     * Get depth height.
     * @return depth height
     */
     int getDepthBufferHeight();

private:
    const StereoString VERIFY_GEO_INFO_TAG = "verify_geo_data";
    const StereoString VERIFY_GEO_INFO_LEVEL = "quality_level";

    const char VALID_MASK = 0xff;
    const StereoString MASKINFO_TAG = "mask_info";
    const StereoString MASKINFO_WIDTH = "width";
    const StereoString MASKINFO_HEIGHT = "height";
    const StereoString MASKINFO_MASK = "mask";
    const StereoString JPSINFO_TAG = "JPS_size";
    const StereoString JPSINFO_WIDTH = "width";
    const StereoString JPSINFO_HEIGHT = "height";
    const StereoString DEPTHINFO_TAG = "depth_buffer_size";
    const StereoString DEPTHINFO_WIDTH = "width";
    const StereoString DEPTHINFO_HEIGHT = "height";
    const StereoString POSINFO_TAG = "main_cam_align_shift";
    const StereoString POSINFO_X = "x";
    const StereoString POSINFO_Y = "y";
    const StereoString TOUCH_COORD_INFO_TAG = "focus_roi";
    const StereoString TOUCH_COORD_INFO_LEFT = "left";
    const StereoString TOUCH_COORD_INFO_TOP = "top";
    const StereoString TOUCH_COORD_INFO_RIGHT = "right";
    const StereoString TOUCH_COORD_INFO_BOTTOM = "bottom";
    const StereoString VIEWINFO_TAG = "input_image_size";
    const StereoString VIEWINFO_WIDTH = "width";
    const StereoString VIEWINFO_HEIGHT = "height";
    const StereoString ORIENTATIONINFO_TAG = "capture_orientation";
    const StereoString ORIENTATIONINFO_ORIENTATION = "orientation";
    const StereoString DEPTH_ROTATION_INFO_TAG = "depthmap_orientation";
    const StereoString DEPTH_ROTATION_INFO_ORIENTATION = "orientation";
    const StereoString MAIN_CAM_POSITION_INFO_TAG = "sensor_relative_position";
    const StereoString MAIN_CAM_POSITION_INFO_POSITION = "relative_position";
    const StereoString VERIFY_PHO_INFO_TAG = "verify_pho_data";
    const StereoString VERIFY_PHO_INFO_LEVEL = "quality_level";
    const StereoString VERIFY_MTK_CHA_INFO_TAG = "verify_mtk_cha";
    const StereoString VERIFY_MTK_CHA_INFO_LEVEL = "quality_level";
    const StereoString FACE_DETECTION_INFO_TAG = "face_detections";
    const StereoString FACE_DETECTION_INFO_LEFT = "left";
    const StereoString FACE_DETECTION_INFO_TOP = "top";
    const StereoString FACE_DETECTION_INFO_RIGHT = "right";
    const StereoString FACE_DETECTION_INFO_BOTTOM = "bottom";
    const StereoString FACE_DETECTION_INFO_RIP = "rotation-in-plane";
    const StereoString DOF_LEVEL_TAG = "dof_level";
    const StereoString CONV_OFFSET_TAG = "conv_offset";
    const StereoString LDCINFO_TAG = "ldc_size";
    const StereoString LDCINFO_WIDTH = "width";
    const StereoString LDCINFO_HEIGHT = "height";

    const StereoString FOCUS_INFO_TAG = "focus_info";
    const StereoString FOCUS_INFO_IS_FACE = "is_face";
    const StereoString FOCUS_INFO_FACE_RATIO = "face_ratio";
    const StereoString FOCUS_INFO_DAC_CUR = "dac_cur";
    const StereoString FOCUS_INFO_DAC_MIN = "dac_min";
    const StereoString FOCUS_INFO_DAC_MAX = "dac_max";
    const StereoString FOCUS_INFO_FOCUS_TYPE = "focus_type";

    // google stereo params
    const StereoString GFOCUSINFO_TAG = "GFocus";
    const StereoString GFOCUSINFO_BLUR_AT_INFINITY = "BlurAtInfinity";
    const StereoString GFOCUSINFO_FOCAL_DISTANCE = "FocalDistance";
    const StereoString GFOCUSINFO_FOCAL_POINT_X = "FocalPointX";
    const StereoString GFOCUSINFO_FOCAL_POINT_Y = "FocalPointY";
    const StereoString GIMAGEINFO_TAG = "GImage";
    const StereoString GIMAGEINFO_MIME = "Mime";
    const StereoString GDEPTHINFO_TAG = "GDepth";
    const StereoString GDEPTHINFO_FORMAT = "Format";
    const StereoString GDEPTHINFO_NEAR = "Near";
    const StereoString GDEPTHINFO_FAR = "Far";
    const StereoString GDEPTHINFO_MIME = "Mime";

    // mtk depth info
    const StereoString DEPTH_INFO_TAG = "depth_buffer_size";
    const StereoString DEPTH_INFO_META_WIDTH = "meta_width";
    const StereoString DEPTH_INFO_META_HEIGHT = "meta_height";
    const StereoString DEPTH_INFO_DEPTH_WIDTH = "width";
    const StereoString DEPTH_INFO_DEPTH_HEIGHT = "height";

    int mFaceRectCount = -1;
    int mMainCamPostion = -1;
    int mOrientation = -1;
    int mDepthRotation = -1;
    int mViewWidth = -1;
    int mViewHeight = -1;
    int mTouchCoordX1st = -1;
    int mTouchCoordY1st = -1;
    int mPosX = -1;
    int mPosY = -1;
    int mMaskWidth = -1;
    int mMaskHeight = -1;
    int mMaskSize = -1;
    int mJpsWidth = -1;
    int mJpsHeight = -1;
    int mGDepthWidth = -1;
    int mGDepthHeight = -1;

    JsonParser *pJsonParser = nullptr;

    void decodeMaskBuffer(StereoVector<StereoVector<int>>* encodedMaskArray,
            int maskSize, StereoBuffer_t &maskBuffer);
};

}

#endif