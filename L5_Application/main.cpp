#include "tasks.hpp"

static EventGroupHandle_t xLightEventGroup;

int main(void)
{
    xLightEventGroup = xEventGroupCreate();
    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
    scheduler_add_task(new konix::lightProducerTask(xLightEventGroup, PRIORITY_MEDIUM));
    scheduler_add_task(new konix::lightConsumerTask(xLightEventGroup, PRIORITY_LOW));
    scheduler_add_task(new konix::lightWatchdogTask(xLightEventGroup, PRIORITY_HIGH));
    scheduler_start();
    return -1;
}
