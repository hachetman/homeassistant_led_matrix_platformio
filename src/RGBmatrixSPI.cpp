#include "RGBmatrixSPI.h"
#include "HardwareSerial.h"
#include <SPI.h>
RGBmatrixSPI::RGBmatrixSPI(int width, int height, int speed)
    : Adafruit_GFX(width, height), matrix_width(width), matrix_height(height),
      spi_speed(speed) {
  Serial.println("Initializing");
  vspi = new SPIClass(VSPI);
  vspi->begin();
  pinMode(VSPI_SS,
          OUTPUT); // VSPI SS
}
void RGBmatrixSPI::drawPixel(int16_t x, int16_t y, uint16_t c) {
  int address = x + matrix_width * y;
  vspi->beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); // pull SS slow to prep other end for transfer
  vspi->transfer(0xf3);
  vspi->transfer(address & 0xff);
  vspi->transfer((address & 0xff00) >> 8);
  vspi->transfer(c & 0xff);
  vspi->transfer((c & 0xff00) >> 8);
  digitalWrite(VSPI_SS, HIGH); // pull ss high to signify end of data transfer
  vspi->endTransaction();
}

void RGBmatrixSPI::setBrightness(int8_t brightness) {
  vspi->beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); // pull SS slow to prep other end for transfer
  vspi->transfer(0xf1);
  vspi->transfer(brightness);
  digitalWrite(VSPI_SS, HIGH); // pull ss high to signify end of data transfer
  vspi->endTransaction();
}
