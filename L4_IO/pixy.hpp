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
            RESET=0x01,          // SW(1)
            EMA_ALPHA_UP=0x02,   // SW(2)
            EMA_ALPHA_DOWN=0x04, // SW(3)
            SW_4=0x08,           // SW(4)
            CALIB=16,
            WAITING_FOR_HUMAN=17,
            WAITING_FOR_BOT=18,
            ERROR=19
        } eState, eLastState;

        Pixy_t ()
        {}

        Pixy_t (uint32_t ulChipsAtATime, uint32_t ulChipsToCalib,
                ChipColor_t eColorCalib) :
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
                pPixyBrain->vReset();
                eState = CALIB;
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

            xFuncMap->vSetHandler(SW_4, [&] ()
            {
                std::cout << "Switch 4 is unmapped" << std::endl;
                eState = eLastState;
            });

            xFuncMap->vSetHandler(CALIB, [&] ()
            {
                if (pPixyBrain->vCalibBoard(pPixyEyes.get()))
                {
                    pPixyBrain->vPrintCorners(Board_t::LOCATION);
                    eState = WAITING_FOR_HUMAN;
                }
                else
                {
                    eState = ERROR;
                }
            });

            xFuncMap->vSetHandler(WAITING_FOR_HUMAN, [&] ()
            {
                int lHumanCol = pPixyBrain->lSampleChips(pPixyEyes.get());
                if (lHumanCol > 0)
                {
                    pPixyBrain->vPrintChips(Board_t::COLOR, true);
                    eState = WAITING_FOR_BOT;
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
                        scheduler_task::getSharedObject(shared_PixyQueue),
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

            xFuncMap->vSetHandler(ERROR, [&] ()
            {
                std::cout << pPixyBrain->xGetErrors() << std::endl;
                eState = CALIB;
            });
        }

        void vInitMapStr()
        {
            xStringMap[CALIB] = std::string("CALIB");
            xStringMap[WAITING_FOR_BOT] = std::string("WAITING_FOR_BOT");
            xStringMap[WAITING_FOR_HUMAN] = std::string("WAITING_FOR_HUMAN");
            xStringMap[ERROR] = std::string("ERROR");
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
