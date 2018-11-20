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
#include "ULPProgrammer.h"
#include <string.h>

void ULPProgrammer::insert_loop_start()
{
  const ulp_insn_t loop_preamble[] = {
    I_MOVI(R3, 0),
    M_LABEL(1)
  };
  copy_instructions(&loop_preamble[0], sizeof(loop_preamble)/sizeof(ulp_insn_t));
}

void ULPProgrammer::insert_loop_end(int32_t finalDelayClocks)
{
  const ulp_insn_t loop_postamble[] = {
    I_LD(R0, R3, 1),        // R0 <- RTC_SLOW_MEM[R3+1], R3 should be 0
    M_BL(2, 1),             // goto label 2 if R0 < 1
    M_BX(1),
    M_LABEL(2)
  };
  insert_delay(finalDelayClocks-5);
  copy_instructions(&loop_postamble[0], sizeof(loop_postamble)/sizeof(ulp_insn_t));
}

void ULPProgrammer::insert_program_end()
{
  const ulp_insn_t halt[] = {
    I_ST(R3, R3, 0),    // RTC_SLOW_MEM[R3+0] = R3
    I_END(),
    I_HALT()
  };
  copy_instructions(&halt[0], sizeof(halt)/sizeof(ulp_insn_t));
}


size_t ULPProgrammer::nextIP()
{
  if ( ip < max )
    return ip++;
  return max-1;
}

void ULPProgrammer::copy_instructions(const ulp_insn_t *prog, unsigned int count)
{
  for ( int x = 0; x < count; x ++ )
  {
      program[nextIP()] = prog[x];
  }
}

void ULPProgrammer::insert_gpio( uint32_t n, uint32_t on, int32_t &clockCounter)
{
  const ulp_insn_t gpio[] = {
    I_WR_REG(RTC_GPIO_OUT_REG, n, n, on) // RTC_GPIO_X + 14 = bit
  };
  clockCounter--;

  program[nextIP()] = gpio[0];
}

void ULPProgrammer::insert_small_delay(uint16_t clocks)
{
  const ulp_insn_t delay[] = {
      I_DELAY(clocks)
  };
  program[nextIP()] = delay[0];
}

void ULPProgrammer::insert_delay(int32_t clocks) {
  while ( clocks > 65535 )
  {
    insert_small_delay(65535);
    clocks -= 65535;
  }
  if ( clocks > 0 )
    insert_small_delay((uint16_t)clocks);
}

ULPProgrammer::ULPProgrammer()
{
  max = 2048;
  program = new ulp_insn_t[max];
  ip = 0;
}

bool ULPProgrammer::createProgram()
{
  stopProgram();
  clearMemory();
  if ( ip < max )
  {
    auto err = ulp_process_macros_and_load(32, program, &ip);
    if (err == ESP_OK)
    {
      RTC_SLOW_MEM[2] = 1; // indicate program loaded.
      return true;
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
  return false;
}

ULPProgrammer::~ULPProgrammer()
{
  delete [] program;
}

void ULPProgrammer::clearMemory()
{
  memset(RTC_SLOW_MEM, 0, CONFIG_ULP_COPROC_RESERVE_MEM);
}

bool ULPProgrammer::isProgramRunning()
{
  return ( RTC_SLOW_MEM[0] & UINT16_MAX ) > 0;
}

bool ULPProgrammer::isProgramLoaded()
{
  return RTC_SLOW_MEM[2];
}

void ULPProgrammer::stopProgram()
{
  if ( isProgramRunning() && isProgramLoaded() )
  {
    ESP_LOGI("ULPProg", "Stopping program");
    RTC_SLOW_MEM[1] = 0; // signal stop
    while ( isProgramRunning() )
      ets_delay_us(100);
    ESP_LOGI("ULPProg", "Stopped");
  }
}

void ULPProgrammer::signalStopProgram()
{
  if ( isProgramRunning() )
  {
    RTC_SLOW_MEM[1] = 0; // signal stop
  }
}

bool ULPProgrammer::startProgram()
{
  if ( !isProgramRunning() && isProgramLoaded() )
  {
    RTC_SLOW_MEM[0] = 1;
    RTC_SLOW_MEM[1] = 1;
    auto err = ulp_run(32);
    if ( err == ESP_OK )
      return true;
  }
  return false;
}
