#include "pixy.hpp"

namespace team9
{
namespace pixy
{

Pixy_t::Pixy_t (uint32_t ulChipsAtATime, uint32_t ulChipsToCalib,
                ChipColor_t eColorCalib) :
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

bool Pixy_t::bReceivedResetFromBotAsync()
{
    bool bReset = false;
    static int lPrintedResetRepeat = 0;

    if (xQueueReceive(scheduler_task::getSharedObject(shared_PixyResetQueueRX), &bReset, 0))
    {
        lPrintedResetRepeat = 0;
        if (bReset)
        {
            printf("(Async) Received reset command\n");
            u0_dbg_printf("(Async) Received reset command\n");
            return true;
        }
        else
        {
            printf("(Async) Odd, received false reset, how is this possible?\n");
            u0_dbg_printf("(Async) Odd, received false reset, how is this possible?\n");
        }
    }
    if (lPrintedResetRepeat++ < 10)
    {
        printf("(Async) No reset\n");
        u0_dbg_printf("(Async) No reset\n");
    }

    return false;
}

void Pixy_t::vAction(Button_t eButton)
{

    switch(eButton)
    {
        case CALIB_BUTTON: bCalibPressed = true; printf("Pressed: %s\n", xButtonStrMap[eButton].c_str());
                                                 u0_dbg_printf("Pressed: %s\n", xButtonStrMap[eButton].c_str()); break;
        case RESET_BUTTON: bResetPressed = true; printf("Pressed: %s\n", xButtonStrMap[eButton].c_str());
                                                 u0_dbg_printf("Pressed: %s\n", xButtonStrMap[eButton].c_str());break;
        case EMA_ALPHA_UP: pPixyBrain->vEMAAlphaUp(); printf("Alpha: %f\n", pPixyBrain->xGetAlpha());
                                                      u0_dbg_printf("Alpha: %f\n", pPixyBrain->xGetAlpha()); break;
        case EMA_ALPHA_DOWN: pPixyBrain->vEMAAlphaDown(); printf("Alpha: %f\n", pPixyBrain->xGetAlpha());
                                                          u0_dbg_printf("Alpha: %f\n", pPixyBrain->xGetAlpha()); break;
    }

    if (bReceivedResetFromBotAsync())
    {
//        eState = CALIB_STATE;
        vUpdateState(CALIB_STATE);
    }

    // WAITING_FOR_RESET SHOULD ONLY BE PRECEEDED BY CALIB_STATE OR WAITING_FOR_RESET
    // RESET_STATE SHOULD ONLY BE PRECEEDED BY CALIB_STATE

    if (eState == WAITING_FOR_RESET && (eLastState != CALIB_STATE && eLastState != WAITING_FOR_RESET))
    {
        printf("Error, eState == WAITING_FOR_RESET && eLastState != CALIB_STATE\n");
        u0_dbg_printf("Error, eState == WAITING_FOR_RESET && eLastState != CALIB_STATE\n");
//        eState = CALIB_STATE;
        vUpdateState(CALIB_STATE);
    }

    if (eState == RESET_STATE && eLastState != CALIB_STATE)
    {
        printf("Error, eState == RESET_STATE && eLastState != CALIB_STATE\n");
        u0_dbg_printf("Error, eState == RESET_STATE && eLastState != CALIB_STATE\n");
//        eState = CALIB_STATE;
        vUpdateState(CALIB_STATE);
    }

    static int lPrintedStateRepeat = 0;
    if (eLastState == eState)
    {
        if (lPrintedStateRepeat < 10)
        {
            lPrintedStateRepeat++;
            printf("State: %s\n", xStateStrMap[eState].c_str());
            u0_dbg_printf("State: %s\n", xStateStrMap[eState].c_str());
        }
    }
    else
    {
        lPrintedStateRepeat = 0;
        printf("State change [%s -> %s]\n", xStateStrMap[eLastState].c_str(), xStateStrMap[eState].c_str());
        u0_dbg_printf("State change [%s -> %s]\n", xStateStrMap[eLastState].c_str(), xStateStrMap[eState].c_str());
    }

//    eLastState = eState;
    xFuncMap->vResponse(eState)();
}

void Pixy_t::vUpdateState(Pixy_t::State_t eState_new)
{
    eLastState = eState;
    eState = eState_new;
}

void Pixy_t::vInitMapFunc()
{
    xFuncMap->vSetHandler(CALIB_STATE, [&] ()
    {
        xCornersPtr.reset(new Corners_t);
        if (bCalibPressed)
        {
            printf("Calib pressed\n");
            u0_dbg_printf("Calib pressed\n");
            bCalibPressed = false;
            pPixyBrain->vCalibCorners(pPixyEyes.get(), *xCornersPtr);
            Storage::write("/corners.calib", Corners_t::pcCornerStrRaw(*xCornersPtr), 256, 0);
//            eState = WAITING_FOR_RESET;
            vUpdateState(WAITING_FOR_RESET);
        }
        else
        {
            printf("Loading corner calibration from file\n");
            u0_dbg_printf("Loading corner calibration from file\n");
            if (Corners_t::bReadCorners("/corners.calib", *xCornersPtr))
            {
                printf("Success loading file\n");
                u0_dbg_printf("Success loading file\n");
//                eState = RESET_STATE;
                vUpdateState(RESET_STATE);
            }
            else
            {
                printf("Problem loading corner calib, getting from camera\n");
                u0_dbg_printf("Problem loading corner calib, getting from camera\n");
                bCalibPressed = true;
                vUpdateState(CALIB_STATE);
//                eState = CALIB_STATE;
            }
        }
    });

    xFuncMap->vSetHandler(RESET_STATE, [&] ()
    {
        bResetPressed = false;
        if (xCornersPtr)
        {
            printf("[RESET_STATE]: Resetting game board\n");
            u0_dbg_printf("[RESET_STATE]: Resetting game board\n");
            pPixyBrain->pBoard.reset(new Board_t(*xCornersPtr, pPixyBrain->eColorCalib));
            vUpdateState(WAITING_FOR_HUMAN);
//            eState = WAITING_FOR_HUMAN;
        }
        else
        {
            printf("[RESET_STATE]: Need to initialize xCornersPtr first\n");
            u0_dbg_printf("[RESET_STATE]: Need to initialize xCornersPtr first\n");
            vUpdateState(CALIB_STATE);
//            eState = CALIB_STATE;
        }

    });

    xFuncMap->vSetHandler(WAITING_FOR_RESET, [&] ()
    {
        vUpdateState(bResetPressed ? RESET_STATE : WAITING_FOR_RESET);
//        eState = bResetPressed ? RESET_STATE : WAITING_FOR_RESET;
    });

    xFuncMap->vSetHandler(WAITING_FOR_HUMAN, [&] ()
    {
//        if (bCalibPressed) { eState = CALIB_STATE; return; }
//        if (bResetPressed) { eState = CALIB_STATE; return; }

        if (bCalibPressed) { vUpdateState(CALIB_STATE); return; }
        if (bResetPressed) { vUpdateState(CALIB_STATE); return; }

        int lLastHumanCol = pPixyBrain->lSampleChips(pPixyEyes.get());
        printf("Last Human Col: %d\n", lLastHumanCol);
        u0_dbg_printf("Last Human Col: %d\n", lLastHumanCol);
        if (lLastHumanCol >= 0)
        {
            pPixyBrain->vPrintChips(Board_t::COLOR, true);
            pPixyMouth->xEmitUpdate(lLastHumanCol);
            vUpdateState(WAITING_FOR_BOT);
//            eState = WAITING_FOR_BOT;
        }
        else
        {
            vUpdateState(WAITING_FOR_HUMAN);
//            eState = WAITING_FOR_HUMAN;
        }
    });

    xFuncMap->vSetHandler(WAITING_FOR_BOT, [&] ()
    {
        //        if (bCalibPressed) { eState = CALIB_STATE; return; }
        //        if (bResetPressed) { eState = CALIB_STATE; return; }

        if (bCalibPressed) { vUpdateState(CALIB_STATE); return; }
        if (bResetPressed) { vUpdateState(CALIB_STATE); return; }

        PixyCmd_t xBotInsertCmd;

        if (xQueueReceive(
                scheduler_task::getSharedObject(shared_PixyQueueRX),
                &xBotInsertCmd, 1000))
        {
            printf("Bot column/color: %d/%d\n", xBotInsertCmd.lColumn, xBotInsertCmd.lColor);
            u0_dbg_printf("Bot column/color: %d/%d\n", xBotInsertCmd.lColumn, xBotInsertCmd.lColor);
            ChipColor_t xChipColor = (ChipColor_t)xBotInsertCmd.lColor;
            int lColumn = xBotInsertCmd.lColumn;
            int lNewRow = pPixyBrain->lBotInsert(xBotInsertCmd);
            if (lNewRow > 0)
            {
                printf("After insertion of color %d into column %d, column height is now %d\n",
                        xChipColor, lColumn, lNewRow);
                u0_dbg_printf("After insertion of color %d into column %d, column height is now %d\n",
                              xChipColor, lColumn, lNewRow);
//                eState = WAITING_FOR_HUMAN;
                vUpdateState(WAITING_FOR_HUMAN);
            }
        }
        else
        {
            vUpdateState(WAITING_FOR_BOT);
//            eState = WAITING_FOR_BOT;
        }
    });

    xFuncMap->vSetHandler(ERROR, [&] ()
    {
        printf("%s\n", pPixyBrain->xGetErrors().c_str());
        u0_dbg_printf("%s\n", pPixyBrain->xGetErrors().c_str());
        vUpdateState(CALIB_STATE);
//        eState = CALIB_STATE;
    });
}

void Pixy_t::vInitMapStr()
{
    xButtonStrMap[CALIB_BUTTON] = std::string("CALIB");
    xButtonStrMap[RESET_BUTTON] = std::string("RESET");
    xButtonStrMap[EMA_ALPHA_UP] = std::string("EMA_ALPHA_UP");
    xButtonStrMap[EMA_ALPHA_DOWN] = std::string("EMA_ALPHA_UP");
    xStateStrMap[CALIB_STATE] = std::string("CALIB_STATE");
    xStateStrMap[RESET_STATE] = std::string("RESET_STATE");
    xStateStrMap[WAITING_FOR_BOT] = std::string("WAITING_FOR_BOT");
    xStateStrMap[WAITING_FOR_HUMAN] = std::string("WAITING_FOR_HUMAN");
    xStateStrMap[WAITING_FOR_RESET] = std::string("WAITING_FOR_RESET");
    xStateStrMap[ERROR] = std::string("ERROR");
}

} // namespace pixy
} // namespace team9
