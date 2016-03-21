INCLUDES := -InvImage/include
CXXFLAGS := -std=c++11 $(INCLUDES)

%.o: %.c
	gcc $(CFLAGS) -c $*.c -o $*.o
	gcc -MM $(CFLAGS) $*.c > $*.d

%.o: %.cpp
	g++ $(CXXFLAGS) -c $*.cpp -o $*.o
	g++ -MM $(CXXFLAGS) $*.cpp > $*.d

%.o: %.S
	g++ $(CFLAGS) -c $*.S -o $*.o
	g++ -MM $(CFLAGS) $*.S > $*.d

OBJS := main.o \
log.o \
GLContextManager.o \
AdaptiveThresholdProcessor.o \
ThresholdProcessor.o \
DilateNonZeroProcessor.o \
ErodeNonZeroProcessor.o \
ImageProcessorWorkflow.o \
GLResources.o \
GLCommon.o \
GLProgramManager.o \
glsl.glsl.o \
nvImage/src/rgbe.o \
nvImage/src/nvImage.o \
nvImage/src/nvImagePng.o \
libpng-1.2.51/png.o \
libpng-1.2.51/pngerror.o \
libpng-1.2.51/pnggccrd.o \
libpng-1.2.51/pngget.o \
libpng-1.2.51/pngmem.o \
libpng-1.2.51/pngpread.o \
libpng-1.2.51/pngread.o \
libpng-1.2.51/pngrio.o \
libpng-1.2.51/pngrtran.o \
libpng-1.2.51/pngrutil.o \
libpng-1.2.51/pngset.o \
libpng-1.2.51/pngtrans.o \
libpng-1.2.51/pngvcrd.o \
libpng-1.2.51/pngwio.o \
libpng-1.2.51/pngwrite.o \
libpng-1.2.51/pngwtran.o \
libpng-1.2.51/pngwutil.o \

CXX_SOURCES := main.cpp \
log.cpp \
GLContextManager.cpp \
ImageProcessorWorkflow.cpp \
GLResources.cpp \
GLCommon.cpp \
GLProgramManager.cpp \
nvImage/src/nvImage.cpp \
nvImage/src/nvImagePng.cpp \

GLSL_BINDING := glsl.glsl.c

C_SOURCES := \
$(GLSL_BINDING) \
nvImage/src/rgbe.c \
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


LDLIBS := -lEGL -lGLESv2 -lz

glthres: $(C_SOURCES:.c=.o) $(CXX_SOURCES:.cpp=.o)
	g++ $(LDFLAGS) -o $@ $^ $(LDLIBS) -ldl

$(GLSL_BINDING): glsl.glsl updateglsl.py
	python updateglsl.py glsl.glsl

all: glthres

.PHONY: all
.SUFFIXES:

clean:
	rm *.o *.d glsl.glsl.c
