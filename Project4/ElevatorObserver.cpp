#include "ElevatorObserver.h"
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <iostream>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>


//waitingPassengers set to empty and number of floors
//cabinX = elevator cabin = middle of elevator
//cabinY = elevator cabin = current y = 500
//passengerRadius = how wide each passenger circle is
//margin = how far apart each passenger is
ECElevatorObserver::ECElevatorObserver(ECGraphicViewImp& viewIn, int numFloors,
    const std::vector<ECElevatorState>& allStates, int lenSim) :
    view(viewIn), numFloors(numFloors), states(allStates), lenSim(lenSim),
    currentTime(0), paused(false), floorHeight(100), topFloorY(100),
    passengerRadius(10), margin(20), framesPerStep(10), currentFrame(0), currentSimTime(0)
{
    bottomFloorY = topFloorY + (numFloors - 1) * floorHeight;
    cabinX = view.GetWidth() / 2;
    //elevatorPos = bottomFloorY;
    cabinY = bottomFloorY;

    //load elevator background image once in constructor
    elevatorImageBack = al_load_bitmap("elevator_back.png");
    if (!elevatorImageBack)
    {
        std::cout << "Failed to load elevator_back.png image!" << std::endl;
    }

    //load elevator cabin image once in constructor
    elevatorImageCabin = al_load_bitmap("elevator_cabin.png");
    if (!elevatorImageCabin)
    {
        std::cout << "Failed to load elevator_cabin.png image!" << std::endl;
    }

    //load elevator music
    backgroundMusic = al_load_sample("elevator_music.ogg");
    if (!backgroundMusic)
    {
        std::cout << "Failed to load elevator_music.ogg music file!" << std::endl;
    }

    //play elevator music
    al_play_sample(backgroundMusic, 0.3, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &backgroundMusicID);

    dingSound = al_load_sample("ding.ogg");
    if (!dingSound)
    {
        std::cout << "Failed to load ding.ogg music file!" << std::endl;
    }

    //draw initally by itself
    view.SetRedraw(true);
}

ECElevatorObserver::~ECElevatorObserver() //destructor free music variables in mem
{
    if (backgroundMusic)
    {
        al_stop_sample(&backgroundMusicID);
        al_destroy_sample(backgroundMusic);
        backgroundMusic = nullptr;
    }
    if (dingSound)
    {
        al_destroy_sample(dingSound);
        dingSound = nullptr;
    }
    if (elevatorImageBack)
    {
        al_destroy_bitmap(elevatorImageBack);
        elevatorImageBack = nullptr;
    }
    if (elevatorImageCabin)
    {
        al_destroy_bitmap(elevatorImageCabin);
        elevatorImageCabin = nullptr;
    }
}

//updates elevator based of key/button press
void ECElevatorObserver::Update()
{
    ECGVEventType evt = view.GetCurrEvent(); //similar to car, get the current event on the keyboard
    if (evt == ECGV_EV_KEY_UP_SPACE)
    {
        paused = !paused; //stop or start when space pressed
        view.SetRedraw(true);
    }
    if (evt == ECGV_EV_TIMER) {
        if (!paused) {
            //advance animation frame
            if (currentSimTime < lenSim - 1)
            {
                currentFrame++;
                if (currentFrame == 1)
                {
                    //elevator ding sound logic
                    const ECElevatorState& prevState = states[currentSimTime];
                    const ECElevatorState& currState = states[currentSimTime + 1];
                    bool wasMoving = (prevState.dir == EC_ELEVATOR_UP || prevState.dir == EC_ELEVATOR_DOWN);
                    bool isStoppedNow = currState.dir == EC_ELEVATOR_STOPPED;
                    if (wasMoving && isStoppedNow)
                    {
                        if (dingSound)
                        {
                            al_play_sample(dingSound, 0.8, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
                            std::cout << "ding ding" << std::endl;
                        }
                    }
                }
                

                if (currentFrame >= framesPerStep)
                {
                    currentFrame = 0;
                    currentSimTime++;
                }
            }

            //stopping background music once elevator simulation time done
            if (currentSimTime == lenSim - 1)
            {
                al_stop_sample(&backgroundMusicID);
            }

            view.SetRedraw(true);
        }
        DrawElevator();
    }
}

//visually draw the elevator
void ECElevatorObserver::DrawElevator()
{
    if (elevatorImageBack)
    {
        int imgWidth = al_get_bitmap_width(elevatorImageBack);
        int imgHeight = al_get_bitmap_height(elevatorImageBack);

        al_draw_scaled_bitmap(elevatorImageBack, 0, 0, imgWidth, imgHeight, 0, 0, view.GetWidth(), view.GetHeight(), 0);
    }
    else
    {
        std::cout << "Failed to draw elevator_back.png image!" << std::endl;
    }

    //draw filled rectangle for whole elevator shaft
    view.DrawFilledRectangle(view.GetWidth() / 2 - 100, topFloorY, view.GetWidth() / 2 + 100, bottomFloorY + floorHeight, ECGV_SILVER);
    //draw elevator shaft border lines
    view.DrawRectangle(view.GetWidth() / 2 - 100, topFloorY, view.GetWidth() / 2 + 100, bottomFloorY + floorHeight, 5, ECGV_BLACK);

    // Draw floor lines and triangles for buttons
    for (int floor = 1; floor < numFloors; floor++)
    {
        int y = bottomFloorY - (floor - 1) * floorHeight; //get y pos of each line
        view.DrawLine(view.GetWidth() / 2 - 100, y, view.GetWidth() / 2 + 100, y, 5, ECGV_BLACK); //draw shaft lines
    }

    //updates cabin position frame by frame
    int prevFloor = states[currentSimTime].floor;
    int prevY = bottomFloorY - (prevFloor - 1) * floorHeight;
    int nextFloor = prevFloor;
    if (currentSimTime < lenSim - 1)
    {
        nextFloor = states[currentSimTime + 1].floor;
    }
    int nextY = bottomFloorY - (nextFloor - 1) * floorHeight;
    double t = double(currentFrame) / framesPerStep;
    cabinY = (int)(prevY + (nextY - prevY) * t);

    //drawing cabin
    if (elevatorImageCabin)
    {
        int imgWidth = al_get_bitmap_width(elevatorImageCabin);
        int imgHeight = al_get_bitmap_height(elevatorImageCabin);

        int cabinWidth = (cabinX + 50) - (cabinX - 50);
        int cabinHeight = floorHeight;

        al_draw_scaled_bitmap(elevatorImageCabin, 0, 0, imgWidth, imgHeight, cabinX - 99, cabinY + 1, cabinWidth + 97, cabinHeight - 3, 0);
        view.DrawRectangle(cabinX - 99, cabinY + 1, cabinX - 99 + cabinWidth + 97, cabinY + 1 + cabinHeight - 3, 3.5, ECGV_BLACK);
    }
    else
    {
        std::cout << "Failed to draw elevator_cabin.png image!" << std::endl;
    }

    //get current state at current time
    const ECElevatorState& st = states[currentSimTime];

    //drawing passengers inside the elevator shaft
    DrawWaitingPassengers(st);

    //draw onboard passengers
    int pxStart = cabinX - 40;
    int pyStart = cabinY + 20;
    for (int i = 0; i < st.onboard.size(); i++)
    {
        auto& reqInfo = st.onboard[i];
        int px = pxStart + (int(i) % 4) * (passengerRadius * 2 + margin);
        int py = pyStart + (int(i) / 4) * (passengerRadius * 2 + margin);

        view.DrawFilledCircle(px, py, passengerRadius, ECGV_WHITE);
        std::string dest = std::to_string(reqInfo.destFloor);
        view.DrawText(px - 5, py - 5, dest.c_str(), ECGV_BLACK);

    }

    DrawTimeAndProgressBar();
}

void ECElevatorObserver::DrawTimeAndProgressBar()
{
    std::string timeText = "Time: " + std::to_string(currentSimTime);
    view.DrawText(210, bottomFloorY + floorHeight + 40, timeText.c_str(), ECGV_BLACK);

    int barX = 115;
    int barY = bottomFloorY + floorHeight + 90;
    int barHeight = 20;
    int barWidth = 200;

    view.DrawRectangle(barX, barY, barX + barWidth, barY + barHeight, 2, ECGV_BLACK);
    double prog = double(currentSimTime) / double(lenSim - 1);
    int filledWidth = int(prog * barWidth);
    view.DrawFilledRectangle(barX, barY, barX + filledWidth, barY + barHeight, ECGV_OLIVE_GREEN);
}

void ECElevatorObserver::DrawWaitingPassengers(const ECElevatorState& st)
{
    for (int floorNum = 1; floorNum <= numFloors; floorNum++) //for each floor
    {
        int y = bottomFloorY - (floorNum - 1) * floorHeight; //position to draw person at

        if (st.waitingMap.count(floorNum) > 0) //if there are ppl to draw
        {
            view.DrawFilledCircle(view.GetWidth() / 2 - 120, y + 20, 10, ECGV_GREEN);
            int upCount = 0;
            int downCount = 0;
            for (auto& reqInfo : st.waitingMap.at(floorNum)) //for each person in RequestInfo on this floor
            {
                bool goingUp = reqInfo.goingUp; //update going up
                int dest = reqInfo.destFloor; //update where they are going

                int px, py = y; //offset
                if (goingUp) //draw button on left if going up
                {
                    px = view.GetWidth() / 2 - 180 - (upCount * (passengerRadius * 2 + margin));
                    upCount++;
                }
                else //draw button on right if going down
                {
                    px = view.GetWidth() / 2 + 180 + (downCount * (passengerRadius * 2 + margin));
                    downCount++;
                }

                view.DrawFilledCircle(px, py, passengerRadius, ECGV_PURPLE); //draw person in yellow

                std::string destStr = std::to_string(dest);
                view.DrawText(px - 5, py - 5, destStr.c_str(), ECGV_BLACK); //write underneath where they are going
            }
        }
    }
}