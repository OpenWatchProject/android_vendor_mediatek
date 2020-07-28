#include <vector>
using std::vector;
class BluetoothTest
{
public:
    BluetoothTest();
    virtual ~BluetoothTest();
    static int32_t startBtRelayer(int32_t port, int32_t speed);
    static int32_t stopBtRelayer();
    static int32_t isBLESupport();
    static bool isBLEEnhancedSupport();
    static int32_t init();
    static int32_t uninit();
    static int32_t doBtTest(int32_t kind, int32_t pattern, int32_t channel, int32_t pocketType,
            int32_t pocketTypeLen, int32_t freq, int32_t power);
    static const vector<uint8_t> hciCommandRun(const vector<uint8_t>& input);
    static bool startNoSigRxTest(int32_t pattern, int32_t pockettype,
        int32_t freq, int32_t address);
    static const vector<int32_t> endNoSigRxTest();
    static int32_t isComboSupport();
    static int32_t pollingStart();
    static int32_t pollingStop();

};


