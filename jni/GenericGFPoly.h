#ifndef GENERICGFPOLY_H
#define GENERICGFPOLY_H
#include <memory>
#include <vector>

class GenericGF;
/**
 * <p>Represents a polynomial whose coefficients are elements of a GF.
 * Instances of this class are immutable.</p>
 *
 * <p>Much credit is due to William Rucklidge since portions of this code are an
 * indirect
 * port of his C++ Reed-Solomon implementation.</p>
 *
 * @author Sean Owen
 */
class GenericGFPoly final : public std::enable_shared_from_this<GenericGFPoly>
{
public:
  /**
   * @param field the {@link GenericGF} instance representing the field to use
   * to perform computations
   * @param coefficients coefficients as ints representing elements of GF(size),
   * arranged
   * from most significant (highest-power term) coefficient to least significant
   * @throws IllegalArgumentException if argument is null or empty,
   * or if leading coefficient is 0 and this is not a
   * constant polynomial (that is, it is not the monomial "0")
   */
  GenericGFPoly(const GenericGF* _field, std::vector<int>&& _coefficients);

  inline const std::vector<int>& getCoefficients() const
  {
    return coefficients;
  }

  /**
   * @return degree of this polynomial
   */
  inline int getDegree() const { return coefficients.size() - 1; }

  /**
   * @return true iff this polynomial is the monomial "0"
   */
  bool isZero() const { return coefficients[0] == 0; }

  /**
   * @return coefficient of x^degree term in this polynomial
   */
  int getCoefficient(int degree) const
  {
    return coefficients[coefficients.size() - 1 - degree];
  }

  /**
   * @return evaluation of this polynomial at a given point
   */
  int evaluateAt(int a) const;

  std::shared_ptr<GenericGFPoly> addOrSubtract(
    std::shared_ptr<GenericGFPoly> other);

  std::shared_ptr<GenericGFPoly> multiply(std::shared_ptr<GenericGFPoly> other);

  std::shared_ptr<GenericGFPoly> multiply(int scalar);

  std::shared_ptr<GenericGFPoly> multiplyByMonomial(int degree,
                                                    int coefficient);
  struct DivideResult
  {
    std::shared_ptr<GenericGFPoly> first, second;
  };
  DivideResult divide(std::shared_ptr<GenericGFPoly>& other);

private:
  const GenericGF* field;
  std::vector<int> coefficients;
};
#endif /* GENERICGFPOLY_H */
