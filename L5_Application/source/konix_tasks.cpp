#include "tasks.hpp"

#include <stdio.h>
#include <inttypes.h>

#include "storage.hpp"

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
    this->ulLightSensorSum += pLightSensor.getRawValue();
    this->usCount++;
    if (this->usCount == 100)
    {
        xSemaphoreTake(this->mLightLock, portMAX_DELAY);
        uint32_t usAverageLight = this->ulLightSensorSum / 100;
        this->ulLightSensorSum = 0;
        QueueHandle_t sensor_queue_task = getSharedObject(shared_sensor_queue);
        xQueueSend(sensor_queue_task, &usAverageLight, portMAX_DELAY);
        printf("Pushed 100ms-avg light %" PRIu32 " to the queue\n", usAverageLight);
        this->usCount = 0;
        xEventGroupSetBits(pLightEventLoop, 0);
        xSemaphoreGive(this->mLightLock);
    }
    return true;
}

lightConsumerTask::lightConsumerTask(EventGroupHandle_t& xLightEventGroup, uint8_t ucPriority) :
    scheduler_task("light_consumer", 3 * 512, ucPriority)
{
    this->pLightEventLoop = xLightEventGroup;
    this->sensorFile = fopen("0:sensor.txt", "w");
    printf("sensor.txt opened.\n");
}

bool lightConsumerTask::init(void)
{
    setRunDuration(500);
    return true;
}

bool lightConsumerTask::run(void *p)
{
    uint32_t data = 0;
    uint32_t ulTime = xTaskGetTickCount();
    xQueueReceive(getSharedObject(shared_sensor_queue), &data, 0);
    printf("Saving: '%" PRIu32 " %" PRIu32 "' to file\n", ulTime, data);
    fprintf(this->sensorFile, "%" PRIu32 " %" PRIu32 "\n", ulTime, data);
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
    if (uxBits)
    {
        printf("Bits cleared successfully\n");
        if (this->ucCPUInfoCount++ == 60)
        {
            return this->prvSaveCPUInfo();
            this->ucCPUInfoCount = 0;
        }
    }
    else
    {
        FILE* stuckFile;
        stuckFile = fopen("0:stuck.txt", "w");
        printf("Bits not cleared... stuck!\n");
        fprintf(stuckFile, "Possible stuck process: %s\n", LPC_RTC->GPREG0);
        fclose(stuckFile);
    }
    return true;
}

bool lightWatchdogTask::prvSaveCPUInfo()
{
    FILE* cpuFile;
    const unsigned long uMaxTasks = 16;
    TaskStatus_t taskStatusTbl[uMaxTasks];
    uint32_t ulTotalRunTime = 0;
    uint32_t ulTasksRunTime = 0;
    const unsigned long uxArraySize =
            uxTaskGetSystemState(&taskStatusTbl[0], uMaxTasks, &ulTotalRunTime);

    cpuFile = fopen("0:cpu.txt", "w");
    printf("Saving to log:\n");
    printf("Saving: \"%10s Sta Pr Stack CPU%%          Time\"\n", "Name");
    fprintf(cpuFile, "%10s Sta Pr Stack CPU%%          Time\n", "Name");
    for(unsigned priorityNum = 0; priorityNum < configMAX_PRIORITIES; priorityNum++)
    {
        for (unsigned i = 0; i < uxArraySize; i++)
        {
            TaskStatus_t *e = &taskStatusTbl[i];
            if (e->uxBasePriority == priorityNum)
            {
                ulTasksRunTime += e->ulRunTimeCounter;

                const uint32_t ulCPUPercent = (0 == ulTotalRunTime) ? 0 : e->ulRunTimeCounter / (ulTotalRunTime/100);
                const uint32_t ulTimeUs = e->ulRunTimeCounter;
                const uint32_t ulStackInBytes = (4 * e->usStackHighWaterMark);
                printf("Saving: \"%10s %s %2u %5u %4u %10u us\"\n",
                       e->pcTaskName, taskStatusTbl[e->eCurrentState], e->uxBasePriority,
                       ulStackInBytes, ulCPUPercent, ulTimeUs);
                fprintf(cpuFile, "%10s %s %2u %5u %4u %10u us\n",
                              e->pcTaskName, taskStatusTbl[e->eCurrentState], e->uxBasePriority,
                              ulStackInBytes, ulCPUPercent, ulTimeUs);
            }
        }
    }

    const uint32_t ulOverheadUs = (ulTotalRunTime - ulTasksRunTime);
    const uint32_t ulOverheadPercent = ulOverheadUs / (ulTotalRunTime / 100);
    printf("%10s --- -- ----- %4u %10u uS\n(overhead)", ulOverheadPercent, ulOverheadUs);

    if (fclose(cpuFile) != 0) {
        printf ("Error closing cpu.txt!\n");
        return false;
    }
    return true;
}

}
