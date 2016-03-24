#include "FinderPatternInfo.h"
#include "FinderPattern.h"

FinderPatternInfo::FinderPatternInfo(
  std::unique_ptr<FinderPattern[]>&& patternCenters)
  : m_storage(std::move(patternCenters))
  , bottomLeft(nullptr)
  , topLeft(nullptr)
  , topRight(nullptr)
{
  if (!m_storage.get()) {
    return;
  }
  this->bottomLeft = m_storage.get() + 0;
  this->topLeft = m_storage.get() + 1;
  this->topRight = m_storage.get() + 2;
}
