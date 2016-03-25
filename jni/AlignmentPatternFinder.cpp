#include "AlignmentPatternFinder.h"
#include "LuminanceImage.h"
#include <string.h>
AlignmentPatternFinder::AlignmentPatternFinder(LuminanceImage* image_,
                                               int startX_, int startY_,
                                               int width_, int height_,
                                               float moduleSize_)
  : image(image_)
  , startX(startX_)
  , startY(startY_)
  , width(width_)
  , height(height_)
  , moduleSize(moduleSize_)
{
}

std::unique_ptr<AlignmentPattern>
AlignmentPatternFinder::find()
{
  const LuminanceImage& image = *this->image;
  int startX = this->startX;
  int height = this->height;
  int maxJ = startX + width;
  int middleI = startY + (height / 2);
  // We are looking for black/white/black modules in 1:1:1 ratio;
  // this tracks the number of black/white/black modules seen so far
  for (int iGen = 0; iGen < height; iGen++) {
    int stateCount[3];
    // Search from middle outwards
    int i = middleI + ((iGen & 0x01) == 0 ? (iGen + 1) / 2 : -((iGen + 1) / 2));
    stateCount[0] = 0;
    stateCount[1] = 0;
    stateCount[2] = 0;
    int j = startX;
    // Burn off leading white pixels before anything else; if we start in the
    // middle of
    // a white run, it doesn't make sense to count its length, since we don't
    // know if the
    // white run continued to the left of the start point
    while (j < maxJ && !image.get(j, i)) {
      j++;
    }
    int currentState = 0;
    while (j < maxJ) {
      if (image.get(j, i)) {
        // Black pixel
        if (currentState == 1) { // Counting black pixels
          stateCount[1]++;
        } else {                                 // Counting white pixels
          if (currentState == 2) {               // A winner?
            if (foundPatternCross(stateCount)) { // Yes
              std::unique_ptr<AlignmentPattern> confirmed =
                handlePossibleCenter(stateCount, i, j);
              if (confirmed.get()) {
                return std::move(confirmed);
              }
            }
            stateCount[0] = stateCount[2];
            stateCount[1] = 1;
            stateCount[2] = 0;
            currentState = 1;
          } else {
            stateCount[++currentState]++;
          }
        }
      } else {                   // White pixel
        if (currentState == 1) { // Counting black pixels
          currentState++;
        }
        stateCount[currentState]++;
      }
      j++;
    }
    if (foundPatternCross(stateCount)) {
      std::unique_ptr<AlignmentPattern> confirmed =
        handlePossibleCenter(stateCount, i, maxJ);
      if (confirmed.get()) {
        return std::move(confirmed);
      }
    }
  }

  // Hmm, nothing we saw was observed and confirmed twice. If we had
  // any guess at all, return it.
  if (!possibleCenters.empty()) {
    auto tmp = std::move(possibleCenters);

    return std::move(tmp.at(0));
  }

  throw 1;
}

float
AlignmentPatternFinder::centerFromEnd(int* stateCount, int end)
{
  return (float)(end - stateCount[2]) - stateCount[1] / 2.0f;
}

bool
AlignmentPatternFinder::foundPatternCross(int* stateCount)
{
  float moduleSize = this->moduleSize;
  float maxVariance = moduleSize / 2.0f;
  for (int i = 0; i < 3; i++) {
    if (std::abs(moduleSize - stateCount[i]) >= maxVariance) {
      return false;
    }
  }
  return true;
}

float
AlignmentPatternFinder::crossCheckVertical(int startI, int centerJ,
                                           int maxCount,
                                           int originalStateCountTotal)
{
  const LuminanceImage& image = *this->image;
  int maxI = image.getHeight();
  int stateCount[3];
  memset(stateCount, 0, sizeof(stateCount));

  // Start counting up from center
  int i = startI;
  while (i >= 0 && image.get(centerJ, i) && stateCount[1] <= maxCount) {
    stateCount[1]++;
    i--;
  }
  // If already too many modules in this state or ran off the edge:
  if (i < 0 || stateCount[1] > maxCount) {
    return NAN;
  }
  while (i >= 0 && !image.get(centerJ, i) && stateCount[0] <= maxCount) {
    stateCount[0]++;
    i--;
  }
  if (stateCount[0] > maxCount) {
    return NAN;
  }

  // Now also count down from center
  i = startI + 1;
  while (i < maxI && image.get(centerJ, i) && stateCount[1] <= maxCount) {
    stateCount[1]++;
    i++;
  }
  if (i == maxI || stateCount[1] > maxCount) {
    return NAN;
  }
  while (i < maxI && !image.get(centerJ, i) && stateCount[2] <= maxCount) {
    stateCount[2]++;
    i++;
  }
  if (stateCount[2] > maxCount) {
    return NAN;
  }

  int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2];
  if (5 * std::abs(stateCountTotal - originalStateCountTotal) >=
      2 * originalStateCountTotal) {
    return NAN;
  }

  return foundPatternCross(stateCount) ? centerFromEnd(stateCount, i) : NAN;
}

std::unique_ptr<AlignmentPattern>
AlignmentPatternFinder::handlePossibleCenter(int* stateCount, int i, int j)
{
  int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2];
  float centerJ = centerFromEnd(stateCount, j);
  float centerI =
    crossCheckVertical(i, (int)centerJ, 2 * stateCount[1], stateCountTotal);
  if (!isnanf(centerI)) {
    float estimatedModuleSize =
      (float)(stateCount[0] + stateCount[1] + stateCount[2]) / 3.0f;
    for (auto& center : possibleCenters) {
      // Look for about the same center and module size:
      if (center->aboutEquals(estimatedModuleSize, centerI, centerJ)) {
        return center->combineEstimate(centerI, centerJ, estimatedModuleSize);
      }
    }
    // Hadn't found this before; save it
    std::unique_ptr<AlignmentPattern> point(
      new AlignmentPattern(centerJ, centerI, estimatedModuleSize));
    possibleCenters.push_back(std::move(point));
  }
  return nullptr;
}
