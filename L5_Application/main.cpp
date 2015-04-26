#include "tasks.hpp"

int main() {
	scheduler_add_task(new team9::PixyTask_t(PRIORITY_MEDIUM));
	scheduler_start();
	return -1;
}
