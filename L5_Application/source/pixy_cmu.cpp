#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "ssp1.h"
#include "pixy_cmu.hpp"
#include "printf_lib.h"

#define PIXY_ARRAYSIZE              100
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


// communication routines
static uint16_t getWord(void);
static int send(uint8_t *data, int len);

// variables for a little circular queue
static uint8_t g_outBuf[PIXY_OUTBUF_SIZE];
static uint8_t g_outLen = 0;
static uint8_t g_outWriteIndex = 0;
static uint8_t g_outReadIndex = 0;

static PixyBlock *g_blocks;
__inline uint16_t getWord()
{
  // ordering is big endian because Pixy is sending 16 bits through SPI
  uint16_t w;
  uint8_t c, cout = 0;

  if (g_outLen)
  {
    w = ssp1_exchange_byte(PIXY_SYNC_BYTE_DATA);
    cout = g_outBuf[g_outReadIndex++];
    g_outLen--;
    if (g_outReadIndex==PIXY_OUTBUF_SIZE)
      g_outReadIndex = 0;
  }
  else
  {
    w = ssp1_exchange_byte(PIXY_SYNC_BYTE); // send out sync byte
  }
  w <<= 8;
  c = ssp1_exchange_byte(cout); // send out data byte
  w |= c;

  return w;
}


void reallyGetBlocks(int max_blocks, std::vector<PixyBlock>& vRecvBlocks)
{
    uint16_t usRecvBlocks = cmu::getBlocks(max_blocks);
    vRecvBlocks.resize(usRecvBlocks);
    PixyBlock* blocks = g_blocks;
    vRecvBlocks.assign(blocks, blocks + usRecvBlocks * sizeof(PixyBlock));
}

int send(uint8_t *data, int len)
{
  int i;

  // check to see if we have enough space in our circular queue
  if (g_outLen+len>PIXY_OUTBUF_SIZE)
    return -1;

  g_outLen += len;
  for (i=0; i<len; i++)
  {
    g_outBuf[g_outWriteIndex++] = data[i];
    if (g_outWriteIndex==PIXY_OUTBUF_SIZE)
      g_outWriteIndex = 0;
  }
  return len;
}

static int g_skipStart = 0;
static BlockType g_blockType;

void init()
{
  g_blocks = (PixyBlock *)malloc(sizeof(PixyBlock)*PIXY_ARRAYSIZE);
}

int getStart(void)
{

  uint16_t w, lastw;

  lastw = 0xffff;

  while(1)
  {
    w = getWord();
    if (w==0 && lastw==0)
      return 0; // no start code
    else if (w==PIXY_START_WORD && lastw==PIXY_START_WORD)
    {
      g_blockType = NORMAL_BLOCK;
      return 1; // code found!
    }
    else if (w==PIXY_START_WORD_CC && lastw==PIXY_START_WORD)
    {
      g_blockType = CC_BLOCK; // found color code block
      return 1;
    }
    else if (w == PIXY_START_WORDX)
      ssp1_exchange_byte(0x00); // we're out of sync! (backwards)
    lastw = w;
  }
}

uint16_t getBlocks(uint16_t maxBlocks)
{
  uint8_t i;
  uint16_t w, blockCount, checksum, sum;
  PixyBlock *block;


  if (!g_skipStart)
  {
    if (getStart()==0)
      return 0;
  }
  else
    g_skipStart = 0;

  for(blockCount=0; blockCount<maxBlocks && blockCount<PIXY_ARRAYSIZE;)
  {
    checksum = getWord();
    if (checksum==PIXY_START_WORD) // we've reached the beginning of the next frame
    {
      g_skipStart = 1;
      g_blockType = NORMAL_BLOCK;
      return blockCount;
    }
    else if (checksum == PIXY_START_WORD_CC)
    {
      g_skipStart = 1;
      g_blockType = CC_BLOCK;
      return blockCount;
    }
    else if (checksum==0)
      return blockCount;

    block = g_blocks + blockCount;

    for (i=0, sum=0; i<sizeof(PixyBlock)/sizeof(uint16_t); i++)
    {
      if (g_blockType==NORMAL_BLOCK && i>=5) // no angle for normal block
      {
        block->angle = 0;
        break;
      }
      w = getWord();
      sum += w;
      *((uint16_t *)block + i) = w;
    }

    // check checksum
    if (checksum==sum)
      blockCount++;
    else
      printf("checksum error!\n");

    w = getWord();
    if (w==PIXY_START_WORD)
      g_blockType = NORMAL_BLOCK;
    else if (w==PIXY_START_WORD_CC)
      g_blockType = CC_BLOCK;
    else
      return blockCount;
  }
}


void vPrintInfo(cmu::PixyBlock& xPixyBlock)
{
    u0_dbg_printf("  Signature   : %02x\n"
                  "  Coordinates : [%02d x %02d]\n"
                  "  Size        : [%02d x %02d]\n",
                  xPixyBlock.signature,
                  xPixyBlock.x, xPixyBlock.y,
                  xPixyBlock.width, xPixyBlock.height);
}

}
