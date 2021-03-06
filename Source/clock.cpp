#include "clock.hpp"


//******************************************************************************
//**********  Class Clock  *****************************************************
//******************************************************************************

Clock::Clock()
{
  _initial_time = ( float )SDL_GetTicks() / 1000.0;
  _current_time = 0.0f;
  _delta_time   = 0.0f;
  TimeUpdate();
}

float Clock::GetCurrentTime()
{
  return _current_time;
}

float Clock::GetDeltaTime()
{
  return _delta_time;
}

void Clock::TimeUpdate()
{ 
  float new_current_time = ( ( float )SDL_GetTicks() - _initial_time ) / 1000.0; 
  _delta_time            = ( new_current_time - _current_time ); 
  _current_time          = new_current_time;
}

void Clock::PrintState()
{ 
  TimeUpdate();

  std::cout << "\nClock Status :" << std::endl 
            <<   "--------------" << std::endl;
  std::cout << "Current_time = " << _current_time << std::endl;
  std::cout << "Delta_time   = " << _delta_time   << std::endl;
  std::cout << std::endl;
}