#ifndef PIXY_BRAIN_HPP
#define PIXY_BRAIN_HPP

#include <vector>
#include <iostream>
#include <queue>
#include <algorithm>

#include "pixy.hpp"
#include "pixy/pixy_common.hpp"
#include "pixy/pixy_eyes.hpp"

namespace team9
{
namespace pixy
{

class PixyBrain_t
{
    public:

        PixyBrain_t(Dims_t xCamDims_arg, ChipColor_t eColorCalib_arg,
                uint32_t ulChipsToCalib_arg) :
                xCamDims(xCamDims_arg), xCamDimsHalf(xCamDims / 2),
                eColorCalib(eColorCalib_arg),
                ulChipsToCalib(ulChipsToCalib_arg),
                pBoard(new Board_t)
        {}

        bool vCalibBoard(PixyEyes_t* pPixyEyes)
        {
            uint32_t ulChips = 0;
            uint32_t ulCalibChips = 0;
            Corners_t xCorners;
            while (ulChips < ulChipsToCalib)
            {
                std::vector<Block_t> xBlocks;
                ulChips += pPixyEyes->ulSeenBlocks(xBlocks);
                for (auto& xBlock : xBlocks)
                {
                    if (xBlock.usSignature == eColorCalib)
                    {
                        ulCalibChips++;
                        Quadrant_t xQuadrant = xComputeQuadrant(xBlock.xPoint);
                        if (xQuadrant < 4)
                        {
                            vUpdateCorners(xCorners, xQuadrant, xBlock);
                        }
                    }
                }
            }
            if (xCorners())
            {
                xLastCorners = xCorners;
                if (vBuildGrid(xCorners))
                {
                    F();
                    return true;
                }
            }
            xErrorQueue.push("Poorly calibrated corners");
            return false;
        }

        std::string xGetErrors()
        {
            size_t xErrIdx = 0;
            std::ostringstream oss;
            while (!xErrorQueue.empty())
            {
                oss << "Error #" << xErrIdx << ": "
                    << xErrorQueue.front() << "\n";
                xErrorQueue.pop();
            }
            return oss.str();
        }

        Corners_t xGetCorners()
        {
            return xLastCorners;
        }

        void vPrintChips()
        {
            Board_t::vPrintChips(*pBoard);
        }

    private:

        std::unique_ptr<Board_t> pBoard;

        std::queue<std::string> xErrorQueue;

        Corners_t xLastCorners;
        Dims_t xCamDims;
        Dims_t xCamDimsHalf;
        ChipColor_t eColorCalib;
        uint32_t ulChipsToCalib;

        std::string sLastError;

        __inline Quadrant_t xComputeQuadrant(Point_t<uint16_t>& xPoint)
        {
            if (xPoint.xX < 0 || xPoint.xY < 0 ||
                xPoint.xX >= xCamDims.usCols || xPoint.xY >= xCamDims.usRows)
            {
                xErrorQueue.push("Chip out of bounds: " +
                                 Point_t<uint16_t>::xPointStr(xPoint));
                return ERROR;
            }
            else if (xPoint.xX <= xCamDimsHalf.usCols &&
                     xPoint.xY <= xCamDimsHalf.usRows)
            {
                return TOP_LEFT;
            }
            else if (xPoint.xX <= xCamDimsHalf.usCols &&
                     xPoint.xY > xCamDimsHalf.usRows)
            {
                return BOT_LEFT;
            }
            else if (xPoint.xX > xCamDimsHalf.usCols &&
                     xPoint.xY <= xCamDimsHalf.usRows)
            {
                return TOP_RIGHT;
            }
            else if (xPoint.xX > xCamDimsHalf.usCols &&
                     xPoint.xY > xCamDimsHalf.usRows)
            {
                return BOT_RIGHT;
            }
            xErrorQueue.push("Point was not in any quadrant or out of bounds?");
            return ERROR;
        }

        __inline void vUpdateCorners(Corners_t& xCorners,
                                     Quadrant_t& xQuadrant, Block_t& xBlock)
        {
            xCorners.xStats[2*xQuadrant].vUpdate(xBlock.xPoint.xY);
            xCorners.xStats[2*xQuadrant+1].vUpdate(xBlock.xPoint.xX);
        }

        bool vBuildGrid (Corners_t& xCorners)
        {
            std::vector<Point_t<float>> xTLBL;
            std::vector<Point_t<float>> xTRBR;
            xPointsOnLine(xCorners(TOP_LEFT), xCorners(BOT_LEFT), 6, xTLBL);
            xPointsOnLine(xCorners(TOP_RIGHT), xCorners(BOT_RIGHT), 6, xTRBR);
            size_t xRowIdx = 0;
            for (auto& xLPoint : xTLBL)
            {
                std::vector<Point_t<float>> xMID;
                xPointsOnLine(xLPoint, xTRBR[xRowIdx], 7, xMID);

                if (!pBoard->xFillRow(xRowIdx++, xMID))
                {
                    xErrorQueue.push("Row vector invalid size");
                    return false;
                }
            }
             return true;
        }
};

} // namespace pixy
} // namespace team9

#endif
