//
//  SimpleObserver.cpp
//  
//
//  Created by Yufeng Wu on 12/1/22.
//

#include "SimpleObserver.h"

//************************************************************

ECSimpleGraphicObserver::ECSimpleGraphicObserver(ECGraphicViewImp& viewIn) : view(viewIn), fIsSpaceBarPressed(false)
{
}

void ECSimpleGraphicObserver::Update()
{
    ECGVEventType evt = view.GetCurrEvent();
    // is this space bar key released?
    if (evt == ECGV_EV_KEY_UP_SPACE)
    {
        fIsSpaceBarPressed = true;

        // set the view to redraw
        // Note: call SetRedraw(true) once there is something changed in the view
        view.SetRedraw(true);
    }
    // if this is the timer event
    if (evt == ECGV_EV_TIMER)
    {
        // if space bar already pressed, draw a winidow
        // Note: you need to draw this in the timer event; otherwise you won't see the rectangle
        if (fIsSpaceBarPressed)
        {
            view.DrawRectangle(100, 100, 400, 400);
        }
    }
}

