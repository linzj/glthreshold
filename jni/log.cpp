#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#if defined(ANDROID) && defined(ANDROID_LOGCAT_ENABLED)
#define USE_ANDROID_LOGCAT 1
#else
#define USE_ANDROID_LOGCAT 0
#endif

#if USE_ANDROID_LOGCAT
#include <android/log.h>
static const char* TAG = "GLIMGPROC";
#else
#include <sys/syscall.h> /* For SYS_xxx definitions */
#include <unistd.h>
#endif // USE_ANDROID_LOGCAT
static const int MAX_BUFFER_SIZE = 256;

void
GLIMPROC_LOGE(const char* fmt, ...)
{
  char buf[MAX_BUFFER_SIZE];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, MAX_BUFFER_SIZE, fmt, args);
  va_end(args);
#if USE_ANDROID_LOGCAT
  __android_log_write(ANDROID_LOG_ERROR, TAG, buf);
#else
  fprintf(stderr, "%lu: ", syscall(__NR_gettid, 0));
  fputs(buf, stderr);
#endif // USE_ANDROID_LOGCAT
}

void
GLIMPROC_LOGI(const char* fmt, ...)
{
  char buf[MAX_BUFFER_SIZE];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, MAX_BUFFER_SIZE, fmt, args);
  va_end(args);
#if USE_ANDROID_LOGCAT
  __android_log_write(ANDROID_LOG_INFO, TAG, buf);
#else
  fprintf(stderr, "%lu: ", syscall(__NR_gettid, 0));
  fputs(buf, stderr);
#endif // USE_ANDROID_LOGCAT
}
