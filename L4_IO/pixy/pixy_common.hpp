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

#define F() std::cout << "Line: " << __LINE__ << ", func: " << __func__ << std::endl;

namespace team9
{
namespace pixy
{

enum ChipColor_t
{
    GREEN = 1, RED = 2, NONE = 3
};

enum Quadrant_t
{
    TOP_LEFT = 0, TOP_RIGHT = 1, BOT_LEFT = 2, BOT_RIGHT = 3, ERROR = 4
};

enum StatEnum_t
{
    M, V, S
};


template<typename T>
struct Point_t
{
    T xX;
    T xY;

    Point_t(T xX_arg = 0, T xY_arg = 0)
    {
        std::tie(xX, xY) = std::tie(xX_arg, xY_arg);
    }

    Point_t& operator = (const Point_t& xRHS)
    {
        std::tie(xX, xY) = std::tie(xRHS.xX, xRHS.xY);
        return *this;
    }

    Point_t& operator += (const Point_t& xRHS)
	{
        std::tie(xX += xRHS.xX, xY += xRHS.xY);
        return *this;
	}

    Point_t operator + (const Point_t& xRHS) const
    {
        return Point_t(xX + xRHS.xX, xY + xRHS.xY);
    }

    Point_t operator - (const Point_t& xRHS) const
    {
        return Point_t(xX - xRHS.xX, xY - xRHS.xY);
    }

    Point_t operator * (const Point_t& xRHS) const
    {
        return Point_t(xX * xRHS.xX, xY * xRHS.xY);
    }

    Point_t operator / (const Point_t& xRHS) const
    {
        return Point_t(xX / xRHS.xX, xY / xRHS.xY);
    }

    Point_t& operator /= (const uint32_t& ulDivisor)
    {
        std::tie(xX /= ulDivisor, xY /= ulDivisor);
        return *this;
    }

    bool operator () ()
    {
        return !(xX == 0 && xY == 0);
    }

    bool operator == (const Point_t& xRHS)
    {
        return (xX == xRHS.xX && xY == xRHS.xY);
    }

    __inline uint32_t operator ++ (int) const
    {
        return xX + xY;
    }

    friend std::ostream& operator << (std::ostream& xLHS,
            const Point_t& xRHS)
    {
        return xLHS << "[" << xRHS.xY << " " << xRHS.xX << "]";
    }

    static std::string xPointStr(const Point_t xPoint)
    {
        std::ostringstream xOss;
        xOss << xPoint;
        return xOss.str();
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
    Point_t<uint16_t> xPoint;
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

template<typename T>
class PointStat_t : public Point_t<T>
{
    public:
        PointStat_t() {}

        PointStat_t(Point_t<T>& xPoint) : Point_t<T>(xPoint)
        {}

        void vUpdate(Point_t<T> xNewPoint)
        {
            if (++ulCnt == 1)
            {
                this->xX = xXOldMean = xNewPoint.xX;
                this->xY = xYOldMean = xNewPoint.xY;
                xXNewStdDev = xXOldStdDev = 0.0;
                xYNewStdDev = xYOldStdDev = 0.0;
            }
            else
            {
                this->xX = xXOldMean + (xNewPoint.xX - xXOldMean) / ulCnt;
                this->xY = xYOldMean + (xNewPoint.xY - xYOldMean) / ulCnt;
                // Love these FreeRTOS conventions!!!
                xXNewStdDev = xXOldStdDev + (xNewPoint.xX- xXOldMean)
                                          * (xNewPoint.xX - this->xX);
                xYNewStdDev = xYOldStdDev + (xNewPoint.xY- xYOldMean)
                                          * (xNewPoint.xY - this->xY);
            }
        }

        std::string xGet()
        {
            std::cout << "[" << this->xY << " " << this->xX << "]";
        }

    private:
    //  T xXNewMean == xX from parent
    //  T xYNewMean == xY from parent
        T xXOldMean;
        T xYOldMean;
        T xXNewStdDev;
        T xYNewStdDev;
        T xXOldStdDev;
        T xYOldStdDev;
        uint64_t ulCnt = 0;
};


template<typename T>
class Stat_t
{
    // johndcook.com/blog/standard_deviation
    // See Knuth TAOCP vol 2, 3rd edition, page 232
    public:
        Stat_t ()
        {
            xNewMean = 0;
            xOldMean = 0;
            xNewStdDev = 0;
            xOldStdDev = 0;
            ulCnt = 0;
        }

        bool operator () ()
        {
            return xNewMean;
        }

        void vUpdate(T xVal)
        {
            if (++ulCnt == 1)
            {
                xNewMean = xOldMean = xVal;
                xNewStdDev = xOldStdDev = 0.0;
            }
            else
            {
                xNewMean = xOldMean + (xVal - xOldMean) / ulCnt;
                xNewStdDev = xOldStdDev + (xVal - xOldMean) * (xVal - xNewMean);
            }
        }

        __inline T xMean() const
        {
            return (ulCnt > 1) ? xNewMean : 0.0;
        }

        __inline T xVariance() const
        {
            return (ulCnt > 1) ? xNewStdDev / ulCnt : 0.0;
        }

        __inline T xStdDev() const
        {
            return std::sqrt(xVariance());
        }

    private:
        T xNewMean;
        T xOldMean;
        T xNewStdDev;
        T xOldStdDev;
        uint64_t ulCnt = 0;
};

struct Corners_t
{
    std::vector<Stat_t<float>> xStats;

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

    Point_t<float> operator() (Quadrant_t xQuadrant) const
    {
        return Point_t<float>(xStats[2*xQuadrant].xMean(),
                              xStats[2*xQuadrant+1].xMean());
    }

    friend std::ostream& operator << (std::ostream& xLHS, const Corners_t& xRHS)
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
        static const uint32_t ulBuffSize = 32;
        char buff[ulBuffSize];
        snprintf(buff, ulBuffSize, "[%3.2f %3.2f]",
                 xCorners.xStats[2*xQuadrant].xMean(),
                 xCorners.xStats[2*xQuadrant+1].xMean());
        return std::string(buff);
    }
};


struct Chip_t
{
    PointStat_t<float> xPtLocation;
    Stat_t<float> xChipColor;

    Chip_t() {}

    Chip_t(Point_t<float>& xPoint)
    {
        xPtLocation = xPoint;
    }
};

class Board_t
{
    public:
        Board_t() : ulRows(6), ulCols(7)
        {}

        bool xFillRow(uint32_t xRowNum, std::vector<Point_t<float>>& xRowVec)
        {
            if (xRowVec.size() >= (ulCols) || xRowVec.size() < 0)
            {
                return false;
            }
            xChips.resize(ulRows * ulCols);
            for (auto xColElem: xRowVec)
            {
                Chip_t xChip(xColElem);
                xChips.push_back(xChip);
            }
            return true;
        }

        static void vPrintChips(Board_t& xBoard)
        {
            uint32_t ulIdx = 0;
            for (auto& xChip : xBoard.xChips)
            {
//                std::cout << xChip.xPtLocation.xY << xChip.xPtLocation.xX << " ";
                if (ulIdx++ % xBoard.ulCols == 0)
                {
//                    std::cout << std::endl;
                }
            }
       }
        std::vector<Chip_t> xChips;

    private:
        std::map<uint32_t, std::string> xStrMap;
//        std::vector<Chip_t> xChips;
        uint32_t ulRows;
        uint32_t ulCols;
};


static void xPointsOnLine(Point_t<float> xPtA, Point_t<float> xPtB,
                          int xNumPts, std::vector<Point_t<float>>& xPointVec)
{
    float xXDistInc = (xPtB.xX - xPtA.xX) / (xNumPts-1);
    float xYDistInc = (xPtB.xY - xPtA.xY) / (xNumPts-1);
    size_t xIdx = -1;
    xPointVec.assign(xNumPts, xPtA);
    std::transform(xPointVec.begin(), xPointVec.end(), xPointVec.begin(),
            [&](Point_t<float> xPoint) {
        xIdx++;
        return xPoint += Point_t<float>(xIdx * xXDistInc, xIdx * xYDistInc);
    });
}


} // namespace pixy
} // namespace team9

#endif


