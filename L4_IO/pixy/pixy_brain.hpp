#ifndef PIXY_BRAIN_HPP
#define PIXY_BRAIN_HPP

#include <iostream>

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
                uint32_t ulCalibFrames_arg) :
        xCamDims(xCamDims_arg),
        xCamDimsHalf(xCamDims / 2),
        eColorCalib(eColorCalib_arg),
        ulCalibFrames(ulCalibFrames_arg)
    {
    }

    bool vCalibBoard(PixyEyes_t* pPixyEyes)
    {
        std::vector<Block_t> xBlocks;
        uint32_t ulCurrFrame = 0;


        while (1)
        {
        uint32_t ulNumBlocks = pPixyEyes->ulSeenBlocks(xBlocks);
        for (auto& xBlock : xBlocks)
        {
            std::cout << xBlock << std::endl;
//            if (xBlock.usSignature == eColorCalib)
//            {
//                Quadrant_t xQuad = xComputeQuadrant(xBlock.xPoint);
//                if (xQuad == ERROR)
//                {
//                    std::cout << sLastError << std::endl;
//                }
//                else
//                {
//                    xCorners.vAccumQuadrant(xQuad, xBlock.xPoint);
//                }
//            }
        }
        xBlocks.clear();
        }
        if (xCorners.vComputeAverage())
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
    Dims_t xCamDims;
    Dims_t xCamDimsHalf;
    Corners_t xCorners;
    ChipColor_t eColorCalib;
    uint32_t ulCalibFrames;

    __inline Quadrant_t xComputeQuadrant(Point_t& xPoint)
    {
        if (xPoint.usX < 0 || xPoint.usY < 0 ||
                xPoint.usX >= xCamDims.usCols || xPoint.usY >= xCamDims.usRows)
        {
            sLastError = "Chip out of bounds: " + xPoint.sStr();
            return ERROR;
        }
        else if (xPoint.usX <= xCamDimsHalf.usCols &&
                xPoint.usY <= xCamDimsHalf.usRows)
        {
            return TOP_LEFT;
        }
        else if (xPoint.usX  > xCamDimsHalf.usCols &&
                xPoint.usY <= xCamDimsHalf.usRows)
        {
            return TOP_LEFT;
        }
        else if (xPoint.usX <= xCamDimsHalf.usCols &&
                xPoint.usY  > xCamDimsHalf.usRows)
        {
            return TOP_LEFT;
        }
        else if (xPoint.usX <= xCamDimsHalf.usCols &&
                xPoint.usY  > xCamDimsHalf.usRows)
        {
            return TOP_LEFT;
        }
        sLastError = "Point was not in any quadrant or out of bounds??";
        return ERROR;
    }

};

} // namespace pixy
} // namespace team9

#endif
