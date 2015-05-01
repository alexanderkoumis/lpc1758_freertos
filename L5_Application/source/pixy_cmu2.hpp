#ifndef PIXY_CMU2_HPP
#define PIXY_CMU2_HPP

#include <inttypes.h>
#include <vector>
#include <map>
#include <functional>

namespace cmu
{

struct PixyBlock2
{
  uint16_t signature;
  uint16_t x;
  uint16_t y;
  uint16_t width;
  uint16_t height;
  uint16_t angle; // angle is only available for color coded blocks
};

class PixyCMU_t
{
    public:
        PixyCMU_t(uint32_t ulMaxBlocks);
        void vGetBlocks(std::vector<PixyBlock2>& xBlocks);
        void vPrintBlock(PixyBlock2& xBlock);

        enum State_t {START, READ_FIRST, READ_BLOCK, READ_NEXT, DONE} eState;
        enum BlockType_t {NORMAL, COLOR} eBlockType;

    private:
        void vSetHandler(State_t eState, std::function<void()> fHandler);
        void vInitFuncMap();

        std::function<void()>& vResponse(const uint8_t ucStat);
        uint16_t usGetWord(void);

        std::vector<PixyBlock2> vPixyBlocks;
        std::map<uint8_t, std::function<void()>> xFuncMap;
        uint32_t ulMaxBlocks;
        uint16_t usRecvLast;
        uint16_t usChecksum;
        uint16_t usBlockCount;
};

}

#endif
