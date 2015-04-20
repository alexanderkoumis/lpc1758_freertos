#include "tasks.hpp"
#include "printf_lib.h"

namespace team9
{

MotorTask::MotorTask (EventGroupHandle_t& xMotorEventGroup, uint8_t priority) :
        scheduler_task("MotorTask", 512 * 8, priority), pwm_dir_(P1_20), pwm_en_(P1_23)
{
    sys_clk_ = sys_get_cpu_clock();
    _init_motor_dir_en();
    _init_motor_pwm();
}

void MotorTask::_init_motor_dir_en()
{
    pwm_dir_.setAsInput();
    pwm_dir_.enableOpenDrainMode();
    pwm_dir_.setHigh();
    pwm_en_.setAsInput();
    pwm_en_.enableOpenDrainMode();
    pwm_en_.setHigh();
}

void MotorTask::_init_motor_pwm()
{
    LPC_SC->PCONP |= (1 << 17);           // Set PCMCPWM - bit 17
    LPC_SC->PCLKSEL1 |= (3 << 30);        // Set PCLK_MCPWM - bits 31:30
    LPC_PINCON->PINMODE_OD1 |= (1 << 19); // Set P1_19 as open drain
    LPC_PINCON->PINSEL3 &= ~(3 << 6);     // Clear p1.19 MC0A0
    LPC_PINCON->PINSEL3 |= (1 << 6);      // Set p1.19 MC0A0
    LPC_MCPWM->MCCON_CLR = 0xE01F1F0F;    // Clear MCCON register
}

bool MotorTask::run(void *p) {
    MotorCommand motor_command;
    uint32_t counter_max;
    if(xQueueReceive(getSharedObject(shared_motorQueueId), &motor_command, portMAX_DELAY))
    {
        int num_steps = motor_command.steps_ * steps_per_rot_;
        for (int i = 0; i < num_steps; ++i) {
            counter_max = _set_frequency(1);
            _start_counter();
            _poll_end(counter_max);
        }
    }
    return true;
}

uint32_t MotorTask::_set_frequency(uint32_t freq_hz)
{
    uint32_t cnt_freq = sys_clk_ / pclk_divider_; // 6MHz iff 48MHz / 8.
    uint32_t cnt_setting = (uint32_t) (cnt_freq / (1.0 * steps_per_rot_ * freq_hz) + 0.5);  // Round up.
    LPC_MCPWM->MCPER0 = cnt_setting; // Set channel 0 limit register
    LPC_MCPWM->MCPW0 = (uint32_t) cnt_setting / 2.0 + 0.5; // Set channel 0 match register - 50% duty cycle rounded up.
    return cnt_setting;
}

void MotorTask::_start_counter()
{
    LPC_MCPWM->MCTIM0 = 0;
    LPC_MCPWM->MCCON_SET = (1 << 0);
}

void MotorTask::_stop_counter()
{
    LPC_MCPWM->MCCON_CLR = 0XE01F1F0F;
    LPC_MCPWM->MCTIM0 = 0;
}

void MotorTask::_poll_end(uint32_t counter_max)
{
    // This method triggers the stop condition for the motor controller.
    // Using upper and lower 10% of max counter value to trigger events.
    bool has_seen_upper_thresh = false;
    uint32_t upper_thresh = counter_max * 0.9;
    uint32_t lower_thresh = counter_max * 0.1;
    while(1 == 1)
    {
        if(LPC_MCPWM->MCTIM0 >= upper_thresh)
        {
            has_seen_upper_thresh = true;
        }
        if(has_seen_upper_thresh && LPC_MCPWM->MCTIM0 <= lower_thresh)
        {
            has_seen_upper_thresh = false;
            _stop_counter();
            break;
        }
    }
}

} // namespace team9
