# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CXXFLAGS += -std=c++11
LOCAL_CFLAGS += -I$(LOCAL_PATH)/nvImage/include -I$(LOCAL_PATH)/libpng-1.2.51
LOCAL_MODULE    := glthreshold
LOCAL_SRC_FILES := main.cpp \
GLContextManager.cpp \
AdaptiveThresholdProcessor.cpp \
DilateNonZeroProcessor.cpp \
ImageProcessorWorkflow.cpp \
GLResources.cpp \
GLCommon.cpp \
GLProgramManager.cpp \
glsl.glsl.c \
nvImage/src/rgbe.c \
nvImage/src/nvImage.cpp \
nvImage/src/nvImagePng.cpp \
libpng-1.2.51/png.c \
libpng-1.2.51/pngerror.c \
libpng-1.2.51/pnggccrd.c \
libpng-1.2.51/pngget.c \
libpng-1.2.51/pngmem.c \
libpng-1.2.51/pngpread.c \
libpng-1.2.51/pngread.c \
libpng-1.2.51/pngrio.c \
libpng-1.2.51/pngrtran.c \
libpng-1.2.51/pngrutil.c \
libpng-1.2.51/pngset.c \
libpng-1.2.51/pngtrans.c \
libpng-1.2.51/pngvcrd.c \
libpng-1.2.51/pngwio.c \
libpng-1.2.51/pngwrite.c \
libpng-1.2.51/pngwtran.c \
libpng-1.2.51/pngwutil.c \

GLSL_BINDING := \
	jni/glsl.glsl.c

$(GLSL_BINDING): jni/glsl.glsl jni/updateglsl.py
	python jni/updateglsl.py $<

.PHONY: $(GLSL_BINDING)

LOCAL_LDLIBS = -lz -lGLESv2 -lEGL
include $(BUILD_EXECUTABLE)
