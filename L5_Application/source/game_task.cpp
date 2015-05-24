#include <stdio.h>

#include "tasks.hpp"
#include "utilities.h"
#include "shared_handles.h"

#include "pixy/common.hpp"

namespace team9
{

GameTask_t::GameTask_t (uint8_t ucPriority) :
		scheduler_task("ServoSlave", 512*8, ucPriority)
{
    QueueHandle_t xQServoHandle = xQueueCreate(1, sizeof(int));
    QueueHandle_t xQGameHandleTX = xQueueCreate(1, sizeof(bool));
    QueueHandle_t xQGameHandleRX = xQueueCreate(1, sizeof(GameCommand_t));

    addSharedObject(shared_ServoQueue, xQServoHandle);
    addSharedObject(shared_GameQueueTX, xQGameHandleTX);
    addSharedObject(shared_GameQueueRX, xQGameHandleRX);

    xServo = new PWM(PWM::pwm2, 50);
    xServo->set(xClosedPWM);
}

void GameTask_t::vRunServo(int lDropCount)
{
    for(int lI = 0; lI < lDropCount; ++lI)
    {
        xServo->set(xOpenPWM);
        vTaskDelayMs(1000*0.65);
        xServo->set(xClosedPWM);
        vTaskDelayMs(1000*1);
    }
}

void GameTask_t::vRunStepper(uint8_t ucInsertCol)
{
    xMotorCommand_t xMotorCommand;
    QueueHandle_t xMotorQueueRX;
    QueueHandle_t xMotorQueueTX;

    bool bInsert;
    float xRotations = (0.4833 * (ucInsertCol) + 1.6);

    // Move over Column from home
    xMotorQueueRX = getSharedObject(shared_MotorQueueRX);
    xMotorCommand.Load(eDirection_t::LEFT, xRotations);
    xQueueSend(xMotorQueueRX, &xMotorCommand, portMAX_DELAY);

    // Wait till we're over the column.
    xMotorQueueTX = getSharedObject(shared_MotorQueueTX);
    xQueueReceive(xMotorQueueTX, &bInsert, portMAX_DELAY);

    // Drop the chip into the board.
    this->vRunServo(1);

    // Send the platform back home.
    xMotorCommand.Load(eDirection_t::RIGHT, xRotations);
    xQueueSend(xMotorQueueRX, &xMotorCommand, portMAX_DELAY);

    // Wait for Home
    xQueueReceive(xMotorQueueTX, &bInsert, portMAX_DELAY);
}

bool GameTask_t::run(void *p)
{
    GameCommand_t xGameCommand;
    PixyCmd_t xPixyCmd;

    QueueHandle_t xPixyTXHandle;
    QueueHandle_t xPixyRXHandle;

    int lHumanCol = 0;

    // Receiving human chip insertion
    printf("WAITING FOR SOME SHIT\n");
    u0_dbg_printf("WAITING FOR SOME OTHER SHIT\n");

    xPixyTXHandle = scheduler_task::getSharedObject(shared_PixyQueueTX);
    xQueueReceive(xPixyTXHandle, &lHumanCol, portMAX_DELAY);


    std::cout << "player move big dicks" << std::endl;

    if (xQueueReceive(getSharedObject(shared_GameQueueRX), &xGameCommand,
    				  portMAX_DELAY))
    {
//        if(xGameCommand.eGame == eGame_t::COMPETE)
//        {
//            printf("player move A5B6_%d\n", lHumanCol);
//        }
//        else if(xGameCommand.eGame == eGame_t::DEBUG)
//        {
//            printf("machine move A5B6_%d\n", xGameCommand.ucCol);
//        }
        this->vRunStepper(xGameCommand.ucCol);
    }

    // Informing Pixy of robot's chip insertion
    xPixyCmd.lColor = pixy::ChipColor_t::RED;
    xPixyCmd.lColumn = xGameCommand.ucCol;

    xPixyRXHandle = scheduler_task::getSharedObject(shared_PixyQueueRX);
    xQueueSend(xPixyRXHandle, &xPixyCmd, portMAX_DELAY);

    return true;
}

} // End team9 namespacee
