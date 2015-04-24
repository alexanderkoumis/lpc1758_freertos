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

#include "scheduler_task.hpp"
#include "soft_timer.hpp"
#include "command_handler.hpp"
#include "wireless.h"
#include "char_dev.hpp"

#include "FreeRTOS.h"
#include "semphr.h"

#include "gpio.hpp"

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

enum eDirection_t {LEFT, RIGHT};

struct xMotorCommand_t
{
    xMotorCommand_t(void) : eDirection(LEFT), xRotations(0.0) {}
    xMotorCommand_t(eDirection_t direction, float& rotations) :
        eDirection(LEFT), xRotations(rotations) {}
    void Load(eDirection_t direction, float rotations)
    {
        eDirection = direction;
        xRotations = rotations;
    }
    eDirection_t eDirection;
	float xRotations;
};

class MotorTask_t : public scheduler_task
{
	public:
		MotorTask_t (uint8_t priority);
		bool run(void *p);

	private:
		void vPollEnd(uint32_t ulLimitReg, xMotorCommand_t& xMotorCommand);
        uint32_t ulSetFrequency(uint32_t ulFreqHz);
		void vInitPWM();
		void vInitGPIO();
		void vStopCounter();
		void vStartCounter();

		GPIO xPWM_DIR;
		GPIO xPWM_EN;
        const int ulPclkDivider = 8;
        const int uxStepsPerRot = 400;
        unsigned int ulSysClk;
};

} // namespace team9


#endif /* TASKS_HPP_ */
