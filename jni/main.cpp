#include "BinarizeProcessor.h"
#include "BinarizeProcessorCPU.h"
#include "FinderPattern.h"
#include "GLContextManager.h"
#include "GLProgramManager.h"
#include "ImageProcessorWorkflow.h"
#include "QRCodeDetector.h"
#include <cmath>
#include <memory>
#include <nvImage.h>
#define LOGE(tag, ...) GLIMPROC_LOGE(__VA_ARGS__)
#if defined(ANDROID)
#define BITMAP_PATH "/sdcard/shit.bmp"
#else
#define BITMAP_PATH "shit.bmp"
#endif
#define USE_CPU 1

static nv::Image*
gaussianLoadImageFromFile(const char* file)
{
  std::unique_ptr<nv::Image> image(new nv::Image());

  if (!image->loadImageFromFile(file)) {
    return NULL;
  }

  return image.release();
}

typedef uint32_t DWORD;
typedef int32_t LONG;
typedef uint16_t WORD;
typedef void* LPVOID;
typedef uint8_t BYTE;
typedef char* LPSTR;
#define BI_RGB (0)

typedef struct __attribute__((packed)) tagRGBQUAD
{
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
} RGBQUAD;
typedef struct __attribute__((packed)) tagBITMAPINFOHEADER
{
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct __attribute__((packed)) tagBITMAPFILEHEADER
{
  WORD bfType;
  DWORD bfSize;
  WORD bfReserved1;
  WORD bfReserved2;
  DWORD bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;

// used to find out every color buffer content.
static void
CreateBMPFile(const char* pszFile, PBITMAPINFOHEADER pbi, const void* lpBits)
{
  FILE* hf;               // file handle
  BITMAPFILEHEADER hdr;   // bitmap file-header
  PBITMAPINFOHEADER pbih; // bitmap info-header
  DWORD cb;               // incremental count of bytes
  const void* hp;         // byte pointer

  pbih = (PBITMAPINFOHEADER)pbi;

  // Create the .BMP file.
  hf = fopen(pszFile, "wb");
  if (hf == nullptr)
    GLIMPROC_LOGE("fails to create file.\n");
  hdr.bfType = 0x4d42; // 0x42 = "B" 0x4d = "M"
  // Compute the size of the entire file.
  hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + pbih->biSize +
                       pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);
  hdr.bfReserved1 = 0;
  hdr.bfReserved2 = 0;

  // Compute the offset to the array of color indices.
  hdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + pbih->biSize +
                  pbih->biClrUsed * sizeof(RGBQUAD);

  // Copy the BITMAPFILEHEADER into the .BMP file.
  if (1 != fwrite((LPVOID)&hdr, sizeof(BITMAPFILEHEADER), 1, hf)) {
    GLIMPROC_LOGE("fails to write file: header.\n");
  }

  // Copy the BITMAPINFOHEADER and RGBQUAD array into the file.
  if (1 != fwrite((LPVOID)pbih,
                  sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD),
                  1, hf))
    GLIMPROC_LOGE("fails to write file info header.\n");
  cb = pbih->biSizeImage;
  // Copy the array of color indices into the .BMP file.
  hp = lpBits;
  if (cb != fwrite((LPSTR)hp, 1, (int)cb, hf))
    GLIMPROC_LOGE("fails to write file: pixels.\n");

  // Close the .BMP file.
  fclose(hf);
}

static void
rgb2bgr(int count, char* data)
{
  for (int i = 0; i < count; i += 3) {
    char r = data[i];
    char b = data[i + 2];
    data[i] = b;
    data[i + 2] = r;
  }
}

static void
saveBitmap(int width, int height, const char* fileName, const char* data)
{
  BITMAPINFOHEADER hdr;

  hdr.biSize = sizeof(hdr);
  hdr.biWidth = width;
  hdr.biHeight = height;
  hdr.biPlanes = 1;
  hdr.biBitCount = 24;
  hdr.biCompression = BI_RGB;
  hdr.biSizeImage = width * height * 3;
  hdr.biXPelsPerMeter = 0;
  hdr.biClrUsed = 0;
  hdr.biClrImportant = 0;
  CreateBMPFile(fileName, &hdr, data);
}

int
main(int argc, char** argv)
{
  if (argc != 2) {
    printf("need a damn file.\n");
    return 1;
  }
  std::unique_ptr<nv::Image> image(gaussianLoadImageFromFile(argv[1]));
  if (image.get() == nullptr) {
    printf("fails to load image.\n");
    return 1;
  }
#if !defined(USE_CPU)
  EGLDisplay dpy;
  dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (dpy == EGL_NO_DISPLAY) {
    printf("fails to create display.\n");
    return 1;
  }

  if (!eglInitialize(dpy, nullptr, nullptr)) {
    printf("fails to init egl.\n");
    return 1;
  }
  GLContextManager glContextManager;
  if (!glContextManager.init()) {
    printf("fails to create context.\n");
    return 1;
  }
  {
    GLContextScope scope(glContextManager);
    GLProgramManager pm(glContextManager.getGL3Interfaces());
    ImageProcessorWorkflow wf;

    std::unique_ptr<BinarizeProcessor> binarizeProcess(new BinarizeProcessor);
    if (!binarizeProcess->init(&pm)) {
      GLIMPROC_LOGE("fails to init binarizeProcess.\n");
      return 1;
    }
    wf.registerIImageProcessor(binarizeProcess.get());
    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    uint8_t* procp;

    ImageDesc desc = { image->getWidth(), image->getHeight(),
                       image->getFormat(), image->getLevel(0) };
    ImageOutput imo = wf.process(glContextManager.getGL3Interfaces(), desc);
    std::unique_ptr<uint8_t[]> readback(std::move(imo.outputBytes));
    std::unique_ptr<uint8_t[]> processed(new uint8_t[desc.width * desc.height]);
    const uint8_t* rbp = readback.get();
    procp = processed.get();
    for (int i = 0; i < desc.width * desc.height; ++i, ++procp, rbp += 4) {
      *procp = *rbp;
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);

#else
  {
    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    uint8_t* procp;
    std::unique_ptr<uint8_t[]> processed(
      binarizeProcessCPU(image->getWidth(), image->getHeight(),
                         static_cast<const uint8_t*>(image->getLevel(0))));
    QRCodeDetector detector;
    clock_gettime(CLOCK_MONOTONIC, &t2);
    FinderPatternInfo fi =
      detector.detect(image->getWidth(), image->getHeight(), processed.get());
    if (fi.getBottomLeft())
      printf("found at (%lf, %lf), (%lf, %lf), (%lf, %lf).\n",
             fi.getBottomLeft()->getX(), fi.getBottomLeft()->getY(),
             fi.getTopLeft()->getX(), fi.getTopLeft()->getY(),
             fi.getTopRight()->getX(), fi.getTopRight()->getY());
    else
      printf("not found.\n");
#endif
    int rowBytes = (image->getWidth() * 24 + 31) / 32 * 4;
    std::unique_ptr<uint8_t[]> saveBits(
      new uint8_t[rowBytes * image->getHeight()]);
    uint8_t* savep = saveBits.get();
    procp = processed.get();
    for (int y = 0; y < image->getHeight(); ++y, savep += rowBytes) {
      uint8_t* rowp = savep;
      for (int x = 0; x < image->getWidth(); ++x, ++procp, rowp += 3) {
        rowp[0] = *procp;
        rowp[1] = *procp;
        rowp[2] = *procp;
#if defined(USE_CPU)
        if (fi.getBottomLeft()) {
          if (std::abs(x - fi.getBottomLeft()->getX()) < 5 &&
              std::abs(y - fi.getBottomLeft()->getY()) < 5) {
            rowp[0] = 0;
            rowp[1] = 0;
            rowp[2] = 255;
          }
        }
        if (fi.getBottomLeft()) {
          if (std::abs(x - fi.getTopLeft()->getX()) < 5 &&
              std::abs(y - fi.getTopLeft()->getY()) < 5) {
            rowp[0] = 0;
            rowp[1] = 0;
            rowp[2] = 255;
          }
        }
        if (fi.getBottomLeft()) {
          if (std::abs(x - fi.getTopRight()->getX()) < 5 &&
              std::abs(y - fi.getTopRight()->getY()) < 5) {
            rowp[0] = 0;
            rowp[1] = 0;
            rowp[2] = 255;
          }
        }
#endif
      }
    }
    saveBitmap(image->getWidth(), image->getHeight(), BITMAP_PATH,
               reinterpret_cast<char*>(saveBits.get()));

    printf("one frame: %lf.\n", ((double)(t2.tv_sec - t1.tv_sec) +
                                 ((double)(t2.tv_nsec - t1.tv_nsec) / 1e9)));
  }
  return 0;
}
