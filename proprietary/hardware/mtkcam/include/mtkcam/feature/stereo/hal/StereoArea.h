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
#ifndef _STEREO_AREA_H_
#define _STEREO_AREA_H_

#include <stdint.h>
#include <math.h>
#include <algorithm>
#include <cmath>        //for std::ceil
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Format.h>
#include "stereo_setting_provider.h"

namespace StereoHAL {

const NSCam::MSize  MSIZE_ZERO(0, 0);
const NSCam::MPoint MPOINT_ZERO(0, 0);

enum ENUM_IMAGE_RATIO_ALIGNMENT
{
    E_KEEP_AREA_SIZE,   //(old width * old height) ~= (new width * new height)
    E_KEEP_WIDTH,       //Keep width, change height
    E_KEEP_HEIGHT,      //Keep height, change width
};

enum ENUM_SIZE_ALIGNMENT
{
    E_128_ALIGN = 128,
    E_64_ALIGN  = 64,
    E_32_ALIGN  = 32,
    E_16_ALIGN  = 16,
    E_8_ALIGN   = 8,
    E_4_ALIGN   = 4,
    E_2_ALIGN   = 2,
    E_NO_ALIGN  = 1,
};

/**
 * \brief Describes the stereo area
 * \details Stereo area consists of three parts: size, padding and start point.
 *          The area can have two rectagles, just like:
 *          ┌───────────────┐
 *          │ ┌───────────┐ │
 *          │ │           │ │
 *          │ │           │ │
 *          │ └───────────┘ │
 *          └───────────────┘
 *          Size: The outter size of the area
 *          Start point: The top-left position of the inner rect related to the outter rect
 *          Padding: The size of the outter rect - the size of the inner rect,
 *                   which meas it's the sum of the spaces.
 */
struct StereoArea {
    NSCam::MSize size;
    NSCam::MSize padding;
    NSCam::MPoint startPt;

    /**
     * \brief Default constructor
     */
    inline StereoArea()
            : size()
            , padding()
            , startPt()
            {
            }

    /**
     * \brief Construct StereoArea with flatten parameteres
     * \details Construct StereoArea with flatten parameteres
     *
     * \param w Width of size
     * \param h Height of size
     * \param paddingX Horizontal padding of the area, default is 0
     * \param paddingY Vertical padding of the area, default is 0
     * \param startX X position of the start point of content, default is 0
     * \param startY Y position of the start point of content, default is 0
     */
    inline StereoArea(MUINT32 w, MUINT32 h, MUINT32 paddingX=0, MUINT32 paddingY=0, MUINT32 startX = 0, MUINT32 startY = 0)
            : size(w, h)
            , padding(paddingX, paddingY)
            , startPt(startX, startY)
            {
            }

    /**
     * \brief Construct StereoArea with structured parameteres
     * \details Construct StereoArea with structured parameteres
     *
     * \param sz Size of the area
     * \param p Padding of the area
     * \param pt Start point of content
     */
    inline StereoArea(NSCam::MSize sz, NSCam::MSize pad=MSIZE_ZERO, NSCam::MPoint pt = MPOINT_ZERO)
            : size(sz)
            , padding(pad)
            , startPt(pt)
            {
            }

    /**
     * \brief Assign operator, it does deep copy
     * \details Assign operator, it does deep copy
     *
     * \param rhs Source area
     */
    inline StereoArea &operator=(const StereoArea &rhs)
            {
                size = rhs.size;
                padding = rhs.padding;
                startPt = rhs.startPt;

                return *this;
            }

    /**
     * \brief Compare operator
     * \details Compares two areas
     *
     * \param rhs Compared area
     * \return True if all data is the same
     */
    inline bool operator==(const StereoArea &rhs) const
            {
                if(size != rhs.size) return false;
                if(padding != rhs.padding) return false;
                if(startPt != rhs.startPt) return false;

                return true;
            }

    /**
     * \brief Compare operator
     * \details Compares two areas
     *
     * \param rhs Compared area
     * \return True if all data is different
     */
    inline bool operator!=(const StereoArea &rhs) const
            {
                if(size != rhs.size) return true;
                if(padding != rhs.padding) return true;
                if(startPt != rhs.startPt) return true;

                return false;
            }

    /**
     * \brief Construct with another area
     * \details Construct with another area, deep copies the data from source
     *
     * \param rhs Construct source
     */
    inline StereoArea(const StereoArea &rhs)
            : size(rhs.size)
            , padding(rhs.padding)
            , startPt(rhs.startPt)
            {
            }

    /**
     * \brief Default type convertor to NSCam::MSize
     * \return size of the area
     */
    inline operator NSCam::MSize() const {
        return size;
    }

    /**
     * \brief Get content size of the area
     * \details Get content size of the area
     * \return The size of content
     */
    inline NSCam::MSize contentSize() const {
        return (size - padding);
    }

    /**
     * \brief Get product area
     *
     * \param RATIO Multiplier
     */
    inline StereoArea &operator*=(const MFLOAT RATIO) {
        size.w *= RATIO;
        size.h *= RATIO;
        padding.w *= RATIO;
        padding.h *= RATIO;
        startPt.x *= RATIO;
        startPt.y *= RATIO;

        return *this;
    }

    /**
     * \brief Get new product area
     *
     * \param RATIO Multiplier
     * \return New product area
     */
    inline StereoArea operator *(const MFLOAT RATIO) const {
        StereoArea newArea( *this );
        newArea *= RATIO;

        return newArea;
    }

    /**
     * \brief Get divided area
     *
     * \param RATIO Divisor
     */
    inline StereoArea &operator/=(const MFLOAT RATIO) {
        size.w /= RATIO;
        size.h /= RATIO;
        padding.w /= RATIO;
        padding.h /= RATIO;
        startPt.x /= RATIO;
        startPt.y /= RATIO;

        return *this;
    }

    /**
     * \brief Get new divide area
     *
     * \param RATIO Divisor
     * \return New divided area
     */
    inline StereoArea operator /(const MFLOAT RATIO) const {
        StereoArea newArea( *this );
        newArea /= RATIO;

        return newArea;
    }

    /**
     * \brief Apply rotation
     * \details As default we will centralize the content
     *
     * \param rotation Clockwised rotation degree
     * \param CENTRALIZE_CONTENT (default)true if you want to centralize content, padding will not change
     *                           false if you really want to rotate size, padding and start point
     * \return Rotated area
     * \see StereoSettingProvider::getModuleRotation()
     */
    inline StereoArea &rotate(ENUM_ROTATION rotation, const bool CENTRALIZE_CONTENT=true) {
        if(CENTRALIZE_CONTENT) {
            switch(rotation)
            {
            case eRotate_0:
            case eRotate_180:
            default:
                break;
            case eRotate_90:
            case eRotate_270:
                {
                    //Only content rotates, padding does not change
                    NSCam::MSize szContent = contentSize();
                    StereoArea rotatedArea(szContent.h+padding.w, szContent.w+padding.h, padding.w, padding.h);
                    rotatedArea.startPt.x = padding.w/2;
                    rotatedArea.startPt.y = padding.h/2;
                    *this = rotatedArea;
                }
                break;
            }
        } else {
            switch(rotation)
            {
            case eRotate_0:
            default:
                break;
            case eRotate_180:
                {
                    StereoArea rotatedArea(*this);
                    rotatedArea.startPt.x = size.w - contentSize().w - startPt.x;
                    rotatedArea.startPt.y = size.h - contentSize().h - startPt.y;
                    *this = rotatedArea;
                }
                break;
            case eRotate_90:
                {
                    StereoArea rotatedArea(size.h, size.w, padding.h, padding.w);
                    rotatedArea.startPt.x = size.h - contentSize().h - startPt.y;
                    rotatedArea.startPt.y = startPt.x;
                    *this = rotatedArea;
                }
                break;
            case eRotate_270:
                {
                    StereoArea rotatedArea(size.h, size.w, padding.h, padding.w);
                    rotatedArea.startPt.x = startPt.y;
                    rotatedArea.startPt.y = size.w - contentSize().w - startPt.x;
                    *this = rotatedArea;
                }
                break;
            }
        }

        return *this;
    }

    /**
     * \brief Apply rotation with module orientation
     * \details As default we will centralize the content
     *
     * \param CENTRALIZE_CONTENT (default)true if you want to centralize content, padding will not change
     *                           false if you really want to rotate size, padding and start point
     * \return Rotated area
     * \see StereoSettingProvider::getModuleRotation()
     */
    inline StereoArea &rotatedByModule(const bool CENTRALIZE_CONTENT=true) {
        return rotate(StereoSettingProvider::getModuleRotation(), CENTRALIZE_CONTENT);
    }

    /**
     * \brief add extra padding in height
     * \details add extra padding to the height of the area
     *
     * \param extendRatio
     * \return Padded area
     * \see StereoSettingProvider::addPaddingToHeight()
     */
    inline StereoArea &addPaddingToHeight(float extendRatio) {

        float fPaddingHeight = (float)(*this).size.h*extendRatio;

        int paddingHeight = static_cast<int>(fPaddingHeight+0.5);

        (*this).startPt.x = 0;
        (*this).startPt.y = 0;

        (*this).padding.w = 0;
        (*this).padding.h = paddingHeight;

        (*this).size.h = (*this).size.h + paddingHeight;

        return *this;
    }

    /**
     * \brief Apply N-align to size
     *        N-align meas its size is multiple of N
     *        N must be one of 128/64/32/16/8/2
     * \details Sometimes user needs N-aligned size for further process
     *
     * \param n must be one of 128/64/32/16/8/2
     * \param keepContent keep content and only changes padding
     * \param centerContent set to centerize content if padding is not zero
     * \return N-aligned area
     */
    inline StereoArea &applyNAlign(ENUM_SIZE_ALIGNMENT n, bool keepContent=false, bool centerContent=true) {
        NSCam::MSize szContent = contentSize();

        StereoHAL::applyNAlign(n, size.w);
        StereoHAL::applyNAlign(n, size.h);

        if(MSIZE_ZERO != padding ||
           keepContent)
        {
            padding = size - szContent;

            if(centerContent) {
                startPt.x = padding.w>>1;
                startPt.y = padding.h>>1;
            }
        }

        return *this;
    }

    inline StereoArea &apply128Align(bool keepContent=false, bool centerContent=true) { return applyNAlign(E_128_ALIGN, keepContent, centerContent); }
    inline StereoArea &apply64Align(bool keepContent=false, bool centerContent=true) { return applyNAlign(E_64_ALIGN, keepContent, centerContent); }
    inline StereoArea &apply32Align(bool keepContent=false, bool centerContent=true) { return applyNAlign(E_32_ALIGN, keepContent, centerContent); }
    inline StereoArea &apply16Align(bool keepContent=false, bool centerContent=true) { return applyNAlign(E_16_ALIGN, keepContent, centerContent); }
    inline StereoArea &apply8Align(bool keepContent=false, bool centerContent=true)  { return applyNAlign(E_8_ALIGN,  keepContent, centerContent); }
    inline StereoArea &apply2Align(bool keepContent=false, bool centerContent=true)  { return applyNAlign(E_2_ALIGN,  keepContent, centerContent); }

    /**
     * \brief Apply N-align to size, size will be rounding, which means it may be smaller after rounding
     *        N-align meas its size is multiple of N
     *        N must be one of 32/16/8/2
     * \details Sometimes user needs N-aligned size for further process
     *
     * \param n must be one of 32/16/8/2
     * \param keepContent keep content and only changes padding
     * \param centerContent set to centerize content if padding is not zero
     * \return N-aligned area
     */
    inline StereoArea &applyNAlignRounding(ENUM_SIZE_ALIGNMENT n, bool keepContent=false, bool centerContent=true) {
        NSCam::MSize szContent = contentSize();

        StereoHAL::applyNAlignRounding(n, size.w);
        StereoHAL::applyNAlignRounding(n, size.h);

        if(size.w < szContent.w) {
            size.w += n;
        }

        if(size.h < szContent.h) {
            size.h += n;
        }

        if(MSIZE_ZERO != padding ||
           keepContent)
        {
            padding = size - szContent;

            if(centerContent) {
                startPt.x = padding.w>>1;
                startPt.y = padding.h>>1;
            }
        }

        return *this;
    }

    inline StereoArea &apply32AlignRounding(bool keepContent=false, bool centerContent=true) { return applyNAlignRounding(E_32_ALIGN, keepContent, centerContent); }
    inline StereoArea &apply16AlignRounding(bool keepContent=false, bool centerContent=true) { return applyNAlignRounding(E_16_ALIGN, keepContent, centerContent); }
    inline StereoArea &apply8AlignRounding(bool keepContent=false, bool centerContent=true)  { return applyNAlignRounding(E_8_ALIGN,  keepContent, centerContent); }
    inline StereoArea &apply2AlignRounding(bool keepContent=false, bool centerContent=true)  { return applyNAlignRounding(E_2_ALIGN,  keepContent, centerContent); }

    /**
     * \brief Apply N-align to content size. This will change overall size if needed.
     *        N-align meas its size is multiple of N
     *        N must be one of 32/16/8/2
     * \details Sometimes user needs N-aligned size for further process
     * \return N-aligned area
     */
    inline StereoArea &applyNAlignToContent(ENUM_SIZE_ALIGNMENT n) {
        NSCam::MSize szContent = contentSize();
        StereoHAL::applyNAlign(n, szContent.w);
        StereoHAL::applyNAlign(n, szContent.h);
        padding = size - szContent;

        return *this;
    }

    inline StereoArea &apply32AlignToContent() { return applyNAlignToContent(E_32_ALIGN); }
    inline StereoArea &apply16AlignToContent() { return applyNAlignToContent(E_16_ALIGN); }
    inline StereoArea &apply8AlignToContent()  { return applyNAlignToContent(E_8_ALIGN); }
    inline StereoArea &apply2AlignToContent()  { return applyNAlignToContent(E_2_ALIGN); }

    /**
     * \brief Apply N align to outter width
     *        N must be one of 128/64/32/16/8/2
     * \details Content size will not change, padding.w and startPt.x may change
     * \return Area with N-aligned outter width
     */
    inline StereoArea &applyNAlignToWidth(ENUM_SIZE_ALIGNMENT n, bool keepContent=true, bool centerContent=true) {
        int contentWidth = size.w - padding.w;
        StereoHAL::applyNAlign(n, size.w);

        if(keepContent) {
            padding.w = size.w - contentWidth;
        }

        if(centerContent) {
            startPt.x = padding.w>>1;
        }

        return *this;
    }

    inline StereoArea &apply128AlignToWidth(bool keepContent=true, bool centerContent=true) { return applyNAlignToWidth(E_128_ALIGN, keepContent, centerContent); }
    inline StereoArea &apply64AlignToWidth(bool keepContent=true, bool centerContent=true) { return applyNAlignToWidth(E_64_ALIGN, keepContent, centerContent); }
    inline StereoArea &apply32AlignToWidth(bool keepContent=true, bool centerContent=true) { return applyNAlignToWidth(E_32_ALIGN, keepContent, centerContent); }
    inline StereoArea &apply16AlignToWidth(bool keepContent=true, bool centerContent=true) { return applyNAlignToWidth(E_16_ALIGN, keepContent, centerContent); }
    inline StereoArea &apply8AlignToWidth(bool keepContent=true, bool centerContent=true)  { return applyNAlignToWidth(E_8_ALIGN, keepContent, centerContent); }
    inline StereoArea &apply2AlignToWidth(bool keepContent=true, bool centerContent=true)  { return applyNAlignToWidth(E_2_ALIGN, keepContent, centerContent); }

    /**
     * \brief Apply N align to outter height
     *        N must be one of 128/64/32/16/8/2
     * \details Content size will not change, padding.w and startPt.x may change
     * \return Area with N-aligned outter height
     */
    inline StereoArea &applyNAlignToHeight(ENUM_SIZE_ALIGNMENT n, bool keepContent=true, bool centerContent=true) {
        int contentHeight = size.h - padding.h;
        StereoHAL::applyNAlign(n, size.h);

        if(keepContent) {
            padding.h = size.h - contentHeight;
        }

        if(centerContent) {
            startPt.y = padding.h>>1;
        }

        return *this;
    }

    inline StereoArea &apply128AlignToHeight(bool keepContent=true, bool centerContent=true) { return applyNAlignToHeight(E_128_ALIGN, keepContent, centerContent); }
    inline StereoArea &apply64AlignToHeight(bool keepContent=true, bool centerContent=true) { return applyNAlignToHeight(E_64_ALIGN, keepContent, centerContent); }
    inline StereoArea &apply32AlignToHeight(bool keepContent=true, bool centerContent=true) { return applyNAlignToHeight(E_32_ALIGN, keepContent, centerContent); }
    inline StereoArea &apply16AlignToHeight(bool keepContent=true, bool centerContent=true) { return applyNAlignToHeight(E_16_ALIGN, keepContent, centerContent); }
    inline StereoArea &apply8AlignToHeight(bool keepContent=true, bool centerContent=true)  { return applyNAlignToHeight(E_8_ALIGN, keepContent, centerContent); }
    inline StereoArea &apply2AlignToHeight(bool keepContent=true, bool centerContent=true)  { return applyNAlignToHeight(E_2_ALIGN, keepContent, centerContent); }

    /**
     * \brief Apply N align to content width
     *        N must be one of 32/16/8/2
     * \details Padding will not change, size.w and startPt.x may change
     *
     * \param n alignment
     * \return Area with N-aligned content width
     */
    inline StereoArea &applyNAlignToContentWidth(ENUM_SIZE_ALIGNMENT n) {
        int contentWidth = size.w - padding.w;
        StereoHAL::applyNAlign(n, contentWidth);

        size.w = contentWidth + padding.w;

        return *this;
    }

    inline StereoArea &apply32AlignToContentWidth() { return applyNAlignToContentWidth(E_32_ALIGN); }
    inline StereoArea &apply16AlignToContentWidth() { return applyNAlignToContentWidth(E_16_ALIGN); }
    inline StereoArea &apply8AlignToContentWidth()  { return applyNAlignToContentWidth(E_8_ALIGN); }
    inline StereoArea &apply2AlignToContentWidth()  { return applyNAlignToContentWidth(E_2_ALIGN); }

    /**
     * \brief Apply N align to content height
     *        N must be one of 32/16/8/2
     * \details Padding will not change, size.w and startPt.x may change
     *
     * \param n alignment
     * \return Area with N-aligned content height
     */
    inline StereoArea &applyNAlignToContentHeight(ENUM_SIZE_ALIGNMENT n) {
        int contentHeight = size.h - padding.h;
        StereoHAL::applyNAlign(n, contentHeight);

        size.h = contentHeight + padding.h;

        return *this;
    }

    inline StereoArea &apply32AlignToContentHeight() { return applyNAlignToContentHeight(E_32_ALIGN); }
    inline StereoArea &apply16AlignToContentHeight() { return applyNAlignToContentHeight(E_16_ALIGN); }
    inline StereoArea &apply8AlignToContentHeight()  { return applyNAlignToContentHeight(E_8_ALIGN); }
    inline StereoArea &apply2AlignToContentHeight()  { return applyNAlignToContentHeight(E_2_ALIGN); }

    /**
     * \brief Apply ratio to content size, padding will not change, and outter size will be content size + padding
     * \details No 16-align or 2-align guaranteed, if original size has the similar ratio, it does nothing.
     *
     * \param ratio Usally pass StereoSettingProvider::imageRatio()
     * \param alignment Alignment to keep when ratio changes
     * \return Area with ratio changed
     */
    inline StereoArea &applyRatio(ENUM_STEREO_RATIO ratio, ENUM_IMAGE_RATIO_ALIGNMENT alignment=E_KEEP_AREA_SIZE) {
        const int MAX_ERROR = 2;    // percent
        NSCam::MSize szContent = contentSize();

        int m, n;
        imageRatioMToN(ratio, m, n);
        if(E_KEEP_WIDTH == alignment) {
            szContent.h = szContent.w * n / m;
            size = szContent + padding;
            return *this;
        } else if(E_KEEP_HEIGHT == alignment) {
            szContent.w = szContent.h * m / n ;
            size = szContent + padding;
            return *this;
        }

        static const std::map<std::pair<int, int>, ENUM_STEREO_RATIO> RATIO_MAP =
        {   //{a, b}
            {{16, 9}, eRatio_16_9},
            {{4,  3}, eRatio_4_3},
        };

        int a, b;
        ENUM_STEREO_RATIO contentRatio = ratio;
        bool isKnownRatio = false;
        for(auto &r : RATIO_MAP) {
            a = r.first.first;
            b = r.first.second;
            if(abs(szContent.w * b / a - szContent.h) < szContent.h * MAX_ERROR / 100) {
                contentRatio = r.second;
                if(contentRatio == ratio) {
                    return *this;
                }
                isKnownRatio = true;
                break;
            }
        }

        //Keep area size
        const int PRECISION = 10000;    //1.0000
        if(!isKnownRatio) {
            a = szContent.w;
            b = szContent.h;
        }

        int WIDTH_RATIO = (int)(std::sqrt(b/(double)a*m/n)*PRECISION);
        szContent.w = szContent.w * WIDTH_RATIO / PRECISION;
        szContent.h = szContent.w * n / m;
        size = szContent + padding;

        return *this;
    }

    /**
     * \brief Apply ratio change to size.w, padding.w and startPt.x
     * \details Height won't change
     *
     * \param ratio Ratio to change
     * \return Area with width changed
     */
    inline StereoArea &applyWidthRatio(const float ratio=1.0f)
    {
        if(1.0f == ratio ||
           ratio < 0.0f)
        {
            return *this;
        }

        if(0 == padding.w) {
            size.w = (int)(size.w*ratio) & ~1;
        } else {
            NSCam::MSize szContent = contentSize();
            size.w = (int)(size.w*ratio) & ~1;

            szContent.w = (int)(szContent.w * ratio);
            padding.w = size.w - szContent.w;
            startPt.x = (int)(startPt.x * ratio);
        }

        return *this;
    }

    /**
     * \brief Apply ratio change to size.h, padding.h and startPt.y
     * \details Width won't change
     *
     * \param ratio Ratio to change
     * \return Area with height changed
     */
    inline StereoArea &applyHeightRatio(const float ratio=1.0f)
    {
        if(1.0f == ratio ||
           ratio < 0.0f)
        {
            return *this;
        }

        if(0 == padding.h) {
            size.h = (int)(size.h*ratio) & ~1;
        } else {
            NSCam::MSize szContent = contentSize();
            size.h = (int)(size.h*ratio) & ~1;

            szContent.h = (int)(szContent.h * ratio);
            padding.h = size.h - szContent.h;
            startPt.y = (int)(startPt.y * ratio);
        }

        return *this;
    }

    /**
     * \brief Double size.w, padding.w and startPt.x
     *
     * \return Area with doubled width
     */
    inline StereoArea &applyDoubleWidth()
    {
        return applyWidthRatio(2.0f);
    }

    /**
     * \brief Double size.w, padding.w and startPt.x
     *
     * \return Area with doubled width
     */
    inline StereoArea &applyDoubleHeight()
    {
        return applyHeightRatio(2.0f);
    }

    /**
     * \brief Remove padding
     * \details Size will be content size, padding and startPt will be 0
     *
     * \return Area without padding
     */
    inline StereoArea &removePadding()
    {
        size = contentSize();
        padding = MSIZE_ZERO;
        startPt = MPOINT_ZERO;
        return *this;
    }

    /**
     * \brief Apply ratio to content, keep width and change height
     *
     * \param ratio Ratio to apply
     * \return Area with content size change by ratio
     */
    inline StereoArea &applyRatioToContent(ENUM_STEREO_RATIO ratio, bool isVertical)
    {
        NSCam::MSize szContent = contentSize();
        int m, n;
        imageRatioMToN(ratio, m, n);

        if(isVertical) {
            szContent.h = szContent.w * m / n;
        } else {
            szContent.h = szContent.w * n / m;
        }
        StereoHAL::applyNAlign(E_2_ALIGN, szContent.h);

        size = padding + szContent;

        return *this;
    }

    /**
     * \brief Apply ratio to outter size, keep width and change height
     *
     * \param ratio Ratio to apply
     * \return Area with content size change by ratio
     */
    inline StereoArea &applyRatioToSize(ENUM_STEREO_RATIO ratio, bool isVertical)
    {
        int m, n;
        imageRatioMToN(ratio, m, n);

        if(isVertical) {
            size.h = size.w * m / n;
        } else {
            size.h = size.w * n / m;
        }
        StereoHAL::applyNAlign(E_2_ALIGN, size.h);

        return *this;
    }

    /**
     * \brief Apply ratio to original size and keep original ratio
     * \details 2-align will cause ratio and a little bit, this will affect depth quality.
     *          So we need to make sure the width/height ratio is exactly the same ratio before enlarged.
     *          In worst case, width and height will be doubled if ratio is 1.x
     *
     * \param RATIO Ratio to enlarge, must > 1.0f
     * \return Enlarged area
     */
    inline StereoArea &enlargeAndKeepOriginalRatio(const float RATIO)
    {
        if(RATIO <= 1.0f) {
            return *this;
        }

        int gcd = StereoGCD(size.w, size.h);
        StereoArea baseSize(size.w/gcd, size.h/gcd);

        *this *= RATIO;

        //Resized size should have exacly same ratio as main1
        int factor = (int)std::ceil(std::max(size.w/(float)baseSize.size.w, size.h/(float)baseSize.size.h));
        factor = (factor + 1) & ~1;   //Factor must be even since baseSize is odd
        *this = baseSize * factor;

        return *this;
    }

    /**
     * \brief Apply ratio and round to closest even number
     *
     * \param RATIO Ratio to enlarge, must > 1.0f
     * \return Enlarged area
     */
    inline StereoArea &enlargeWith2AlignedRounding(const float RATIO)
    {
        if(RATIO <= 1.0f) {
            return *this;
        }

        size.w = (int)(std::round((size.w * RATIO)/2.0f)*2.0f);
        size.h = (int)(std::round((size.h * RATIO)/2.0f)*2.0f);

        return *this;
    }

    /**
     * \brief Print area, mostly for debugging
     *
     * \param TAG LOG_TAG
     * \param BUFFER_NAME name of the buffer
     * \return Area itself
     */
    inline StereoArea &print(const char *TAG, const char *BUFFER_NAME) {
        if(StereoSettingProvider::isLogEnabled()) {
            __android_log_print(ANDROID_LOG_DEBUG, TAG,
                                "[%s]Size(%dx%d), Padding(%dx%d), StartPt(%d, %d), Content size(%dx%d)", BUFFER_NAME,
                                size.w, size.h, padding.w, padding.h, startPt.x, startPt.y,
                                size.w-padding.w, size.h-padding.h);
        }

        return *this;
    }
};

const StereoArea STEREO_AREA_ZERO;

}  //namespace StereoHAL
#endif