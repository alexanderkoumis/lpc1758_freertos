#ifndef PIXY_HPP
#define PIXY_HPP

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <memory>
#include <functional>

#include "L5_Application/source/pixy_cmu.hpp"
//#include "L5_Application/source/pixy_cmu2.hpp"

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
            void vInit();
//            pPixyCMU.reset(new cmu::PixyCMU_t(128));
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
//                    std::vector<cmu::PixyBlock2> vPixyBlocks;
                    std::vector<cmu::PixyBlock> vPixyBlocks;
//                    pPixyCMU->vGetBlocks(vPixyBlocks);
                    cmu::usGetBlocks(1024, vPixyBlocks);
                    pConnect4Ptr->vSampleCalibrationChips(vPixyBlocks);
                    vPixyBlocks.clear();
                    eSystemState = CALIB;
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

//    std::unique_ptr<cmu::PixyCMU_t> pPixyCMU;



};

}

#endif