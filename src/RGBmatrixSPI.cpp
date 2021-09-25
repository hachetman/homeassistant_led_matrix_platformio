#include "RGBmatrixSPI.h"
#include "stdint.h"

#include <stdlib.h>     /* malloc, free, rand */
#include <sys/types.h>

RGBmatrixSPI::RGBmatrixSPI(int16_t width, int16_t height, int speed)
    : Adafruit_GFX(width, height), matrix_width(width), matrix_height(height),
      spi_speed(speed), spi(VSPI) {
  Serial.println("Initializing");
  spi.begin();
  pinMode(VSPI_SS, OUTPUT);
  frameBuffer = (uint8_t*)malloc(width * height * 2);
}
void RGBmatrixSPI::drawPixel(int16_t x, int16_t y, uint16_t c) {
  uint16_t address = 2* (x + matrix_width * y);
  *(frameBuffer + address) = (c & lower_bytes);
  *(frameBuffer + address + 1) = (c >> upper_shift);

}

void RGBmatrixSPI::setBrightness(int8_t brightness) {
  spi.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); // pull SS slow to prep other end for transfer
  spi.transfer(static_cast<uint8_t>(Registers::brightness));
  spi.transfer(brightness);
  digitalWrite(VSPI_SS, HIGH); // pull ss high to signify end of data transfer
  spi.endTransaction();
}
void RGBmatrixSPI::transfer() {
  spi.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); // pull SS slow to prep other end for transfer
  spi.transfer(static_cast<uint8_t>(Registers::transfer));
  spi.transfer(0x00);
  spi.transfer(0x00);
  for (int i = 0; i < matrix_width*matrix_height*2; i++) {
    spi.transfer(*(frameBuffer+i));
  }

  digitalWrite(VSPI_SS, HIGH); // pull ss high to signify end of data transfer
  spi.endTransaction();//
  spi.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); // pull SS slow to prep other end for transfer
  spi.transfer(static_cast<uint8_t>(Registers::buffer_switch));
  digitalWrite(VSPI_SS, HIGH); // pull ss high to signify end of data transfer                               //
  spi.endTransaction();
}
