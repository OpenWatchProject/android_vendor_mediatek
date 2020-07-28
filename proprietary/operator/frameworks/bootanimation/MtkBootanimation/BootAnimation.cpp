/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_NDEBUG 0
#define LOG_TAG "MtkBootAnimation"

#include <stdint.h>
#include <inttypes.h>
#include <sys/inotify.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <utils/misc.h>
#include <signal.h>
#include <time.h>
#include <string.h>


#include <cutils/properties.h>

#include <androidfw/AssetManager.h>
#include <binder/IPCThreadState.h>
#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/SystemClock.h>

#include <android-base/properties.h>

#include <ui/PixelFormat.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <ui/DisplayInfo.h>

#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>

// TODO: Fix Skia.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <SkBitmap.h>
#include <SkImage.h>
#include <SkStream.h>
#pragma GCC diagnostic pop

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <EGL/eglext.h>

#include "BootAnimation.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <ETC1/etc1.h>
#include <media/IMediaHTTPService.h>
#include <media/mediaplayer.h>
#include <media/MediaPlayerInterface.h>
#include <system/audio.h>
#include <gui/LayerState.h>
#include <binder/Parcel.h>

#define PATH_COUNT 3


#ifdef MSSI_MTK_CARRIEREXPRESS_PACK
#define GLOBAL_DEVICE_BOOTANIM_OPTR_NAME "persist.vendor.operator.optr"
#define PATH_COUNT_USP 2

 char mResourcePath_gb[PATH_COUNT_USP][PROPERTY_VALUE_MAX] =
{ "/custom/media/", /*  0  */
 "/system/media/"  /*  1  */
};

static const char* mAudioResource_gb[2] =
{"/bootaudio.mp3", /*  bootaudio path  */
 "/shutaudio.mp3"/*  shutaudio path  */
};
char mBootaudioFileName[PROPERTY_VALUE_MAX];
#else
#define REGIONAL_BOOTANIM_FILE_NAME "persist.vendor.bootanim.logopath"
#define SYSTEM_REGIONALPHONE_DB     "/system/etc/regionalphone/regionalphone.db"
#define CUSTOM_REGIONALPHONE_DB     "/custom/etc/regionalphone/regionalphone.db"

enum MNC
{
    MNC_VODAFONE     = 46692,
    MNC_HUTCH        = 40411,
    MNC_CHINAUNICOM1 = 46001,
    MNC_CHINAUNICOM2 = 46009,
    MNC_COUNT        = 2
};

static const char* mResourcePath[MNC_COUNT][PATH_COUNT] =
{{"/system/media/bootanimation1.zip", "/custom/media/bootanimation1.zip", "/data/local/bootanimation1.zip"}, /*  0  */
 {"/system/media/bootanimation2.zip", "/custom/media/bootanimation2.zip", "/data/local/bootanimation2.zip"} /*  1  */
};

#endif


#if !defined(MSSI_MTK_CARRIEREXPRESS_PACK)
    static const char* mAudioPath[2][PATH_COUNT] =
    {{"/system/media/bootaudio.mp3", "/custom/media/bootaudio.mp3", "/data/local/bootaudio.mp3"} , /*  bootaudio path  */
     {"/system/media/shutaudio.mp3", "/custom/media/shutaudio.mp3", "/data/local/shutaudio.mp3"} /*  shutaudio path  */
    };
#endif

namespace android {

static const char CUSTOM_BOOTANIMATION_FILE[] = "/custom/media/bootanimation.zip";
static const char USER_BOOTANIMATION_FILE[] = "/data/local/bootanimation.zip";
static const char SYSTEM_SHUTANIMATION_FILE[] = "/system/media/shutanimation.zip";
static const char CUSTOM_SHUTANIMATION_FILE[] = "/custom/media/shutanimation.zip";
static const char USER_SHUTANIMATION_FILE[] = "/data/local/shutanimation.zip";

static const char OEM_BOOTANIMATION_FILE[] = "/oem/media/bootanimation.zip";
static const char SYSTEM_BOOTANIMATION_FILE[] = "/system/media/bootanimation.zip";
static const char SYSTEM_ENCRYPTED_BOOTANIMATION_FILE[] = "/system/media/bootanimation-encrypted.zip";
static const char OEM_SHUTDOWNANIMATION_FILE[] = "/oem/media/shutdownanimation.zip";
static const char SYSTEM_SHUTDOWNANIMATION_FILE[] = "/system/media/shutdownanimation.zip";

static const char SYSTEM_DATA_DIR_PATH[] = "/data/system";
static const char SYSTEM_TIME_DIR_NAME[] = "time";
static const char SYSTEM_TIME_DIR_PATH[] = "/data/system/time";
static const char CLOCK_FONT_ASSET[] = "images/clock_font.png";
static const char CLOCK_FONT_ZIP_NAME[] = "clock_font.png";
static const char LAST_TIME_CHANGED_FILE_NAME[] = "last_time_change";
static const char LAST_TIME_CHANGED_FILE_PATH[] = "/data/system/time/last_time_change";
static const char ACCURATE_TIME_FLAG_FILE_NAME[] = "time_is_accurate";
static const char ACCURATE_TIME_FLAG_FILE_PATH[] = "/data/system/time/time_is_accurate";
static const char TIME_FORMAT_12_HOUR_FLAG_FILE_PATH[] = "/data/system/time/time_format_12_hour";
// Java timestamp format. Don't show the clock if the date is before 2000-01-01 00:00:00.
static const long long ACCURATE_TIME_EPOCH = 946684800000;
static constexpr char FONT_BEGIN_CHAR = ' ';
static constexpr char FONT_END_CHAR = '~' + 1;
static constexpr size_t FONT_NUM_CHARS = FONT_END_CHAR - FONT_BEGIN_CHAR + 1;
static constexpr size_t FONT_NUM_COLS = 16;
static constexpr size_t FONT_NUM_ROWS = FONT_NUM_CHARS / FONT_NUM_COLS;
static const int TEXT_CENTER_VALUE = INT_MAX;
static const int TEXT_MISSING_VALUE = INT_MIN;
static const char EXIT_PROP_NAME[] = "service.bootanim.exit";
static const int ANIM_ENTRY_NAME_MAX = 256;
static constexpr size_t TEXT_POS_LEN_MAX = 16;

// ---------------------------------------------------------------------------

BootAnimation::BootAnimation(sp<Callbacks> callbacks, bool bSetBootOrShutDown, bool bSetPlayMP3,bool bSetRotated)
        : Thread(false), mClockEnabled(true), mTimeIsAccurate(false),
        mTimeFormat12Hour(false), mTimeCheckThread(NULL), mCallbacks(callbacks), mZip(NULL) {
    ALOGD("[BootAnimation %s %d]",__FUNCTION__,__LINE__);
    mSession = new SurfaceComposerClient();

    std::string powerCtl = android::base::GetProperty("sys.powerctl", "");
    bBootOrShutDown = bSetBootOrShutDown;
    if (bBootOrShutDown) {
        mShuttingDown = false;
    } else {
        mShuttingDown = true;
    }
    bShutRotate = bSetRotated;
    bPlayMP3 = bSetPlayMP3;
    mProgram = 0;
    bETC1Movie = false;
    mBootVideoPlayType = BOOT_VIDEO_PLAY_FULL;
    mBootVideoPlayState = MEDIA_NOP;
    bAudioStarted = false;
    ALOGD("[MtkBootAnimation %s %d]bBootOrShutDown=%d,bPlayMP3=%d,bShutRotate=%d",__FUNCTION__,__LINE__,bBootOrShutDown,bPlayMP3,bShutRotate);
}

BootAnimation::BootAnimation(bool bSetBootOrShutDown, bool bSetPlayMP3,bool bSetRotated) : Thread(false), mZip(NULL)
{
    ALOGD("[MtkBootAnimation %s %d]",__FUNCTION__,__LINE__);

    mSession = new SurfaceComposerClient();
    bBootOrShutDown = bSetBootOrShutDown;
    bShutRotate = bSetRotated;
    bPlayMP3 = bSetPlayMP3;
    mProgram = 0;
    bETC1Movie = false;
    mBootVideoPlayType = BOOT_VIDEO_PLAY_FULL;
    mBootVideoPlayState = MEDIA_NOP;
    bAudioStarted = false;
    ALOGD("[MtkBootAnimation %s %d]bBootOrShutDown=%d,bPlayMP3=%d,bShutRotate=%d",__FUNCTION__,__LINE__,bBootOrShutDown,bPlayMP3,bShutRotate);
}
BootAnimation::~BootAnimation() {

    if (mZip != NULL) {
        delete mZip;
    }

    if (mProgram) {
        ALOGD("mProgram: %d", mProgram);
        glDeleteProgram(mProgram);
    }
}
BootVideoListener::BootVideoListener(const sp<BootAnimation> &bootanim) {
    ALOGD("[MtkBootAnimation %s %d]",__FUNCTION__,__LINE__);
    mBootanim = bootanim;
}
BootVideoListener::~BootVideoListener() {
    ALOGD("[MtkBootAnimation %s %d]",__FUNCTION__,__LINE__);
}
void BootVideoListener::notify(int msg, int ext1, int ext2, const Parcel *obj) {
    ALOGD("[MtkBootAnimation %s %d] msg=%d ext1=%d ext2=%d",__FUNCTION__,__LINE__, msg, ext1, ext2);
    if(msg == MEDIA_PLAYBACK_COMPLETE || msg == MEDIA_SEEK_COMPLETE) {
        mBootanim->setBootVideoPlayState(MEDIA_PLAYBACK_COMPLETE);
        ALOGD("[MtkBootAnimation %s %d] media player complete",__FUNCTION__,__LINE__);
    }
    if(msg == MEDIA_ERROR || msg == MEDIA_SKIPPED) {
        mBootanim->setBootVideoPlayState(MEDIA_ERROR);
        ALOGD("[MtkBootAnimation %s %d] media player error",__FUNCTION__,__LINE__);
    }
    if(msg == MEDIA_STARTED) {
        mBootanim->setBootAudioStarted();
        ALOGD("[MtkBootAnimation %s %d] AudioStarted",__FUNCTION__,__LINE__);
    }
#ifdef MTK_AOSP_ENHANCEMENT
    if(msg ==  MEDIA_ERROR_TYPE_NOT_SUPPORTED || msg ==  MEDIA_ERROR_BAD_FILE
        || msg == MEDIA_ERROR_CANNOT_CONNECT_TO_SERVER) {
        mBootanim->setBootVideoPlayState(MEDIA_ERROR);
        ALOGD("[MtkBootAnimation %s %d] media player error",__FUNCTION__,__LINE__);
    }
#endif
    if(obj == NULL){
        ALOGD("[MtkBootAnimation %s %d]obj is null \n",__FUNCTION__,__LINE__);
    }
}
void BootAnimation::setBootVideoPlayState(int playState){
    mBootVideoPlayState = playState;
    ALOGD("[MtkBootAnimation %s %d]mBootVideoPlayState=%d",__FUNCTION__,__LINE__, mBootVideoPlayState);
}

void BootAnimation::setBootAudioStarted(void)
{
    AutoMutex _l(mMyLock);
    bAudioStarted = true;
    mCondition.signal();
}

void BootAnimation::onFirstRef() {
    status_t err = mSession->linkToComposerDeath(this);
    ALOGE_IF(err, "linkToComposerDeath failed (%s) ", strerror(-err));
    if (err == NO_ERROR) {
        run("BootAnimation", PRIORITY_DISPLAY);
    }
}

sp<SurfaceComposerClient> BootAnimation::session() const {
    return mSession;
}


void BootAnimation::binderDied(const wp<IBinder>&)
{
    // woah, surfaceflinger died!
    ALOGD("SurfaceFlinger died, exiting...");

    // calling requestExit() is not enough here because the Surface code
    // might be blocked on a condition variable that will never be updated.
    kill( getpid(), SIGKILL );
    requestExit();
}

status_t BootAnimation::initTexture(Texture* texture, AssetManager& assets,
        const char* name) {
    Asset* asset = assets.open(name, Asset::ACCESS_BUFFER);
    if (asset == NULL)
        return NO_INIT;
    SkBitmap bitmap;
    sk_sp<SkData> data = SkData::MakeWithoutCopy(asset->getBuffer(false),
            asset->getLength());
    sk_sp<SkImage> image = SkImage::MakeFromEncoded(data);
    image->asLegacyBitmap(&bitmap, SkImage::kRO_LegacyBitmapMode);
    asset->close();
    delete asset;

    const int w = bitmap.width();
    const int h = bitmap.height();
    const void* p = bitmap.getPixels();

    GLint crop[4] = { 0, h, w, -h };
    texture->w = w;
    texture->h = h;

    glGenTextures(1, &texture->name);
    glBindTexture(GL_TEXTURE_2D, texture->name);

    switch (bitmap.colorType()) {
        case kAlpha_8_SkColorType:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA,
                    GL_UNSIGNED_BYTE, p);
            break;
        case kARGB_4444_SkColorType:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                    GL_UNSIGNED_SHORT_4_4_4_4, p);
            break;
        case kN32_SkColorType:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                    GL_UNSIGNED_BYTE, p);
            break;
        case kRGB_565_SkColorType:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
                    GL_UNSIGNED_SHORT_5_6_5, p);
            break;
        default:
            break;
    }

    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return NO_ERROR;
}

status_t BootAnimation::initTexture(FileMap* map, int* width, int* height)
{
    SkBitmap bitmap;
    sk_sp<SkData> data = SkData::MakeWithoutCopy(map->getDataPtr(),
            map->getDataLength());
    sk_sp<SkImage> image = SkImage::MakeFromEncoded(data);
    image->asLegacyBitmap(&bitmap, SkImage::kRO_LegacyBitmapMode);

    // FileMap memory is never released until application exit.
    // Release it now as the texture is already loaded and the memory used for
    // the packed resource can be released.
    delete map;

    const int w = bitmap.width();
    const int h = bitmap.height();
    const void* p = bitmap.getPixels();

    GLint crop[4] = { 0, h, w, -h };
    int tw = 1 << (31 - __builtin_clz(w));
    int th = 1 << (31 - __builtin_clz(h));
    if (tw < w) tw <<= 1;
    if (th < h) th <<= 1;

    switch (bitmap.colorType()) {
        case kN32_SkColorType:
            if (!mUseNpotTextures && (tw != w || th != h)) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA,
                        GL_UNSIGNED_BYTE, 0);
                glTexSubImage2D(GL_TEXTURE_2D, 0,
                        0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, p);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                        GL_UNSIGNED_BYTE, p);
            }
            break;

        case kRGB_565_SkColorType:
            if (!mUseNpotTextures && (tw != w || th != h)) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tw, th, 0, GL_RGB,
                        GL_UNSIGNED_SHORT_5_6_5, 0);
                glTexSubImage2D(GL_TEXTURE_2D, 0,
                        0, 0, w, h, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, p);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
                        GL_UNSIGNED_SHORT_5_6_5, p);
            }
            break;
        default:
            break;
    }

    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop);

    *width = w;
    *height = h;

    return NO_ERROR;
}

status_t BootAnimation::readyToRun() {
	  SurfaceComposerClient::Transaction t;
    if (bBootOrShutDown) {
        initBootanimationZip();
    } else {
        initShutanimationZip();
    }

    if ((mZip != NULL)&&!(mZipFileName.isEmpty())) {
        ZipEntryRO desc = mZip->findEntryByName("desc.txt");
        uint16_t method;
        mZip->getEntryInfo(desc, &method, NULL, NULL, NULL, NULL, NULL);
        mZip->releaseEntry(desc);
        if (method == ZipFileRO::kCompressStored) {
            bETC1Movie = false;
        } else {
            bETC1Movie = true;
        }
    }

    mAssets.addDefaultAssets();

    sp<IBinder> dtoken(SurfaceComposerClient::getBuiltInDisplay(
            ISurfaceComposer::eDisplayIdMain));
    DisplayInfo dinfo;
    status_t status = SurfaceComposerClient::getDisplayInfo(dtoken, &dinfo);
    if (status)
        return -1;
    /// M: The tablet rotation maybe 90/270 degrees, so set the lcm config for tablet
    //SurfaceComposerClient::setDisplayProjection(dtoken, DisplayState::eOrientationDefault, Rect(dinfo.w, dinfo.h), Rect(dinfo.w, dinfo.h));
    t.setDisplayProjection(dtoken, DisplayState::eOrientationDefault, Rect(dinfo.w, dinfo.h), Rect(dinfo.w, dinfo.h));
    t.apply();
    // create the native surface
    sp<SurfaceControl> control = session()->createSurface(String8("BootAnimation"),
            dinfo.w, dinfo.h, PIXEL_FORMAT_RGB_565);

/*
    SurfaceComposerClient::openGlobalTransaction();
    control->setLayer(0x2000010);
    SurfaceComposerClient::closeGlobalTransaction();
*/
    t.setLayer(control, 0x2000010).apply();
    sp<Surface> s = control->getSurface();

/*  M: we do this if use android movie(),but ETC1Movie not use this @{

    // initialize opengl and egl
    const EGLint attribs[] = {
            EGL_RED_SIZE,   8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE,  8,
            EGL_DEPTH_SIZE, 0,
            EGL_NONE
    };
@}  */
    EGLint w, h;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    ALOGD("initialize opengl and egl");
    EGLBoolean eglret = eglInitialize(display, 0, 0);
    if (eglret == EGL_FALSE) {
        ALOGE("eglInitialize(display, 0, 0) return EGL_FALSE");
    }
    if (!bETC1Movie) {
        const EGLint attribs[] = {
                EGL_RED_SIZE,   8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE,  8,
                EGL_DEPTH_SIZE, 0,
                EGL_NONE
        };
        eglChooseConfig(display, attribs, &config, 1, &numConfigs);
        context = eglCreateContext(display, config, NULL, NULL);
     } else {
        const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_RED_SIZE,   5,
            EGL_GREEN_SIZE, 6,
            EGL_BLUE_SIZE,  5,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
        };
        eglChooseConfig(display, attribs, &config, 1, &numConfigs);
        int attrib_list[] = {EGL_CONTEXT_CLIENT_VERSION, 2,
                       EGL_NONE, EGL_NONE};
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, attrib_list);
    }

    surface = eglCreateWindowSurface(display, config, s.get(), NULL);
    eglret = eglQuerySurface(display, surface, EGL_WIDTH, &w);
    if (eglret == EGL_FALSE) {
        ALOGE("eglQuerySurface(display, surface, EGL_WIDTH, &w) return EGL_FALSE");
    }
    eglret = eglQuerySurface(display, surface, EGL_HEIGHT, &h);
    if (eglret == EGL_FALSE) {
        ALOGE("eglQuerySurface(display, surface, EGL_HEIGHT, &h) return EGL_FALSE");
    }

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        ALOGE("eglMakeCurrent(display, surface, surface, context) return EGL_FALSE");
        return NO_INIT;
    }

    mDisplay = display;
    mContext = context;
    mSurface = surface;
    mWidth = w;
    mHeight = h;
    mFlingerSurfaceControl = control;
    mFlingerSurface = s;
    // If the device has encryption turned on or is in process
    // of being encrypted we show the encrypted boot animation.
    char decrypt[PROPERTY_VALUE_MAX];
    property_get("vold.decrypt", decrypt, "");

    bool encryptedAnimation = atoi(decrypt) != 0 ||
        !strcmp("trigger_restart_min_framework", decrypt);

    if (!mShuttingDown && encryptedAnimation &&
        (access(SYSTEM_ENCRYPTED_BOOTANIMATION_FILE, R_OK) == 0)) {
        mZipFileName = SYSTEM_ENCRYPTED_BOOTANIMATION_FILE;
        return NO_ERROR;
    }
    static const char* bootFiles[] = {OEM_BOOTANIMATION_FILE, SYSTEM_BOOTANIMATION_FILE};
    static const char* shutdownFiles[] =
        {OEM_SHUTDOWNANIMATION_FILE, SYSTEM_SHUTDOWNANIMATION_FILE};

    for (const char* f : (!mShuttingDown ? bootFiles : shutdownFiles)) {
        if (access(f, R_OK) == 0) {
            mZipFileName = f;
            return NO_ERROR;
        }
    }
    return NO_ERROR;
}

bool BootAnimation::threadLoop()
{
    bool r;
    // We have no bootanimation file, so we use the stock android logo
    // animation.
    sp<MediaPlayer> mediaplayer;
    const char* resourcePath = NULL;
    status_t mediastatus = NO_ERROR;
    if (resourcePath != NULL) {
        bPlayMP3 = true;
        ALOGD("sound file path: %s", resourcePath);
        mediaplayer = new MediaPlayer();
        mediastatus = mediaplayer->setDataSource(NULL, resourcePath, NULL);

        sp<BootVideoListener> listener = new BootVideoListener(this);
        mediaplayer->setListener(listener);

        if (mediastatus == NO_ERROR) {
            ALOGD("mediaplayer is initialized");
            Parcel* attributes = new Parcel();
            attributes->writeInt32(AUDIO_USAGE_MEDIA);            //usage
            attributes->writeInt32(AUDIO_CONTENT_TYPE_MUSIC);     //audio_content_type_t
            attributes->writeInt32(AUDIO_SOURCE_DEFAULT);         //audio_source_t
            attributes->writeInt32(0);                            //audio_flags_mask_t
            attributes->writeInt32(1);                            //kAudioAttributesMarshallTagFlattenTags of mediaplayerservice.cpp
            attributes->writeString16(String16("BootAnimationAudioTrack")); // tags
            mediaplayer->setParameter(KEY_PARAMETER_AUDIO_ATTRIBUTES, *attributes);
            mediaplayer->setAudioStreamType(AUDIO_STREAM_MUSIC);
            mediastatus = mediaplayer->prepare();
        }
        if (mediastatus == NO_ERROR) {
            ALOGD("media player is prepared");
            mediastatus = mediaplayer->start();
        }

    }else{
        bPlayMP3 = false;
    }

    //mediaplayer start is an async API, and it will be ready after mediaserver parsing all
    // components which will take some time and cause aduio and video not sync.
    if (bPlayMP3 && mediastatus == NO_ERROR) {
        AutoMutex _l(mMyLock);
        if (!bAudioStarted) {
            status_t err = mCondition.waitRelative(mMyLock, s2ns(4));
            ALOGD("audio started re %d", err);
        }
    }

    if ((mZip == NULL)&&(mZipFileName.isEmpty())) {
        r = android();
    } else if(mZip != NULL){
        if (!bETC1Movie) {
            ALOGD("threadLoop() movie()");
            r = movie();
        } else {
            ALOGD("threadLoop() ETC1movie()");
            r = ETC1movie();
        }
    }
    else
    {
        r = android();
    }

    if (resourcePath != NULL) {
        if (mediastatus == NO_ERROR) {
            ALOGD("mediaplayer was stareted successfully, now it is going to be stoped");
            mediaplayer->stop();
            mediaplayer->disconnect();
            mediaplayer.clear();
        }
    }
    eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(mDisplay, mContext);
    eglDestroySurface(mDisplay, mSurface);
    mFlingerSurface.clear();
    mFlingerSurfaceControl.clear();
    eglTerminate(mDisplay);
    eglReleaseThread();
    IPCThreadState::self()->stopProcess();
    return r;
}

bool BootAnimation::android()
{
    ALOGD("%sAnimationShownTiming start time: %" PRId64 "ms", mShuttingDown ? "Shutdown" : "Boot",
            elapsedRealtime());
    initTexture(&mAndroid[0], mAssets, "images/android-logo-mask.png");
    initTexture(&mAndroid[1], mAssets, "images/android-logo-shine.png");

    mCallbacks->init({});

    // clear screen
    glShadeModel(GL_FLAT);
    glDisable(GL_DITHER);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(mDisplay, mSurface);

    glEnable(GL_TEXTURE_2D);
    glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    const GLint xc = (mWidth  - mAndroid[0].w) / 2;
    const GLint yc = (mHeight - mAndroid[0].h) / 2;
//  const Rect updateRect(xc, yc, xc + mAndroid[0].w, yc + mAndroid[0].h);  //MR1 ADDED

    int x = xc, y = yc;
    int w = mAndroid[0].w, h = mAndroid[0].h;
    if (x < 0) {
        w += x;
        x  = 0;
    }
    if (y < 0) {
        h += y;
        y  = 0;
    }
    if (w > mWidth) {
        w = mWidth;
    }
    if (h > mHeight) {
        h = mHeight;
    }
    ALOGD("[MtkBootAnimation %s %d]x=%d,y=%d,w=%d,h=%d",__FUNCTION__,__LINE__,x,y,w,h);

    const Rect updateRect(x, y, x+w, y+h);

    glScissor(updateRect.left, mHeight - updateRect.bottom, updateRect.width(),
            updateRect.height());

    // Blend state
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    const nsecs_t startTime = systemTime();
    do {
        nsecs_t now = systemTime();
        double time = now - startTime;
        float t = 4.0f * float(time / us2ns(16667)) / mAndroid[1].w;
        GLint offset = (1 - (t - floorf(t))) * mAndroid[1].w;
        GLint x = xc - offset;

        glDisable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_SCISSOR_TEST);
        glDisable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D, mAndroid[1].name);
        glDrawTexiOES(x,                 yc, 0, mAndroid[1].w, mAndroid[1].h);
        glDrawTexiOES(x + mAndroid[1].w, yc, 0, mAndroid[1].w, mAndroid[1].h);

        glEnable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D, mAndroid[0].name);
        glDrawTexiOES(xc, yc, 0, mAndroid[0].w, mAndroid[0].h);

        EGLBoolean res = eglSwapBuffers(mDisplay, mSurface);
        if (res == EGL_FALSE)
            break;

        // 12fps: don't animate too fast to preserve CPU
        const nsecs_t sleepTime = 83333 - ns2us(systemTime() - now);
        if (sleepTime > 0)
            usleep(sleepTime);

        if(!bPlayMP3){
            checkExit();
        }else{
            if(mBootVideoPlayState == MEDIA_PLAYBACK_COMPLETE || mBootVideoPlayState == MEDIA_ERROR) {
               checkExit();
            }
        }
    } while (!exitPending());

    glDeleteTextures(1, &mAndroid[0].name);
    glDeleteTextures(1, &mAndroid[1].name);
    return false;
}

void BootAnimation::checkExit() {
    // Allow surface flinger to gracefully request shutdown
    char value[PROPERTY_VALUE_MAX];
    property_get(EXIT_PROP_NAME, value, "0");
    int exitnow = atoi(value);
    if (exitnow) {
        requestExit();
        mCallbacks->shutdown();
    }
}

bool BootAnimation::validClock(const Animation::Part& part) {
    return part.clockPosX != TEXT_MISSING_VALUE && part.clockPosY != TEXT_MISSING_VALUE;
}

bool parseTextCoord(const char* str, int* dest) {
    if (strcmp("c", str) == 0) {
        *dest = TEXT_CENTER_VALUE;
        return true;
    }

    char* end;
    int val = (int) strtol(str, &end, 0);
    if (end == str || *end != '\0' || val == INT_MAX || val == INT_MIN) {
        return false;
    }
    *dest = val;
    return true;
}

// Parse two position coordinates. If only string is non-empty, treat it as the y value.
void parsePosition(const char* str1, const char* str2, int* x, int* y) {
    bool success = false;
    if (strlen(str1) == 0) {  // No values were specified
        // success = false
    } else if (strlen(str2) == 0) {  // we have only one value
        if (parseTextCoord(str1, y)) {
            *x = TEXT_CENTER_VALUE;
            success = true;
        }
    } else {
        if (parseTextCoord(str1, x) && parseTextCoord(str2, y)) {
            success = true;
        }
    }

    if (!success) {
        *x = TEXT_MISSING_VALUE;
        *y = TEXT_MISSING_VALUE;
    }
}

// Parse a color represented as an HTML-style 'RRGGBB' string: each pair of
// characters in str is a hex number in [0, 255], which are converted to
// floating point values in the range [0.0, 1.0] and placed in the
// corresponding elements of color.
//
// If the input string isn't valid, parseColor returns false and color is
// left unchanged.
static bool parseColor(const char str[7], float color[3]) {
    float tmpColor[3];
    for (int i = 0; i < 3; i++) {
        int val = 0;
        for (int j = 0; j < 2; j++) {
            val *= 16;
            char c = str[2*i + j];
            if      (c >= '0' && c <= '9') val += c - '0';
            else if (c >= 'A' && c <= 'F') val += (c - 'A') + 10;
            else if (c >= 'a' && c <= 'f') val += (c - 'a') + 10;
            else                           return false;
        }
        tmpColor[i] = static_cast<float>(val) / 255.0f;
    }
    memcpy(color, tmpColor, sizeof(tmpColor));
    return true;
}


static bool readFile(ZipFileRO* zip, const char* name, String8& outString)
{
    ZipEntryRO entry = zip->findEntryByName(name);
    ALOGE_IF(!entry, "couldn't find %s", name);
    if (!entry) {
        return false;
    }

    FileMap* entryMap = zip->createEntryFileMap(entry);
    zip->releaseEntry(entry);
    ALOGE_IF(!entryMap, "entryMap is null");
    if (!entryMap) {
        return false;
    }

    outString.setTo((char const*)entryMap->getDataPtr(), entryMap->getDataLength());
    delete entryMap;
    return true;
}

// The font image should be a 96x2 array of character images.  The
// columns are the printable ASCII characters 0x20 - 0x7f.  The
// top row is regular text; the bottom row is bold.
status_t BootAnimation::initFont(Font* font, const char* fallback) {
    status_t status = NO_ERROR;

    if (font->map != nullptr) {
        glGenTextures(1, &font->texture.name);
        glBindTexture(GL_TEXTURE_2D, font->texture.name);

        status = initTexture(font->map, &font->texture.w, &font->texture.h);

        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    } else if (fallback != nullptr) {
        status = initTexture(&font->texture, mAssets, fallback);
    } else {
        return NO_INIT;
    }

    if (status == NO_ERROR) {
        font->char_width = font->texture.w / FONT_NUM_COLS;
        font->char_height = font->texture.h / FONT_NUM_ROWS / 2;  // There are bold and regular rows
    }

    return status;
}

void BootAnimation::drawText(const char* str, const Font& font, bool bold, int* x, int* y) {
    glEnable(GL_BLEND);  // Allow us to draw on top of the animation
    glBindTexture(GL_TEXTURE_2D, font.texture.name);

    const int len = strlen(str);
    const int strWidth = font.char_width * len;

    if (*x == TEXT_CENTER_VALUE) {
        *x = (mWidth - strWidth) / 2;
    } else if (*x < 0) {
        *x = mWidth + *x - strWidth;
    }
    if (*y == TEXT_CENTER_VALUE) {
        *y = (mHeight - font.char_height) / 2;
    } else if (*y < 0) {
        *y = mHeight + *y - font.char_height;
    }

    int cropRect[4] = { 0, 0, font.char_width, -font.char_height };

    for (int i = 0; i < len; i++) {
        char c = str[i];

        if (c < FONT_BEGIN_CHAR || c > FONT_END_CHAR) {
            c = '?';
        }

        // Crop the texture to only the pixels in the current glyph
        const int charPos = (c - FONT_BEGIN_CHAR);  // Position in the list of valid characters
        const int row = charPos / FONT_NUM_COLS;
        const int col = charPos % FONT_NUM_COLS;
        cropRect[0] = col * font.char_width;  // Left of column
        cropRect[1] = row * font.char_height * 2; // Top of row
        // Move down to bottom of regular (one char_heigh) or bold (two char_heigh) line
        cropRect[1] += bold ? 2 * font.char_height : font.char_height;
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, cropRect);

        glDrawTexiOES(*x, *y, 0, font.char_width, font.char_height);

        *x += font.char_width;
    }

    glDisable(GL_BLEND);  // Return to the animation's default behaviour
    glBindTexture(GL_TEXTURE_2D, 0);
}

// We render 12 or 24 hour time.
void BootAnimation::drawClock(const Font& font, const int xPos, const int yPos) {
    static constexpr char TIME_FORMAT_12[] = "%l:%M";
    static constexpr char TIME_FORMAT_24[] = "%H:%M";
    static constexpr int TIME_LENGTH = 6;

    time_t rawtime;
    time(&rawtime);
    struct tm* timeInfo = localtime(&rawtime);

    char timeBuff[TIME_LENGTH];
    const char* timeFormat = mTimeFormat12Hour ? TIME_FORMAT_12 : TIME_FORMAT_24;
    size_t length = strftime(timeBuff, TIME_LENGTH, timeFormat, timeInfo);

    if (length != TIME_LENGTH - 1) {
        ALOGE("Couldn't format time; abandoning boot animation clock");
        mClockEnabled = false;
        return;
    }

    char* out = timeBuff[0] == ' ' ? &timeBuff[1] : &timeBuff[0];
    int x = xPos;
    int y = yPos;
    drawText(out, font, false, &x, &y);
}

bool BootAnimation::parseAnimationDesc(Animation& animation)
{
    String8 desString;

    if (!readFile(animation.zip, "desc.txt", desString)) {
        return false;
    }
    char const* s = desString.string();

    // Parse the description file
    for (;;) {
        const char* endl = strstr(s, "\n");
        if (endl == NULL) break;
        String8 line(s, endl - s);
        const char* l = line.string();
        int fps = 0;
        int width = 0;
        int height = 0;
        int count = 0;
        int pause = 0;
        char path[ANIM_ENTRY_NAME_MAX];
        char color[7] = "000000"; // default to black if unspecified
        char clockPos1[TEXT_POS_LEN_MAX + 1] = "";
        char clockPos2[TEXT_POS_LEN_MAX + 1] = "";

        char pathType;
        if (sscanf(l, "%d %d %d", &width, &height, &fps) == 3) {
            // ALOGD("> w=%d, h=%d, fps=%d", width, height, fps);
            animation.width = width;
            animation.height = height;
            animation.fps = fps;
        } else if (sscanf(l, " %c %d %d %s #%6s %16s %16s",
                          &pathType, &count, &pause, path, color, clockPos1, clockPos2) >= 4) {
            //ALOGD("> type=%c, count=%d, pause=%d, path=%s, color=%s, clockPos1=%s, clockPos2=%s",
            //    pathType, count, pause, path, color, clockPos1, clockPos2);
            Animation::Part part;
            part.playUntilComplete = pathType == 'c';
            part.count = count;
            part.pause = pause;
            part.path = path;
            part.audioData = NULL;
            part.animation = NULL;
            if (!parseColor(color, part.backgroundColor)) {
                ALOGE("> invalid color '#%s'", color);
                part.backgroundColor[0] = 0.0f;
                part.backgroundColor[1] = 0.0f;
                part.backgroundColor[2] = 0.0f;
            }
            parsePosition(clockPos1, clockPos2, &part.clockPosX, &part.clockPosY);
            animation.parts.add(part);
        }
        else if (strcmp(l, "$SYSTEM") == 0) {
            // ALOGD("> SYSTEM");
            Animation::Part part;
            part.playUntilComplete = false;
            part.count = 1;
            part.pause = 0;
            part.audioData = NULL;
            part.animation = loadAnimation(String8(SYSTEM_BOOTANIMATION_FILE));
            if (part.animation != NULL)
                animation.parts.add(part);
        }
        s = ++endl;
    }

    return true;
}

bool BootAnimation::preloadZip(Animation& animation)
{
    // read all the data structures
    const size_t pcount = animation.parts.size();
    void *cookie = NULL;
    ZipFileRO* zip = animation.zip;
    if (!zip->startIteration(&cookie)) {
        return false;
    }

    ZipEntryRO entry;
    char name[ANIM_ENTRY_NAME_MAX];
    while ((entry = zip->nextEntry(cookie)) != NULL) {
        const int foundEntryName = zip->getEntryFileName(entry, name, ANIM_ENTRY_NAME_MAX);
        if (foundEntryName > ANIM_ENTRY_NAME_MAX || foundEntryName == -1) {
            ALOGE("Error fetching entry file name");
            continue;
        }

        const String8 entryName(name);
        const String8 path(entryName.getPathDir());
        const String8 leaf(entryName.getPathLeaf());
        if (leaf.size() > 0) {
            if (entryName == CLOCK_FONT_ZIP_NAME) {
                FileMap* map = zip->createEntryFileMap(entry);
                if (map) {
                    animation.clockFont.map = map;
                }
                continue;
            }

            for (size_t j = 0; j < pcount; j++) {
                if ((path == animation.parts[j].path) || (leaf == "audio.wav")) {
                    uint16_t method;
                    // supports only stored png files
                    if (zip->getEntryInfo(entry, &method, NULL, NULL, NULL, NULL, NULL)) {
                        if (method == ZipFileRO::kCompressStored) {
                            FileMap* map = zip->createEntryFileMap(entry);
                            if (map) {
                                Animation::Part& part(animation.parts.editItemAt(j));
                                if (leaf == "audio.wav") {
                                    // a part may have at most one audio file
                                    part.audioData = (uint8_t *)map->getDataPtr();
                                    part.audioLength = map->getDataLength();
                                    break;
                                } else if (leaf == "trim.txt") {
                                    part.trimData.setTo((char const*)map->getDataPtr(),
                                                        map->getDataLength());
                                } else {
                                    Animation::Frame frame;
                                    frame.name = leaf;
                                    frame.map = map;
                                    frame.trimWidth = animation.width;
                                    frame.trimHeight = animation.height;
                                    frame.trimX = 0;
                                    frame.trimY = 0;
                                    part.frames.add(frame);
                                }
                            }
                        } else {
                            ALOGE("bootanimation.zip is compressed; must be only stored");
                        }
                    }
                }
            }
        }
    }

    // If there is trimData present, override the positioning defaults.
    for (Animation::Part& part : animation.parts) {
        const char* trimDataStr = part.trimData.string();
        for (size_t frameIdx = 0; frameIdx < part.frames.size(); frameIdx++) {
            const char* endl = strstr(trimDataStr, "\n");
            // No more trimData for this part.
            if (endl == NULL) {
                break;
            }
            String8 line(trimDataStr, endl - trimDataStr);
            const char* lineStr = line.string();
            trimDataStr = ++endl;
            int width = 0, height = 0, x = 0, y = 0;
            if (sscanf(lineStr, "%dx%d+%d+%d", &width, &height, &x, &y) == 4) {
                Animation::Frame& frame(part.frames.editItemAt(frameIdx));
                frame.trimWidth = width;
                frame.trimHeight = height;
                frame.trimX = x;
                frame.trimY = y;
            } else {
                ALOGE("Error parsing trim.txt, line: %s", lineStr);
                break;
            }
        }
    }

    mCallbacks->init(animation.parts);

    zip->endIteration(cookie);

    return true;
}

bool BootAnimation::movie()
{
    Animation* animation = loadAnimation(mZipFileName);
    if (animation == NULL)
        return false;

    bool anyPartHasClock = false;
    for (size_t i=0; i < animation->parts.size(); i++) {
        if(validClock(animation->parts[i])) {
            anyPartHasClock = true;
            break;
        }
    }
    if (!anyPartHasClock) {
        mClockEnabled = false;
    }

    // Check if npot textures are supported
    mUseNpotTextures = false;
    String8 gl_extensions;
    const char* exts = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    if (!exts) {
        glGetError();
    } else {
        gl_extensions.setTo(exts);
        if ((gl_extensions.find("GL_ARB_texture_non_power_of_two") != -1) ||
            (gl_extensions.find("GL_OES_texture_npot") != -1)) {
            mUseNpotTextures = true;
        }
    }

    // Blend required to draw time on top of animation frames.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_FLAT);
    glDisable(GL_DITHER);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);

    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_TEXTURE_2D);
    glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    bool clockFontInitialized = false;
    if (mClockEnabled) {
        clockFontInitialized =
            (initFont(&animation->clockFont, CLOCK_FONT_ASSET) == NO_ERROR);
        mClockEnabled = clockFontInitialized;
    }

    if (mClockEnabled && !updateIsTimeAccurate()) {
        mTimeCheckThread = new TimeCheckThread(this);
        mTimeCheckThread->run("BootAnimation::TimeCheckThread", PRIORITY_NORMAL);
    }

    playAnimation(*animation);

    if (mTimeCheckThread != nullptr) {
        mTimeCheckThread->requestExit();
        mTimeCheckThread = nullptr;
    }

    releaseAnimation(animation);

    if (clockFontInitialized) {
        glDeleteTextures(1, &animation->clockFont.texture.name);
    }

    return false;
}

bool BootAnimation::playAnimation(const Animation& animation)
{
    const size_t pcount = animation.parts.size();
    nsecs_t frameDuration = s2ns(1) / animation.fps;
    const int animationX = (mWidth - animation.width) / 2;
    const int animationY = (mHeight - animation.height) / 2;

    ALOGD("%sAnimationShownTiming start time: %" PRId64 "ms", mShuttingDown ? "Shutdown" : "Boot",
            elapsedRealtime());
    for (size_t i=0 ; i<pcount ; i++) {
        const Animation::Part& part(animation.parts[i]);
        const size_t fcount = part.frames.size();
        glBindTexture(GL_TEXTURE_2D, 0);

        // Handle animation package
        if (part.animation != NULL) {
            playAnimation(*part.animation);
            if (exitPending())
                break;
            continue; //to next part
        }

        for (int r=0 ; !part.count || r<part.count ; r++) {
            // Exit any non playuntil complete parts immediately
            if(exitPending() && !part.playUntilComplete)
                break;

            mCallbacks->playPart(i, part, r);

            glClearColor(
                    part.backgroundColor[0],
                    part.backgroundColor[1],
                    part.backgroundColor[2],
                    1.0f);

            for (size_t j=0 ; j<fcount && (!exitPending() || part.playUntilComplete) ; j++) {
                const Animation::Frame& frame(part.frames[j]);
                nsecs_t lastFrame = systemTime();

                if (r > 0) {
                    glBindTexture(GL_TEXTURE_2D, frame.tid);
                } else {
                    if (part.count != 1) {
                        glGenTextures(1, &frame.tid);
                        glBindTexture(GL_TEXTURE_2D, frame.tid);
                        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    }
                    int w, h;
                    initTexture(frame.map, &w, &h);
                }

                const int xc = animationX + frame.trimX;
                const int yc = animationY + frame.trimY;
                Region clearReg(Rect(mWidth, mHeight));
                clearReg.subtractSelf(Rect(xc, yc, xc+frame.trimWidth, yc+frame.trimHeight));
                if (!clearReg.isEmpty()) {
                    Region::const_iterator head(clearReg.begin());
                    Region::const_iterator tail(clearReg.end());
                    glEnable(GL_SCISSOR_TEST);
                    while (head != tail) {
                        const Rect& r2(*head++);
                        glScissor(r2.left, mHeight - r2.bottom, r2.width(), r2.height());
                        glClear(GL_COLOR_BUFFER_BIT);
                    }
                    glDisable(GL_SCISSOR_TEST);
                }
                // specify the y center as ceiling((mHeight - frame.trimHeight) / 2)
                // which is equivalent to mHeight - (yc + frame.trimHeight)
                glDrawTexiOES(xc, mHeight - (yc + frame.trimHeight),
                              0, frame.trimWidth, frame.trimHeight);
                if (mClockEnabled && mTimeIsAccurate && validClock(part)) {
                    drawClock(animation.clockFont, part.clockPosX, part.clockPosY);
                }

                eglSwapBuffers(mDisplay, mSurface);

                nsecs_t now = systemTime();
                nsecs_t delay = frameDuration - (now - lastFrame);
                //ALOGD("%lld, %lld", ns2ms(now - lastFrame), ns2ms(delay));
                lastFrame = now;

                if (delay > 0) {
                    struct timespec spec;
                    spec.tv_sec  = (now + delay) / 1000000000;
                    spec.tv_nsec = (now + delay) % 1000000000;
                    int err;
                    do {
                        err = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &spec, NULL);
                    } while (err<0 && errno == EINTR);
                }
                if(!bPlayMP3){
                    checkExit();
                }else{
                    if(mBootVideoPlayState == MEDIA_PLAYBACK_COMPLETE || mBootVideoPlayState == MEDIA_ERROR) {
                       checkExit();
                    }
                }
            }

            usleep(part.pause * ns2us(frameDuration));

            // For infinite parts, we've now played them at least once, so perhaps exit
            if(exitPending() && !part.count)
                break;
        }

    }

    // Free textures created for looping parts now that the animation is done.
    for (const Animation::Part& part : animation.parts) {
        if (part.count != 1) {
            const size_t fcount = part.frames.size();
            for (size_t j = 0; j < fcount; j++) {
                const Animation::Frame& frame(part.frames[j]);
                glDeleteTextures(1, &frame.tid);
            }
        }
    }

    return true;
}

void BootAnimation::releaseAnimation(Animation* animation) const
{
    for (Vector<Animation::Part>::iterator it = animation->parts.begin(),
         e = animation->parts.end(); it != e; ++it) {
        if (it->animation)
            releaseAnimation(it->animation);
    }
    if (animation->zip)
        delete animation->zip;
    delete animation;
}

BootAnimation::Animation* BootAnimation::loadAnimation(const String8& fn)
{
    if (mLoadedFiles.indexOf(fn) >= 0) {
        ALOGE("File \"%s\" is already loaded. Cyclic ref is not allowed",
            fn.string());
        return NULL;
    }
    ZipFileRO *zip = ZipFileRO::open(fn);
    if (zip == NULL) {
        ALOGE("Failed to open animation zip \"%s\": %s",
            fn.string(), strerror(errno));
        return NULL;
    }

    Animation *animation =  new Animation;
    animation->fileName = fn;
    animation->zip = zip;
    animation->clockFont.map = nullptr;
    mLoadedFiles.add(animation->fileName);

    parseAnimationDesc(*animation);
    if (!preloadZip(*animation)) {
        return NULL;
    }


    mLoadedFiles.remove(fn);
    return animation;
}

bool BootAnimation::updateIsTimeAccurate() {
    static constexpr long long MAX_TIME_IN_PAST =   60000LL * 60LL * 24LL * 30LL;  // 30 days
    static constexpr long long MAX_TIME_IN_FUTURE = 60000LL * 90LL;  // 90 minutes

    if (mTimeIsAccurate) {
        return true;
    }
    if (mShuttingDown) return true;
    struct stat statResult;

    if(stat(TIME_FORMAT_12_HOUR_FLAG_FILE_PATH, &statResult) == 0) {
        mTimeFormat12Hour = true;
    }

    if(stat(ACCURATE_TIME_FLAG_FILE_PATH, &statResult) == 0) {
        mTimeIsAccurate = true;
        return true;
    }

    FILE* file = fopen(LAST_TIME_CHANGED_FILE_PATH, "r");
    if (file != NULL) {
      long long lastChangedTime = 0;
      fscanf(file, "%lld", &lastChangedTime);
      fclose(file);
      if (lastChangedTime > 0) {
        struct timespec now;
        clock_gettime(CLOCK_REALTIME, &now);
        // Match the Java timestamp format
        long long rtcNow = (now.tv_sec * 1000LL) + (now.tv_nsec / 1000000LL);
        if (ACCURATE_TIME_EPOCH < rtcNow
            && lastChangedTime > (rtcNow - MAX_TIME_IN_PAST)
            && lastChangedTime < (rtcNow + MAX_TIME_IN_FUTURE)) {
            mTimeIsAccurate = true;
        }
      }
    }

    return mTimeIsAccurate;
}

BootAnimation::TimeCheckThread::TimeCheckThread(BootAnimation* bootAnimation) : Thread(false),
    mInotifyFd(-1), mSystemWd(-1), mTimeWd(-1), mBootAnimation(bootAnimation) {}

BootAnimation::TimeCheckThread::~TimeCheckThread() {
    // mInotifyFd may be -1 but that's ok since we're not at risk of attempting to close a valid FD.
    close(mInotifyFd);
}

bool BootAnimation::TimeCheckThread::threadLoop() {
    bool shouldLoop = doThreadLoop() && !mBootAnimation->mTimeIsAccurate
        && mBootAnimation->mClockEnabled;
    if (!shouldLoop) {
        close(mInotifyFd);
        mInotifyFd = -1;
    }
    return shouldLoop;
}

bool BootAnimation::TimeCheckThread::doThreadLoop() {
    static constexpr int BUFF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1));

    // Poll instead of doing a blocking read so the Thread can exit if requested.
    struct pollfd pfd = { mInotifyFd, POLLIN, 0 };
    ssize_t pollResult = poll(&pfd, 1, 1000);

    if (pollResult == 0) {
        return true;
    } else if (pollResult < 0) {
        ALOGE("Could not poll inotify events");
        return false;
    }

    char buff[BUFF_LEN] __attribute__ ((aligned(__alignof__(struct inotify_event))));;
    ssize_t length = read(mInotifyFd, buff, BUFF_LEN);
    if (length == 0) {
        return true;
    } else if (length < 0) {
        ALOGE("Could not read inotify events");
        return false;
    }

    const struct inotify_event *event;
    for (char* ptr = buff; ptr < buff + length; ptr += sizeof(struct inotify_event) + event->len) {
        event = (const struct inotify_event *) ptr;
        if (event->wd == mSystemWd && strcmp(SYSTEM_TIME_DIR_NAME, event->name) == 0) {
            addTimeDirWatch();
        } else if (event->wd == mTimeWd && (strcmp(LAST_TIME_CHANGED_FILE_NAME, event->name) == 0
                || strcmp(ACCURATE_TIME_FLAG_FILE_NAME, event->name) == 0)) {
            return !mBootAnimation->updateIsTimeAccurate();
        }
    }

    return true;
}

void BootAnimation::TimeCheckThread::addTimeDirWatch() {
        mTimeWd = inotify_add_watch(mInotifyFd, SYSTEM_TIME_DIR_PATH,
                IN_CLOSE_WRITE | IN_MOVED_TO | IN_ATTRIB);
        if (mTimeWd > 0) {
            // No need to watch for the time directory to be created if it already exists
            inotify_rm_watch(mInotifyFd, mSystemWd);
            mSystemWd = -1;
        }
}

status_t BootAnimation::TimeCheckThread::readyToRun() {
    mInotifyFd = inotify_init();
    if (mInotifyFd < 0) {
        ALOGE("Could not initialize inotify fd");
        return NO_INIT;
    }

    mSystemWd = inotify_add_watch(mInotifyFd, SYSTEM_DATA_DIR_PATH, IN_CREATE | IN_ATTRIB);
    if (mSystemWd < 0) {
        close(mInotifyFd);
        mInotifyFd = -1;
        ALOGE("Could not add watch for %s", SYSTEM_DATA_DIR_PATH);
        return NO_INIT;
    }

    addTimeDirWatch();

    if (mBootAnimation->updateIsTimeAccurate()) {
        close(mInotifyFd);
        mInotifyFd = -1;
        return ALREADY_EXISTS;
    }

    return NO_ERROR;
}

// ---------------------------------------------------------------------------

const char* BootAnimation::initAudioPath() {
    if (!bPlayMP3) {
        ALOGD("initAudioPath: DON'T PLAY AUDIO!");
        return NULL;
    }

    int index = 0;
    if (bBootOrShutDown) {
        index = 0;
    } else {
        index = 1;
    }
#if defined(MSSI_MSSI_MTK_CARRIEREXPRESS_PACK)
    char AudioFileName[PROPERTY_VALUE_MAX];
    char OPTR[PROPERTY_VALUE_MAX];
    ALOGD(" MSSI_MTK_CARRIEREXPRESS_PACK: YES");
    ALOGD("enter initAudioPath while loop read property");
        for(int j=0;j<50;j++) {
            if (property_get(GLOBAL_DEVICE_BOOTANIM_OPTR_NAME,OPTR, NULL) > 0) {
                ALOGD("property is able to be read");
                break;
            }
            ALOGD("wait to read OPTR config property");
            usleep(100000);
        }
    if (property_get(GLOBAL_DEVICE_BOOTANIM_OPTR_NAME,OPTR, NULL) > 0) {
        for(int i=0;i<PATH_COUNT_USP;i++) {
            strcpy(AudioFileName,mResourcePath_gb[i]);
            strcat(AudioFileName,OPTR);
            ALOGD("[MtkBootAnimation %s %d]use operator = %s",__FUNCTION__,__LINE__,OPTR);
            strcat(AudioFileName,mAudioResource_gb[index]);
            ALOGD("[MtkBootAnimation %s %d]use the boot or shut audio mp3  path = %s",__FUNCTION__,__LINE__,AudioFileName);
            if (access(AudioFileName, F_OK) == 0) {
                ALOGD("initAudioPath: audio path = %s",AudioFileName);
                strcpy(mBootaudioFileName,AudioFileName);
                return mBootaudioFileName;
            }
        }
        ALOGD("[MtkBootAnimation %s %d]Sim Optr = %s do not have audio",__FUNCTION__,__LINE__,OPTR);
        return NULL;
    }
    ALOGD("Sim Optr is null");
    return NULL;
#else

    for (int i = 0; i < PATH_COUNT; i++) {
        if (access(mAudioPath[index][i], F_OK) == 0) {
            ALOGD("initAudioPath: audio path = %s", mAudioPath[index][i]);
            return mAudioPath[index][i];
        }
    }
    return NULL;
#endif
}

void BootAnimation::initBootanimationZip() {
    ZipFileRO* zipFile = NULL;
    String8     ZipFileName;
#ifdef MSSI_MTK_CARRIEREXPRESS_PACK
    char OPTR[PROPERTY_VALUE_MAX];
    // ter-service
    sp<ITerService> terService = 0;
    const String16 serviceName("terservice");
    sp<IBinder> service = defaultServiceManager()->checkService(serviceName);
    ALOGD("carrierexpress service check OK");
    if(service != NULL) {
        ALOGD("[MtkBootAnimation %s %d]= service ",__FUNCTION__,__LINE__);
        status_t terService_err = getService(serviceName,&terService);
        if (terService_err == NO_ERROR && terService->isEarlyReadServiceEnabled()) {
            ALOGD("[MtkBootAnimation %s %d]terService_err= ok",__FUNCTION__,__LINE__);
            while(true) {
                if(!terService->isEarlyDataReady()) {
                    usleep(100000);
                    ALOGD("terservice not ready");
                    continue;
                }
                break;
            }
            String8 mncStr("");
            status_t mnc_err = terService->getSimMccMnc(&mncStr);
            ALOGD("[MtkBootAnimation %s %d]mnc_err= %d",__FUNCTION__,__LINE__,mnc_err);
            if (mnc_err == NO_ERROR) {
                ALOGD("[MtkBootAnimation %s %d]mncStr= %d",__FUNCTION__,__LINE__,atoi(mncStr));
                property_set(REGIONAL_BOOTANIM_GET_MNC, mncStr);
            }
        }
    }
    ALOGD("enter while loop");
    for(int j=0;j<50;j++){
        if (property_get(GLOBAL_DEVICE_BOOTANIM_OPTR_NAME, OPTR, NULL) > 0){
            ALOGD("property is able to be read");
            usleep(100000);
            break;
        }
        ALOGD("wait bootanimation");
        usleep(100000);
    }
    if (property_get(GLOBAL_DEVICE_BOOTANIM_OPTR_NAME, OPTR, NULL) > 0){
        for(int i=0;i<PATH_COUNT_USP;i++) {
            strcpy(BootanimFileName,mResourcePath_gb[i]);
            strcat(BootanimFileName,OPTR);
            ALOGD("[MtkBootAnimation %s %d]use operator = %s",__FUNCTION__,__LINE__,OPTR);
            strcat(BootanimFileName,"/bootanimation.zip");
            ALOGD("[MtkBootAnimation %s %d]use the bootanimation zip path = %s",__FUNCTION__,__LINE__,BootanimFileName);
                if ((access(BootanimFileName, R_OK) == 0) &&
                     ((zipFile = ZipFileRO::open(BootanimFileName)) != NULL)){
                    mZip = zipFile;
                    mZipFileName = BootanimFileName;
                    break;
                }
        }
    }
    else {
        ALOGD("terservice = null, after wait 5 second use android default animation");
    }
#else
    if ((access(CUSTOM_REGIONALPHONE_DB, F_OK) == 0)||(access(SYSTEM_REGIONALPHONE_DB, F_OK) == 0)) {
        ALOGD("regionalphone.db check OK");

        // use property to set resource zip
        if (property_get(REGIONAL_BOOTANIM_FILE_NAME, BootanimFileName, NULL) <= 0) {
            ALOGD("[MtkBootAnimation %s %d]need get the bootanimation zip path for regional phone",__FUNCTION__,__LINE__);

            // get the terservice for regional phone
            sp<ITerService> terService = 0;
            const String16 serviceName("terservice");
            sp<IBinder> service = defaultServiceManager()->checkService(serviceName);
            ALOGD("regionalphone service check OK");
            if(service != NULL) {
                status_t terService_err = getService(serviceName,&terService);
                if (terService_err == NO_ERROR && terService->isEarlyReadServiceEnabled()) {
                    while(true) {
                        if(!terService->isEarlyDataReady()) {
                            usleep(100000);
                            continue;
                        }
                        if (property_get(REGIONAL_BOOTANIM_FILE_NAME, BootanimFileName, NULL) > 0) {
                            ALOGD("[MtkBootAnimation %s %d]use the bootanimation zip path = %s",__FUNCTION__,__LINE__, BootanimFileName);
                            if ((access(BootanimFileName, R_OK) == 0) &&
                                ((zipFile = ZipFileRO::open(BootanimFileName)) != NULL)) {
                                mZip = zipFile;
                               mZipFileName = BootanimFileName;
                            }
                            break;
                        } else {
                            String8 mncStr("");
                            status_t mnc_err = terService->getSimMccMnc(&mncStr);
                            ALOGD("[MtkBootAnimation %s %d]mnc_err= %d",__FUNCTION__,__LINE__,mnc_err);
                            if (mnc_err == NO_ERROR) {
                                ALOGD("[MtkBootAnimation %s %d]mncStr= %d",__FUNCTION__,__LINE__,atoi(mncStr));
                                property_set(REGIONAL_BOOTANIM_GET_MNC, mncStr);
                                int index = -1;
                                switch (atoi(mncStr)) {
                                case MNC_VODAFONE:
                                case MNC_HUTCH:
                                    index = 0;
                                    break;
                                case MNC_CHINAUNICOM1:
                                case MNC_CHINAUNICOM2:
                                    index = 1;
                                    break;
                                default :
                                    ALOGD("[MtkBootAnimation %s %d]get mnc invalid: not 46692 or 46001, quit get mnc",__FUNCTION__,__LINE__);
                                    break;
                                }
                                if (index >= 0) {
                                    for (int i = 0; i < PATH_COUNT; i++) {
                                        if ((access(mResourcePath[index][i], F_OK) == 0) &&
                                            ((zipFile = ZipFileRO::open(mResourcePath[index][i])) != NULL)) {
                                            mZip = zipFile;
                                            mZipFileName =mResourcePath[index][i];
                                            property_set(REGIONAL_BOOTANIM_FILE_NAME, mResourcePath[index][i]);
                                            ALOGD("[MtkBootAnimation %s %d]logopath= %s", __FUNCTION__, __LINE__, mResourcePath[index][i]);
                                            break;
                                        }
                                    }
                                }
                            } else {
                                ALOGD("[MtkBootAnimation %s %d]get mnc error, quit get mnc",__FUNCTION__,__LINE__);
                            }
                            break;
                        }
                    }
                } else {
                    ALOGD("terservice = null, use android default animation");
                }
            }
        } else {
            ALOGD("[MtkBootAnimation %s %d]use the bootanimation zip path = %s",__FUNCTION__,__LINE__, BootanimFileName);
            if ((access(BootanimFileName, R_OK) == 0) &&
                ((zipFile = ZipFileRO::open(BootanimFileName)) != NULL)) {
                mZip = zipFile;
                mZipFileName = BootanimFileName ;
            }
        }

    }
#endif

    if (zipFile == NULL) {
        // If the device has encryption turned on or is in process
        // of being encrypted we show the encrypted boot animation.
        char decrypt[PROPERTY_VALUE_MAX];
        property_get("vold.decrypt", decrypt, "");

        ALOGD("regionalphone.db not OK, use android default animation");
        bool encryptedAnimation = atoi(decrypt) != 0 || !strcmp("trigger_restart_min_framework", decrypt);

        if ((encryptedAnimation &&
            (access(SYSTEM_ENCRYPTED_BOOTANIMATION_FILE, R_OK) == 0) && (ZipFileName=SYSTEM_ENCRYPTED_BOOTANIMATION_FILE)
            &&((zipFile = ZipFileRO::open(SYSTEM_ENCRYPTED_BOOTANIMATION_FILE)) != NULL)) ||

            ((access(SYSTEM_BOOTANIMATION_FILE, R_OK) == 0) && (ZipFileName=SYSTEM_BOOTANIMATION_FILE)
            &&((zipFile = ZipFileRO::open(SYSTEM_BOOTANIMATION_FILE)) != NULL)) ||

            ((access(CUSTOM_BOOTANIMATION_FILE, R_OK) == 0) && (ZipFileName=CUSTOM_BOOTANIMATION_FILE)
            &&((zipFile = ZipFileRO::open(CUSTOM_BOOTANIMATION_FILE)) != NULL)) ||

            ((access(OEM_BOOTANIMATION_FILE, R_OK) == 0) && (ZipFileName=OEM_BOOTANIMATION_FILE)
            &&((zipFile = ZipFileRO::open(OEM_BOOTANIMATION_FILE)) != NULL)) ||

            ((access(USER_BOOTANIMATION_FILE, R_OK) == 0) && (ZipFileName=USER_BOOTANIMATION_FILE)
            &&((zipFile = ZipFileRO::open(USER_BOOTANIMATION_FILE)) != NULL))) {
            mZip = zipFile;
            mZipFileName = ZipFileName;
        }
    }
}

void BootAnimation::initShutanimationZip() {
    ZipFileRO* zipFile = NULL;
    String8     ZipFileName;
#if defined(MSSI_MTK_CARRIEREXPRESS_PACK)
    char ShutanimFileName[PROPERTY_VALUE_MAX];
    char OPTR[PROPERTY_VALUE_MAX];
    ALOGD(" MSSI_MTK_CARRIEREXPRESS_PACK: YES");
    ALOGD("enter while loop to read property");
    if (property_get(GLOBAL_DEVICE_BOOTANIM_OPTR_NAME,OPTR, NULL) > 0) {
        ALOGD("property is able to be read");
        for(int i=0;i<PATH_COUNT_USP;i++) {
            strcpy(ShutanimFileName, mResourcePath_gb[i]);
            strcat(ShutanimFileName,OPTR);
            ALOGD("[Shutanimation %s %d]use operator = %s",__FUNCTION__,__LINE__,OPTR);
            strcat(ShutanimFileName,"/shutanimation.zip");
            ALOGD("[Shutanimation %s %d]use the Shutanimation zip path = %s",__FUNCTION__,__LINE__,ShutanimFileName);
            if ((access(ShutanimFileName, R_OK) == 0) &&
                ((zipFile = ZipFileRO::open(ShutanimFileName)) != NULL)) {
                mZip = zipFile;
                mZipFileName = ShutanimFileName;
                break;
            }
        }
    }
    else {
        ALOGD("Sim Optr is null");
    }
#endif
    if (zipFile == NULL) {
        if (((access(SYSTEM_SHUTANIMATION_FILE, R_OK) == 0) &&(ZipFileName=SYSTEM_SHUTANIMATION_FILE)
                &&((zipFile = ZipFileRO::open(SYSTEM_SHUTANIMATION_FILE)) != NULL)) ||

                ((access(CUSTOM_SHUTANIMATION_FILE, R_OK) == 0) &&(ZipFileName=CUSTOM_SHUTANIMATION_FILE)
                &&((zipFile = ZipFileRO::open(CUSTOM_SHUTANIMATION_FILE)) != NULL)) ||

                ((access(USER_SHUTANIMATION_FILE, R_OK) == 0) &&(ZipFileName=USER_SHUTANIMATION_FILE)
                &&((zipFile = ZipFileRO::open(USER_SHUTANIMATION_FILE)) != NULL))) {
                mZip = zipFile;
                mZipFileName = ZipFileName;
        }
    }
}

status_t BootAnimation::initTexture(const char* EntryName)
{
    /* Calculate number of Mipmap levels. */
    ALOGD("[MtkBootAnimation %s %d] ",__FUNCTION__,__LINE__);
    ZipEntryRO entry = mZip->findEntryByName(EntryName);
    ALOGE_IF(!entry, "couldn't find EntryName = %s", EntryName);
    if (!entry) {
        return -1;
    }
    uint32_t actualLen;

    mZip->getEntryInfo(entry, NULL, &actualLen, NULL, NULL, NULL, NULL);
    ALOGD("[MtkBootAnimation %s %d]   actualLen = %d ",__FUNCTION__,__LINE__, actualLen);
    char* buffer = (char*) malloc(actualLen);
    if (buffer == NULL) {
        ALOGD("[MtkBootAnimation %s %d]  malloc failed",__FUNCTION__,__LINE__);
        mZip->releaseEntry(entry);
        return NO_MEMORY;
    }
    if (!mZip->uncompressEntry(entry, buffer, actualLen)) {
       ALOGD("[MtkBootAnimation %s %d]  uncompressEntry failed",__FUNCTION__,__LINE__);
       mZip->releaseEntry(entry);
       return -1;
    }
    mZip->releaseEntry(entry);
    int width = etc1_pkm_get_width((unsigned char *)buffer);
    int height = etc1_pkm_get_height((unsigned char *)buffer);

    int Size = etc1_get_encoded_data_size(width, height);

    /* Load base Mipmap level into level 0 of texture.
     * Skip the 16 byte header of the PKM file before passing the data to OpenGL ES.
     * Data size (taken in number of bytes) of the texture is:
     *      Number of pixels = padded width * padded height.
     *      The number of pixels is divided by two as there are 4 bits per pixel in ETC (half a byte)
     */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_ETC1_RGB8_OES, width, height, 0, Size, buffer + 16);
    ALOGD("[MtkBootAnimation %s %d]  texture width = %d, height =%d",__FUNCTION__,__LINE__, width, height);

    free(buffer);
    buffer = NULL;
    return NO_ERROR;
}

// ---------------------------------------------------------------------------

GLuint BootAnimation::buildShader(const char* source, GLenum shaderType)
{
    GLuint shaderHandle = glCreateShader(shaderType);

    if (shaderHandle)
    {
        glShaderSource(shaderHandle, 1, &source, 0);
        glCompileShader(shaderHandle);

        GLint compiled = 0;
        glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            GLint infoLen = 0;
            glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen)
            {
                char* buf = (char*) malloc(infoLen);
                if (buf)
                {
                    glGetShaderInfoLog(shaderHandle, infoLen, NULL, buf);
                    ALOGD("error::Could not compile shader %d:\n%s\n", shaderType, buf);
                    free(buf);
                    buf = NULL;
                }
                glDeleteShader(shaderHandle);
                shaderHandle = 0;
            }
        }

    }

    return shaderHandle;
}

GLuint BootAnimation::buildProgram (const char* vertexShaderSource,
        const char* fragmentShaderSource)
{
    GLuint vertexShader = buildShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = buildShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    GLuint programHandle = glCreateProgram();

    if (programHandle)
    {
        glAttachShader(programHandle, vertexShader);
        glAttachShader(programHandle, fragmentShader);
        glLinkProgram(programHandle);

        GLint linkStatus = GL_FALSE;
        glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(programHandle, bufLength, NULL, buf);
                    ALOGD("error::Could not link program:\n%s\n", buf);
                    free(buf);
                    buf = NULL;
                }
            }
            glDeleteProgram(programHandle);
            programHandle = 0;
        }

    }
    return programHandle;
}

 void BootAnimation::initShader() {

    const char* VERTEX_SHADER =
        "attribute vec4 a_position;\n"
        "attribute vec2 a_texCoord;\n"
        "varying vec2 v_texCoord;\n"
        "void main()\n"
        "{\n"
            "gl_Position = a_position;\n"
            "v_texCoord = a_texCoord;\n"
        "}\n";

    const char* FRAG_SHADER =
        "precision mediump float;\n"
        "varying vec2 v_texCoord;\n"
        "uniform sampler2D u_samplerTexture;\n"
        "void main()\n"
        "{\n"
            "gl_FragColor = texture2D(u_samplerTexture, v_texCoord);\n"
        "}\n";

    mProgram = buildProgram(VERTEX_SHADER, FRAG_SHADER);
    mAttribPosition = glGetAttribLocation(mProgram, "a_position");
    mAttribTexCoord = glGetAttribLocation(mProgram, "a_texCoord");
    mUniformTexture = glGetUniformLocation(mProgram, "u_samplerTexture");
    glUseProgram (mProgram);
}

bool BootAnimation::ETC1movie()
{
    ZipEntryRO desc = mZip->findEntryByName("desc.txt");
    ALOGE_IF(!desc, "couldn't find desc.txt");
    if (!desc) {
        return false;
    }
    uint32_t uncomplen = 0;
    char* tmp = NULL;
    mZip->getEntryInfo(desc, NULL, &uncomplen, NULL, NULL, NULL, NULL);
    ALOGD("[MtkBootAnimation %s %d]   uncomplen = %d ",__FUNCTION__,__LINE__, uncomplen);
    tmp = (char*) malloc(uncomplen);
    mZip->uncompressEntry(desc, tmp, uncomplen);
    String8 desString((char const*)tmp, uncomplen);
    free(tmp);
    tmp = NULL;
    mZip->releaseEntry(desc);
    char const* s = desString.string();

    Animation animation;

    // Parse the description file
    for (;;) {
        const char* endl = strstr(s, "\n");
        if (!endl) break;
        String8 line(s, endl - s);
        const char* l = line.string();
        int fps, width, height, count, pause;
        char path[ANIM_ENTRY_NAME_MAX];
        char color[7] = "000000"; // default to black if unspecified
        char pathType;
        if (sscanf(l, "%d %d %d", &width, &height, &fps) == 3) {
            ALOGD("> w=%d, h=%d, fps=%d", width, height, fps); // add log
            animation.width = width;
            animation.height = height;
            animation.fps = fps;
        }
        else if (sscanf(l, "%c %d %d %s", &pathType, &count, &pause, path) == 4) {
            ALOGD("> type=%c, count=%d, pause=%d, path=%s", pathType, count, pause, path); // add log
            Animation::Part part;
            part.playUntilComplete = pathType == 'c';
            part.count = count;
            part.pause = pause;
            part.path = path;
            animation.parts.add(part);
            if (!parseColor(color, part.backgroundColor)) {
                ALOGD("> invalid color '#%s'", color);
                part.backgroundColor[0] = 0.0f;
                part.backgroundColor[1] = 0.0f;
                part.backgroundColor[2] = 0.0f;
            }
        }

        s = ++endl;
    }

    // read all the data structures
    const size_t pcount = animation.parts.size();
    void *cookie = NULL;
    if (!mZip->startIteration(&cookie)) {
        return false;
    }

    ZipEntryRO entry;
    char name[ANIM_ENTRY_NAME_MAX];
    while ((entry = mZip->nextEntry(cookie)) != NULL) {
        const int foundEntryName = mZip->getEntryFileName(entry, name, ANIM_ENTRY_NAME_MAX);
        if (foundEntryName > ANIM_ENTRY_NAME_MAX || foundEntryName == -1) {
            ALOGE("Error fetching entry file name");
            continue;
        }

        const String8 entryName(name);
        const String8 path(entryName.getPathDir());
        const String8 leaf(entryName.getPathLeaf());
        if (leaf.size() > 0) {
            for (size_t j=0 ; j<pcount ; j++) {
                if (path == animation.parts[j].path) {
                    Animation::Frame frame;
                    frame.name = leaf;
                            // frame.map = map;
                    frame.fullPath = entryName;
                    Animation::Part& part(animation.parts.editItemAt(j));
                    part.frames.add(frame);
                }
            }
        }
    }

    mZip->endIteration(cookie);

    initShader();

    glViewport((mWidth - animation.width) >> 1, (mHeight - animation.height) >> 1,
            animation.width, animation.height);
    // clear screen
    glDisable(GL_DITHER);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(mDisplay, mSurface);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(mUniformTexture, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    nsecs_t lastFrame = systemTime();
    nsecs_t frameDuration = s2ns(1) / animation.fps;
    ALOGD("[MtkBootAnimation %s %d]lastFrame=%lld,frameDuration=%lld,ms=%lld",__FUNCTION__,__LINE__,(long long)lastFrame,(long long)frameDuration,(long long)ns2ms(systemTime()));

    for (size_t i=0 ; i<pcount ; i++) {
        const Animation::Part& part(animation.parts[i]);
        const size_t fcount = part.frames.size();
        ALOGD("[MtkBootAnimation %s %d]i=%zu,i<pcount=%zu,r<part.count=%d,j<fcount=%zu",__FUNCTION__,__LINE__,i,pcount,part.count,fcount);
        glBindTexture(GL_TEXTURE_2D, 0);

        for (int r=0 ; !part.count || r<part.count ; r++) {
            // Exit any non playuntil complete parts immediately
            if(exitPending() && !part.playUntilComplete) {
                //ALOGD("[MtkBootAnimation %s %d]part.playUntilComplete=%d", __FUNCTION__, __LINE__ ,part.playUntilComplete);
                break;
             }
            glClearColor(
                    part.backgroundColor[0],
                    part.backgroundColor[1],
                    part.backgroundColor[2],
                    1.0f);
            for (size_t j=0 ; j<fcount && (!exitPending() || part.playUntilComplete) ; j++) {
                const Animation::Frame& frame(part.frames[j]);
                nsecs_t lastFrame = systemTime();
                //ALOGD("[MtkBootAnimation %s %d]i=%d,r=%d,j=%d,lastFrame=%lld(%lld ms),file=%s",__FUNCTION__,__LINE__,i,r,j,lastFrame,ns2ms(lastFrame),frame.name.string());
                if (r > 0) {
                    glBindTexture(GL_TEXTURE_2D, frame.tid);
                } else {
                    if (part.count != 1) {
                        glGenTextures(1, &frame.tid);
                        glBindTexture(GL_TEXTURE_2D, frame.tid);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    }
                    initTexture(frame.fullPath.string());
                }

                static GLfloat quadVertex[] = { -1.0f,  1.0f, 0.0f,  // Position 0
                                                0.0f,  0.0f,        // TexCoord 0
                                                -1.0f, -1.0f, 0.0f,  // Position 1
                                                0.0f,  1.0f,        // TexCoord 1
                                                1.0f, -1.0f, 0.0f,  // Position 2
                                                1.0f,  1.0f,        // TexCoord 2
                                                1.0f,  1.0f, 0.0f,  // Position 3
                                                1.0f,  0.0f         // TexCoord 3
                                              };

                static GLushort quadIndex[] = { 0, 1, 2, 0, 2, 3 };
                glVertexAttribPointer(mAttribPosition,
                                     3, GL_FLOAT,
                                     false, 5*sizeof(GL_FLOAT), quadVertex);

                glVertexAttribPointer(mAttribTexCoord,
                                     2, GL_FLOAT,
                                     false, 5*sizeof(GL_FLOAT), &quadVertex[3]);
                glEnableVertexAttribArray(mAttribPosition);
                glEnableVertexAttribArray(mAttribTexCoord);

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, quadIndex);
                eglSwapBuffers(mDisplay, mSurface);

                nsecs_t now = systemTime();
                nsecs_t delay = frameDuration - (now - lastFrame);
                //ALOGD("[MtkBootAnimation %s %d]%lld,delay=%lld",__FUNCTION__,__LINE__,ns2ms(now - lastFrame), ns2ms(delay));
                lastFrame = now;

                if (delay > 0) {
                    struct timespec spec;
                    spec.tv_sec  = (now + delay) / 1000000000;
                    spec.tv_nsec = (now + delay) % 1000000000;
                    int err;
                    do {
                        err = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &spec, NULL);
                    } while (err<0 && errno == EINTR);
                }
               if(!bPlayMP3){
                    checkExit();
               }else{
                    if(mBootVideoPlayState == MEDIA_PLAYBACK_COMPLETE || mBootVideoPlayState == MEDIA_ERROR) {
                        checkExit();
                    }
               }
            }

            usleep(part.pause * ns2us(frameDuration));

            // For infinite parts, we've now played them at least once, so perhaps exit
            if(exitPending() && !part.count) {
                ALOGD("[MtkBootAnimation %s %d]break,exitPending()=%d,part.count=%d",__FUNCTION__,__LINE__,exitPending(),part.count);
                break;
            }
        }

        // free the textures for this part
        if (part.count != 1) {
            for (size_t j=0 ; j<fcount ; j++) {
                const Animation::Frame& frame(part.frames[j]);
                glDeleteTextures(1, &frame.tid);
                ALOGD("[MtkBootAnimation %s %d]del,part.count=%d,j=%zu,fcount=%zu",__FUNCTION__,__LINE__,part.count,j,fcount);
            }
        }
    }
    ALOGD("[MtkBootAnimation %s %d]end",__FUNCTION__,__LINE__);
    return false;
}
}
; // namespace android
