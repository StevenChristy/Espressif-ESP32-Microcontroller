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

#include <rom/ets_sys.h>
#include <rom/gpio.h>
#include <esp32/ulp.h>
#include <soc/soc.h>
#include <soc/rtc.h>
#include <soc/rtc_cntl_reg.h>
#include <driver/rtc_io.h>
#include <esp_log.h>

class ULPProgrammer
{
protected:
  ulp_insn_t *program;
  size_t ip;
  size_t max;

  size_t nextIP();
  void copy_instructions(const ulp_insn_t *prog, unsigned int count);
  void insert_gpio( uint32_t n, uint32_t on, int32_t &clockCounter);
  void insert_small_delay(uint16_t clocks);
  void insert_delay(int32_t clocks);
  void insert_loop_start();
  void insert_loop_end(int32_t finalDelayClocks);
  void insert_program_end();
  
public:
  ULPProgrammer();
  virtual ~ULPProgrammer();

  void clearMemory();
  bool createProgram();
  bool startProgram();
  bool isProgramRunning();
  bool isProgramLoaded();
  void stopProgram();
  void signalStopProgram();
};
