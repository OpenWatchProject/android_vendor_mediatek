#ifndef __RTC_AGPS_CONTROLLER_H__
#define __RTC_AGPS_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include "RtcAgpsUtil.h"
#include "RtcAgpsdAdapter.h"
#include "RtcAgpsThread.h"

/*****************************************************************************
 * Class Declaraion
 *****************************************************************************/
class RtcAgpsdAdapter;
class RtcAgpsMessage;

/*****************************************************************************
 * Class RtcAgpsController
 *****************************************************************************/

class RtcAgpsNSlotController : public RfxController {
    // Required: declare this class
    RFX_DECLARE_CLASS(RtcAgpsNSlotController);

public:
    // The service defined for AGPS
    enum ServiceState {
        IN_SERVICE,     // MD3 is working
        OUT_OF_SERVICE  // MD3 is out of work
    };

    // Data Enabled State num
    enum DataEnabledState {
        DATA_ENABLED_ON_CDMA_LTE  = 1, // Data enabled on C card
        DATA_ENABLED_ON_GSM       = 2, // Data enabled on G card/disabled
    };

    // Network type enum
    // Same as the value in TelephonyManager.java
    enum NetworkType {
        NETWORK_TYPE_UNKNOWN = 0,
        NETWORK_TYPE_GPRS    = 1,
        NETWORK_TYPE_EDGE    = 2,
        NETWORK_TYPE_UMTS    = 3,
        NETWORK_TYPE_CDMA    = 4,
        NETWORK_TYPE_EVDO_0  = 5,
        NETWORK_TYPE_EVDO_A  = 6,
        NETWORK_TYPE_1xRTT   = 7,
        NETWORK_TYPE_HSDPA   = 8,
        NETWORK_TYPE_HSUPA   = 9,
        NETWORK_TYPE_HSPA    = 10,
        NETWORK_TYPE_IDEN    = 11,
        NETWORK_TYPE_EVDO_B  = 12,
        NETWORK_TYPE_LTE     = 13,
        NETWORK_TYPE_EHRPD   = 14,
        NETWORK_TYPE_HSPAP   = 15,
        NETWORK_TYPE_GSM     = 16,
    };

    // MTK extend radio tech see ServiceState.java
    enum RadioTechExt {
        RIL_RADIO_TECHNOLOGY_MTK = 128,
        RIL_RADIO_TECHNOLOGY_HSDPAP,        //129
        RIL_RADIO_TECHNOLOGY_HSDPAP_UPA,    //130
        RIL_RADIO_TECHNOLOGY_HSUPAP,        //131
        RIL_RADIO_TECHNOLOGY_HSUPAP_DPA,    //132
        RIL_RADIO_TECHNOLOGY_DC_DPA,        //133
        RIL_RADIO_TECHNOLOGY_DC_UPA,        //134
        RIL_RADIO_TECHNOLOGY_DC_HSDPAP,     //135
        RIL_RADIO_TECHNOLOGY_DC_HSDPAP_UPA, //136
        RIL_RADIO_TECHNOLOGY_DC_HSDPAP_DPA, //137
        RIL_RADIO_TECHNOLOGY_DC_HSPAP,      //138
        RIL_RADIO_TECHNOLOGY_LTEA           //139
    };

public:
    RtcAgpsNSlotController();
    virtual ~RtcAgpsNSlotController();

    // Calback when the state changed of data connection
    void onDataConnectionChanged(
        int slotId,
        RfxStatusKeyEnum key,  // [IN] the key in status manager
        RfxVariant oldValue,   // [IN] the old data connection status
        RfxVariant newValue    // [IN] the new data connection status
        );

    void onServiceStateChanged(
        int slotId,
        RfxStatusKeyEnum key,
        RfxVariant oldValue,
        RfxVariant newValue);

    void onHandleAgpsMessage(
        sp<RtcAgpsMessage> & message);

    RtcAgpsdAdapter *getAgpsdAdapter();

    void sendAgpsMessage(int msgType, int value);

// Override
protected:
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual void onInit();
    virtual void onDeinit();

public:
    enum GpsEvent {
        REQUEST_DATA_CONNECTION = 0, // MD3 request to setup APN
        CLOSE_DATA_CONNECTION,       // MD3 request to destroy APN
    };

    typedef enum {
        APN_STATE_UNKNOWN     = -1,
        APN_STATE_SETUP_BEGIN = 0,
        APN_STATE_SETUP_DONE  = 1,
        APN_STATE_RESET_BEGIN = 2,
        APN_STATE_RESET_DONE  = 3
    } ApnStateEnum;


private:
    void setConnectionState(int state);

    void controlApn(int event, bool force);

    void handleApnResult(int result);

    void onApnSetResult(int result);

    sp<RtcAgpsThread> getAgpsThread();

    bool isDataOff(int slotId);
    int  getDefaultDataSlotId();

    void setActiveSlotId(int slotId);

    // Convert MTK extend radio tech to AOSP
    int convertMtkRadioTech(int radioTech);

    // Convert the radio tech to network type
    NetworkType convertRadioTech(int radioTech);

    // Convert the reg state to service state
    ServiceState convertServiceState(int regState);

    char* stateToString(ApnStateEnum state);
    char* msgTypeToString(int type);

private:
    //Only be access from working thread !!!
    RtcAgpsdAdapter *m_agpsdAdapter;

    // reference of agps working thread
    sp<RtcAgpsThread> m_agpsThread;

    int m_activeSlotId;


    ApnStateEnum m_apnState;

    // previous network type
    NetworkType m_networkType;

    // previous service state
    ServiceState m_serviceState;

    // previous data enabled status
    int m_dataEnabledStatus;
};

#endif /* __RTCAGPS_CONTROLLER_H__ */

