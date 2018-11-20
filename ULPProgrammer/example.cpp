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

      int32_t clocks = 8000000/f;
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
