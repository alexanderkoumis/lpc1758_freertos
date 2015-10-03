#ifndef PIXY_COMMON_HPP
#define PIXY_COMMON_HPP

#include <cstdio> // snprint
#include <cstring>
#include <cmath>
#include <cassert>
#include <vector>
#include <iostream>
#include <string>     // string
#include <map>        // map
#include <sstream>    // ostringstream
#include <tuple>      // tie
#include <functional> // function
#include <algorithm>  // transform

#include "printf_lib.h"

#define F() printf("Line: %d, func: %s\n", __LINE__, __func__); u0_dbg_printf("Line: %d, func: %s\n", __LINE__, __func__)

namespace team9
{
namespace pixy
{

enum ChipColor_t
{
    NONE = 0, GREEN = 1, RED = 2
};

enum Quadrant_t
{
    TOP_LEFT = 0, TOP_RIGHT = 1, BOT_LEFT = 2, BOT_RIGHT = 3, ERROR = 4
};

enum StatEnum_t
{
    M, V, S, C
};

} // namespace pixy
} // namespace team9

#endif
