#include <SDL2/SDL.h>

#include <iostream>

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

    void PrintState();


    // Clock class members
    // -------------------
    
    float _initial_time;
    float _current_time;
    float _delta_time;   

};