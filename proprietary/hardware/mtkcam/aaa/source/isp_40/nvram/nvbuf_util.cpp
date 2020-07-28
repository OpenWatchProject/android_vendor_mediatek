//#######################################################
// how to add code, modify code?
// for almost all cases, it is only change the number (sensor, nvram type) and related code.
// please modify the code before the multi line "//####" (about L124).
//#######################################################

#define LOG_TAG "nvbuf_util.cpp"

#ifdef WIN32
  #include "win32_test.h"
#else
  #include <utils/Errors.h>
  #include <dirent.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <mutex>
  #include <mtkcam/utils/std/Log.h>
  #include <nvram_drv.h>
  #include "nvram_drv_imp.h"
  #include <aaa_types.h>
  #include "flash_param.h"
  #include "flash_tuning_custom.h"
  #include "camera_custom_msdk.h"
  #include <private/aaa_hal_private.h>
  #include <private/aaa_utils.h>
  #include <array>

#include <mtkcam/drv/IHalSensor.h>
#endif
#include <nvbuf_util.h>
#include <cutils/properties.h>

using namespace NSCam;
using namespace android;

DEF_CriticalSection_Win(m_cs);

#ifdef WIN32
    #define logI(fmt, ...)
    #define logE(fmt, ...)
#else
    #define logW(fmt, arg...)    CAM_LOGW("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##arg)
    #define logI(fmt, arg...)    CAM_LOGD("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##arg)
    #define logE(fmt, arg...)    CAM_LOGE("[%s:%d] MError: " fmt, __FUNCTION__, __LINE__, ##arg)
#endif

//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
enum
{
    e_NvramTypeNum = 13,
    e_SensorTypeNum = 5,
};

static int g_nvramArrInd[e_NvramTypeNum]=
{
    (int)CAMERA_NVRAM_DATA_ISP,
    (int)CAMERA_NVRAM_DATA_3A,
    (int)CAMERA_NVRAM_DATA_SHADING,
    (int)CAMERA_NVRAM_DATA_LENS,
    (int)CAMERA_DATA_AE_PLINETABLE,
    (int)CAMERA_NVRAM_DATA_STROBE,
    (int)CAMERA_DATA_TSF_TABLE,
    (int)CAMERA_DATA_PDC_TABLE,
    (int)CAMERA_NVRAM_DATA_GEOMETRY,
    (int)CAMERA_NVRAM_DATA_FOV,
    (int)CAMERA_NVRAM_DATA_FEATURE,
    (int)CAMERA_NVRAM_DATA_AF,
    (int)CAMERA_NVRAM_DATA_FLASH_CALIBRATION
};

static int g_nvramSize[e_NvramTypeNum]=
{
    (int)sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
    (int)sizeof(NVRAM_CAMERA_3A_STRUCT),
    (int)sizeof(NVRAM_CAMERA_SHADING_STRUCT),
    (int)sizeof(NVRAM_LENS_PARA_STRUCT),
    (int)sizeof(AE_PLINETABLE_T),
    (int)sizeof(NVRAM_CAMERA_STROBE_STRUCT),
    (int)sizeof(CAMERA_TSF_TBL_STRUCT),
    (int)sizeof(CAMERA_BPCI_STRUCT),
    (int)sizeof(NVRAM_CAMERA_GEOMETRY_STRUCT),
    (int)sizeof(NVRAM_CAMERA_FOV_STRUCT),
    (int)sizeof(NVRAM_CAMERA_FEATURE_STRUCT),
    (int)sizeof(NVRAM_AF_PARA_STRUCT),
    (int)sizeof(NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT),
};

//for sensor driver
// static int g_sensorArrInd[e_SensorTypeNum]=
// {
//     (int)DUAL_CAMERA_MAIN_SENSOR,
//     (int)DUAL_CAMERA_SUB_SENSOR,
//     (int)DUAL_CAMERA_MAIN_2_SENSOR,
//     (int)DUAL_CAMERA_SUB_2_SENSOR
// };

// [ Debug ] for dump buf using
static void dump_nv_buf(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, void* buf, int nvSize, int mode);
static Mutex gdumpNvLock;
#define MAX_FILE_PATH_LEN 100
#define MAX_FILE_PRE_PATH_LEN 64
static char gCreateDir[MAX_FILE_PATH_LEN] = "mnt/vendor/nvcfg/camera/debug";
static char gFilePath[CAMERA_DATA_TYPE_NUM][MAX_FILE_PRE_PATH_LEN]=
{
    "mnt/vendor/nvcfg/camera/debug/nv_isp",
    "mnt/vendor/nvcfg/camera/debug/nv_3a",
    "mnt/vendor/nvcfg/camera/debug/nv_shading",
    "mnt/vendor/nvcfg/camera/debug/nv_lens",
    "mnt/vendor/nvcfg/camera/debug/nv_pline",
    "mnt/vendor/nvcfg/camera/debug/nv_strobe",
    "mnt/vendor/nvcfg/camera/debug/nv_tsf",
    "mnt/vendor/nvcfg/camera/debug/nv_pdc",
    "mnt/vendor/nvcfg/camera/debug/nv_geometry",
    "mnt/vendor/nvcfg/camera/debug/nv_fov",
    "mnt/vendor/nvcfg/camera/debug/nv_feature",
    "mnt/vendor/nvcfg/camera/debug/nv_af_calibration",
    "mnt/vendor/nvcfg/camera/debug/nv_flash_calibration",
    "mnt/vendor/nvcfg/camera/debug/nv_version",
};
//

static char gFacingName[2][MAX_FILE_PRE_PATH_LEN]=
{
    "_main",
    "_sub",
};
static char gFacingNum[3][MAX_FILE_PRE_PATH_LEN]=
{
    "",
    "_sec",
    "_third",
};

static void getNvFileName(int dev, int id, char* outName)
{
    auto facing = 0; auto num = 0;
    NS3Av3::mapSensorDevToFace(dev, facing, num);

    strncpy(outName, gFilePath[id], MAX_FILE_PRE_PATH_LEN);
    strncat(outName, gFacingName[facing], sizeof(gFacingName[facing]));

    char devSuffix[20] = {'\0'};
    if(num < 3) sprintf(devSuffix, "%s", gFacingNum[num]);
    else sprintf(devSuffix, "_%d", num);

    strncat(outName, devSuffix, sizeof(devSuffix));

    logI("outName(%s)", outName);
}
//

static int g_isNvBufRead[e_NvramTypeNum][SENSOR_IDX_MAX];
static int g_isVerNvBufRead;

//In order to prevent from twice allocation in singleton
static std::mutex g_singletonLock;
static std::mutex g_lock;

////////////////////////////////////////////////////
template <class T>
static T* getMemT(CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev)
{
    static std::array<NS3Av3::INST_T<T>, SENSOR_IDX_MAX> gMultiton;

    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(sensorDev));

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return NULL;
    }

    NS3Av3::INST_T<T>& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<T>();
    } );

    return rSingleton.instance.get();
}

int getVerMem(void*& buf)
{
    static NVRAM_CAMERA_VERSION_STRUCT obj;
    buf = (void*)&obj;
    return 0;
}

template <class T>
static int getMemDataType(CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, void*& m)
{
    m = getMemT<T>(sensorDev);
    if(!m)
    {
        return NvBufUtil::e_NV_SensorDevWrong;
    }
    return 0;
}

static int getMem(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, void*& buf)
{
    if(nvRamId==CAMERA_NVRAM_DATA_ISP)      return getMemDataType<NVRAM_CAMERA_ISP_PARAM_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_3A)      return getMemDataType<NVRAM_CAMERA_3A_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_SHADING)     return getMemDataType<NVRAM_CAMERA_SHADING_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_LENS)        return getMemDataType<NVRAM_LENS_PARA_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_DATA_AE_PLINETABLE)     return getMemDataType<AE_PLINETABLE_T>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_STROBE)      return getMemDataType<NVRAM_CAMERA_STROBE_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_DATA_TSF_TABLE)     return getMemDataType<CAMERA_TSF_TBL_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_DATA_PDC_TABLE)     return getMemDataType<CAMERA_BPCI_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_FOV)     return getMemDataType<NVRAM_CAMERA_FOV_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_FEATURE)     return getMemDataType<NVRAM_CAMERA_FEATURE_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_GEOMETRY)        return getMemDataType<NVRAM_CAMERA_GEOMETRY_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_AF)      return getMemDataType<NVRAM_AF_PARA_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_FLASH_CALIBRATION)       return getMemDataType<NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT>(sensorDev, buf);

    return NvBufUtil::e_NvramIdWrong;
}


static int readVerNvramNoLock(void*& p);
// int getSenorArrInd(CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev);
static int getBufAndReadNoLock(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p, int bForceRead);

static int readRamVersion(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, int& version)
{
    int err = 0;
    void* buf = nullptr;
    err = readVerNvramNoLock(buf);
    if(err!=0)
    {
    logI("readVerNvramNoLock error!, err(%d)", err);
        return err;
    }
    NVRAM_CAMERA_VERSION_STRUCT* verStruct = (NVRAM_CAMERA_VERSION_STRUCT*)buf;

    auto devArrInd = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(sensorDev));
    if(devArrInd<0)
        return NvBufUtil::e_SensorDevWrong;

    logW("nvRamId(%d), ver: isp(%d),3a(%d),sh(%d),lens(%d),pl(%d),stb(%d),tsf(%d),pdc(%d),geo(%d),fov(%d),feature(%d)",nvRamId,
        verStruct->ispVer[devArrInd], verStruct->aaaVer[devArrInd], verStruct->shadingVer[devArrInd],
        verStruct->lensVer[devArrInd], verStruct->aePlineVer[devArrInd], verStruct->strobeVer[devArrInd],
        verStruct->tsfVer[devArrInd], verStruct->pdcVer[devArrInd], verStruct->geometryVer[devArrInd],
        verStruct->fovVer[devArrInd], verStruct->featureVer[devArrInd]);

    if(nvRamId==CAMERA_NVRAM_DATA_ISP)          {version = verStruct->ispVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_3A)      {version = verStruct->aaaVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_SHADING) {version = verStruct->shadingVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_LENS)    {version = verStruct->lensVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_DATA_AE_PLINETABLE) {version = verStruct->aePlineVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_STROBE)  {version = verStruct->strobeVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_DATA_TSF_TABLE) {version = verStruct->tsfVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_DATA_PDC_TABLE) {version = verStruct->pdcVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_GEOMETRY) {version = verStruct->geometryVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FOV) {version = verStruct->fovVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FEATURE)   {version = verStruct->featureVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_AF) {version = verStruct->afVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FLASH_CALIBRATION)   {version = verStruct->flashCalibrationVer[devArrInd]; return 0;}

    return NvBufUtil::e_NvramIdWrong;
}

int writeVerNvramNoLock();
static int writeRamVersion(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, int version)
{
    logI("nvId(%d), ver(%d)", (int)nvRamId, version);
    int err = 0;
    void* buf = nullptr;
    err = getVerMem(buf);
    if(err!=0)
        return err;

    auto devArrInd = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(sensorDev));
    if(devArrInd<0)
        return NvBufUtil::e_SensorDevWrong;

    NVRAM_CAMERA_VERSION_STRUCT* verStruct = (NVRAM_CAMERA_VERSION_STRUCT*)buf;
    if(nvRamId==CAMERA_NVRAM_DATA_ISP)          {verStruct->ispVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_3A)      {verStruct->aaaVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_SHADING) {verStruct->shadingVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_LENS)    {verStruct->lensVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_DATA_AE_PLINETABLE) {verStruct->aePlineVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_STROBE)  {verStruct->strobeVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_DATA_TSF_TABLE) {verStruct->tsfVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_DATA_PDC_TABLE) {verStruct->pdcVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_GEOMETRY)   {verStruct->geometryVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FOV) {verStruct->fovVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FEATURE)   {verStruct->featureVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_AF)   {verStruct->afVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FLASH_CALIBRATION)   {verStruct->flashCalibrationVer[devArrInd]=version;}

    else return NvBufUtil::e_NvramIdWrong;
    err = writeVerNvramNoLock();
    return err;
}




static int writeRamUpdatedVersion(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev)
{
    int err = 0;
    void* buf = nullptr;
    err = getVerMem(buf);
    if(err!=0)
         return err;

    auto devArrInd = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(sensorDev));
    if(devArrInd<0)
        return NvBufUtil::e_SensorDevWrong;

    NVRAM_CAMERA_VERSION_STRUCT* verStruct = (NVRAM_CAMERA_VERSION_STRUCT*)buf;
    unsigned char* buf2;
    buf2  = (unsigned char*)buf;
    logI("qq %d %d %d %d %d %d %d %d %d %d buf(%p)",
        buf2[0], buf2[1], buf2[2], buf2[3], buf2[4],
        buf2[5], buf2[6], buf2[7], buf2[8], buf2[9], buf
    );
    if(nvRamId==CAMERA_NVRAM_DATA_ISP)            {verStruct->ispVer[devArrInd]=NVRAM_CAMERA_PARA_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_3A)        {verStruct->aaaVer[devArrInd]=NVRAM_CAMERA_3A_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_SHADING)    {verStruct->shadingVer[devArrInd]=NVRAM_CAMERA_SHADING_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_LENS)       {verStruct->lensVer[devArrInd]=NVRAM_CAMERA_LENS_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_STROBE)  {verStruct->strobeVer[devArrInd]=NVRAM_CAMERA_STROBE_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_GEOMETRY)   {verStruct->geometryVer[devArrInd]=NVRAM_CAMERA_GEOMETRY_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FEATURE)   {verStruct->featureVer[devArrInd]=NVRAM_CAMERA_FEATURE_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FOV)   {verStruct->fovVer[devArrInd]=NVRAM_CAMERA_FOV_FILE_VERSION;}
    else if(nvRamId==CAMERA_DATA_AE_PLINETABLE)  {verStruct->aePlineVer[devArrInd]=NVRAM_CAMERA_PLINE_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_AF)   {verStruct->afVer[devArrInd]=NVRAM_CAMERA_AF_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FLASH_CALIBRATION)   {verStruct->flashCalibrationVer[devArrInd]=NVRAM_CAMERA_FLASH_CALIBRATION_FILE_VERSION;}

    else return NvBufUtil::e_NvramIdWrong;

    logI("qq %d %d %d %d %d %d %d %d %d %d buf2(%p)",
        buf2[0], buf2[1], buf2[2], buf2[3], buf2[4],
        buf2[5], buf2[6], buf2[7], buf2[8], buf2[9], buf);

    err = writeVerNvramNoLock();

    logI("- nvRamId(%d)", nvRamId);
    return err;
}

//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################

// int getSenorArrInd(CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev)
// {
//     int sensorArrSz;
//     sensorArrSz = sizeof(g_sensorArrInd)/sizeof(int);
//     int arrInd=-1;
//     int i;
//     for(i=0;i<sensorArrSz;i++)
//     {
//         if(sensorDev==g_sensorArrInd[i])
//             arrInd=i;
//     }
//     return arrInd;
// }
int getNvramArrInd(CAMERA_DATA_TYPE_ENUM nvEnum)
{
    int nvArrSz;
    nvArrSz = sizeof(g_nvramArrInd)/sizeof(int);
    int arrInd=-1;
    int i;
    for(i=0;i<nvArrSz;i++)
    {
        if(nvEnum==g_nvramArrInd[i])
            arrInd=i;
    }
    return arrInd;
}
NvBufUtil::NvBufUtil()
{
    logI("not support");
}

void NvBufUtil::setAndroidMode(int isAndroid)
{
    logI("not support, isAndroid(%d)", isAndroid);
}

NvBufUtil& NvBufUtil::getInstance()
{
    std::lock_guard<std::mutex> _lock(g_lock);
    static int bInit=0;
    static NvBufUtil obj;

    logW("getInstance, bInit(%d)", bInit);
    if(bInit==0)
    {
        DEF_InitCs(m_cs);
        bInit=1;
        int nvRamArrSz;
        nvRamArrSz = sizeof(g_nvramArrInd)/sizeof(int);

        MBOOL shouldDump =  property_get_int32("vendor.debug.nvram.dump.en", 0);
        MINT32 sensorDump = property_get_int32("vendor.nvram.dump.sensor", -1);
        MINT32 typeDump =   property_get_int32("vendor.nvram.dump.type", -1);

        g_isVerNvBufRead=0;
        int i = 0, j = 0;
        void* p = nullptr;
        for(i=0;i<nvRamArrSz;i++) {
            for(j=0;j<SENSOR_IDX_MAX;j++) {
                g_isNvBufRead[i][j]=0;

                if(shouldDump)
                {
                    if(sensorDump == j && (typeDump == -1 || typeDump == i) && i != 7) //skip CAMERA_DATA_PDC_TABLE
                    {
                        MINT32 sensorDev = NS3Av3::mapSensorIdxToDev(static_cast<MINT32>(j));
                        obj.getBufAndRead(static_cast<CAMERA_DATA_TYPE_ENUM>(g_nvramArrInd[i]), sensorDev, p, MFALSE);
                        obj.write(static_cast<CAMERA_DATA_TYPE_ENUM>(g_nvramArrInd[i]), sensorDev);
                        logI("read and dump nvram: NVRAM struct id(%d), sensorInd(%d), sensorDev(%d), i(%d)", g_nvramArrInd[i], j, sensorDev, i);
                        g_isNvBufRead[i][j]=1;
                    }
                }
            }
        }
    }

    return obj;
}

extern "C" INvBufUtil* getInstance_NvBufUtil()
{
    return &NvBufUtil::getInstance();
}

static int getSensorID(CAMERA_DUAL_CAMERA_SENSOR_ENUM i4SensorDev, int& sensorId)
{
    sensorId = NS3Av3::mapSensorDevToSensorId(static_cast<MINT32>(i4SensorDev));

#ifdef WIN32
    sensorId=100;
#endif
    if(sensorId==0)
    {
        logE("sensorId(%d)",0);
        return NvBufUtil::e_NV_SensorIdNull;
    }
    return 0;
}


int nvbufUtil_getSensorId(int sensorDev, int& sensorId)
{
    return getSensorID((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, sensorId);
}


static int getNvSize(CAMERA_DATA_TYPE_ENUM camDataType, int& sz)
{
    int arrInd;
    arrInd = getNvramArrInd(camDataType);
    if(arrInd<0)
        return NvBufUtil::e_NvramIdWrong;
    sz = g_nvramSize[arrInd];
    return 0;
}

static int isBufRead(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, int& isRead)
{
    int nvArrInd;
    nvArrInd = getNvramArrInd(nvRamId);
    if(nvArrInd<0)
        return NvBufUtil::e_NvramIdWrong;

        auto sensorArrInd = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(sensorDev));
    if(sensorArrInd<0)
        return NvBufUtil::e_SensorDevWrong;

    isRead = g_isNvBufRead[nvArrInd][sensorArrInd];
    g_isNvBufRead[nvArrInd][sensorArrInd]=1;

    return 0;
}

static int getSensorIdandMem(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, int& seonsorId, void*& buf, int& nvSize)
{
    int err;
    err = getSensorID((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, seonsorId);
    if(err!=0)
        return err;
    err = getMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, buf);
    if(err!=0)
        return err;
    err = getNvSize(nvRamId, nvSize);
    if(err!=0)
        return err;
    return 0;
}

static int getMemWithSize(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, void*& buf, int& nvSize)
{
    int err;
    err = getMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, buf);
    if(err!=0)
        return err;
    err = getNvSize(nvRamId, nvSize);
    if(err!=0)
        return err;
    return 0;
}




int readDefaultNoLock(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev)
{
    int err = 0;
    int u4SensorID = 0;
    void* buf = nullptr;
    int nvSize = 0;
    err = getSensorIdandMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, buf, nvSize);

    logW("sensorDev(%d), u4SensorID(%d), buf(%p), nvSize(%d)", nvRamId, sensorDev, buf, nvSize);

    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    err = nvDrv->readDefaultData (
             (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, nvRamId,
             buf
           );
    nvDrv->destroyInstance();
    return err;
}

//==============================================================
// for camera version use
int readVerNvramNoLock(void*& p)
{
    int err=0;
    err = getVerMem(p);
    if(err!=0)
        return err;
    if(g_isVerNvBufRead==1)
    {
        return 0;
    }
    int nvSize;
    nvSize = (int)sizeof(NVRAM_CAMERA_VERSION_STRUCT);
    //----------------------------
    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    if(nvDrv->isNvramExist(DUAL_CAMERA_MAIN_SENSOR, CAMERA_NVRAM_VERSION))
    {
    err  = nvDrv->readNoDefault(
            (CAMERA_DUAL_CAMERA_SENSOR_ENUM)DUAL_CAMERA_MAIN_SENSOR, CAMERA_NVRAM_VERSION, p, nvSize);
    }
    nvDrv->destroyInstance();
    g_isVerNvBufRead=1;
    return err;
}
int writeVerNvramNoLock()
{
    int err;
    void* buf;
    err = getVerMem(buf);
    if(err!=0)
        return err;
    int nvSize;
    nvSize = (int)sizeof(NVRAM_CAMERA_VERSION_STRUCT);
    //----------------------------
    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    err  = nvDrv->writeNvram(
            (CAMERA_DUAL_CAMERA_SENSOR_ENUM)DUAL_CAMERA_MAIN_SENSOR, 0, CAMERA_NVRAM_VERSION,
            buf, nvSize );
    nvDrv->destroyInstance();
    return err;
}
//==============================================================

int writeNvramNoLock(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev)
{
    int err = 0;
    void* buf = nullptr;
    int u4SensorID = 0;
    int nvSize = 0;
    err = getSensorIdandMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, buf,  nvSize);
    if(err!=0)
        return err;

    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    err  = nvDrv->writeNvram(
            (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, nvRamId,
            buf, nvSize );
    nvDrv->destroyInstance();
    return err;
}
//===========================================================================
//===========================================================================
//===========================================================================

int NvBufUtil::getSensorIdAndModuleId(MINT32 sensorType ,MINT32 sensorIdx, MINT32 &sensorId, MUINT32 &moduleId)
{
    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();

    getSensorID((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorType, sensorId);

    //moduleId=nvDrv->readModuleIdFromEEPROM((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorType, MTRUE);
    nvDrv->destroyInstance();

    return 0;
}

int NvBufUtil::getBuf(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p)
{
    logW("+ nvRamId(%d), sensorDev(%d), p(%p)", nvRamId, sensorDev, p);
    DEF_AutoLock(m_cs);
    int err;
    err = getMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, p);
    return err;
}

int NvBufUtil::getBufAndReadNoDefault(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p, int bForceRead)
{
    logW("ramId(%d) dev(%d), p(%p), bForceRead(%d) +",(int)nvRamId, sensorDev, p, bForceRead);
    DEF_AutoLock(m_cs);
    int err=0;
    int bRead;
    err = getMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, p);
    if(err!=0)
    {
        logE("getMem error!");
        return err;
    }

    err = isBufRead(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, bRead);
    if(err!=0)
    {
        logE("isBufRead error!");
        return err;
    }
    if(bForceRead==1)
        bRead=0;

    logW("bRead(%d)", bRead);
    if(bRead==1)
    {
        return 0;
    }

    int nvSize;
    err = getNvSize(nvRamId, nvSize);
    if(err!=0)
    {
        logE("getNvSize error!");
        return err;
    }

    logW("nvSize(%d), err(%d), p(%p)", nvSize, err, p);

    //----------------------------
    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    err  = nvDrv->readNoDefault(
            (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, nvRamId, p, nvSize);
    nvDrv->destroyInstance();

    logI("-");
    return err;
}
int NvBufUtil::getBufAndRead(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p, int bForceRead)
{
    DEF_AutoLock(m_cs);
    return getBufAndReadNoLock(nvRamId, sensorDev, p, bForceRead);
}

static int getBufAndReadNoLock(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p, int bForceRead)
{
    logW("ramId(%d), sensorDev(%d), p(%p), bForceRead(%d) +",(int)nvRamId, sensorDev, p, bForceRead);
    int err = 0;
    int bRead = 0;
    err = isBufRead(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, bRead);
    if(err!=0)
        return err;
    if(bForceRead==1)
        bRead=0;

    if(bRead==1)
    {
        err = getMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, p);
        logI("getMem , p(%p), err(%d)", p, err);
        if(err!=0)
            return err;
        return 0;
    }

    int ramVer = 0;
    err = readRamVersion(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, ramVer);
    logI("readRamVersion, ver(%d), err(%d)", ramVer, err);

    if(err!=0)
        return err;

    int u4SensorID = 0;
    int nvSize = 0;
    err = getSensorIdandMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, p,  nvSize);
    logI("getSensorIdandMem, p(%p), nvSize(%d), err(%d)", p, nvSize, err);
    if(err!=0)
        return err;

    //----------------------------
    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    err  = nvDrv->readNvrameEx (
            (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, nvRamId,
            p, nvSize, ramVer);
    nvDrv->destroyInstance();
    dump_nv_buf(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, p, nvSize, 1);

    //-----------------------------
    logI("-");
    return err;
}
int NvBufUtil::write(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev)
{
    logW("nvRamId(%d), sensorDev(%d)", (int)nvRamId, sensorDev);
    DEF_AutoLock(m_cs);
    int err = 0;
    int err2 = 0;
    void* buf = nullptr;
    int nvSize = 0;

    err = getMemWithSize(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, buf,  nvSize);
    dump_nv_buf(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, buf, nvSize, 2);

    logI("buf(%p), nvSize(%d), err(%d)", buf, nvSize, err);
    if(err!=0)
        return err;

    int ver;
    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    err  = nvDrv->writeNvram(
            (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, 0, nvRamId,  //sensor id is not used
            buf, nvSize );
    nvDrv->destroyInstance();
    logI("writeNvram, err(%d)", err);
    if(err!=0)
        return err;

    err = writeRamUpdatedVersion(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev);
    logI("writeRamUpdatedVersion, err(%d)", err);
    if(err!=0)
        return err;
    int ramVer;
    err = readRamVersion(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, ramVer);
    logI("readRamVersion, ver(%d), err(%d)", ramVer, err);
    if(err!=0)
        return err;

    void* p=NULL;
    err = getBufAndReadNoLock(nvRamId, sensorDev, p, 1);
    if(err!=0)
        return err;

    return err;
}
int NvBufUtil::readDefault(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev)
{
    logW("nvRamId(%d), sensorDev(%d) +", nvRamId, sensorDev);
    DEF_AutoLock(m_cs);
    return readDefaultNoLock(nvRamId, sensorDev);
}

int NvBufUtil::readDefault(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void* p)
{
    logW("nvRamId(%d), sensorDev(%d), p(%p)", nvRamId, sensorDev, p);
    DEF_AutoLock(m_cs);
    int err = 0;
    int u4SensorID = 0;
    void* buf = nullptr;
    int nvSize = 0;
    err = getSensorIdandMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, buf, nvSize);
    logW("u4SensorID(%d), buf(%p), nvSize(%d)", u4SensorID, buf, nvSize);

    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    err  = nvDrv->readDefaultData (
        (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, nvRamId,
        p);
    nvDrv->destroyInstance();
    return err;
}

static void dump_nv_buf(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, void* buf, int nvSize, int mode)
{
    DIR* dir = opendir(gCreateDir);
    if(!dir)
    {
        return;
    }
    logI("sensorDev(%d), nvRamId(%d), buf(%p), nvSize(%d)", sensorDev, nvRamId, buf, nvSize);
    Mutex::Autolock lock(gdumpNvLock);
    int mask = umask(0);
    logI("umask(%d)", mask);
    char fname[MAX_FILE_PATH_LEN];
    getNvFileName(sensorDev, nvRamId, fname);

    if(mode == 1) strncat(fname, "_read", 5);
    else if(mode == 2) strncat(fname, "_write", 6);
    else
    {
        logE("mode exception");
        umask(mask);
        closedir(dir);
        return;
    }

    FILE* fp;
    fp = fopen(fname, "wb");
    logI("fname(%s)", fname);
    if(fp==0)
    {
        logE("fopen fail");
        umask(mask);
        closedir(dir);
        return;
    }

    fwrite(buf, 1, nvSize, fp);
    fclose(fp);
    closedir(dir);
    umask(mask);
    return;
}
