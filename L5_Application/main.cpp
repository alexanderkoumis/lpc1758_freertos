#include "tasks.hpp"

int main()
{
	scheduler_add_task(new terminalTask(PRIORITY_LOW));
	scheduler_start();
	return -1;
}
