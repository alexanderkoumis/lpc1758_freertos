#include "pixy.hpp"

namespace team9
{
namespace pixy
{

Pixy_t::Pixy_t (uint32_t ulChipsAtATime, uint32_t ulChipsToCalib,
                ChipColor_t eColorCalib) :
//        bStartPressed(false),
        bCalibPressed(false),
        bResetPressed(false),
        eState(CALIB_STATE),
        eLastState(CALIB_STATE),
        pPixyBrain(new pixy::PixyBrain_t(eColorCalib, ulChipsToCalib)),
        pPixyEyes(new pixy::PixyEyes_t(ulChipsAtATime)),
        pPixyMouth(new pixy::PixyMouth_t),
        xFuncMap(new FuncMap_t<State_t, void>)
{
    vInitMapFunc();
    vInitMapStr();
}

void Pixy_t::vAction(Button_t eButton)
{

    switch(eButton)
    {
//        case START_BUTTON: bStartPressed = true; printf("Pressed: %s\n", xButtonStrMap[xButton].c_str()); break;
        case CALIB_BUTTON: bCalibPressed = true; printf("Pressed: %s\n", xButtonStrMap[eButton].c_str());
                                                 u0_dbg_printf("Pressed: %s\n", xButtonStrMap[eButton].c_str()); break;
        case RESET_BUTTON: bResetPressed = true; printf("Pressed: %s\n", xButtonStrMap[eButton].c_str());
                                                 u0_dbg_printf("Pressed: %s\n", xButtonStrMap[eButton].c_str());break;
        case EMA_ALPHA_UP: pPixyBrain->vEMAAlphaUp(); printf("Alpha: %f\n", pPixyBrain->xGetAlpha());
                                                      u0_dbg_printf("Alpha: %f\n", pPixyBrain->xGetAlpha()); break;
        case EMA_ALPHA_DOWN: pPixyBrain->vEMAAlphaDown(); printf("Alpha: %f\n", pPixyBrain->xGetAlpha());
                                                          u0_dbg_printf("Alpha: %f\n", pPixyBrain->xGetAlpha()); break;
    }

    static int printed = 0;
    if (eLastState == eState)
    {
        if (printed < 10)
        {
            printed++;
            printf("State: %s\n", xStateStrMap[eState].c_str());
            u0_dbg_printf("State: %s\n", xStateStrMap[eState].c_str());
        }
    }
    else
    {
        printed = 0;
        printf("State change [%s -> %s]\n", xStateStrMap[eLastState].c_str(), xStateStrMap[eState].c_str());
        u0_dbg_printf("State change [%s -> %s]\n", xStateStrMap[eLastState].c_str(), xStateStrMap[eState].c_str());
    }
    eLastState = eState;
    xFuncMap->vResponse(eState)();
}

void Pixy_t::vInitMapFunc()
{

    xFuncMap->vSetHandler(CALIB_STATE, [&] ()
    {
        if (bCalibPressed)
        {
            printf("Calib pressed\n");
            u0_dbg_printf("Calib pressed\n");
            bCalibPressed = false;
            pPixyBrain->vCalibCorners(pPixyEyes.get(), xCorners);
            Storage::write("/corners.calib", Corners_t::pcCornerStrRaw(xCorners), 256, 0);
            eState = WAITING_FOR_RESET;
        }
        else
        {
            if (Corners_t::bReadCorners("/corners.calib", xCorners))
            {
                eState = RESET_STATE;
            }
            else
            {
                printf("Problem loading corner calib, getting from camera\n");
                u0_dbg_printf("Problem loading corner calib, getting from camera\n");
                bCalibPressed = true;
                eState = CALIB_STATE;
            }
        }
    });

    xFuncMap->vSetHandler(RESET_STATE, [&] ()
    {
//        pPixyBrain->pBoard->vBuildGrid(xCorners);
//        pPixyBrain->pBoard->vReset();
        bResetPressed = false;
        pPixyBrain->pBoard.reset(new Board_t(xCorners, pPixyBrain->eColorCalib));
        eState = WAITING_FOR_HUMAN;
    });

    xFuncMap->vSetHandler(WAITING_FOR_RESET, [&] ()
    {
        eState = bCalibPressed ? CALIB_STATE : bResetPressed ? RESET_STATE : WAITING_FOR_RESET;
    });

    xFuncMap->vSetHandler(WAITING_FOR_HUMAN, [&] ()
    {
        if (bCalibPressed) { eState = CALIB_STATE; return; }
        if (bResetPressed) { eState = RESET_STATE; return; }

        int lLastHumanCol = pPixyBrain->lSampleChips(pPixyEyes.get());
        printf("Last Human Col: %d\n", lLastHumanCol);
        u0_dbg_printf("Last Human Col: %d\n", lLastHumanCol);
        if (lLastHumanCol >= 0)
        {
            pPixyBrain->vPrintChips(Board_t::COLOR, true);
            pPixyMouth->xEmitUpdate(lLastHumanCol);
            eState = WAITING_FOR_BOT;
        }
        else
        {
            eState = WAITING_FOR_HUMAN;
        }
    });

    xFuncMap->vSetHandler(WAITING_FOR_BOT, [&] ()
    {
        if (bCalibPressed) { eState = CALIB_STATE; return; }
        if (bResetPressed) { eState = RESET_STATE; return; }

        PixyCmd_t xBotInsertCmd;

        if (xQueueReceive(
                scheduler_task::getSharedObject(shared_PixyQueueRX),
                &xBotInsertCmd, portMAX_DELAY))
        {
            std::cout << "Bot column: " << xBotInsertCmd.lColumn << "\n"
                      << "Bot color: " << xBotInsertCmd.lColor << std::endl;
            if (xBotInsertCmd.bReset)
            {
                printf("Received reset command\n");
                u0_dbg_printf("Received reset command\n");
                eState = CALIB_STATE;
                return;
            }
            ChipColor_t xChipColor = (ChipColor_t)xBotInsertCmd.lColor;
            int lColumn = xBotInsertCmd.lColumn;
            int lNewRow = pPixyBrain->lBotInsert(xBotInsertCmd);
            if (lNewRow > 0)
            {
                printf("After insertion of color %d into column %d, column height is now %d\n",
                        xChipColor, lColumn, lNewRow);
                u0_dbg_printf("After insertion of color %d into column %d, column height is now %d\n",
                              xChipColor, lColumn, lNewRow);
                eState = WAITING_FOR_HUMAN;
            }
            else
            {
                eState = ERROR;
            }
        }
        else
        {
            eState = WAITING_FOR_BOT;
        }
    });

    xFuncMap->vSetHandler(ERROR, [&] ()
    {
        printf("%s\n", pPixyBrain->xGetErrors().c_str());
        u0_dbg_printf("%s\n", pPixyBrain->xGetErrors().c_str());
        eState = CALIB_STATE;
    });
}

void Pixy_t::vInitMapStr()
{
//    xButtonStrMap[START_BUTTON] = std::string("START");
    xButtonStrMap[CALIB_BUTTON] = std::string("CALIB");
    xButtonStrMap[RESET_BUTTON] = std::string("RESET");
    xButtonStrMap[EMA_ALPHA_UP] = std::string("EMA_ALPHA_UP");
    xButtonStrMap[EMA_ALPHA_DOWN] = std::string("EMA_ALPHA_UP");
//    xStateStrMap[START_STATE] = std::string("START_STATE");
    xStateStrMap[CALIB_STATE] = std::string("CALIB_STATE");
    xStateStrMap[RESET_STATE] = std::string("RESET_STATE");
    xStateStrMap[WAITING_FOR_BOT] = std::string("WAITING_FOR_BOT");
    xStateStrMap[WAITING_FOR_HUMAN] = std::string("WAITING_FOR_HUMAN");
    xStateStrMap[WAITING_FOR_RESET] = std::string("WAITING_FOR_RESET");
    xStateStrMap[ERROR] = std::string("ERROR");
//    xStateStrMap[NOTIFY_BOT_OF_HUMAN_ACTIVIY] = std::string("NOTIFY_BOT_OF_HUMAN_ACTIVIY");

}

} // namespace pixy

} // namespace team9
