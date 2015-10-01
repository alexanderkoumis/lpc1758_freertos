#ifndef PIXY_BRAIN_HPP
#define PIXY_BRAIN_HPP

#include <vector>
#include <iostream>
#include <queue>
#include <algorithm>
#include <stack>

#include "storage.hpp"

#include "pixy.hpp"
#include "pixy/pixy_eyes.hpp"
#include "pixy/common.hpp"
#include "pixy/common/board.hpp"
#include "pixy/common/block.hpp"

#define FUCK() printf("%d %s\n", __LINE__, __func__)

namespace team9
{
namespace pixy
{

class PixyBrain_t
{
    public:
        PixyBrain_t(ChipColor_t eColorCalib_arg, uint32_t ulChipsToCalib_arg) :
                pBoard(new Board_t),
                lLastInsertCol(0),
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

        void vCalibCorners(PixyEyes_t* pPixyEyes, Corners_t& xCorners)
        {
            uint32_t ulChips = 0;
            uint32_t ulCalibChips = 0;
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
        }

        void vSetCorners(const Corners_t& xCorners)
        {
            pBoard->xCorners = xCorners;
            pBoard->vBuildGrid(xCorners);
            printf("Corners:\n");
            Corners_t::vPrint(xCorners);
            printf("Board\n");
            // pBoard->vOpenCVPrint();
        }

        int lSampleChips(PixyEyes_t* pPixyEyes)
        {

            FUCK();std::vector<Block_t> xBlocks;
            FUCK();std::vector<std::pair<int, ChipColor_t>> xSeenChips;
            FUCK();pPixyEyes->ulSeenBlocks(xBlocks);
            FUCK();this->pBoard->vCalcSeenChips(xBlocks, xSeenChips);
            FUCK();this->xLastSeen = xSeenChips;
            FUCK();this->pBoard->vUpdate(xSeenChips);
            FUCK();int lLastChipInserted = this->pBoard->lColChanged();
            FUCK();return lLastChipInserted;
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
        std::stack<int> xColUpdate;

        std::vector<std::pair<int, ChipColor_t>> xLastSeen;
        Corners_t xLastCorners;
        int lLastInsertCol;

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

