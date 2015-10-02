#ifndef CORNERS_HPP
#define CORNERS HPP

#include <vector>

#include "storage.hpp"

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
            printf("Reset %d stats\n", xStats.size());
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

        static char* pcCornerStrRaw(const Corners_t& xCorners)
        {
            if ((int)xCorners.xStats.size() == 8)
            {
                static const uint32_t ulBuffSize = 256;
                char buff[ulBuffSize] = "";
                snprintf(buff, ulBuffSize,
                         "%3.2f %3.2f %3.2f %3.2f %3.2f %3.2f %3.2f %3.2f",
                          xCorners.xStats[2 * TOP_LEFT].xMean(),
                          xCorners.xStats[2 * TOP_LEFT + 1].xMean(),
                          xCorners.xStats[2 * TOP_RIGHT].xMean(),
                          xCorners.xStats[2 * TOP_RIGHT + 1].xMean(),
                          xCorners.xStats[2 * BOT_LEFT].xMean(),
                          xCorners.xStats[2 * BOT_LEFT + 1].xMean(),
                          xCorners.xStats[2 * BOT_RIGHT].xMean(),
                          xCorners.xStats[2 * BOT_RIGHT + 1].xMean());
                return buff;
            }
            std::cout << "Error, need 8 corners to return xCornerStrRaw";
            return "";
        }

        static bool bReadCorners(const char* calib_file_path, Corners_t& xCorners)
        {
            char corner_str[256] = "";
            if (Storage::read("/corners.calib", corner_str, 256, 0) == FR_NO_FILE)
            {
                printf("/corners.calib doesn't exist\n");
                u0_dbg_printf("/corners.calib doesn't exist\n");
                return false;
            }
            float tl_y;
            float tl_x;
            float tr_y;
            float tr_x;
            float bl_y;
            float bl_x;
            float br_y;
            float br_x;
            int corner_tokens = sscanf(corner_str,
                                "%f %f %f %f %f %f %f %f",
                                &tl_y, &tl_x,
                                &tr_y, &tr_x,
                                &bl_y, &bl_x,
                                &br_y, &br_x);
            if (corner_tokens == 8)
            {
                xCorners.vReset();
                xCorners.xStats[2 * TOP_LEFT].vSetMean(tl_y);
                xCorners.xStats[2 * TOP_LEFT + 1].vSetMean(tl_x);
                xCorners.xStats[2 * TOP_RIGHT].vSetMean(tr_y);
                xCorners.xStats[2 * TOP_RIGHT + 1].vSetMean(tr_x);
                xCorners.xStats[2 * BOT_LEFT].vSetMean(bl_y);
                xCorners.xStats[2 * BOT_LEFT + 1].vSetMean(bl_x);
                xCorners.xStats[2 * BOT_RIGHT].vSetMean(br_y);
                xCorners.xStats[2 * BOT_RIGHT + 1].vSetMean(br_x);
                return true;
            }
            printf("Error reading corners "
                   "(read %d floats instead of 8\n)",
                   corner_tokens);
            return false;
        }

        static void vPrint(const Corners_t& xCorners)
        {
            printf("[\n"
                   "\t[%f %f] [%f %f]\n"
                   "\t[%f %f] [%f %f]\n"
                   "]\n",
                    xCorners.xStats[2 * TOP_LEFT].xMean(),
                    xCorners.xStats[2 * TOP_LEFT + 1].xMean(),
                    xCorners.xStats[2 * TOP_RIGHT].xMean(),
                    xCorners.xStats[2 * TOP_RIGHT + 1].xMean(),
                    xCorners.xStats[2 * BOT_LEFT].xMean(),
                    xCorners.xStats[2 * BOT_LEFT + 1].xMean(),
                    xCorners.xStats[2 * BOT_RIGHT].xMean(),
                    xCorners.xStats[2 * BOT_RIGHT + 1].xMean());
            u0_dbg_printf("[\n"
                   "\t[%f %f] [%f %f]\n"
                   "\t[%f %f] [%f %f]\n"
                   "]\n",
                    xCorners.xStats[2 * TOP_LEFT].xMean(),
                    xCorners.xStats[2 * TOP_LEFT + 1].xMean(),
                    xCorners.xStats[2 * TOP_RIGHT].xMean(),
                    xCorners.xStats[2 * TOP_RIGHT + 1].xMean(),
                    xCorners.xStats[2 * BOT_LEFT].xMean(),
                    xCorners.xStats[2 * BOT_LEFT + 1].xMean(),
                    xCorners.xStats[2 * BOT_RIGHT].xMean(),
                    xCorners.xStats[2 * BOT_RIGHT + 1].xMean());
        }

    private:
        std::vector<StatEMA_t> xStats;
};

} // namespace pixy
} // namespace team9


#endif
