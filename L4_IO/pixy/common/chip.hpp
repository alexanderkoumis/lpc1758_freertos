#ifndef CHIP_HPP
#define CHIP_HPP

#include "pixy/config.hpp"
#include "pixy/common/stat.hpp"
#include "pixy/common/point_stat.hpp"

namespace team9
{
namespace pixy
{

class Chip_t
{
    public:
        Chip_t() : Chip_t(CHIP_PROXIM_TOLERANCE) {}
        Chip_t(float xPointEMA_alpha) :
                xChipColor(ChipColor_t::NONE),
                xLastChipColor(ChipColor_t::NONE),
                bNoneGreatest(false),
                bGreenGreatest(false),
                bRedGreatest(false),
                lChangedCnt(0), xLastKnownColorFreq(0.0f)
        {
            xPtLocation.vSetAlpha(xPointEMA_alpha);
        }

        void vReset()
        {
            xPtLocation.vReset();
            xChipColorNone.vReset();
            xChipColorGreen.vReset();
            xChipColorRed.vReset();

            xChipColor = NONE;
            xLastChipColor = NONE;

            bNoneGreatest = true;
            bGreenGreatest = false;
            bRedGreatest = false;
        }

        ChipColor_t xMaxChip()
        {
            return xChipColor;
        }

        float xMeanVal()
        {
            return (xChipColor == GREEN) ? xChipColorGreen.xMean() :
                    ((xChipColor == RED) ? xChipColorRed.xMean() : 0.0);
        }

        float xStdDevVal()
        {
            return (xChipColor == GREEN) ? xChipColorGreen.xStdDev() :
                    ((xChipColor == RED) ? xChipColorRed.xStdDev() : 0.0);
        }

        std::string xLocStr()
        {
            return xPtLocation.xStr();
        }

        std::string xStdDevStr()
        {
            char buff[16];
            snprintf(buff, 16, "%2.1f", xStdDevVal());
            return buff;
        }

        std::string xRangeStr()
        {
            char buff[16];
            snprintf(buff, 16, "%2.1f < x < %2.1f", xMeanVal() + xStdDevVal(),
                                                    xMeanVal() - xStdDevVal());
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
            lChangedCnt += (bChanged()) ? 1 : 0;
        }

        int lGetChangedCnt()
        {
            return lChangedCnt;
        }

        void vResetChangedCnt()
        {
            lChangedCnt = 0;
        }

        bool bChanged()
        {
            return xChipColor != xLastChipColor;
        }

        template<size_t N>
        bool bNotNoneAndTheSameNTimes() // Enterprise edition!!!!
        {
            if (xChipColor == xLastChipColor)
            {
                    return (++lInARowCnt == N) ? true : false;
            }
            return false;
        }

        float xGetColor(StatEnum_t xStatEnum)
        {
            if (bNoneGreatest) return xChipColorNone.xStat(xStatEnum);
            if (bGreenGreatest) return xChipColorGreen.xStat(xStatEnum);
            if (bRedGreatest) return xChipColorRed.xStat(xStatEnum);
            return xChipColorNone.xStat(xStatEnum);
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

        int lChangedCnt;
        size_t lInARowCnt;
        float xLastKnownColorFreq;
};

} // namespace pixy
} // namespace team9

#endif



