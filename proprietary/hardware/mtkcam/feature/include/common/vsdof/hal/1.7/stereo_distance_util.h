/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef STEREO_DISTANCE_UTIL_H_
#define STEREO_DISTANCE_UTIL_H_

#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <stereo_tuning_provider.h>
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/feature/stereo/hal/FastLogger.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>

namespace StereoHAL {

#define DISTANCE_LOG_PROPERTY "vendor.STEREO.log.distance"

class StereoDistanceUtil
{
public:
    StereoDistanceUtil()
        : mLogger("StereoDistance", DISTANCE_LOG_PROPERTY)
    {
        mLogger.setSingleLineMode(false);
        //Get AF table
        int32_t main1Idx, main2Idx;
        StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
        NS3Av3::IHal3A *pHal3A = MAKE_Hal3A(main1Idx, LOG_TAG);
        if(NULL == pHal3A) {
            ALOGE("Cannot get 3A HAL");
        } else {
            pHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetAFDAFTable, (MUINTPTR)&mpAFTable, 0);
            if(NULL == mpAFTable) {
                ALOGE("Cannot get AF table");
            }

            pHal3A->destroyInstance(LOG_TAG);
        }

        __initAFWinTransform();

#if (1==HAS_HW_OCC)
        NSCam::NSIoPipe::OCCConfig config;
        StereoTuningProvider::getHWOCCTuningInfo(config, eSTEREO_SCENARIO_PREVIEW);
        INVALID_DEPTH_VALUE = config.occ_invalid_value;
        OCC_VEC_OFFSET      = config.occ_vec_offset;
#else
        MUINT32 coreNumber;
        std::vector<std::pair<std::string, int>> tuningParams;
        StereoTuningProvider::getOCCTuningInfo(coreNumber, tuningParams, eSTEREO_SCENARIO_PREVIEW);
        for(auto &tuning : tuningParams) {
            if(tuning.first == "occ.invalid_value") {
                INVALID_DEPTH_VALUE = tuning.second;
                mLogger.FastLogD("Update INVALID_DEPTH_VALUE to %d", INVALID_DEPTH_VALUE);
            } else if(tuning.first == "occ.vec_offset") {
                OCC_VEC_OFFSET = tuning.second;
                mLogger.FastLogD("Update OCC_VEC_OFFSET to %d", OCC_VEC_OFFSET);
            }
        }
#endif
        mLogger.FastLogD("INVALID_DEPTH_VALUE: %d\nOCC_VEC_OFFSET     : %d", INVALID_DEPTH_VALUE, OCC_VEC_OFFSET);
        mLogger.print();
    }

    /**
     * \brief Default destructor
     * \details Default destructor, callers should delete them after used
     */
    virtual ~StereoDistanceUtil() {}

    virtual float getDistance(NSCam::IImageBuffer *depthmap, float focalLensFactor, int magicNumberMain1, int magicNumberMain2)
    {
        const int AF_INDEX = magicNumberMain1 % DAF_TBL_QLEN;
        NS3Av3::DAF_VEC_STRUCT *afVec = &mpAFTable->daf_vec[AF_INDEX];

        mLogger.FastLogD("AF Status: %d -> %d", mLastAFStatus, afVec->is_af_stable);

        if(afVec->is_af_stable != mLastAFStatus)
        {
            mDistance = __calculateDistance(depthmap, __getAFRect(AF_INDEX), focalLensFactor);
            mLogger.FastLogD("Update distance %.2f cm, frame %d %d, FB %f, DAC %d(Macro: %d, Inf: %d)",
                            mDistance, magicNumberMain1, magicNumberMain2, focalLensFactor,
                            (afVec->posture_dac > 0) ? afVec->posture_dac : afVec->af_dac_pos,
                            mDacMacro, mDacInf);
        }
        mLastAFStatus = afVec->is_af_stable;
        mLogger.print();

        return mDistance;
    }

private:
    //For distance calculation
    void __initAFWinTransform()
    {
        MINT32 err = 0;
        int main1SensorIndex, main2SensorIndex;
        StereoSettingProvider::getStereoSensorIndex(main1SensorIndex, main2SensorIndex);

        int main1SensorDevIndex, main2SensorDevIndex;
        StereoSettingProvider::getStereoSensorDevIndex(main1SensorDevIndex, main2SensorDevIndex);

        NSCam::IHalSensorList* sensorList = MAKE_HalSensorList();
        if(NULL == sensorList) {
            ALOGE("Cannot get sensor list");
        } else {
            MUINT32 junkStride;
            //========= Get main1 size =========
            NSCam::IHalSensor* pIHalSensor = sensorList->createSensor(LOG_TAG, main1SensorIndex);
            if(NULL == pIHalSensor) {
                ALOGE("Cannot get hal sensor of main1");
                err = 1;
            } else {
                NSCam::SensorCropWinInfo sensorCropInfoZSD;
                const int STEREO_FEATURE_MODE = NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP;
                const int STEREO_MODULE_TYPE = (StereoSettingProvider::isBayerPlusMono()) ? NSCam::v1::Stereo::BAYER_AND_MONO : NSCam::v1::Stereo::BAYER_AND_BAYER;
                MUINT sensorScenarioMain1, sensorScenarioMain2;

                ::memset(&sensorCropInfoZSD, 0, sizeof(NSCam::SensorCropWinInfo));
                StereoSettingProvider::getSensorScenario(STEREO_FEATURE_MODE,
                                                         STEREO_MODULE_TYPE,
                                                         PipelineMode_ZSD,
                                                         sensorScenarioMain1,
                                                         sensorScenarioMain2);
                err = pIHalSensor->sendCommand(main1SensorDevIndex, NSCam::SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                                               (MUINTPTR)&sensorScenarioMain1, (MUINTPTR)&sensorCropInfoZSD, 0);
                if(err) {
                    ALOGE("Cannot get sensor crop info for preview/record");
                } else {
                    int m, n;
                    imageRatioMToN(StereoSettingProvider::imageRatio(), m, n);
                    const float HEIGHT_RATIO = n/(float)m / (3.0f/4.0f);
                    mAFOffsetX = sensorCropInfoZSD.x0_offset;
                    mAFOffsetY = sensorCropInfoZSD.y0_offset + sensorCropInfoZSD.scale_h * (1.0f-HEIGHT_RATIO)/2.0f;
                    mAFScaleW  = (float)AF_ROTATE_DOMAIN.w / sensorCropInfoZSD.scale_w;
                    mAFScaleH  = (float)AF_ROTATE_DOMAIN.h / (sensorCropInfoZSD.scale_h * HEIGHT_RATIO);
                }

                pIHalSensor->destroyInstance(LOG_TAG);

                mLogger.FastLogD("AF Transform: offset(%d, %d), scale(%f, %f)",
                                 mAFOffsetX, mAFOffsetY, mAFScaleW, mAFScaleH).print();
            }
        }

        //Init AF DAC min/max
        if(StereoSettingProvider::isSensorAF(main1SensorIndex)) {
            CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance();
            CAM_CAL_DATA_STRUCT camCalData;
            MUINT32 err = pCamCalDrvObj->GetCamCalCalData(main1SensorDevIndex, CAMERA_CAM_CAL_DATA_3A_GAIN, (void *)&camCalData);
            if(!err) {
                mDacInf   = camCalData.Single2A.S2aAf[0];
                mDacMacro = camCalData.Single2A.S2aAf[1];
            } else {
                ALOGE("Read CAMERA_CAM_CAL_DATA_3A_GAIN failed");
            }
            pCamCalDrvObj->destroyInstance();
        }
    }

    NSCam::MPoint __AFToDepthmapPoint(const NSCam::MPoint &ptAF)
    {
        return NSCam::MPoint( (ptAF.x - mAFOffsetX) * mAFScaleW ,
                              (ptAF.y - mAFOffsetY) * mAFScaleH );
    }

    NSCam::MRect __getAFRect(const int AF_INDEX)
    {
        NS3Av3::DAF_VEC_STRUCT *afVec = &mpAFTable->daf_vec[AF_INDEX];
        NSCam::MPoint topLeft     = __AFToDepthmapPoint(NSCam::MPoint(afVec->af_win_start_x, afVec->af_win_start_y));
        NSCam::MPoint bottomRight = __AFToDepthmapPoint(NSCam::MPoint(afVec->af_win_end_x, afVec->af_win_end_y));
        NSCam::MRect focusRect = NSCam::MRect(topLeft, NSCam::MSize(bottomRight.x - topLeft.x + 1, bottomRight.y - topLeft.y + 1));
        if(mIsVerticalModule) {
            focusRect = StereoHAL::rotateRect( focusRect, AF_ROTATE_DOMAIN, StereoSettingProvider::getModuleRotation() );
        }

        mLogger.FastLogD("======================================================\n"
                         "AF Top Left:     (%d, %d) -> (%d, %d)\n"
                         "AF Bottom Right: (%d, %d) -> (%d, %d)\n"
                         "AF Rect Domain size %dx%d\n"
                         "%s rect: (%d, %d), %dx%d, Center (%d, %d), Dephtmap Size %dx%d",
                         afVec->af_win_start_x, afVec->af_win_start_y, topLeft.x, topLeft.y,
                         afVec->af_win_end_x, afVec->af_win_end_y, bottomRight.x, bottomRight.y,
                         AF_ROTATE_DOMAIN.w, AF_ROTATE_DOMAIN.h,
                         mIsVerticalModule ? "Rotated focus" : "Focus",
                         focusRect.p.x, focusRect.p.y, focusRect.s.w, focusRect.s.h,
                         focusRect.p.x+focusRect.s.w/2, focusRect.p.y+focusRect.s.h/2,
                         DEPTHMAP_SIZE.w, DEPTHMAP_SIZE.h);

        //To depthmap domain
        focusRect.p.x *= mDepthmapWidthFactor;
        focusRect.s.w *= mDepthmapWidthFactor;

        //Handle out-of-range, center point will not change, but size will
        if(focusRect.p.x < 0) {
            focusRect.s.w -= -focusRect.p.x * 2;
            focusRect.p.x = 0;
        }

        if(focusRect.p.y < 0) {
            focusRect.s.h -= -focusRect.p.y * 2;
            focusRect.p.y = 0;
        }

        int offset = focusRect.p.x + focusRect.s.w - DEPTHMAP_SIZE.w;
        if(offset > 0) {
            focusRect.s.w -= offset * 2;
            focusRect.p.x += offset;
        }

        offset = focusRect.p.y + focusRect.s.h - DEPTHMAP_SIZE.h;
        if(offset > 0) {
            focusRect.s.h -= offset * 2;
            focusRect.p.y += offset;
        }

        return focusRect;
    }

    MFLOAT __calculateDistance(NSCam::IImageBuffer *depthmap, NSCam::MRect afROI, MFLOAT focalLensFactor)
    {
        mLogger.FastLogD("------------------------------------------------------\n"
                         "  AF ROI: (%d, %d) size %dx%d, depthmap size %dx%d\n"
                         "================= Content of AF ROI ==================",
                         afROI.p.x, afROI.p.y, afROI.s.w, afROI.s.h,
                         depthmap->getImgSize().w, depthmap->getImgSize().h);
        ::memset(mROIStatistics, 0, sizeof(mROIStatistics));
        const size_t STRIDE = depthmap->getImgSize().w;
        MUINT8 *posHead = (MUINT8*)depthmap->getBufVA(0) + STRIDE * afROI.p.y + afROI.p.x;
        MUINT8 *posRun = posHead;

        char debugLine[afROI.s.w+2];
        debugLine[afROI.s.w+1] = 0;
        int debugValue = 0;
        //Decide depth, find index of max count of near depth
        float distance      = mDistance;    //means infinite
        int statisticsStart = OCC_VEC_OFFSET;
        int statisticsEnd   = ROI_STATISTICS_SIZE - 1;
        int depthIndex      = 0;
        int minIndex        = 0;
        if(0 == INVALID_DEPTH_VALUE) {
            statisticsStart = OCC_VEC_OFFSET + 1;
            statisticsEnd   = ROI_STATISTICS_SIZE;
            minIndex        = 1;
        }
        int logDivisor = (int)(std::ceil((255-statisticsStart)/10.0f));

        //Depthmap statistics
        if(!mDistnaceLogEnabled)
        {
            for(int row = 0; row < afROI.s.h; ++row) {
                for(int col = 0; col < afROI.s.w; ++col, ++posRun) {
                    mROIStatistics[*posRun]++;
                }

                posHead += STRIDE;
                posRun = posHead;
            }
        }
        else
        {
            int col = 0;
            for(int row = 0; row < afROI.s.h; ++row) {
                for(col = 0; col < afROI.s.w; ++col, ++posRun) {
                    mROIStatistics[*posRun]++;

                    if(*posRun==INVALID_DEPTH_VALUE) {
                        //Hole
                        debugLine[col] = ' ';
                    } else {
                        if(*posRun < statisticsStart) {
                            //Invalid depth
                            debugLine[col] = '-';
                        } else {
                            //Valid depth to 0~9
                            debugValue = (*posRun-statisticsStart)/logDivisor;
                            debugLine[col] = debugValue+48;
                        }
                    }
                }
                debugLine[col] = '|';
                mLogger.FastLogD("%s", debugLine);

                posHead += STRIDE;
                posRun = posHead;
            }
        }

        int MAX_COUNT = 0;
        int targetDepth = statisticsStart;
        for(depthIndex = statisticsStart; depthIndex < statisticsEnd; ++depthIndex) {
            if(mROIStatistics[depthIndex] > MAX_COUNT) {
                MAX_COUNT = mROIStatistics[depthIndex];
                targetDepth = depthIndex;
            }
        }

        if(targetDepth >= statisticsStart) {
            distance = focalLensFactor/(targetDepth - statisticsStart + 1);
        }

        if(mDistnaceLogEnabled) {
            mLogger.FastLogD("============= Depth Histogram in AF ROI =============");
            const int TOTAL_MAX_COUNT = *std::max_element(mROIStatistics+statisticsStart-OCC_VEC_OFFSET, mROIStatistics+statisticsEnd);
            int COUNT_PER_SYMBOL = TOTAL_MAX_COUNT/40;    //40 '*'
            for(depthIndex = statisticsEnd-1; depthIndex >= minIndex; --depthIndex) {
                if(statisticsStart-1 == depthIndex &&
                   MAX_COUNT > 0)
                {
                    mLogger.FastLogD("-----------------------------------------------------");
                }

                if(mROIStatistics[depthIndex] > 0)
                {
                    if(targetDepth != depthIndex ||
                       depthIndex <= statisticsStart)
                    {
                        mLogger.FastLogD("%4d|%-40.*s (%5d)",
                                         depthIndex,
                                         mROIStatistics[depthIndex]/COUNT_PER_SYMBOL, "****************************************",
                                         mROIStatistics[depthIndex]);
                    } else {
                        mLogger.FastLogD("%4d|%-40.*s (%5d) <-- Target depth",
                                         depthIndex,
                                         mROIStatistics[depthIndex]/COUNT_PER_SYMBOL, "****************************************",
                                         mROIStatistics[depthIndex]);
                    }

                }
            }

            mLogger.FastLogD("=====================================================");
            if(MAX_COUNT > 0) {
                mLogger.FastLogD("   Depth %d, FB %.2f, Distance %.2f\n", targetDepth, focalLensFactor, distance);
            } else {
                mLogger.FastLogD("   Depth --, FB %.2f, Distance %.2f\n", focalLensFactor, distance);
            }
            mLogger.FastLogD("=====================================================");
        }
        else
        {
            if(MAX_COUNT > 0) {
                ALOGD("   Depth %d, FB %.2f, Distance %.2f\n", targetDepth, focalLensFactor, distance);
            } else {
                ALOGD("   Depth --, FB %.2f, Distance %.2f\n", focalLensFactor, distance);
            }
        }
        mLogger.print();

        return distance;
    }

private:
    const NSCam::MSize                      AF_ROTATE_DOMAIN = StereoSizeProvider::getInstance()->getBufferSize(E_DMG);
    const NSCam::MSize                      DEPTHMAP_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_NOC);
    const static size_t                     ROI_STATISTICS_SIZE = 256;  //we'll skip hole value
    int                                     mROIStatistics[ROI_STATISTICS_SIZE];
    int                                     INVALID_DEPTH_VALUE = 0;    //query from tuning later
    int                                     OCC_VEC_OFFSET      = 128;  //query from tuning later
    NS3Av3::DAF_TBL_STRUCT                  *mpAFTable          = NULL;
    float                                   mDistance           = 0.0f; //Distance in cm
    int                                     mLastAFStatus       = 0;
    int                                     mDacMacro           = 0;
    int                                     mDacInf             = 0;
    int                                     mDacCurrent         = 0;
    float                                   mAFScaleW           = 1.0f;
    float                                   mAFScaleH           = 1.0f;
    int                                     mAFOffsetX          = 0;
    int                                     mAFOffsetY          = 0;
    FastLogger                              mLogger;
    bool                                    mDistnaceLogEnabled = property_get_bool(DISTANCE_LOG_PROPERTY, false);
    bool                                    mIsVerticalModule   = (StereoSettingProvider::getModuleRotation() & 0x2);
    int                                     mDepthmapWidthFactor = mIsVerticalModule
                                                                   ? DEPTHMAP_SIZE.w/AF_ROTATE_DOMAIN.h
                                                                   : DEPTHMAP_SIZE.w/AF_ROTATE_DOMAIN.w;
};

};  //namespace StereoHAL
#endif