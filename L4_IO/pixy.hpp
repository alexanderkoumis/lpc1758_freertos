#ifndef PIXY_HPP
#define PIXY_HPP

#include <iostream>
#include <map>
#include <functional>

#include "pixy/pixy_config.hpp"
#include "pixy/pixy_common.hpp"
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
		Pixy_t () {}

		Pixy_t (uint32_t ulMaxBlocks, ChipColor_t eColorCalib,
				uint32_t ulCalibFrames) :
				eState(CALIB),
				pPixyBrain(new pixy::PixyBrain_t(Dims_t(320, 200), eColorCalib,
												 ulCalibFrames)),
				pPixyEyes(new pixy::PixyEyes_t(ulMaxBlocks)),
				pPixyMouth(new pixy::PixyMouth_t)
		{
			vInitMapFunc();
			vInitMapStr();
		}

		void vAction()
		{
			auto& fpResponse = this->vResponse(eState);
			fpResponse();
		}

		enum State_t {CALIB, RUN, ERROR} eState;

	private:

		void vSetHandler(State_t eState_arg, std::function<void()> fnHandler)
		{
		    fpMap[eState_arg] = fnHandler;
		}

		std::function<void()>& vResponse(State_t eState_arg)
		{
			std::cout << "State: " << xStringMap[eState_arg] << std::endl;
		    return fpMap[eState_arg];
		}

		void vInitMapFunc()
		{
			vSetHandler(CALIB, [&] ()
			{
				if (pPixyBrain->vCalibBoard(pPixyEyes.get()))
				{
					std::cout << "Calibrated board" << pPixyBrain->xGetCorners()
							  << std::endl;
					eState = RUN;
				}
				else
				{
					eState = ERROR;
				}
			});

			vSetHandler(RUN, [&] ()
			{
			    eState = CALIB;
			});

			vSetHandler(ERROR, [&] ()
			{
				std::cout << pPixyBrain->sGetLastError() << std::endl;
				eState = ERROR;
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
        std::map<State_t, std::function<void()>> fpMap;
        std::map<State_t, std::string> xStringMap;
};

} // namespace pixy
} // namespace team9

#endif
