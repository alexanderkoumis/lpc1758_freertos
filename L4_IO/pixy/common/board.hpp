#ifndef BOARD_HPP
#define BOARD_HPP

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

        Board_t() : ulRows(6), ulCols(7), xPointEMA_alpha(0.2), xTolerance(0.75)
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

        int lUpdate(std::vector<Block_t>& xBlocks)
        {
            // TODO: Find projection matrix during xFillBoard(), use 2D
            // binary search on corrected points to locate closest
            uint32_t ulChipIdx = 0;
            float xMaxDist = (float)ulDistBetweenCols * xTolerance;

            for (auto& xChip : xChips)
            {
                Block_t xNewBlock;
                float xMinDist = 9999;
                for (size_t xJ = ulChipIdx+1; xJ < xBlocks.size(); ++xJ)
                {
                    Block_t& xBlock = xBlocks[xJ];
                    Point_t<float> xChipPt = xChip.xPtLocation.xPoint();
                    float xDist = Point_t<float>::xCalcDist(xChipPt,
                                                            xBlock.xPoint);
                    if (xDist < xMinDist)
                    {
                        xNewBlock = xBlock;
                        xMinDist = xDist;
                    }
                }
                if (xMinDist < xMaxDist)
                 {
                    Chip_t::vUpdate(xChip, xNewBlock.usSignature);
                }
                ulChipIdx++;
            }
            return 0;
        }

        void vReset()
        {
            for (auto& xChip : xChips)
            {
                xChip.vReset();
            }
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
                         << Point_t<float>::xOpenCVPt(xChip.xPtLocation.xPoint())
                         << ");\n";
                }
                std::cout << xOss.str() << std::endl;
            }
            else
            {
                xOss << "Col:   0 1 2 3 4 5 6\n";
                for (size_t xI = 0; xI < ulRows; ++xI)
                {
                    xOss << "Row: " << xI;
                    for (size_t xJ = 0; xJ < ulCols; ++xJ)
                    {
                        if (xPrintStyle == COLOR)
                        {
                            ChipColor_t xChipColor = (ChipColor_t)xChips[ulIdx].lMaxChip();
                            xOss << " " << xStringMap[xChipColor];
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
            xStringMap[NONE] = "-";
            xStringMap[GREEN] = "G";
            xStringMap[RED] = "R";
        }

        std::vector<Chip_t> xChips;
        std::map<ChipColor_t, std::string> xStringMap;
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
