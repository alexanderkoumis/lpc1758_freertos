#ifndef PIXY_BRAIN_HPP
#define PIXY_BRAIN_HPP

#include <vector>
#include <iostream>
#include <queue>
#include <algorithm>

#include "pixy.hpp"
#include "pixy/pixy_eyes.hpp"
#include "pixy/common.hpp"
#include "pixy/common/board.hpp"
#include "pixy/common/block.hpp"
#include "pixy/common/dims.hpp"

namespace team9
{
namespace pixy
{

class PixyBrain_t
{
    public:
        PixyBrain_t(Dims_t xCamDims_arg, ChipColor_t eColorCalib_arg,
                    uint32_t ulChipsToCalib_arg) :
                pBoard(new Board_t),
                xCamDims(xCamDims_arg),
                xCamDimsHalf(xCamDims.usRows/2, xCamDims.usCols/2),
                eColorCalib(eColorCalib_arg),
                ulChipsToCalib(ulChipsToCalib_arg)
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
                            xCorners.vUpdate(xQuadrant, xBlock);
                        }
                    }
                }
            }
            switch(pBoard->vBuildGrid(xCorners))
            {
                case 0:
                {
                    pBoard->xCorners = xCorners;
                    return true;
                }
                case 1:
                {
                    xErrorQueue.push("TLBL or TRBR lines had invalid # of pts");
                    break;
                }
                case 2:
                {
                    std::ostringstream xOss;
                    xOss << "Invalid # of chips after instantiation. "
                         << "Expected: " << pBoard->ulExpectedTotalChips()
                         << ", Actual: " << pBoard->ulActualTotalChips();
                    xErrorQueue.push(xOss.str());
                    break;
                }
            }
            xErrorQueue.push("Problem building grid");
            return false;
        }

        int lSampleChips(PixyEyes_t* pPixyEyes)
        {
            std::vector<Block_t> xBlocks;
            pPixyEyes->ulSeenBlocks(xBlocks);
            switch (pBoard->lUpdate(xBlocks))
            {

            }
            return 0;
        }

        int lGetUpdate()
        {
            if (xUpdateQueue.empty())
            {
                xErrorQueue.push("I have no updates for you! "
                                 "What do you want??");
                return -1;
            }
            return 0;
        }

        std::string xGetErrors()
        {
            size_t xErrIdx = 0;
            std::ostringstream oss;
            while (!xErrorQueue.empty())
            {
                oss << "Error #" << xErrIdx++ << ": "
                    << xErrorQueue.front() << "\n";
                xErrorQueue.pop();
            }
            return oss.str();
        }

        std::unique_ptr<Board_t> pBoard;

    private:

        __inline Quadrant_t xComputeQuadrant(Point_t<uint16_t>& xPoint)
        {
            if (xPoint.xX < 0 || xPoint.xY < 0 ||
                xPoint.xX >= xCamDims.usCols || xPoint.xY >= xCamDims.usRows)
            {
                xErrorQueue.push("Chip out of bounds: " + xPoint.xStr());
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



        std::queue<std::string> xErrorQueue;
        std::queue<int> xUpdateQueue;

        Corners_t xLastCorners;
        Dims_t xCamDims;
        Dims_t xCamDimsHalf;

        ChipColor_t eColorCalib;
        uint32_t ulChipsToCalib;

        std::string sLastError;
};

} // namespace pixy
} // namespace team9

#endif

