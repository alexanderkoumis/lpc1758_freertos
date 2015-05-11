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

namespace team9
{
namespace pixy
{

class PixyBrain_t
{
    public:
        PixyBrain_t(ChipColor_t eColorCalib_arg, uint32_t ulChipsToCalib_arg) :
                pBoard(new Board_t),
                eColorCalib(eColorCalib_arg),
                ulChipsToCalib(ulChipsToCalib_arg),
                usCamRows(200), usCamCols(320),
                usCamRowsHalf(usCamRows/2), usCamColsHalf(usCamCols/2)
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
            std::vector<std::pair<size_t, ChipColor_t>> xSeenChips;
            pPixyEyes->ulSeenBlocks(xBlocks);
            pBoard->vCalcSeenChips(xBlocks, xSeenChips);
            pBoard->vUpdate(xSeenChips);
            return pBoard->lChipHasChanged();
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

        inline Quadrant_t xComputeQuadrant(Point_t<uint16_t>& xPoint)
        {
            uint16_t& xY = xPoint.xY;
            uint16_t& xX = xPoint.xX;
            if (xY < 0 || xX < 0 || xY >= usCamRows || xX >= usCamCols)
            {
                xErrorQueue.push("Chip out of bounds: " + xPoint.xStr());
                return ERROR;
            }
            else if (xY <= usCamRowsHalf && xX <= usCamColsHalf)
            {
                return TOP_LEFT;
            }
            else if (xY > usCamRowsHalf && xX <= usCamColsHalf)
            {
                return BOT_LEFT;
            }
            else if (xY <= usCamRowsHalf && xX > usCamColsHalf)
            {
                return TOP_RIGHT;
            }
            else if (xY > usCamRowsHalf && xX > usCamColsHalf)
            {
                return BOT_RIGHT;
            }
            xErrorQueue.push("Point was not in any quadrant or out of bounds?");
            return ERROR;
        }

        std::queue<std::string> xErrorQueue;
        std::queue<int> xUpdateQueue;

        Corners_t xLastCorners;

        ChipColor_t eColorCalib;
        uint32_t ulChipsToCalib;

        uint16_t usCamRows;
        uint16_t usCamCols;
        uint16_t usCamRowsHalf;
        uint16_t usCamColsHalf;

        std::string sLastError;
};

} // namespace pixy
} // namespace team9

#endif

