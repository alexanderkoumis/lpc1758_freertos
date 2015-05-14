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
                xKnownChipColor(ChipColor_t::NONE),
                xLastChipColor(ChipColor_t::NONE),
                bNoneGreatest(false),
                bGreenGreatest(false),
                bRedGreatest(false),
                bEnabled(true),
                lInARowCnt(0),
                lTimesRepeatedForMax(NUM_TIMES_FOR_CNT),
                xLastKnownColorFreq(0.0f)
        {
            xPtLoc.vSetAlpha(xPointEMA_alpha);
        }

        void vReset()
        {
            xPtLoc.vReset();
            this->vResetCounters();
        }

        void vSet(ChipColor_t xChipColor)
        {
            bEnabled = false;
            xKnownChipColor = xChipColor;
        }

        ChipColor_t xMaxChip()
        {
            return xKnownChipColor;
        }

        float xMeanVal()
        {
            return (xKnownChipColor == GREEN) ? xChipColorGreen.xMean() :
                    ((xKnownChipColor == RED) ? xChipColorRed.xMean() : 0.0);
        }

        float xStdDevVal()
        {
            return (xKnownChipColor == GREEN) ? xChipColorGreen.xStdDev() :
                    ((xKnownChipColor == RED) ? xChipColorRed.xStdDev() : 0.0);
        }

        std::string xLocStr()
        {
            return xPtLoc.xStr();
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

        void vUpdateFreq(int xNoneFreq, int xGreenFreq, int xRedFreq)
        {
            if (bEnabled)
            {
                xChipColorNone.vUpdate(xNoneFreq);
                xChipColorGreen.vUpdate(xGreenFreq);
                xChipColorRed.vUpdate(xRedFreq);

                float xIsNone = xChipColorNone.xMean();
                float xIsGreen = xChipColorGreen.xMean();
                float xIsRed = xChipColorRed.xMean();

                bGreenGreatest = bRedGreatest = bNoneGreatest = false;
                bGreenGreatest = (xIsGreen > xIsNone) && (xIsGreen > xIsRed);
                bRedGreatest = (xIsRed > xIsNone) && (xIsRed > xIsGreen);

                ChipColor_t xTempChipColor = NONE;
                if (bGreenGreatest) xTempChipColor = GREEN;
                else if (bRedGreatest) xTempChipColor = RED;

                if (xTempChipColor == NONE)
                {
                    lInARowCnt = 0;
                    return;
                }

                if (xLastChipColor == NONE)
                {
                    xLastChipColor = xTempChipColor;
                    lInARowCnt++;
                    return;
                }

                // xTempChipColor is either red or blue,
                // xLastChipColor is red or blue
                if (xTempChipColor == xLastChipColor)
                {
                    if (lInARowCnt++ == (int)lTimesRepeatedForMax)
                    {
                        xKnownChipColor = xLastChipColor;
                        bEnabled = false;
                    }
                }
                else
                {
                    lInARowCnt = 1;
                }
            }
        }

        bool bChipKnown()
        {
            return !bEnabled;
        }

        void vResetCounters()
        {
            xChipColorNone.vReset();
            xChipColorGreen.vReset();
            xChipColorRed.vReset();

            xKnownChipColor = NONE;
            xLastChipColor = NONE;

            bNoneGreatest = true;
            bGreenGreatest = false;
            bRedGreatest = false;
            bEnabled = true;
        }

        int lGetInARowCnt()
        {
            return lInARowCnt;
        }

        float xGetColor(StatEnum_t xStatEnum)
        {
            if (bNoneGreatest) return xChipColorNone.xStat(xStatEnum);
            if (bGreenGreatest) return xChipColorGreen.xStat(xStatEnum);
            if (bRedGreatest) return xChipColorRed.xStat(xStatEnum);
            return xChipColorNone.xStat(xStatEnum);
        }

        PointEMA_t xPtLoc;

    private:

        StatEMA_t xChipColorNone;
        StatEMA_t xChipColorGreen;
        StatEMA_t xChipColorRed;

        ChipColor_t xKnownChipColor;
        ChipColor_t xLastChipColor;

        bool bNoneGreatest;
        bool bGreenGreatest;
        bool bRedGreatest;
        bool bEnabled;

        int lInARowCnt;
        int lTimesRepeatedForMax; // Enterprise strength!
        float xLastKnownColorFreq;

};

} // namespace pixy
} // namespace team9

#endif



