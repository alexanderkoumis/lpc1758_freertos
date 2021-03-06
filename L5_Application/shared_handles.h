/*
 *     SocialLedge.com - Copyright (C) 2014
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
 *
 * @file
 * @brief Contains enumeration of shared handles used in the project.
 * @see   scheduler_task::addSharedHandle()
 */
#ifndef SHARED_HANDLES_H__
#define SHARED_HANDLES_H__



/**
 * Enumeration of shared handles
 * You can add additional IDs here to use addSharedHandle() and getSharedHandle() API
 */

struct PixyCmd_t
{
    bool bReset;
    int lColumn;
    int lColor;
};

enum {
    shared_MotorQueueRX,     ///< Shared handle used between command handler and terminal
    shared_MotorQueueTX,
    shared_GameQueueRX,
    shared_GameQueueTX,
    shared_ServoQueue,
    shared_SensorQueue,    ///< Shared handle used by examples (producer and consumer tasks)
    shared_learnSemaphore, ///< Terminal command gives this semaphore to remoteTask (IR sensor task)
    shared_PixyQueueTX,
    shared_PixyQueueRX,
    shared_PixyResetQueueTX,
    shared_PixyResetQueueRX,
    shared_KillPixyQueue
};



#endif /* SHARED_HANDLES_H__ */
