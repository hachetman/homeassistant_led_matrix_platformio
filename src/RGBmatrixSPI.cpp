#include "RGBmatrixSPI.h"

RGBmatrixSPI::RGBmatrixSPI(int width, int height, int speed)
    : Adafruit_GFX(width, height), matrix_width(width), matrix_height(height),
      spi_speed(speed), spi(VSPI) {
  Serial.println("Initializing");
  spi.begin();
  pinMode(VSPI_SS,
          OUTPUT); // VSPI SS
}
void RGBmatrixSPI::drawPixel(int16_t x, int16_t y, uint16_t c) {
  uint16_t address = x + matrix_width * y;
  spi.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); // pull SS slow to prep other end for transfer
  spi.transfer(static_cast<uint8_t>(Registers::transfer));
  spi.transfer(address & lower_bytes);
  spi.transfer(address >> upper_shift);
  spi.transfer(c & lower_bytes);
  spi.transfer(c >> upper_shift);
  digitalWrite(VSPI_SS, HIGH); // pull ss high to signify end of data transfer
  spi.endTransaction();
}

void RGBmatrixSPI::setBrightness(int8_t brightness) {
  spi.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); // pull SS slow to prep other end for transfer
  spi.transfer(static_cast<uint8_t>(Registers::brightness));
  spi.transfer(brightness);
  digitalWrite(VSPI_SS, HIGH); // pull ss high to signify end of data transfer
  spi.endTransaction();
}
