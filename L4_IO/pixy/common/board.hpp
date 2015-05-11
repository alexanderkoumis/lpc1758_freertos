#ifndef BOARD_HPP
#define BOARD_HPP

#include <vector>
#include <utility>
#include <queue>

#include "pixy/config.hpp"
#include "pixy/common.hpp"
#include "pixy/common/chip.hpp"
#include "pixy/common/corners.hpp"
#include "pixy/common/stat.hpp"

namespace team9
{
namespace pixy
{

class Board_t
{
    public:

        enum PrintMode_t {LOCATION, COLOR, OPENCV_META};

        Board_t() :
            ulRows(6), ulCols(7),
            xPointEMA_alpha(CHIP_LOC_EMA_ALPHA),
            xTolerance(CHIP_PROXIM_TOLERANCE)
        {
            vInitStringMap();
        }

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
                Chip_t xChip(xPointEMA_alpha);
                xChip.xPtLocation.vUpdate(xColElem);
                xTempChips.push_back(xChip);
            }
            xChips.insert(xChips.begin() + ulCols * xRowNum, xTempChips.begin(),
                          xTempChips.end());
            return true;
        }

        int vBuildGrid (Corners_t& xCorners_arg)
        {
            std::vector<Point_t<float>> xTLBLPoints;
            std::vector<Point_t<float>> xTRBRPoints;

            Point_t<float>::xPointsOnLine(xCorners_arg(TOP_LEFT),
                                          xCorners_arg(BOT_LEFT), 6,
                                          xTLBLPoints);
            Point_t<float>::xPointsOnLine(xCorners_arg(TOP_RIGHT),
                                          xCorners_arg(BOT_RIGHT), 6,
                                          xTRBRPoints);

            ulDistBetweenRows = Point_t<float>::xCalcDist(
                    xCorners_arg(TOP_LEFT), xCorners_arg(BOT_LEFT)) / ulRows;
            ulDistBetweenCols = Point_t<float>::xCalcDist(
                    xCorners_arg(TOP_LEFT), xCorners_arg(TOP_RIGHT)) / ulCols;

            return xFillBoard(xTLBLPoints, xTRBRPoints);
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
                    Chip_t xChip(xPointEMA_alpha);
                    xChip.xPtLocation.vUpdate(xPt);
                    xChips.push_back(xChip);
                }
            }
            return (xChips.size() == ulRows * ulCols) ? 0 : 2;
        }

        void vCalcSeenChips(
                std::vector<Block_t>& xBlocks,
                std::vector<std::pair<size_t, ChipColor_t>>& xSeenChips)
        {
            float xMaxDist = (float)ulDistBetweenCols * xTolerance;
            for (size_t xI = 0; xI < xChips.size(); ++xI)
            {
                Chip_t& xChip = xChips[xI];
                ChipColor_t xTempColor = NONE;
                float xTempDist = 9999;
                for (size_t xJ = 0; xJ < xBlocks.size(); ++xJ)
                {
                    Point_t<uint16_t>& xBlockPt = xBlocks[xJ].xPoint;
                    const Point_t<float>& xChipPt = xChip.xPtLocation.xPoint();
                    float xDist = Point_t<float>::xCalcDist(xChipPt, xBlockPt);
                    if (xDist < xTempDist)
                    {
                        xTempColor = (ChipColor_t)xBlocks[xJ].usSignature;
                        xTempDist = xDist;
                    }
                }
                if (xTempDist < xMaxDist)
                {
                    xSeenChips.push_back(std::make_pair(xI, xTempColor));
                }
            }
        }

        void vUpdate(std::vector<std::pair<size_t, ChipColor_t>>& xSeenChips)
        {
            for (size_t xI = 0; xI < xChips.size(); ++xI)
            {
                size_t xNoneCount = 0;
                size_t xGreenCount = 0;
                size_t xRedCount = 0;
                for (auto& xSeenChip : xSeenChips)
                {
                    if (xSeenChip.first == xI)
                    {
                        switch (xSeenChip.second)
                        {
                            case NONE: xNoneCount++; break;
                            case GREEN: xGreenCount++; break;
                            case RED: xRedCount++; break;
                        }
                    }
                }
                xChips[xI].vUpdateFreq(xNoneCount, xGreenCount, xRedCount);
            }
        }

        int lChipHasChanged()
        {
            // -2 == No change, -1 == Multiple changed, >0 == index of change
            int lReturnVal = -2;
            int lCurrChanged = 0;
            for (size_t xI = 0; xI < xChips.size(); ++xI)
            {
                Chip_t& xChip = xChips[xI];
                if (xChip.bChanged())
                {
                    if (lCurrChanged == 0)
                    {
                        lReturnVal = xI;
                    }
                    else if (lCurrChanged == 1)
                    {
                        xChangedChips.push(lReturnVal);
                        xChangedChips.push(xI);
                        lReturnVal = -1;
                    }
                    else if (lCurrChanged > 1)
                    {
                        xChangedChips.push(xI);
                    }
                    lCurrChanged++;
                }
            }
            return lReturnVal;
        }

        void vGetChanged(std::vector<int>& xChanged)
        {
            while (!xChangedChips.empty())
            {
                xChanged.push_back(xChangedChips.front());
                xChangedChips.pop();
            }
        }

        void vReset()
        {
            for (auto& xChip : xChips)
            {
                xChip.vReset();
            }
        }

        void vPrintChips(
                PrintMode_t xPrintStyle = LOCATION,
                const std::vector<std::pair<size_t, ChipColor_t>> xSeenChips =
                      std::vector<std::pair<size_t, ChipColor_t>>())
        {
            if (xChips.size() != ulRows * ulCols)
            {
                std::cout << "Chip print error: Num chips: " << xChips.size()
                          << ", rows: " << ulRows << "  cols: " << ulCols
                          << std::endl;
                return;
            }
            switch (xPrintStyle)
            {
                case LOCATION: vLocationPrint(); break;
                case COLOR: vColorPrint(xSeenChips); break;
                case OPENCV_META: vOpenCVPrint(); break;
            }
        }

        void vLocationPrint()
        {
            std::ostringstream xOss;
            uint32_t ulIdx = 0;
            for (size_t xI = 0; xI < ulRows; ++xI)
             {
                 xOss << "Row: " << xI;
                 for (size_t xJ = 0; xJ < ulCols; ++xJ)
                 {
                     xOss << " " << xChips[ulIdx++].xPtLocation.xStr();
                     ulIdx++;
                 }
                 xOss << "\n";
             }
             std::cout << xOss.str() << std::endl;
        }

        void vColorPrint(
                const std::vector<std::pair<size_t, ChipColor_t>> xSeenChips =
                      std::vector<std::pair<size_t, ChipColor_t>>())
        {
            std::ostringstream xOss;
            uint32_t ulIdx1 = 0;
            uint32_t ulIdx2 = 0;
            bool bSeenChipsEmpty = xSeenChips.empty();
            xOss << "Col:   0 1 2 3 4 5 6";
            xOss << ((bSeenChipsEmpty) ? "\n" : "     0 1 2 3 4 5 6\n");
            for (size_t xI = 0; xI < ulRows; ++xI)
            {
                xOss << "Row " << xI << ":";
                if (!bSeenChipsEmpty)
                {
                    //// Print incoming chip colors
                    for (size_t xJ = 0; xJ < ulCols; ++xJ)
                    {
                        ChipColor_t xChipColor = NONE;
                        for (auto& xChipColorPair : xSeenChips)
                        {
                            if (xChipColorPair.first == ulIdx1)
                            {
                                xChipColor = xChipColorPair.second;
                            }
                        }
                        ulIdx1++;
                        xOss << " " << xStringMap[xChipColor];
                    }
                    xOss << "     ";
                }
                //// Print known chip colors
                for (size_t xJ = 0; xJ < ulCols; ++xJ)
                {
                    ChipColor_t xChipColor = xChips[ulIdx2++].xMaxChip();
                    xOss << " " << xStringMap[xChipColor];
                }
                xOss << "\n";
            }
            std::cout << xOss.str() << std::endl;
        }

        void vOpenCVPrint()
        {
            std::ostringstream xOss;
            xOss << "std::vector<cv::Point2f> point_vec;\n";
            for (auto& xChip : xChips)
            {
                xOss << "point_vec.push_back("
                     << Point_t<float>::xOpenCVPt(
                        xChip.xPtLocation.xPoint()) << ");\n";
            }
            std::cout << xOss.str() << std::endl;
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
            xStringMap[NONE] = "-";
            xStringMap[GREEN] = "G";
            xStringMap[RED] = "R";
        }

        std::vector<Chip_t> xChips;
        std::map<ChipColor_t, std::string> xStringMap;
        std::queue<int> xChangedChips;
        uint32_t ulRows;
        uint32_t ulCols;
        uint32_t ulDistBetweenRows;
        uint32_t ulDistBetweenCols;
        float xPointEMA_alpha;
        float xTolerance;
};


} // namespace pixy
} // namespace team9

#endif
