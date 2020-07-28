include $(CLEAR_VARS)
LOCAL_MODULE := libmtee_serv
LOCAL_SRC_FILES := prebuilts/libs/$(PROJECT)/lib/arm/mtee_serv.mod.a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libmtee_api
LOCAL_SRC_FILES := prebuilts/libs/$(PROJECT)/lib/arm/mtee_api.mod.a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libgp_client
LOCAL_SRC_FILES := prebuilts/libs/$(PROJECT)/lib/arm/gp_client.mod.a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libc-trusty
LOCAL_SRC_FILES := prebuilts/libs/$(PROJECT)/lib/arm/libc-trusty.mod.a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libstdc++-trusty
LOCAL_SRC_FILES := prebuilts/libs/$(PROJECT)/lib/arm/libstdc++-trusty.mod.a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcxx-mtee
LOCAL_SRC_FILES := prebuilts/libs/$(PROJECT)/lib/arm/libcxx-mtee.mod.a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libm-mtee
LOCAL_SRC_FILES := prebuilts/libs/$(PROJECT)/lib/arm/libm-mtee.mod.a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libc.mod
LOCAL_SRC_FILES := prebuilts/libs/$(PROJECT)/lib/arm/libc.mod.a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libhwkey
LOCAL_SRC_FILES := prebuilts/libs/$(PROJECT)/lib/arm/hwkey.mod.a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libmtee_boringssl
LOCAL_SRC_FILES := prebuilts/libs/$(PROJECT)/lib/arm/boringssl.mod.a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libopenssl_stubs
LOCAL_SRC_FILES := prebuilts/libs/$(PROJECT)/lib/arm/openssl-stubs.mod.a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := librng
LOCAL_SRC_FILES := prebuilts/libs/$(PROJECT)/lib/arm/rng.mod.a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libmtee_storage
LOCAL_SRC_FILES := prebuilts/libs/$(PROJECT)/lib/arm/storage.mod.a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

# fpp lib
include $(CLEAR_VARS)
LOCAL_MODULE := libmegtee
LOCAL_SRC_FILES := prebuilts/libs/$(PROJECT)/lib/arm/libmegtee.a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

