#ifndef PIXY_COMMON_HPP
#define PIXY_COMMON_HPP

#include <cstdio> // snprint
#include <cstring>
#include <cmath>
#include <cassert>
#include <vector>
#include <iostream>
#include <string>     // string
#include <map>        // map
#include <sstream>    // ostringstream
#include <tuple>      // tie
#include <functional> // function
#include <algorithm>  // transform

namespace team9
{
namespace pixy
{

enum ChipColor_t
{
    GREEN = 1, RED = 2, NONE = 999
};

enum Quadrant_t
{
    TOP_LEFT = 0, TOP_RIGHT = 1, BOT_LEFT = 2, BOT_RIGHT = 3, ERROR = 4
};

enum Stat_t
{
    M, V, S
};

struct Point_t
{
    uint32_t ulX;
    uint32_t ulY;

    Point_t(uint32_t ulX_arg = 0, uint32_t ulY_arg = 0)
    {
        std::tie(ulX, ulY) = std::tie(ulX_arg, ulY_arg);
    }

    Point_t& operator = (const Point_t& xRHS)
    {
        std::tie(ulX, ulY) = std::tie(xRHS.ulX, xRHS.ulY);
        return *this;
    }

    Point_t& operator += (const Point_t& xRHS)
	{
        std::tie(ulX += xRHS.ulX, ulY += xRHS.ulY);
        return *this;
	}

    Point_t operator + (const Point_t& xRHS) const
    {
        return Point_t(ulX + xRHS.ulX, ulY + xRHS.ulY);
    }

    Point_t operator - (const Point_t& xRHS) const
    {
        return Point_t(ulX - xRHS.ulX, ulY - xRHS.ulY);
    }

    Point_t operator * (const Point_t& xRHS) const
    {
        return Point_t(ulX * xRHS.ulX, ulY * xRHS.ulY);
    }

    Point_t operator / (const Point_t& xRHS) const
    {
        return Point_t(ulX / xRHS.ulX, ulY / xRHS.ulY);
    }

    Point_t& operator /= (const uint32_t& ulDivisor)
    {
        std::tie(ulX /= ulDivisor, ulY /= ulDivisor);
        return *this;
    }

    bool operator () ()
    {
        return !(ulX == 0 && ulY == 0);
    }

    bool operator == (const Point_t& xRHS)
    {
        return (ulX == xRHS.ulX && ulY == xRHS.ulY);
    }

    __inline uint32_t operator ++ (int) const
    {
        return ulX + ulY;
    }

    friend std::ostream& operator << (std::ostream& xLHS,
            const Point_t& xRHS)
    {
        return xLHS << "[" << xRHS.ulX << " " << xRHS.ulY << "]";
    }

    __inline std::string sStr()
    {
        std::ostringstream oss;
        oss.str("");
        oss.clear();
        oss << this;
        return oss.str();
    }

    static void vPrintPoints(std::vector<Point_t>& xPoints)
    {
        std::cout << "Points" << std::endl;
        for (auto& xPoint : xPoints)
        {
            std::cout << xPoint << std::endl;
        }
    }

};

struct Dims_t
{
    uint16_t usCols;
    uint16_t usRows;

    Dims_t(uint16_t usCols_arg = 0, uint16_t usRows_arg = 0)
    {
        std::tie(usCols, usRows) = std::tie(usCols_arg, usRows_arg);
    }

    Dims_t& operator = (const Dims_t& xRHS)
    {
        std::tie(usCols, usRows) = std::tie(xRHS.usCols, xRHS.usRows);
        return *this;
    }

    Dims_t operator / (const uint32_t& ulDivisor) const
    {
        return Dims_t(usCols / ulDivisor, usRows / ulDivisor);
	}

    friend std::ostream& operator << (std::ostream& xLHS,
                                      const Dims_t& xRHS)
    {
        return xLHS << "[" << xRHS.usCols << " " << xRHS.usRows << "]";
    }
};

struct Block_t
{
    Point_t xPoint;
    uint16_t usSignature;
    uint16_t usWidth;
    uint16_t usHeight;
    uint16_t usAngle;

    __inline uint16_t operator ++ (int) const
    {
        return xPoint++ + usSignature + usWidth + usHeight + usAngle;
	}

    friend std::ostream& operator << (std::ostream& xLHS,
                                      const Block_t& xRHS)
    {
        return xLHS << "Signature: " << xRHS.usSignature << std::endl
                    << "Point: " << xRHS.xPoint << std::endl
                    << "Size: [" << xRHS.usHeight << " "
                    << xRHS.usWidth << "]" << std::endl
                    << "Angle: " << xRHS.usAngle;
    }
};

template<typename KEY_T, typename FUN_T, typename ... ARG_T>
struct FuncMap_t
{
    std::map<KEY_T, std::function<FUN_T(ARG_T ... xArgs)>> fpMap;

    void vSetHandler(KEY_T xElem,
                     std::function<FUN_T(ARG_T ... xArgs)> fnHandler)
    {
        fpMap[xElem] = fnHandler;
    }

    std::function<FUN_T(ARG_T ... xArgs)>& vResponse(KEY_T xElem)
    {
        return fpMap[xElem];
    }
};
template<typename IN_T, typename OUT_T>
class RunningStat_t
{
    // johndcook.com/blog/standard_deviation
    // See Knuth TAOCP vol 2, 3rd edition, page 232

    public:

        RunningStat_t ()
        {
            vInitStringMap();
        }

        bool operator () ()
        {
            return xMeanNew;
        }

        void vUpdate(IN_T& xVal)
        {
            if (++ulCnt == 1)
            {
                xMeanOld = (OUT_T)xVal;
                xMeanNew = (OUT_T)xVal;
                xStdDevOld = (OUT_T)0.0;
            }
            else
            {
                xMeanNew = xMeanOld + (OUT_T)(xVal - (OUT_T)xMeanOld) / ulCnt;
                xStdDevNew = xStdDevOld + (OUT_T)(xVal - (OUT_T)xMeanOld) *
                                                 (xVal - (OUT_T)xMeanNew);
            }
        }

        void vInitStringMap()
        {
            xStringMap[M] = "Mean";
            xStringMap[V] = "Variance";
            xStringMap[S] = "Standard Deviation";
        }

        __inline OUT_T xMean() const
        {
            return (ulCnt > 1) ? xMeanNew : 0.0;
        }

        __inline OUT_T xVariance() const
        {
            return (ulCnt > 1) ? xStdDevNew / ulCnt : 0.0;
        }

        __inline OUT_T xStdDev() const
        {
            return std::sqrt(xVariance());
        }

    private:
        std::map<Stat_t, std::string> xStringMap;
        uint64_t ulCnt = 0;
        OUT_T xMeanOld;
        OUT_T xMeanNew;
        OUT_T xStdDevOld;
        OUT_T xStdDevNew;
};

struct Corners_t
{
    std::vector<RunningStat_t<uint32_t, float>> xStats;

    Corners_t()
    {
        xStats.resize(8);
    }

    bool operator () ()
    {
        return xStats[2*TOP_LEFT]()  && xStats[2*TOP_LEFT+1]()  &&
               xStats[2*TOP_RIGHT]() && xStats[2*TOP_RIGHT+1]() &&
               xStats[2*BOT_LEFT]()  && xStats[2*BOT_LEFT+1]()  &&
               xStats[2*BOT_RIGHT]() && xStats[2*BOT_RIGHT+1]();
    }

    friend std::ostream& operator << (std::ostream& xLHS,
            const Corners_t& xRHS)
    {
        return xLHS << std::noskipws << "[" << std::endl
                    << "    " << xCornerStr(xRHS, TOP_LEFT)
                    << " " << xCornerStr(xRHS, TOP_RIGHT) << std::endl
                    << "    " << xCornerStr(xRHS, BOT_LEFT)
                    << " " << xCornerStr(xRHS, BOT_RIGHT) << std::endl
                    << "]";
    }

    static std::string xCornerStr(const Corners_t& xCorners,
                                  Quadrant_t xQuadrant)
    {
        char buff[128];
        snprintf(buff, sizeof(buff)/sizeof(buff[0]), "[%3.2f %3.2f]",
                 xCorners.xStats[2*xQuadrant+1].xMean(),
                 xCorners.xStats[2*xQuadrant].xMean());
        return std::string(buff);
    }

    std::tuple<float, float> xGet(Quadrant_t xQuadrant)
    {
       return std::make_tuple(xStats[2*xQuadrant].xMean(), xStats[2*xQuadrant+1].xMean());
    }

};

struct Chip_t
{
    std::pair<RunningStat_t<uint32_t, float>,
              RunningStat_t<uint32_t, float>> xChipPair;
    ChipColor_t xChipColor;
};

class Board_t
{
    public:
        Board_t(float x) : ulRows(6), ulCols(7)
        {
            xChips.resize(ulRows * ulCols);
        }

    private:
        std::vector<Chip_t> xChips;
        uint32_t ulRows;
        uint32_t ulCols;
};

template<typename T>
void xPointsOnLine(std::tuple<T, T>& xPtA, std::tuple<T, T>& xPtB,
              int xNumPts, std::vector<Point_t>& xPointVec)
{
    float xX1 = std::get<0>(xPtA);
    float xY1 = std::get<1>(xPtA);
    float xX2 = std::get<0>(xPtB);
    float xY2 = std::get<1>(xPtB);
    float xXDistFull = xX2 - xX1;
    float xYDistFull = xY2 - xY2;
    float xXDistInc = xXDistFull / (xNumPts-1);
    float xYDistInc = xYDistFull / (xNumPts-1);
    size_t xIdx = -1;
    xPointVec.assign(xNumPts, Point_t(xX1, xY1));
    std::transform(xPointVec.begin(), xPointVec.end(), xPointVec.begin(),
            [&](Point_t xPoint) {
        xIdx++;
        return xPoint += Point_t(xIdx * xXDistInc, xIdx * xYDistInc);
    });
}


} // namespace pixy
} // namespace team9

#endif

