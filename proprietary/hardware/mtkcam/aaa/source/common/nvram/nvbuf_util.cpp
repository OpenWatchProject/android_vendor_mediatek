//#######################################################
// how to add code, modify code?
// for almost all cases, it is only change the number (sensor, nvram type) and related code.
// please modify the code before the multi line "//####" (about L124).
//#######################################################

#define LOG_TAG "nvbuf_util"

#ifdef WIN32
  #include "win32_test.h"
#else
  #include <utils/Errors.h>
  #include <dirent.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <mtkcam/utils/std/Log.h>
  #include <nvram_drv.h>
  #include "nvram_drv_imp.h"
  #include <aaa_types.h>
  #include "flash_param.h"
  #include "flash_tuning_custom.h"
  #include "camera_custom_msdk.h"

#include <mtkcam/drv/IHalSensor.h>
#endif
#include <nvbuf_util.h>

#include "nvbuf_util_dep.h"
#include <cutils/properties.h>

#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>

using namespace NSCam;

DEF_CriticalSection_Win(m_cs);

#ifdef WIN32
    #define logI(fmt, ...)
    #define logE(fmt, ...)
#else
    #define logI(fmt, arg...)    CAM_LOGD("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##arg)
    #define logE(fmt, arg...)    CAM_LOGE("[%s:%d] MError: " fmt, __FUNCTION__, __LINE__, ##arg)
#endif

//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################




// [ Debug ] for dump buf using
static void dump_nv_buf(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, void* buf, int nvSize, int mode);
static std::mutex gdumpNvLock;
static std::mutex g_lock;
//

extern void getNvFileNameU(int dev, int id, char* outName);
//

static int g_isNvBufRead[e_NvramTypeNum][SENSOR_IDX_MAX];
int g_isVerNvBufRead;

////////////////////////////////////////////////////
int getVerMem(void*& buf)
{
    static NVRAM_CAMERA_VERSION_STRUCT obj;
    buf = (void*)&obj;
    return 0;
}

extern int getMem(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, void*& buf);

int readVerNvramNoLock(void*& p);

static int getBufAndReadNoLock(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p, int bForceRead);

extern int readRamVersion(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, int& version);

int writeVerNvramNoLock();
extern int writeRamVersion(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, int version);

extern int writeRamUpdatedVersion(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev);

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
        int i, j;
        void* p = nullptr;
        for(i=0;i<nvRamArrSz;i++) {
            for(j=0;j<SENSOR_IDX_MAX;j++) {
                g_isNvBufRead[i][j]=0;

                if(shouldDump)
                {
                    if(sensorDump == j && (typeDump == -1 || typeDump == i))
                    {
                        MINT32 sensorDev = NS3Av3::mapSensorIdxToDev(static_cast<MINT32>(j));
                        obj.getBufAndRead(static_cast<CAMERA_DATA_TYPE_ENUM>(g_nvramArrInd[i]), sensorDev, p, MFALSE);
                        obj.write(static_cast<CAMERA_DATA_TYPE_ENUM>(g_nvramArrInd[i]), sensorDev);
                        logI("read and dump nvram: NVRAM struct id(%d), sensorInd(%d), sensorDev(%d)", g_nvramArrInd[i], j, sensorDev);
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

extern int getSensorID(CAMERA_DUAL_CAMERA_SENSOR_ENUM i4SensorDev, int& sensorId);

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

    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    err  = nvDrv->readDefaultData (
            (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, nvRamId,
            buf );
    nvDrv->destroyInstance();
    return err;
}

//==============================================================
// for camera version use
extern int readVerNvramNoLock(void*& p);
extern int writeVerNvramNoLock();

//==============================================================

int writeNvramNoLock(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev)
{
    int err;
    void* buf;
    int u4SensorID;
    int nvSize;
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

int NvBufUtil::getSensorIdAndModuleId(MINT32 sensorType, MINT32 sensoridx, MINT32 &sensorId, MUINT32 &moduleId)
{
    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();

    getSensorID((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorType, sensorId);

    moduleId=nvDrv->readModuleIdFromEEPROM((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorType, sensoridx, MTRUE);
    nvDrv->destroyInstance();

    return 0;
}

int NvBufUtil::getBuf(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p)
{
    logI("+");
    DEF_AutoLock(m_cs);
    int err;
    err = getMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, p);
    return err;
}

int NvBufUtil::getBufAndReadNoDefault(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p, int bForceRead)
{
    logI("+ ramId(%d) dev(%d)",(int)nvRamId, sensorDev);
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

    if(bRead==1)
    {
        logE("error!, bRead(%d)", bRead);
        return 0;
    }

    int nvSize;
    err = getNvSize(nvRamId, nvSize);
    if(err!=0)
    {
        logE("getNvSize error!");
        return err;
    }

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
    logI("+ ramId(%d), sensorDev(%d)",(int)nvRamId, sensorDev);
    int err;
    int bRead;
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

    int ramVer;
    err = readRamVersion(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, ramVer);
    logI("readRamVersion, ver(%d), err(%d)", ramVer, err);

    if(err!=0)
        return err;

    int u4SensorID;
    int nvSize;
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
    logI("nvRamId(%d), sensorDev(%d)", (int)nvRamId, sensorDev);
    DEF_AutoLock(m_cs);
    int err;
    int err2;
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

    void* p;
    err = getBufAndReadNoLock(nvRamId, sensorDev, p, 1);
    if(err!=0)
        return err;

    return err;
}
int NvBufUtil::readDefault(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev)
{
    logI("+");
    DEF_AutoLock(m_cs);
    return readDefaultNoLock(nvRamId, sensorDev);
}

int NvBufUtil::readDefault(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void* p)
{
    logI("+ with buf para p(%p)", p);
    DEF_AutoLock(m_cs);
    int err;
    int u4SensorID;
    void* buf;
    int nvSize;
    err = getSensorIdandMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, buf, nvSize);

    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    err  = nvDrv->readDefaultData (
            (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, nvRamId,
            p   );
    nvDrv->destroyInstance();
    return err;
}

static void dump_nv_buf(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, void* buf, int nvSize, int mode)
{
    DIR* dir = opendir(gCreateDirD);
    if(!dir)
    {
        return;
    }
    logI("sensorDev(%d), nvRamId(%d), buf(%p), nvSize(%d)", sensorDev, nvRamId, buf, nvSize);
    std::lock_guard<std::mutex> lock(gdumpNvLock);
    int mask = umask(0);
    logI("umask(%d)", mask);
    char fname[MAX_FILE_PATH_LEN];
    getNvFileNameU(sensorDev, nvRamId, fname);

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
