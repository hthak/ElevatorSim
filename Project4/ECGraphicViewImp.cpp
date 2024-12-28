//
//  ECGraphicViewImp.cpp
//

#include "ECGraphicViewImp.h"
#include "allegro5/allegro_primitives.h"
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_ttf.h>
#include <iostream>


using namespace std;

const float FPS = 10;

//***********************************************************
// Allegro colors

ALLEGRO_COLOR arrayAllegroColors[ECGV_NUM_COLORS] =
{
    al_map_rgb_f(0, 0, 0),
    al_map_rgb_f(255,255,255),
    al_map_rgb_f(255,0,0),
    al_map_rgb_f(0,255,0),
    al_map_rgb_f(0,0,255),
    al_map_rgb_f(255,255,0),
    al_map_rgb_f(255,0,255),
    al_map_rgb_f(0,255,255),
    al_map_rgb(173, 216, 230),
    al_map_rgb(210, 210, 210),
    //al_map_rgb(77, 81, 109)
    al_map_rgb(68, 68, 68)
};

//***********************************************************
// A graphic view implementation
// This is built on top of Allegro library

ECGraphicViewImp::ECGraphicViewImp(int width, int height) : widthView(width), heightView(height), fRedraw(false), display(NULL), timer(NULL), event_queue(NULL)
{
    Init();
}
ECGraphicViewImp :: ~ECGraphicViewImp()
{
    Shutdown();
}

// Show the view. This would enter a forever loop, until quit is set
void ECGraphicViewImp::Show()
{
    //
    //int cursorxDown=-100, cursoryDown=-100, cursorxUp=-100, cursoryUp=-100;
    while (true)
    {
        // get current event
        evtCurrent = WaitForEvent();
        //std::cout << "evt: " << evtCurrent << std::endl;

        if (evtCurrent == ECGV_EV_NULL)
        {
            continue;
        }

        if (evtCurrent == ECGV_EV_CLOSE)
        {
            break;
        }

        // render start
        RenderStart();

        // draw something
        //DrawRectangle(100,100,200,200,3);
    //SetRedraw(true);

        // Notify clients
        Notify();

        // refresh view
        if (evtCurrent == ECGV_EV_TIMER)
        {
            if (fRedraw)
            {
                RenderEnd();
                fRedraw = false;
            }
        }

#if 0
        // handle this event: TBD
        int cursorx, cursory;
        GetCursorPosition(cursorx, cursory);
        //cout << "Event: " << evt << endl;
        if (evtCurrent == ECGV_EV_MOUSE_BUTTON_DOWN)
        {
            cout << "Cursor down: (" << cursorx << "," << cursory << ")\n";
            cursorxDown = cursorx;
            cursoryDown = cursory;
        }
        if (evtCurrent == ECGV_EV_MOUSE_BUTTON_UP)
        {
            cout << "Cursor up: (" << cursorx << "," << cursory << ")\n";
            cursorxUp = cursorx;
            cursoryUp = cursory;
            fRedraw = true;
        }
        if (evtCurrent == ECGV_EV_MOUSE_MOVING)
        {
            if (cursorxDown >= 0)
            {
                cout << "Current cursor position: " << cursorx << "," << cursory << endl;
                RenderStart();
                DrawRectangle(cursorxDown, cursoryDown, cursorx, cursory, 1);
                RenderEnd();
            }
        }
        if (evtCurrent == ECGV_EV_TIMER)
        {
            if (fRedraw)
            {
                RenderStart();
                cout << "down:(" << cursorxDown << "," << cursoryDown << ") up: (" << cursorxUp << "," << cursoryUp << ")\n";
                //DrawLine(cursorxDown, cursoryDown, cursorxUp, cursoryUp);
                DrawRectangle(cursorxDown, cursoryDown, cursorxUp, cursoryUp);
                RenderEnd();
                fRedraw = false;
                cursorxDown = -100;
                cursoryDown = -100;
            }
        }
#endif
    }
}

void ECGraphicViewImp::RenderStart()
{
    //std::cout << "Redraw bitmap..." << GetPosX() << "," << GetPosY() << std::endl;
    al_clear_to_color(al_map_rgb(255, 255, 255));
}


void ECGraphicViewImp::RenderEnd()
{
    //    al_draw_bitmap(algBitmap, GetPosX(), GetPosY(), 0);
    al_flip_display();
}


void ECGraphicViewImp::Init()
{
    cout << "Start init..\n";
    if (!al_init()) {
        cout << "failed to initialize allegro!\n";
        exit(-1);
    }

    if (!al_install_keyboard()) {
        cout << "failed to initialize the keyboard!\n";
        exit(-1);
    }

    if (!al_install_mouse()) {
        cout << "failed to initialize the mouse!\n";
        exit(-1);
    }
    timer = al_create_timer(1.0 / FPS);
    if (!timer) {
        cout << "failed to create timer!\n";
        exit(-1);
    }
    // create the display
    display = al_create_display(widthView, heightView);
    if (!display) {
        cout << "failed to create display!\n";
        Shutdown();
        exit(-1);
    }
    al_set_target_bitmap(al_get_backbuffer(display));
    // setup events
    event_queue = al_create_event_queue();
    if (!event_queue) {
        fprintf(stderr, "failed to create event_queue!\n");
        Shutdown();
        exit(-1);
    }
    //cout << "3\n";
    //#if 0
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());
    //#endif
    al_clear_to_color(al_map_rgb(255, 255, 255));
    al_flip_display();
    al_start_timer(timer);

    // init image
    al_init_image_addon();
    al_init_primitives_addon();

    // init font
    al_init_font_addon();
    al_init_ttf_addon();
    this->fontDef = al_load_font("C:\\Users\\Himanshu Thakur\\source\\repos\\Project4\\Project4\\lucon.ttf", 40, 0);
    if (this->fontDef == NULL)
    {
        cout << "Warning: font is not loaded!\n";
    }

    cout << "Done with initialization.\n";
}

void ECGraphicViewImp::Shutdown()
{
    //
    if (display != NULL)
    {
        al_destroy_display(display);
        display = NULL;
    }
    if (timer != NULL)
    {
        al_destroy_timer(timer);
        timer = NULL;
    }
    if (event_queue != NULL)
    {
        al_destroy_event_queue(event_queue);
        event_queue = NULL;
    }
}

ECGVEventType ECGraphicViewImp::WaitForEvent()
{
    //
    ALLEGRO_EVENT ev;
    al_wait_for_event(event_queue, &ev);
    //cout << "Process event...\n";

    if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
    {
        return ECGV_EV_CLOSE;
    }
    else if (ev.type == ALLEGRO_EVENT_TIMER) {
        return ECGV_EV_TIMER;
    }
    else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (ev.keyboard.keycode) {
        case ALLEGRO_KEY_UP:
            return ECGV_EV_KEY_DOWN_UP;

        case ALLEGRO_KEY_DOWN:
            return ECGV_EV_KEY_DOWN_DOWN;

        case ALLEGRO_KEY_LEFT:
            return ECGV_EV_KEY_DOWN_LEFT;

        case ALLEGRO_KEY_RIGHT:
            return ECGV_EV_KEY_DOWN_RIGHT;

        case ALLEGRO_KEY_Z:
            return ECGV_EV_KEY_DOWN_Z;

        case ALLEGRO_KEY_Y:
            return ECGV_EV_KEY_DOWN_Y;

        case ALLEGRO_KEY_D:
            return ECGV_EV_KEY_DOWN_D;

        case ALLEGRO_KEY_SPACE:
            return ECGV_EV_KEY_DOWN_SPACE;

        case ALLEGRO_KEY_G:
            return ECGV_EV_KEY_DOWN_G;

        }
    }
    else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
        switch (ev.keyboard.keycode) {
        case ALLEGRO_KEY_UP:
            return ECGV_EV_KEY_UP_UP;

        case ALLEGRO_KEY_DOWN:
            return ECGV_EV_KEY_UP_DOWN;

        case ALLEGRO_KEY_LEFT:
            return ECGV_EV_KEY_UP_LEFT;

        case ALLEGRO_KEY_RIGHT:
            return ECGV_EV_KEY_UP_RIGHT;

        case ALLEGRO_KEY_ESCAPE:
            return ECGV_EV_KEY_UP_ESCAPE;

        case ALLEGRO_KEY_Z:
            return ECGV_EV_KEY_UP_Z;

        case ALLEGRO_KEY_Y:
            return ECGV_EV_KEY_UP_Y;

        case ALLEGRO_KEY_D:
            return ECGV_EV_KEY_UP_D;

        case ALLEGRO_KEY_SPACE:
            return ECGV_EV_KEY_UP_SPACE;

        case ALLEGRO_KEY_G:
            return ECGV_EV_KEY_UP_G;

        }
    }
    else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
    {
        return ECGV_EV_MOUSE_BUTTON_DOWN;
    }
    else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
    {
        return ECGV_EV_MOUSE_BUTTON_UP;
    }
    else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES)
    {
        return ECGV_EV_MOUSE_MOVING;
    }
    //cout << "Event not recognized...\n";

    return ECGV_EV_NULL;
}

void ECGraphicViewImp::GetCursorPosition(int& cx, int& cy) const
{
    ALLEGRO_MOUSE_STATE state;
    al_get_mouse_state(&state);
    cx = state.x;
    cy = state.y;
}

// Drawing functions
void  ECGraphicViewImp::DrawLine(int x1, int y1, int x2, int y2, int thickness, ECGVColor color)
{
    // draw a line
    al_draw_line(x1, y1, x2, y2, arrayAllegroColors[color], thickness);
    //cout << "Draw line: (" << x1 << "," << y1 << " to (" << x2 << "," << y2 << ")\n";
}

void ECGraphicViewImp::DrawRectangle(int x1, int y1, int x2, int y2, int thickness, ECGVColor color)
{
    al_draw_rectangle(x1, y1, x2, y2, arrayAllegroColors[color], thickness);
}

void ECGraphicViewImp::DrawCircle(int xcenter, int ycenter, double radius, int thickness, ECGVColor color)
{
    al_draw_circle(xcenter, ycenter, radius, arrayAllegroColors[color], thickness);
}

void ECGraphicViewImp::DrawEllipse(int xcenter, int ycenter, double radiusx, double radiusy, int thickness, ECGVColor color)
{
    al_draw_ellipse(xcenter, ycenter, radiusx, radiusy, arrayAllegroColors[color], thickness);
}

void ECGraphicViewImp::DrawFilledRectangle(int x1, int y1, int x2, int y2, ECGVColor color)
{
    al_draw_filled_rectangle(x1, y1, x2, y2, arrayAllegroColors[color]);;
}

void ECGraphicViewImp::DrawFilledCircle(int xcenter, int ycenter, double radius, ECGVColor color)
{
    al_draw_filled_circle(xcenter, ycenter, radius, arrayAllegroColors[color]);
}

void ECGraphicViewImp::DrawFilledEllipse(int xcenter, int ycenter, double radiusx, double radiusy, ECGVColor color)
{
    al_draw_filled_ellipse(xcenter, ycenter, radiusx, radiusy, arrayAllegroColors[color]);
}

void ECGraphicViewImp::DrawText(int xcenter, int ycenter, const char* ptext, ECGVColor color)
{
    al_draw_text(this->fontDef, arrayAllegroColors[color], xcenter, ycenter, ALLEGRO_ALIGN_CENTER, ptext);
}

void ECGraphicViewImp::DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int thickness, ECGVColor color) {
    al_draw_triangle(x1, y1, x2, y2, x3, y3, arrayAllegroColors[color], thickness);
}

void ECGraphicViewImp::DrawFilledTriangle(int x1, int y1, int x2, int y2, int x3, int y3, ECGVColor color) {
    al_draw_filled_triangle(x1, y1, x2, y2, x3, y3, arrayAllegroColors[color]);
}