#ifndef PIXY_HPP
#define PIXY_HPP

#include <cstdio>
#include <iostream>
#include <map>
#include <functional>

#include "shared_handles.h"
#include "storage.hpp"

#include "pixy/config.hpp"
#include "pixy/common.hpp"
#include "pixy/common/func_map.hpp"
#include "pixy/common/board.hpp"
#include "pixy/pixy_brain.hpp"
#include "pixy/pixy_eyes.hpp"
#include "pixy/pixy_mouth.hpp"

#include "printf_lib.h"

namespace team9
{
namespace pixy
{

typedef enum {
    shared_motor_queue
} sharedHandleId_t;

class Pixy_t
{
    public:
        enum State_t
        {
            RESET=0x01,             // SW(1)
            CALIB=0x02,             // SW(2)
            START=0x04,             // SW(3)
            EMA_ALPHA_UP=0x08,      // SW(4)
            EMA_ALPHA_DOWN=0x09,    // SW(4&1)
            WAITING_FOR_HUMAN=17,
            WAITING_FOR_BOT=18,
            NOTIFY_BOT_OF_HUMAN_ACTIVIY=19,
            WAITING_FOR_RESET=20,
            ERROR=22
        } eState, eLastState;

        bool bReset;
        bool bCalib;
        int lLastHumanCol;

        Pixy_t () :
                bReset(false),
                bCalib(false),
                lLastHumanCol(-1),
                eState(WAITING_FOR_RESET),
                eLastState(WAITING_FOR_RESET)
        {}

        Pixy_t (uint32_t ulChipsAtATime, uint32_t ulChipsToCalib,
                ChipColor_t eColorCalib) :
                bReset(false),
                bCalib(false),
                lLastHumanCol(-1),
                eState(WAITING_FOR_RESET),
                eLastState(WAITING_FOR_RESET),
                pPixyBrain(new pixy::PixyBrain_t(eColorCalib, ulChipsToCalib)),
                pPixyEyes(new pixy::PixyEyes_t(ulChipsAtATime)),
                pPixyMouth(new pixy::PixyMouth_t),
                xFuncMap(new FuncMap_t<State_t, void>)
        {
            vInitMapFunc();
            vInitMapStr();
        }

        void vAction(uint8_t xButton)
        {
            if (xButton)
            {
                eState = (State_t)xButton;
            }
            printf("State: %s\n", xStringMap[eState].c_str());
            if (eLastState != eState)
            {
                printf("State change [%s -> %s]\n", xStringMap[eLastState].c_str(), xStringMap[eState].c_str());
            }
            eLastState = eState;
            xFuncMap->vResponse(eState)();
        }

    private:
        void vInitMapFunc()
        {

            xFuncMap->vSetHandler(CALIB, [&] ()
            {
                printf("Set calibration flag\n");
                bCalib = true;
                eState = WAITING_FOR_RESET;
            });

            xFuncMap->vSetHandler(RESET, [&] ()
            {
                printf("Resetting\n");
                pPixyBrain->vReset();
                bReset = true;
                eState = WAITING_FOR_RESET;
            });

            xFuncMap->vSetHandler(START, [&] ()
            {
                printf("Starting\n");
                eState = WAITING_FOR_HUMAN;
            });

            xFuncMap->vSetHandler(EMA_ALPHA_UP, [&] ()
            {
                pPixyBrain->vEMAAlphaUp();
                printf("Alpha: %f\n", pPixyBrain->xGetAlpha());
                eState = eLastState;
            });

            xFuncMap->vSetHandler(EMA_ALPHA_DOWN, [&] ()
            {
                pPixyBrain->vEMAAlphaDown();
                printf("Alpha: %f\n", pPixyBrain->xGetAlpha());
                eState = eLastState;
            });

            xFuncMap->vSetHandler(WAITING_FOR_RESET, [&] ()
            {
                if (bReset)
                {
                    bReset = false;
                    Corners_t xCorners;
                    if (bCalib)
                    {
                        printf("Corner calib from camera\n");
                        bCalib = false;
                        pPixyBrain->vCalibCorners(pPixyEyes.get(), xCorners);
                        char* corner_str = Corners_t::pcCornerStrRaw(xCorners);
                        Storage::write("/corners.calib", corner_str, 256, 0);
                        pPixyBrain->vSetCorners(xCorners);
                        eState = WAITING_FOR_RESET;
                    }
                    else if (Corners_t::bReadCorners("/corners.calib", xCorners))
                    {
                        printf("Problem loading corner calib, getting from camera\n");
                        pPixyBrain->vCalibCorners(pPixyEyes.get(), xCorners);
                        pPixyBrain->vSetCorners(xCorners);
                        eState = WAITING_FOR_RESET;
                    }
                    else
                    {
                        printf("Loaded corner calib from camera");
                        pPixyBrain->vSetCorners(xCorners);

                        eState = WAITING_FOR_HUMAN;
                    }
                }
                else
                {
                    eState = WAITING_FOR_RESET;
                }
            });

            xFuncMap->vSetHandler(WAITING_FOR_HUMAN, [&] ()
            {
                lLastHumanCol = pPixyBrain->lSampleChips(pPixyEyes.get());
                printf("Last Human Col: %d\n", lLastHumanCol);
                if (lLastHumanCol >= 0)
                {
                    pPixyBrain->vPrintChips(Board_t::COLOR, true);
                    eState = NOTIFY_BOT_OF_HUMAN_ACTIVIY;
                }
                else
                {
                    eState = WAITING_FOR_HUMAN;
                }
                pPixyBrain->vPrintCorners(Board_t::COLOR);
            });

            xFuncMap->vSetHandler(WAITING_FOR_BOT, [&] ()
            {
                PixyCmd_t xBotInsertCmd;

                if (xQueueReceive(
                        scheduler_task::getSharedObject(shared_PixyQueueRX),
                        &xBotInsertCmd, portMAX_DELAY))
                {
                    std::cout << "Bot column: " << xBotInsertCmd.lColumn << "\n"
                              << "Bot color: " << xBotInsertCmd.lColor << std::endl;
                    if (xBotInsertCmd.bReset)
                    {
                        printf("Reset command\n");
                        eState = RESET;
                        return;
                    }
                    ChipColor_t xChipColor = (ChipColor_t)xBotInsertCmd.lColor;
                    int lColumn = xBotInsertCmd.lColumn;
                    int lNewRow = pPixyBrain->lBotInsert(xBotInsertCmd);
                    if (lNewRow > 0)
                    {
                        std::cout << "After insertion of color " << xChipColor
                                  << " into column " << lColumn
                                  << ", column height is now: " << lNewRow
                                  << std::endl;
                    }
                    else
                    {
                        eState = ERROR;
                        return;
                    }
                    pPixyBrain->vPrintChips(Board_t::COLOR, true);
                    eState = WAITING_FOR_HUMAN;
                }
                else
                {
                    eState = WAITING_FOR_BOT;
                }
            });

            xFuncMap->vSetHandler(NOTIFY_BOT_OF_HUMAN_ACTIVIY, [&] ()
            {
                pPixyMouth->xEmitUpdate(lLastHumanCol);
                lLastHumanCol = -1;
                eState = WAITING_FOR_BOT;
            });

            xFuncMap->vSetHandler(ERROR, [&] ()
            {
                std::cout << pPixyBrain->xGetErrors() << std::endl;
                eState = CALIB;
            });
        }

        void vInitMapStr()
        {
            xStringMap[RESET] = std::string("RESET");
            xStringMap[CALIB] = std::string("CALIB");
            xStringMap[WAITING_FOR_BOT] = std::string("WAITING_FOR_BOT");
            xStringMap[WAITING_FOR_HUMAN] = std::string("WAITING_FOR_HUMAN");
            xStringMap[NOTIFY_BOT_OF_HUMAN_ACTIVIY] = std::string("NOTIFY_BOT_OF_HUMAN_ACTIVIY");
            xStringMap[ERROR] = std::string("ERROR");
            xStringMap[EMA_ALPHA_UP] = std::string("EMA_ALPHA_UP");
            xStringMap[EMA_ALPHA_DOWN] = std::string("EMA_ALPHA_UP");
            xStringMap[START] = std::string("START");
            xStringMap[WAITING_FOR_RESET] = std::string("WAITING_FOR_RESET");
        }

        std::unique_ptr<pixy::PixyBrain_t> pPixyBrain;
        std::unique_ptr<pixy::PixyEyes_t> pPixyEyes;
        std::unique_ptr<pixy::PixyMouth_t> pPixyMouth;

        std::map<State_t, std::string> xStringMap;
        std::unique_ptr<FuncMap_t<State_t, void>> xFuncMap;
};

} // namespace pixy
} // namespace team9

#endif
