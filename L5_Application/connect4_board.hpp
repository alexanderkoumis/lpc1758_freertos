#ifndef CONNECT4_BOARD_HPP
#define CONNECT4_BOARD_HPP

#include <cstring>
#include <vector>

#include "printf_lib.h"

namespace team9
{

enum ChipColor_t {RED = 1, YELLOW = 2, GREEN = 3};

class Connect4Board_t
{

    public:

        struct Point2D_t
        {
            uint32_t usX;
            uint32_t usY;
        };

        struct CalibParams_t
        {
            uint32_t ulCamRows;
            uint32_t ulCamCols;
            uint32_t ulNumFrames;
            ChipColor_t eChipColor;
        } xCalibParams;

        struct BoardCorners_t
        {
            Point2D_t eTopLeft;
            Point2D_t eTopRight;
            Point2D_t eBottomLeft;
            Point2D_t eBottomRight;
        } xBoardCorners;

        Connect4Board_t(CalibParams_t& xCalibParams_arg) :
            xCalibParams(xCalibParams_arg)
        {
            ulColsHalf = xCalibParams.ulCamCols / 2;
            ulRowsHalf = xCalibParams.ulCamRows / 2;
            usChipCount[0] = 0;
            usChipCount[1] = 0;
            usChipCount[2] = 0;
            usChipCount[3] = 0;
        }

        uint32_t ulFrames = 0;
        uint32_t usChipCount[4];

//        bool vSampleCalibrationChips(std::vector<cmu::PixyBlock2>& xCalibChips)
        bool vSampleCalibrationChips(std::vector<cmu::PixyBlock>& xCalibChips)
        {
            for (auto& xCalibChip : xCalibChips)
            {
                if (xCalibChip.signature == GREEN)
                {
                    if (xCalibChip.x < 0x0000 || xCalibChip.y < 0x0000 ||
                        xCalibChip.x >= xCalibParams.ulCamCols ||
                        xCalibChip.y >= xCalibParams.ulCamRows)
                    {
                        u0_dbg_printf("Calib Chip out of bounds (cols: %d rows: %d)\n", xCalibParams.ulCamCols, xCalibParams.ulCamRows);
                        u0_dbg_printf("x: %d\n y: %d\n", xCalibChip.x, xCalibChip.y);
                    }

                    else if (xCalibChip.x < ulColsHalf &&
                             xCalibChip.y < ulRowsHalf)
                    {
                    	usChipCount[0]++;
                        xBoardCorners.eTopLeft.usX += xCalibChip.x;
                        xBoardCorners.eTopLeft.usY += xCalibChip.y;
                    }
                    else if (xCalibChip.x < ulColsHalf &&
                             xCalibChip.y > ulRowsHalf)
                    {
                    	usChipCount[1]++;
                        xBoardCorners.eBottomLeft.usX += xCalibChip.x;
                        xBoardCorners.eBottomLeft.usY += xCalibChip.y;
                    }
                    else if (xCalibChip.x > ulColsHalf &&
                             xCalibChip.y < ulRowsHalf)
                    {
                    	usChipCount[2]++;
                        xBoardCorners.eTopRight.usX += xCalibChip.x;
                        xBoardCorners.eTopRight.usY += xCalibChip.y;
                    }
                    else if (xCalibChip.x > ulColsHalf &&
                             xCalibChip.y > ulRowsHalf)
                    {
                    	usChipCount[3]++;
                        xBoardCorners.eBottomRight.usX += xCalibChip.x;
                        xBoardCorners.eBottomRight.usY += xCalibChip.y;
                    }
                    else
                    {
                        u0_dbg_printf("How did this happen??\n");
                    }
                }
            }
            if (ulFrames++ == xCalibParams.ulNumFrames)
            {
                return false;
            }
            return true;
        }

        void vCalibrate()
        {
            xBoardCorners.eTopLeft.usX /= usChipCount[0];
            xBoardCorners.eTopLeft.usY /= usChipCount[0];
            xBoardCorners.eBottomLeft.usX /= usChipCount[1];
            xBoardCorners.eBottomLeft.usY /= usChipCount[1];
            xBoardCorners.eTopRight.usX /= usChipCount[2];
            xBoardCorners.eTopRight.usY /= usChipCount[2];
            xBoardCorners.eBottomRight.usX /= usChipCount[3];
            xBoardCorners.eBottomRight.usY /= usChipCount[3];
            u0_dbg_printf("Estimated corners:\n"
                          "TL: [%3d x %3d] TR: [%3d x %3d]\n"
                          "BL: [%3d x %3d] BR: [%3d x %3d]\n",
                          xBoardCorners.eTopLeft.usY,
                          xBoardCorners.eTopLeft.usX,
                          xBoardCorners.eTopRight.usY,
                          xBoardCorners.eTopRight.usX,
                          xBoardCorners.eBottomLeft.usY,
                          xBoardCorners.eBottomLeft.usX,
                          xBoardCorners.eBottomRight.usY,
                          xBoardCorners.eBottomRight.usX);
            ulFrames = 0;
            memset(usChipCount, 0, 4 * sizeof(uint32_t));
            xBoardCorners.eTopLeft.usX = 0;
            xBoardCorners.eTopLeft.usY = 0;
            xBoardCorners.eBottomLeft.usX =0;
            xBoardCorners.eBottomLeft.usY = 0;
            xBoardCorners.eTopRight.usX = 0;
            xBoardCorners.eTopRight.usY = 0;
            xBoardCorners.eBottomRight.usX = 0;
            xBoardCorners.eBottomRight.usY = 0;
        }

    private:
        uint32_t usCalibPic = 0;
        uint32_t ulRowsHalf = 0;
        uint32_t ulColsHalf = 0;

};

}

#endif
