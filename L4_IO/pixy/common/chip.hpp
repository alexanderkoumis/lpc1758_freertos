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
        Chip_t(float xPointEMA_alpha) :
                xChipColor(ChipColor_t::NONE),
                xLastChipColor(ChipColor_t::NONE),
                bNoneGreatest(true),
                bGreenGreatest(false),
                bRedGreatest(false)
        {
            xPtLocation.vSetAlpha(xPointEMA_alpha);
        }

        void vReset()
        {
            xPtLocation.vReset();
        }

        ChipColor_t xMaxChip()
        {
            return xChipColor;
        }

        std::string xStr()
        {
            char buff[32];
            snprintf(buff, 32, "%1.1f/%1.1f/%1.1f",
                    xChipColorNone.xMean(),
                    xChipColorGreen.xMean(),
                    xChipColorRed.xMean());
            return buff;
        }

        void vUpdateFreq(size_t xNoneFreq, size_t xGreenFreq, size_t xRedFreq)
        {
            xChipColorNone.vUpdate(xNoneFreq);
            xChipColorGreen.vUpdate(xGreenFreq);
            xChipColorRed.vUpdate(xRedFreq);

            float xIsNone = xChipColorNone.xMean();
            float xIsGreen = xChipColorGreen.xMean();
            float xIsRed = xChipColorRed.xMean();

            bGreenGreatest = (xIsGreen > xIsNone) && (xIsGreen > xIsRed);
            bRedGreatest = (xIsRed > xIsNone) && (xIsRed > xIsGreen);
            bNoneGreatest = !bGreenGreatest && !bRedGreatest;

            xLastChipColor = xChipColor;
            if (bNoneGreatest) xChipColor = NONE;
            if (bGreenGreatest) xChipColor = GREEN;
            if (bRedGreatest) xChipColor = RED;
        }

        bool bChanged()
        {
            return xChipColor != xLastChipColor;
        }

        PointEMA_t xPtLocation;

    private:
        StatEMA_t xChipColorNone;
        StatEMA_t xChipColorGreen;
        StatEMA_t xChipColorRed;

        ChipColor_t xChipColor;
        ChipColor_t xLastChipColor;

        bool bNoneGreatest;
        bool bGreenGreatest;
        bool bRedGreatest;


};

} // namespace pixy
} // namespace team9

#endif



