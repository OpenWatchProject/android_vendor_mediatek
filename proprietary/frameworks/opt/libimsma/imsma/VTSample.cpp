#define LOG_TAG "MATest"

#include <sys/types.h>
#include <ImsMa.h>
#include <utils/Log.h>
//#include <RefBase.h>
//#include <StrongPointer.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AHandler.h>
#include <sys/types.h>
#include <comutils.h>
using namespace android;

#define MIN(a, b)                                               ((a) < (b) ? (a) : (b))
#define MAX(a, b)                                               ((a) < (b) ? (b) : (a))


int main()
{



    char sprop_vps[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH] = "QAEMAf//IWAAAAMAAAMAAAMAAAMAP6wJ\0";
    char sprop_sps[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH] = "QgEBIWAAAAMAAAMAAAMAAAMAP6APCAWH+WtJJSS7IA==\0";
    char sprop_pps[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH] = "RAHBc8DsIQ==\0";



    uint32_t sprop_vps_size = 0;

    for(int32_t i = 0; i < VOLTE_MAX_SDP_PARAMETER_SET_LENGTH ; i++) {
        if(* (sprop_vps + i) == '\0') {
            sprop_vps_size = i;
            break;
        }
    }

    printf("sprop_vps_size %d",sprop_vps_size);
    VT_LOGD("sprop_vps_size %d",sprop_vps_size);


    uint32_t sprop_sps_size = 0;

    for(int32_t i = 0; i < VOLTE_MAX_SDP_PARAMETER_SET_LENGTH ; i++) {
        if(* (sprop_sps + i) == '\0') {
            sprop_sps_size = i;
            break;
        }
    }

    printf("sprop_sps_size %d",sprop_sps_size);
    VT_LOGD("sprop_sps_size %d",sprop_sps_size);

    uint32_t sprop_pps_size = 0;

    for(int32_t i = 0; i < VOLTE_MAX_SDP_PARAMETER_SET_LENGTH ; i++) {
        if(* (sprop_pps + i) == '\0') {
            sprop_pps_size = i;
            break;
        }
    }

    printf("sprop_pps_size %d",sprop_pps_size);
    VT_LOGD("sprop_pps_size %d",sprop_pps_size);

    int32_t total_size = sprop_pps_size + sprop_sps_size + sprop_vps_size + 3/*add dilimeter*/;
    char *sprop_vps_sps_pps = (char *) malloc(total_size);

    int32_t offset =0;
    memcpy((char *)(sprop_vps_sps_pps + offset), (char *)(sprop_vps),sprop_vps_size);
    offset += sprop_vps_size;
    sprop_vps_sps_pps[offset] = ',';
    offset += 1;


    memcpy((char *)(sprop_vps_sps_pps + offset), (char *)(sprop_sps),sprop_sps_size);
    offset += sprop_sps_size;
    sprop_vps_sps_pps[offset] = ',';
    offset += 1;

    memcpy((char *)(sprop_vps_sps_pps+ offset), (char *)(sprop_pps),sprop_pps_size);
    offset += sprop_pps_size;
    sprop_vps_sps_pps[offset] = '\0';

    printf("sprop_vps_sps_pps= %s", sprop_vps_sps_pps);

    int32_t  width =-1  ;
    int32_t  height =-1  ;
    /*
    int32_t  sarWidth =-1  ;
    int32_t  sarHeight =-1  ;
    int32_t  profile = -1  ;
    int32_t  level =-1  ;
    */

    sp<ABuffer> csd = MakeHEVCCodecSpecificData(sprop_vps_sps_pps,&width, &height);



}


