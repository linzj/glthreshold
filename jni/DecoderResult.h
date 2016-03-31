#ifndef DECODERRESULT_H
#define DECODERRESULT_H
#include <string>
#include <vector>

class DecoderResult final
{
public:
  typedef std::vector<std::vector<uint8_t>> ByteSegments;
  DecoderResult(const std::vector<uint8_t>& rawBytes, const std::string& text,
                ByteSegments&& byteSegments, const std::string& ecLevel);

  DecoderResult(const std::vector<uint8_t>& rawBytes, const std::string& text,
                ByteSegments&& byteSegments, const std::string& ecLevel,
                int saSequence, int saParity);

  DecoderResult(const DecoderResult&) = delete;
  DecoderResult& operator=(const DecoderResult&) = delete;
  ~DecoderResult();

  inline const std::vector<uint8_t>& getRawBytes() const { return rawBytes; }

  inline const std::string& getText() const { return text; }

  const ByteSegments& getByteSegments() const { return byteSegments; }

  const std::string& getECLevel() const { return ecLevel; }

  int getErrorsCorrected() const { return errorsCorrected; }

  void setErrorsCorrected(int errorsCorrected)
  {
    this->errorsCorrected = errorsCorrected;
  }

  int getErasures() const { return erasures; }

  void setErasures(int erasures) { this->erasures = erasures; }

  void* getOther() { return other; }

  void setOther(void* other) { this->other = other; }

  bool hasStructuredAppend() const
  {
    return structuredAppendParity >= 0 && structuredAppendSequenceNumber >= 0;
  }

  int getStructuredAppendParity() const { return structuredAppendParity; }

  int getStructuredAppendSequenceNumber() const
  {
    return structuredAppendSequenceNumber;
  }

private:
  std::vector<uint8_t> rawBytes;
  std::string text;
  ByteSegments byteSegments;
  std::string ecLevel;
  int errorsCorrected;
  int erasures;
  void* other;
  int structuredAppendParity;
  int structuredAppendSequenceNumber;
};
#endif /* DECODERRESULT_H */
