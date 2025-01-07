#include "ElevatorObserver.h"
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <iostream>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

ECElevatorObserver::ECElevatorObserver(ECGraphicViewImp& viewIn, int numFloors,
    const std::vector<ECElevatorState>& allStates, int lenSim) :
    view(viewIn), numFloors(numFloors), states(allStates), lenSim(lenSim),
    paused(false), topFloorY(100), currentSimTime(0), currentFrame(0)
{
    bottomFloorY = topFloorY + (numFloors - 1) * floorHeight;
    cabinX = view.GetWidth() / 2; //x position of cabin
    cabinY = bottomFloorY; //y position of cabin

    //create images from ResourceFactory
    elevatorImageBack = ResourceFactory::loadBitmap("elevator_back_new.png");
    elevatorImageCabin = ResourceFactory::loadBitmap("elevator_cabin.png");
    shaftImage = ResourceFactory::loadBitmap("front_door.png");
    manImage = ResourceFactory::loadBitmap("man.png");
    upArrowImage = ResourceFactory::loadBitmap("up_arrow.png");
    downArrowImage = ResourceFactory::loadBitmap("down_arrow.png");

    //create fonts from ResourceFactory
    jerseyFont = ResourceFactory::loadFont("jersey.ttf", 22);
    segmentedFont = ResourceFactory::loadFont("segmented.ttf", 60);

    //create sound samples from ResourceFactory
    backgroundMusic = ResourceFactory::loadSample("elevator_music.ogg");
    dingSound = ResourceFactory::loadSample("ding.ogg");

    //create sample instances and set mixers from ResourceFactory for "elevator_music.ogg"
    backgroundMusicInstance = ResourceFactory::loadSampleInstance(backgroundMusic.get());
    al_attach_sample_instance_to_mixer(backgroundMusicInstance.get(), al_get_default_mixer()); //add to mixer
    al_set_sample_instance_gain(backgroundMusicInstance.get(), 0.7); //sets volume
    al_set_sample_instance_playmode(backgroundMusicInstance.get(), ALLEGRO_PLAYMODE_LOOP); //loops
    al_play_sample_instance(backgroundMusicInstance.get()); //play elevator background music

    //create sample instances and set mixers from ResourceFactory for "ding.ogg"
    dingSoundInstance = ResourceFactory::loadSampleInstance(dingSound.get());
    al_attach_sample_instance_to_mixer(dingSoundInstance.get(), al_get_default_mixer());
    al_set_sample_instance_gain(dingSoundInstance.get(), 0.4); //set volume
    al_set_sample_instance_playmode(dingSoundInstance.get(), ALLEGRO_PLAYMODE_ONCE); //playmode once

    //draw initally by itself
    view.SetRedraw(true);
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
            if (!al_get_sample_instance_playing(backgroundMusicInstance.get()))
            {
                al_set_sample_instance_position(backgroundMusicInstance.get(), currentBackMusicPos);
                al_set_sample_instance_playing(backgroundMusicInstance.get(), true);
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
                        al_play_sample_instance(dingSoundInstance.get());
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
                al_stop_sample_instance(backgroundMusicInstance.get());
            }

            view.SetRedraw(true);
        }
        else //paused
        {
            if (al_get_sample_instance_playing(backgroundMusicInstance.get()))
            {
                currentBackMusicPos = al_get_sample_instance_position(backgroundMusicInstance.get());
                al_set_sample_instance_playing(backgroundMusicInstance.get(), false);
            }
            al_set_sample_instance_playing(dingSoundInstance.get(), false);
        }
        DrawElevator();
    }
}

//visually draw the elevator
void ECElevatorObserver::DrawElevator()
{
    if (elevatorImageBack)
    {
        int imgWidth = al_get_bitmap_width(elevatorImageBack.get());
        int imgHeight = al_get_bitmap_height(elevatorImageBack.get());

        al_draw_scaled_bitmap(elevatorImageBack.get(), 0, 0, imgWidth, imgHeight, 0, 0, view.GetWidth(), view.GetHeight(), 0);
    }
    else
    {
        std::cout << "Failed to draw elevator_back.png image!" << std::endl;
    }

    //draw filled rectangle for whole elevator shaft
    view.DrawFilledRectangle(view.GetWidth() / 2 - 100, topFloorY, view.GetWidth() / 2 + 100, bottomFloorY + floorHeight, ECGV_SILVER);
    //draw elevator shaft border lines
    view.DrawRectangle(view.GetWidth() / 2 - 100, topFloorY, view.GetWidth() / 2 + 100, bottomFloorY + floorHeight, 5, ECGV_WHITE);

    //draw floor lines and triangles for buttons
    for (int floor = 1; floor <= numFloors; floor++)
    {
        int y = bottomFloorY - (floor - 1) * floorHeight; //get y pos of each line

        //shaft images
        int shaftImgWidth = al_get_bitmap_width(shaftImage.get());
        int shaftImgHeight = al_get_bitmap_height(shaftImage.get());
        al_draw_scaled_bitmap(shaftImage.get(), 0, 0, shaftImgWidth, shaftImgHeight, view.GetWidth() / 2 - 100, y, 200, floorHeight, 0);

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
                    upColor = ECGV_RED;
                }
                else
                {
                    downColor = ECGV_RED;
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
        al_draw_scaled_bitmap(upArrowImage.get(), 0, 0, al_get_bitmap_width(upArrowImage.get()), al_get_bitmap_height(upArrowImage.get()),
            screenX + 30, screenY + (screenHeight - 40) / 2, 40, 40, 0);
    }
    else if (direction == EC_ELEVATOR_DOWN)
    {
        al_draw_scaled_bitmap(downArrowImage.get(), 0, 0, al_get_bitmap_width(downArrowImage.get()), al_get_bitmap_height(downArrowImage.get()),
            screenX + 30, screenY + (screenHeight - 40) / 2, 40, 40, 0);
    }
    else
    {
        view.DrawTextFont(screenX + 60, screenY + (screenHeight / 2) - 30, "-", ECGV_RED, segmentedFont.get());
    }

    std::string floorText = std::to_string(currentFloor);
    view.DrawTextFont(screenX + 120, screenY + (screenHeight / 2) - 30, floorText.c_str(), ECGV_RED, segmentedFont.get());

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
        int imgWidth = al_get_bitmap_width(elevatorImageCabin.get());
        int imgHeight = al_get_bitmap_height(elevatorImageCabin.get());

        int cabinWidth = (cabinX + 50) - (cabinX - 50);
        int cabinHeight = floorHeight;

        al_draw_scaled_bitmap(elevatorImageCabin.get(), 0, 0, imgWidth, imgHeight, cabinX - 99, cabinY + 1, cabinWidth + 97, cabinHeight - 3, 0);
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

    //draw cabin passengers
    int pxStart = cabinX - 100;
    int pyStart = cabinY + 25;
    int manW = al_get_bitmap_width(manImage.get());
    int manH = al_get_bitmap_height(manImage.get());
    int scaledH = floorHeight / 1.5;
    double scaleFactor = double(scaledH) / double(manH);
    int scaledW = int(manW * scaleFactor);
    for (int i = 0; i < st.onboard.size(); i++)
    {
        auto& reqInfo = st.onboard[i];
        int px = pxStart + i * (scaledW * 0.5);
        int py = pyStart;
        al_draw_scaled_bitmap(manImage.get(), 0, 0, manW, manH, px, py, scaledW, scaledH, 0);

        //view.DrawFilledCircle(px, py, passengerRadius, ECGV_WHITE);
        std::string dest = std::to_string(reqInfo.destFloor);
        //view.DrawText(px - 5, py - 5, dest.c_str(), ECGV_WHITE);
        view.DrawTextFont(px + (scaledW / 2), py + 15, dest.c_str(), ECGV_WHITE, jerseyFont.get());

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
            int manW = al_get_bitmap_width(manImage.get());
            int manH = al_get_bitmap_height(manImage.get());
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
                
                al_draw_scaled_bitmap(manImage.get(), 0, 0, manW, manH, px, py, scaledW, scaledH, 0);

                std::string destStr = std::to_string(dest);
                view.DrawTextFont(px + scaledW/2, py + 15, destStr.c_str(), ECGV_WHITE, jerseyFont.get()); //write underneath where they are going
            }
        }
    }
}