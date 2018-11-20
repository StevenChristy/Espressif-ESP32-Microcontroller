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
