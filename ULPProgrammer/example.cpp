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

// Don't forget to change CONFIG_ULP_COPROC_RESERVE_MEM to a reasonable value to
// hold your program. 1024 should be plenty, higher if needed.

#include "ULPProgrammer.h"


class ULP_PWM : public ULPProgrammer
{
public:
  ULP_PWM() : ULPProgrammer()
  {

  }

  bool PWM( gpio_num_t gpio, double f, double d )
  {
    if ( rtc_gpio_is_valid_gpio(gpio) )
    {
      signalStopProgram(); // notify ULP it should stop.

      ip = 0; // reset for reprogramming.

      insert_loop_start();

      uint32_t rtc_8md256_period = rtc_clk_cal(RTC_CAL_8MD256, 100);
      uint32_t rtc_fast_freq_hz = 1000000ULL * (1 << RTC_CLK_CAL_FRACT) * 256 / rtc_8md256_period;
      ESP_LOGI("ULP_PWM", "Found clock frequency %u", rtc_fast_freq_hz);
      uint32_t clocks = rtc_fast_freq_hz/f;

      int32_t onTime = clocks * d;
      int32_t offTime = clocks - onTime;

      insert_gpio(rtc_gpio_desc[gpio].rtc_num+14,1,onTime);
      insert_delay(onTime);
      insert_gpio(rtc_gpio_desc[gpio].rtc_num+14,0,offTime);
      insert_loop_end(offTime);
      insert_program_end();
      
      stopProgram(); // Hard stop

      if ( createProgram() )
      {
          startProgram();
      }
      else
      {
        ESP_LOGI("ULP_PWM", "Problem creating program.");
      }
    }
    else
    {
      ESP_LOGI("ULP_PWM", "GPIO must be an RTC_GPIO.");
    }
    return false;
  }
};

void test_ulp()
{
  ESP_ERROR_CHECK(rtc_gpio_init(GPIO_NUM_2));
  ESP_ERROR_CHECK(rtc_gpio_set_direction(GPIO_NUM_2, RTC_GPIO_MODE_OUTPUT_ONLY));
  ESP_ERROR_CHECK(rtc_gpio_set_level(GPIO_NUM_2,0));

  ULP_PWM ulp;
  ulp.PWM(GPIO_NUM_2, 100, 0.01);
}
