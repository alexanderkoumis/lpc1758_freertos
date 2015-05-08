
#include "tasks.hpp"
#include "utilities.h"
#include "shared_handles.h"
#include <stdio.h>


namespace team9
{

GameTask_t::GameTask_t (uint8_t ucPriority) : scheduler_task("ServoSlave", 512*8, ucPriority)
{
    QueueHandle_t xQServoHandle = xQueueCreate(1, sizeof(int));
    QueueHandle_t xQGameHandleTX = xQueueCreate(1, sizeof(bool));
    QueueHandle_t xQGameHandleRX = xQueueCreate(1, sizeof(xGameCommand_t));

    addSharedObject(shared_ServoQueue, xQServoHandle);
    addSharedObject(shared_GameQueueTX, xQGameHandleTX);
    addSharedObject(shared_GameQueueRX, xQGameHandleRX);

    my_servo = new PWM(PWM::pwm2, 50);
    my_servo->set(closed_pwm);
}

void GameTask_t::run_servo(int drop_count)
{
    int i=0;
    for(i=0; i<drop_count; i++)
    {
        my_servo->set(open_pwm);
        vTaskDelayMs(1000*0.65);
        my_servo->set(closed_pwm);
        vTaskDelayMs(1000*1);
    }
}

void GameTask_t::run_stepper(uint8_t insert_column)
{
    xMotorCommand_t xMotorCommand;
    QueueHandle_t xMotorQueueRX;
    QueueHandle_t xMotorQueueTX;
    bool insert;
    float xRotations = (0.5 * insert_column);
    // Move over Column from home

    xMotorQueueRX = getSharedObject(shared_MotorQueueRX);
    xMotorCommand.Load(eDirection_t::LEFT, xRotations);
    xQueueSend(xMotorQueueRX, &xMotorCommand, portMAX_DELAY);

    // Wait till we're over the column.
    xMotorQueueTX = getSharedObject(shared_MotorQueueTX);
    xQueueReceive(xMotorQueueTX, &insert, portMAX_DELAY);

    // Drop the chip into the board.
    this->run_servo(1);

    // Send the platform back home.
    xMotorCommand.Load(eDirection_t::RIGHT, xRotations);
    xQueueSend(xMotorQueueRX, &xMotorCommand, portMAX_DELAY);
    // Wait for Home
    xQueueReceive(xMotorQueueTX, &insert, portMAX_DELAY);
}


bool GameTask_t::run(void *p)
{
    int xServoCommand;
    int xGameCommand;

    xGameCommand_t game_c;

    uint8_t xColumn;
    int steps;
    bool sent = false;

    if (xQueueReceive(getSharedObject(shared_GameQueueRX), &game_c, portMAX_DELAY))
    {
        this->run_stepper(game_c.xColumn);
    }
    if(game_c.eGame == eGame_t::compete) {
        printf("player move A5B6_%d\n", game_c.xColumn);
    } else if(game_c.eGame == eGame_t::debug) {
        printf("machine move A5B6_%d\n", game_c.xColumn);
    }
    return true;
}

} // End team9 namespacee
