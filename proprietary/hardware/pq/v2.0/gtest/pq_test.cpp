#define LOG_TAG "PQ-Test"

#include <cstdlib>
#include <iostream>
#include <log/log.h>
#include <cutils/properties.h>
#include <cust_color.h>
#include <PQCommon.h>
#include "cust_tdshp.h"
#include "gtest/gtest.h"

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
        if (!(stat)) { \
            cerr << __func__ << ":" << __LINE__ << endl; \
            cerr << "    " << #stat << " failed." << endl; \
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

bool checkResult(bool stat)
{
    if(!stat)
        return 1;
    else
        return 0;
}

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

bool test_BluLightTuning()
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
    bool err = 0;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return false;
    }

    service->enableBlueLight(true, PQ_DEFAULT_TRANSITION_ON_STEP);

    // Reading mode on/off
    ret = service->setTuningField(MOD_BLUE_LIGHT_INPUT, 0xffff, 1);
    err |= checkResult(ret == Result::OK);
    service->getTuningField(MOD_BLUE_LIGHT_INPUT, 0xffff,
        [&] (Result retval, int32_t retValue) {
        if (retval == Result::OK) {
            value = retValue;
        }
        ret = retval;
    });
    err |= checkResult(ret == Result::OK);
    err |= checkResult(value == 1);

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
    err |= checkResult(ret != Result::OK);
    ret = service->setTuningField(MOD_BLUE_LIGHT_INPUT, 0x5, 0);
    err |= checkResult(ret != Result::OK);

    // Error handing: invalid mode
    service->setTuningField(MOD_BLUE_LIGHT_INPUT, 0xffff, 1);
    ret = service->setTuningField(MOD_BLUE_LIGHT_INPUT, 0xffff, 10);
    err |= checkResult(ret != Result::OK);
    service->getTuningField(MOD_BLUE_LIGHT_INPUT, 0xffff,
        [&] (Result retval, int32_t retValue) {
        if (retval == Result::OK) {
            value = retValue;
        }
        ret = retval;
    });
    err |= checkResult(value == 1); // Should keep previous mode

    // Error handing: write in reading mode should be forbidden
    service->setTuningField(MOD_BLUE_LIGHT_INPUT, 0xffff, 1);
    ret = service->setTuningField(MOD_BLUE_LIGHT_INPUT, RegOffset(GLOBAL_SAT), 512);
    err |= checkResult(ret != Result::OK);

    #undef RegOffset

    CASE_DONE();

    return err;
}

bool test_CustParameters()
{
    CustParameters &cust = CustParameters::getPQCust();
    int var = 0;
    bool err = 0;

    if (cust.isGood()) {
        err |= checkResult(cust.loadVar("BaseOnly", &var));
        err |= checkResult(var == 1111);

        err |= checkResult(cust.loadVar("ProjectOnly", &var));
        err |= checkResult(var == 2222);

        err |= checkResult(cust.loadVar("ProjectOverwritten", &var));
        err |= checkResult(var == 2222);
    } else {
        cerr << "Load libpq_cust*.so failed." << endl;
    }

    CASE_DONE();
    return err;
}

bool test_setPQMode(volatile int32_t mode)
{
    Result ret;
    bool err = 0;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return false;
    }

    char value[PROPERTY_VALUE_MAX];
    cout << "mode =" << mode  << endl;
    ret = service->setPQMode(mode, PQ_DEFAULT_TRANSITION_OFF_STEP);
    err |=  checkResult(ret == Result::OK);
    CHECK(ret == Result::OK);
    property_get(PQ_PIC_MODE_PROPERTY_STR, value, PQ_PIC_MODE_DEFAULT);
    err |=  checkResult(atoi(value) == mode);
    CHECK(atoi(value) == mode);
    CASE_DONE();
    return err;
}

bool test_setPQIndex(volatile int32_t level, volatile int32_t  scenario, volatile int32_t  tuning_mode, volatile int32_t index)
{
    Result ret;
    DISP_PQ_PARAM pqparam_test;
    bool err = 0;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return false;
    }

    cout << " index = " << index << " ,level = " << level << endl;

    ret = service->setPQIndex(level, scenario, tuning_mode, index, PQ_DEFAULT_TRANSITION_OFF_STEP);
    err |= checkResult(ret == Result::OK);
    CHECK(ret == Result::OK);

    CASE_DONE();
    return err;
}

bool test_getMappedPQIndex(volatile int32_t scenario, volatile int32_t mode)
{
    Result ret;
    int32_t dlopen_error;
    int32_t scenario_index;
    bool err = 0;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return false;
    }

    DISP_PQ_PARAM pqparam_test;
    DISP_PQ_PARAM pqparam_user_def;
    DISP_PQ_PARAM pqparam_table[PQ_PARAM_TABLE_SIZE];
    DISP_PQ_MAPPING_PARAM pqparam_mapping;

    cout <<  " scenario = " << scenario << " mode = " << mode << endl;

    //int pqparam_table_index = (mode < PQ_PIC_MODE_USER_DEF)? (mode) * PQ_SCENARIO_COUNT + scenario : PQ_PREDEFINED_MODE_COUNT * PQ_SCENARIO_COUNT
    dlopen_error = getPqparam_table(&pqparam_table[0]);
    err |= checkResult(dlopen_error == 0);
    CHECK(dlopen_error == 0);

    LoadUserModePQParam(&pqparam_table[PQ_PREDEFINED_MODE_COUNT * PQ_SCENARIO_COUNT]);

    dlopen_error = getPqparam_mapping(&pqparam_mapping);
    err |= checkResult(dlopen_error == 0);
    CHECK(dlopen_error == 0);
    service->getMappedColorIndex(scenario, mode,
        [&] (Result retval, const dispPQIndexParams &index) {
        if (retval == Result::OK) {
            memcpy(&pqparam_test, &index, sizeof(DISP_PQ_PARAM));
        }
        ret = retval;
    });
    err |= checkResult(ret == Result::OK);
    CHECK(ret == Result::OK);
    service->getMappedTDSHPIndex(scenario, mode,
        [&] (Result retval, const dispPQIndexParams &index) {
        if (retval == Result::OK) {
            memcpy(&pqparam_test, &index, sizeof(DISP_PQ_PARAM));
        }
        ret = retval;
    });
    err |= checkResult(ret == Result::OK);
    CHECK(ret == Result::OK);

    scenario_index = PQPictureMode::getScenarioIndex(scenario);

    if(mode == PQ_PIC_MODE_STANDARD || mode == PQ_PIC_MODE_VIVID)
    {
        err |= checkResult(0 == memcmp(&pqparam_test, &pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index], sizeof(DISP_PQ_PARAM) - sizeof(unsigned int)));
        CHECK(0 == memcmp(&pqparam_test, &pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index], sizeof(DISP_PQ_PARAM) - sizeof(unsigned int)));
        cout << "pqparam_table_index " << (mode) * PQ_SCENARIO_COUNT + scenario_index << endl;
        cout << "pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index], shp " << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4SHPGain << ",gsat " << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4SatGain << ",cont " << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4Contrast << ",bri " << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4Brightness << endl;
        cout << "pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index], hue0 " << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4HueAdj[0] << ",hue1 "  << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4HueAdj[1]  << ",hue2 " << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4HueAdj[2]  << ",hue3 "<< pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4HueAdj[3] << endl;
        cout << "pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index], sat0 " << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4SatAdj[0] << ",sat1 "  << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4SatAdj[1]  << ",sat2 " << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4SatAdj[2]  << ",sat3 "<< pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4SatAdj[3] << endl;
        cout << "pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index], partialY" << pqparam_table[(mode) * PQ_SCENARIO_COUNT + scenario_index].u4PartialY   << endl;

        cout << "pqparam_test, shp " << pqparam_test.u4SHPGain << ",gsat " << pqparam_test.u4SatGain << ",cont " << pqparam_test.u4Contrast << ",bri " << pqparam_test.u4Brightness << endl;
        cout << "pqparam_test, hue0 " << pqparam_test.u4HueAdj[0] << ",hue1 "  << pqparam_test.u4HueAdj[1]  << ",hue2 " << pqparam_test.u4HueAdj[2]  << ",hue3 "<< pqparam_test.u4HueAdj[3] << endl;
        cout << "pqparam_test, sat0 " << pqparam_test.u4SatAdj[0] << ",sat1 "  << pqparam_test.u4SatAdj[1]  << ",sat2 " << pqparam_test.u4SatAdj[2]  << ",sat3 "<< pqparam_test.u4SatAdj[3] << endl;
        cout << "pqparam_test, partialY" << pqparam_test.u4PartialY  <<  endl;

    }
    else if ( mode == PQ_PIC_MODE_USER_DEF)
    {
        calcPQStrength(&pqparam_user_def, &pqparam_table[PQ_PREDEFINED_MODE_COUNT * PQ_SCENARIO_COUNT], getPQStrengthPercentage(scenario, &pqparam_mapping));
        err |= checkResult(0 == memcmp(&pqparam_test, &pqparam_user_def, sizeof(DISP_PQ_PARAM) - sizeof(unsigned int)));
        CHECK(0 == memcmp(&pqparam_test, &pqparam_user_def, sizeof(DISP_PQ_PARAM) - sizeof(unsigned int)));

        cout << "pqparam_user_def, shp " << pqparam_user_def.u4SHPGain << ",gsat " << pqparam_user_def.u4SatGain << ",cont " << pqparam_user_def.u4Contrast << ",bri " << pqparam_user_def.u4Brightness << endl;
        cout << "pqparam_user_def, hue0" << pqparam_user_def.u4HueAdj[0] << ",hue1 "  << pqparam_user_def.u4HueAdj[1]  << ",hue2 " << pqparam_user_def.u4HueAdj[2]  << ",hue3 "<< pqparam_user_def.u4HueAdj[3] << endl;
        cout << "pqparam_user_def, sat0" << pqparam_user_def.u4SatAdj[0] << ",sat1 "  << pqparam_user_def.u4SatAdj[1]  << ",sat2 " << pqparam_user_def.u4SatAdj[2]  << ",sat3 "<< pqparam_user_def.u4SatAdj[3] << endl;
        cout << "pqparam_user_def, partialY" << pqparam_user_def.u4PartialY  << endl;

        cout << "pqparam_test, shp " << pqparam_test.u4SHPGain << ",gsat " << pqparam_test.u4SatGain << ",cont " << pqparam_test.u4Contrast << ",bri " << pqparam_test.u4Brightness << endl;
        cout << "pqparam_test, hue0 " << pqparam_test.u4HueAdj[0] << ",hue1 "  << pqparam_test.u4HueAdj[1]  << ",hue2 " << pqparam_test.u4HueAdj[2]  << ",hue3 "<< pqparam_test.u4HueAdj[3] << endl;
        cout << "pqparam_test, sat0 " << pqparam_test.u4SatAdj[0] << ",sat1 "  << pqparam_test.u4SatAdj[1]  << ",sat2 " << pqparam_test.u4SatAdj[2]  << ",sat3 "<< pqparam_test.u4SatAdj[3] << endl;
        cout << "pqparam_test, partialY" << pqparam_test.u4PartialY  << endl;

    }

    CASE_DONE();
    return err;
}

bool test_getPQIndex(volatile int32_t scenario,volatile int32_t mode)
{
    Result ret;
    int32_t dlopen_error;
    int32_t scenario_index;
    bool err = 0;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return false;
    }

    cout <<  " scenario = " << scenario << " mode = " << mode << endl;

    DISP_PQ_PARAM pqparam_test;
    DISP_PQ_PARAM pqparam_user_def;
    DISP_PQ_PARAM pqparam_table[PQ_PARAM_TABLE_SIZE];
    DISP_PQ_MAPPING_PARAM pqparam_mapping;

    scenario_index = PQPictureMode::getScenarioIndex(scenario);

    int pqparam_table_index = (mode < PQ_PIC_MODE_USER_DEF)? (mode) * PQ_SCENARIO_COUNT + scenario_index : PQ_PREDEFINED_MODE_COUNT * PQ_SCENARIO_COUNT;

    dlopen_error = getPqparam_table(&pqparam_table[0]);
    err |= checkResult(dlopen_error == 0);
    CHECK(dlopen_error == 0);

    LoadUserModePQParam(&pqparam_table[PQ_PREDEFINED_MODE_COUNT * PQ_SCENARIO_COUNT]);
    cout <<  " scenario = " << scenario << " mode = " << mode << " scenario_index = " << scenario_index  <<  endl;

    ret = service->setPQMode(mode, PQ_DEFAULT_TRANSITION_OFF_STEP);
    err |=  checkResult(ret == Result::OK);
    CHECK(ret == Result::OK);

    service->getColorIndex(scenario, mode,
        [&] (Result retval, const dispPQIndexParams &index) {
        if (retval == Result::OK) {
            memcpy(&pqparam_test, &index, sizeof(DISP_PQ_PARAM));
        }
        ret = retval;
    });
    err |= checkResult(ret == Result::OK);
    CHECK(ret == Result::OK);
    service->getTDSHPIndex(scenario, mode,
        [&] (Result retval, const dispPQIndexParams &index) {
        if (retval == Result::OK) {
            memcpy(&pqparam_test, &index, sizeof(DISP_PQ_PARAM));
        }
        ret = retval;
    });
    err |= checkResult(ret == Result::OK);
    CHECK(ret == Result::OK);

    err |= checkResult(0 == memcmp(&pqparam_test, &pqparam_table[pqparam_table_index], sizeof(DISP_PQ_PARAM) - sizeof(unsigned int))); // discard ccorr
    CHECK(0 == memcmp(&pqparam_test, &pqparam_table[pqparam_table_index], sizeof(DISP_PQ_PARAM) - sizeof(unsigned int))); // discard ccorr
    cout << "pqparam_table_index " << pqparam_table_index << endl;
    cout << "pqparam_table[pqparam_table_index], shp " << pqparam_table[pqparam_table_index].u4SHPGain << ",gsat " << pqparam_table[pqparam_table_index].u4SatGain << ",cont " << pqparam_table[pqparam_table_index].u4Contrast << ",bri " << pqparam_table[pqparam_table_index].u4Brightness << endl;
    cout << "pqparam_table[pqparam_table_index], hue0 " << pqparam_table[pqparam_table_index].u4HueAdj[0] << ",hue1 "  << pqparam_table[pqparam_table_index].u4HueAdj[1]  << ",hue2 " << pqparam_table[pqparam_table_index].u4HueAdj[2]  << ",hue3 "<< pqparam_table[pqparam_table_index].u4HueAdj[3] << endl;
    cout << "pqparam_table[pqparam_table_index], sat0 " << pqparam_table[pqparam_table_index].u4SatAdj[0] << ",sat1 "  << pqparam_table[pqparam_table_index].u4SatAdj[1]  << ",sat2 " << pqparam_table[pqparam_table_index].u4SatAdj[2]  << ",sat3 "<< pqparam_table[pqparam_table_index].u4SatAdj[3] << endl;
    cout << "pqparam_table[pqparam_table_index], partialY" << pqparam_table[pqparam_table_index].u4PartialY  << endl;


    cout << "pqparam_test, shp " << pqparam_test.u4SHPGain << ",gsat " << pqparam_test.u4SatGain << ",cont " << pqparam_test.u4Contrast << ",bri " << pqparam_test.u4Brightness << endl;
    cout << "pqparam_test, hue0 " << pqparam_test.u4HueAdj[0] << ",hue1 "  << pqparam_test.u4HueAdj[1]  << ",hue2 " << pqparam_test.u4HueAdj[2]  << ",hue3 "<< pqparam_test.u4HueAdj[3] << endl;
    cout << "pqparam_test, sat0 " << pqparam_test.u4SatAdj[0] << ",sat1 "  << pqparam_test.u4SatAdj[1]  << ",sat2 " << pqparam_test.u4SatAdj[2]  << ",sat3 "<< pqparam_test.u4SatAdj[3] << endl;
    cout << "pqparam_test, partialY" << pqparam_test.u4PartialY  << endl;
    CASE_DONE();
    return err;
}

bool test_TuningFlag(volatile int32_t FlagEnum)
{
    Result ret;
    int32_t  TuninfFlag;
    bool err = 0;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return false;
    }

    cout <<  " Flag = " << FlagEnum <<  endl;

    ret = service->setTDSHPFlag(FlagEnum);
    err |= checkResult(ret == Result::OK);
    CHECK(ret == Result::OK);

    service->getTDSHPFlag(
        [&] (Result retval, int32_t value) {
        if (retval == Result::OK) {
            TuninfFlag = value;
        }
        ret = retval;
    });
    err |= checkResult(ret == Result::OK);
    CHECK(ret == Result::OK);
    err |= checkResult(TuninfFlag == FlagEnum);
    CHECK(TuninfFlag == FlagEnum);

    CASE_DONE();
    return err;
}

bool test_setDISPScenario(volatile int32_t scenario)
{
    Result ret;
    bool err = 0;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return false;
    }

    cout <<  " scenario = " << scenario <<  endl;

    ret = service->setDISPScenario(scenario, PQ_DEFAULT_TRANSITION_ON_STEP);
    err |= checkResult(ret == Result::OK);
    CHECK(ret == Result::OK);

    CASE_DONE();
    return err;
}

bool test_FeatureSwitch(volatile PQFeatureID id, volatile uint32_t value)
{
    Result ret;
    uint32_t test_value;
    bool err = 0;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return false;
    }

    cout <<  " PQ Feature id = " << static_cast<int32_t>(id) <<  " value = " << value <<  endl;

    ret = service->setFeatureSwitch(id, value);
    err |= checkResult(ret == Result::OK);
    CHECK(ret == Result::OK);

    service->getFeatureSwitch(id,
        [&] (Result retval, uint32_t value) {
        if (retval == Result::OK) {
            test_value = value;
        }
        ret = retval;
    });
    err |= checkResult(ret == Result::OK);
    CHECK(ret == Result::OK);

    err |= checkResult(test_value == value);
    CHECK(test_value == value);

    CASE_DONE();
    return err;
}

bool test_UltraResolutionReg(volatile int32_t level, volatile int32_t index)
{
    Result ret;
    int32_t value;
    bool err = 0;
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGD("[test_PQ] failed to get HW service");
        return false;
    }

    cout <<  " index = " << index <<  endl;

    ret = service->setTuningField(MOD_ULTRARESOLUTION, index, level);
    err |= checkResult(ret == Result::OK);
    CHECK(ret == Result::OK);

    service->getTuningField(MOD_ULTRARESOLUTION, index,
        [&] (Result retval, int32_t retValue) {
        if (retval == Result::OK) {
            value = retValue;
        }
        ret = retval;
    });
    err |= checkResult(ret == Result::OK);
    CHECK(ret == Result::OK);
    err |= checkResult(value == level);
    CHECK(value == level);

    CASE_DONE();
    return err;
}


TEST (PQTest, PQ_001)
{
    EXPECT_EQ(0,test_setDISPScenario(SCENARIO_PICTURE));
    EXPECT_EQ(0,test_setDISPScenario(SCENARIO_ISP_PREVIEW));

    EXPECT_EQ(0,test_getPQIndex(SCENARIO_PICTURE, PQ_PIC_MODE_STANDARD));
    EXPECT_EQ(0,test_getPQIndex(SCENARIO_PICTURE, PQ_PIC_MODE_VIVID));
    EXPECT_EQ(0,test_getPQIndex(SCENARIO_PICTURE, PQ_PIC_MODE_USER_DEF));
    EXPECT_EQ(0,test_getPQIndex(SCENARIO_PICTURE, PQ_PIC_MODE_STANDARD));
    EXPECT_EQ(0,test_getPQIndex(SCENARIO_ISP_PREVIEW, PQ_PIC_MODE_VIVID));
    EXPECT_EQ(0,test_getPQIndex(SCENARIO_ISP_PREVIEW, PQ_PIC_MODE_USER_DEF));
}

TEST (PQTest, PQ_002)
{
    EXPECT_EQ(0,test_setPQMode(PQ_PIC_MODE_STANDARD));
    EXPECT_EQ(0,test_setPQMode(PQ_PIC_MODE_VIVID));
    EXPECT_EQ(0,test_setPQMode(PQ_PIC_MODE_USER_DEF));
}

TEST (PQTest, PQ_003)
{
    int i,j;

    for (i = SET_PQ_SHP_GAIN; i <= SET_PQ_BRIGHTNESS + OUT_OF_UPPERBOUND_ONE; i++)
    {
        EXPECT_EQ(0,test_setPQIndex(9, SCENARIO_PICTURE, TDSHP_FLAG_NORMAL, i));
        EXPECT_EQ(0,test_setPQIndex(0, SCENARIO_PICTURE, TDSHP_FLAG_NORMAL, i));
    }
}

TEST (PQTest, PQ_004)
{
    EXPECT_EQ(0,test_FeatureSwitch(static_cast<PQFeatureID>(PQFeatureID::DYNAMIC_CONTRAST), 0));
    EXPECT_EQ(0,test_FeatureSwitch(static_cast<PQFeatureID>(PQFeatureID::DYNAMIC_CONTRAST), 1));
}

TEST (PQTest, PQ_005)
{
#ifdef MDP_COLOR_ENABLE
    EXPECT_EQ(0,test_FeatureSwitch(static_cast<PQFeatureID>(PQFeatureID::CONTENT_COLOR_VIDEO), 0));
    EXPECT_EQ(0,test_FeatureSwitch(static_cast<PQFeatureID>(PQFeatureID::CONTENT_COLOR_VIDEO), 1));
#endif
}


