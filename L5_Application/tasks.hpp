/*
 *     SocialLedge.com - Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */

/**
 * @file
 * @brief Contains FreeRTOS Tasks
 */
#ifndef TASKS_HPP_
#define TASKS_HPP_

#include <cmath>
#include <memory>
#include <map>
#include <vector>

#include "pixy.hpp"

#include "scheduler_task.hpp"
#include "soft_timer.hpp"
#include "command_handler.hpp"
#include "wireless.h"
#include "char_dev.hpp"
#include "utilities.h"

#include "printf_lib.h"
#include "ssp1.h"


/**
 * Terminal task is our UART0 terminal that handles our commands into the board.
 * This also saves and restores the "disk" telemetry.  Disk telemetry variables
 * are automatically saved and restored across power-cycles to help us preserve
 * any non-volatile information.
 */
class terminalTask : public scheduler_task
{
    public:
        terminalTask(uint8_t priority);     ///< Constructor
        bool regTlm(void);                  ///< Registers telemetry
        bool taskEntry(void);               ///< Registers commands.
        bool run(void *p);                  ///< The main loop

    private:
        // Command channels device and input command str
        typedef struct {
            CharDev *iodev; ///< The IO channel
            str *cmdstr;    ///< The command string
            bool echo;      ///< If input should be echo'd back
        } cmdChan_t;

        VECTOR<cmdChan_t> mCmdIface;   ///< Command interfaces
        CommandProcessor mCmdProc;     ///< Command processor
        uint16_t mCommandCount;        ///< terminal command count
        uint16_t mDiskTlmSize;         ///< Size of disk variables in bytes
        char *mpBinaryDiskTlm;         ///< Binary disk telemetry
        SoftTimer mCmdTimer;           ///< Command timer

        cmdChan_t getCommand(void);
        void addCommandChannel(CharDev *channel, bool echo);
        void handleEchoAndBackspace(cmdChan_t *io, char c);
        bool saveDiskTlm(void);
};


namespace team9
{


class PixyTask_t : public scheduler_task
{


    struct Block_t
    {
        uint16_t usSignature;
        uint16_t usX;
        uint16_t usY;
        uint16_t usWidth;
        uint16_t usHeight;

        Block_t(uint16_t usSignature_arg,
                uint16_t usX_arg,
                uint16_t usY_arg,
                uint16_t usWidth_arg,
                uint16_t usHeight_arg) :
                    usSignature(usSignature_arg),
                    usX(usX_arg),
                    usY(usY_arg),
                    usWidth(
                            usWidth_arg),
                    usHeight(usHeight_arg) {}

        void vClear()
        {
            usSignature = 0x0000;
            usX = 0x0000;
            usY = 0x0000;
            usWidth = 0x0000;
            usHeight = 0x0000;
        }

        uint16_t usSum()
        {
            return usSignature + usX + usY + usWidth + usHeight;
        }
    };

    public:
        PixyTask_t(uint8_t priority) : scheduler_task("pixy", 2048, priority)
        {
            ssp1_set_max_clock(1);
            delay_ms(64);
            while(LPC_SSP1->SR & (1 << 4));
            LPC_GPIO0->FIOCLR = (1 << 16); // P0[16] as SSP1
            pPixyPtr.reset(new team9::Pixy);
        }

        bool run(void *p)
        {
            pPixyPtr->vStateMachine();
            return true;
        }

	private:
	    std::unique_ptr<Pixy> pPixyPtr;
};

} // namespace team9

#endif /* TASKS_HPP_ */
