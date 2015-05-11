#ifndef POINT_STAT_HPP
#define POINT_STAT_HPP

#include "pixy/common/point.hpp"

namespace team9
{
namespace pixy
{

extern "C"
{
    #include "L4_IO/pixy/libfixmatrix/fixvector2d.h"
}

class PointStat_t
{
    public:

        enum StatMode_t {SIMPLE_MOVING_AVG, EXPONENTIAL_MOVING_AVG} eStatMode;

        PointStat_t()
        {}

        void vReset()
        {
            xNewMean = {int16_t(0), int16_t(0)};
            xNewStdDev = {int16_t(0), int16_t(0)};
            xOldMean = {int16_t(0), int16_t(0)};
            xOldStdDev = {int16_t(0), int16_t(0)};
            ulCnt = 0;
        }

        virtual void vUpdate(Point_t<float>& xNewPoint) = 0;

        Point_t<float> xPoint()
        {
            return Point_t<float>(fix16_to_float(xNewMean.y),
                                  fix16_to_float(xNewMean.x));
        }

        std::string xStr() const
        {
            return Point_t<float>::xPointStr(fix16_to_float(xNewMean.y),
                                             fix16_to_float(xNewMean.x));
        }

        v2d xNewMean;
        v2d xNewStdDev;

    protected:
        v2d xOldMean;
        v2d xOldStdDev;
        uint64_t ulCnt = 0;
};

class PointSMA_t : public PointStat_t
{
    public:
        virtual void vUpdate(Point_t<float>& xNewPoint)
        {
            v2d xNewVec = {fix16_from_float(xNewPoint.xX),
                           fix16_from_float(xNewPoint.xY)};
            if (++ulCnt == 1)
            {
                this->xNewMean = this->xOldMean = xNewVec;
                this->xNewStdDev = {fix16_from_float(0.0f),
                                    fix16_from_float(0.0f)};
            }
            else
            {
                v2d xTempMean;
                v2d_sub(&xTempMean, &xNewVec, &xOldMean);
                v2d_div_s(&xTempMean, &xTempMean, fix16_from_float(ulCnt));
                v2d_add(&xNewMean, &xOldMean, &xTempMean);

                v2d xDx, xDxx, xTempStdDev;
                v2d_sub(&xDx, &xNewVec, &xNewMean);
                v2d_sub(&xDxx, &xNewVec, &xOldMean);

                xTempStdDev.y = fix16_mul(xDx.y, xDxx.y);
                xTempStdDev.x = fix16_mul(xDx.x, xDxx.x);

                v2d_add(&xNewStdDev, &xOldStdDev, &xTempStdDev);

                xOldMean = xNewMean;
                xOldStdDev = xNewStdDev;
            }
        }
};

class PointEMA_t : public PointStat_t
{
    public:

        PointEMA_t() : bInit(false) {}

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

        virtual void vUpdate(Point_t<float>& xNewPoint)
        {
            if (bInit)
            {
                v2d xNewVec = {fix16_from_float(xNewPoint.xX),
                               fix16_from_float(xNewPoint.xY)};
                if (++ulCnt == 1)
                {
                    this->xNewMean = this->xOldMean = xNewVec;
                    this->xNewStdDev = {fix16_from_float(0.0f),
                                        fix16_from_float(0.0f)};
                }
                else
                {
                    // New mean = (alpha * new_value) + (1.0 - alpha) * old mean
                    v2d xTempMean;
                    v2d_mul_s(&xNewVec, &xNewVec, xAlpha);
                    v2d_mul_s(&xTempMean, &xOldMean, xOneMinusAlpha);
                    v2d_add(&xNewMean, &xNewVec, &xTempMean);

                    v2d xDx, xDxx, xTempStdDev;
                    v2d_sub(&xDx, &xNewVec, &xNewMean);
                    v2d_sub(&xDxx, &xNewVec, &xOldMean);

                    xTempStdDev.y = xDx.y * xDxx.y;
                    xTempStdDev.x = xDx.x * xDxx.x;

                    v2d_add(&xNewStdDev, &xOldStdDev, &xTempStdDev);

                    xOldMean = xNewMean;
                    xOldStdDev = xNewStdDev;
                }
            }
            else
            {
                std::cout << "ERROR: Attempt to update PointEMA before init\n";
            }
        }

    private:
        bool bInit;
        fix16_t xAlpha;
        fix16_t xOneMinusAlpha;
};

}
}


#endif
