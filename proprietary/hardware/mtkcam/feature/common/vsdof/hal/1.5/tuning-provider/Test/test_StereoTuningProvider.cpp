#include <limits.h>
#include <gtest/gtest.h>
#include <mtkcam/feature/stereo/hal/stereo_tuning_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>

#define PROPERTY_TUNING_PREFIX  STEREO_PROPERTY_PREFIX"tuning."
#define PROPERTY_DPE_TUNING     PROPERTY_TUNING_PREFIX"dpe"     //debug.STEREO.tuning.dpe, 1 to read/write tuning file
#define PROPERTY_WMF_TUNING     PROPERTY_TUNING_PREFIX"wmf"     //debug.STEREO.tuning.wmf,  1 to read/write tuning file
#define PROPERTY_GF_TUNING      PROPERTY_TUNING_PREFIX"gf"      //debug.STEREO.tuning.gf,   value of 0, 1, 2
#define PROPERTY_BOKEH_TUNING   PROPERTY_TUNING_PREFIX"bokeh"   //debug.STEREO.tuning.bokeh, 1 to read/write tuning file

#define DPE_TUNING_FILE_PATH    "/sdcard/tuning/dpe_params"    //0: Not to read tuning file
#define WMF_TUNING_FILE_PATH    "/sdcard/tuning/wmf_params"
#define BOKEH_TUNING_FILE_PATH  "/sdcard/tuning/bokeh_params"

//=============================================================================
//  DPE
//=============================================================================
TEST(TUNING_TEST, DPE_TEST)
{
    //Get default
    property_set(PROPERTY_DPE_TUNING, "0");
    DVEConfig tuningBuffer;
    EXPECT_TRUE(StereoTuningProvider::getDPETuningInfo(&tuningBuffer));

    //Write to file
    remove(DPE_TUNING_FILE_PATH);
    property_set(PROPERTY_DPE_TUNING, "1");
    EXPECT_TRUE(StereoTuningProvider::getDPETuningInfo(&tuningBuffer));

    //Read from file
    DVEConfig tuningBuffer2;
    EXPECT_TRUE(StereoTuningProvider::getDPETuningInfo(&tuningBuffer2));
}
//=============================================================================
//  WMF
//=============================================================================
TEST(TUNING_TEST, WMF_TEST)
{
    WMFEFILTERSIZE size;
    short *tbliBuffer = new short[128];

    //Get default
    property_set(PROPERTY_WMF_TUNING, "0");
    EXPECT_TRUE(StereoTuningProvider::getWMFTuningInfo(size, tbliBuffer));
    EXPECT_TRUE((size == WMFE_FILTER_SIZE_7x7));

    //Write to file
    remove(WMF_TUNING_FILE_PATH);
    property_set(PROPERTY_WMF_TUNING, "1");
    EXPECT_TRUE(StereoTuningProvider::getWMFTuningInfo(size, tbliBuffer));

    //Read from file
    EXPECT_TRUE(StereoTuningProvider::getWMFTuningInfo(size, tbliBuffer));

    delete [] tbliBuffer;
}
//=============================================================================
//  BOKEH
//=============================================================================
TEST(TUNING_TEST, BOKEH_TEST)
{
    dip_x_reg_t tuningBuffer;

    //Get default
    property_set(PROPERTY_BOKEH_TUNING, "0");
    EXPECT_TRUE(StereoTuningProvider::getBokehTuningInfo((void *)&tuningBuffer));

    //Write to file
    remove(BOKEH_TUNING_FILE_PATH);
    property_set(PROPERTY_BOKEH_TUNING, "1");
    EXPECT_TRUE(StereoTuningProvider::getBokehTuningInfo((void *)&tuningBuffer));

    //Read from file
    EXPECT_TRUE(StereoTuningProvider::getBokehTuningInfo((void *)&tuningBuffer));
}