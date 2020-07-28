#ifndef __MFLLFEATUREDUMP_H__
#define __MFLLFEATUREDUMP_H__

#include "IMfllCore.h"
#include "IMfllEventListener.h"

#include <string>

#define MFLL_DUMP_DEFAUL_PATH "/data/vendor/camera_dump/"

#define MFLL_DUMP_PATTERN_UNIQUEKEY         "{UNIQUEKEY}"
#define MFLL_DUMP_PATTERN_FRAMENUM          "{FRAMENO}"
#define MFLL_DUMP_PATTERN_REQUESTNUM        "{REQUESTNO}"
#define MFLL_DUMP_PATTERN_ISO               "{ISO}"
#define MFLL_DUMP_PATTERN_EXP               "{EXP}"
#define MFLL_DUMP_PATTERN_STAGE             "{STAGE}"
#define MFLL_DUMP_PATTERN_BUFFERNAME        "{BUFNAME}"
#define MFLL_DUMP_PATTERN_WIDTH             "{WIDTH}"
#define MFLL_DUMP_PATTERN_HEIGHT            "{HEIGHT}"
#define MFLL_DUMP_PATTERN_BAYERORDER        "{BAYERORDER}"
#define MFLL_DUMP_PATTERN_BITS              "{BITS}"
#define MFLL_DUMP_PATTERN_EXTENSION         "{EXT}"

#define MFLL_DUMP_FILE_NAME_RAW \
    "{UNIQUEKEY}-{FRAMENO}-{REQUESTNO}-mfll-iso-{ISO}-exp-{EXP}-{STAGE}-{BUFNAME}__{WIDTH}x{HEIGHT}_{BITS}_{BAYERORDER}.{EXT}"

#define MFLL_DUMP_FILE_NAME_OTHER \
    "{UNIQUEKEY}-{FRAMENO}-{REQUESTNO}-mfll-iso-{ISO}-exp-{EXP}-{STAGE}-{BUFNAME}__{WIDTH}x{HEIGHT}.{EXT}"


namespace mfll {

/**
 *  Dump Mechanism
 *
 *  This plug-in provides a mechanism to dump frame data for every stage.
 */
class MfllFeatureDump : public IMfllEventListener {
public:
    typedef struct _DumpFlag {
        int raw;
        int yuv;
        int mfb;
        int mix;
        int jpeg;
        int postview;
        int exif;
    } DumpFlag;

    typedef enum tagDumpStage {
        DumpStage_Capture = 0,
        DumpStage_Bss,
        DumpStage_Base,
        DumpStage_Golden,
        DumpStage_Memc,
        DumpStage_Mfb,
        DumpStage_Mix,
        DumpStage_Rwb,
        DumpStage_Jpeg,
        DumpStage_Postview,
        /* size */
        DumpStage_Size
    } DumpStage;

public:
    MfllFeatureDump(void);

protected:
    virtual ~MfllFeatureDump(void);

/* implementation */
public:
    virtual void onEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1 = NULL, void *param2 = NULL);
    virtual void doneEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1 = NULL, void *param2 = NULL);
    virtual vector<enum EventType> getListenedEventTypes(void);

/* interface */
public:
    inline void setFilePath(const std::string &s)
    { m_filepath = s; }
    inline void setDumpFlag(const DumpFlag &flags)
    { m_dumpFlag = flags; }
    std::string getFilePath(void)
    { return m_filepath; }

private:
    DumpFlag m_dumpFlag;
    std::string m_filepath;

    void dump_image(IMfllImageBuffer *pImg, const DumpStage &stage, const char *name, int number = 0);
    void dumpCapture(IMfllCore *pCore);
    void dumpBaseStage(IMfllCore *pCore);
    void dumpMfbStage(IMfllCore *pCore, int index);
    void dumpMixStage(IMfllCore *pCore);

}; /* class MfllFeatureDump */
}; /* namespace mfll */
#endif//__MFLLFEATUREDUMP_H__

