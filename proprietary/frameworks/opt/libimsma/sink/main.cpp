#define LOG_TAG "SINK"
#include <utils/Log.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "Sink.h"
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <media/AudioSystem.h>
#include <media/DataSource.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <ui/DisplayInfo.h>
#include <media/stagefright/NuMediaExtractor.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/ALooper.h>
#include <cutils/properties.h>
namespace android
{

/*
static void usage(const char *me)
{
    fprintf(stderr,
            "usage:\n"
            "           %s -r \tinput the mediafile\n",
            me);
}

static void createFileSource()
{
}


static void hexdump(const void *_data, size_t size)
{
    const uint8_t *data = (const uint8_t *) _data;
    size_t offset = 0;

    while(offset < size) {
        printf("0x%04zx  ", offset);

        size_t n = size - offset;

        if(n > 16) {
            n = 16;
        }

        for(size_t i = 0; i < 16; ++i) {
            if(i == 8) {
                printf(" ");
            }

            if(offset + i < size) {
                printf("%02x ", data[offset + i]);
            } else {
                printf("   ");
            }
        }

        printf(" ");

        for(size_t i = 0; i < n; ++i) {
            if(isprint(data[offset + i])) {
                printf("%c", data[offset + i]);
            } else {
                printf(".");
            }
        }

        printf("\n");

        offset += 16;
    }
}

*/

struct testMA : public AHandler {
    enum {
        kWhatSinkNotify,
    };
    testMA() {
        ALOGD("new testMA");
    }


protected:
    virtual ~testMA() {}
    void onMessageReceived(const sp<AMessage> &msg) {
        ALOGI("testMA msg->what() %d",msg->what());

        switch(msg->what()) {
        case Sink::kWhatError: {
            ALOGE("MA receive error");
        }
        default:
            break;
        }


    }

private:


    DISALLOW_EVIL_CONSTRUCTORS(testMA);
};



}  // namespace android

int main(int argc, char **argv)
{
    using namespace android;

    ALOGD("argc %d  argv %p", argc,argv);

    ProcessState::self()->startThreadPool();/*
    // 1 prepare parameter
    // 1.1 creat surface

    sp<SurfaceComposerClient> composerClient = new SurfaceComposerClient;
    CHECK_EQ(composerClient->initCheck(), (status_t) OK);

    sp<IBinder> display(SurfaceComposerClient::getBuiltInDisplay(
                            ISurfaceComposer::eDisplayIdMain));
    DisplayInfo info;
    SurfaceComposerClient::getDisplayInfo(display, &info);
    ssize_t displayWidth = 1280;//info.w;
    ssize_t displayHeight = 720;//info.h;

    ALOGD("argc %d  argv %p ,display is %zu x %zu\n", argc,argv,displayWidth, displayHeight);


    sp<SurfaceControl> control =
        composerClient->createSurface(
            String8("A Surface"),
            displayWidth,
            displayHeight,
            PIXEL_FORMAT_RGB_565,
            0);

    CHECK(control != NULL);
    CHECK(control->isValid());

    SurfaceComposerClient::openGlobalTransaction();
    CHECK_EQ(control->setLayer(INT_MAX), (status_t) OK);
    CHECK_EQ(control->show(), (status_t) OK);
    SurfaceComposerClient::closeGlobalTransaction();

    sp<Surface> surface = control->getSurface();
    CHECK(surface != NULL);


    bool disableAudio= true;
    bool disableRecord = true;
    bool testH264= false;
    char value[PROPERTY_VALUE_MAX];

    property_set("vendor.vt.sink.avsync.enable", "0");

    if(property_get("vendor.sink.dis.audio", value, NULL)) {
        disableAudio =!strcmp("1", value) || !strcasecmp("true", value);
    }

    if(property_get("vendor.sink.dis.record", value, NULL)) {
        disableRecord =!strcmp("1", value) || !strcasecmp("true", value);
    }

    if(property_get("vendor.sink.test.h264", value, NULL)) {
        testH264 =!strcmp("1", value) || !strcasecmp("true", value);
    }


    sp<testMA> mMA= new  testMA() ;
    sp<ALooper> testLooper = new ALooper;
    testLooper->setName("MA_looper");
    testLooper->start(
        false   ,
        false ,
        PRIORITY_AUDIO);

    testLooper->registerHandler(mMA);
    sp<AMessage> notify = new AMessage(testMA::kWhatSinkNotify, mMA);



    //MediaExtractor::RegisterDefaultSniffers();

    sp<NuMediaExtractor> mExtractor = new NuMediaExtractor();

    int fd =  open("/sdcard/test.mp4",O_LARGEFILE | O_RDONLY);

    struct stat sb;
    int ret = fstat(fd, &sb);

    if(ret != 0) {
        ALOGE("fstat(%d) failed: %d, %s", fd, ret, strerror(errno));

        return -1;
    }

    status_t err;
    err = mExtractor->setDataSource(fd, 0, sb.st_size);
    ALOGD("setDataSource err =%d",err);

    int numtracks  = mExtractor->countTracks();

    sp<AMessage> audioDownlink = new AMessage();
    sp<AMessage> videoDownlink = new AMessage();


    sp<AMessage>  vsformat,asformat ;
    int videoTrack = -1;
    int audioTrack = -1;
    ALOGD("input has %d tracks", numtracks);

    for(int i = 0; i < numtracks; i++) {
        sp<AMessage>  format ;
        mExtractor->getTrackFormat(i,&format);;
        ALOGI("track %d format: %s", i, format->debugString(0).c_str());
        AString mime;

        if(!format->findString("mime", &mime)) {
            ALOGE("no mime type");
            return -1;
        } else if(!strncmp(mime.c_str(), "video/", 6)) {
            vsformat = format;
            mExtractor->selectTrack(i);
            videoTrack = i;
        } else if(!strncmp(mime.c_str(), "audio/", 6)) {
            asformat = format;
            mExtractor->selectTrack(i);
            audioTrack = i;
        }
    }

    err = mExtractor->getTrackFormat(videoTrack,&videoDownlink);

    if(err == OK) {
        ALOGD("videoTrack %d getTrackFormat videoDownlink  %s  ", videoTrack,videoDownlink->debugString(0).c_str());
    }

    err = mExtractor->getTrackFormat(audioTrack,&audioDownlink);
    if(err == OK){
        ALOGD("audioTrack %d getTrackFormat audioDownlink  %s  ",audioTrack,audioDownlink->debugString(0).c_str());
    }


    AString mimev;
    int32_t width;
    int32_t height;
    int32_t profile = 1;
    int32_t level = 30;
    int32_t max_buffer_size;
    sp<ABuffer> csd0 = NULL;
    sp<ABuffer> csd1 = NULL;
    sp<ABuffer> csd = NULL;
    CHECK(videoDownlink->findString("mime", &mimev));
    CHECK(videoDownlink->findInt32("width", &width));
    CHECK(videoDownlink->findInt32("height", &height));
    CHECK(videoDownlink->findInt32("max-input-size", &max_buffer_size));
    videoDownlink->findBuffer("csd-0", &csd0);
    videoDownlink->findBuffer("csd-1", &csd1);

    if(csd0.get() != NULL && csd1.get() != NULL) {
        csd = new ABuffer(csd0->size() +csd1->size());
        memcpy(csd->data(),csd0->data(),csd0->size());
        csd->setRange(0, csd0->size());
        memcpy(csd->data() +csd0->size(),csd1->data(),csd1->size());
        csd->setRange(0, csd0->size() +csd1->size());
        //ALOGD("csd0 size =%d,csd1 size %d",csd0->size(),csd1->size());
    } else if(csd0.get() != NULL) {
        csd = new ABuffer(csd0->size());
        memcpy(csd->data(),csd0->data(),csd0->size());
        csd->setRange(0, csd0->size());
    } else if(csd1.get() != NULL) {
        csd = new ABuffer(csd1->size());
        memcpy(csd->data(),csd1->data(),csd1->size());
        csd->setRange(0, csd1->size());
    }

    hexdump(csd->data(), csd->size());

        AString mimea;
        int32_t channel_count;
        int32_t sample_rate;

        CHECK(audioDownlink->findString("mime", &mimea));
        CHECK(audioDownlink->findInt32("channel-count", &channel_count));
        CHECK(audioDownlink->findInt32("sample-rate", &sample_rate));





    //create sink

    sp<Sink> mSink = new Sink(0,disableAudio? (Sink::FLAG_SPECIAL_MODE) : (Sink::FLAG_NORMAL_MODE),0,1);
    mSink->SetNotify(notify);
    mSink->SetSurface(surface->getIGraphicBufferProducer());



    audio_dec_fmt* audioFormat = NULL;
    video_dec_fmt   *videoFormat =NULL;
    videoFormat = (video_dec_fmt*) malloc(sizeof(video_dec_fmt));
    videoFormat->mimetype = testH264?MEDIA_MIMETYPE_VIDEO_AVC:MEDIA_MIMETYPE_VIDEO_HEVC;
    videoFormat->profile = profile;
    videoFormat->level = level;
    videoFormat->width = width;
    videoFormat->height = height;
    videoFormat->sarWidth = width;
    videoFormat->sarHeight = height;
    videoFormat->csd = csd;
    mSink->setSinkCongfig(audioFormat,videoFormat);
    //if wihich one is not NULL. means this trackwill be play,modem handle audio tracks
    mSink->start(true,false);
    mSink->stop();
    mSink->SetSurface(surface->getIGraphicBufferProducer());
    mSink->setSinkCongfig(audioFormat,videoFormat);
    mSink->start(true,false);

    int32_t count = 0;
    bool recording =false;
    int32_t degree = 0;

    while(1) {
        size_t trackIndex=0;
        sp<ABuffer>  accessUnit = new ABuffer(max_buffer_size);
        mExtractor->readSampleData(accessUnit);

        int64_t time ;
        mExtractor->getSampleTime(&time);
        mExtractor->getSampleTrackIndex(&trackIndex);
        ALOGI("queue track %zu videoTrack %d time =%lld ms",trackIndex,videoTrack,time/1000ll);
        int64_t timeUs = ALooper::GetNowUs();

        if(count < 100)
            accessUnit->meta()->setInt64("ntp-time",  0);    //must put 64 bit NTP  ,if no ntp, put 0 in this meta to tell sink no ntp to use
        else
            accessUnit->meta()->setInt64("ntp-time",  timeUs);


        accessUnit->meta()->setInt64("timeUs",  time);

        if(trackIndex == (size_t) videoTrack) {

            char value[PROPERTY_VALUE_MAX];

            if(property_get("vendor.sink.tr.degree", value, NULL)) {
                degree = atoi(value);
            }

            accessUnit->meta()->setInt32("rotation-degrees",degree);


            mSink->queueAccessUnit(Sink::VIDEO_DL, accessUnit);

        } else if(trackIndex == (size_t) audioTrack && !disableAudio) {
            mSink->queueAccessUnit(Sink::AUDIO_DL, accessUnit);
        }


        status_t err = mExtractor->advance();

        if(err == ERROR_END_OF_STREAM) {
            ALOGI("eos");
            break;
        }


        if(count == 100 && !disableRecord) {
            ALOGI("starting record");
            record_config info;
            info.outf =OUTPUT_FORMAT_MPEG_4;//must
            info.useFd =false;
            info.path ="/sdcard/record.mp4";//"sdcard/xx.3gp";//must
            info.fd=-1; //
            info.mode =RECORD_DLVIDEO_MIXADUIO;//ms/


            info.ve = VIDEO_ENCODER_H264;
            info.ae  = AUDIO_ENCODER_AMR_NB;
            info.params = new  AMessage();
            info.params->setInt32("rotation-degrees", 90);

            mSink->initRecorder(&info);

            //we handle all tracks:
            mSink->startRecord();
            recording= true;
        }

        count++;

        if(count == 500) {
            //feed data ----
            if(recording) mSink->stopRecord();
        }


        usleep(5000);

        int32_t exsitNow =0 ;
        if(scanf("%d",&exsitNow) && exsitNow == 1){
            printf("stop now by user\n");
            break;
        }
    }

    printf("stop now\n");
    mSink->stop();

    printf("start now\n");
    mSink->SetSurface(surface->getIGraphicBufferProducer());
    mSink->setSinkCongfig(audioFormat,videoFormat);
    mSink->start(true,false);

    //printf("stop now\n");
    //mSink->stop();
    usleep(10000000);
    //printf("start with not set params\n");
    //mSink->start(true,false);
    //printf("stop now\n");
    //mSink->stop();

    printf("SINK test done===============\n");

    composerClient->dispose();
*/
    return 0;
}
