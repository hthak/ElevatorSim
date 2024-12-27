//
//  SimpleObserver.h
//  
//
//  Created by Yufeng Wu on 12/1/22.
//  Demonstrate how a simple observer would work
//  This observer waits for the space bar key;
//  after the user presses (and releases) the space bar
//  it displays a rectangle in the middle of the window
//  That is it!
//  This code is meant to demonstrate the functionalities
//  especially the Observer pattern implemented in
//  ECGraphicViewImp.
//

#ifndef SimpleObserver_h
#define SimpleObserver_h

#include "ECObserver.h"
#include "ECGraphicViewImp.h"


//************************************************************
class ECSimpleGraphicObserver : public ECObserver
{
public:
    ECSimpleGraphicObserver(ECGraphicViewImp& viewIn);
    virtual void Update();

private:
    ECGraphicViewImp& view;
    bool fIsSpaceBarPressed;
};

#endif /* SimpleObserver_h */
#pragma once
