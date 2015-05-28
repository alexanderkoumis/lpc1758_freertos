#ifndef PIXY_HPP
#define PIXY_HPP

#include <iostream>
#include <map>
#include <functional>

#include "shared_handles.h"

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
            EMA_ALPHA_UP=0x02,      // SW(2)
            EMA_ALPHA_DOWN=0x04,    // SW(3)
            START=0x08,             // SW(4)
            CALIB=16,
            WAITING_FOR_HUMAN=17,
            WAITING_FOR_BOT=18,
            NOTIFY_BOT_OF_HUMAN_ACTIVIY=19,
            WAITING_FOR_START=20,
            ERROR=21
        } eState, eLastState;

        bool bStarted;
        int lLastHumanCol;

        Pixy_t () : bStarted(false), lLastHumanCol(-1), eState(CALIB), eLastState(CALIB)
        {}

        Pixy_t (uint32_t ulChipsAtATime, uint32_t ulChipsToCalib,
                ChipColor_t eColorCalib) :
                bStarted(false),
                lLastHumanCol(-1),
                eState(CALIB),
                eLastState(CALIB),
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
            eLastState = eState;
            u0_dbg_printf("State: %s\n", xStringMap[eState].c_str());
            if (eLastState != eState)
            {
                u0_dbg_printf("CHANGE STATE\n");
            }
            if (xButton)
            {
                eState = (State_t)xButton;
            }
            xFuncMap->vResponse(eState)();
        }

    private:
        void vInitMapFunc()
        {
            xFuncMap->vSetHandler(RESET, [&] ()
            {
                pPixyEyes->vReset();
                pPixyBrain->vReset();
                eState = WAITING_FOR_START;
            });

            xFuncMap->vSetHandler(EMA_ALPHA_UP, [&] ()
            {
                pPixyBrain->vEMAAlphaUp();
                std::cout << "Alpha: " << pPixyBrain->xGetAlpha() << std::endl;
                eState = eLastState;
            });

            xFuncMap->vSetHandler(EMA_ALPHA_DOWN, [&] ()
            {
                pPixyBrain->vEMAAlphaDown();
                std::cout << "Alpha: " << pPixyBrain->xGetAlpha() << std::endl;
                eState = eLastState;
            });

            xFuncMap->vSetHandler(WAITING_FOR_START, [&] ()
            {
                if (bStarted)
                {
                    eState = WAITING_FOR_HUMAN;
                    return;
                }
                else
                {
                    eState = WAITING_FOR_START;
                }
            });

            xFuncMap->vSetHandler(CALIB, [&] ()
            {
                if (pPixyBrain->vCalibBoard(pPixyEyes.get()))
                {
                    pPixyBrain->vPrintCorners(Board_t::LOCATION);
                    eState = WAITING_FOR_START;
                }
                else
                {
                    eState = ERROR;
                }
            });

            xFuncMap->vSetHandler(START, [&] ()
            {
                bStarted = true;
                eState = WAITING_FOR_START;
            });

            xFuncMap->vSetHandler(WAITING_FOR_HUMAN, [&] ()
            {
                lLastHumanCol = pPixyBrain->lSampleChips(pPixyEyes.get());
                printf("Last Human Col: %d\n", lLastHumanCol);
                u0_dbg_printf("Last Human Col: %d\n", lLastHumanCol);
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
                    std::cout << "command column: " << xBotInsertCmd.lColumn << "\n"
                              << "command color: " << xBotInsertCmd.lColor << std::endl;
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
            xStringMap[WAITING_FOR_START] = std::string("WAITING_FOR_START");
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
