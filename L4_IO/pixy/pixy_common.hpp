#ifndef PIXY_COMMON_HPP
#define PIXY_COMMON_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <tuple>

namespace team9
{
namespace pixy
{

enum ChipColor_t {RED=1, YELLOW=2, GREEN=3};
enum Quadrant_t {TOP_LEFT=0, TOP_RIGHT=1, BOTTOM_LEFT=2, BOTTOM_RIGHT=3, ERROR};

struct Point_t
{
	uint16_t usX;
	uint16_t usY;

	Point_t(uint16_t usX_arg = 0, uint16_t usY_arg = 0)
	{
		std::tie(usX, usY) = std::tie(usX_arg, usY_arg);
	}

	Point_t& operator = (const Point_t& xRHS)
	{
		std::tie(usX, usY) = std::tie(xRHS.usX, xRHS.usY);
		return *this;
	}

	Point_t& operator += (const Point_t& xRHS)
	{
		std::tie(usX += xRHS.usX, usY += xRHS.usY);
		return *this;
	}

	Point_t operator + (const Point_t& xRHS) const
	{
		return Point_t(usX + xRHS.usX, usY + xRHS.usY);
	}

	Point_t operator /= (const uint32_t& ulDivisor) const
	{
		return Point_t(usX / ulDivisor, usY / ulDivisor);
	}

	bool operator () ()
	{
		return !(usX == 0 && usY == 0);
	}

	__inline uint16_t operator ++ (int) const
	{
		return usX + usY;
	}

	friend std::ostream& operator << (std::ostream& xLHS,
									  const Point_t& xRHS)
	{
		return xLHS << "[" << xRHS.usX << " " << xRHS.usY << "]";
	}

	__inline std::string sStr()
	{
		std::ostringstream oss;
		oss.str("");
		oss.clear();
		oss << this;
		return oss.str();
	}

};

struct Corners_t
{
	uint32_t pCornerCount[4];
	Point_t pCorners[4];

	Corners_t()
	{
		pCornerCount[TOP_LEFT] = 0;
		pCornerCount[TOP_RIGHT] = 0;
		pCornerCount[BOTTOM_LEFT] = 0;
		pCornerCount[BOTTOM_RIGHT] = 0;
	}

	bool operator ! ()
	{

	}

	friend std::ostream& operator << (std::ostream& xLHS,
									  const Corners_t& xRHS)
	{
		return xLHS << "["  << xRHS.pCorners[TOP_LEFT] << " "
				            << xRHS.pCorners[TOP_RIGHT] << ", "
							<< xRHS.pCorners[BOTTOM_LEFT] << " "
	                        << xRHS.pCorners[BOTTOM_RIGHT] << "]";
	}

	__inline void vAccumQuadrant(Quadrant_t& xQuadrant, Point_t& xPoint)
	{
		pCorners[xQuadrant] += xPoint;
		pCornerCount[xQuadrant]++;
	}

	bool vComputeAverage()
	{
		pCorners[TOP_LEFT] /= pCornerCount[TOP_LEFT];
		pCorners[TOP_RIGHT] /= pCornerCount[TOP_RIGHT];
		pCorners[BOTTOM_LEFT] /= pCornerCount[BOTTOM_LEFT];
		pCorners[BOTTOM_RIGHT] /= pCornerCount[BOTTOM_RIGHT];

		return pCorners[TOP_LEFT]() && pCorners[TOP_RIGHT]() &&
			   pCorners[BOTTOM_LEFT]() && pCorners[BOTTOM_RIGHT]();
	}

};

struct Dims_t
{
	uint16_t usCols;
	uint16_t usRows;

	Dims_t(uint16_t usCols_arg = 0, uint16_t usRows_arg = 0)
	{
		std::tie(usCols, usRows) = std::tie(usCols_arg, usRows_arg);
	}

	Dims_t& operator = (const Dims_t& xRHS)
	{
		std::tie(usCols, usRows) = std::tie(xRHS.usCols, xRHS.usRows);
		return *this;
	}

	Dims_t operator / (const uint32_t& ulDivisor) const
	{
		return Dims_t(usCols / ulDivisor, usRows / ulDivisor);
	}

	friend std::ostream& operator << (std::ostream& xStreamL,
									  const Dims_t& xRHS)
	{
		return xStreamL << "[" << xRHS.usCols << " " << xRHS.usRows << "]";
	}
};

struct Block_t
{
	Point_t xPoint;
	uint16_t usSignature;
	uint16_t usWidth;
	uint16_t usHeight;
	uint16_t usAngle;

	__inline uint16_t operator ++ (int) const
	{
		return xPoint++ + usSignature + usWidth + usHeight + usAngle;
	}

};

} // namespace pixy
} // namespace team9

#endif
