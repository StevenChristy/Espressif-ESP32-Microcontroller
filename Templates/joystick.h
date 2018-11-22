/*
Copyright (c) 2018 Steven Christy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once
#include "task.h"
#include <driver/adc.h>

class JoystickModule : public Timer< JoystickModule >
{
protected:
    adc1_channel_t xchan;
    adc1_channel_t ychan;
    gpio_num_t sw;
    uint8_t stickyState;
public:
    enum ButtonStates {SWITCH=1, LEFT=2, RIGHT=4, UP=8, DOWN=16 };
    uint8_t state;
    double xAxis, yAxis;

    JoystickModule(adc1_channel_t xchannel, adc1_channel_t ychannel, gpio_num_t stickswitch)
      : xchan(xchannel), ychan(ychannel), sw(stickswitch)
    {
      stickyState = state = xAxis = yAxis = 0;
    }

    void start()
    {
        adc1_config_channel_atten(xchan,ADC_ATTEN_DB_11);
        adc1_config_channel_atten(ychan,ADC_ATTEN_DB_11);
        gpio_set_pull_mode(sw,GPIO_PULLUP_ONLY);
        gpio_set_direction(sw,GPIO_MODE_INPUT);
        setInterval(25);
        startTask("joystick", 3072);
    }

    void tick()
    {
        uint16_t xpos = adc1_get_raw(xchan);
        uint16_t ypos = adc1_get_raw(ychan);
        uint16_t swst = gpio_get_level(sw);
        uint8_t newStates = 0;
        if ( xpos > (2048+512) )
        {
          newStates |= ButtonStates::LEFT;
          xAxis = (double)(xpos - 2560.0)/1536.0;
        }
        else if ( xpos < (2048-512) )
        {
          newStates |= ButtonStates::RIGHT;
          xAxis = -1.0 + (double)xpos/1536.0;
        }
        else
        {
          xAxis = 0;
        }

        if ( ypos > (2048+512) )
        {
          newStates |= ButtonStates::DOWN;
          yAxis = (double)(ypos - 2560.0)/1536.0;
        }
        else if ( ypos < (2048-512) )
        {
          newStates |= ButtonStates::UP;
          yAxis = -1.0 + (double)ypos/1536.0;
        }
        else
        {
          yAxis = 0;
        }

        if ( !swst )
          newStates |= ButtonStates::SWITCH;

        state = newStates;
        stickyState |= state;
    }

    uint8_t getButtonChanges()
    {
      static uint8_t states = 0;
      uint8_t result = (~states) & stickyState;
      states = stickyState;
      stickyState = 0;
      if ( (result & (JoystickModule::ButtonStates::RIGHT|JoystickModule::ButtonStates::LEFT)) == (JoystickModule::ButtonStates::RIGHT|JoystickModule::ButtonStates::LEFT) )
      {
        result &= ~(JoystickModule::ButtonStates::RIGHT|JoystickModule::ButtonStates::LEFT);
      }
      if ( (result & (JoystickModule::ButtonStates::UP|JoystickModule::ButtonStates::DOWN)) == (JoystickModule::ButtonStates::UP|JoystickModule::ButtonStates::DOWN) )
      {
        result &= ~(JoystickModule::ButtonStates::UP|JoystickModule::ButtonStates::DOWN);
      }
      return result;
    }
};
