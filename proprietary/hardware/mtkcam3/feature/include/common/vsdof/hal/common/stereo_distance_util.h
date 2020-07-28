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

#include <algorithm>    //sort, fill, minmax_element
#include <numeric>      //iota
#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <stereo_tuning_provider.h>
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam3/feature/stereo/hal/FastLogger.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>
#include <vsdof/hal/ProfileUtil.h>

namespace StereoHAL {

#define DISTANCE_LOG_PROPERTY "vendor.STEREO.log.distance"

typedef std::vector<int> DEPTH_VEC_T;
typedef DEPTH_VEC_T::iterator DEPTH_ITOR_T;
typedef std::pair<DEPTH_ITOR_T, DEPTH_ITOR_T> DEPTH_RANGE_T;
const int DEPTH_STATISTICS_SIZE  = 256;

enum ENUM_DISTANCE_SOURCE_BUFFER
{
    E_DIST_SRC_NOC,  //OCC
    E_DIST_SRC_DMW,  //WMF
    E_DIST_SRC_DMG,  //GF
};

class StereoDistanceUtil
{
public:
    StereoDistanceUtil(ENUM_DISTANCE_SOURCE_BUFFER source, __attribute__((unused)) bool isRecording=false)
        : mLogger("StereoDistance", DISTANCE_LOG_PROPERTY)
        , BUFFER_SRC(source)
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


        switch(BUFFER_SRC) {
            case E_DIST_SRC_NOC:
                mpCalculator = new DistanceCalculatorOCC(mLogger);
                break;
            case E_DIST_SRC_DMW:
                mpCalculator = new DistanceCalculatorWMF(mLogger);
                break;
            case E_DIST_SRC_DMG:
                mpCalculator = new DistanceCalculatorGF(mLogger);
                break;
            default:
                {
                    if(StereoSettingProvider::getDepthmapRefineLevel() == E_DEPTHMAP_REFINE_SW_OPTIMIZED)
                    {
                        mpCalculator = new DistanceCalculatorGF(mLogger);
                        ALOGI("[StereoDistnace] Unknown source for distance calculation, use DMG as default");
                    } else {
                        mpCalculator = new DistanceCalculatorWMF(mLogger);
                        ALOGI("[StereoDistnace] Unknown source for distance calculation, use DMW as default");
                    }
                }
                break;
        }

        MAX_AF_RECT_SIZE = mpCalculator->getMaxAFRectSize();

        __initAFWinTransform(isRecording);
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

        if(afVec->is_af_stable != mLastAFStatus)
        {
            mLogger.FastLogD("AF Status: %d -> %d", mLastAFStatus, afVec->is_af_stable);

            mDistance = mpCalculator->getDistance(depthmap, __getAFRect(AF_INDEX), focalLensFactor);

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
    void __initAFWinTransform(__attribute__((unused)) bool isRecording=false)
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
            //========= Get main1 size =========
            NSCam::IHalSensor* pIHalSensor = sensorList->createSensor(LOG_TAG, main1SensorIndex);
            if(NULL == pIHalSensor) {
                ALOGE("Cannot get hal sensor of main1");
                err = 1;
            } else {
                NSCam::SensorCropWinInfo sensorCropInfoZSD;
                const int STEREO_FEATURE_MODE = NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP;
                MUINT sensorScenarioMain1, sensorScenarioMain2;

                ::memset(&sensorCropInfoZSD, 0, sizeof(NSCam::SensorCropWinInfo));
                StereoSettingProvider::getSensorScenario(STEREO_FEATURE_MODE,
                                                         isRecording,
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
                    mAFOffsetX = 0;
                    mAFOffsetY = sensorCropInfoZSD.h2_tg_size * (1.0f-HEIGHT_RATIO)/2.0f;
                    mAFScaleW  = (float)AF_ROTATE_DOMAIN.w / sensorCropInfoZSD.w2_tg_size;
                    mAFScaleH  = (float)AF_ROTATE_DOMAIN.h / (sensorCropInfoZSD.h2_tg_size * HEIGHT_RATIO);
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
        if(IS_DEPTHMAP_ROTATED)
        {
            focusRect = StereoHAL::rotateRect( focusRect, AF_ROTATE_DOMAIN, StereoSettingProvider::getModuleRotation() );
        }

        //Shrink AF rect for FD AF since depth for a face should be similar and enhance distance calculation
        NSCam::MPoint centerPt(focusRect.p.x+focusRect.s.w/2, focusRect.p.y+focusRect.s.h/2);
        if(focusRect.s.w > MAX_AF_RECT_SIZE ||
           focusRect.s.h > MAX_AF_RECT_SIZE)
        {
            int minSize = std::min({focusRect.s.w, focusRect.s.h, MAX_AF_RECT_SIZE});
            focusRect.s.w = minSize;
            focusRect.s.h = minSize;
            focusRect.p.x = centerPt.x - focusRect.s.w/2;
            focusRect.p.y = centerPt.y - focusRect.s.h/2;
        }

        mLogger.FastLogD("======================================================\n"
                         "AF Type: %d\n"
                         "AF Top Left:     (%d, %d) -> (%d, %d)\n"
                         "AF Bottom Right: (%d, %d) -> (%d, %d)\n"
                         "AF Rect Domain size %dx%d\n"
                         "%s rect: (%d, %d), %dx%d, Center (%d, %d), Dephtmap Size %dx%d",
                         afVec->af_roi_sel,
                         afVec->af_win_start_x, afVec->af_win_start_y, topLeft.x, topLeft.y,
                         afVec->af_win_end_x, afVec->af_win_end_y, bottomRight.x, bottomRight.y,
                         AF_ROTATE_DOMAIN.w, AF_ROTATE_DOMAIN.h,
                         IS_DEPTHMAP_ROTATED ? "Rotated focus" : "Focus",
                         focusRect.p.x, focusRect.p.y, focusRect.s.w, focusRect.s.h,
                         centerPt.x, centerPt.y,
                         DEPTHMAP_SIZE.w, DEPTHMAP_SIZE.h);

        //To depthmap domain
        focusRect.p.x *= DEPTHMAP_WIDTH_FACTOR;
        focusRect.s.w *= DEPTHMAP_WIDTH_FACTOR;

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

private:
    //=============================================================
    // Private classes
    //=============================================================
    class StereoDistanceCalculator
    {
    public:
        StereoDistanceCalculator(FastLogger &logger)
            : mDepthCounts(DEPTH_STATISTICS_SIZE)
            , mSortedDepthCounts(DEPTH_STATISTICS_SIZE)
        {
            mLogger = &logger;
        }

        virtual ~StereoDistanceCalculator() {}
        virtual float getDistance(NSCam::IImageBuffer *depthmap, NSCam::MRect afROI, MFLOAT focalLensFactor) = 0;
        virtual int getMaxAFRectSize() { return 40; }

    protected:
        virtual void _updateDepthCounts(NSCam::IImageBuffer *depthmap, NSCam::MRect afROI)
        {
            const size_t STRIDE = depthmap->getImgSize().w;
            MUINT8 *posHead = (MUINT8*)depthmap->getBufVA(0) + STRIDE * afROI.p.y + afROI.p.x;
            MUINT8 *posRun = posHead;

            std::fill(mDepthCounts.begin(), mDepthCounts.end(), 0);
            for(int row = 0; row < afROI.s.h; ++row) {
                for(int col = 0; col < afROI.s.w; ++col, ++posRun) {
                    mDepthCounts[*posRun]++;
                }

                posHead += STRIDE;
                posRun = posHead;
            }

            _logDepthmap(depthmap, afROI);
        }

        virtual void _logDepthmap(NSCam::IImageBuffer *depthmap, NSCam::MRect afROI)
        {
            if(!mDistnaceLogEnabled) {
                return;
            }

            mLogger->FastLogD("------------------------------------------------------\n"
                             "  AF ROI: (%d, %d) size %dx%d, depthmap size %dx%d\n"
                             "================= Content of AF ROI ==================",
                             afROI.p.x, afROI.p.y, afROI.s.w, afROI.s.h,
                             depthmap->getImgSize().w, depthmap->getImgSize().h);

            const size_t STRIDE = depthmap->getImgSize().w;
            MUINT8 *posHead = (MUINT8*)depthmap->getBufVA(0) + STRIDE * afROI.p.y + afROI.p.x;
            MUINT8 *posRun = posHead;

            int logDivisor = (int)(std::ceil(255/10.0f));
            char debugLine[afROI.s.w+2];
            debugLine[afROI.s.w+1] = 0;
            int debugValue = 0;
            int col = 0;
            for(int row = 0; row < afROI.s.h; ++row) {
                for(col = 0; col < afROI.s.w; ++col, ++posRun) {
                    mDepthCounts[*posRun]++;

                    debugValue = (*posRun)/logDivisor;
                    debugLine[col] = debugValue+48;
                }
                debugLine[col] = '|';
                mLogger->FastLogD("%s", debugLine);

                posHead += STRIDE;
                posRun = posHead;
            }

            mLogger->print();
        }

        virtual void _logResult(const int TARGET_DEPTH, const float FB)
        {
            int MAX_COUNT = mDepthCounts[mSortedDepthCounts[0]];
            if(!mDistnaceLogEnabled)
            {
                if(MAX_COUNT > 0) {
                    ALOGD("=====================================================\n"
                          "   Depth %d, FB %.2f, Distance %.2f\n"
                          "=====================================================", TARGET_DEPTH, FB, mDistance);
                } else {
                    ALOGD("=====================================================\n"
                          "   Depth --, FB %.2f, Distance %.2f\n"
                          "=====================================================", FB, mDistance);
                }
            }
            else
            {
                mLogger->FastLogD("============= Depth Histogram in AF ROI =============");
                auto dist = std::distance(mSortedDepthCounts.begin(),
                                          find_if(mSortedDepthCounts.begin(), mSortedDepthCounts.end(),
                                                  [&](auto &x){return mDepthCounts[x]==0;}));
                DEPTH_RANGE_T range = std::minmax_element(mSortedDepthCounts.begin(), mSortedDepthCounts.begin()+dist);
                const int MIN_DEPTH = *range.first;
                const int MAX_DEPTH = *range.second;

                const size_t PRINT_WIDTH = (MAX_COUNT >= 40) ? 40 : MAX_COUNT;
                int COUNT_PER_SYMBOL = MAX_COUNT/PRINT_WIDTH;
                for(int depthIndex = MIN_DEPTH; depthIndex <= MAX_DEPTH; ++depthIndex) {
                    if(TARGET_DEPTH != depthIndex)
                    {
                        if(mDepthCounts[depthIndex] > 0) {
                            mLogger->FastLogD("%4d|%-*.*s (%5d)",
                                             depthIndex,
                                             PRINT_WIDTH,
                                             mDepthCounts[depthIndex]/COUNT_PER_SYMBOL, "****************************************",
                                             mDepthCounts[depthIndex]);
                        }
                    } else {
                        mLogger->FastLogD("%4d|%-*.*s (%5d) <-- Target depth",
                                         depthIndex,
                                         PRINT_WIDTH,
                                         mDepthCounts[depthIndex]/COUNT_PER_SYMBOL, "****************************************",
                                         mDepthCounts[depthIndex]);
                    }
                }

                mLogger->FastLogD("=====================================================");
                if(MAX_COUNT > 0) {
                    mLogger->FastLogD("   Depth %d, FB %.2f, Distance %.2f\n", TARGET_DEPTH, FB, mDistance);
                } else {
                    mLogger->FastLogD("   Depth --, FB %.2f, Distance %.2f\n", FB, mDistance);
                }
                mLogger->FastLogD("=====================================================");
                mLogger->print();
            }
        }

    protected:
        bool                mDistnaceLogEnabled = property_get_bool(DISTANCE_LOG_PROPERTY, false);
        FastLogger          *mLogger = NULL;
        float               mDistance = 0.0f;
        DEPTH_VEC_T         mDepthCounts;
        DEPTH_VEC_T         mSortedDepthCounts;
    };

    class DistanceCalculatorOCC : public StereoDistanceCalculator
    {
    public:
        DistanceCalculatorOCC(FastLogger &logger)
            : StereoDistanceCalculator(logger)
        {
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
                    mLogger->FastLogD("Update INVALID_DEPTH_VALUE to %d", INVALID_DEPTH_VALUE);
                } else if(tuning.first == "occ.vec_offset") {
                    OCC_VEC_OFFSET = tuning.second;
                    mLogger->FastLogD("Update OCC_VEC_OFFSET to %d", OCC_VEC_OFFSET);
                }
            }
    #endif

            mDepthStartValue = OCC_VEC_OFFSET;
            if(0 == INVALID_DEPTH_VALUE) {
                mDepthStartValue = OCC_VEC_OFFSET + 1;
            }

            mLogger->FastLogD("INVALID_DEPTH_VALUE: %d\n"
                              "OCC_VEC_OFFSET     : %d\n"
                              "mDepthStartValue   : %d",
                              INVALID_DEPTH_VALUE, OCC_VEC_OFFSET, mDepthStartValue);
            mLogger->print();

            mSortedDepthCounts.resize(DEPTH_STATISTICS_SIZE - OCC_VEC_OFFSET - 1);  //1 represents the invalid value count
        }

        virtual ~DistanceCalculatorOCC() {}

        virtual float getDistance(NSCam::IImageBuffer *depthmap, NSCam::MRect afROI, MFLOAT focalLensFactor) override
        {
            AutoProfileUtil profile("StereoDistance", "DistanceCalculatorOCC::getDistance");

            _updateDepthCounts(depthmap, afROI);   //update mDepthCounts

            //Fill with mDepthStartValue, mDepthStartValue+1, ... and sort by count to get count->index vector
            std::iota(mSortedDepthCounts.begin(), mSortedDepthCounts.end(), mDepthStartValue);
            sort(mSortedDepthCounts.begin(), mSortedDepthCounts.end(),
                 [&](int a, int b) { return mDepthCounts[a] > mDepthCounts[b]; });

            int targetDepth = mSortedDepthCounts[0];
            mDistance = focalLensFactor;
            if(targetDepth > mDepthStartValue) {
                mDistance = focalLensFactor/(targetDepth - mDepthStartValue);
            }

            _logResult(targetDepth, focalLensFactor);

            return mDistance;
        }

        virtual void _logDepthmap(NSCam::IImageBuffer *depthmap, NSCam::MRect afROI)
        {
            mLogger->FastLogD("------------------------------------------------------\n"
                             "  AF ROI: (%d, %d) size %dx%d, depthmap size %dx%d\n"
                             "================= Content of AF ROI ==================",
                             afROI.p.x, afROI.p.y, afROI.s.w, afROI.s.h,
                             depthmap->getImgSize().w, depthmap->getImgSize().h);

            const size_t STRIDE = depthmap->getImgSize().w;
            MUINT8 *posHead = (MUINT8*)depthmap->getBufVA(0) + STRIDE * afROI.p.y + afROI.p.x;
            MUINT8 *posRun = posHead;

            int logDivisor = (int)(std::ceil((255-mDepthStartValue)/10.0f));
            char debugLine[afROI.s.w+2];
            debugLine[afROI.s.w+1] = 0;
            int debugValue = 0;
            int col = 0;
            for(int row = 0; row < afROI.s.h; ++row) {
                for(col = 0; col < afROI.s.w; ++col, ++posRun) {
                    mDepthCounts[*posRun]++;

                    if(*posRun==INVALID_DEPTH_VALUE) {
                        //Hole
                        debugLine[col] = ' ';
                    } else {
                        if(*posRun < mDepthStartValue) {
                            //Invalid depth
                            debugLine[col] = '-';
                        } else {
                            //Valid depth to 0~9
                            debugValue = (*posRun-mDepthStartValue)/logDivisor;
                            debugLine[col] = debugValue+48;
                        }
                    }
                }
                debugLine[col] = '|';
                mLogger->FastLogD("%s", debugLine);

                posHead += STRIDE;
                posRun = posHead;
            }

            mLogger->print();
        }

    private:
        int     INVALID_DEPTH_VALUE = 0;    //query from tuning later
        int     OCC_VEC_OFFSET      = 128;  //query from tuning later
        int     mDepthStartValue    = 0;
    };

    class DistanceCalculatorGF : public StereoDistanceCalculator
    {
    public:
        DistanceCalculatorGF(FastLogger &logger)
            : StereoDistanceCalculator(logger)
        {
        }

        virtual ~DistanceCalculatorGF() {}

        virtual float getDistance(NSCam::IImageBuffer *depthmap, NSCam::MRect afROI, MFLOAT focalLensFactor) override
        {
            AutoProfileUtil profile("StereoDistance", "DistanceCalculatorGF::getDistance");

            _updateDepthCounts(depthmap, afROI);   //update mDepthCounts

            //Fill with 0, 1, ... and sort by count to get count->index vector
            std::iota(mSortedDepthCounts.begin(), mSortedDepthCounts.end(), 0);
            sort(mSortedDepthCounts.begin(), mSortedDepthCounts.end(),
                [&](int a, int b) { return mDepthCounts[a] > mDepthCounts[b]; });
            int targetDepth = mSortedDepthCounts[0];

            mDistance = focalLensFactor;
            if(targetDepth > 0) {
                mDistance = focalLensFactor/targetDepth;
            }

            _logResult(targetDepth, focalLensFactor);

            return mDistance;
        }
    };

    class DistanceCalculatorWMF : public StereoDistanceCalculator
    {
    public:
        DistanceCalculatorWMF(FastLogger &logger)
            : StereoDistanceCalculator(logger)
        {
            MUINT32 coreNumber;
            GF_TUNING_T tuningTable;
            GF_TUNING_T dispCtrlPoints;
            GF_TUNING_T blurGainTable;
            TUNING_PAIR_LIST_T tuningParams;
            StereoTuningProvider::getGFTuningInfo(coreNumber,
                                                  tuningTable,
                                                  dispCtrlPoints,
                                                  blurGainTable,
                                                  tuningParams,
                                                  eSTEREO_SCENARIO_PREVIEW);

            for(auto &tuning : tuningParams) {
                if(tuning.first == "gf.bk_ifocus") {
                    WIN_SIZE = tuning.second;
                    mLogger->FastLogD("Update WIN_SIZE to %d", WIN_SIZE);
                } else if(tuning.first == "gf.bk_ifocus_ratio") {
                    BG_RATIO = tuning.second;
                    mLogger->FastLogD("Update BG_RATIO to %d", BG_RATIO);
                }
            }

            AF_AREA_SIZE = WIN_SIZE*WIN_SIZE;
            COUNT_THREASHOLD = AF_AREA_SIZE * BG_RATIO/100;

            mLogger->FastLogD("WIN_SIZE: %d\n"
                              "BG_RATIO: %d",
                              WIN_SIZE, BG_RATIO);
            mLogger->print();
        }

        virtual ~DistanceCalculatorWMF() {}

        // virtual int getMaxAFRectSize() { return WIN_SIZE; }

        virtual float getDistance(NSCam::IImageBuffer *depthmap, NSCam::MRect afROI, MFLOAT focalLensFactor) override
        {
            AutoProfileUtil profile("StereoDistance", "DistanceCalculatorWMF::getDistance");

            _updateDepthCounts(depthmap, afROI);   //update mDepthCounts

            // int cumulatedCount = 0;
            // int targetDepth = 0;
            // for(int depth = DEPTH_STATISTICS_SIZE-1; depth >= 0; --depth) {
            //     cumulatedCount += mDepthCounts[depth];
            //     if(cumulatedCount >= COUNT_THREASHOLD) {
            //         targetDepth = depth;
            //         break;
            //     }
            // }

            //Fill with 0, 1, ... and sort by count to get count->index vector
            std::iota(mSortedDepthCounts.begin(), mSortedDepthCounts.end(), 0);
            sort(mSortedDepthCounts.begin(), mSortedDepthCounts.end(),
                [&](int a, int b) { return mDepthCounts[a] > mDepthCounts[b]; });
            int targetDepth = mSortedDepthCounts[0];

            mDistance = focalLensFactor;
            if(targetDepth > 0) {
                mDistance = focalLensFactor/targetDepth;
            }

            if(mDistnaceLogEnabled) {
                // //Fill with 0, 1, ... and sort by count to get count->index vector
                // std::iota(mSortedDepthCounts.begin(), mSortedDepthCounts.end(), 0);
                // sort(mSortedDepthCounts.begin(), mSortedDepthCounts.end(),
                //     [&](int a, int b) { return mDepthCounts[a] > mDepthCounts[b]; });

                _logResult(targetDepth, focalLensFactor);
            }

            return mDistance;
        }
    private:
        int WIN_SIZE = 10;
        int BG_RATIO = 20;
        int COUNT_THREASHOLD = 20;
        int AF_AREA_SIZE = 100;
    };

private:
    //=============================================================
    // Private members
    //=============================================================
    float                                   mDistance = 0.0f;   //unit: cm

    StereoDistanceCalculator                *mpCalculator = NULL;
    const ENUM_DISTANCE_SOURCE_BUFFER       BUFFER_SRC;

    const NSCam::MSize                      AF_ROTATE_DOMAIN = StereoSizeProvider::getInstance()->getBufferSize(E_DMG);
    const NSCam::MSize                      DEPTHMAP_SIZE = StereoSizeProvider::getInstance()->getBufferSize((BUFFER_SRC==E_DIST_SRC_NOC) ? E_NOC : E_DMG);
    const bool                              IS_VERTICAL_MODULE   = (StereoSettingProvider::getModuleRotation() & 0x2);
    const bool                              IS_DEPTHMAP_ROTATED = (IS_VERTICAL_MODULE && (BUFFER_SRC != E_DIST_SRC_DMG));
    const int                               DEPTHMAP_WIDTH_FACTOR = IS_DEPTHMAP_ROTATED
                                                                    ? DEPTHMAP_SIZE.w/AF_ROTATE_DOMAIN.h
                                                                    : DEPTHMAP_SIZE.w/AF_ROTATE_DOMAIN.w;
    int                                     MAX_AF_RECT_SIZE;

    NS3Av3::DAF_TBL_STRUCT                  *mpAFTable          = NULL;
    int                                     mLastAFStatus       = 0;
    int                                     mDacMacro           = 0;
    int                                     mDacInf             = 0;
    float                                   mAFScaleW           = 1.0f;
    float                                   mAFScaleH           = 1.0f;
    int                                     mAFOffsetX          = 0;
    int                                     mAFOffsetY          = 0;

    FastLogger                              mLogger;
};

};  //namespace StereoHAL
#endif