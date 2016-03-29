#ifndef GENERICGF_H
#define GENERICGF_H
#include <memory>

class GenericGFPoly;
class GenericGF final
{

public:
  static const GenericGF AZTEC_DATA_12;         // x^12 + x^6 + x^5 + x^3 + 1
  static const GenericGF AZTEC_DATA_10;         // x^10 + x^3 + 1
  static const GenericGF AZTEC_DATA_6;          // x^6 + x + 1
  static const GenericGF AZTEC_PARAM;           // x^4 + x + 1
  static const GenericGF QR_CODE_FIELD_256;     // x^8 + x^4 + x^3 + x^2 + 1
  static const GenericGF DATA_MATRIX_FIELD_256; // x^8 + x^5 + x^3 + x^2 + 1
  static const GenericGF AZTEC_DATA_8;
  static const GenericGF MAXICODE_FIELD_64;

private:
  std::unique_ptr<int[]> expTable;
  std::unique_ptr<int[]> logTable;
  mutable std::shared_ptr<GenericGFPoly> zero;
  mutable std::shared_ptr<GenericGFPoly> one;
  int size;
  int primitive;
  int generatorBase;

public:
  /**
   * Create a representation of GF(size) using the given primitive polynomial.
   *
   * @param primitive irreducible polynomial whose coefficients are represented
   * by
   *  the bits of an int, where the least-significant bit represents the
   * constant
   *  coefficient
   * @param size the size of the field
   * @param b the factor b in the generator polynomial can be 0- or 1-based
   *  (g(x) = (x+a^b)(x+a^(b+1))...(x+a^(b+2t-1))).
   *  In most cases it should be 1, but for QR code it is 0.
   */
  GenericGF(int primitive, int size, int b);

  std::shared_ptr<GenericGFPoly>& getZero() const { return zero; }

  std::shared_ptr<GenericGFPoly>& getOne() const { return one; }

  /**
   * @return the monomial representing coefficient * x^degree
   */
  std::shared_ptr<GenericGFPoly> buildMonomial(int degree,
                                               int coefficient) const;

  /**
   * Implements both addition and subtraction -- they are the same in GF(size).
   *
   * @return sum/difference of a and b
   */
  inline static int addOrSubtract(int a, int b) { return a ^ b; }

  /**
   * @return 2 to the power of a in GF(size)
   */
  inline int exp(int a) const { return expTable.get()[a]; }

  /**
   * @return base 2 log of a in GF(size)
   */
  int log(int a) const;

  /**
   * @return multiplicative inverse of a
   */
  int inverse(int a) const;

  /**
   * @return product of a and b in GF(size)
   */
  int multiply(int a, int b) const;

  inline int getSize() const { return size; }

  inline int getGeneratorBase() const { return generatorBase; }
};

#endif /* GENERICGF_H */
