#ifndef QRCODEDETECTOR_H
#define QRCODEDETECTOR_H
#include "FinderPatternInfo.h"
#include "LuminanceImage.h"
#include <memory>
#include <vector>

class FinderPattern;
class AlignmentPattern;
class ResultPoint;

class QRCodeDetector
{
public:
  class DetectorResult
  {
  public:
    DetectorResult(std::unique_ptr<LuminanceImage>&& bits,
                   std::unique_ptr<ResultPoint[]>&& points);
    ~DetectorResult();

    const LuminanceImage& getBits() const { return *bits; }
    LuminanceImage& getBits() { return *bits; }

    const ResultPoint* getPoints() const { return points.get(); }
  private:
    std::unique_ptr<LuminanceImage> bits;
    std::unique_ptr<ResultPoint[]> points;
  };
  std::unique_ptr<DetectorResult> detect(int width, int height,
                                         const uint8_t* data);
  QRCodeDetector(const QRCodeDetector&) = delete;
  QRCodeDetector& operator=(const QRCodeDetector&) = delete;
  QRCodeDetector();
  ~QRCodeDetector();

private:
  typedef std::unique_ptr<FinderPattern> FinderPatternPtr;
  typedef std::vector<FinderPatternPtr> FinderPatternVec;
  float crossCheckVertical(int startI, int centerJ, int maxCount,
                           int originalStateCountTotal);
  float crossCheckHorizontal(int startJ, int centerI, int maxCount,
                             int originalStateCountTotal);
  bool crossCheckDiagonal(int startI, int centerJ, int maxCount,
                          int originalStateCountTotal);
  bool handlePossibleCenter(int stateCount[5], int i, int j, bool pureBarcode);
  bool haveMultiplyConfirmedCenters();
  FinderPatternVec selectBestPatterns();
  int findRowSkip();
  float sizeOfBlackWhiteBlackRun(int fromX, int fromY, int toX, int toY);
  float sizeOfBlackWhiteBlackRunBothWays(int fromX, int fromY, int toX,
                                         int toY);
  float calculateModuleSizeOneWay(const ResultPoint* pattern,
                                  const ResultPoint* otherPattern);
  float calculateModuleSize(const ResultPoint* topLeft,
                            const ResultPoint* topRight,
                            const ResultPoint* bottomLeft);

  std::unique_ptr<AlignmentPattern> findAlignmentInRegion(
    float overallEstModuleSize, int estAlignmentX, int estAlignmentY,
    float allowanceFactor);

  std::unique_ptr<DetectorResult> processFinderPatternInfo(
    const FinderPatternInfo& info);

  LuminanceImage image;
  FinderPatternVec m_possibleCenters;
  bool hasSkipped;
};

#endif /* QRCODEDETECTOR_H */
