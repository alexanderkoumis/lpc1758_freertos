#include "tasks.hpp"
#include "LPC17xx.h"
#include "ssp1.h"
#include "printf_lib.h"
#include "shared_handles.h"

namespace team9
{

PixyTask_t::PixyTask_t(uint8_t priority) : scheduler_task("pixy", 2048, priority)
{
    while(LPC_SSP1->SR & (1 << 4));
    LPC_GPIO0->FIOCLR = (1 << 16); // P0[16] as SSP1
    ssp1_set_max_clock(1);
}

bool PixyTask_t::run(void *p)
{
    vStateMachine();
    return true;
}

void PixyTask_t::vStateMachine()
{
    switch(eState)
    {
        case START:
        {
          ulTimesSeen = (ReadShort() == 0xaa55) ? ulTimesSeen + 1 : ulTimesSeen;
          eState = (ulTimesSeen == 2) ? START : WAITING;
          break;
        }
        case WAITING:
        {
            switch (ReadShort())
            {
                case 0xaa55:
                {
                    eState = READING_NORMAL_FRAME;
                    break;
                }
                case 0xaa56:
                {
                    eState = READING_COLOR_FRAME;
                    break;
                }
                default:
                {
                    eState = WAITING;
                    break;
                }
            }
            break;
        }
        case READING_NORMAL_FRAME:
        {
            usRecv = ReadShort();
            switch (usRecv)
            {
                case 0xaa55:
                {
                    ulObjectCount = 0;
                    ulImgCount++;
                    eState = READING_NORMAL_FRAME;
                    return;
                }
                case 0xaa56:
                {
                    ulObjectCount = 0;
                    ulImgCount++;
                    eState = READING_COLOR_FRAME;
                    return;
                }
                default:
                {
                    // Checksum
                    ulObjectCount++;
                    break;
                }
            }
            xObject = Block_t(ReadShort(), ReadShort(), ReadShort(),
                              ReadShort(), ReadShort(), ReadShort());
            eState = DONE;
            break;
        }
        case READING_COLOR_FRAME:
        {
            ReadShort(); // checksum
            xObject = Block_t(ReadShort(), ReadShort(), ReadShort(),
                              ReadShort(), ReadShort(), ReadShort());
            eState = DONE;
            break;
        }
        case DONE:
        {
          u0_dbg_printf("Image #%d - Block #%d\n"
                        "  Signature   : %02d\n"
                        "  Coordinates : [%02d x %02d]\n"
                        "  Size        : [%02d x %02d]\n"
                        "  Angle       : %02d\n",
                        ulImgCount, ulObjectCount,
                        xObject.usSignature,
                        xObject.usX, xObject.usY,
                        xObject.usWidth, xObject.usHeight,
                        xObject.usAngle);
          eState = WAITING;
          break;
        }
        default:
        {
            u0_dbg_printf("Error! State machine should not hit default case!\n");
            break;
        }
    }

}

__inline uint16_t PixyTask_t::ReadShort() {
    uint8_t recv_byte = ssp1_exchange_byte(0x00);
    return ((ssp1_exchange_byte(0x00) << 8) | (0x00 | recv_byte));
}

} // namespace team9
