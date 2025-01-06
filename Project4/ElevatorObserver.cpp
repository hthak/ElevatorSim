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
//margin = how far apart each passenger is
ECElevatorObserver::ECElevatorObserver(ECGraphicViewImp& viewIn, int numFloors,
    const std::vector<ECElevatorState>& allStates, int lenSim) :
    view(viewIn), numFloors(numFloors), states(allStates), lenSim(lenSim),
    currentTime(0), paused(false), floorHeight(100), topFloorY(100),
    framesPerStep(10), currentFrame(0), currentSimTime(0)
{
    bottomFloorY = topFloorY + (numFloors - 1) * floorHeight;
    cabinX = view.GetWidth() / 2;
    cabinY = bottomFloorY;

    //load elevator music
    backgroundMusic = al_load_sample("elevator_music.ogg");
    if (!backgroundMusic)
    {
        std::cout << "Failed to load elevator_music.ogg music file!" << std::endl;
    }
    backgroundMusicInstance = al_create_sample_instance(backgroundMusic);
    al_attach_sample_instance_to_mixer(backgroundMusicInstance, al_get_default_mixer()); //add to mixer
    al_set_sample_instance_gain(backgroundMusicInstance, 0.7); //sets volume
    al_set_sample_instance_playmode(backgroundMusicInstance, ALLEGRO_PLAYMODE_LOOP); //loops
    al_play_sample_instance(backgroundMusicInstance); //play elevator background music

    //load elevator background image once in constructor
    elevatorImageBack = al_load_bitmap("elevator_back_new.png");
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

    //load elevator shaft image
    shaftImage = al_load_bitmap("front_door.png");
    if (!shaftImage)
    {
        std::cout << "Failed to load front_door.png image!" << std::endl;
    }

    dingSound = al_load_sample("ding.ogg");
    if (!dingSound)
    {
        std::cout << "Failed to load ding.ogg music file!" << std::endl;
    }
    dingSoundInstance = al_create_sample_instance(dingSound);
    al_attach_sample_instance_to_mixer(dingSoundInstance, al_get_default_mixer());
    al_set_sample_instance_gain(dingSoundInstance, 0.3); //set volume
    al_set_sample_instance_playmode(dingSoundInstance, ALLEGRO_PLAYMODE_ONCE); //playmode once

    manImage = al_load_bitmap("man.png");
    if (!manImage)
    {
        std::cout << "Failed to load man.png image!" << std::endl;
    }

    jerseyFont = al_load_font("jersey.ttf", 22, 0);
    if (!jerseyFont)
    {
        std::cout << "Failed to load jersey.ttf font!" << std::endl;
    }

    segmentedFont = al_load_font("segmented.ttf", 60, 0);
    if (!segmentedFont)
    {
        std::cout << "Failed to load segmented.ttf font!" << std::endl;
    }

    upArrowImage = al_load_bitmap("up_arrow.png");
    if (!upArrowImage)
    {
        std::cout << "Failed to load up_arrow.png" << std::endl;
    }

    downArrowImage = al_load_bitmap("down_arrow.png");
    if (!downArrowImage)
    {
        std::cout << "Failed to load down_arrow.png" << std::endl;
    }

    //draw initally by itself
    view.SetRedraw(true);
}

ECElevatorObserver::~ECElevatorObserver() //destructor free music variables in mem
{
    if (backgroundMusicInstance)
    {
        al_destroy_sample_instance(backgroundMusicInstance);
        backgroundMusicInstance = nullptr;
    }
    if (dingSoundInstance)
    {
        al_destroy_sample_instance(dingSoundInstance);
        dingSoundInstance = nullptr;
    }
    if (backgroundMusic)
    {
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
    if (shaftImage)
    {
        al_destroy_bitmap(shaftImage);
        shaftImage = nullptr;
    }
    if (manImage)
    {
        al_destroy_bitmap(manImage);
        manImage = nullptr;
    }
    if (jerseyFont)
    {
        al_destroy_font(jerseyFont);
        jerseyFont = nullptr;
    }
    if (upArrowImage)
    {
        al_destroy_bitmap(upArrowImage);
        upArrowImage = nullptr;
    }
    if (downArrowImage)
    {
        al_destroy_bitmap(downArrowImage);
        downArrowImage = nullptr;
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
        if (!paused)
        {
            if (!al_get_sample_instance_playing(backgroundMusicInstance))
            {
                al_set_sample_instance_position(backgroundMusicInstance, currentBackMusicPos);
                al_set_sample_instance_playing(backgroundMusicInstance, true);
            }
            
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
                        al_play_sample_instance(dingSoundInstance);
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
                al_stop_sample_instance(backgroundMusicInstance);
            }

            view.SetRedraw(true);
        }
        else //paused
        {
            if (al_get_sample_instance_playing(backgroundMusicInstance))
            {
                currentBackMusicPos = al_get_sample_instance_position(backgroundMusicInstance);
                al_set_sample_instance_playing(backgroundMusicInstance, false);
            }
            al_set_sample_instance_playing(dingSoundInstance, false);
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
    view.DrawRectangle(view.GetWidth() / 2 - 100, topFloorY, view.GetWidth() / 2 + 100, bottomFloorY + floorHeight, 5, ECGV_WHITE);

    // Draw floor lines and triangles for buttons
    for (int floor = 1; floor <= numFloors; floor++)
    {
        int y = bottomFloorY - (floor - 1) * floorHeight; //get y pos of each line

        //shaft images
        int shaftImgWidth = al_get_bitmap_width(shaftImage);
        int shaftImgHeight = al_get_bitmap_height(shaftImage);
        al_draw_scaled_bitmap(shaftImage, 0, 0, shaftImgWidth, shaftImgHeight, view.GetWidth() / 2 - 100, y, 200, floorHeight, 0);

        //triangles
        int buttonBaseX = view.GetWidth() / 2 + 50;
        int floorMidY = y + floorHeight / 2 - 5;

        ECGVColor upColor = ECGV_SILVER;
        ECGVColor downColor = ECGV_SILVER;

        const ECElevatorState& st = states[currentSimTime];
        if (st.waitingMap.count(floor) > 0) //if there are ppl waiting at the floor
        {
            for (const auto& info : st.waitingMap.at(floor))
            {
                if (info.goingUp)
                {
                    upColor = ECGV_YELLOW;
                }
                else
                {
                    downColor = ECGV_YELLOW;
                }
            }
        }

        view.DrawRectangle(buttonBaseX - 9, floorMidY - 14, buttonBaseX + 9, floorMidY + 14, 1, ECGV_WHITE);
        view.DrawFilledRectangle(buttonBaseX - 8, floorMidY - 13, buttonBaseX + 8, floorMidY + 13, ECGV_BLACK);
        
        view.DrawFilledTriangle(buttonBaseX, floorMidY - 8, buttonBaseX + 6, floorMidY - 2, buttonBaseX - 6, floorMidY - 2, upColor);
        view.DrawFilledTriangle(buttonBaseX, floorMidY + 8, buttonBaseX + 6, floorMidY + 2, buttonBaseX - 6, floorMidY + 2, downColor);
    }

    //draw screen for elevator
    int screenWidth = 180;
    int screenHeight = 90;
    int screenX = view.GetWidth() / 2 - screenWidth - 250;
    int screenY = topFloorY + 200;
    view.DrawRectangle(screenX - 3, screenY - 3, screenX + screenWidth + 3, screenY + screenHeight + 3, 3, ECGV_WHITE);
    view.DrawFilledRectangle(screenX, screenY, screenX + screenWidth, screenY + screenHeight, ECGV_BLACK);
    int currentFloor = states[currentSimTime].floor;
    EC_ELEVATOR_DIR direction = states[currentSimTime].dir;
    if (direction == EC_ELEVATOR_UP)
    {
        al_draw_scaled_bitmap(upArrowImage, 0, 0, al_get_bitmap_width(upArrowImage), al_get_bitmap_height(upArrowImage),
            screenX + 30, screenY + (screenHeight - 40) / 2, 40, 40, 0);
    }
    else if (direction == EC_ELEVATOR_DOWN)
    {
        al_draw_scaled_bitmap(downArrowImage, 0, 0, al_get_bitmap_width(downArrowImage), al_get_bitmap_height(downArrowImage),
            screenX + 30, screenY + (screenHeight - 40) / 2, 40, 40, 0);
    }
    else
    {
        view.DrawTextFont(screenX + 60, screenY + (screenHeight / 2) - 30, "-", ECGV_RED, segmentedFont);
    }

    std::string floorText = std::to_string(currentFloor);
    view.DrawTextFont(screenX + 120, screenY + (screenHeight / 2) - 30, floorText.c_str(), ECGV_RED, segmentedFont);

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
        view.DrawRectangle(cabinX - 99, cabinY + 1, cabinX - 99 + cabinWidth + 97, cabinY + 1 + cabinHeight - 3, 4, ECGV_SILVER);
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
    int pxStart = cabinX - 100;
    int pyStart = cabinY + 25;
    int manW = al_get_bitmap_width(manImage);
    int manH = al_get_bitmap_height(manImage);
    int scaledH = floorHeight / 1.5;
    double scaleFactor = double(scaledH) / double(manH);
    int scaledW = int(manW * scaleFactor);
    for (int i = 0; i < st.onboard.size(); i++)
    {
        auto& reqInfo = st.onboard[i];
        int px = pxStart + i * (scaledW * 0.5);
        int py = pyStart;
        al_draw_scaled_bitmap(manImage, 0, 0, manW, manH, px, py, scaledW, scaledH, 0);

        //view.DrawFilledCircle(px, py, passengerRadius, ECGV_WHITE);
        std::string dest = std::to_string(reqInfo.destFloor);
        //view.DrawText(px - 5, py - 5, dest.c_str(), ECGV_WHITE);
        view.DrawTextFont(px + (scaledW / 2), py + 15, dest.c_str(), ECGV_WHITE, jerseyFont);

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
        int y = bottomFloorY - (floorNum - 2) * floorHeight; //position to draw person at

        if (st.waitingMap.count(floorNum) > 0) //if there are ppl to draw
        {            
            int baseX = view.GetWidth() / 2 + 90;
            int spacing = 10;
            int manW = al_get_bitmap_width(manImage);
            int manH = al_get_bitmap_height(manImage);
            int scaledH = floorHeight / 1.5;
            double scaleFactor = double(scaledH) / double(manH);
            int scaledW = int(manW * scaleFactor);
            int count = 0;

            for (auto& reqInfo : st.waitingMap.at(floorNum)) //for each person in RequestInfo on this floor
            {
                bool goingUp = reqInfo.goingUp; //update going up
                int dest = reqInfo.destFloor; //update where they are going
                count++;

                int px = baseX + count * (scaledW * 0.5);
                int py = y - scaledH;
                
                al_draw_scaled_bitmap(manImage, 0, 0, manW, manH, px, py, scaledW, scaledH, 0);

                std::string destStr = std::to_string(dest);
                view.DrawTextFont(px + scaledW/2, py + 15, destStr.c_str(), ECGV_WHITE, jerseyFont); //write underneath where they are going
            }
        }
    }
}