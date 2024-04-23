################################################################################
#
# LittleEndian Utility Android make file
#
# Copyright (c) 2014 - 2016. Little Endian Ltd. All rights reserved.
#
################################################################################

ifndef LE_SDK_PATH
    LE_SDK_PATH := $(call my-dir)/..
endif

################################################################################
# Define the LE Utility module:
################################################################################

LOCAL_PATH:= $(LE_SDK_PATH)

include $(CLEAR_VARS)

LOCAL_MODULE            := le_utility
LOCAL_EXPORT_C_INCLUDES := $(LE_SDK_PATH)/include
LOCAL_EXPORT_LDLIBS     += -landroid -llog
ifeq ($(TARGET_ARCH_ABI),x86)
    LOCAL_SRC_FILES     := libs/release/libLE_Utility_Android_x86-32_SSSE3.a
else
ifeq ($(TARGET_ARCH_ABI),x86_64)
    LOCAL_SRC_FILES     := libs/release/libLE_Utility_Android_x86-64_SSE4.2.a
else
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
   #LOCAL_SRC_FILES     := libs/release/libLE_Utility_Android_ARMv7a_VFP3-D16.a
    LOCAL_SRC_FILES     := libs/release/libLE_Utility_Android_ARMv7a_NEON.a
    LOCAL_ARM_NEON      := true
else
    LOCAL_SRC_FILES     := libs/release/libLE_Utility_Android_ARMv6_VFP2.a
    LOCAL_EXPORT_LDLIBS += -latomic
endif
endif
endif

include $(PREBUILT_STATIC_LIBRARY)
