#include "tasks.hpp"
#include "printf_lib.h"

namespace team9
{

ControllerTask::ControllerTask(EventGroupHandle_t& xMotorEventGroup, uint8_t priority) :
	    scheduler_task("ControllerTask", 512 * 8, priority),
	    pMotorEventLoop(xMotorEventGroup)
{
	QueueHandle_t motor_queue = xQueueCreate(1, sizeof(MotorCommand));
	addSharedObject(shared_motorQueueId, motor_queue);
	setRunDuration(5000);
	u0_dbg_printf("Motor Bloater.\r\n");
}

bool ControllerTask::run(void *p)
{
	MotorCommand motor_command(MotorCommand::LEFT, 5);
	QueueHandle_t motor_queue_task = getSharedObject(shared_motorQueueId);
	xQueueSend(motor_queue_task, &motor_command, portMAX_DELAY);
	xEventGroupSetBits(pMotorEventLoop, 0);
	return true;
}

} // namespace team9
