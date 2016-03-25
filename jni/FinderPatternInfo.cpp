#include "FinderPatternInfo.h"
#include "FinderPattern.h"

FinderPatternInfo::FinderPatternInfo(
  std::vector<std::unique_ptr<FinderPattern>>&& patternCenters)
  : m_storage(std::move(patternCenters))
  , bottomLeft(nullptr)
  , topLeft(nullptr)
  , topRight(nullptr)
{
  if (m_storage.empty()) {
    return;
  }
  this->bottomLeft = m_storage.at(0).get();
  this->topLeft = m_storage.at(1).get();
  this->topRight = m_storage.at(2).get();
}
