#ifndef __RP_AGPS_NSLOT_CONTROLLER_H__
#define __RP_AGPS_NSLOT_CONTROLLER_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include "RpAgpsMessage.h"
#include "RfxDefs.h"

/*****************************************************************************
 * Class Decalartion
 *****************************************************************************/
class RpAgpsThread;
class RpAgpsdAdapter;

/*****************************************************************************
 * Class RpAgpsNSlotController
 *****************************************************************************/
/*
 * RpAgpsNSlotController is non slot controller which is created by
 * controller factory. which is used to send the telephony status
 * to AGPSD
 */
class RpAgpsNSlotController : public RfxController {
    RFX_DECLARE_CLASS(RpAgpsNSlotController);

public:
    // Constructor
    RpAgpsNSlotController();

    // Destructor
    virtual ~RpAgpsNSlotController();

    // Get the reference of agps working thread
    //
    // RETURNS: the reference of agps working thread
    sp<RpAgpsThread>& getAgpsThread();

    // Handle the message from AGPSD
    void onHandleAgpsMessage(
        sp<RpAgpsMessage> & message // [IN] message from AGPSD
        );

    // Callback when service state changed
    void onServiceStateChanged(
        int slotId,            // [IN] slot ID
        RfxStatusKeyEnum key,  // [IN] key in status manager
        RfxVariant oldValue,   // old service state value
        RfxVariant newValue    // new service state value
        );

    // Enable test mode
    void setTestMode(
        bool testMode          // [IN] test mode or not
        ) {
        m_testMode = testMode;
    }

    // Only used in test mode
    void setDefaultDataSlot(
        int slot               // [IN] 0, no data, 1~4 data on the slot
        ) {
         m_mockDefaultDataSlot = slot;
    }

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

// Override
protected:
    virtual void onInit();
    virtual void onDeinit();

// Implement
private:
    // Register/unRegister service state changed
    void registerServiceStateChanged();
    void unRegisterServiceStateChanged();

    // Data enabled on C card or not
    bool isDataEnabledOnCCard();

    // MD3 in service or not
    bool isC2KinService();

    // Get the current network type
    int getNetworkType();

    // Send Data Enabled state to AGPSD
    void sendDataEnabledStatus(int status);

    // Get Current Data Enabled Status
    int getCurrentDataEnabledStatus();

    // Send MD3 status to AGPSD
    void sendC2KMDStatus(int state);

    // Send the network type to AGPSD
    void sendNetworkType(int type);

    // Get the slot Id of the data
    int getDefaultDataSlotId();

    // Get the slot Id of CDMA
    int getCDMASlotId();

    // Convert MTK extend radio tech to AOSP
    int convertMtkRadioTech(int radioTech);

    // Convert the radio tech to network type
    NetworkType convertRadioTech(int radioTech);

    // Convert the reg state to service state
    ServiceState convertServiceState(int regState);

    // pointer of agpsd adapter
    RpAgpsdAdapter *m_agpsdAdapter;

    // reference of agps working thread
    sp<RpAgpsThread> m_agpsThread;

    // previous network type
    NetworkType m_networkType;

    // previous service state
    ServiceState m_state;

    // previous data enabled status
    int m_dataEnabledStatus;

    // test mode switch
    bool m_testMode;

    // mock default data slot
    int m_mockDefaultDataSlot;
};

#endif /* __RP_AGPS_SLOT_CONTROLLER_H__ */