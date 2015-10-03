#ifndef STAT_HPP
#define STAT_HPP

#include <iomanip>

#include "pixy/config.hpp"
#include "L4_IO/pixy/libfixmath/fix16.hpp"

namespace team9
{
namespace pixy
{

class Stat_t
{
    // johndcook.com/blog/standard_deviation
    // See Knuth TAOCP vol 2, 3rd edition, page 232
    public:

        Stat_t() {}

        bool operator () ()
        {
            return xNewMean.value != 0;
        }

        virtual void vUpdate(const float xVal_arg, bool bPrint) = 0;

        bool vPreOp(float xVal)
        {
            xCnt += int16_t(1);
            if (xCnt == int16_t(1))
            {
                xNewMean = xOldMean = Fix16(xVal);
                xNewStdDev = xOldStdDev = Fix16();
                return false;
            }
            return true;
        }

        void vPostOp(Fix16 xVal)
        {
            xNewStdDev = xOldStdDev + (xVal - xOldMean) * (xVal - xNewMean);
            xOldMean = xNewMean;
            xOldStdDev = xNewStdDev;
        }

        void vReset()
        {
            xNewMean = int16_t(0);
            xOldMean = int16_t(0);
            xNewStdDev = int16_t(0);
            xOldStdDev = int16_t(0);
            xCnt = int16_t(0);
        }

        void vSetMean(float xVal)
        {
            xNewMean = Fix16(xVal);
            xOldMean = Fix16(xVal);
            xCnt = int16_t(1);
        }

        float xMean() const
        {
            return (xCnt >= int16_t(1)) ? fix16_to_float(xNewMean) : 0.0;
        }

        float xVariance() const
        {
            return (xCnt >= int16_t(1)) ? fix16_to_float(xNewStdDev/xCnt) : 0.0;
        }

        float xStdDev() const
        {
            return std::sqrt(xVariance());
        }

        float xStat(StatEnum_t xStatType)
        {
            switch(xStatType)
            {
                case M:  return xMean();
                case V:  return xVariance();
                case S:  return xStdDev();
                default: return -999.99;
            }
        }

    protected:
        Fix16 xNewMean;
        Fix16 xOldMean;
        Fix16 xNewStdDev;
        Fix16 xOldStdDev;
        Fix16 xCnt;
};


class StatSMA_t : public Stat_t
{
    public:
        StatSMA_t() {}

        virtual void vUpdate(float xNewVal, bool bPrint = false)
        {
            Fix16 xVal(xNewVal);
            if (vPreOp(xVal))
            {
                xNewMean = xOldMean + (xVal - xOldMean) / xCnt;
                vPostOp(xVal);
            }
        }

};

class StatEMA_t : public Stat_t
{
    public:
        StatEMA_t(float xAlpha_arg = CHIP_COLOR_EMA_ALPHA) :
                xAlpha(xAlpha_arg),
                xOneMinusAlpha(1.0f-xAlpha_arg)
        {}

        void vSetAlpha(float xAlpha_arg)
        {
            if (xAlpha_arg <= 0.0f || xAlpha_arg >= 1.0f)
            {
                std::cout << "ERROR: PointEMA alpha must be between 0 and 1\n";
                return;
            }
            xAlpha = fix16_from_float(xAlpha_arg);
            xOneMinusAlpha = fix16_from_float(1.0f-xAlpha_arg);
        }

        virtual void vUpdate(float xVal, bool bPrint = false)
        {
            if (vPreOp(xVal))
            {
                xNewMean = (xAlpha * xVal) + (xOneMinusAlpha * xOldMean);
                vPostOp(xVal);
            }
        }

    private:
        Fix16 xAlpha;
        Fix16 xOneMinusAlpha;
};


} // namespace pixy
} // namespace team9

#endif
