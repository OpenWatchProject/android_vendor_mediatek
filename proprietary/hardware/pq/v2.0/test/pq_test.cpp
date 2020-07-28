#define LOG_TAG "PQ-Test"

#include <cstdlib>
#include <iostream>
#include <log/log.h>
#include <cutils/properties.h>
#include <cust_color.h>
#include <PQCommon.h>
#include "cust_tdshp.h"
#include <PQDSImpl.h>

#include <PQPictureMode.h>
#include <vendor/mediatek/hardware/pq/2.0/IPictureQuality.h>

using vendor::mediatek::hardware::pq::V2_0::IPictureQuality;
using vendor::mediatek::hardware::pq::V2_0::Result;
using vendor::mediatek::hardware::pq::V2_0::PQFeatureID;
using vendor::mediatek::hardware::pq::V2_0::dispPQIndexParams;

using namespace std;
using namespace android;

#define PQ_LOGD(fmt, arg...) ALOGD(fmt, ##arg)
#define PQ_LOGE(fmt, arg...) ALOGE(fmt, ##arg)

#define CHECK(stat) \
    do { \
        try \
        { \
            if (!(stat)) \
            { \
                cerr << __func__ << ":" << __LINE__ << endl; \
                cerr << "    " << #stat << " failed." << endl; \
            } \
        } \
        catch(...) \
        { \
            cerr << "Exception caught." << endl; \
        } \
    } while (0)

#define CASE_DONE() //do { cout << __func__ << " done." << endl; } while (0)


#define CLR_PARTIAL_Y_SIZE 16
#define CLR_PQ_PARTIALS_CONTROL 5
#define CLR_PURP_TONE_SIZE 3
#define CLR_SKIN_TONE_SIZE 8
#define CLR_GRASS_TONE_SIZE 6
#define CLR_SKY_TONE_SIZE 3

#define OUT_OF_UPPERBOUND_ONE 0

int32_t getPqparam_table(DISP_PQ_PARAM *pqparam_table)
{
    DISP_PQ_PARAM *pq_table_ptr;

    /* open the needed object */
    CustParameters &cust = CustParameters::getPQCust();
    if (!cust.isGood()) {
        PQ_LOGD("[PQ_SERVICE] can't open libpq_cust.so\n");
        return 1;
    }

    /* find the address of function and data objects */
    pq_table_ptr = (DISP_PQ_PARAM *)cust.getSymbol("pqparam_table");
    if (!pq_table_ptr) {
        PQ_LOGD("[PQ_SERVICE] pqparam_table is not found in libpq_cust.so\n");
        return 1;
    }

    memcpy(pqparam_table, pq_table_ptr, sizeof(DISP_PQ_PARAM) * PQ_PARAM_TABLE_SIZE);

    return 0;
}

int32_t getPqparam_mapping(DISP_PQ_MAPPING_PARAM *pqparam_mapping)
{
    DISP_PQ_MAPPING_PARAM *pq_mapping_ptr;

    /* open the needed object */
    CustParameters &cust = CustParameters::getPQCust();
    if (!cust.isGood()) {
        PQ_LOGD("[PQ_SERVICE] can't open libpq_cust.so\n");
        return 1;
    }
    /* find the address of function and data objects */

    pq_mapping_ptr = (DISP_PQ_MAPPING_PARAM *)cust.getSymbol("pqparam_mapping");
    if (!pq_mapping_ptr) {
        PQ_LOGD("[PQ_SERVICE] pqparam_mapping is not found in libpq_cust.so\n");
        return 1;
    }
    memcpy(pqparam_mapping, pq_mapping_ptr, sizeof(DISP_PQ_MAPPING_PARAM));

    return 0;
}

int32_t getPQStrengthPercentage(int PQScenario, DISP_PQ_MAPPING_PARAM *pqparam_mapping)
{
    if (PQScenario == SCENARIO_PICTURE) {
        return pqparam_mapping->image;
    }
    else  if (PQScenario == SCENARIO_VIDEO) {
        return pqparam_mapping->video;
    }
    else  if (PQScenario == SCENARIO_ISP_PREVIEW) {
        return pqparam_mapping->camera;
    }
    else {
        PQ_LOGD("[PQ_SERVICE] PQService : getMappedTDSHPIndex, invalid scenario\n");
        return pqparam_mapping->image;
    }
}

void calcPQStrength(DISP_PQ_PARAM *pqparam_dst, DISP_PQ_PARAM *pqparam_src, int percentage)
{
    memcpy(pqparam_dst, pqparam_src, sizeof(DISP_PQ_PARAM));

    pqparam_dst->u4SatGain = pqparam_src->u4SatGain * percentage / 100;
    pqparam_dst->u4SatAdj[1] = pqparam_src->u4SatAdj[1] * percentage / 100;
    pqparam_dst->u4SatAdj[2] = pqparam_src->u4SatAdj[2] * percentage / 100;
    pqparam_dst->u4SatAdj[3] = pqparam_src->u4SatAdj[3] * percentage / 100;
    pqparam_dst->u4Contrast = pqparam_src->u4Contrast * percentage / 100;
    pqparam_dst->u4Brightness = pqparam_src->u4Brightness * percentage / 100;
    pqparam_dst->u4SHPGain = pqparam_src->u4SHPGain * percentage / 100;
}

void LoadUserModePQParam(DISP_PQ_PARAM *pqparam)
{
    char value[PROPERTY_VALUE_MAX];
    int i;

    property_get(PQ_TDSHP_PROPERTY_STR, value, PQ_TDSHP_INDEX_DEFAULT);
    i = atoi(value);
    PQ_LOGD("[PQ_SERVICE] property get... tdshp[%d]", i);
    pqparam->u4SHPGain = i;

    property_get(PQ_GSAT_PROPERTY_STR, value, PQ_GSAT_INDEX_DEFAULT);
    i = atoi(value);
    PQ_LOGD("[PQ_SERVICE] property get... gsat[%d]", i);
    pqparam->u4SatGain = i;

    property_get(PQ_CONTRAST_PROPERTY_STR, value, PQ_CONTRAST_INDEX_DEFAULT);
    i = atoi(value);
    PQ_LOGD("[PQ_SERVICE] property get... contrast[%d]", i);
    pqparam->u4Contrast = i;

    property_get(PQ_PIC_BRIGHT_PROPERTY_STR, value, PQ_PIC_BRIGHT_INDEX_DEFAULT);
    i = atoi(value);
    PQ_LOGD("[PQ_SERVICE] property get... pic bright[%d]", i);
    pqparam->u4Brightness = i;
}

void test_BluLightTuning()
{
    struct ColorRegistersTuning
    {
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
    #define RegOffset(field) (unsigned int)((char*)&( ((ColorRegistersTuning*)(0))->field) - (char*)(0))

    Result ret;
    int32_t value;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return;
    }

    service->enableBlueLight(true, PQ_DEFAULT_TRANSITION_ON_STEP);

    // Reading mode on/off
    ret = service->setTuningField(MOD_BLUE_LIGHT_INPUT, 0xffff, 1);
    CHECK(ret == Result::OK);
    service->getTuningField(MOD_BLUE_LIGHT_INPUT, 0xffff,
        [&] (Result retval, int32_t retValue) {
        if (retval == Result::OK) {
            value = retValue;
        }
        ret = retval;
    });
    CHECK(ret == Result::OK);
    CHECK(value == 1);

    // Now it is in reading mode, we verify the reading functionality
    service->setTuningField(MOD_DISPLAY, 0x0, 0);
    // Read out CCORR_COEF
    static const size_t ccorrBase = RegOffset(CCORR_COEF);
    for (size_t i = 0; i < 9; i++) {
        service->getTuningField(MOD_BLUE_LIGHT_INPUT, ccorrBase + 4 * i,
            [&] (Result retval, int32_t retValue) {
            if (retval == Result::OK) {
                value = retValue;
            }
        });
        cout << value << " ";
    }
    cout << endl;

    // Test input overwritten, we should observe on display panel
    static const int CCORR[9] = { 512, 0, 0,   0, 1023, 0,   0, 0, 1023 };
    for (size_t i = 0; i < 9; i++) {
        service->setTuningField(MOD_BLUE_LIGHT_INPUT, ccorrBase + 4 * i, CCORR[i]);
    }
    service->setTuningField(MOD_DISPLAY, 0x0, 0);

    // Error handing: access non-aligned address
    service->getTuningField(MOD_BLUE_LIGHT_INPUT, 0x3,
        [&] (Result retval, int32_t retValue) {
        if (retval == Result::OK) {
            value = retValue;
        }
        ret = retval;
    });
    CHECK(ret != Result::OK);
    ret = service->setTuningField(MOD_BLUE_LIGHT_INPUT, 0x5, 0);
    CHECK(ret != Result::OK);

    // Error handing: invalid mode
    service->setTuningField(MOD_BLUE_LIGHT_INPUT, 0xffff, 1);
    ret = service->setTuningField(MOD_BLUE_LIGHT_INPUT, 0xffff, 10);
    CHECK(ret != Result::OK);
    service->getTuningField(MOD_BLUE_LIGHT_INPUT, 0xffff,
        [&] (Result retval, int32_t retValue) {
        if (retval == Result::OK) {
            value = retValue;
        }
        ret = retval;
    });
    CHECK(value == 1); // Should keep previous mode

    // Error handing: write in reading mode should be forbidden
    service->setTuningField(MOD_BLUE_LIGHT_INPUT, 0xffff, 1);
    ret = service->setTuningField(MOD_BLUE_LIGHT_INPUT, RegOffset(GLOBAL_SAT), 512);
    CHECK(ret != Result::OK);

    #undef RegOffset

    CASE_DONE();
}

void test_CustParameters()
{
    CustParameters &cust = CustParameters::getPQCust();
    int var = 0;

    if (cust.isGood()) {
        CHECK(cust.loadVar("BaseOnly", &var));
        CHECK(var == 1111);

        CHECK(cust.loadVar("ProjectOnly", &var));
        CHECK(var == 2222);

        CHECK(cust.loadVar("ProjectOverwritten", &var));
        CHECK(var == 2222);
    } else {
        cerr << "Load libpq_cust*.so failed." << endl;
    }

    CASE_DONE();
}

void test_setPQMode(int32_t mode)
{
    Result ret;
    char value[PROPERTY_VALUE_MAX];
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return;
    }

    cout << "mode =" << mode  << endl;
    ret = service->setPQMode(mode, PQ_DEFAULT_TRANSITION_OFF_STEP);
    CHECK(ret == Result::OK);
    property_get(PQ_PIC_MODE_PROPERTY_STR, value, PQ_PIC_MODE_DEFAULT);
    CHECK(atoi(value) == mode);

    CASE_DONE();
}

void test_setPQIndex(int32_t level, int32_t  scenario, int32_t  tuning_mode, int32_t index)
{
    Result ret;
    DISP_PQ_PARAM pqparam_test;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return;
    }

    cout << " index = " << index << " ,level = " << level << endl;

    ret = service->setPQIndex(level, scenario, tuning_mode, index, PQ_DEFAULT_TRANSITION_OFF_STEP);
    CHECK(ret == Result::OK);

    CASE_DONE();
}

void test_getMappedPQIndex(int32_t scenario, int32_t mode)
{
    Result ret;
    int32_t dlopen_error;
    int32_t scenario_index;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return;
    }

    DISP_PQ_PARAM pqparam_test;
    DISP_PQ_PARAM pqparam_user_def;
    DISP_PQ_PARAM pqparam_table[PQ_PARAM_TABLE_SIZE];
    DISP_PQ_MAPPING_PARAM pqparam_mapping;

    cout <<  " scenario = " << scenario << " mode = " << mode << endl;

    //int pqparam_table_index = (mode < PQ_PIC_MODE_USER_DEF)? (mode) * PQ_SCENARIO_COUNT + scenario : PQ_PREDEFINED_MODE_COUNT * PQ_SCENARIO_COUNT
    dlopen_error = getPqparam_table(&pqparam_table[0]);
    CHECK(dlopen_error == 0);

    LoadUserModePQParam(&pqparam_table[PQ_PREDEFINED_MODE_COUNT * PQ_SCENARIO_COUNT]);

    dlopen_error = getPqparam_mapping(&pqparam_mapping);
    CHECK(dlopen_error == 0);

    service->getMappedColorIndex(scenario, mode,
        [&] (Result retval, const dispPQIndexParams &index) {
        if (retval == Result::OK) {
            memcpy(&pqparam_test, &index, sizeof(DISP_PQ_PARAM));
        }
        ret = retval;
    });
    CHECK(ret == Result::OK);
    service->getMappedTDSHPIndex(scenario, mode,
        [&] (Result retval, const dispPQIndexParams &index) {
        if (retval == Result::OK) {
            memcpy(&pqparam_test, &index, sizeof(DISP_PQ_PARAM));
        }
        ret = retval;
    });
    CHECK(ret == Result::OK);

    scenario_index = PQPictureMode::getScenarioIndex(scenario);

    if(mode == PQ_PIC_MODE_STANDARD || mode == PQ_PIC_MODE_VIVID)
    {
        CHECK(0 == memcmp(&pqparam_test, &pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index], sizeof(DISP_PQ_PARAM) - sizeof(unsigned int)));
        cout << "pqparam_table_index " << (mode) * PQ_SCENARIO_COUNT + scenario_index << endl;
        cout << "pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index], shp[%d] " << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4SHPGain << ",gsat[%d] " << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4SatGain << ",cont[%d] " << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4Contrast << ",bri[%d] " << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4Brightness << endl;
        cout << "pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index], hue0[%d]" << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4HueAdj[0] << ",hue1[%d] "  << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4HueAdj[1]  << ",hue2[%d] " << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4HueAdj[2]  << ",hue3[%d] "<< pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4HueAdj[3] << endl;
        cout << "pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index], sat0[%d]" << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4SatAdj[0] << ",sat1[%d] "  << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4SatAdj[1]  << ",sat2[%d] " << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4SatAdj[2]  << ",sat3[%d] "<< pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4SatAdj[3] << endl;
        cout << "pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index], partialY[%d]" << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4PartialY << ",u4Ccorr[%d] "  << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4Ccorr   << endl;

        cout << "pqparam_test, shp[%d] " << pqparam_test.u4SHPGain << ",gsat[%d] " << pqparam_test.u4SatGain << ",cont[%d] " << pqparam_test.u4Contrast << ",bri[%d] " << pqparam_test.u4Brightness << endl;
        cout << "pqparam_test, hue0[%d]" << pqparam_test.u4HueAdj[0] << ",hue1[%d] "  << pqparam_test.u4HueAdj[1]  << ",hue2[%d] " << pqparam_test.u4HueAdj[2]  << ",hue3[%d] "<< pqparam_test.u4HueAdj[3] << endl;
        cout << "pqparam_test, sat0[%d]" << pqparam_test.u4SatAdj[0] << ",sat1[%d] "  << pqparam_test.u4SatAdj[1]  << ",sat2[%d] " << pqparam_test.u4SatAdj[2]  << ",sat3[%d] "<< pqparam_test.u4SatAdj[3] << endl;
        cout << "pqparam_test, partialY[%d]" << pqparam_test.u4PartialY << ",u4Ccorr[%d] "  << pqparam_test.u4Ccorr   << endl;

    }
    else if ( mode == PQ_PIC_MODE_USER_DEF)
    {
        calcPQStrength(&pqparam_user_def, &pqparam_table[PQ_PREDEFINED_MODE_COUNT * PQ_SCENARIO_COUNT], getPQStrengthPercentage(scenario, &pqparam_mapping));
        CHECK(0 == memcmp(&pqparam_test, &pqparam_user_def, sizeof(DISP_PQ_PARAM) - sizeof(unsigned int)));

        cout << "pqparam_user_def, shp[%d] " << pqparam_user_def.u4SHPGain << ",gsat[%d] " << pqparam_user_def.u4SatGain << ",cont[%d] " << pqparam_user_def.u4Contrast << ",bri[%d] " << pqparam_user_def.u4Brightness << endl;
        cout << "pqparam_user_def, hue0[%d]" << pqparam_user_def.u4HueAdj[0] << ",hue1[%d] "  << pqparam_user_def.u4HueAdj[1]  << ",hue2[%d] " << pqparam_user_def.u4HueAdj[2]  << ",hue3[%d] "<< pqparam_user_def.u4HueAdj[3] << endl;
        cout << "pqparam_user_def, sat0[%d]" << pqparam_user_def.u4SatAdj[0] << ",sat1[%d] "  << pqparam_user_def.u4SatAdj[1]  << ",sat2[%d] " << pqparam_user_def.u4SatAdj[2]  << ",sat3[%d] "<< pqparam_user_def.u4SatAdj[3] << endl;
        cout << "pqparam_user_def, partialY[%d]" << pqparam_user_def.u4PartialY << ",u4Ccorr[%d] "  << pqparam_user_def.u4Ccorr   << endl;

        cout << "pqparam_test, shp[%d] " << pqparam_test.u4SHPGain << ",gsat[%d] " << pqparam_test.u4SatGain << ",cont[%d] " << pqparam_test.u4Contrast << ",bri[%d] " << pqparam_test.u4Brightness << endl;
        cout << "pqparam_test, hue0[%d]" << pqparam_test.u4HueAdj[0] << ",hue1[%d] "  << pqparam_test.u4HueAdj[1]  << ",hue2[%d] " << pqparam_test.u4HueAdj[2]  << ",hue3[%d] "<< pqparam_test.u4HueAdj[3] << endl;
        cout << "pqparam_test, sat0[%d]" << pqparam_test.u4SatAdj[0] << ",sat1[%d] "  << pqparam_test.u4SatAdj[1]  << ",sat2[%d] " << pqparam_test.u4SatAdj[2]  << ",sat3[%d] "<< pqparam_test.u4SatAdj[3] << endl;
        cout << "pqparam_test, partialY[%d]" << pqparam_test.u4PartialY << ",u4Ccorr[%d] "  << pqparam_test.u4Ccorr   << endl;

    }

    CASE_DONE();
}

void test_getPQIndex(int32_t scenario, int32_t mode)
{
    Result ret;
    int32_t dlopen_error;
    int32_t scenario_index;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return;
    }

    cout <<  " scenario = " << scenario << " mode = " << mode << endl;

    DISP_PQ_PARAM pqparam_test;
    DISP_PQ_PARAM pqparam_user_def;
    DISP_PQ_PARAM pqparam_table[PQ_PARAM_TABLE_SIZE];
    DISP_PQ_MAPPING_PARAM pqparam_mapping;

    scenario_index = PQPictureMode::getScenarioIndex(scenario);

    int pqparam_table_index = (mode < PQ_PIC_MODE_USER_DEF)? (mode) * PQ_SCENARIO_COUNT + scenario_index : PQ_PREDEFINED_MODE_COUNT * PQ_SCENARIO_COUNT;

    dlopen_error = getPqparam_table(&pqparam_table[0]);
    CHECK(dlopen_error == 0);

    LoadUserModePQParam(&pqparam_table[PQ_PREDEFINED_MODE_COUNT * PQ_SCENARIO_COUNT]);

    service->getColorIndex(scenario, mode,
        [&] (Result retval, const dispPQIndexParams &index) {
        if (retval == Result::OK) {
            memcpy(&pqparam_test, &index, sizeof(DISP_PQ_PARAM));
        }
        ret = retval;
    });
    CHECK(ret == Result::OK);
    service->getTDSHPIndex(scenario, mode,
        [&] (Result retval, const dispPQIndexParams &index) {
        if (retval == Result::OK) {
            memcpy(&pqparam_test, &index, sizeof(DISP_PQ_PARAM));
        }
        ret = retval;
    });
    CHECK(ret == Result::OK);

    CHECK(0 == memcmp(&pqparam_test, &pqparam_table[pqparam_table_index], sizeof(DISP_PQ_PARAM) - sizeof(unsigned int))); // discard ccorr
    cout << "pqparam_table_index " << pqparam_table_index << endl;
    cout << "pqparam_table[pqparam_table_index], shp[%d] " << pqparam_table[pqparam_table_index].u4SHPGain << ",gsat[%d] " << pqparam_table[pqparam_table_index].u4SatGain << ",cont[%d] " << pqparam_table[pqparam_table_index].u4Contrast << ",bri[%d] " << pqparam_table[pqparam_table_index].u4Brightness << endl;
    cout << "pqparam_table[pqparam_table_index], hue0[%d]" << pqparam_table[pqparam_table_index].u4HueAdj[0] << ",hue1[%d] "  << pqparam_table[pqparam_table_index].u4HueAdj[1]  << ",hue2[%d] " << pqparam_table[pqparam_table_index].u4HueAdj[2]  << ",hue3[%d] "<< pqparam_table[pqparam_table_index].u4HueAdj[3] << endl;
    cout << "pqparam_table[pqparam_table_index], sat0[%d]" << pqparam_table[pqparam_table_index].u4SatAdj[0] << ",sat1[%d] "  << pqparam_table[pqparam_table_index].u4SatAdj[1]  << ",sat2[%d] " << pqparam_table[pqparam_table_index].u4SatAdj[2]  << ",sat3[%d] "<< pqparam_table[pqparam_table_index].u4SatAdj[3] << endl;
    cout << "pqparam_table[pqparam_table_index], partialY[%d]" << pqparam_table[pqparam_table_index].u4PartialY << ",u4Ccorr[%d] "  << pqparam_table[pqparam_table_index].u4Ccorr   << endl;


    cout << "pqparam_test, shp[%d] " << pqparam_test.u4SHPGain << ",gsat[%d] " << pqparam_test.u4SatGain << ",cont[%d] " << pqparam_test.u4Contrast << ",bri[%d] " << pqparam_test.u4Brightness << endl;
    cout << "pqparam_test, hue0[%d]" << pqparam_test.u4HueAdj[0] << ",hue1[%d] "  << pqparam_test.u4HueAdj[1]  << ",hue2[%d] " << pqparam_test.u4HueAdj[2]  << ",hue3[%d] "<< pqparam_test.u4HueAdj[3] << endl;
    cout << "pqparam_test, sat0[%d]" << pqparam_test.u4SatAdj[0] << ",sat1[%d] "  << pqparam_test.u4SatAdj[1]  << ",sat2[%d] " << pqparam_test.u4SatAdj[2]  << ",sat3[%d] "<< pqparam_test.u4SatAdj[3] << endl;
    cout << "pqparam_test, partialY[%d]" << pqparam_test.u4PartialY << ",u4Ccorr[%d] "  << pqparam_test.u4Ccorr   << endl;
    CASE_DONE();
}

void test_TuningFlag(int32_t FlagEnum)
{
    Result ret;
    int32_t  TuninfFlag;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return;
    }

    cout <<  " Flag = " << FlagEnum <<  endl;

    ret = service->setTDSHPFlag(FlagEnum);
    CHECK(ret == Result::OK);

    service->getTDSHPFlag(
        [&] (Result retval, int32_t value) {
        if (retval == Result::OK) {
            TuninfFlag = value;
        }
        ret = retval;
    });
    CHECK(ret == Result::OK);
    CHECK(TuninfFlag == FlagEnum);

    CASE_DONE();
}

void test_setDISPScenario(int32_t scenario)
{
    Result ret;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return;
    }

    cout <<  " scenario = " << scenario <<  endl;

    ret = service->setDISPScenario(scenario, PQ_DEFAULT_TRANSITION_ON_STEP);
    CHECK(ret == Result::OK);

    CASE_DONE();
}

void test_FeatureSwitch(PQFeatureID id, uint32_t value)
{
    Result ret;
    uint32_t test_value;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return;
    }

    cout <<  " PQ Feature id = " << static_cast<int32_t>(id) <<  " value = " << value <<  endl;

    ret = service->setFeatureSwitch(id, value);
    CHECK(ret == Result::OK);

    service->getFeatureSwitch(id,
        [&] (Result retval, uint32_t value) {
        if (retval == Result::OK) {
            test_value = value;
        }
        ret = retval;
    });
    CHECK(ret == Result::OK);

    CHECK(test_value == value);

    CASE_DONE();
}

void test_UltraResolutionReg(int32_t level, int32_t index)
{
    Result ret;
    int32_t value;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return;
    }

    cout <<  " index = " << index <<  endl;

    ret = service->setTuningField(MOD_ULTRARESOLUTION, index, level);
    CHECK(ret == Result::OK);

    service->getTuningField(MOD_ULTRARESOLUTION, index,
        [&] (Result retval, int32_t retValue) {
        if (retval == Result::OK) {
            value = retValue;
        }
        ret = retval;
    });
    CHECK(ret == Result::OK);
    CHECK(value == level);

    CASE_DONE();
}

int main(int, char *[])
{
    int i,j;

    // test_BluLightTuning();

    // test_CustParameters();
#if 0
{   // test_setPQMode
    test_setPQMode(PQ_PIC_MODE_STANDARD);
    test_setPQMode(PQ_PIC_MODE_VIVID);
    test_setPQMode(PQ_PIC_MODE_USER_DEF);
}



{   // test_getMappedPQIndex
    for (i = SCENARIO_VIDEO; i <= SCENARIO_ISP_PREVIEW + OUT_OF_UPPERBOUND_ONE; i++)
    {
        for (j = PQ_PIC_MODE_STANDARD; j <= PQ_PIC_MODE_USER_DEF; j++)
        {
            test_setPQMode(j);
            test_getMappedPQIndex(i, j);
        }
    }

}

{   // test_getPQIndex
    for (i = SCENARIO_VIDEO; i <= SCENARIO_ISP_PREVIEW + OUT_OF_UPPERBOUND_ONE; i++)
    {
        for (j = PQ_PIC_MODE_STANDARD; j <= PQ_PIC_MODE_USER_DEF; j++)
        {
            test_setPQMode(j);
            test_getPQIndex(i, j);
        }
    }
}

{
    // test UltraResolutionReg
    for (i = RSZ_tableMode; i < RSZ_INDEX_MAX + OUT_OF_UPPERBOUND_ONE; i++)
    {
        test_UltraResolutionReg(0, i);
    }

}

{   // test_TuningFlag
    test_TuningFlag(TDSHP_FLAG_NCS_SHP_TUNING );
    test_TuningFlag(TDSHP_FLAG_NORMAL);
}

{   // test_setDISPScenario
    for (i = SCENARIO_PICTURE; i <= SCENARIO_ISP_PREVIEW + OUT_OF_UPPERBOUND_ONE; i++)
    {
        test_setDISPScenario(i);
    }

}

{
    // test_FeatureSwitch
    for (i = static_cast<int32_t>(PQFeatureID::DISPLAY_COLOR); i < static_cast<int32_t>(PQFeatureID::PQ_FEATURE_MAX) + OUT_OF_UPPERBOUND_ONE; i++)
    {
#ifndef MDP_COLOR_ENABLE
        if (i == static_cast<int32_t>(PQFeatureID::CONTENT_COLOR) || i == static_cast<int32_t>(PQFeatureID::CONTENT_COLOR_VIDEO))
            continue;
#endif
        if (i == static_cast<int32_t>(PQFeatureID::DISPLAY_CCORR))  //CCORR feature switch has not been implemented
            continue;

        test_FeatureSwitch(static_cast<PQFeatureID>(i), 0);
        test_FeatureSwitch(static_cast<PQFeatureID>(i), 1);
    }

}

{   // test_setPQIndex
    for (i = SET_PQ_SHP_GAIN; i <= SET_PQ_BRIGHTNESS + OUT_OF_UPPERBOUND_ONE; i++)
    {
        test_setPQIndex(9, SCENARIO_PICTURE, TDSHP_FLAG_NORMAL, i);
        test_setPQIndex(0, SCENARIO_PICTURE, TDSHP_FLAG_NORMAL, i);
    }

}
#endif
    #define DSRegOffset(field) (unsigned int)((char*)&( ((DSReg*)(0))->field) - (char*)(0))

    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return 0;
    }
    service->setTuningField(MOD_DS_SWREG, 0xffff, 1);
    int32_t value;
    while (1){
        service->getTuningField(MOD_DS_SWREG, 0xfffe,
            [&] (Result retval, int32_t retValue) {
            if (retval == Result::OK) {
                value = retValue;
            }
        });
        PQ_LOGD("0xfffe = [%d]",value);
        if(value == 1)
            break;
    }


    service->setTuningField(MOD_DS_SWREG, 0xffff, 2);
    service->setTuningField(MOD_DS_SWREG, 0, 0);

    service->setTuningField(MOD_DS_SWREG, 0xfffe, 0);
    service->setTuningField(MOD_DS_SWREG, 0xffff, 0);
    if (0){
    service->setTuningField(MOD_DYNAMIC_SHARPNESS, 0xffff, 1);
    //service->setTuningField(MOD_DYNAMIC_SHARPNESS, PQDS_DS_en, 0);
    int32_t value;
    while (1){
    service->getTuningField(MOD_DYNAMIC_SHARPNESS, 0xfffe,
        [&] (Result retval, int32_t retValue) {
        if (retval == Result::OK) {
            value = retValue;
        }
    });

    PQ_LOGD("0xfffe = [%d]",value);
    if(value == 1)
        break;
    }
    service->getTuningField(MOD_DYNAMIC_SHARPNESS, PQDS_iGain_clip2,
        [&] (Result retval, int32_t retValue) {
        if (retval == Result::OK) {
            value = retValue;
        }
    });
    //service->setTuningField(MOD_DYNAMIC_SHARPNESS, 0xfffe, 0);
    PQ_LOGD("PQDS_iGain_clip2 = [%d]",value);
    }
    PQ_LOGD("[PQ_TEST] Refactor");

    return 0;
}
