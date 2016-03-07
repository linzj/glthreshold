#include "GLContextManager.h"
#include "ImageProcessor.h"
#include <memory>
#include <nvImage.h>
#include <stdio.h>

static nv::Image*
gaussianLoadImageFromFile(const char* file)
{
  std::unique_ptr<nv::Image> image(new nv::Image());

  if (!image->loadImageFromFile(file)) {
    return NULL;
  }

  return image.release();
}

#if 0
typedef uint32_t DWORD;
typedef long LONG;
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
  DWORD dwTmp;

  pbih = (PBITMAPINFOHEADER)pbi;

  // Create the .BMP file.
  hf = fopen(pszFile, "wb");
  if (hf == nullptr)
    fprintf(stderr, "fails to create file.\n");
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
    fprintf(stderr, "fails to write file: header.\n");
  }

  // Copy the BITMAPINFOHEADER and RGBQUAD array into the file.
  if (1 != fwrite((LPVOID)pbih,
                  sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD),
                  1, hf))
    fprintf(stderr, "fails to write file info header.\n");
  cb = pbih->biSizeImage;
  // Copy the array of color indices into the .BMP file.
  hp = lpBits;
  if (cb != fwrite((LPSTR)hp, 1, (int)cb, hf))
    fprintf(stderr, "fails to write file: pixels.\n");

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
#endif

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
    ImageProcessor processor;
    if (!processor.init(211)) {
      printf("fails to create image processor.\n");
      return 1;
    }
    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    ImageDesc desc = { image->getWidth(), image->getHeight(),
                       image->getFormat(), image->getLevel(0) };
    std::unique_ptr<uint8_t[]> readback(processor.process(desc));
    std::unique_ptr<uint8_t[]> processed(new uint8_t[desc.width * desc.height]);
    const uint8_t* rbp = readback.get();
    uint8_t* procp = processed.get();
    for (int i = 0; i < desc.width * desc.height; ++i, ++procp, rbp += 3) {
      *procp = *rbp;
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);
#if 0
    saveBitmap(desc.width, desc.height, "/sdcard/shit.bmp",
               reinterpret_cast<char*>(readback.get()));
#endif
    printf("one frame: %lf.\n", ((double)(t2.tv_sec - t1.tv_sec) +
                                 ((double)(t2.tv_nsec - t1.tv_nsec) / 1e9)));
  }
  return 0;
}
