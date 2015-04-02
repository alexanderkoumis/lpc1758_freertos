#include "tasks.hpp"

int main() {
	scheduler_add_task(new konix::pixyTask(PRIORITY_MEDIUM));
	scheduler_start();
	return -1;
}
