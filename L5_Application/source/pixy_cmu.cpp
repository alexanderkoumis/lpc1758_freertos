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

#define PIXY_ARRAYSIZE              1000
#define PIXY_START_WORD             0xaa55
#define PIXY_START_WORD_CC          0xaa56
#define PIXY_START_WORDX            0x55aa
#define PIXY_SERVO_SYNC             0xff
#define PIXY_CAM_BRIGHTNESS_SYNC    0xfe
#define PIXY_LED_SYNC               0xfd
#define PIXY_OUTBUF_SIZE            64

#define PIXY_SYNC_BYTE              0x5a
#define PIXY_SYNC_BYTE_DATA         0x5b

namespace cmu
{


enum State_t {START, READ_FIRST, READ_BLOCK, READ_NEXT, DONE};
enum BlockType {NORMAL, COLOR};

static State_t eState = START;
static BlockType g_blockType;
static PixyBlock *g_blocks;

void vPrintInfo(cmu::PixyBlock& xPixyBlock)
{
    u0_dbg_printf("Signature   : %4x\n"
                  "  Coordinates : [%4d x %4d]\n"
                  "  Size        : [%4d x %4d]\n"
                  "  Angle       : [%4x]\n",
                  xPixyBlock.signature,
                  xPixyBlock.x, xPixyBlock.y,
                  xPixyBlock.width, xPixyBlock.height,
                  xPixyBlock.angle);
}

void vInit()
{
    g_blocks = (PixyBlock*)malloc(sizeof(PixyBlock) * PIXY_ARRAYSIZE);
}

uint16_t usGetWord()
{
    uint16_t usRecv = ssp1_exchange_byte(0x5a); // Sync byte
    usRecv <<= 8;
    return usRecv |= ssp1_exchange_byte(0);
}

uint16_t usGetBlocks(int lMaxBlocks, std::vector<PixyBlock>& vRecvBlocks)
{
    uint16_t usRecvLast = 0xffff;
    uint16_t usChecksum = 0x0000;
    uint16_t usBlockCount = 0x0000;

    vRecvBlocks.clear();
    vRecvBlocks.resize(lMaxBlocks);

    while (eState != DONE)
    {
        switch (eState)
        {
            case START:
            {
                uint16_t usRecv = usGetWord();
                if (usRecv == 0xaa55 && usRecvLast == 0xaa55)
                {
                    g_blockType = NORMAL;
                    eState = READ_FIRST;
                }
                else if (usRecv == 0xaa56 && usRecvLast == 0xaa55)
                {
                    g_blockType = COLOR;
                    eState = READ_FIRST;
                }
                else if (usRecv == 0x55aa)
                {
                    ssp1_exchange_byte(0); // out of sync
                    eState = START;
                }
                else
                {
                    eState = START;
                }
                usRecvLast = usRecv;
                break;
            }
            case READ_FIRST:
            {
                uint16_t usRecv = usGetWord();
                switch (usRecv)
                {
                    case 0xaa55:
                    {
                        g_blockType = NORMAL;
                        eState = DONE;
                        break;
                    }
                    case 0xaa56:
                    {
                        g_blockType = COLOR;
                        eState = DONE;
                        break;
                    }
                    case 0x0000:
                    {
                        eState = START;
                        break;
                    }
                    default:
                    {
                        usChecksum = usRecv;
                        eState = READ_BLOCK;
                        break;
                    }
                }
                usRecvLast = usRecv;
                break;
            }
            case READ_BLOCK:
            {
                PixyBlock xPixyBlock;
                xPixyBlock.signature = usGetWord();
                xPixyBlock.x = usGetWord();
                xPixyBlock.y = usGetWord();
                xPixyBlock.width = usGetWord();
                xPixyBlock.height = usGetWord();
                xPixyBlock.angle = (g_blockType == NORMAL) ? 0 : usGetWord();
                usRecvLast = xPixyBlock.height;

                uint16_t usBlockSum = xPixyBlock.signature + xPixyBlock.x +
                                      xPixyBlock.y + xPixyBlock.width +
                                      xPixyBlock.height + xPixyBlock.angle;
                if (usChecksum == usBlockSum)
                {
                    vRecvBlocks[usBlockCount] = xPixyBlock;
                    usBlockCount++;
                }
                else
                {
                    u0_dbg_printf("usChecksum error! Checksum %d\tBlocksum: %d\n",
                                   usChecksum, usBlockSum);
                }
                eState = (usBlockCount == lMaxBlocks) ? DONE : READ_NEXT;
                break;
            }
            case READ_NEXT:
            {
                uint16_t usRecv = usGetWord();
                switch (usRecv)
                {
                    case 0xaa55:
                    {
                        g_blockType = NORMAL;
                        eState = READ_FIRST;
                        break;
                    }
                    case 0xaa56:
                    {
                        g_blockType = COLOR;
                        eState = READ_FIRST;
                        break;
                    }
                    default:
                    {
                        eState = START;
                        break;
                    }
                }
                usRecvLast = usRecv;
                break;
            }
            case DONE:
            {
                u0_dbg_printf("Block count: %d\n", usBlockCount);
                break;
            }
        }
    }
    vRecvBlocks.resize(usBlockCount);
    eState = START;
    return usBlockCount;
}

}
