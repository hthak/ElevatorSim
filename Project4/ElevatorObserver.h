#pragma once
#ifndef ElevatorObserver_h
#define ElevatorObserver_h

#include "ECObserver.h"
#include "ECGraphicViewImp.h"
#include "ECElevatorSim.h"
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

class ECElevatorSim;

class ResourceFactory //factory class to load resources using shared pointers
{
public:
    //image factory
    static std::shared_ptr<ALLEGRO_BITMAP> loadBitmap(const std::string& name)
    {
        ALLEGRO_BITMAP* bitmap = al_load_bitmap(name.c_str()); //create ALLEGRO_BITMAP object
        if (!bitmap) //error fallback
        {
            std::cout << "Failed to load bitmap: " << name << std::endl;
        }
        return std::shared_ptr<ALLEGRO_BITMAP>(bitmap, al_destroy_bitmap); //shared pointer allows it to be deleted automatically
    }

    //font factory
    static std::shared_ptr<ALLEGRO_FONT> loadFont(const std::string& name, int size)
    {
        ALLEGRO_FONT* font = al_load_font(name.c_str(), size, 0); //create ALLEGRO_FONT object
        if (!font) //error fallback
        {
            std::cout << "Failed to load font: " << name << std::endl;
        }
        return std::shared_ptr<ALLEGRO_FONT>(font, al_destroy_font);
    }

    //music sample factory
    static std::shared_ptr<ALLEGRO_SAMPLE> loadSample(const std::string& name)
    {
        ALLEGRO_SAMPLE* sample = al_load_sample(name.c_str());
        if (!sample)
        {
            std::cout << "Failed to load sample: " << name << std::endl;
        }
        return std::shared_ptr<ALLEGRO_SAMPLE>(sample, al_destroy_sample);
    }

    //music sample instance factory
    static std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> loadSampleInstance(ALLEGRO_SAMPLE* sample)
    {
        ALLEGRO_SAMPLE_INSTANCE* sampleInstance = al_create_sample_instance(sample);
        if (!sampleInstance)
        {
            std::cout << "Failed to load sample instance from sample: " << sample << std::endl;
        }
        return std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>(sampleInstance, al_destroy_sample_instance);
    }
};


//frontend
class ECElevatorObserver : public ECObserver
{
public:
    ECElevatorObserver(ECGraphicViewImp& viewIn, int numFloors, const std::vector<ECElevatorState>& allStates, int lenSim);

    virtual void Update();

private:
    //view reference
    ECGraphicViewImp& view;

    //constants
    static const int floorHeight = 100;
    static const int cabinWidth = 100;
    static const int cabinHeight = 100;

    //music shared pointer variables
    std::shared_ptr<ALLEGRO_SAMPLE> backgroundMusic;
    std::shared_ptr<ALLEGRO_SAMPLE> dingSound;
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> backgroundMusicInstance;
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> dingSoundInstance;
    unsigned int currentBackMusicPos = 0;

    //image shared pointer variables
    std::shared_ptr<ALLEGRO_BITMAP> elevatorImageBack;
    std::shared_ptr<ALLEGRO_BITMAP> elevatorImageCabin;
    std::shared_ptr<ALLEGRO_BITMAP> shaftImage;
    std::shared_ptr<ALLEGRO_BITMAP> manImage;
    std::shared_ptr<ALLEGRO_BITMAP> upArrowImage;
    std::shared_ptr<ALLEGRO_BITMAP> downArrowImage;

    //font shared pointer variables
    std::shared_ptr<ALLEGRO_FONT> jerseyFont;
    std::shared_ptr<ALLEGRO_FONT> segmentedFont;

    //redraw the frontend
    void SetRedraw(bool f)
    {
        view.SetRedraw(f);
    }

    //drawing variables
    int numFloors; //num of elevator floors
    int topFloorY; //y pos of top floor
    int bottomFloorY; //y pos of bottom floor
    int cabinX;
    int cabinY;

    //stores all action states
    const std::vector<ECElevatorState>& states;

    //timing properties
    int lenSim;
    bool paused;
    static const int framesPerStep = 8;
    int currentFrame;
    int currentSimTime;

    //helper methods
    void DrawElevator();
    void DrawTimeAndProgressBar();
    void DrawWaitingPassengers(const ECElevatorState& st);

};
#endif /* ElevatorObserver_h */