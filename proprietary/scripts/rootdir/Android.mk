LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := gen_mount_point_for_ab

ifneq ($(MTK_BOARD_ROOT_EXTRA_FOLDERS),)
LOCAL_POST_INSTALL_CMD := mkdir -p $(addprefix $(TARGET_OUT_VENDOR)/,$(MTK_BOARD_ROOT_EXTRA_FOLDERS));
endif

ifdef MTK_BOARD_ROOT_EXTRA_SYMLINKS
# MTK_BOARD_ROOT_EXTRA_SYMLINKS is a list of <target>:<link_name>.
  LOCAL_POST_INSTALL_CMD += $(foreach s, $(MTK_BOARD_ROOT_EXTRA_SYMLINKS),\
    $(eval p := $(subst :,$(space),$(s)))\
     mkdir -p $(dir $(TARGET_OUT_VENDOR)/$(word 2,$(p))) \
    ; ln -sf $(word 1,$(p)) $(TARGET_OUT_VENDOR)/$(word 2,$(p));)
endif

include $(BUILD_PHONY_PACKAGE)



