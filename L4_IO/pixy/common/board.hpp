#ifndef BOARD_HPP
#define BOARD_HPP

#include <vector>
#include <utility>
#include <queue>

#include "pixy/config.hpp"
#include "pixy/common.hpp"
#include "pixy/common/chip.hpp"
#include "pixy/common/corners.hpp"
#include "pixy/common/point.hpp"
#include "pixy/common/stat.hpp"

namespace team9
{
namespace pixy
{

class Board_t
{
    public:

        enum PrintMode_t {LOCATION, COLOR, OPENCV_META};

        Board_t() : xWatchedChips(7), ulRows(6), ulCols(7),
                    xPointEMA_alpha(CHIP_LOC_EMA_ALPHA),
                    xTolerance(CHIP_PROXIM_TOLERANCE)
        {
            vInitStringMap();
        }

        void vReset()
        {
            for (auto& xChip : xAllChips)
            {
                xChip.vReset();
            }
            for (auto& xWatchedChip : xWatchedChips)
            {
                xWatchedChip.vReset();
            }
            vPayAttentionTo(-1, -1, -1, -1, -1, -1, -1);
        }

        void vAdjustEMAAlpha(bool bEMAAlphaUp)
        {
            xPointEMA_alpha = (bEMAAlphaUp) ?
                    std::min(xPointEMA_alpha + EMA_ALPHA_ADJ, 0.9999f):
                    std::max(xPointEMA_alpha - EMA_ALPHA_ADJ, 0.0001f);
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
            xAllChips.insert(xAllChips.begin() + ulCols * xRowNum,
                             xTempChips.begin(), xTempChips.end());
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
            xAllChips.clear();
            for (Point_t<float>& xLeftPt : xTLBLPoints)
            {
                std::vector<Point_t<float>> xPts;
                Point_t<float>& xRightPt = xTRBRPoints[xRightPtIdx++];
                Point_t<float>::xPointsOnLine(xLeftPt, xRightPt, ulCols, xPts);
                for (auto& xPt : xPts)
                {
                    Chip_t xChip(xPointEMA_alpha);
                    xChip.xPtLocation.vUpdate(xPt);
                    xAllChips.push_back(xChip);
                }
            }
            return (xAllChips.size() == ulRows * ulCols) ? 0 : 2;
        }

        void vPayAttentionTo(int lCol0, int lCol1, int lCol2, int lCol3,
                             int lCol4, int lCol5, int lCol6)
        {
            if (!xWatchedChips.empty())
            {
                std::fill(xWatchedChips.begin(), xWatchedChips.end(), Chip_t());
            }
            xWatchedIdxs.clear();
            xWatchedIdxs.push_back(lCol0);
            xWatchedIdxs.push_back(lCol1);
            xWatchedIdxs.push_back(lCol2);
            xWatchedIdxs.push_back(lCol3);
            xWatchedIdxs.push_back(lCol4);
            xWatchedIdxs.push_back(lCol5);
            xWatchedIdxs.push_back(lCol6);
        }

        void vCalcSeenChips(
                std::vector<Block_t>& xBlocks,
                std::vector<std::pair<size_t, ChipColor_t>>& xSeenChips)
        {

//            for (Block_t& xBlock : xBlocks)
//            {
//                bool bDone = false;
//                float xDistCurr = 0.0;
//
//                size_t xIdx = 0;
//
//                while (!bDone)
//                {
//                    size_t xRow = xIdx / 7;
//                    size_t xCol = xIdx % 7;
//                    bool bCheckRight = true;
//                    bool bCheckDown = true;
//
//                    if (xRow == 6 || xCol == 7)
//                    {
//                        bDone = true;
//                        break;
//                    }
//
//                    bCheckRight = (xCol <= 5);
//                    bCheckDown = (xRow <= 4);
//
//                    xDistCurr = xAllChips[xIdx].xPtLocation.xPoint().xCalcDist(xBlock.xPoint);
//                    float xDistRight = (bCheckRight) ? xAllChips[xIdx+1].xPtLocation.xPoint().xCalcDist(xBlock.xPoint) : 0.0f;
//                    float xDistDown = (bCheckDown) ? xAllChips[xIdx+7].xPtLocation.xPoint().xCalcDist(xBlock.xPoint) : 0.0f;
//
//                    if (bCheckRight && bCheckRight)
//                    {
//                        if (xDistCurr < xDistRight && xDistCurr < xDistDown)
//                        {
//                            bDone = true;
//                            break;
//                        }
//                        xIdx += (xDistRight < xDistDown) ? 1 : 7;
//                    }
//                    else if (bCheckRight) // Don't check down, row == 5
//                    {
//                        if (xDistCurr < xDistRight)
//                        {
//                            bDone = true;
//                            break;
//                        }
//                        xIdx += 1;
//                    }
//                    else if (bCheckDown) // Don't check right, col == 6
//                    {
//                        if (xDistCurr < xDistDown)
//                        {
//                            bDone = true;
//                            break;
//                        }
//                        xIdx += 7;
//                    }
//                    else
//                    {
//                        bDone = true;
//                    }
//                }
//                if (xDistCurr < ulDistBetweenCols * xTolerance)
//                {
//                    xSeenChips.push_back(std::make_pair(xIdx, (ChipColor_t)xBlock.usSignature));
//                }
//            }

            float xMaxDist = (float)ulDistBetweenCols * xTolerance;
            for (size_t xI = 0; xI < xWatchedChips.size(); ++xI)
            {
                if (xWatchedIdxs[xI] < 0 || xWatchedIdxs[xI] > ulRows)
                {
                    continue;
                }
                Chip_t& xChip = xAllChips[xWatchedIdxs[xI]];
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
//            for (size_t xI = 0; xI < xAllChips.size(); ++xI)
            for (size_t xI = 0; xI < xWatchedIdxs.size(); ++xI)
            {
                if (xWatchedIdxs[xI] < 0)
                {
                    continue;
                }
                size_t xNoneCnt = 0;
                size_t xGreenCnt = 0;
                size_t xRedCnt = 0;
                for (auto& xSeenChip : xSeenChips)
                {
                    if (xSeenChip.first == xI)
                    {
                        switch (xSeenChip.second)
                        {
                            case NONE: xNoneCnt++; break;
                            case GREEN: xGreenCnt++; break;
                            case RED: xRedCnt++; break;
                        }
                    }
                }
                xWatchedChips[xI].vUpdateFreq(xNoneCnt, xGreenCnt, xRedCnt);
            }
        }

        // Results in -1 if no change. If change, value is col. index of change
        int lChipChanged()
        {
            for (size_t& xIdx : xWatchedIdxs)
            {
                if (xIdx < 0)
                {
                    continue;
                }
                if (xWatchedChips[xIdx].bNotNoneAndTheSameNTimes<5>())
                {
                    xWatchedChips[xIdx].vReset();
                    return xIdx++;
                }
            }
            return -1;
        }

        uint32_t ulExpectedTotalChips()
        {
            return ulRows * ulCols;
        }

        uint32_t ulActualTotalChips()
        {
            return xAllChips.size();
        }

        float xGetAlpha()
        {
            return xPointEMA_alpha;
        }

        void vPrintChips(
                PrintMode_t xPrintStyle = LOCATION,
                const std::vector<std::pair<size_t, ChipColor_t>> xSeenChips =
                      std::vector<std::pair<size_t, ChipColor_t>>())
        {
            if (xAllChips.size() != ulRows * ulCols)
            {
                std::cout << "Chip print error: Num chips: " << xAllChips.size()
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
                     xOss << " " << xAllChips[ulIdx++].xLocStr();
                 }
                 xOss << "\n";
             }
             std::cout << xOss.str() << std::endl;
        }


        void vColorPrint(
                const std::vector<std::pair<size_t, ChipColor_t>> xSeenChips =
                      std::vector<std::pair<size_t, ChipColor_t>>(),
                bool bPrintStdDev = false)
        {
            std::ostringstream xOss;
            uint32_t ulIdx1 = 0;
            uint32_t ulIdx2 = 0;
            uint32_t ulIdx3 = 0;
            bool bSeenChipsEmpty = xSeenChips.empty();
            xOss << "Col:   0 1 2 3 4 5 6";
            xOss << ((bSeenChipsEmpty) ? "" : "      0 1 2 3 4 5 6");
            xOss << ((bPrintStdDev) ?
                    "         0       1       2       3       4       5       6\n" : "\n");
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
                    ChipColor_t xChipColor = xAllChips[ulIdx2++].xMaxChip();
                    xOss << " " << xStringMap[xChipColor];
                }
                //// Print standard deviation
                if (bPrintStdDev)
                {
                    xOss << "     ";
                    for (size_t xJ = 0; xJ < ulCols; ++xJ)
                    {
                        if (xAllChips[ulIdx3++].xGetColor(StatEnum_t::S) > 0.0f)
                        {
                            xOss << " " << xAllChips[ulIdx3++].xStdDevStr();
                        }
                        else
                        {
                            xOss << "    -   ";
                        }
                    }
                }
                xOss << "\n";
            }
            std::cout << xOss.str() << std::endl;
        }

        void vOpenCVPrint()
        {
            std::ostringstream xOss;
            xOss << "std::vector<cv::Point2f> point_vec;\n";
            for (auto& xChip : xAllChips)
            {
                xOss << "point_vec.push_back("
                     << Point_t<float>::xOpenCVPt(
                        xChip.xPtLocation.xPoint()) << ");\n";
            }
            std::cout << xOss.str() << std::endl;
        }

        Corners_t xCorners;

    private:
        void vInitStringMap()
        {
            xStringMap[NONE] = "-";
            xStringMap[GREEN] = "G";
            xStringMap[RED] = "R";
        }

        std::vector<Chip_t> xAllChips;
        std::vector<Chip_t> xWatchedChips;
        std::vector<size_t> xWatchedIdxs;
        uint8_t ucInterestingMask;

        std::map<ChipColor_t, std::string> xStringMap;
//        std::queue<size_t> xChangedChips;
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
