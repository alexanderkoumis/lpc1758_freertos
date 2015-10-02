#ifndef PIXY_MOUTH_HPP
#define PIXY_MOUTH_HPP

#include "printf_lib.h"

#include "pixy/common.hpp"

namespace team9
{
namespace pixy
{

class PixyMouth_t
{
	public:
		PixyMouth_t()
		{}

		bool xEmitUpdate(const int lColUpdate)
		{
		    if (lColUpdate < 0)
		    {
		        return false;
		    }
		    printf("Trying to send %d over queue\n", lColUpdate);
            u0_dbg_printf("Trying to send %d over queue\n", lColUpdate);

		    printf("player move A5B6_%d\n", lColUpdate);
		    u0_dbg_printf("player move A5B6_%d\n", lColUpdate);

		    QueueHandle_t xQueueTXHandle =
		            scheduler_task::getSharedObject(shared_PixyQueueTX);
		    xQueueSend(xQueueTXHandle, &lColUpdate, portMAX_DELAY);
		    return true;
		}
};

} // namespace pixy
} // namespace team9

#endif
