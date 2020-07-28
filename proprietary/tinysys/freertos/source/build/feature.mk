DEPS += $(lastword $(MAKEFILE_LIST))

MY_FEATURE_BUILT_DIR           := $($(PROCESSOR).DO_BUILT_DIR)/$(do)/$(feature)

$(PROCESSOR).$(feature).C_FILES := \
  $(patsubst ./%,%,$($(PROCESSOR).$(feature).C_FILES))
$(PROCESSOR).$(feature).S_FILES := \
  $(patsubst ./%,%,$($(PROCESSOR).$(feature).S_FILES))
$(PROCESSOR).$(feature).C_OBJS := \
  $(sort $($(PROCESSOR).$(feature).C_FILES:%.c=$(MY_FEATURE_BUILT_DIR)/%.o))
$(PROCESSOR).$(feature).S_OBJS := \
  $(sort $($(PROCESSOR).$(feature).S_FILES:%.S=$(MY_FEATURE_BUILT_DIR)/%.o))
$(PROCESSOR).$(feature).OBJS   := \
  $($(PROCESSOR).$(feature).C_OBJS) $($(PROCESSOR).$(feature).S_OBJS)

$($(PROCESSOR).$(feature).OBJS): $($(PROCESSOR).TINYSYS_CONFIG_H)

$(PROCESSOR).$(do).ALL_FEATURE_OBJS := \
  $(sort \
    $($(PROCESSOR).$(do).ALL_FEATURE_OBJS) \
    $($(PROCESSOR).$(feature).OBJS) \
  )

$($(PROCESSOR).$(feature).OBJS): PRIVATE_CC := $(CC)
$($(PROCESSOR).$(feature).OBJS): PRIVATE_CFLAGS := \
  $(FEATURE_COMMON_CFLAGS) $($(PROCESSOR).$(feature).CFLAGS) \
  -include $($(PROCESSOR).TINYSYS_CONFIG_H)
$($(PROCESSOR).$(feature).OBJS): PRIVATE_INCLUDES := $(INCLUDES)
$($(PROCESSOR).$(feature).OBJS): PRIVATE_LIBFLAGS := \
  $($(PROCESSOR).$(feature).LIBFLAGS)
$($(PROCESSOR).$(feature).OBJS): $(MY_FEATURE_BUILT_DIR)/%.o: %.c
	$(compile-c-or-s-to-o)

$($(PROCESSOR).$(feature).OBJS): $(DEPS)
-include $(patsubst %.o,%.d,$($(PROCESSOR).$(feature).OBJS))
