#ifndef PIXY_HPP
#define PIXY_HPP

#include <iostream>
#include <map>
#include <functional>

#include "pixy/config.hpp"
#include "pixy/common.hpp"
#include "pixy/common/func_map.hpp"
#include "pixy/pixy_brain.hpp"
#include "pixy/pixy_eyes.hpp"
#include "pixy/pixy_mouth.hpp"

namespace team9
{
namespace pixy
{

class Pixy_t
{
    public:
        enum State_t {CALIB, RUN, UPDATE, ERROR} eState;

        Pixy_t () {}

        Pixy_t (uint32_t ulChipsAtATime, uint32_t ulChipsToCalib,
                ChipColor_t eColorCalib) :
                eState(CALIB),
                pPixyBrain(new pixy::PixyBrain_t(eColorCalib, ulChipsToCalib)),
                pPixyEyes(new pixy::PixyEyes_t(ulChipsAtATime)),
                pPixyMouth(new pixy::PixyMouth_t),
                xFuncMap(new FuncMap_t<State_t, void>)
        {
            vInitMapFunc();
            vInitMapStr();
        }

        void vAction()
        {
            xFuncMap->vResponse(eState)();
        }

    private:


        void vInitMapFunc()
        {
            xFuncMap->vSetHandler(CALIB, [&] ()
            {
                if (pPixyBrain->vCalibBoard(pPixyEyes.get()))
                {
                    std::cout << "Corners: " << std::endl
                            << pPixyBrain->pBoard->xCorners << std::endl
                            << "Points: " << std::endl;
                    pPixyBrain->pBoard->vPrintChips();
                    eState = RUN;
                }
                else
                {
                    eState = ERROR;
                }
            });

            xFuncMap->vSetHandler(RUN, [&] ()
            {
                int lChangedChip = pPixyBrain->lSampleChips(pPixyEyes.get());
                switch(lChangedChip)
                {
                    case -2:
                    {
                        // No chips changed
                        break;
                    }
                    case -1:
                    {
                        std::vector<int> xChangedChips;
                        pPixyBrain->pBoard->vGetChanged(xChangedChips);
                        std::cout << "Multiple changed chips:" << std::endl;
                        for (auto& lChip : xChangedChips)
                        {
                            int lRow = lChip / 7;
                            int lCol = lChip % 7;
                            std::cout << "[" << lRow << "]"
                                      << "[" << lCol << "]" << std::endl;
                        }
                        pPixyBrain->pBoard->vPrintChips(Board_t::COLOR);
                        break;
                    }
                    default:
                    {
                        int lRow = lChangedChip / 7;
                        int lCol = lChangedChip % 7;
                        std::cout << "Single changed chip: "
                                  << "[" << lRow << "]"
                                  << "[" << lCol << "]" << std::endl;
                        pPixyBrain->pBoard->vPrintChips(Board_t::COLOR);
                        break;
                    }
                }
                eState = RUN;
            });

            xFuncMap->vSetHandler(UPDATE, [&] ()
            {
                if (!pPixyMouth->xEmitUpdate(pPixyBrain->lGetUpdate()))
                {
                    eState = ERROR;
                };
                eState = RUN;
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
            xStringMap[RUN] = std::string("RUN");
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
