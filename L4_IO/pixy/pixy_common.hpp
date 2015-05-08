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
    NONE = 0, GREEN = 1, RED = 2
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

    operator Point_t<float> ()
    {
        return Point_t<T>(xX, xY);
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

    uint32_t operator ++ (int) const
    {
        return xX + xY;
    }

    std::string xStr() const
    {
        std::ostringstream xOss;
        xOss << "cv::Point2f(" << xX << ", " << xY << ")";
        return xOss.str();
    }

    // TODO: Get rid of this...
    static std::string xPointStr(const Point_t xPoint)
    {
        std::ostringstream xOss;
        xOss << xPoint;
        return xOss.str();
    }

    static std::string xOpenCVPt(const Point_t<float>& xPoint)
    {
        return xPoint.xStr();
    }

    friend std::ostream& operator << (std::ostream& xLHS, const Point_t& xRHS)
    {
        return xLHS << "[" << xRHS.xY << " " << xRHS.xX << "]";
    }

    T xCalcDist(const Point_t<T> xRightPt)
    {
        return sqrt(pow(xRightPt.xY-xY,2) + pow(xRightPt.xX-xX,2));
    }

    static T xCalcDist(Point_t<T> xPtA, Point_t<T> xPtB)
    {
        return sqrt(pow(xPtB.xY-xPtA.xY,2) + pow(xPtB.xX-xPtA.xX,2));
    }

    static void xPointsOnLine(Point_t<float> xPtA, Point_t<float> xPtB,
                              int xNumPts,
                              std::vector<Point_t<float>>& xPointVec)
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

    uint16_t operator ++ (int) const
    {
        return xPoint++ + usSignature + usWidth + usHeight + usAngle;
	}

    operator Point_t<float> ()
    {
        return Point_t<float>(xPoint.xX, xPoint.xY);
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

        operator Point_t<T> ()
        {
            return Point_t<T>(this->xX, this->xY);
        }

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

        std::string xStr() const
        {
            Point_t<float>::xPointStr(Point_t<float>(this->xX, this->xY));
        }

    private:
    //  TODO: Refactor this class to use Point_t!!!
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

// TODO: Refactor to use PointStat_t, deprecate Stat_t
class Corners_t
{
    public:
        Corners_t()
        {
            xStats.resize(8);
        }

        Point_t<float> operator() (Quadrant_t xQuadrant) const
        {
            return Point_t<float>(xStats[2*xQuadrant].xMean(),
                                  xStats[2*xQuadrant+1].xMean());
        }

        void vUpdate(Quadrant_t& xQuadrant, Block_t& xBlock)
        {
            xStats[2*xQuadrant].vUpdate(xBlock.xPoint.xY);
            xStats[2*xQuadrant+1].vUpdate(xBlock.xPoint.xX);
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

    private:
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

        std::vector<Stat_t<float>> xStats;
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

        enum PrintMode_t {LOCATION, COLOR, OPENCV_META};

        Board_t() : ulRows(6), ulCols(7)
        {}

        bool xFillRow(uint32_t xRowNum, std::vector<Point_t<float>>& xRowVec)
        {
            std::cout << "xRowNum: " << xRowNum << std::endl
                      << "xRowVec.size(): " << xRowVec.size() << std::endl;
            if (xRowVec.size() > ulCols || xRowVec.size() < 0 ||
                xRowNum >= ulRows || xRowNum < 0)
            {
                return false;
            }
            std::vector<Chip_t> xTempChips;
            for (auto xColElem: xRowVec)
            {
                Chip_t xChip(xColElem);
                xTempChips.push_back(xChip);
            }
            xChips.insert(xChips.begin() + ulCols * xRowNum, xTempChips.begin(),
                          xTempChips.end());
            return true;
        }

        int xFillBoard(std::vector<Point_t<float>>& xTLBLPoints,
                        std::vector<Point_t<float>>& xTRBRPoints)
        {
            if (xTLBLPoints.size() != ulRows || xTRBRPoints.size() != ulRows)
            {
                return 1; // Invalid number of points
            }
            size_t xRightPtIdx = 0;
            xChips.clear();
            for (Point_t<float>& xLeftPt : xTLBLPoints)
            {
                std::vector<Point_t<float>> xPts;
                Point_t<float>& xRightPt = xTRBRPoints[xRightPtIdx++];
                Point_t<float>::xPointsOnLine(xLeftPt, xRightPt, ulCols, xPts);
                for (auto& xPt : xPts)
                {
                    xChips.push_back(Chip_t(xPt));
                }
            }
            return (xChips.size() == ulRows * ulCols) ? 0 : 2;
        }

        int vBuildGrid (Corners_t& xCorners)
        {
            std::vector<Point_t<float>> xTLBLPoints;
            std::vector<Point_t<float>> xTRBRPoints;

            Point_t<float>::xPointsOnLine(xCorners(TOP_LEFT),
                                          xCorners(BOT_LEFT), 6, xTLBLPoints);
            Point_t<float>::xPointsOnLine(xCorners(TOP_RIGHT),
                                          xCorners(BOT_RIGHT), 6, xTRBRPoints);

            ulDistBetweenRows = Point_t<float>::xCalcDist(
                    xCorners(TOP_LEFT), xCorners(BOT_LEFT)) / ulRows;
            ulDistBetweenCols = Point_t<float>::xCalcDist(
                    xCorners(TOP_LEFT), xCorners(TOP_RIGHT)) / ulCols;

            return xFillBoard(xTLBLPoints, xTRBRPoints);
        }

        int lUpdate(std::vector<Block_t>& xBlocks)
        {
            // TODO: Find projection matrix during xFillBoard(), use 2D
            // binary search on corrected points to locate closest
            float xMaxDist = ulDistBetweenCols * xTolerance;
            for (auto& xChip : xChips)
            {
                uint32_t ulChipIdx = 0;
                float xMinDist = 999999;
                std::pair<uint32_t, Block_t> xDistPair;
                for (auto& xBlock : xBlocks)
                {
                    Point_t<float> xChipPt(xChip.xPtLocation);
                    float xDist = xChipPt.xCalcDist(xBlock.xPoint);
                    if (xDist < xMinDist && xDist < xMaxDist)
                    {
                        xDistPair = std::make_pair(ulChipIdx, xBlock);
                    }
                    ulChipIdx++;
                }
                ulChipIdx = xDistPair.first;
                Block_t& xBlock = xDistPair.second;
                xChip.xChipColor.vUpdate(xBlock.usSignature);
                xChip.xPtLocation.vUpdate(xBlock.xPoint);
            }
            vPrintChips(COLOR);
        }

        void vPrintChips(PrintMode_t xPrintStyle = LOCATION)
        {
            std::ostringstream xOss;
            uint32_t ulIdx = 0;
            if (xChips.size() != ulRows * ulCols)
            {
                std::cout << "Chip print error: Num chips: " << xChips.size()
                          << ", rows: " << ulRows << "  cols: " << ulCols
                          << std::endl;
                return;
            }
            if (xPrintStyle == OPENCV_META)
            {
                xOss << "std::vector<cv::Point2f> point_vec;\n";
                for (auto& xChip : xChips)
                {
                    xOss << "point_vec.push_back("
                         << Point_t<float>::xOpenCVPt(xChip.xPtLocation)
                         << ");\n";
                }
                std::cout << xOss.str() << std::endl;
            }
            else
            {
                xOss << "Number of chips: " << xChips.size() << std::endl;
                for (size_t xI = 0; xI < ulRows; ++xI)
                {

                    xOss << "Row " << xI << ":";
                    for (size_t xJ = 0; xJ < ulCols; ++xJ)
                    {
                        if (xPrintStyle == COLOR)
                        {
                            ChipColor_t xChipColor =
                                (ChipColor_t)xChips[ulIdx].xChipColor.xMean();
                            xOss << " " << xStringMap[xChipColor] << std::endl;
                        }
                        else
                        {
                            xOss << " " << xChips[ulIdx].xPtLocation.xStr();
                        }
                        ulIdx++;
                    }
                    xOss << "\n";
                }
                std::cout << xOss.str() << std::endl;
            }
        }

        uint32_t ulExpectedTotalChips()
        {
            return ulRows * ulCols;
        }

        uint32_t ulActualTotalChips()
        {
            return xChips.size();
        }

        Corners_t xCorners;

    private:

        void vInitStringMap()
        {
            xStringMap[NONE] = std::string("NONE");
            xStringMap[GREEN] = std::string("GREEN");
            xStringMap[RED] = std::string("RED");
        }


        std::vector<Chip_t> xChips;
        std::map<ChipColor_t, std::string> xStringMap;
        uint32_t ulRows;
        uint32_t ulCols;
        uint32_t ulDistBetweenRows;
        uint32_t ulDistBetweenCols;
        float xTolerance = 0.75;
};


} // namespace pixy
} // namespace team9

#endif


