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
        Chip_t() : Chip_t(CHIP_PROXIM_TOLERANCE, ChipColor_t::GREEN) {}
        Chip_t(const float xPointEMA_alpha, const ChipColor_t eHumanChipColor_arg) :
                eKnownChipColor(ChipColor_t::NONE),
                eLastChipColor(ChipColor_t::NONE),
                eHumanChipColor(eHumanChipColor_arg),
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
            this->vResetCounters();
        }

        void vSet(ChipColor_t xChipColor)
        {
            bEnabled = false;
            eKnownChipColor = xChipColor;
        }

        ChipColor_t xMaxChip()
        {
            return eKnownChipColor;
        }

        float xMeanVal()
        {
            return (eKnownChipColor == GREEN) ? xChipColorGreen.xMean() :
                    ((eKnownChipColor == RED) ? xChipColorRed.xMean() : 0.0);
        }

        float xStdDevVal()
        {
            return (eKnownChipColor == GREEN) ? xChipColorGreen.xStdDev() :
                    ((eKnownChipColor == RED) ? xChipColorRed.xStdDev() : 0.0);
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

                ChipColor_t eTempChipColor = NONE;
                if (bGreenGreatest) eTempChipColor = GREEN;
                else if (bRedGreatest) eTempChipColor = RED;

//                if (eTempChipColor == NONE)
                if (eTempChipColor != eHumanChipColor)
                {
                    lInARowCnt = 0;
                    return;
                }

                if (eLastChipColor == NONE)
                {
                    eLastChipColor = eTempChipColor;
                    lInARowCnt = 1;
                    return;
                }

                if (eTempChipColor == eLastChipColor)
                {
                    if (lInARowCnt++ == (int)lTimesRepeatedForMax)
                    {
                        eKnownChipColor = eLastChipColor;
                        bEnabled = false;
                    }
                }
                else
                {
                    lInARowCnt = 1;
                }
            }
            else
            {
                printf("Error in vUpdateFreq: Enabled is false\n");
                u0_dbg_printf("Error in vUpdateFreq: Enabled is false\n");
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

            eKnownChipColor = NONE;
            eLastChipColor = NONE;

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

        ChipColor_t eKnownChipColor;
        ChipColor_t eLastChipColor;
        ChipColor_t eHumanChipColor;

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



