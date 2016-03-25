#ifndef FINDERPATTERNINFO_H
#define FINDERPATTERNINFO_H
#include <memory>

class FinderPattern;
class FinderPatternInfo
{

public:
  FinderPatternInfo(std::unique_ptr<FinderPattern[]>&& patternCenters);

  inline const FinderPattern* getBottomLeft() const { return bottomLeft; }

  inline const FinderPattern* getTopLeft() const { return topLeft; }

  inline const FinderPattern* getTopRight() const { return topRight; }

private:
  FinderPattern* bottomLeft;
  FinderPattern* topLeft;
  FinderPattern* topRight;
  std::unique_ptr<FinderPattern[]> m_storage;
};

#endif /* FINDERPATTERNINFO_H */
