#ifndef CLOCK_H
#define CLOCK_H

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

    float GetCurrentTime();

    float GetDeltaTime();
    
    void PrintState();


    // Clock class members
    // -------------------
    
    float _initial_time;
    float _current_time;
    float _delta_time;   

};

#endif  // CLOCK_H
