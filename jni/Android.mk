LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := image
LOCAL_SRC_FILES := image_process.c
LOCAL_STATIC_LIBRARIES := jpeg
LOCAL_STATIC_LIBRARIES += png
LOCAL_C_INCLUDES += $(LOCAL_PATH)/libjpeg
LOCAL_C_INCLUDES += $(LOCAL_PATH)/libpng
LOCAL_LDLIBS := -llog -lz

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

include $(LOCAL_PATH)/libjpeg/Android.mk $(LOCAL_PATH)/libpng/Android.mk