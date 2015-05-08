#ifndef PIXY_DISPLAY_HPP
#define PIXY_DISPLAY_HPP

#include <string>
#include <vector>
#include <iostream>
#include <printf_lib.h>

#include "pixy_common.hpp"

namespace team9
{
namespace pixy
{

class PixyDisplay_t
{
    public:
        PixyDisplay_t(std::string xPix_arg, uint32_t ulRows, uint32_t ulCols) :
            xPix(xPix_arg), ulCurrElems(0), xRows(ulRows), xCols(ulCols)
        {
            xScale = 2;
            xScaledRows = xRows / xScale;
            xScaledCols = xCols / xScale;
        }

        void vUpdate(size_t xRow, size_t xCol, bool xState)
        {
            if (xRow < xRows && xCol < xCols)
            {
                xRow /= xScale;
                xCol /= xScale;
                xPointVec.push_back(Point_t<uint32_t>(xRow, xCol));
            }
        }

        void vUpdate(std::vector<Point_t<uint32_t>>& xPoints, bool xState)
        {
            for (auto& xPoint : xPoints)
            {
                if (xPoint.xY < xRows && xPoint.xX < xCols)
                {
                    xPointVec.push_back(xPoint / xScale);
                }
            }
        }

//        void vPrint()
//        {
//            for (size_t i = 0; i < xScaledRows; ++i)
//            {
//                static std::string xRowStr = "";
//                for (size_t j = 0; j < xScaledCols; ++j)
//                {
//                    xRowStr += xVal(i, j);
//                }
//                std::cout << xRowStr << "\n\r";
//                xRowStr.clear();
//            }
//            std::cout << "\n\r" << std::endl;
//            xPointVec.clear();
//        }

    private:

        __inline std::string xVal(size_t xRow, size_t xCol)
        {
            return xIsInMat(xRow, xCol) ? "#" : " ";
        }

        __inline bool xIsInMat(size_t xRow, size_t xCol)
        {
            bool xFound = false;
            for (auto& xPair : xPointVec)
            {
                xFound |= (xPair == Point_t<uint32_t>(xRow, xCol));
            }
            return xFound;
        }

        std::vector<Point_t<uint32_t>> xPointVec;
        std::ostringstream oss;
        std::string xPix;
        uint32_t ulCurrElems;

        size_t xRows;
        size_t xCols;
        size_t xScale;
        size_t xScaledRows;
        size_t xScaledCols;
};

}
}

#endif
