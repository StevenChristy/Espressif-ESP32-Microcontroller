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

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <rom/ets_sys.h>

template<class T> class Task
{
protected:
  void startTask( const char *taskName, uint16_t stackSize, UBaseType_t priority = tskIDLE_PRIORITY+1 )
  {
    int ret;
    ret = xTaskCreate(&task,
                      taskName,
                      stackSize,
                      this,
                      priority,
                      &Handle);

    if (ret != pdPASS)  {
        ESP_LOGI("TASK", "Create task %s failed.", taskName);
    }
  }

public:
    xTaskHandle Handle;
    bool freeOnTerminate;

    Task()
    {
      freeOnTerminate = false;
      Handle = nullptr;
    }

    virtual ~Task()
    {
      if ( Handle )
      {
          vTaskDelete(Handle);
      }
    }

    inline void snooze()
    {
      vTaskDelay((1000 / CONFIG_FREERTOS_HZ) / portTICK_PERIOD_MS);
    }

private:
    static void task(void *p)
    {
      if ( p )
      {
        ((T*)p)->run();
        ((T*)p)->Handle = nullptr;
        if ( ((T*)p)->freeOnTerminate )
          delete ((T*)p);
      }
      else
      {
        ESP_LOGI("TASK", "Task::task with nullptr.");
      }
      vTaskDelete(nullptr);
    }
};


template<class T> class Timer: public Task<T>
{
protected:
    int32_t m_timeTaken;
    int64_t m_start;
private:
    friend Task<T>;
    int32_t m_delay;
    int64_t m_when;

    void run()
    {
        m_when = esp_timer_get_time();
        while (1)
        {
          m_start = esp_timer_get_time();
          m_when = m_when + m_delay;
          ((T*)this)->tick();
          int64_t now = esp_timer_get_time();
          m_timeTaken = now - m_start;
          int32_t delta = m_when - now;
          if ( delta < 0 )
          {
              m_when = now;
          }
          else
          {
              vTaskDelay(delta / portTICK_PERIOD_MS / 1000);
          }
        }
    }

public:
    Timer() : m_delay(50)
    {
    }

    void setInterval( int32_t delay_ms )
    {
       m_delay = delay_ms*1000;
    }

};
