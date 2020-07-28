#define LOG_TAG "EM/HIDL/BT"
#define MTK_LOG_ENABLE 1

#include <log/log.h>

#include "bt_em.h"
#include "BluetoothTest.h"

extern "C" void RELAYER_exit(void);
extern "C" BOOL RELAYER_start(int serial_port, int serial_speed);




int32_t BluetoothTest::startBtRelayer(int32_t port, int32_t speed)
{
    ALOGI("enter startBtRelayer port =%d speed =%d\n", port, speed);
    return RELAYER_start(port, speed) ? 0 : -1;
}

int32_t BluetoothTest::stopBtRelayer()
{
    ALOGI("enter stopBtRelayer\n");
    RELAYER_exit();
    return 0;
}

int32_t BluetoothTest::isBLESupport()
{

    unsigned char ucEvent[512] = { 0 };
    unsigned int u4ResultLen = 0;
    int result = -1;

    if (EM_BT_init() == false) {
        ALOGE("Leave isBLESupport() due to EM_BT_init() failure...\n");
        return -1;
    }
    unsigned char peer_write_buf4[] = { 0x01, 0x03, 0x10, 0x00 };
    ALOGD("Enter isBLESupport()...\n");
    if (false == EM_BT_write(peer_write_buf4, sizeof(peer_write_buf4))) {
        ALOGE("Leave isBLESupport() due to EM_BT_write() failure...  (4) \n");
        EM_BT_deinit();
        return -1;
    }

    ALOGD("EM_BT_write:");
    for (unsigned int i = 0; i < sizeof(peer_write_buf4); i++) {
        ALOGD("%1$02x    ", peer_write_buf4[i]);
    }

    if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &u4ResultLen)) {
        ALOGE("Leave isBLESupport() due to EM_BT_read() fail...  (4)\n");
        EM_BT_deinit();
        return -1;
    }
    ALOGE("EM_BT_read: event length = %d", u4ResultLen);
    for (unsigned int i = 0; i < u4ResultLen; i++) {
        ALOGE("%1$02x    ", ucEvent[i]);
    }
    if (u4ResultLen > 11) {
        result = (ucEvent[11] & (1 << 6)) == 0 ? 0 : 1;
    } else {
        result = 0;
    }
    EM_BT_deinit();
    ALOGD("Leave isBLESupport()...\n");
    return result;
}

bool BluetoothTest::isBLEEnhancedSupport()
{

    ALOGE("Enter BtTest_isBLEEnhancedSupport.");
    unsigned char ucResult[512] = {0};
    unsigned int uiResultLen = 0;


    if (EM_BT_init() == false) {
        ALOGE("EM_BT_init() failure\n");
        return false;
    }
    unsigned char cmd[] = { 0x01, 0x02, 0x10, 0x00 };
    if (false == EM_BT_write(cmd, sizeof(cmd))) {
        ALOGE("EM_BT_write() failure\n");
        EM_BT_deinit();
        return false;
    }

    if (false == EM_BT_read(ucResult, sizeof(ucResult), &uiResultLen)) {
        ALOGE("EM_BT_read() failure\n");
        EM_BT_deinit();
        return false;
    }
    ALOGE("EM_BT_read: event length = %d", uiResultLen);
    for (unsigned int i = 0; i < uiResultLen; i++) {
        ALOGD("%1$02x   ", ucResult[i]);
    }

    if(uiResultLen >= 37) {
        if (ucResult[6] != 0) {
            ALOGD("Status error");
            return false;
        }
        bool isRecSupport = !((ucResult[42] & (1 << 7)) == 0);
        bool isTransSupport = !((ucResult[43] & 1) == 0);
        ALOGD("isRecSupport:%1d   isTransSupport:%2d", isRecSupport, isTransSupport);
        EM_BT_deinit();
        return isRecSupport && isTransSupport;
    } else {
        EM_BT_deinit();
        return false;
    }
}

int32_t BluetoothTest::init()
{
    ALOGE("Enter BtTest_Init\n");
    return EM_BT_init() ? 0 : -1;;
}

int32_t BluetoothTest:: uninit()
{
    ALOGE("Enter BtTest_UnInit\n");
    EM_BT_deinit();
    return 0;
}



static int process_modetest(int32_t nPower) {
    ALOGE("Enter process_modetest()...\n");

    unsigned char ucEvent[512];
    unsigned int u4ResultLen = 0;

    if (EM_BT_init() == false) {
        ALOGE("Leave process_modetest() due to EM_BT_init() failure...\n");
        // jniThrowException(env, "BT init failed", NULL);
        return -1;
    }
    ALOGD("Power val %d:", nPower);
    //If power passed valid, set power. If not valid, power is not set and value in NVRAM will be used.
    if (nPower != -1) {
        // ==============set power==============
        unsigned char power_write[] = { 0x01, 0x79, 0xfc, 0x06, 0x07, 0x80, 0x00, 0x06, 0x05, 0x07 };
        if (nPower <= 9 && nPower >= 0) {
            power_write[4] = (unsigned char) nPower;
            power_write[9] = (unsigned char) nPower;
        }


        if (false == EM_BT_write(power_write, sizeof(power_write))) {
            ALOGE("Leave process_modetest() due to power_write EM_BT_write() failure... (3)\n");
            EM_BT_deinit();
            return -1;
        }
        if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &u4ResultLen)) {
            ALOGE("Leave process_modetest() due to power_write EM_BT_read() failure. (3)\n");
            EM_BT_deinit();
            return -1;
        }
        ALOGE("Power EM_BT_read:");
        for (unsigned int i = 0; i < u4ResultLen; i++) {
            ALOGE("%1$02x    ", ucEvent[i]);
        }
    }

    unsigned char peer_write_buf2[] = { 0x01, 0x03, 0x18, 0x00 };

    if (false == EM_BT_write(peer_write_buf2, sizeof(peer_write_buf2))) {
        ALOGE("Leave process_modetest() due to EM_BT_write() failure... (2)\n");
        EM_BT_deinit();
        return -1;
    }

    ALOGD("EM_BT_write:");
    for (unsigned int i = 0; i < sizeof(peer_write_buf2); i++) {
        ALOGD("%1$02x    ", peer_write_buf2[i]);
    }

    if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &u4ResultLen)) {
        ALOGE("Leave process_modetest() due to EM_BT_read() failure... (2)\n");
        EM_BT_deinit();
        return -1;
    }

    ALOGE("EM_BT_read:length = %d", u4ResultLen);
    for (unsigned int i = 0; i < u4ResultLen; i++) {
        ALOGE("%1$02x    ", ucEvent[i]);
    }

    unsigned char peer_write_buf3[] = { 0x01, 0x1A, 0x0C, 0x01, 0x03 };

    if (false == EM_BT_write(peer_write_buf3, sizeof(peer_write_buf3))) {
        ALOGE("Leave process_modetest() due to EM_BT_write() failure... (3)\n");
        EM_BT_deinit();
        return -1;
    }

    ALOGD("EM_BT_write:");
    for (unsigned int i = 0; i < sizeof(peer_write_buf3); i++) {
        ALOGD("%1$02x    ", peer_write_buf3[i]);
    }

    if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &u4ResultLen)) {
        ALOGE("Leave process_modetest() due to EM_BT_read() failure. (3)\n");
        EM_BT_deinit();
        return -1;
    }

    ALOGE("EM_BT_read:length = %d", u4ResultLen);
    for (unsigned int i = 0; i < u4ResultLen; i++) {
        ALOGE("%1$02x    ", ucEvent[i]);
    }
    unsigned char peer_write_buf4[] = { 0x01, 0x05, 0x0C, 0x03, 0x02, 0x00, 0x02 };

    if (false == EM_BT_write(peer_write_buf4, sizeof(peer_write_buf4))) {
        ALOGE("Leave process_modetest() due to EM_BT_write() failure...  (4) \n");
        EM_BT_deinit();
        return -1;
    }

    ALOGD("EM_BT_write:");
    for (unsigned int i = 0; i < sizeof(peer_write_buf4); i++) {
        ALOGD("%1$02x    ", peer_write_buf4[i]);
    }

    if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &u4ResultLen)) {
        ALOGE("Leave process_modetest() due to EM_BT_read() fail...  (4)\n");
        EM_BT_deinit();
        return -1;
    }

    ALOGE("EM_BT_read:length = %d", u4ResultLen);
    for (unsigned int i = 0; i < u4ResultLen; i++) {
        ALOGE("%1$02x    ", ucEvent[i]);
    }


    ALOGE("Leave process_modetest()...\n");
    return 0;
}

static unsigned char Pattern_Map[] = { 0x01, 0x02, 0x03, 0x04, 0x09 };

static unsigned char Channels_Map[] = { 0x00, 0x01, 0x03, 0x05 };

static unsigned char PocketType_Map[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0E, 0x0F, 0x17, 0x1C, 0x1D, 0x24, 0x28, 0x2A, 0x2B, 0x2E, 0x2F, 0x36,
        0x37, 0x3C, 0x3D };

static int process_txtest(int32_t nPatter, int32_t nChannels, int32_t nPocketType,
        int32_t nPocketTypeLen, int32_t nFreq) {

    if (EM_BT_init() == false) {
        ALOGE("Leave process_txtest() due to EM_BT_init() failure...\n");
        return -1;
    }

    unsigned char ucEvent[512];
    unsigned int u4ResultLen = 0;

    char channel = Channels_Map[1]; //default 79 hop
    if (nChannels < sizeof(Channels_Map)) {
        channel =  Channels_Map[nChannels];
    } else {
        unsigned char pre_cmd_buf[] = { 0x01, 0x90, 0xfd, 0x01,
                (unsigned char) (nChannels - sizeof(Channels_Map)) };

        if (false == EM_BT_write(pre_cmd_buf, sizeof(pre_cmd_buf))) {
            ALOGE("Leave process_txtest() due to EM_BT_write() failure...\n");
            EM_BT_deinit();
            return -1;
        }

        ALOGD("EM_BT_write:");
        for (unsigned int i = 0; i < sizeof(pre_cmd_buf); i++) {
            ALOGD("%1$02x    ", pre_cmd_buf[i]);
        }

        memset(ucEvent, 0, sizeof(ucEvent));
        if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &u4ResultLen)) {
            ALOGE("Leave process_txtest() due to EM_BT_read() failure...\n");
            EM_BT_deinit();
            return -1;
        }

        ALOGE("EM_BT_read:length = %d", u4ResultLen);
        for (unsigned int i = 0; i < u4ResultLen; i++) {
            ALOGE("%1$02x    ", ucEvent[i]);
        }
    }
    unsigned char peer_write_buf[] = { 0x01, 0x0D, 0xfc, 0x17, 0x00, 0x00, Pattern_Map[nPatter],
            channel, (unsigned char) nFreq, 0x00, 0x00, 0x01,
            PocketType_Map[nPocketType], ((unsigned char) (nPocketTypeLen & 0xff)),
            ((unsigned char) ((0xff00 & nPocketTypeLen) >> 8)), 0x02, 0x00, 0x01, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    if (false == EM_BT_write(peer_write_buf, sizeof(peer_write_buf))) {
        ALOGE("Leave process_txtest() due to EM_BT_write() failure...\n");
        EM_BT_deinit();
        return -1;
    }

    ALOGD("EM_BT_write:");
    for (unsigned int i = 0; i < sizeof(peer_write_buf); i++) {
        ALOGD("%1$02x    ", peer_write_buf[i]);
    }

    memset(ucEvent, 0, sizeof(ucEvent));
    if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &u4ResultLen)) {
        ALOGE("Leave process_txtest() due to EM_BT_read() failure...\n");
        EM_BT_deinit();
        return -1;
    }

    ALOGE("EM_BT_read:length = %d", u4ResultLen);
    for (unsigned int i = 0; i < u4ResultLen; i++) {
        ALOGE("%1$02x    ", ucEvent[i]);
    }
    // this means there are more than one event data in this message, if u4ResultLen > 3 + ucEvent[2]
    // if there are 2 piece of event data in this message, we donot need to read event again
    if (!(u4ResultLen > 3 + ucEvent[2])) {

        memset(ucEvent, 0, sizeof(ucEvent));
        if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &u4ResultLen)) {
            ALOGE("Leave process_txtest() due to EM_BT_read() failure...\n");
            EM_BT_deinit();
            return -1;
        }

        ALOGE("EM_BT_read:length = %d", u4ResultLen);
        for (unsigned int i = 0; i < u4ResultLen; i++) {
            ALOGE("%1$02x    ", ucEvent[i]);
        }
    }

    ALOGE("Leave process_txtest()...\n");

    return 0;
}

static int post_process_txtest() {
    ALOGE("Enter post_process_txtest()...\n");
    EM_BT_deinit();
    ALOGE("Leave post_process_txtest()...\n");
    return 0;
}

static int post_process_modetest() {
    ALOGE("Enter post_process_modetest()...\n");
    EM_BT_deinit();
    ALOGE("Leave post_process_modetest()...\n");
    return 0;
}

int32_t BluetoothTest::doBtTest(int32_t kind, int32_t pattern, int32_t channel, int32_t pocketType,
        int32_t pocketTypeLen, int32_t freq, int32_t power)
{
    ALOGE("Enter BtTest_doBtTest(kind=%x)... \n", kind);

    if (0 == kind) {
        return process_txtest(pattern, channel, pocketType, pocketTypeLen, freq);
    } else if (1 == kind) {
        return process_modetest(power);
    } else if (2 == kind) {
        return post_process_modetest();
    } else if (3 == kind) {
        return post_process_txtest();
    }

    ALOGE("Leave BtTest_doBtTest()...\n");
    return 0;

}

const vector<uint8_t> BluetoothTest::hciCommandRun(const vector<uint8_t>& input)
{
    // call EM_BT_write to send HCI command
    unsigned int u4ResultLen = 0;
    unsigned char ucResultArray[512] = { 0 };

    int length = input.size();
    vector<uint8_t> output;
    unsigned char *ucHCICmdBuffer = (unsigned char *)input.data();
    if (ucHCICmdBuffer == NULL) {
        ALOGE("Error retrieving source of EM paramters");
        return output;
    }
    ALOGD("Enter BtTest_HCICommandRun()...\n");

    ALOGD("EM_BT_write:");
    for (int i = 0; i < length; i++) {
        ALOGD("%1$02x", ucHCICmdBuffer[i]);
    }
    if (!EM_BT_write(ucHCICmdBuffer, length)) {
        ALOGE("EM_BT_write call failed, Leave BtTest_HCICommandRun()...\n");
        return output;
    }

    if (false == EM_BT_read(ucResultArray, sizeof(ucResultArray), &u4ResultLen)) {
        ALOGE("EM_BT_read call failed, Leave BtTest_HCICommandRun()...\n");
        return output;
    }
    ALOGD("EM_BT_read:  event length = %d", u4ResultLen);

    for (unsigned int i = 0; i < u4ResultLen; i++) {
        ALOGD("%1$02x", ucResultArray[i]);
        output.push_back((uint8_t)ucResultArray[i]);
    }
    return output;

}

static unsigned char NoSigRxBBMap[] = { 0x01, 0x02, 0x03, 0x04, 0x09 };
static unsigned char NoSigRxRRMap[] = { 0x03, 0x04, 0x0A, 0x0B, 0x0E, 0x0F, 0x24, 0x28, 0x2A, 0x2B,
        0x2E, 0x2F };

bool BluetoothTest::startNoSigRxTest(int32_t pattern, int32_t pockettype,
    int32_t freq, int32_t address)
{

    ALOGE("Enter BtTest_NoSigRxTest.");

    unsigned char ucEvent[512];
    unsigned int u4ResultLen = 0;

    if (EM_BT_init() == false) {
        ALOGE("Leave BtTest_NoSigRxTest() due to EM_BT_init() failure\n");
        return 0;
    }

    unsigned char peer_write_buf[] = { 0x01, 0x0D, 0xFC, 0x17, 0x00, NoSigRxBBMap[pattern],
            0x0B, 0x00, 0x00, (char) freq, 0x00, 0x01, NoSigRxRRMap[pockettype], 0x00, 0x00,
            0x02, 0x00, 0x01, 0x00, 0x00, 0x00, ((unsigned char) ((0xff000000 & address) >> 24)),
            ((unsigned char) ((0xff0000 & address) >> 16)), ((unsigned char) ((0xff00 & address)
                    >> 8)), ((unsigned char) (address & 0xff)), 0x00, 0x00 };

    if (false == EM_BT_write(peer_write_buf, sizeof(peer_write_buf))) {
        ALOGE("Leave BtTest_NoSigRxTest() due to EM_BT_write() failure...\n");
        EM_BT_deinit();
        return 0;
    }

    ALOGD("EM_BT_write:");
    for (unsigned int i = 0; i < sizeof(peer_write_buf); i++) {
        ALOGD("%1$02x    ", peer_write_buf[i]);
    }

    memset(ucEvent, 0, sizeof(ucEvent));
    if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &u4ResultLen)) {
        ALOGE("Leave BtTest_NoSigRxTest() due to EM_BT_read() failure...\n");
        EM_BT_deinit();
        return 0;
    }

    ALOGE("EM_BT_read:");
    for (unsigned int i = 0; i < u4ResultLen; i++) {
        ALOGE("%1$02x    ", ucEvent[i]);
    }

    // receive it again
    if (u4ResultLen == 14) {
        memset(ucEvent, 0, sizeof(ucEvent));
        if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &u4ResultLen)) {
            ALOGE("Leave BtTest_NoSigRxTest() due to EM_BT_read() failure...\n");
            EM_BT_deinit();
            return 0;
        }

        ALOGE("EM_BT_read:");
        for (unsigned int i = 0; i < u4ResultLen; i++) {
            ALOGE("%1$02x    ", ucEvent[i]);
        }
    }

    return 1;

}

const vector<int32_t> BluetoothTest::endNoSigRxTest()
{

    ALOGE("Enter BtTest_EndNoSigRxTest.");

    unsigned char ucEvent[512];
    unsigned int u4ResultLen = 0;
//=============================GET result===
    unsigned char peer_write_buf2[] = { 0x01, 0x0D, 0xFC, 0x17, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
            0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00 };

    vector<int32_t> data;
    if (false == EM_BT_write(peer_write_buf2, sizeof(peer_write_buf2))) {
        ALOGE("Leave BtTest_NoSigRxTest() due to EM_BT_write(2) failure...\n");
        EM_BT_deinit();
        return data;
    }

    ALOGD("EM_BT_write:");
    for (unsigned int i = 0; i < sizeof(peer_write_buf2); i++) {
        ALOGD("%1$02x    ", peer_write_buf2[i]);
    }

    memset(ucEvent, 0, sizeof(ucEvent));
    if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &u4ResultLen)) {
        ALOGE("Leave BtTest_NoSigRxTest() due to EM_BT_read() failure...\n");
        EM_BT_deinit();
        return data;
    }

    ALOGE("EM_BT_read:");
    for (unsigned int i = 0; i < u4ResultLen; i++) {
        ALOGE("%1$02x    ", ucEvent[i]);
    }

    int iRes[4];
    iRes[0] = *(int*) (&ucEvent[7]);
    iRes[1] = *(int*) (&ucEvent[11]) / 10000;  // 1.24 -> 124
    iRes[2] = *(int*) (&ucEvent[15]);
    iRes[3] = *(int*) (&ucEvent[19]) / 10000;

    EM_BT_deinit();
    for (int i = 0; i < 4; i++) {
        data.push_back((int32_t)iRes[i]);
    }
    ALOGE("Leave BtTest_EndNoSigRxTest.");
    return data;

}

int32_t BluetoothTest::isComboSupport()
{
#ifdef MTK_EM_BT_COMBO_SUPPORT
    return 1;
#else
    return 0;
#endif
}

int32_t BluetoothTest::pollingStart()
{
    ALOGI("enter pollingStart\n");
    EM_BT_polling_start();
    return 1;;
}

int32_t BluetoothTest::pollingStop()
{
    ALOGI("enter pollingStop\n");
    EM_BT_polling_stop();
    return 1;
}
