/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_DEF_IMAGEBUFFERINFO_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_DEF_IMAGEBUFFERINFO_H_

#include <type_traits>
#include <vector>
#include "ImageFormat.h"
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 *  BufPlane
 ******************************************************************************/

struct BufPlane
{

    /**
     * The size for this color plane, in bytes.
     */
    size_t sizeInBytes;

    /**
     * The row stride for this color plane, in bytes.
     *
     * This is the distance between the start of two consecutive rows of
     * pixels in the image. The row stride is always greater than 0.
     */
    size_t rowStrideInBytes;

};
static_assert(std::is_standard_layout_v<BufPlane> && std::is_trivial_v<BufPlane>);


/**
 * The planes of per-buffer.
 *
 * For example, 3 planes for YV12 and 2 planes for NV21.
 */
struct BufPlanes
{
    /**
     * The buffer planes.
     */
    BufPlane    planes[3];

    /**
     * The buffer plane count.
     *
     * This field indicates the number of valid planes.
     * The number of planes is determined by the format of the Image.
     */
    size_t      count;
};
static_assert(std::is_standard_layout_v<BufPlanes> && std::is_trivial_v<BufPlanes>);


/**
 * ImageBufferInfo
 */
struct ImageBufferInfo
{
    /**********************************************************************
     * Buffer information
     */

    /**
     * The planes of per-buffer.
     */
    BufPlanes   bufPlanes;

    /**
     * The number of buffers.
     */
    size_t      count;

    /**
     * The buffer step, in bytes.
     *
     * This field is the distance in bytes from one buffer to the next.
     *
     * If 'count' > 1, this field must be greater than or equal to
     * the sum of size of the buffer planes.
     * Otherwise, this field is ignored.
     */
    size_t      bufStep;

    /**
     * The start offset, in bytes, of the first buffer.
     */
    size_t      startOffset;


    /**********************************************************************
     * Image information
     */

    /**
     * The image format (eImgFmt_XXX).
     */
    int         imgFormat;

    /**
     * The image size, in pixels.
     */
    int         imgWidth;
    int         imgHeight;

};
static_assert(std::is_standard_layout_v<ImageBufferInfo> && std::is_trivial_v<ImageBufferInfo>);


/******************************************************************************
 * Buffer access permission
 ******************************************************************************/
enum class SecType : uint32_t
{
    /**********************************************************************
    * Bus fabric conforms to ARM TrustZone and data protection to avoid
    * a slave can be accessed by one or more unexpected masters.
    *
    * ARM creates multiple exception state, each of which owns
    * a specific Exception Level (EL), from 1 to 3, respectively.
    * The increased exception level indicates a higher level of execution privilege.
    *
    * Data protection is enforced by a MPU (Memory Protection Unit) and
    * a set of access permission between each master (domain)
    * and memory space (region). MPU ensures that only the domain with the right
    * access permission can read and/or write a corresponding region.
    *
    * Based on the aforementioned security fundamentals, we define buffer
    * access permission and memory type thereof.
    */

    /**
    * Readable/Writable from EL0, EL1, EL2, Secure-EL0, Secure-EL1, EL3
    */
    mem_normal = 0,

    /**
    * Readable/Writable from EL2, Secure-EL0, Secure-EL1, EL3
    * Reference:
    * vendor/mediatek/proprietary/hardware/gralloc_extra/include/gralloc1_mtk_defs.h
    * GRALLOC1_USAGE_PROT, GRALLOC1_USAGE_PROT_*
    */
    mem_protected,

    /**
    * Readable/Writable from Secure-EL0, Secure-EL1, EL3
    * Reference:
    * vendor/mediatek/proprietary/hardware/gralloc_extra/include/gralloc1_mtk_defs.h
    * GRALLOC1_USAGE_SECURE_CAMERA
    */
    mem_secure
};

struct SecureInfo
{
    SecType type;
};

/******************************************************************************
 * Raw Buffer Info
 ******************************************************************************/
struct RawBufferInfo
{
  int packedRaw;
  int processed;
  int bpp;           // bit per pixel
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_DEF_IMAGEBUFFERINFO_H_

