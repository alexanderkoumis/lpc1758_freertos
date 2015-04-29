#ifndef PIXY_CMU_HPP
#define PIXY_CMU_HPP

namespace cmu
{


struct PixyBlock
{
  uint16_t signature;
  uint16_t x;
  uint16_t y;
  uint16_t width;
  uint16_t height;
  uint16_t angle; // angle is only available for color coded blocks
};

void vInit();
int getStart(void);
void vReallyGetBlocks(int num_blocks, std::vector<PixyBlock>& blocks);
uint16_t usGetBlocks(uint16_t maxBlocks);
void vPrintInfo(cmu::PixyBlock& xPixyBlock);


}

#endif
