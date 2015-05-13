#ifndef CORNERS_HPP
#define CORNERS HPP

#include <vector>

#include "pixy/common.hpp"
#include "pixy/common/block.hpp"
#include "pixy/common/point.hpp"
#include "pixy/common/stat.hpp"

namespace team9
{
namespace pixy
{

// TODO: Refactor to use PointStat_t, deprecate Stat_t
class Corners_t
{
    public:
        Corners_t()
        {
            xStats.resize(8);
        }

        void vReset()
        {
            for (auto& xStat : xStats)
            {
                xStat.vReset();
            }
        }

        Point_t<float> operator() (Quadrant_t xQuadrant) const
        {
            return Point_t<float>(xStats[2*xQuadrant].xMean(),
                                  xStats[2*xQuadrant+1].xMean());
        }

        void vUpdate(Quadrant_t& xQuadrant, Block_t& xBlock)
        {
            xStats[2*xQuadrant].vUpdate(xBlock.xPoint.xY);
            xStats[2*xQuadrant+1].vUpdate(xBlock.xPoint.xX);
        }

        static std::string xCornerStr(const Corners_t& xCorners,
                                      Quadrant_t xQuadrant)
        {
            static const uint32_t ulBuffSize = 32;
            char buff[ulBuffSize];
            snprintf(buff, ulBuffSize, "[%3.2f %3.2f]",
                     xCorners.xStats[2*xQuadrant].xMean(),
                     xCorners.xStats[2*xQuadrant+1].xMean());
            return std::string(buff);
        }

        void vPrint()
        {
            std::cout << std::noskipws << "[" << std::endl
                      << "    " << xCornerStr(*this, TOP_LEFT)
                      << " " << xCornerStr(*this, TOP_RIGHT) << std::endl
                      << "    " << xCornerStr(*this, BOT_LEFT)
                      << " " << xCornerStr(*this, BOT_RIGHT) << std::endl
                      << "]" << std::endl;
        }

    private:


        std::vector<StatEMA_t> xStats;
};

} // namespace pixy
} // namespace team9


#endif
