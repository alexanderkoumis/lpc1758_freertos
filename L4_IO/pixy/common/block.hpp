#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "pixy/common/point.hpp"

namespace team9
{
namespace pixy
{

struct Block_t
{
    Point_t<uint16_t> xPoint;
    uint16_t usSignature;
    uint16_t usWidth;
    uint16_t usHeight;
    uint16_t usAngle;

    uint16_t operator ++ (int) const
    {
        return xPoint++ + usSignature + usWidth + usHeight + usAngle;
    }

    operator Point_t<float> ()
    {
        return Point_t<float>(xPoint.xY, xPoint.xX);
    }

    friend std::ostream& operator << (std::ostream& xLHS,
                                      const Block_t& xRHS)
    {
        return xLHS << "Signature: " << xRHS.usSignature << std::endl
                    << "Point: " << xRHS.xPoint << std::endl
                    << "Size: [" << xRHS.usHeight << " "
                    << xRHS.usWidth << "]" << std::endl
                    << "Angle: " << xRHS.usAngle;
    }
};

}
}

#endif
