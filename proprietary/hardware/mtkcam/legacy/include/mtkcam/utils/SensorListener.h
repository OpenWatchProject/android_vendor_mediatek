#ifndef SENSOR_LISTENER_H
#define SENSOR_LISTENER_H
//-----------------------------------------------------------------------------
class SensorListener
{
    protected:
        virtual ~SensorListener() {};
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
        struct ALooper;
        struct ASensorEvent;

        // Concrete types for the NDK
        struct ASensorEventQueue {
            ALooper* looper;
        };
        //
        typedef void (*Listener) (ASensorEvent event);
        //
        static SensorListener*  createInstance(void);
        virtual MVOID   destroyInstance(void) = 0;
        virtual MBOOL   setListener(Listener func) = 0;
        virtual MBOOL   enableSensor(
                            SensorTypeEnum  sensorType,
                            MUINT32         periodInMs) = 0;
        virtual MBOOL   disableSensor(SensorTypeEnum sensorType) = 0;
};
//-----------------------------------------------------------------------------
#endif

