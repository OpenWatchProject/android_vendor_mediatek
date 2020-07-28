#include <camera_custom_pq.h>

class CustomPQImpl: public CustomPQBase
{
public:

    CustomPQImpl()
    {
        configAllFeaturePQPath();
    }
    virtual ~CustomPQImpl() {}

    virtual void decidePQEnable_Normal(uint32_t idx);
    virtual void decidePQEnable_EIS12_VIDEO_4k(uint32_t idx);
    virtual void decidePQEnable_EIS30_VIDEO(uint32_t idx);
    virtual void decidePQEnable_EIS30_VIDEO_4k(uint32_t idx);
    virtual void decidePQEnable_EIS35_VIDEO(uint32_t idx);
    virtual void decidePQEnable_EIS35_VIDEO_4k(uint32_t idx);
    virtual void decidePQEnable_EIS35_NO_VIDEO(uint32_t idx);
};

/* Define for getHelper function used */
CustomPQBase* getCustomPqImpl()
{
    static CustomPQImpl sInst;
    return &sInst;
}

// === functions to replace on each platform: START ===
void CustomPQImpl::decidePQEnable_Normal(uint32_t idx)
{
    // CZ:  display
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].czEnableMask);

    // DRE: display, record
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].dreEnableMask);
    CUSTOM_PQ_PATH_ENABLE_RECORD(mCustomPQCtrlTable[idx].dreEnableMask);

    // HFG: display
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].hfgEnableMask);

    return;
}

void CustomPQImpl::decidePQEnable_EIS12_VIDEO_4k(uint32_t idx)
{
    // CZ:  display
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].czEnableMask);
    // DRE: none
    // HFG: display
    //CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].hfgEnableMask);

    return;
}

// FEATURE_PQ_EIS_30
void CustomPQImpl::decidePQEnable_EIS30_VIDEO(uint32_t idx)
{
    // CZ:  display
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].czEnableMask);
    // DRE: record
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].dreEnableMask);
    // HFG: display
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].hfgEnableMask);

    return;
}

void CustomPQImpl::decidePQEnable_EIS30_VIDEO_4k(uint32_t idx)
{
    // CZ:  display
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].czEnableMask);
    // DRE: none
    // HFG: display
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].hfgEnableMask);

    return;
}

void CustomPQImpl::decidePQEnable_EIS35_VIDEO(uint32_t idx)
{
    // CZ:  display
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].czEnableMask);
    // DRE: display, record, no vss
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].dreEnableMask);
    CUSTOM_PQ_PATH_ENABLE_RECORD(mCustomPQCtrlTable[idx].dreEnableMask);
    // HFG: display
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].hfgEnableMask);

    return;
}

void CustomPQImpl::decidePQEnable_EIS35_VIDEO_4k(uint32_t idx)
{
    // CZ:  display
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].czEnableMask);
    // DRE: none
    // HFG: display
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].hfgEnableMask);

    return;
}

void CustomPQImpl::decidePQEnable_EIS35_NO_VIDEO(uint32_t idx)
{
    // CZ:  display
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].czEnableMask);
    // DRE: display, record, vss
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].dreEnableMask);
    CUSTOM_PQ_PATH_ENABLE_RECORD(mCustomPQCtrlTable[idx].dreEnableMask);
    CUSTOM_PQ_PATH_ENABLE_VSS(mCustomPQCtrlTable[idx].dreEnableMask);
    // HFG: display
    CUSTOM_PQ_PATH_ENABLE_DISPLAY(mCustomPQCtrlTable[idx].hfgEnableMask);

    return;
}
