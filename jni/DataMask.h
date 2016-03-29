#ifndef DATAMASK_H
#define DATAMASK_H
#include <memory>
class LuminanceImage;

/**
 * <p>Encapsulates data masks for the data bits in a QR code, per ISO 18004:2006
 * 6.8. Implementations
 * of this class can un-mask a raw BitMatrix. For simplicity, they will unmask
 * the entire BitMatrix,
 * including areas used for finder patterns, timing patterns, etc. These areas
 * should be unused
 * after the point they are unmasked anyway.</p>
 *
 * <p>Note that the diagram in section 6.8.1 is misleading since it indicates
 * that i is column position
 * and j is row position. In fact, as the text says, i is row position and j is
 * column position.</p>
 *
 * @author Sean Owen
 */
class DataMask
{
public:
  /**
   * <p>Implementations of this method reverse the data masking process applied
   * to a QR Code and
   * make its bits ready to read.</p>
   *
   * @param bits representation of QR Code bits
   * @param dimension dimension of QR Code, represented by bits, being unmasked
   */
  void unmaskBitMatrix(LuminanceImage* bits, int dimension) const;

  virtual bool isMasked(int i, int j) const = 0;

  /**
   * @param reference a value between 0 and 7 indicating one of the eight
   * possible
   * data mask patterns a QR Code may use
   * @return DataMask encapsulating the data mask pattern
   */
  static const DataMask& forReference(int reference);

  static const DataMask& getStatic(int i);

protected:
  DataMask() = default;
};
#endif /* DATAMASK_H */
