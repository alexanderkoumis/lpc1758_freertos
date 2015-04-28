#include "tasks.hpp"
#include "LPC17xx.h"
#include "ssp1.h"
#include "printf_lib.h"
#include "shared_handles.h"
#include "utilities.h"

namespace team9
{


PixyTask_t::PixyTask_t(uint8_t priority) : scheduler_task("pixy", 2048, priority)
{
    vPopulateMap();
    ssp1_set_max_clock(1);
    delay_ms(64);
    while(LPC_SSP1->SR & (1 << 4));
    LPC_GPIO0->FIOCLR = (1 << 16); // P0[16] as SSP1
}

bool PixyTask_t::run(void *p)
{
    if (!bCalibrated)
    {
        vCalibrateBoard(4, 4, 50);
    }
    vStateMachine();
    return true;
}

void PixyTask_t::vPopulateMap()
{
    xStateMap[START] = std::string("START");
    xStateMap[READING_FRAME] = std::string("READING_FRAME");
}

void PixyTask_t::vCalibrateBoard(int rows, int cols, int samples)
{
    while (ulImgCount < samples)
    {
        vStateMachine();

    }
}

void PixyTask_t::vStateMachine()
{
    u0_dbg_printf("Imgcount: %d\tobjecount: %d\tState: %s\n", ulImgCount, ulObjectCount, xStateMap[eState].c_str());
    switch(eState)
    {
        case START:
        {
            while (1)
            {
                ReadShort();
                if (usRecv == 0xaa55 && usRecvLast == 0xaa55)
                {
                    eState = READING_FRAME;
                    ulObjectCount = 0;
                    ulImgCount++;
                    return;
                }
                else if (usRecv == 0xaa55)
                {
                    ReadShort();
                    if (usRecv == 0xaa55 && usRecvLast == 0xaa55)
                    {
                        ulObjectCount++;
                    }
                    eState = READING_FRAME;
                    return;
                }
            }
            break;
        }
        case READING_FRAME:
        {
            xObject.usChecksum = ReadShort();
            xObject.usSignature = ReadShort();
            xObject.usX = ReadShort();
            xObject.usY = ReadShort();
            xObject.usWidth = ReadShort();
            xObject.usHeight = ReadShort();
            ulObjectCount++;
            if (xObject.usSignature != 1 && xObject.usSignature != 2)
            {
                u0_dbg_printf("Yo wtf!!!\n\n\n");
            }
            xObject.vClear();
            eState = START;
            break;
        }
        default:
        {
            u0_dbg_printf("Error! State machine should not hit default case!\n");
            break;
        }
    }
}

void PixyTask_t::vPrintInfo(Block_t& xObjectBlock)
{
    u0_dbg_printf("Image #%d - Block #%d\n"
                  "  Checksum    : %02x\n"
                  "  Signature   : %02x\n"
                  "  Coordinates : [%02d x %02d]\n"
                  "  Size        : [%02d x %02d]\n",
                  ulImgCount, ulObjectCount,
                  xObjectBlock.usChecksum,
                  xObjectBlock.usSignature,
                  xObjectBlock.usX, xObject.usY,
                  xObjectBlock.usWidth, xObject.usHeight);
}


__inline uint16_t PixyTask_t::ReadShort()
{
    usRecvLast = usRecv;
    uint16_t usResponse = ssp1_exchange_byte(0x5a);
    usRecv = ssp1_exchange_byte(0x5a) | (usResponse << 8);
    return usRecv;
}

} // namespace team9
