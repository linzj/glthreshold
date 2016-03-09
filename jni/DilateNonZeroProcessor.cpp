#include "DilateNoneZeroProcessor.h"

DilateNoneZeroProcessor::DilateNoneZeroProcessor()
  : m_vPositionIndexRowFirst(0)
  , m_uTextureRowFirst(0)
  , m_uScreenGeometryRowFirst(0)
  , m_uKWidthRowFirst(0)
  , m_programRowFirst(0)

  , m_vPositionIndexRowSecond(0)
  , m_uTextureRowSecond(0)
  , m_uScreenGeometryRowSecond(0)
  , m_uKWidthRowSecond(0)
  , m_programRowSecond(0)

  , m_vPositionIndexColumn(0)
  , m_uTextureColumn(0)
  , m_uScreenGeometryColumn(0)
  , m_uKHeightColumn(0)
  , m_programColumn(0)
  , m_kwidth(0)
  , m_kheight(0)
{
}

DilateNoneZeroProcessor::~DilateNoneZeroProcessor()
{
  if (m_programRowFirst) {
    glDeleteProgram(m_programRowFirst);
  }
  if (m_programRowSecond) {
    glDeleteProgram(m_programRowSecond);
  }
  if (m_programColumn) {
    glDeleteProgram(m_programColumn);
  }
}

bool
DilateNoneZeroProcessor::init(unsigned kwidth, unsigned kheight)
{
  m_kwidth = kwidth;
  m_kheight = kheight;
  return initProgram();
}

ProcessorOutput
DilateNoneZeroProcessor::process(const ProcessorInput& desc)
{
}

bool
DilateNoneZeroProcessor::initProgram()
{
}
