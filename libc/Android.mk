LOCAL_PATH := $(call my-dir)

libc_ext_sources := \
	bionic/stubs.c \
	kernel/common/linux/ifaddrs.c \
	netbsd/gethnamaddr.c \
	stdlib/locale.c \
	unistd/swab.c \
	unistd/opendir.c \

libc_ext_includes := $(LOCAL_PATH) $(LOCAL_PATH)/include

include $(CLEAR_VARS)

LOCAL_MODULE      := libc-ext
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES   := $(libc_ext_sources)
LOCAL_CFLAGS      += $(libc_ext_cflags)
LOCAL_C_INCLUDES  += $(libc_ext_includes)

include $(BUILD_STATIC_LIBRARY)

