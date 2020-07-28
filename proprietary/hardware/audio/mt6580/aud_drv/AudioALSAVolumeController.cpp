#include <math.h>
#include "AudioALSAVolumeController.h"
#include "SpeechEnhancementController.h"
#include "SpeechDriverFactory.h"
#include "audio_custom_exp.h"
#include "AudioVIBSPKControl.h"
#include "AudioUtility.h"
#include "AudioALSADriverUtility.h"
#include "AudioVolumeFactory.h"


#include "AudioALSAHardwareResourceManager.h"

#include "AudioALSASpeechPhoneCallController.h"


#define LOG_TAG "AudioALSAVolumeController"

#include <utils/Log.h>

#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))

namespace android {

AudioALSAVolumeController *AudioALSAVolumeController::UniqueVolumeInstance = NULL;
static const char* PROPERTY_KEY_SPH_DRC_VER = "persist.vendor.audiohal.sph_drc_ver";
static const char *PGA_Gain_String[] = {"-6Db", "0Db", "6Db", "12Db", "18Db", "24Db"};      //6dB/step
static const char *DL_PGA_Gain_String[] = {"0Db", "6Db", "12Db", "18Db", "24Db", "30Db"};
//static const char *DAC_SampleRate_function[] = {"8000", "11025", "16000", "24000", "32000", "44100", "48000"};
static const char *DL_PGA_Headset_GAIN[] = {"9Db", "7Db", "5Db", "3Db", "1Db", "-1Db", "-3Db", "-5Db"};

static const char *DL_PGA_Handset_GAIN[] = {"-21Db", "-19Db", "-17Db", "-15Db", "-13Db", "-11Db", "-9Db", "-7Db", "-5Db", "-3Db", "-1Db", "1Db",
                                            "3Db", "5Db", "7Db", "9Db"
                                           };

//not use on 82
static const char *DL_PGA_LINEOUT_GAIN[] = {"8Db", "7Db", "6Db", "5Db", "4Db", "3Db", "2Db", "1Db", "0Db", "-1Db", "-2Db", "-3Db",
                                            "-4Db", "-5Db", "-6Db", "-7Db", "-8Db", "-9Db", "-10Db", "-40Db"
                                           };
//speaker gain
static const char *DL_PGA_SPEAKER_GAIN[] = {"MUTE", "0Db", "4Db", "5Db", "6Db", "7Db", "8Db", "9Db", "10Db",
                                            "11Db", "12Db", "13Db", "14Db", "15Db", "16Db", "17Db"
                                           };

// here can change to match audiosystem
#if 1
// total 64 dB
static const float dBPerStep = 0.25f;
static const float VOLUME_MAPPING_STEP = 256.0f;
#else
static const float dBPerStep = 0.5f;
static const float VOLUME_MAPPING_STEP = 100.0f;
#endif

// shouldn't need to touch these
static const float dBConvert = -dBPerStep * 2.302585093f / 20.0f;
static const float dBConvertInverse = 1.0f / dBConvert;

/* 9~-5dB */
#define  AUDIO_BUFFER_HW_GAIN_STEP (14)
#define  UL_PGA_GAIN_OFFSET (2)

#define  GAIN_STEP (2)
#define  RECEIVER_BUFFER_DB_OFFSET (-21)
#define  HEADPHONE_BUFFER_DB_OFFSET (9)
#define  SPEAKER_PGAGAIN_DB_OFFSET (2)
#define  LINE_OUT_BUFFER_ODB_INDEX (8)


#ifdef EVDO_DT_VEND_SUPPORT
#define  VOICE_EVDO_HP_HS_EP 8
#define  VOICE_EVDO_SPK 14
#endif

static const uint16_t SideToneTable[] = {
    32767, 29204, 26027, 23196, 20674, 18426, 16422, 14636, 13044, 11625,  /*1dB per step*/
    10361, 9234,  8230,  7335,  6537,  5826,  5193,  4628,  4125,  3676,
    3276,  2919,  2602,  2319,  2066,  1841,  1641,  1463,  1304,  1162,
    1035,  923,   822,   733,   653,   582,   519,   462,   412,   367,
    327,   291,   260,   231,   206,   183,   163,   145
};


static const uint16_t SwAgc_Gain_Map[AUDIO_SYSTEM_UL_GAIN_MAX + 1] = {
    19, 18, 17, 16, 15, 14, 13, 12, 11,
    16, 15, 14, 13, 12, 11,
    16, 15, 14, 13, 12, 11,
    16, 15, 14, 13, 12, 11,
    16, 15, 14, 13, 12, 11,
    16, 15, 14, 13, 12, 11,
    10, 9, 8, 7, 6, 5, 4
};

static const uint16_t PGA_Gain_Map[AUDIO_SYSTEM_UL_GAIN_MAX + 1] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    6, 6, 6, 6, 6, 6,
    12, 12, 12, 12, 12, 12,
    18, 18, 18, 18, 18, 18,
    24, 24, 24, 24, 24, 24,
    30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30
};

static const uint16_t Dmic_SwAgc_Gain_Map[AUDIO_SYSTEM_UL_GAIN_MAX + 1] = {
    30, 29, 28, 27, 26, 25, 24, 23, 22,
    21, 20, 19, 18, 17, 16,
    15, 14, 13, 12, 11, 10,
    9, 8, 7, 6, 5, 4,
    3, 2, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0
};

static const uint16_t Dmic_PGA_Gain_Map[AUDIO_SYSTEM_UL_GAIN_MAX + 1] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    6, 6, 6, 6, 6, 6,
    12, 12, 12, 12, 12, 12,
    18, 18, 18, 18, 18, 18,
    24, 24, 24, 24, 24, 24,
    30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30
};


//DRC1.0 mapping
static const uint16_t DLPGA_Gain_Map_Ver1[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    0, 0, 2, 2, 4, 4, 6, 6, 8, 8,
    10, 10, 12, 12, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14
};

static const uint16_t DlEnh1_Gain_Map_Ver1[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const uint16_t DlDigital_Gain_Map_Ver1[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
    0, 1, 0, 1, 0, 1, 2, 3, 4, 5,
    6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26
};

static const uint16_t DLPGA_HeadsetGain_Map_Incall[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};



static const uint16_t DlDigital_HeadsetGain_Map_Incall[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40
};

#ifdef USE_PREV_DESINGED
static const uint16_t DLPGA_HeadsetGain_Map_Ver1[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
    14, 15, 16, 17, 18, 18, 18, 18, 18, 18,
    18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
    18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18
};

static const uint16_t DlEnh1_HeadsetGain_Map_Ver1[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const uint16_t DlDigital_HeadsetGain_Map_Ver1[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 3, 4, 5, 6, 7,
    8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28
};

static const uint16_t Extspk_DlDigital_Gain_Map[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40
};

static const uint16_t Intspk_DlDigital_Gain_Map[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    0, 0, 0, 0, 0, 0, 1, 2, 3, 4,
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35
};

static const uint16_t Intspk_DLPGA_Gain_Map[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    0, 1, 2, 3, 4, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
};

#ifdef MTK_AUDIO_EXTCODEC_SUPPORT
#ifdef HP_USING_EXTDAC
static const uint16_t DlDigital_Gain_Map_InEXTDACModeVer1[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
    35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45
};
#endif
#endif

#endif

//DRC2.0 mapping
static const uint16_t DLPGA_Gain_Map_Ver2[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    0, 0, 2, 2, 4, 4, 6, 6, 8, 8,
    8, 10, 10, 10, 12, 12, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14
};

static const uint16_t DlEnh1_Gain_Map_Ver2[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 4, 4, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

static const uint16_t DlDigital_Gain_Map_Ver2[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
    1, 0, 1, 1, 0, 1, 0, 1, 2, 3,
    4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
    14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24
};

#ifdef USE_PREV_DESINGED

//2IN1 speaker mapping
static const uint16_t DLPGA_Gain_Map_Ver3[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    9, 10, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11
};

static const uint16_t DlEnh1_Gain_Map_Ver3[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const uint16_t DlDigital_Gain_Map_Ver3[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    0, 0, 0, 1, 2, 3, 4, 5, 6, 7,
    8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
    28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38
};
#endif

// HW Gain mappring
static const uint32_t kHWGainMap[] = {
    0x00000, //   0, -64.0 dB (mute)
    0x0015E, //   1, -63.5 dB
    0x00173, //   2, -63.0 dB
    0x00189, //   3, -62.5 dB
    0x001A0, //   4, -62.0 dB
    0x001B9, //   5, -61.5 dB
    0x001D3, //   6, -61.0 dB
    0x001EE, //   7, -60.5 dB
    0x0020C, //   8, -60.0 dB
    0x0022B, //   9, -59.5 dB
    0x0024C, //  10, -59.0 dB
    0x0026F, //  11, -58.5 dB
    0x00294, //  12, -58.0 dB
    0x002BB, //  13, -57.5 dB
    0x002E4, //  14, -57.0 dB
    0x00310, //  15, -56.5 dB
    0x0033E, //  16, -56.0 dB
    0x00370, //  17, -55.5 dB
    0x003A4, //  18, -55.0 dB
    0x003DB, //  19, -54.5 dB
    0x00416, //  20, -54.0 dB
    0x00454, //  21, -53.5 dB
    0x00495, //  22, -53.0 dB
    0x004DB, //  23, -52.5 dB
    0x00524, //  24, -52.0 dB
    0x00572, //  25, -51.5 dB
    0x005C5, //  26, -51.0 dB
    0x0061D, //  27, -50.5 dB
    0x00679, //  28, -50.0 dB
    0x006DC, //  29, -49.5 dB
    0x00744, //  30, -49.0 dB
    0x007B2, //  31, -48.5 dB
    0x00827, //  32, -48.0 dB
    0x008A2, //  33, -47.5 dB
    0x00925, //  34, -47.0 dB
    0x009B0, //  35, -46.5 dB
    0x00A43, //  36, -46.0 dB
    0x00ADF, //  37, -45.5 dB
    0x00B84, //  38, -45.0 dB
    0x00C32, //  39, -44.5 dB
    0x00CEC, //  40, -44.0 dB
    0x00DB0, //  41, -43.5 dB
    0x00E7F, //  42, -43.0 dB
    0x00F5B, //  43, -42.5 dB
    0x01044, //  44, -42.0 dB
    0x0113B, //  45, -41.5 dB
    0x01240, //  46, -41.0 dB
    0x01355, //  47, -40.5 dB
    0x0147A, //  48, -40.0 dB
    0x015B1, //  49, -39.5 dB
    0x016FA, //  50, -39.0 dB
    0x01857, //  51, -38.5 dB
    0x019C8, //  52, -38.0 dB
    0x01B4F, //  53, -37.5 dB
    0x01CED, //  54, -37.0 dB
    0x01EA4, //  55, -36.5 dB
    0x02075, //  56, -36.0 dB
    0x02261, //  57, -35.5 dB
    0x0246B, //  58, -35.0 dB
    0x02693, //  59, -34.5 dB
    0x028DC, //  60, -34.0 dB
    0x02B48, //  61, -33.5 dB
    0x02DD9, //  62, -33.0 dB
    0x03090, //  63, -32.5 dB
    0x03371, //  64, -32.0 dB
    0x0367D, //  65, -31.5 dB
    0x039B8, //  66, -31.0 dB
    0x03D24, //  67, -30.5 dB
    0x040C3, //  68, -30.0 dB
    0x04499, //  69, -29.5 dB
    0x048AA, //  70, -29.0 dB
    0x04CF8, //  71, -28.5 dB
    0x05188, //  72, -28.0 dB
    0x0565D, //  73, -27.5 dB
    0x05B7B, //  74, -27.0 dB
    0x060E6, //  75, -26.5 dB
    0x066A4, //  76, -26.0 dB
    0x06CB9, //  77, -25.5 dB
    0x0732A, //  78, -25.0 dB
    0x079FD, //  79, -24.5 dB
    0x08138, //  80, -24.0 dB
    0x088E0, //  81, -23.5 dB
    0x090FC, //  82, -23.0 dB
    0x09994, //  83, -22.5 dB
    0x0A2AD, //  84, -22.0 dB
    0x0AC51, //  85, -21.5 dB
    0x0B687, //  86, -21.0 dB
    0x0C157, //  87, -20.5 dB
    0x0CCCC, //  88, -20.0 dB
    0x0D8EF, //  89, -19.5 dB
    0x0E5CA, //  90, -19.0 dB
    0x0F367, //  91, -18.5 dB
    0x101D3, //  92, -18.0 dB
    0x1111A, //  93, -17.5 dB
    0x12149, //  94, -17.0 dB
    0x1326D, //  95, -16.5 dB
    0x14496, //  96, -16.0 dB
    0x157D1, //  97, -15.5 dB
    0x16C31, //  98, -15.0 dB
    0x181C5, //  99, -14.5 dB
    0x198A1, // 100, -14.0 dB
    0x1B0D7, // 101, -13.5 dB
    0x1CA7D, // 102, -13.0 dB
    0x1E5A8, // 103, -12.5 dB
    0x2026F, // 104, -12.0 dB
    0x220EA, // 105, -11.5 dB
    0x24134, // 106, -11.0 dB
    0x26368, // 107, -10.5 dB
    0x287A2, // 108, -10.0 dB
    0x2AE02, // 109,  -9.5 dB
    0x2D6A8, // 110,  -9.0 dB
    0x301B7, // 111,  -8.5 dB
    0x32F52, // 112,  -8.0 dB
    0x35FA2, // 113,  -7.5 dB
    0x392CE, // 114,  -7.0 dB
    0x3C903, // 115,  -6.5 dB
    0x4026E, // 116,  -6.0 dB
    0x43F40, // 117,  -5.5 dB
    0x47FAC, // 118,  -5.0 dB
    0x4C3EA, // 119,  -4.5 dB
    0x50C33, // 120,  -4.0 dB
    0x558C4, // 121,  -3.5 dB
    0x5A9DF, // 122,  -3.0 dB
    0x5FFC8, // 123,  -2.5 dB
    0x65AC8, // 124,  -2.0 dB
    0x6BB2D, // 125,  -1.5 dB
    0x72148, // 126,  -1.0 dB
    0x78D6F, // 127,  -0.5 dB
    0x80000, // 128,   0.0 dB
};


#ifdef MTK_AUDIO_EXTCODEC_SUPPORT
#ifdef HP_USING_EXTDAC
static const uint16_t DlDigital_Gain_Map_InEXTDACModeVer2[(VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1] = {
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    14, 15, 16, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 48, 49, 40, 41, 42, 43
};
#endif
#endif

/* input:  DL_PGA_Gain          (dB/step)         */
/*         MMI_Sidetone_Volume  (dB/8 steps)        */
/*         SW_AGC_Ul_Gain       (dB/step)         */
/* output: DSP_ST_GAIN          (dB/step)          */

uint16_t AudioALSAVolumeController::UpdateSidetone(int DL_PGA_Gain, int  Sidetone_Volume, uint8_t SW_AGC_Ul_Gain) {
    uint16_t sidetone_vol = 0;
    int vol = 0;
    uint16_t DSP_ST_GAIN = 0;

    ALOGD("UpdateSidetone DL_PGA_Gain = %d MMI_Sidetone_Volume = %d SW_AGC_Ul_Gain = %d",
          DL_PGA_Gain, Sidetone_Volume, SW_AGC_Ul_Gain);

    vol = Sidetone_Volume + SW_AGC_Ul_Gain; //1dB/step
    vol = DL_PGA_Gain - vol + 67 - UL_PGA_GAIN_OFFSET;
    ALOGD("vol = %d", vol);
    if (vol < 0) {
        vol = 0;
    }
    if (vol > 47) {
        vol = 47;
    }
    DSP_ST_GAIN = SideToneTable[vol]; //output 1dB/step
    if (Sidetone_Volume == 0) {
        DSP_ST_GAIN = 0 ;
    }
    ALOGD("DSP_ST_GAIN = %d", DSP_ST_GAIN);
    return DSP_ST_GAIN;
}

float AudioALSAVolumeController::linearToLog(int volume) {
    return volume ? exp(float(VOLUME_MAPPING_STEP - volume) * dBConvert) : 0;
}

int AudioALSAVolumeController::logToLinear(float volume) {
    return volume ? VOLUME_MAPPING_STEP - int(dBConvertInverse * log(volume) + 0.5) : 0;
}

AudioALSAVolumeController *AudioALSAVolumeController::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);
    if (UniqueVolumeInstance == 0) {
        ALOGD("+UniqueVolumeInstance\n");
        UniqueVolumeInstance = new AudioALSAVolumeController();
        ALOGD("-UniqueVolumeInstance\n");
    }
    return UniqueVolumeInstance;
}

AudioALSAVolumeController::AudioALSAVolumeController() :
    mHardwareResourceManager(AudioALSAHardwareResourceManager::getInstance()),
    mAudioSpeechEnhanceInfoInstance(AudioSpeechEnhanceInfo::getInstance()) {
    ALOGD("AudioALSAVolumeController contructor\n");

    mVoiceVolume = 1.0f;
    mMasterVolume = 1.0f;
    mMixer = NULL;
    mHeadPhoneImpedence = 32; // general hheadphone impedence
    for (size_t i = 0; i < AUDIO_STREAM_CNT; ++i) {
        mStreamVolume[i] = 1.0f;
    }
    mInitDone = false;
    mSwAgcGain = 12;
    mULTotalGain = 184;
    mHeadPhoneImpedenceEnable = false;
    mMode = AUDIO_MODE_NORMAL;
    mOutputDevices = AUDIO_DEVICE_OUT_SPEAKER;

    mMixer = AudioALSADriverUtility::getInstance()->getMixer();
    ALOGD("mMixer = %p", mMixer);
    ASSERT(mMixer != NULL);

    initCheck();
}

status_t AudioALSAVolumeController::initCheck() {
    ALOGD("AudioALSAVolumeController initCheck\n");
    if (!mInitDone) {
        initVolumeController();
        mInitDone = true;
    }
    return NO_ERROR;
}

bool AudioALSAVolumeController::SetVolumeRange(uint32_t mode, int32_t MaxVolume, int32_t MinVolume, int32_t VolumeRange) {
    if (mode >= Num_of_Audio_gain) {
        ALOGD("SetVolumeRange mode >= Num_of_Audio_gain");
        return false;
    }
    mVolumeMax[mode] = MaxVolume;
    mVolumeMin[mode] = MinVolume;
    mVolumeRange[mode] = VolumeRange;
    //#if defined(MTK_VIBSPK_SUPPORT)
    if (mode == Audio_Speaker) {
        AudioVIBSPKControl::getInstance()->setVibSpkGain(MaxVolume, MinVolume, VolumeRange);
    }
    //#endif
    ALOGD("SetVolumeRange mode=%d, MaxVolume=%d, MinVolume=%d VolumeRange = %d", mode, MaxVolume, MinVolume, VolumeRange);
    return true;
}

uint32_t AudioALSAVolumeController::SortHeadPhoneImpedance(uint32_t Impedance) {
#ifdef USE_PREV_DESINGED
    ALOGD("SortHeadPhoneImpedance Impedance = %d", Impedance);
    if (Impedance < 24) {
        return HEADPHONE_IMPEDANCE_16;
    } else if (Impedance < 48) {
        return HEADPHONE_IMPEDANCE_32;
    } else if (Impedance < 96) {
        return HEADPHONE_IMPEDANCE_64;
    } else if (Impedance < 192) {
        return HEADPHONE_IMPEDANCE_128;
    } else if (Impedance <= 512) {
        return HEADPHONE_IMPEDANCE_256;
    }
    return HEADPHONE_IMPEDANCE_16;
#else
    ALOGD("SortHeadPhoneImpedance not support");
    return HEADPHONE_IMPEDANCE_16;
#endif
}

bool AudioALSAVolumeController::GetHeadPhoneImpedance(void) {
#ifdef USE_PREV_DESINGED
    ALOGD("GetHeadPhoneImpedance mHeadPhoneImpedenceEnable = %d mHeadPhoneImpedence= %d ",
          mHeadPhoneImpedenceEnable, mHeadPhoneImpedence);
    if (mHeadPhoneImpedenceEnable == false) {
        mHeadPhoneImpedence =  HEADPHONE_IMPEDANCE_32;
    } else {
        struct mixer_ctl *ctl;
        enum mixer_ctl_type type;
        unsigned int num_values, i ;
        ctl = mixer_get_ctl_by_name(mMixer, "Audio HP ImpeDance Setting");
        type = mixer_ctl_get_type(ctl);
        num_values = mixer_ctl_get_num_values(ctl);
        for (i = 0; i < num_values; i++) {
            mHeadPhoneImpedence = mixer_ctl_get_value(ctl, i);
            ALOGD("GetHeadPhoneImpedance i = %d mHeadPhoneImpedence = %d ", i, mHeadPhoneImpedence);
        }
        mHeadPhoneImpedence = SortHeadPhoneImpedance(mHeadPhoneImpedence);
        ALOGD("mHeadPhoneImpedence = %d ", mHeadPhoneImpedence);
    }
    // update volume setting
    setMasterVolume();
#else
    ALOGD("GetHeadPhoneImpedance not support ");
#endif
    return true;
}

bool AudioALSAVolumeController::GetHeadPhoneImpedanceEnable(void) {
    return mHeadPhoneImpedenceEnable;
}

int  AudioALSAVolumeController::MapHeadPhoneImpedance(void) {
    switch (mHeadPhoneImpedence) {
    case HEADPHONE_IMPEDANCE_16:
        return 3;
    case HEADPHONE_IMPEDANCE_32:
        return 0;
    case HEADPHONE_IMPEDANCE_64:
        return  -3;
    case HEADPHONE_IMPEDANCE_128:
        return  -6;
    case HEADPHONE_IMPEDANCE_256:
        return  -9;
    default:
        return 0;
    }
    return 0;
}

void AudioALSAVolumeController::ApplyMdDlGain(int32_t degradeDb) {
    // set degarde db to mode side, DL part, here use degrade dbg
#ifdef EVDO_DT_VEND_SUPPORT  //vend EVDO use volume level
    modem_index_t modem_index = SpeechDriverFactory::GetInstance()->GetActiveModemIndex();
    ALOGD("ApplyMdDlGain degradeDb = %d to modem=%d", degradeDb, modem_index);
    if (modem_index == MODEM_EXTERNAL) {
        return; //skip vend evdo modem
    }
#endif
    ALOGD("ApplyMdDlGain degradeDb = %d", degradeDb);
    SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetDownlinkGain((-1 * degradeDb) << 2); // degrade db * 4
}

void AudioALSAVolumeController::ApplyMdDlEhn1Gain(int32_t Gain) {
    // set degarde db to mode side, DL part, here use degrade dbg
#ifdef EVDO_DT_VEND_SUPPORT  //vend EVDO use volume level
    modem_index_t modem_index = SpeechDriverFactory::GetInstance()->GetActiveModemIndex();
    ALOGD("ApplyMdDlGain Gain = %d to modem=%d", Gain, modem_index);
    if (modem_index == MODEM_EXTERNAL) {
        return; //skip vend evdo modem
    }
#endif
    ALOGD("ApplyMdDlEhn1Gain degradeDb = %d", Gain);
    SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetEnh1DownlinkGain(-1 * (Gain) << 2); // degrade db * 4
}


void AudioALSAVolumeController::ApplyMdUlGain(int32_t IncreaseDb) {
    // set degarde db to mode side, UL part, here use positive gain becasue SW_agc always positive
    ALOGD("ApplyMdUlGain degradeDb = %d", IncreaseDb);
    SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetUplinkGain(IncreaseDb << 2); // degrade db * 4
}

bool AudioALSAVolumeController::IsHeadsetMicInput(uint32_t device) {
    //check mic with headset or normal mic.
    if (device == Idle_Headset_Record || device == Voice_Rec_Mic_Headset || device == Idle_Video_Record_Headset ||
        device == Headset_Mic  || device == VOIP_Headset_Mic) {
        return true;
    } else {
        return false;
    }
}

//this function map 255 ==> Audiocustomvolume
static float MampVoiceBufferVolume(unsigned char Volume) {
    if (Volume > VOICE_VOLUME_MAX) {
        Volume = VOICE_VOLUME_MAX;
    }
    float DegradedB = VOICE_VOLUME_MAX - Volume;
    DegradedB = DegradedB / AUDIO_TWODB_STEP; // how many 2dB degrade(MT6323 HP and HS are 2 db step)

    ALOGD("Volume = %d MampVoiceVolume DegradedB = %f ", Volume, DegradedB);
    return DegradedB * 2 ;
}

//this function map 255 ==> Audiocustomvolume
static float MampAudioBufferVolume(unsigned char Volume) {
    if (Volume > AUDIO_VOLUME_MAX) {
        Volume = AUDIO_VOLUME_MAX;
    }
    float DegradedB = AUDIO_VOLUME_MAX - Volume;
    DegradedB = DegradedB / AUDIO_TWODB_STEP; // how many 2dB degrade(MT6323 HP and HS are 2 db step)

    ALOGD("Volume = %d MampAudioVolume DegradedB = %f ", Volume, DegradedB);
    return DegradedB * 2 ;
}

//this function map 255 ==> Audiocustomvolume
static float MampSPKAMPVolume(unsigned char Volume) {
    if (Volume > AMP_VOLUME_MAX) {
        Volume = AMP_VOLUME_MAX;
    }
    float DegradedB = AMP_VOLUME_MAX - Volume;
    DegradedB = DegradedB / AMP_ONEDB_STEP; // how many dB degrade

    ALOGD("Volume = %d MampSpkAmpVolume DegradedB = %f ", Volume, DegradedB);
    // for volume peroformance ,start with 15
    return (DegradedB);
}


//this function map 255 ==> Audiocustomvolume
static float MampUplinkGain(unsigned char Volume) {
    if (Volume > UPLINK_GAIN_MAX) {
        Volume = UPLINK_GAIN_MAX;
    }
    float DegradedB = UPLINK_GAIN_MAX - Volume;
    DegradedB = DegradedB / UPLINK_ONEDB_STEP; // how many dB degrade

    ALOGD("Volume = %d UPLINK_GAIN_MAX DegradedB = %f ", Volume, DegradedB);
    return DegradedB;
}


uint16_t AudioALSAVolumeController::MappingToDigitalGain(unsigned char Gain) {
    uint16_t DegradedBGain = (uint16_t)MampUplinkGain(Gain);

    // bounded systen total gain
    if (DegradedBGain > AUDIO_SYSTEM_UL_GAIN_MAX) {
        DegradedBGain = AUDIO_SYSTEM_UL_GAIN_MAX;
    }

    return SwAgc_Gain_Map[DegradedBGain];
}

uint16_t AudioALSAVolumeController::MappingToPGAGain(unsigned char Gain) {
    uint16_t DegradedBGain = (uint16_t)MampUplinkGain(Gain);

    // bounded systen total gain
    if (DegradedBGain > AUDIO_SYSTEM_UL_GAIN_MAX) {
        DegradedBGain = AUDIO_SYSTEM_UL_GAIN_MAX;
    }
    return PGA_Gain_Map[DegradedBGain];
}


status_t AudioALSAVolumeController::setFmVolume(const float fm_volume) {
    ALOGV("%s(), fm_volume = %f", __FUNCTION__, fm_volume);

    // Calculate HW Gain Value
    uint32_t volume_index = logToLinear(fm_volume); // 0 ~ 256
    uint32_t hw_gain = kHWGainMap[volume_index >> 1]; // 0 ~ 0x80000

    // Set HW Gain
    return mHardwareResourceManager->setHWGain2DigitalGain(hw_gain);
}

uint32_t AudioALSAVolumeController::GetOffloadGain(float vol_f) {
    return 0;
}

status_t AudioALSAVolumeController::initVolumeController() {
    ALOGD("AudioALSAVolumeController initVolumeController\n");
    AudioCustParamClient *pAudioCustParamClient = AudioCustParamClient::GetInstance();
    pAudioCustParamClient->GetVolumeVer1ParamFromNV(&mVolumeParam);
    pAudioCustParamClient->GetNBSpeechParamFromNVRam(&mSphParamNB);
    pAudioCustParamClient->GetWBSpeechParamFromNVRam(&mSphParamWB);
#if defined(MTK_HAC_SUPPORT)
    ALOGD("LoadCustomVolume mHacParam");
    pAudioCustParamClient->GetHACSpeechParamFromNVRam(&mHacParam);
    bool mHACon = false;
#endif

    for (int i = 0 ; i < NORMAL_VOLUME_TYPE_MAX ; i++) {
        ALOGD("normalaudiovolume %d = %d", i, mVolumeParam.normalaudiovolume[i]);
    }
    for (int i = 0 ; i < HEADSET_VOLUME_TYPE_MAX ; i++) {
        ALOGD("headsetaudiovolume %d = %d", i, mVolumeParam.headsetaudiovolume[i]);
    }
    for (int i = 0 ; i < SPEAKER_VOLUME_TYPE_MAX ; i++) {
        ALOGD("speakeraudiovolume %d = %d", i, mVolumeParam.speakeraudiovolume[i]);
    }
    for (int i = 0 ; i < HEADSET_SPEAKER_VOLUME_TYPE_MAX ; i++) {
        ALOGD("headsetspeakeraudiovolume %d = %d", i, mVolumeParam.headsetspeakeraudiovolume[i]);
    }
    for (int i = 0 ; i < VER1_NUM_OF_VOL_TYPE ; i++) {
        ALOGD("audiovolume_level %d = %d", i, mVolumeParam.audiovolume_level[i]);
    }

    for (int i = 0 ; i < AUDIO_MAX_VOLUME_STEP ; i++) {
        ALOGD("mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][%d] = %d", i,
              mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][i]);
    }
    for (int i = 0 ; i < AUDIO_MAX_VOLUME_STEP ; i++) {
        ALOGD("mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][%d] = %d", i,
              mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][i]);
    }
    for (int i = 0 ; i < AUDIO_MAX_VOLUME_STEP ; i++) {
        ALOGD("mVolumeParam.audiovolume_mic[VOLUME_SPEAKER_MODE][%d] = %d", i,
              mVolumeParam.audiovolume_mic[VOLUME_SPEAKER_MODE][i]);
    }

    for (int i = 0 ; i < AUDIO_MAX_VOLUME_STEP ; i++) {
        ALOGD("normalaudiovolume %d = %d", i, mVolumeParam.audiovolume_media[VOLUME_HEADSET_MODE][i]);
    }

    // for normal platyback , let audio drvier can achevie maximun volume , and let computecustomvolume to
    // set mastervolume

    float MaxdB = 0.0, MindB = 0.0, micgain = 0.0;
    int degradegain = 0;
    degradegain = (unsigned char)MampVoiceBufferVolume(mVolumeParam.normalaudiovolume[NORMAL_AUDIO_BUFFER]);

    SetVolumeRange(Audio_Earpiece, DEVICE_VOICE_MAX_VOLUME, DEVICE_VOICE_MIN_VOLUME, degradegain);

    degradegain = (unsigned char)MampAudioBufferVolume(mVolumeParam.headsetaudiovolume[HEADSET_AUDIO_BUFFER]);
    SetVolumeRange(Audio_Headset, DEVICE_MAX_VOLUME, DEVICE_MIN_VOLUME, degradegain);
    SetVolumeRange(Audio_Headphone, DEVICE_MAX_VOLUME, DEVICE_MIN_VOLUME, degradegain);
#ifdef USING_EXTAMP_HP
    degradegain = (unsigned char)MampAudioBufferVolume(mVolumeParam.speakeraudiovolume[SPEAKER_AMP]);
#else
    degradegain = (unsigned char)MampSPKAMPVolume(mVolumeParam.speakeraudiovolume[SPEAKER_AMP]);
#endif
    SetVolumeRange(Audio_Speaker, DEVICE_AMP_MAX_VOLUME, DEVICE_AMP_MIN_VOLUME, degradegain);

    degradegain = (unsigned char)MampVoiceBufferVolume(mVolumeParam.normalaudiovolume[NORMAL_AUDIO_BUFFER]);
    SetVolumeRange(Audio_DualMode_Earpiece, DEVICE_VOICE_MAX_VOLUME, DEVICE_VOICE_MIN_VOLUME, degradegain);
    degradegain = (unsigned char)MampAudioBufferVolume(mVolumeParam.headsetspeakeraudiovolume[HEADSET_SPEAKER_AUDIO_BUFFER]);
    SetVolumeRange(Audio_DualMode_Headset, DEVICE_MAX_VOLUME, DEVICE_MIN_VOLUME, degradegain);
    SetVolumeRange(Audio_DualMode_Headphone, DEVICE_MAX_VOLUME, DEVICE_MIN_VOLUME, degradegain);
    degradegain = (unsigned char)MampSPKAMPVolume(mVolumeParam.headsetspeakeraudiovolume[HEADSET_SPEAKER_AMP]);
    SetVolumeRange(Audio_DualMode_speaker, DEVICE_AMP_MAX_VOLUME, DEVICE_AMP_MIN_VOLUME, degradegain);

    degradegain = (unsigned char)MampVoiceBufferVolume(mVolumeParam.normalaudiovolume[NORMAL_AUDIO_BUFFER]);
    SetVolumeRange(Ringtone_Earpiece, DEVICE_VOICE_MAX_VOLUME, DEVICE_VOICE_MIN_VOLUME, degradegain);
    degradegain = (unsigned char)MampAudioBufferVolume(mVolumeParam.headsetaudiovolume[HEADSET_AUDIO_BUFFER]);
    SetVolumeRange(Ringtone_Headset, DEVICE_MAX_VOLUME, DEVICE_MIN_VOLUME, degradegain);
    SetVolumeRange(Ringtone_Headphone, DEVICE_MAX_VOLUME, DEVICE_MIN_VOLUME, degradegain);
    degradegain = (unsigned char)MampSPKAMPVolume(mVolumeParam.speakeraudiovolume[HEADSET_SPEAKER_AMP]);
    SetVolumeRange(Ringtone_Speaker, DEVICE_AMP_MAX_VOLUME, DEVICE_AMP_MIN_VOLUME, degradegain);

    degradegain = (unsigned char)MampVoiceBufferVolume(mVolumeParam.normalaudiovolume[NORMAL_SIP_AUDIO_BUFFER]);
    SetVolumeRange(Sipcall_Earpiece, DEVICE_VOICE_MAX_VOLUME, DEVICE_VOICE_MIN_VOLUME, degradegain);
    degradegain = (unsigned char)MampAudioBufferVolume(mVolumeParam.headsetaudiovolume[HEADSET_SIP_AUDIO_BUFFER]);
    SetVolumeRange(Sipcall_Headset, DEVICE_MAX_VOLUME, DEVICE_MIN_VOLUME, degradegain);
    SetVolumeRange(Sipcall_Headphone, DEVICE_MAX_VOLUME, DEVICE_MIN_VOLUME, degradegain);
    degradegain = (unsigned char)MampSPKAMPVolume(mVolumeParam.speakeraudiovolume[SPEAKER_SIP_AUDIO_BUFFER]);
    SetVolumeRange(Sipcall_Speaker, DEVICE_AMP_MAX_VOLUME, DEVICE_AMP_MIN_VOLUME, degradegain);

    //-----MIC VOLUME SETTING
    ALOGD(" not define MTK_AUDIO_GAIN_TABLE_SUPPORT");
    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][4]);
    SetULTotalGain(Idle_Normal_Record, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][4]);
    SetMicGain(Idle_Normal_Record,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][4]);
    SetULTotalGain(Idle_Headset_Record, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][4]);
    SetMicGain(Idle_Headset_Record, degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][3]);
    SetULTotalGain(Normal_Mic, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][3]);
    SetMicGain(Normal_Mic,         degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][3]);
    SetULTotalGain(Headset_Mic, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][3]);
    SetMicGain(Headset_Mic,       degradegain);


    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_SPEAKER_MODE][3]);
    SetULTotalGain(Handfree_Mic, mVolumeParam.audiovolume_mic[VOLUME_SPEAKER_MODE][3]);
    SetMicGain(Handfree_Mic,     degradegain);


    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][0]);
    SetULTotalGain(TTY_CTM_Mic, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][0]);
    SetMicGain(TTY_CTM_Mic,     degradegain);

    // voice reconition usage
    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][5]);
    SetULTotalGain(Voice_Rec_Mic_Handset, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][5]);
    SetMicGain(Voice_Rec_Mic_Handset,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][5]);
    SetULTotalGain(Voice_Rec_Mic_Headset, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][5]);
    SetMicGain(Voice_Rec_Mic_Headset,  degradegain);

    // add by chiepeng for VOIP mode
    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][6]);
    SetULTotalGain(VOIP_Normal_Mic, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][6]);
    SetMicGain(VOIP_Normal_Mic,     degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][6]);
    SetULTotalGain(VOIP_Headset_Mic, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][6]);
    SetMicGain(VOIP_Headset_Mic,   degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_SPEAKER_MODE][6]);
    SetULTotalGain(VOIP_Handfree_Mic, mVolumeParam.audiovolume_mic[VOLUME_SPEAKER_MODE][6]);
    SetMicGain(VOIP_Handfree_Mic, degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][2]);
    SetULTotalGain(Idle_Video_Record_Handset, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][2]);
    SetMicGain(Idle_Video_Record_Handset,   degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][2]);
    SetULTotalGain(Idle_Video_Record_Headset, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][2]);
    SetMicGain(Idle_Video_Record_Headset,  degradegain);

    // voice unlock usage (input source AUDIO_SOURCE_VOICE_UNLOCK)
    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][7]);
    SetULTotalGain(Voice_UnLock_Mic_Handset, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][7]);
    SetMicGain(Voice_UnLock_Mic_Handset,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][7]);
    SetULTotalGain(Voice_UnLock_Mic_Headset, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][7]);
    SetMicGain(Voice_UnLock_Mic_Headset,  degradegain);

    //MIC gain for customization input source usage
    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][8]);
    SetULTotalGain(Customization1_Mic_Handset, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][8]);
    SetMicGain(Customization1_Mic_Handset,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][8]);
    SetULTotalGain(Customization1_Mic_Headset, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][8]);
    SetMicGain(Customization1_Mic_Headset,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][9]);
    SetULTotalGain(Customization2_Mic_Handset, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][9]);
    SetMicGain(Customization2_Mic_Handset,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][9]);
    SetULTotalGain(Customization2_Mic_Headset, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][9]);
    SetMicGain(Customization2_Mic_Headset,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][10]);
    SetULTotalGain(Customization3_Mic_Handset, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][10]);
    SetMicGain(Customization3_Mic_Handset,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][10]);
    SetULTotalGain(Customization3_Mic_Headset, mVolumeParam.audiovolume_mic[VOLUME_HEADSET_MODE][10]);
    SetMicGain(Customization3_Mic_Headset,  degradegain);

    degradegain = (unsigned char)MampUplinkGain(mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][11]);
    SetULTotalGain(Individual_2ndSet_Mic, mVolumeParam.audiovolume_mic[VOLUME_NORMAL_MODE][11]);
    SetMicGain(Individual_2ndSet_Mic,  degradegain);

    for (int i = 0; i < Num_Mic_Gain ; i++) {
        ALOGD("micgain %d = %d", i, mMicGain[i]);
    }

    // here save sidewtone gain to msidetone

#if defined(MTK_HAC_SUPPORT)
    mHACon = SpeechEnhancementController::GetInstance()->GetHACOn();

    if (mHACon) {
        ALOGD("%s(): mHACon=%d", __FUNCTION__, mHACon);
        SetSideTone(EarPiece_SideTone_Gain, mHacParam.audiovolume_sid_hac[3]);
    } else
#endif
    {
        SetSideTone(EarPiece_SideTone_Gain, mVolumeParam.audiovolume_sid[VOLUME_NORMAL_MODE][3]);
    }
    SetSideTone(Headset_SideTone_Gain, mVolumeParam.audiovolume_sid[VOLUME_HEADSET_MODE][3]);
    SetSideTone(LoudSpk_SideTone_Gain, mVolumeParam.audiovolume_sid[VOLUME_SPEAKER_MODE][3]);
    mSpeechDrcType  =  MagiLoudness_TE_mode;

    return NO_ERROR;
}

void AudioALSAVolumeController::SetReceiverGain(int DegradedBGain) {
    if (DegradedBGain >= (_countof(DL_PGA_Handset_GAIN) - 1)) {
        DegradedBGain = _countof(DL_PGA_Handset_GAIN) - 1;
    }
    // this will base on hw spec.
    uint32_t index = 15;
    //const int HWgain[] =  {-21, -19, -17, -15, -13, -11, -9, -7, -5, -3, -1, 1, 3, 5, 7, 9};
    DegradedBGain = DegradedBGain / GAIN_STEP;

    index -= DegradedBGain;
    if (index < 8) {
        index = 8;    // Min bounded at -5 dB
    }
    ALOGD("SetReceiverGain, index = %d, DegradedBGain = %d", index, DegradedBGain);
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    ctl = mixer_get_ctl_by_name(mMixer, "Handset_PGA_GAIN");
    type = mixer_ctl_get_type(ctl);
    if (mixer_ctl_set_enum_by_string(ctl, DL_PGA_Handset_GAIN[index])) {
        ALOGE("Error: Handset_PGA_GAIN invalid value");
    }
}
int GetReceiverGain(void);

void AudioALSAVolumeController::SetHeadPhoneLGain(int DegradedBGain) {
    DegradedBGain = DegradedBGain / GAIN_STEP;
    if (DegradedBGain >= (_countof(DL_PGA_Headset_GAIN) - 1)) {
        DegradedBGain = _countof(DL_PGA_Headset_GAIN) - 1;
    }
    ALOGD("SetHeadPhoneLGain = %d", DegradedBGain);
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    ctl = mixer_get_ctl_by_name(mMixer, "Headset_PGAL_GAIN");
    type = mixer_ctl_get_type(ctl);
    if (mixer_ctl_set_enum_by_string(ctl, DL_PGA_Headset_GAIN[DegradedBGain])) {
        ALOGE("Error: Headset_PGAL_GAIN invalid value");
    }
}

void AudioALSAVolumeController::SetHeadPhoneRGain(int DegradedBGain) {
    DegradedBGain = DegradedBGain / GAIN_STEP;
    if (DegradedBGain >= (_countof(DL_PGA_Headset_GAIN) - 1)) {
        DegradedBGain = _countof(DL_PGA_Headset_GAIN) - 1;
    }
    ALOGD("SetHeadPhoneRGain = %d", DegradedBGain);
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    ctl = mixer_get_ctl_by_name(mMixer, "Headset_PGAR_GAIN");
    type = mixer_ctl_get_type(ctl);
    if (DegradedBGain < 0) {
        DegradedBGain = 0;
    }
    if (mixer_ctl_set_enum_by_string(ctl, DL_PGA_Headset_GAIN[DegradedBGain])) {
        ALOGE("Error: Headset_PGAL_GAIN invalid value");
    }
}

void AudioALSAVolumeController::SetLinoutRGain(int DegradedBGain) {
#ifdef USE_PREV_DESINGED
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    ctl = mixer_get_ctl_by_name(mMixer, "Lineout_PGAR_GAIN");
    type = mixer_ctl_get_type(ctl);
    if (DegradedBGain < 0) {
        DegradedBGain = 0;
    }

    if ((uint32_t)DegradedBGain >= _countof(DL_PGA_LINEOUT_GAIN)) {
        DegradedBGain = _countof(DL_PGA_LINEOUT_GAIN) - 1;
    }

    ALOGD("SetRealLinoutRGain = %d", DegradedBGain);
    if (mixer_ctl_set_enum_by_string(ctl, DL_PGA_LINEOUT_GAIN[DegradedBGain])) {
        ALOGE("Error: SetLinoutLGain invalid value");
    }
#else
    ALOGD("SetLinoutRGain not suppot\n");
#endif
}

void AudioALSAVolumeController::SetLinoutLGain(int DegradedBGain) {
#ifdef USE_PREV_DESINGED
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    ctl = mixer_get_ctl_by_name(mMixer, "Lineout_PGAL_GAIN");
    type = mixer_ctl_get_type(ctl);
    if (DegradedBGain < 0) {
        DegradedBGain = 0;
    }

    if ((uint32_t)DegradedBGain >= _countof(DL_PGA_LINEOUT_GAIN)) {
        DegradedBGain = _countof(DL_PGA_LINEOUT_GAIN) - 1;
    }

    ALOGD("SetRealLinoutRGain = %d", DegradedBGain);
    if (mixer_ctl_set_enum_by_string(ctl, DL_PGA_LINEOUT_GAIN[DegradedBGain])) {
        ALOGE("Error: SetLinoutLGain invalid value");
    }
#else
    ALOGD("SetLinoutRGain not support\n");
#endif
}

void AudioALSAVolumeController::SetSpeakerGain(int DegradedBGain) {
    ALOGD("SetLSpeakerGain,DegradedBGain=%d \n", DegradedBGain);
    if (DegradedBGain >= (_countof(DL_PGA_SPEAKER_GAIN) - 1)) {
        DegradedBGain = _countof(DL_PGA_SPEAKER_GAIN) - 1;
    }
    // this will base on hw spec, use 15dB for
    uint32_t index =  13;
    // condition for gainb not mute
    if (DegradedBGain > 11) {
        DegradedBGain = 11;
    }
    //const int HWgain[] =  {-60,0,4,5,6,7,8,9,10,11,12,13,14,15,16,17};
    index -= DegradedBGain;
    if (index < 1) {
        index = 1; // min to 0dB
    }
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    ctl = mixer_get_ctl_by_name(mMixer, "Audio_Speaker_PGA_gain");
    type = mixer_ctl_get_type(ctl);
    if (mixer_ctl_set_enum_by_string(ctl, DL_PGA_SPEAKER_GAIN[index])) {
        ALOGE("Error: SetSpeakerGain invalid value");
    }
}

void AudioALSAVolumeController::SetAdcPga1(int DegradedBGain) {
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    int num_values = 0;
    ctl = mixer_get_ctl_by_name(mMixer, "Audio_PGA1_Setting");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);
    if (DegradedBGain < 0) {
        DegradedBGain = 0;
    }

    if ((uint32_t)DegradedBGain >= _countof(PGA_Gain_String)) {
        DegradedBGain = _countof(PGA_Gain_String) - 1;
    }

    ALOGD("SetRealAdcPga1 = %d", DegradedBGain);
    if (mixer_ctl_set_enum_by_string(ctl, PGA_Gain_String[DegradedBGain])) {
        ALOGE("Error: Audio_PGA1_Setting invalid value");
    }
}
void AudioALSAVolumeController::SetAdcPga2(int DegradedBGain) {
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    int num_values = 0;

    ctl = mixer_get_ctl_by_name(mMixer, "Audio_PGA2_Setting");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);
    if (DegradedBGain < 0) {
        DegradedBGain = 0;
    }

    if ((uint32_t)DegradedBGain >= _countof(PGA_Gain_String)) {
        DegradedBGain = _countof(PGA_Gain_String) - 1;
    }

    ALOGD("SetRealAdcPga2 = %d", DegradedBGain);
    if (mixer_ctl_set_enum_by_string(ctl, PGA_Gain_String[DegradedBGain])) {
        ALOGE("Error: Audio_PGA2_Setting invalid value");
    }
}
void AudioALSAVolumeController::SetAdcPga3(int DegradedBGain) {
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    int num_values = 0;

    ctl = mixer_get_ctl_by_name(mMixer, "Audio_PGA3_Setting");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);
    if (DegradedBGain < 0) {
        DegradedBGain = 0;
    }

    if ((uint32_t)DegradedBGain >= _countof(PGA_Gain_String)) {
        DegradedBGain = _countof(PGA_Gain_String) - 1;
    }

    ALOGD("SetRealAdcPga3 = %d", DegradedBGain);

    if (mixer_ctl_set_enum_by_string(ctl, PGA_Gain_String[DegradedBGain])) {
        ALOGE("Error: Audio_PGA3_Setting invalid value");
    }
}
void AudioALSAVolumeController::SetAdcPga4(int DegradedBGain) {
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    int num_values = 0;

    ctl = mixer_get_ctl_by_name(mMixer, "Audio_PGA4_Setting");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);

    if (DegradedBGain < 0) {
        DegradedBGain = 0;
    }

    if ((uint32_t)DegradedBGain >= _countof(PGA_Gain_String)) {
        DegradedBGain = _countof(PGA_Gain_String) - 1;
    }

    ALOGD("SetRealAdcPga4 = %d", DegradedBGain);

    if (mixer_ctl_set_enum_by_string(ctl, PGA_Gain_String[DegradedBGain])) {
        ALOGE("Error: Audio_PGA4_Setting invalid value");
    }
}

int AudioALSAVolumeController::GetReceiverGain(void) {
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_values, i ;
    int gain = 0;
    ALOGD("GetReceiverGain");
    ctl = mixer_get_ctl_by_name(mMixer, "Handset_PGA_GAIN");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);
    for (i = 0; i < num_values; i++) {
        gain = mixer_ctl_get_value(ctl, i);
        ALOGD("GetReceiverGain i = %d gain index = %d ", i, gain);
    }
    //return dB value
    return RECEIVER_BUFFER_DB_OFFSET + gain * GAIN_STEP;
    //const int HWgain[] =  {-21, -19, -17, -15, -13, -11, -9, -7, -5, -3, -1, 1, 3, 5, 7, 9};
}
int AudioALSAVolumeController::GetHeadphoneRGain(void) {
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_values, i ;
    int gain = 0;
    ALOGD("GetHeadphoneRGain");
    ctl = mixer_get_ctl_by_name(mMixer, "Headset_PGAR_GAIN");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);
    for (i = 0; i < num_values; i++) {
        gain = mixer_ctl_get_value(ctl, i);
        ALOGD("GetHeadphoneRGain i = %d gain = %d ", i, gain);
    }
    //return dB value
    return HEADPHONE_BUFFER_DB_OFFSET - gain * GAIN_STEP;
}
int AudioALSAVolumeController::GetHeadphoneLGain(void) {
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_values, i ;
    int gain = 0;
    ALOGD("GetHeadphoneLGain");
    ctl = mixer_get_ctl_by_name(mMixer, "Headset_PGAL_GAIN");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);
    for (i = 0; i < num_values; i++) {
        gain = mixer_ctl_get_value(ctl, i);
        ALOGD("GetHeadphoneLGain i = %d gain = %d ", i, gain);
    }
    //return dB value
    return HEADPHONE_BUFFER_DB_OFFSET - gain * GAIN_STEP;
}
int AudioALSAVolumeController::GetLineOutphoneRGain(void) {
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_values, i ;
    int gain = 0;
    ALOGD("GetLineOutphoneRGain");
    ctl = mixer_get_ctl_by_name(mMixer, "Lineout_PGAR_GAIN");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);
    for (i = 0; i < num_values; i++) {
        gain = mixer_ctl_get_value(ctl, i);
        ALOGD("GetLineOutphoneRGain i = %d gain = %d ", i, gain);
    }
    return LINE_OUT_BUFFER_ODB_INDEX - gain;

}
int AudioALSAVolumeController::GetLineOutphoneLGain(void) {
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_values, i ;
    int gain = 0;
    ALOGD("GetLineOutphoneLGain");
    ctl = mixer_get_ctl_by_name(mMixer, "Lineout_PGAL_GAIN");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);
    for (i = 0; i < num_values; i++) {
        gain = mixer_ctl_get_value(ctl, i);
        ALOGD("GetLineOutphoneLGain i = %d gain = %d ", i, gain);
    }
    return LINE_OUT_BUFFER_ODB_INDEX - gain ;
}

int AudioALSAVolumeController::GetSPKGain(void) {
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_values, i ;
    int gain = 0;
    int RetMapGain;
    ALOGD("GetSPKGain");
    ctl = mixer_get_ctl_by_name(mMixer, "Audio_Speaker_PGA_gain");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);
    for (i = 0; i < num_values; i++) {
        gain = mixer_ctl_get_value(ctl, i);
        ALOGD("GetSPKGain i = %d gain = %d ", i, gain);
    }
    // for 0 is mute...
    if (gain == 0) {
        RetMapGain = -1;
    } else if (gain == 1) { //0dB
        RetMapGain = 0;
    } else {
        RetMapGain = gain + SPEAKER_PGAGAIN_DB_OFFSET;
    }
    return RetMapGain;
}

int AudioALSAVolumeController::ApplyHpimpedanceGain(int DegradedBGain) {
    ALOGD("ApplyHpimpedanceGain DegradedBGain = %d", DegradedBGain);
    if (mHeadPhoneImpedenceEnable == false) {
        return DegradedBGain;
    }
    switch (mHeadPhoneImpedence) {
    case HEADPHONE_IMPEDANCE_16:
        return (DegradedBGain + 3);
    case HEADPHONE_IMPEDANCE_32:
        return (DegradedBGain);
    case HEADPHONE_IMPEDANCE_64:
        return (DegradedBGain - 3);
    case  HEADPHONE_IMPEDANCE_128:
        return (DegradedBGain - 6);
    case HEADPHONE_IMPEDANCE_256:
        return (DegradedBGain - 9);
    default:
        return DegradedBGain;
    }
    return DegradedBGain;
}

// cal and set and set analog gain
void AudioALSAVolumeController::ApplyAudioGain(int Gain, uint32_t mode, uint32_t device) {
    ALOGD("ApplyAudioGain  Gain = %d mode= %d device = %d", Gain, mode, device);
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    struct mixer_ctl *ctl1;
    enum mixer_ctl_type type1;

    if (device >=  Num_of_Audio_gain) {
        ALOGW(" Calgain out of boundary mode = %d device = %0x%x", mode, device);
        return;
    }

    int DegradedBGain = mVolumeRange[device];
    DegradedBGain = DegradedBGain + (DEVICE_VOLUME_RANGE - DegradedBGain) * ((VOLUME_MAPPING_STEP - Gain) / VOLUME_MAPPING_STEP);
    ALOGD("ApplyAudioGain  DegradedBGain = %d mVolumeRange[mode] = %d ", DegradedBGain, mVolumeRange[device]);
    if (device  ==  Audio_Earpiece || device == Audio_DualMode_Earpiece || device == Sipcall_Earpiece) {
        SetReceiverGain(DegradedBGain);
    } else if ((device  == Audio_Headset) || (device == Audio_Headphone) || (device == Sipcall_Headset) || (device == Sipcall_Headphone)) {
        ALOGD("ApplyAudioGain Audio_Headset\n");
#ifdef USE_PREV_DESINGED    //no headphone impedance
        if (GetHeadPhoneImpedanceEnable() == true) {
            DegradedBGain += MapHeadPhoneImpedance();
            ALOGD("GetHeadPhoneImpedanceEnable DegradedBGain = %d ", DegradedBGain);

            SetHeadPhoneLGain(DegradedBGain);
            SetHeadPhoneRGain(DegradedBGain);
        } else
#endif
        {
            SetHeadPhoneLGain(DegradedBGain);
            SetHeadPhoneRGain(DegradedBGain);
        }
    } else if ((device  == Audio_DualMode_Headset) || (device == Audio_DualMode_Headphone)) {
        SetHeadPhoneLGain(DegradedBGain);
        SetHeadPhoneRGain(DegradedBGain);
    } else if (device == Audio_Speaker) {
        ALOGD("ApplyAudioGain Audio_Speaker\n");
        if (DegradedBGain >= (_countof(DL_PGA_LINEOUT_GAIN) - 1)) {
            DegradedBGain = _countof(DL_PGA_LINEOUT_GAIN) - 1;
        }
        SetLinoutLGain(DegradedBGain);
        SetLinoutRGain(DegradedBGain);
    }
}


// cal and set and set analog gain
void AudioALSAVolumeController::ApplyAmpGain(int Gain, uint32_t mode, uint32_t device) {
    ALOGD("ApplyAmpGain  Gain = %d mode= %d device = %d", Gain, mode, device);
    if (device >= Num_of_Audio_gain) {
        ALOGW(" Calgain out of boundary mode = %d device = %0x%x", mode, device);
    }
    int DegradedBGain = mVolumeRange[device];
    DegradedBGain = DegradedBGain + (DEVICE_VOLUME_RANGE - DegradedBGain) * ((VOLUME_MAPPING_STEP - Gain) / VOLUME_MAPPING_STEP);
    // set line out buffer to 0Db.
#ifdef USE_PREV_DESINGED
    SetLinoutLGain(LINE_OUT_BUFFER_ODB_INDEX);
    SetLinoutRGain(LINE_OUT_BUFFER_ODB_INDEX);
#endif
    SetSpeakerGain(DegradedBGain);
}

// cal and set and set analog gain
void AudioALSAVolumeController::ApplyExtAmpHeadPhoneGain(int Gain, uint32_t mode, uint32_t device) {
    ALOGD("ApplyExtAmpHeadPhoneGain  Gain = %d mode= %d device = %d", Gain, mode, device);
    if (device >= Num_of_Audio_gain) {
        ALOGW(" Calgain out of boundary mode = %d device = %0x%x", mode, device);
        return;
    }
    int DegradedBGain = mVolumeRange[device];
    DegradedBGain = DegradedBGain + (DEVICE_VOLUME_RANGE - DegradedBGain) * ((VOLUME_MAPPING_STEP - Gain) / VOLUME_MAPPING_STEP);
    ALOGD("DegradedBGain   = %d", DegradedBGain);

#ifdef USE_PREV_DESINGED
    if (DegradedBGain >= (_countof(DL_PGA_LINEOUT_GAIN) - 1)) {
        DegradedBGain = _countof(DL_PGA_LINEOUT_GAIN) - 1;
    }
    SetLinoutLGain(DegradedBGain);
    SetLinoutRGain(DegradedBGain);
#endif

    SetHeadPhoneLGain(DegradedBGain);
    SetHeadPhoneRGain(DegradedBGain);
}

// cal and set and set analog gain
void AudioALSAVolumeController::ApplyDualmodeGain(int Gain, uint32_t mode, uint32_t devices) {
    ALOGD("ApplyDualmodeGain gain = %d mode = %d devices = %d", Gain, mode, devices);
    if (devices  == Audio_DualMode_speaker) {
        ApplyAmpGain(Gain,  mode, Audio_DualMode_speaker);
    }
    if (devices == Audio_DualMode_Earpiece) {
        ApplyAudioGain(Gain,  mode, Audio_DualMode_Earpiece);
    }
    if (devices == Audio_DualMode_Headset) {
        ApplyAudioGain(Gain,  mode, Audio_DualMode_Headset);
    }
    if (devices == Audio_DualMode_Headphone) {
        ApplyAudioGain(Gain,  mode, Audio_DualMode_Headphone);
    }
}

status_t AudioALSAVolumeController::setMasterVolume(void) {
    return setMasterVolume(mMasterVolume, mMode, mOutputDevices);
}

status_t AudioALSAVolumeController::setMasterVolume(float v, audio_mode_t mode, uint32_t devices) {
    ALOGD("AudioALSAVolumeController setMasterVolume v = %f mode = %d devices = 0x%x", v, mode, devices);
    int MapVolume = AudioALSAVolumeController::logToLinear(v);
    mMasterVolume = v;
    mMode = mode;
    mOutputDevices = devices;
    switch (mode) {
    case AUDIO_MODE_NORMAL : { // normal mode
        if (popcount(devices) == 1) {
            switch (devices) {
            case (AUDIO_DEVICE_OUT_EARPIECE): {
                ApplyAudioGain(MapVolume,  mode, Audio_Earpiece);
                break;
            }
            case (AUDIO_DEVICE_OUT_WIRED_HEADSET): {
                ApplyAudioGain(MapVolume,  mode, Audio_Headset);
                break;
            }
            case (AUDIO_DEVICE_OUT_WIRED_HEADPHONE): {
                ApplyAudioGain(MapVolume,  mode, Audio_Headphone);
                break;
            }
            case (AUDIO_DEVICE_OUT_SPEAKER) : {
#ifdef USING_EXTAMP_HP
                ApplyExtAmpHeadPhoneGain(MapVolume,  mode, Audio_Speaker);
#else
                ApplyAmpGain(MapVolume,  mode, Audio_Speaker);
#endif
                break;
            }
            default: {
                ALOGD("setMasterVolume with device = 0x%x", devices);
                break;
            }
            }
        }
        // pop device is more than one , should use dual mode.
        else {
            ApplyDualmodeGain(MapVolume,  mode,  Audio_DualMode_Headphone);
            ApplyDualmodeGain(MapVolume,  mode,  Audio_DualMode_speaker);
        }
        break;
    }
    case AUDIO_MODE_RINGTONE : {
        if (popcount(devices) == 1) {
            switch (devices) {
            case (AUDIO_DEVICE_OUT_EARPIECE): {
                ApplyAudioGain(MapVolume,  mode, Ringtone_Earpiece);
                break;
            }
            case (AUDIO_DEVICE_OUT_WIRED_HEADSET): {
                ApplyAudioGain(MapVolume,  mode, Ringtone_Headset);
                break;
            }
            case (AUDIO_DEVICE_OUT_WIRED_HEADPHONE): {
                ApplyAudioGain(MapVolume,  mode, Ringtone_Headphone);
                break;
            }
            case (AUDIO_DEVICE_OUT_SPEAKER) : {
#ifdef USING_EXTAMP_HP
                ApplyExtAmpHeadPhoneGain(MapVolume,  mode, Audio_Speaker);
#else
                ApplyAmpGain(MapVolume,  mode, Audio_Speaker);
#endif
                break;
            }
            default: {
                ALOGD("setMasterVolume with device = 0x%x", devices);
                break;
            }
            }
        }
        // pop device is more than one , should use dual mode.
        else {
            ALOGD("AudioALSAVolumeController setMasterVolume with dual mode");
            ApplyDualmodeGain(MapVolume,  mode,  Audio_DualMode_Headphone);
            ApplyDualmodeGain(MapVolume,  mode,  Audio_DualMode_speaker);
        }
        break;
    }
    case AUDIO_MODE_IN_CALL: {
        ALOGW("set mastervolume with in call ~~~~");
        default:
            break;
        }
    case AUDIO_MODE_IN_COMMUNICATION : {
        if (popcount(devices) == 1) {
            switch (devices) {
            case (AUDIO_DEVICE_OUT_EARPIECE): {
                ApplyAudioGain(MapVolume, mode, Sipcall_Earpiece);
                break;
            }
            case (AUDIO_DEVICE_OUT_WIRED_HEADSET): {
                ApplyAudioGain(MapVolume, mode, Sipcall_Headset);
                break;
            }
            case (AUDIO_DEVICE_OUT_WIRED_HEADPHONE): {
                ApplyAudioGain(MapVolume, mode, Sipcall_Headphone);
                break;
            }
            case (AUDIO_DEVICE_OUT_SPEAKER) : {
#ifdef USING_EXTAMP_HP
                ApplyExtAmpHeadPhoneGain(MapVolume,  mode, Sipcall_Speaker);
#else
                ApplyAmpGain(MapVolume,  mode, Sipcall_Speaker);
#endif
                break;
            }
            default: {
                ALOGD("setMasterVolume with device = 0x%x", devices);
                break;
            }
            }
        }
        // pop device is more than one , should use dual mode.
        else {
            ApplyDualmodeGain(MapVolume,  mode,  Audio_DualMode_Headphone);
            ApplyDualmodeGain(MapVolume,  mode,  Audio_DualMode_speaker);
        }
        break;
    }
    }

    return NO_ERROR;
}

float AudioALSAVolumeController::getMasterVolume() {
    ALOGD("AudioALSAVolumeController getMasterVolume");
    return mMasterVolume;
}

bool AudioALSAVolumeController::ModeSetVoiceVolume(int mode) {
    return (mode == AUDIO_MODE_IN_CALL);
}

uint32_t AudioALSAVolumeController::GetDRCVersion(uint32_t device) {
    // todo , get volume.
    int DrcSpeechModeBits = 0;

    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_SPH_DRC_VER, property_value, "0");
    int Sph_Drc_Version = atoi(property_value);
    if (Sph_Drc_Version) {
        ALOGD("change mSpeechDrcType to Sph_Drc_Version = %d", Sph_Drc_Version);
    }

    if (device & AUDIO_DEVICE_OUT_EARPIECE) {
        DrcSpeechModeBits = 1;
    } else if (device & AUDIO_DEVICE_OUT_WIRED_HEADSET ||  device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
        DrcSpeechModeBits = 1 << 1;
    } else if (device & AUDIO_DEVICE_OUT_SPEAKER) {
        DrcSpeechModeBits = 1 << 2;
    }
    ALOGD("GetDRCVersion DrcSpeechModeBits = %d device= 0x%x mSpeechDrcType = 0x%x Sph_Drc_Version = 0x%x", DrcSpeechModeBits, device, mSpeechDrcType, Sph_Drc_Version);
    if ((mSpeechDrcType & DrcSpeechModeBits) || (Sph_Drc_Version & DrcSpeechModeBits)) {
        ALOGD("DRC_VERSION_2");
        return DRC_VERSION_2;
    } else {
        ALOGD("DRC_VERSION_1");
        return DRC_VERSION_1;
    }
}

void AudioALSAVolumeController::SetInternalSpkGain(int degradeDb) {
    ALOGD("SetInternalSpkGain degradeDb = %d", degradeDb);
    int DRCversion = GetDRCVersion(AUDIO_DEVICE_OUT_SPEAKER);
    int DigitalgradeDb = 0;  // digital gain
    int Enh1degradeDb = 0;  // digital enh1 gain
    int VoiceAnalogRange = 0;
    modem_index_t modem_index = SpeechDriverFactory::GetInstance()->GetActiveModemIndex();

    if (VoiceAnalogRange >= (_countof(DL_PGA_SPEAKER_GAIN) - 1)) {
        VoiceAnalogRange = _countof(DL_PGA_SPEAKER_GAIN) - 1;
    }
    //Rainier using
    degradeDb += 3; //At the same EM, voice buffer gain for speaker should be lower 3 dB than for HS/HP
    ALOGD("SetInternalSpkGain after degradeDb = %d", degradeDb);

#ifdef EVDO_DT_VEND_SUPPORT  //vend EVDO use volume level
    if (modem_index != MODEM_EXTERNAL) {
#endif
        if (degradeDb > (VOICE_VOLUME_MAX / VOICE_ONEDB_STEP)) {
            degradeDb  = (VOICE_VOLUME_MAX / VOICE_ONEDB_STEP);
        }
        if (DRCversion == DRC_VERSION_1) {
            VoiceAnalogRange = DLPGA_Gain_Map_Ver1[degradeDb];
            DigitalgradeDb =  DlDigital_Gain_Map_Ver1[degradeDb];
            Enh1degradeDb = DlEnh1_Gain_Map_Ver1[degradeDb];
        } else {
            VoiceAnalogRange = DLPGA_Gain_Map_Ver2[degradeDb];
            DigitalgradeDb = DlDigital_Gain_Map_Ver2[degradeDb];
            Enh1degradeDb = DlEnh1_Gain_Map_Ver2[degradeDb];
        }
#ifdef EVDO_DT_VEND_SUPPORT
    }
#endif
    ALOGD("DigitalgradeDb = %d Enh1degradeDb = %d VoiceAnalogRange = %d ", DigitalgradeDb, Enh1degradeDb, VoiceAnalogRange);

    SetLinoutLGain(LINE_OUT_BUFFER_ODB_INDEX);
    SetLinoutRGain(LINE_OUT_BUFFER_ODB_INDEX);
#ifdef USING_EXTAMP_HP
    SetHeadPhoneRGain(VoiceAnalogRange);
    SetHeadPhoneLGain(VoiceAnalogRange);
#else
    //Rainier using
    SetSpeakerGain(3);
    SetReceiverGain(VoiceAnalogRange);
#endif
    ApplyMdDlGain(DigitalgradeDb);
    ApplyMdDlEhn1Gain(Enh1degradeDb);
}

void AudioALSAVolumeController::SetExternalSpkGain(int degradeDb) {
#ifdef USE_PREV_DESINGED
    ALOGD("SetExternalSpkGain degradeDb = %d", degradeDb);
    int DigitalgradeDb = 0;  // digital gain
    int Enh1degradeDb = 0;  // digital enh1 gain
    modem_index_t modem_index = SpeechDriverFactory::GetInstance()->GetActiveModemIndex();
#ifdef EVDO_DT_VEND_SUPPORT  //vend EVDO use volume level
    if (modem_index != MODEM_EXTERNAL) {
#endif
        if (degradeDb > (VOICE_VOLUME_MAX / VOICE_ONEDB_STEP)) {
            degradeDb  = (VOICE_VOLUME_MAX / VOICE_ONEDB_STEP);
        }
        DigitalgradeDb =  Extspk_DlDigital_Gain_Map[degradeDb];
        Enh1degradeDb = DlEnh1_Gain_Map_Ver1[degradeDb];

#ifdef EVDO_DT_VEND_SUPPORT
    }
#endif
    ALOGD("DigitalgradeDb = %d Enh1degradeDb = %d ", DigitalgradeDb, Enh1degradeDb);
    ApplyMdDlGain(DigitalgradeDb);
    ApplyMdDlEhn1Gain(Enh1degradeDb);
#else
    ALOGD("SetExternalSpkGain degradeDb = %d, not support", degradeDb);
#endif
}

void AudioALSAVolumeController::Set2IN1InternalSpkGain(int degradeDb) {
#ifdef USE_PREV_DESINGED

    ALOGD("Set2IN1InternalSpkGain degradeDb = %d", degradeDb);
    //int DRCversion = GetDRCVersion(AUDIO_DEVICE_OUT_SPEAKER);
    int DigitalgradeDb = 0;  // digital gain
    int Enh1degradeDb = 0;  // digital enh1 gain
    int VoiceAnalogRange = 0;
    modem_index_t modem_index = SpeechDriverFactory::GetInstance()->GetActiveModemIndex();

    if (VoiceAnalogRange >= (_countof(DL_PGA_SPEAKER_GAIN) - 1)) {
        VoiceAnalogRange = _countof(DL_PGA_SPEAKER_GAIN) - 1;
    }
#ifdef EVDO_DT_VEND_SUPPORT  //vend EVDO use volume level
    if (modem_index != MODEM_EXTERNAL) {
#endif
        if (degradeDb > (VOICE_VOLUME_MAX / VOICE_ONEDB_STEP)) {
            degradeDb  = (VOICE_VOLUME_MAX / VOICE_ONEDB_STEP);
        }

        VoiceAnalogRange = DLPGA_Gain_Map_Ver3[degradeDb];
        DigitalgradeDb =  DlDigital_Gain_Map_Ver3[degradeDb];
        Enh1degradeDb = DlEnh1_Gain_Map_Ver3[degradeDb];

#ifdef EVDO_DT_VEND_SUPPORT
    }
#endif
    ALOGD("DigitalgradeDb = %d Enh1degradeDb = %d VoiceAnalogRange = %d ", DigitalgradeDb, Enh1degradeDb, VoiceAnalogRange);

    SetLinoutLGain(LINE_OUT_BUFFER_ODB_INDEX);
    SetLinoutRGain(LINE_OUT_BUFFER_ODB_INDEX);
    SetSpeakerGain(VoiceAnalogRange);
    ApplyMdDlGain(DigitalgradeDb);
    ApplyMdDlEhn1Gain(Enh1degradeDb);
#else
    ALOGD("Set2IN1InternalSpkGain degradeDb = %d, not support", degradeDb);
#endif
}


void AudioALSAVolumeController::Set2IN1ExternalSpkGain(int degradeDb) {
#ifdef USE_PREV_DESINGED
    ALOGD("Set2IN1ExternalSpkGain degradeDb = %d", degradeDb);
    int DigitalgradeDb = 0;  // digital gain
    int Enh1degradeDb = 0;  // digital enh1 gain
    modem_index_t modem_index = SpeechDriverFactory::GetInstance()->GetActiveModemIndex();
#ifdef EVDO_DT_VEND_SUPPORT  //vend EVDO use volume level
    if (modem_index != MODEM_EXTERNAL) {
#endif
        if (degradeDb > (VOICE_VOLUME_MAX / VOICE_ONEDB_STEP)) {
            degradeDb  = (VOICE_VOLUME_MAX / VOICE_ONEDB_STEP);
        }
        DigitalgradeDb =  Extspk_DlDigital_Gain_Map[degradeDb];
        Enh1degradeDb = DlEnh1_Gain_Map_Ver1[degradeDb];

#ifdef EVDO_DT_VEND_SUPPORT
    }
#endif
    ALOGD("DigitalgradeDb = %d Enh1degradeDb = %d ", DigitalgradeDb, Enh1degradeDb);
    ApplyMdDlGain(DigitalgradeDb);
    ApplyMdDlEhn1Gain(Enh1degradeDb);
#else
    ALOGD("Set2IN1ExternalSpkGain degradeDb = %d, not support", degradeDb);
#endif
}


status_t AudioALSAVolumeController::setVoiceVolume(float v, audio_mode_t mode, uint32_t device) {
    ALOGD("+%s(), v=%f, mode=0x%x, device=0x%x", __FUNCTION__, v, mode, device);
    mVoiceVolume = v;
    int MapVolume = 0;
    int DRCversion = GetDRCVersion(device);
    int degradeDb = 0;

    if (ModeSetVoiceVolume(mode) == false) {
        return INVALID_OPERATION;
    }

    // set drc version to modem side.
    SpeechEnhancementController::GetInstance()->SetDynamicMaskOnToAllModem(SPH_ENH_DYNAMIC_MASK_SIDEKEY_DGAIN, (bool)DRCversion);

    MapVolume = AudioALSAVolumeController::logToLinear(v);
    degradeDb = (DEVICE_VOLUME_STEP - MapVolume) / VOICE_ONEDB_STEP;

    if (degradeDb > (VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1) {
        degradeDb = (VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1;
        ALOGE("%s(), degradeDb over max index, set to max", __FUNCTION__);
    } else if (degradeDb < 0) {
        degradeDb = 0;
        ALOGE("%s(), degradeDb under min index, set to 0", __FUNCTION__);
    }

    ApplyVoiceGain(degradeDb, mode, device);
    return NO_ERROR;

}

status_t AudioALSAVolumeController::setVoiceVolume(int MapVolume, uint32_t device) {
    ALOGD("+%s(), MapVolume=0x%x, device=0x%x", __FUNCTION__, MapVolume, device);
    int DRCversion = GetDRCVersion(device);
    int degradeDb = 0;

    // set drc version to modem side.
    SpeechEnhancementController::GetInstance()->SetDynamicMaskOnToAllModem(SPH_ENH_DYNAMIC_MASK_SIDEKEY_DGAIN, (bool)DRCversion);

    degradeDb = (VOICE_VOLUME_MAX - MapVolume) / VOICE_ONEDB_STEP;//(160-144)/4=4
    ApplyVoiceGain(degradeDb, AUDIO_MODE_IN_CALL, device);
    return NO_ERROR;

}


status_t AudioALSAVolumeController::ApplyVoiceGain(int degradeDb, audio_mode_t mode, uint32_t device) {
    int DRCversion = GetDRCVersion(device);
    int DigitalgradeDb = 0;  // digital gain
    int Enh1degradeDb = 0;  // digital enh1 gain
    int VoiceAnalogRange = 0;
    ALOGD("+%s(), degradeDb=0x%x, mode=0x%x, device=0x%x, DRCversion=0x%x", __FUNCTION__, degradeDb, mode, device, DRCversion);
    if (DRCversion == DRC_VERSION_1) {
        VoiceAnalogRange = DLPGA_Gain_Map_Ver1[degradeDb];
        DigitalgradeDb = DlDigital_Gain_Map_Ver1[degradeDb];
        Enh1degradeDb = DlEnh1_Gain_Map_Ver1[degradeDb];
    } else {
        VoiceAnalogRange = DLPGA_Gain_Map_Ver2[degradeDb];
        DigitalgradeDb = DlDigital_Gain_Map_Ver2[degradeDb];
        Enh1degradeDb = DlEnh1_Gain_Map_Ver2[degradeDb];
    }
#ifdef EVDO_DT_VEND_SUPPORT  //vend EVDO use volume level
    modem_index_t modem_index = SpeechDriverFactory::GetInstance()->GetActiveModemIndex();
    if (modem_index == MODEM_EXTERNAL) {
        ALOGD("SpeechDriver ModemType=VendEVDO");
        if (device & (AUDIO_DEVICE_OUT_EARPIECE | AUDIO_DEVICE_OUT_WIRED_HEADSET | AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
            VoiceAnalogRange = VOICE_EVDO_HP_HS_EP;  //setting volume to 1db
        } else if (device & AUDIO_DEVICE_OUT_SPEAKER) {
            VoiceAnalogRange = VOICE_EVDO_SPK;   //setting volume to -5db
        } else {
            VoiceAnalogRange = VOICE_EVDO_HP_HS_EP;  //setting volume to maximum
        }
        ALOGD("VoiceAnalogRange = %d MapVolume = %d (after adjust for vend evdo)", VoiceAnalogRange, MapVolume);
        Enh1degradeDb = 0;
        DigitalgradeDb = 0;
    }
#endif

    if (device & AUDIO_DEVICE_OUT_EARPIECE) {
        ALOGD("DigitalgradeDb = %d Enh1degradeDb = %d VoiceAnalogRange = %d ", DigitalgradeDb, Enh1degradeDb, VoiceAnalogRange);

        if (IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER)) {
            ApplyMdDlGain(DigitalgradeDb);
            ApplyMdDlEhn1Gain(Enh1degradeDb);
            SetReceiverGain(VoiceAnalogRange);
        } else {
            ApplyMdDlGain(DigitalgradeDb);
            ApplyMdDlEhn1Gain(Enh1degradeDb);
            SetReceiverGain(VoiceAnalogRange);
        }
        if (AudioALSASpeechPhoneCallController::getInstance()->checkTtyNeedOn() == false) {
            ApplyMicGain(Normal_Mic, mode); // set incall mic gain
        }
    }
    if (device & AUDIO_DEVICE_OUT_WIRED_HEADSET ||  device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {

        ALOGD("DigitalgradeDb = %d Enh1degradeDb = %d VoiceAnalogRange = %d ", DigitalgradeDb, Enh1degradeDb, VoiceAnalogRange);
#if 0
        if (VoiceAnalogRange >= (_countof(DL_PGA_Handset_GAIN) - 1)) {
            VoiceAnalogRange = _countof(DL_PGA_Handset_GAIN) - 1;
        }
#endif

        VoiceAnalogRange = DLPGA_HeadsetGain_Map_Incall[degradeDb];
        DigitalgradeDb = DlDigital_HeadsetGain_Map_Incall[degradeDb];

        if (ModeSetVoiceVolume(mode) == true) { //HP switch, phone call mode always use internal DAC
            ApplyMdDlGain(DigitalgradeDb);
            ApplyMdDlEhn1Gain(Enh1degradeDb);
            SetHeadPhoneRGain(VoiceAnalogRange);
            SetHeadPhoneLGain(VoiceAnalogRange);
            if (AudioALSASpeechPhoneCallController::getInstance()->checkTtyNeedOn() == false) {
                ApplyMicGain(Headset_Mic, mode); // set incall mic gain
            }
        } else {
        }
    }
    if (device & AUDIO_DEVICE_OUT_SPEAKER) {
#ifndef EXT_SPK_SUPPORT
        SetInternalSpkGain(degradeDb);
#else
        SetExternalSpkGain(degradeDb);
#endif
        if (AudioALSASpeechPhoneCallController::getInstance()->checkTtyNeedOn() == false) {
            ApplyMicGain(Handfree_Mic, mode); // set incall mic gain
        }
    }

    if ((device & AUDIO_DEVICE_OUT_BLUETOOTH_SCO) || (device & AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET) || (device & AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET)) {
        //when use BT_SCO , apply digital to 0db.
        ApplyMdDlGain(0);
        ApplyMdDlEhn1Gain(0);
        ApplyMdUlGain(0);
    }
    ApplySideTone(GetSideToneGainType(device));
    return NO_ERROR;
}

float AudioALSAVolumeController::getVoiceVolume(void) {
    ALOGD("AudioALSAVolumeController getVoiceVolume");
    return mVoiceVolume;
}

status_t AudioALSAVolumeController::setStreamVolume(int stream, float v) {
    ALOGD("AudioALSAVolumeController setStreamVolume stream = %d", stream);
    return NO_ERROR;
}

status_t AudioALSAVolumeController::setStreamMute(int stream, bool mute) {
    ALOGD("AudioALSAVolumeController setStreamMute stream = %d mute = %d", stream, mute);
    return NO_ERROR;
}

float AudioALSAVolumeController::getStreamVolume(int stream) {
    ALOGD("AudioALSAVolumeController getStreamVolume stream = %d", stream);
    return mStreamVolume[stream];
}

status_t AudioALSAVolumeController::SetSideTone(uint32_t Mode, uint32_t Gain) {
    ALOGD("SetSideTone type Mode = %d devices = %d", Mode, Gain);
    if (Mode >=  Num_Side_Tone_Gain) {
        ALOGD("Mode >Num_Side_Tone_Gain");
        return INVALID_OPERATION;
    }
    mSideTone[Mode] = Gain;
    return NO_ERROR;
}

uint32_t AudioALSAVolumeController::GetSideToneGain(uint32_t device) {
    ALOGD("GetSideToneGain type device = %d ", device);
    uint32_t Gain_type = GetSideToneGainType(device);
    if (Gain_type >=  Num_Side_Tone_Gain) {
        ALOGD("Mode >Num_Side_Tone_Gain");
        return INVALID_OPERATION;
    }
    ALOGD("GetSideToneGain = %d", mSideTone[Gain_type]);
    return mSideTone[Gain_type];
}

uint32_t AudioALSAVolumeController::GetSideToneGainType(uint32_t devices) {
    if (devices & AUDIO_DEVICE_OUT_EARPIECE) {
        return EarPiece_SideTone_Gain;
    } else if (devices & AUDIO_DEVICE_OUT_SPEAKER) {
        return LoudSpk_SideTone_Gain;
    } else if ((devices & AUDIO_DEVICE_OUT_WIRED_HEADPHONE) || (devices & AUDIO_DEVICE_OUT_WIRED_HEADSET)) {
        return Headset_SideTone_Gain;
    } else {
        ALOGW("GetSideToneGainType with devices = 0x%x", devices);
        return LoudSpk_SideTone_Gain;
    }
}

status_t AudioALSAVolumeController::ApplySideTone(uint32_t Mode) {
    // here apply side tone gain, need base on UL and DL analog gainQuant
    uint16_t DspSideToneGain = 0;
    int SidetoneDb = 0;
    if (Mode == EarPiece_SideTone_Gain) {

#if defined(MTK_HAC_SUPPORT)
        bool mHACon = SpeechEnhancementController::GetInstance()->GetHACOn();

        if (mHACon) {
            ALOGD("%s(): mHACon=%d", __FUNCTION__, mHACon);
            SidetoneDb = mHacParam.audiovolume_sid_hac[3] >> 3;
        } else
#endif
        {
            SidetoneDb = mVolumeParam.audiovolume_sid[VOLUME_NORMAL_MODE][3] >> 3;
        }
        DspSideToneGain = UpdateSidetone(GetReceiverGain(), SidetoneDb, mSwAgcGain);
    } else if (Mode == Headset_SideTone_Gain) {
        SidetoneDb = mVolumeParam.audiovolume_sid[VOLUME_HEADSET_MODE][3] >> 3;
        DspSideToneGain = UpdateSidetone(GetHeadphoneRGain(), SidetoneDb, mSwAgcGain);
    } else if (Mode == LoudSpk_SideTone_Gain) {
        SidetoneDb = mVolumeParam.audiovolume_sid[VOLUME_SPEAKER_MODE][3] >> 3;
        // mute sidetone gain when speaker mode.
        DspSideToneGain = 0;
    }
    ALOGD("ApplySideTone mode = %d DspSideToneGain = %d", Mode, DspSideToneGain);
    SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetSidetoneGain(DspSideToneGain);

    return NO_ERROR;
}

status_t AudioALSAVolumeController::SetMicGain(uint32_t Mode, uint32_t Gain) {
    if (Mode >= Num_Mic_Gain) {
        ALOGD("SetMicGain error");
        return false;
    }
    ALOGD("SetMicGain MicMode=%d, Gain=%d", Mode, Gain);
    mMicGain[Mode] = Gain;
    return NO_ERROR;
}

status_t AudioALSAVolumeController::SetULTotalGain(uint32_t Mode, unsigned char Volume) {
    if (Volume > UPLINK_GAIN_MAX) {
        Volume = UPLINK_GAIN_MAX;
    }

    ALOGD("SetULTotalGain MicMode=%d, Volume=%d", Mode, Volume);
    mULTotalGainTable[Mode] = Volume;
    return NO_ERROR;
}

bool AudioALSAVolumeController::CheckMicUsageWithMode(uint32_t MicType, int mode) {
    if ((MicType == Normal_Mic ||
         MicType == Headset_Mic ||
         MicType == Handfree_Mic) &&
        (mode != AUDIO_MODE_IN_CALL)) {
        return true;
    } else {
        return false;
    }
}

status_t AudioALSAVolumeController::SetCaptureGain(audio_mode_t mode, audio_source_t source, audio_devices_t input_device, audio_devices_t output_devices) {
    ALOGD("+%s(), mode=%d, source=%d, input device=0x%x, output device=0x%x", __FUNCTION__, mode, source, input_device, output_devices);
    switch (mode) {
    case AUDIO_MODE_NORMAL:
    case AUDIO_MODE_RINGTONE: {
        if (source == AUDIO_SOURCE_VOICE_RECOGNITION) {
            if (input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
                ApplyMicGain(Voice_Rec_Mic_Headset, mode);
            } else {
                ApplyMicGain(Voice_Rec_Mic_Handset, mode);
            }
        } else if (source == AUDIO_SOURCE_CAMCORDER) {
            if (input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
                ApplyMicGain(Idle_Video_Record_Headset, mode);
            } else {
                ApplyMicGain(Idle_Video_Record_Handset, mode);
            }
        } else if (source == AUDIO_SOURCE_VOICE_UNLOCK) {
            if (input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
                ApplyMicGain(Voice_UnLock_Mic_Headset, mode);
            } else {
                ApplyMicGain(Voice_UnLock_Mic_Handset, mode);
            }
        } else if (source == AUDIO_SOURCE_CUSTOMIZATION1) {
            if (input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
                ApplyMicGain(Customization1_Mic_Headset, mode);
            } else {
                ApplyMicGain(Customization1_Mic_Handset, mode);
            }
        } else if (source == AUDIO_SOURCE_CUSTOMIZATION2) {
            if (input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
                ApplyMicGain(Customization2_Mic_Headset, mode);
            } else {
                ApplyMicGain(Customization2_Mic_Handset, mode);
            }
        } else if (source == AUDIO_SOURCE_CUSTOMIZATION3) {
            if (input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
                ApplyMicGain(Customization3_Mic_Headset, mode);
            } else {
                ApplyMicGain(Customization3_Mic_Handset, mode);
            }
        } else {
            //for audio tuning tool tuning case.
            if (mAudioSpeechEnhanceInfoInstance->IsAPDMNRTuningEnable()) {  //for DMNR tuning
                if (mAudioSpeechEnhanceInfoInstance->GetAPTuningMode() == HANDSFREE_MODE_DMNR) {
                    ApplyMicGain(Handfree_Mic, mode);
                } else if (mAudioSpeechEnhanceInfoInstance->GetAPTuningMode() == NORMAL_MODE_DMNR) {
                    ApplyMicGain(Normal_Mic, mode);
                } else {
                    ApplyMicGain(Idle_Normal_Record, mode);
                }
            } else {
                if (input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
                    ApplyMicGain(Idle_Headset_Record, mode);
                } else {
                    ApplyMicGain(Idle_Normal_Record, mode);
                }
            }
        }
        break;
    }
    case AUDIO_MODE_IN_CALL: {
        if (AudioALSASpeechPhoneCallController::getInstance()->checkTtyNeedOn() == false) {
            if (output_devices == AUDIO_DEVICE_OUT_EARPIECE) {
                ApplyMicGain(Normal_Mic, mode);
            } else if (output_devices == AUDIO_DEVICE_OUT_SPEAKER) {
                ApplyMicGain(Handfree_Mic, mode);
            } else {
                ApplyMicGain(Headset_Mic, mode);
            }
        }
        break;
    }
    case AUDIO_MODE_IN_COMMUNICATION: {
        if (output_devices == AUDIO_DEVICE_OUT_EARPIECE) {
            ApplyMicGain(VOIP_Normal_Mic, mode);
        } else if (output_devices == AUDIO_DEVICE_OUT_SPEAKER) {
            ApplyMicGain(VOIP_Handfree_Mic, mode);
        } else {
            ApplyMicGain(VOIP_Headset_Mic, mode);
        }
        break;
    }
    }
    return NO_ERROR;
}

status_t AudioALSAVolumeController::ApplyMicGain(uint32_t MicType, int mode) {
    if (MicType >= Num_Mic_Gain) {
        ALOGD("SetMicGain error");
        return false;
    }
    mSwAgcGain = SW_AGC_GAIN_MAX;
    mULTotalGain = mULTotalGainTable[MicType];
    // here base on mic  and use degrade gain to set hardware register
    int DegradedBGain = mMicGain[MicType];

    // bounded systen total gain
    if (DegradedBGain > AUDIO_SYSTEM_UL_GAIN_MAX) {
        DegradedBGain = AUDIO_SYSTEM_UL_GAIN_MAX;
    }

    if (IsAudioSupportFeature(AUDIO_SUPPORT_DMIC)) {
        mSwAgcGain  =  Dmic_SwAgc_Gain_Map[DegradedBGain];
        DegradedBGain  =  Dmic_PGA_Gain_Map[DegradedBGain];
    } else {
        mSwAgcGain  =  SwAgc_Gain_Map[DegradedBGain];
        DegradedBGain  =  PGA_Gain_Map[DegradedBGain];
    }

    ALOGD("ApplyMicGain MicType = %d DegradedBGain = %d SwAgcGain = %d, mULTotalGain = %d",
          MicType, DegradedBGain, mSwAgcGain, mULTotalGain);
#if defined(MTK_SPEAKER_MONITOR_SUPPORT)
    if (MicType == Individual_2ndSet_Mic) {
        // For Speaker Monitor, it doens't care the mode.
        DegradedBGain = (MAX_PGA_GAIN_RANGE - DegradedBGain) / AUDIO_UL_PGA_STEP;
        SetAdcPga3(DegradedBGain);
        SetAdcPga4(DegradedBGain);
    } else
#endif
        // fix me: here need t send reminder DB to HD record or modem side
        if (mode == AUDIO_MODE_IN_CALL) {
            ApplyMdUlGain(mSwAgcGain);
            DegradedBGain = (MAX_PGA_GAIN_RANGE - DegradedBGain) / AUDIO_UL_PGA_STEP;
            SetAdcPga1(DegradedBGain);
            SetAdcPga2(DegradedBGain);
#if !defined(MTK_SPEAKER_MONITOR_SUPPORT)
            SetAdcPga3(DegradedBGain);
            SetAdcPga4(DegradedBGain);
#endif
        } else {
            ALOGD("ApplyMicGain mSwAgcGain = %d, mULTotalGain=%d DegradedBGain = %d", mSwAgcGain, mULTotalGain, DegradedBGain);
            DegradedBGain = (MAX_PGA_GAIN_RANGE - DegradedBGain) / AUDIO_UL_PGA_STEP;
            SetAdcPga1(DegradedBGain);
            SetAdcPga2(DegradedBGain);
#if !defined(MTK_SPEAKER_MONITOR_SUPPORT)
            SetAdcPga3(DegradedBGain);
            SetAdcPga4(DegradedBGain);
#endif
        }
    return NO_ERROR;
}

uint32_t AudioALSAVolumeController::MapDigitalHwGain(uint32_t Gain) {
    uint32_t DegradeDB = 0;
    if (Gain > HW_DIGITAL_GAIN_MAX) {
        Gain = HW_DIGITAL_GAIN_MAX;
    }
    DegradeDB = (HW_DIGITAL_GAIN_MAX - Gain) / HW_DIGITAL_GAIN_STEP;
    return DegradeDB;
}

status_t AudioALSAVolumeController::SetDigitalHwGain(uint32_t Mode, uint32_t Gain, uint32_t routes) {
    uint32_t DegradeDB = MapDigitalHwGain(Gain);
    return NO_ERROR;
}

void AudioALSAVolumeController::GetDefaultVolumeParameters(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *volume_param) {
    AudioCustParamClient::GetInstance()->GetVolumeVer1ParamFromNV(volume_param);

    // fix me , here just get default , need to get from nvram
    for (int i = 0 ; i < NORMAL_VOLUME_TYPE_MAX ; i++) {
        ALOGD("ad_audio_custom_default normalaudiovolume %d = %d", i, volume_param->normalaudiovolume[i]);
    }
    for (int i = 0 ; i < HEADSET_VOLUME_TYPE_MAX ; i++) {
        ALOGD("ad_audio_custom_default headsetaudiovolume %d = %d", i, volume_param->headsetaudiovolume[i]);
    }
    for (int i = 0 ; i < SPEAKER_VOLUME_TYPE_MAX ; i++) {
        ALOGD("ad_audio_custom_default speakeraudiovolume %d = %d", i, volume_param->speakeraudiovolume[i]);
    }
    for (int i = 0 ; i < HEADSET_SPEAKER_VOLUME_TYPE_MAX ; i++) {
        ALOGD("ad_audio_custom_default headsetspeakeraudiovolume %d = %d", i, volume_param->headsetspeakeraudiovolume[i]);
    }
    //memcpy((void *)volume_param, (void *)&ad_audio_custom_default, sizeof(ADAUDIO_CUSTOM_VOLUME_PARAM_STRUCT));
}

status_t AudioALSAVolumeController::SetMicGainTuning(uint32_t Mode, uint32_t Gain) {
    ALOGD("SetMicGainTuning Mode = %d, Gain = %d", Mode, Gain);
    int degradegain;
    degradegain = (unsigned char)MampUplinkGain(Gain);
    SetMicGain(Mode,  degradegain);
    return NO_ERROR;
}

int AudioALSAVolumeController::ApplyAudioGainTuning(int Gain, uint32_t mode, uint32_t device) {
    ALOGD("+%s, Gain = %d, mode = %d device = %d", __FUNCTION__, Gain, mode, device);

    int MapVolume = 0;
    int DRCversion = GetDRCVersion(device);
    int degradeDb = 0;
    int DigitalgradeDb = 0;  // digital gain
    int VoiceAnalogRange = 0;

    if (DRCversion == DRC_VERSION_1) {
        VoiceAnalogRange = DLPGA_Gain_Map_Ver1[degradeDb];
        DigitalgradeDb = DlDigital_Gain_Map_Ver1[degradeDb];
    } else {
        VoiceAnalogRange = DLPGA_Gain_Map_Ver2[degradeDb];
        DigitalgradeDb = DlDigital_Gain_Map_Ver2[degradeDb];
    }

    ALOGD("%s, DigitalgradeDb = %d, VoiceAnalogRange = %d ", __FUNCTION__, DigitalgradeDb, VoiceAnalogRange);

    switch (device) {
    case Audio_Earpiece: {
        if (VoiceAnalogRange >= (_countof(DL_PGA_Handset_GAIN) - 1)) {
            VoiceAnalogRange = _countof(DL_PGA_Handset_GAIN) - 1;
        }
        if (IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER)) {
#ifndef EXT_SPK_SUPPORT
            Set2IN1InternalSpkGain(degradeDb);
#else
            Set2IN1ExternalSpkGain(degradeDb);
#endif
        } else {
            SetReceiverGain(VoiceAnalogRange);
        }
        break;
    }
    case Audio_Headset: {
#if 0
        if (VoiceAnalogRange >= (_countof(DL_PGA_Headset_GAIN) - 1)) {
            VoiceAnalogRange = _countof(DL_PGA_Headset_GAIN) - 1;
        }
#endif
        SetHeadPhoneRGain(VoiceAnalogRange);
        SetHeadPhoneLGain(VoiceAnalogRange);
        break;
    }
    default:
        ALOGD("%s, not support device ", __FUNCTION__);
        break;
    }

    return DigitalgradeDb;
}

#ifdef MTK_AUDIO_GAIN_TABLE
status_t AudioALSAVolumeController::setAnalogVolume(int stream, int device, int index, audio_mode_t mode) {
    return INVALID_OPERATION;
}

status_t AudioALSAVolumeController::speechBandChange(bool nb) {
    return  INVALID_OPERATION;
}
bool AudioALSAVolumeController::isNbSpeechBand(void) {
    return true;
}
status_t AudioALSAVolumeController::setBtVolumeCapability(bool support) {
    return  INVALID_OPERATION;
}
#endif

}
