#ifndef __CAMERA_CUSTOM_PQ_H__
#define __CAMERA_CUSTOM_PQ_H__

#include <stdint.h>
#include <string.h>

typedef enum CUSTOM_PQ_INDEX
{
//!!NOTES:
// naming rule: CUSTOM_[FEATURE-NAME]_PQ_[DESRIPTION]
    CUSTOM_PQ_DEFAULT           = 0,
    CUSTOM_PQ_NORMAL            = 1,
    // EIS part
    CUSTOM_PQ_EIS12_VIDEO_4k    = 2,
    CUSTOM_PQ_EIS30_VIDEO       = 3,
    CUSTOM_PQ_EIS30_VIDEO_4k    = 4,
    CUSTOM_PQ_EIS35_VIDEO       = 5,
    CUSTOM_PQ_EIS35_VIDEO_4k    = 6,
    CUSTOM_PQ_EIS35_NO_VIDEO    = 7,
    // SLOW MOTION part
    CUSTOM_PQ_SMVRCONSTRAINT    = 8,
    CUSTOM_PQ_SMVRBATCH         = 9,

    //!!NOTES: do not change FEATURE_PQ_NUM
    CUSTOM_PQ_MAX_NUM
} eCUSTOM_PQ_INDEX;

// --- PQ PATH ---
typedef enum CUSTOM_PQ_PATH
{
    CUSTOM_PQ_PATH_DISPLAY = 0,
    CUSTOM_PQ_PATH_RECORD  = 1,
    CUSTOM_PQ_PATH_VSS     = 2,
    CUSTOM_PQ_PATH_OTHER,
} eCUSTOM_PQ_PATH;

#define CUSTOM_PQ_PATH_ENABLE_DISPLAY(x)              (x)|=(1<<CUSTOM_PQ_PATH_DISPLAY)
#define CUSTOM_PQ_PATH_IS_DISPLAY_ENABLED(x)          ((x& (1<<CUSTOM_PQ_PATH_DISPLAY))?true:false)

#define CUSTOM_PQ_PATH_ENABLE_RECORD(x)               (x)|=(1<<CUSTOM_PQ_PATH_RECORD)
#define CUSTOM_PQ_PATH_IS_RECORD_ENABLED(x)           ((x& (1<<CUSTOM_PQ_PATH_RECORD))?true:false)

#define CUSTOM_PQ_PATH_ENABLE_VSS(x)                  (x)|=(1<<CUSTOM_PQ_PATH_VSS)
#define CUSTOM_PQ_PATH_IS_VSS_ENABLED(x)              ((x& (1<<CUSTOM_PQ_PATH_VSS))?true:false)

typedef struct CustomPQCtrl
{
    // ref: CUSTOM_PQ_PATH: DISPLAY=0, RECORD=1, VSS=2
    uint32_t czEnableMask  = 0;
    uint32_t dreEnableMask = 0;
    uint32_t hfgEnableMask = 0;
} CustomPQCtrl;

class CustomPQBase;
extern CustomPQBase* getCustomPqImpl() __attribute__((weak));

class CustomPQBase
{
public:
    static CustomPQBase* getImpl()
    {
        static CustomPQBase sDefault;
        return (getCustomPqImpl != (CustomPQBase*(*)(void))0) ? getCustomPqImpl() : &sDefault;
    }

    CustomPQBase()
    {
        configAllFeaturePQPath();
    }
    virtual ~CustomPQBase() {}

public: // utility function
    static char *PQIdx2String(uint32_t pqIdx)
    {
        if (pqIdx == CUSTOM_PQ_DEFAULT)
        {
            return "pq_default";
        }
        else if (pqIdx == CUSTOM_PQ_NORMAL)
        {
            return "pq_normal";
        }
        else if (pqIdx == CUSTOM_PQ_EIS12_VIDEO_4k)
        {
            return "pq_eis12_v4k";
        }
        else if (pqIdx == CUSTOM_PQ_EIS30_VIDEO)
        {
            return "pq_eis30_v";
        }
        else if (pqIdx == CUSTOM_PQ_EIS30_VIDEO_4k)
        {
            return "pq_eis30_v4k";
        }
        else if (pqIdx == CUSTOM_PQ_EIS35_VIDEO)
        {
            return "pq_eis35_v";
        }
        else if (pqIdx == CUSTOM_PQ_EIS35_VIDEO_4k)
        {
            return "pq_eis35_v4k";
        }
        else if (pqIdx == CUSTOM_PQ_EIS35_NO_VIDEO)
        {
            return "pq_eis35_noV";
        }
        else if (pqIdx == CUSTOM_PQ_SMVRCONSTRAINT)
        {
            return "pq_smvrC";
        }
        else if (pqIdx == CUSTOM_PQ_SMVRBATCH)
        {
            return "pq_smvrB";
        }
        else
        {
            return "pq_unknown";
        }
    }
public: // virtual functions
    virtual const CustomPQCtrl* queryFeaturePQTablePtr()
    {
        return mCustomPQCtrlTable;
    }

    virtual void configAllFeaturePQPath()
    {
        decidePQEnable_Default(CUSTOM_PQ_DEFAULT);
        decidePQEnable_Normal(CUSTOM_PQ_NORMAL);
        decidePQEnable_EIS12_VIDEO_4k(CUSTOM_PQ_EIS12_VIDEO_4k);
        decidePQEnable_EIS30_VIDEO(CUSTOM_PQ_EIS30_VIDEO);
        decidePQEnable_EIS30_VIDEO_4k(CUSTOM_PQ_EIS30_VIDEO_4k);
        decidePQEnable_EIS35_VIDEO(CUSTOM_PQ_EIS35_VIDEO);
        decidePQEnable_EIS35_VIDEO_4k(CUSTOM_PQ_EIS35_VIDEO_4k);
        decidePQEnable_EIS35_NO_VIDEO(CUSTOM_PQ_EIS35_NO_VIDEO);
        decidePQEnable_SMVRConstraint(CUSTOM_PQ_SMVRCONSTRAINT);
        decidePQEnable_SMVRBatch(CUSTOM_PQ_SMVRBATCH);
        return;
    }

    // === functions to replace on each platform: START ===
    virtual void decidePQEnable_Default(uint32_t idx)
    {
        return;
    }

    virtual void decidePQEnable_Normal(uint32_t idx)
    {
        // CZ:  display
        CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].czEnableMask);

        // DRE: display, record
        CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].dreEnableMask);
        CUSTOM_PQ_PATH_ENABLE_RECORD(mCustomPQCtrlTable[idx].dreEnableMask);
        CUSTOM_PQ_PATH_ENABLE_VSS(mCustomPQCtrlTable[idx].dreEnableMask);

        // HFG: display
        CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].hfgEnableMask);

        return;
    }

    // EIS part
    virtual void decidePQEnable_EIS12_VIDEO_4k(uint32_t idx)
    {
        return;
    }

    virtual void decidePQEnable_EIS30_VIDEO(uint32_t idx)
    {
        return;
    }
    virtual void decidePQEnable_EIS30_VIDEO_4k(uint32_t idx)
    {
        return;
    }

    virtual void decidePQEnable_EIS35_VIDEO(uint32_t idx)
    {
        return;
    }

    virtual void decidePQEnable_EIS35_VIDEO_4k(uint32_t idx)
    {
        return;
    }
    virtual void decidePQEnable_EIS35_NO_VIDEO(uint32_t idx)
    {
        return;
    }

    // SLOW MOTION part
    virtual void decidePQEnable_SMVRConstraint(uint32_t idx)
    {
        return;
    }

    virtual void decidePQEnable_SMVRBatch(uint32_t idx)
    {
        return;
    }

    virtual void decidePQEnable_SMVRBatch_3DNR(uint32_t idx)
    {
        return;
    }
    // === functions to replace on each platform: END ===

// member:
public:
    CustomPQCtrl mCustomPQCtrlTable[CUSTOM_PQ_MAX_NUM];
};

#endif /* __CAMERA_CUSTOM_PQ_H__ */

