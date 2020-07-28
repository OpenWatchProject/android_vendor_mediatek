#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <string.h>

namespace NSCam {
namespace TuningUtils {

FILE_DUMP_NAMING_HINT::FILE_DUMP_NAMING_HINT()
{
    ImgWidth = 0;
    ImgHeight = 0;
    BufWidth = 0;
    BufHeight = 0;
    BufStride = 0;
    ImgFormat = -1;
    IspProfile = -1;
    SensorFormatOrder = -1;
    SensorType = -1;
    SensorOpenId = -1;
    SensorDev = -1;
    UniqueKey = -1;
    FrameNo = -1;
    RequestNo = -1;
    EvValue = -1;
    memset(additStr, '\0', 32);
}

static void calBufDimen(const IImageBuffer *buffer, int &outW, int &outH)
{
    MUINT32 strideB, sizeB, pbpp, ibpp;
    MINT format = buffer->getImgFormat();
    strideB = buffer->getBufStridesInBytes(0);
    sizeB = buffer->getBufSizeInBytes(0);
    pbpp = buffer->getPlaneBitsPerPixel(0);
    ibpp = buffer->getImgBitsPerPixel();
    pbpp = pbpp ? pbpp : 8;
    ibpp = ibpp ? ibpp : 8;
    outW = strideB * 8 / pbpp;
    outW = outW ? outW : 1;
    outH = buffer->getBufScanlines(0);
#if 0
    outH = sizeB / outW;
    if( buffer->getPlaneCount() == 1 )
    {
        outH = outH * 8 / ibpp;
    }
#endif
}

bool extract(FILE_DUMP_NAMING_HINT *pHint, const IMetadata *pMetadata)
{
    bool ret = true;
    int t;

    if (pMetadata == NULL) {
        return false;
    }

    ret &= IMetadata::getEntry<int>(pMetadata, MTK_PIPELINE_UNIQUE_KEY, pHint->UniqueKey);
    ret &= IMetadata::getEntry<int>(pMetadata, MTK_PIPELINE_FRAME_NUMBER, pHint->FrameNo);
    ret &= IMetadata::getEntry<int>(pMetadata, MTK_PIPELINE_REQUEST_NUMBER, pHint->RequestNo);
    ret &= IMetadata::getEntry<int>(pMetadata, MTK_PIPELINE_EV_VALUE, pHint->EvValue);

    return true;
}

bool extract(FILE_DUMP_NAMING_HINT *pHint, const IImageBuffer *pImgBuf)
{
    pHint->ImgWidth = pImgBuf->getImgSize().w;
    pHint->ImgHeight = pImgBuf->getImgSize().h;
    pHint->ImgFormat = pImgBuf->getImgFormat();
    calBufDimen(pImgBuf, pHint->BufWidth, pHint->BufHeight);
    pHint->BufStride = pImgBuf->getBufStridesInBytes(0);
    return true;
}

bool extract_by_SensorDev(FILE_DUMP_NAMING_HINT *pHint, int sensorDev)
{
    MUINT sensorFormatOrder = SENSOR_FORMAT_ORDER_NONE;
    pHint->SensorDev = sensorDev;
    IHalSensorList *const pIHalSensorList = MAKE_HalSensorList();
    if (pIHalSensorList) {
        NSCam::SensorStaticInfo sensorStaticInfo;
        memset(&sensorStaticInfo, 0, sizeof(NSCam::SensorStaticInfo));
        pIHalSensorList->querySensorStaticInfo((MUINT)sensorDev, &sensorStaticInfo);
        pHint->SensorFormatOrder = sensorStaticInfo.sensorFormatOrder;
        return true;
    } else {
        pHint->SensorFormatOrder = -1;
    }
    return false;
}

bool extract_by_SensorOpenId(FILE_DUMP_NAMING_HINT *pHint, int openId)
{
    pHint->SensorOpenId = openId;
    MUINT sensorDev = 0;
    IHalSensorList *const pIHalSensorList = MAKE_HalSensorList();
    if (pIHalSensorList) {
        sensorDev = (MUINT32)pIHalSensorList->querySensorDevIdx(openId);
        pHint->SensorDev = sensorDev;
        NSCam::SensorStaticInfo sensorStaticInfo;
        memset(&sensorStaticInfo, 0, sizeof(NSCam::SensorStaticInfo));
        pIHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);
        pHint->SensorFormatOrder = sensorStaticInfo.sensorFormatOrder;
        return true;
    } else {
        pHint->SensorFormatOrder = -1;
    }
    return false;
}

} //namespace FileDump
} //namespace NSCam

