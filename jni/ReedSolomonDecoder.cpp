#include "ReedSolomonDecoder.h"
#include "GenericGF.h"
#include "GenericGFPoly.h"

ReedSolomonDecoder::ReedSolomonDecoder(const GenericGF* _field)
  : field(_field)
{
}

void
ReedSolomonDecoder::decode(std::vector<int>& received, int twoS)
{
  std::vector<int> copy(received);
  std::shared_ptr<GenericGFPoly> poly(
    new GenericGFPoly(field, std::move(copy)));
  std::vector<int> syndromeCoefficients(twoS);
  bool noError = true;
  for (int i = 0; i < twoS; i++) {
    int eval = poly->evaluateAt(field->exp(i + field->getGeneratorBase()));
    syndromeCoefficients[syndromeCoefficients.size() - 1 - i] = eval;
    if (eval != 0) {
      noError = false;
    }
  }
  if (noError) {
    return;
  }
  std::shared_ptr<GenericGFPoly> syndrome(
    new GenericGFPoly(field, std::move(syndromeCoefficients)));
  EuclideanResult sigmaOmega =
    runEuclideanAlgorithm(field->buildMonomial(twoS, 1), syndrome, twoS);
  auto sigma = sigmaOmega.first;
  auto omega = sigmaOmega.second;
  auto errorLocations(std::move(findErrorLocations(sigma)));
  auto errorMagnitudes(std::move(findErrorMagnitudes(omega, errorLocations)));
  for (int i = 0; i < errorLocations.size(); i++) {
    int position = received.size() - 1 - field->log(errorLocations[i]);
    if (position < 0) {
      throw 1;
    }
    received[position] =
      GenericGF::addOrSubtract(received[position], errorMagnitudes[i]);
  }
}

ReedSolomonDecoder::EuclideanResult
ReedSolomonDecoder::runEuclideanAlgorithm(std::shared_ptr<GenericGFPoly> a,
                                          std::shared_ptr<GenericGFPoly> b,
                                          int R)
{
  // Assume a's degree is >= b's
  if (a->getDegree() < b->getDegree()) {
    auto temp = a;
    a = b;
    b = temp;
  }

  auto rLast = a;
  auto r = b;
  auto tLast = field->getZero();
  auto t = field->getOne();

  // Run Euclidean algorithm until r's degree is less than R/2
  while (r->getDegree() >= R / 2) {
    auto rLastLast = rLast;
    auto tLastLast = tLast;
    rLast = r;
    tLast = t;

    // Divide rLastLast by rLast, with quotient in q and remainder in r
    if (rLast->isZero()) {
      // Oops, Euclidean algorithm already terminated?
      throw 1;
    }
    r = rLastLast;
    auto q = field->getZero();
    int denominatorLeadingTerm = rLast->getCoefficient(rLast->getDegree());
    int dltInverse = field->inverse(denominatorLeadingTerm);
    while (r->getDegree() >= rLast->getDegree() && !r->isZero()) {
      int degreeDiff = r->getDegree() - rLast->getDegree();
      int scale =
        field->multiply(r->getCoefficient(r->getDegree()), dltInverse);
      q = q->addOrSubtract(field->buildMonomial(degreeDiff, scale));
      r = r->addOrSubtract(rLast->multiplyByMonomial(degreeDiff, scale));
    }

    t = q->multiply(tLast)->addOrSubtract(tLastLast);

    if (r->getDegree() >= rLast->getDegree()) {
      throw 1;
    }
  }

  int sigmaTildeAtZero = t->getCoefficient(0);
  if (sigmaTildeAtZero == 0) {
    throw 1;
  }

  int inverse = field->inverse(sigmaTildeAtZero);
  auto sigma = t->multiply(inverse);
  auto omega = r->multiply(inverse);
  return EuclideanResult{ sigma, omega };
}

std::vector<int>
ReedSolomonDecoder::findErrorLocations(
  std::shared_ptr<GenericGFPoly> errorLocator)
{
  // This is a direct application of Chien's search
  int numErrors = errorLocator->getDegree();
  if (numErrors == 1) { // shortcut
    return std::vector<int>{ errorLocator->getCoefficient(1) };
  }
  std::vector<int> result(numErrors);
  int e = 0;
  for (int i = 1; i < field->getSize() && e < numErrors; i++) {
    if (errorLocator->evaluateAt(i) == 0) {
      result[e] = field->inverse(i);
      e++;
    }
  }
  if (e != numErrors) {
    throw 1;
  }
  return result;
}

std::vector<int>
ReedSolomonDecoder::findErrorMagnitudes(
  std::shared_ptr<GenericGFPoly> errorEvaluator,
  const std::vector<int>& errorLocations)
{
  // This is directly applying Forney's Formula
  int s = errorLocations.size();
  std::vector<int> result(s);
  for (int i = 0; i < s; i++) {
    int xiInverse = field->inverse(errorLocations[i]);
    int denominator = 1;
    for (int j = 0; j < s; j++) {
      if (i != j) {
        // denominator = field->multiply(denominator,
        //    GenericGF.addOrSubtract(1, field->multiply(errorLocations[j],
        //    xiInverse)));
        // Above should work but fails on some Apple and Linux JDKs due to a
        // Hotspot bug.
        // Below is a funny-looking workaround from Steven Parkes
        int term = field->multiply(errorLocations[j], xiInverse);
        int termPlus1 = (term & 0x1) == 0 ? term | 1 : term & ~1;
        denominator = field->multiply(denominator, termPlus1);
      }
    }
    result[i] = field->multiply(errorEvaluator->evaluateAt(xiInverse),
                                field->inverse(denominator));
    if (field->getGeneratorBase() != 0) {
      result[i] = field->multiply(result[i], xiInverse);
    }
  }
  return result;
}
