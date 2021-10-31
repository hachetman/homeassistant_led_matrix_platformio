#include "HAconnect.h"
#include "HardwareSerial.h"
#include "RGBmatrixSPI.h"
#include "secrets.h"
#include "stdint.h"
#include "sys/unistd.h"
#include <Arduino.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <array>
#include <cstdint>
#include <cstdio>
#include <time.h>

#include <WiFi.h>

enum class color : uint16_t {
  BLACK = 0x0000,
  BLUE = 0x001f,
  RED = 0xf800,
  DIM_RED = 0x3800,
  GREEN = 0x07E0,
  DIM_GREEN = 0x01E0,
  CYAN = 0x07FF,
  MAGENTA = 0xF81f,
  YELLOW = 0xFFE0,
  DIM_YELLOW = 0x39E0,
  ORANGE = 0xFC00,
  DIM_ORANGE = 0x3900,
  WHITE = 0xFFFF,
  DIM_WHITE = 0x39E7,
};

enum class brightness : uint16_t { DAY = 0x0020, NIGHT = 0x0002 };
enum class co2_thresh : uint16_t { GOOD = 700, MEDIUM = 900, BAD = 1000 };
const int matrix_width = 64;
const int matrix_height = 64;
const int uart_speed = 115200;
const int spi_speed = 8000000;
const int delay_1s = 1000;
const int delay_500ms = 500;
const int delay_update = 5000;

const int stacksize = 10000;
const int task_prio = 1;

const uint16_t first_line = 26;
const uint16_t second_line = 36;
const uint16_t third_line = 46;
const uint16_t fourth_line = 56;

const uint16_t first_off = 0;
const uint16_t second_off = 4;
const uint16_t third_off = 9;

const char *ntpServer = "pool.ntp.org";

TaskHandle_t TaskHA = nullptr;
TaskHandle_t TaskMatrix = nullptr;
void TaskHA_update(void *pvParameters);
void TaskMatrix_update(void *pvParameters);

struct HaExchange matrix_data;
void setup() {

  Serial.begin(uart_speed);
  WiFiClass::mode(WIFI_MODE_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  Serial.print("Connecting to WiFi ..");
  while (WiFiClass::status() != WL_CONNECTED) {
    Serial.print('.');
    delay(delay_1s);
  }
  Serial.println(WiFi.localIP());
  // create a task that will be executed in the Task1code() function, with
  // priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
      TaskHA_update,    /* Task function. */
      "Task HA Update", /* name of task. */
      stacksize,        /* Stack size of task */
      &matrix_data,     /* parameter of the task */
      task_prio,        /* priority of the task */
      &TaskHA,          /* Task handle to keep track of created task */
      0);               /* pin task to core 0 */
  delay(delay_500ms);

  // create a task that will be executed in the Task2code() function, with
  // priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
      TaskMatrix_update,    /* Task function. */
      "Task Matrix Update", /* name of task. */
      stacksize,            /* Stack size of task */
      &matrix_data,         /* parameter of the task */
      task_prio,            /* priority of the task */
      &TaskMatrix,          /* Task handle to keep track of created task */
      1);                   /* pin task to core 1 */
  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void TaskHA_update(void *pvParameters) {
  auto *data = (struct HaExchange *)pvParameters;
  struct tm timeinfo;
  Serial.print("HA Update running on core ");
  Serial.println(xPortGetCoreID());
  HAconnect ha(address, port, auth);
  for (;;) {
    Serial.println("Updating sensors");
    data->co2 = ha.getState("sensor.co2");
    ha.getWeather(data);
    data->ping = ha.getPing();
    data->sun = ha.getSun();
    data->precipation =
        ha.getState("sensor.openweathermap_forecast_precipitation_probability");
    data->mintemp =
        ha.getState("sensor.openweathermap_forecast_temperature_low");
    data->maxtemp = ha.getState("sensor.openweathermap_forecast_temperature");
    getLocalTime(&timeinfo);
    if (timeinfo.tm_hour == 4 && timeinfo.tm_min == 0 &&
        timeinfo.tm_sec == 00) {
      ESP.restart();
    }
    delay(delay_update);
  }
}

uint16_t co2_color(int co2, bool sun) {
  if (co2 < static_cast<uint16_t>(co2_thresh::GOOD)) {
    if (sun) {
      return static_cast<uint16_t>(color::GREEN);
    } else {
      return static_cast<uint16_t>(color::DIM_GREEN);
    }
  }
  if (co2 < static_cast<uint16_t>(co2_thresh::MEDIUM)) {
    if (sun) {
      return static_cast<uint16_t>(color::YELLOW);
    } else {
      return static_cast<uint16_t>(color::DIM_YELLOW);
    }
  }
  if (co2 < static_cast<uint16_t>(co2_thresh::BAD)) {
    if (sun) {
      return static_cast<uint16_t>(color::ORANGE);
    } else {
      return static_cast<uint16_t>(color::DIM_ORANGE);
    }
  }
  if (sun) {
    return static_cast<uint16_t>(color::RED);
  } else {
    return static_cast<uint16_t>(color::DIM_RED);
  }
}
uint16_t get_text_color(bool sun) {
  if (sun) {
    return static_cast<uint16_t>(color::WHITE);
  } else {
    return static_cast<uint16_t>(color::DIM_WHITE);
  }
}
void TaskMatrix_update(void *pvParameters) {
  std::array<char, 10> buffer;
  int16_t str_len = 0;
  int16_t str_len2 = 0;
  HaExchange *data = (struct HaExchange *)pvParameters;
  Serial.print("Matrix update running on core ");
  Serial.println(xPortGetCoreID());
  RGBmatrixSPI matrix(matrix_width, matrix_height, spi_speed);
  struct tm timeinfo;
  matrix.setTextWrap(false);

  for (;;) {
    getLocalTime(&timeinfo);
    if (data->sun) {
      matrix.setBrightness(static_cast<uint16_t>(brightness::DAY));
    } else {
      matrix.setBrightness(static_cast<uint16_t>(brightness::NIGHT));
    }
    matrix.setTextColor(get_text_color(data->sun));
    matrix.fillRect(0, 0, matrix_width, matrix_height,
                    static_cast<uint16_t>(color::BLACK));
    matrix.setFont(&FreeMonoBold12pt7b);
    matrix.setCursor(0, 16);
    matrix.printf("%02d", timeinfo.tm_hour);
    matrix.setCursor(26, 14);
    if (timeinfo.tm_sec % 2) {
      matrix.printf(":");
    } else {
      matrix.printf(" ");
    }
    matrix.setCursor(36, 16);
    matrix.printf("%02d", timeinfo.tm_min);

    matrix.setFont();
    str_len = sprintf(buffer.data(), "%2d", data->temperature);
    matrix.setCursor(first_off, first_line);
    matrix.printf("T");
    matrix.setCursor(second_off, first_line);
    matrix.printf(":");
    matrix.setCursor(third_off, first_line);
    matrix.printf("%s", buffer.data());
    matrix.drawCircle(third_off + str_len * 6, 25, 1,
                      static_cast<uint16_t>(color::WHITE));

    matrix.setCursor(first_off + matrix_width / 2, first_line);
    matrix.printf("H");
    matrix.setCursor(second_off + matrix_width / 2, first_line);
    matrix.printf(":");
    matrix.setCursor(third_off + matrix_width / 2, first_line);
    matrix.printf("%02d%%", data->humidity);

    matrix.setCursor(first_off, second_line);
    matrix.printf("P");
    matrix.setCursor(second_off, second_line);
    matrix.printf(":");
    matrix.setCursor(third_off, second_line);
    matrix.printf("%2d", data->ping);

    matrix.setCursor(first_off + matrix_width / 2, second_line);
    matrix.printf("R");
    matrix.setCursor(second_off + matrix_width / 2, second_line);
    matrix.printf(":");
    matrix.setCursor(third_off + matrix_width / 2, second_line);
    matrix.printf("%02d%%", data->precipation);

    matrix.setTextColor(co2_color(data->co2, data->sun));
    matrix.setCursor(10, third_line);
    matrix.printf("CO2");
    matrix.setCursor(28, third_line);
    matrix.printf(":");
    matrix.setCursor(34, third_line);
    matrix.printf("%d", data->co2);

    matrix.setTextColor(get_text_color(data->sun));
    str_len = sprintf(buffer.data(), "%2d", data->mintemp);
    str_len2 = sprintf(buffer.data(), "%2d", data->maxtemp);
    matrix.setCursor(9, fourth_line);
    matrix.printf("%2d", data->mintemp);
    matrix.setCursor(9 + str_len * 6 + 3, fourth_line);
    matrix.printf("->");
    matrix.setCursor(9 + str_len * 6 + 18, fourth_line);
    matrix.printf("%2d", data->maxtemp);
    matrix.drawCircle(9 + str_len * 6, 55, 1,
                      static_cast<uint16_t>(color::WHITE));
    matrix.drawCircle(9 + str_len * 6 + 18 + str_len2 * 6, 55, 1,
                      static_cast<uint16_t>(color::WHITE));
    matrix.transfer();
  }
}

void loop() {}
// end loop
