#ifndef STAT_HPP
#define STAT_HPP

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

        virtual void vUpdate(const float xVal_arg) = 0;

        bool vPreOp(float xVal)
        {
            xCnt += int16_t(1);
            if (xCnt == int16_t(1))
            {
                xNewMean = xOldMean = xVal;
                xNewStdDev = xOldStdDev = Fix16();
                return false;
            }
            return true;
        }

        void vPostOp(Fix16& xVal)
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

        float xMean() const
        {
            return (xCnt > int16_t(1)) ? fix16_to_float(xNewMean) : 0.0;
        }

        float xVariance() const
        {
            return (xCnt > int16_t(1)) ? fix16_to_float(xNewStdDev/xCnt) : 0.0;
        }

        float xStdDev() const
        {
            return std::sqrt(xVariance());
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

        virtual void vUpdate(const float xNewVal)
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
        StatEMA_t() : bInit(false), xAlpha(0.95)
        {}

        void vSetAlpha(float xAlpha_arg)
        {
            if (xAlpha_arg <= 0.0f || xAlpha_arg >= 1.0f)
            {
                std::cout << "ERROR: PointEMA alpha must be between 0 and 1\n";
                return;
            }
            xAlpha = fix16_from_float(xAlpha_arg);
            xOneMinusAlpha = fix16_from_float(1.0f - xAlpha_arg);
            bInit = true;
        }

        virtual void vUpdate(const float xNewVal)
        {
            Fix16 xVal(xNewVal);
            if (vPreOp(xVal))
            {
                xNewMean = (xAlpha * xVal) + (xOneMinusAlpha * xOldMean);
                vPostOp(xVal);
            }
        }

    private:
        bool bInit;
        Fix16 xAlpha;
        Fix16 xOneMinusAlpha;


};


} // namespace pixy
} // namespace team9

#endif
