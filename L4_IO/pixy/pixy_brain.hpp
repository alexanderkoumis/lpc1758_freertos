#ifndef PIXY_BRAIN_HPP
#define PIXY_BRAIN_HPP

#include <vector>
#include <iostream>
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
                            xCorners.vUpdate(xQuadrant, xBlock.xPoint);
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

};

} // namespace pixy
} // namespace team9

#endif
