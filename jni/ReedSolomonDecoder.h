#ifndef REEDSOLOMONDECODER_H
#define REEDSOLOMONDECODER_H
#include <memory>
#include <vector>

class GenericGF;
class GenericGFPoly;
class ReedSolomonDecoder final
{
private:
  const GenericGF* field;

public:
  explicit ReedSolomonDecoder(const GenericGF* _field);

  /**
   * <p>Decodes given set of received codewords, which include both data and
   * error-correction
   * codewords. Really, this means it uses Reed-Solomon to detect and correct
   * errors, in-place,
   * in the input.</p>
   *
   * @param received data and error-correction codewords
   * @param twoS number of error-correction codewords available
   * @throws ReedSolomonException if decoding fails for any reason
   */
  void decode(std::vector<int>& received, int twoS);

private:
  struct EuclideanResult
  {
    std::shared_ptr<GenericGFPoly> first, second;
  };
  EuclideanResult runEuclideanAlgorithm(std::shared_ptr<GenericGFPoly> a,
                                        std::shared_ptr<GenericGFPoly> b,
                                        int R);

  std::vector<int> findErrorLocations(
    std::shared_ptr<GenericGFPoly> errorLocator);

  std::vector<int> findErrorMagnitudes(
    std::shared_ptr<GenericGFPoly> errorEvaluator,
    const std::vector<int>& errorLocations);
};
#endif /* REEDSOLOMONDECODER_H */
