ifeq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 0)
# Local modifications:
# * removed com.google.android.geo.API_KEY key. This should be added to
#      the manifest files in java/com/android/incallui/calllocation/impl/
#      and /java/com/android/incallui/maps/impl/
# * b/62417801 modify translation string naming convention:
#      $ find . -type d | grep 262 | rename 's/(values)\-([a-zA-Z\+\-]+)\-(mcc262-mnc01)/$1-$3-$2/'
# * b/37077388 temporarily disable proguard with javac
# * b/62875795 include manually generated GRPC service class:
#      $ protoc --plugin=protoc-gen-grpc-java=prebuilts/tools/common/m2/repository/io/grpc/protoc-gen-grpc-java/1.0.3/protoc-gen-grpc-java-1.0.3-linux-x86_64.exe \
#               --grpc-java_out=lite:"packages/apps/Dialer/java/com/android/voicemail/impl/" \
#               --proto_path="packages/apps/Dialer/java/com/android/voicemail/impl/transcribe/grpc/" "packages/apps/Dialer/java/com/android/voicemail/impl/transcribe/grpc/voicemail_transcription.proto"
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# The base directory for Dialer sources.
BASE_DIR := java/com/android

# Exclude files incompatible with AOSP.
EXCLUDE_FILES := \
	$(BASE_DIR)/incallui/calllocation/impl/AuthException.java \
	$(BASE_DIR)/incallui/calllocation/impl/CallLocationImpl.java \
	$(BASE_DIR)/incallui/calllocation/impl/CallLocationModule.java \
	$(BASE_DIR)/incallui/calllocation/impl/DownloadMapImageTask.java \
	$(BASE_DIR)/incallui/calllocation/impl/GoogleLocationSettingHelper.java \
	$(BASE_DIR)/incallui/calllocation/impl/HttpFetcher.java \
	$(BASE_DIR)/incallui/calllocation/impl/LocationFragment.java \
	$(BASE_DIR)/incallui/calllocation/impl/LocationHelper.java \
	$(BASE_DIR)/incallui/calllocation/impl/LocationPresenter.java \
	$(BASE_DIR)/incallui/calllocation/impl/LocationUrlBuilder.java \
	$(BASE_DIR)/incallui/calllocation/impl/ReverseGeocodeTask.java \
	$(BASE_DIR)/incallui/calllocation/impl/TrafficStatsTags.java \
	$(BASE_DIR)/incallui/maps/impl/MapsImpl.java \
	$(BASE_DIR)/incallui/maps/impl/MapsModule.java \
	$(BASE_DIR)/incallui/maps/impl/StaticMapFragment.java \

# Exclude testing only class, not used anywhere here
EXCLUDE_FILES += \
	$(BASE_DIR)/contacts/common/format/testing/SpannedTestUtils.java

# Exclude rootcomponentgenerator
EXCLUDE_FILES += \
	$(call all-java-files-under, $(BASE_DIR)/dialer/rootcomponentgenerator) \
	$(call all-java-files-under, $(BASE_DIR)/dialer/inject/demo)

# Exclude build variants for now
EXCLUDE_FILES += \
	$(BASE_DIR)/dialer/constants/googledialer/ConstantsImpl.java \
	$(BASE_DIR)/dialer/binary/google/GoogleStubDialerRootComponent.java \
	$(BASE_DIR)/dialer/binary/google/GoogleStubDialerApplication.java \

# * b/62875795
ifneq ($(wildcard vendor/mediatek/proprietary/packages/apps/Dialer/java/com/android/voicemail/impl/com/google/internal/communications/voicemailtranscription/v1/VoicemailTranscriptionServiceGrpc.java),)
$(error Please remove file vendor/mediatek/proprietary/packages/apps/Dialer/java/com/android/voicemail/impl/com/google/internal/communications/voicemailtranscription/v1/VoicemailTranscriptionServiceGrpc.java )
endif

EXCLUDE_RESOURCE_DIRECTORIES := \
	java/com/android/incallui/maps/impl/res \

# All Dialers resources.
RES_DIRS := $(call all-subdir-named-dirs,res,.)
RES_DIRS := $(filter-out $(EXCLUDE_RESOURCE_DIRECTORIES),$(RES_DIRS))

EXCLUDE_MANIFESTS := \
	$(BASE_DIR)/dialer/binary/aosp/testing/AndroidManifest.xml \
	$(BASE_DIR)/dialer/binary/google/AndroidManifest.xml \
	$(BASE_DIR)/incallui/calllocation/impl/AndroidManifest.xml \
	$(BASE_DIR)/incallui/maps/impl/AndroidManifest.xml \

# Dialer manifest files to merge.
DIALER_MANIFEST_FILES := $(call all-named-files-under,AndroidManifest.xml,.)
DIALER_MANIFEST_FILES := $(filter-out $(EXCLUDE_MANIFESTS),$(DIALER_MANIFEST_FILES))

# Merge all manifest files.
LOCAL_FULL_LIBS_MANIFEST_FILES := \
	$(addprefix $(LOCAL_PATH)/, $(DIALER_MANIFEST_FILES))

LOCAL_SRC_FILES := $(call all-java-files-under, $(BASE_DIR))
LOCAL_SRC_FILES += $(call all-proto-files-under, $(BASE_DIR))
LOCAL_SRC_FILES += $(call all-Iaidl-files-under, $(BASE_DIR))
LOCAL_SRC_FILES := $(filter-out $(EXCLUDE_FILES),$(LOCAL_SRC_FILES))
# M: fix runtime ClassCastException issue ALPS04409704
LOCAL_SRC_FILES += $(call all-java-files-under, java/com/bumptech)

LOCAL_AIDL_INCLUDES := $(call all-Iaidl-files-under, $(BASE_DIR))

LOCAL_PROTOC_FLAGS := --proto_path=$(LOCAL_PATH)

LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(RES_DIRS))

EXCLUDE_EXTRA_PACKAGES := \
	com.android.dialer.binary.aosp.testing \
	com.android.dialer.binary.google \
	com.android.incallui.calllocation.impl \
	com.android.incallui.maps.impl \

# We specify each package explicitly to glob resource files.
include ${LOCAL_PATH}/packages.mk

LOCAL_AAPT_FLAGS := $(filter-out $(EXCLUDE_EXTRA_PACKAGES),$(LOCAL_AAPT_FLAGS))
LOCAL_AAPT_FLAGS := $(addprefix --extra-packages , $(LOCAL_AAPT_FLAGS))
LOCAL_AAPT_FLAGS += \
	--auto-add-overlay \
	--extra-packages me.leolin.shortcutbadger \

LOCAL_STATIC_JAVA_LIBRARIES := \
	android-common \
	android-support-dynamic-animation \
	com.android.vcard \
	dialer-animal-sniffer-annotations-target-vendor \
	dialer-commons-io-target-vendor \
	dialer-dagger2-target-vendor \
	dialer-disklrucache-target-vendor \
	dialer-gifdecoder-target-vendor \
	dialer-glide-target-vendor \
	dialer-grpc-all-target-vendor \
	dialer-grpc-context-target-vendor \
	dialer-grpc-core-target-vendor \
	dialer-grpc-okhttp-target-vendor \
	dialer-grpc-protobuf-lite-target-vendor \
	dialer-grpc-stub-target-vendor \
	dialer-j2objc-annotations-target-vendor \
	dialer-javax-annotation-api-target-vendor \
	dialer-javax-inject-target-vendor \
	dialer-libshortcutbadger-target-vendor \
	dialer-mime4j-core-target-vendor \
	dialer-mime4j-dom-target-vendor \
	dialer-okhttp-target-vendor \
	dialer-okio-target-vendor \
	dialer-error-prone-target-vendor \
	dialer-guava-target-vendor \
	dialer-glide-target-vendor \
	dialer-glide-annotation-target-vendor \
	dialer-zxing-target-vendor \
	jsr305 \
	libbackup \
	libphonenumber \
	volley \

LOCAL_STATIC_ANDROID_LIBRARIES := \
	android-support-core-ui \
	$(ANDROID_SUPPORT_DESIGN_TARGETS) \
	android-support-transition \
	android-support-v13 \
	android-support-v4 \
	android-support-v7-appcompat \
	android-support-v7-cardview \
	android-support-v7-recyclerview \

LOCAL_JAVA_LIBRARIES := \
	dialer-auto-value-target-vendor \
	org.apache.http.legacy \

LOCAL_ANNOTATION_PROCESSORS := \
	dialer-auto-value-vendor \
	javapoet-prebuilt-jar \
	dialer-dagger2-vendor \
	dialer-dagger2-compiler-vendor \
	dialer-dagger2-producers-vendor \
	dialer-glide-annotation-vendor \
	dialer-glide-compiler-vendor \
	dialer-guava-vendor \
	dialer-javax-annotation-api-vendor \
	dialer-javax-inject-vendor \
	dialer-rootcomponentprocessor-vendor

LOCAL_ANNOTATION_PROCESSOR_CLASSES := \
  com.google.auto.value.processor.AutoValueProcessor,dagger.internal.codegen.ComponentProcessor,com.bumptech.glide.annotation.compiler.GlideAnnotationProcessor,com.android.dialer.rootcomponentgenerator.RootComponentProcessor

# Proguard includes
LOCAL_PROGUARD_FLAG_FILES := proguard.flags $(call all-named-files-under,proguard.*flags,$(BASE_DIR))
LOCAL_PROGUARD_ENABLED := custom

LOCAL_PROGUARD_ENABLED += optimization

LOCAL_MODULE_TAGS := optional
LOCAL_PACKAGE_NAME := MtkDialer
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true
LOCAL_PRODUCT_MODULE := true
LOCAL_USE_AAPT2 := true
LOCAL_OVERRIDES_PACKAGES := Dialer
LOCAL_REQUIRED_MODULES := privapp_whitelist_com.android.dialer

# M: MediaTek Add.
include $(LOCAL_PATH)/ext.mk

include $(BUILD_PACKAGE)

# Cleanup local state
BASE_DIR :=
EXCLUDE_FILES :=
RES_DIRS :=
DIALER_MANIFEST_FILES :=
EXCLUDE_MANIFESTS :=
EXCLUDE_EXTRA_PACKAGES :=

# Create references to prebuilt libraries.
include $(CLEAR_VARS)

LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := \
    dialer-auto-value-vendor:../../../../../../prebuilts/tools/common/m2/repository/com/google/auto/value/auto-value/1.5.2/auto-value-1.5.2.jar \
    dialer-dagger2-compiler-vendor:../../../../../../prebuilts/tools/common/m2/repository/com/google/dagger/dagger-compiler/2.7/dagger-compiler-2.7.jar \
    dialer-dagger2-vendor:../../../../../../prebuilts/tools/common/m2/repository/com/google/dagger/dagger/2.7/dagger-2.7.jar \
    dialer-dagger2-producers-vendor:../../../../../../prebuilts/tools/common/m2/repository/com/google/dagger/dagger-producers/2.7/dagger-producers-2.7.jar \
    dialer-glide-annotation-vendor:../../../../../../prebuilts/maven_repo/bumptech/com/github/bumptech/glide/annotation/SNAPSHOT/annotation-SNAPSHOT.jar \
    dialer-glide-compiler-vendor:../../../../../../prebuilts/maven_repo/bumptech/com/github/bumptech/glide/compiler/SNAPSHOT/compiler-SNAPSHOT.jar \
    dialer-grpc-all-vendor:../../../../../../prebuilts/tools/common/m2/repository/io/grpc/grpc-all/1.0.3/grpc-all-1.0.3.jar \
    dialer-grpc-core-vendor:../../../../../../prebuilts/tools/common/m2/repository/io/grpc/grpc-core/1.0.3/grpc-core-1.0.3.jar \
    dialer-grpc-okhttp-vendor:../../../../../../prebuilts/tools/common/m2/repository/io/grpc/grpc-okhttp/1.0.3/grpc-okhttp-1.0.3.jar \
    dialer-grpc-protobuf-lite-vendor:../../../../../../prebuilts/tools/common/m2/repository/io/grpc/grpc-protobuf-lite/1.0.3/grpc-protobuf-lite-1.0.3.jar \
    dialer-grpc-stub-vendor:../../../../../../prebuilts/tools/common/m2/repository/io/grpc/grpc-stub/1.0.3/grpc-stub-1.0.3.jar \
    dialer-guava-vendor:../../../../../../prebuilts/tools/common/m2/repository/com/google/guava/guava/23.0/guava-23.0.jar \
    dialer-javax-annotation-api-vendor:../../../../../../prebuilts/tools/common/m2/repository/javax/annotation/javax.annotation-api/1.2/javax.annotation-api-1.2.jar \
    dialer-javax-inject-vendor:../../../../../../prebuilts/tools/common/m2/repository/javax/inject/javax.inject/1/javax.inject-1.jar \
    dialer-auto-service-vendor:../../../../../../prebuilts/tools/common/m2/repository/com/google/auto/service/auto-service/1.0-rc2/auto-service-1.0-rc2.jar \
    dialer-auto-common-vendor:../../../../../../prebuilts/tools/common/m2/repository/com/google/auto/auto-common/0.9/auto-common-0.9.jar \

include $(BUILD_HOST_PREBUILT)

# Enumerate target prebuilts to avoid linker warnings like
# Dialer (java:sdk) should not link to dialer-guava (java:platform)
include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-guava-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/com/google/guava/guava/23.0/guava-23.0.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-error-prone-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/com/google/errorprone/error_prone_annotations/2.0.18/error_prone_annotations-2.0.18.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-dagger2-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/com/google/dagger/dagger/2.7/dagger-2.7.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-disklrucache-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/maven_repo/bumptech/com/github/bumptech/glide/disklrucache/SNAPSHOT/disklrucache-SNAPSHOT.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-gifdecoder-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/maven_repo/bumptech/com/github/bumptech/glide/gifdecoder/SNAPSHOT/gifdecoder-SNAPSHOT.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-glide-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/maven_repo/bumptech/com/github/bumptech/glide/glide/SNAPSHOT/glide-SNAPSHOT.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-glide-annotation-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/maven_repo/bumptech/com/github/bumptech/glide/annotation/SNAPSHOT/annotation-SNAPSHOT.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-javax-annotation-api-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/javax/annotation/javax.annotation-api/1.2/javax.annotation-api-1.2.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-libshortcutbadger-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/me/leolin/ShortcutBadger/1.1.13/ShortcutBadger-1.1.13.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-javax-inject-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/javax/inject/javax.inject/1/javax.inject-1.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-commons-io-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/commons-io/commons-io/2.4/commons-io-2.4.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-mime4j-core-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/org/apache/james/apache-mime4j-core/0.7.2/apache-mime4j-core-0.7.2.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-mime4j-dom-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/org/apache/james/apache-mime4j-dom/0.7.2/apache-mime4j-dom-0.7.2.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-grpc-core-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/io/grpc/grpc-core/1.0.3/grpc-core-1.0.3.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-grpc-okhttp-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/io/grpc/grpc-okhttp/1.0.3/grpc-okhttp-1.0.3.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-grpc-protobuf-lite-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/io/grpc/grpc-protobuf-lite/1.0.3/grpc-protobuf-lite-1.0.3.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-grpc-stub-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/io/grpc/grpc-stub/1.0.3/grpc-stub-1.0.3.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-grpc-all-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/io/grpc/grpc-all/1.0.3/grpc-all-1.0.3.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-grpc-context-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/io/grpc/grpc-context/1.0.3/grpc-context-1.0.3.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-auto-value-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/com/google/auto/value/auto-value/1.5.2/auto-value-1.5.2.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-zxing-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../external/zxing/core/core.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-okhttp-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/com/squareup/okhttp/okhttp/2.7.4/okhttp-2.7.4.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-okio-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/com/squareup/okio/okio/1.9.0/okio-1.9.0.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-j2objc-annotations-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/com/google/j2objc/j2objc-annotations/1.1/j2objc-annotations-1.1.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE := dialer-animal-sniffer-annotations-target-vendor
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SRC_FILES := ../../../../../../prebuilts/tools/common/m2/repository/org/codehaus/mojo/animal-sniffer-annotations/1.14/animal-sniffer-annotations-1.14.jar
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE := dialer-rootcomponentprocessor-vendor
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_IS_HOST_MODULE := true
BASE_DIR := java/com/android

LOCAL_SRC_FILES := \
	$(call all-java-files-under, $(BASE_DIR)/dialer/rootcomponentgenerator) \
        $(BASE_DIR)/dialer/inject/DialerRootComponent.java \
        $(BASE_DIR)/dialer/inject/DialerVariant.java \
        $(BASE_DIR)/dialer/inject/HasRootComponent.java \
        $(BASE_DIR)/dialer/inject/IncludeInDialerRoot.java \
        $(BASE_DIR)/dialer/inject/InstallIn.java \
        $(BASE_DIR)/dialer/inject/RootComponentGeneratorMetadata.java

LOCAL_STATIC_JAVA_LIBRARIES := \
	dialer-guava \
	dialer-dagger2 \
	javapoet-prebuilt-jar \
	dialer-auto-service \
	dialer-auto-common \
	dialer-javax-annotation-api \
	dialer-javax-inject

LOCAL_JAVA_LANGUAGE_VERSION := 1.8

include $(BUILD_HOST_JAVA_LIBRARY)

include $(CLEAR_VARS)

# M: Use the following include to make our plugin host jar.
include $(call all-makefiles-under,$(LOCAL_PATH))

endif