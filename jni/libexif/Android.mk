LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := exif
LOCAL_SRC_FILES := exif-byte-order.c exif-content.c exif-data.c exif-entry.c \
			exif-format.c exif-ifd.c exif-loader.c exif-log.c exif-mem.c \
			exif-mnote-data.c exif-tag.c exif-utils.c \
			canon/exif-mnote-data-canon.c canon/mnote-canon-entry.c canon/mnote-canon-tag.c \
			fuji/exif-mnote-data-fuji.c fuji/mnote-fuji-entry.c fuji/mnote-fuji-tag.c \
			olympus/exif-mnote-data-olympus.c olympus/mnote-olympus-entry.c olympus/mnote-olympus-tag.c \
			pentax/exif-mnote-data-pentax.c pentax/mnote-pentax-entry.c pentax/mnote-pentax-tag.c
        
include $(BUILD_STATIC_LIBRARY)