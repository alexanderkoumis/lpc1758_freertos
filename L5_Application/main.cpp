#include "pixy.hpp"
#include "tasks.hpp"

int main() {
	scheduler_add_task(new team9::PixyTask_t(PRIORITY_HIGH));
	u0_dbg_printf("sup\n");
	scheduler_start();
	return -1;
}
