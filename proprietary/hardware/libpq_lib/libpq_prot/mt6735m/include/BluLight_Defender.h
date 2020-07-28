#ifndef __BLULIGHT_DEFENDER_H__
#define __BLULIGHT_DEFENDER_H__

#define CLR_PARTIAL_Y_SIZE 16
#define CLR_PQ_PARTIALS_CONTROL 5
#define CLR_PURP_TONE_SIZE 3
#define CLR_SKIN_TONE_SIZE 8
#define CLR_GRASS_TONE_SIZE 6
#define CLR_SKY_TONE_SIZE 3

struct ColorRegisters {
    unsigned int GLOBAL_SAT  ;
    unsigned int CONTRAST    ;
    unsigned int BRIGHTNESS  ;
    unsigned int PARTIAL_Y    [CLR_PARTIAL_Y_SIZE];
    unsigned int PURP_TONE_S  [CLR_PQ_PARTIALS_CONTROL][CLR_PURP_TONE_SIZE];
    unsigned int SKIN_TONE_S  [CLR_PQ_PARTIALS_CONTROL][CLR_SKIN_TONE_SIZE];
    unsigned int GRASS_TONE_S [CLR_PQ_PARTIALS_CONTROL][CLR_GRASS_TONE_SIZE];
    unsigned int SKY_TONE_S   [CLR_PQ_PARTIALS_CONTROL][CLR_SKY_TONE_SIZE];
    unsigned int PURP_TONE_H  [CLR_PURP_TONE_SIZE];
    unsigned int SKIN_TONE_H  [CLR_SKIN_TONE_SIZE];
    unsigned int GRASS_TONE_H [CLR_GRASS_TONE_SIZE];
    unsigned int SKY_TONE_H   [CLR_SKY_TONE_SIZE];
    unsigned int CCORR_COEF   [3][3];
};

struct BluLightInitParam {
    unsigned int reserved; // not used
};

struct BluLightReg;

class BluLightDefender {
private:
    bool m_isEnabled;
    int m_strength;
    int m_percentage;
    unsigned int m_debugFlag;

    BluLightReg *BLReg;
public:

    BluLightDefender();
    ~BluLightDefender();

    void setEnabled(bool enabled) {
        m_isEnabled = enabled;
    }

    bool isEnabled() {
        return m_isEnabled;
    }

    void setStrength(int strength); // 0 ~ 255

    int getStrength() {
        return m_strength;
    }
    int getPercentage() {
        return m_percentage;
    }
    void onInitCommon(const BluLightInitParam &initParam);
    void onInitPlatform(const BluLightInitParam &initParam);
    void onCalculate(const ColorRegisters &input, ColorRegisters *output);

    void setDebugFlag(unsigned int debugFlag) {
        m_debugFlag = debugFlag;
    }

    // Return true if success
    bool setTuningField(int field, unsigned int value);
    bool getTuningField(int field, unsigned int *value);

};

#endif
