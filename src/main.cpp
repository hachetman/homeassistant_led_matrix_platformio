#include "HAconnect.h"
#include "HardwareSerial.h"
#include "RGBmatrixSPI.h"
#include "secrets.h"
#include <Arduino.h>

#include <WiFi.h>
const int matrix_width = 64;
const int matrix_height = 64;
const int uart_speed = 115200;
const int spi_speed = 10000;
const int delay_1s = 1000;
const int initial_brightness = 64;
void setup() {

  Serial.begin(uart_speed);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(delay_1s);
  }
  Serial.println(WiFi.localIP());

  RGBmatrixSPI matrix(matrix_width, matrix_height, spi_speed);
  matrix.setBrightness(initial_brightness);
  matrix.println("Hi Mopsi");

  HAconnect ha(address, port, auth);
  int test = ha.getState("sensor.co2");
  Serial.println(test);
}
void loop() {}
// end loop
