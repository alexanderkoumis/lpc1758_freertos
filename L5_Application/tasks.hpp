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

#ifndef TASKS_HPP_
#define TASKS_HPP_

#include <memory>

#include "scheduler_task.hpp"
#include "event_groups.h"
#include "soft_timer.hpp"
#include "command_handler.hpp"
#include "wireless.h"
#include "char_dev.hpp"

#include "FreeRTOS.h"
#include "semphr.h"
#include "io.hpp"

#include "gpio.hpp"

#include "lpc_pwm.hpp"
#include "pixy.hpp"

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

enum eGame_t {DEBUG, COMPETE, RESET};
enum eDirection_t {LEFT, RIGHT};

struct xMotorCommand_t
{
    xMotorCommand_t(void) : eDirection(LEFT), xRotations(0.0) {}
    xMotorCommand_t(eDirection_t xDirection_arg, float& xRotations_arg) :
        eDirection(LEFT), xRotations(xRotations_arg) {}
    void Load(eDirection_t eDirection_arg, float xRotation_arg)
    {
        eDirection = eDirection_arg;
        xRotations = xRotation_arg;
    }
    eDirection_t eDirection;
    float xRotations;
};

struct GameCommand_t
{
        GameCommand_t(void) : eGame(DEBUG), ucCol(0) {}
        void Load(eGame_t eGame_arg, int ucCol_arg)
        {
            eGame = eGame_arg;
            ucCol = ucCol_arg;
        }
        eGame_t eGame;
        uint8_t ucCol;
};

class GameTask_t : public scheduler_task
{
    public:
        GameTask_t (uint8_t ucPriority);
        bool run(void *p);

    private:
        PWM *xServo;
        //PWM my_servo(PWM::pwm2, 50);
        const float xClosedPWM = 5.5;
        const float xOpenPWM = 11.5;
        void vRunServo(int lDropCount);
        void vRunStepper(uint8_t ucInsertCol);
};

class MotorTask_t : public scheduler_task
{
	public:
		MotorTask_t (uint8_t ucPriority);
		bool run(void *p);

	private:
		void vPollEnd(uint32_t ulLimitReg, xMotorCommand_t& xMotorCommand);
        uint32_t ulSetFrequency(float ulFreqHz);
		void vInitPWM();
		void vInitGPIO();
		void vStopCounter();
		void vStartCounter();

		GPIO xPWM_DIR;
		GPIO xPWM_EN;
        const int lPclkDivider = 8;
        const int lStepsPerRot = 400;
        unsigned int ulSysClk;
        float xMotorFreq = 1.0;
};

namespace pixy
{

class PixyTask_t : public scheduler_task
{
    public:
        PixyTask_t (uint8_t ucPriority) :
				scheduler_task("pixy", 2048, ucPriority),
				pPixy(new Pixy_t(CHIPS_AT_A_TIME, CHIPS_TO_CALIB, GREEN))
		{
            QueueHandle_t xQueueTXHandle = xQueueCreate(1, sizeof(int));
            QueueHandle_t xQueueRXHandle = xQueueCreate(1, sizeof(PixyCmd_t));
            QueueHandle_t xQueueResetTXHandle = xQueueCreate(1, sizeof(bool));
            QueueHandle_t xQueueResetRXHandle = xQueueCreate(1, sizeof(bool));
            addSharedObject(shared_PixyQueueTX, xQueueTXHandle);
            addSharedObject(shared_PixyQueueRX, xQueueRXHandle);
            addSharedObject(shared_PixyResetQueueTX, xQueueResetTXHandle);
            addSharedObject(shared_PixyResetQueueRX, xQueueResetRXHandle);
            ssp1_set_max_clock(1);
			delay_ms(128);
			while(LPC_SSP1->SR & (1 << 4));
			LPC_GPIO0->FIOCLR = (1 << 16); // P0[16] as SSP1
		}

        bool init(void)
        {
            Switches& xSwitches = SW.getInstance();
            bool bSwInit = xSwitches.init();
//            return bSwInit;
            return true;
        }

        bool run(void *p)
        {
            pPixy->vAction((Pixy_t::Button_t)SW.getSwitchValues());
            return true;
        }

    private:
        std::unique_ptr<Pixy_t> pPixy;
};

} // namespace pixy

} // namespace team9

#endif /* TASKS_HPP_ */
