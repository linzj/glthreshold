#include "QRCodeDetector.h"
#include "FinderPattern.h"
#include <algorithm>
#include <cmath>
#include <string.h>
static const int MIN_SKIP = 3; // 1 pixel/module times 3 modules/center
static const int CENTER_QUORUM = 2;
static const int MAX_MODULES =
  57; // support up to version 10 for mobile clients

bool
QRCodeDetector::Image::get(int x, int y) const
{
  return !m_data[y * m_width + x];
}

static bool
foundPatternCross(int stateCount[5])
{
  int totalModuleSize = 0;
  for (int i = 0; i < 5; i++) {
    int count = stateCount[i];
    if (count == 0) {
      return false;
    }
    totalModuleSize += count;
  }
  if (totalModuleSize < 7) {
    return false;
  }
  float moduleSize = totalModuleSize / 7.0f;
  float maxVariance = moduleSize / 2.0f;
  // Allow less than 50% variance from 1-1-3-1-1 proportions
  return std::abs(moduleSize - stateCount[0]) < maxVariance &&
         std::abs(moduleSize - stateCount[1]) < maxVariance &&
         std::abs(3.0f * moduleSize - stateCount[2]) < 3 * maxVariance &&
         std::abs(moduleSize - stateCount[3]) < maxVariance &&
         std::abs(moduleSize - stateCount[4]) < maxVariance;
}

static float
centerFromEnd(int stateCount[5], int end)
{
  return (float)(end - stateCount[4] - stateCount[3]) - stateCount[2] / 2.0f;
}

float
QRCodeDetector::crossCheckVertical(int startI, int centerJ, int maxCount,
                                   int originalStateCountTotal)
{
  const Image& image = m_image;
  int maxI = image.m_height;
  int stateCount[5];
  memset(stateCount, 0, sizeof(stateCount));

  // Start counting up from center
  int i = startI;
  while (i >= 0 && image.get(centerJ, i)) {
    stateCount[2]++;
    i--;
  }
  if (i < 0) {
    return NAN;
  }
  while (i >= 0 && !image.get(centerJ, i) && stateCount[1] <= maxCount) {
    stateCount[1]++;
    i--;
  }
  // If already too many modules in this state or ran off the edge:
  if (i < 0 || stateCount[1] > maxCount) {
    return NAN;
  }
  while (i >= 0 && image.get(centerJ, i) && stateCount[0] <= maxCount) {
    stateCount[0]++;
    i--;
  }
  if (stateCount[0] > maxCount) {
    return NAN;
  }

  // Now also count down from center
  i = startI + 1;
  while (i < maxI && image.get(centerJ, i)) {
    stateCount[2]++;
    i++;
  }
  if (i == maxI) {
    return NAN;
  }
  while (i < maxI && !image.get(centerJ, i) && stateCount[3] < maxCount) {
    stateCount[3]++;
    i++;
  }
  if (i == maxI || stateCount[3] >= maxCount) {
    return NAN;
  }
  while (i < maxI && image.get(centerJ, i) && stateCount[4] < maxCount) {
    stateCount[4]++;
    i++;
  }
  if (stateCount[4] >= maxCount) {
    return NAN;
  }

  // If we found a finder-pattern-like section, but its size is more than 40%
  // different than
  // the original, assume it's a false positive
  int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] +
                        stateCount[3] + stateCount[4];
  if (5 * std::abs(stateCountTotal - originalStateCountTotal) >=
      2 * originalStateCountTotal) {
    return NAN;
  }

  return foundPatternCross(stateCount) ? centerFromEnd(stateCount, i) : NAN;
}

float
QRCodeDetector::crossCheckHorizontal(int startJ, int centerI, int maxCount,
                                     int originalStateCountTotal)
{
  const Image& image = m_image;

  int maxJ = image.m_width;
  int stateCount[5];
  memset(stateCount, 0, sizeof(stateCount));

  int j = startJ;
  while (j >= 0 && image.get(j, centerI)) {
    stateCount[2]++;
    j--;
  }
  if (j < 0) {
    return NAN;
  }
  while (j >= 0 && !image.get(j, centerI) && stateCount[1] <= maxCount) {
    stateCount[1]++;
    j--;
  }
  if (j < 0 || stateCount[1] > maxCount) {
    return NAN;
  }
  while (j >= 0 && image.get(j, centerI) && stateCount[0] <= maxCount) {
    stateCount[0]++;
    j--;
  }
  if (stateCount[0] > maxCount) {
    return NAN;
  }

  j = startJ + 1;
  while (j < maxJ && image.get(j, centerI)) {
    stateCount[2]++;
    j++;
  }
  if (j == maxJ) {
    return NAN;
  }
  while (j < maxJ && !image.get(j, centerI) && stateCount[3] < maxCount) {
    stateCount[3]++;
    j++;
  }
  if (j == maxJ || stateCount[3] >= maxCount) {
    return NAN;
  }
  while (j < maxJ && image.get(j, centerI) && stateCount[4] < maxCount) {
    stateCount[4]++;
    j++;
  }
  if (stateCount[4] >= maxCount) {
    return NAN;
  }

  // If we found a finder-pattern-like section, but its size is significantly
  // different than
  // the original, assume it's a false positive
  int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] +
                        stateCount[3] + stateCount[4];
  if (5 * std::abs(stateCountTotal - originalStateCountTotal) >=
      originalStateCountTotal) {
    return NAN;
  }

  return foundPatternCross(stateCount) ? centerFromEnd(stateCount, j) : NAN;
}

bool
QRCodeDetector::crossCheckDiagonal(int startI, int centerJ, int maxCount,
                                   int originalStateCountTotal)
{
  const Image& image = m_image;
  int stateCount[5];
  memset(stateCount, 0, sizeof(stateCount));

  // Start counting up, left from center finding black center mass
  int i = 0;
  while (startI >= i && centerJ >= i && image.get(centerJ - i, startI - i)) {
    stateCount[2]++;
    i++;
  }

  if (startI < i || centerJ < i) {
    return false;
  }

  // Continue up, left finding white space
  while (startI >= i && centerJ >= i && !image.get(centerJ - i, startI - i) &&
         stateCount[1] <= maxCount) {
    stateCount[1]++;
    i++;
  }

  // If already too many modules in this state or ran off the edge:
  if (startI < i || centerJ < i || stateCount[1] > maxCount) {
    return false;
  }

  // Continue up, left finding black border
  while (startI >= i && centerJ >= i && image.get(centerJ - i, startI - i) &&
         stateCount[0] <= maxCount) {
    stateCount[0]++;
    i++;
  }
  if (stateCount[0] > maxCount) {
    return false;
  }

  int maxI = image.m_height;
  int maxJ = image.m_width;

  // Now also count down, right from center
  i = 1;
  while (startI + i < maxI && centerJ + i < maxJ &&
         image.get(centerJ + i, startI + i)) {
    stateCount[2]++;
    i++;
  }

  // Ran off the edge?
  if (startI + i >= maxI || centerJ + i >= maxJ) {
    return false;
  }

  while (startI + i < maxI && centerJ + i < maxJ &&
         !image.get(centerJ + i, startI + i) && stateCount[3] < maxCount) {
    stateCount[3]++;
    i++;
  }

  if (startI + i >= maxI || centerJ + i >= maxJ || stateCount[3] >= maxCount) {
    return false;
  }

  while (startI + i < maxI && centerJ + i < maxJ &&
         image.get(centerJ + i, startI + i) && stateCount[4] < maxCount) {
    stateCount[4]++;
    i++;
  }

  if (stateCount[4] >= maxCount) {
    return false;
  }

  // If we found a finder-pattern-like section, but its size is more than 100%
  // different than
  // the original, assume it's a false positive
  int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] +
                        stateCount[3] + stateCount[4];
  return std::abs(stateCountTotal - originalStateCountTotal) <
           2 * originalStateCountTotal &&
         foundPatternCross(stateCount);
}

bool
QRCodeDetector::handlePossibleCenter(int stateCount[5], int i, int j,
                                     bool pureBarcode)
{
  int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] +
                        stateCount[3] + stateCount[4];
  float centerJ = centerFromEnd(stateCount, j);
  float centerI =
    crossCheckVertical(i, (int)centerJ, stateCount[2], stateCountTotal);
  if (!isnanf(centerI)) {
    // Re-cross check
    centerJ = crossCheckHorizontal((int)centerJ, (int)centerI, stateCount[2],
                                   stateCountTotal);
    if (!isnanf(centerJ) &&
        (!pureBarcode || crossCheckDiagonal((int)centerI, (int)centerJ,
                                            stateCount[2], stateCountTotal))) {
      float estimatedModuleSize = (float)stateCountTotal / 7.0f;
      bool found = false;
      for (int index = 0; index < m_possibleCenters.size(); index++) {
        auto& center = m_possibleCenters.at(index);
        // Look for about the same center and module size:
        if (center->aboutEquals(estimatedModuleSize, centerI, centerJ)) {
          center = std::move(
            center->combineEstimate(centerI, centerJ, estimatedModuleSize));
          found = true;
          break;
        }
      }
      if (!found) {
        FinderPatternPtr point(
          new FinderPattern(centerJ, centerI, estimatedModuleSize));
        m_possibleCenters.push_back(std::move(point));
      }
      return true;
    }
  }
  return false;
}

FinderPatternInfo
QRCodeDetector::detect(int width, int height, const uint8_t* data)
{
  bool tryHarder = false;
  bool pureBarcode = true;
  m_image.m_width = width;
  m_image.m_height = height;
  m_image.m_data = data;
  hasSkipped = false;
  int maxI = height;
  int maxJ = width;
  // We are looking for black/white/black/white/black modules in
  // 1:1:3:1:1 ratio; this tracks the number of such modules seen so far

  // Let's assume that the maximum version QR Code we support takes up 1/4 the
  // height of the
  // image, and then account for the center being 3 modules in size. This gives
  // the smallest
  // number of pixels the center could be, so skip this often. When trying
  // harder, look for all
  // QR versions regardless of how dense they are.
  int iSkip = (3 * maxI) / (4 * MAX_MODULES);
  if (iSkip < MIN_SKIP || tryHarder) {
    iSkip = MIN_SKIP;
  }

  bool done = false;
  int stateCount[5];
  const Image& image = m_image;
  for (int i = iSkip - 1; i < maxI && !done; i += iSkip) {
    // Get a row of black/white values
    memset(stateCount, 0, sizeof(stateCount));
    int currentState = 0;
    for (int j = 0; j < maxJ; j++) {
      if (image.get(j, i)) {
        // Black pixel
        if ((currentState & 1) == 1) { // Counting white pixels
          currentState++;
        }
        stateCount[currentState]++;
      } else {                                   // White pixel
        if ((currentState & 1) == 0) {           // Counting black pixels
          if (currentState == 4) {               // A winner?
            if (foundPatternCross(stateCount)) { // Yes
              bool confirmed =
                handlePossibleCenter(stateCount, i, j, pureBarcode);
              if (confirmed) {
                // Start examining every other line. Checking each line turned
                // out to be too
                // expensive and didn't improve performance.
                iSkip = 2;
                if (hasSkipped) {
                  done = haveMultiplyConfirmedCenters();
                } else {
                  int rowSkip = findRowSkip();
                  if (rowSkip > stateCount[2]) {
                    // Skip rows between row of lower confirmed center
                    // and top of presumed third confirmed center
                    // but back up a bit to get a full chance of detecting
                    // it, entire width of center of finder pattern

                    // Skip by rowSkip, but back off by stateCount[2] (size of
                    // last center
                    // of pattern we saw) to be conservative, and also back off
                    // by iSkip which
                    // is about to be re-added
                    i += rowSkip - stateCount[2] - iSkip;
                    j = maxJ - 1;
                  }
                }
              } else {
                stateCount[0] = stateCount[2];
                stateCount[1] = stateCount[3];
                stateCount[2] = stateCount[4];
                stateCount[3] = 1;
                stateCount[4] = 0;
                currentState = 3;
                continue;
              }
              // Clear state to start looking again
              currentState = 0;
              stateCount[0] = 0;
              stateCount[1] = 0;
              stateCount[2] = 0;
              stateCount[3] = 0;
              stateCount[4] = 0;
            } else { // No, shift counts back by two
              stateCount[0] = stateCount[2];
              stateCount[1] = stateCount[3];
              stateCount[2] = stateCount[4];
              stateCount[3] = 1;
              stateCount[4] = 0;
              currentState = 3;
            }
          } else {
            stateCount[++currentState]++;
          }
        } else { // Counting white pixels
          stateCount[currentState]++;
        }
      }
    }
    if (foundPatternCross(stateCount)) {
      bool confirmed = handlePossibleCenter(stateCount, i, maxJ, pureBarcode);
      if (confirmed) {
        iSkip = stateCount[0];
        if (hasSkipped) {
          // Found a third one
          done = haveMultiplyConfirmedCenters();
        }
      }
    }
  }

  std::unique_ptr<FinderPattern[]> patternInfo = selectBestPatterns();
  ResultPoint::orderBestPatterns(patternInfo.get());

  return FinderPatternInfo(std::move(patternInfo));
}

bool
QRCodeDetector::haveMultiplyConfirmedCenters()
{
  int confirmedCount = 0;
  float totalModuleSize = 0.0f;
  int max = m_possibleCenters.size();
  for (auto& pattern : m_possibleCenters) {
    if (pattern->getCount() >= CENTER_QUORUM) {
      confirmedCount++;
      totalModuleSize += pattern->getEstimatedModuleSize();
    }
  }
  if (confirmedCount < 3) {
    return false;
  }
  // OK, we have at least 3 confirmed centers, but, it's possible that one is a
  // "false positive"
  // and that we need to keep looking. We detect this by asking if the estimated
  // module sizes
  // vary too much. We arbitrarily say that when the total deviation from
  // average exceeds
  // 5% of the total module size estimates, it's too much.
  float average = totalModuleSize / (float)max;
  float totalDeviation = 0.0f;
  for (auto& pattern : m_possibleCenters) {
    totalDeviation += std::abs(pattern->getEstimatedModuleSize() - average);
  }
  return totalDeviation <= 0.05f * totalModuleSize;
}

std::unique_ptr<FinderPattern[]>
QRCodeDetector::selectBestPatterns()
{

  int startSize = m_possibleCenters.size();
  if (startSize < 3) {
    // Couldn't find enough finder patterns
    return std::unique_ptr<FinderPattern[]>();
  }

  // Filter outlier possibilities whose module size is too different
  if (startSize > 3) {
    // But we can only afford to do so if we have at least 4 possibilities to
    // choose from
    float totalModuleSize = 0.0f;
    float square = 0.0f;
    for (auto& center : m_possibleCenters) {
      float size = center->getEstimatedModuleSize();
      totalModuleSize += size;
      square += size * size;
    }
    float average = totalModuleSize / (float)startSize;
    float stdDev = sqrtf(square / startSize - average * average);

    std::stable_sort(
      m_possibleCenters.begin(), m_possibleCenters.end(),
      [=](const FinderPatternPtr& center1, const FinderPatternPtr& center2) {
        float dA = std::abs(center2->getEstimatedModuleSize() - average);
        float dB = std::abs(center1->getEstimatedModuleSize() - average);
        return dA < dB;
      });

    float limit = std::max(0.2f * average, stdDev);

    for (int i = 0;
         i < m_possibleCenters.size() && m_possibleCenters.size() > 3; i++) {
      auto& pattern = m_possibleCenters.at(i);
      if (std::abs(pattern->getEstimatedModuleSize() - average) > limit) {
        m_possibleCenters.erase(m_possibleCenters.begin() + i);
        i--;
      }
    }
  }

  if (m_possibleCenters.size() > 3) {
    // Throw away all but those first size candidate points we found.

    float totalModuleSize = 0.0f;
    for (auto& possibleCenter : m_possibleCenters) {
      totalModuleSize += possibleCenter->getEstimatedModuleSize();
    }

    float average = totalModuleSize / (float)m_possibleCenters.size();

    std::stable_sort(
      m_possibleCenters.begin(), m_possibleCenters.end(),
      [=](const FinderPatternPtr& center1, const FinderPatternPtr& center2) {
        if (center2->getCount() == center1->getCount()) {
          float dA = std::abs(center2->getEstimatedModuleSize() - average);
          float dB = std::abs(center1->getEstimatedModuleSize() - average);
          return dA > dB;
        } else {
          return center2->getCount() - center1->getCount() < 0;
        }
      });

    m_possibleCenters.resize(3);
  }
  FinderPatterVec tmp(std::move(m_possibleCenters));

  return std::unique_ptr<FinderPattern[]>(
    new FinderPattern[3]{ *m_possibleCenters.at(0), *m_possibleCenters.at(1),
                          *m_possibleCenters.at(2) });
}

int
QRCodeDetector::findRowSkip()
{
  int max = m_possibleCenters.size();
  if (max <= 1) {
    return 0;
  }
  ResultPoint* firstConfirmedCenter = nullptr;
  for (auto& center : m_possibleCenters) {
    if (center->getCount() >= CENTER_QUORUM) {
      if (firstConfirmedCenter == nullptr) {
        firstConfirmedCenter = center.get();
      } else {
        // We have two confirmed centers
        // How far down can we skip before resuming looking for the next
        // pattern? In the worst case, only the difference between the
        // difference in the x / y coordinates of the two centers.
        // This is the case where you find top left last.
        hasSkipped = true;
        return (int)(std::abs(firstConfirmedCenter->getX() - center->getX()) -
                     std::abs(firstConfirmedCenter->getY() - center->getY())) /
               2;
      }
    }
  }
  return 0;
}
