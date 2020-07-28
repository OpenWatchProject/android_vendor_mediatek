LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

LOCAL_MODULE := check-mtk-hidl
LOCAL_MODULE_CLASS := FAKE

my_hal_files := $(shell find $(LOCAL_PATH) -type f -name "*.hal")
intermediates := $(call local-intermediates-dir)

GEN1 := $(intermediates)/current.txt
$(GEN1): $(LOCAL_PATH)/current.txt
	@mkdir -p $(dir $@)
	@cat $< | cut -d ' ' -f 2 | sort > $@

GEN2 := $(intermediates)/update.txt
$(GEN2): PRIVATE_HAL_FILES := $(patsubst $(LOCAL_PATH)/%,%,$(my_hal_files))
$(GEN2): PRIVATE_HAL_PREFIX := vendor/mediatek/hardware/
$(GEN2): $(my_hal_files)
	@mkdir -p $(dir $@)
	@rm -f $@.tmp
	@$(foreach f,$(PRIVATE_HAL_FILES), echo $(PRIVATE_HAL_PREFIX)$(f) | sed -e 's/\/\([0-9]\+\.[0-9]\+\)\//@\1::/g' -e 's/\//./g' -e 's/\.hal$$//' >>$@.tmp;)
	@touch $@.tmp
	@cat $@.tmp | sort > $@

GEN3 := $(intermediates)/diff.txt
LOCAL_ADDITIONAL_DEPENDENCIES += $(GEN3)
$(GEN3): PRIVATE_TXT := $(LOCAL_PATH)/current.txt
$(GEN3): $(GEN1) $(GEN2)
	@mkdir -p $(dir $@)
	@diff --report-identical-files --ignore-blank-lines --suppress-common-lines --side-by-side $^ > $@ || (echo ERROR: $(PRIVATE_TXT) differs from expected: ; cat $@; false)
	@touch $@

include $(BUILD_PHONY_PACKAGE)
sys_images:check-mtk-hidl
droid:check-mtk-hidl
include $(call first-makefiles-under,$(LOCAL_PATH))