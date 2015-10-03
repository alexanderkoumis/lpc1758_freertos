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
        enum Button_t
        {
            RESET_BUTTON=0x01,  // SW(1)
            CALIB_BUTTON=0x02,  // SW(2)
            EMA_ALPHA_UP=0x04,  // SW(3)
            EMA_ALPHA_DOWN=0x08 // SW(4)
        };

        enum State_t
        {
            CALIB_STATE,
            RESET_STATE,
            WAITING_FOR_HUMAN,
            WAITING_FOR_BOT,
            WAITING_FOR_RESET,
            ERROR
        };

        Pixy_t(uint32_t ulChipsAtATime, uint32_t ulChipsToCalib, ChipColor_t eColorCalib);
        void vAction(Button_t eButton);

        bool bCalibPressed;
        bool bResetPressed;
        State_t eState;
        State_t eLastState;

    private:
        bool bReceivedResetFromBotAsync();
        void vUpdateState(Pixy_t::State_t eState_new);
        void vInitMapFunc();
        void vInitMapStr();

        std::unique_ptr<pixy::PixyBrain_t> pPixyBrain;
        std::unique_ptr<pixy::PixyEyes_t> pPixyEyes;
        std::unique_ptr<pixy::PixyMouth_t> pPixyMouth;

        std::map<Button_t, std::string> xButtonStrMap;
        std::map<State_t, std::string> xStateStrMap;
        std::unique_ptr<FuncMap_t<State_t, void>> xFuncMap;

        std::unique_ptr<Corners_t> xCornersPtr;
};

} // namespace pixy
} // namespace team9

#endif
