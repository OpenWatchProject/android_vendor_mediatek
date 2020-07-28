#######################################
#include $(TRUSTZONE_CUSTOM_BUILD_PATH)/base_rules.mk
include $(BUILD_SYSTEM)/base_rules.mk
#######################################

##################################################
# Compute the dependency of the shared libraries
##################################################
# On the target, we compile with -nostdlib, so we must add in the
# default system shared libraries, unless they have requested not
# to by supplying a LOCAL_SYSTEM_SHARED_LIBRARIES value.  One would
# supply that, for example, when building libc itself.
ifeq ($(LOCAL_SYSTEM_SHARED_LIBRARIES),none)
      my_system_shared_libraries :=libm libdl
  else
      my_system_shared_libraries := $(LOCAL_SYSTEM_SHARED_LIBRARIES)
      my_system_shared_libraries := $(patsubst libc,libc libdl,$(my_system_shared_libraries))
endif


my_soong_problems :=

# The following LOCAL_ variables will be modified in this file.
# Because the same LOCAL_ variables may be used to define modules for both 1st arch and 2nd arch,
# we can't modify them in place.
my_src_files := $(LOCAL_SRC_FILES)
my_src_files_exclude := $(LOCAL_SRC_FILES_EXCLUDE)
my_static_libraries := $(LOCAL_STATIC_LIBRARIES)
my_whole_static_libraries := $(LOCAL_WHOLE_STATIC_LIBRARIES)
my_shared_libraries := $(filter-out $(my_system_shared_libraries),$(LOCAL_SHARED_LIBRARIES))
my_header_libraries := $(LOCAL_HEADER_LIBRARIES)
my_cflags := $(LOCAL_CFLAGS)
my_conlyflags := $(LOCAL_CONLYFLAGS)
my_cppflags := $(LOCAL_CPPFLAGS)
my_cflags_no_override := 
my_cppflags_no_override := 
my_ldflags := $(LOCAL_LDFLAGS)
my_ldlibs := $(LOCAL_LDLIBS)
my_asflags := $(LOCAL_ASFLAGS)
my_cc := $(LOCAL_CC)
my_cc_wrapper := $(CC_WRAPPER)
my_cxx := $(LOCAL_CXX)
my_cxx_ldlibs :=
my_cxx_wrapper := $(CXX_WRAPPER)
my_c_includes := $(LOCAL_C_INCLUDES)
my_generated_sources := $(LOCAL_GENERATED_SOURCES)
my_additional_dependencies := $(LOCAL_ADDITIONAL_DEPENDENCIES)
my_export_c_include_dirs := $(LOCAL_EXPORT_C_INCLUDE_DIRS)
my_export_c_include_deps := $(LOCAL_EXPORT_C_INCLUDE_DEPS)
my_arflags :=


my_allow_undefined_symbols := $(strip $(LOCAL_ALLOW_UNDEFINED_SYMBOLS))

my_ndk_sysroot :=
my_ndk_sysroot_include :=
my_ndk_sysroot_lib :=

ifndef LOCAL_IS_HOST_MODULE
my_allowed_ldlibs :=

# Sort ldlibs and ldflags between -l and other linker flags
# We'll do this again later, since there are still changes happening, but that's fine.
my_ldlib_flags := $(my_ldflags) $(my_ldlibs)
my_ldlibs := $(filter -l%,$(my_ldlib_flags))
my_ldflags := $(filter-out -l%,$(my_ldlib_flags))
my_ldlib_flags :=

# Move other ldlibs back to shared libraries
my_shared_libraries += $(patsubst -l%,lib%,$(filter-out $(my_allowed_ldlibs),$(my_ldlibs)))
my_ldlibs := $(filter $(my_allowed_ldlibs),$(my_ldlibs))
endif

# MinGW spits out warnings about -fPIC even for -fpie?!) being ignored because
# all code is position independent, and then those warnings get promoted to
# errors.
ifneq ($(LOCAL_NO_PIC),true)
ifneq ($($(my_prefix)OS),windows)
ifneq ($(filter EXECUTABLES NATIVE_TESTS,$(LOCAL_MODULE_CLASS)),)
my_cflags += -fPIE
else
my_cflags += -fPIC
endif
endif
endif

my_src_files += $(LOCAL_SRC_FILES_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)) $(LOCAL_SRC_FILES_$(my_32_64_bit_suffix))
my_src_files_exclude += $(LOCAL_SRC_FILES_EXCLUDE_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)) $(LOCAL_SRC_FILES_EXCLUDE_$(my_32_64_bit_suffix))
my_shared_libraries += $(LOCAL_SHARED_LIBRARIES_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)) $(LOCAL_SHARED_LIBRARIES_$(my_32_64_bit_suffix))
my_cflags += $(LOCAL_CFLAGS_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)) $(LOCAL_CFLAGS_$(my_32_64_bit_suffix))
my_cppflags += $(LOCAL_CPPFLAGS_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)) $(LOCAL_CPPFLAGS_$(my_32_64_bit_suffix))
my_ldflags += $(LOCAL_LDFLAGS_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)) $(LOCAL_LDFLAGS_$(my_32_64_bit_suffix))
my_asflags += $(LOCAL_ASFLAGS_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)) $(LOCAL_ASFLAGS_$(my_32_64_bit_suffix))
my_c_includes += $(LOCAL_C_INCLUDES_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)) $(LOCAL_C_INCLUDES_$(my_32_64_bit_suffix))
my_generated_sources += $(LOCAL_GENERATED_SOURCES_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)) $(LOCAL_GENERATED_SOURCES_$(my_32_64_bit_suffix))

my_missing_exclude_files := $(filter-out $(my_src_files),$(my_src_files_exclude))
ifneq ($(my_missing_exclude_files),)
$(warning Files are listed in LOCAL_SRC_FILES_EXCLUDE but not LOCAL_SRC_FILES)
$(error $(my_missing_exclude_files))
endif
my_src_files := $(filter-out $(my_src_files_exclude),$(my_src_files))

# Strip '/' from the beginning of each src file. This helps the ../ detection in case
# the source file is in the form of /../file
my_src_files := $(patsubst /%,%,$(my_src_files))

my_clang := $(strip $(LOCAL_CLANG))
ifdef LOCAL_CLANG_$(my_32_64_bit_suffix)
my_clang := $(strip $(LOCAL_CLANG_$(my_32_64_bit_suffix)))
endif
ifdef LOCAL_CLANG_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)
my_clang := $(strip $(LOCAL_CLANG_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)))
endif

# if custom toolchain is in use, default is not to use clang, if not explicitly required
ifneq ($(my_cc)$(my_cxx),)
    ifeq ($(my_clang),)
        my_clang := false
    endif
endif


ifeq ($(LOCAL_C_STD),)
    my_c_std_version := $(DEFAULT_C_STD_VERSION)
else ifeq ($(LOCAL_C_STD),experimental)
    my_c_std_version := $(EXPERIMENTAL_C_STD_VERSION)
else
    my_c_std_version := $(LOCAL_C_STD)
endif

ifeq ($(LOCAL_CPP_STD),)
    my_cpp_std_version := $(DEFAULT_CPP_STD_VERSION)
else ifeq ($(LOCAL_CPP_STD),experimental)
    my_cpp_std_version := $(EXPERIMENTAL_CPP_STD_VERSION)
else
    my_cpp_std_version := $(LOCAL_CPP_STD)
endif



my_c_std_conlyflags :=
my_cpp_std_cppflags :=
ifneq (,$(my_c_std_version))
    my_c_std_conlyflags := -std=$(my_c_std_version)
endif

ifneq (,$(my_cpp_std_version))
   my_cpp_std_cppflags := -std=$(my_cpp_std_version)
endif

# arch-specific static libraries go first so that generic ones can depend on them
my_static_libraries := $(LOCAL_STATIC_LIBRARIES_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)) $(LOCAL_STATIC_LIBRARIES_$(my_32_64_bit_suffix)) $(my_static_libraries)
my_whole_static_libraries := $(LOCAL_WHOLE_STATIC_LIBRARIES_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)) $(LOCAL_WHOLE_STATIC_LIBRARIES_$(my_32_64_bit_suffix)) $(my_whole_static_libraries)
my_header_libraries := $(LOCAL_HEADER_LIBRARIES_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)) $(LOCAL_HEADER_LIBRARIES_$(my_32_64_bit_suffix)) $(my_header_libraries)

# soong defined modules already have done through this
ifneq ($(LOCAL_MODULE_MAKEFILE),$(SOONG_ANDROID_MK))
include $(BUILD_SYSTEM)/cxx_stl_setup.mk
endif


ifneq ($(strip $(CUSTOM_$(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)LINKER)),)
  my_linker := $(CUSTOM_$(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)LINKER)
else
  my_linker := $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)LINKER)
endif

# Modules from soong do not need this since the dependencies are already handled there.
ifneq ($(LOCAL_MODULE_MAKEFILE),$(SOONG_ANDROID_MK))
#include $(TRUSTZONE_CUSTOM_BUILD_PATH)/build_config_sanitizers.mk
include $(BUILD_SYSTEM)/config_sanitizers.mk
ifneq ($(LOCAL_NO_LIBCOMPILER_RT),true)
# Add in libcompiler_rt for all regular device builds
ifeq (,$(WITHOUT_LIBCOMPILER_RT))
  my_static_libraries += $(COMPILER_RT_CONFIG_EXTRA_STATIC_LIBRARIES)
endif
endif

endif # this module is not from soong

ifneq ($(filter ../%,$(my_src_files)),)
my_soong_problems += dotdot_srcs
endif
ifneq ($(foreach i,$(my_c_includes),$(filter %/..,$(i))$(findstring /../,$(i))),)
my_soong_problems += dotdot_incs
endif
ifneq ($(filter %.arm,$(my_src_files)),)
my_soong_problems += srcs_dotarm
endif

####################################################
## Add FDO flags if FDO is turned on and supported
## Please note that we will do option filtering during FDO build.
## i.e. Os->O2, remove -fno-early-inline and -finline-limit.
##################################################################
my_fdo_build :=

my_asflags += -D__ASSEMBLY__

###########################################################
## Define PRIVATE_ variables from global vars
###########################################################
ifdef LOCAL_USE_VNDK
my_target_global_c_includes := \
    $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)PROJECT_INCLUDES)
my_target_global_c_system_includes := \
    $(TARGET_OUT_HEADERS) \
    $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)PROJECT_SYSTEM_INCLUDES)
else ifdef LOCAL_SDK_VERSION
my_target_global_c_includes :=
my_target_global_c_system_includes := $(my_ndk_stl_include_path) $(my_ndk_sysroot_include)
else ifdef BOARD_VNDK_VERSION
my_target_global_c_includes := $(SRC_HEADERS) \
    $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)PROJECT_INCLUDES) \
    $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)C_INCLUDES)
my_target_global_c_system_includes := $(SRC_SYSTEM_HEADERS) \
    $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)PROJECT_SYSTEM_INCLUDES) \
    $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)C_SYSTEM_INCLUDES)
else

my_target_global_c_includes := $(SRC_HEADERS) \
    $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)PROJECT_INCLUDES) \
    $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)C_INCLUDES)
my_target_global_c_system_includes := $(SRC_SYSTEM_HEADERS) $(TARGET_OUT_HEADERS) \
    $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)PROJECT_SYSTEM_INCLUDES) \
    $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)C_SYSTEM_INCLUDES)
endif



###########################################################
## Define PRIVATE_ variables used by multiple module types
###########################################################
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_NO_DEFAULT_COMPILER_FLAGS := \
    $(strip $(LOCAL_NO_DEFAULT_COMPILER_FLAGS))

ifeq ($(strip $(WITH_STATIC_ANALYZER)),)
  LOCAL_NO_STATIC_ANALYZER := true
endif

# Clang does not recognize all gcc flags.
# Use static analyzer only if clang is used.
ifneq ($(my_clang),true)
  LOCAL_NO_STATIC_ANALYZER := true
endif

ifneq ($(strip $(LOCAL_IS_HOST_MODULE)),)
  my_syntax_arch := host
else
  my_syntax_arch := $($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)
endif

ifeq ($(strip $(my_cc)),)
  ifeq ($(my_clang),true)
    my_cc := $(CLANG)
  else
    my_cc := $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)CC)
  endif
  my_cc := $(my_cc_wrapper) $(my_cc)
endif

SYNTAX_TOOLS_PREFIX := \
    $(LLVM_PREBUILTS_BASE)/$(BUILD_OS)-x86/$(LLVM_PREBUILTS_VERSION)/libexec

ifneq ($(LOCAL_NO_STATIC_ANALYZER),true)
  my_cc := CCC_CC=$(CLANG) CLANG=$(CLANG) \
           $(SYNTAX_TOOLS_PREFIX)/ccc-analyzer
endif

$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_CC := $(my_cc)

ifeq ($(strip $(my_cxx)),)
  ifeq ($(my_clang),true)
    my_cxx := $(CLANG_CXX)
  else
    my_cxx := $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)CXX)
  endif
  my_cxx := $(my_cxx_wrapper) $(my_cxx)
endif

ifneq ($(LOCAL_NO_STATIC_ANALYZER),true)
  my_cxx := CCC_CXX=$(CLANG_CXX) CLANG_CXX=$(CLANG_CXX) \
            $(SYNTAX_TOOLS_PREFIX)/c++-analyzer
endif

$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_LINKER := $(my_linker)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_CXX := $(my_cxx)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_CLANG := $(my_clang)

# TODO: support a mix of standard extensions so that this isn't necessary
LOCAL_CPP_EXTENSION := $(strip $(LOCAL_CPP_EXTENSION))
ifeq ($(LOCAL_CPP_EXTENSION),)
  LOCAL_CPP_EXTENSION := .cpp
endif

# Certain modules like libdl have to have symbols resolved at runtime and blow
# up if --no-undefined is passed to the linker.
ifeq ($(strip $(LOCAL_NO_DEFAULT_COMPILER_FLAGS)),)
  ifeq ($(my_allow_undefined_symbols),)
    ifneq ($(HOST_OS),darwin)
      my_ldflags += -Wl,--no-undefined
    endif
  else
    ifdef LOCAL_IS_HOST_MODULE
      ifeq ($(HOST_OS),darwin)
        # darwin defaults to treating undefined symbols as errors
        my_ldflags += -Wl,-undefined,dynamic_lookup
      endif
    endif
  endif
endif

ifeq (true,$(LOCAL_GROUP_STATIC_LIBRARIES))
$(LOCAL_BUILT_MODULE): PRIVATE_GROUP_STATIC_LIBRARIES := true
else
$(LOCAL_BUILT_MODULE): PRIVATE_GROUP_STATIC_LIBRARIES :=
endif

###########################################################
## Define arm-vs-thumb-mode flags.
###########################################################
LOCAL_ARM_MODE := $(strip $(LOCAL_ARM_MODE))
ifeq ($($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH),arm)
arm_objects_mode := $(if $(LOCAL_ARM_MODE),$(LOCAL_ARM_MODE),arm)
normal_objects_mode := $(if $(LOCAL_ARM_MODE),$(LOCAL_ARM_MODE),thumb)

arm_objects_cflags := $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)$(arm_objects_mode)_CFLAGS)
normal_objects_cflags := $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)$(normal_objects_mode)_CFLAGS)
#ifeq ($(my_clang),true)
#arm_objects_cflags := $(call convert-to-clang-flags,$(arm_objects_cflags))
#normal_objects_cflags := $(call convert-to-clang-flags,$(normal_objects_cflags))
#endif

else
arm_objects_mode :=
normal_objects_mode :=
arm_objects_cflags :=
normal_objects_cflags :=
endif

###########################################################
## Define per-module debugging flags.  Users can turn on
## debugging for a particular module by setting DEBUG_MODULE_ModuleName
## to a non-empty value in their environment or buildspec.mk,
## and setting HOST_/TARGET_CUSTOM_DEBUG_CFLAGS to the
## debug flags that they want to use.
ifdef DEBUG_MODULE_$(strip $(LOCAL_MODULE))
  debug_cflags := $($(my_prefix)CUSTOM_DEBUG_CFLAGS)
else
  debug_cflags :=
endif

####################################################
## Keep track of src -> obj mapping
####################################################

my_tracked_gen_files :=
my_tracked_src_files :=

###########################################################
## Stuff source generated from one-off tools
###########################################################
$(my_generated_sources): PRIVATE_MODULE := $(my_register_name)

my_gen_sources_copy := $(patsubst $(generated_sources_dir)/%,$(intermediates)/%,$(filter $(generated_sources_dir)/%,$(my_generated_sources)))

$(my_gen_sources_copy): $(intermediates)/% : $(generated_sources_dir)/%
	@echo "Copy: $@"
	$(copy-file-to-target)

my_generated_sources := $(patsubst $(generated_sources_dir)/%,$(intermediates)/%,$(my_generated_sources))

# Generated sources that will actually produce object files.
# Other files (like headers) are allowed in LOCAL_GENERATED_SOURCES,
# since other compiled sources may depend on them, and we set up
# the dependencies.
my_gen_src_files := $(filter %.c %$(LOCAL_CPP_EXTENSION) %.S %.s,$(my_generated_sources))

ALL_GENERATED_SOURCES += $(my_generated_sources)

###########################################################
## C++: Compile .cpp files to .o.
###########################################################

# we also do this on host modules, even though
# it's not really arm, because there are files that are shared.
cpp_arm_sources := $(patsubst %$(LOCAL_CPP_EXTENSION).arm,%$(LOCAL_CPP_EXTENSION),$(filter %$(LOCAL_CPP_EXTENSION).arm,$(my_src_files)))
dotdot_arm_sources := $(filter ../%,$(cpp_arm_sources))
cpp_arm_sources := $(filter-out ../%,$(cpp_arm_sources))
cpp_arm_objects := $(addprefix $(intermediates)/,$(cpp_arm_sources:$(LOCAL_CPP_EXTENSION)=.o))
$(call track-src-file-obj,$(patsubst %,%.arm,$(cpp_arm_sources)),$(cpp_arm_objects))

# For source files starting with ../, we remove all the ../ in the object file path,
# to avoid object file escaping the intermediate directory.
dotdot_arm_objects :=
$(foreach s,$(dotdot_arm_sources),\
  $(eval $(call compile-dotdot-cpp-file,$(s),\
  $(my_additional_dependencies),\
  dotdot_arm_objects)))
$(call track-src-file-obj,$(patsubst %,%.arm,$(dotdot_arm_sources)),$(dotdot_arm_objects))

dotdot_sources := $(filter ../%$(LOCAL_CPP_EXTENSION),$(my_src_files))
dotdot_objects :=
$(foreach s,$(dotdot_sources),\
  $(eval $(call compile-dotdot-cpp-file,$(s),\
    $(my_additional_dependencies),\
    dotdot_objects)))
$(call track-src-file-obj,$(dotdot_sources),$(dotdot_objects))

cpp_normal_sources := $(filter-out ../%,$(filter %$(LOCAL_CPP_EXTENSION),$(my_src_files)))
cpp_normal_objects := $(addprefix $(intermediates)/,$(cpp_normal_sources:$(LOCAL_CPP_EXTENSION)=.o))
$(call track-src-file-obj,$(cpp_normal_sources),$(cpp_normal_objects))

$(dotdot_arm_objects) $(cpp_arm_objects): PRIVATE_ARM_MODE := $(arm_objects_mode)
$(dotdot_arm_objects) $(cpp_arm_objects): PRIVATE_ARM_CFLAGS := $(arm_objects_cflags)
$(dotdot_objects) $(cpp_normal_objects): PRIVATE_ARM_MODE := $(normal_objects_mode)
$(dotdot_objects) $(cpp_normal_objects): PRIVATE_ARM_CFLAGS := $(normal_objects_cflags)

cpp_objects        := $(cpp_arm_objects) $(cpp_normal_objects)

ifneq ($(strip $(cpp_objects)),)
$(cpp_objects): $(intermediates)/%.o: \
    $(TOPDIR)$(LOCAL_PATH)/%$(LOCAL_CPP_EXTENSION) \
    $(my_additional_dependencies)
	$(transform-$(PRIVATE_HOST)cpp-to-o)
$(call include-depfiles-for-objs, $(cpp_objects))
endif

cpp_objects += $(dotdot_arm_objects) $(dotdot_objects)

###########################################################
## C++: Compile generated .cpp files to .o.
###########################################################

gen_cpp_sources := $(filter %$(LOCAL_CPP_EXTENSION),$(my_generated_sources))
gen_cpp_objects := $(gen_cpp_sources:%$(LOCAL_CPP_EXTENSION)=%.o)
$(call track-gen-file-obj,$(gen_cpp_sources),$(gen_cpp_objects))

ifneq ($(strip $(gen_cpp_objects)),)
# Compile all generated files as thumb.
# TODO: support compiling certain generated files as arm.
$(gen_cpp_objects): PRIVATE_ARM_MODE := $(normal_objects_mode)
$(gen_cpp_objects): PRIVATE_ARM_CFLAGS := $(normal_objects_cflags)
$(gen_cpp_objects): $(intermediates)/%.o: \
    $(intermediates)/%$(LOCAL_CPP_EXTENSION) \
    $(my_additional_dependencies)
	$(transform-$(PRIVATE_HOST)cpp-to-o)
$(call include-depfiles-for-objs, $(gen_cpp_objects))
endif

###########################################################
## o: Include generated .o files in output.
###########################################################

gen_o_objects := $(filter %.o,$(my_generated_sources))

###########################################################
## C: Compile .c files to .o.
###########################################################

c_arm_sources := $(patsubst %.c.arm,%.c,$(filter %.c.arm,$(my_src_files)))
dotdot_arm_sources := $(filter ../%,$(c_arm_sources))
c_arm_sources := $(filter-out ../%,$(c_arm_sources))
c_arm_objects := $(addprefix $(intermediates)/,$(c_arm_sources:.c=.o))
$(call track-src-file-obj,$(patsubst %,%.arm,$(c_arm_sources)),$(c_arm_objects))

# For source files starting with ../, we remove all the ../ in the object file path,
# to avoid object file escaping the intermediate directory.
dotdot_arm_objects :=
$(foreach s,$(dotdot_arm_sources),\
  $(eval $(call compile-dotdot-c-file,$(s),\
    $(my_additional_dependencies),\
    dotdot_arm_objects)))
$(call track-src-file-obj,$(patsubst %,%.arm,$(dotdot_arm_sources)),$(dotdot_arm_objects))

dotdot_sources := $(filter ../%.c, $(my_src_files))
dotdot_objects :=
$(foreach s, $(dotdot_sources),\
  $(eval $(call compile-dotdot-c-file,$(s),\
    $(my_additional_dependencies),\
    dotdot_objects)))
$(call track-src-file-obj,$(dotdot_sources),$(dotdot_objects))

c_normal_sources := $(filter-out ../%,$(filter %.c,$(my_src_files)))
c_normal_objects := $(addprefix $(intermediates)/,$(c_normal_sources:.c=.o))
$(call track-src-file-obj,$(c_normal_sources),$(c_normal_objects))

$(dotdot_arm_objects) $(c_arm_objects): PRIVATE_ARM_MODE := $(arm_objects_mode)
$(dotdot_arm_objects) $(c_arm_objects): PRIVATE_ARM_CFLAGS := $(arm_objects_cflags)
$(dotdot_objects) $(c_normal_objects): PRIVATE_ARM_MODE := $(normal_objects_mode)
$(dotdot_objects) $(c_normal_objects): PRIVATE_ARM_CFLAGS := $(normal_objects_cflags)

c_objects        := $(c_arm_objects) $(c_normal_objects)

ifneq ($(strip $(c_objects)),)
$(c_objects): $(intermediates)/%.o: $(TOPDIR)$(LOCAL_PATH)/%.c \
    $(my_additional_dependencies)
	$(transform-$(PRIVATE_HOST)c-to-o)
$(call include-depfiles-for-objs, $(c_objects))
endif

c_objects += $(dotdot_arm_objects) $(dotdot_objects)

###########################################################
## C: Compile generated .c files to .o.
###########################################################

gen_c_sources := $(filter %.c,$(my_generated_sources))
gen_c_objects := $(gen_c_sources:%.c=%.o)
$(call track-gen-file-obj,$(gen_c_sources),$(gen_c_objects))

ifneq ($(strip $(gen_c_objects)),)
# Compile all generated files as thumb.
# TODO: support compiling certain generated files as arm.
$(gen_c_objects): PRIVATE_ARM_MODE := $(normal_objects_mode)
$(gen_c_objects): PRIVATE_ARM_CFLAGS := $(normal_objects_cflags)
$(gen_c_objects): $(intermediates)/%.o: $(intermediates)/%.c \
    $(my_additional_dependencies)
	$(transform-$(PRIVATE_HOST)c-to-o)
$(call include-depfiles-for-objs, $(gen_c_objects))
endif

###################################################################
## When compiling a CFI enabled target, use the .cfi variant of any
## static dependencies (where they exist).
##################################################################
define use_soong_cfi_static_libraries
  $(foreach l,$(1),$(if $(filter $(l),$(SOONG_CFI_STATIC_LIBRARIES)),\
      $(l).cfi,$(l)))
endef

ifneq ($(filter cfi,$(my_sanitize)),)
  my_whole_static_libraries := $(call use_soong_cfi_static_libraries,\
    $(my_whole_static_libraries))
  my_static_libraries := $(call use_soong_cfi_static_libraries,\
    $(my_static_libraries))
endif

###########################################################
## When compiling against the VNDK, use LL-NDK libraries
###########################################################
ifneq ($(LOCAL_USE_VNDK),)
  ifneq ($(LOCAL_MODULE_MAKEFILE),$(SOONG_ANDROID_MK))
    # We don't do this renaming for soong-defined modules since they already have correct
    # names (with .vendor suffix when necessary) in their LOCAL_*_LIBRARIES.
    my_whole_static_libraries := $(foreach l,$(my_whole_static_libraries),\
      $(if $(SPLIT_VENDOR.STATIC_LIBRARIES.$(l)),$(l).vendor,$(l)))
    my_static_libraries := $(foreach l,$(my_static_libraries),\
      $(if $(SPLIT_VENDOR.STATIC_LIBRARIES.$(l)),$(l).vendor,$(l)))
    my_shared_libraries := $(foreach l,$(my_shared_libraries),\
      $(if $(SPLIT_VENDOR.SHARED_LIBRARIES.$(l)),$(l).vendor,$(l)))
    my_system_shared_libraries := $(foreach l,$(my_system_shared_libraries),\
      $(if $(SPLIT_VENDOR.SHARED_LIBRARIES.$(l)),$(l).vendor,$(l)))
    my_header_libraries := $(foreach l,$(my_header_libraries),\
      $(if $(SPLIT_VENDOR.HEADER_LIBRARIES.$(l)),$(l).vendor,$(l)))
  endif
endif

##########################################################
## Set up installed module dependency
## We cannot compute the full path of the LOCAL_SHARED_LIBRARIES for
## they may cusomize their install path with LOCAL_MODULE_PATH
##########################################################
# Get the list of INSTALLED libraries as module names.
ifneq ($(LOCAL_SDK_VERSION),)
  installed_shared_library_module_names := \
      $(my_shared_libraries)
else
  installed_shared_library_module_names := \
      $(my_shared_libraries) $(my_system_shared_libraries)
endif

# The real dependency will be added after all Android.mks are loaded and the install paths
# of the shared libraries are determined.
ifdef LOCAL_INSTALLED_MODULE
ifdef installed_shared_library_module_names
$(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)DEPENDENCIES_ON_SHARED_LIBRARIES += \
    $(my_register_name):$(LOCAL_INSTALLED_MODULE):$(subst $(space),$(comma),$(installed_shared_library_module_names))
endif
endif

####################################################
## Import includes
####################################################
import_includes := $(intermediates)/import_includes
import_includes_deps := $(strip \
    $(if $(LOCAL_USE_VNDK),\
      $(call intermediates-dir-for,HEADER_LIBRARIES,device_kernel_headers,$(my_kind),,$(LOCAL_2ND_ARCH_VAR_PREFIX),$(my_host_cross))/export_includes) \
    $(foreach l, $(installed_shared_library_module_names), \
      $(call intermediates-dir-for,SHARED_LIBRARIES,$(l),$(my_kind),,$(LOCAL_2ND_ARCH_VAR_PREFIX),$(my_host_cross))/export_includes) \
    $(foreach l, $(my_static_libraries) $(my_whole_static_libraries), \
      $(call intermediates-dir-for,STATIC_LIBRARIES,$(l),$(my_kind),,$(LOCAL_2ND_ARCH_VAR_PREFIX),$(my_host_cross))/export_includes) \
    $(foreach l, $(my_header_libraries), \
      $(call intermediates-dir-for,HEADER_LIBRARIES,$(l),$(my_kind),,$(LOCAL_2ND_ARCH_VAR_PREFIX),$(my_host_cross))/export_includes))
$(import_includes): PRIVATE_IMPORT_EXPORT_INCLUDES := $(import_includes_deps)
$(import_includes) : $(import_includes_deps)
	@echo Import includes file: $@
	$(hide) mkdir -p $(dir $@) && rm -f $@
ifdef import_includes_deps
	$(hide) for f in $(PRIVATE_IMPORT_EXPORT_INCLUDES); do \
	  cat $$f >> $@; \
	done
else
	$(hide) touch $@
endif

####################################################
## Verify that NDK-built libraries only link against
## other NDK-built libraries
####################################################

#include $(BUILD_SYSTEM)/allowed_ndk_types.mk

ifdef LOCAL_SDK_VERSION
my_link_type := native:ndk:$(my_ndk_stl_family):$(my_ndk_stl_link_type)
my_warn_types := $(my_warn_ndk_types)
my_allowed_types := $(my_allowed_ndk_types)
else ifdef LOCAL_USE_VNDK
    _name := $(patsubst %.vendor,%,$(LOCAL_MODULE))
    ifneq ($(filter $(_name),$(VNDK_CORE_LIBRARIES) $(VNDK_SAMEPROCESS_LIBRARIES) $(LLNDK_LIBRARIES)),)
        ifeq ($(filter $(_name),$(VNDK_PRIVATE_LIBRARIES)),)
            my_link_type := native:vndk
        else
            my_link_type := native:vndk_private
        endif
        my_warn_types :=
        my_allowed_types := native:vndk native:vndk_private
    else
        # Modules installed to /vendor cannot directly depend on modules marked
        # with vendor_available: false
        my_link_type := native:vendor
        my_warn_types :=
        my_allowed_types := native:vendor native:vndk
    endif
else
my_link_type := native:platform
my_warn_types := $(my_warn_ndk_types)
my_allowed_types := $(my_allowed_ndk_types) native:platform
endif

my_link_deps := $(addprefix STATIC_LIBRARIES:,$(my_whole_static_libraries) $(my_static_libraries))
ifneq ($(filter-out STATIC_LIBRARIES HEADER_LIBRARIES,$(LOCAL_MODULE_CLASS)),)
my_link_deps += $(addprefix SHARED_LIBRARIES:,$(my_shared_libraries))
endif

my_2nd_arch_prefix := $(LOCAL_2ND_ARCH_VAR_PREFIX)
my_common :=
include $(BUILD_SYSTEM)/link_type.mk

###########################################################
## Common object handling.
###########################################################

my_unused_src_files := $(filter-out $(logtags_sources) $(my_tracked_src_files),$(my_src_files) $(my_gen_src_files))
ifneq ($(my_unused_src_files),)
  $(error $(LOCAL_MODULE_MAKEFILE): $(LOCAL_MODULE): Unused source files: $(my_unused_src_files))
endif

# some rules depend on asm_objects being first.  If your code depends on
# being first, it's reasonable to require it to be assembly
normal_objects := \
    $(asm_objects) \
    $(cpp_objects) \
    $(gen_cpp_objects) \
    $(gen_asm_objects) \
    $(c_objects) \
    $(gen_c_objects) \
    $(objc_objects) \
    $(objcpp_objects)

new_order_normal_objects := $(foreach f,$(my_src_files),$(my_src_file_obj_$(f)))
new_order_normal_objects += $(foreach f,$(my_gen_src_files),$(my_src_file_obj_$(f)))

ifneq ($(sort $(normal_objects)),$(sort $(new_order_normal_objects)))
$(warning $(LOCAL_MODULE_MAKEFILE) Internal build system warning: New object list does not match old)
$(info Only in old: $(filter-out $(new_order_normal_objects),$(sort $(normal_objects))))
$(info Only in new: $(filter-out $(normal_objects),$(sort $(new_order_normal_objects))))
endif

ifeq ($(BINARY_OBJECTS_ORDER),soong)
normal_objects := $(new_order_normal_objects)
endif

normal_objects += $(addprefix $(TOPDIR)$(LOCAL_PATH)/,$(LOCAL_PREBUILT_OBJ_FILES))

all_objects := $(normal_objects) $(gen_o_objects)

# Cleanup file tracking
$(foreach f,$(my_tracked_gen_files),$(eval my_src_file_gen_$(s):=))
my_tracked_gen_files :=
$(foreach f,$(my_tracked_src_files),$(eval my_src_file_obj_$(s):=))
my_tracked_src_files :=

my_c_includes += $(TOPDIR)$(LOCAL_PATH) $(intermediates) $(generated_sources_dir)

# The platform JNI header is for platform modules only.
ifeq ($(LOCAL_SDK_VERSION)$(LOCAL_USE_VNDK),)
  my_c_includes += $(JNI_H_INCLUDE)
endif

my_outside_includes := $(filter-out $(OUT_DIR)/%,$(filter /%,$(my_c_includes)))
ifneq ($(my_outside_includes),)
$(error $(LOCAL_MODULE_MAKEFILE): $(LOCAL_MODULE): C_INCLUDES must be under the source or output directories: $(my_outside_includes))
endif

# all_objects includes gen_o_objects which were part of LOCAL_GENERATED_SOURCES;
# use normal_objects here to avoid creating circular dependencies. This assumes
# that custom build rules which generate .o files don't consume other generated
# sources as input (or if they do they take care of that dependency themselves).
$(normal_objects) : | $(my_generated_sources)
$(all_objects) : $(import_includes)
ALL_C_CPP_ETC_OBJECTS += $(all_objects)


###########################################################
# Standard library handling.
###########################################################
# Get the list of BUILT libraries, which are under
# various intermediates directories.
so_suffix := $($(my_prefix)SHLIB_SUFFIX)
a_suffix := $($(my_prefix)STATIC_LIB_SUFFIX)

ifneq ($(LOCAL_SDK_VERSION),)

built_shared_libraries := \
    $(addprefix $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)OUT_INTERMEDIATE_LIBRARIES)/, \
      $(addsuffix $(so_suffix), \
        $(my_shared_libraries)))
built_shared_library_deps := $(addsuffix .toc, $(built_shared_libraries))

# Add the NDK libraries to the built module dependency
my_system_shared_libraries_fullpath := \
    $(my_ndk_stl_shared_lib_fullpath) \
    $(addprefix $(my_ndk_sysroot_lib)/, \
        $(addsuffix $(so_suffix), $(my_system_shared_libraries)))

# We need to preserve the ordering of LOCAL_SHARED_LIBRARIES regardless of
# whether the libs are generated or prebuilt, so we simply can't split into two
# lists and use addprefix.
my_ndk_shared_libraries_fullpath := \
    $(foreach _lib,$(my_ndk_shared_libraries),\
        $(if $(filter $(NDK_MIGRATED_LIBS),$(_lib)),\
            $(my_built_ndk_libs)/$(_lib)$(so_suffix),\
            $(my_ndk_sysroot_lib)/$(_lib)$(so_suffix)))

built_shared_libraries += \
    $(my_ndk_shared_libraries_fullpath) \
    $(my_system_shared_libraries_fullpath) \

else

built_shared_libraries := \
    $(addprefix $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)OUT_INTERMEDIATE_LIBRARIES)/, \
      $(addsuffix $(so_suffix), \
        $(installed_shared_library_module_names)))
built_shared_library_deps := $(addsuffix .toc, $(built_shared_libraries))
my_system_shared_libraries_fullpath :=
endif

built_static_libraries := \
    $(foreach lib,$(my_static_libraries), \
      $(call intermediates-dir-for, \
        STATIC_LIBRARIES,$(lib),$(my_kind),,$(LOCAL_2ND_ARCH_VAR_PREFIX),$(my_host_cross))/$(lib)$(a_suffix))

ifdef LOCAL_SDK_VERSION
built_static_libraries += $(my_ndk_stl_static_lib)
endif

built_whole_libraries := \
    $(foreach lib,$(my_whole_static_libraries), \
      $(call intermediates-dir-for, \
        STATIC_LIBRARIES,$(lib),$(my_kind),,$(LOCAL_2ND_ARCH_VAR_PREFIX),$(my_host_cross))/$(lib)$(a_suffix))


installed_static_library_notice_file_targets := \
    $(foreach lib,$(my_static_libraries) $(my_whole_static_libraries), \
      NOTICE-$(if $(LOCAL_IS_HOST_MODULE),HOST,TARGET)-STATIC_LIBRARIES-$(lib))

# Default is -fno-rtti.
ifeq ($(strip $(LOCAL_RTTI_FLAG)),)
LOCAL_RTTI_FLAG := -fno-rtti
endif

###########################################################
# Rule-specific variable definitions
###########################################################

ifeq ($(my_clang),true)
my_cflags += $(LOCAL_CLANG_CFLAGS)
else
# gcc does not handle hidden functions in a manner compatible with LLVM libcxx
# see b/27908145
my_cflags += -Wno-attributes
endif

my_illegal_flags := -w
my_cflags := $(filter-out $(my_illegal_flags),$(my_cflags))
my_cppflags := $(filter-out $(my_illegal_flags),$(my_cppflags))
my_conlyflags := $(filter-out $(my_illegal_flags),$(my_conlyflags))

# We can enforce some rules more strictly in the code we own. my_strict
# indicates if this is code that we can be stricter with. If we have rules that
# we want to apply to *our* code (but maybe can't for vendor/device specific
# things), we could extend this to be a ternary value.
my_strict := true
# Can be used to make some annotations stricter for code we can fix (such as
# when we mark functions as deprecated).
ifeq ($(my_strict),true)
    my_cflags += -DANDROID_STRICT
endif

# Check if -Werror or -Wno-error is used in C compiler flags.
# Modules defined in $(SOONG_ANDROID_MK) are checked in soong's cc.go.
ifneq ($(LOCAL_MODULE_MAKEFILE),$(SOONG_ANDROID_MK))
  # Header libraries do not need cflags.
  ifneq (HEADER_LIBRARIES,$(LOCAL_MODULE_CLASS))
    # Prebuilt modules do not need cflags.
    ifeq (,$(LOCAL_PREBUILT_MODULE_FILE))
      my_all_cflags := $(my_cflags) $(my_cppflags) $(my_cflags_no_override)
      # Issue warning if -Wno-error is used.
      ifneq (,$(filter -Wno-error,$(my_all_cflags)))
        $(eval MODULES_USING_WNO_ERROR := $(MODULES_USING_WNO_ERROR) $(LOCAL_MODULE_MAKEFILE):$(LOCAL_MODULE))
      else
        # Issue warning if -Werror is not used. Add it.
        ifeq (,$(filter -Werror,$(my_all_cflags)))
          # Add -Wall -Werror unless the project is in the WARNING_ALLOWED project list.
          ifeq (,$(strip $(call find_warning_allowed_projects,$(LOCAL_PATH))))
            my_cflags := -Wall -Werror $(my_cflags)
          else
            $(eval MODULES_ADDED_WALL := $(MODULES_ADDED_WALL) $(LOCAL_MODULE_MAKEFILE):$(LOCAL_MODULE))
            my_cflags := -Wall $(my_cflags)
          endif
        endif
      endif
    endif
  endif
endif

# Disable clang-tidy if it is not found.
my_tidy_checks := $(subst $(space),,$(my_tidy_checks))

# Move -l* entries from ldflags to ldlibs, and everything else to ldflags
my_ldlib_flags := $(my_ldflags) $(my_ldlibs)
my_ldlibs := $(filter -l%,$(my_ldlib_flags))
my_ldflags := $(filter-out -l%,$(my_ldlib_flags))

# One last verification check for ldlibs
my_allowed_ldlibs :=
ifndef LOCAL_IS_HOST_MODULE
  ifneq ($(LOCAL_SDK_VERSION),)
    my_allowed_ldlibs := $(addprefix -l,$(NDK_PREBUILT_SHARED_LIBRARIES))
  endif
else
  my_allowed_ldlibs := $($(my_prefix)AVAILABLE_LIBRARIES)
endif

my_bad_ldlibs := $(filter-out $(my_allowed_ldlibs),$(my_ldlibs))
ifneq ($(my_bad_ldlibs),)
  $(error $(LOCAL_MODULE_MAKEFILE): $(LOCAL_MODULE): Bad LOCAL_LDLIBS entries: $(my_bad_ldlibs))
endif

# my_cxx_ldlibs may contain linker flags need to wrap certain libraries
# (start-group/end-group), so append after the check above.
my_ldlibs += $(my_cxx_ldlibs)

$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_YACCFLAGS := $(LOCAL_YACCFLAGS)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_ASFLAGS := $(my_asflags)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_CONLYFLAGS := $(my_conlyflags)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_CFLAGS := $(my_cflags)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_CPPFLAGS := $(my_cppflags)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_CFLAGS_NO_OVERRIDE := $(my_cflags_no_override)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_CPPFLAGS_NO_OVERRIDE := $(my_cppflags_no_override)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_RTTI_FLAG := $(LOCAL_RTTI_FLAG)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_DEBUG_CFLAGS := $(debug_cflags)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_C_INCLUDES := $(my_c_includes)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_IMPORT_INCLUDES := $(import_includes)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_LDFLAGS := $(my_ldflags)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_LDLIBS := $(my_ldlibs)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_TIDY_CHECKS := $(my_tidy_checks)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_TIDY_FLAGS := $(my_tidy_flags)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_ARFLAGS := $(my_arflags)

# this is really the way to get the files onto the command line instead
# of using $^, because then LOCAL_ADDITIONAL_DEPENDENCIES doesn't work
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_ALL_SHARED_LIBRARIES := $(built_shared_libraries)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_ALL_STATIC_LIBRARIES := $(built_static_libraries)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_ALL_WHOLE_STATIC_LIBRARIES := $(built_whole_libraries)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_ALL_OBJECTS := $(strip $(all_objects))

###########################################################
# Define library dependencies.
###########################################################
# all_libraries is used for the dependencies on LOCAL_BUILT_MODULE.
all_libraries := \
    $(built_shared_library_deps) \
    $(my_system_shared_libraries_fullpath) \
    $(built_static_libraries) \
    $(built_whole_libraries)

# Also depend on the notice files for any static libraries that
# are linked into this module.  This will force them to be installed
# when this module is.
$(LOCAL_INSTALLED_MODULE): | $(installed_static_library_notice_file_targets)

###########################################################
# Export includes
###########################################################
export_includes := $(intermediates)/export_includes
export_cflags := $(foreach d,$(my_export_c_include_dirs),-I $(d))
# Soong exports cflags instead of include dirs, so that -isystem can be included.
ifeq ($(LOCAL_MODULE_MAKEFILE),$(SOONG_ANDROID_MK))
export_cflags += $(LOCAL_EXPORT_CFLAGS)
else ifdef LOCAL_EXPORT_CFLAGS
$(call pretty-error,LOCAL_EXPORT_CFLAGS can only be used by Soong, use LOCAL_EXPORT_C_INCLUDE_DIRS instead)
endif
$(export_includes): PRIVATE_EXPORT_CFLAGS := $(export_cflags)
# Headers exported by whole static libraries are also exported by this library.
export_include_deps := $(strip \
   $(foreach l,$(my_whole_static_libraries), \
     $(call intermediates-dir-for,STATIC_LIBRARIES,$(l),$(my_kind),,$(LOCAL_2ND_ARCH_VAR_PREFIX),$(my_host_cross))/export_includes))
# Re-export requested headers from shared libraries.
export_include_deps += $(strip \
   $(foreach l,$(LOCAL_EXPORT_SHARED_LIBRARY_HEADERS), \
     $(call intermediates-dir-for,SHARED_LIBRARIES,$(l),$(my_kind),,$(LOCAL_2ND_ARCH_VAR_PREFIX),$(my_host_cross))/export_includes))
# Re-export requested headers from static libraries.
export_include_deps += $(strip \
   $(foreach l,$(LOCAL_EXPORT_STATIC_LIBRARY_HEADERS), \
     $(call intermediates-dir-for,STATIC_LIBRARIES,$(l),$(my_kind),,$(LOCAL_2ND_ARCH_VAR_PREFIX),$(my_host_cross))/export_includes))
# Re-export requested headers from header libraries.
export_include_deps += $(strip \
   $(foreach l,$(LOCAL_EXPORT_HEADER_LIBRARY_HEADERS), \
     $(call intermediates-dir-for,HEADER_LIBRARIES,$(l),$(my_kind),,$(LOCAL_2ND_ARCH_VAR_PREFIX),$(my_host_cross))/export_includes))
$(export_includes): PRIVATE_REEXPORTED_INCLUDES := $(export_include_deps)
# By adding $(my_generated_sources) it makes sure the headers get generated
# before any dependent source files get compiled.
$(export_includes) : $(my_export_c_include_deps) $(my_generated_sources) $(export_include_deps) $(LOCAL_EXPORT_C_INCLUDE_DEPS)
	@echo Export includes file: $< -- $@
	$(hide) mkdir -p $(dir $@) && rm -f $@.tmp && touch $@.tmp
ifdef export_cflags
	$(hide) echo "$(PRIVATE_EXPORT_CFLAGS)" >>$@.tmp
endif
ifdef export_include_deps
	$(hide) for f in $(PRIVATE_REEXPORTED_INCLUDES); do \
		cat $$f >> $@.tmp; \
		done
endif
	$(hide) if cmp -s $@.tmp $@ ; then \
	  rm $@.tmp ; \
	else \
	  mv $@.tmp $@ ; \
	fi
export_cflags :=

# Kati adds restat=1 to ninja. GNU make does nothing for this.
.KATI_RESTAT: $(export_includes)

# Make sure export_includes gets generated when you are running mm/mmm
$(LOCAL_BUILT_MODULE) : | $(export_includes)

ifneq ($(LOCAL_MODULE_MAKEFILE),$(SOONG_ANDROID_MK))
ifneq (,$(filter-out $(LOCAL_PATH)/%,$(my_export_c_include_dirs)))
my_soong_problems += non_local__export_c_include_dirs
endif

SOONG_CONV.$(LOCAL_MODULE).PROBLEMS := \
    $(SOONG_CONV.$(LOCAL_MODULE).PROBLEMS) $(my_soong_problems)
SOONG_CONV.$(LOCAL_MODULE).DEPS := \
    $(SOONG_CONV.$(LOCAL_MODULE).DEPS) \
    $(filter-out $($(LOCAL_2ND_ARCH_VAR_PREFIX)UBSAN_RUNTIME_LIBRARY),\
        $(my_static_libraries) \
        $(my_whole_static_libraries) \
        $(my_shared_libraries) \
        $(my_system_shared_libraries))
SOONG_CONV.$(LOCAL_MODULE).TYPE := native
SOONG_CONV := $(SOONG_CONV) $(LOCAL_MODULE)
endif
