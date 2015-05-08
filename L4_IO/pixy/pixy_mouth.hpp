#ifndef PIXY_MOUTH_HPP
#define PIXY_MOUTH_HPP

#include "pixy/pixy_common.hpp"

namespace team9
{
namespace pixy
{

class PixyMouth_t
{
	public:
		PixyMouth_t()
		{}

		bool xEmitUpdate(int lColUpdate)
		{
		    if (lColUpdate < 0)
		    {
		        return false;
		    }
		}
};

} // namespace pixy
} // namespace team9

#endif
