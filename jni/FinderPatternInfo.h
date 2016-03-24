#ifndef FINDERPATTERNINFO_H
#define FINDERPATTERNINFO_H
#include <memory>

class FinderPattern;
class FinderPatternInfo
{

public:
  FinderPatternInfo(std::unique_ptr<FinderPattern[]>&& patternCenters);

  inline FinderPattern* getBottomLeft() { return bottomLeft; }

  inline FinderPattern* getTopLeft() { return topLeft; }

  inline FinderPattern* getTopRight() { return topRight; }

private:
  FinderPattern* bottomLeft;
  FinderPattern* topLeft;
  FinderPattern* topRight;
  std::unique_ptr<FinderPattern[]> m_storage;
};

#endif /* FINDERPATTERNINFO_H */
