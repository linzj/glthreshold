#ifndef BINARIZEPROCESSORCPU_H
#define BINARIZEPROCESSORCPU_H
#include <memory>
#include <stdint.h>
std::unique_ptr<uint8_t[]> binarizeProcessCPU(int width, int height,
                                              const uint8_t* data);
#endif /* BINARIZEPROCESSORCPU_H */
