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
    ~ECElevatorObserver();

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

    //music variables
    ALLEGRO_SAMPLE* backgroundMusic;
    ALLEGRO_SAMPLE* dingSound;
    ALLEGRO_SAMPLE_INSTANCE* backgroundMusicInstance;
    ALLEGRO_SAMPLE_INSTANCE* dingSoundInstance;
    unsigned int currentBackMusicPos = 0;

    //image variables
    ALLEGRO_BITMAP* elevatorImageBack;
    ALLEGRO_BITMAP* elevatorImageCabin;
    ALLEGRO_BITMAP* shaftImage;
    ALLEGRO_BITMAP* manImage;

    //fonts
    ALLEGRO_FONT* jerseyFont;
};
#endif /* ElevatorObserver_h */