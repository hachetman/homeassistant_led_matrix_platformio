#include "HAconnect.h"
#include "HardwareSerial.h"
#include "RGBmatrixSPI.h"
#include "secrets.h"
#include "stdint.h"
#include "stdio.h"
#include <Arduino.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <NTPClient.h>

#include <WiFi.h>

enum class color : uint16_t {
  BLACK = 0x0000,
  BLUE = 0x001f,
  RED = 0xf800,
  GREEN = 0x07E0,
  CYAN = 0x07FF,
  MAGENTA = 0xF81f,
  YELLOW = 0xFFE0,
  ORANGE = 0xFC00,
  WHITE = 0xFFFF
};

enum class brightness : uint16_t { DAY = 0x0020, NIGHT = 0x0003 };
const int matrix_width = 64;
const int matrix_height = 64;
const int uart_speed = 115200;
const int spi_speed = 8000000;
const int delay_1s = 1000;
TaskHandle_t TaskHA;
TaskHandle_t TaskMatrix;
void TaskHA_update(void *pvParameters);
void TaskMatrix_update(void *pvParameters);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 7200, 60000);

struct HaExchange matrix_data;
void setup() {

  matrix_data.co2 = 1000;
  matrix_data.temperature = 7;
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
      10000,            /* Stack size of task */
      &matrix_data,     /* parameter of the task */
      1,                /* priority of the task */
      &TaskHA,          /* Task handle to keep track of created task */
      0);               /* pin task to core 0 */
  delay(500);

  // create a task that will be executed in the Task2code() function, with
  // priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
      TaskMatrix_update, /* Task function. */
      "Task2",           /* name of task. */
      10000,             /* Stack size of task */
      &matrix_data,      /* parameter of the task */
      1,                 /* priority of the task */
      &TaskMatrix,       /* Task handle to keep track of created task */
      1);                /* pin task to core 1 */
  timeClient.begin();
  timeClient.update();
}

void TaskHA_update(void *pvParameters) {
  HaExchange *data = (struct HaExchange *)pvParameters;
  Serial.print("HA Update running on core ");
  Serial.println(xPortGetCoreID());
  HAconnect ha(address, port, auth);
  for (;;) {
    delay(1000);
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
    if (timeClient.getHours() == 4 && timeClient.getMinutes() == 0 &&
        timeClient.getSeconds() == 00) {
      ESP.restart();
    }
  }
}

uint16_t co2_color(int co2) {
  if (co2 < 700) {
    return static_cast<uint16_t>(color::GREEN);
  }
  if (co2 < 900) {
    return static_cast<uint16_t>(color::YELLOW);
  }
  if (co2 < 1100) {
    return static_cast<uint16_t>(color::ORANGE);
  }
  return static_cast<uint16_t>(color::RED);
}
void TaskMatrix_update(void *pvParameters) {
  char buffer[10];
  int str_len;
  int str_len2;
  HaExchange *data = (struct HaExchange *)pvParameters;
  Serial.print("Matrix update running on core ");
  Serial.println(xPortGetCoreID());
  RGBmatrixSPI matrix(matrix_width, matrix_height, spi_speed);

  matrix.setBrightness(static_cast<uint16_t>(brightness::NIGHT));
  matrix.fillRect(0, 0, 64, 64, 0x00);
  for (;;) {
    if (data->sun) {
      matrix.setBrightness(static_cast<uint16_t>(brightness::DAY));
    } else {
      matrix.setBrightness(static_cast<uint16_t>(brightness::NIGHT));
    }
    matrix.fillRect(0, 0, 64, 64, 0x00);
    matrix.setFont(&FreeMonoBold12pt7b);
    matrix.setCursor(0, 16);
    matrix.printf("%02d", timeClient.getHours());
    matrix.setCursor(26, 14);
    if (timeClient.getSeconds() % 2) {
      matrix.printf(":");
    } else {
      matrix.printf(" ");
    }
    matrix.setCursor(36, 16);
    matrix.printf("%02d", timeClient.getMinutes());

    matrix.setFont();
    str_len = sprintf(buffer, "%2d", data->temperature);
    matrix.setCursor(1, 26);
    matrix.printf("T");
    matrix.setCursor(6, 26);
    matrix.printf(":");
    matrix.setCursor(11, 26);
    matrix.printf("%s", buffer);
    matrix.drawCircle(11 + str_len * 6, 25, 1, 0xffff);

    matrix.setCursor(33, 26);
    matrix.printf("H");
    matrix.setCursor(38, 26);
    matrix.printf(":");
    matrix.setCursor(43, 26);
    matrix.printf("%02d%%", data->humidity);

    matrix.setCursor(1, 36);
    matrix.printf("P");
    matrix.setCursor(6, 36);
    matrix.printf(":");
    matrix.setCursor(11, 36);
    matrix.printf("%2d", data->ping);

    matrix.setCursor(33, 36);
    matrix.printf("R");
    matrix.setCursor(38, 36);
    matrix.printf(":");
    matrix.setCursor(43, 36);
    matrix.printf("%02d%%", data->precipation);

    matrix.setTextColor(co2_color(data->co2));
    matrix.setCursor(10, 46);
    matrix.printf("CO2");
    matrix.setCursor(28, 46);
    matrix.printf(":");
    matrix.setCursor(34, 46);
    matrix.printf("%d", data->co2);

    matrix.setTextColor(static_cast<uint16_t>(color::WHITE));
    str_len = sprintf(buffer, "%2d", data->mintemp);
    str_len2 = sprintf(buffer, "%2d", data->maxtemp);
    matrix.setCursor(9, 56);
    matrix.printf("%2d", data->mintemp);
    matrix.setCursor(9 + str_len * 6 + 3, 56);
    matrix.printf("->");
    matrix.setCursor(9 + str_len * 6 + 18, 56);
    matrix.printf("%2d", data->maxtemp);
    matrix.drawCircle(9 + str_len * 6, 55, 1, 0xffff);
    matrix.drawCircle(9 + str_len * 6 + 18 + str_len2 * 6, 55, 1, 0xffff);

    matrix.transfer();
  }
}

void loop() {}
// end loop
