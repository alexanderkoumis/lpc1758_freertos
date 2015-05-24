#include "tasks.hpp"

int main(void)
{
	scheduler_add_task(new terminalTask(PRIORITY_MEDIUM));
	scheduler_add_task(new team9::MotorTask_t(PRIORITY_MEDIUM));
	scheduler_add_task(new team9::GameTask_t(PRIORITY_MEDIUM));
	scheduler_add_task(new team9::pixy::PixyTask_t(PRIORITY_HIGH));
    scheduler_start();
    return -1;
}
