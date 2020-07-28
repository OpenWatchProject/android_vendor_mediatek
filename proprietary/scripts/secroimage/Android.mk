# secro image
SECRO_TYPE := GMP
ifeq ($(strip $(SECRO_TYPE)),)
  MTK_SECRO_POST_TOOL := $(MTK_PATH_SOURCE)/scripts/secroimage/SECRO_POST
else
  MTK_SECRO_POST_TOOL := $(MTK_PATH_SOURCE)/scripts/secroimage/SECRO_POST_$(SECRO_TYPE)
endif
ifeq ($(HOST_OS),darwin)
  MTK_SECRO_POST_TOOL := $(MTK_SECRO_POST_TOOL).darwin
endif


# $(1): secro.img output
# $(2): SECRO_WP.ini output
# $(3): SECRO_POST
# $(4): SML_ENCODE_KEY.ini
# $(5): SECRO_CFG
# $(6): AND_SECRO
# $(7): AC_REGION
# $(8): SECRO[0]
# $(9): SECRO[1]
# $(10): SECRO[2]
# $(11): SECRO[3]
# $(12): SECRO[4]
# $(13): SECRO[5]
# $(14): SECRO[6]
# $(15): SECRO[7]
# $(16): SECRO[8]
# $(17): SECRO[9]
define secro-one-image
$(1): $(2) $(3) $(4) $(5) $(6) $(7) $(8) $(9) $(10) $(11) $(12) $(13) $(14) $(15) $(16) $(17)
	@mkdir -p $$(dir $$@)
	$(3) $(2) $(4) $$@

$(2): $(5) $(6) $(7) $(8) $(9) $(10) $(11) $(12) $(13) $(14) $(15) $(16) $(17)
	@echo Write $$@ for $(1)
	@mkdir -p $$(dir $$@)
	@rm -f $$@
	@echo "SECRO_CFG = $(5)" >> $$@
	@echo "AND_SECRO = $(6)" >> $$@
	@echo "AC_REGION = $(7)" >> $$@
	@echo "SECRO[0] = $(8)" >> $$@
	@echo "SECRO[1] = $(9)" >> $$@
	@echo "SECRO[2] = $(10)" >> $$@
	@echo "SECRO[3] = $(11)" >> $$@
	@echo "SECRO[4] = $(12)" >> $$@
	@echo "SECRO[5] = $(13)" >> $$@
	@echo "SECRO[6] = $(14)" >> $$@
	@echo "SECRO[7] = $(15)" >> $$@
	@echo "SECRO[8] = $(16)" >> $$@
	@echo "SECRO[9] = $(17)" >> $$@
endef


ifneq ($(strip $(MTK_PROJECT_NAME)),)
ifneq ($(TARGET_ARCH),x86)
ifneq ($(TARGET_ARCH),x86_64)

MTK_SEC_SECRO_AC_SUPPORT := yes
INSTALLED_SECROIMAGE_TARGET := $(PRODUCT_OUT)/secro.img
MTK_SECRO_WP_INI := $(TARGET_OUT_INTERMEDIATES)/SECRO/SECRO_WP.ini
ifeq ($(strip $(SECRO_TYPE)),GMP)
  MTK_SECRO_SECRO_CFG := $(MTK_PATH_COMMON)/secro/SECRO_GMP.ini
else
  MTK_SECRO_SECRO_CFG := $(MTK_PATH_COMMON)/secro/SECRO_DEFAULT_LOCK_CFG.ini
endif
MTK_SECRO_SML_CONFIG := $(MTK_PATH_CUSTOM)/security/sml_auth/SML_ENCODE_KEY.ini
MTK_SECRO_AC_REGION := $(firstword $(wildcard $(MTK_PATH_CUSTOM)/secro/AC_REGION) $(MTK_PATH_COMMON)/secro/AC_REGION)
MTK_SECRO_AND_SECRO := $(firstword $(wildcard $(MTK_PATH_CUSTOM)/secro/AND_SECURE_RO) $(MTK_PATH_COMMON)/secro/AND_SECURE_RO)
MTK_COMMON_SECURE_RO := $(MTK_PATH_COMMON)/secro/SECURE_RO
ifneq ($(CUSTOM_MODEM),)
  MTK_MODEM_SECURO_RO := $(MTK_PATH_CUSTOM)/modem/$(firstword $(CUSTOM_MODEM))/SECURE_RO
endif
ifeq ($(MTK_SEC_SECRO_AC_SUPPORT),yes)
  ifeq ($(wildcard $(MTK_MODEM_SECURO_RO)),)
    MTK_MODEM_SECURO_RO := $(MTK_COMMON_SECURE_RO)
  endif
else
  MTK_MODEM_SECURO_RO := $(MTK_COMMON_SECURE_RO)
endif

.PHONY: secroimage
$(eval $(call secro-one-image,\
	$(INSTALLED_SECROIMAGE_TARGET),\
	$(MTK_SECRO_WP_INI),\
	$(MTK_SECRO_POST_TOOL),\
	$(MTK_SECRO_SML_CONFIG),\
	$(MTK_SECRO_SECRO_CFG),\
	$(MTK_SECRO_AND_SECRO),\
	$(MTK_SECRO_AC_REGION),\
	$(MTK_MODEM_SECURO_RO),\
	$(MTK_MODEM_SECURO_RO),\
	$(MTK_COMMON_SECURE_RO),\
	$(MTK_COMMON_SECURE_RO),\
	$(MTK_COMMON_SECURE_RO),\
	$(MTK_COMMON_SECURE_RO),\
	$(MTK_COMMON_SECURE_RO),\
	$(MTK_COMMON_SECURE_RO),\
	$(MTK_COMMON_SECURE_RO),\
	$(MTK_COMMON_SECURE_RO)\
	))

secroimage: $(INSTALLED_SECROIMAGE_TARGET)
droidcore: secroimage

endif
endif
endif
