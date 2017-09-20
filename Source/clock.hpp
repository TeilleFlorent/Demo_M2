#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

using namespace std;


//******************************************************************************
//**********  Class Clock  *****************************************************
//******************************************************************************

class Clock
{

  public:


    // Clock functions
    // ---------------

    Clock();

    void TimeUpdate();


    // Clock class members
    // -------------------
    
    float _initial_time;
    float _current_time;
    float _delta_time;   

};