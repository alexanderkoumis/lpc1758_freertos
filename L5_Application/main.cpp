#include "tasks.hpp"

static EventGroupHandle_t xMotorEventGroup;

int main(void)
{
	xMotorEventGroup = xEventGroupCreate();
	scheduler_add_task(new terminalTask(PRIORITY_MEDIUM));
	scheduler_add_task(new team9::ControllerTask(xMotorEventGroup, PRIORITY_HIGH));
	scheduler_add_task(new team9::MotorTask(xMotorEventGroup, PRIORITY_HIGH));
	scheduler_start();
	return -1;
}
