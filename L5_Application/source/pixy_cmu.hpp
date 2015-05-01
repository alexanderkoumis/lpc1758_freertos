#ifndef PIXY_CMU_HPP
#define PIXY_CMU_HPP

#include <vector>

#include "printf_lib.h"

#define FUCK(); u0_dbg_printf("line: %d\tfunc: %s\n", __LINE__, __func__);

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


void vPrintInfo(cmu::PixyBlock& xPixyBlock);

uint16_t usGetBlocks(int lMaxBlocks, std::vector<PixyBlock>& blocks);
uint16_t usGetBlocks(uint16_t usMaxBlocks);
//void vReallyGetBlocks(int lMaxBlocks, std::vector<PixyBlock>& blocks);
uint16_t usGetBlocks(int lMaxBlocks, std::vector<PixyBlock>& vRecvBlocks);

}

#endif
