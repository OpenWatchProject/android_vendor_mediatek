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
#ifndef _STEREO_COMMON_H_
#define _STEREO_COMMON_H_

#define DUMP_START_CAPTURE  3000
#define WITH16ALIGN true
#define STEREO_PROPERTY_PREFIX  "vendor.STEREO."

#include <stdint.h>
#include <math.h>
#include <mtkcam/def/common.h>
#include <cutils/properties.h>
#include <mtkcam/utils/std/Format.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <mtkcam/drv/IHalSensor.h>
#pragma GCC diagnostic pop
#include <camera_custom_stereo.h>   //for ENUM_STEREO_CAM_SCENARIO
#include <sys/stat.h>

namespace StereoHAL {

enum ENUM_STEREO_SCENARIO
{
    eSTEREO_SCENARIO_UNKNOWN,
    eSTEREO_SCENARIO_PREVIEW,
    eSTEREO_SCENARIO_RECORD,
    eSTEREO_SCENARIO_CAPTURE,
    TOTAL_STEREO_SCENARIO
};

enum ENUM_STEREO_SENSOR
{
    eSTEREO_SENSOR_UNKNOWN,
    eSTEREO_SENSOR_MAIN1,
    eSTEREO_SENSOR_MAIN2
};

typedef struct AF_WIN_COORDINATE_STRUCT
{
    MINT af_win_start_x;
    MINT af_win_start_y;
    MINT af_win_end_x;
    MINT af_win_end_y;

    inline  AF_WIN_COORDINATE_STRUCT()
            : af_win_start_x(0)
            , af_win_start_y(0)
            , af_win_end_x(0)
            , af_win_end_y(0)
            {
            }

    inline  AF_WIN_COORDINATE_STRUCT(MINT startX, MINT startY, MINT endX, MINT endY)
            : af_win_start_x(startX)
            , af_win_start_y(startY)
            , af_win_end_x(endX)
            , af_win_end_y(endY)
            {
            }

    inline  NSCam::MPoint centerPoint()
            {
                return NSCam::MPoint((af_win_start_x+af_win_end_x)/2, (af_win_start_y+af_win_end_y)/2);
            }

} *P_AF_WIN_COORDINATE_STRUCT;

/**
 * \brief Get stereo scenario
 * \details Note that record sensor scenario is unsupported
 *
 * \param sensorCenario Sensor scenario
 * \return sensor scenario
 */
inline ENUM_STEREO_SCENARIO getStereoSenario(int sensorCenario)
{
    switch(sensorCenario) {
        case NSCam::SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
        case NSCam::SENSOR_SCENARIO_ID_CUSTOM2:
            return eSTEREO_SCENARIO_RECORD;
            break;
        case NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
        case NSCam::SENSOR_SCENARIO_ID_CUSTOM1:
            return eSTEREO_SCENARIO_CAPTURE;
            break;
        default:
            break;
    }

    return eSTEREO_SCENARIO_UNKNOWN;
}

/**
 * \brief Get sensor scenario
 * \details Get sensor scenario, note that preview and record both use preview scenario.
 *
 * \param eScenario Stereo scenario
 * \return sensor scenario
 */
// inline int getSensorSenario(ENUM_STEREO_SCENARIO eScenario)
// {
//     switch(eScenario) {
//         case eSTEREO_SCENARIO_PREVIEW:
//         case eSTEREO_SCENARIO_RECORD:
//             return NSCam::SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
//             break;
//         // case eSTEREO_SCENARIO_RECORD:
//         //     return NSCam::SENSOR_SCENARIO_ID_NORMAL_VIDEO;
//         //     break;
//         case eSTEREO_SCENARIO_CAPTURE:
//             return NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
//             break;
//         default:
//             break;
//     }

//     return NSCam::SENSOR_SCENARIO_ID_UNNAMED_START;
// }

/**
* \brief Diff two time
* \details Diff two time, sample code:
*           struct timespec t_start, t_end, t_result;
*           clock_gettime(CLOCK_MONOTONIC, &t_start);
*           ...
*           clock_gettime(CLOCK_MONOTONIC, &t_end);
*           t_result = timeDiff(t_start, t_end);
*           ALOGD("Runnning Time: %lu.%.9lu", t_result.tv_sec, t_result.tv_nsec);
*
* \param timespec Start of the time
* \param timespec End of the time
*
* \return Diff of two times
*/
inline struct timespec timeDiff( struct timespec start, struct timespec end)
{
    struct timespec t_result;

    if( ( end.tv_nsec - start.tv_nsec ) < 0) {
        t_result.tv_sec = end.tv_sec - start.tv_sec - 1;
        t_result.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    } else {
        t_result.tv_sec = end.tv_sec - start.tv_sec;
        t_result.tv_nsec = end.tv_nsec - start.tv_nsec;
    }

    return t_result;
}

/**
 * \brief Check system property's value, only for number values
 * \details Check system property's value, only for number values
 *
 * \param PROPERTY_NAME The property to query, e.g. "vendor.STEREO.enable_verify"
 * \param DEFAULT Default value of the property. Default is 0.
 * \return -1: property not been set; otherwise the property value
 */
inline int checkStereoProperty(const char *PROPERTY_NAME, const int DEFAULT=0)
{
    return property_get_int32(PROPERTY_NAME, DEFAULT);
}

/**
 * \brief Set system property
 *
 * \param PROPERTY_NAME Property to set
 * \param val value of the property
 *
 * \return true if success
 */
inline bool setProperty(const char *PROPERTY_NAME, int val)
{
    if(NULL == PROPERTY_NAME) {
        return false;
    }

    char value[PROPERTY_VALUE_MAX];
    sprintf(value, "%d", val);
    int ret = property_set(PROPERTY_NAME, value);

    return (0 == ret);
}

inline ENUM_STEREO_CAM_SCENARIO transferStereoScenario(ENUM_STEREO_SCENARIO s)
{
    switch(s) {
        case eSTEREO_SCENARIO_PREVIEW:
        default:
            return eStereoCamPreview;
            break;
        case eSTEREO_SCENARIO_RECORD:
            return eStereoCamRecord;
            break;
        case eSTEREO_SCENARIO_CAPTURE:
            return eStereoCamCapture;
            break;
    }

    return eStereoCamPreview;
}

inline NSCam::MPoint
rotatePoint(NSCam::MPoint ptIn, const NSCam::MSize DOMAIN_SIZE_HORIZONTAL, ENUM_ROTATION rotate)
{
    if(eRotate_0 == rotate) {
        return ptIn;
    }

    NSCam::MPoint newPt = ptIn;
    switch(rotate) {
        case eRotate_90:
            newPt.x = DOMAIN_SIZE_HORIZONTAL.h - ptIn.y;
            newPt.y = ptIn.x;
            break;
        case eRotate_180:
            newPt.x = DOMAIN_SIZE_HORIZONTAL.w - ptIn.x;
            newPt.y = DOMAIN_SIZE_HORIZONTAL.h - ptIn.y;
            break;
        case eRotate_270:
            newPt.x = ptIn.y;
            newPt.y = DOMAIN_SIZE_HORIZONTAL.w - ptIn.x;
            break;
        default:
            break;
    }

    return newPt;
}

inline NSCam::MRect
rotateRect(NSCam::MRect rect, const NSCam::MSize DOMAIN_SIZE_HORIZONTAL, ENUM_ROTATION rotate)
{
    if(eRotate_0 == rotate) {
        return rect;
    }

    NSCam::MRect newRect = rect;
    switch(rotate) {
        case eRotate_90:
            newRect.p = rotatePoint(rect.leftBottom(), DOMAIN_SIZE_HORIZONTAL, rotate);
            std::swap(newRect.s.w, newRect.s.h);
            break;
        case eRotate_180:
            newRect.p = rotatePoint(rect.rightBottom(), DOMAIN_SIZE_HORIZONTAL, rotate);
            break;
        case eRotate_270:
            newRect.p = rotatePoint(rect.rightTop(), DOMAIN_SIZE_HORIZONTAL, rotate);
            std::swap(newRect.s.w, newRect.s.h);
            break;
        default:
            break;
    }

    return newRect;
}

inline unsigned int StereoGCD(unsigned int u, unsigned int v)
{
    //The binary GCD algorithm, also known as Stein's algorithm
    if(0 == u) return v;
    if(0 == v) return u;

    //__builtin_ctz: count the number of tail zeros
    //equals to while ((v & 1) == 0) v >>= 1;
    int shift = __builtin_ctz(u | v);
    u >>= __builtin_ctz(u);
    unsigned int t = 0;
    do {
        v >>= __builtin_ctz(v);
        if (u > v) {
            t = v;
            v = u;
            u = t;
        }
        v = v - u;
    } while (v != 0);

    return u << shift;
}

/**
 * \brief Convert degree to radians
 *
 * \param degree degree
 * \return positive radians
 */
inline float degreeToRadians(float degree)
{
    degree = fmod(degree, 360.0f);
    if(degree < 0.0f) {
        degree += 360.0f;
    }
    return degree * M_PI/180.0f;
}

/**
 * \brief Convert radians to degree
 *
 * \param radians radians
 * \return positive degree
 */
inline float radiansToDegree(float radians)
{
    radians = fmod(radians, 2.0f*M_PI);
    if(radians < 0.0f) {
        radians += 2.0f*M_PI;
    }
    return radians * 180.0f/M_PI;
}

/**
 * \brief Ratio of degrees degree1/degree2
  *
 * \param degree1 Degree1
 * \param degree2 Degree2
 *
 * \return Ratio of degree, e.g. 50deg/60deg->0.8076
 */
inline float degreeRatio(float degree1, float degree2)
{
    return tan(degreeToRadians(degree1/2.0f))/tan(degreeToRadians(degree2/2.0f));
}

/**
 * \brief Ratio of radians/radians2
 *
 * \param radians1 Radians1
 * \param radians2 Radians2
 *
 * \return Ratio of radians'
 */
inline float radiansRatio(float radians1, float radians2)
{
    return tan(radians1/2.0f)/tan(radians2/2.0f);
}

/**
 * \brief Get image ratio proportion m:n
 * \details eRatio_16_9 will get m = 16, n = 9, etc
 *
 * \param m antecedent of ratio, if 16:9, m is 16
 * \param n consequent of ratio, if 16:9, n is 9
 * \param ratio Input ratio, use current image ratio as default
 */
inline void imageRatioMToN(ENUM_STEREO_RATIO ratio, int &m, int &n)
{
    if(eRatio_Unknown == ratio) {
        ratio = eRatio_Default;
    }

#ifdef STEREO_RATIO_CONSEQUENT_BITS
    m = ratio >> STEREO_RATIO_CONSEQUENT_BITS;
    n = ratio & STEREO_RATIO_CONSEQUENT_MASK;
#else
    switch(ratio) {
    case eRatio_16_9:
        m = 16;
        n = 9;
        break;
    case eRatio_4_3:
        m = 4;
        n = 3;
        break;
    default:
        imageRatioMToN(eRatio_Default, m, n);
        break;
    }
#endif
}

inline ENUM_STEREO_RATIO MToNToImageRatio(unsigned int m, unsigned int n)
{
#ifdef STEREO_RATIO_CONSEQUENT_BITS
    return static_cast<ENUM_STEREO_RATIO>(m<<STEREO_RATIO_CONSEQUENT_BITS|n);
#else
    if(16 == m && 9 == n) {
        return eRatio_16_9;
    }

    return eRatio_4_3;
#endif
}

inline int applyNAlign(unsigned int n, int &value) {
    if(n > 1 &&
       __builtin_popcount(n) == 1)  //must be power of 2, i.e. 2, 4, 8, 16, etc
    {
        int m = n-1;    //Up scale
        value = (value+m) & (~m);
    }
    return value;
}

inline int applyNAlignRounding(unsigned int n, int &value) {
    if(n > 1 &&
       __builtin_popcount(n) == 1)  //must be power of 2, i.e. 2, 4, 8, 16, etc
    {
        int m = n>>1;
        value = (value+m) & (~(n-1));
    }
    return value;
}

};
#endif