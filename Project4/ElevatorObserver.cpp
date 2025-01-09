#include "ElevatorObserver.h"
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <iostream>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

//----------------------------------------------------------------------------------------------------------------------------
// ECElevatorObserver Implementation
//----------------------------------------------------------------------------------------------------------------------------
ECElevatorObserver::ECElevatorObserver(ECGraphicViewImp& viewIn, int numFloors,
    const std::vector<ECElevatorState>& allStates, int lenSim) :
    view(viewIn), numFloors(numFloors), states(allStates), lenSim(lenSim),
    paused(false), topFloorY(100), currentSimTime(0), currentFrame(0)
{
    bottomFloorY = topFloorY + (numFloors - 1) * FLOOR_HEIGHT;
    cabinX = view.GetWidth() / 2;
    cabinY = bottomFloorY;

    //create images from ResourceFactory
    elevatorImageBack = ResourceFactory::loadBitmap("elevator_back_new.png");
    elevatorImageCabin = ResourceFactory::loadBitmap("elevator_cabin.png");
    shaftImage = ResourceFactory::loadBitmap("front_door.png");
    manImage = ResourceFactory::loadBitmap("man.png");
    upArrowImage = ResourceFactory::loadBitmap("up_arrow.png");
    downArrowImage = ResourceFactory::loadBitmap("down_arrow.png");

    //create fonts from ResourceFactory
    jerseyFont = ResourceFactory::loadFont("jersey.ttf", 27);
    segmentedFont = ResourceFactory::loadFont("segmented.ttf", 60);
    displayFont = ResourceFactory::loadFont("MiguerSans-Regular.ttf", 50);

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
    ECGVEventType evt = view.GetCurrEvent(); //current keyboard event
    if (evt == ECGV_EV_TIMER)
    {
        if (!paused)
        {
            //if music is not playing, resume where it was paused
            if (!al_get_sample_instance_playing(backgroundMusicInstance.get()) && musicOn)
            {
                PlayAllMusic();
            }
            
            //advance animation frame
            if (currentSimTime < lenSim - 1)
            {
                currentFrame++;
                if (currentFrame == 1 && dingSoundInstance)
                {
                    //elevator ding sound logic
                    const ECElevatorState& prevState = states[currentSimTime];
                    const ECElevatorState& currState = states[currentSimTime + 1];
                    bool wasMoving = (prevState.dir == EC_ELEVATOR_UP || prevState.dir == EC_ELEVATOR_DOWN);
                    bool isStoppedNow = currState.dir == EC_ELEVATOR_STOPPED;
                    if (wasMoving && isStoppedNow && musicOn) //only play when stops at a floor
                    {
                        al_play_sample_instance(dingSoundInstance.get());
                    }
                }
                
                if (currentFrame >= FRAMES_PER_STEP)
                {
                    currentFrame = 0;
                    currentSimTime++;
                }
            }

            //stopping background music once elevator simulation time done
            if (currentSimTime == lenSim - 1 && backgroundMusicInstance)
            {
                al_stop_sample_instance(backgroundMusicInstance.get());
                al_stop_sample_instance(dingSoundInstance.get());
            }
            view.SetRedraw(true);
        }
        else //paused
        {
            PauseAllMusic();
        }
        DrawElevator();
    }
    else if (evt == ECGV_EV_MOUSE_BUTTON_DOWN) //click event heard
    {
        int pressX, pressY;
        view.GetCursorPosition(pressX, pressY);
        if (IsInRect(pressX, pressY, pauseBtnRect))
        {
            paused = !paused;
            PauseAllMusic();
            SetRedraw(true);
        }
        else if (IsInRect(pressX, pressY, musicBtnRect))
        {
            musicOn = !musicOn;
            if (musicOn)
            {
                PlayAllMusic();
            }
            else
            {
                PauseAllMusic();
            }
            SetRedraw(true);
        }
    }
}

void ECElevatorObserver::PauseAllMusic()
{
    //stop music and save time stopped at
    if (backgroundMusicInstance && al_get_sample_instance_playing(backgroundMusicInstance.get()))
    {
        currentBackMusicPos = al_get_sample_instance_position(backgroundMusicInstance.get());
        al_set_sample_instance_playing(backgroundMusicInstance.get(), false);
    }
    if (dingSoundInstance) //don't need to save time since really short track
    {
        al_set_sample_instance_playing(dingSoundInstance.get(), false);
    }
}

void ECElevatorObserver::PlayAllMusic()
{
    //play background music where paused
    al_set_sample_instance_position(backgroundMusicInstance.get(), currentBackMusicPos);
    al_set_sample_instance_playing(backgroundMusicInstance.get(), true);
}

bool ECElevatorObserver::IsInRect(int x, int y, const ALLEGRO_RECT& rect)
{
    return (x >= rect.left && y >= rect.top && x <= rect.right && y <= rect.bottom);
}

void ECElevatorObserver::DrawElevator()
{
    if (elevatorImageBack)
    {
        int w = al_get_bitmap_width(elevatorImageBack.get());
        int h = al_get_bitmap_height(elevatorImageBack.get());
        al_draw_scaled_bitmap(elevatorImageBack.get(), 0, 0, w, h, 0, 0, view.GetWidth(), view.GetHeight(), 0);
    }

    //draw border around elevator
    view.DrawRectangle(view.GetWidth()/2 - 100, topFloorY, view.GetWidth()/2 + 100, bottomFloorY + FLOOR_HEIGHT, 5, ECGV_WHITE);

    const ECElevatorState& st = states[currentSimTime];

    //draw floor images, font, and buttons
    for (int floor = 1; floor <= numFloors; floor++)
    {
        int y = bottomFloorY - (floor - 1) * FLOOR_HEIGHT;

        //shaft door images
        int shaftW = al_get_bitmap_width(shaftImage.get());
        int shaftH = al_get_bitmap_height(shaftImage.get());
        al_draw_scaled_bitmap(shaftImage.get(), 0, 0, shaftW, shaftH, view.GetWidth() / 2 - 100, y, 200, FLOOR_HEIGHT, 0);

        //button variables
        int buttonBaseX = view.GetWidth() / 2 + 50;
        int floorMidY = y + FLOOR_HEIGHT / 2 - 5;
        ECGVColor upColor = ECGV_SILVER;
        ECGVColor downColor = ECGV_SILVER;

        if (st.waitingMap.count(floor) > 0) //if there are ppl waiting at the floor
        {
            for (const auto& info : st.waitingMap.at(floor))
            {
                if (info.goingUp) upColor = ECGV_RED;
                else downColor = ECGV_RED;
            }
        }

        //draw back plate for buttons
        view.DrawRectangle(buttonBaseX - 9, floorMidY - 14, buttonBaseX + 9, floorMidY + 14, 1, ECGV_WHITE);
        view.DrawFilledRectangle(buttonBaseX - 8, floorMidY - 13, buttonBaseX + 8, floorMidY + 13, ECGV_BLACK);
        
        //draw triangle buttons
        view.DrawFilledTriangle(buttonBaseX, floorMidY - 8, buttonBaseX + 6, floorMidY - 2, buttonBaseX - 6, floorMidY - 2, upColor);
        view.DrawFilledTriangle(buttonBaseX, floorMidY + 8, buttonBaseX + 6, floorMidY + 2, buttonBaseX - 6, floorMidY + 2, downColor);
    }

    DrawElevatorScreen(st);

    //update cabin position frame by frame
    int prevFloor = st.floor;
    int prevY = bottomFloorY - (prevFloor - 1) * FLOOR_HEIGHT;
    int nextFloor = (currentSimTime < lenSim - 1) ? states[currentSimTime + 1].floor : prevFloor;
    int nextY = bottomFloorY - (nextFloor - 1) * FLOOR_HEIGHT;
    double t = double(currentFrame)/FRAMES_PER_STEP;
    cabinY = (int)(prevY + (nextY - prevY) * t);

    DrawElevatorCabin();

    DrawWaitingPassengers(st);

    DrawOnboardPassengers(st);

    DrawTimeAndProgressBar();

    DrawButtons();
}

void ECElevatorObserver::DrawButtons()
{
    view.DrawFilledRectangle(pauseBtnRect.left, pauseBtnRect.top, pauseBtnRect.right, pauseBtnRect.bottom, ECGV_GREY);
    view.DrawRectangle(pauseBtnRect.left, pauseBtnRect.top, pauseBtnRect.right, pauseBtnRect.bottom, 3, ECGV_BLACK);

    const char* labelPause = paused ? "Resume" : "Pause";
    int midX = (pauseBtnRect.left + pauseBtnRect.right) / 2;
    int midY = (pauseBtnRect.top + pauseBtnRect.bottom) / 2;
    view.DrawTextFont(midX , midY - 30, labelPause, ECGV_WHITE, displayFont.get());

    view.DrawFilledRectangle(musicBtnRect.left, musicBtnRect.top, musicBtnRect.right, musicBtnRect.bottom, ECGV_DARK_GREY);
    view.DrawRectangle(musicBtnRect.left, musicBtnRect.top,musicBtnRect.right, musicBtnRect.bottom, 3, ECGV_BLACK);

    const char* labelMusic = musicOn ? "Music On" : "Music Off";
    int midX2 = (musicBtnRect.left + musicBtnRect.right) / 2;
    int midY2 = (musicBtnRect.top + musicBtnRect.bottom) / 2;
    view.DrawTextFont(midX2, midY2 - 30, labelMusic, ECGV_WHITE, displayFont.get());
}

void ECElevatorObserver::DrawElevatorScreen(const ECElevatorState& st)
{
    //constants
    int screenWidth = 130;
    int screenHeight = 90;
    int screenX = view.GetWidth()/2 - screenWidth - 250;
    int screenY = topFloorY + 200;

    //draw outer rectangle border
    view.DrawRectangle(screenX - 2, screenY - 2, screenX + screenWidth + 2, screenY + screenHeight + 2, 3, ECGV_GREY);

    //draw black inner fill
    view.DrawFilledRectangle(screenX, screenY, screenX + screenWidth, screenY + screenHeight, ECGV_BLACK);

    //logic for screen's up or down arrow
    EC_ELEVATOR_DIR direction = st.dir;
    if (direction == EC_ELEVATOR_UP && upArrowImage)
    {
        al_draw_scaled_bitmap(upArrowImage.get(), 0, 0, al_get_bitmap_width(upArrowImage.get()), al_get_bitmap_height(upArrowImage.get()),
            screenX + 20, screenY + (screenHeight - 40) / 2, 40, 40, 0);
    }
    else if (direction == EC_ELEVATOR_DOWN && downArrowImage)
    {
        al_draw_scaled_bitmap(downArrowImage.get(), 0, 0, al_get_bitmap_width(downArrowImage.get()), al_get_bitmap_height(downArrowImage.get()),
            screenX + 20, screenY + (screenHeight - 40) / 2, 40, 40, 0);
    }
    else
    {
        view.DrawTextFont(screenX + 45, screenY + (screenHeight / 2) - 30, "-", ECGV_RED, segmentedFont.get());
    }

    //write curent floor in segmented font
    std::string floorText = std::to_string(st.floor);
    view.DrawTextFont(screenX + 100, screenY + (screenHeight / 2) - 30, floorText.c_str(), ECGV_RED, segmentedFont.get());
}

void ECElevatorObserver::DrawElevatorCabin()
{
    int w = al_get_bitmap_width(elevatorImageCabin.get());
    int h = al_get_bitmap_height(elevatorImageCabin.get());

    int cabinWidth = 100;
    int cabinHeight = FLOOR_HEIGHT;

    al_draw_scaled_bitmap(elevatorImageCabin.get(), 0, 0, w, h, cabinX - 99, cabinY + 1, cabinWidth + 97, cabinHeight - 3, 0);
    view.DrawRectangle(cabinX - 99, cabinY + 1, cabinX - 99 + cabinWidth + 97, cabinY + 1 + cabinHeight - 3, 4, ECGV_BLACK);
    
}

void ECElevatorObserver::DrawOnboardPassengers(const ECElevatorState& st)
{
    int xStart = cabinX - 100;
    int yStart = cabinY + 25;
    int manW = al_get_bitmap_width(manImage.get());
    int manH = al_get_bitmap_height(manImage.get());
    int scaledH = FLOOR_HEIGHT / 1.5;
    double scaleFactor = double(scaledH) / double(manH);
    int scaledW = int(manW * scaleFactor);

    //for each onboard passenger
    for (int i = 0; i < st.onboard.size(); i++)
    {
        int px = xStart + i * (scaledW * 0.5);
        int py = yStart;

        al_draw_scaled_bitmap(manImage.get(), 0, 0, manW, manH, px, py, scaledW, scaledH, 0); //draw man image

        //draw floor destination for this passenger
        std::string dest = std::to_string(st.onboard[i].destFloor);
        view.DrawTextFont(px + (scaledW / 2), py + 15, dest.c_str(), ECGV_WHITE, jerseyFont.get());
    }
}

void ECElevatorObserver::DrawTimeAndProgressBar()
{
    std::string timeText = "Time: " + std::to_string(currentSimTime);
    view.DrawTextFont(285, bottomFloorY - 70, timeText.c_str(), ECGV_WHITE, displayFont.get());

    int barX = 150;
    int barY = bottomFloorY - 100;
    int barHeight = 20;
    int barWidth = 280;

    view.DrawRectangle(barX, barY, barX + barWidth, barY + barHeight, 3, ECGV_BLACK);
    double prog = double(currentSimTime) / double(lenSim - 1);
    int filledWidth = int(prog * barWidth);
    view.DrawFilledRectangle(barX + 1, barY + 2, barX + filledWidth - 2, barY + barHeight - 2, ECGV_WHITE);
}

void ECElevatorObserver::DrawWaitingPassengers(const ECElevatorState& st)
{
    for (int floorNum = 1; floorNum <= numFloors; floorNum++) //for each floor
    {
        int y = bottomFloorY - (floorNum - 2) * FLOOR_HEIGHT; //position to draw person at

        if (st.waitingMap.count(floorNum) > 0) //if there are ppl to draw
        {            
            int baseX = view.GetWidth() / 2 + 90;
            int manW = al_get_bitmap_width(manImage.get());
            int manH = al_get_bitmap_height(manImage.get());
            int scaledH = FLOOR_HEIGHT / 1.5;
            double scaleFactor = double(scaledH) / double(manH);
            int scaledW = int(manW * scaleFactor);
            int count = 0;

            for (auto& reqInfo : st.waitingMap.at(floorNum)) //for each person in RequestInfo on this floor
            {
                count++;
                int px = baseX + count * (scaledW * 0.5);
                int py = y - scaledH;
                
                al_draw_scaled_bitmap(manImage.get(), 0, 0, manW, manH, px, py, scaledW, scaledH, 0);

                std::string destStr = std::to_string(reqInfo.destFloor);
                view.DrawTextFont(px + scaledW/2, py + 15, destStr.c_str(), ECGV_WHITE, jerseyFont.get()); //write underneath where they are going
            }
        }
    }
}