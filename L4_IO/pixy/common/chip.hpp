#ifndef CHIP_HPP
#define CHIP_HPP

#include "pixy/common/stat.hpp"
#include "pixy/common/point_stat.hpp"

namespace team9
{
namespace pixy
{

class Chip_t
{
    public:
        Chip_t(float xPointEMA_alpha)
        {
            xPtLocation.vSetAlpha(xPointEMA_alpha);
        }

        void vReset()
        {
            xPtLocation.vReset();
        }

        int lMaxChip()
        {
            float xIsNone = xChipColorNone.xMean();
            float xIsGreen = xChipColorGreen.xMean();
            float xIsRed = xChipColorRed.xMean();
            bool bGreenGreatest = (xIsGreen > xIsNone) && (xIsGreen > xIsRed);
            bool bRedGreatest = (xIsRed > xIsNone) && (xIsRed > xIsGreen);
            bool bNoneGreatest = !bGreenGreatest && !bRedGreatest;
            if (bNoneGreatest) return NONE;
            if (bGreenGreatest) return GREEN;
            if (bRedGreatest) return RED;
        }

        static void vUpdate(Chip_t& xChip, uint16_t usSignature)
        {
            xChip.xChipColorNone.vUpdate(usSignature == NONE);
            xChip.xChipColorGreen.vUpdate(usSignature == GREEN);
            xChip.xChipColorRed.vUpdate(usSignature == RED);
        }


        PointEMA_t xPtLocation;

    private:


        StatSMA_t xChipColorNone;
        StatSMA_t xChipColorGreen;
        StatSMA_t xChipColorRed;

};

} // namespace pixy
} // namespace team9

#endif



