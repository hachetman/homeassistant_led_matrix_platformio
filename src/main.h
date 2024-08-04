#ifndef HOMEASSISTANT_LED_MATRIX_MAIN_H
#define HOMEASSISTANT_LED_MATRIX_MAIN_H
#include "HAconnect.h"
#include <cstdint>
#include <Arduino.h>

enum class [[maybe_unused]] color : uint16_t {
  BLACK = 0x0000,
  RED = 0xf800,
  DIM_RED = 0x3800,
  GREEN = 0x07E0,
  DIM_GREEN = 0x01E0,
  YELLOW = 0xFFE0,
  DIM_YELLOW = 0x39E0,
  ORANGE = 0xFC00,
  DIM_ORANGE = 0x3900,
  WHITE = 0xFFFF,
  DIM_WHITE = 0x39E7,
};

enum class brightness : uint16_t { DAY = 0x0020, NIGHT = 0x0008 };
enum class co2_thresh : uint16_t { GOOD = 700, MEDIUM = 900, BAD = 1000 };
const int matrix_width = 64;
const int uart_speed = 115200;
const int spi_speed = 12000000;
const int delay_1s = 1000;
const int delay_500ms = 500;

const int stacksize = 10000;
const int task_prio = 1;

const uint16_t first_line = 26;
const uint16_t second_line = 36;
const uint16_t third_line = 46;
const uint16_t fourth_line = 56;

const int first_off = 0;
__attribute__((unused)) const int second_off = 4;
const int third_off = 9;

const char *ntpServer = "pool.ntp.org";

TaskHandle_t TaskHA = nullptr;
TaskHandle_t TaskMatrix = nullptr;
[[noreturn]] void TaskHA_update(void *pvParameters);
[[noreturn]] void TaskMatrix_update(void *pvParameters);

struct HaExchange matrix_data;
#endif // HOMEASSISTANT_LED_MATRIX_MAIN_H
