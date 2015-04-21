#include <cmath>

#include "tasks.hpp"
#include "printf_lib.h"
#include "utilities.h"

SemaphoreHandle_t xMotorSemaphore = NULL;

extern "C"
{
    void PWM1_IRQHandler()
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(xMotorSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

namespace team9
{

typedef enum {
    shared_motor_queue
} sharedHandleId_t;

MotorMasterTask::MotorMasterTask (EventGroupHandle_t& xMotorEventGroup, uint8_t priority) :
        scheduler_task("MotorMaster", 512 * 8, priority),
        pMotorEventGroup(xMotorEventGroup)
{
    QueueHandle_t qh = xQueueCreate(1, sizeof(int));
    addSharedObject(shared_motor_queue, qh);
}

bool MotorMasterTask::run(void *p)
{
//    Rotate(MotorDirection::LEFT, 1);
//    Rotate(MotorDirection::LEFT, 2);
//    Rotate(MotorDirection::LEFT, 3);
//    Rotate(MotorDirection::LEFT, 10);
    return true;
}

void MotorMasterTask::Rotate(MotorDirection direction, uint32_t ulRotations)
{
//    if (xSemaphoreTake(xMotorSemaphore, portMAX_DELAY) == pdTRUE)
//    {
//        LPC_PWM1->IR = 0xff;
//        u0_dbg_printf("LPC_PWM1->IR & (1 << 2) == %8x\n",
//                       LPC_PWM1->IR & (1 << 2));
//        //    QueueHandle_t sensor_queue_task = getSharedObject(shared_motor_queue);
//        //    xQueueSend(sensor_queue_task, &ulRotations, portMAX_DELAY);
//
//    }


}

MotorSlaveTask::MotorSlaveTask (EventGroupHandle_t& xMotorEventGroup, uint8_t priority) :
        scheduler_task("MotorSlave", 512 * 8, priority),
        pMotorEventGroup(xMotorEventGroup), pwm_dir_(P1_20), pwm_en_(P1_23)
{
    sys_clk_ = sys_get_cpu_clock();
    xMotorSemaphore = xSemaphoreCreateBinary();
    _init_motor_pwm();
    _init_motor_dir_en();
    _set_frequency(1);

}

void MotorSlaveTask::_init_motor_dir_en()
{
    pwm_en_.setAsInput();
    pwm_en_.enableOpenDrainMode();
    pwm_en_.setHigh();
    delay_ms(50);
    pwm_dir_.setAsInput();
    pwm_dir_.enableOpenDrainMode();
    pwm_dir_.setHigh();
}

void MotorSlaveTask::_init_motor_pwm()
{
    LPC_SC->PCONP |= (1 << 17); // Set PCMCPWM, bit 17 - UM pg. 64
    LPC_PWM1->TCR = 2;          // Reset PWM TC PSCL counter on next PCLK pos. edge - UM pg. 525
    LPC_PWM1->IR = 0xff;        // Reset any pending PWM interrupts - UM pg. 524
    LPC_SC->PCLKSEL1 |= (3 << 30);    // Set PCLK_MCPWM - bits 31:30 - UM pg. 57
    LPC_PINCON->PINMODE_OD1 |= (1 << 19);// Set P1_19 as open drain - UM pg. 125
    LPC_PINCON->PINSEL3 = (LPC_PINCON->PINSEL3 & ~(3 << 6)) | (1 << 6);
     LPC_MCPWM->MCCON_CLR = 0xE01F1F0F;    // Clear MCCON register
}


void MotorSlaveTask::_set_frequency(uint32_t freq_hz)
{
//  LPC_MCPWM->MCPER0 = cnt_setting;
//  LPC_MCPWM->MCPW0 = (uint32_t) cnt_setting / 2.0 + 0.5;
    uint32_t cnt_freq = sys_clk_ / pclk_divider_;                       // 6MHz iff 48MHz / 8.
    uint32_t cnt_setting = cnt_freq / (steps_per_rot_ * freq_hz) + 0.5; // 15000 iff 6MHz / (400 * 1) + 0.5
    LPC_PWM1->LER = 0x7;                     // PWM1LER (Load enable register)              // UM pg. 529
    LPC_PWM1->MCR = 0x2 | (1 << 6);          // PWMMRS (Match control register stop)        // UM pg. 526
    LPC_PWM1->MR0 = cnt_setting;             // PWMMR0 (Match register 0) - Period, 15000   // UM pg. 74
    LPC_PWM1->MR1 = cnt_setting / 2.0 + 0.5; // PWMMR1 (Match register 1) - Duty cycle, 50% // UM pg. 74
    LPC_PWM1->MR2 = cnt_setting - 1000;      // PWMMR2 (Match register 2) - 1000ms
    LPC_PWM1->PCR = 1 << 9;                  // PWMENA1 (Output enable)                     // UM pg. 528
    LPC_PWM1->TCR = (1 << 3) | 1;     // PWM1TCR (Timer Control Register)            // UM pg. 523
}

bool MotorSlaveTask::run(void *p)
{
    while (true)
    {
        if (xSemaphoreTake(xMotorSemaphore, portMAX_DELAY) == pdTRUE) {
            _start_counter();
            delay_ms(1000);
            _stop_counter();
            LPC_PWM1->TCR |= (1 << 1);     // PWM1TCR (Timer Control Register)            // UM pg. 523
        }
    }
    return true;
}


void MotorSlaveTask::_start_counter()
{
    LPC_MCPWM->MCTIM0 = 0;
    LPC_MCPWM->MCCON_SET = (1 << 0);
}

void MotorSlaveTask::_stop_counter()
{
    LPC_MCPWM->MCCON_CLR = 0XE01F1F0F;
    LPC_MCPWM->MCTIM0 = 0;
}

} // namespace team9
