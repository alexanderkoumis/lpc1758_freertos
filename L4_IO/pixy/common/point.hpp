#ifndef POINT_HPP
#define POINT_HPP

#include "L4_IO/pixy/libfixmath/fix16.hpp"

namespace team9
{
namespace pixy
{

template<typename T>
struct Point_t
{
    T xY;
    T xX;

    Point_t(T xY_arg = 0, T xX_arg = 0)
    {
        std::tie(xY, xX) = std::tie(xY_arg, xX_arg);
    }


    template<typename T1>
    operator Point_t<T1> ()
    {
        return Point_t<T1>(xY, xX);
    }

    Point_t& operator = (const Point_t& xRHS)
    {
        std::tie(xY, xX) = std::tie(xRHS.xY, xRHS.xX);
        return *this;
    }

    Point_t& operator += (const Point_t& xRHS)
    {
        std::tie(xY += xRHS.xY, xX += xRHS.xX);
        return *this;
    }

    Point_t operator + (const Point_t& xRHS) const
    {
        return Point_t(xY + xRHS.xY, xX + xRHS.xX);
    }

    Point_t operator - (const Point_t& xRHS) const
    {
        return Point_t(xY - xRHS.xY, xX - xRHS.xX);
    }

    Point_t operator * (const Point_t& xRHS) const
    {
        return Point_t(xY * xRHS.xY, xX * xRHS.xX);
    }

    Point_t operator / (const Point_t& xRHS) const
    {
        return Point_t(xY / xRHS.xY, xX / xRHS.xX);
    }

    Point_t& operator /= (const uint32_t& ulDivisor)
    {
        std::tie(xY /= ulDivisor, xX /= ulDivisor);
        return *this;
    }

    bool operator () ()
    {
        return !(xY == 0 && xX == 0);
    }

    bool operator == (const Point_t& xRHS)
    {
        return (xY == xRHS.xY && xX == xRHS.xX);
    }

    uint32_t operator ++ (int) const
    {
        return xY + xX;
    }

    std::string xStr() const
    {
        std::ostringstream xOss;
        xOss << "[" << xY << " " << xX << "]";
        return xOss.str();
    }

    template<typename T1>
    float xCalcDist(Point_t<T1>& xPt) const
    {
        return sqrt(pow(xPt.xY - xY, 2) + pow(xPt.xX - xX, 2));
    }

    template<typename T1>
    static std::string xPointStr(const T1 xY, const T1 xX)
    {
        std::ostringstream xOss;
        xOss << "[" << xY << " " << xX << "]";
        return xOss.str();
    }

    template<typename T1>
    static std::string xOpenCVPt(const Point_t<T1>& xPoint)
    {
        std::ostringstream xOss;
        xOss << "cv::Point2f(" << xPoint.xX << ", " << xPoint.xY << ")";
        return xOss.str();
    }

    friend std::ostream& operator << (std::ostream& xLHS, const Point_t& xRHS)
    {
        return xLHS << xRHS.xStr();
    }

    static T xCalcDist(Point_t<T> xPtA, Point_t<T> xPtB)
    {
        return sqrt(pow(xPtB.xY-xPtA.xY,2) + pow(xPtB.xX-xPtA.xX,2));
    }

    template<typename T1, typename T2>
    static float xCalcDist(Point_t<T1> xPtA, Point_t<T2> xPtB)
    {
        return sqrt(pow(xPtB.xY - xPtA.xY, 2) + pow(xPtB.xX - xPtA.xX, 2));
    }

    static void xPointsOnLine(Point_t<float> xPtA, Point_t<float> xPtB,
                              int xNumPts,
                              std::vector<Point_t<float>>& xPointVec)
    {
        float xYDistInc = (xPtB.xY - xPtA.xY) / (xNumPts-1);
        float xXDistInc = (xPtB.xX - xPtA.xX) / (xNumPts-1);
        size_t xIdx = -1;
        xPointVec.assign(xNumPts, xPtA);
        std::transform(xPointVec.begin(), xPointVec.end(), xPointVec.begin(),
                [&](Point_t<float> xPoint) {
            xIdx++;
            return xPoint += Point_t<float>(xIdx*xYDistInc, xIdx*xXDistInc);
        });
    }


};

} // namespace pixy
} // namespace team9

#endif
