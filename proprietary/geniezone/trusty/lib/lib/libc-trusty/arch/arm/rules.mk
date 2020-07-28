LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/trusty_syscall.S

FIRST_OBJ := $(BUILDDIR)/crtbegin.o
LAST_OBJ  := $(BUILDDIR)/crtend.o

$(FIRST_OBJ): $(LOCAL_DIR)/crtbegin.c $(CONFIGHEADER)
	@$(MKDIR)
	@echo compiling $<
	$(NOECHO)$(CC) $(GLOBAL_COMPILEFLAGS) $(GLOBAL_CFLAGS) $(GLOBAL_INCLUDES) $(ARCH_COMPILEFLAGS) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

$(LAST_OBJ): $(LOCAL_DIR)/crtend.S $(CONFIGHEADER)
	@$(MKDIR)
	@echo compiling $<
	$(NOECHO)$(CC) $(GLOBAL_COMPILEFLAGS) $(GLOBAL_ASMFLAGS) $(GLOBAL_INCLUDES) $(ARCH_COMPILEFLAGS) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

ALLMODULE_OBJS += $(FIRST_OBJ) $(LAST_OBJ)

FIRST_OBJ :=
LAST_OBJ  :=
