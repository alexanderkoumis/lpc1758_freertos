#ifndef CONNECT4_BOARD_HPP
#define CONNECT4_BOARD_HPP

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
            uint16_t usX;
            uint16_t usY;
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
        }

//        bool vSampleCalibrationChips(std::vector<cmu::PixyBlock2>& xCalibChips)
        bool vSampleCalibrationChips(std::vector<cmu::PixyBlock>& xCalibChips)
        {
            for (auto& xCalibChip : xCalibChips)
            {
                if (xCalibChip.signature == GREEN)
                {
                    usGreenChips++;
                    if (xCalibChip.x < 0 || xCalibChip.y < 0 ||
                        xCalibChip.x >= xCalibParams.ulCamRows ||
                        xCalibChip.y >= xCalibParams.ulCamCols)
                    {
                        u0_dbg_printf("Calib Chip out of bounds\n");
                    }

                    else if (xCalibChip.x < ulColsHalf &&
                             xCalibChip.y < ulRowsHalf)
                    {
                        xBoardCorners.eTopLeft.usX += xCalibChip.x;
                        xBoardCorners.eTopLeft.usY += xCalibChip.y;
                    }
                    else if (xCalibChip.x < ulColsHalf &&
                             xCalibChip.y > ulRowsHalf)
                    {
                        xBoardCorners.eBottomLeft.usX += xCalibChip.x;
                        xBoardCorners.eBottomLeft.usY += xCalibChip.y;
                    }
                    else if (xCalibChip.x > ulColsHalf &&
                             xCalibChip.y < ulRowsHalf)
                    {
                        xBoardCorners.eTopRight.usX += xCalibChip.x;
                        xBoardCorners.eTopRight.usY += xCalibChip.y;
                    }
                    else if (xCalibChip.x > ulColsHalf &&
                             xCalibChip.y > ulRowsHalf)
                    {
                        xBoardCorners.eBottomRight.usX += xCalibChip.x;
                        xBoardCorners.eBottomRight.usY += xCalibChip.y;
                    }
                    else
                    {
                        u0_dbg_printf("How did this happen??\n");
                    }
                }
            }
            if (usGreenChips > 1000)
            {
                return false;
            }
            return true;
        }

        void vCalibrate()
        {
            xBoardCorners.eTopLeft.usX /= usGreenChips;
            xBoardCorners.eTopLeft.usY /= usGreenChips;
            xBoardCorners.eTopRight.usX /= usGreenChips;
            xBoardCorners.eTopRight.usY /= usGreenChips;
            xBoardCorners.eBottomLeft.usX /= usGreenChips;
            xBoardCorners.eBottomLeft.usY /= usGreenChips;
            xBoardCorners.eBottomRight.usX /= usGreenChips;
            xBoardCorners.eBottomRight.usY /= usGreenChips;
            u0_dbg_printf("Estimated corners:\n"
                          "TL: [%dx%d]\tTR: [%dx%d]\n"
                          "BL: [%dx%d]\tBR: [%dx%d]\n",
                          xBoardCorners.eTopLeft.usX,
                          xBoardCorners.eTopLeft.usY,
                          xBoardCorners.eTopRight.usX,
                          xBoardCorners.eTopRight.usY,
                          xBoardCorners.eBottomLeft.usX,
                          xBoardCorners.eBottomLeft.usY,
                          xBoardCorners.eBottomRight.usX,
                          xBoardCorners.eBottomRight.usY);
            usGreenChips = 0;
        }

    private:
        uint32_t usCalibPic = 0;
        uint32_t usGreenChips = 0;
        uint32_t ulRowsHalf = 0;
        uint32_t ulColsHalf = 0;

};

}

#endif
