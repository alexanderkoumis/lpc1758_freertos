#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <vector>

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

// data types
enum BlockType {NORMAL_BLOCK, CC_BLOCK};

static PixyBlock *g_blocks;

static BlockType g_blockType;
static int g_skipStart = 0;
static uint8_t g_outBuf[PIXY_OUTBUF_SIZE];
static uint8_t g_outLen = 0;
static uint8_t g_outWriteIndex = 0;
static uint8_t g_outReadIndex = 0;

#define FUCK(); u0_dbg_printf("line: %d\n", __LINE__);


static int lSend(uint8_t *data, int len)
{
    int i;
    if (g_outLen + len > PIXY_OUTBUF_SIZE) // Check for space in circular queue
    {
        return -1;
    }
    g_outLen += len;
    for (i = 0; i < len; i++)
    {
        g_outBuf[g_outWriteIndex++] = data[i];
        if (g_outWriteIndex==PIXY_OUTBUF_SIZE)
        {
          g_outWriteIndex = 0;
        }
    }
    return len;
}


void vInit()
{
    g_blocks = (PixyBlock*)malloc(sizeof(PixyBlock) * PIXY_ARRAYSIZE);
}

static uint16_t usGetWord(void)
{
    delay_ms(3);
    uint16_t usRecv = 0;
    uint8_t ucRecv = 0;
    uint8_t ucOut = 0;
    if (g_outLen)
    {
        usRecv = ssp1_exchange_byte(PIXY_SYNC_BYTE_DATA);
        ucOut = g_outBuf[g_outReadIndex++];
        g_outLen--;
        if (g_outReadIndex == PIXY_OUTBUF_SIZE)
        {
            g_outReadIndex = 0;
        }
    }
    else
    {
      usRecv = ssp1_exchange_byte(PIXY_SYNC_BYTE); // send out sync byte
    }
    delay_ms(3);

    ucRecv = ssp1_exchange_byte(ucOut); // send out data byte
    usRecv <<= 8;
    usRecv |= ucRecv;
    return usRecv;
}

void vReallyGetBlocks(int max_blocks, std::vector<PixyBlock>& vRecvBlocks)
{
    uint16_t usRecvBlocks = usGetBlocks(1000);
    PixyBlock* xBlocks = g_blocks;
    vRecvBlocks.assign(xBlocks, xBlocks + 50 * sizeof(PixyBlock));
}

uint16_t usGetBlocks(uint16_t usMaxBlocks)
{
    uint8_t ucI;
    uint16_t usRecv;
    uint16_t usChecksum;
    uint16_t usBlockCount = 0;
    uint16_t usSum;
    PixyBlock *xBlock;

    if (g_skipStart)
    {
        g_skipStart = 0;
    }
    else
    {
        uint16_t usRecvLast = 0xffff;
        while(1)
        {
            usRecv = usGetWord();
            if (usRecv == 0 && usRecvLast==0)
            {
                return 0;
            }
            else if (usRecv == PIXY_START_WORD && usRecvLast == PIXY_START_WORD)
            {
                g_blockType = NORMAL_BLOCK;
                break;
            }
            else if (usRecv == PIXY_START_WORD_CC && usRecvLast == PIXY_START_WORD)
            {
                g_blockType = CC_BLOCK;
                break;
            }
            else if (usRecv == PIXY_START_WORDX)
            {
                ssp1_exchange_byte(0); // out of sync
            }
            usRecvLast = usRecv;
        }
    }

    while(usBlockCount < usMaxBlocks && usBlockCount < PIXY_ARRAYSIZE)
    {
        usChecksum = usGetWord();
        if (usChecksum == PIXY_START_WORD) // Beginning of next frame
        {
            FUCK();
            g_skipStart = 1;
            g_blockType = NORMAL_BLOCK;
            return usBlockCount;
        }
        else if (usChecksum == PIXY_START_WORD_CC)
        {
            FUCK();
            g_skipStart = 1;
            g_blockType = CC_BLOCK;;
            return usBlockCount;
        }
        else if (usChecksum == 0)
        {
            FUCK();
            return usBlockCount;
        }

        xBlock = g_blocks + usBlockCount;

        for (ucI = 0, usSum = 0; ucI < sizeof(PixyBlock)/sizeof(uint16_t); ucI++)
        {
            if (g_blockType == NORMAL_BLOCK && ucI >= 5) // no angle
            {
                xBlock->angle = 0;
                break;
            }
            usRecv = usGetWord();
            usSum += usRecv;
            *((uint16_t *)xBlock + ucI) = usRecv;
        }

        if (usChecksum == usSum)
        {
            usBlockCount++;
        }
        else
        {
          u0_dbg_printf("usChecksum error!\n");
        }

        usRecv = usGetWord();
        if (usRecv == PIXY_START_WORD)
        {
            g_blockType = NORMAL_BLOCK;
        }
        else if (usRecv == PIXY_START_WORD_CC)
        {
            g_blockType = CC_BLOCK;
        }
        else
        {
            FUCK();
            u0_dbg_printf("blockcount: %d\treceived: %x\n", usBlockCount, usRecv);
            return usBlockCount;
        }
    }
}


void vPrintInfo(cmu::PixyBlock& xPixyBlock)
{
    u0_dbg_printf("Signature   : %04x\n"
                  "  Coordinates : [%04x x %04x]\n"
                  "  Size        : [%04x x %04x]\n"
                  "  Angle       : [%04x]\n",
                  xPixyBlock.signature,
                  xPixyBlock.x, xPixyBlock.y,
                  xPixyBlock.width, xPixyBlock.height,
                  xPixyBlock.angle);
}

}
