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
#include "pixy/pixy_display.hpp"

namespace team9
{
namespace pixy
{

class Pixy_t
{
	public:
        enum State_t {CALIB, RUN, ERROR} eState;

        Pixy_t () {}

		Pixy_t (uint32_t ulChipsAtATime, uint32_t ulChipsToCalib,
		        ChipColor_t eColorCalib) :
				eState(CALIB),
				pPixyBrain(new pixy::PixyBrain_t(Dims_t(320, 200), eColorCalib,
				                                 ulChipsToCalib)),
				pPixyEyes(new pixy::PixyEyes_t(ulChipsAtATime)),
				pPixyMouth(new pixy::PixyMouth_t),
				pPixyDisplay(new pixy::PixyDisplay_t("#", 320, 200)),
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
//					std::cout << "Calibrated board" << std::endl
//					          << pPixyBrain->xGetCorners()
//							  << std::endl;
				    std::vector<Point_t> xPoints;
					pPixyBrain->vInitBoard(pPixyEyes.get(), xPoints);

//					pPixyDisplay->vUpdate(xPoints, true);
//					pPixyDisplay->vPrint();
					eState = RUN;
				}
				else
				{
					eState = ERROR;
				}
			});

			xFuncMap->vSetHandler(RUN, [&] ()
			{



			    eState = CALIB;
			});

			xFuncMap->vSetHandler(ERROR, [&] ()
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
        std::unique_ptr<pixy::PixyDisplay_t> pPixyDisplay;

        std::map<State_t, std::string> xStringMap;
        std::unique_ptr<FuncMap_t<State_t, void>> xFuncMap;

};

} // namespace pixy
} // namespace team9

#endif
