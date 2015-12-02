LOCAL_PATH := $(call my-dir)

SLIB_HOME = ../../../../Slib
SRC_BASE = ../../../../src
SLIB_MAP = $(SRC_BASE)/slibx/map
SLIB_SFILE = $(SRC_BASE)/slibx/sfile
SLIB_SNET = $(SRC_BASE)/slibx/snet

include $(CLEAR_VARS)
LOCAL_MODULE    := slibx
LOCAL_ARM_MODE := arm 
LOCAL_C_INCLUDES += $(SLIB_HOME)/inc

SLIB_MAP_FILES := $(wildcard $(LOCAL_PATH)/$(SLIB_MAP)/*.cpp)
SLIB_SFILE_FILES := $(wildcard $(LOCAL_PATH)/$(SLIB_SFILE)/*.cpp)
SLIB_SNET_FILES := $(wildcard $(LOCAL_PATH)/$(SLIB_SNET)/*.cpp)

LOCAL_SRC_FILES := $(SLIB_MAP_FILES:$(LOCAL_PATH)/%=%) 
LOCAL_SRC_FILES += $(SLIB_SFILE_FILES:$(LOCAL_PATH)/%=%) 
LOCAL_SRC_FILES += $(SLIB_SNET_FILES:$(LOCAL_PATH)/%=%) 

include $(BUILD_STATIC_LIBRARY)
