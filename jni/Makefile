.PHONY: all
all: glthres
.SUFFIXES:

OMPFLAGS := -fopenmp

INCLUDES := -InvImage/include
CFLAGS := -O2 -ggdb
CXXFLAGS := -std=c++11 $(INCLUDES) $(CFLAGS) $(OMPFLAGS) #-fsanitize=address
LDFLAGS := $(OMPFLAGS) #-fsanitize=address

%.o: %.c
	gcc $(CFLAGS) -c $*.c -o $*.o
	gcc -MM $(CFLAGS) $*.c > $*.d

%.o: %.cpp
	g++ $(CXXFLAGS) -c $*.cpp -o $*.o
	g++ -MM $(CXXFLAGS) $*.cpp > $*.d

%.o: %.S
	g++ $(CFLAGS) -c $*.S -o $*.o
	g++ -MM $(CFLAGS) $*.S > $*.d

CXX_SOURCES := main.cpp \
log.cpp \
GLContextManager.cpp \
ImageProcessorWorkflow.cpp \
BinarizeProcessor.cpp \
BinarizeProcessorCPU.cpp \
RGBToLuminance.cpp \
FinderPattern.cpp \
FinderPatternInfo.cpp \
QRCodeDetector.cpp \
AlignmentPattern.cpp \
AlignmentPatternFinder.cpp \
FormatInformation.cpp \
DataMask.cpp \
BitMatrixParser.cpp \
GenericGF.cpp \
GenericGFPoly.cpp \
ReedSolomonDecoder.cpp \
DecoderResult.cpp \
BitSource.cpp \
DecodedBitStreamParser.cpp \
QRCodeDecoder.cpp \
DataBlock.cpp \
ErrorCorrectionLevel.cpp \
DefaultGridSampler.cpp \
GridSampler.cpp \
LuminanceImage.cpp \
PerspectiveTransform.cpp \
Version.cpp \
ResultPoint.cpp \
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

OBJS := $(CXX_SOURCES:.cpp=.o) $(C_SOURCES:.c=.o)

-include $(OBJS:.o=.d)

LDLIBS := -lEGL -lGLESv2 -lz

glthres: $(OBJS)
	g++ $(LDFLAGS) -o $@ $^ $(LDLIBS) -ldl

$(GLSL_BINDING): glsl.glsl updateglsl.py
	python updateglsl.py glsl.glsl

clean:
	rm glsl.glsl.c $(OBJS) $(CXX_SOURCES:.cpp=.d) $(C_SOURCES:.c=.d)
