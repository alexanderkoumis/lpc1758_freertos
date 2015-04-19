#include "tasks.hpp"
#include "printf_lib.h"

namespace team9
{

MotorProducerTask::MotorProducerTask(EventGroupHandle_t& xMotorEventGroup, uint8_t priority) :
	    scheduler_task("MotorProducerTask", 512 * 8, priority),
	    pMotorEventLoop(xMotorEventGroup), pwm_in(P1_20), step_count(0)
{
	QueueHandle_t motor_queue = xQueueCreate(1, sizeof(motor_command_t));
	addSharedObject(shared_motorQueueId, motor_queue);
	u0_dbg_printf("Motor Bloater.\r\n");
	pwm_in.setAsInput();
}

bool MotorProducerTask::init(void)
{
	setRunDuration(1);
	return true;
}

bool MotorProducerTask::run(void *p)
{
	motor_command_t motor_command(pwm_in.read(), step_count++);
	QueueHandle_t motor_queue_task = getSharedObject(shared_motorQueueId);
	xQueueSend(motor_queue_task, &motor_command, portMAX_DELAY);
	xEventGroupSetBits(pMotorEventLoop, 0);
	return true;
}

MotorConsumerTask::MotorConsumerTask (EventGroupHandle_t& xMotorEventGroup, uint8_t priority) :
		scheduler_task("MotorConsumerTask", 512 * 8, priority), pMotorEventLoop(xMotorEventGroup)
{
	sys_clk = sys_get_cpu_clock();
}

bool MotorConsumerTask::run(void *p) {
	motor_command_t operation;
	uint32_t counter_max;
	if(xQueueReceive(getSharedObject(shared_motorQueueId), &operation, portMAX_DELAY))
	{
		counter_max = _set_frequency(1);
		_start_counter();

		_poll_end(counter_max);
	}
	return true;
}

uint32_t MotorConsumerTask::_set_frequency(uint32_t freq_hz)
{
	uint32_t cnt_freq = sys_clk / pclk_divider; // 6MHz iff 48MHz / 8.
	uint32_t cnt_setting = (uint32_t) (cnt_freq / (1.0 * steps_per_rotation * freq_hz) + 0.5);  // Round up.
	LPC_MCPWM->MCPER0 = cnt_setting; // Set the Limit register
	LPC_MCPWM->MCPW0 = (uint32_t) cnt_setting / 2.0 + 0.5; // Set the Match register - 50% duty cycle rounded up.
	return cnt_setting;
}

void MotorConsumerTask::_init_motor_pwm()
{
	LPC_SC->PCONP |= (1 << 17);         // Set PCMCPWM - bit 17
	LPC_SC->PCLKSEL1 |= (3 << 30);      // Set PCLK_MCPWM - bits 31:30
	LPC_PINCON->PINSEL3 &= ~(3 << 6);   // Clear p1.19 MC0A0
	LPC_PINCON->PINSEL3 |= (1 << 6);    // Set p1.19 MC0A0
	// Configure the MCCON Register
	LPC_MCPWM->MCCON_CLR = 0xE01F1F0F;  // Clear Everything MCCON
}

void MotorConsumerTask::_poll_end(uint32_t counter_max)
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

void MotorConsumerTask::_stop_counter()
{
	LPC_MCPWM->MCCON_CLR = 0XE01F1F0F;
	LPC_MCPWM->MCTIM0 = 0;
}

void MotorConsumerTask::_start_counter()
{
	LPC_MCPWM->MCTIM0 = 0;
	LPC_MCPWM->MCCON_SET = (1 << 0);
}

} // namespace team9
