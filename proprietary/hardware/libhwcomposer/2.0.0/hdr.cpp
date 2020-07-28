#define DEBUG_LOG_TAG "hdr"
#define STRINGIFY_ENUMS // for asString in HardwareAPI.h/VideoAPI.h

#include <cutils/properties.h>

#include <hardware/hwcomposer2.h>

#include "hdr.h"
#include "utils/debug.h"
#include "ui/gralloc_extra.h"
#include "media/hardware/VideoAPI.h"

ANDROID_SINGLETON_STATIC_INSTANCE(HwcHdrUtils);
typedef ColorAspects CA;

// ref HDRColorPrimariesMap
uint32_t mapHdrPrimaries (int32_t dataspace)
{
    switch (dataspace & HAL_DATASPACE_STANDARD_MASK)
    {
        case HAL_DATASPACE_STANDARD_BT709:
            return CA::PrimariesBT709_5;
        case HAL_DATASPACE_STANDARD_BT601_625:
        case HAL_DATASPACE_STANDARD_BT601_625_UNADJUSTED:
            return CA::PrimariesBT601_6_625;
        case HAL_DATASPACE_STANDARD_BT601_525:
        case HAL_DATASPACE_STANDARD_BT601_525_UNADJUSTED:
            return CA::PrimariesBT601_6_525;
        case HAL_DATASPACE_STANDARD_BT2020:
        case HAL_DATASPACE_STANDARD_BT2020_CONSTANT_LUMINANCE:
            return CA::PrimariesBT2020;
        case HAL_DATASPACE_STANDARD_BT470M:
            return CA::PrimariesBT470_6M;
        case HAL_DATASPACE_STANDARD_FILM:
            return CA::PrimariesGenericFilm;
        case 0:
            switch (dataspace & 0xffff) {
                case HAL_DATASPACE_JFIF:
                case HAL_DATASPACE_BT601_625:
                    return CA::PrimariesBT601_6_625;;
                case HAL_DATASPACE_BT601_525:
                    return CA::PrimariesBT601_6_525;

                case HAL_DATASPACE_SRGB_LINEAR:
                case HAL_DATASPACE_SRGB:
                case HAL_DATASPACE_BT709:
                    return CA::PrimariesBT709_5;;
            }
       default:
           return CA::PrimariesUnspecified;
    }
}

// ref HDRTransferMap
uint32_t mapHdrTransfer (int32_t dataspace)
{
    switch (dataspace & HAL_DATASPACE_TRANSFER_MASK)
    {
        case HAL_DATASPACE_TRANSFER_LINEAR:
            return CA::TransferLinear;
        case HAL_DATASPACE_TRANSFER_SRGB:
            return CA::TransferSRGB;
        case HAL_DATASPACE_TRANSFER_SMPTE_170M:
            return CA::TransferSMPTE170M;
        case HAL_DATASPACE_TRANSFER_GAMMA2_2:
            return CA::TransferGamma22;
        case HAL_DATASPACE_TRANSFER_GAMMA2_8:
            return CA::TransferGamma28;
        case HAL_DATASPACE_TRANSFER_ST2084:
            return CA::TransferST2084;
        case HAL_DATASPACE_TRANSFER_HLG:
            return CA::TransferHLG;
        case 0:
            switch (dataspace & 0xffff) {
                case HAL_DATASPACE_JFIF:
                case HAL_DATASPACE_BT601_625:
                case HAL_DATASPACE_BT601_525:
                    return CA::TransferSMPTE170M;
                case HAL_DATASPACE_SRGB_LINEAR:
                    return CA::TransferLinear;
                case HAL_DATASPACE_SRGB:
                    return CA::TransferSRGB;
                case HAL_DATASPACE_BT709:
                    return CA::TransferSMPTE170M;
            }
        default:
            return CA::TransferUnspecified;
    }
}

// ref HDRMatrixCoeffMap
uint32_t mapHdrMatrixCoeffs (int32_t dataspace)
{
    switch (dataspace & HAL_DATASPACE_STANDARD_MASK)
    {
        case HAL_DATASPACE_STANDARD_BT709:
            return CA::MatrixBT709_5;
        case HAL_DATASPACE_STANDARD_BT601_625:
            return CA::MatrixBT601_6;
        case HAL_DATASPACE_STANDARD_BT601_625_UNADJUSTED:
            return CA::MatrixBT709_5;
        case HAL_DATASPACE_STANDARD_BT601_525:
            return CA::MatrixBT601_6;
        case HAL_DATASPACE_STANDARD_BT601_525_UNADJUSTED:
            return CA::MatrixSMPTE240M;
        case HAL_DATASPACE_STANDARD_BT2020:
            return CA::MatrixBT2020;
        case HAL_DATASPACE_STANDARD_BT2020_CONSTANT_LUMINANCE:
            return CA::MatrixBT2020Constant;
        case HAL_DATASPACE_STANDARD_BT470M:
            return CA::MatrixBT470_6M;
        case HAL_DATASPACE_STANDARD_FILM:
            return CA::MatrixBT2020;
        case 0:
            switch (dataspace & 0xffff) {
                case HAL_DATASPACE_JFIF:
                case HAL_DATASPACE_BT601_625:
                case HAL_DATASPACE_BT601_525:
                    return CA::MatrixBT601_6;
                case HAL_DATASPACE_SRGB_LINEAR:
                case HAL_DATASPACE_SRGB:
                case HAL_DATASPACE_BT709:
                    return CA::MatrixBT709_5;
            }
        default:
            return CA::MatrixUnspecified;
    }
}

bool isGeHdrInfoEqual(ge_hdr_info_t src, ge_hdr_info_t dst)
{
    if (src.u4ColorPrimaries != dst.u4ColorPrimaries ||
        src.u4TransformCharacter != dst.u4TransformCharacter ||
        src.u4MatrixCoeffs != dst.u4MatrixCoeffs ||
        src.u4MaxDisplayMasteringLuminance != dst.u4MaxDisplayMasteringLuminance ||
        src.u4MinDisplayMasteringLuminance != dst.u4MinDisplayMasteringLuminance ||
        src.u4WhitePointX != dst.u4WhitePointX ||
        src.u4WhitePointY != dst.u4WhitePointY ||
        src.u4DisplayPrimariesX[0] != dst.u4DisplayPrimariesX[0] ||
        src.u4DisplayPrimariesX[1] != dst.u4DisplayPrimariesX[1] ||
        src.u4DisplayPrimariesX[2] != dst.u4DisplayPrimariesX[2] ||
        src.u4DisplayPrimariesY[0] != dst.u4DisplayPrimariesY[0] ||
        src.u4DisplayPrimariesY[1] != dst.u4DisplayPrimariesY[1] ||
        src.u4DisplayPrimariesY[2] != dst.u4DisplayPrimariesY[2] ||
        src.u4MaxContentLightLevel != dst.u4MaxContentLightLevel ||
        src.u4MaxPicAverageLightLevel != dst.u4MaxPicAverageLightLevel)
    {
        HWC_LOGV("GeHdr diff \n src %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
        src.u4ColorPrimaries, src.u4TransformCharacter, src.u4MatrixCoeffs,
        src.u4MaxDisplayMasteringLuminance, src.u4MinDisplayMasteringLuminance,
        src.u4WhitePointX, src.u4WhitePointY,
        src.u4DisplayPrimariesX[0], src.u4DisplayPrimariesX[1], src.u4DisplayPrimariesX[2],
        src.u4DisplayPrimariesY[0], src.u4DisplayPrimariesY[1], src.u4DisplayPrimariesY[2],
        src.u4MaxContentLightLevel, src.u4MaxPicAverageLightLevel);
        HWC_LOGV("dst %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
        dst.u4ColorPrimaries, dst.u4TransformCharacter, dst.u4MatrixCoeffs,
        dst.u4MaxDisplayMasteringLuminance, dst.u4MinDisplayMasteringLuminance,
        dst.u4WhitePointX, dst.u4WhitePointY,
        dst.u4DisplayPrimariesX[0], dst.u4DisplayPrimariesX[1], dst.u4DisplayPrimariesX[2],
        dst.u4DisplayPrimariesY[0], dst.u4DisplayPrimariesY[1], dst.u4DisplayPrimariesY[2],
        dst.u4MaxContentLightLevel, dst.u4MaxPicAverageLightLevel);
        return false;
    }
    return true;

}

void HwcHdrUtils::setHDRFeature(const bool& enable)
{
    m_enable = enable;

    if (m_enable)
    {
        m_hdr_capabilities.push_back(HAL_HDR_HDR10);
        m_hdr_capabilities.push_back(HAL_HDR_HLG);

        //smpte2086
        m_hdr_metadata_keys.push_back(HWC2_DISPLAY_RED_PRIMARY_X);
        m_hdr_metadata_keys.push_back(HWC2_DISPLAY_RED_PRIMARY_Y);
        m_hdr_metadata_keys.push_back(HWC2_DISPLAY_GREEN_PRIMARY_X);
        m_hdr_metadata_keys.push_back(HWC2_DISPLAY_GREEN_PRIMARY_Y);
        m_hdr_metadata_keys.push_back(HWC2_DISPLAY_BLUE_PRIMARY_X);
        m_hdr_metadata_keys.push_back(HWC2_DISPLAY_BLUE_PRIMARY_Y);
        m_hdr_metadata_keys.push_back(HWC2_WHITE_POINT_X);
        m_hdr_metadata_keys.push_back(HWC2_WHITE_POINT_Y);
        m_hdr_metadata_keys.push_back(HWC2_MAX_LUMINANCE);
        m_hdr_metadata_keys.push_back(HWC2_MIN_LUMINANCE);

        //cta861_3
        m_hdr_metadata_keys.push_back(HWC2_MAX_CONTENT_LIGHT_LEVEL);
        m_hdr_metadata_keys.push_back(HWC2_MAX_FRAME_AVERAGE_LIGHT_LEVEL);
        property_set("vendor.debug.sf.hdr_enable", "1");
    }
    else
    {
        m_hdr_capabilities.clear();
        m_hdr_metadata_keys.clear();
        property_set("vendor.debug.sf.hdr_enable", "0");
    }
}

bool HwcHdrUtils::shouldChangeMetadata(uint32_t numElements,
                                        const int32_t* keys, const float* metadata,
                                        const HwcHdrMetadata& hwcmetadata)
{
    if (!m_enable)
        return false;

    bool changed = false;
    uint32_t type = 0;
    for (uint32_t i = 0; i < numElements; ++i)
    {
        type |= 1 << keys[i];
        switch(keys[i])
        {
            case HWC2_DISPLAY_RED_PRIMARY_X:
                changed |= (hwcmetadata.smpte2086.displayPrimaryRed.x != metadata[i]);
                break;
            case HWC2_DISPLAY_RED_PRIMARY_Y:
                changed |= (hwcmetadata.smpte2086.displayPrimaryRed.y != metadata[i]);
                break;
            case HWC2_DISPLAY_GREEN_PRIMARY_X:
                changed |= (hwcmetadata.smpte2086.displayPrimaryGreen.x != metadata[i]);
                break;
            case HWC2_DISPLAY_GREEN_PRIMARY_Y:
                changed |= (hwcmetadata.smpte2086.displayPrimaryGreen.y != metadata[i]);
                break;
            case HWC2_DISPLAY_BLUE_PRIMARY_X:
                changed |= (hwcmetadata.smpte2086.displayPrimaryBlue.x != metadata[i]);
                break;
            case HWC2_DISPLAY_BLUE_PRIMARY_Y:
                changed |= (hwcmetadata.smpte2086.displayPrimaryBlue.y != metadata[i]);
                break;
            case HWC2_WHITE_POINT_X:
                changed |= (hwcmetadata.smpte2086.whitePoint.x != metadata[i]);
                break;
            case HWC2_WHITE_POINT_Y:
                changed |= (hwcmetadata.smpte2086.whitePoint.y != metadata[i]);
                break;
            case HWC2_MAX_LUMINANCE:
                changed |= (hwcmetadata.smpte2086.maxLuminance != metadata[i]);
                break;
            case HWC2_MIN_LUMINANCE:
                changed |= (hwcmetadata.smpte2086.minLuminance != metadata[i]);
                break;
            case HWC2_MAX_CONTENT_LIGHT_LEVEL:
                changed |= (hwcmetadata.cta8613.maxContentLightLevel != metadata[i]);
                break;
            case HWC2_MAX_FRAME_AVERAGE_LIGHT_LEVEL:
                changed |= (hwcmetadata.cta8613.maxFrameAverageLightLevel != metadata[i]);
                break;
        }
    }

    if ((type & SMPTE2086) == SMPTE2086 || (type & CTA861_3) == CTA861_3)
    {
        return changed;
    }
    else
    {
        HWC_LOGE(" Metadata is not sufficiant:%x in_num:%d", type, numElements);
        return false;
    }
}

void HwcHdrUtils::setMetadata(uint32_t numElements, const int32_t* keys, const float* metadata,
                              HwcHdrMetadata* hwcmetadata)
{
    if (!m_enable)
        return;

    uint32_t type = 0;
    for (uint32_t i = 0; i < numElements; ++i)
    {
        type |= 1 << keys[i];
        switch(keys[i])
        {
            case HWC2_DISPLAY_RED_PRIMARY_X:
                hwcmetadata->smpte2086.displayPrimaryRed.x = metadata[i];
                break;
            case HWC2_DISPLAY_RED_PRIMARY_Y:
                hwcmetadata->smpte2086.displayPrimaryRed.y = metadata[i];
                break;
            case HWC2_DISPLAY_GREEN_PRIMARY_X:
                hwcmetadata->smpte2086.displayPrimaryGreen.x = metadata[i];
                break;
            case HWC2_DISPLAY_GREEN_PRIMARY_Y:
                hwcmetadata->smpte2086.displayPrimaryGreen.y = metadata[i];
                break;
            case HWC2_DISPLAY_BLUE_PRIMARY_X:
                hwcmetadata->smpte2086.displayPrimaryBlue.x = metadata[i];
                break;
            case HWC2_DISPLAY_BLUE_PRIMARY_Y:
                hwcmetadata->smpte2086.displayPrimaryBlue.y = metadata[i];
                break;
            case HWC2_WHITE_POINT_X:
                hwcmetadata->smpte2086.whitePoint.x = metadata[i];
                break;
            case HWC2_WHITE_POINT_Y:
                hwcmetadata->smpte2086.whitePoint.y = metadata[i];
                break;
            case HWC2_MAX_LUMINANCE:
                hwcmetadata->smpte2086.maxLuminance = metadata[i];
                break;
            case HWC2_MIN_LUMINANCE:
                hwcmetadata->smpte2086.minLuminance = metadata[i];
                break;
            case HWC2_MAX_CONTENT_LIGHT_LEVEL:
                hwcmetadata->cta8613.maxContentLightLevel = metadata[i];
                break;
            case HWC2_MAX_FRAME_AVERAGE_LIGHT_LEVEL:
                hwcmetadata->cta8613.maxFrameAverageLightLevel = metadata[i];
                break;
        }
    }
    hwcmetadata->type = type;
}

void HwcHdrUtils::fillMetadatatoGrallocExtra(HwcHdrMetadata hwcmetadata,
                                                PrivateHandle hnd, int32_t dataspace)
{
    if (!m_enable)
        return;

    if (NULL == hnd.handle)
        return;

    if (((hwcmetadata.type & SMPTE2086) != SMPTE2086) || ((hwcmetadata.type & CTA861_3) != CTA861_3))
        return;
    HWC_LOGV("type:%d %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f !!!!!",
             hwcmetadata.type,
             hwcmetadata.smpte2086.maxLuminance,
             hwcmetadata.smpte2086.minLuminance,
             hwcmetadata.smpte2086.whitePoint.x,
             hwcmetadata.smpte2086.whitePoint.y,
             hwcmetadata.smpte2086.displayPrimaryGreen.x,
             hwcmetadata.smpte2086.displayPrimaryBlue.x,
             hwcmetadata.smpte2086.displayPrimaryRed.x,
             hwcmetadata.smpte2086.displayPrimaryGreen.y,
             hwcmetadata.smpte2086.displayPrimaryBlue.y,
             hwcmetadata.smpte2086.displayPrimaryRed.y,
             hwcmetadata.cta8613.maxContentLightLevel,
             hwcmetadata.cta8613.maxFrameAverageLightLevel);

    HWC_LOGV("%d %s %s %s", dataspace, asString(static_cast<CA::Primaries> (mapHdrPrimaries(dataspace))),
                        asString(static_cast<CA::Transfer> (mapHdrTransfer(dataspace))),
                        asString(static_cast<CA::MatrixCoeffs> (mapHdrMatrixCoeffs(dataspace))));

    const int&& type = (hnd.ext_info.status & GRALLOC_EXTRA_MASK_TYPE);
    ge_hdr_info_t hdr_from_metadata;

    hdr_from_metadata.u4ColorPrimaries = mapHdrPrimaries(dataspace);
    hdr_from_metadata.u4TransformCharacter = mapHdrTransfer(dataspace);
    hdr_from_metadata.u4MatrixCoeffs = mapHdrMatrixCoeffs(dataspace);
    hdr_from_metadata.u4MaxDisplayMasteringLuminance = static_cast<uint32_t> (hwcmetadata.smpte2086.maxLuminance);
    hdr_from_metadata.u4MinDisplayMasteringLuminance = static_cast<uint32_t> (hwcmetadata.smpte2086.minLuminance);
    hdr_from_metadata.u4WhitePointX = static_cast<uint32_t> (hwcmetadata.smpte2086.whitePoint.x);
    hdr_from_metadata.u4WhitePointY = static_cast<uint32_t> (hwcmetadata.smpte2086.whitePoint.y);
    hdr_from_metadata.u4DisplayPrimariesX[0] = static_cast<uint32_t> (hwcmetadata.smpte2086.displayPrimaryGreen.x);
    hdr_from_metadata.u4DisplayPrimariesX[1] = static_cast<uint32_t> (hwcmetadata.smpte2086.displayPrimaryBlue.x);
    hdr_from_metadata.u4DisplayPrimariesX[2] = static_cast<uint32_t> (hwcmetadata.smpte2086.displayPrimaryRed.x);
    hdr_from_metadata.u4DisplayPrimariesY[0] = static_cast<uint32_t> (hwcmetadata.smpte2086.displayPrimaryGreen.y);
    hdr_from_metadata.u4DisplayPrimariesY[1] = static_cast<uint32_t> (hwcmetadata.smpte2086.displayPrimaryBlue.y);
    hdr_from_metadata.u4DisplayPrimariesY[2] = static_cast<uint32_t> (hwcmetadata.smpte2086.displayPrimaryRed.y);
    hdr_from_metadata.u4MaxContentLightLevel = static_cast<uint32_t> (hwcmetadata.cta8613.maxContentLightLevel);
    hdr_from_metadata.u4MaxPicAverageLightLevel = static_cast<uint32_t> (hwcmetadata.cta8613.maxFrameAverageLightLevel);

    // check hwdecode
    ge_hdr_info_t hdr_from_handle;
    int32_t err = gralloc_extra_query(hnd.handle, GRALLOC_EXTRA_GET_HDR_INFO, &hdr_from_handle);
    if (err)
    {
        HWC_LOGE("%s err(%x), (handle=%p)", __func__, err, hnd.handle);
    }

    if (isGeHdrInfoEqual(hdr_from_handle, hdr_from_metadata))
        return;
    HWC_LOGV("perform metadata  ishwdec:%d", (type == GRALLOC_EXTRA_BIT_TYPE_VIDEO));
    gralloc_extra_perform(hnd.handle, GRALLOC_EXTRA_SET_HDR_INFO, &hdr_from_metadata);
}

