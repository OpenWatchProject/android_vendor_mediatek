#ifndef __FILE_DUMP_NAMING_RULE_H__
#define __FILE_DUMP_NAMING_RULE_H__

#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/metadata/IMetadata.h>

namespace NSCam {
namespace TuningUtils {

enum RAW_PORT
{
    RAW_PORT_IMGO,
    RAW_PORT_RRZO,
    RAW_PORT_MFBO,
    RAW_PORT_UNDEFINED = -1,
    // do NOT show port name
    RAW_PORT_NULL = -2,
};

enum YUV_PORT
{
    YUV_PORT_IMG2O,
    YUV_PORT_IMG3O,
    YUV_PORT_WROTO,
    YUV_PORT_WDMAO,
    YUV_PORT_TIMGO,
    YUV_PORT_DP0,
    YUV_PORT_DP1,
    YUV_PORT_CRZO1,
    YUV_PORT_VNRO,
    YUV_PORT_IMGI,
    YUV_PORT_UNDEFINED = -1,
    // do NOT show port name
    YUV_PORT_NULL = -2,
};

enum DUMP_FEATURE
{
    DUMP_FEATURE_OFF,
    DUMP_FEATURE_STREAMING,
    DUMP_FEATURE_CAPTURE,
};

struct FILE_DUMP_NAMING_HINT
{
    FILE_DUMP_NAMING_HINT();

    // from IMetadata
    int UniqueKey;
    int FrameNo;
    int RequestNo;

    int ImgWidth;
    int ImgHeight;
    int BufWidth;
    int BufHeight;
    int BufStride;
    // image buffer size

    int ImgFormat;
    // mtkcam/def/ImageFormat.h (EImageFormat)

    // set directly
    int IspProfile;

    // Sensor
    int SensorFormatOrder;
    // mtkcam/drv/IHalSensor.h
    // - SENSOR_FORMAT_ORDER_RAW_B = 0x0,
    // - SENSOR_FORMAT_ORDER_RAW_Gb,
    // - SENSOR_FORMAT_ORDER_RAW_Gr,
    // - SENSOR_FORMAT_ORDER_RAW_R,


    int SensorType;
    int SensorOpenId;

    int SensorDev;
    // mtkcam/drv/IHalSensor.h
    // - SENSOR_DEV_MAIN
    // - SENSOR_DEV_SUB
    // - SENSOR_DEV_MAIN_2
    // - SENSOR_DEV_SUB_2

    int EvValue;

    //additional string for all getFilename
    char additStr[32];

};

bool extract(FILE_DUMP_NAMING_HINT *pHint, const IMetadata *pMetadata);
bool extract(FILE_DUMP_NAMING_HINT *pHint, const IImageBuffer *pImgBuf);
bool extract_by_SensorOpenId(FILE_DUMP_NAMING_HINT *pHint, int openId);
bool extract_by_SensorDev(FILE_DUMP_NAMING_HINT *pHint, int sensorDev);


// P2-like node dump
void genFileName_RAW(
    char *pFilename,
    int nFilename,
    const FILE_DUMP_NAMING_HINT *pHint,
    RAW_PORT port,
    const char *pUserString = NULL);

void genFileName_LCSO(
    char *pFilename,
    int nFilename,
    const FILE_DUMP_NAMING_HINT *pHint,
    const char *pUserString = NULL);

void genFileName_YUV(
    char *pFilename,
    int nFilename,
    const FILE_DUMP_NAMING_HINT *pHint,
    YUV_PORT port,
    const char *pUserString = NULL);

// 3A dump
void genFileName_HW_AAO(
    char *pFilename,
    int nFilename,
    const FILE_DUMP_NAMING_HINT *pHint,
    const char *pUserString = NULL);

void genFileName_TUNING(
    char *pFilename,
    int nFilename,
    const FILE_DUMP_NAMING_HINT *pHint,
    const char *pUserString = NULL);

void genFileName_Reg(
     char *pFilename,
     int nFilename,
     const FILE_DUMP_NAMING_HINT *pHint,
     const char *pUserString = NULL);

void genFileName_LSC(
    char *pFilename,
    int nFilename,
    const FILE_DUMP_NAMING_HINT *pHint,
    const char *pUserString = NULL);

void genFileName_LSC2(
    char *pFilename,
    int nFilename,
    const FILE_DUMP_NAMING_HINT *pHint,
    const char *pUserString = NULL);

void genFileName_TSF(
    char *pFilename,
    int nFilename,
    const FILE_DUMP_NAMING_HINT *pHint,
    const char *pUserString = NULL);

void genFileName_face_info(
    char *pFilename,
    int nFilename,
    const FILE_DUMP_NAMING_HINT *pHint,
    const char *pUserString = NULL);

// Jpeg dump
void genFileName_JPG(
    char *pFilename,
    int nFilename,
    const FILE_DUMP_NAMING_HINT *pHint,
    const char *pUserString = NULL);

void genFileName_VSDOF_BUFFER(
    char *pFilename,
    int nFilename,
    const FILE_DUMP_NAMING_HINT *pHint,
    const char *pUserString);

 void genFileName_YNR_FD_TBL(
     char *pFilename,
     int nFilename,
     const FILE_DUMP_NAMING_HINT *pHint,
     const char *pUserString = NULL);

 void genFileName_YNR_INT_TBL(
     char *pFilename,
     int nFilename,
     const FILE_DUMP_NAMING_HINT *pHint,
     const char *pUserString = NULL);

  void genFileName_YNR_ALPHA_MAP(
      char *pFilename,
      int nFilename,
      const FILE_DUMP_NAMING_HINT *pHint,
      const char *pUserString = NULL);

  void genFileName_DCESO(
     char *pFilename,
     int nFilename,
     const FILE_DUMP_NAMING_HINT *pHint,
     const char *pUserString = NULL);

  void genFileName_LTMSO(
     char *pFilename,
     int nFilename,
     const FILE_DUMP_NAMING_HINT *pHint,
     const char *pUserString = NULL);

   void genFileName_LTM_CURVE(
      char *pFilename,
      int nFilename,
      const FILE_DUMP_NAMING_HINT *pHint,
      const char *pUserString = NULL);

   void genFileName_BPC_TBL(
      char *pFilename,
      int nFilename,
      const FILE_DUMP_NAMING_HINT *pHint,
      const char *pUserString = NULL);

   void genFileName_LCESHO(
      char *pFilename,
      int nFilename,
      const FILE_DUMP_NAMING_HINT *pHint,
      const char *pUserString = NULL);

} //namespace FileDump
} //namespace NSCam

#endif // #ifndef __FILE_DUMP_NAMING_RULE_H__

