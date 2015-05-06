#ifndef PIXY_BRAIN_HPP
#define PIXY_BRAIN_HPP

#include <vector>
#include <iostream>
#include <algorithm>

#include "L5_Application/source/libfixmatrix/fixmatrix.h"
#include "L5_Application/source/libfixmatrix/fixvector3d.h"

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
                eColorCalib(eColorCalib_arg), ulChipsToCalib(ulChipsToCalib_arg)
        {}

        bool vCalibBoard(PixyEyes_t* pPixyEyes)
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
                            vUpdateCorners(xCorners, xQuadrant, xBlock);
                        }
                    }
                }
            }
            if (xCorners())
            {
                return true;
            }
            sLastError = "Poorly calibrated corners";
            return false;
        }

        void vInitBoard(PixyEyes_t* pPixyEyes, std::vector<Point_t>& xPoints)
        {
            float xRadDistParam;
            vComputeRadialDistortion(xCorners, xRadDistParam);
        }

        std::string sGetLastError()
        {
            return sLastError;
        }

        Corners_t xGetCorners()
        {
            return xCorners;
        }

    private:

        std::string sLastError;
        Corners_t xCorners;
        Dims_t xCamDims;
        Dims_t xCamDimsHalf;
        ChipColor_t eColorCalib;
        uint32_t ulChipsToCalib;

        std::unique_ptr<Board_t> pBoard;

        __inline Quadrant_t xComputeQuadrant(Point_t& xPoint)
        {
            if (xPoint.ulX < 0 || xPoint.ulY < 0 ||
                xPoint.ulX >= xCamDims.usCols || xPoint.ulY >= xCamDims.usRows)
            {
                sLastError = "Chip out of bounds: " + xPoint.sStr();
                return ERROR;
            }
            else if (xPoint.ulX <= xCamDimsHalf.usCols &&
                     xPoint.ulY <= xCamDimsHalf.usRows)
            {
                return TOP_LEFT;
            }
            else if (xPoint.ulX <= xCamDimsHalf.usCols &&
                     xPoint.ulY > xCamDimsHalf.usRows)
            {
                return BOT_LEFT;
            }
            else if (xPoint.ulX > xCamDimsHalf.usCols &&
                     xPoint.ulY <= xCamDimsHalf.usRows)
            {
                return TOP_RIGHT;
            }
            else if (xPoint.ulX > xCamDimsHalf.usCols &&
                     xPoint.ulY > xCamDimsHalf.usRows)
            {
                return BOT_RIGHT;
            }
            sLastError = "Point was not in any quadrant or out of bounds??";
            return ERROR;
        }

        __inline void vUpdateCorners(Corners_t& xCorners_arg,
                                     Quadrant_t& xQuadrant, Block_t& xBlock)
        {
            uint32_t usCenterRow = xBlock.xPoint.ulY + (xBlock.usHeight/2);
            uint32_t usCenterCol = xBlock.xPoint.ulX + (xBlock.usWidth/2);
            xCorners_arg.xStats[2*xQuadrant].vUpdate(usCenterRow);
            xCorners_arg.xStats[2*xQuadrant+1].vUpdate(usCenterCol);
        }

        void vComputeRadialDistortion(Corners_t& xCorners_arg,
                                               float& xLambda)
        {
            auto xTL = xCorners.xGet(TOP_LEFT);
            auto xTR = xCorners.xGet(TOP_RIGHT);
            auto xBL = xCorners.xGet(BOT_LEFT);

            std::cout << "TL: " << std::get<0>(xTL) << " " << std::get<1>(xTL) << std::endl;
            std::cout << "TL: " << std::get<0>(xTR) << " " << std::get<1>(xTR) << std::endl;
            std::vector<Point_t> xPoints;
            xPointsOnLine<float>(xTL, xBL, 6, xPoints);
            Point_t::vPrintPoints(xPoints);
        }





};

} // namespace pixy
} // namespace team9

#endif
