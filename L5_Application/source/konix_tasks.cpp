#include "tasks.hpp"

#include <stdio.h>

#include "queue.h"

namespace konix {

typedef enum {
    shared_sensor_queue
} sharedHandleId_t;

static __inline unsigned long int BIT_N(int shift_by)
{
    return (1 << shift_by);
}

lightProducerTask::lightProducerTask(EventGroupHandle_t& xLightEventGroup, uint8_t ucPriority) :
        scheduler_task("light_producer", 3 * 512, ucPriority),
        mLightLock(0), usCount(0), ulLightSensorSum(0)
{
    pLightEventLoop = xLightEventGroup;
    Light_Sensor::getInstance().init();
    QueueHandle_t qh = xQueueCreate(1, sizeof(int));
    addSharedObject(shared_sensor_queue, qh);
}

bool lightProducerTask::init(void)
{
    mLightLock = xSemaphoreCreateMutex();
    setRunDuration(1);
    return true;
}

bool lightProducerTask::run(void *p)
{
    Light_Sensor& pLightSensor = Light_Sensor::getInstance();
    ulLightSensorSum += pLightSensor.getRawValue();
    usCount++;
    if (usCount == 100)
    {
        xSemaphoreTake(this->mLightLock, portMAX_DELAY);
        uint32_t usAverageLight = ulLightSensorSum / 100;
        ulLightSensorSum = 0;
        QueueHandle_t sensor_queue_task = getSharedObject(shared_sensor_queue);
        xQueueSend(sensor_queue_task, &usAverageLight, portMAX_DELAY);
        printf("Pushed 100ms-avg light %u to the queue\n", (int)usAverageLight);
        usCount = 0;
        xSemaphoreGive(this->mLightLock);
    }
    xEventGroupSetBits(pLightEventLoop, 0);
    return true;
}

lightConsumerTask::lightConsumerTask(EventGroupHandle_t& xLightEventGroup, uint8_t ucPriority) :
    scheduler_task("light_consumer", 3 * 512, ucPriority)
{
    this->pLightEventLoop = xLightEventGroup;
    this->sensorFile = fopen("sensor.txt", "r+");
}

bool lightConsumerTask::init(void)
{
    setRunDuration(500);
    return true;
}

bool lightConsumerTask::run(void *p)
{
    uint32_t data = 0;
    xQueueReceive(getSharedObject(shared_sensor_queue), &data, 0);
    printf("Saving: \"%i, %i\" to file\n", (int)xTaskGetTickCount(), (int)data);
    fprintf(this->sensorFile, "%i, %i\n", (int)xTaskGetTickCount(), (int)data);
    xEventGroupSetBits(pLightEventLoop, 1);
    return true;
}

lightWatchdogTask::lightWatchdogTask(EventGroupHandle_t& xLightEventGroup, uint8_t ucPriority) :
        scheduler_task("light_watchdog", 2048, ucPriority)
{
    this->pLightEventGroup = xLightEventGroup;
}

bool lightWatchdogTask::run(void *p)
{
    EventBits_t uxBits = xEventGroupWaitBits(pLightEventGroup, BIT_N(0) | BIT_N(1), pdTRUE, pdTRUE, 1000);
    if((uxBits & (BIT_N(0) | BIT_N(1))) == (BIT_N(0) | BIT_N(1))) {
        printf("Both pins OK!\n");
    }
}

}
