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

#include <rom/ets_sys.h>
#include <rom/gpio.h>
#include <esp32/ulp.h>
#include <soc/soc.h>
#include <soc/rtc.h>
#include <soc/rtc_cntl_reg.h>
#include <driver/rtc_io.h>
#include <esp_log.h>

const ulp_insn_t preamble[] = {
  M_LABEL(1)
};
const ulp_insn_t postamble[] = {
  M_BX(1),                // goto 1
  I_HALT()
};

class ULPProgrammer
{
private:
  ulp_insn_t *program;
  size_t ip;
  size_t max;

  size_t nextIP()
  {
    if ( ip < max )
      return ip++;
    return max-1;
  }

  void copy_instructions(const ulp_insn_t *prog, unsigned int count)
  {
    for ( int x = 0; x < count; x ++ )
    {
        program[nextIP()] = prog[x];
    }
  }
  void insert_gpio( uint32_t n, uint32_t on, int32_t &clockCounter)
  {
    const ulp_insn_t gpio[] = {
      I_WR_REG(RTC_GPIO_OUT_REG, n, n, on) // RTC_GPIO_X + 14 = bit
    };
    clockCounter--;

    program[nextIP()] = gpio[0];
  }

  void insert_small_delay(uint16_t clocks)
  {
    const ulp_insn_t delay[] = {
        I_DELAY(clocks)
    };
    program[nextIP()] = delay[0];
  }

  void insert_delay(int32_t clocks) {
    while ( clocks > 65535 )
    {
      insert_small_delay(65535);
      clocks -= 65535;
    }
    if ( clocks > 0 )
      insert_small_delay((uint16_t)clocks);
  }

public:
  ULPProgrammer()
  {
    max = 2048;
    program = new ulp_insn_t[max];
    ip = 0;
  }

  bool PWM( gpio_num_t gpio, double f, double d )
  {
    if ( rtc_gpio_is_valid_gpio(gpio) )
    {
      copy_instructions(&preamble[0], sizeof(preamble)/sizeof(ulp_insn_t));

      int32_t clocks = 8000000/f;
      int32_t onTime = clocks * d;
      int32_t offTime = clocks - onTime;

      insert_gpio(rtc_gpio_desc[gpio].rtc_num+14,1,onTime);
      insert_delay(onTime);
      insert_gpio(rtc_gpio_desc[gpio].rtc_num+14,0,offTime);
      insert_delay(offTime-1); // -1 for the jump to beginning
      copy_instructions(&postamble[0], sizeof(postamble)/sizeof(ulp_insn_t));
      return true;
    }
    else
    {
      ESP_LOGI("ULPProg", "GPIO must be an RTC_GPIO.");
    }
    return false;
  }

  void Program()
  {
    if ( ip < max )
    {
      auto err = ulp_process_macros_and_load(0, program, &ip);
      if (err == ESP_OK)
      {
        ESP_ERROR_CHECK(ulp_run(0));
      }
      else
      {
        ESP_ERROR_CHECK(err);
      }
    }
    else
    {
      ESP_LOGI("ULPProg", "Program size overflow.");
    }
  }

  ~ULPProgrammer()
  {
    delete [] program;
  }
};

void test_ulp()
{
  ESP_ERROR_CHECK(rtc_gpio_init(GPIO_NUM_2));
  ESP_ERROR_CHECK(rtc_gpio_set_direction(GPIO_NUM_2, RTC_GPIO_MODE_OUTPUT_ONLY));
  ESP_ERROR_CHECK(rtc_gpio_set_level(GPIO_NUM_2,0));

  ULPProgrammer ulp;
  if ( ulp.PWM(GPIO_NUM_2, 100, 0.01) )
  {
    ulp.Program();
  }
}
