#pragma once
#ifndef ElevatorObserver_h
#define ElevatorObserver_h

#include "ECObserver.h"
#include "ECGraphicViewImp.h"
#include "ECElevatorSim.h"
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

class ECElevatorSim;

//frontend
class ECElevatorObserver : public ECObserver
{
public:
    //take in view, numFloors, allStates = actions, lenSim = how long to run
    ECElevatorObserver(ECGraphicViewImp& viewIn, int numFloors, const std::vector<ECElevatorState>& allStates, int lenSim);

    virtual void Update(); // What to do on update

    //redraw the frontend
    void SetRedraw(bool f)
    {
        view.SetRedraw(f);
    }

private:
    //view reference
    ECGraphicViewImp& view;

    //drawing variables
    int numFloors; //num of elevator floors
    int floorHeight; //height of each floor
    int topFloorY; //y pos of top floor
    int bottomFloorY; //y pos of bottom floor
    int cabinX;
    int cabinY;
    int passengerRadius;
    int margin;

    //stores all action states
    const std::vector<ECElevatorState>& states;

    //timing properties
    int lenSim;
    int currentTime;
    bool paused;
    int framesPerStep;
    int currentFrame;
    int currentSimTime;

    //helper methods
    void DrawElevator();
    void DrawTimeAndProgressBar();
    void DrawWaitingPassengers(const ECElevatorState& st);
    //void DrawOnboardPassengers(const ECElevatorState& st);

    //bool isPointInTriangle(int px, int py, const Triangle& tri);

    //music variables
    ALLEGRO_SAMPLE* music;
    ALLEGRO_SAMPLE_ID musicID;
};
#endif /* ElevatorObserver_h */