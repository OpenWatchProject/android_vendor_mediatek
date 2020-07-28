define gz-transform-c-to-o
$(gz-transform-c-to-o-no-deps)
$(transform-d-to-p)
endef

define gz-transform-c-to-o-no-deps
@echo "target $(PRIVATE_ARM_MODE) C: $(PRIVATE_MODULE) <= $<"
$(call gz-transform-c-or-s-to-o-no-deps, \
    $(PRIVATE_CFLAGS) \
    $(PRIVATE_CONLYFLAGS) \
    $(PRIVATE_DEBUG_CFLAGS) \
    $(PRIVATE_CFLAGS_NO_OVERRIDE))
endef


define gz-transform-s-to-o
$(gz-transform-s-to-o-no-deps)
$(transform-d-to-p)
endef

define gz-transform-s-to-o-no-deps
@echo "target asm: $(PRIVATE_MODULE) <= $<"
$(call gz-transform-c-or-s-to-o-no-deps, $(PRIVATE_ASFLAGS))
endef

define gz-transform-c-or-s-to-o-no-deps
@mkdir -p $(dir $@)
$(hide) $(RELATIVE_PWD) $(GZ_LD_LIBRARY_PATH_arm) $(GZ_CC_arm) \
        $(addprefix -I , $(GLOBAL_GZ_INCLUDES)) \
        $(addprefix -I , $(PRIVATE_C_INCLUDES)) \
        -c \
        -fno-exceptions \
        $(GLOBAL_GZ_COMPILEFLAGS) \
        $(GLOBAL_GZ_OPTFLAGS) \
        $(GLOBAL_GZ_CFLAGS) \
        $(PRIVATE_CFLAGS) \
        -MD -MF $(patsubst %.o,%.d,$@) -o $@ $<
endef

define gz-transform-o-to-executable
$(hide) $(GZ_LD_arm) --gc-sections \
        -z max-page-size=0x1000 \
        -T $(PREBUILTS_LDSCRIPTS)/arch/criterion.ld \
        -T $(PREBUILTS_LDSCRIPTS)/user_task.ld \
        $(PRIVATE_TARGET_GLOBAL_LD_DIRS) \
        $(PREBUILTS_LIB_PATH)/arm/crtbegin.o \
        $(PRIVATE_ALL_OBJECTS) \
        --whole-archive \
        $(PRIVATE_ALL_WHOLE_STATIC_LIBRARIES) \
        --no-whole-archive \
        --start-group \
        $(PRIVATE_ALL_STATIC_LIBRARIES) \
        --end-group \
        $(LIBGCC_arm) \
        $(PREBUILTS_LIB_PATH)/arm/crtend.o \
        -o $@
endef

