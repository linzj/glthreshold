#include "DecoderResult.h"

DecoderResult::DecoderResult(const std::vector<uint8_t>& _rawBytes,
                             const std::string& _text,
                             ByteSegments&& _byteSegments,
                             const std::string& _ecLevel)
  : DecoderResult(_rawBytes, _text, std::move(_byteSegments), _ecLevel, -1, -1)
{
}

DecoderResult::DecoderResult(const std::vector<uint8_t>& _rawBytes,
                             const std::string& _text,
                             ByteSegments&& _byteSegments,
                             const std::string& _ecLevel, int _saSequence,
                             int _saParity)
  : rawBytes(_rawBytes)
  , text(_text)
  , byteSegments(std::move(_byteSegments))
  , ecLevel(_ecLevel)
  , structuredAppendParity(_saParity)
  , structuredAppendSequenceNumber(_saSequence)
{
}

DecoderResult::~DecoderResult()
{
}
