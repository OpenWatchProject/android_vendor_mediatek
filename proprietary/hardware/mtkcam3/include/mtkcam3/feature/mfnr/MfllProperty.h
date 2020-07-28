#ifndef __MFLLFPROPERTY_H__
#define __MFLLFPROPERTY_H__

#include "MfllTypes.h"

#include <utils/RefBase.h>
// STL
#include <mutex>

#define MFLL_PROPERTY_ON        1
#define MFLL_PROPERTY_OFF       0
#define MFLL_PROPERTY_DEFAULT   -1

namespace mfll {
    typedef enum tagProperty {
        Property_ForceMfll = 0,
        Property_CaptureNum,
        Property_BlendNum,
        Property_DropNum,
        Property_Iso,
        Property_Exposure,
        Property_Mrp,
        Property_Rwb,
        Property_FullSizeMc,
        Property_MvBadRange,
        Property_MbBadTh,
        Property_PerfServ,
        Property_Bss,
        Property_BssOn,
        Property_BssRoiWidth,
        Property_BssRoiHeight,
        Property_BssRoiX0,
        Property_BssRoiY0,
        Property_BssScaleFactor,
        Property_BssClipTh0,
        Property_BssClipTh1,
        Property_BssZero,
        Property_BssAdfTh,
        Property_BssSdfTh,
        Property_BssFdDump,
        Property_ForceBssOrder,
        Property_Gyro,
        Property_ForceGmv,
        Property_ForceGmvZero,
        Property_ForceMmdvfsOff,
        Property_ForceDownscale,
        Property_ForceDownscaleDividend,
        Property_ForceDownscaleDivisor,
        Property_PostNrRefine,
        Property_PostMfbRefine,
        Property_AlgoThreadsPriority,
        Property_MfbP2DirectLink,

        // Feed
        Property_FeedBfbld,
        Property_FeedBss,
        Property_FeedMemc,
        Property_FeedSf,
        Property_FeedMfb,
        Property_FeedMix,
        Property_FeedJpeg,

        // Log
        Property_LogLevel,

        // Dump
        Property_DumpAll,
        Property_DumpRaw,
        Property_DumpYuv,
        Property_DumpMfb,
        Property_DumpMix,
        Property_DumpMixOut,
        Property_DumpJpeg,
        Property_DumpPostview,
        Property_DumpExif,
        Property_DumpGyro, // since MFNR v2.0
        Property_DumpSim, // Dump for simulation

        Property_Size
    } Property_t;

    const char* const PropertyString[Property_Size] = {
        "vendor.mfll.force",
        "vendor.mfll.capture_num",
        "vendor.mfll.blend_num",
        "vendor.mfll.drop_num",
        "vendor.mfll.iso",
        "vendor.mfll.exp",
        "vendor.mfll.mrp",
        "vendor.mfll.rwb",
        "vendor.mfll.full_size_mc",
        "vendor.mfll.mv_bad_range",
        "vendor.mfll.mb_bad_th",
        "vendor.mfll.perfserv",
        "vendor.mfll.bss",
        "vendor.mfll.bss_on",
        "vendor.mfll.bss_roi_width",
        "vendor.mfll.bss_roi_height",
        "vendor.mfll.bss_roi_x0",
        "vendor.mfll.bss_roi_y0",
        "vendor.mfll.bss_scaler_factor",
        "vendor.mfll.bss_clip_th0",
        "vendor.mfll.bss_clip_th1",
        "vendor.mfll.bss_zero",
        "vendor.mfll.bss_adf_th",
        "vendor.mfll.bss_sdf_th",
        "vendor.mfll.bss_fd_dump",
        "vendor.mfll.force_bss_order",
        "vendor.mfll.gyro",
        "vendor.mfll.force_gmv",
        "vendor.mfll.force_gmv_zero",
        "vendor.mfll.force_mmdvfs_off",
        "vendor.mfll.force_downscale",
        "vendor.mfll.force_downscale_dividend",
        "vendor.mfll.force_downscale_divisor",
        "vendor.mfll.post_nr_refine",
        "vendor.mfll.post_mfb_refine",
        "vendor.mfll.algo_priority",
        "vendor.mfll.mfb_p2.directlink",

        "vendor.mfll.feed.bfbld",
        "vendor.mfll.feed.bss",
        "vendor.mfll.feed.memc",
        "vendor.mfll.feed.sf",
        "vendor.mfll.feed.mfb",
        "vendor.mfll.feed.mix",
        "vendor.mfll.feed.jpeg",

        "vendor.mfll.log_level",

        "vendor.mfll.dump.all",
        "vendor.mfll.dump.raw",
        "vendor.mfll.dump.yuv",
        "vendor.mfll.dump.mfb",
        "vendor.mfll.dump.mixer",
        "vendor.mfll.dump.mixer.out",
        "vendor.mfll.dump.jpeg",
        "vendor.mfll.dump.postview",
        "vendor.mfll.dump.exif",
        "vendor.mfll.dump.gyro",
        "vendor.mfll.dump.sim"
    };

/**
 *  Property Read/Write/Wait
 *
 *  This interface provides a mechanism to read property and with these properties,
 *  some features might be for enabled or not.
 *
 *  All property will be read while creating, and after created, all property
 *  is retrieved from memory. If caller want the property from device realtime,
 *  invoke IMfllProperty::readProperty to get it.
 *
 *  @note This is a thread-safe class
 */
class MfllProperty : public android::RefBase {
public:
    MfllProperty(void);
    virtual ~MfllProperty(void);

/* interface */
public:
    /* To read property from device directly*/
    static int readProperty(const Property_t &t);

    /* To read property from device directly with a default value */
    static int readProperty(const Property_t &t, int defaultVal);

    /* To check if force on MFNR */
    static int isForceMfll(void);

    /* Check if force full size MC, -1 is not to set, use default */
    static int getFullSizeMc(void);

    /* To get frame capture number */
    static int getCaptureNum(void);

    /* To get frame blend number */
    static int getBlendNum(void);

    /* To get frame drop number */
    static int getDropNum(void);

    /* To get force exposure */
    static int getExposure(void);

    /* To get force iso */
    static int getIso(void);

    /* To get if disable BSS (returns 0 for disable) */
    static int getBss(void);

    /* To get if force BSS order */
    static bool getForceBssOrder(std::vector<int> &bssOrder);

    /* To get if force GMV to zero */
    static int getForceGmvZero(void);

    /* To force GMV as manual setting */
    static bool getForceGmv(MfllMotionVector_t (&globalMv)[MFLL_MAX_FRAMES]);

    /* To get if force MMDVFS to be off */
    static int getForceMmdvfsOff(void);

    /* To get MfllCore debug log level */
    static int getDebugLevel(void);

    /* To get MfllCore mfb p2 directlink support or not*/
    static int getMfbP2DirectLink(void);

/**
 *  Dump information will be available after MFNR core has been inited
 */
public:
    inline bool isDumpRaw(void)
    { return m_propValue[Property_DumpRaw] == MFLL_PROPERTY_ON ? true : false; }

    inline bool isDumpYuv(void)
    { return m_propValue[Property_DumpYuv] == MFLL_PROPERTY_ON ? true : false; }

    inline bool isDumpMfb(void)
    { return m_propValue[Property_DumpMfb] == MFLL_PROPERTY_ON ? true : false; }

    inline bool isDumpMix(void)
    { return m_propValue[Property_DumpMix] == MFLL_PROPERTY_ON ? true : false; }

    inline bool isDumpMixOut(void)
    { return m_propValue[Property_DumpMixOut] == MFLL_PROPERTY_ON ? true : false; }

    inline bool isDumpJpeg(void)
    { return m_propValue[Property_DumpJpeg] == MFLL_PROPERTY_ON ? true : false; }

    inline bool isDumpPostview(void)
    { return m_propValue[Property_DumpPostview] == MFLL_PROPERTY_ON ? true : false; }

    inline bool isDumpExif(void)
    { return m_propValue[Property_DumpExif] == MFLL_PROPERTY_ON ? true : false; }

/**
 *  Feed information will be available after MFNR core has been inited
 */
public:
    inline bool isFeedBfbld(void)
    { return m_propValue[Property_FeedBfbld] == MFLL_PROPERTY_ON ? true : false; }

    inline bool isFeedBss(void)
    { return m_propValue[Property_FeedBss] == MFLL_PROPERTY_ON ? true : false; }

    inline bool isFeedMemc(void)
    { return m_propValue[Property_FeedMemc] == MFLL_PROPERTY_ON ? true : false; }

    inline bool isFeedSf(void)
    { return m_propValue[Property_FeedSf]  == MFLL_PROPERTY_ON ? true : false; }

    inline bool isFeedMfb(void)
    { return m_propValue[Property_FeedMfb] == MFLL_PROPERTY_ON ? true : false; }

    inline bool isFeedMix(void)
    { return m_propValue[Property_FeedMix] == MFLL_PROPERTY_ON ? true : false; }

    inline bool isFeedJpeg(void)
    { return m_propValue[Property_FeedJpeg] == MFLL_PROPERTY_ON ? true : false; }

public:
    /**
     *  To check if any dump property is specified
     *  @return             Ture for yes.
     */
    bool isDump(void);

    /**
     *  To check if any feed property is specified
     *  @return             True for yes.
     */
    bool isFeed(void);

    /**
     *  Get the cached value of the specified property
     *  @param t            Propery to get
     */
    int getProperty(const Property_t &t);

    /**
     *  Set the value to the specified property to device and cached memory
     *  @param t            Propery to set
     *  @param v            Value to set
     */
    void setProperty(const Property_t &t , const int &v);

private:
    std::mutex m_mutex; // for operation thread-safe
    int m_propValue[Property_Size]; // saves status of this property

}; /* class MfllProperty */
}; /* namespace mfll */
#endif//__MFLLFPROPERTY_H__
