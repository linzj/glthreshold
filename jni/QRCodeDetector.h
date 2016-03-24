#ifndef QRCODEDETECTOR_H
#define QRCODEDETECTOR_H
#include "FinderPatternInfo.h"
#include <memory>
#include <vector>

class FinderPattern;
class QRCodeDetector
{
public:
  FinderPatternInfo detect(int width, int height, const uint8_t* data);

private:
  float crossCheckVertical(int startI, int centerJ, int maxCount,
                           int originalStateCountTotal);
  float crossCheckHorizontal(int startJ, int centerI, int maxCount,
                             int originalStateCountTotal);
  bool crossCheckDiagonal(int startI, int centerJ, int maxCount,
                          int originalStateCountTotal);
  bool handlePossibleCenter(int stateCount[5], int i, int j, bool pureBarcode);
  bool haveMultiplyConfirmedCenters();
  std::unique_ptr<FinderPattern[]> selectBestPatterns();
  int findRowSkip();

  class Image
  {
  public:
    int m_width;
    int m_height;
    const uint8_t* m_data;
    bool get(int x, int y) const;
  };
  Image m_image;
  typedef std::unique_ptr<FinderPattern> FinderPatternPtr;
  typedef std::vector<FinderPatternPtr> FinderPatterVec;
  FinderPatterVec m_possibleCenters;
  bool hasSkipped;
};

#endif /* QRCODEDETECTOR_H */
