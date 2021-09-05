#include "RGBmatrixSPI.h"
#include <Arduino.h>

void setup() {

  Serial.begin(115200);

  Serial.println(F("*****************************************************"));
  Serial.println(F("*        ESP32-HUB75-MatrixPanel-I2S-DMA DEMO       *"));
  Serial.println(F("*****************************************************"));

  RGBmatrixSPI matrix(64, 64, 10000);
  matrix.setBrightness(64);
  matrix.println("Hi Mopsi");
}
void loop() {}
// end loop
