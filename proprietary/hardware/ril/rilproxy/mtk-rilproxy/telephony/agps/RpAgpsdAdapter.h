/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxObject.h"
#include "RpAgpsMessage.h"
#include <utils/Looper.h>

/*****************************************************************************
 * Defines
 *****************************************************************************/
#define EVENT_AGPS_NETWORK_TYPE          1
#define EVENT_AGPS_CDMA_PHONE_STATUS     2
#define EVENT_AGPS_MOBILE_DATA_STATUS    3
#define EVENT_AGPS_SET_APN               5
#define EVENT_AGPS_DESTROY_APN           6

#define EVENT_UPDATE_STATE_TO_AGPSD      100
#define EVENT_SET_APN_RESULT             101

typedef enum {
    APN_TYPE_CTWAP
} ApnTypeEnum;

/*****************************************************************************
 * Namespace Declaration
 *****************************************************************************/
using ::android::LooperCallback;
using ::android::Looper;

/*****************************************************************************
 * Class Declaraion
 *****************************************************************************/
class RpAgpsdAdapter;

/*****************************************************************************
 * class AgpsLooperCallback
 *****************************************************************************/
/*
 * The looper callback when AGPS IN fd is readable
 */
class AgpsLooperCallback: public LooperCallback {
public:
    // Constructor
    AgpsLooperCallback(RpAgpsdAdapter *adapter): m_adapter(adapter){}

    // Destructor
    virtual ~AgpsLooperCallback() {}

// Override
protected:
    int handleEvent(int fd, int events, void* data);

// Implement
private:
    // referenc of agps adapter
    wp<RpAgpsdAdapter> m_adapter;
};


/*****************************************************************************
 * class RpAgpsdAdapter
 *****************************************************************************/
/*
 * RpAgpsdAdapter is a class that is used to communicate with AGPSD
 */
class RpAgpsdAdapter :public RfxObject {
    RFX_DECLARE_CLASS(RpAgpsdAdapter);
    RFX_OBJ_DECLARE_SINGLETON_CLASS(RpAgpsdAdapter);
    RFX_DECLARE_NO_COPY_CLASS(RpAgpsdAdapter);

public:
    // process the message from the AGPS working that
    // these messages will be sent to AGPSD
    void processOutputMessage(
        sp<RpAgpsMessage> &message // [IN] message to AGPSD
        );

    // register the input socket fd to the looper of the
    // AGPS working thread
    void registerFd(
        sp<Looper> looper // [IN] looper of the AGPS working thread
        );

    // Constructor
    RpAgpsdAdapter();

    // Destructor
    virtual ~RpAgpsdAdapter();

    // Set test mode enable
    void setTestMode(
        bool test // true to set test mode
        ) {
        m_testMode = test;
    }

public:
    // Write data to socket
    static int safeWrite(int fd, void *buf, int len);
    // Read data from socket
    static int safeRead(int fd, void* buf, int len);
    // Write a byte to the buffer
    static void putByte(
        char* buff,                 // [IN] buffer to write
        int* offset,                // [IN] write position
        const char input            // [IN] the value to be write
        );
    // Write a 16 bit integer to the buffer
    static void putInt16(
        char* buff,                 // [IN] buffer to write
        int* offset,                // [IN] write position
        const int16_t input         // [IN] the value to be write
        );
    // Write a 32 bit integer to the buffer
    static void putInt32(
        char* buff,                 // [IN] buffer to write
        int* offset,                // [IN] write position
        const int32_t input         // [IN] the value to be write
        );
    // Read a byte from the buffer
    static char getByte(
        char* buff,                 // [IN] buffer to read to
        int* offset                 // [IN/OUT] read position and next position
        );
    // Read a 16bit integer from the buffer
    static int16_t getInt16(
        char* buff,                 // [IN] buffer to read to
        int* offset                 // [IN/OUT] read position and next position
        );
    // Read a 32bit integer from the buffer
    static int32_t getInt32(
        char* buff,                 // [IN] buffer to read to
        int* offset                 // [IN/OUT] read position and next position
        );

// Override
protected:
    void onInit();
    void onDeinit();

// Implement
private:
    // Send Message to AGPSD
    int sendToAgpsd(sp<RpAgpsMessage> &message);
    // Create the linux abstract socket
    int createSocket(
        const char *name,         // [IN] socket name
        int type,                 // [IN] socket type
        struct sockaddr_un *addr, // [IN] socket address
        socklen_t *len            // [IN] socket address len
        );
    // Create the TCP socket to send message to AGPSD
    int createOutSocket();
    // Create teh UDP socket to receve message from AGPSD
    int createInSocket();
    // make the linux abstract socket address
    int makeAddress(
        const char *name,           // [IN] socket name
        struct sockaddr_un *addr,   // [IN] socket address
        socklen_t *len              // [IN] socket address len
        );

    // Process the input from the AGPSD
    void processInput(
        void *buf,                 // [IN] input buffer
        int len                    // [IN] len of the data
        );

    // Check the peer socket status
    int checkCanWrite(int sock_fd);

    // Output TCP socket name
    const char* c_strOutSocket = "c2kagpsd";

    // Output TCP socket name used in test mode
    const char* c_strOutSocketTest = "c2kagpsdTest";

    // Input UDP socket name
    const char* c_strInSocket = "c2kagpsd2";

    const char* c_strInSocketTest = "c2kagpsd2Test";

    // The input fd
    int m_inFd;

    // The reference of the looper callback when input fd readable
    sp<AgpsLooperCallback> m_looperCallback;

    // Test mode switch
    bool m_testMode;

    friend class AgpsLooperCallback;
};
