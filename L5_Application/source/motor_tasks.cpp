#include "tasks.hpp"
#include "printf_lib.h"
#include "utilities.h"
#include "shared_handles.h" // shared_MotorQueue
#include "pixy/common.hpp"
#include <stdio.h>

namespace team9
{

MotorTask_t::MotorTask_t (uint8_t ucPriority) :
        scheduler_task("MotorSlave", 512 * 8, ucPriority),
        xPWM_DIR(P1_20), xPWM_EN(P1_23), ulSysClk(48000000)
{
    QueueHandle_t xQHandleRX = xQueueCreate(1, sizeof(xMotorCommand_t));
    QueueHandle_t xQueueHandleTX = xQueueCreate(1, sizeof(bool));
    addSharedObject(shared_MotorQueueRX, xQHandleRX);
    addSharedObject(shared_MotorQueueTX, xQueueHandleTX);
    ulSysClk = sys_get_cpu_clock();
    vInitGPIO();
    vInitPWM();
    ulSetFrequency(xMotorFreq);
}

void MotorTask_t::vInitGPIO()
{
    xPWM_EN.setAsInput();
    xPWM_EN.enableOpenDrainMode();
    xPWM_EN.setLow();
    delay_ms(1000);
    xPWM_DIR.setAsInput();
    xPWM_DIR.enableOpenDrainMode();
    xPWM_DIR.setLow();
    delay_ms(1000);
    //xPWM_EN.setLOW();
}

void MotorTask_t::vInitPWM()
{
    LPC_SC->PCONP |= (1 << 17);           // Set PCMCPWM - bit 17
    LPC_SC->PCLKSEL1 |= (3 << 30);        // Set PCLK_MCPWM - bits 31:30
    LPC_PINCON->PINMODE_OD1 |= (1 << 19); // Set P1_19 as open drain
    LPC_PINCON->PINSEL3 &= ~(3 << 6);     // Clear p1.19 MC0A0
    LPC_PINCON->PINSEL3 |= (1 << 6);      // Set p1.19 MC0A0
    LPC_MCPWM->MCCON_CLR = 0xE01F1F0F;    // Clear MCCON register
}

bool MotorTask_t::run(void *p)
{
    F();
    xMotorCommand_t xMotorCommandRX;
    bool xMotorCommandTX = true;
    if (xQueueReceive(getSharedObject(shared_MotorQueueRX),
                      &xMotorCommandRX, portMAX_DELAY))
    {
        xPWM_EN.setHigh();
        delay_ms(10);
        xPWM_DIR.set(xMotorCommandRX.eDirection == eDirection_t::LEFT ? true : false);
        delay_ms(10);
        uint32_t ulCyclesPerStep = ulSetFrequency(xMotorFreq);
        vStartCounter();
        vPollEnd(ulCyclesPerStep, xMotorCommandRX);
        xPWM_DIR.setLow();
        xPWM_EN.setLow();
        // Indicate we've finished.
        xQueueSend(getSharedObject(shared_MotorQueueTX), &xMotorCommandTX, portMAX_DELAY);
    }
    return true;
}

uint32_t MotorTask_t::ulSetFrequency(float ulFreqHz)
{
    float ulCntFreq = (1.0 * ulSysClk) / lPclkDivider;  // 6MHz iff 48MHz / 8
    uint32_t ulLimitReg = ulCntFreq / (lStepsPerRot * ulFreqHz) + 0.5;
    LPC_MCPWM->MCPER0 = ulLimitReg;                 // Setting Limit register
    LPC_MCPWM->MCPW0 = ulLimitReg / 2.0 + 0.5;      // Setting Match register
    return ulLimitReg;
}

void MotorTask_t::vPollEnd(uint32_t ulCyclesPerStep,
                           xMotorCommand_t& xMotorCommand)
{
    // This method triggers the stop condition for the motor controller.
    // Using upper and lower 10% of max counter value to trigger events.
    bool bSeenUpperThresh = false;
    uint32_t ulUpperThresh = ulCyclesPerStep * 0.9;
    uint32_t ulLowerThresh = ulCyclesPerStep * 0.1;
    uint32_t ulStepCount = 0;
    uint32_t ulStepMax = lStepsPerRot * xMotorCommand.xRotations;

    while(1 == 1)
    {
        if(LPC_MCPWM->MCTIM0 >= ulUpperThresh)
        {
            bSeenUpperThresh = true;
        }
        if(bSeenUpperThresh && LPC_MCPWM->MCTIM0 <= ulLowerThresh)
        {
            bSeenUpperThresh = false;
            if (ulStepCount > ulStepMax)
            {
                vStopCounter();
                break;
            }
            ulStepCount++;
        }
    }
    xPWM_DIR.setLow();
}

void MotorTask_t::vStartCounter()
{
    LPC_MCPWM->MCTIM0 = 0;
    LPC_MCPWM->MCCON_SET = (1 << 0);
}

void MotorTask_t::vStopCounter()
{
    LPC_MCPWM->MCCON_CLR = 0XE01F1F0F;
    LPC_MCPWM->MCTIM0 = 0;
}

} // namespace team9
