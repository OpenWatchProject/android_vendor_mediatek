#ifndef __CHAMELEON_DISPLAY_H__
#define __CHAMELEON_DISPLAY_H__
////////////////General Define////////////////
#define BIT0    0x01
#define BIT1    0x02
#define BIT2    0x04
#define BIT3    0x08
#define BIT4    0x10
#define BIT5    0x20
#define BIT6    0x40
#define BIT7    0x80


////////////////TChameleonDisplayProcess:Sensor Debounce////////////////
enum{
    ENUM_SensorValueNotChanged = 0,
    ENUM_SensorValueChanged
};

/*Color Class*/
#define SCALE_TXYZ  1000.0f
struct TXYZ {
    int X;
    int Y;
    int Z;
};         //integer x 3 (double x 10^3)

struct TRGBXYZ {
    TXYZ R;
    TXYZ G;
    TXYZ B;
};      //TXYZ x 3

struct TColorSensorValue {
    int Status;     //0x00: Normal; 0x10: No Sensor; >0x10: Sensor Error Message; 0x80: Status is changed; 0x80AABB: Sensor status is just change from status AA to BB
    int R;
    int G;
    int B;
    int W;
};

struct TColorSensorSettings{
    int Mode;               //0: Settings is static; 1: Settings is controlled by Chameleon; 2: Settings is controlled by other function
    int Gain;               //<=0: don't care or default settings
    int AccumulateInterval; //HW settings; <=0: don't care or default settings; >0: ms
};

#define SCALE_TColorSensorFactoryFactor 10000
#define TColorSensorFactoryFactor_CheckCode 0x3355AACC
struct TColorSensorFactoryFactor{       //(double x 10^4)
    int GainR;
    int GainG;
    int GainB;
    int GainW;

    int OffsetR;
    int OffsetG;
    int OffsetB;
    int OffsetW;

    int CheckCode;                      //It must 0x3355AACC
};

#define SCALE_TCcorrConfig  1024
struct TCcorrConfig{                //(double x 1024)
    unsigned int Coef[3][3];
};

#define LS_CNT  10
#define FC_CNT  6
#define SCALE_TColorSensorCalibrationFactor 10000.0f
#define TColorSensorCalibrationFactor_CheckCode 0x2ACC3355
struct TColorSensorCalibrationFactor{   //(double x 10^4)
    int CalibrationData_LS[LS_CNT][FC_CNT];
    int CalibrationData_RX[3][3];
    int W_RGBWeight[3];
    int Y_ScaleUp;
    int BrightAmbientFallbackEnabled;
    int BrightAmbientFallbackLevel;
    int TColorSensorCalibrationFactorTemp[10];

    int CheckCode;                      //It must 0x2ACC3355
};

#define SCALE_THRESHOLD_PERCENT     1000
#define TColorSensorDebounce_CheckCode 0x55AACC33
struct TColorSensorDebounce{
    int SWAccumulateTimes;          //For SW debounce
    int InvalidDataThreshold;       //Too low and not use the data
    int LowLightDataThreshold;
    int LowLightLuminanceThreshold;
    int ChangeThreshold;            //In Percent (double x 10^3)
    int StableTimesThreshold;
    int ForceChange;                //0: No Force, 1: Force once, 2: Always Force
    int TellLightSourceWThreshold;
    int TColorSensorDebounceTemp[10];

    int CheckCode;                      //It must 0x55AACC33
};


struct TChameleonDisplayInput{
    //Sensor Info
    TColorSensorValue SensorValue;
    TColorSensorSettings SensorSettings;
    //Color Settings
    TCcorrConfig OriginalCCORR;
    int CurrentBacklightSettings;
};

struct TChameleonDisplayReg{
    //Chameleon Settings
    int Enabled;
    int Strength;
    int BacklightControlEnabled;
    int StrengthLimit;
    int LightTellMode;                              //0: Tell Light, 1: Linear Insertion Mode
    int DebugFlag;                                  //Bit 0: Sensor Log (Original Data), 1: Sensor Log (After ACC), 2: Sensor Log (After Debounce), 3: Calculate Process, 4: Calculate Process Detail, 5: In/Out/SW settings

    //Sensor Calibrator
    TColorSensorFactoryFactor SensorFactoryFactor;
    TColorSensorCalibrationFactor SensorCalibrationFactor;

    //Sensor Debounce
    TColorSensorDebounce SensorDebounce;

    //Panel Information
    TRGBXYZ Panel;

    //Debug
    int DebugTrace;
};

struct TChameleonDisplayOutput
{
    //Color Settings
    TCcorrConfig TargetCCORR;
    int TargetBacklight;
    //Sensor Info
    TColorSensorSettings SensorSettings;
};


struct TChameleonDisplayInitParam {
    unsigned int reserved; // not used
};


class TChameleonDisplayProcess {
private:
    TChameleonDisplayReg *ChameleonDisplayReg;
    TColorSensorValue *PreviousSensorValue;
    TColorSensorValue *CurrentSensorValue;
    TXYZ *AmbientXYZ;

    int AccumulateTimes;

    bool CheckIfSensorValueValidInSingleChannel(int current_ali);
    bool CheckIfSensorValueValid();
    bool CheckIfAmbientChangedInSingleChannel(int current_ali, int previous_ali);
    bool CheckIfAmbientChanged();
    int onSensorSWAccumulate(const TColorSensorValue &input);
    int SensorValueFactoryCalibrate();

    int GetColorTypeIndex();
    int GetTargetXYZFromSensor(TXYZ &TargetXYZ);
    int GetRGBGainBYStandardFormulaCorrection(const TChameleonDisplayInput &input, TChameleonDisplayOutput *output);

public:
    TChameleonDisplayProcess();
    ~TChameleonDisplayProcess();

    void onInitCommon(const TChameleonDisplayInitParam &initParam);
    void onInitPlatform(const TChameleonDisplayInitParam &initParam);

    void setEnabled(bool enabled);
    bool isEnabled();
    void setStrength(int strength); // 0 ~ 255
    int getStrength();
    void setBacklightControlEnabled(bool enabled);
    bool isBacklightControlEnabled();
    void setForceChange(int force_change);

    int AssignChameleonDisplayRegSensorFactoryFactor(int *cd_ff_param_ptr);
    int AssignChameleonDisplayRegSensorCalibrationFactor(int *cd_cf_param_ptr);
    int AssignChameleonDisplayRegSensorDebounce(int *input);
    int AssignChameleonDisplayRegPanel(int *cd_panel_param_ptr);

    int onSensorDebounce(const int aliR, const int aliG, const int aliB, const int aliW);
    int onSensorDebounce(const TColorSensorValue &input);

    int onCalculate(const TChameleonDisplayInput &input, TChameleonDisplayOutput *output);  //0: Nothing change, 1: Parameters change

    void setDebugFlag(int debugFlag);

    // Return true if success
    bool setTuningField(int field, unsigned int value);
    bool getTuningField(int field, unsigned int *value);
};

#endif
