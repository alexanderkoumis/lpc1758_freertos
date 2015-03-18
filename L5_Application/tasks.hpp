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

#include <memory> // unique_ptr

#include "scheduler_task.hpp"
#include "soft_timer.hpp"
#include "command_handler.hpp"
#include "wireless.h"
#include "char_dev.hpp"

#include "FreeRTOS.h"
#include "semphr.h"

#include "io.hpp" // Light_Sensor
#include "event_groups.h"



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

/**
 * Remote task is the task that monitors the IR remote control signals.
 * It can "learn" remote control codes by typing "learn" into the UART0 terminal.
 * Thereafter, if a user enters a 2-digit number through a remote control, then
 * your function handleUserEntry() is called where you can take an action.
 */
class remoteTask : public scheduler_task
{
    public:
        remoteTask(uint8_t priority);   ///< Constructor
        bool init(void);                ///< Inits the task
        bool regTlm(void);              ///< Registers non-volatile variables
        bool taskEntry(void);           ///< One time entry function
        bool run(void *p);              ///< The main loop

    private:
        /** This function is called when a 2-digit number is decoded */
        void handleUserEntry(int num);
        
        /**
         * @param code  The IR code
         * @param num   The matched number 0-9 that mapped the IR code.
         * @returns true if the code has been successfully mapped to the num
         */
        bool getNumberFromCode(uint32_t code, uint32_t& num);

        uint32_t mNumCodes[10];      ///< IR Number codes
        uint32_t mIrNumber;          ///< Current IR number we're decoding
        SemaphoreHandle_t mLearnSem; ///< Semaphore to enable IR code learning
        SoftTimer mIrNumTimer;       ///< Time-out for user entry for 1st and 2nd digit
};

/**
 * Nordic wireless task to participate in the mesh network and handle retry logic
 * such that packets are resent if an ACK has not been received
 */
class wirelessTask : public scheduler_task
{
    public:
        wirelessTask(uint8_t priority) :
            scheduler_task("wireless", 512, priority)
        {
            /* Nothing to init */
        }

        bool run(void *p)
        {
            wireless_service(); ///< This is a non-polling function if FreeRTOS is running.
            return true;
        }
};



namespace konix {

class lightProducerTask : public scheduler_task
{
    public:
        lightProducerTask(EventGroupHandle_t& xLightEventLoop, uint8_t ucPriority);
        bool init(void);
        bool run(void *p);

    private:
        SemaphoreHandle_t mLightLock;
        EventGroupHandle_t pLightEventLoop;
        uint32_t usCount;
        uint64_t ulLightSensorSum;
};

class lightConsumerTask : public scheduler_task
{
    public:
        lightConsumerTask(EventGroupHandle_t& xLightEventLoop, uint8_t ucPriority);
        bool init(void);
        bool run(void *p);

    private:
        EventGroupHandle_t pLightEventLoop;
        FILE* sensorFile;
};

class lightWatchdogTask : public scheduler_task
{
    public:
        lightWatchdogTask(EventGroupHandle_t& xLightEventGroup, uint8_t ucPriority);
        bool run(void *p);
    private:
        bool prvSaveCPUInfo();
        EventGroupHandle_t pLightEventGroup;
        uint32_t ucCPUInfoCount = 0;
};

} // namespace konix

#endif /* TASKS_HPP_ */
