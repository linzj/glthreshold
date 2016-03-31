#include "DataBlock.h"
#include "Version.h"
DataBlock::DataBlock(int _numDataCodewords, std::vector<uint8_t>&& _codewords)
  : numDataCodewords(_numDataCodewords)
  , codewords(std::move(_codewords))
{
}

std::vector<std::unique_ptr<DataBlock>>
DataBlock::getDataBlocks(std::vector<uint8_t>&& rawCodewords,
                         const Version& version, ErrorCorrectionLevel ecLevel)
{

  if (rawCodewords.size() != version.getTotalCodewords()) {
    throw std::exception();
  }

  // Figure out the number and size of data blocks used by this version and
  // error correction level
  const ECBlocks& ecBlocks = version.getECBlocksForLevel(ecLevel);

  // First count the total number of data blocks
  int totalBlocks = 0;
  auto ecBlockArray = ecBlocks.getECBlocks();
  for (auto& ecBlock : ecBlockArray) {
    totalBlocks += ecBlock.getCount();
  }

  // Now establish DataBlocks of the appropriate size and number of data
  // codewords
  std::vector<std::unique_ptr<DataBlock>> result(totalBlocks);
  int numResultBlocks = 0;
  for (auto& ecBlock : ecBlockArray) {
    for (int i = 0; i < ecBlock.getCount(); i++) {
      int numDataCodewords = ecBlock.getDataCodewords();
      int numBlockCodewords =
        ecBlocks.getECCodewordsPerBlock() + numDataCodewords;
      result[numResultBlocks++].reset(new DataBlock(
        numDataCodewords, std::vector<uint8_t>(numBlockCodewords)));
    }
  }

  // All blocks have the same amount of data, except that the last n
  // (where n may be 0) have 1 more byte. Figure out where these start.
  int shorterBlocksTotalCodewords = result[0]->codewords.size();
  int longerBlocksStartAt = result.size() - 1;
  while (longerBlocksStartAt >= 0) {
    int numCodewords = result[longerBlocksStartAt]->codewords.size();
    if (numCodewords == shorterBlocksTotalCodewords) {
      break;
    }
    longerBlocksStartAt--;
  }
  longerBlocksStartAt++;

  int shorterBlocksNumDataCodewords =
    shorterBlocksTotalCodewords - ecBlocks.getECCodewordsPerBlock();
  // The last elements of result may be 1 element longer;
  // first fill out as many elements as all of them have
  int rawCodewordsOffset = 0;
  for (int i = 0; i < shorterBlocksNumDataCodewords; i++) {
    for (int j = 0; j < numResultBlocks; j++) {
      result[j]->codewords[i] = rawCodewords[rawCodewordsOffset++];
    }
  }
  // Fill out the last data block in the longer ones
  for (int j = longerBlocksStartAt; j < numResultBlocks; j++) {
    result[j]->codewords[shorterBlocksNumDataCodewords] =
      rawCodewords[rawCodewordsOffset++];
  }
  // Now add in error correction blocks
  int max = result[0]->codewords.size();
  for (int i = shorterBlocksNumDataCodewords; i < max; i++) {
    for (int j = 0; j < numResultBlocks; j++) {
      int iOffset = j < longerBlocksStartAt ? i : i + 1;
      result[j]->codewords[iOffset] = rawCodewords[rawCodewordsOffset++];
    }
  }
  return result;
}
