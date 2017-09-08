LOCAL_PATH := entityx/src/entityx

include $(CLEAR_VARS)

LOCAL_MODULE := entityx 

LOCAL_MODULE_FILENAME := libentityx

LOCAL_SRC_FILES := \
entityx/Entity.cc  \
entityx/Event.cc  \
entityx/System.cc \
entityx/help/Pool.cc \
entityx/help/Timer.cc \


LOCAL_C_INCLUDES := entityx/src/entityx \
entityx/Entity.h  \
entityx/Event.h  \
entityx/System.h \
entityx/help/Pool.h \
entityx/help/Timer.h \

include $(BUILD_SHARED_LIBRARY)
