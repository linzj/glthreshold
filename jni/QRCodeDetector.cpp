#include "QRCodeDetector.h"
#include "AlignmentPatternFinder.h"
#include "DefaultGridSampler.h"
#include "FinderPattern.h"
#include "PerspectiveTransform.h"
#include "Version.h"
#include <algorithm>
#include <cmath>
#include <string.h>
static const int MIN_SKIP = 3; // 1 pixel/module times 3 modules/center
static const int CENTER_QUORUM = 2;
static const int MAX_MODULES =
  57; // support up to version 10 for mobile clients

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

std::unique_ptr<QRCodeDetector::DetectorResult>
QRCodeDetector::detect(int width, int height, const uint8_t* data)
{
  bool tryHarder = false;
  bool pureBarcode = false;
  image.reset(width, height, data);
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

  auto patternInfo = selectBestPatterns();
  if (patternInfo.empty()) {
    throw 1;
  }
  ResultPoint::orderBestPatterns(
    reinterpret_cast<const ResultPoint**>(patternInfo.data()));

  return processFinderPatternInfo(FinderPatternInfo(std::move(patternInfo)));
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

QRCodeDetector::FinderPatternVec
QRCodeDetector::selectBestPatterns()
{

  int startSize = m_possibleCenters.size();
  if (startSize < 3) {
    // Couldn't find enough finder patterns
    return FinderPatternVec();
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
  return std::move(m_possibleCenters);
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

float
QRCodeDetector::sizeOfBlackWhiteBlackRun(int fromX, int fromY, int toX, int toY)
{
  // Mild variant of Bresenham's algorithm;
  // see http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
  bool steep = std::abs(toY - fromY) > std::abs(toX - fromX);
  if (steep) {
    int temp = fromX;
    fromX = fromY;
    fromY = temp;
    temp = toX;
    toX = toY;
    toY = temp;
  }

  int dx = std::abs(toX - fromX);
  int dy = std::abs(toY - fromY);
  int error = -dx / 2;
  int xstep = fromX < toX ? 1 : -1;
  int ystep = fromY < toY ? 1 : -1;

  // In black pixels, looking for white, first or second time.
  int state = 0;
  // Loop up until x == toX, but not beyond
  int xLimit = toX + xstep;
  for (int x = fromX, y = fromY; x != xLimit; x += xstep) {
    int realX = steep ? y : x;
    int realY = steep ? x : y;

    // Does current pixel mean we have moved white to black or vice versa?
    // Scanning black in state 0,2 and white in state 1, so if we find the wrong
    // color, advance to next state or end if we are in state 2 already
    if ((state == 1) == image.get(realX, realY)) {
      if (state == 2) {
        return mdistance(x, y, fromX, fromY);
      }
      state++;
    }

    error += dy;
    if (error > 0) {
      if (y == toY) {
        break;
      }
      y += ystep;
      error -= dx;
    }
  }
  // Found black-white-black; give the benefit of the doubt that the next pixel
  // outside the image
  // is "white" so this last point at (toX+xStep,toY) is the right ending. This
  // is really a
  // small approximation; (toX+xStep,toY+yStep) might be really correct. Ignore
  // this.
  if (state == 2) {
    return mdistance(toX + xstep, toY, fromX, fromY);
  }
  // else we didn't find even black-white-black; no estimate is really possible
  return NAN;
}

float
QRCodeDetector::sizeOfBlackWhiteBlackRunBothWays(int fromX, int fromY, int toX,
                                                 int toY)
{
  float result = sizeOfBlackWhiteBlackRun(fromX, fromY, toX, toY);

  // Now count other way -- don't run off image though of course
  float scale = 1.0f;
  int otherToX = fromX - (toX - fromX);
  if (otherToX < 0) {
    scale = (float)fromX / (float)(fromX - otherToX);
    otherToX = 0;
  } else if (otherToX >= image.getWidth()) {
    scale = (float)(image.getWidth() - 1 - fromX) / (float)(otherToX - fromX);
    otherToX = image.getWidth() - 1;
  }
  int otherToY = (int)(fromY - (toY - fromY) * scale);

  scale = 1.0f;
  if (otherToY < 0) {
    scale = (float)fromY / (float)(fromY - otherToY);
    otherToY = 0;
  } else if (otherToY >= image.getHeight()) {
    scale = (float)(image.getHeight() - 1 - fromY) / (float)(otherToY - fromY);
    otherToY = image.getHeight() - 1;
  }
  otherToX = (int)(fromX + (otherToX - fromX) * scale);

  result += sizeOfBlackWhiteBlackRun(fromX, fromY, otherToX, otherToY);

  // Middle pixel is double-counted this way; subtract 1
  return result - 1.0f;
}

float
QRCodeDetector::calculateModuleSizeOneWay(const ResultPoint* pattern,
                                          const ResultPoint* otherPattern)
{
  float moduleSizeEst1 = sizeOfBlackWhiteBlackRunBothWays(
    (int)pattern->getX(), (int)pattern->getY(), (int)otherPattern->getX(),
    (int)otherPattern->getY());
  float moduleSizeEst2 = sizeOfBlackWhiteBlackRunBothWays(
    (int)otherPattern->getX(), (int)otherPattern->getY(), (int)pattern->getX(),
    (int)pattern->getY());
  if (isnanf(moduleSizeEst1)) {
    return moduleSizeEst2 / 7.0f;
  }
  if (isnanf(moduleSizeEst2)) {
    return moduleSizeEst1 / 7.0f;
  }
  // Average them, and divide by 7 since we've counted the width of 3 black
  // modules,
  // and 1 white and 1 black module on either side. Ergo, divide sum by 14.
  return (moduleSizeEst1 + moduleSizeEst2) / 14.0f;
}

float
QRCodeDetector::calculateModuleSize(const ResultPoint* topLeft,
                                    const ResultPoint* topRight,
                                    const ResultPoint* bottomLeft)
{
  // Take the average
  return (calculateModuleSizeOneWay(topLeft, topRight) +
          calculateModuleSizeOneWay(topLeft, bottomLeft)) /
         2.0f;
}

static int
computeDimension(const ResultPoint* topLeft, const ResultPoint* topRight,
                 const ResultPoint* bottomLeft, float moduleSize)
{
  int tltrCentersDimension =
    roundf(ResultPoint::distance(*topLeft, *topRight) / moduleSize);
  int tlblCentersDimension =
    roundf(ResultPoint::distance(*topLeft, *bottomLeft) / moduleSize);
  int dimension = ((tltrCentersDimension + tlblCentersDimension) / 2) + 7;
  switch (dimension & 0x03) { // mod 4
    case 0:
      dimension++;
      break;
    // 1? do nothing
    case 2:
      dimension--;
      break;
    case 3:
      throw 1;
  }
  return dimension;
}

std::unique_ptr<AlignmentPattern>
QRCodeDetector::findAlignmentInRegion(float overallEstModuleSize,
                                      int estAlignmentX, int estAlignmentY,
                                      float allowanceFactor)
{
  // Look for an alignment pattern (3 modules in size) around where it
  // should be
  int allowance = (int)(allowanceFactor * overallEstModuleSize);
  int alignmentAreaLeftX = std::max(0, estAlignmentX - allowance);
  int alignmentAreaRightX =
    std::min(image.getWidth() - 1, estAlignmentX + allowance);
  if (alignmentAreaRightX - alignmentAreaLeftX < overallEstModuleSize * 3) {
    throw 1;
  }

  int alignmentAreaTopY = std::max(0, estAlignmentY - allowance);
  int alignmentAreaBottomY =
    std::min(image.getHeight() - 1, estAlignmentY + allowance);
  if (alignmentAreaBottomY - alignmentAreaTopY < overallEstModuleSize * 3) {
    throw 1;
  }

  AlignmentPatternFinder alignmentFinder(
    &image, alignmentAreaLeftX, alignmentAreaTopY,
    alignmentAreaRightX - alignmentAreaLeftX,
    alignmentAreaBottomY - alignmentAreaTopY, overallEstModuleSize);
  return alignmentFinder.find();
}

static std::unique_ptr<PerspectiveTransform>
createTransform(const ResultPoint* topLeft, const ResultPoint* topRight,
                const ResultPoint* bottomLeft,
                const ResultPoint* alignmentPattern, int dimension)
{
  float dimMinusThree = (float)dimension - 3.5f;
  float bottomRightX;
  float bottomRightY;
  float sourceBottomRightX;
  float sourceBottomRightY;
  if (alignmentPattern) {
    bottomRightX = alignmentPattern->getX();
    bottomRightY = alignmentPattern->getY();
    sourceBottomRightX = dimMinusThree - 3.0f;
    sourceBottomRightY = sourceBottomRightX;
  } else {
    // Don't have an alignment pattern, just make up the bottom-right point
    bottomRightX = (topRight->getX() - topLeft->getX()) + bottomLeft->getX();
    bottomRightY = (topRight->getY() - topLeft->getY()) + bottomLeft->getY();
    sourceBottomRightX = dimMinusThree;
    sourceBottomRightY = dimMinusThree;
  }

  return PerspectiveTransform::quadrilateralToQuadrilateral(
    3.5f, 3.5f, dimMinusThree, 3.5f, sourceBottomRightX, sourceBottomRightY,
    3.5f, dimMinusThree, topLeft->getX(), topLeft->getY(), topRight->getX(),
    topRight->getY(), bottomRightX, bottomRightY, bottomLeft->getX(),
    bottomLeft->getY());
}

static std::unique_ptr<LuminanceImage>
sampleGrid(const LuminanceImage& image, PerspectiveTransform& transform,
           int dimension)
{

  DefaultGridSampler sampler;
  return std::move(sampler.sampleGrid(image, dimension, dimension, transform));
}

std::unique_ptr<QRCodeDetector::DetectorResult>
QRCodeDetector::processFinderPatternInfo(const FinderPatternInfo& info)
{
  const FinderPattern* topLeft = info.getTopLeft();
  const FinderPattern* topRight = info.getTopRight();
  const FinderPattern* bottomLeft = info.getBottomLeft();

  float moduleSize = calculateModuleSize(topLeft, topRight, bottomLeft);
  if (moduleSize < 1.0f) {
    throw 1;
  }
  int dimension = computeDimension(topLeft, topRight, bottomLeft, moduleSize);
  const Version& provisionalVersion =
    Version::getProvisionalVersionForDimension(dimension);
  int modulesBetweenFPCenters = provisionalVersion.getDimensionForVersion() - 7;

  std::unique_ptr<AlignmentPattern> alignmentPattern;
  // Anything above version 1 has an alignment pattern
  if (provisionalVersion.getAlignmentPatternCenters().size() > 0) {

    // Guess where a "bottom right" finder pattern would have been
    float bottomRightX =
      topRight->getX() - topLeft->getX() + bottomLeft->getX();
    float bottomRightY =
      topRight->getY() - topLeft->getY() + bottomLeft->getY();

    // Estimate that alignment pattern is closer by 3 modules
    // from "bottom right" to known top left location
    float correctionToTopLeft = 1.0f - 3.0f / (float)modulesBetweenFPCenters;
    int estAlignmentX =
      (int)(topLeft->getX() +
            correctionToTopLeft * (bottomRightX - topLeft->getX()));
    int estAlignmentY =
      (int)(topLeft->getY() +
            correctionToTopLeft * (bottomRightY - topLeft->getY()));

    // Kind of arbitrary -- expand search radius before giving up
    for (int i = 4; i <= 16; i <<= 1) {
      alignmentPattern = std::move(findAlignmentInRegion(
        moduleSize, estAlignmentX, estAlignmentY, (float)i));
    }
    // If we didn't find alignment pattern... well try anyway without it
  }

  std::unique_ptr<PerspectiveTransform> transform = createTransform(
    topLeft, topRight, bottomLeft, alignmentPattern.get(), dimension);

  std::unique_ptr<LuminanceImage> bits =
    sampleGrid(image, *transform, dimension);

  std::unique_ptr<ResultPoint[]> points;
  if (!alignmentPattern.get()) {
    points.reset(new ResultPoint[3]{ *bottomLeft, *topLeft, *topRight });
  } else {
    points.reset(new ResultPoint[4]{ *bottomLeft, *topLeft, *topRight,
                                     *alignmentPattern });
  }
  return std::unique_ptr<DetectorResult>(
    new DetectorResult(std::move(bits), std::move(points)));
}
