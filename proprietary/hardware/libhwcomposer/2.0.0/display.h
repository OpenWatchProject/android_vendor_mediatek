#ifndef HWC_DISPLAY_H_
#define HWC_DISPLAY_H_

#include <hardware/hwcomposer2.h>

#include <ui/Rect.h>
#include <utils/Singleton.h>
#include "hwc_priv.h"

#include "event.h"
#include "ged/ged.h"

#define FAKE_DISPLAY -30

using namespace android;

struct dump_buff;
class SessionInfo;

// ---------------------------------------------------------------------------

struct DisplayData
{
    int32_t width;
    int32_t height;
    uint32_t format;
    float xdpi;
    float ydpi;
    int density;
    nsecs_t refresh;
    bool has_vsync;
    bool connected;
    bool secure;

    // hwrotation is physical display rotation
    uint32_t hwrotation;

    // pixels is the area of the display
    uint32_t pixels;

    int subtype;

    float aspect_portrait;
    float aspect_landscape;
    Rect mir_portrait;
    Rect mir_landscape;

    int vsync_source;

    bool is_s3d_support;

    // DispatchThread starts jobs when HWC receives VSync
    bool trigger_by_vsync;

    // supporting hdcp version = minimum(dongle, phone sw)
    uint32_t hdcp_version;
};

class DisplayManager : public Singleton<DisplayManager>
{
public:
    DisplayManager();
    ~DisplayManager();

    enum
    {
        MAX_DISPLAYS = HWC_NUM_DISPLAY_TYPES,
    };

    enum DISP_QUERY_TYPE
    {
        DISP_CURRENT_NUM  = 0x00000001,
    };

    // query() is used for client to get capability
    int query(int what, int* value);

    // dump() for debug prupose
    void dump(struct dump_buff* log);

    // init() is used to initialize DisplayManager
    void init();

    void resentCallback();

    struct EventListener : public virtual RefBase
    {
        // onVSync() is called to notify vsync signal
        virtual void onVSync(int dpy, nsecs_t timestamp, bool enabled) = 0;

        // onPlugIn() is called to notify a display is plugged
        virtual void onPlugIn(int dpy) = 0;

        // onPlugOut() is called to notify a display is unplugged
        virtual void onPlugOut(int dpy) = 0;

        // onHotPlug() is called to notify external display hotplug event
        virtual void onHotPlugExt(int dpy, int connected) = 0;

        // onRefresh() is called to notify a display to refresh
        virtual void onRefresh(int dpy) = 0;

        virtual void onRefresh(int dpy, unsigned int type) = 0;
    };

    // setListener() is used for client to register listener to get event
    void setListener(const sp<EventListener>& listener);

    inline sp<EventListener> getListener() const {return m_listener;}
    // requestVSync() is used for client to request vsync signal
    void requestVSync(int dpy, bool enabled);

    // requestNextVSync() is used by HWCDispatcher to request next vsync
    void requestNextVSync(int dpy);

    // vsync() is callback by vsync thread
    void vsync(int dpy, nsecs_t timestamp, bool enabled);

    // hotplugExt() is called to insert or remove extrenal display
    void hotplugExt(int dpy, bool connected, bool fake = false, bool notify = true);

    void hotplugExtOut();

    // hotplugVir() is called to insert or remove virtual display
    void hotplugVir(
        const int& dpy, const bool& connected, const uint32_t& width,
        const uint32_t& height, const int32_t& format);

    // refresh() is called to refresh display
    void refresh(int dpy);

    // for display self refresh
    void refresh(unsigned int type);

    // setDisplayData() is called to init display data in DisplayManager
    void setDisplayDataForPhy(int dpy, buffer_handle_t outbuf = NULL);
    void setDisplayDataForVir(const int& dpy, const uint32_t& width, const uint32_t& height, const int32_t& format);

    // setPowerMode() notifies which display's power mode
    void setPowerMode(int dpy, int mode);

    // getFakeDispNum() gets amount of fake external displays
    int getFakeDispNum() { return m_fake_disp_num; }

    // isUltraDisplay() checks specific display whether it is a 4k display
    bool isUltraDisplay(int dpy);

    // isAllDisplayOff() checks all displays whether they are in the suspend mode
    bool isAllDisplayOff();

    // setDisplayPowerState() sets the power state of specific display
    void setDisplayPowerState(int dpy, int state);
    int getDisplayPowerState(const int& dpy);

    enum
    {
        WAKELOCK_TIMER_PAUSE,
        WAKELOCK_TIMER_START,
        WAKELOCK_TIMER_PLAYOFF,
    };

    // setWakelockTimerState() used to control the watch dog of wakelock
    void setWakelockTimerState(int state);

    // m_data is the detailed information for each display device
    const DisplayData* getDisplayData(int dpy, hwc2_config_t config = 0);

    // m_profile_level is used for profiling purpose
    static int m_profile_level;

    // accesor of m_video_hdcp
    uint32_t getVideoHdcp() const;
    void setVideoHdcp(const uint32_t&);

    bool checkIsWfd();

    int32_t getSupportedColorMode(int dpy);

    void getSupportedColorModeVector(int dpy,  Vector<int32_t>& color_modes);
private:
    enum
    {
        DISP_PLUG_NONE       = 0,
        DISP_PLUG_CONNECT    = 1,
        DISP_PLUG_DISCONNECT = 2,
    };

    // setMirrorRect() is used to calculate valid region for mirror mode
    void setMirrorRegion(int dpy);

    // hotplugPost() is used to do post jobs after insert/remove display
    void hotplugPost(int dpy, bool connected, int state, bool print_info = true);

    // createVsyncThread() is used to create vsync thread
    void createVsyncThread(int dpy);

    // destroyVsyncThread() is used to destroy vsync thread
    void destroyVsyncThread(int dpy);

    // printDisplayInfo() used to print out display data
    void printDisplayInfo(int dpy);

    // compareDisplaySize() used to compare display size with primary display
    void compareDisplaySize(int dpy);

    // checkSecondBuildInPanel() used to check whether secondary display exist
    void checkSecondBuildInPanel(void);

    // getPhysicalPanelSize() used to get the correct physical size of panel
    void getPhysicalPanelSize(unsigned int *width, unsigned int *height, SessionInfo &info);

    // m_data is the detailed information for each display device
    DisplayData* m_data;

    // m_data_primary_extra is the detailed information for primary display extra config
    DisplayData* m_data_primary_extra;

    // m_curr_disp_num is current amount of displays
    unsigned int m_curr_disp_num;

    // m_fake_disp_num is amount of fake external displays
    unsigned int m_fake_disp_num;

    // m_listener is used for listen vsync event
    sp<EventListener> m_listener;

    mutable Mutex m_power_lock;

    // a wrapper class of VSyncThread
    struct HwcVSyncSource
    {
        HwcVSyncSource()
            :thread(NULL)
        {}

        // a thread to receive VSync from display driver
        sp<VSyncThread> thread;

        // Because device plug-out and vsync requests happened on different threads
        // add a lock to resolve the race condition
        mutable Mutex lock;
    };

    // a list of HwcVSyncSource
    // Each item is responsible for vsync receiving of each display device
    HwcVSyncSource m_vsyncs[DisplayManager::MAX_DISPLAYS];

    // for display state lock
    mutable Mutex m_state_lock;

    // this flag is set when the display size is more than primary display
    bool m_state_ultra_display[MAX_DISPLAYS];

    // m_display_power_state is used to stored power state of all display
    bool m_display_power_state[MAX_DISPLAYS];

    // hdcp version required by video provider
    mutable RWLock m_video_hdcp_rwlock;
    uint32_t m_video_hdcp;

    GED_LOG_HANDLE m_ged_log_handle;
};

#endif // HWC_DISPLAY_H_
