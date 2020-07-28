#ifndef __PQ_TRANSITION_H__
#define __PQ_TRANSITION_H__
////////////////General Define////////////////
#define BIT0    0x01
#define BIT1    0x02
#define BIT2    0x04
#define BIT3    0x08
#define BIT4    0x10
#define BIT5    0x20
#define BIT6    0x40
#define BIT7    0x80

#include "BluLight_Defender.h"

enum{
    ENUM_BrightToDark,
    ENUM_DarkToBright
};


#define TColorReg_CheckCode 0x3355AACC
struct TColorReg {
    ColorRegisters ColorReg;
    int Backlight;
    int CheckCode;
};

#define TTransitionSettings_CheckCode 0x3355AACC
struct TTransitionSettings {
    int Step;
    int Speed;      //How often to do transition. 0: every times (fastest), >0, every x
    int Strength;   //Reserved (Gain, for easy tuning)
    int Mode;       //0: Base on Step and Speed, 1: Weight Mode, 2: Fixed time mode
    int CheckCode;
};

struct TPQTransitionInput{
    TColorReg InputColor;
};

struct TPQTransitionReg{
    int Enabled;   //false: one step, true: progressive
    int DebugFlag;              //Bit 0: All Transition Data, 1: Transition Diff, 2: General Log, 3: Current Step
    TTransitionSettings BrightToDarkTransitionSettings;
    TTransitionSettings DarkToBrightTransitionSettings;

    //Debug
    int DebugTrace;
};

struct TPQTransitionOutput
{
    TColorReg OutputColor;
};


struct TPQTransitionInitParam {
    unsigned int reserved; // not used
};

class TPQTransitionProcess {
private:
    TPQTransitionReg *PQTransitionReg;
    TColorReg *PreviousColorReg;
    TColorReg *TargetColorReg;
    TColorReg *CurrentColorReg;

    int CurrentStep;
    int CurrentCount;
    int CurrentBrightDarkMode;  //0: Bright to Dark, 1: Dark to Bright
    int Finished;     //To check if transition is finish yet

    int PrintTColorReg(TColorReg *CurrentColorReg);
    int PrintTColorReg(TColorReg *TargetColorReg, TColorReg *PreviousColorReg, TColorReg *CurrentColorReg);

    int CheckIfFinished();      //0: not finished, 1: finished

    int onTransitionMode0(const TTransitionSettings &TransitionSettings);
    int onTransitionMode1(const TTransitionSettings &TransitionSettings);
    int onTransitionMode2(const TTransitionSettings &TransitionSettings);
    int onTransition(const TTransitionSettings &TransitionSettings, TPQTransitionOutput *output);
    int onTransition(TPQTransitionOutput *output);         //0: not finished, 1: finished

public:
    TPQTransitionProcess();
    ~TPQTransitionProcess();

    void onInitCommon(const TPQTransitionInitParam &initParam);
    void onInitPlatform(const TPQTransitionInitParam &initParam);

    void setEnabled(bool enabled);
    bool isEnabled();
    void SetBrightToDarkStep(int step);
    void SetDarkToBrightStep(int step);

    int GetBrightToDarkStep(void);
    int GetDarkToBrightStep(void);

    int isFinished();           //0: not finished, 1: finished

    int AssignPQTransitionRegBrightToDarkTransitionSettings(int *trs_bd_param_ptr);
    int AssignPQTransitionRegDarkToBrightTransitionSettings(int *trs_db_param_ptr);

    int AssignPQTransitionNewTarget(const TPQTransitionInput &input);

    int onCalculate(const TPQTransitionInput &input, TPQTransitionOutput *output);  //0: finished, 1: not finished

    void setDebugFlag(int debugFlag);

    // Return true if success
    bool setTuningField(int field, unsigned int value);
    bool getTuningField(int field, unsigned int *value);
};

#endif
