#ifndef AALIMPLINTERFACE_H
#define AALIMPLINTERFACE_H
#include "AALCust.h"
#include "AALDef.h"

#undef LOG_TAG
#define LOG_TAG "AAL"

class CAALFW;



enum LCM_TYPE {
    LCD = 0,
    AMOLED = 1
};

enum UD_CURVE_TYPE {
    PIXEL_GAIN = 0,
    BACKLIGHT_LEVEL = 1
};

struct AALInfo {
    int lcm_type;
    bool ess_ready;
    bool dre_ready;
    float ess_version;
    float dre_version;
};

class AALInterface {
public:
    AALInterface();
    ~AALInterface();

    CAALFW *mAALFW;

    int setESS_CurveType(int key);
    int setESS_PanelType(int key);
    int setESS_HistType(int key);
    int setSmartBacklightStrength(int level);
    int setSmartBacklightRange(int level);
    int setESSRegByVer(int level);
    int setDRERegByVer(int level);
    int setReadabilityLevel(int level);
    int setReadabilityBLILevel(int level);
    void getParameters(ImplParameters *outParam);
    void setUDCurve(int type, int index, int level);
    int setTuningReg(char *reg_name, int *value);
    int getTuningReg(char *reg_name, int *value);
    int setBL_SmoothType(int key);

    void readField(uint32_t field, uint32_t *value);
    void writeField(uint32_t field, uint32_t value);
    bool isFieldLegal(uint32_t field);

    bool isAALFwBypassed();

    void onCalculate(AALInput &input, AALOutput *output);
    void onInitFunction();
    void onInitCommon(const AALInitParam &initParam, AALInitReg *initReg);
    void onInitPlatform(const AALInitParam &initParam, CustParameters &cust, AALInitReg *initReg);
    void setDebugFlags(unsigned int debug);
    void getAALInfo(struct AALInfo *aalinfo);
};


#endif

