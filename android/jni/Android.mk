#include $(call all-subdir-makefiles)

LOCAL_PATH := $(call my-dir)

ZPATH := $(LOCAL_PATH)

include $(ZPATH)/SDL2/src/SDL2_DL/Android.mk
include $(ZPATH)/SDL2_image/src/SDL2_image_DL/Android.mk
include $(ZPATH)/SDL2_ttf/src/SDL2_ttf_DL/Android.mk
include $(ZPATH)/EntityX.mk
include $(ZPATH)/Bullet.mk
