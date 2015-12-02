
include $(CLEAR_VARS)
LOCAL_MODULE	:= slibx
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_SRC_FILES := $(SLIBX_BASE)/projects/Slibx/android-eclipse/obj/local/armeabi-v7a/libslibx.a
endif
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
LOCAL_SRC_FILES := $(SLIBX_BASE)/projects/Slibx/android-eclipse/obj/local/arm64-v8a/libslibx.a
endif
include $(PREBUILT_STATIC_LIBRARY)
