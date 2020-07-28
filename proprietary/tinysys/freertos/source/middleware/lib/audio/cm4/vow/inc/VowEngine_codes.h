#ifndef VOWENGINE_CODES
#define VOWENGINE_CODES

#ifdef __cplusplus
extern "C" {
#endif

/*-- return codes -----------------------------------------------------------------------------------------------*/
enum {
    vowe_bad = 0,
    vowe_ok = 1,

    vowe_ready_to_enroll           = 10001 ,
    vowe_next_frame                = 10002 ,
    vowe_no_speech                 = 10003 ,
    vowe_good_utterance            = 10004 ,
    vowe_bad_utterance             = 10005 ,
    vowe_noisy_utterance           = 10006 ,
    vowe_low_snr_utterance         = 10007 ,
    vowe_mismatched_cmd_utterance  = 10008 ,
    vowe_not_recording             = 10011 ,

    vowe_hold_on                   = 20001 ,
    vowe_accept                    = 20002 ,
    vowe_reject                    = 20003 ,

    vowe_error_out_of_memory       = 90001 ,
    vowe_error_file_not_exist      = 90002 ,
    vowe_error_invalid_file_format = 90003 ,
    vowe_error_invalid_argument    = 90004
};

/*-- modes ------------------------------------------------------------------------------------------------------*/
enum                                             // +--------------------+-------------------+-------------+
{                                                // | Enable DSP testing | Enable AP testing | Enable SID  |
    vowe_mode_pdk_fullRecognizer      = 10001 ,  // |         v          |         v         |      v      |
    vowe_mode_pdk_speakerIndependent  = 10002 ,  // |         v          |         v         |             |
    vowe_mode_pdk_lowPower            = 11000 ,  // |         v          |                   |             |
    vowe_mode_udk_fullRecognizer      = 20001 ,  // |         v          |         v         |      v      |
    vowe_mode_udk_speakerIndependent  = 20002 ,  // |         v          |         v         |             |
    vowe_mode_udk_lowPower            = 21000 ,  // |         v          |                   |             |
};                                               // +--------------------+-------------------+-------------+

/*-- argument ids -----------------------------------------------------------------------------------------------*/
enum {
    vowe_argid_pushToTalk =   101 ,

    vowe_argid_printLog   = 10001
};



#ifdef __cplusplus
}
#endif

#endif // VOWENGINE_CODES

