#ifndef PIXY_HPP
#define PIXY_HPP

#include <map>
#include <string>
#include <sstream>
#include <memory>
#include <vector>

#include "L5_Application/source/pixy_cmu.hpp"

#include "connect4_board.hpp"

namespace team9
{

class Pixy
{
    public:
        enum SystemState_t {CAM_INIT, CALIB, RUN, ERROR} eSystemState;
        enum BlockType_t {NORMAL, COLOR};

        Pixy() : eSystemState(CAM_INIT)
        {
            cmu::vInit();
        }

        void vPopulateMap()
        {
            xStateMap[CAM_INIT] = std::string("CAM_INIT");
            xStateMap[CALIB] = std::string("CALIB");
            xStateMap[RUN] = std::string("RUN");
            xStateMap[ERROR] = std::string("ERROR");
        }

        void vStateMachine()
        {
            switch(eSystemState)
            {
                case CAM_INIT:
                {
                    team9::Connect4Board_t::CalibParams_t xCalibParams;
                    xCalibParams.ulCamCols = 640;
                    xCalibParams.ulCamRows = 480;
                    xCalibParams.ulNumFrames = 30000;
                    xCalibParams.eChipColor = GREEN;
                    pConnect4Ptr.reset(new team9::Connect4Board_t(xCalibParams));
                    eSystemState = CALIB;
                    break;
                }
                case CALIB:
                {
                    std::vector<cmu::PixyBlock> vPixyBlocks;
                    cmu::vReallyGetBlocks(1000, vPixyBlocks);
                    size_t init_size = vPixyBlocks.size();
                    int counter = 0;
                    std::ostringstream oss;
                    for (auto& block : vPixyBlocks)
                    {
                        if (block.signature > 3)
                        {
                            break;
                        }
                        else if (counter++ == 0)
                        {
                            oss << block.signature;
                        }
                        else
                        {
                            oss << " " << block.signature;
                        }
                    }
                    u0_dbg_printf("Expected: init_size: %d\tReceived: %d [%s]\n", init_size, counter, oss.str().c_str());
                    oss.str("");
                    oss.clear();
                    vPixyBlocks.clear();
                    eSystemState = CALIB;
//                    if (pConnect4Ptr->vSampleCalibrationChips(vBlocks))
//                    {
//                        eSystemState = CALIB;
//                    }
//                    else
//                    {
//                        pConnect4Ptr->vCalibrate();
//                        eSystemState = RUN;
//                    }
                    break;
                }
                case RUN:
                {

                    break;
                }
                case ERROR:
                {
                    break;
                }
            }
        }

    private:
        std::unique_ptr<team9::Connect4Board_t> pConnect4Ptr;
        std::map<SystemState_t, std::string> xStateMap;
//        std::vector<cmu::PixyBlock> vBlocks;
};

}

#endif
