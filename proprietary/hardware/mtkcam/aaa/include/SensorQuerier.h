#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_SYS_SENSORQUERIER_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_SYS_SENSORQUERIER_H_
//-----------------------------------------------------------------------------
class SensorQuerier
{
    protected:
        virtual ~SensorQuerier() {};
    //
    public:
        typedef enum
        {
            SensorType_Acc,
            SensorType_Mag,
            SensorType_Gyro,
            SensorType_Light,
            SensorType_Proxi
        }SensorTypeEnum;
        //
        typedef void (*Querier) (ASensorEvent event);
        //
//        static SensorQuerier*  createInstance(void);
        static SensorQuerier*  createInstance(char* Caller = "unknown");
        virtual MVOID   destroyInstance(void) = 0;
        virtual MBOOL   setQuerier(Querier func) = 0;
        virtual MBOOL   enableSensor(
                            SensorTypeEnum  sensorType,
                            MUINT32         periodInMs) = 0;
        virtual MBOOL   disableSensor(SensorTypeEnum sensorType) = 0;
};
//-----------------------------------------------------------------------------
#endif

