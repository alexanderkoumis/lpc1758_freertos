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
            vPayAttentionTo(0, 0, 0, 0, 0, 0, 0);
        }

        void vAdjustEMAAlpha(bool bEMAAlphaUp)
        {
            xPointEMA_alpha = (bEMAAlphaUp) ?
                    std::min(xPointEMA_alpha + EMA_ALPHA_ADJ, 0.9999f):
                    std::max(xPointEMA_alpha - EMA_ALPHA_ADJ, 0.0001f);
        }

        bool xFillRow(int lRowNum, std::vector<Point_t<float>>& xRowVec)
        {
            if ((int)xRowVec.size() > ulCols ||
                 lRowNum >= ulRows || lRowNum < 0)
            {
                return false;
            }
            std::vector<Chip_t> xTempChips;
            for (auto lColElem: xRowVec)
            {
                Chip_t xChip(xPointEMA_alpha);
                xChip.xPtLoc.vUpdate(lColElem);
                xTempChips.push_back(xChip);
            }
            xAllChips.insert(xAllChips.begin() + ulCols * lRowNum,
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
            if ((int)xTLBLPoints.size() != ulRows ||
                (int)xTRBRPoints.size() != ulRows)
            {
                return 1; // Invalid number of points
            }
            int xRightPtIdx = 0;
            xAllChips.clear();
            for (Point_t<float>& xLeftPt : xTLBLPoints)
            {
                std::vector<Point_t<float>> xPts;
                Point_t<float>& xRightPt = xTRBRPoints[xRightPtIdx++];
                Point_t<float>::xPointsOnLine(xLeftPt, xRightPt, ulCols, xPts);
                for (auto& xPt : xPts)
                {
                    Chip_t xChip(xPointEMA_alpha);
                    xChip.xPtLoc.vUpdate(xPt);
                    xAllChips.push_back(xChip);
                }
            }
            return ((int)xAllChips.size() == ulRows * ulCols) ? 0 : 2;
        }

        void vPayAttentionTo(int lCol0, int lCol1, int lCol2, int lCol3,
                             int lCol4, int lCol5, int lCol6)
        {
            if (!xWatchedChips.empty())
            {
                std::fill(xWatchedChips.begin(), xWatchedChips.end(), Chip_t());
            }
            xWatchedCols.clear();
            xWatchedCols.push_back(lCol0);
            xWatchedCols.push_back(lCol1);
            xWatchedCols.push_back(lCol2);
            xWatchedCols.push_back(lCol3);
            xWatchedCols.push_back(lCol4);
            xWatchedCols.push_back(lCol5);
            xWatchedCols.push_back(lCol6);
        }

        void vCalcSeenChips(std::vector<Block_t>& xBlocks,
                std::vector<std::pair<int, ChipColor_t>>& xSeenChips)
        {
            switch (eSeenChipAlgo)
            {
                case STUPID: vChipAlgoStupid(xBlocks, xSeenChips); break;
                case DOWN_RIGHT: vChipAlgoDownRight(xBlocks, xSeenChips); break;
                default: std::cout << "Error selecting seen chip algo.\n";
            }
        }

        void vChipAlgoStupid(std::vector<Block_t>& xBlocks,
                std::vector<std::pair<int, ChipColor_t>>& xSeenChips)
        {
            float xMaxDist = (float)ulDistBetweenCols * xTolerance;
            for (int lCol = 0; lCol < (int)xWatchedChips.size(); ++lCol)
            {
                int xRow = xWatchedCols[lCol];
                if (xRow < 0 || xRow > (int)ulRows)
                {
                    continue;
                }
                int xIdx = (ulRows-xRow-1) * ulCols + lCol;
                Chip_t& xChip = xAllChips[xIdx];
                ChipColor_t xTempColor = NONE;
                float xTempDist = 9999;
                for (int lJ = 0; lJ < (int)xBlocks.size(); ++lJ)
                {
                    Point_t<uint16_t>& xBlockPt = xBlocks[lJ].xPoint;
                    const Point_t<float>& xChipPt = xChip.xPtLoc.xPoint();
                    float xDist = Point_t<float>::xCalcDist(xChipPt, xBlockPt);
                    if (xDist < xTempDist)
                    {
                        xTempColor = (ChipColor_t)xBlocks[lJ].usSignature;
                        xTempDist = xDist;
                    }
                }
                if (xTempDist < xMaxDist)
                {
                    xSeenChips.push_back(std::make_pair(xIdx, xTempColor));
                }
            }
        }

        void vChipAlgoDownRight(std::vector<Block_t>& xBlocks,
                std::vector<std::pair<int, ChipColor_t>>& xSeenChips)
        {
            for (Block_t& xBlock : xBlocks)
            {
                bool bDone = false;
                float xDistCurr = 0.0;
                size_t xIdx = 0;
                while (!bDone)
                {
                    size_t xRow = xIdx / 7;
                    size_t lCol = xIdx % 7;
                    bool bCheckRight = true;
                    bool bCheckDown = true;
                    if (xRow == 6 || lCol == 7)
                    {
                        bDone = true;
                        break;
                    }
                    bCheckRight = (lCol <= 5);
                    bCheckDown = (xRow <= 4);
                    // The following 10 lines tells me (functional) refactoring
                    // is in order
                    const Point_t<float>& xThisPt =
                            xAllChips[xIdx].xPtLoc.xPoint();
                    const Point_t<float>& xRPt =
                            xAllChips[xIdx+1].xPtLoc.xPoint();
                    const Point_t<float>& xDPt =
                            xAllChips[xIdx+1].xPtLoc.xPoint();
                    xDistCurr = xThisPt.xCalcDist(xBlock.xPoint);
                    float xDistRight = bCheckRight ?
                                       xRPt.xCalcDist(xBlock.xPoint) : 0.0f;
                    float xDistDown = bCheckDown ?
                                      xDPt.xCalcDist(xBlock.xPoint) : 0.0f;
                    if (bCheckRight && bCheckRight)
                    {
                        if (xDistCurr < xDistRight && xDistCurr < xDistDown)
                        {
                            bDone = true;
                            break;
                        }
                        xIdx += (xDistRight < xDistDown) ? 1 : 7;
                    }
                    else if (bCheckRight) // Don't check down, row == 5
                    {
                        if (xDistCurr < xDistRight)
                        {
                            bDone = true;
                            break;
                        }
                        xIdx += 1;
                    }
                    else if (bCheckDown) // Don't check right, col == 6
                    {
                        if (xDistCurr < xDistDown)
                        {
                            bDone = true;
                            break;
                        }
                        xIdx += 7;
                    }
                    else
                    {
                        bDone = true;
                    }
                }
                if (xDistCurr < ulDistBetweenCols * xTolerance)
                {
                    xSeenChips.push_back(
                        std::make_pair(xIdx, (ChipColor_t)xBlock.usSignature));
                }
            }
        }

        void vUpdate(std::vector<std::pair<int, ChipColor_t>>& xSeenChips)
        {
            for (int lCol = 0; lCol < (int)xWatchedCols.size(); ++lCol)
            {
                if (xWatchedCols[lCol] < 0)
                {
                    continue;
                }
                int xNoneCnt = 0;
                int xGreenCnt = 0;
                int xRedCnt = 0;

                int lRow = xWatchedCols[lCol];
                int lBoardIdx = ((int)ulRows-lRow-1) * ulCols + lCol;

                for (auto& xSeenChip : xSeenChips)
                {
//                  std::cout << "Checking out [" << lRow << "][" << lCol << "]"
//                            << "(" << lBoardIdx << "):"
//                            << xSeenChip.first << std::endl;
                    if (lRow < 0 || lRow > 5)
                    {
                        std::cout << "Warning, lRow in vUpdate is: " << lRow
                                  << ", xWatchedCols[" << lCol << "]: "
                                  << xWatchedCols[lCol] << std::endl;
                    }
                    if (xSeenChip.first == lBoardIdx)
                    {
                        switch (xSeenChip.second)
                        {
                            case NONE: xNoneCnt++; break;
                            case GREEN: xGreenCnt++; break;
                            case RED: xRedCnt++; break;
                        }
                    }
                }
                if (xNoneCnt || xGreenCnt || xRedCnt)
                {
                    std::cout << "This many: n" << xNoneCnt
                              << ", g: " << xGreenCnt
                              << ", r: " << xRedCnt << std::endl;
                    xWatchedChips[lCol].vUpdateFreq(xNoneCnt, xGreenCnt,
                                                    xRedCnt);
                }
            }
        }

        int lChipChanged()
        {
            // Poll all of the watched chips (indexes stored in xWatchedCols)
            // asking if they are "not none and the same N times"

            // For every entry in size 7 vector of "watched col heights"
            // (ex: [-1 0 0 0 0 0 -1])
            int lCols = xWatchedCols.size();
            for (int lCol = 0; lCol < lCols; ++lCol)
            {
                // Get current column height
                int lHeight = xWatchedCols[lCol];
                int lBoardIdx = (ulRows - lHeight - 1) * ulCols + lCol;

                // Out of bounds
                if ((lHeight < 0) || (lHeight > (ulRows - 1))) continue;

                // Dereference chip in current column (xWatchedChips same
                // size as xWatchedCols
                Chip_t& xChip = xWatchedChips[lCol];

                // Ask if it has been the same for the last two samples
                if (xChip.bChipKnown())
                {
                    xAllChips[lBoardIdx].vSet(xChip.xMaxChip());
                    xWatchedChips[lCol].vResetCounters();
                    xWatchedCols[lCol]++;
                    return ((ulRows-lHeight)*ulCols+lCol);
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
                const std::vector<std::pair<int, ChipColor_t>> xSeenChips =
                      std::vector<std::pair<int, ChipColor_t>>())
        {
            if ((int)xAllChips.size() != ulRows * ulCols)
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
            int lIdx = 0;
            for (int lI = ulRows-1; lI >= 0; --lI)
             {
                 xOss << "Row: " << lI;
                 for (int lJ = 0; lJ < ulCols; ++lJ)
                 {
                     xOss << " " << xAllChips[lIdx++].xLocStr();
                 }
                 xOss << "\n";
             }
             std::cout << xOss.str() << std::endl;
        }

        std::string xChipColorRow(std::ostringstream& xOss,
                                  ChipColor_t lCol0, ChipColor_t lCol1,
                                  ChipColor_t lCol2, ChipColor_t lCol3,
                                  ChipColor_t lCol4, ChipColor_t lCol5,
                                  ChipColor_t lCol6)
        {
            xOss << xStringMap[lCol0] << " "
                 << xStringMap[lCol1] << " "
                 << xStringMap[lCol2] << " "
                 << xStringMap[lCol3] << " "
                 << xStringMap[lCol4] << " "
                 << xStringMap[lCol5] << " "
                 << xStringMap[lCol6];
            std::string xRowStr(xOss.str());
            xOss.str("");
            xOss.clear();
            return xRowStr;
        }

        void vColorPrint(
                const std::vector<std::pair<int, ChipColor_t>> xSeenChips =
                      std::vector<std::pair<int, ChipColor_t>>(),
                bool bPrintStdDev = false)
        {
            std::ostringstream xOss;
            int lIdx1 = 0;
            int lIdx2 = 0;
            xOss << "Col:   0 1 2 3 4 5 6            0 1 2 3 4 5 6";
            xOss << "\n";
            for (int xI = (int)ulRows - 1; xI >= 0; --xI)
            {
                if (!xSeenChips.empty())
                {
                    xOss << "Row " << xI << ":";
                    //// Print incoming chip colors
                    for (int xJ = 0; xJ < (int)ulCols; ++xJ)
                    {
                        ChipColor_t xChipColor = NONE;
                        for (auto& xChipColorPair : xSeenChips)
                        {
                            if (xChipColorPair.first == lIdx1)
                            {
                                xChipColor = xChipColorPair.second;
                            }
                        }
                        lIdx1++;
                        xOss << " " << xStringMap[xChipColor];
                    }
                    xOss << "     ";
                }
                Chip_t& xChipRowICol0 = xAllChips[lIdx2++];
                Chip_t& xChipRowICol1 = xAllChips[lIdx2++];
                Chip_t& xChipRowICol2 = xAllChips[lIdx2++];
                Chip_t& xChipRowICol3 = xAllChips[lIdx2++];
                Chip_t& xChipRowICol4 = xAllChips[lIdx2++];
                Chip_t& xChipRowICol5 = xAllChips[lIdx2++];
                Chip_t& xChipRowICol6 = xAllChips[lIdx2++];
                xOss << "Row: " << xI << " "
                     << xStringMap[xChipRowICol0.xMaxChip()] << " "
                     << xStringMap[xChipRowICol1.xMaxChip()] << " "
                     << xStringMap[xChipRowICol2.xMaxChip()] << " "
                     << xStringMap[xChipRowICol3.xMaxChip()] << " "
                     << xStringMap[xChipRowICol4.xMaxChip()] << " "
                     << xStringMap[xChipRowICol5.xMaxChip()] << " "
                     << xStringMap[xChipRowICol6.xMaxChip()] << "\n";
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
                        xChip.xPtLoc.xPoint()) << ");\n";
            }
            std::cout << xOss.str() << std::endl;
        }

        void vPrintFillStatus()
        {
            std::ostringstream xOss;
            for (int xI = ulRows-1; xI >= 0; --xI)
            {
                xOss << "Row " << xI << ":";
                for (int xJ = 0; xJ < (int)ulCols; ++xJ)
                {
                    if (xWatchedCols[xJ] == (int)xI)
                    {
                        xOss << " " << xI;
                    }
                    else
                    {
                        xOss << " -";
                    }
                }
                xOss << "\n";
            }

            if (xOss.str().length() == 0)
            {
                xOss.str("");
                xOss.clear();
                return;
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
        std::vector<int> xWatchedCols;

        std::map<ChipColor_t, std::string> xStringMap;

        int ulRows;
        int ulCols;
        int ulDistBetweenRows;
        int ulDistBetweenCols;
        float xPointEMA_alpha;
        float xTolerance;
};

} // namespace pixy
} // namespace team9

#endif
