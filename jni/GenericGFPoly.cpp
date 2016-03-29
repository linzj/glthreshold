#include "GenericGFPoly.h"
#include "GenericGF.h"
#include <algorithm>

GenericGFPoly::GenericGFPoly(const GenericGF* _field,
                             std::vector<int>&& _coefficients)
  : field(_field)
{
  if (_coefficients.size() == 0) {
    throw 1;
  }
  int coefficientsLength = _coefficients.size();
  if (coefficientsLength > 1 && _coefficients[0] == 0) {
    // Leading term must be non-zero for anything except the constant polynomial
    // "0"
    int firstNonZero = 1;
    while (firstNonZero < coefficientsLength &&
           _coefficients[firstNonZero] == 0) {
      firstNonZero++;
    }
    if (firstNonZero == coefficientsLength) {
      coefficients.resize(1, 0);
    } else {
      coefficients.reserve(coefficientsLength - firstNonZero);
      std::copy(_coefficients.begin() + firstNonZero, _coefficients.end(),
                std::back_inserter(coefficients));
    }
  } else {
    this->coefficients = std::move(coefficients);
  }
}

int
GenericGFPoly::evaluateAt(int a) const
{
  if (a == 0) {
    // Just return the x^0 coefficient
    return getCoefficient(0);
  }
  int size = coefficients.size();
  if (a == 1) {
    // Just the sum of the coefficients
    int result = 0;
    for (int coefficient : coefficients) {
      result = GenericGF::addOrSubtract(result, coefficient);
    }
    return result;
  }
  int result = coefficients[0];
  for (int i = 1; i < size; i++) {
    result =
      GenericGF::addOrSubtract(field->multiply(a, result), coefficients[i]);
  }
  return result;
}

std::shared_ptr<GenericGFPoly>
GenericGFPoly::addOrSubtract(std::shared_ptr<GenericGFPoly> other)
{
  if (field != other->field) {
    throw 1;
  }
  if (isZero()) {
    return other;
  }
  if (other->isZero()) {
    return shared_from_this();
  }

  const std::vector<int>* smallerCoefficients = &this->coefficients;
  const std::vector<int>* largerCoefficients = &other->coefficients;
  if (smallerCoefficients->size() > largerCoefficients->size()) {
    const std::vector<int>* temp = smallerCoefficients;
    smallerCoefficients = largerCoefficients;
    largerCoefficients = temp;
  }
  std::vector<int> sumDiff;
  sumDiff.reserve(largerCoefficients->size());
  int lengthDiff = largerCoefficients->size() - smallerCoefficients->size();
  // Copy high-order terms only found in higher-degree polynomial's coefficients
  std::copy(largerCoefficients->begin(), largerCoefficients->end(),
            std::back_inserter(sumDiff));

  for (int i = lengthDiff; i < largerCoefficients->size(); i++) {
    sumDiff[i] = GenericGF::addOrSubtract(
      (*smallerCoefficients)[i - lengthDiff], (*largerCoefficients)[i]);
  }

  return std::shared_ptr<GenericGFPoly>(
    new GenericGFPoly(field, std::move(sumDiff)));
}

std::shared_ptr<GenericGFPoly>
GenericGFPoly::multiply(std::shared_ptr<GenericGFPoly> other)
{
  if (field != other->field) {
    throw 1;
  }
  if (isZero() || other->isZero()) {
    return field->getZero();
  }
  const std::vector<int>& aCoefficients = this->coefficients;
  int aLength = aCoefficients.size();
  const std::vector<int>& bCoefficients = other->coefficients;
  int bLength = bCoefficients.size();
  std::vector<int> product(aLength + bLength - 1);
  for (int i = 0; i < aLength; i++) {
    int aCoeff = aCoefficients[i];
    for (int j = 0; j < bLength; j++) {
      product[i + j] = GenericGF::addOrSubtract(
        product[i + j], field->multiply(aCoeff, bCoefficients[j]));
    }
  }
  return std::shared_ptr<GenericGFPoly>(
    new GenericGFPoly(field, std::move(product)));
}

std::shared_ptr<GenericGFPoly>
GenericGFPoly::multiply(int scalar)
{
  if (scalar == 0) {
    return field->getZero();
  }
  if (scalar == 1) {
    return shared_from_this();
  }
  int size = coefficients.size();
  std::vector<int> product(size);
  for (int i = 0; i < size; i++) {
    product[i] = field->multiply(coefficients[i], scalar);
  }
  return std::shared_ptr<GenericGFPoly>(
    new GenericGFPoly(field, std::move(product)));
}

std::shared_ptr<GenericGFPoly>
GenericGFPoly::multiplyByMonomial(int degree, int coefficient)
{
  if (degree < 0) {
    throw 1;
  }
  if (coefficient == 0) {
    return field->getZero();
  }
  int size = coefficients.size();
  std::vector<int> product(size + degree);
  for (int i = 0; i < size; i++) {
    product[i] = field->multiply(coefficients[i], coefficient);
  }
  return std::shared_ptr<GenericGFPoly>(
    new GenericGFPoly(field, std::move(product)));
}

GenericGFPoly::DivideResult
GenericGFPoly::divide(std::shared_ptr<GenericGFPoly>& other)
{
  if (field != other->field) {
    throw 1;
  }
  if (other->isZero()) {
    throw 1;
  }

  std::shared_ptr<GenericGFPoly> quotient = field->getZero();
  std::shared_ptr<GenericGFPoly> remainder = shared_from_this();

  int denominatorLeadingTerm = other->getCoefficient(other->getDegree());
  int inverseDenominatorLeadingTerm = field->inverse(denominatorLeadingTerm);

  while (remainder->getDegree() >= other->getDegree() && !remainder->isZero()) {
    int degreeDifference = remainder->getDegree() - other->getDegree();
    int scale =
      field->multiply(remainder->getCoefficient(remainder->getDegree()),
                      inverseDenominatorLeadingTerm);
    auto term = other->multiplyByMonomial(degreeDifference, scale);
    auto iterationQuotient = field->buildMonomial(degreeDifference, scale);
    quotient = quotient->addOrSubtract(iterationQuotient);
    remainder = remainder->addOrSubtract(term);
  }

  return DivideResult{ quotient, remainder };
}
