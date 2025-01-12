#pragma once
#ifndef ElevatorObserver_h
#define ElevatorObserver_h

#include "ECObserver.h"
#include "ECGraphicViewImp.h"
#include "ECElevatorSim.h"
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

class ECElevatorSim;

//----------------------------------------------------------------------------------------------------------------------------
// ResourceFactory
// A factory class to load shared resources (bitmaps, fonts, samples, etc.) using shared pointers
//----------------------------------------------------------------------------------------------------------------------------
class ResourceFactory
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

//----------------------------------------------------------------------------------------------------------------------------
// Structure to hold cooridinates for rectangular buttons
//----------------------------------------------------------------------------------------------------------------------------
struct ALLEGRO_RECT
{
    float left, top, right, bottom;
};

//----------------------------------------------------------------------------------------------------------------------------
// ECElevatorobserver
// Inherits from ECObserver
// Frontend Observer that listens for timer events and draws the elevator, passengers, etc. visually
// Utilizes ResourceFactory to create images, fonts, audio instances, etc.
//----------------------------------------------------------------------------------------------------------------------------
class ECElevatorObserver : public ECObserver
{
public:
    //constructor
    ECElevatorObserver(ECGraphicViewImp& viewIn, int numFloors, const std::vector<ECElevatorState>& allStates, int lenSim);

    //defualt deconstructor since shared pointers deallocate automatically
    virtual ~ECElevatorObserver() = default;

    //updates view after new event
    virtual void Update() override;

private:
    //view reference
    ECGraphicViewImp& view;

    //redraws the frontend
    void SetRedraw(bool f) { view.SetRedraw(f); }

    //draw helper methods
    void DrawElevator();
    void DrawTimeAndProgressBar();
    void DrawWaitingPassengers(const ECElevatorState& st);
    void DrawOnboardPassengers(const ECElevatorState& st);
    void DrawElevatorScreen(const ECElevatorState& st);
    void DrawElevatorCabin();
    void DrawButtons();

    //helper method(s)
    bool IsInRect(int x, int y, const ALLEGRO_RECT& rect);
    void PauseAllMusic();
    void PlayAllMusic();

    //constants
    static constexpr int FLOOR_HEIGHT = 100;
    static constexpr int FRAMES_PER_STEP = 65;
    static const int cabinWidth = 100;
    static const int cabinHeight = 100;

    //elevator variables
    int numFloors;
    int topFloorY; //y pos of top floor
    int bottomFloorY; //y pos of bottom floor
    int cabinX; //x position of cabin
    int cabinY; //y position of cabin

    //timing properties
    int lenSim;
    bool paused;
    int currentFrame;
    int currentSimTime;
    const std::vector<ECElevatorState>& states;

    //state button
    bool musicOn = true;

    //clickable button positioning
    ALLEGRO_RECT pauseBtnRect = {185, 430, 390, 520};
    ALLEGRO_RECT musicBtnRect = {160, 560, 415, 650};

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
    std::shared_ptr<ALLEGRO_FONT> displayFont;
};
#endif /* ElevatorObserver_h */