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
                usCamRowsHalf(usCamRows/2),
                usCamColsHalf(usCamCols/2)
        {}

        void vReset()
        {
            pBoard->vReset();
        }

        void vEMAAlphaUp()
        {
            pBoard->vAdjustEMAAlpha(true);
        }

        void vEMAAlphaDown()
        {
            pBoard->vAdjustEMAAlpha(false);
        }

        float xGetAlpha()
        {
            return pBoard->xGetAlpha();
        }

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
                    pBoard->vPayAttentionTo(-1, 0, 0, 0, 0, 0, -1);
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
            std::vector<std::pair<int, ChipColor_t>> xSeenChips;
            pPixyEyes->ulSeenBlocks(xBlocks);
            this->pBoard->vCalcSeenChips(xBlocks, xSeenChips);
            this->xLastSeen = xSeenChips;
            this->pBoard->vUpdate(xSeenChips);
            return this->pBoard->lChipChanged();
        }

        int lBotInsert(PixyCmd_t& xInsertCmd)
        {
            int lNewRow = pBoard->lInsert(xInsertCmd);
            if (lNewRow == -1)
            {
                xErrorQueue.push("Row overflowing!");
            }
            if (lNewRow == -2)
            {
                xErrorQueue.push("Column value out of bounds");
            }
            return lNewRow;
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

        void vPrintChips(Board_t::PrintMode_t xPrintMode,
                         bool bPrintLastSeen = false)
        {
            std::vector<std::pair<int, ChipColor_t>> xSeenChips;
            if (bPrintLastSeen)
            {
                xSeenChips = this->xLastSeen;
            }
            switch (xPrintMode)
            {
                case Board_t::LOCATION: pBoard->vLocationPrint(); break;
                case Board_t::COLOR: pBoard->vColorPrint(xSeenChips, false); break;
                case Board_t::OPENCV_META: pBoard->vOpenCVPrint(); break;
            }
        }

         void vPrintCorners(Board_t::PrintMode_t xPrintMode)
        {
            pBoard->vPrintChips(xPrintMode);
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

        std::unique_ptr<Board_t> pBoard;

        std::queue<std::string> xErrorQueue;
        std::queue<int> xUpdateQueue;

        std::vector<std::pair<int, ChipColor_t>> xLastSeen;
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

