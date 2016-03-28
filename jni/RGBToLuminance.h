#ifndef RGBTOLUMINANCE_H
#define RGBTOLUMINANCE_H
#include <memory>
#include <stdint.h>
std::unique_ptr<uint8_t[]> RGBToLuminance(int width, int height,
                                          const void* data);
std::unique_ptr<uint8_t[]> RGBAToLuminance(int width, int height,
                                           const void* data);
#endif /* RGBTOLUMINANCE_H */
