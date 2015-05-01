#include "pixy_cmu2.hpp"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <vector>
#include <map>
#include <memory>
#include <functional>

#include "ssp1.h"
#include "pixy_cmu.hpp"
#include "printf_lib.h"
#include "utilities.h" // delay_ms

namespace cmu
{

PixyCMU_t::PixyCMU_t(uint32_t ulMaxBlocks_arg) : ulMaxBlocks(ulMaxBlocks_arg),
                                                 usRecvLast(0xffff),
                                                 usChecksum(0)
{
    vPixyBlocks.resize(ulMaxBlocks);
    vInitFuncMap();
}

void PixyCMU_t::vSetHandler(State_t eState_arg, std::function<void()> fHandler)
{
    this->xFuncMap[eState_arg] = fHandler;
}

std::function<void()>& PixyCMU_t::vResponse(const uint8_t ucStat)
{
    return this->xFuncMap[ucStat];
}

void PixyCMU_t::vPrintBlock(cmu::PixyBlock2& xBlock)
{
    u0_dbg_printf("Signature   : %04x\n"
                  "  Coordinates : [%04x x %04x]\n"
                  "  Size        : [%04x x %04x]\n"
                  "  Angle       : [%04x]\n",
                  xBlock.signature,
                  xBlock.x, xBlock.y,
                  xBlock.width, xBlock.height,
                  xBlock.angle);
}


void PixyCMU_t::vGetBlocks(std::vector<PixyBlock2>& xBlocks)
{
    while (eState != DONE)
    {
        auto& fpResponse = this->vResponse(eState);
        fpResponse();
    }
    xBlocks = this->vPixyBlocks;
    vPixyBlocks.clear();
    vPixyBlocks.resize(ulMaxBlocks);
}

__inline uint16_t PixyCMU_t::usGetWord()
{
    delay_ms(3);
    uint16_t usRecv = ssp1_exchange_byte(0) << 8;
    delay_ms(3);
    return (usRecv |= ssp1_exchange_byte(0));
}

void PixyCMU_t::vInitFuncMap()
{
    u0_dbg_printf("Initializing function map\n");

    vSetHandler(START, [&] ()
    {
        uint16_t usRecv = usGetWord();
        if (usRecv == 0xaa55 && usRecvLast == 0xaa55)
        {
            eBlockType = NORMAL;
            eState = READ_FIRST;
            FUCK();
        }
        else if (usRecv == 0xaa56 && usRecvLast == 0xaa55)
        {
            eBlockType = COLOR;
            eState = READ_FIRST;
            FUCK();
        }
        else if (usRecv == 0x55aa)
        {
            ssp1_exchange_byte(0); // out of sync
            eState = START;
            FUCK();
        }
        else
        {
            eState = START;
            FUCK();
        }
        eState = START;
        usRecvLast = usRecv;
        FUCK();
    });

    vSetHandler(READ_FIRST, [&] ()
    {
        uint16_t usRecv = usGetWord();
        FUCK();
        switch (usRecv)
        {
            case 0xaa55:
                {
                eBlockType = NORMAL;
                eState = DONE;
                FUCK();
                break;
            }
            case 0xaa56:
            {
                eBlockType = COLOR;
                eState = DONE;
                FUCK();
                break;
            }
            case 0x0000:
            {
                eState = START;
                FUCK();
                break;
            }
            default:
            {
                usChecksum = usRecv;
                eState = READ_BLOCK;
                FUCK();
            }
        }
        eState = READ_FIRST;
        usRecvLast = usRecv;
    });


    vSetHandler(READ_BLOCK, [&] ()
    {
        FUCK();
        std::unique_ptr<PixyBlock2> pBlock(new PixyBlock2);
        pBlock->signature = usGetWord();
        pBlock->x = usGetWord();
        pBlock->y = usGetWord();
        pBlock->width = usGetWord();
        pBlock->height = usGetWord();
        pBlock->angle = (eBlockType == NORMAL) ? 0 : usGetWord();

        usRecvLast = pBlock->height;
        vPrintBlock(*pBlock);

        uint16_t usBlockSum = pBlock->signature + pBlock->x + pBlock->y +
                              pBlock->width + pBlock->height + pBlock->angle;

        if (usChecksum == pBlock->signature + pBlock->x + pBlock->y +
                          pBlock->width + pBlock->height + pBlock->angle)
        {
           vPixyBlocks[usBlockCount] = *pBlock;
           usBlockCount++;
        }
        else
        {
            u0_dbg_printf("Checksum error! usChecksum %d\tusBlocksum: %d\n",
                          usChecksum, usBlockSum);
        }
        eState = (usBlockCount == ulMaxBlocks) ? DONE : READ_NEXT;
        FUCK();
    });

    vSetHandler(READ_NEXT, [&] ()
    {
        FUCK();
        uint16_t usRecv = usGetWord();
        switch (usRecv)
        {
            case 0xaa55:
            {
                FUCK();
                eBlockType = NORMAL;
                eState = READ_FIRST;
                break;
            }
            case 0xaa56:
            {
                FUCK();
                eBlockType = COLOR;
                eState = READ_FIRST;
                break;
            }
            default:
            {
                FUCK();
                eState = START;
                break;
            }
        }
        eState = READ_NEXT;
        usRecvLast = usRecv;
    });

    vSetHandler(DONE, [&] ()
    {
        FUCK();
        u0_dbg_printf("DONE: Block count: %d\n", usBlockCount);
        eState = DONE;
    });
}


} // namespace cmu
