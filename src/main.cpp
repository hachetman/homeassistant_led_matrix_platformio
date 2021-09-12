#include "HAconnect.h"
#include "HardwareSerial.h"
#include "RGBmatrixSPI.h"
#include "secrets.h"
#include <Arduino.h>
#include <NTPClient.h>

#include <WiFi.h>
const int matrix_width = 64;
const int matrix_height = 64;
const int uart_speed = 115200;
const int spi_speed = 10000;
const int delay_1s = 1000;
const int initial_brightness = 64;
TaskHandle_t TaskHA;
TaskHandle_t TaskMatrix;
void TaskHA_update(void *pvParameters);
void TaskMatrix_update(void *pvParameters);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 7200, 60000);


struct MatrixData {
  int co2;
  int temperature;
} matrix_data;
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
      &matrix_data,             /* parameter of the task */
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
      &matrix_data,              /* parameter of the task */
      1,                 /* priority of the task */
      &TaskMatrix,       /* Task handle to keep track of created task */
      1);                /* pin task to core 1 */
  timeClient.begin();
  timeClient.update();
}

void TaskHA_update(void *pvParameters) {
  MatrixData * data = (struct MatrixData*) pvParameters;
  Serial.print("HA Update running on core ");
  Serial.println(xPortGetCoreID());
  int counter = 0;
  HAconnect ha(address, port, auth);
  for (;;) {
    delay(1000);
    data->co2 = counter;
    Serial.println("Updating sensor ");
    Serial.println(counter);
    data->temperature = ha.getState("sensor.co2");
    counter ++;
  }
}

void TaskMatrix_update(void *pvParameters) {
  MatrixData * data = (struct MatrixData*) pvParameters;
  Serial.print("Matrix update running on core ");
  Serial.println(xPortGetCoreID());
  RGBmatrixSPI matrix(matrix_width, matrix_height, spi_speed);
  matrix.setBrightness(initial_brightness);
  matrix.println("Hi Mopsi");
  for (;;) {
    Serial.print("CO2: ");
    Serial.println(data->co2);
    Serial.print("COO2: ");
    Serial.println(data->temperature);
    Serial.println(timeClient.getFormattedTime());
    delay(700);
  }
}

void loop() {}
// end loop
