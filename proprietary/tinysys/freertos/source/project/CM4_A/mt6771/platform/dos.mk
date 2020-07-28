###################################################################
# This file contains all the supported modules for this platform,
# along with platform-default DO (Dynamic Object) configs, if any.
#
# To overwrite the DO configurations in projects, config them in
# project-specific ProjectConfig.mk
#
# DO config options are always named with leading CFG_DO_ string.
###################################################################
#CFG_DO_MTK_AUDIO_TUNNELING = yes
#CFG_DO_MTK_AURISYS_PHONE_CALL = yes

###################################################################
# A Dynamic Object entry is composed with the following sections:
#
# 1. include $(CLEAR_DO_VARS)
#    [REQUIRED] Clear former DO definitions
# 2. DO_CONFIG
#    [REQUIRED] Variable that indicates what DO config option this
#    DO relates to. Build system will register the DO only when DO
#    config option is yes.
# 3. DO_NAME
#    [REQUIRED] Variable that names the DO. It is highly recommended
#    to name your DO by removing leading CFG_ string of DO config
#    option.
#
#    For example, a DO config option CFG_DO_FOO shall be named
#    with DO_FOO.
#
#    You can also use the function get-do-name to get the name from
#    DO config option:
#      DO_NAME := $(call get-do-name,$(DO_CONFIG))
# 4. DO_ID
#    [OPTIONAL] identifier of the DO. Combining with DO_NAME this C
#    macro will be recorded in generated header file:
#      #define $(DO_ID) "$(DO_NAME)"
#    If DO_ID is not defined , it will be automatically generated
#    in the format: $(DO_NAME)_ID
# 5. Resource indication directives of this DO
#    * DO_INCLUDES : DO specific header files
#    * DO_C_FILES  : DO specific C files
#    * DO_S_FILES  : DO specific S files
#    * DO_LDFLAGS  : DO specific linker flags
#    * DO_CFLAGS   : DO specific C flags
#    * DO_FEATURES : DO specific feature sets, described with
#                    FEATURE_NAME directives
# 6. include $(REGISTER_DO)
#    [REQUIRED] Register the DO entry using the directives above
###################################################################

include $(CLEAR_DO_VARS)
DO_CONFIG := CFG_DO_SAMPLE1
DO_NAME   := SAMPLE1
DO_ID     := $(DO_NAME)_ID
DO_FEATURES := FEATURE01 FEATURE02
include $(REGISTER_DO)

include $(CLEAR_DO_VARS)
DO_CONFIG := CFG_DO_SAMPLE2
DO_NAME   := SAMPLE2
DO_ID     := $(DO_NAME)_ID
DO_FEATURES := FEATURE02 FEATURE03
include $(REGISTER_DO)
